all-html : SRM-HOWTO/index.html

clean :
	rm -rf SRM-HOWTO 

SRM-HOWTO/index.html : SRM-HOWTO.sgml
	            sgmltools --backend=html SRM-HOWTO.sgml

