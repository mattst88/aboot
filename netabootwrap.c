#include <sys/types.h>
#include <asm/console.h>
#include "system.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "netwrap.h"
#include "bootloader.h"


char *tfn="netboot.img", *kfn="vmlinux.gz", *ifn=NULL, *barg=NULL;
char *progname;

void print_usage(void )
{
	printf("Following shows options and default values or example value\n");
	printf("%s -t netboot.img -k vmlinux.gz -i initrd.gz -a \"root=/dev/hda1 single\"\n", progname);
	exit(1);
}

void open_file(char *fn, int *fd, int *sz)
{
	struct stat buf;

	*fd = open(fn, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s: Cannot open %s\n", progname, fn);
		print_usage();
		exit(1);
	}

	fstat(*fd, &buf);

	if (buf.st_size <= 10*1024) {
		fprintf(stderr, "%s:Is this a right file %s, size = %d\n", progname, fn, (int)buf.st_size);
		print_usage();
		exit(1);
	}

	*sz = buf.st_size;
}

void append_file(int tfd, int sfd)
{
	char buf[4096];
	int  red;

	while ((red=read(sfd, buf, 4096)))
		write(tfd, buf, red);
}

int main(int argc, char **argv)
{
	int tfd=0, kfd=0, ifd=0, ksz=0, isz=0;
	struct header hdr;
	char *stmp;

	progname=argv[0];

	/*
	 * Read switches.
	 */
	for (argc--, argv++; argc > 0; argc--, argv++) {
		if (argv[0][0] != '-')
			break;
		switch (argv[0][1]) {

		case 't':		    /* override target file name */
			if (argv[0][2]) {
				stmp = &(argv[0][2]);
			} else {
				argc--;
				argv++;
				stmp = argv[0];
			}
			if (!stmp) {
				fprintf(stderr,
				    "%s: missing file name for target\n",progname);
				break;
			}
			tfn = stmp;
			break;
		case 'k':			/* override kernel name */
			if (argv[0][2]) {
				stmp = &(argv[0][2]);
			} else {
				argc--;
				argv++;
				stmp = argv[0];
			}
			if (!stmp) {
				fprintf(stderr,
					"%s: missing file name for kernel\n",progname);
				break;
			}
			kfn = stmp;
			break;
		case 'i':			/* override file name for initrd */
			if (argv[0][2]) {
				stmp = &(argv[0][2]);
			} else {
				argc--;
				argv++;
				stmp = argv[0];
			}
			if (!stmp) {
				fprintf(stderr,
					"%s: missing file name for initial RAM-disk\n",progname);
				break;
			}
			ifn = stmp;
			break;
		case 'a':			/* add kernel parameters */
			if (argv[0][2]) {
				stmp = &(argv[0][2]);
			} else {
				argc--;
				argv++;
				stmp = argv[0];
			}
			if (!stmp) {
				fprintf(stderr,
					"%s: No kernel parameters specified\n",progname);
				break;
			}
			barg = stmp;
			break;
		default:
			fprintf(stderr, "%s: unknown switch: -%c\n",
					progname, argv[0][1]);
			print_usage();
			break;

		} /* switch */
	} /* for args */

	open_file(kfn, &kfd, &ksz);

	if (ifn)
		open_file(ifn, &ifd, &isz);

	printf("Target file name is %s\n", tfn);
	unlink(tfn);
	tfd = open(tfn, O_RDWR|O_CREAT, 0644);

	write(tfd, bootloader, sizeof(bootloader));

	hdr.header_size = sizeof(int)*3;
	hdr.kern_size = ksz;
	hdr.ird_size = isz;

	if (barg) printf("With kernel arguments : %s \n", barg);
	else printf("Without kernel argument\n");

	if (barg) {
		if (strlen(barg) >= 200) {
			printf("Kernel argument-list is too long\n");
			exit(1);
		}
		strncpy(hdr.boot_arg, barg, strlen(barg)+1);
		hdr.header_size += strlen(barg)+1;
	}

	lseek(tfd, align_512(sizeof(bootloader)), SEEK_SET);
	write(tfd, &hdr, hdr.header_size);

	printf("Binding kernel %s\n", kfn);
	lseek(tfd, align_512((unsigned long)lseek(tfd, 0, SEEK_CUR)), SEEK_SET);
	append_file(tfd, kfd);

	if (ifn) {
		printf("Binding initrd %s\n", ifn);
		lseek(tfd, align_512((unsigned long)lseek(tfd, 0, SEEK_CUR)), SEEK_SET);
		append_file(tfd, ifd);
	}

	close(tfd);
	printf("Done.\n");
	return 0;
}



