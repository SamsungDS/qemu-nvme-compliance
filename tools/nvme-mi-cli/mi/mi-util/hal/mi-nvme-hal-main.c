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
 * mi-nvme-hal-main.c - Implementation of HAL Layer for supporting Multiple
 * Sideband Hardware(s) for NVMe Management Interface command support
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-hal-main.h"

struct hal_entry *objHAL;

struct hal_entry objHAL_QEMU = {
    .hal_open = qemu_mi_open,
    .hal_close = qemu_mi_close,
    .hal_i2c_write = qemu_mi_write,
    .hal_i2c_read = qemu_mi_read
};

void setsidebandinterface(SidebandInterface interface)
{
    sbInterface = interface;
}

SidebandInterface getsidebandinterface()
{
    return sbInterface;
}

int identifyhardware()
{
    int result = 0;
    objHAL = NULL;
    switch (getsidebandinterface()) {
    case qemu_nvme_mi:
        printf("Assigning QEMU HAL Object\n");
        objHAL = &objHAL_QEMU;
        break;
    default:
        break;
    }

    if (objHAL == NULL) {
        printf("Unable to allocate Hardware Functions.\n");
        result = -1;
    }
    return result;
}
