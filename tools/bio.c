#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Buffered I/O functions.  By cacheing the most recently used blocks,
 * we can cut WAAY down on disk traffic...
 */

static int	bio_fd = -1;
static int	bio_counter = 0;
static int	bio_blocksize = 0;

struct bio_buf {
    int		blkno;
    int		last_access;
    int		dirty;
    char *	data;
};

#define NBUFS	32
struct bio_buf	buflist[NBUFS];

/* initialize the buffer cache.  Blow away anything that may
 * have been previously cached...
 */
void
binit(int fd, int blocksize)
{
    int		i;

    bio_fd = fd;
    bio_blocksize = blocksize;

    for(i = 0; i < NBUFS; i++) {
	buflist[i].blkno = 0;
	if(buflist[i].data) {
	    free(buflist[i].data);
	}
	buflist[i].data = 0;
	buflist[i].last_access = 0;
	buflist[i].dirty = 0;
    }
}

/* Flush out any dirty blocks */
void
bflush(void)
{
    int		i;

    for(i = 0; i < NBUFS; i++) {
	if(buflist[i].dirty) {
#ifdef BIO_DEBUG
	    printf("bflush: writing block %d\n", buflist[i].blkno);
#endif
	    lseek(bio_fd, buflist[i].blkno * bio_blocksize, 0);
	    write(bio_fd, buflist[i].data, bio_blocksize);
	    buflist[i].dirty = 0;
	}
    }
}

/* Read a block.  */
void
bread(int blkno, void * blkbuf)
{
    int		i;
    int		lowcount;
    int		lowcount_buf;

    /* First, see if the block is already in memory... */
    for(i = 0; i < NBUFS; i++) {
	if(buflist[i].blkno == blkno) {
	    /* Got it!  Bump the access count and return. */
	    buflist[i].last_access = ++bio_counter;
#ifdef BIO_DEBUG
	    printf("bread: buffer hit on block %d\n", blkno);
#endif
	    memcpy(blkbuf, buflist[i].data, bio_blocksize);
	    return;
	}
    }

    /* Not in memory; need to find a buffer and read it in. */
    lowcount = buflist[0].last_access;
    lowcount_buf = 0;
    for(i = 1; i < NBUFS; i++) {
	if(buflist[i].last_access < lowcount) {
	    lowcount = buflist[i].last_access;
	    lowcount_buf = i;
	}
    }

    /* If the buffer is dirty, we need to write it out... */
    if(buflist[lowcount_buf].dirty) {
#ifdef BIO_DEBUG
	printf("bread: recycling dirty buffer %d for block %d\n", 
			lowcount_buf, buflist[lowcount_buf].blkno);
#endif
	lseek(bio_fd, buflist[lowcount_buf].blkno * bio_blocksize, 0);
	write(bio_fd, buflist[lowcount_buf].data, bio_blocksize);
	buflist[lowcount_buf].dirty = 0;
    }

#ifdef BIO_DEBUG
    printf("bread: Using buffer %d for block %d\n", lowcount_buf, blkno);
#endif

    buflist[lowcount_buf].blkno = blkno;
    if(!buflist[lowcount_buf].data) {
	buflist[lowcount_buf].data = (char *)malloc(bio_blocksize);
    }
    lseek(bio_fd, blkno * bio_blocksize, 0);
    if(read(bio_fd,buflist[lowcount_buf].data,bio_blocksize)!=bio_blocksize) {
	perror("bread: I/O error");
    }

    buflist[lowcount_buf].last_access = ++bio_counter;
    memcpy(blkbuf, buflist[lowcount_buf].data, bio_blocksize);
}


/* Write a block */
void
bwrite(int blkno, void * blkbuf)
{
    int		i;
    int		lowcount;
    int		lowcount_buf;

    /* First, see if the block is already in memory... */
    for(i = 0; i < NBUFS; i++) {
	if(buflist[i].blkno == blkno) {
	    /* Got it!  Bump the access count and return. */
#ifdef BIO_DEBUG
	    printf("bwrite: buffer hit on block %d\n", blkno);
#endif
	    buflist[i].last_access = ++bio_counter;
	    memcpy(buflist[i].data, blkbuf, bio_blocksize);
	    buflist[i].dirty = 1;
	    return;
	}
    }

    /* Not in memory; need to find a buffer and stuff it. */
    lowcount = buflist[0].last_access;
    lowcount_buf = 0;
    for(i = 1; i < NBUFS; i++) {
	if(buflist[i].last_access < lowcount) {
	    lowcount = buflist[i].last_access;
	    lowcount_buf = i;
	}
    }

    /* If the buffer is dirty, we need to write it out... */
    if(buflist[lowcount_buf].dirty) {
#ifdef BIO_DEBUG
	printf("bwrite: recycling dirty buffer %d for block %d\n", 
			lowcount_buf, buflist[lowcount_buf].blkno);
#endif
	lseek(bio_fd, buflist[lowcount_buf].blkno * bio_blocksize, 0);
	write(bio_fd, buflist[lowcount_buf].data, bio_blocksize);
	buflist[lowcount_buf].dirty = 0;
    }

#ifdef BIO_DEBUG
    printf("bwrite: Using buffer %d for block %d\n", lowcount_buf, blkno);
#endif

    buflist[lowcount_buf].blkno = blkno;
    if(!buflist[lowcount_buf].data) {
	buflist[lowcount_buf].data = (char *)malloc(bio_blocksize);
    }
    buflist[lowcount_buf].last_access = ++bio_counter;
    memcpy(buflist[lowcount_buf].data, blkbuf, bio_blocksize);
    buflist[lowcount_buf].dirty = 1;
}
