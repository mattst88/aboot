#ifndef MTOOLS_MSDOS_H
#define MTOOLS_MSDOS_H

/*
 * msdos common header file
 */

#define MAX_SECTOR	8192   		/* largest sector size */
#define MDIR_SIZE	32		/* MSDOS directory entry size in bytes*/
#define MAX_CLUSTER	8192		/* largest cluster size */
#define MAX_PATH	128		/* largest MSDOS path length */
#define MAX_DIR_SECS	64		/* largest directory (in sectors) */

#define NEW		1
#define OLD		0

#define _WORD(x) ((unsigned char)(x)[0] + (((unsigned char)(x)[1]) << 8))
#define _DWORD(x) (_WORD(x) + (_WORD((x)+2) << 16))

#define DELMARK ((char) 0xe5)

struct directory {
	char name[8];			/*  0 file name */
	char ext[3];			/*  8 file extension */
	unsigned char attr;		/* 11 attribute byte */
	unsigned char Case;		/* 12 case of short filename */
	unsigned char ctime_ms;		/* 13 creation time, milliseconds (?) */
	unsigned char ctime[2];		/* 14 creation time */
	unsigned char cdate[2];		/* 16 creation date */
	unsigned char adate[2];		/* 18 last access date */
	unsigned char startHi[2];	/* 20 start cluster, Hi */
	unsigned char time[2];		/* 22 time stamp */
	unsigned char date[2];		/* 24 date stamp */
	unsigned char start[2];		/* 26 starting cluster number */
	unsigned char size[4];		/* 28 size of the file */
};

#define EXTCASE 0x10
#define BASECASE 0x8

#define MAX32 0xffffffff
#define MAX_SIZE 0x7fffffff



typedef struct InfoSector_t {
	unsigned char signature0[4];
	unsigned char filler[0x1e0];
	unsigned char signature[4];
	unsigned char count[4];
	unsigned char pos[4];
} InfoSector_t;

#define INFOSECT_SIGNATURE0 0x41615252
#define INFOSECT_SIGNATURE 0x61417272


/* FAT32 specific info in the bootsector */
typedef struct fat32_t {
	unsigned char bigFat[4];	/* 36 nb of sectors per FAT */
	unsigned char extFlags[2];     	/* 40 extension flags */
	unsigned char fsVersion[2];	/* 42 ? */
	unsigned char rootCluster[4];	/* 44 start cluster of root dir */
	unsigned char infoSector[2];	/* 48 changeable global info */
	unsigned char backupBoot[2];	/* 50 back up boot sector */
	unsigned char reserved[6];	/* 52 ? */
} fat32; /* ends at 58 */

typedef struct oldboot_t {
	unsigned char physdrive;	/* 36 physical drive ? */
	unsigned char reserved;		/* 37 reserved */
	unsigned char dos4;		/* 38 dos > 4.0 diskette */
	unsigned char serial[4];       	/* 39 serial number */
	char label[11];			/* 43 disk label */
	char fat_type[8];		/* 54 FAT type */
			
	unsigned char res_2m;		/* 62 reserved by 2M */
	unsigned char CheckSum;		/* 63 2M checksum (not used) */
	unsigned char fmt_2mf;		/* 64 2MF format version */
	unsigned char wt;		/* 65 1 if write track after format */
	unsigned char rate_0;		/* 66 data transfer rate on track 0 */
	unsigned char rate_any;		/* 67 data transfer rate on track<>0 */
	unsigned char BootP[2];		/* 68 offset to boot program */
	unsigned char Infp0[2];		/* 70 T1: information for track 0 */
	unsigned char InfpX[2];		/* 72 T2: information for track<>0 */
	unsigned char InfTm[2];		/* 74 T3: track sectors size table */
	unsigned char DateF[2];		/* 76 Format date */
	unsigned char TimeF[2];		/* 78 Format time */
	unsigned char junk[1024 - 80];	/* 80 remaining data */
} oldboot_t;

struct bootsector {
	unsigned char jump[3];		/* 0  Jump to boot code */
	char banner[8];		       	/* 3  OEM name & version */
	unsigned char secsiz[2];	/* 11 Bytes per sector hopefully 512 */
	unsigned char clsiz;    	/* 13 Cluster size in sectors */
	unsigned char nrsvsect[2];	/* 14 Number of reserved (boot) sectors */
	unsigned char nfat;		/* 16 Number of FAT tables hopefully 2 */
	unsigned char dirents[2];	/* 17 Number of directory slots */
	unsigned char psect[2]; 	/* 19 Total sectors on disk */
	unsigned char descr;		/* 21 Media descriptor=first byte of FAT */
	unsigned char fatlen[2];	/* 22 Sectors in FAT */
	unsigned char nsect[2];		/* 24 Sectors/track */
	unsigned char nheads[2];	/* 26 Heads */
	unsigned char nhs[4];		/* 28 number of hidden sectors */
	unsigned char bigsect[4];	/* 32 big total sectors */

	union {
		struct fat32_t fat32;
		struct oldboot_t old;
	} ext;
};

extern struct OldDos_t {
	unsigned int tracks;
	unsigned int sectors;
	unsigned int heads;
	
	unsigned int dir_len;
	unsigned int cluster_size;
	unsigned int fat_len;

	unsigned int media;
} old_dos[];

#define FAT12 4085 /* max. number of clusters described by a 12 bit FAT */
#define FAT16 65525

#define MAX_SECT_PER_CLUSTER 64
/* Experimentally, it turns out that DOS only accepts cluster sizes
 * which are powers of two, and less than 128 sectors (else it gets a
 * divide overflow) */


#define FAT_SIZE(bits, sec_siz, clusters) \
	((((clusters)+2) * ((bits)/4) - 1) / 2 / (sec_siz) + 1)

#define NEEDED_FAT_SIZE(x) FAT_SIZE((x)->fat_bits, (x)->sector_size, \
				    (x)->num_clus)

/* disk size taken by FAT and clusters */
#define DISK_SIZE(bits, sec_siz, clusters, n, cluster_size) \
	((n) * FAT_SIZE(bits, sec_siz, clusters) + \
	 (clusters) * (cluster_size))

#define TOTAL_DISK_SIZE(bits, sec_siz, clusters, n, cluster_size) \
	(DISK_SIZE(bits, sec_siz, clusters, n, cluster_size) + 2)
/* approx. total disk size: assume 1 boot sector and one directory sector */

extern const char *mversion;
extern const char *mdate;



int init(char drive, int mode);

#define MT_READ 1
#define MT_WRITE 2

#endif
