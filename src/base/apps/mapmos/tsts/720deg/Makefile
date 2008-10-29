APPNAME = mapmos

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/input.cub MOSAIC=$(OUTPUT)/mosaic.cub \
	CREATE=YES SLAT=-75 ELAT=-72 SLON=0 ELON=720 > /dev/null;
