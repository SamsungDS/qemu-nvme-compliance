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
 * mi-nvme-util-crc.c - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-util-master.h"

static __u8 crc8(__u8 crc, __u8 crc_data)
{
    __u8 i = 0, data = 0;
    data = crc ^ crc_data;

    for (i = 0; i < 8; i++) {
        if ((data & 0x80) != 0) {
            data <<= 1;
            data ^= 0x07;
        } else {
            data <<= 1;
        }
    }
    return data;
}

__u8 Calc_Crc8(__u8 *Buffer, __u8 byte_cnt)
{
    __u8 crc = 0, *p;
    int i;
    p = Buffer;

    for (i = 0; i < byte_cnt; i++) {
        crc = crc8(crc, *p++);
    }
    return crc;
}

uint32_t GenerateCRC(uint8_t *message, uint32_t length)
{
    if (message != NULL) {
        uint32_t crc = Calc_Crc32(0x1EDC6F41, -1, message, length);
        printf("Generated CRC32 : %"PRIx32"\n", crc);
        return crc;
    }
    return 0;
}

void gen_crc_table(uint32_t poly)
{
  register uint16_t i = 0, j = 0;
  register uint32_t crc_accum = 0;

  for (i = 0;  i < 256;  i++) {
    crc_accum = ((uint32_t)i << 24);
    for (j = 0;  j < 8;  j++) {
        if (crc_accum & 0x80000000L) {
            crc_accum = (crc_accum << 1) ^ poly;
        } else {
            crc_accum = (crc_accum << 1);
        }
    }
    crc_table[i] = crc_accum;
  }
}

uint32_t Calc_Crc32(uint32_t poly, uint32_t crc_accum,
    uint8_t *data_blk_ptr, uint32_t data_blk_size)
{
    register uint32_t i = 0, j = 0;
    gen_crc_table(poly);

    for (j = 0; j < data_blk_size; j++) {
        i = ((int) (crc_accum >> 24) ^ *data_blk_ptr++) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }
    crc_accum = ~crc_accum;
    return crc_accum;
}
