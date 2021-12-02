CFLAGS ?= -O2 -g -Wall -Werror -fcommon
override CFLAGS += -std=gnu99 -I.
override CPPFLAGS += -D_GNU_SOURCE -D__CHECK_ENDIAN__
NVME-MI = nvme-mi
INSTALL ?= install

INC=-Iutil

AUTHOR=Arun Kumar Kashinath Agasar <arun.kka@samsung.com>

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	QUIET_CC	= @echo '   ' CC $@;
endif
endif

default: $(NVME-MI)

NVME_DPKG_VERSION=1~`lsb_release -sc`

OBJS := plugin.o \

UTIL_OBJS := util/argconfig.o util/suffix.o\


MI_OBJS :=					\
	mi/mi-util/hal/mi-nvme-hal-main.o 		\
	mi/mi-util/hal/mi-nvme-qemu/mi-nvme-qemu.o \
	mi/mi-util/mi-nvme-util-base.o			\
	mi/mi-util/mi-nvme-util-crc.o 			\
	mi/mi-util/mi-nvme-util-tool.o 			\
	mi/mi-nvme-cmd.o

nvme-mi: mi_nvme.c mi_nvme.h $(OBJS) $(MI_OBJS) $(UTIL_OBJS)
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) $< -o $(NVME-MI) $(OBJS) $(MI_OBJS) $(UTIL_OBJS) $(LDFLAGS)

verify-no-dep: mi_nvme.c mi_nvme.h $(OBJS) $(UTIL_OBJS)
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) $< -o $@ $(OBJS) $(UTIL_OBJS) $(LDFLAGS)

nvme-mi.o: mi_nvme.c mi_nvme.h util/argconfig.h util/suffix.h
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) -c $<

%.o: %.c %.h mi_nvme.h util/argconfig.h
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) -o $@ -c $<

%.o: %.c mi_nvme.h util/argconfig.h
	$(QUIET_CC)$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) -o $@ -c $<

doc: $(NVME-MI)
	$(MAKE) -C Documentation

all: doc

clean:
	$(RM) $(NVME-MI) $(MI_OBJS) $(UTIL_OBJS) *~ a.out *.tar* nvme.spec version control nvme-*.deb 70-nvmf-autoconnect.conf
