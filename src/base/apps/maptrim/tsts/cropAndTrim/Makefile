APPNAME = maptrim

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isisTruth.map.cub \
	to=$(OUTPUT)/cropAndTrimTruth.cub \
	mode=both \
	slat=15 elat=20 slon=222 elon=230> /dev/null;
