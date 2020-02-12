all:  srmbootraw.8 aboot.8 aboot.conf.5 abootconf.8 isomarkboot.1 sdisklabel.8  srmbootfat.1 netabootwrap.1

clean :
	rm -f *.html srmbootraw.8 srmbootfat.1 sdisklabel.8 isomarkboot.1 abootconf.8 aboot.conf.5 aboot.8 netabootwrap.1 manpage.links manpage.log manpage.refs

%.1: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

%.5: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

%.8: %.sgml
	docbook2man -d docbook2man-de-spec.pl $<

#.PHONY clean
