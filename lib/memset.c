/*
 *  aboot/lib/memset.c
 *
 *  Copyright (c) 1995  David Mosberger (davidm@cs.arizona.edu)
 */
#include <linux/types.h>

/*
 * Booting is I/O bound so rather than a time-optimized, we want
 * a space-optimized memcpy.  Not that the rest of the loader
 * were particularly small, though...
 */
void *__memset(void *s, char c, size_t n)
{
	char *dst = s;

	while (n--) {
		*dst++ = c;
	}
	return s;
}


void *__constant_c_memset(void *dest, char c, size_t n)
{
	return __memset(dest, c, n);
}
