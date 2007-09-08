# eventually this should have rules for recreating the man pages from the 
# sgml source, but since this doesn't work on my machine right now i'm
# leaving it out.

# if this is a recursive make, and mandir was set in the top-level Makefile,
# use that value. Otherwise default to /usr/man.

ifeq ($(mandir),)
MANDIR=/usr/man
else
MANDIR=$(mandir)
endif
MAN1=$(MANDIR)/man1
MAN5=$(MANDIR)/man5
MAN8=$(MANDIR)/man8

default: 

install: 
	install -d $(MAN1) $(MAN5) $(MAN8)
	install -c isomarkboot.1 $(MAN1)
	install -c aboot.conf.5 $(MAN5)
	install -c aboot.8 abootconf.8 e2writeboot.8 swriteboot.8 sdisklabel.8 $(MAN8)	
install-gz: install
	gzip -f9 $(MAN1)/isomarkboot.1
	gzip -f9 $(MAN5)/aboot.conf.5
	gzip -f9 $(MAN8)/aboot.8 $(MAN8)/abootconf.8 $(MAN8)/e2writeboot.8 \
		 $(MAN8)/swriteboot.8 $(MAN8)/sdisklabel.8
install-gzip: install-gz
