/* mknetboot.c 
 * 
 * quick-and-dirty utility to make a netboot image from a stripped net_aboot
 * binary, a linux kernel image (compressed or not), and (optionally) an
 * initial ramdisk.
 * 
 * Copyright (c) 2002 Will Woods and Compaq Computer Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/system.h>

#include "net.h"
#include "config.h"
#include "netaboot.h"

#define PAD_SIZE 512
#define MKNETBOOT_VERSION "1.0"

const char *prog_name;
char *bootpfile;
char pad[PAD_SIZE];
int verbose=0, output_started=0;

void usage(void) {
	fprintf(stderr,"
usage: 
%s -k kernel [-i initrd] [-f kflags] [-o output] [-v]
  -k kernel    kernel to use (gzipped OK)
  -i initrd    initrd image to use
  -f kflags    default kernel boot flags / command line
  -o output    file to write output to (default: \"bootpfile\")
  -v           verbose operation
  -V           version info
",prog_name);
	exit(1);
}

void die(const char *s) {
	fprintf(stderr,"%s: ",prog_name);
	perror(s);
	if (output_started) unlink(bootpfile);
	exit(1);
}

/* we pad everything to a multiple of PAD_SIZE bytes */
int append(int out, const char *file) {
	int in, count, total;
	char buf[4096];
	char err[256];
	snprintf(err,256,"failed to append file %s",file);
	in = open(file,O_RDONLY);
	if (in == -1) die(err);

	total=0;
	while ((count = read(in,buf,sizeof(buf)))) {
		if ((count == -1) || (write(out,buf,count) == -1)) { 
			close(in); die(err); 
		}
		total += count;
	}
	if (write(out,pad,ALIGN_512(total)-total) == -1) {
		close(in); die(err); 
	}
	return(total); 
}

int main(int argc, char **argv) {
	netabootheader_t header;
	struct stat statbuf;
	char *kernel, *initrd, *command_line;
	char bootpfile_default[] = "bootpfile";
	char ch;
	int out, size;
	
	kernel = initrd = bootpfile = command_line = NULL;
	memset(pad,0,sizeof(pad));
	prog_name=strrchr(argv[0],'/');
	if (prog_name) {
		prog_name++;
	} else {
		prog_name=argv[0];
	}
	header.header_size=(int)sizeof(netabootheader_t);
	header.command_line[0]='\0'; /* header.command_line="" */

	while ((ch = getopt(argc, argv, "k:i:f:o:vV")) != -1) {
		switch (ch) {
		case 'k':
			kernel=optarg;
			break;
		case 'i':
			initrd=optarg;
			break;
		case 'f':
			command_line=optarg;
			break;
		case 'o':
			bootpfile=optarg;
			break;
		case 'v':
			verbose=1;
			break;
		case 'V':
			printf("%s version %s, part of aboot %s\n",
			       prog_name, MKNETBOOT_VERSION, ABOOT_VERSION);
			exit(0);
			break; /* obligatory */
		}
	}

	if (!kernel) 
		usage();

	if (!bootpfile) 
		bootpfile=bootpfile_default;

	if (command_line) 
		strcpy(header.command_line,command_line);

	if (stat(kernel,&statbuf) == -1) {
		die("could not stat kernel");
	} else {
		header.kernel_size=(long)statbuf.st_size;
	}
	
	if (!initrd) {
		header.initrd_size = 0;
	} else {
		if (stat(initrd,&statbuf) == -1) {
			die("could not stat initrd");
		} else {
			header.initrd_size=(long)statbuf.st_size;
		}
	}
	
	out = creat(bootpfile,0666);
	if (out == -1) 
		die("could not open bootpfile for output");
	else 
		output_started=1;
	if (verbose) printf("writing to %s\n",bootpfile);

	/* netaboot comes pre-padded */
	if (verbose) printf("adding netaboot: ");
	if ((size = write(out,netaboot,sizeof(netaboot))) == -1)
		die("failed to write netaboot image to bootpfile");
	else if (verbose) 
		printf("%i bytes\n",size);

	/* header */
	if (verbose) printf("adding header info: ");
	size=sizeof(header);
	if (write(out,&header,size) == -1) 
		die("failed to write header date to bootpfile");
	/* padding for header */
	if (write(out,pad,ALIGN_512(size)-size) == -1)
		die("failed to zero-pad header");
	else if (verbose) 
		printf("%i bytes, padded to %lu\n",size,ALIGN_512(size));
	       

	/* kernel */
	if (verbose) printf("adding %s: ",kernel);
	size=append(out,kernel);
	if (verbose) 
		printf("%i bytes, padded to %lu\n",size,ALIGN_512(size));

	/* initrd, if specified */
	if (initrd) {
		if (verbose) printf("adding %s: ",initrd);
		size=append(out,initrd);
		if (verbose) 
			printf("%i bytes, padded to %lu\n",size,ALIGN_512(size));
	}

	/* FIXME: is this really necessary? */
	if (write(out,pad,sizeof(pad)) == -1) 
		die("could not append zero padding to bootpfile");

	close(out);
	return 0;
}

