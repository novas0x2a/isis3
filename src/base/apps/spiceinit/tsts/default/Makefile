APPNAME = spiceinit

labels.txt.IGNORELINES = ByteOrder StartByte Bytes
labelsTwice.txt.IGNORELINES = ByteOrder StartByte Bytes

include $(ISISROOT)/make/isismake.tsts

commands:
	cp $(INPUT)/spiceinitTruth.cub $(OUTPUT)/ > /dev/null;
	$(APPNAME) from=$(OUTPUT)/spiceinitTruth.cub > /dev/null;
	catlab from=$(OUTPUT)/spiceinitTruth.cub > $(OUTPUT)/labels.txt;
	$(APPNAME) from=$(OUTPUT)/spiceinitTruth.cub > /dev/null;
	catlab from=$(OUTPUT)/spiceinitTruth.cub > $(OUTPUT)/labelsTwice.txt;
	rm $(OUTPUT)/spiceinitTruth.cub;
