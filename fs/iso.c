/* 
 * This code is based on the ISO filesystem support in MILO (by
 * Dave Rusling).
 *
 * This is a set of functions that provides minimal filesystem
 * functionality to the Linux bootstrapper.  All we can do is
 * open and read files... but that's all we need 8-)
 */
#include <cons.h>
#include <bootfs.h>
#include <isolib.h>

#ifdef DEBUG_ISO
#include <utils.h>
#endif

extern const struct bootfs isofs;

static long cd_device = -1;


long
iso_dev_read (void * buf, long offset, long size)
{
	return cons_read(cd_device, buf, size, offset);
}


static int
iso_mount (long cons_dev, long p_offset, long quiet)
{
#ifdef DEBUG_ISO 
	printf("iso_mount() called\n");
#endif	
	cd_device = cons_dev;
	/*
	 * Read the super block (this determines the file system type
	 * and other important information)
	 */
	return iso_read_super(NULL, quiet);
}

static const char *
iso_readdir(int fd, int rewind)
{
	return iso_readdir_i(fd,rewind); 
}

const struct bootfs iso = {
  -1,	/* isofs is not partitioned */
  1024,	/* block size */
  iso_mount,
  iso_open,
  iso_bread,
  iso_close,
  iso_readdir,
  iso_fstat
};
