APPNAME = enlarge

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/isisTruth.cub \
	  to=$(OUTPUT)/enlargeTruth.cub \
	  smag=1.0 \
	  lmag=1.5 \
	  interp=nearestneighbor > /dev/null;
