APPNAME  = mapmos

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/ab102401.lev2.cub \
	  mosaic=$(OUTPUT)/mapmosTruth2.cub \
	  create=true \
	  match=false \
	  slat=10.765 \
	  elat=34.445 \
	  slon=219.720 \
	  elon=236.190 > /dev/null;
	map2map from=$(INPUT)/ab102402.lev2.cub map=$(INPUT)/ab102401.lev2.cub to=$(OUTPUT)/map.cub defaultrange=map inter=near > /dev/null;
	$(APPNAME) from=$(OUTPUT)/map.cub \
	  mosaic=$(OUTPUT)/mapmosTruth2.cub > /dev/null;
