#ifndef __disklabel_h__
#define __disklabel_h__

#ifndef __KERNEL_STRICT_NAMES
  /* ask kernel to be careful about name-space pollution: */
# define __KERNEL_STRICT_NAMES
# define fd_set kernel_fd_set
#endif

#include <linux/types.h>

#define DISKLABELMAGIC (0x82564557UL)

#define LABELSECTOR	0			/* sector containing label */
#define LABELOFFSET	64			/* offset of label in sector */

#define MAXPARTITIONS	8			/* max. # of partitions */

/*
 * Filesystem type and version.  Used to interpret other
 * filesystem-specific per-partition information.
 */
#define	FS_UNUSED	0		/* unused */
#define	FS_SWAP		1		/* swap */
#define	FS_V6		2		/* Sixth Edition */
#define	FS_V7		3		/* Seventh Edition */
#define	FS_SYSV		4		/* System V */
#define	FS_V71K		5		/* V7 with 1K blocks (4.1, 2.9) */
#define	FS_V8		6		/* Eighth Edition, 4K blocks */
#define	FS_BSDFFS	7		/* 4.2BSD fast file system */
#define FS_EXT2		8		/* Linux ext2 file system */
/* OSF will reserve 16--31 for vendor-specific entries */
#define	FS_ADVFS	16		/* Digital Advanced File System */
#define	FS_LSMpubl	17		/* Digital Log Strg public region  */
#define	FS_LSMpriv	18		/* Digital Log Strg private region */
#define	FS_LSMsimp	19		/* Digital Log Strg simple disk    */

struct disklabel {
    __u32	d_magic;				/* must be DISKLABELMAGIC */
    __u16	d_type, d_subtype;
    __u8	d_typename[16];
    __u8	d_packname[16];
    __u32	d_secsize;
    __u32	d_nsectors;
    __u32	d_ntracks;
    __u32	d_ncylinders;
    __u32	d_secpercyl;
    __u32	d_secprtunit;
    __u16	d_sparespertrack;
    __u16	d_sparespercyl;
    __u32	d_acylinders;
    __u16	d_rpm, d_interleave, d_trackskew, d_cylskew;
    __u32	d_headswitch, d_trkseek, d_flags;
    __u32	d_drivedata[5];
    __u32	d_spare[5];
    __u32	d_magic2;				/* must be DISKLABELMAGIC */
    __u16	d_checksum;
    __u16	d_npartitions;
    __u32	d_bbsize, d_sbsize;
    struct d_partition {
	__u32	p_size;
	__u32	p_offset;
	__u32	p_fsize;
	__u8	p_fstype;
	__u8	p_frag;
	__u16	p_cpg;
    } d_partitions[MAXPARTITIONS];
};

#define DTYPE_SMD		 1
#define DTYPE_MSCP		 2
#define DTYPE_DEC		 3
#define DTYPE_SCSI		 4
#define DTYPE_ESDI		 5
#define DTYPE_ST506		 6
#define DTYPE_FLOPPY		10

#ifdef DKTYPENAMES
static char *fstypenames[] = {
	"unused",
	"swap",
	"Version 6",
	"Version 7",
	"System V",
	"4.1BSD",
	"Eighth Edition",
	"4.2BSD",
	"ext2",			/* is this a good choice for ext2?? */
	"resrvd9",
	"resrvd10",
	"resrvd11",
	"resrvd12",
	"resrvd13",
	"resrvd14",
	"resrvd15",
	"ADVfs",
	"LSMpubl",
	"LSMpriv",
	"LSMsimp",
	0
};
#define FSMAXTYPES	(sizeof(fstypenames) / sizeof(fstypenames[0]) - 1)
#endif

#endif /* __disklabel_h__ */
