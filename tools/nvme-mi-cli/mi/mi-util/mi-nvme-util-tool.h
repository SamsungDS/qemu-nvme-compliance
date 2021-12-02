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
 * mi-nvme-util-tool.h - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */



#ifndef __MI_NVME_UTIL_TOOL_H__
#define __MI_NVME_UTIL_TOOL_H__

#include "mi-nvme-util-base.h"

#define SYS_FLAG_PEC_OVERRIDE 0x00000400
#define SYS_FLAG_NVME_MI 0x00010000
#define MAX_OPEN_RETRY 10
#define MAX_CLOSE_RETRY 5

typedef struct _system_cfg_t {
    __u32 sys_flags;
    __u32 verbose_level;
    __u32 op_state;

    int phandle;
    int dhandle;
    __u16 iport;
    __u16 aport;
    __u16 dport;

    __u8 taddr;
    __u8 saddr;

    __u8 peccode;
} system_cfg_t;

__u32 mctp_tus;
system_cfg_t sys_cfg;
bool state_dev;
bool hardware_init;

bool initialize(__u32 uiMCTP_TUS);
int send_data(__u16 num_write, __u8 *data_out);
bool open_device();
bool close_device();

#endif
