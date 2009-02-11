APPNAME = mapmos

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/input.cub MOSAIC=$(OUTPUT)/mosaic.cub \
	CREATE=YES MINLAT=-75 MAXLAT=-72 MINLON=0 MAXLON=720 > /dev/null;
