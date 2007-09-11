/*
 * This is a set of functions that provides access to a Linux kernel
 * starting at sector BOOT_SECT+aboot_size/SECT_SIZE
 *
 * Michael Schwingen (rincewind@discworld.oche.de).
 */
#include "system.h"

#include <config.h>
#include <aboot.h>
#include <bootfs.h>
#include <cons.h>
#include <utils.h>

#define BLOCKSIZE (16*SECT_SIZE)

static int dummy_mount(long cons_dev, long p_offset, long quiet);
static int dummy_bread(int fd, long blkno, long nblks, char *buffer);
static int dummy_open(const char *filename);
static void dummy_close(int fd);

struct bootfs dummyfs = {
	0, BLOCKSIZE,
	dummy_mount,
	dummy_open,  dummy_bread,  dummy_close
};

static long dev = -1;


/*
 * Initialize 'filesystem' 
 * Returns 0 if successful, -1 on failure.
 */
static int
dummy_mount(long cons_dev, long p_offset, long quiet)
{
	dev = cons_dev;
	return 0;
}


/*
 * Read block number "blkno".
 */
static int
dummy_bread(int fd, long blkno, long nblks, char *buffer)
{
	extern char _end;
	static long aboot_size = 0;

	if (!aboot_size) {
		aboot_size = &_end - (char *) BOOT_ADDR + SECT_SIZE - 1;
		aboot_size &= ~(SECT_SIZE - 1);
	}

	if (cons_read(dev, buffer, nblks*BLOCKSIZE, 
		      BOOT_SECTOR*SECT_SIZE + blkno*BLOCKSIZE + aboot_size)
	    != nblks*BLOCKSIZE)
	{
		printf("dummy_bread: read error\n");
		return -1;
	}
	return nblks*BLOCKSIZE;
}


/*
 * Unix-like open routine.  Returns a small integer 
 * (does not care what file, we say it's OK)
 */
static int dummy_open(const char *filename)
{
	return 1;
}


static void dummy_close(int fd)
{
}
