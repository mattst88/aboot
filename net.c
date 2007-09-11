/*
 * net.c
 *
 * This file is part of aboot, the SRM bootloader for Linux/Alpha
 * Copyright (C) 1996 Dave Larson, and David Mosberger.
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
#include <asm/console.h>
#include "system.h"

#include "config.h"
#include "cons.h"
#include "aboot.h"
#include "bootfs.h"
#include "utils.h"
#include "string.h"
#include "netwrap.h"

extern char boot_file[256];

void
dang (void)
{
	printf("aboot: oops, unimplemented net-bfs function called!\n");
}

extern char _end;
static char *src = 0;
static char *kern_src=0, *ird_src=0;
static int  header_size=0, kern_size=0, ird_size=0;

int
net_bread (int fd, long blkno, long nblks, char * buf)
{
	int nbytes;

#ifdef DEBUG
	printf("net_bread: %p -> %p (%ld blocks at %ld)\n", src, buf,
	       nblks, blkno);
#endif
	nbytes = bfs->blocksize * nblks;

        memcpy(buf, src, nbytes);
        src += nbytes;

        return nbytes;
}


struct bootfs netfs = {
	-1, 512,
	(int (*)(long, long, long))	dang,	/* mount */
	(int (*)(const char *))		dang,	/* open */
	net_bread,				/* bread */
	(void (*)(int fd))		dang,	/* close */
	(const char* (*)(int, int))	dang,	/* readdir */
	(int (*)(int, struct stat*))	dang,	/* fstat */
};

long
read_initrd()
{
	int nblocks, nread;

	/* put it as high up in memory as possible */
	initrd_start = free_mem_ptr - align_pagesize(ird_size);
	initrd_size = ird_size;
	/* update free_mem_ptr so malloc() still works */
	free_mem_ptr = initrd_start;
#ifdef DEBUG
	printf("memory_end %x %x\n", free_mem_ptr, initrd_start);
#endif

	nblocks = align_512(ird_size)/ 512;
	printf("aboot: loading initrd (%d bytes/%d blocks) at %#lx\n",
	        ird_size, nblocks, initrd_start);
	nread = (*bfs->bread)(-1, 0, nblocks, (char*) initrd_start);
	return 0;
}



long
load_kernel (void)
{
	struct header *header;
	bfs = &netfs;

	header =  (struct header *)align_512( (unsigned long)&_end );
	header_size = header->header_size;
	kern_src = (char *)align_512((unsigned long)header + header_size);
	kern_size = header->kern_size;
	ird_src = (char *)align_512((unsigned long)kern_src + kern_size);
	ird_size = header->ird_size;

	if (!free_mem_ptr)
		free_mem_ptr = memory_end();
	free_mem_ptr = free_mem_ptr & ~(PAGE_SIZE-1);

#ifdef DEBUG
	printf("head %x %x kernel %x %x, initrd %x %x \n", header, header_size, kern_src, kern_size, ird_src, ird_size);
#endif

	if (ird_size) {
		src = ird_src;
		if (read_initrd() < 0) {
			return -1;
		}
	}

	strcpy(boot_file, "network");

	//Move kernel to safe place before uncompression
	src = (char*)free_mem_ptr - align_pagesize(kern_size);
	free_mem_ptr = (unsigned long)src;
	memcpy(src, kern_src, kern_size);

	uncompress_kernel(-1);

	memset((char*)bss_start, 0, bss_size);	        /* clear bss */

	if (!kernel_args[0] && header->boot_arg[0]) { //have argument?
		strncpy(kernel_args, header->boot_arg, header_size - sizeof(int)*3);
	}

	while (kernel_args[0] == 'i' && !kernel_args[1]) {
	    printf("Enter kernel arguments:\n");
	    printf("aboot> ");
	    getline(kernel_args, sizeof(kernel_args));
	    printf("\n");
	}
	return 0;
}
