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
 * mi-nvme-util-base.h - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#ifndef __MI_NVME_UTIL_BASE_H__
#define __MI_NVME_UTIL_BASE_H__

#include <pthread.h>
#include <linux/types.h>

#define MCTP_CMD_CODE                       0x0F
#define MCTP_HEADER_VER                     0x1

#define MCTP_EID_DESTINATION                0x00
#define MCTP_EID_SOURCE                     0x00

/* Byte 7 Packet Control Bits */
#define MCTP_CTRL_PKT_MSGTAG_MASK 0x07
#define MCTP_CTRL_PKT_TO 0x08
#define MCTP_CTRL_PKT_TO_SHIFT 3
#define MCTP_CTRL_PKT_PKTSEQ_MASK 0x30
#define MCTP_CTRL_PKT_PKTSEQ_SHIFT 4
#define MCTP_CTRL_PKT_EOM 0x40
#define MCTP_CTRL_PKT_EOM_SHIFT 6
#define MCTP_CTRL_PKT_SOM 0x80
#define MCTP_CTRL_PKT_SOM_SHIFT 7
#define MCTP_CTRL_MSGTAG(x) (x & MCTP_CTRL_PKT_MSGTAG_MASK)
#define MCTP_CTRL_PKTSEQ(x) ((x << MCTP_CTRL_PKT_PKTSEQ_SHIFT) & MCTP_CTRL_PKT_PKTSEQ_MASK)

#define VDM_ARG_COUNT 20
#define SIZE_OF_BUFFER_ADDRESS 8
#define SIZE_OF_32_BUFFER_ADDRESS 4
#define OFST_TILL_BUFFER_NVME_MI_ADMIN_CMD 76
#define OFST_TILL_BUFFER_NVME_MI_CMD 24
#define SIZE_OF_MIC 4
#define MCTP_HEADER_SIZE 8
#define CRC_SIZE 4
#define BYTE_COUNT_WHEN_DATA_EXCEEDS_MTU 69
#define REPLY_BUFFER_SIZE 5120
#define REPLAY_BUFFER_SIZE 5120
#define REPLAY_RESPONSE_MESSAGE_SIZE 20

#define PACKED
#pragma pack(push, 1)

typedef struct _mctp_i2c_header {
    __u8                 destAddr;
    __u8                 cmdCode;
    __u8                 byteCnt;
    __u8                 srcAddr;
    __u8                 headerVer;
    __u8                 destEID;
    __u8                 srcEID;
    /* Byte 3 for msgTag(2:0), TO(3), pktSeq(5:4), EOM(6), SOM(7) */
    __u8                 pktCtrl;
} mctp_i2c_header;

typedef struct _mctp_msg_header_t {
    __u8                 msgTpe;
    union {
        struct {
            /* Byte for InstID(4:0), D(6), Rq(7) fields */
            __u8    InstCde;
            __u8    cmdCode;
            __u8    OpCpl;
        } ctrMsg;
    } msgReqRsp;
} mctp_msg_header_t;

typedef struct mctp_msg_payload_ {
    union {
        struct {
            __u8        EID_status;
            __u8        EP_Type;
            __u8        Misc;
            __u8        byte[((VDM_ARG_COUNT * 4) + 13)];
        } baseCtrl;
    } dataPld;
} mctp_msg_payload_t;

typedef struct mctp_message_ {
    mctp_i2c_header  i2cHdr;
    mctp_msg_header_t  msgHdr;
    mctp_msg_payload_t msgPld;
    __u32 pad[1];
} mctp_message_t;

typedef struct _message_header {
    __u8 messsage_type:7;
    __u8 ic:1;
    __u8 instance_id:5;
    __u8 rsvd:1;
    __u8 d:1;
    __u8 rq:1;
    __u8 command_code;
} message_header;

typedef struct _nvme_mi_mctp_cmd_pld {
    message_header nvme_mi_message_header;
    __u8 *buffer;
} nvme_mi_mctp_cmd_pld;

typedef struct _mctp_command_packet {
    mctp_i2c_header  i2cHdr;
    nvme_mi_mctp_cmd_pld msgPld;
} mctp_command_packet;

typedef struct _mctp_command_reply_packet {
    mctp_i2c_header  i2cHdr;
    nvme_mi_mctp_cmd_pld msgPld;
} mctp_command_reply_packet;

struct nvme_mi_mctp_message_pld {
    __u32 nvme_mi_message_header;
    __u8 opcode;
    __u8 reserved0;
    __u8 reserved1;
    __u8 reserved2;
    __u32 dword0;
    __u32 dword1;
    __u8 *buffer;
    __u32 mic;
} __attribute__((packed));

typedef struct _nvme_mi_Admin_message_pld {
    __u32 nvme_mi_message_header;
    __u8 opcode;
    __u8 cflgs;
    __u16 ctlid;
    __u32 cdw1;
    __u32 cdw2;
    __u32 cdw3;
    __u32 cdw4;
    __u32 cdw5;
    __u32 dofst;
    __u32 dlen;
    __u32 cdw8;
    __u32 cdw9;
    __u32 cdw10;
    __u32 cdw11;
    __u32 cdw12;
    __u32 cdw13;
    __u32 cdw14;
    __u32 cdw15;
    __u8 *buffer;
    __u32 mic;
} nvme_mi_admin_message_pld;

struct nvme_mi_message {
    mctp_i2c_header i2cHdr;
    struct nvme_mi_mctp_message_pld msgPld;
} __attribute__((packed));

typedef struct _nvme_mi_admin_message {
    mctp_i2c_header i2cHdr;
    nvme_mi_admin_message_pld msgPld;
} nvme_mi_admin_message;

typedef struct _rcv_parm_t {
    __u16 buf_size;
    __u8 *buffer;
    __u32 *ret_bytes;
    __u32 *errcode;
} rcv_parm_t;

typedef struct _reply_buffer_struct {
    __u8 replybuf[REPLY_BUFFER_SIZE];
    __u32 length;
    __u32 errorcode;
} reply_buffer_struct;

#pragma pack(pop)
#undef PACKED

__u32 TotalByteCount;

void format_base_pkt(mctp_message_t *m);
int rcv_pkt(void *inp_parm);
int xmit_pkt(__u8 *buffer);
bool mi_pkt_transaction(__u8 *TxBuf, __u8 *RxBuf, __u16 Rxbuf_size);
bool execute_nvme_mi_command(struct nvme_mi_message message,
    reply_buffer_struct *stReply, int replysize, int RequestDataSize);
bool execute_nvme_mi_admin_command(nvme_mi_admin_message message,
    reply_buffer_struct *stReply, int replysize, int RequestDataSize);

#endif
