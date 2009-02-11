APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isis2fitsTruth.cub \
	  to=$(OUTPUT)/isis2fitsTruth3.fits \
	  bittype= U16BIT > /dev/null;
	raw2isis from=$(OUTPUT)/isis2fitsTruth3.fits \
	  to=$(OUTPUT)/truth.cub \
	  SAMPLES=640 LINES=768 BANDS=1 SKIP=2880 \
	  BITTYPE=UNSIGNEDWORD BYTEORDER=MSB > /dev/null;
