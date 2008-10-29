APPNAME = autoregtemplate

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) algorithm=MaximumCorrelation  \
	  psamp=15 pline=15 ssamp=31 sline=31 \
	  to=$(OUTPUT)/autoregtemplateTruth1.txt > /dev/null;
