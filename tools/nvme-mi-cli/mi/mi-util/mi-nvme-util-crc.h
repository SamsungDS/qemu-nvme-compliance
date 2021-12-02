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
 * mi-nvme-util-crc.h - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#ifndef __MI_NVME_UTIL_CRC_H__
#define __MI_NVME_UTIL_CRC_H__

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

uint32_t crc_table[256];

uint32_t GenerateCRC(uint8_t *message, uint32_t length);
void gen_crc_table();
uint32_t Calc_Crc32(uint32_t poly, uint32_t crc_accum, uint8_t *message, uint32_t size);
__u8 Calc_Crc8(__u8 *Buffer, __u8 byte_cnt);

#endif
