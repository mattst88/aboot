extern void	binit (int fd, int blocksize);
extern void	bflush (void);
extern void	bread (int blkno, void * blkbuf);
extern void	bwrite (int blkno, void * blkbuf);
