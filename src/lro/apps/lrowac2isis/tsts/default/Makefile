APPNAME = lrowac2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/M010368000MC.IMG TO=$(OUTPUT)/out.cub > /dev/null;
	catlab FROM=$(OUTPUT)/out.vis.even.cub > $(OUTPUT)/out.vis.even.pvl;
	catlab FROM=$(OUTPUT)/out.vis.odd.cub > $(OUTPUT)/out.vis.odd.pvl;
