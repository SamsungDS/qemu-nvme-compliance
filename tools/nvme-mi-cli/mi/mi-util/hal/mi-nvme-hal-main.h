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
 * mi-nvme-hal-main.h - Implementation of HAL Layer for supporting Multiple
 * Sideband Hardware(s) for NVMe Management Interface command support
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#ifndef _MI_NVME_HAL_MAIN_H_
#define _MI_NVME_HAL_MAIN_H_
#include <stdint.h>
#include "mi-nvme-qemu/mi-nvme-qemu.h"

typedef enum _SidebandInterface {
    qemu_nvme_mi
} SidebandInterface;

SidebandInterface sbInterface;

struct hal_entry {
    int (*hal_open)(void);
    int (*hal_close)(void);
    int (*hal_i2c_write)(uint8_t *data_out, uint16_t num_bytes);
    int (*hal_i2c_read)(uint8_t *data_in, uint16_t num_bytes);
};

extern struct hal_entry *objHAL;

int identifyhardware();
void setsidebandinterface(SidebandInterface interface);
SidebandInterface getsidebandinterface();

#endif