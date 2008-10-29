APPNAME = phocube

# 1.52587890625e-05
phocubeTruth.cub.TOLERANCE = .0000153

include $(ISISROOT)/make/isismake.tsts

commands:
	 $(APPNAME) from=$(INPUT)/ab102401.cub \
	  to=$(OUTPUT)/phocubeTruth.cub \
	  phase=true \
	  emission=true \
	  incidence=true \
	  latitude=true \
	  longitude=true > /dev/null;
