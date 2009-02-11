APPNAME = interest

defautTruth.txt.IGNORELINES = UserName Created LastModified DateTime

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) -1 $(INPUT)/*.cub > $(OUTPUT)/cube.lis;
	$(APPNAME) fromlist=$(OUTPUT)/cube.lis \
	  network=$(INPUT)/autoseed.net \
	  interest=$(INPUT)/sdinterest.def \
	  to=$(OUTPUT)/defautTruth.txt \
	  overlaplist=$(INPUT)/overlaps.lis \
	  networkid=DefaultTruth \
	  Description=Testing > /dev/null;
	$(RM) $(OUTPUT)/cube.lis;
