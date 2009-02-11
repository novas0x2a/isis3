APPNAME = cisscal

labels.truth.txt.IGNORELINES = Bytes ByteOrder

include $(ISISROOT)/make/isismake.tsts


commands:
	$(APPNAME) from=$(INPUT)/N1355543510_1.c2i.nospice.cub \
	to=$(OUTPUT)/N1355543510_1.truth.cub \
	> /dev/null;
	catlab from=$(OUTPUT)/N1355543510_1.truth.cub >& $(OUTPUT)/labels.truth.txt;
