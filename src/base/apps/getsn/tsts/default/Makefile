APPNAME = getsn

peaks.txt.IGNORELINES = Filename
hirise.txt.IGNORELINES = Filename

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/peaks.cub \
	TO=$(OUTPUT)/peaks.txt FILE=TRUE OBSERVATION=TRUE \
	APPEND=false;
	$(APPNAME) FROM=$(INPUT)/hirise.cub \
	TO= $(OUTPUT)/hirise.txt FILE=TRUE OBSERVATION=TRUE \
	APPEND=false;
