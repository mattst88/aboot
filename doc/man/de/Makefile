all:  srmbootraw.8 aboot.8 aboot.conf.5 abootconf.8 isomarkboot.8 sdisklabel.8  srmbootfat.1

aboot.8: aboot.sgml
	nsgmls aboot.sgml | sgmlspl docbook2man-de-spec.pl

aboot.conf.5: aboot.conf.sgml
	nsgmls aboot.conf.sgml | sgmlspl docbook2man-de-spec.pl

abootconf.8: abootconf.sgml
	nsgmls abootconf.sgml | sgmlspl docbook2man-de-spec.pl

isomarkboot.8: isomarkboot.sgml
	nsgmls isomarkboot.sgml | sgmlspl docbook2man-de-spec.pl

sdisklabel.8: sdisklabel.sgml
	nsgmls sdisklabel.sgml | sgmlspl docbook2man-de-spec.pl

srmbootfat.1: srmbootfat.sgml
	nsgmls srmbootfat.sgml | sgmlspl docbook2man-de-spec.pl

srmbootraw.8: srmbootraw.sgml
	nsgmls srmbootraw.sgml | sgmlspl docbook2man-de-spec.pl


clean :
	rm -f *.html srmbootraw.8 srmbootfat.1 sdisklabel.8 isomarkboot.8 abootconf.8 aboot.conf.5 aboot.8 manpage.links manpage.log manpage.refs
	rm -rf SRM-HOWTO 


#.PHONY clean
