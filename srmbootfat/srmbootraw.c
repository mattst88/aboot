/*
 *  SRMbootraw - SRM boot block composer for raw boot partitions.
 *  Copyright (C) 1998 Nikita Schmidt  <cetus@snowball.ucd.ie>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

/* We assume little endiannes and possibly other Alpha properties as well. */
#ifndef __alpha__
#error This stuff must be compiled for Alpha.
#endif

#define BUFSIZE	65536

union bootsector {
	struct {
		unsigned char textlabel[64];
		unsigned char disklabel[276];
		unsigned char unused[140];
		u_int64_t count, start, flags;
		u_int64_t checksum;
	} srm;
	struct {
		u_int64_t contents[63];
		u_int64_t checksum;
	} check;
};


int main (int argc, char *argv[])
{
	int device, image;
	int i;
	union bootsector boot;
	char *buf;
	unsigned long size;
	ssize_t len;
	u_int64_t checksum;

	if (argc != 3)
		return printf ("Usage:  srmbootraw <boot device> <boot image>\n"), 1;

	if ((device = open (argv[1], O_RDWR)) < 0)
		return perror (argv[1]), 2;
	if ((image = open (argv[2], O_RDONLY)) < 0)
		return perror (argv[2]), 2;

	/* Read in the old bootsector */
	if (read (device, &boot, sizeof boot) != sizeof boot)
		return fprintf (stderr, "Can't read boot sector from %s\n", argv[1]), 2;

	if (!(buf = malloc (BUFSIZE)))
		return fprintf (stderr, "Can't allocate memory for %s\n", argv[2]), 2;

	/* Copy image onto the device */
	size = 0;
	while ((len = read (image, buf, BUFSIZE)) > 0)
	{
		if (write (device, buf, len) != len)
			return fprintf (stderr, "Can't write to %s\n", argv[1]), 2;
		size += len;
	}
	close (image);
	if (len == -1)
		return perror (argv[2]), 2;

	/* Fill in the bootstrap information */
	boot.srm.start = 1;
	boot.srm.count = (size + 511) / 512;	/* Convert to sectors */
	boot.srm.flags = 0;

	/* Put the checksum and write the boot sector. */
	checksum = 0;
	for (i = 0; i < 63; i++)
		checksum += boot.check.contents[i];
	boot.check.checksum = checksum;

	printf ("Writing SRM boot block: starting sector %u, block count %u\n",
		(unsigned)boot.srm.start, (unsigned)boot.srm.count);

	if (lseek (device, 0, SEEK_SET) == -1
	 || write (device, &boot, sizeof boot) != sizeof boot)
		return perror (argv[2]), 2;

	close (device);
	return 0;
}
