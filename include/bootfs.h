#ifndef boot_fs_h
#define boot_fs_h

#include <linux/types.h>
#include <asm/stat.h>

struct bootfs {
	int	fs_type;
	int	blocksize;
	
	int	(*mount)(long dev, long partition_start, long quiet);

	int	(*open)(const char *filename);
	int	(*bread)(int fd, long blkno, long nblks, char *buf);
	void	(*close)(int fd);

	/* You'll probably want to use this like:
		while ((ent = fs->readdir(fd, !rewind++)));
	   so that it rewinds only on the first access.  Also don't
	   mix it with other I/O or you will die horribly */
	const char *	(*readdir)(int fd, int rewind);
	int	(*fstat)(int fd, struct stat* buf);
} ext2fs;

#endif /* boot_fs_h */
