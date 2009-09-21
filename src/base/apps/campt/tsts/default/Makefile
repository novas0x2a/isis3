APPNAME = campt

camptTruth1.txt.IGNORELINES = Filename

include $(ISISROOT)/make/isismake.tsts

commands:
	cp $(INPUT)/output.txt $(OUTPUT)/camptTruth1.txt
	$(APPNAME) \
	from=$(INPUT)/ab102401.cub \
	to=$(OUTPUT)/camptTruth1.txt \
	> /dev/null 
