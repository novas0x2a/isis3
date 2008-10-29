APPNAME = mdis2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/EN0001426030M.IMG \
	  to=$(OUTPUT)/mdis2isisTruth.cub > /dev/null
	$(APPNAME) from=$(INPUT)/EN0089569526M.IMG \
	  to=$(OUTPUT)/mdis2isisLutTruth.cub > /dev/null
