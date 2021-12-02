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
 * mi_nvme.h - Implementation of NVMe Management Interface commands in Nvme
 *
 * Developer: Mohit Kapoor <mohit.kap@samsung.com>
 *            Arun Kumar Kashinath Agasar <arun.kka@samsung.com>
 */

#undef CMD_INC_FILE
#define CMD_INC_FILE mi_nvme

#if !defined(MI) || defined(CMD_HEADER_MULTI_READ)
#define MI

#define NVME_FEAT_NUM_QUEUES      0x7
#define NVME_FEAT_TEMP_THRESH     0x1
#define NVME_ADMIN_GET_LOG_PAGE   0x2
#define NVME_ADMIN_GET_FEATURES   0xA
#define NVME_NO_LOG_LSP           0x0
#define NVME_NO_LOG_LPO           0x0
#define NVME_ADMIN_IDENTIFY       0x06
#define NVME_ID_CNS_NS            0x00
#define NVME_ID_CNS_CTRL          0x01
#include "cmd.h"
#include "plugin.h"
#include <linux/types.h>


    COMMAND_LIST(
        ENTRY("readnvmemids", "nvme-mi : Read NVMe-MI Data Structure",
              readnvmemids)
        ENTRY("shspoll", "nvme-mi : NVM Subsystem Health Status Poll", shspoll)
        ENTRY("chspoll", "nvme-mi : Controller Health Status Poll", chspoll)
        ENTRY("configget", "nvme-mi : Configuration Get", configget)
        ENTRY("configset", "nvme-mi : Configuration Set", configset)
        ENTRY("vpdread", "nvme-mi : VPD Read", vpdread)
        ENTRY("vpdwrite", "nvme-mi : VPD Write", vpdwrite)
        ENTRY("identify", "nvme-mi-admin : Identify", identify)
        ENTRY("getlogpage", "nvme-mi-admin : Get Log Page", getlog)
        ENTRY("getfeatures", "nvme-mi-admin : Get Features", getfeature)
    );


#endif

#include "define_cmd.h"

#ifndef STRUCT_H
#define STRUCT_H
enum nvme_print_flags {
    NORMAL  = 0,
    VERBOSE = 1 << 0,   /* verbosely decode complex values for humans */
    JSON    = 1 << 1,   /* display in json format */
    VS  = 1 << 2,   /* hex dump vendor specific data areas */
    BINARY  = 1 << 3,   /* binary dump raw bytes */
};

struct nvme_id_power_state {
    __le16          max_power;  /* centiwatts */
    __u8            rsvd2;
    __u8            flags;
    __le32          entry_lat;  /* microseconds */
    __le32          exit_lat;   /* microseconds */
    __u8            read_tput;
    __u8            read_lat;
    __u8            write_tput;
    __u8            write_lat;
    __le16          idle_power;
    __u8            idle_scale;
    __u8            rsvd19;
    __le16          active_power;
    __u8            active_work_scale;
    __u8            rsvd23[9];
};

struct nvme_id_ctrl {
    __le16          vid;
    __le16          ssvid;
    char            sn[20];
    char            mn[40];
    char            fr[8];
    __u8            rab;
    __u8            ieee[3];
    __u8            cmic;
    __u8            mdts;
    __le16          cntlid;
    __le32          ver;
    __le32          rtd3r;
    __le32          rtd3e;
    __le32          oaes;
    __le32          ctratt;
    __le16          rrls;
    __u8            rsvd102[9];
    __u8            cntrltype;
    char            fguid[16];
    __le16          crdt1;
    __le16          crdt2;
    __le16          crdt3;
    __u8            rsvd134[119];
    __u8            nvmsr;
    __u8            vwci;
    __u8            mec;
    __le16          oacs;
    __u8            acl;
    __u8            aerl;
    __u8            frmw;
    __u8            lpa;
    __u8            elpe;
    __u8            npss;
    __u8            avscc;
    __u8            apsta;
    __le16          wctemp;
    __le16          cctemp;
    __le16          mtfa;
    __le32          hmpre;
    __le32          hmmin;
    __u8            tnvmcap[16];
    __u8            unvmcap[16];
    __le32          rpmbs;
    __le16          edstt;
    __u8            dsto;
    __u8            fwug;
    __le16          kas;
    __le16          hctma;
    __le16          mntmt;
    __le16          mxtmt;
    __le32          sanicap;
    __le32          hmminds;
    __le16          hmmaxd;
    __le16          nsetidmax;
    __le16          endgidmax;
    __u8            anatt;
    __u8            anacap;
    __le32          anagrpmax;
    __le32          nanagrpid;
    __le32          pels;
    __le16          domainid;
    __u8            rsvd358[10];
    __u8            megcap[16];
    __u8            rsvd384[128];
    __u8            sqes;
    __u8            cqes;
    __le16          maxcmd;
    __le32          nn;
    __le16          oncs;
    __le16          fuses;
    __u8            fna;
    __u8            vwc;
    __le16          awun;
    __le16          awupf;
    __u8            icsvscc;
    __u8            nwpc;
    __le16          acwu;
    __le16          ocfs;
    __le32          sgls;
    __le32          mnan;
    __u8            maxdna[16];
    __le32          maxcna;
    __u8            rsvd564[204];
    char            subnqn[256];
    __u8            rsvd1024[768];
    __le32          ioccsz;
    __le32          iorcsz;
    __le16          icdoff;
    __u8            fcatt;
    __u8            msdbd;
    __le16          ofcs;
    __u8            rsvd1806[242];
    struct nvme_id_power_state  psd[32];
    __u8            vs[1024];
};

struct nvme_lbaf {
    __le16          ms;
    __u8            ds;
    __u8            rp;
};

struct nvme_id_ns {
    __le64          nsze;
    __le64          ncap;
    __le64          nuse;
    __u8            nsfeat;
    __u8            nlbaf;
    __u8            flbas;
    __u8            mc;
    __u8            dpc;
    __u8            dps;
    __u8            nmic;
    __u8            rescap;
    __u8            fpi;
    __u8            dlfeat;
    __le16          nawun;
    __le16          nawupf;
    __le16          nacwu;
    __le16          nabsn;
    __le16          nabo;
    __le16          nabspf;
    __le16          noiob;
    __u8            nvmcap[16];
    __le16          npwg;
    __le16          npwa;
    __le16          npdg;
    __le16          npda;
    __le16          nows;
    __le16          mssrl;
    __le32          mcl;
    __u8            msrc;
    __u8            rsvd81[11];
    __le32          anagrpid;
    __u8            rsvd96[3];
    __u8            nsattr;
    __le16          nvmsetid;
    __le16          endgid;
    __u8            nguid[16];
    __u8            eui64[8];
    struct nvme_lbaf    lbaf[16];
    __u8            rsvd192[192];
    __u8            vs[3712];
};
#endif
