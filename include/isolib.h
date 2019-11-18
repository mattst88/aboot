#ifndef isolib_h
#define isolib_h

int  iso_read_super (void * data, int quiet);
int  iso_open (const char * filename);
int  iso_bread (int fd, long blkno, long nblks, char * buffer);
void iso_close (int fd);
long iso_map (int fd, long block);
int  iso_fstat (int fd, struct stat * buf);
char *iso_readdir_i(int fd, int rewind);
int  isonum_711 (char *p);
int  isonum_712 (char *p);
int  isonum_721 (char *p);
int  isonum_722 (char *p);
int  isonum_723 (char *p);
int  isonum_731 (char *p);
int  isonum_732 (char *p);
int  isonum_733 (char *p);

#endif /* isolib_h */


