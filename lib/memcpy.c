/*
 *  aboot/lib/memcpy.c
 *
 *  Copyright (c) 1995  David Mosberger (davidm@cs.arizona.edu)
 */
#include <linux/types.h>

/*
 * Booting is I/O bound so rather than a time-optimized, we want
 * a space-optimized memcpy.  Not that the rest of the loader
 * were particularly small, though...
 */
void *__memcpy(void *dest, const void *source, size_t n)
{
	char *dst = dest;
	const char *src = source;

	while (n--) {
		*dst++ = *src++;
	}
	return dest;
}
