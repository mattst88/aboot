/*
 * aboot.c
 *
 * This file is part of aboot, the SRM bootloader for Linux/Alpha
 * Copyright (C) 1996 Linus Torvalds, David Mosberger, and Michael Schwingen.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/elf.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#ifndef TESTING
#  include <linux/string.h>
#endif
#include <linux/version.h>

#include <asm/console.h>
#include <asm/hwrpb.h>
#include <asm/system.h>

#include <alloca.h>

#include <aboot.h>
#include <config.h>
#include <cons.h>
#include <setjmp.h>
#include <utils.h>

#ifndef elf_check_arch
# define aboot_elf_check_arch(e)	1
#else
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#  define aboot_elf_check_arch(e)        elf_check_arch(e)
# else
#  define aboot_elf_check_arch(e)        elf_check_arch(e->e_machine)
# endif
#endif

struct bootfs *	bfs = 0;		/* filesystem to boot from */
char *		dest_addr = 0;
jmp_buf		jump_buffer;

unsigned long	start_addr = START_ADDR; /* default only */
struct segment* chunks;
int             nchunks;

char		boot_file[256] = "";
char		initrd_file[256] = "";
char		kernel_args[256] = "";
char *		bss_start = 0;
long		bss_size = 0;		 /* size of bss */
unsigned long   initrd_start = 0;
unsigned long   initrd_size = 0;

/* Offsets from start_addr (yes, they are negative, because start_addr
   is actually the load address plus the text offset) */
#define PARAM_OFFSET	-0x6000 /* load address + 0xa000 */
#define STACK_OFFSET	-0xe000 /* load address + 0x2000 */

/* eventually we may have to deal with 44-bit physical addressing and
   also possibly larger pages */
unsigned long	page_offset = 0xfffffc0000000000;
unsigned long   page_shift  = 13;

static unsigned long entry_addr = START_ADDR;

/*
 * The decompression code calls this function after decompressing the
 * first block of the object file.  The first block must contain all
 * the relevant header information.
 */
long
first_block (const char *buf, long blocksize)
{
	struct elfhdr *elf;
	struct elf_phdr *phdrs;

	elf  = (struct elfhdr *) buf;
	
	if (elf->e_ident[0] == 0x7f
	    && strncmp(elf->e_ident + 1, "ELF", 3) == 0)
	{
		int i;
		/* looks like an ELF binary: */
		if (elf->e_type != ET_EXEC) {
			printf("aboot: not an executable ELF file\n");
			return -1;
		}
		if (!aboot_elf_check_arch(elf)) {
			printf("aboot: ELF executable not for this machine\n");
			return -1;
		}
		if (elf->e_phoff + elf->e_phnum * sizeof(*phdrs) > (unsigned) blocksize) {
			printf("aboot: "
			       "ELF program headers not in first block (%ld)\n",
			       (long) elf->e_phoff);
			return -1;
		}
		phdrs = (struct elf_phdr *) (buf + elf->e_phoff);
		chunks = malloc(sizeof(struct segment) * elf->e_phnum);
		nchunks = elf->e_phnum;
		start_addr = phdrs[0].p_vaddr; /* assume they are sorted */
		entry_addr = elf->e_entry;
#ifdef DEBUG
		printf("aboot: %d program headers, start address %#lx, entry %#lx\n",
		       nchunks, start_addr, entry_addr);
#endif
		for (i = 0; i < elf->e_phnum; ++i) {
			chunks[i].addr   = phdrs[i].p_vaddr;
			chunks[i].offset = phdrs[i].p_offset;
			chunks[i].size   = phdrs[i].p_filesz;
#ifdef DEBUG
			printf("aboot: segment %d vaddr %#lx offset %#lx size %#lx\n",
			       i, chunks[i].addr, chunks[i].offset, chunks[i].size);
#endif
		}
		bss_start = (char *) (phdrs[elf->e_phnum - 1].p_vaddr +
				      phdrs[elf->e_phnum - 1].p_filesz);
		bss_size = (phdrs[elf->e_phnum - 1].p_memsz -
			    phdrs[elf->e_phnum - 1].p_filesz);
#ifdef DEBUG
		printf("aboot: bss at 0x%p, size %#lx\n", bss_start, bss_size);
#endif
	} else {
		/* Fail silently, it might be a compressed file */
		return -1;
	}

	return 0;
}

static void
get_boot_args(void)
{
	long result;

	/* get boot command line: */
#ifdef TESTING
	const char *e;
	if ((e = getenv("BOOTED_FILE"))) {
		strncpy(boot_file, e, sizeof(boot_file)-1);
		boot_file[sizeof(boot_file)-1] = 0;
	} else {
		strcpy(boot_file, "vmlinux.gz");
	}
	if ((e = getenv("BOOTED_OSFLAGS"))) {
		strncpy(kernel_args, e, sizeof(kernel_args)-1);
		kernel_args[sizeof(kernel_args)-1] = 0;
	} else {
		strcpy(kernel_args, "i");
	}
#else
	result = cons_getenv(ENV_BOOTED_FILE, boot_file, sizeof(boot_file));
	if (result < 0) {
		printf("aboot: warning: can't get ENV_BOOTED_FILE "
		       "(result=%lx)!\n", result);
		strcpy(boot_file, "vmlinux.gz");
	}
	result = cons_getenv(ENV_BOOTED_OSFLAGS,
			     kernel_args, sizeof(kernel_args));
	if  (result < 0) {
		printf("aboot: warning: can't get ENV_BOOTED_OSFLAGS "
		       "(result=%lx)!\n", result);
		strcpy(kernel_args, "i");
	}
#endif /* TESTING */
}

#ifdef TESTING
long config_file_partition = 1;
void halt()
{
	exit(0);
}

void unzip_error(char *x)
{
	printf("unzip: %s\n", x);
}


int main()
{
	extern long load_kernel();
	long result;

	get_boot_args();
	result = load_kernel();
	if (result < 0) {
		printf("aboot: kernel load failed (%ld)\n", result);
		return 0;
	}
	return 0;
}
#else /* not TESTING */
/*
 * Head transfers control to this function.  Don't call it main() to avoid
 * gcc doing magic initialization things that we don't want.
 */
void
main_ (void)
{
	extern long load_kernel (void);
	long i, result;

	cons_init();

	printf("aboot: Linux/Alpha SRM bootloader version "ABOOT_VERSION"\n");

	/* don't know how to deal with this yet */
	if (INIT_HWRPB->pagesize != 8192) {
		printf("aboot: expected 8kB pages, got %ldkB\n",
		       INIT_HWRPB->pagesize >> 10);
		return;
	}

	pal_init();
	get_boot_args();
	result = load_kernel();
	if (result < 0) {
		printf("aboot: kernel load failed (%ld)\n", result);
		return;
	}
	printf("aboot: starting kernel %s with arguments %s\n",
	       boot_file, kernel_args);
	strcpy((char*)start_addr + PARAM_OFFSET, kernel_args);
	*(unsigned long *)(start_addr + PARAM_OFFSET + 0x100)
		= initrd_start;
	*(unsigned long *)(start_addr + PARAM_OFFSET + 0x108)
		= initrd_size;

	run_kernel(entry_addr, start_addr + STACK_OFFSET);

	printf("aboot: kernel returned unexpectedly.  Halting slowly...\n");
	for (i = 0 ; i < 0x100000000 ; i++)
		/* nothing */;
	halt();
}
#endif /* TESTING */
