APPNAME = autoregtemplate

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) algorithm=MinimumDifference \
	  tolerance=0.7 sampling=50 psamp=15 pline=15 \
	  pmin=1200 pmax=2800 ppercent=75 ssamp=31 sline=31 smin=1200 smax=2800 \
	  spercent=75 sinc=4 linc=6  \
	  minimumpatternzscore = 0.5 \
	  to=$(OUTPUT)/autoregtemplateTruth2.txt > /dev/null;
