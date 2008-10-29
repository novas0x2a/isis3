APPNAME = isis2fits

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isisTruth.cub+2 \
	  to=$(OUTPUT)/isis2fitsTruth7.fits \
	  bittype= U16BIT \
	  stretch= manual \
	  minimum = 0 \
	  maximum = 65535 > /dev/null;
