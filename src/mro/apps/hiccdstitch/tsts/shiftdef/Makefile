APPNAME = hiccdstitch

placement.txt.IGNORELINES=File

include $(ISISROOT)/make/isismake.tsts

commands:
	t=`ls $(INPUT)/PSP_001446_1790_RED?.cub > $(OUTPUT)/list.temp`; \
	$(APPNAME) fromlist=$(OUTPUT)/list.temp \
	  shiftdef=$(INPUT)/shift.txt \
	  to=$(OUTPUT)/combined.cub \
	  placement=$(OUTPUT)/placement.txt > /dev/null; \
	t=`rm -f $(OUTPUT)/list.temp`;
