APPNAME = footprintinit

include $(ISISROOT)/make/isismake.tsts

# cp so I don't destroy the input cube
commands:
	cp $(INPUT)/CM_1504753137_1.ir.cub $(OUTPUT)/CM_1504753137_1.ir.cub;
	$(APPNAME) from=$(OUTPUT)/CM_1504753137_1.ir.cub > /dev/null;
	$$ISISROOT/bin/blobdump \
	  from=$(OUTPUT)/CM_1504753137_1.ir.cub \
	  to=$(OUTPUT)/CM_1504753137_1.txt \
	  name=\"footprint\" \
	  type=Polygon > /dev/null;
	rm $(OUTPUT)/CM_1504753137_1.ir.cub
