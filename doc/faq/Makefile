all: aboot.8 aboot.conf.5 abootconf.8 isomarkboot.8 sdisklabel.8 srmbootfat.1 srmbootraw.8

aboot.8: aboot.sgml
	nsgmls aboot.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

aboot.conf.5: aboot.conf.sgml
	nsgmls aboot.conf.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

abootconf.8: abootconf.sgml
	nsgmls abootconf.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

isomarkboot.8: isomarkboot.sgml
	nsgmls isomarkboot.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

sdisklabel.8: sdisklabel.sgml
	nsgmls sdisklabel.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

srmbootfat.1: srmbootfat.sgml
	nsgmls srmbootfat.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl

srmbootraw.8: srmbootraw.sgml
	nsgmls srmbootraw.sgml | sgmlspl sgmlspl-specs/docbook2man-spec.pl


all-html : FAQ.html index.html aboot.html

clean :
	rm -f *.html srmbootraw.8 srmbootfat.1 sdisklabel.8 isomarkboot.8 abootconf.8 aboot.conf.5 aboot.8 

FAQ.html : FAQ.sgml
	    sgml2html FAQ.sgml

index.html :  MILO-HOWTO.sgml
	    docbook2html MILO-HOWTO.sgml

aboot.html : SRM-HOWTO.sgml
	    docbook2html SRM-HOWTO.sgml

#.PHONY clean
