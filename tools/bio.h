void	binit(int fd, int blocksize);
void	bflush(void);
void	bread(int blkno, void * blkbuf);
void	bwrite(int blkno, void * blkbuf);
