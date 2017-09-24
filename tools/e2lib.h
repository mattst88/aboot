#ifndef EXT2_LIB_H
#define EXT2_LIB_H

struct ext2_inode;

int 			ext2_init(char * name, int access);
void 			ext2_close();
struct ext2_inode *	ext2_iget(int ino);
void 			ext2_iput(struct ext2_inode *ip);
int			ext2_balloc(void);
int			ext2_ialloc(void);
int			ext2_blocksize(void);
int			ext2_blkno(struct ext2_inode *ip, int blkoff,
				   int allocate);
void			ext2_bread(struct ext2_inode *ip, int blkno,
				   char * buffer);
void			ext2_bwrite(struct ext2_inode *ip, int blkno,
				    char * buffer);
struct ext2_inode *	ext2_namei(char * name);
void			ext2_truncate(struct ext2_inode *ip);
void			ext2_mknod(struct ext2_inode *dip,
				   char * name, int ino);
int			ext2_fill_contiguous(struct ext2_inode * ip,
					     int nblocks);
void			ext2_write_bootblock(char *bb);

#endif /* EXT2_LIB_H */
