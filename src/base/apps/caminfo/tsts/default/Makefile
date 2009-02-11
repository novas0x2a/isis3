APPNAME = caminfo

caminfo.pvl.IGNORELINES = RunDate IsisVersion From Version

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/input.cub \
	TO=$(OUTPUT)/caminfo.pvl ISISLABEL=true ORIGINAL=true \
	STATISTICS=true CAMSTATS=true > /dev/null;
