#ifndef EXT2_LIB_H
#define EXT2_LIB_H

#include <fcntl.h>


/* Definitions cribbed from ext2_fs.h, modified so's to be 64-bit clean
 * when cross-compiling on Alpha
 */

/*
 * The second extended filesystem constants/structures
 */

/*
 * Define EXT2FS_DEBUG to produce debug messages
 */
#undef EXT2FS_DEBUG

/*
 * Define EXT2FS_DEBUG_CACHE to produce cache debug messages
 */
#undef EXT2FS_DEBUG_CACHE

/*
 * Define EXT2FS_CHECK_CACHE to add some checks to the name cache code
 */
#undef EXT2FS_CHECK_CACHE

/*
 * Define EXT2FS_PRE_02B_COMPAT to convert ext 2 fs prior to 0.2b
 */
#undef EXT2FS_PRE_02B_COMPAT

/*
 * Define DONT_USE_DCACHE to inhibit the directory cache
 */
#define DONT_USE_DCACHE

/*
 * Define EXT2_PREALLOCATE to preallocate data blocks for expanding files
 */
#define EXT2_PREALLOCATE

/*
 * The second extended file system version
 */
#define EXT2FS_DATE		"94/03/10"
#define EXT2FS_VERSION		"0.5"


/*
 * Special inodes numbers
 */
#define	EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_ACL_IDX_INO	 3	/* ACL inode */
#define EXT2_ACL_DATA_INO	 4	/* ACL inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT2_FIRST_INO		11	/* First non reserved inode */

/*
 * The second extended file system magic number
 */
#define EXT2_PRE_02B_MAGIC	0xEF51
#define EXT2_SUPER_MAGIC	0xEF53
#define EXT2_SUPER_BIGMAGIC	0x53EF	/* Accessing on big-endian system... */

/*
 * Maximal count of links to a file
 */
#define EXT2_LINK_MAX		32000

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT2_MIN_BLOCK_SIZE		1024
#define	EXT2_MAX_BLOCK_SIZE		4096
#define EXT2_MIN_BLOCK_LOG_SIZE		  10
# define EXT2_BLOCK_SIZE(s)		(EXT2_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#define EXT2_ACLE_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_acl_entry))
#define	EXT2_ADDR_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (unsigned int))
# define EXT2_BLOCK_SIZE_BITS(s)	((s)->s_log_block_size + 10)
#define	EXT2_INODES_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_inode))

/*
 * Macro-instructions used to manage fragments
 */
#define EXT2_MIN_FRAG_SIZE		1024
#define	EXT2_MAX_FRAG_SIZE		4096
#define EXT2_MIN_FRAG_LOG_SIZE		  10
# define EXT2_FRAG_SIZE(s)		(EXT2_MIN_FRAG_SIZE << (s)->s_log_frag_size)
# define EXT2_FRAGS_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s) / EXT2_FRAG_SIZE(s))

/*
 * ACL structures
 */
struct ext2_acl_header	/* Header of Access Control Lists */
{
	unsigned int aclh_size;
	unsigned int aclh_file_count;
	unsigned int aclh_acle_count;
	unsigned int aclh_first_acle;
};

struct ext2_acl_entry	/* Access Control List Entry */
{
	unsigned int  acle_size;
	unsigned short acle_perms;	/* Access permissions */
	unsigned short acle_type;	/* Type of entry */
	unsigned short acle_tag;	/* User or group identity */
	unsigned short acle_pad1;
	unsigned int  acle_next;	/* Pointer on next entry for the */
					/* same inode or on next free entry */
};

/*
 * Structure of a blocks group descriptor
 */
struct ext2_old_group_desc
{
	unsigned int  bg_block_bitmap;		/* Blocks bitmap block */
	unsigned int  bg_inode_bitmap;		/* Inodes bitmap block */
	unsigned int  bg_inode_table;		/* Inodes table block */
	unsigned short bg_free_blocks_count;	/* Free blocks count */
	unsigned short bg_free_inodes_count;	/* Free inodes count */
};

struct ext2_group_desc
{
	unsigned int  bg_block_bitmap;		/* Blocks bitmap block */
	unsigned int  bg_inode_bitmap;		/* Inodes bitmap block */
	unsigned int  bg_inode_table;		/* Inodes table block */
	unsigned short bg_free_blocks_count;	/* Free blocks count */
	unsigned short bg_free_inodes_count;	/* Free inodes count */
	unsigned short bg_used_dirs_count;	/* Directories count */
	unsigned short bg_pad;
	unsigned int  bg_reserved[3];
};

/*
 * Macro-instructions used to manage group descriptors
 */
# define EXT2_BLOCKS_PER_GROUP(s)	((s)->s_blocks_per_group)
# define EXT2_DESC_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_group_desc))
# define EXT2_INODES_PER_GROUP(s)	((s)->s_inodes_per_group)

/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

/*
 * Inode flags
 */
#define	EXT2_SECRM_FL			0x0001	/* Secure deletion */
#define	EXT2_UNRM_FL			0x0002	/* Undelete */
#define	EXT2_COMPR_FL			0x0004	/* Compress file */
#define EXT2_SYNC_FL			0x0008	/* Synchronous updates */

/*
 * ioctl commands
 */
#define	EXT2_IOC_GETFLAGS		_IOR('f', 1, int)
#define	EXT2_IOC_SETFLAGS		_IOW('f', 2, int)
#define	EXT2_IOC_GETVERSION		_IOR('v', 1, int)
#define	EXT2_IOC_SETVERSION		_IOW('v', 2, int)

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
	unsigned short i_mode;		/* File mode */
	unsigned short i_uid;		/* Owner Uid */
	unsigned int  i_size;		/* Size in bytes */
	unsigned int  i_atime;		/* Access time */
	unsigned int  i_ctime;		/* Creation time */
	unsigned int  i_mtime;		/* Modification time */
	unsigned int  i_dtime;		/* Deletion Time */
	unsigned short i_gid;		/* Group Id */
	unsigned short i_links_count;	/* Links count */
	unsigned int  i_blocks;	/* Blocks count */
	unsigned int  i_flags;		/* File flags */
	unsigned int  i_reserved1;
	unsigned int  i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	unsigned int  i_version;	/* File version (for NFS) */
	unsigned int  i_file_acl;	/* File ACL */
	unsigned int  i_dir_acl;	/* Directory ACL */
	unsigned int  i_faddr;		/* Fragment address */
	unsigned char  i_frag;		/* Fragment number */
	unsigned char  i_fsize;		/* Fragment size */
	unsigned short i_pad1;
	unsigned int  i_reserved2[2];
};

/*
 * File system states
 */
#define	EXT2_VALID_FS			0x0001	/* Unmounted cleany */
#define	EXT2_ERROR_FS			0x0002	/* Errors detected */

/*
 * Mount flags
 */
#define EXT2_MOUNT_CHECK_NORMAL		0x0001	/* Do some more checks */
#define EXT2_MOUNT_CHECK_STRICT		0x0002	/* Do again more checks */
#define EXT2_MOUNT_CHECK		(EXT2_MOUNT_CHECK_NORMAL | \
					 EXT2_MOUNT_CHECK_STRICT)
#define EXT2_MOUNT_GRPID		0x0004	/* Create files with directory's group */
#define EXT2_MOUNT_DEBUG		0x0008	/* Some debugging messages */
#define EXT2_MOUNT_ERRORS_CONT		0x0010	/* Continue on errors */
#define EXT2_MOUNT_ERRORS_RO		0x0020	/* Remount fs ro on errors */
#define EXT2_MOUNT_ERRORS_PANIC		0x0040	/* Panic on errors */

#define clear_opt(o, opt)		o &= ~EXT2_MOUNT_##opt
#define set_opt(o, opt)			o |= EXT2_MOUNT_##opt
#define test_opt(sb, opt)		((sb)->u.ext2_sb.s_mount_opt & \
					 EXT2_MOUNT_##opt)
/*
 * Maximal mount counts between two filesystem checks
 */
#define EXT2_DFL_MAX_MNT_COUNT		20	/* Allow 20 mounts */
#define EXT2_DFL_CHECKINTERVAL		0	/* Don't use interval check */

/*
 * Behaviour when detecting errors
 */
#define EXT2_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT2_ERRORS_RO			2	/* Remount fs read-only */
#define EXT2_ERRORS_PANIC		3	/* Panic */
#define EXT2_ERRORS_DEFAULT		EXT2_ERRORS_CONTINUE

/*
 * Structure of the super block
 */
struct ext2_super_block {
	unsigned int  s_inodes_count;	/* 0: Inodes count */
	unsigned int  s_blocks_count;	/* 4: Blocks count */
	unsigned int  s_r_blocks_count;/* 8: Reserved blocks count */
	unsigned int  s_free_blocks_count;/* 12: Free blocks count */
	unsigned int  s_free_inodes_count;/* 16: Free inodes count */
	unsigned int  s_first_data_block;/* 20: First Data Block */
	unsigned int  s_log_block_size;/* 24: Block size */
	int           s_log_frag_size;	/* 28: Fragment size */
	unsigned int  s_blocks_per_group;/* 32: # Blocks per group */
	unsigned int  s_frags_per_group;/* 36: # Fragments per group */
	unsigned int  s_inodes_per_group;/* 40: # Inodes per group */
	unsigned int  s_mtime;		/* 44: Mount time */
	unsigned int  s_wtime;		/* 48: Write time */
	unsigned short s_mnt_count;	/* 52: Mount count */
	short          s_max_mnt_count;	/* 54: Maximal mount count */
	unsigned short s_magic;		/* 56: Magic signature */
	unsigned short s_state;		/* 58: File system state */
	unsigned short s_errors;	/* 60: Behaviour when detecting errors */
	unsigned short s_pad;		/* 62: */
	unsigned int  s_lastcheck;	/* 64: time of last check */
	unsigned int  s_checkinterval;	/* 68: max. time between checks */
	unsigned int  s_reserved[238];	/* 72: Padding to the end of the block */
};

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

struct ext2_dir_entry {
	unsigned int  inode;			/* Inode number */
	unsigned short rec_len;			/* Directory entry length */
	unsigned short name_len;		/* Name length */
	char           name[EXT2_NAME_LEN];	/* File name */
};

/*
 * EXT2_DIR_PAD defines the directory entries boundaries
 *
 * NOTE: It must be a multiple of 4
 */
#define EXT2_DIR_PAD		 	4
#define EXT2_DIR_ROUND 			(EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT2_DIR_ROUND) & \
					 ~EXT2_DIR_ROUND)


/* These definitions are cribbed from other file system include files, so that
 * we can take a stab at identifying non-ext2 file systems as well...
 */
/*
 * minix super-block data on disk
 */
struct minix_super_block {
        unsigned short s_ninodes;
        unsigned short s_nzones;
        unsigned short s_imap_blocks;
        unsigned short s_zmap_blocks;
        unsigned short s_firstdatazone;
        unsigned short s_log_zone_size;
        unsigned int s_max_size;
        unsigned short s_magic;
        unsigned short s_state;
};

#define MINIX_SUPER_MAGIC       0x137F          /* original minix fs */
#define MINIX_SUPER_MAGIC2      0x138F          /* minix fs, 30 char names */
#define NEW_MINIX_SUPER_MAGIC   0x2468          /* minix V2 - not implemented */

extern int 			ext2_init(char * name, int access);
extern void 			ext2_close();
extern struct ext2_inode *	ext2_iget(int ino);
extern void 			ext2_iput(struct ext2_inode *ip);
extern int			ext2_balloc(void);
extern int			ext2_ialloc(void);
extern int			ext2_blocksize(void);
extern int			ext2_blkno(struct ext2_inode *ip, int blkoff,
					   int allocate);
extern void			ext2_bread(struct ext2_inode *ip, int blkno,
					   char * buffer);
extern void			ext2_bwrite(struct ext2_inode *ip, int blkno,
					    char * buffer);
extern struct ext2_inode *	ext2_namei(char * name);
extern void			ext2_truncate(struct ext2_inode *ip);
extern void			ext2_mknod(struct ext2_inode *dip,
					   char * name, int ino);
extern int			ext2_fill_contiguous(struct ext2_inode * ip,
						     int nblocks);
extern void			ext2_write_bootblock(char *bb);

#endif /* EXT2_LIB_H */
