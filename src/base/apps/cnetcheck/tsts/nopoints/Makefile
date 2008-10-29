APPNAME = cnetcheck

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) list=$(OUTPUT)/list.lis prefix=$(OUTPUT)/cat \
	control=$(INPUT)/cnet.net | grep -v "100% Processed" \
	> $(OUTPUT)/output.txt;
	$(RM) $(OUTPUT)/list.lis;
