APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isisTruth.cub+2 \
	  to=$(OUTPUT)/isis2fitsTruth5.fits \
	  bittype= 8BIT \
	  stretch= none > /dev/null;
