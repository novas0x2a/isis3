APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isis2fitsTruth.cub \
	  to=$(OUTPUT)/isis2fitsTruth2.fits \
	  bittype= S16BIT > /dev/null;
	raw2isis from=$(OUTPUT)/isis2fitsTruth2.fits \
	  to=$(OUTPUT)/truth.cub \
	  SAMPLES=640 LINES=768 BANDS=1 SKIP=2880 \
	  BITTYPE=SIGNEDWORD BYTEORDER=MSB > /dev/null;
