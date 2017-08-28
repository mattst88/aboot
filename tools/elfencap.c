#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include <linux/elf.h>


int
main (int argc, char ** argv)
{
    int ifd;
    ssize_t n;
    char buf[8192];
    struct stat st;
    struct {
	struct elf64_hdr  ehdr;
	struct elf64_phdr phdr;
    } h;

    ifd = open(argv[1], O_RDONLY);
    if (ifd < 0) {
	perror(argv[1]);
	return 1;
    }

    if (fstat(ifd, &st) < 0) {
	perror(argv[1]);
	return 1;
    }

    memset(&h, 0, sizeof(h));

    h.ehdr.e_ident[0] = 0x7f;
    memcpy(&h.ehdr.e_ident[1], "ELF", 4);
    h.ehdr.e_ident[EI_CLASS]	= ELFCLASS64;
    h.ehdr.e_ident[EI_DATA]	= ELFDATA2LSB;
    h.ehdr.e_ident[EI_VERSION]	= EV_CURRENT;
    h.ehdr.e_type		= ET_EXEC;
    h.ehdr.e_machine		= EM_ALPHA;
    h.ehdr.e_version		= EV_CURRENT;
    h.ehdr.e_entry		= 0xfffffc0000310000;
    h.ehdr.e_phnum		= 1;
    h.ehdr.e_phoff		= (char *) &h.phdr - (char *) &h;
    h.phdr.p_vaddr		= 0xfffffc0000310000;
    h.phdr.p_offset		= sizeof(h);
    h.phdr.p_filesz		= st.st_size;
    h.phdr.p_memsz		= h.phdr.p_filesz;

    write(1, &h, sizeof(h));

    while ((n = read(ifd, buf, sizeof(buf))) > 0) {
	if (write(1, buf, n) != n) {
	    perror("short write");
	    return 1;
	}
    }
    return 0;
}
