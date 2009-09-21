APPNAME = lronac2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/nacr000000c9.img TO=$(OUTPUT)/nacr000000c9.cub > /dev/null;
	crop FROM=$(OUTPUT)/nacr000000c9.cub NSAMPLES=1024 NLINES=1024 sample=1000 line=18000 TO=$(OUTPUT)/nacr000000c9.crop.cub > /dev/null;
	$(RM) $(OUTPUT)/nacr000000c9.cub > /dev/null;
	catlab FROM=$(OUTPUT)/nacr000000c9.crop.cub > $(OUTPUT)/nacr000000c9.pvl;
	
	$(APPNAME) FROM=$(INPUT)/nacl000000c9.img TO=$(OUTPUT)/nacl000000c9.cub > /dev/null;
	crop FROM=$(OUTPUT)/nacl000000c9.cub NSAMPLES=1024 NLINES=1024 sample=1000 line=18000 TO=$(OUTPUT)/nacl000000c9.crop.cub > /dev/null;
	$(RM) $(OUTPUT)/nacl000000c9.cub > /dev/null;
	catlab FROM=$(OUTPUT)/nacl000000c9.crop.cub > $(OUTPUT)/nacl000000c9.pvl;
