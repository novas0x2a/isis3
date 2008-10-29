APPNAME = lo2isis

labels.txt.IGNORELINES = Bytes StartByte ByteOrder

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/4164H_Full_mirror.cub \
	to=$(OUTPUT)/4164H_Full_mirror.truth.cub > /dev/null;
	catlab from=$(OUTPUT)/4164H_Full_mirror.truth.cub >& $(OUTPUT)/labels.txt;
