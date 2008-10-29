APP1NAME = reduce
APP2NAME = crop
APP3NAME = cam2map

labels.txt.IGNORELINES = ByteOrder StartByte Bytes

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APP1NAME) from= $(INPUT)/lua0825f.cub \
	  to= $(OUTPUT)/reduceTruth.cub \
	  ons=100 onl=100 > /dev/null;
	$(APP2NAME) from= $(OUTPUT)/reduceTruth.cub \
	  to= $(OUTPUT)/ReduceCropTruth.cub \
	  sample=10 nsamples=50 \
	  line=10 nlines=50 > /dev/null;
	$(APP3NAME) from= $(OUTPUT)/ReduceCropTruth.cub \
	  to= $(OUTPUT)/ReduceCropCam2mapTruth.cub \
	  > /dev/null;
	catlab from=$(OUTPUT)/ReduceCropCam2mapTruth.cub > $(OUTPUT)/labels.txt;
