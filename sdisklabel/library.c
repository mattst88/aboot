#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <disklabel.h>
#include "library.h"

int read_disklabel(int fd,struct disklabel *d) {
   if(lseek(fd,LABELOFFSET,SEEK_SET)<0) {
      return -1;
   }
   if(read(fd,d,sizeof(*d))!=sizeof(*d)) {
      return -1;
   }
   if(d->d_magic!=DISKLABELMAGIC || d->d_magic2!=DISKLABELMAGIC) {
      fprintf(stderr,"Existing disk label is corrupt\n");
      return -1;
   }
   return 0;
}

int dosumlabel(int fd,struct disklabel *d) {
  u_int64_t buf[128];
  int x;
  u_int64_t sum=0;

  if(lseek(fd,0,SEEK_SET)<0) {
     return -1;
  }
  if(read(fd,buf,64*sizeof(u_int64_t))!=(64*sizeof(u_int64_t))) {
     return -1;
  }
  memcpy(&buf[LABELOFFSET/sizeof(u_int64_t)],d,sizeof(*d));
  for(x=0;x<63;x++) {
    sum+=buf[x];
  }
  if(lseek(fd,63*sizeof(u_int64_t),SEEK_SET)<0) {
     return -1;
  }
  if(write(fd,&sum,sizeof(sum))!=sizeof(sum)) {
     return -1;
  }
  return 0;
}

static int does_overlap(int a1,int a2,int b1,int b2) {
   if(a1>b1 && a1<b2) {
      return 1;
   }
   if(a2>b1 && a2<b2) {
      return 1;
   }
   if(b1>a1 && b1<a2) {
      return 1;
   }
   if(b2>a1 && b2<a2) {
      return 1;
   }
   return 0;
}

/*
   returns the number of the partition overlapping with the area
   from offset to endplace, while ignoring partitions in the bitset ignore.
*/
int overlaplabel(struct disklabel *d,int offset,int endplace,unsigned force) {
   int x;

   for(x=0;x<d->d_npartitions;x++) {
      if((force & (1U << x)) == 0) {
	 int part_offset=d->d_partitions[x].p_offset;
	 int part_end=d->d_partitions[x].p_offset+d->d_partitions[x].p_size;
	 if(part_end>0 && does_overlap(part_offset,part_end,offset,endplace)) 
	    return x;
      }
   }
   return -1;
}

