#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <disklabel.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include "library.h"

/* from linux/fs.h */
#define BLKSSZGET  _IO(0x12,104)   /* get block device sector size */
#define BLKGETSIZE _IO(0x12,96)    /* return device size */

int label_modified=0;
int force=0;

int
write_disklabel (int fd,struct disklabel *d)
{
	dosumlabel(fd,d);
	if(lseek(fd,LABELOFFSET,SEEK_SET)<0) {
		return -1;
	}
	if(write(fd,d,sizeof(*d))!=sizeof(*d)) {
		return -1;
	}
	return 0;
}

void
fixmagic (int fd,struct disklabel *d)
{
	d->d_magic=DISKLABELMAGIC;
	d->d_magic2=DISKLABELMAGIC;
	d->d_type=DTYPE_SCSI;
	d->d_secsize=512;
	strcpy(d->d_typename,"SCSI");
}

void
zero_disklabel (int fd,struct disklabel *d)
{
	memset(d,0,sizeof(*d));
	fixmagic(fd,d);
	label_modified=1;
}

void
print_disklabel(int fd,struct disklabel *d)
{
	int x;
	for(x=0;x<d->d_npartitions;x++) {
		printf("partition %d: type %d, starts sector %d, size %d\n",
		       x, d->d_partitions[x].p_fstype,
		       d->d_partitions[x].p_offset, d->d_partitions[x].p_size);
	}
}

int total_disk_size, heads, sectors, cylinders;
/* FIXME: use BLKSSZGET to get sector size. (depends on linux >=2.4) */
int sector_size=512;

#ifdef __linux__

void
set_disk_size (int fd)
{
	struct hd_geometry hd;
	unsigned long blocks;
	unsigned int sectors_per_block;
	int r1, r2;
	
	sectors_per_block = sector_size / 512;
	heads = sectors = cylinders = blocks = 0;

	r1 = ioctl(fd,HDIO_GETGEO,&hd);

	if (r1) {
		perror("ioctl HDIO_GETGEO");
	} else {
		heads = hd.heads;
		sectors = hd.sectors;
		cylinders = hd.cylinders;
		if (heads * sectors * cylinders == 0) { r1 = -1; }
		/* fdisk says: "never use hd.cylinders - it is truncated" 
		   if BLKGETSIZE works we'll calculate our own value for
		   cylinders in a little bit, but for now, use it anyway */
		total_disk_size=(heads*sectors*cylinders); /* in sectors */ 
	}
	
	r2 = ioctl(fd,BLKGETSIZE, &blocks);

	if (r2) {
		perror("ioctl BLKGETSIZE");
	}

	if (r1 && r2) {
		if (!total_disk_size) {
			fprintf(stderr, "Unable to get disk size. Please specify it with the size [size_in_sectors] option.\n\n");
		}
		return;
	}

	if (r1 == 0 && r2 == 0) {
		total_disk_size = blocks; /* sizes in sectors */
		cylinders = blocks / (heads * sectors);
		cylinders /= sectors_per_block;
	} else if (r1 == 0) {
		fprintf(stderr, "Unable to get disk geometry. Guessing number of sectors from disk size.\n");
		cylinders = heads = 1;
		sectors = blocks / sectors_per_block;
	}
	fprintf(stderr,"%d heads, %d sectors, %d cylinders %dK total size\n", 
		heads, sectors, cylinders, total_disk_size/2);
}
#endif

int
set_partition (int fd,struct disklabel *d,int num,int offset,int size,int fstype)
{
	int endplace=offset+size;
	int x;

	if(endplace>total_disk_size) {
		fprintf(stderr,"endplace is %d total_disk_size is %d\n",endplace,total_disk_size);
		if (!force) return -1;
		/* correct the discrepancy */
		size = total_disk_size-offset;
		endplace=total_disk_size;
		fprintf(stderr,"Warning: changing endplace to %d and size to %d\n",endplace,size);
	}

	if(num>d->d_npartitions) {
		fprintf(stderr,"Partition not consecutive! This would leave empty partitions.\nNext unset partition is %d.\n",d->d_npartitions);
		if (!force) return -1;
	}
	x=overlaplabel(d,offset,endplace,1U<<num);
	if(x!=-1)
	  fprintf(stderr,"Warning: added partition %d overlaps with partition %d\n",num,x);
	  
	d->d_partitions[num].p_offset=offset;
	d->d_partitions[num].p_size=size;
	d->d_partitions[num].p_fstype=fstype;
	if(num==d->d_npartitions) {
		d->d_npartitions++;
	}
	label_modified=1;
	return 0;
}

void
usage (char *cmd_name)
{
	fprintf(stderr,"Usage: %s drive print\n",cmd_name);
	fprintf(stderr,"       %s drive zero\n",cmd_name);
	fprintf(stderr,"       %s drive partition_number offset_in_sectors size_in_sectors partition_type\n",cmd_name);
	fprintf(stderr,"       %s drive sum\n",cmd_name);
	fprintf(stderr,"       %s drive size size_in_sectors [other command]\n\n",cmd_name);
	fprintf(stderr,"The print command may be placed before or after any other command.\n");
	fprintf(stderr,"The size command is used to override the size of the disk, if the\nprogram isn't able to obtain this information for some reason.\n");
	fprintf(stderr,"The partition type should be 8, unless you are creating\nlabels for OSF/1 partitions.\n");
}


int
main (int argc,char **argv)
{
	struct disklabel d;
	int fd,x;

	if(argc < 3) {
		usage(argv[0]);
		exit(1);
	}
	fd=open(argv[1],O_RDWR);
	if(fd<0) {
		perror("couldn't open scsi disk");
		exit(1);
	}
#ifdef __linux__
	set_disk_size(fd);
#endif
	if(strcmp(argv[2],"zero")==0) {
		zero_disklabel(fd,&d);
	} else {
		if(read_disklabel(fd,&d)) {
			fprintf(stderr,"Error reading disklabel\n");
			exit(1);
		}
	}
	for(x=2;x<argc;) {
		if(strcmp(argv[x],"size")==0 && ((x+1)<argc)) {
			total_disk_size=atoi(argv[x+1]);
			x+=2;
		}
		if(strcmp(argv[x],"sum")==0) {
			dosumlabel(fd,&d);
			x++;
		}
		else if(strcmp(argv[x],"zero")==0) {
			zero_disklabel(fd,&d);
			x++;
		}
		else if(strcmp(argv[x],"print")==0) {
			print_disklabel(fd,&d);
			x++;
		}
		else if(strcmp(argv[x],"force")==0) {
		        force=1;
			x++;
		}
		else {
			if((argc-x)>3 && isdigit(argv[x][0]) && isdigit(argv[x+1][0]) && isdigit(argv[x+2][0]) && isdigit(argv[x+3][0])) {
				int partnum=atoi(argv[x]);
				int offset=atoi(argv[x+1]);
				int size=atoi(argv[x+2]);
				int fstype=atoi(argv[x+3]);
				if(partnum<0 || partnum>7) {
					fprintf(stderr,"Partition number %d out of range--partitions should be between 0 and 7\n",partnum);
					exit(1);
				}
				if(set_partition(fd,&d,partnum,offset,size,fstype)) {
					fprintf(stderr,"Set of partition failed\n");
					exit(1);
				}
				x+=4;
			} else {
				fprintf(stderr,"Unrecognized option %s\n",argv[x]);
				usage(argv[0]);
				exit(1);
			}
		}
	}
	if(label_modified && write_disklabel(fd,&d)) {
		fprintf(stderr,"Error writing disklabel\n");
		exit(1);
	}
	return 0;
}
