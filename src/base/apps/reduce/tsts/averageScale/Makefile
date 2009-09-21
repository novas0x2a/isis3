APPNAME = reduce

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from= $(INPUT)/isisTruth.cub \
	  to= $(OUTPUT)/reduceTruth1.cub \
	  algorithm=average \
	  reduction_type=scale \
	  sscale=1 \
	  lscale=1 > /dev/null;
