APPNAME = cam2map

cam2mapTruth.cub.TOLERANCE = .000001

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from= $(INPUT)/ab102401.cub \
	  to=$(OUTPUT)/cam2mapTruth.cub \
	  pixres=mpp \
	  defaultrange=camera \
	  resolution=1000 > /dev/null;
	$(APPNAME) from= $(INPUT)/ab102401.cub \
	  to=$(OUTPUT)/cam2mapCheckRange.cub \
	  defaultrange=camera \
	  SLAT=15.0 ELAT=20.0 SLON=225.0 ELON=230.0 \
	  pixres=mpp \
	  resolution=1000 > /dev/null;
