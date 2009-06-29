APPNAME = jigsaw

include $(ISISROOT)/make/isismake.tsts

commands:
	$(CP) $(INPUT)/*.cub $(OUTPUT)
	$(LS) -1 $(OUTPUT)/*.cub > $(OUTPUT)/cube.lis;	
	$(APPNAME) fromlist=$(OUTPUT)/cube.lis \
	cnet=$(INPUT)/redPntreg.net \
	onet=$(OUTPUT)/case01OutNet.pvl \
	observations=yes \
	method=qrd \
	tol=1.194048 \
	solvedegree=3 \
	camsolve=all \
	twist=no | grep -v "100% Processed" > $(OUTPUT)/case01OutLog.pvl;
	$(RM) $(OUTPUT)/cube.lis;
