APPNAME = cnetextract

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) cnet=$(INPUT)/cnet.net outnet=$(OUTPUT)/newNet.pvl \
	networkid=new description=new \
	noignore=true noheld=true nomeasureless=true > /dev/null;
