APPNAME = lrowac2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/M010368000MC.IMG EVEN=$(OUTPUT)/even.cub ODD=$(OUTPUT)/odd.cub > /dev/null;
	catlab FROM=$(OUTPUT)/even.cub > $(OUTPUT)/even.pvl;
	catlab FROM=$(OUTPUT)/odd.cub > $(OUTPUT)/odd.pvl;
