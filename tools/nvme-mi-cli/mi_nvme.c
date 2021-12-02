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
 * mi_nvme.c - Implementation of NVMe Management Interface commands in NVMe.
 * This file contains the MI command implementation for the plugin
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 *            Arun Kumar Kashinath Agasar <arun.kka@samsung.com>
 */

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <locale.h>
#include <asm/byteorder.h>

#include "util/argconfig.h"
#include "util/suffix.h"

#define CREATE_CMD
#include <time.h>
#include "mi_nvme.h"
#include "mi/mi-nvme-cmd.h"
#include "mi/mi-util/hal/mi-nvme-hal-main.h"

static struct plugin builtin = {
    .commands = commands,
    .name = NULL,
    .desc = NULL,
    .next = NULL,
    .tail = &builtin,
};

static struct program nvme = {
    .name = "nvme-mi",
    .version = "1.1",
    .usage = "<command> [<device>] [<args>]",
    .desc = "The '<device>' may be either an NVMe character "\
        "device (ex: /dev/nvme0) or an nvme block device "\
        "(ex: /dev/nvme0n1).",
    .extensions = &builtin,
};


void register_extension(struct plugin *plugin)
{
    plugin->parent = &nvme;
    nvme.extensions->tail->next = plugin;
    nvme.extensions->tail = plugin;
}

void cleanup_nvmemi()
{
    bool ret = close_device();
    if (ret == false) {
        printf("Error : Close device Failed!\n");
    }
}

static void nvme_show_id_ns(struct nvme_id_ns ns, __u32 nsid)
{

    __u32 i = 0;
    printf("NVME Identify Namespace %d:\n", nsid);

    printf("nsze    : %llx\n", __le64_to_cpu(ns.nsze));
    printf("ncap    : %llx\n", __le64_to_cpu(ns.ncap));
    printf("nuse    : %llx\n", __le64_to_cpu(ns.nuse));
    printf("nsfeat  : %#x\n", ns.nsfeat);
    printf("nlbaf   : %d\n", ns.nlbaf);
    printf("flbas   : %#x\n", ns.flbas);
    printf("mc      : %#x\n", ns.mc);
    printf("dpc     : %#x\n", ns.dpc);
    printf("dps     : %#x\n", ns.dps);
    printf("nmic    : %#x\n", ns.nmic);
    printf("rescap  : %#x\n", ns.rescap);
    printf("fpi     : %#x\n", ns.fpi);
    printf("dlfeat  : %d\n", ns.dlfeat);
    printf("nawun   : %d\n", __le16_to_cpu(ns.nawun));
    printf("nawupf  : %d\n", __le16_to_cpu(ns.nawupf));
    printf("nacwu   : %d\n", __le16_to_cpu(ns.nacwu));
    printf("nabsn   : %d\n", __le16_to_cpu(ns.nabsn));
    printf("nabo    : %d\n", __le16_to_cpu(ns.nabo));
    printf("nabspf  : %d\n", __le16_to_cpu(ns.nabspf));
    printf("noiob   : %d\n", __le16_to_cpu(ns.noiob));
    if (ns.nsfeat & 0x10) {
        printf("npwg    : %u\n", __le16_to_cpu(ns.npwg));
        printf("npwa    : %u\n", __le16_to_cpu(ns.npwa));
        printf("npdg    : %u\n", __le16_to_cpu(ns.npdg));
        printf("npda    : %u\n", __le16_to_cpu(ns.npda));
        printf("nows    : %u\n", __le16_to_cpu(ns.nows));
    }
    printf("mssrl   : %u\n", __le16_to_cpu(ns.mssrl));
    printf("mcl     : %d\n", __le32_to_cpu(ns.mcl));
    printf("msrc    : %u\n", ns.msrc);
    printf("anagrpid: %u\n", __le32_to_cpu(ns.anagrpid));
    printf("nsattr  : %u\n", ns.nsattr);
    printf("nvmsetid: %d\n", __le16_to_cpu(ns.nvmsetid));
    printf("endgid  : %d\n", __le16_to_cpu(ns.endgid));

    printf("nguid   : ");
    for (i = 0; i < 16; i++) {
        printf("%02x", ns.nguid[i]);
    }
    printf("\n");

    printf("eui64   : ");
    for (i = 0; i < 8; i++) {
        printf("%02x", ns.eui64[i]);
    }
    printf("\n");

    for (i = 0; i <= ns.nlbaf; i++) {
        printf("lbaf %2d : ms:%-3d lbads:%-2d rp:%#x %s\n", i,
            __le16_to_cpu(ns.lbaf[i].ms), ns.lbaf[i].ds,
            ns.lbaf[i].rp,
            i == (ns.flbas & 0xf) ? "(in use)" : "");
    }
}
static void nvme_show_id_ctrl(struct nvme_id_ctrl ctrl)
{
        printf("NVME Identify Control__ler:\n");
    printf("vid       : %#x\n", __le16_to_cpu(ctrl.vid));
    printf("ssvid     : %#x\n", __le16_to_cpu(ctrl.ssvid));
    printf("sn        : %-.*s\n", (int)sizeof(ctrl.sn), ctrl.sn);
    printf("mn        : %-.*s\n", (int)sizeof(ctrl.mn), ctrl.mn);
    printf("fr        : %-.*s\n", (int)sizeof(ctrl.fr), ctrl.fr);
    printf("rab       : %d\n", ctrl.rab);
    printf("ieee      : %02x%02x%02x\n",
        ctrl.ieee[2], ctrl.ieee[1], ctrl.ieee[0]);
    printf("cmic      : %#x\n", ctrl.cmic);
    printf("mdts      : %d\n", ctrl.mdts);
    printf("cntlid    : %#x\n", __le16_to_cpu(ctrl.cntlid));
    printf("ver       : %#x\n", __le32_to_cpu(ctrl.ver));
    printf("rtd3r     : %#x\n", __le32_to_cpu(ctrl.rtd3r));
    printf("rtd3e     : %#x\n", __le32_to_cpu(ctrl.rtd3e));
    printf("oaes      : %#x\n", __le32_to_cpu(ctrl.oaes));
    printf("ctratt    : %#x\n", __le32_to_cpu(ctrl.ctratt));
    printf("rrls      : %#x\n", __le16_to_cpu(ctrl.rrls));
    printf("cntrltype : %d\n", ctrl.cntrltype);
    printf("fguid     : %-.*s\n", (int)sizeof(ctrl.fguid), ctrl.fguid);
    printf("crdt1     : %u\n", __le16_to_cpu(ctrl.crdt1));
    printf("crdt2     : %u\n", __le16_to_cpu(ctrl.crdt2));
    printf("crdt3     : %u\n", __le16_to_cpu(ctrl.crdt3));
    printf("nvmsr     : %u\n", ctrl.nvmsr);
    printf("vwci      : %u\n", ctrl.vwci);
    printf("mec       : %u\n", ctrl.mec);
    printf("oacs      : %#x\n", __le16_to_cpu(ctrl.oacs));
    printf("acl       : %d\n", ctrl.acl);
    printf("aerl      : %d\n", ctrl.aerl);
    printf("frmw      : %#x\n", ctrl.frmw);
    printf("lpa       : %#x\n", ctrl.lpa);
    printf("elpe      : %d\n", ctrl.elpe);
    printf("npss      : %d\n", ctrl.npss);
    printf("avscc     : %#x\n", ctrl.avscc);
    printf("apsta     : %#x\n", ctrl.apsta);
    printf("wctemp    : %d\n", __le16_to_cpu(ctrl.wctemp));
    printf("cctemp    : %d\n", __le16_to_cpu(ctrl.cctemp));
    printf("mtfa      : %d\n", __le16_to_cpu(ctrl.mtfa));
    printf("hmpre     : %d\n", __le32_to_cpu(ctrl.hmpre));
    printf("hmmin     : %d\n", __le32_to_cpu(ctrl.hmmin));
    printf("rpmbs     : %#x\n", __le32_to_cpu(ctrl.rpmbs));
    printf("edstt     : %d\n", __le16_to_cpu(ctrl.edstt));
    printf("dsto      : %d\n", ctrl.dsto);
    printf("fwug      : %d\n", ctrl.fwug);
    printf("kas       : %d\n", __le16_to_cpu(ctrl.kas));
    printf("hctma     : %#x\n", __le16_to_cpu(ctrl.hctma));
    printf("mntmt     : %d\n", __le16_to_cpu(ctrl.mntmt));
    printf("mxtmt     : %d\n", __le16_to_cpu(ctrl.mxtmt));
    printf("sanicap   : %#x\n", __le32_to_cpu(ctrl.sanicap));
    printf("hmminds   : %d\n", __le32_to_cpu(ctrl.hmminds));
    printf("hmmaxd    : %d\n", __le16_to_cpu(ctrl.hmmaxd));
    printf("nsetidmax : %d\n", __le16_to_cpu(ctrl.nsetidmax));
    printf("endgidmax : %d\n", __le16_to_cpu(ctrl.endgidmax));
    printf("anatt     : %d\n", ctrl.anatt);
    printf("anacap    : %d\n", ctrl.anacap);
    printf("anagrpmax : %d\n", ctrl.anagrpmax);
    printf("nanagrpid : %d\n", __le32_to_cpu(ctrl.nanagrpid));
    printf("pels      : %d\n", __le32_to_cpu(ctrl.pels));
    printf("domainid  : %d\n", __le16_to_cpu(ctrl.domainid));
    printf("sqes      : %#x\n", ctrl.sqes);
    printf("cqes      : %#x\n", ctrl.cqes);
    printf("maxcmd    : %d\n", __le16_to_cpu(ctrl.maxcmd));
    printf("nn        : %d\n", __le32_to_cpu(ctrl.nn));
    printf("oncs      : %#x\n", __le16_to_cpu(ctrl.oncs));
    printf("fuses     : %#x\n", __le16_to_cpu(ctrl.fuses));
    printf("fna       : %#x\n", ctrl.fna);
    printf("vwc       : %#x\n", ctrl.vwc);
    printf("awun      : %d\n", __le16_to_cpu(ctrl.awun));
    printf("awupf     : %d\n", __le16_to_cpu(ctrl.awupf));
    printf("icsvscc     : %d\n", ctrl.icsvscc);
    printf("nwpc      : %d\n", ctrl.nwpc);
    printf("acwu      : %d\n", __le16_to_cpu(ctrl.acwu));
    printf("ocfs      : %#x\n", __le16_to_cpu(ctrl.ocfs));
    printf("sgls      : %#x\n", __le32_to_cpu(ctrl.sgls));
    printf("mnan      : %d\n", __le32_to_cpu(ctrl.mnan));
    printf("maxcna    : %d\n", __le32_to_cpu(ctrl.maxcna));
    printf("subnqn    : %-.*s\n", (int)sizeof(ctrl.subnqn), ctrl.subnqn);
    printf("ioccsz    : %d\n", __le32_to_cpu(ctrl.ioccsz));
    printf("iorcsz    : %d\n", __le32_to_cpu(ctrl.iorcsz));
    printf("icdoff    : %d\n", __le16_to_cpu(ctrl.icdoff));
    printf("fcatt     : %#x\n", ctrl.fcatt);
    printf("msdbd     : %d\n", ctrl.msdbd);
    printf("ofcs      : %d\n", __le16_to_cpu(ctrl.ofcs));
}

int parse_and_open_nvmemi(int argc, char **argv, const char *desc,
    const struct argconfig_commandline_options *opts)
{
    int ret = 0;
    ret = argconfig_parse(argc, argv, desc, opts);
    if (ret < 0) {
        return ret;
    }

    printf("Setting Sideband Interface to QEMU MI\n");
    setsidebandinterface(qemu_nvme_mi);

    return ret;
}

void msg_header_nvmemi(struct nvmemi_message_header_struct *str,
    MI_COMMAND_TYPE cmdtype)
{
    if (cmdtype == COMMAND_TYPE_MI) {
        str->message_type = 4;
        str->ic = 1;
        str->csi = 0;
        str->reserved = 0;
        str->nmimt = 1;
        str->ror = 0;
        str->reserved1 = 0;
    } else if (cmdtype == COMMAND_TYPE_MI_ADMIN) {
        str->message_type = 4;
        str->ic = 1;
        str->csi = 0;
        str->reserved = 0;
        str->nmimt = 2;
        str->ror = 0;
        str->reserved1 = 0;
    }
}

void gencmd_nvmemi_init(struct gencmd_nvmemi *cmd,
    struct nvmemi_message_header_struct str, __u32 opc, __u32 cdw0, __u32 cdw1)
{
    cmd->msg_header = str;
    cmd->opcode = opc;
    cmd->reserved0 = 0;
    cmd->reserved1 = 0;
    cmd->reserved2 = 0;
    cmd->cdw0 = cdw0;
    cmd->cdw1 = cdw1;
    cmd->buf = NULL;
    cmd->mic = 0;
}
static int nvmemi_cmd_response(struct nvme_mi_cmd_resp *resp)
{
    int ret = -1;
    ret = getresponsemessage((__u8 *)resp, sizeof(struct nvme_mi_cmd_resp));
    if (ret == -1) {
        printf("Unable to fetch Management Response\n");
    }
    return ret;
}

static int nvmemi_readnvmemids(__u16 ctrlid, __u8 portid, __u8 dtyp)
{
    int retval = 0;
    __u32 cdw0 = ctrlid | portid << 16 | dtyp << 24;
    __u32 cdw1 = 0;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_readnvmemids, cdw0, cdw1);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    /*Checking Response*/
    struct read_nvme_mi_data_struct_resp readNVMeDS = {};
    struct nvme_mi_cmd_resp resp = {};

    retval = nvmemi_cmd_response(&resp);
    if (retval == -1) {
        return retval;
    }

    /*Copy the Management Response*/
    uint64_t address = (uint64_t)&resp;
    memcpy(&readNVMeDS, (void *)(address + 5),
        sizeof(struct read_nvme_mi_data_struct_resp));

    if (readNVMeDS.resp_data_len == 0) {
        printf("Response data length is 0 in NVMe Management Response!\n");
        return -1;
    }

    __u8 *Respbuffer = (__u8 *)malloc(readNVMeDS.resp_data_len);
    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }

    retval = getresponsedata(Respbuffer, readNVMeDS.resp_data_len, true);
    if (retval == -1) {
        printf("Error : Failed to get command response!\n");
        return retval;
    }

    if (dtyp == 0x0) {
        struct nvme_subsys_info_data NVMeSubsysInfoDS = {};
        memcpy(&NVMeSubsysInfoDS, Respbuffer, readNVMeDS.resp_data_len);

        printf("NVMe-MI Major Version Number = %u\n", NVMeSubsysInfoDS.mjr);
        printf("NVMe-MI Minor Version Number = %u\n", NVMeSubsysInfoDS.mnr);
        printf("Number of Ports = %u\n", NVMeSubsysInfoDS.nump);
    } else if (dtyp == 0x4) {
        struct option_sup_cmd_list_struct opCommandList = {};
        memcpy(&opCommandList, Respbuffer, readNVMeDS.resp_data_len);

        printf("Number of commands = %u\n", opCommandList.numcmd);
    }

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int readnvmemids(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "Read NVMe MI Data Structure";
    const char *ctrlid = "Controller Identifier";
    const char *portid = "Port Identifier";
    const char *dtyp = "Data Structure Type";

    int retval;
    int err = -1;

    struct config {
        __u16 ctrlid;
        __u8 portid;
        __u8 dtyp;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_SHRT("ctrlid", 'c', &cfg.ctrlid, ctrlid),
        OPT_BYTE("portid", 'p', &cfg.portid, portid),
        OPT_BYTE("dtyp", 'd', &cfg.dtyp, dtyp),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing readnvmemids Command, ctrlid:%"PRIx16" portid:%d dtyp:%d\n",
            (uint16_t)cfg.ctrlid, cfg.portid, cfg.dtyp);
    err = nvmemi_readnvmemids(cfg.ctrlid, cfg.portid, cfg.dtyp);
    if (!err) {
        printf("readnvmemids: Success\n");
    } else if (err > 0) {
        printf("readnvmemids: Fail, ctrlid:%"PRIx16" portid:%d dtyp:%d\n",
            (uint16_t)cfg.ctrlid, cfg.portid, cfg.dtyp);
    }

    cleanup_nvmemi();
    return err;
}

static int nvmemi_shspoll(__u8 cs)
{
    int retval = 0;
    __u32 Reserved = 0;
    __u32 cdw0 = Reserved | cs << 31;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_subsyshealthstpoll, cdw0, 0);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    /*Checking Response*/
    struct nvm_subsys_health_struct subsysStruct = {};
    uint16_t sizetocopy = sizeof(struct nvm_subsys_health_struct);

    __u8 *Respbuffer = (__u8 *)malloc(sizetocopy);

    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }

    retval = getresponsedata(Respbuffer, sizetocopy, true);
    if (retval == -1) {
        printf("Error : Failed to get response data for the command!\n");
        return retval;
    }

    memcpy(&subsysStruct, Respbuffer, sizetocopy);

    printf("**********COMMAND RESPONSE START**********\n");
    printf("SMART Warnings = %u\n", subsysStruct.smart_warnings);
    printf("Composite Temprature = %u\n", subsysStruct.composite_temp);
    printf("Percentage Drive Life Used = %u\n", subsysStruct.per_drv_life_used);
    printf("**********COMMAND RESPONSE END**********\n");

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int shspoll(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "NVM Subsystem Health Status Poll";
    const char *cs = "Clear Status";

    int retval;
    int err = -1;

    struct config {
        __u8 cs;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_SHRT("cs", 'c', &cfg.cs, cs),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing Subsystem Health Status Poll Command, cs:%"PRIx8"\n",
            (uint8_t)cfg.cs);
    err = nvmemi_shspoll(cfg.cs);
    if (!err) {
        printf("NVM Subsystem Health Status Poll: Success\n");
    } else if (err > 0) {
        printf("NVM Subsystem Health Status Poll Fail, ctrlid:%"PRIx8"\n",
            (uint8_t)cfg.cs);
    }

    cleanup_nvmemi();
    return err;
}

static int nvmemi_chspoll(__u32 cdw0, __u32 cdw1)
{
    int retval = 0;
    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_chsp, cdw0, cdw1);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    /*Checking Response*/
    struct ctrl_health_status_poll_resp mgmtresp = {};
    struct nvme_mi_cmd_resp resp = {};

    retval = nvmemi_cmd_response(&resp);
    if (retval == -1) {
        return retval;
    }

    /*Copy the Management Response*/
    uint64_t address = (uint64_t)&resp;
    memcpy(&mgmtresp, (void *)(address + 5),
        sizeof(struct ctrl_health_status_poll_resp));

    if (mgmtresp.rent == 0) {
        printf("The number of Response Entries is zero!\n");
        return -1;
    }

    __u8 *Respbuffer = (__u8 *)malloc(mgmtresp.rent * \
    sizeof(struct ctrl_health_data));
    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }

    retval = getresponsedata(Respbuffer,
    mgmtresp.rent * sizeof(struct ctrl_health_data), true);
    if (retval == -1) {
        printf("Error : Failed to get command response!\n");
        return retval;
    }

    struct ctrl_health_data chds = {};
    memcpy(&chds, Respbuffer, sizeof(struct ctrl_health_data));

    printf("Controller Identifier = %u\n", chds.ctlid);
    printf("Composite Temprature = %u\n", chds.ctemp);
    printf("Percentage Used = %u\n", chds.pdlu);
    printf("Available Space = %u\n", chds.spare);

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int chspoll(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "NVM Controller Health Status Poll";
    const char *cdw0 = "Command DWORD0 Value";
    const char *cdw1 = "Command DWORD1 Value";

    int retval;
    int err = -1;

    struct config {
        __u32 cdw0;
        __u32 cdw1;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_SHRT("cdw0", 'c', &cfg.cdw0, cdw0),
        OPT_SHRT("cdw1", 'd', &cfg.cdw1, cdw1),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing Controller Health Status Poll Command, cdw0 : \
         %"PRIx32" cdw1 : %"PRIx32"\n", cfg.cdw0, cfg.cdw1);

    err = nvmemi_chspoll(cfg.cdw0, cfg.cdw1);
    if (!err) {
        printf("Controller Health Status Poll: Success\n");
    } else if (err > 0) {
        printf("Controller Health Status Poll Failed!\n");
    }

    cleanup_nvmemi();
    return err;
}

static int nvmemi_configget(__u8 configid, __u8 portid)
{
    int retval = 0;
    __u32 cdw0 = configid | portid << 24;
    __u32 cdw1 = 0;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_configget, cdw0, cdw1);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    /*Checking Response*/
    struct nvme_mi_cmd_resp resp = {};

    retval = nvmemi_cmd_response(&resp);
    if (retval == -1) {
        return retval;
    }


    uint64_t address = (uint64_t)&resp;

    if (configid == SMBUS_I2C_FREQ) {
        struct smbus_i2c_freq_cfgget_resp mgmt_resp = {};
        memcpy(&mgmt_resp, (void *)(address + 5),
            sizeof(struct smbus_i2c_freq_cfgget_resp));
        printf("SMBus frequency:%d\n", mgmt_resp.smbus_i2c_freq);
    } else if (configid == MCTP_TRANS_UNIT_SIZE) {
        struct mctp_tus_cfgget_resp mgmt_resp = {};
        memcpy(&mgmt_resp, (void *)(address + 5),
            sizeof(struct mctp_tus_cfgget_resp));
        printf("MCTP Transmission unit size:%d\n" ,
            mgmt_resp.mctp_trans_unit_size);
    }

    cleanup_nvmemi();
    return retval;
}

static int configget(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "Configuration Get";
    const char *configid = "Configuration Identifier";
    const char *portid = "Port Identifier";

    int retval;
    int err = -1;

    struct config {
        __u8 configid;
        __u8 portid;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_BYTE("configid", 'c', &cfg.configid, configid),
        OPT_BYTE("portid", 'p', &cfg.portid, portid),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing Configuration Get  Command, configid:%d\t portid:%d\n",
            cfg.configid, cfg.portid);
    err = nvmemi_configget(cfg.configid, cfg.portid);
    return err;
}

static int nvmemi_configset(__u8 configid, __u8 portid, __u8 smbusfreq,
    __u16 mctpunitsz)
{
    int retval = 0;
    __u32 cdw0 = 0;
    __u32 cdw1 = 0;

    if (configid == SMBUS_I2C_FREQ) {
        cdw0 = configid | smbusfreq << 8 | portid << 24;
        cdw1 = 0;
    } else if (configid == MCTP_TRANS_UNIT_SIZE) {
        cdw0 = configid | portid << 24;
        cdw1 = mctpunitsz;
    }

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_configset, cdw0, cdw1);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    cleanup_nvmemi();
    return retval;
}

static int configset(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "Configuration Set";
    const char *configid = "Configuration Identifier";
    const char *portid = "Port Identifier";
    const char *smbusfreq = "SMBus I2C frequency";
    const char *mctpunitsz = "MCTP Transmission Unit Size";

    int retval;
    int err = -1;

    struct config {
        __u8 configid;
        __u8 portid;
        __u8 smbusfreq;
        __u16 mctpunitsz;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_BYTE("configid", 'c', &cfg.configid, configid),
        OPT_BYTE("portid", 'p', &cfg.portid, portid),
        OPT_BYTE("smbusfreq", 'f', &cfg.smbusfreq, smbusfreq),
        OPT_BYTE("mctpunitsz", 's', &cfg.mctpunitsz, mctpunitsz),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing Configuration Set Command \
    configid:%d portid:%d smbusfreq:%d mctpunitsz:%d\n",
        cfg.configid, cfg.portid, cfg.smbusfreq, cfg.mctpunitsz);
    err = nvmemi_configset(cfg.configid, cfg.portid,
        cfg.smbusfreq, cfg.mctpunitsz);
    return err;
}

static int nvmemi_vpdread(__u16 dofst, __u16 dlen, char *file)
{
    int retval = 0;
    __u32 cdw0 = dofst;
    __u32 cdw1 = dlen;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd;
    gencmd_nvmemi_init(&cmd, str, nvmemi_cmd_vpdread, cdw0, cdw1);

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    /*Checking Response*/
    struct nvme_mi_cmd_resp resp = {};

    retval = nvmemi_cmd_response(&resp);
    if (retval == -1) {
        return retval;
    }

    int dfd = -1;
    int opcode = 2;
    int flags = opcode & 1 ? O_RDONLY : O_WRONLY | O_CREAT;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    uint64_t address = (uint64_t)&resp;

    if (strlen(file)) {
        dfd = open(file, flags, mode);
        if (dfd < 0) {
            printf("Failed to open the file\n");
        } else {
            printf("Open successful\n");
        }
    }

    int sz = write(dfd, (void *)(address + 8), dlen);
    if (sz < 0) {
        printf("Failed to write\n");
    }

    cleanup_nvmemi();
    return retval;
}

static int vpdread(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "VPD Read";
    const char *dofst = "Data Offset";
    const char *dlen = "Data Length";
    const char *data = "Response Data";

    int retval;
    int err = -1;

    struct config {
        __u16 dofst;
        __u16 dlen;
        char *data;
    };

    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_SHRT("dofst", 'o', &cfg.dofst, dofst),
        OPT_SHRT("dlen", 'l', &cfg.dlen, dlen),
        OPT_FILE("data", 'd', &cfg.data, data),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    printf("Issuing VPD Read Command, dofst:%d\t dlen:%d\n",
            cfg.dofst, cfg.dlen);
    err = nvmemi_vpdread(cfg.dofst, cfg.dlen, cfg.data);
    return err;
}

static int nvmemi_vpdwrite(__u16 dofst, __u16 dlen, char *req_data)
{
    int retval = 0;
    __u32 cdw0 = dofst;
    __u32 cdw1 = dlen;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI);

    struct gencmd_nvmemi cmd = {
        .msg_header = str,
        .opcode = nvmemi_cmd_vpdread,
        .reserved0 = 0,
        .reserved1 = 0,
        .reserved2 = 0,
        .cdw0 = cdw0,
        .cdw1 = cdw1,
        .buf = (uint8_t *)req_data,
        .mic = 0
    };

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    cleanup_nvmemi();
    return retval;
}

static int vpdwrite(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "VPD Write";
    const char *dofst = "Data Offset";
    const char *dlen = "Data Length";
    const char *data = "Request Data";

    int retval;
    int err = -1;
    int dfd = -1;
    int opcode = 1;
    int flags = opcode & 1 ? O_RDONLY : O_WRONLY | O_CREAT;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    struct config {
        __u16 dofst;
        __u16 dlen;
        char *data;
    };

    struct config cfg = {
        .data = "",
    };

    OPT_ARGS(opts) = {
        OPT_SHRT("dofst", 'o', &cfg.dofst, dofst),
        OPT_SHRT("dlen", 'l', &cfg.dlen, dlen),
        OPT_FILE("data", 'd', &cfg.data, data),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    if (strlen(cfg.data)) {
        dfd = open(cfg.data, flags, mode);
        if (dfd < 0) {
            printf("Failed to open the file\n");
        } else {
            printf("Open successful\n");
        }
    }
    char req_data[cfg.dlen];
    int sz = read(dfd, req_data, cfg.dlen);
    if (sz < 0) {
        printf("Failed to read\n");
    }
    printf("Issuing VPD Write Command, dofst:%d\t dlen:%d\n",
        cfg.dofst, cfg.dlen);
    err = nvmemi_vpdwrite(cfg.dofst, cfg.dlen, req_data);
    return err;
}

static int nvmemi_id(__u8 cns, __u16 cntid, __u16 nsid)
{
    __u32 data_len = 0;
    int retval = 0;
    __u32 cdw10 = 0;
    cdw10 = cns | cntid << 16;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI_ADMIN);

    if (cns == NVME_ID_CNS_NS) {
        data_len = sizeof(struct nvme_id_ns);
    } else if (cns == NVME_ID_CNS_CTRL) {
        data_len = sizeof(struct nvme_id_ctrl);
    }

    struct gencmd_nvmemi_admin cmd = {
        .msg_header = str,
        .opcode = NVME_ADMIN_IDENTIFY,
        .cflgs = 0x3,
        .ctlid = 0,
        .nsid = nsid,
        .cdw2 = 0,
        .cdw3 = 0,
        .cdw4 = 0,
        .cdw5 = 0,
        .dofst = 0,
        .dlen = data_len,
        .cdw8 = 0,
        .cdw9 = 0,
        .cdw10 = cdw10,
        .cdw11 = 0,
        .cdw12 = 0,
        .cdw13 = 0,
        .cdw14 = 0,
        .cdw15 = 0,
        .buf = NULL,
        .mic = 0
    };
    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    __u8 *Respbuffer = (__u8 *)malloc(data_len);
    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }
    memset(Respbuffer, 0, data_len);

    retval = getresponsedata(Respbuffer, data_len, false);
    if (retval == -1) {
        printf("Error : Failed to get command response!\n");
        return retval;
    }

    if (cns == NVME_ID_CNS_NS) {
        struct nvme_id_ns idns = {};
        memcpy(&idns, Respbuffer, 4096);
        nvme_show_id_ns(idns, nsid);
    } else if (cns == NVME_ID_CNS_CTRL) {
        struct nvme_id_ctrl idctrl = {};
        memcpy(&idctrl, Respbuffer, sizeof(struct nvme_id_ctrl));
        nvme_show_id_ctrl(idctrl);
    }

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int identify(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "Identify Command";
    char *cns = "Controller or Namespace Structure";
    const char *cntid = "Controller Identifier";
    const char *nsid = "Namespace ID";

    struct config {
        __u8 cns;
        __u16 cntid;
        __u16 nsid;
    };
    struct config cfg;

    OPT_ARGS(opts) = {
        OPT_SHRT("cns", 'c', &cfg.cns, cns),
        OPT_BYTE("cntid", 'C', &cfg.cntid, cntid),
        OPT_BYTE("nsid", 'i', &cfg.nsid, nsid),
        OPT_END()
    };

    int retval = -1;
    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    retval = nvmemi_id(cfg.cns, cfg.cntid, cfg.nsid);
    if (!retval) {
        printf("identify: Success\n");
    } else if (retval > 0) {
        printf("identify: Failed\n");
    }

    cleanup_nvmemi();
    return retval;
}

static int nvmemi_getlog(__u8 log_id, __u8 lsp, __u64 lpo,
                 __u16 lsi, bool rae, __u8 uuid_ix, __u32 data_len)
{
    __u32 numd = (data_len >> 2) - 1;
    __u16 numdu = numd >> 16, numdl = numd & 0xffff;
    __u32 cdw10 = log_id | (numdl << 16) | (rae ? 1 << 15 : 0) | lsp << 8;
    int retval = 0;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI_ADMIN);

    struct gencmd_nvmemi_admin cmd = {
        .msg_header = str,
        .opcode = NVME_ADMIN_GET_LOG_PAGE,
        .cflgs = 0x3,
        .ctlid = 0,
        .nsid = 0,
        .cdw2 = 0,
        .cdw3 = 0,
        .cdw4 = 0,
        .cdw5 = 0,
        .dofst = 0,
        .dlen = data_len,
        .cdw8 = 0,
        .cdw9 = 0,
        .cdw10 = cdw10,
        .cdw11 = numdu | (lsi << 16),
        .cdw12 = lpo & 0xffffffff,
        .cdw13 = lpo >> 32,
        .cdw14 = uuid_ix,
        .cdw15 = 0,
        .buf = NULL,
        .mic = 0
    };

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    __u8 *Respbuffer = (__u8 *)malloc(data_len);
    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }

    retval = getresponsedata(Respbuffer, data_len, false);
    if (retval == -1) {
        printf("Error : Failed to get command response!\n");
        return retval;
    }

    printf("sizeof logpage error info : 0x%lx\n",
        sizeof(struct log_page_error_info));
    if (log_id == 0x1) {
        struct log_page_error_info resp = {};
        memcpy(&resp, Respbuffer, sizeof(struct log_page_error_info));
        printf("Error Count = %"PRIx64"\n", resp.errcnt);
        printf("Submission Queue ID = %"PRIx16"\n", resp.subqid);
        printf("Command ID = %"PRIx16"\n", resp.cid);
        printf("Status Field = %"PRIx16"\n", resp.cid);
        printf("LBA = %"PRIx64"\n", resp.lba);
        printf("Namespace = %"PRIx32"\n", resp.ns);
        printf("Vendor Specific Information Available = %"PRIx8"\n", resp.ns);
    }

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int getlog(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "NVMe Get Log Page command via Sideband";
    const char *log_id = "identifier of log to retrieve";
    const char *log_len = "how many bytes to retrieve";
    const char *lsp = "log specific field";
    const char *lpo = "log page offset specifies the location within \
    a log page from where to start returning data";
    const char *rae = "retain an asynchronous event";
    const char *uuid_index = "UUID index";
    int retval = 0;

    struct config {
        __u8  log_id;
        __u32 log_len;
        __u64 lpo;
        __u8  lsp;
        __u8  uuid_index;
        int   rae;
    };

    struct config cfg = {
        .log_id       = 0xff,
        .log_len      = 0,
        .lpo          = NVME_NO_LOG_LPO,
        .lsp          = NVME_NO_LOG_LSP,
        .rae          = 0,
        .uuid_index   = 0,
    };

    OPT_ARGS(opts) = {
        OPT_UINT("log-id",       'i', &cfg.log_id,       log_id),
        OPT_UINT("log-len",      'l', &cfg.log_len,      log_len),
        OPT_LONG("lpo",          'o', &cfg.lpo,          lpo),
        OPT_BYTE("lsp",          's', &cfg.lsp,          lsp),
        OPT_FLAG("rae",          'r', &cfg.rae,          rae),
        OPT_BYTE("uuid-index",   'U', &cfg.uuid_index,   uuid_index),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    retval = nvmemi_getlog(cfg.log_id,
                     cfg.lsp, cfg.lpo, 0, cfg.rae,
                     cfg.uuid_index, cfg.log_len);
    if (!retval) {
        printf("Get Log Page: Success\n");
    } else if (retval > 0) {
        printf("Get Log Page: Failed\n");
    }

    cleanup_nvmemi();
    return retval;
}

static int nvmemi_getfeature(__u8 fid, __u8 sel, __u32 cdw11,
    __u32 data_len)
{
    __u32 cdw10 = fid | sel << 8;
    int retval = 0;

    struct nvmemi_message_header_struct str;
    msg_header_nvmemi(&str, COMMAND_TYPE_MI_ADMIN);

    struct gencmd_nvmemi_admin cmd = {
        .msg_header = str,
        .opcode = NVME_ADMIN_GET_FEATURES,
        .cflgs = 0x3,
        .ctlid = 0,
        .nsid = 0,
        .cdw2 = 0,
        .cdw3 = 0,
        .cdw4 = 0,
        .cdw5 = 0,
        .dofst = 0,
        .dlen = data_len,
        .cdw8 = 0,
        .cdw9 = 0,
        .cdw10 = cdw10,
        .cdw11 = cdw11,
        .cdw12 = 0,
        .cdw13 = 0,
        .cdw14 = 0,
        .cdw15 = 0,
        .buf = NULL,
        .mic = 0
    };

    /*Sending Command*/
    retval = executecommand((__u8 *)&cmd);
    if (retval == -1) {
        return retval;
    }

    __u8 *Respbuffer = (__u8 *)malloc(data_len);
    if (Respbuffer == NULL) {
        printf("Memory allocation error.\n");
        return -1;
    }

    retval = getresponsedata(Respbuffer, data_len, false);
    if (retval == -1) {
        printf("Error : Failed to get command response!\n");
        return retval;
    }

    if (fid == NVME_FEAT_TEMP_THRESH) {
        struct getf_temp_thres resp = {};
        memcpy(&resp, Respbuffer, sizeof(struct getf_temp_thres));
        printf("Temprature Threshold = %"PRIx16"\n", resp.tmpth);
        printf("Threshold Temprature Select = %"PRIx16"\n", resp.tmpsel);
        printf("Threshold Type Select = %"PRIx16"\n", resp.thsel);
    } else if (fid == NVME_FEAT_NUM_QUEUES) {
        struct getf_no_queues resp = {};
        memcpy(&resp, Respbuffer, sizeof(struct getf_no_queues));
        printf("Number of I/O Submission Queues Requested = \
        %"PRIx16"\n", resp.nsqa);
        printf("Number of I/O Completion Queues Requested = \
        %"PRIx16"\n", resp.ncqa);
    }

    if (Respbuffer != NULL) {
        free(Respbuffer);
        Respbuffer = NULL;
    }

    return retval;
}

static int getfeature(int argc, char **argv, struct command *cmd,
    struct plugin *plugin)
{
    const char *desc = "NVMe Get Features Command via Sideband Interface.";
    const char *namespace_id = "identifier of desired namespace";
    const char *feature_id = "feature identifier";
    const char *sel = "[0-3]: current/default/saved/supported";
    const char *data_len = "buffer len if data is returned \
    through host memory buffer";
    const char *cdw11 = "dword 11 for interrupt vector config";

    int retval = 0;

    struct config {
        __u32 namespace_id;
        __u8  feature_id;
        __u8  sel;
        __u32 cdw11;
        __u32 data_len;
    };

    struct config cfg = {
        .namespace_id = 0,
        .feature_id   = 0,
        .sel          = 0,
        .cdw11        = 0,
        .data_len     = 0,
    };

    OPT_ARGS(opts) = {
        OPT_UINT("namespace-id",  'n', &cfg.namespace_id,   namespace_id),
        OPT_UINT("feature-id",    'f', &cfg.feature_id,     feature_id),
        OPT_BYTE("sel",           's', &cfg.sel,            sel),
        OPT_UINT("data-len",      'l', &cfg.data_len,       data_len),
        OPT_UINT("cdw11",         'c', &cfg.cdw11,          cdw11),
        OPT_END()
    };

    retval = parse_and_open_nvmemi(argc, argv, desc, opts);
    if (retval < 0) {
        printf("parse_and_open_nvmemi failed!\n");
        return errno;
    }

    retval = nvmemi_getfeature(cfg.feature_id,
                     cfg.sel, cfg.cdw11, cfg.data_len);
    if (!retval) {
        printf("Get Log Page: Success\n");
    } else if (retval > 0) {
        printf("Get Log Page: Failed\n");
    }

    cleanup_nvmemi();
    return retval;
}

int main(int argc, char **argv)
{
    int err;

    nvme.extensions->parent = &nvme;
    if (argc < 2) {
        general_help(&builtin);
        return 0;
    }
    setlocale(LC_ALL, "");

    err = handle_plugin(argc - 1, &argv[1], nvme.extensions);
    if (err == -ENOTTY) {
        general_help(&builtin);
    }
    return err;
}
