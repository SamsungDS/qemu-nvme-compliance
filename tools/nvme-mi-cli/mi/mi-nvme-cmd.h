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
 * mi-nvme-cmd.h - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#ifndef _MI_NVME_CMD_H_
#define _MI_NVME_CMD_H_

#include "mi-nvme-struct.h"
#include "mi-util/mi-nvme-util-base.h"
#include "mi-util/mi-nvme-util-tool.h"

reply_buffer_struct streply;

uint32_t gettransmissionunitsize();
int executecommand(__u8 *cmdobj);
int getresponsedata(uint8_t *buf, uint32_t size, bool flagmi);
int getresponsemessage(uint8_t *buf, uint32_t size);

#endif
