/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
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
 * $Log: ufs.h,v $
 * Revision 1.1  2001/10/08 23:03:52  wgwoods
 * Initial revision
 *
 * Revision 1.1.1.1  2000/05/03 03:58:23  dhd
 * Initial import (from 0.7 release)
 *
 * Revision 2.3  93/03/09  10:49:48  danner
 * 	Make damn sure we get sys/types.h from the right place.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  08:01:43  danner
 * 	Adapted for alpha.
 * 	[93/02/04            af]
 * 
 * Revision 2.2  90/08/27  21:45:05  dbg
 * 	Created.
 * 	[90/07/16            dbg]
 * 
 */

/*
 * Common definitions for Berkeley Fast File System.
 */
#include <linux/types.h>

#define DEV_BSIZE	512

#ifndef NBBY
#define	NBBY	8
#endif

/*
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool.  It may be made larger without any effect on existing
 * file systems; however, making it smaller may make some file
 * systems unmountable.
 *
 * Note that the disk devices are assumed to have DEV_BSIZE "sectors"
 * and that fragments must be some multiple of this size.
 */
#define	MAXBSIZE	8192
#define	MAXFRAG		8

/*
 * MAXPATHLEN defines the longest permissible path length
 * after expanding symbolic links.
 *
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name.  It should be set
 * high enough to allow all legitimate uses, but halt infinite
 * loops reasonably quickly.
 */

#define	MAXPATHLEN	1024
#define	MAXSYMLINKS	8


/*
 * Error codes for file system errors.
 */

#define	FS_NOT_DIRECTORY	5000		/* not a directory */
#define	FS_NO_ENTRY		5001		/* name not found */
#define	FS_NAME_TOO_LONG	5002		/* name too long */
#define	FS_SYMLINK_LOOP		5003		/* symbolic link loop */
#define	FS_INVALID_FS		5004		/* bad file system */
#define	FS_NOT_IN_FILE		5005		/* offset not in file */
#define	FS_INVALID_PARAMETER	5006		/* bad parameter to
						   a routine */
/*
 * Each disk drive contains some number of file systems.
 * A file system consists of a number of cylinder groups.
 * Each cylinder group has inodes and data.
 *
 * A file system is described by its super-block, which in turn
 * describes the cylinder groups.  The super-block is critical
 * data and is replicated in each cylinder group to protect against
 * catastrophic loss.  This is done at `newfs' time and the critical
 * super-block data does not change, so the copies need not be
 * referenced further unless disaster strikes.
 *
 * For file system fs, the offsets of the various blocks of interest
 * are given in the super block as:
 *	[fs->fs_sblkno]		Super-block
 *	[fs->fs_cblkno]		Cylinder group block
 *	[fs->fs_iblkno]		Inode blocks
 *	[fs->fs_dblkno]		Data blocks
 * The beginning of cylinder group cg in fs, is given by
 * the ``cgbase(fs, cg)'' macro.
 *
 * The first boot and super blocks are given in absolute disk addresses.
 * The byte-offset forms are preferred, as they don't imply a sector size.
 */
#define BBSIZE		8192
#define SBSIZE		8192
#define	BBOFF		((off_t)(0))
#define	SBOFF		((off_t)(BBOFF + BBSIZE))
#define	BBLOCK		((daddr_t)(0))
#define	SBLOCK		((daddr_t)(BBLOCK + BBSIZE / DEV_BSIZE))

/*
 * Addresses stored in inodes are capable of addressing fragments
 * of `blocks'. File system blocks of at most size MAXBSIZE can 
 * be optionally broken into 2, 4, or 8 pieces, each of which is
 * addressable; these pieces may be DEV_BSIZE, or some multiple of
 * a DEV_BSIZE unit.
 *
 * Large files consist of exclusively large data blocks.  To avoid
 * undue wasted disk space, the last data block of a small file may be
 * allocated as only as many fragments of a large block as are
 * necessary.  The file system format retains only a single pointer
 * to such a fragment, which is a piece of a single large block that
 * has been divided.  The size of such a fragment is determinable from
 * information in the inode, using the ``blksize(fs, ip, lbn)'' macro.
 *
 * The file system records space availability at the fragment level;
 * to determine block availability, aligned fragments are examined.
 *
 * The root inode is the root of the file system.
 * Inode 0 can't be used for normal purposes and
 * historically bad blocks were linked to inode 1,
 * thus the root inode is 2. (inode 1 is no longer used for
 * this purpose, however numerous dump tapes make this
 * assumption, so we are stuck with it)
 */
#define	ROOTINO		((ino_t)2)	/* i number of all roots */

/*
 * MINBSIZE is the smallest allowable block size.
 * In order to insure that it is possible to create files of size
 * 2^32 with only two levels of indirection, MINBSIZE is set to 4096.
 * MINBSIZE must be big enough to hold a cylinder group block,
 * thus changes to (struct cg) must keep its size within MINBSIZE.
 * Note that super blocks are always of size SBSIZE,
 * and that both SBSIZE and MAXBSIZE must be >= MINBSIZE.
 */
#define MINBSIZE	4096

/*
 * The path name on which the file system is mounted is maintained
 * in fs_fsmnt. MAXMNTLEN defines the amount of space allocated in 
 * the super block for this name.
 * The limit on the amount of summary information per file system
 * is defined by MAXCSBUFS. It is currently parameterized for a
 * maximum of two million cylinders.
 */
#define MAXMNTLEN 512
#define MAXCSBUFS 32

/*
 * Per cylinder group information; summarized in blocks allocated
 * from first cylinder group data blocks.  These blocks have to be
 * read in from fs_csaddr (size fs_cssize) in addition to the
 * super block.
 *
 * N.B. sizeof(struct csum) must be a power of two in order for
 * the ``fs_cs'' macro to work (see below).
 */
struct csum {
	int	cs_ndir;	/* number of directories */
	int	cs_nbfree;	/* number of free blocks */
	int	cs_nifree;	/* number of free inodes */
	int	cs_nffree;	/* number of free frags */
};

typedef int ext_time_t;
typedef struct {
	unsigned int val[2];
} quad;

/*
 * Super block for a file system.
 */
#define	FS_MAGIC	0x011954
struct fs
{
	int	xxx1;			/* struct	fs *fs_link;*/
	int	xxx2;			/* struct	fs *fs_rlink;*/
	daddr_t	fs_sblkno;		/* addr of super-block in filesys */
	daddr_t	fs_cblkno;		/* offset of cyl-block in filesys */
	daddr_t	fs_iblkno;		/* offset of inode-blocks in filesys */
	daddr_t	fs_dblkno;		/* offset of first data after cg */
	int	fs_cgoffset;		/* cylinder group offset in cylinder */
	int	fs_cgmask;		/* used to calc mod fs_ntrak */
	ext_time_t fs_time;    		/* last time written */
	int	fs_size;		/* number of blocks in fs */
	int	fs_dsize;		/* number of data blocks in fs */
	int	fs_ncg;			/* number of cylinder groups */
	int	fs_bsize;		/* size of basic blocks in fs */
	int	fs_fsize;		/* size of frag blocks in fs */
	int	fs_frag;		/* number of frags in a block in fs */
/* these are configuration parameters */
	int	fs_minfree;		/* minimum percentage of free blocks */
	int	fs_rotdelay;		/* num of ms for optimal next block */
	int	fs_rps;			/* disk revolutions per second */
/* these fields can be computed from the others */
	int	fs_bmask;		/* ``blkoff'' calc of blk offsets */
	int	fs_fmask;		/* ``fragoff'' calc of frag offsets */
	int	fs_bshift;		/* ``lblkno'' calc of logical blkno */
	int	fs_fshift;		/* ``numfrags'' calc number of frags */
/* these are configuration parameters */
	int	fs_maxcontig;		/* max number of contiguous blks */
	int	fs_maxbpg;		/* max number of blks per cyl group */
/* these fields can be computed from the others */
	int	fs_fragshift;		/* block to frag shift */
	int	fs_fsbtodb;		/* fsbtodb and dbtofsb shift constant */
	int	fs_sbsize;		/* actual size of super block */
	int	fs_csmask;		/* csum block offset */
	int	fs_csshift;		/* csum block number */
	int	fs_nindir;		/* value of NINDIR */
	int	fs_inopb;		/* value of INOPB */
	int	fs_nspf;		/* value of NSPF */
/* yet another configuration parameter */
	int	fs_optim;		/* optimization preference, see below */
/* these fields are derived from the hardware */
	int	fs_npsect;		/* # sectors/track including spares */
	int	fs_interleave;		/* hardware sector interleave */
	int	fs_trackskew;		/* sector 0 skew, per track */
	int	fs_headswitch;		/* head switch time, usec */
	int	fs_trkseek;		/* track-to-track seek, usec */
/* sizes determined by number of cylinder groups and their sizes */
	daddr_t fs_csaddr;		/* blk addr of cyl grp summary area */
	int	fs_cssize;		/* size of cyl grp summary area */
	int	fs_cgsize;		/* cylinder group size */
/* these fields are derived from the hardware */
	int	fs_ntrak;		/* tracks per cylinder */
	int	fs_nsect;		/* sectors per track */
	int  	fs_spc;   		/* sectors per cylinder */
/* this comes from the disk driver partitioning */
	int	fs_ncyl;   		/* cylinders in file system */
/* these fields can be computed from the others */
	int	fs_cpg;			/* cylinders per group */
	int	fs_ipg;			/* inodes per group */
	int	fs_fpg;			/* blocks per group * fs_frag */
/* this data must be re-computed after crashes */
	struct	csum fs_cstotal;	/* cylinder summary information */
/* these fields are cleared at mount time */
	char   	fs_fmod;    		/* super block modified flag */
	char   	fs_clean;    		/* file system is clean flag */
	char   	fs_ronly;   		/* mounted read-only flag */
	char   	fs_flags;   		/* currently unused flag */
	char	fs_fsmnt[MAXMNTLEN];	/* name mounted on */
/* these fields retain the current block allocation info */
	int	fs_cgrotor;		/* last cg searched */
#ifdef __alpha__
	int	was_fs_csp[MAXCSBUFS];	/* unused on Alpha */
#else
	struct	csum *fs_csp[MAXCSBUFS];/* list of fs_cs info buffers */
#endif
	int	fs_cpc;			/* cyl per cycle in postbl */
	short	fs_opostbl[16][8];	/* old rotation block list head */
	int	fs_sparecon[56];	/* reserved for future constants */
	quad	fs_qbmask;		/* ~fs_bmask - for use with quad size */
	quad	fs_qfmask;		/* ~fs_fmask - for use with quad size */
	int	fs_postblformat;	/* format of positional layout tables */
	int	fs_nrpos;		/* number of rotaional positions */
	int	fs_postbloff;		/* (short) rotation block list head */
	int	fs_rotbloff;		/* (u_char) blocks for each rotation */
	int	fs_magic;		/* magic number */
	u_char	fs_space[1];		/* list of blocks for each rotation */
/* actually longer */
};
/*
 * Preference for optimization.
 */
#define FS_OPTTIME	0	/* minimize allocation time */
#define FS_OPTSPACE	1	/* minimize disk fragmentation */

/*
 * Rotational layout table format types
 */
#define FS_42POSTBLFMT		-1	/* 4.2BSD rotational table format */
#define FS_DYNAMICPOSTBLFMT	1	/* dynamic rotational table format */
/*
 * Macros for access to superblock array structures
 */
#define fs_postbl(fs, cylno) \
    (((fs)->fs_postblformat == FS_42POSTBLFMT) \
    ? ((fs)->fs_opostbl[cylno]) \
    : ((short *)((char *)(fs) + (fs)->fs_postbloff) + (cylno) * (fs)->fs_nrpos))
#define fs_rotbl(fs) \
    (((fs)->fs_postblformat == FS_42POSTBLFMT) \
    ? ((fs)->fs_space) \
    : ((u_char *)((char *)(fs) + (fs)->fs_rotbloff)))

/*
 * Convert cylinder group to base address of its global summary info.
 *
 * N.B. This macro assumes that sizeof(struct csum) is a power of two.
 */
#define fs_cs(fs, indx) \
	fs_csp[(indx) >> (fs)->fs_csshift][(indx) & ~(fs)->fs_csmask]

/*
 * Cylinder group block for a file system.
 */
#define	CG_MAGIC	0x090255
struct	cg {
	int	xxx1;			/* struct	cg *cg_link;*/
	int	cg_magic;		/* magic number */
	ext_time_t cg_time;		/* time last written */
	int	cg_cgx;			/* we are the cgx'th cylinder group */
	short	cg_ncyl;		/* number of cyl's this cg */
	short	cg_niblk;		/* number of inode blocks this cg */
	int	cg_ndblk;		/* number of data blocks this cg */
	struct	csum cg_cs;		/* cylinder summary information */
	int	cg_rotor;		/* position of last used block */
	int	cg_frotor;		/* position of last used frag */
	int	cg_irotor;		/* position of last used inode */
	int	cg_frsum[MAXFRAG];	/* counts of available frags */
	int	cg_btotoff;		/* (long) block totals per cylinder */
	int	cg_boff;		/* (short) free block positions */
	int	cg_iusedoff;		/* (char) used inode map */
	int	cg_freeoff;		/* (u_char) free block map */
	int	cg_nextfreeoff;		/* (u_char) next available space */
	int	cg_sparecon[16];	/* reserved for future use */
	u_char	cg_space[1];		/* space for cylinder group maps */
/* actually longer */
};
/*
 * Macros for access to cylinder group array structures
 */
#define cg_blktot(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_btot) \
    : ((int *)((char *)(cgp) + (cgp)->cg_btotoff)))
#define cg_blks(fs, cgp, cylno) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_b[cylno]) \
    : ((short *)((char *)(cgp) + (cgp)->cg_boff) + (cylno) * (fs)->fs_nrpos))
#define cg_inosused(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_iused) \
    : ((char *)((char *)(cgp) + (cgp)->cg_iusedoff)))
#define cg_blksfree(cgp) \
    (((cgp)->cg_magic != CG_MAGIC) \
    ? (((struct ocg *)(cgp))->cg_free) \
    : ((u_char *)((char *)(cgp) + (cgp)->cg_freeoff)))
#define cg_chkmagic(cgp) \
    ((cgp)->cg_magic == CG_MAGIC || ((struct ocg *)(cgp))->cg_magic == CG_MAGIC)

/*
 * The following structure is defined
 * for compatibility with old file systems.
 */
struct	ocg {
	int	xxx1;			/* struct	ocg *cg_link;*/
	int	xxx2;			/* struct	ocg *cg_rlink;*/
	ext_time_t cg_time;		/* time last written */
	int	cg_cgx;			/* we are the cgx'th cylinder group */
	short	cg_ncyl;		/* number of cyl's this cg */
	short	cg_niblk;		/* number of inode blocks this cg */
	int	cg_ndblk;		/* number of data blocks this cg */
	struct	csum cg_cs;		/* cylinder summary information */
	int	cg_rotor;		/* position of last used block */
	int	cg_frotor;		/* position of last used frag */
	int	cg_irotor;		/* position of last used inode */
	int	cg_frsum[8];		/* counts of available frags */
	int	cg_btot[32];		/* block totals per cylinder */
	short	cg_b[32][8];		/* positions of free blocks */
	char	cg_iused[256];		/* used inode map */
	int	cg_magic;		/* magic number */
	u_char	cg_free[1];		/* free block map */
/* actually longer */
};

/*
 * Turn file system block numbers into disk block addresses.
 * This maps file system blocks to device size blocks.
 */
#define fsbtodb(fs, b)	((b) << (fs)->fs_fsbtodb)
#define	dbtofsb(fs, b)	((b) >> (fs)->fs_fsbtodb)

/*
 * Cylinder group macros to locate things in cylinder groups.
 * They calc file system addresses of cylinder group data structures.
 */
#define	cgbase(fs, c)	((daddr_t)((fs)->fs_fpg * (c)))
#define cgstart(fs, c) \
	(cgbase(fs, c) + (fs)->fs_cgoffset * ((c) & ~((fs)->fs_cgmask)))
#define	cgsblock(fs, c)	(cgstart(fs, c) + (fs)->fs_sblkno)	/* super blk */
#define	cgtod(fs, c)	(cgstart(fs, c) + (fs)->fs_cblkno)	/* cg block */
#define	cgimin(fs, c)	(cgstart(fs, c) + (fs)->fs_iblkno)	/* inode blk */
#define	cgdmin(fs, c)	(cgstart(fs, c) + (fs)->fs_dblkno)	/* 1st data */

/*
 * Macros for handling inode numbers:
 *     inode number to file system block offset.
 *     inode number to cylinder group number.
 *     inode number to file system block address.
 */
#define	itoo(fs, x)	((x) % INOPB(fs))
#define	itog(fs, x)	((x) / (fs)->fs_ipg)
#define	itod(fs, x) \
	((daddr_t)(cgimin(fs, itog(fs, x)) + \
	(blkstofrags((fs), (((x) % (fs)->fs_ipg) / INOPB(fs))))))

/*
 * Give cylinder group number for a file system block.
 * Give cylinder group block number for a file system block.
 */
#define	dtog(fs, d)	((d) / (fs)->fs_fpg)
#define	dtogd(fs, d)	((d) % (fs)->fs_fpg)

/*
 * Extract the bits for a block from a map.
 * Compute the cylinder and rotational position of a cyl block addr.
 */
#define blkmap(fs, map, loc) \
    (((map)[(loc) / NBBY] >> ((loc) % NBBY)) & (0xff >> (NBBY - (fs)->fs_frag)))
#define cbtocylno(fs, bno) \
    ((bno) * NSPF(fs) / (fs)->fs_spc)
#define cbtorpos(fs, bno) \
    (((bno) * NSPF(fs) % (fs)->fs_spc / (fs)->fs_nsect * (fs)->fs_trackskew + \
     (bno) * NSPF(fs) % (fs)->fs_spc % (fs)->fs_nsect * (fs)->fs_interleave) % \
     (fs)->fs_nsect * (fs)->fs_nrpos / (fs)->fs_npsect)

/*
 * The following macros optimize certain frequently calculated
 * quantities by using shifts and masks in place of divisions
 * modulos and multiplications.
 */
#define blkoff(fs, loc)		/* calculates (loc % fs->fs_bsize) */ \
	((loc) & ~(fs)->fs_bmask)
#define fragoff(fs, loc)	/* calculates (loc % fs->fs_fsize) */ \
	((loc) & ~(fs)->fs_fmask)
#define lblkno(fs, loc)		/* calculates (loc / fs->fs_bsize) */ \
	((loc) >> (fs)->fs_bshift)
#define numfrags(fs, loc)	/* calculates (loc / fs->fs_fsize) */ \
	((loc) >> (fs)->fs_fshift)
#define blkroundup(fs, size)	/* calculates roundup(size, fs->fs_bsize) */ \
	(((size) + (fs)->fs_bsize - 1) & (fs)->fs_bmask)
#define fragroundup(fs, size)	/* calculates roundup(size, fs->fs_fsize) */ \
	(((size) + (fs)->fs_fsize - 1) & (fs)->fs_fmask)
#define fragstoblks(fs, frags)	/* calculates (frags / fs->fs_frag) */ \
	((frags) >> (fs)->fs_fragshift)
#define blkstofrags(fs, blks)	/* calculates (blks * fs->fs_frag) */ \
	((blks) << (fs)->fs_fragshift)
#define fragnum(fs, fsb)	/* calculates (fsb % fs->fs_frag) */ \
	((fsb) & ((fs)->fs_frag - 1))
#define blknum(fs, fsb)		/* calculates rounddown(fsb, fs->fs_frag) */ \
	((fsb) &~ ((fs)->fs_frag - 1))

/*
 * Determine the number of available frags given a
 * percentage to hold in reserve
 */
#define freespace(fs, percentreserved) \
	(blkstofrags((fs), (fs)->fs_cstotal.cs_nbfree) + \
	(fs)->fs_cstotal.cs_nffree - ((fs)->fs_dsize * (percentreserved) / 100))

/*
 * Determining the size of a file block in the file system.
 */
#define blksize(fs, ip, lbn) \
	(((lbn) >= NDADDR || (ip)->i_size >= ((lbn) + 1) << (fs)->fs_bshift) \
	    ? (fs)->fs_bsize \
	    : (fragroundup(fs, blkoff(fs, (ip)->i_size))))
#define dblksize(fs, dip, lbn) \
	(((lbn) >= NDADDR || (dip)->di_size >= ((lbn) + 1) << (fs)->fs_bshift) \
	    ? (fs)->fs_bsize \
	    : (fragroundup(fs, blkoff(fs, (dip)->di_size))))

/*
 * Number of disk sectors per block; assumes DEV_BSIZE byte sector size.
 */
#define	NSPB(fs)	((fs)->fs_nspf << (fs)->fs_fragshift)
#define	NSPF(fs)	((fs)->fs_nspf)

/*
 * INOPB is the number of inodes in a secondary storage block.
 */
#define	INOPB(fs)	((fs)->fs_inopb)
#define	INOPF(fs)	((fs)->fs_inopb >> (fs)->fs_fragshift)

/*
 * NINDIR is the number of indirects in a file system block.
 */
#define	NINDIR(fs)	((fs)->fs_nindir)

/*
 * Copyright (c) 1982, 1986, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)dir.h	7.6 (Berkeley) 5/9/89
 */

/*
 * A directory consists of some number of blocks of DIRBLKSIZ
 * bytes, where DIRBLKSIZ is chosen such that it can be transferred
 * to disk in a single atomic operation (e.g. 512 bytes on most machines).
 *
 * Each DIRBLKSIZ byte block contains some number of directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 *
 * The macro DIRSIZ(dp) gives the amount of space required to represent
 * a directory entry.  Free space in a directory is represented by
 * entries which have dp->d_reclen > DIRSIZ(dp).  All DIRBLKSIZ bytes
 * in a directory block are claimed by the directory entries.  This
 * usually results in the last entry in a directory having a large
 * dp->d_reclen.  When entries are deleted from a directory, the
 * space is returned to the previous entry in the same directory
 * block by increasing its dp->d_reclen.  If the first entry of
 * a directory block is free, then its dp->d_ino is set to 0.
 * Entries other than the first in a directory do not normally have
 * dp->d_ino set to 0.
 */
#define DIRBLKSIZ	DEV_BSIZE
#define	MAXNAMLEN	255

struct	direct {
	u_int	d_ino;			/* inode number of entry */
	u_short	d_reclen;		/* length of this record */
	u_short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN + 1];	/* name with length <= MAXNAMLEN */
};

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

/*
 * The I node is the focus of all file activity in the BSD Fast File System.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An inode is 'named' by its dev/inumber pair. (iget/iget.c)
 * Data in icommon is read in from permanent inode on volume.
 */

#define	NDADDR	12		/* direct addresses in inode */
#define	NIADDR	3		/* indirect addresses in inode */

#define	MAX_FASTLINK_SIZE	((NDADDR + NIADDR) * sizeof(daddr_t))

struct 	icommon {
	u_short	ic_mode;	/*  0: mode and type of file */
	short	ic_nlink;	/*  2: number of links to file */
	u_short	ic_uid;		/*  4: owner's user id */
	u_short	ic_gid;		/*  6: owner's group id */
	long	ic_size;	/*  8: number of bytes in file */
	ext_time_t ic_atime;	/* 16: time last accessed */
	int	ic_atspare;
	ext_time_t ic_mtime;	/* 24: time last modified */
	int	ic_mtspare;
	ext_time_t ic_ctime;	/* 32: last time inode changed */
	int	ic_ctspare;
	union {
	    struct {
		daddr_t	Mb_db[NDADDR];	/* 40: disk block addresses */
		daddr_t	Mb_ib[NIADDR];	/* 88: indirect blocks */
	    } ic_Mb;
	    char	ic_Msymlink[MAX_FASTLINK_SIZE];
					/* 40: symbolic link name */
	} ic_Mun;
#define	ic_db		ic_Mun.ic_Mb.Mb_db
#define	ic_ib		ic_Mun.ic_Mb.Mb_ib
#define	ic_symlink	ic_Mun.ic_Msymlink
	int	ic_flags;	/* 100: status, currently unused */
#define	IC_FASTLINK	0x0001		/* Symbolic link in inode */
	int	ic_blocks;	/* 104: blocks actually held */
	int	ic_gen;		/* 108: generation number */
	int	ic_spare[4];	/* 112: reserved, currently unused */
};

#define	i_mode		i_ic.ic_mode
#define	i_nlink		i_ic.ic_nlink
#define	i_uid		i_ic.ic_uid
#define	i_gid		i_ic.ic_gid
#if	BYTE_MSF
#define	i_size		i_ic.ic_size
#else	/* BYTE_LSF */
#define	i_size		i_ic.ic_size
#endif
#define	i_db		i_ic.ic_db
#define	i_ib		i_ic.ic_ib
#define	i_atime		i_ic.ic_atime
#define	i_mtime		i_ic.ic_mtime
#define	i_ctime		i_ic.ic_ctime
#define i_blocks	i_ic.ic_blocks
#define	i_rdev		i_ic.ic_db[0]
#define	i_symlink	i_ic.ic_symlink
#define i_flags		i_ic.ic_flags
#define i_gen		i_ic.ic_gen

/* modes */
#define	IFMT		0170000		/* type of file */
#define	IFCHR		0020000		/* character special */
#define	IFDIR		0040000		/* directory */
#define	IFBLK		0060000		/* block special */
#define	IFREG		0100000		/* regular */
#define	IFLNK		0120000		/* symbolic link */
#define	IFSOCK		0140000		/* socket */

#define	ISUID		04000		/* set user id on execution */
#define	ISGID		02000		/* set group id on execution */
#define	ISVTX		01000		/* save swapped text even after use */
#define	IREAD		0400		/* read, write, execute permissions */
#define	IWRITE		0200
#define	IEXEC		0100

/*
 *	Same structure, but on disk.
 */
struct dinode {
	union {
	    struct icommon	di_com;
	    char		di_char[128];
	} di_un;
};
#define	di_ic	di_un.di_com

struct dev_t {
	int		handle;
	unsigned int	first_block;
	unsigned int	last_block;
};

typedef unsigned int recnum_t;

