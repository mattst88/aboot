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

extern char boot_file[256];

void
dang (void)
{
	printf("aboot: oops, unimplemented net-bfs function called!\n");
}


int
net_bread (int fd, long blkno, long nblks, char * buf)
{
        static char * src = 0;
	extern char _end;
	int nbytes;

	if (!src)
		src = (char *) (((unsigned long) &_end + 511) & ~511);

#ifdef DEBUG
	printf("net_bread: %p -> %p (%ld blocks at %ld)\n", src, buf,
	       nblks, blkno);
#else
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
load_kernel (void)
{
	long nbytes;

	bfs = &netfs;

	strcpy(boot_file, "network");
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
