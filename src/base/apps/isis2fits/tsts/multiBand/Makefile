APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isis2fitsTruthMultiBand.cub \
	  to=$(OUTPUT)/isis2fitsTruth6.fits \
	  bittype= S16BIT > /dev/null;
	raw2isis from=$(OUTPUT)/isis2fitsTruth6.fits \
	  to=$(OUTPUT)/truth.cub \
	  SAMPLES=640 LINES=768 BANDS=3 SKIP=2880 \
	  BITTYPE=SIGNEDWORD BYTEORDER=MSB > /dev/null;
