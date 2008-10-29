APPNAME = mer2isis

labels1.txt.IGNORELINES = StartByte Bytes ByteOrder

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/1M189529263EFF64KCP2977M2F1.IMG \
	  to=$(OUTPUT)/mer2isisTruth1.cub > /dev/null;
	catlab from=$(OUTPUT)/mer2isisTruth1.cub > $(OUTPUT)/labels1.txt;
