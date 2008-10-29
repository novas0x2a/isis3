APPNAME = isisui
standardOutTruth1.txt.IGNORELINES=TO

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) ratio num=numerator den=denominator \
	  to=$(OUTPUT)/output.txt > $(OUTPUT)/standardOutTruth1.txt;
