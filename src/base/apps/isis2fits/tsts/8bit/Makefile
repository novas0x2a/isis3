APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isis2fitsTruth.cub \
	  to=$(OUTPUT)/isis2fitsTruth4.fits \
	  bittype= 8BIT > /dev/null;
	raw2isis from=$(OUTPUT)/isis2fitsTruth4.fits \
	  to=$(OUTPUT)/isis2fitsTruth4.cub+detached SAMPLES=640 \
	  LINES=768 BANDS=1 SKIP=2880 BYTEORDER=MSB > /dev/null;
	$(CP) $(INPUT)/isis2fitsTruth4.lbl $(OUTPUT)/;
	cubeatt FROM=$(OUTPUT)/isis2fitsTruth4.lbl TO=$(OUTPUT)/truth.cub > /dev/null;
	$(RM) $(OUTPUT)/isis2fitsTruth4.cub $(OUTPUT)/isis2fitsTruth4.lbl \
	  $(OUTPUT)/isis2fitsTruth4.History.IsisCube;
