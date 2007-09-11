#include <alloca.h>

#include <linux/kernel.h>

#include <asm/console.h>
#include "hwrpb.h"
#include "system.h"

#include "aboot.h"
#include "cons.h"
#include "utils.h"
#include "string.h"

#ifndef CCB_OPEN_CONSOLE	/* new callback w/ ARM v4 */
# define CCB_OPEN_CONSOLE 0x07
#endif

#ifndef CCB_CLOSE_CONSOLE	/* new callback w/ ARM v4 */
# define CCB_CLOSE_CONSOLE 0x08
#endif

long cons_dev;			/* console device */

long
cons_puts(const char *str, long len)
{
	long remaining, written;
	union ccb_stsdef {
		long int l_sts;
		struct {
			int written;
			unsigned discard : 29;
			unsigned v_sts0  : 1;
			unsigned v_sts1  : 1;
			unsigned v_err   : 1;
		} s;
	} ccb_sts;
	
	for (remaining = len; remaining; remaining -= written) {
		ccb_sts.l_sts = dispatch(CCB_PUTS, cons_dev, str, remaining);
		if (!ccb_sts.s.v_err) {
			written = ccb_sts.s.written;
			str += written;
		} else {
			if (ccb_sts.s.v_sts1)
				halt();		/* This is a hard error */
			written = 0;
		}
	}
	return len;
}

void
cons_putchar(char c)
{
	char buf[2];

	buf[0] = c;
	buf[1] = 0;
	cons_puts(buf,1);
}

int
cons_getchar(void)
{
	long c;

	while ((c = dispatch(CCB_GETC, cons_dev)) < 0)
		;
	return c;
}


long
cons_getenv(long index, char *envval, long maxlen)
{
	/*
	 * This may seem silly, but some SRM implementations have
	 * problems returning values to buffers that are not 8 byte
	 * aligned.  We work around this by always using a buffer
	 * allocated on the stack (which guaranteed to by 8 byte
	 * aligned).
	 */
	char * tmp = alloca(maxlen);
	long len;

	len = dispatch(CCB_GET_ENV, index, tmp, maxlen - 1);
	if (len >= 0) {
		memcpy(envval, tmp, len);
		envval[len] = '\0';
	}
	return len;
}


long
cons_open(const char *devname)
{
	return dispatch(CCB_OPEN, devname, strlen(devname));
}


long
cons_close(long dev)
{
	return dispatch(CCB_CLOSE, dev);
}


long
cons_read(long dev, void *buf, long count, long offset)
{
	static char readbuf[SECT_SIZE];		/* minimize frame size */

	if ((count & (SECT_SIZE-1)) == 0 && (offset & (SECT_SIZE-1)) == 0) {
		/* I/O is aligned... this is easy! */
		return dispatch(CCB_READ, dev, count, buf,
				offset / SECT_SIZE);
	} else {
		long bytesleft, iocount, blockoffset, iosize, lbn, retval;

		bytesleft = count;
		iocount = 0;
		blockoffset = offset % SECT_SIZE;
		lbn = offset / SECT_SIZE;

		while (bytesleft > 0) {
			if ((blockoffset == 0) && (bytesleft >= SECT_SIZE)) {
				/*
				 * This portion of the I/O is aligned,
				 * so read it straight in:
				 */
				iosize = SECT_SIZE;
				retval = dispatch(CCB_READ, dev, iosize, buf,
						  lbn);
				if (retval != iosize) {
					printf("read error 0x%lx\n",retval);
					return -1;
				}
			} else {
				/*
				 * Not aligned; must read it into a
				 * temporary buffer and go from there.
				 */
				retval = dispatch(CCB_READ, dev, SECT_SIZE,
						  readbuf, lbn);
				if (retval != SECT_SIZE) {
					printf("read error, lbn %ld: 0x%lx\n",
					       lbn, retval);
					return -1;
				}
				iosize = bytesleft;
				if (blockoffset + iosize >= SECT_SIZE) {
					iosize = SECT_SIZE - blockoffset;
				}
				memcpy(buf, readbuf + blockoffset, iosize);
			}
			buf += iosize;
			iocount += iosize;
			bytesleft -= iosize;
			blockoffset = 0;
			++lbn;
		}
		return iocount;
	}
}


void cons_open_console(void)
{
	dispatch(CCB_OPEN_CONSOLE);
}

void cons_close_console(void)
{
	dispatch(CCB_CLOSE_CONSOLE);
}

void
cons_init(void)
{
	char envval[256];

	if (cons_getenv(ENV_TTY_DEV, envval, sizeof(envval)) < 0) {
		halt();		/* better than random crash */
	}
	cons_dev = simple_strtoul(envval, 0, 10);

	cons_open_console();
}
