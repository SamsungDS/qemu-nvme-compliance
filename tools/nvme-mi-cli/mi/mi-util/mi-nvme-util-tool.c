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
 * mi-nvme-util-tool.c - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 */

#include "mi-nvme-util-master.h"

bool initialize(__u32 uiMCTP_TUS)
{
    bool retval = false;
    memset(&sys_cfg, 0, sizeof(system_cfg_t));
    state_dev = false;
    hardware_init = false;
    mctp_tus = 0;
    TotalByteCount = 0;

    mctp_tus = uiMCTP_TUS;
    if (!hardware_init) {
        int ret = identifyhardware();
        if (ret == -1) {
            printf("Initialiation Failed.\n");
            return false;
        }
        hardware_init = true;
    }

    retval = open_device();
    if (retval == false) {
        printf("open device Failed!\n");
        return false;
    }

    return retval;
}

int send_data(__u16 num_write, __u8 *data_out)
{
    int count = 0;

    printf("\nData being written to the device, byte count = 0x%X:\n", num_write);
    count = objHAL->hal_i2c_write(data_out, num_write);
    usleep(10);

    if (count < 0) {
        printf("Error in sending data\n");
        return -1;
    }
    printf("Number of bytes written to the device 0x%02x(%d)\n", count, count);
    return count;
}

bool open_device()
{
    int status = -1, i = 0;

    if (!state_dev) {
        for (i = 0; i < MAX_OPEN_RETRY; i++) {
            status = objHAL->hal_open();
            if (status < 0) {
                printf("Unable to open device\n");
            } else {
                break;
            }
        }

        if (status <= 0) {
            printf("Unable to open device on port %d\n", sys_cfg.aport);
            return false;
        }
        state_dev = true;
    }
    return true;
}

bool close_device()
{
    int status = -1, i = 0;

    if (state_dev) {
        for (i = 0; i < MAX_CLOSE_RETRY; i++) {
            status = objHAL->hal_close();
            if (status == -1) {
                break;
            }
        }

        if (status != -1) {
            printf("Device handle close unsuccessful!\n");
            return false;
        }
        state_dev = false;
    }
    return true;
}
