APPNAME = cnetextract

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) fromlist=$(OUTPUT)/list.lis cnet=$(INPUT)/cnet.net \
	tolist=$(OUTPUT)/newList.lis outnet=$(OUTPUT)/newNetPoint.pvl \
	networkid=new description=new \
	noignore=true noheld=true \
	cubepoints=$(OUTPUT)/list.lis > /dev/null;
	$(APPNAME) fromlist=$(OUTPUT)/list.lis cnet=$(INPUT)/cnet.net \
	tolist=$(OUTPUT)/newList.lis outnet=$(OUTPUT)/newNetMeasure.pvl \
	networkid=new description=new \
	noignore=true noheld=true \
	cubepoints=$(OUTPUT)/list.lis  cubemeasures=true > /dev/null;
	$(RM) $(OUTPUT)/newList.lis;
	$(RM) $(OUTPUT)/list.lis;
