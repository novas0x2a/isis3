IMPORT    = lrowac2isis
SPICEINIT = spiceinit
CROP      = crop
PROJECT   = cam2map
MOSAIC    = automos

# These cubes, if there is a problem, will be
#   very significantly different
wac000000d2.uv.lev2.cub.TOLERANCE = 0.0002
wac000000d2.vis.lev2.cub.TOLERANCE = 0.0003

include $(ISISROOT)/make/isismake.tsts

commands:
	$(IMPORT) from=$(INPUT)/wac000000d2.img \
	  to=$(OUTPUT)/wac000000d2.cub \
	  > /dev/null;
	$(SPICEINIT) from=$(OUTPUT)/wac000000d2.vis.even.cub > /dev/null;
	$(SPICEINIT) from=$(OUTPUT)/wac000000d2.vis.odd.cub > /dev/null;
	$(SPICEINIT) from=$(OUTPUT)/wac000000d2.uv.even.cub > /dev/null;
	$(SPICEINIT) from=$(OUTPUT)/wac000000d2.uv.odd.cub > /dev/null;
	catlab from=$(OUTPUT)/wac000000d2.vis.even.cub > $(OUTPUT)/vislabel.pvl;
	catlab from=$(OUTPUT)/wac000000d2.uv.odd.cub > $(OUTPUT)/uvlabel.pvl;
	$(CROP) from=$(OUTPUT)/wac000000d2.vis.even.cub nlines=500 \
	  TO=$(OUTPUT)/wac000000d2.vis.even.crop.cub > /dev/null;
	$(MV) $(OUTPUT)/wac000000d2.vis.even.crop.cub \
	  $(OUTPUT)/wac000000d2.vis.even.cub;
	$(CROP) from=$(OUTPUT)/wac000000d2.vis.odd.cub nlines=500 \
	  TO=$(OUTPUT)/wac000000d2.vis.odd.crop.cub > /dev/null;
	$(MV) $(OUTPUT)/wac000000d2.vis.odd.crop.cub \
	  $(OUTPUT)/wac000000d2.vis.odd.cub;
	$(CROP) from=$(OUTPUT)/wac000000d2.uv.even.cub nlines=100 \
	  TO=$(OUTPUT)/wac000000d2.uv.even.crop.cub > /dev/null;
	$(MV) $(OUTPUT)/wac000000d2.uv.even.crop.cub \
	  $(OUTPUT)/wac000000d2.uv.even.cub;
	$(CROP) from=$(OUTPUT)/wac000000d2.uv.odd.cub nlines=100 \
	  TO=$(OUTPUT)/wac000000d2.uv.odd.crop.cub > /dev/null;
	$(MV) $(OUTPUT)/wac000000d2.uv.odd.crop.cub \
	  $(OUTPUT)/wac000000d2.uv.odd.cub;
	$(PROJECT) FROM=$(OUTPUT)/wac000000d2.vis.even.cub \
	  TO=$(OUTPUT)/wac000000d2.vis.even.lev2.cub PIXRES=MPP \
	  RESOLUTION=500 > /dev/null;
	$(PROJECT) FROM=$(OUTPUT)/wac000000d2.vis.odd.cub \
	  TO=$(OUTPUT)/wac000000d2.vis.odd.lev2.cub \
	  MAP=$(OUTPUT)/wac000000d2.vis.even.lev2.cub MATCHMAP=TRUE \
	  > /dev/null;
	$(PROJECT) FROM=$(OUTPUT)/wac000000d2.uv.even.cub \
	  TO=$(OUTPUT)/wac000000d2.uv.even.lev2.cub \
	  MAP=$(OUTPUT)/wac000000d2.vis.even.lev2.cub MATCHMAP=TRUE \
	  > /dev/null;
	$(PROJECT) FROM=$(OUTPUT)/wac000000d2.uv.odd.cub \
	  TO=$(OUTPUT)/wac000000d2.uv.odd.lev2.cub \
	  MAP=$(OUTPUT)/wac000000d2.vis.even.lev2.cub MATCHMAP=TRUE \
	  > /dev/null;
	$(LS) $(OUTPUT)/wac000000d2.vis.even.lev2.cub \
	  $(OUTPUT)/wac000000d2.vis.odd.lev2.cub > $(OUTPUT)/vis.lis;
	$(MOSAIC) FROMLIST=$(OUTPUT)/vis.lis \
	  MOSAIC=$(OUTPUT)/wac000000d2.vis.lev2.cub > /dev/null;
	$(LS) $(OUTPUT)/wac000000d2.uv.even.lev2.cub \
	  $(OUTPUT)/wac000000d2.uv.odd.lev2.cub > $(OUTPUT)/uv.lis;
	$(MOSAIC) FROMLIST=$(OUTPUT)/uv.lis \
	  MOSAIC=$(OUTPUT)/wac000000d2.uv.lev2.cub > /dev/null;
	$(RM) $(OUTPUT)/vis.lis;
	$(RM) $(OUTPUT)/uv.lis;
	$(RM) $(OUTPUT)/wac000000d2.vis.even.lev2.cub;
	$(RM) $(OUTPUT)/wac000000d2.vis.odd.lev2.cub;
	$(RM) $(OUTPUT)/wac000000d2.uv.even.lev2.cub;
	$(RM) $(OUTPUT)/wac000000d2.uv.odd.lev2.cub;
	$(RM) $(OUTPUT)/wac000000d2.vis.odd.cub;
	$(RM) $(OUTPUT)/wac000000d2.uv.odd.cub;
