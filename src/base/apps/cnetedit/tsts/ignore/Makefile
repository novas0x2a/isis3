APPNAME = cnetedit

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) CNET=$(INPUT)/cnet.net \
	IGNORELIST=$(OUTPUT)/list.lis \
	ONET=$(OUTPUT)/cnet.pvl > /dev/null;
	$(RM) $(OUTPUT)/list.lis;
