#include <linux/kernel.h>

#include <asm/hwrpb.h>
#include <linux/version.h>
#include <asm/system.h>

#include <stdarg.h>

#include "aboot.h"
#include "cons.h"
#include "string.h"


unsigned long free_mem_ptr = 0;


int printf(const char *fmt, ...)
{
	static char buf[1024];
	va_list args;
	long len, num_lf;
	char *src, *dst;

	va_start(args, fmt);
	len = vsprintf(buf, fmt, args);
	va_end(args);

	/* count number of linefeeds in string: */

	num_lf = 0;
	for (src = buf; *src; ++src) {
		if (*src == '\n') {
			++num_lf;
		}
	}

	if (num_lf) {
		/* expand each linefeed into carriage-return/linefeed: */
		for (dst = src + num_lf; src >= buf; ) {
			if (*src == '\n') {
				*dst-- = '\r';
			}
			*dst-- = *src--;
		}
	}
	return cons_puts(buf, len + num_lf);
}


/*
 * Find a physical address of a virtual object..
 *
 * This is easy using the virtual page table address.
 */
struct pcb_struct *find_pa(unsigned long *vptb, struct pcb_struct *pcb)
{
	unsigned long address = (unsigned long) pcb;
	unsigned long result;

	result = vptb[address >> 13];
	result >>= 32;
	result <<= 13;
	result |= address & 0x1fff;
	return (struct pcb_struct *) result;
}	

/*
 * This function moves into OSF/1 pal-code, and has a temporary
 * PCB for that. The kernel proper should replace this PCB with
 * the real one as soon as possible.
 *
 * The page table muckery in here depends on the fact that the boot
 * code has the L1 page table identity-map itself in the second PTE
 * in the L1 page table. Thus the L1-page is virtually addressable
 * itself (through three levels) at virtual address 0x200802000.
 *
 * As we don't want it there anyway, we also move the L1 self-map
 * up as high as we can, so that the last entry in the L1 page table
 * maps the page tables.
 *
 * As a result, the OSF/1 pal-code will instead use a virtual page table
 * map located at 0xffffffe00000000.
 */
#define pcb_va ((struct pcb_struct *) 0x20000000)
#define old_vptb (0x0000000200000000UL)
#define new_vptb (0xfffffffe00000000UL)
void pal_init(void)
{
	unsigned long i, rev, sum;
	unsigned long *L1, *l;
	struct percpu_struct * percpu;
	struct pcb_struct * pcb_pa;

	/* Find the level 1 page table and duplicate it in high memory */
	L1 = (unsigned long *) 0x200802000UL; /* (1<<33 | 1<<23 | 1<<13) */
	L1[1023] = L1[1];

	percpu = (struct percpu_struct *) (INIT_HWRPB->processor_offset
					   + (unsigned long) INIT_HWRPB),
	pcb_va->ksp = 0;
	pcb_va->usp = 0;
	pcb_va->ptbr = L1[1] >> 32;
	pcb_va->asn = 0;
	pcb_va->pcc = 0;
	pcb_va->unique = 0;
	pcb_va->flags = 1;
	pcb_pa = find_pa((unsigned long *) old_vptb, pcb_va);
	printf("aboot: switching to OSF/1 PALcode");
	/*
	 * a0 = 2 (OSF)
	 * a1 = return address, but we give the asm the virtual addr of the PCB
	 * a2 = physical addr of PCB
	 * a3 = new virtual page table pointer
	 * a4 = KSP (but we give it 0, asm sets it)
	 */
	i = switch_to_osf_pal(
		2,
		pcb_va,
		pcb_pa,
		new_vptb,
		0);
	if (i) {
		printf("---failed, code %ld\n", i);
		halt();
	}
	rev = percpu->pal_revision = percpu->palcode_avail[2];

	INIT_HWRPB->vptb = new_vptb;

	/* update checksum: */
	sum = 0;
	for (l = (unsigned long *) INIT_HWRPB; l < (unsigned long *) &INIT_HWRPB->chksum; ++l)
		sum += *l;
	INIT_HWRPB->chksum = sum;

	printf(" version %ld.%ld\n", (rev >> 8) & 0xff, rev & 0xff);
	/* remove the old virtual page-table mapping */
	L1[1] = 0;
	tbia();
}


unsigned long memory_end(void)
{
	int i;
	unsigned long high = 0;
	struct memclust_struct *cluster;
	struct memdesc_struct *memdesc;

	memdesc = (struct memdesc_struct *)
	  (INIT_HWRPB->mddt_offset + (unsigned long) INIT_HWRPB);
	cluster = memdesc->cluster;
	for (i = memdesc->numclusters; i > 0; i--, cluster++) {
		unsigned long tmp;

		if (cluster->usage != 0) {
			/* this is a PAL or NVRAM cluster (not for the OS) */
			continue;
		}

		tmp = (cluster->start_pfn + cluster->numpages) << page_shift;
		if (tmp > high) {
			high = tmp;
		}
	}
	return page_offset + high;
}


static void error(char *x)
{
	printf("%s\n", x);
	_longjmp(jump_buffer, 1);
}


void unzip_error(char *x)
{
	printf("\nunzip: ");
	error(x);
}


void *malloc(size_t size)
{
	if (!free_mem_ptr) {
		free_mem_ptr = memory_end();
	}

	free_mem_ptr = (free_mem_ptr - size) & ~(sizeof(long) - 1);
	if ((char*) free_mem_ptr <= dest_addr + INIT_HWRPB->pagesize) {
		error("\nout of memory");
	}
	return (void*) free_mem_ptr;
}


void free(void *where)
{
	/* don't care */
}


void
getline (char *buf, int maxlen)
{
	int len=0;
	char c;

	do {
		c = cons_getchar();
		switch (c) {
		      case 0:
		      case 10:
		      case 13:
			break;
		      case 8:
		      case 127:
			if (len > 0) {
				--len;
				cons_putchar(8);
				cons_putchar(' ');
				cons_putchar(8);
			}
			break;

		      default:
			if (len < maxlen-1 && c >= ' ') {
				buf[len] = c;
				len++;
				cons_putchar(c);
			}
			break;
		}
	} while (c != 13 && c != 10);
	buf[len] = 0;
}
