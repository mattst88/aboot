/* 
 * aboot/include/string.h 
 * prototypes for the functions in lib/string.c.
 * We used to use the definitions from linux/string.h but more recent
 * kernels don't provide these to userspace code.
 */

#include <linux/types.h>

extern char * ___strtok;
extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern char * strcat(char *, const char *);
extern char * strncat(char *, const char *, size_t);
extern int strcmp(const char *,const char *);
extern int strncmp(const char *,const char *,size_t);
extern char * strchr(const char *,int);
extern char * strrchr(const char *,int);
extern size_t strlen(const char *);
extern size_t strnlen(const char *, size_t);
extern size_t strspn(const char *,const char *);
extern char * strpbrk(const char * cs,const char * ct);
extern char * strtok(char * s,const char * ct);
extern void * memset(void * s, int c, size_t count);
extern char * bcopy(const char * src, char * dest, int count);
extern void * memcpy(void * dest,const void *src,size_t count);
extern void * memmove(void * dest,const void *src,size_t count);
extern int memcmp(const void * cs,const void * ct,size_t count);
extern void * memscan(void * addr, unsigned char c, size_t size);
