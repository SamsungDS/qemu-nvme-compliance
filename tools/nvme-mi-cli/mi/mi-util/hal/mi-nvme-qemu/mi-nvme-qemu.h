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
 * mi-nvme-qemu.h - Implementation of QEMU HAL Layer for
 * for NVMe Management Interface command support via QEMU
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#ifndef _MI_NVME_QEMU_H_
#define _MI_NVME_QEMU_H_

#include <stdint.h>
#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define QEMU_SMBUS_ADDRESS_WRITE 0x15
#define QEMU_SMBUS_ADDRESS_READ 0x16
#define QEMU_I2C_BUS_NUM  "0"

int i2cfwr, i2cfrd;

int qemu_mi_open();
int qemu_mi_close();
int qemu_mi_read(uint8_t *data_in, uint16_t num_bytes);
int qemu_mi_write(uint8_t *data_out, uint16_t num_bytes);

#endif