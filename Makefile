#
# aboot/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (c) 1995, 1996 by David Mosberger (davidm@cs.arizona.edu)
#

# location of linux kernel sources (must be absolute path):
KSRC		= /usr/src/linux
VMLINUX		= $(KSRC)/vmlinux
VMLINUXGZ	= $(KSRC)/arch/alpha/boot/vmlinux.gz

# for userspace testing
# TESTING	= yes

# for boot testing
#CFGDEFS       	= -DDEBUG_ISO -DDEBUG_ROCK -DDEBUG

# root, aka prefix
root		=
bindir		= $(root)/sbin
bootdir		= $(root)/boot
mandir		= /usr/man

#
# There shouldn't be any need to change anything below this line.
#
LOADADDR	= 20000000

ABOOT_LDFLAGS = -static -N -Taboot.lds

CC		= gcc
TOP		= $(shell pwd)
ifeq ($(TESTING),)
CPPFLAGS	= $(CFGDEFS) -I$(TOP)/include -I$(KSRC)/include
CFLAGS		= $(CPPFLAGS) -D__KERNEL__ -mcpu=ev4 -Os -Wall -Wcast-align -mno-fp-regs -ffixed-8
else
CPPFLAGS	= -DTESTING $(CFGDEFS) -I$(TOP)/include
CFLAGS		= $(CPPFLAGS) -O -g3 -Wall -D__KERNEL__ -ffixed-8
endif
ASFLAGS		= $(CPPFLAGS)


.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<
.S.s:
	$(CC) $(ASFLAGS) -D__ASSEMBLY__ -traditional -E -o $*.o $<
.S.o:
	$(CC) $(ASFLAGS) -D__ASSEMBLY__ -traditional -c -o $*.o $<

NET_OBJS = net.o
DISK_OBJS = disk.o fs/ext2.o fs/ufs.o fs/dummy.o fs/iso.o
ifeq ($(TESTING),)
ABOOT_OBJS = \
	head.o aboot.o cons.o utils.o \
	zip/misc.o zip/unzip.o zip/inflate.o
else
ABOOT_OBJS = aboot.o zip/misc.o zip/unzip.o zip/inflate.o
endif
LIBS	= lib/libaboot.a

ifeq ($(TESTING),)
all:	diskboot
else
all:	aboot
endif

diskboot:	bootlx sdisklabel/sdisklabel sdisklabel/swriteboot \
		tools/e2writeboot tools/isomarkboot tools/abootconf \
		tools/elfencap

netboot: vmlinux.bootp

bootlx:	aboot tools/objstrip
	tools/objstrip -vb aboot bootlx

install-man: 
	make -C doc/man install

install-man-gz:
	make -C doc/man install-gz

install: tools/abootconf tools/e2writeboot tools/isomarkboot \
	sdisklabel/swriteboot install-man
	install -d $(bindir) $(bootdir)
	install -c -s tools/abootconf $(bindir)
	install -c -s tools/e2writeboot $(bindir)
	install -c -s tools/isomarkboot $(bindir)
	install -c -s sdisklabel/swriteboot $(bindir)
	install -c bootlx $(bootdir)

installondisk:	bootlx sdisklabel/swriteboot
	sdisklabel/swriteboot -vf0 /dev/sda bootlx vmlinux.gz

ifeq ($(TESTING),)
aboot:	$(ABOOT_OBJS) $(DISK_OBJS) $(LIBS)
	$(LD) $(ABOOT_LDFLAGS) $(ABOOT_OBJS) $(DISK_OBJS) -o $@ $(LIBS)
else
aboot:	$(ABOOT_OBJS) $(DISK_OBJS) $(LIBS)
	$(CC) $(ABOOT_OBJS) $(DISK_OBJS) -o $@ $(LIBS)
endif

vmlinux.bootp: net_aboot.nh $(VMLINUXGZ) net_pad
	cat net_aboot.nh $(VMLINUXGZ) net_pad > $@

net_aboot.nh: net_aboot tools/objstrip
	tools/objstrip -vb net_aboot $@

net_aboot: $(ABOOT_OBJS) $(ABOOT_OBJS) $(NET_OBJS) $(LIBS)
	$(LD) $(ABOOT_LDFLAGS) $(ABOOT_OBJS) $(NET_OBJS) -o $@ $(LIBS)

net_pad:
	dd if=/dev/zero of=$@ bs=512 count=1

clean:	sdisklabel/clean tools/clean lib/clean
	rm -f aboot abootconf net_aboot net_aboot.nh net_pad vmlinux.bootp \
		$(ABOOT_OBJS) $(DISK_OBJS) $(NET_OBJS) bootlx \
		include/ksize.h vmlinux.nh

distclean: clean
	find . -name \*~ | xargs rm -f

lib/%:
	make -C lib $* CPPFLAGS="$(CPPFLAGS)" TESTING="$(TESTING)"

tools/%:
	make -C tools $* CPPFLAGS="$(CPPFLAGS)"

sdisklabel/%:
	make -C sdisklabel $* CPPFLAGS="$(CPPFLAGS)"

vmlinux.nh: $(VMLINUX) tools/objstrip
	tools/objstrip -vb $(VMLINUX) vmlinux.nh

include/ksize.h: vmlinux.nh
	echo "#define KERNEL_SIZE `ls -l vmlinux.nh | awk '{print $$5}'` > $@

dep:
