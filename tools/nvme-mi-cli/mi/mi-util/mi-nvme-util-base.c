/*
 * Copyright (C) 2021 Samsung Electronics Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * mi-nvme-util-base.c - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-util-master.h"

void print_mctp_packet(mctp_i2c_header *m)
{
    printf("\t\t\t Destination Addr    : 0x%02X (7-bit: 0x%02x)\n",
        m->destAddr, m->destAddr >> 1);
    printf("\t\t\t Source      Addr    : 0x%02X (7-bit: 0x%02x)\n",
        m->srcAddr & 0xFE, m->srcAddr >> 1);
    printf("\t\t\t Byte Count          : 0x%02X (%d dec)\n",
        m->byteCnt, m->byteCnt);
    printf("\t\t\t MCTP Header Version : %d\n", m->headerVer & 0xF);
    printf("\t\t\t Destination Endpoint: 0x%02X\n", m->destEID);
    printf("\t\t\t Source      Endpoint: 0x%02X\n", m->srcEID);
    printf("\t\t\t Message Tag         : 0x%X\n", m->pktCtrl &
        MCTP_CTRL_PKT_MSGTAG_MASK);
    printf("\t\t\t MCTP Pkt Seq Num    : %d\n", (m->pktCtrl &
        MCTP_CTRL_PKT_PKTSEQ_MASK) >> MCTP_CTRL_PKT_PKTSEQ_SHIFT);
    printf("\t\t\t Packet Control bits : SOM(%d), EOM(%d), TO(%d)\n",
        (m->pktCtrl & MCTP_CTRL_PKT_SOM) >> MCTP_CTRL_PKT_SOM_SHIFT,
        (m->pktCtrl & MCTP_CTRL_PKT_EOM) >> MCTP_CTRL_PKT_EOM_SHIFT,
        (m->pktCtrl & MCTP_CTRL_PKT_TO) >> MCTP_CTRL_PKT_TO_SHIFT);
}

void format_base_pkt(mctp_message_t *m)
{
    /* Prepare the I2C header */
    m->i2cHdr.cmdCode = MCTP_CMD_CODE;
    m->i2cHdr.headerVer = MCTP_HEADER_VER;
    m->i2cHdr.destEID = MCTP_EID_DESTINATION;
    m->i2cHdr.srcEID = MCTP_EID_SOURCE;
    m->i2cHdr.pktCtrl = MCTP_CTRL_MSGTAG(3) | MCTP_CTRL_PKT_SOM |
        MCTP_CTRL_PKT_EOM | MCTP_CTRL_PKT_TO | 1;
}

int rcv_pkt(void *inp_parm)
{
    unsigned int status = 0;
    uint8_t eom = 0;
    mctp_i2c_header mctp_hdr;
    rcv_parm_t *rcv_parm = (rcv_parm_t *)inp_parm;

    printf("Getting Data from the device.\n");

    rcv_parm->buf_size = 0;
    memset(&mctp_hdr, 0, sizeof(mctp_i2c_header));
    uint32_t bytesread = 0;
    while (!eom) {
        uint8_t buf[8];
        /*Reading first 8 bytes of header info*/
        int ret = objHAL->hal_i2c_read(buf, 8);
        if (ret == -1) {
            printf("Unable to receive MI response header from device.\n");
            status = -1;
            break;
        }
        mctp_hdr.byteCnt = buf[2];
        eom = (buf[7] & 0x40) >> 6;
        printf("Header info received from device:\n");
        print_mctp_packet((mctp_i2c_header *)buf);

        /*copy header info to response buffer*/
        memcpy(rcv_parm->buffer + bytesread, buf, 8);
        bytesread += 8;

        /*Reading the data sent in next transaction*/
        ret = objHAL->hal_i2c_read(rcv_parm->buffer + bytesread,
            mctp_hdr.byteCnt - 4);
        if (ret == -1) {
            printf("Unable to receive data from the device.\n");
            status = -1;
            break;
        }
        printf("Data Received from Device:\n");
        rcv_parm->buf_size += mctp_hdr.byteCnt - 4;
        bytesread += mctp_hdr.byteCnt - 4;
    }

    *(rcv_parm->ret_bytes) = rcv_parm->buf_size;
    return status;
}

int xmit_pkt(__u8 *buffer)
{
    mctp_message_t *message = NULL;
    __u8 *p = NULL;
    __u32 mtu = mctp_tus;
    int bytesleft = 0;
    int ret = -1;

    message = (mctp_message_t *)buffer;
    bytesleft = TotalByteCount - 5;
    /* Append PEC byte to the end of the packet */
    p = buffer + TotalByteCount + 3;
    message->i2cHdr.byteCnt = TotalByteCount;
    *p = Calc_Crc8((__u8 *)buffer, TotalByteCount + 3);

    *p ^= *buffer;
    
    usleep(10);

    print_mctp_packet(&message->i2cHdr);

    if (bytesleft <= mtu) {
       ret = send_data(TotalByteCount + 3,
             (__u8 *)&message->i2cHdr.cmdCode);
    } else {
        bool isStart = true;
        __u32 counter = 0;
        do {
            mctp_message_t msg_in_chunks;
            __u8 *buffer_in_chunks;

            buffer_in_chunks = (__u8 *)&msg_in_chunks;

            memcpy(&msg_in_chunks, message, MCTP_HEADER_SIZE);
            msg_in_chunks.i2cHdr.byteCnt = mtu + 5;

            msg_in_chunks.i2cHdr.pktCtrl &= 0x0F;
            msg_in_chunks.i2cHdr.pktCtrl |= ((counter % 4) << 4);

            if (isStart == true) {
                msg_in_chunks.i2cHdr.pktCtrl |= 0x80; /*Start of message*/
                memcpy(&msg_in_chunks.msgHdr, buffer +
                    MCTP_HEADER_SIZE + (mtu * counter), mtu);
                isStart = false;
            } else if (bytesleft <= mtu) {
                msg_in_chunks.i2cHdr.pktCtrl |= 0x40; /*End of message*/
                msg_in_chunks.i2cHdr.byteCnt = bytesleft + 5;
                memcpy(&msg_in_chunks.msgHdr, buffer +
                    MCTP_HEADER_SIZE + (mtu * counter), bytesleft);
            } else {
                msg_in_chunks.i2cHdr.byteCnt = BYTE_COUNT_WHEN_DATA_EXCEEDS_MTU;
                memcpy(&msg_in_chunks.msgHdr, buffer +
                    MCTP_HEADER_SIZE + (mtu * counter), mtu);
            }

            p = buffer_in_chunks + msg_in_chunks.i2cHdr.byteCnt + 3;
            *p = Calc_Crc8((__u8 *)buffer_in_chunks,
                msg_in_chunks.i2cHdr.byteCnt + 3);

            *p ^= *buffer_in_chunks;
            
            ret = send_data(msg_in_chunks.i2cHdr.byteCnt + 3,
                  (__u8 *)&msg_in_chunks.i2cHdr.cmdCode);
            if (ret == -1) {
                    break;
            }

            bytesleft -= mtu;
            counter++;
        } while (bytesleft > 0);
    }
    return ret;
}

bool mi_pkt_transaction(__u8 *TxBuf, __u8 *RxBuf, __u16 Rxbuf_size)
{
    reply_buffer_struct *stReplyStruct;
    stReplyStruct = (reply_buffer_struct *)RxBuf;

    rcv_parm_t rcv_parm;
    rcv_parm.buf_size = Rxbuf_size;
    rcv_parm.buffer = stReplyStruct->replybuf;
    rcv_parm.ret_bytes = &stReplyStruct->length;
    rcv_parm.errcode = &stReplyStruct->errorcode;

    int ret = xmit_pkt(TxBuf);
    if (ret == -1) {
        printf("Unable to send command to device.\n");
        return false;
    }
    sleep(1);
    ret = rcv_pkt(&rcv_parm);
    if (ret == -1) {
        printf("Unable to receive receive response from device.\n");
        return false;
    }
    return true;
}

bool execute_nvme_mi_command(struct nvme_mi_message message,
    reply_buffer_struct *stReply, int replysize, int RequestDataSize)
{
    uint8_t *buffer  = NULL;
    uint32_t size_of_message = 0;
    uint32_t crc = 0;
    bool ret = false;

    format_base_pkt((mctp_message_t *)&message);

    sys_cfg.sys_flags |= SYS_FLAG_NVME_MI;

    if (message.msgPld.buffer == NULL) {
        size_of_message = sizeof(struct nvme_mi_message) -
             SIZE_OF_BUFFER_ADDRESS;
        buffer = (uint8_t *)malloc(size_of_message + 1);
        TotalByteCount = size_of_message - 3;

        /*Copy the contents of message apart from buffer, as it is NULL*/
        if (buffer != NULL) {
            memcpy(buffer, &message, OFST_TILL_BUFFER_NVME_MI_CMD);
        }
    } else if (message.msgPld.opcode == 06) {   /*Check for VPD Write*/
        size_of_message = sizeof(struct nvme_mi_message) - SIZE_OF_BUFFER_ADDRESS;
        int buffer_len = message.msgPld.dword1 & 0xFFFF;

        size_of_message += buffer_len;
        TotalByteCount = size_of_message - 3;

        buffer = (uint8_t *)malloc(size_of_message + 1);
        if (buffer != NULL) {
            memcpy(buffer, &message, OFST_TILL_BUFFER_NVME_MI_CMD);
            memcpy(buffer + OFST_TILL_BUFFER_NVME_MI_CMD,
                message.msgPld.buffer, buffer_len);
        }

    } else {
            size_of_message = sizeof(struct nvme_mi_message) -
            SIZE_OF_BUFFER_ADDRESS + RequestDataSize;
            /*Add one for the PEC byte*/
            buffer = (uint8_t *)malloc(size_of_message + 1);

            TotalByteCount = size_of_message - 3;
            if (buffer != NULL) {
                memcpy(buffer, &message, OFST_TILL_BUFFER_NVME_MI_CMD);
                memcpy(buffer + OFST_TILL_BUFFER_NVME_MI_CMD,
                    message.msgPld.buffer, RequestDataSize);
            }
    }

    if (buffer != NULL) {
        crc = GenerateCRC(buffer + MCTP_HEADER_SIZE, size_of_message -
        MCTP_HEADER_SIZE - CRC_SIZE);
        memcpy(buffer + size_of_message - CRC_SIZE, &crc , CRC_SIZE);
        ret = mi_pkt_transaction(buffer, (__u8 *)stReply, replysize);
        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
        }
    }

    return ret;
}

bool execute_nvme_mi_admin_command(nvme_mi_admin_message message,
    reply_buffer_struct *stReply, int replysize, int RequestDataSize)
{
    uint8_t *buffer  = NULL;
    uint32_t size_of_message = 0;
    uint32_t crc = 0;
    bool ret = false;

    format_base_pkt((mctp_message_t *)&message);

    if (message.msgPld.buffer == NULL) {
        size_of_message = sizeof(nvme_mi_admin_message) -
        SIZE_OF_BUFFER_ADDRESS;
        buffer = (uint8_t *)malloc(size_of_message + 1);
        TotalByteCount = size_of_message - 3;

        if (buffer != NULL) {
            memcpy(buffer, &message, OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD);
            memcpy(buffer + OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD,
            (char *)&message + OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD +
            SIZE_OF_BUFFER_ADDRESS, SIZE_OF_MIC);
        }
    } else {
        size_of_message = sizeof(nvme_mi_admin_message) -
            SIZE_OF_BUFFER_ADDRESS + RequestDataSize;
        buffer = (uint8_t *)malloc(size_of_message + 1);
        printf("sz = %d, req_data_size = %d\n", size_of_message, RequestDataSize);
        printf("ofst = %d\n", OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD);
        TotalByteCount = size_of_message - 3;
        if (buffer != NULL) {
            memcpy(buffer, &message, OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD);
            memcpy(buffer + OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD,
                message.msgPld.buffer, RequestDataSize);
        }
    }

    if (buffer != NULL) {
        crc = GenerateCRC(buffer + MCTP_HEADER_SIZE,
            size_of_message - MCTP_HEADER_SIZE - CRC_SIZE);
        memcpy(buffer + size_of_message - CRC_SIZE, &crc, CRC_SIZE);
        ret = mi_pkt_transaction(buffer, (__u8 *)stReply, replysize);
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    return ret;
}
