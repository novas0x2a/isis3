APPNAME = overlapstats

error.txt.IGNORELINES = Processed

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	if [ `$(APPNAME) from=$(OUTPUT)/list.lis \
	to=$(OUTPUT)/tab.txt OVERLAPLIST=$(INPUT)/overlaps.lis \
	>& $(OUTPUT)/error.txt` ]; then \
	  true; \
        fi; \
	$(RM) $(OUTPUT)/list.lis;
