/*
 * abootconf.c
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/fcntl.h>

#include <config.h>


const char * prog_name;

int
main (int argc, char ** argv)
{
	unsigned long sector[512 / sizeof(unsigned long)];
	off_t aboot_pos;
	size_t nbytes;
	long part = -1;
	int disk, i;

	prog_name = argv[0];

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "usage: %s device [partition]\n", prog_name);
		exit(1);
	}

	if (argc > 2) {
	    errno = 0;
	    part = strtol(argv[2], 0, 10);
	    if (errno == ERANGE) {
		fprintf(stderr, "%s: bad partition number %s\n",
			prog_name, argv[2]);
		exit(1);
	    }
	}

	disk = open(argv[1], part < 0 ? O_RDONLY : O_RDWR);
	if (disk < 0) {
		perror(argv[1]);
		exit(1);
	}

	nbytes = read(disk, sector, sizeof(sector));
	if (nbytes != sizeof(sector)) {
		if ((long) nbytes < 0) {
			perror("read");
		} else {
			fprintf(stderr, "%s: short read\n", prog_name);
		}
		exit(1);
	}

	aboot_pos = sector[61] * 512;

	if (lseek(disk, aboot_pos, SEEK_SET) != aboot_pos) {
		perror("lseek");
		exit(1);
	}

	nbytes = read(disk, sector, sizeof(sector));
	if (nbytes != sizeof(sector)) {
		if ((long) nbytes < 0) {
			perror("read");
		} else {
			fprintf(stderr, "%s: short read\n", prog_name);
		}
		exit(1);
	}

	for (i = 0; i < (int) (sizeof(sector)/sizeof(sector[0])); ++i) {
		if (sector[i] == ABOOT_MAGIC)
			break;
	}
	if (i >= (int) (sizeof(sector)/sizeof(sector[0]))) {
		fprintf(stderr, "%s: could not find aboot on disk %s\n",
			prog_name, argv[1]);
		exit(1);
	}

	if (part < 0) {
	    printf("aboot.conf partition currently set to %ld\n",
		   sector[i + 1]);
	    exit(0);
	}

	if (lseek(disk, aboot_pos, SEEK_SET) != aboot_pos) {
		perror("lseek");
		exit(1);
	}

	sector[i + 1] = atoi(argv[2]);

	nbytes = write(disk, sector, sizeof(sector));
	if (nbytes != sizeof(sector)) {
		if ((long) nbytes < 0) {
			perror("write");
		} else {
			fprintf(stderr, "%s: short write\n", prog_name);
		}
		exit(1);
	}
	return 0;
}
