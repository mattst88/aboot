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
#include <asm/system.h>

#include <linux/string.h>

#include <config.h>
#include <cons.h>
#include <aboot.h>
#include <bootfs.h>
#include <utils.h>
#include "net.h"

extern char boot_file[256];
extern char _end;
char *src = 0; 

void
dang (void)
{
	printf("aboot: oops, unimplemented net-bfs function called!\n");
}


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

/* prerequisites: src points to the initrd in memory, initrd_size is set */
long read_initrd() {
	int nblocks;
	
	if (!free_mem_ptr) 
		free_mem_ptr = memory_end();
	/* page aligned (downward) */
	initrd_start = (free_mem_ptr - initrd_size) & ~(PAGE_SIZE-1);
	/* update free_mem_ptr so malloc() still works */
	free_mem_ptr = initrd_start;

	nblocks = ALIGN_512(initrd_size) / bfs->blocksize;
	printf("aboot: loading initrd (%ld bytes/%d blocks) at %#lx\n",
		initrd_size, nblocks, initrd_start);
	(*bfs->bread)(-1, 0, nblocks, (char*) initrd_start);
	return(0);
}

long
load_kernel (void)
{
	netabootheader_t *header;
	char *kernel, *initrd;
	bfs = &netfs;

	header=(netabootheader_t *)ALIGN_512(&_end);
	kernel=(char *)ALIGN_512(((unsigned long)header)+header->header_size);
	initrd=(char *)ALIGN_512(((unsigned long)kernel)+header->kernel_size);

	if (header->initrd_size) {
		src=initrd;
		initrd_size=header->initrd_size;
		read_initrd();
	}

	src=kernel;

	strcpy(boot_file, "network");
	strcpy(kernel_args, header->command_line);
	
	uncompress_kernel(-1);

	memset((char*)bss_start, 0, bss_size);	        /* clear bss */

	while (kernel_args[0] == 'i' && !kernel_args[1]) {
	    printf("Enter kernel arguments:\n");
	    printf("aboot> ");
	    getline(kernel_args, sizeof(kernel_args));
	    printf("\n");
	}
	return 0;
}
