APPNAME = mappt
mappt3.txt.IGNORELINES=Filename

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/I00824006RDR.lev2.cub \
	  to=$(OUTPUT)/mappt3.txt \
	  append=false \
	  type=projection \
	  x=-51500 \
	  y=-105500 > /dev/null;
