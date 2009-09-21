APPNAME = handmos

include $(ISISROOT)/make/isismake.tsts

commands:
	makecube \
	  to=$(OUTPUT)/handmosTruth2.cub \
	  pixels=null \
	  samples=150 \
	  lines=150 \
	  bands=2 > /dev/null;
	$(APPNAME) from=$(INPUT)/isisTruth.cub \
	  mosaic=$(OUTPUT)/handmosTruth2.cub \
	  priority=beneath \
	  insample=5 \
	  inline= 2 \
	  outsample=10 \
	  outline=10 \
	  outband=1 \
	  create=no  > /dev/null;
