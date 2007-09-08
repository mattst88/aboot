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

all: aboot.8 aboot.conf.5 abootconf.8 isomarkboot.1 sdisklabel.8 netabootwrap.1

install: 
	install -d $(MAN1) $(MAN5) $(MAN8)
	install -c isomarkboot.1 netabootwrap.1 $(MAN1)
	install -c aboot.conf.5 $(MAN5)
	install -c aboot.8 abootconf.8 e2writeboot.8 swriteboot.8 sdisklabel.8 $(MAN8)	
install-gz: install
	gzip -f9 $(MAN1)/isomarkboot.1 $(MAN1)/netabootwrap.1
	gzip -f9 $(MAN5)/aboot.conf.5
	gzip -f9 $(MAN8)/aboot.8 $(MAN8)/abootconf.8 $(MAN8)/e2writeboot.8 \
		 $(MAN8)/swriteboot.8 $(MAN8)/sdisklabel.8
install-gzip: install-gz

clean:
	rm -f aboot.8 aboot.conf.5 abootconf.8 isomarkboot.1 sdisklabel.8 netabootwrap.1 manpage.log manpage.links manpage.refs

aboot.8: aboot.sgml
	nsgmls aboot.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

aboot.conf.5: aboot.conf.sgml
	nsgmls aboot.conf.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

abootconf.8: abootconf.sgml
	nsgmls abootconf.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

isomarkboot.1: isomarkboot.sgml
	nsgmls isomarkboot.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

netabootwrap.1: netabootwrap.sgml
	nsgmls netabootwrap.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

sdisklabel.8: sdisklabel.sgml
	nsgmls sdisklabel.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

