#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "system.h"
#include <disklabel.h>
#include <config.h>
#include "library.h"

#define SECT_SIZE 512
#define BOOT_SECTOR 2

int read_configured_partition(int disk_fd, char* buf)
{
  u_int64_t bootsize, bootsect, bootpart = 0;
  long *p = (long *) buf;
  
  if(lseek(disk_fd,60*8,SEEK_SET)<0) {
    perror("lseek on disk");
    exit(1);
  }
  /* Find old configuration */
  read(disk_fd, &bootsize, sizeof(bootsize));
  read(disk_fd, &bootsect, sizeof(bootsect));
  if(lseek(disk_fd,SECT_SIZE*bootsect,SEEK_SET)<0)
    return 0; /* probably random garbage in the boot block - not
		 a fatal error */
  read(disk_fd, buf, SECT_SIZE);
  while ((char *)p < buf + SECT_SIZE)
    if (*p++ == ABOOT_MAGIC)
      bootpart = *p;
  return bootpart;
}

int main(int argc, char **argv)
{
   u_int64_t bootsize,kernelsize=0;
   u_int64_t bootsect=BOOT_SECTOR;
   u_int64_t magicnum=0;
   int disk_fd,file_fd,kernel_fd=0;
   struct disklabel dlabel;
   struct stat s;
   int x;
   char buf[2048];
   int c;
   int err=0, part, bootpart=0;
   unsigned force_overlap=0;
   int verbose=0;
   extern int optind;
   extern char *optarg;
   char *bootfile=0, *device=0, *kernel=0;
   
   while ((c=getopt(argc,argv,"f:c:v?"))!=EOF)
     switch(c)
     {
       case '?':
         err=1;
         break;
       case 'f':
	 part = atoi(optarg);
	 if (part < 1 || part > 8) {
	     fprintf(stderr, "%s: partition number must be in range 1-8\n",
		     argv[0]);
	     exit(1);
	 }
         force_overlap |= 1U << (part - 1);
         break;
       case 'c':
	 bootpart = atoi(optarg);
	 if (bootpart < 1 || bootpart > 8) {
	     fprintf(stderr, "%s: partition number must be in range 1-8\n",
		     argv[0]);
	     exit(1);
	 }
	 break;
       case 'v':
         verbose=1;
         break;
       default:
         err=1;
         break;
     }  

  if(optind<argc)
    device=argv[optind++];
  if(optind<argc)
    bootfile=argv[optind++];
  if(optind<argc)
    kernel=argv[optind++];
    
  if(!bootfile || !device || err)
  {
      fprintf(stderr, "Usage: %s [-f[1-8]] [-c[1-8]] [-v] disk bootfile [kernel]\n",
	      argv[0]);
      exit(1);
   }
  
   disk_fd=open(device,O_RDWR);
   file_fd=open(bootfile,O_RDONLY);
   if(disk_fd<0) {
      perror("open disk device");
      exit(1);
   }
   if(file_fd<0) {
      perror("open bootfile");
      exit(1);
   }
   
   if(kernel)
   {
     kernel_fd=open(kernel,O_RDONLY);
     if (kernel_fd<0)
     {
       perror("open kernel");
       exit(1);
     }
     else
     {
       if(fstat(kernel_fd,&s)) {
         perror("fstat kernel");
         exit(1);
       }
       kernelsize=(s.st_size+SECT_SIZE-1)/SECT_SIZE;
     }
   }
   if(read_disklabel(disk_fd,&dlabel)) {
      fprintf(stderr,"Couldn't get a valid disk label, exiting\n");
      exit(1);
   }
   if(fstat(file_fd,&s)) {
      perror("fstat bootfile");
      exit(1);
   }
   bootsize=(s.st_size+SECT_SIZE-1)/SECT_SIZE;

   if(-1 !=(x=overlaplabel(&dlabel,bootsect,bootsize+bootsect+kernelsize,force_overlap)))
   {
      fprintf(stderr,
	      "error: bootcode overlaps with partition #%d. "
	      "If you really want this, use -f%d\n",
	      x + 1, x + 1);
      exit(1);
   }

   if(!bootpart) {
      bootpart = read_configured_partition(disk_fd, buf);
      if (verbose) {
	 if (bootpart) {
	    printf("preserving boot partition %d\n", bootpart);
	 } else {
	    printf("could not find existing aboot, configuring for second partition\n");
	 }
      }
   } else {
      if (verbose) {
	 printf("setting boot partition to %d\n", bootpart);
      }
   }
   if(lseek(disk_fd,60*8,SEEK_SET)<0) {
      perror("lseek on disk");
      exit(1);
   }
   write(disk_fd,&bootsize,sizeof(bootsize));
   write(disk_fd,&bootsect,sizeof(bootsect));
   write(disk_fd,&magicnum,sizeof(magicnum));
   if (verbose)
   {
     fprintf(stderr,"bootsize:%lu sectors\n",bootsize);
     fprintf(stderr,"bootsect:%lu\n",bootsect);
   }
   if(lseek(disk_fd,SECT_SIZE*bootsect,SEEK_SET)<0) {
      perror("lseek #3 on disk\n");
      exit(1);
   }
   while((x=read(file_fd,buf,2048))>0) {
      write(disk_fd,buf,x);
   }
   close(file_fd);
   if (kernel_fd > 0 && kernelsize>0)
   {
     unsigned long len = 0;

     if (verbose)
       fprintf(stderr,"kernel:%lu sectors\n",kernelsize);
#if 0
     if(lseek(disk,BOOT_SIZE+BOOT_SECTOR*SECT_SIZE,SEEK_SET)<0) {
       perror("lseek #4 on disk\n");
       exit(1);
     }
#endif
     while((x=read(kernel_fd,buf,2048))>0)
     {
       write(disk_fd,buf,x);
       len += x;
     }
     close(kernel_fd);
     if ((len+SECT_SIZE-1)/SECT_SIZE != kernelsize)
       fprintf(stderr,"warning: kernel read %lu, should be %lu\n",(len+SECT_SIZE-1)/SECT_SIZE,kernelsize);
   }
   /* Now write the aboot partition config if we had one */
   if (bootpart) {
     long *p = (long *) buf;

     if(lseek(disk_fd,SECT_SIZE*bootsect,SEEK_SET)<0) {
       perror("lseek #5 on disk\n");
       exit(1);
     }
     read(disk_fd, buf, SECT_SIZE);
     while ((char *)p < buf + SECT_SIZE) {
       if (*p++ == ABOOT_MAGIC) {
	 *p = bootpart;
       }
     }
     lseek(disk_fd,SECT_SIZE*bootsect,SEEK_SET);
     write(disk_fd, buf, SECT_SIZE);
   }
   dosumlabel(disk_fd,&dlabel);
   close(disk_fd);
   if(verbose)
     fprintf(stderr,"done!\n");
   return 0;
}
