#ifndef _ISOFS_FS_H
#define _ISOFS_FS_H

#include <linux/types.h>
/*
 * The isofs filesystem constants/structures
 */

/* This part borrowed from the bsd386 isofs */
#define ISODCL(from, to) (to - from + 1)

struct iso_volume_descriptor {
	char type[ISODCL(1,1)]; /* 711 */
	char id[ISODCL(2,6)];
	char version[ISODCL(7,7)];
	char data[ISODCL(8,2048)];
};

/* volume descriptor types */
#define ISO_VD_PRIMARY 1
#define ISO_VD_END 255

#define ISO_STANDARD_ID "CD001"

struct iso_primary_descriptor {
	char type			[ISODCL (  1,   1)]; /* 711 */
	char id				[ISODCL (  2,   6)];
	char version			[ISODCL (  7,   7)]; /* 711 */
	char unused1			[ISODCL (  8,   8)];
	char system_id			[ISODCL (  9,  40)]; /* achars */
	char volume_id			[ISODCL ( 41,  72)]; /* dchars */
	char unused2			[ISODCL ( 73,  80)];
	char volume_space_size		[ISODCL ( 81,  88)]; /* 733 */
	char unused3			[ISODCL ( 89, 120)];
	char volume_set_size		[ISODCL (121, 124)]; /* 723 */
	char volume_sequence_number	[ISODCL (125, 128)]; /* 723 */
	char logical_block_size		[ISODCL (129, 132)]; /* 723 */
	char path_table_size		[ISODCL (133, 140)]; /* 733 */
	char type_l_path_table		[ISODCL (141, 144)]; /* 731 */
	char opt_type_l_path_table	[ISODCL (145, 148)]; /* 731 */
	char type_m_path_table		[ISODCL (149, 152)]; /* 732 */
	char opt_type_m_path_table	[ISODCL (153, 156)]; /* 732 */
	char root_directory_record	[ISODCL (157, 190)]; /* 9.1 */
	char volume_set_id		[ISODCL (191, 318)]; /* dchars */
	char publisher_id		[ISODCL (319, 446)]; /* achars */
	char preparer_id		[ISODCL (447, 574)]; /* achars */
	char application_id		[ISODCL (575, 702)]; /* achars */
	char copyright_file_id		[ISODCL (703, 739)]; /* 7.5 dchars */
	char abstract_file_id		[ISODCL (740, 776)]; /* 7.5 dchars */
	char bibliographic_file_id	[ISODCL (777, 813)]; /* 7.5 dchars */
	char creation_date		[ISODCL (814, 830)]; /* 8.4.26.1 */
	char modification_date		[ISODCL (831, 847)]; /* 8.4.26.1 */
	char expiration_date		[ISODCL (848, 864)]; /* 8.4.26.1 */
	char effective_date		[ISODCL (865, 881)]; /* 8.4.26.1 */
	char file_structure_version	[ISODCL (882, 882)]; /* 711 */
	char unused4			[ISODCL (883, 883)];
	char application_data		[ISODCL (884, 1395)];
	char unused5			[ISODCL (1396, 2048)];
};


#define HS_STANDARD_ID "CDROM"

struct  hs_volume_descriptor {
	char foo			[ISODCL (  1,   8)]; /* 733 */
	char type			[ISODCL (  9,   9)]; /* 711 */
	char id				[ISODCL ( 10,  14)];
	char version			[ISODCL ( 15,  15)]; /* 711 */
	char data[ISODCL(16,2048)];
};


struct hs_primary_descriptor {
	char foo			[ISODCL (  1,   8)]; /* 733 */
	char type			[ISODCL (  9,   9)]; /* 711 */
	char id				[ISODCL ( 10,  14)];
	char version			[ISODCL ( 15,  15)]; /* 711 */
	char unused1			[ISODCL ( 16,  16)]; /* 711 */
	char system_id			[ISODCL ( 17,  48)]; /* achars */
	char volume_id			[ISODCL ( 49,  80)]; /* dchars */
	char unused2			[ISODCL ( 81,  88)]; /* 733 */
	char volume_space_size		[ISODCL ( 89,  96)]; /* 733 */
	char unused3			[ISODCL ( 97, 128)]; /* 733 */
	char volume_set_size		[ISODCL (129, 132)]; /* 723 */
	char volume_sequence_number	[ISODCL (133, 136)]; /* 723 */
	char logical_block_size		[ISODCL (137, 140)]; /* 723 */
	char path_table_size		[ISODCL (141, 148)]; /* 733 */
	char type_l_path_table		[ISODCL (149, 152)]; /* 731 */
	char unused4			[ISODCL (153, 180)]; /* 733 */
	char root_directory_record	[ISODCL (181, 214)]; /* 9.1 */
};

/* We use this to help us look up the parent inode numbers. */

struct iso_path_table{
	unsigned char  name_len[2];	/* 721 */
	char extent[4];		/* 731 */
	char  parent[2];	/* 721 */
	char name[0];
};

/* high sierra is identical to iso, except that the date is only 6 bytes, and
   there is an extra reserved byte after the flags */

struct iso_directory_record {
	char length			[ISODCL (1, 1)]; /* 711 */
	char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	char extent			[ISODCL (3, 10)]; /* 733 */
	char size			[ISODCL (11, 18)]; /* 733 */
	char date			[ISODCL (19, 25)]; /* 7 by 711 */
	char flags			[ISODCL (26, 26)];
	char file_unit_size		[ISODCL (27, 27)]; /* 711 */
	char interleave			[ISODCL (28, 28)]; /* 711 */
	char volume_sequence_number	[ISODCL (29, 32)]; /* 723 */
	unsigned char name_len		[ISODCL (33, 33)]; /* 711 */
	char name			[0];
};

#define ISOFS_BLOCK_BITS 11
#define ISOFS_BLOCK_SIZE 2048

#define ISOFS_BUFFER_SIZE(INODE) ((INODE)->i_sb->s_blocksize)
#define ISOFS_BUFFER_BITS(INODE) ((INODE)->i_sb->s_blocksize_bits)

#if 0
#ifdef ISOFS_FIXED_BLOCKSIZE
/* We use these until the buffer cache supports 2048 */
#define ISOFS_BUFFER_BITS 10
#define ISOFS_BUFFER_SIZE 1024

#define ISOFS_BLOCK_NUMBER(X) (X<<1)
#else
#define ISOFS_BUFFER_BITS 11
#define ISOFS_BUFFER_SIZE 2048

#define ISOFS_BLOCK_NUMBER(X) (X)
#endif
#endif

#define ISOFS_SUPER_MAGIC 0x9660
#define ISOFS_FILE_UNKNOWN 0
#define ISOFS_FILE_TEXT 1
#define ISOFS_FILE_BINARY 2
#define ISOFS_FILE_TEXT_M 3

struct isofs_super_block {
  unsigned long s_ninodes;
  unsigned long s_nzones;
  unsigned long s_firstdatazone;
  unsigned long s_log_zone_size;
  unsigned long s_max_size;

  unsigned char s_high_sierra; /* A simple flag */
  unsigned char s_mapping;
  unsigned char s_conversion;
  unsigned char s_rock;
  unsigned char s_cruft; /* Broken disks with high
			    byte of length containing
			    junk */
  unsigned int s_blocksize;
  unsigned int s_blocksize_bits;
  unsigned int s_mounted;
  unsigned char s_unhide;
  unsigned char s_nosuid;
  unsigned char s_nodev;
  mode_t s_mode;
};

/*
 * iso fs inode data in memory
 */
struct iso_inode {
	unsigned int i_first_extent;
	unsigned int i_backlink;
	unsigned char i_file_format;
};

/* From fs/isofs/rock.h in Linux, (c) 1995, 1996 Eric Youngdale */

/* These structs are used by the system-use-sharing protocol, in which the
   Rock Ridge extensions are embedded.  It is quite possible that other
   extensions are present on the disk, and this is fine as long as they
   all use SUSP */

struct SU_SP{
  unsigned char magic[2];
  unsigned char skip;
};

struct SU_CE{
  char extent[8];
  char offset[8];
  char size[8];
};

struct SU_ER{
  unsigned char len_id;
  unsigned char len_des;
  unsigned char len_src;
  unsigned char ext_ver;
  char data[0];
};

struct RR_RR{
  char flags[1];
};

struct RR_PX{
  char mode[8];
  char n_links[8];
  char uid[8];
  char gid[8];
};

struct RR_PN{
  char dev_high[8];
  char dev_low[8];
};


struct SL_component{
  unsigned char flags;
  unsigned char len;
  char text[0];
};

struct RR_SL{
  unsigned char flags;
  struct SL_component link;
};

struct RR_NM{
  unsigned char flags;
  char name[0];
};

struct RR_CL{
  char location[8];
};

struct RR_PL{
  char location[8];
};

struct stamp{
  char time[7];
};

struct RR_TF{
  char flags;
  struct stamp times[0];  /* Variable number of these beasts */
};

/* These are the bits and their meanings for flags in the TF structure. */
#define TF_CREATE 1
#define TF_MODIFY 2
#define TF_ACCESS 4
#define TF_ATTRIBUTES 8
#define TF_BACKUP 16
#define TF_EXPIRATION 32
#define TF_EFFECTIVE 64
#define TF_LONG_FORM 128

struct rock_ridge{
  char signature[2];
  unsigned char len;
  unsigned char version;
  union{
    struct SU_SP SP;
    struct SU_CE CE;
    struct SU_ER ER;
    struct RR_RR RR;
    struct RR_PX PX;
    struct RR_PN PN;
    struct RR_SL SL;
    struct RR_NM NM;
    struct RR_CL CL;
    struct RR_PL PL;
    struct RR_TF TF;
  } u;
};

#define RR_PX 1   /* POSIX attributes */
#define RR_PN 2   /* POSIX devices */
#define RR_SL 4   /* Symbolic link */
#define RR_NM 8   /* Alternate Name */
#define RR_CL 16  /* Child link */
#define RR_PL 32  /* Parent link */
#define RR_RE 64  /* Relocation directory */
#define RR_TF 128 /* Timestamps */

#endif
