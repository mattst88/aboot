#ifndef cons_h
#define cons_h

extern long cons_dev;		/* console device */

extern long dispatch(long proc, ...);

#ifdef TESTING
#define STRINGIFY(sym) #sym
#define cons_init()
#define cons_puts(s,l) puts(s, strlen(s))
#define cons_open(d)   open(d, O_RDONLY)
#define cons_close(d)  close(d)
#define cons_read(d,b,c,o) ({ lseek(d, o, 0); read(d,b,c);})
#define cons_putchar(c)    putchar(c)
#define cons_getchar()     getchar()
#define cons_open_console()
#define cons_close_console()
#else
extern void cons_init (void);
extern long cons_getenv (long index, char *envval, long maxlen);
extern long cons_puts (const char *str, long len);
extern long cons_open (const char *devname);
extern long cons_close (long dev);
extern long cons_read (long dev, void *buf, long count, long offset);
extern void cons_putchar(char c);
extern int cons_getchar(void);
extern void cons_open_console(void);
extern void cons_close_console(void);
#endif

/* this isn't in the kernel for some reason */
#define CTB_TYPE_NONE     0
#define CTB_TYPE_DETACHED 1
#define CTB_TYPE_SERIAL   2
#define CTB_TYPE_GRAPHICS 3
#define CTB_TYPE_MULTI    4

struct ctb_struct {
	unsigned long type;
	unsigned long id;
	unsigned long reserved;
	unsigned long dsd_len;
	char dsd[0];
};

#endif /* cons_h */
