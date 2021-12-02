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
 * mi-nvme-struct.h - Implementation of NVMe Management Interface commands in NVMe.
 * This file contains required header and response structures for MI commands.
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
              Arun Kumar Kashinath Agasar <arun.kka@samsung.com>
 */

#ifndef _MI_NVME_STRUCT_H_
#define _MI_NVME_STRUCT_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/types.h>

#define MCTP_PKT_HDR_SIZE 8
#define NVME_MI_HEADER_SIZE 4
#define NVME_MI_STATUS_SIZE 4
#define CQENTRY_SIZE 12
#define MCTP_TRANS_UNIT_SIZE_VAL_DEF 64

enum nvmemi_opcode {
    nvmemi_cmd_readnvmemids = 0x00,
    nvmemi_cmd_subsyshealthstpoll = 0x01,
    nvmemi_cmd_chsp = 0x02,
    nvmemi_cmd_configset = 0x03,
    nvmemi_cmd_configget = 0x04,
    nvmemi_cmd_vpdread = 0x05,
    nvmemi_cmd_vpdwrite = 0x06
};

typedef enum _MI_COMMAND_TYPE {
    COMMAND_TYPE_MI,
    COMMAND_TYPE_MI_ADMIN
} MI_COMMAND_TYPE;

enum CONFIGURATION_IDENTIFIER {
    RESERVED,
    SMBUS_I2C_FREQ,
    HEALTH_STATUS_CHG,
    MCTP_TRANS_UNIT_SIZE,
};

struct nvmemi_message_header_struct {
    uint32_t message_type:7;
    uint32_t ic:1;
    uint32_t csi:1;
    uint32_t reserved:2;
    uint32_t nmimt:4;
    uint32_t ror:1;
    uint32_t reserved1:16;
};

/*Generic command Structure for NVMe MI Commands*/
struct gencmd_nvmemi {
    struct nvmemi_message_header_struct msg_header;
    uint8_t opcode;
    uint8_t reserved0;
    uint8_t reserved1;
    uint8_t reserved2;
    uint32_t cdw0;
    uint32_t cdw1;
    uint8_t *buf;
    uint32_t mic;
};

/*Generic command Structure for NVMe MI Admin Commands*/
struct gencmd_nvmemi_admin {
    struct nvmemi_message_header_struct msg_header;
    uint8_t opcode;
    uint8_t cflgs;
    uint16_t ctlid;
    uint32_t nsid;
    uint32_t cdw2;
    uint32_t cdw3;
    uint32_t cdw4;
    uint32_t cdw5;
    uint32_t dofst;
    uint32_t dlen;
    uint32_t cdw8;
    uint32_t cdw9;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
    uint8_t *buf;
    uint32_t mic;
};

struct admin_cmd_resp_dw3 {
    uint16_t cid;
    uint16_t p:1;
    uint16_t sc:8;
    uint16_t sct:3;
    uint16_t crd:2;
    uint16_t m:1;
    uint16_t dnr:1;
};

struct nvme_admin_cmd_resp_status {
    uint8_t msg_type:7;
    uint8_t ic:1;

    uint8_t csi:1;
    uint8_t reserved:2;
    uint8_t nvme_mi_msg_type:4;
    uint8_t ror:1;

    uint16_t reserved1;

    uint32_t status:8;
    uint32_t nvme_mgmt_response:24;

    uint32_t cqdw0;
    uint32_t cqdw1;
    struct admin_cmd_resp_dw3 cqdw3;
};

struct nvme_mi_cmd_resp {
    uint8_t msg_type:7;
    uint8_t ic:1;

    uint8_t csi:1;
    uint8_t reserved:2;
    uint8_t nvme_mi_msg_type:4;
    uint8_t ror:1;

    uint16_t reserved1;

    uint32_t status:8;
    uint32_t nvme_mgmt_response:24;
};

struct read_nvme_mi_data_struct_resp {
    uint16_t resp_data_len;
    uint8_t reserved;
};

struct smbus_i2c_freq_cfgget_resp {
    uint8_t smbus_i2c_freq:4;
    uint8_t reserved1:4;
    uint16_t reserved2;
};

struct mctp_tus_cfgget_resp {
    uint16_t mctp_trans_unit_size;
    uint8_t reserved;
};

struct nvme_subsys_info_data {
    uint8_t nump;
    uint8_t mjr;
    uint8_t mnr;
    uint8_t reserved[29];
};

struct option_sup_cmd_struct {
    uint8_t cmdtype;
    uint8_t opc;
};

struct option_sup_cmd_list_struct {
    uint16_t numcmd;
    struct option_sup_cmd_struct cmdstruct[2047];
};

struct nss_status_struct {
    uint8_t reserved:2;
    uint8_t port1_pla:1;
    uint8_t port2_pla:1;
    uint8_t reset_not_req:1;
    uint8_t drive_func:1;
    uint8_t reserved1:2;
};

struct comp_ctrl_status_struct {
    uint16_t ready:1;
    uint16_t cfs:1;
    uint16_t shn_sts:2;
    uint16_t nssr_occured:1;
    uint16_t ceco:1;
    uint16_t nsac:1;
    uint16_t fwact:1;
    uint16_t cs_ch:1;
    uint16_t ctc:1;
    uint16_t percentage_used:1;
    uint16_t available_spare:1;
    uint16_t critical_warning:1;
    uint16_t reserved:3;
};

struct nvm_subsys_health_struct {
    struct nss_status_struct nss_status;
    uint8_t smart_warnings;
    uint8_t composite_temp;
    uint8_t per_drv_life_used;
    struct comp_ctrl_status_struct  comp_ctrl_status;
    uint16_t reserved;
};

struct ctrl_health_status_poll_resp {
    __u16 reserved;
    __u8 rent;
};

struct cwarn_struct {
    __u8 spare_thresh : 1;
    __u8 temp_above_or_under_thresh : 1;
    __u8 rel_degraded : 1;
    __u8 read_only : 1;
    __u8 vol_mem_bup_fail : 1;
    __u8 reserved : 3;
};

struct csts_struct {
    __u16 rdy : 1;
    __u16 cfs : 1;
    __u16 shst : 2;
    __u16 nssro : 1;
    __u16 en : 1;
    __u16 nssac : 1;
    __u16 fwact : 1;
    __u16 reserved : 8;
};

struct ctrl_health_data {
    __u16 ctlid;
    struct csts_struct csts;
    __u16 ctemp;
    __u8 pdlu;
    __u8 spare;
    struct cwarn_struct cwarn;
    __u8 reserved[7];
};

struct log_page_error_info {
    uint64_t errcnt;
    uint16_t subqid;
    uint16_t cid;
    uint16_t statusfield;
    uint8_t perr_loc_byte;
    uint8_t perr_loc_bit:3;
    uint8_t perr_loc_res:5;
    uint64_t lba;
    uint32_t ns;
    uint8_t vsinfoavl;
    uint8_t reserved[35];
};

struct getf_temp_thres {
    uint16_t tmpth;
    uint16_t tmpsel:4;
    uint16_t thsel:2;
    uint16_t reserved:10;
};

struct getf_no_queues {
    uint16_t nsqa;
    uint16_t ncqa;
};

#endif
