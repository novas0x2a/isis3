APPNAME = cnetadd

include $(ISISROOT)/make/isismake.tsts

outnet.pvl.IGNORELINES = DateTime

commands:
	$(LS) $(INPUT)/*M.cub > $(OUTPUT)/add.lis;
	$(LS) $(INPUT)/*cal.cub > $(OUTPUT)/files.lis;
	$(APPNAME) fromlist=$(OUTPUT)/files.lis \
        cnet=$(INPUT)/input.net fromlist2=$(OUTPUT)/add.lis \
	to=$(OUTPUT)/outnet.pvl log=$(OUTPUT)/outlog.pvl \
	> /dev/null;
	$(RM) $(OUTPUT)/add.lis;
	$(RM) $(OUTPUT)/files.lis;
