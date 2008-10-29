APPNAME = pointreg

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/cub.list;
	$(APPNAME) files=$(OUTPUT)/cub.list \
	cnet=$(INPUT)/red.net \
	template=$(INPUT)/reg.txt \
	to=$(OUTPUT)/redPointreg.pvl \
	> /dev/null;
	$(RM) $(OUTPUT)/cub.list;
