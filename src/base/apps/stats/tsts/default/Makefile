APPNAME = stats

stats.txt.IGNORELINES  = From
stats2.txt.IGNORELINES = From

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isisTruth.cub \
	  to=$(OUTPUT)/stats.txt > /dev/null;
	$(APPNAME) from=$(INPUT)/isisTruth.cub \
	  validmin=-20 validmax=20 to=$(OUTPUT)/stats2.txt > /dev/null;
