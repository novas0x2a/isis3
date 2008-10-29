APPNAME = map2map

labels.txt.IGNORELINES = Bytes ByteOrder

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/lub0428b.cub \
	MAP=$(INPUT)/mapfile.map \
	TO=$(OUTPUT)/map2mapTruth.cub \
	ELON=-109 > /dev/null;
	catlab FROM=$(OUTPUT)/map2mapTruth.cub >& $(OUTPUT)/labels.txt;
