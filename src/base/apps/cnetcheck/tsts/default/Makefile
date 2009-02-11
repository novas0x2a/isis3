APPNAME = cnetcheck

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) fromlist=$(OUTPUT)/list.lis prefix=$(OUTPUT)/cat \
	network=$(INPUT)/NearSide_All.net | grep -v "100% Processed" \
	> $(OUTPUT)/output.txt;
	$(RM) $(OUTPUT)/list.lis;
	$(RM) $(OUTPUT)/catIsland.1
	$(RM) $(OUTPUT)/catIsland.2
