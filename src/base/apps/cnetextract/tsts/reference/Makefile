APPNAME = cnetextract

include $(ISISROOT)/make/isismake.tsts

commands:
	$(LS) $(INPUT)/*.cub > $(OUTPUT)/list.lis;
	$(APPNAME) fromlist=$(OUTPUT)/list.lis cnet=$(INPUT)/cnet.net \
	tolist=$(OUTPUT)/newList.lis outnet=$(OUTPUT)/newNet.pvl \
	networkid=new description=new \
	reference=true > /dev/null;
	$(RM) $(OUTPUT)/newList.lis;
	$(RM) $(OUTPUT)/list.lis;
