APPNAME = footprintmerge

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/fromList.lis;
	$(APPNAME) fromlist=$(OUTPUT)/fromList.lis \
	to=$(OUTPUT)/islands.txt mode=FULL > /dev/null;> /dev/null;
	$(RM) $(OUTPUT)/fromList.lis;
	$(SED) -e 's+input/++' -e 's+/.*/++' $(OUTPUT)/islands.txt > $(OUTPUT)/islands2.txt
	$(RM) $(OUTPUT)/islands.txt

