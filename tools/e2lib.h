#ifndef EXT2_LIB_H
#define EXT2_LIB_H

struct ext2_inode;

extern int 			ext2_init(char * name, int access);
extern void 			ext2_close();
extern struct ext2_inode *	ext2_iget(int ino);
extern void 			ext2_iput(struct ext2_inode *ip);
extern int			ext2_balloc(void);
extern int			ext2_ialloc(void);
extern int			ext2_blocksize(void);
extern int			ext2_blkno(struct ext2_inode *ip, int blkoff,
					   int allocate);
extern void			ext2_bread(struct ext2_inode *ip, int blkno,
					   char * buffer);
extern void			ext2_bwrite(struct ext2_inode *ip, int blkno,
					    char * buffer);
extern struct ext2_inode *	ext2_namei(char * name);
extern void			ext2_truncate(struct ext2_inode *ip);
extern void			ext2_mknod(struct ext2_inode *dip,
					   char * name, int ino);
extern int			ext2_fill_contiguous(struct ext2_inode * ip,
						     int nblocks);
extern void			ext2_write_bootblock(char *bb);

#endif /* EXT2_LIB_H */
