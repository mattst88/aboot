/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: ufs.c,v $
 * Revision 1.1  2001/10/08 23:03:52  wgwoods
 * Initial revision
 *
 * Revision 1.1.1.1  2000/05/03 03:58:22  dhd
 * Initial import (from 0.7 release)
 *
 * Revision 2.2  93/02/05  08:01:36  danner
 * 	Adapted for alpha.
 * 	[93/02/04            af]
 * 
 */
/*
 *	File: ufs.c
 * 	Author: David Golub, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Stand-alone file reading package.
 *
 *	Modified for use by Linux/Alpha by David Mosberger
 *	(davidm@cs.arizona.edu)
 */
#include <linux/kernel.h>
#include <asm/stat.h>
#ifndef TESTING
#  include <linux/string.h>
#endif

#include "aboot.h"
#include "bootfs.h"
#include "cons.h"
#include "disklabel.h"
#include "ufs.h"
#include "utils.h"

#define MAX_OPEN_FILES	1

extern struct bootfs ufs;

static long dev;
static long partition_offset;
static struct fs *fs;
static struct file {
	int 		inuse;
	struct icommon	i_ic;		/* copy of on-disk inode */
	int		f_nindir[NIADDR+1];
					/* number of blocks mapped by
					   indirect block at level i */
	void		*f_blk[NIADDR];	/* buffer for indir block at level i */
	long		f_blksize[NIADDR];
					/* size of buffer */
	daddr_t		f_blkno[NIADDR];
					/* disk address of block in buffer */
	void		*f_buf;		/* buffer for data block */
	long		f_buf_size;	/* size of data block */
	daddr_t		f_buf_blkno;	/* block number of data block */
} inode_table[MAX_OPEN_FILES];


static int read_inode(ino_t inumber, struct file *fp)
{
	daddr_t disk_block;
	long offset;
	struct dinode *dp;
	int level;

	disk_block = itod(fs, inumber);

	offset = fsbtodb(fs, disk_block) * DEV_BSIZE + partition_offset;
	if (cons_read(dev, fp->f_buf, fs->fs_bsize, offset) != fs->fs_bsize) {
		printf("ufs_read_inode: read error\n");
		return 1;
	}
	dp = (struct dinode *)fp->f_buf;
	dp += itoo(fs, inumber);
	fp->i_ic = dp->di_ic;
	/*
	 * Clear out the old buffers
	 */
	for (level = 0; level < NIADDR; level++) {
		if (fp->f_blk[level]) {
			free(fp->f_blk[level]);
			fp->f_blk[level] = 0;
		}
		fp->f_blkno[level] = -1;
	}
	return 0;
}


/*
 * Given an offset in a file, find the disk block number that
 * contains that block.
 */
static daddr_t block_map(struct file *fp, daddr_t file_block)
{
	daddr_t ind_block_num, *ind_p;
	int level, idx;
	long offset;
	/*
	 * Index structure of an inode:
	 *
	 * i_db[0..NDADDR-1]	hold block numbers for blocks
	 *			0..NDADDR-1
	 *
	 * i_ib[0]		index block 0 is the single indirect
	 *			block
	 *			holds block numbers for blocks
	 *			NDADDR .. NDADDR + NINDIR(fs)-1
	 *
	 * i_ib[1]		index block 1 is the double indirect
	 *			block
	 *			holds block numbers for INDEX blocks
	 *			for blocks
	 *			NDADDR + NINDIR(fs) ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 - 1
	 *
	 * i_ib[2]		index block 2 is the triple indirect
	 *			block
	 *			holds block numbers for double-indirect
	 *			blocks for blocks
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2
	 *				+ NINDIR(fs)**3 - 1
	 */
	if (file_block < NDADDR) {
		/* Direct block. */
		return fp->i_db[file_block];
	}

	file_block -= NDADDR;

	/*
	 * nindir[0] = NINDIR
	 * nindir[1] = NINDIR**2
	 * nindir[2] = NINDIR**3
	 *	etc
	 */
	for (level = 0; level < NIADDR; level++) {
		if (file_block < fp->f_nindir[level])
		  break;
		file_block -= fp->f_nindir[level];
	}
	if (level == NIADDR) {
		printf("ufs_block_map: block number too high\n");
		return -1;
	}

	ind_block_num = fp->i_ib[level];

	for (; level >= 0; level--) {
		if (ind_block_num == 0) {
			return 0;
		}

		if (fp->f_blkno[level] != ind_block_num) {
			if (fp->f_blk[level]) {
				free(fp->f_blk[level]);
			}

			offset = fsbtodb(fs, ind_block_num) * DEV_BSIZE
			  + partition_offset;
			fp->f_blk[level] = malloc(fs->fs_bsize);
			if (cons_read(dev, fp->f_blk[level], fs->fs_bsize,
				      offset)
			    != fs->fs_bsize) 
			{
				printf("ufs_block_map: read error\n");
				return -1;
			}
			fp->f_blkno[level] = ind_block_num;
		}

		ind_p = (daddr_t *)fp->f_blk[level];

		if (level > 0) {
			idx = file_block / fp->f_nindir[level-1];
			file_block %= fp->f_nindir[level-1];
		} else {
			idx = file_block;
		}
		ind_block_num = ind_p[idx];
	}
	return ind_block_num;
}


static int breadi(struct file *fp, long blkno, long nblks, char *buffer)
{
	long block_size, offset, tot_bytes, nbytes, ncontig;
	daddr_t disk_block;

	tot_bytes = 0;
	while (nblks) {
		/*
		 * Contiguous reads are a lot faster, so we try to group
		 * as many blocks as possible:
		 */
		ncontig = 0;	/* # of *fragments* that are contiguous */
		nbytes = 0;
		disk_block = block_map(fp, blkno);
		do {
			block_size = blksize(fs, fp, blkno);
			nbytes += block_size;
			ncontig += numfrags(fs, block_size);
			++blkno; --nblks;
		} while (nblks &&
			 block_map(fp, blkno) == disk_block + ncontig);

		if (!disk_block) {
			/* it's a hole... */
			memset(buffer, 0, nbytes);
		} else {
			offset = fsbtodb(fs, disk_block) * DEV_BSIZE
			  + partition_offset;
			if (cons_read(dev, buffer, nbytes, offset) != nbytes) {
				printf("ufs_breadi: read error\n");
				return -1;
			}
		}
		buffer    += nbytes;
		tot_bytes += nbytes;
	}
	return tot_bytes;
}


/*
 * Search a directory for a name and return its
 * i_number.
 */
static int search_dir(const char *name, struct file *fp, ino_t *inumber_p)
{
	long offset, blockoffset;
	struct direct *dp;
	int len;

	len = strlen(name);

	offset = 0;
	while (offset < fp->i_size) {
		blockoffset = 0;
		if (breadi(fp, offset / fs->fs_bsize, 1, fp->f_buf) < 0) {
			return -1;
		}
		while (blockoffset < fs->fs_bsize) {
			dp = (struct direct *)((char*)fp->f_buf + blockoffset);
			if (dp->d_ino) {
				if (dp->d_namlen == len
				    && strcmp(name, dp->d_name) == 0)
				{
					/* found entry */
					*inumber_p = dp->d_ino;
					return 0;
				}
			}
			blockoffset += dp->d_reclen;
		}
		offset += fs->fs_bsize;
	}
	return -1;
}


/*
 * Initialize a BSD FFS partition starting at offset P_OFFSET; this is
 * sort-of the same idea as "mounting" it.  Read in the relevant
 * control structures and make them available to the user.  Returns 0
 * if successful, -1 on failure.
 */
static int ufs_mount(long cons_dev, long p_offset, long quiet)
{
	static char buf[SBSIZE];	/* minimize frame size */
	long rc;

	memset(&inode_table, 0, sizeof(inode_table));

	dev = cons_dev;
	partition_offset = p_offset;

	rc = cons_read(dev, buf, SBSIZE, SBLOCK*DEV_BSIZE + partition_offset);
	if (rc != SBSIZE)
	{
		printf("ufs_mount: superblock read failed (retval=%ld)\n", rc);
		return -1;
	}

	fs = (struct fs *)buf;
	if (fs->fs_magic != FS_MAGIC ||
	    fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < (int) sizeof(struct fs))
	{
		if (!quiet) {
			printf("ufs_mount: invalid superblock "
			       "(magic=%x, bsize=%d)\n",
			       fs->fs_magic, fs->fs_bsize);
		}
		return -1;
	}
	ufs.blocksize = fs->fs_bsize;

	/* don't read cylinder groups - we aren't modifying anything */
	return 0;
}


static int ufs_open(const char *path)
{
	char *cp = 0, *component;
	int fd;
	ino_t inumber, parent_inumber;
	int nlinks = 0;
	struct file *fp;
	static char namebuf[MAXPATHLEN+1];
	
	if (!path || !*path) {
		return -1;
	}

	for (fd = 0; inode_table[fd].inuse; ++fd) {
		if (fd >= MAX_OPEN_FILES) {
			return -1;
		}
	}
	fp = &inode_table[fd];
	fp->f_buf_size = fs->fs_bsize;
	fp->f_buf = malloc(fp->f_buf_size);

	/* copy name into buffer to allow modifying it: */
	memcpy(namebuf, path, (unsigned)(strlen(path) + 1));

	inumber = (ino_t) ROOTINO;
	if (read_inode(inumber, fp) < 0) {
		return -1;
	}

	component = strtok(namebuf, "/");
	while (component) {
		/* verify that current node is a directory: */
		if ((fp->i_mode & IFMT) != IFDIR) {
			return -1;
		}
		/*
		 * Look up component in current directory.
		 * Save directory inumber in case we find a
		 * symbolic link.
		 */
		parent_inumber = inumber;
		if (search_dir(component, fp, &inumber))
		  return -1;

		/* open next component: */
		if (read_inode(inumber, fp))
		  return -1;

		/* check for symbolic link: */
		if ((fp->i_mode & IFMT) == IFLNK) {
			int link_len = fp->i_size;
			int len;

			len = strlen(cp) + 1;

			if (link_len + len >= MAXPATHLEN - 1) {
				return -1;
			}

			if (++nlinks > MAXSYMLINKS) {
				return FS_SYMLINK_LOOP;
			}
			memcpy(&namebuf[link_len], cp, len);
#ifdef IC_FASTLINK
			if ((fp->i_flags & IC_FASTLINK) != 0) {
				memcpy(namebuf, fp->i_symlink, link_len);
			} else
#endif /* IC_FASTLINK */
			{
				/* read file for symbolic link: */
				long rc, offset;
				daddr_t	disk_block;

				disk_block = block_map(fp, (daddr_t)0);
				offset = fsbtodb(fs, disk_block) * DEV_BSIZE
				  + partition_offset;
				rc = cons_read(dev, namebuf, sizeof(namebuf),
					       offset);
				if (rc != sizeof(namebuf)) {
					return -1;
				}
			}
			/*
			 * If relative pathname, restart at parent directory.
			 * If absolute pathname, restart at root.
			 */
			cp = namebuf;
			if (*cp != '/') {
				inumber = parent_inumber;
			} else
			  inumber = (ino_t)ROOTINO;

			if (read_inode(inumber, fp))
			  return -1;
		}
		component = strtok(NULL, "/");
	}
	/* calculate indirect block levels: */
	{
		register int mult;
		register int level;

		mult = 1;
		for (level = 0; level < NIADDR; level++) {
			mult *= NINDIR(fs);
			fp->f_nindir[level] = mult;
		}
	}
	return fd;
}


static int ufs_bread(int fd, long blkno, long nblks, char *buffer)
{
	struct file *fp;

	fp = &inode_table[fd];
	return breadi(fp, blkno, nblks, buffer);
}


static void ufs_close(int fd)
{
	inode_table[fd].inuse = 0;
}

static const char *
ufs_readdir(int fd, int rewind)
{
	return NULL;
}

static int
ufs_fstat(int fd, struct stat* buf)
{
	return -1;
}

struct bootfs ufs = {
	FS_BSDFFS, 0,
	ufs_mount,
	ufs_open,  ufs_bread,  ufs_close, ufs_readdir, ufs_fstat
};
