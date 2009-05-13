APPNAME = lronac2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/nacl00159579_unpacked.IMG TO=$(OUTPUT)/nacl00159579_unpacked.cub > /dev/null;
	crop FROM=$(OUTPUT)/nacl00159579_unpacked.cub NSAMPLES=5 NLINES=5 TO=$(OUTPUT)/cropped.cub > /dev/null;
	$(RM) $(OUTPUT)/nacl00159579_unpacked.cub > /dev/null;
	catlab FROM=$(OUTPUT)/cropped.cub > $(OUTPUT)/cropped.pvl;
