APPNAME = reduce

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from= $(INPUT)/isisTruth.cub \
	  to= $(OUTPUT)/reduceTruth4.cub \
	  algorithm=nearestneighbor \
	  ons=4 \
	  onl=3 > /dev/null;
