/*
 * isomarkboot.c
 *
 * This file is part of aboot, the SRM bootloader for Linux/Alpha
 * Copyright (C) 1996 David Mosberger.
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
/*
 * Making an ISO9660 filesystem bootable is straight-forward since all
 * files are contiguous.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>

#include <config.h>
#include <isolib.h>
#include <iso.h>

const char * prog_name;

static int disk;

void
__memcpy (void * dest, const void * src, size_t n)
{
	memcpy (dest, src, n);
}

long
iso_dev_read (void * buf, long offset, long size)
{
	if (lseek(disk, offset, SEEK_SET) != offset) {
		perror("lseek");
		return -1;
	}
	return read(disk, buf, size);
}

/* Write a 64-bit quantity out into memory in LITTLE ENDIAN order */
static void write_64 (unsigned char* out, unsigned long long in)
{
	out[0] = in & 0xFF;
	out[1] = (in >> 8) & 0xFF;
	out[2] = (in >> 16) & 0xFF;
	out[3] = (in >> 24) & 0xFF;
	out[4] = (in >> 32) & 0xFF;
	out[5] = (in >> 40) & 0xFF;
	out[6] = (in >> 48) & 0xFF;
	out[7] = (in >> 56) & 0xFF;
}

/* Read in a 64-bit LITTLE ENDIAN quantity */
static unsigned long long read_64 (unsigned char *in)
{
	unsigned long long result = 0;

	result |= (unsigned long long) in[0];
	result |= (unsigned long long) in[1] << 8;
	result |= (unsigned long long) in[2] << 16;
	result |= (unsigned long long) in[3] << 24;
	result |= (unsigned long long) in[4] << 32;
	result |= (unsigned long long) in[5] << 40;
	result |= (unsigned long long) in[6] << 48;
	result |= (unsigned long long) in[7] << 56;

	return result;
}

int
main (int argc, char ** argv)
{
	u_int64_t sector[512 / 8], sum;
        struct iso_primary_descriptor vol_desc;
	size_t nbytes, aboot_size;
	off_t aboot_pos;
	int i, aboot_fd;
        int rootbin_fd;
        off_t rootbin_pos;
        char root_start[100];

	prog_name = argv[0];

	if (argc < 3 || argc > 4) {
	    fprintf(stderr, "usage: %s filesys path [root.bin]\n", prog_name);
	    exit(1);
	}
	disk = open(argv[1], O_RDWR);
	if (disk < 0) {
		perror(argv[1]);
		exit(1);
	}

	if (iso_read_super (0, 0) < 0) {
		fprintf(stderr, "%s: cannot mount\n", argv[1]);
		exit(1);
	}

	aboot_fd = iso_open(argv[2]);
	if (aboot_fd < 0) {
		fprintf(stderr, "%s: file not found\n", argv[2]);
		exit(1);
	}

	{
		struct stat buf;
		iso_fstat(aboot_fd, &buf);
		aboot_size = buf.st_size;
	}
		
	aboot_pos = iso_map (aboot_fd, 0);

	printf("%s: %s is at offset %ld and is %lu bytes long\n",
	       prog_name, argv[2], aboot_pos, aboot_size);

	if (lseek(disk, 0, SEEK_SET) != 0) {
		perror("lseek");
		return -1;
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

	strcpy((char *) sector, "Linux/Alpha aboot for ISO filesystem.");
	write_64 ((unsigned char *) &sector[60], aboot_size /  512);/* sector count */
	write_64 ((unsigned char *) &sector[61], aboot_pos /  512); /* starting LBM */
	write_64 ((unsigned char *) &sector[62], 0);                /* flags */

	/* update checksum: */
	sum = 0;
	for (i = 0; i < 63; i++)
		sum += read_64 ((unsigned char *) &sector[i]);

	write_64 ((unsigned char *) &sector[63], sum);

	if (lseek(disk, 0, SEEK_SET) != 0) {
		perror("lseek");
		return -1;
	}

	nbytes = write(disk, sector, sizeof(sector));
	if (nbytes != sizeof(sector)) {
		if ((long) nbytes < 0) {
			perror("write");
		} else {
			fprintf(stderr, "%s: short write\n", prog_name);
		}
		exit(1);
	}

        if (argc < 4)
          return 0;

	rootbin_fd = iso_open(argv[3]);
	if (rootbin_fd < 0) {
		fprintf(stderr, "%s: file not found\n", argv[3]);
		exit(1);
	}

	rootbin_pos = iso_map (rootbin_fd, 0);
        iso_close(rootbin_fd);

	{
		struct stat buf;
		iso_fstat(rootbin_fd, &buf);
		printf("%s: %s is at offset %ld and is %lu bytes long\n",
		       prog_name, argv[3], rootbin_pos, buf.st_size);
	}

        
	if (lseek(disk, 16*2048, SEEK_SET) != 16*2048) {
		perror("lseek");
		return -1;
	}
	nbytes = read(disk, &vol_desc, sizeof(vol_desc));
	if (nbytes != sizeof(vol_desc)) {
          if ((long) nbytes < 0) {
            perror("read");
          } else {
            fprintf(stderr, "%s: short read\n", prog_name);
          }
          exit(1);
	}

        if (strncmp (vol_desc.id, ISO_STANDARD_ID, sizeof vol_desc.id) != 0) {
          fprintf(stderr,"first volume descriptor has not an ISO_STANDARD_ID!!\n");
          exit(1);
        }
        if (isonum_711 (vol_desc.type) != ISO_VD_PRIMARY) {
          fprintf(stderr,"first volume descriptor is not a primary one!!\n");
          exit(1);
        }
        if (rootbin_pos & 2047) {
          fprintf(stderr,"erreur:rootbin_pos=%ld is not at a isoblock boundary\n",rootbin_pos);
            exit(1);
        }
        sprintf(root_start,"ROOT START=%ld        ",rootbin_pos/2048);
        printf("writing %s in application_data of first volume descriptor\n", root_start); 
        memcpy(vol_desc.application_data,root_start,strlen(root_start));
	if (lseek(disk, 16*2048, SEEK_SET) != 16*2048) {
		perror("lseek");
		return -1;
	}
        
	nbytes = write(disk, &vol_desc, sizeof(vol_desc));
	if (nbytes != sizeof(vol_desc)) {
          if ((long) nbytes < 0) {
			perror("write");
		} else {
                  fprintf(stderr, "%s: short write\n", prog_name);
		}
		exit(1);
	}
        
	return 0;
}
