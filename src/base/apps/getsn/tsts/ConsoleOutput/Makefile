APPNAME = getsn

peaks.txt.IGNORELINES = Filename
hirise.txt.IGNORELINES = Filename

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/peaks.cub \
        >& $(OUTPUT)/peaks.txt \
	FILE=false OBSERVATION=false;
	$(APPNAME) FROM=$(INPUT)/hirise.cub \
	>& $(OUTPUT)/hirise.txt FILE=false;
