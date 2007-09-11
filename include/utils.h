#ifndef utils_h
#define utils_h

#include "hwrpb.h"

#ifdef TESTING
#define pal_init()
#else
extern int		printf (const char *fmt, ...);
extern struct pcb_struct *find_pa (unsigned long vptb, struct pcb_struct *pcb);
extern void		pal_init (void);

extern void *		malloc (size_t size);
extern void		free (void *ptr);
extern void		getline (char *buf, int maxlen);
#endif

extern int		check_memory(unsigned long, unsigned long);
extern unsigned long	memory_end(void);
extern unsigned long	free_mem_ptr;

#endif /* utils_h */
