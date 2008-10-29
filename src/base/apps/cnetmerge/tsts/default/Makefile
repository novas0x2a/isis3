APPNAME = cnetmerge

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM1=$(INPUT)/testNet.ctl \
	FROM2=$(INPUT)/testNet2.ctl \
	TO=$(OUTPUT)/testSum.pvl \
	FORCE=yes \
	ID=test DESCRIPTION=NewTextCtl \
	> /dev/null;
