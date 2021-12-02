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
 * mi-nvme-cmd.c - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-cmd.h"

uint32_t gettransmissionunitsize()
{
    return MCTP_TRANS_UNIT_SIZE_VAL_DEF;
}

int executecommand(__u8 *cmdobj)
{
    struct nvme_mi_message message;
    memset(&message, 0, sizeof(struct nvme_mi_message));
    nvme_mi_admin_message adminmessage;
    memset(&adminmessage, 0, sizeof(nvme_mi_admin_message));
    bool ret = false;
    uint32_t RequestDataSize = 0;
    struct gencmd_nvmemi *micmd;
    struct gencmd_nvmemi_admin *miadmincmd;
    uint32_t uiMCTP_TUS = gettransmissionunitsize();

    ret = initialize(uiMCTP_TUS);
    if (ret == false) {
        return -1;
    }

    streply.length = 0;
    streply.errorcode = 0;
    memset(streply.replybuf, 0, sizeof(streply.replybuf));
    memcpy(&message.msgPld, cmdobj, sizeof(struct nvme_mi_message) - 8);

    /*Check if the incoming command is an MI/MI-Admin Command*/
    if (message.msgPld.nvme_mi_message_header & 0x1000) {
        miadmincmd = (struct gencmd_nvmemi_admin *)cmdobj;
        memcpy(&adminmessage.msgPld, cmdobj, sizeof(nvme_mi_admin_message) - 8);
        if (miadmincmd->buf != NULL) {
            adminmessage.msgPld.buffer = miadmincmd->buf;
        }
        ret = execute_nvme_mi_admin_command(adminmessage, &(streply),
            REPLY_BUFFER_SIZE, miadmincmd->dlen);
    } else if (message.msgPld.nvme_mi_message_header & 0x800) {
        micmd = (struct gencmd_nvmemi *)cmdobj;
        if (micmd->buf != NULL) {
            message.msgPld.buffer = micmd->buf;
        }
        ret = execute_nvme_mi_command(message, &(streply),
            REPLY_BUFFER_SIZE, RequestDataSize);
    }

    if (!ret) {
        return -1;
    } else {
        return 0;
    }
}

int getresponsedata(uint8_t *buf, uint32_t size, bool ismicmd)
{
    uint32_t offset = 0;
    uint32_t length = gettransmissionunitsize() - NVME_MI_HEADER_SIZE;
    uint32_t bytesread = 0;

    if (buf == NULL) {
        printf("Error : getresponsedata input buf is NULL\n");
        return -1;
    }

    if (ismicmd == true) {
        offset = MCTP_PKT_HDR_SIZE + NVME_MI_STATUS_SIZE +
            NVME_MI_HEADER_SIZE;
        length = gettransmissionunitsize() - NVME_MI_HEADER_SIZE -
            NVME_MI_STATUS_SIZE;
        streply.length = streply.length -  NVME_MI_HEADER_SIZE -
            NVME_MI_STATUS_SIZE - CRC_SIZE;
    } else {
        offset = MCTP_PKT_HDR_SIZE + NVME_MI_STATUS_SIZE +
            NVME_MI_HEADER_SIZE + CQENTRY_SIZE;
        length = gettransmissionunitsize() - NVME_MI_HEADER_SIZE -
            NVME_MI_STATUS_SIZE - CQENTRY_SIZE;
        streply.length = streply.length - NVME_MI_HEADER_SIZE -
            NVME_MI_STATUS_SIZE - CRC_SIZE - CQENTRY_SIZE;
    }

    if (length > size) {
        length = size;
    }

    while (bytesread < streply.length) {
        memcpy(buf + bytesread, streply.replybuf + offset, length);
        offset += length + MCTP_PKT_HDR_SIZE;
        bytesread += length;

        if (bytesread + gettransmissionunitsize() > streply.length) {
            length = streply.length - bytesread;
        } else {
            length = gettransmissionunitsize();
        }
    }
    return 0;
}

int getresponsemessage(uint8_t *buf, uint32_t size)
{
    if (buf == NULL) {
        return -1;
    } else {
        memcpy(buf, streply.replybuf + MCTP_PKT_HDR_SIZE, size);
        return 0;
    }
}
