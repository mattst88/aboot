#ifndef isolib_h
#define isolib_h

#ifndef __KERNEL_STRICT_NAMES
  /* ask kernel to be careful about name-space pollution: */
# define __KERNEL_STRICT_NAMES
# define fd_set kernel_fd_set
#endif

#include <asm/stat.h>

extern int  iso_read_super (void * data, int quiet);
extern int  iso_open (const char * filename);
extern int  iso_bread (int fd, long blkno, long nblks, char * buffer);
extern void iso_close (int fd);
extern long iso_map (int fd, long block);
extern int  iso_fstat (int fd, struct stat * buf);
extern char *iso_readdir_i(int fd, int rewind);
extern int  isonum_711 (char *p);
extern int  isonum_712 (char *p);
extern int  isonum_721 (char *p);
extern int  isonum_722 (char *p);
extern int  isonum_723 (char *p);
extern int  isonum_731 (char *p);
extern int  isonum_732 (char *p);
extern int  isonum_733 (char *p);

#endif /* isolib_h */


