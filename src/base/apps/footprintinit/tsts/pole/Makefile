APPNAME = footprintinit

include $(ISISROOT)/make/isismake.tsts

# cp so I don't destroy the input cube
commands:
	cp $(INPUT)/N1500056057_2.cub $(OUTPUT)/N1500056057_2.cub;
	$(APPNAME) from=$(OUTPUT)/N1500056057_2.cub > /dev/null;
	$$ISISROOT/bin/blobdump \
	  from=$(OUTPUT)/N1500056057_2.cub \
	  to=$(OUTPUT)/N1500056057_2.txt \
	  name=\"footprint\" \
	  type=Polygon > /dev/null;
	rm $(OUTPUT)/N1500056057_2.cub
