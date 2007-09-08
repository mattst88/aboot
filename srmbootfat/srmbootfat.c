/*
 *  SRMbootFAT - SRM boot block composer for FAT filesystems.
 *  Copyright (C) 1998 Nikita Schmidt  <cetus@snowball.ucd.ie>
 *  msdos.h is Copyright (C) 1995 Alain Knaff.
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
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

#include "msdos.h"

/* This should test for little-endianness, but why bother,
   the whole thing is Alpha-specific anyway. */
#ifdef __alpha__	/* Little endian */
#define WORD(x)	 (*(u_int16_t *)(x))
#define DWORD(x) (*(u_int32_t *)(x))
#else			/* Big endian; in fact, generic stuff */
#define WORD	 _WORD
#define DWORD	 _DWORD
#endif

union full_bootsector {
	struct bootsector dos;
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

#ifdef __alpha__	/* Should be test for little endian */
unsigned int get_fat_entry (unsigned char *fat, int fatbits, unsigned entry)
{
	unsigned i, a, b;

	/* No check for fat boundaries... */
	switch (fatbits) {
	    case 12:
		i = ((entry *= 3) & 7) << 2;	/* Number of bits to shift */
		a = ((u_int32_t *)fat)[entry >>= 3];
		b = ((u_int32_t *)fat)[entry + 1];   /* May be outside FAT */
		return ((a >> i) | (b << (32 - i))) & 0xFFF;
	    case 16:
		return ((u_int16_t *)fat)[entry];
	}
	fprintf (stderr, "Unknown FAT type - FAT-%d\n", fatbits);
	exit (3);
}
#else
// Portable Version from Falk Hueffner <falk.hueffner@student.uni-tuebingen.de>
unsigned int get_fat_entry (unsigned char *fat, int fatbits, unsigned entry)
{
        unsigned i, a, b;

        /* No check for fat boundaries... */
        switch (fatbits) {
            case 12:
                i = ((entry *= 3) & 7) << 2;    /* Number of bits to shift */
                entry >>= 3;
                a = DWORD(fat + 4 * entry);
                b = DWORD(fat + 4 * entry + 4);
                return ((a >> i) | (b << (32 - i))) & 0xFFF;
            case 16:
                return WORD(fat + 2 * entry);
        }
        fprintf (stderr, "Unknown FAT type - FAT-%d\n", fatbits);
        exit (3);
}
#endif

int main (int argc, char *argv[])
{
	int f;
	int i;
	char *p;
	union full_bootsector boot;
	unsigned secsize;	/* Bytes per sector, hopefully 512 */
	unsigned clusize;    	/* Cluster size in sectors */
	unsigned fatstart;	/* Number of reserved (boot) sectors */
	unsigned nfat;		/* Number of FAT tables, hopefully 2 */
	unsigned dirents;	/* Number of directory slots */
	unsigned psect; 	/* Total sectors on disk */
	unsigned media;		/* Media descriptor=first byte of FAT */
	unsigned fatsize;	/* Sectors in FAT */
	unsigned fatbits;	/* FAT type (bits per entry) */
	unsigned char *fat;
	struct directory *rootdir;
	unsigned start;		/* Starting cluster of the boot file */
	unsigned size;		/* Boot file size */
	unsigned fat_end;
	unsigned j;
	u_int64_t checksum;
	char dosname[12];

	if (argc != 3)
		return printf ("Usage:  srmbootfat <filesystem image file> <boot file>\n"), 1;

	if ((f = open (argv[1], O_RDWR)) < 0)
		return perror (argv[1]), 2;

	if (read (f, &boot, sizeof boot) != sizeof boot)
		return fprintf (stderr, "Can't read boot sector from %s\n", argv[1]), 2;

	secsize = _WORD (boot.dos.secsiz);
	clusize = boot.dos.clsiz;
	fatstart = WORD (boot.dos.nrsvsect);
	nfat = boot.dos.nfat;
	dirents = _WORD (boot.dos.dirents);
	psect = _WORD (boot.dos.psect);
	media = boot.dos.descr;
	fatsize = WORD (boot.dos.fatlen);

	if ((media & ~7) == 0xf8) {
		i = media & 3;
		clusize = old_dos[i].cluster_size;
		fatstart = 1;
		fatsize = old_dos[i].fat_len;
		dirents = old_dos[i].dir_len * (512 / MDIR_SIZE);
		nfat = 2;
		fatbits = 12;
	} else if (strncmp (boot.dos.ext.old.fat_type, "FAT12", 5) == 0)
		fatbits = 12;
	else if (strncmp (boot.dos.ext.old.fat_type, "FAT16", 5) == 0)
		fatbits = 16;
	else return fprintf (stderr, "%s: unrecognisable FAT type\n", argv[1]),
			3;
#ifdef DEBUG
	printf ("%s: filesystem type is FAT-%d\n", argv[1], fatbits);
#endif

	if (secsize != 512)
		return fprintf (stderr, "%s: sector size is %d; "
				"unfortunately, this is not supported\n",
				argv[1], secsize),
			3;
	if (nfat != 1 && nfat != 2)
		fprintf (stderr,
			"%s: warning: weird number of FAT tables (%d)\n",
			argv[1], nfat);

	fat = malloc (i = fatsize * secsize);
	rootdir = malloc (dirents * MDIR_SIZE);
	if (!fat || !rootdir)
		return fprintf (stderr, "Not enough memory\n"), 2;
	if (lseek (f, fatstart * secsize, SEEK_SET) == -1
	 || read (f, fat, i) != i
	 || lseek (f, (nfat - 1) * i, SEEK_CUR) == -1
	 || read (f, rootdir, dirents * MDIR_SIZE) != dirents * MDIR_SIZE)
		return perror (argv[1]), 2;

	memset (dosname, ' ', sizeof dosname);
	i = 0;
	for (p = argv[2]; *p; p++)
		if (*p == '.')
			i = 8;
		else if (i < sizeof dosname)
			dosname[i++] = toupper (*p);

	for (i = 0; i < dirents; i++)
		if (memcmp (rootdir[i].name, dosname, 11) == 0
		 && (rootdir[i].attr & (8 | 16)) == 0)
			break;
	if (i == dirents)
		return fprintf (stderr,
			"Can't find %s in the root directory in %s\n",
			argv[2], argv[1]), 4;

	start = WORD (rootdir[i].start);
	size = DWORD (rootdir[i].size);

	if (start * fatbits > fatsize * secsize * 8)
		return fprintf (stderr,
			"%s: first cluster (%u) is beyond the end of FAT",
			argv[2], start), 3;

	/* Fill in the bootstrap information */
	size = (size + secsize - 1) / secsize;	/* Size is now in sectors */
	boot.srm.start = (start - 2) * clusize + fatstart + nfat * fatsize
		+ dirents / (512 / MDIR_SIZE);
	boot.srm.count = size;
	boot.srm.flags = 0;

	/* Check that the image is contiguous */
	i = 1;
	fat_end = (1 << fatbits) - 9;	/* 0xFF7, 0xFFF7 or whatever */
	while ((j = get_fat_entry (fat, fatbits, start)) < fat_end) {
		if (j != start + 1)
			return fprintf (stderr,
				"Unfortunately, %s is not contiguous\n",
				argv[2]), 4;
		start = j;
		i++;
	}
	if ((size + clusize - 1) / clusize != i)
		return fprintf (stderr, "Inconsistency: file size contradicts "
				"with the number of clusters allocated\n"), 3;

	/* Put the checksum and write the boot sector. */
	checksum = 0;
	for (i = 0; i < 63; i++)
		checksum += boot.check.contents[i];
	boot.check.checksum = checksum;

	printf ("Writing SRM boot block: starting sector %u, block count %u\n",
		(unsigned)boot.srm.start, (unsigned)boot.srm.count);

	if (lseek (f, 0, SEEK_SET) == -1
	 || write (f, &boot, sizeof boot) != sizeof boot)
		return perror (argv[2]), 2;

	close (f);
	return 0;
}

struct OldDos_t old_dos[]={
	{   40,  9,  1, 4, 1, 2, 0xfc },
	{   40,  9,  2, 7, 2, 2, 0xfd },
	{   40,  8,  1, 4, 1, 1, 0xfe },
	{   40,  8,  2, 7, 2, 1, 0xff },
	{   80,  9,  2, 7, 2, 3, 0xf9 },
	{   80, 15,  2,14, 1, 7, 0xf9 },
	{   80, 18,  2,14, 1, 9, 0xf0 },
	{   80, 36,  2,15, 2, 9, 0xf0 },
	{    1,  8,  1, 1, 1, 1, 0xf0 }
};
