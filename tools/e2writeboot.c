/* Program to write a primary bootstrap file to a LINUX ext2
 * file system.
 *
 * Usage: e2writeboot fs-image bootfile
 *
 * It is assumed that the "bootfile" is a COFF executable with text,
 * data, and bss contiguous.
 */

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>

#include <e2lib.h>

#if defined(__linux__)
# include <sys/types.h>
#elif defined(__alpha__) && defined(__osf1__)
  typedef unsigned long		u_int64_t;
#elif defined(__GNUC__)
  typedef unsigned long long	u_int64_t;
#endif

struct  boot_block {
    u_int64_t	vax_code[17];
    u_int64_t	reserved[43];
    u_int64_t	count;
    u_int64_t	lbn;
    u_int64_t	flags;
    u_int64_t	chk_sum;
};

#define CONSOLE_BLOCK_SIZE	512

extern int big_endian;
extern unsigned short swap16();
extern unsigned int swap32();


int
main(int argc, char ** argv)
{
    char		fsname[512];
    char		iobuf[1024];
    char		namebuf[EXT2_NAME_LEN+1];
    struct ext2_inode *	ip;
    int			filesize;
    int			infile;
    int 		blkno;
    int			bootstrap_size;
    int			i;
    int			bs_start;
    char		*bsbuf;
    int			blocksize;
    struct boot_block	*bbp;
    u_int64_t		*lbp, checksum;
    struct stat		st;

    if (argc != 3) {
	printf("Usage: %s ext2-fs input-file\n", argv[0]);
	exit(1);
    }

    strcpy(fsname, argv[1]);
    strcpy(namebuf, "/linuxboot");


    /* "Open" the file system */
    if (ext2_init(fsname, O_RDWR) < 0) {
	exit(1);
    }

    /* Open the input file */
    infile = open(argv[2], 0);
    if (infile < 0) {
	perror(argv[2]);
	ext2_close();
	exit(1);
    }

    /* Figure out just how much data the input file is going
     * to require us to put out. (text+data+bss).
     */
    if (fstat(infile, &st) == -1) {
	perror("fstat");
	ext2_close();
	exit(1);
    }
    blocksize = ext2_blocksize();
    bootstrap_size = st.st_size;
    printf("bootstrap_size: %d -> ", bootstrap_size);
    bootstrap_size += blocksize-1;
    bootstrap_size &= ~(blocksize-1);
    printf("%d\n", bootstrap_size);

    /* Allocate a buffer to hold the entire bootstrap, then read
     * in the text+data segments.
     */
    bsbuf = (char *)malloc(bootstrap_size);
    memset(bsbuf, 0, bootstrap_size);
    read(infile, bsbuf, bootstrap_size);
    close(infile);
    
    /* Get the inode for the file we want to create */
    ip = ext2_namei(namebuf);
    if(ip) {
	/* This file exists.  Make sure it's a regular file, then
	 * truncate it.
	 */
	if(!S_ISREG(ip->i_mode)) {
	    printf("%s: Not a regular file.  Must remove it first.\n", namebuf);
	    ext2_iput(ip);
	    ext2_close();
	    exit(1);
	}

	printf("Using existing file %s\n", namebuf);
	ext2_truncate(ip);
    }
    else {
	/* Doesn't exist.  Must get the parent directory's inode. */
	char	dirname[EXT2_NAME_LEN+1];
	char	filename[EXT2_NAME_LEN+1];
	struct ext2_inode *dip;
	int	inumber;
	char	*cp;
	int	i;

	strcpy(dirname, namebuf);
	cp = strrchr(dirname, '/');
	if(cp) {
	    *cp = '\0';
	    strcpy(filename, cp+1);
	}
	else {
	    strcpy(filename, dirname);
	    strcpy(dirname, "/");
	}

	dip = ext2_namei(dirname);
	if(!dip) {
	    printf("Directory %s does not exist\n", dirname);
	    ext2_close();
	    exit(1);
	}

	printf("Creating new file %s in directory %s\n", filename, dirname);

	/* Get an inode for the file */
	inumber = ext2_ialloc();
	ip = ext2_iget(inumber);

	if(!ip) {
	    printf("PANIC! ip == NULL\n");
	    exit(1);
	}

	/* Create the directory entry */
	ext2_mknod(dip, filename, inumber);

	/* We're done with the directory for now... */
	ext2_iput(dip);

	/* Set certain fields in the inode (we're not going to get
	 * fancy here... just choose a safe set of defaults...)
	 */
	ip->i_mode = 0550 | S_IFREG;	/* Regular file, r-xr-x--- */
	ip->i_uid = 0;		/* Owned by root */
	ip->i_gid = 0;		/* Group is system */
	ip->i_size = 0;
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0);
	ip->i_dtime = 0;
	ip->i_links_count = 1;
	ip->i_blocks = 0;
	ip->i_flags = 0;	/* Nothing special */
	for(i = 0; i < EXT2_N_BLOCKS; i++) {
	    ip->i_block[i] = 0;
	}
	ip->i_version = 0;
	ip->i_file_acl = 0;
	ip->i_frag = 0;
	ip->i_fsize = 0;
	ip->i_reserved1 = ip->i_pad1 = ip->i_reserved2[0] = 0;

    }

    /* At this point we have an inode for an empty regular file.
     * Fill it up!
     */
    
    bs_start = ext2_fill_contiguous(ip, bootstrap_size/blocksize);
    if(bs_start <= 0) {
	printf("Cannot allocate blocks for %s... goodbye!\n", argv[2]);
	ext2_close();
	exit(1);
    }

    /* Write what we've got out to the file */
    filesize = bootstrap_size;
    blkno = 0;
    while(filesize > 0) {
	ext2_bwrite(ip, blkno, bsbuf+(blkno*blocksize));
	blkno++;
	filesize -= blocksize;
    }

    ip->i_size = bootstrap_size;
    ip->i_mtime = time(0);


    /* Prepare and write out a bootblock */
    memset(iobuf, 0, blocksize);
    bbp = (struct boot_block *)iobuf;
    
    bbp->count = bootstrap_size / CONSOLE_BLOCK_SIZE;
    bbp->lbn   = (bs_start * blocksize) / CONSOLE_BLOCK_SIZE;
    bbp->flags = 0;

    /* Compute the checksum */
    checksum = 0;
    lbp = (u_int64_t*) bbp;
    for (i = 0; i < CONSOLE_BLOCK_SIZE/8; ++i) {
	checksum += lbp[i];
    }
    bbp->chk_sum = checksum;

    if(big_endian) {
	/* Need to flip the bootblock fields so they come out
	 * right on disk...
	 */
	bbp->count   = (((u_int64_t) swap32(bbp->count & 0xffffffff) << 32)
			| swap32(bbp->count >> 32));
	bbp->lbn     = (((u_int64_t) swap32(bbp->lbn & 0xffffffff) << 32)
			| swap32(bbp->lbn >> 32));
	bbp->flags   = (((u_int64_t) swap32(bbp->flags & 0xffffffff) << 32)
			| swap32(bbp->flags >> 32));
	bbp->chk_sum = (((u_int64_t) swap32(bbp->chk_sum & 0xffffffff) << 32)
			| swap32(bbp->chk_sum >> 32));
    }

    ext2_write_bootblock((char *) bbp);

    ext2_iput(ip);
    ext2_close();

    printf("%d bytes written to %s\n", bootstrap_size, namebuf);
    return 0;
}
