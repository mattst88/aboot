/* gzip.h -- common declarations for all gzip modules
 * Copyright (C) 1992-1993 Jean-loup Gailly.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License, see the file COPYING.
 */
#ifndef GZIP_H
#define GZIP_H

#include "string.h"

#define memzero(s, n)     memset ((s), 0, (n))

/* Return codes from gzip */
#define OK      0
#define ERROR   1
#define WARNING 2

/* Compression methods (see algorithm.doc) */
#define STORED     0
#define COMPRESSED 1
#define PACKED     2
/*
 * methods 3 to 7 reserved
 */
#define DEFLATED   8

extern unsigned long bytes_out;		/* # of uncompressed bytes */
extern int method;			/* compression method */

#define INBUFSIZ	0x20000	/* input buffer size */
#define WSIZE		 0x8000	/* window size--must be a power of two, and */
				/*  at least 32K for zip's deflate method */

unsigned char *inbuf;	/* input buffer */
unsigned char *window;	/* sliding window and suffix table (unlzw) */

extern unsigned insize; /* valid bytes in inbuf */
extern unsigned inptr;  /* index of next byte to be processed in inbuf */
extern unsigned outcnt; /* bytes in output buffer */

#define	GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */
#define	OLD_GZIP_MAGIC "\037\236" /* Magic header for gzip 0.5 = freeze 1.x */
#define	PKZIP_MAGIC  "PK\003\004" /* Magic header for pkzip files */
#define	PACK_MAGIC     "\037\036" /* Magic header for packed files */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

/* internal file attribute */
#define UNKNOWN (-1)
#define BINARY  0
#define ASCII   1


#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())
#define put_char(c) {window[outcnt++]=(unsigned char)(c); if (outcnt==WSIZE)\
   flush_window();}

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((unsigned short)(unsigned char)((p)[0]) | ((unsigned short)(unsigned char)((p)[1]) << 8))
#define LG(p) ((unsigned long)(SH(p)) | ((unsigned long)(SH((p)+2)) << 16))

/* in unzip.c */
extern void unzip (int in, int out);

/* in misc.c: */
extern unsigned long updcrc (unsigned char *s, unsigned n);
extern void clear_bufs (void);
extern int  fill_inbuf (void);
extern void flush_window (void);
extern void unzip_error (char *m);

/* in inflate.c */
extern int inflate (void);

#endif /* GZIP_H */
