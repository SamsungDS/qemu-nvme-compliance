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
 * mi-nvme-qemu.c - Implementation of QEMU HAL Layer for
 * for NVMe Management Interface command support via QEMU
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-qemu.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int qemu_mi_read(uint8_t *data_in, uint16_t num_bytes)
{
    struct i2c_smbus_ioctl_data args;
    union i2c_smbus_data data;
    args.read_write = I2C_SMBUS_READ;
    args.size = I2C_SMBUS_BYTE;
    args.data = &data;
    int retry = 0;
    int status = 0;
    do {
        if (ioctl(i2cfrd, I2C_SMBUS, &args) < 0) {
            printf("read ioctl failed %d\n", errno);
        }
        usleep(200000);
        retry++;
        if (retry == 10) {
            printf("retry #%d\n", retry);
            status = -1;
            break;
        }
    } while ((data.byte & 0xFF) == -1);

    if (status != -1) {
        data_in[0] = data.byte & 0xFF;
        for (int i = 1; i < num_bytes; i++)  {
            if (ioctl(i2cfrd, I2C_SMBUS, &args) < 0) {
                printf("read ioctl failed = %d\n", errno);
                return -1;
            } else {
                data_in[i] = data.byte & 0xFF;
            }
        }
    }
    return status;
}

int qemu_mi_write(uint8_t *data_out, uint16_t num_bytes)
{
    struct i2c_smbus_ioctl_data args;
    args.read_write = I2C_SMBUS_WRITE;
    args.size = I2C_SMBUS_BYTE;
    args.data = NULL;
    data_out[2] = (QEMU_SMBUS_ADDRESS_READ << 1) | 1;
    for (int i = 0; i < num_bytes; i++) {
        args.command = data_out[i];
        if (ioctl(i2cfwr, I2C_SMBUS, &args) < 0) {
            printf("write ioctl failed %d\n", errno);
            return -1;
        }
    }
    return num_bytes;
}

int qemu_mi_open()
{
    char i2cbus[256];
    strcpy(i2cbus, "/dev/i2c-");
    strcat(i2cbus, QEMU_I2C_BUS_NUM);

    i2cfwr = open(i2cbus, O_RDWR | O_SYNC);
    if (ioctl(i2cfwr, I2C_SLAVE, QEMU_SMBUS_ADDRESS_WRITE) < 0) {
        printf("write ioctl failed = %d\n", errno);
        return -1;
    }
    i2cfrd = open(i2cbus, O_RDWR | O_SYNC);
    if (ioctl(i2cfrd,I2C_SLAVE, QEMU_SMBUS_ADDRESS_READ) < 0) {
       printf("read ioctl failed = %d\n", errno);
       return -1;
    }
    return 1;
}

int qemu_mi_close()
{
    if (i2cfwr != -1) {
        printf("Closing i2c device handle : 0x%x\n", i2cfwr);
        close(i2cfwr);
        i2cfwr = -1;
    }
    if (i2cfrd != -1) {
        printf("Closing i2c device handle : 0x%x\n", i2cfrd);
        close(i2cfrd);
        i2cfrd = -1;
    }

    return -1;
}
