#ifndef net_h
#define net_h

#include <asm/system.h> /* for COMMAND_LINE_SIZE */

/* 
 * Memory for a BOOTP image goes thus:
 * 
 *        netabootheader  
 *        |
 * +-----+-+-----------------------+------------------------+
 * |     | |                       |                        |
 * |aboot| |     kernel image      |      initrd image      |
 * |     | |                       |                        |
 * +-----+-+-----------------------+------------------------+
 * |     |
 * |   _end    
 * 0x20000000
 * 
 * the dividing lines between parts are aligned on 512-byte boundaries, for 
 * net_bread()'s sake. 
 */

typedef struct netabootheader {
  int header_size;
  long kernel_size;
  long initrd_size;
  char command_line[COMMAND_LINE_SIZE];
} netabootheader_t;

#define ALIGN_512(p) (((unsigned long) p + 511) & ~511)

#endif

