APPNAME = spiceinit

labels.txt.IGNORELINES = ByteOrder StartByte Bytes

include $(ISISROOT)/make/isismake.tsts

commands:
	cp $(INPUT)/v1545949478_1.cub $(OUTPUT)/v1545949478_1.cub;
	$(APPNAME) FROM=$(OUTPUT)/v1545949478_1.cub \
	STARTPAD=1.1 ENDPAD=0.5 ATTACH=FALSE fk='$$cassini/kernels/fk/cas_v39_usgs.tf' > /dev/null;
	catlab FROM=$(OUTPUT)/v1545949478_1.cub > $(OUTPUT)/labels.txt;
	rm $(OUTPUT)/v1545949478_1.cub;
