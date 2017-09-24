#ifndef aboot_h
#define aboot_h

#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>

#include "hwrpb.h"

#include <setjmp.h>

#define SECT_SIZE	512	/* console block size for disk reads */
#define BOOT_SECTOR	2	/* first sector of 2ndary bootstrap loader */

extern struct segment {
	unsigned long addr, offset, size;
} *chunks;
extern int nchunks;

extern const struct bootfs *	bfs;
extern char *		dest_addr;
extern long		bytes_to_copy;
extern long		text_offset;
extern jmp_buf		jump_buffer;

extern long		config_file_partition;

extern char		boot_file[256];
extern char		initrd_file[256];
extern char		kernel_args[256];
extern unsigned long	start_addr;
extern char *		bss_start;
extern long		bss_size;
extern unsigned long	initrd_start, initrd_size;
/* page size is in the INIT_HWRPB */
extern unsigned long	page_offset, page_shift;

/* From aboot.c */
bool is_loadable_elf(const unsigned char *buf, long blocksize);

/* From aboot.lds */
extern char _end; /* The program break. The address where the program ends. */

/* From disk.c */
long load_kernel(void);

/* From head.S */
unsigned long switch_to_osf_pal(unsigned long nr,
				struct pcb_struct *pcb_va,
				struct pcb_struct *pcb_pa,
				unsigned long vptb,
				unsigned long *kstk);
void run_kernel(unsigned long entry, unsigned long stack)
     __attribute__((noreturn));

/* From lib/vsprintf.c */
int vsprintf(char *, const char *, va_list);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);

/* From zip/misc.c */
int uncompress_kernel(int fd);

#endif /* aboot_h */
