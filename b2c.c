

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void print_usage(void )
{
	printf("Usage: b2c bin_img tar_file.h symname\n");
	exit(1);
}

void open_file(char *fn, int *fd, int *sz)
{
	struct stat buf;

	*fd = open(fn, O_RDONLY);
	if (fd < 0) {
		printf("cannot open %s\n", fn);
		exit(1);
	}

	fstat(*fd, &buf);

	if (buf.st_size <= 10*1024) {
		printf("Is this a right file %s, size = %ld\n", fn, buf.st_size);
		exit(1);
	}

	*sz = (int)buf.st_size;
}

int main(int argc, char **argv)
{
	int sfd, ssz, red, i;
	int buf[1024];
	char *sfn, *tfn, *symname;
	FILE *tfd;

	if (argc != 4) {
		print_usage();
	}

	sfn = argv[1];
	tfn = argv[2];
	symname = argv[3];

	open_file(sfn, &sfd, &ssz);

	tfd = fopen(tfn, "w");

	fprintf(tfd, "int %s[] = {\n", symname);

	while((red=read(sfd, buf, 1024*sizeof(int)))) {
		for (i=0; i<red/sizeof(int); i++) 
			fprintf(tfd, "0x%x, \n", buf[i]);
	}

	fprintf(tfd, "};");

	fclose(tfd);
	
	return 0;

}



