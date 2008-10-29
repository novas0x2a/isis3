APPNAME = footprintinit

include $(ISISROOT)/make/isismake.tsts

# cp so I don't destroy the input cube
commands:
	cp $(INPUT)/ab102401.cub $(OUTPUT)/ab102401.cub;
	$(APPNAME) from=$(OUTPUT)/ab102401.cub > /dev/null;
	$$ISISROOT/bin/blobdump \
	  from=$(OUTPUT)/ab102401.cub \
	  to=$(OUTPUT)/ab102401.txt \
	  name=\"MGS/561812335:32/MOC-WA/RED\" \
	  type=Polygon > /dev/null;
	rm $(OUTPUT)/ab102401.cub
