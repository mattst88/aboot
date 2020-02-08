override CFLAGS	+= -O2 -g -pipe -Wall
#override LDFLAGS	+= -s
SBINDIR	= ../bin

all:	srmbootfat srmbootraw srmbootfat.1 srmbootraw.8

srmbootfat: srmbootfat.c msdos.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o srmbootfat srmbootfat.c

srmbootraw: srmbootraw.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o srmbootraw srmbootraw.c

%.1: %.sgml
	docbook2man -d sgmlspl-specs/docbook2man-spec.pl $<

%.8: %.sgml
	docbook2man -d sgmlspl-specs/docbook2man-spec.pl $<

clean:
	rm -f srmbootfat srmbootraw srmbootfat.1 srmbootraw.8 manpage.links manpage.log manpage.refs

