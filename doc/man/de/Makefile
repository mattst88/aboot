all: aboot.8 aboot.conf.5 abootconf.8 isomarkboot.1 netabootwrap.1 srmbootfat.1 srmbootraw.8 sdisklabel.8

clean:
	rm -f *.html aboot.8 aboot.conf.5 abootconf.8 isomarkboot.1 netabootwrap.1 srmbootfat.1 srmbootraw.8 sdisklabel.8 manpage.links manpage.log manpage.refs

%.1: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

%.5: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

%.8: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

#.PHONY clean
