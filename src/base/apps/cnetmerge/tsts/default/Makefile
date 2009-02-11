APPNAME = cnetmerge

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) INPUTTYPE=cnets \
	FROM1=$(INPUT)/testNet.ctl \
	FROM2=$(INPUT)/testNet2.ctl \
	TO=$(OUTPUT)/testForce.pvl \
	DUPLICATEPOINTS=rename \
	ID=testForceMerge DESCRIPTION=NewTextCtl \
	> /dev/null;
	$(APPNAME) INPUTTYPE=cnets \
	FROM1=$(INPUT)/testNet.ctl \
	FROM2=$(INPUT)/testNet2.ctl \
	TO=$(OUTPUT)/testReplace.pvl \
	DUPLICATEPOINTS=replace \
	ID=testReplaceMerge DESCRIPTION=NewTextCtl \
	> /dev/null;
