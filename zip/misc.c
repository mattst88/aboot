/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 * puts by Nick Holloway 1993
 *
 * Adapted to Linux/Alpha boot by David Mosberger (davidm@cs.arizona.edu).
 */
#include <linux/kernel.h>

#include <asm/page.h>

#include "aboot.h"
#include "bootfs.h"
#include "setjmp.h"
#include "utils.h"
#include "gzip.h"


unsigned char *inbuf;
unsigned char *window;
unsigned outcnt;
unsigned insize;
unsigned inptr;
unsigned long bytes_out;
int method;

static int block_number = 0;
static unsigned long crc_32_tab[256];
static int input_fd = -1;
static int chunk;                 /* current segment */
size_t file_offset;

void
makecrc(void)
{
	/* Not copyrighted 1990 Mark Adler */
	unsigned long c;      /* crc shift register */
	unsigned long e;      /* polynomial exclusive-or pattern */
	int i;                /* counter for all possible eight bit values */
	int k;                /* byte being shifted into crc apparatus */

	/* terms of polynomial defining this crc (except x^32): */
	static int p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

	/* Make exclusive-or pattern from polynomial */
	e = 0;
	for (i = 0; i < (int) (sizeof(p)/sizeof(int)); i++)
	  e |= 1L << (31 - p[i]);

	crc_32_tab[0] = 0;

	for (i = 1; i < 256; i++) {
		c = 0;
		for (k = i | 256; k != 1; k >>= 1) {
			c = c & 1 ? (c >> 1) ^ e : c >> 1;
			if (k & 1)
			  c ^= e;
		}
		crc_32_tab[i] = c;
	}
}


/*
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 *
 * Input:
 *	S	pointer to bytes to pump through.
 *	N	number of bytes in S[].
 */
unsigned long
updcrc(unsigned char *s, unsigned n)
{
	register unsigned long c;
	static unsigned long crc = 0xffffffffUL; /* shift register contents */

	if (!s) {
		c = 0xffffffffL;
	} else {
		c = crc;
		while (n--) {
			c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
		}
	}
	crc = c;
	return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}


/*
 * Clear input and output buffers
 */
void
clear_bufs(void)
{
	outcnt = 0;
	insize = inptr = 0;
	block_number = 0;
	bytes_out = 0;
	chunk = 0;
	file_offset = 0;
}


/*
 * Check the magic number of the input file and update ofname if an
 * original name was given and to_stdout is not set.
 * Return the compression method, -1 for error, -2 for warning.
 * Set inptr to the offset of the next byte to be processed.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */
static int
get_method(void)
{
	unsigned char flags;
	char magic[2]; /* magic header */

	magic[0] = get_byte();
	magic[1] = get_byte();

	method = -1;                 /* unknown yet */
	if (memcmp(magic, GZIP_MAGIC, 2) == 0
	    || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) {

		method = get_byte();
		flags  = get_byte();
		if ((flags & ENCRYPTED) != 0)
		  unzip_error("input is encrypted");
		if ((flags & CONTINUATION) != 0)
		  unzip_error("multi part input");
		if ((flags & RESERVED) != 0)
		  unzip_error("input has invalid flags");
		get_byte();	/* skip over timestamp */
		get_byte();
		get_byte();
		get_byte();

		get_byte();	/* skip extra flags */
		get_byte();	/* skip OS type */

		if ((flags & EXTRA_FIELD) != 0) {
			unsigned len = get_byte();
			len |= get_byte() << 8;
			while (len--) get_byte();
		}

		/* Get original file name if it was truncated */
		if ((flags & ORIG_NAME) != 0) {
			/* skip name */
			while (get_byte() != 0) /* null */ ;
		}

		/* Discard file comment if any */
		if ((flags & COMMENT) != 0) {
			while (get_byte() != 0) /* null */ ;
		}
	} else {
		unzip_error("unknown compression method");
	}
	return method;
}

/*
 * Fill the input buffer and return the first byte in it. This is called
 * only when the buffer is empty and at least one byte is really needed.
 */
int
fill_inbuf(void)
{
	long nblocks, nread;

	if (INBUFSIZ % bfs->blocksize != 0) {
		printf("INBUFSIZ (%d) is not multiple of block-size (%d)\n",
		       INBUFSIZ, bfs->blocksize);
		unzip_error("bad block-size");
	}

	if (block_number < 0) {
		unzip_error("attempted to read past eof");
	}

	nblocks = INBUFSIZ / bfs->blocksize;
	nread = (*bfs->bread)(input_fd, block_number, nblocks, inbuf);
#ifdef DEBUG
	printf("read %ld blocks of %d, got %ld\n", nblocks, bfs->blocksize,
	       nread);
#endif
	if (nread != nblocks * bfs->blocksize) {
		if (nread < nblocks * bfs->blocksize) {
			/* this is the EOF */
#ifdef DEBUG
			printf("at EOF\n");
#endif
			insize = nblocks * bfs->blocksize;
			block_number = -1;
		} else {
			printf("Read returned %ld instead of %ld bytes\n",
			       nread, nblocks * bfs->blocksize);
			unzip_error("read error");
		}
	} else {
		block_number += nblocks;
		insize = INBUFSIZ;
	}
	inptr = 1;
	return inbuf[0];
}


/*
 * Write the output window window[0..outcnt-1] holding uncompressed
 * data and update crc.
 */
void
flush_window(void)
{
	if (!outcnt) {
		return;
	}

	updcrc(window, outcnt);

	if (!bytes_out) /* first block - look for headers */
		if (first_block(window, outcnt) < 0)
			unzip_error("invalid exec header"); /* does a longjmp() */

	bytes_out += outcnt;
	while (chunk < nchunks) {
		/* position within the current segment */
		ssize_t chunk_offset = file_offset - chunks[chunk].offset;
		unsigned char *dest = (char *) chunks[chunk].addr + chunk_offset;
		ssize_t to_copy;
		unsigned char *src = window;

		/* window overlaps beginning of current segment */
		if (chunk_offset < 0) {
			src = window - chunk_offset;
			dest = (unsigned char *) chunks[chunk].addr;
		}
		if (src - window >= outcnt) {
			file_offset += outcnt;
			break; /* next window */
		}

		/* print a vanity message */
		if (chunk_offset == 0)
			printf("aboot: segment %d, %ld bytes at %#lx\n",
			       chunk, chunks[chunk].size,
			       chunks[chunk].addr);

		to_copy = chunks[chunk].offset + chunks[chunk].size
			- file_offset;
		if (to_copy > outcnt)
			to_copy = outcnt;
#ifdef DEBUG
		printf("copying %ld bytes from offset %#lx "
		       "(segment %d) to %p\n",
		       to_copy, file_offset, chunk, dest);
#endif
#ifndef TESTING
		memcpy(dest, src, to_copy);
#endif
		file_offset += to_copy;

		if (to_copy < outcnt) {
#ifdef DEBUG
			printf("new segment or EOF\n");
#endif
			outcnt -= to_copy;
			chunk++;
		} else
			break; /* done this window */
 	}
}


/*
 * We have to be careful with the memory-layout during uncompression.
 * The stack we're currently executing on lies somewhere between the
 * end of this program (given by _end) and lastfree.  However, as I
 * understand it, there is no guarantee that the stack occupies the
 * lowest page-frame following the page-frames occupied by this code.
 *
 * Thus, we are stuck allocating memory towards decreasing addresses,
 * starting with lastfree.  Unfortunately, to know the size of the
 * kernel-code, we need to uncompress the image and we have a circular
 * dependency.  To make the long story short: we put a limit on
 * the maximum kernel size at MAX_KERNEL_SIZE and allocate dynamic
 * memory starting at (lastfree << ALPHA_PG_SHIFT) - MAX_KERNEL_SIZE.
 */
int
uncompress_kernel(int fd)
{
	input_fd = fd;

	inbuf = (unsigned char*) malloc(INBUFSIZ);
	window = (unsigned char*) malloc(WSIZE);

	clear_bufs();
	makecrc();

	method = get_method();
	unzip(0, 0);

	return 1;
}
