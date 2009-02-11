APPNAME = overlapstats

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) from=$(OUTPUT)/list.lis \
	to=$(OUTPUT)/csv.txt OVERLAPLIST=$(INPUT)/overlaps.lis \
	tabletype=csv > /dev/null;
	$(APPNAME) from=$(OUTPUT)/list.lis \
	to=$(OUTPUT)/tab.txt OVERLAPLIST=$(INPUT)/overlaps.lis \
	> /dev/null;
	$(RM) $(OUTPUT)/list.lis;
