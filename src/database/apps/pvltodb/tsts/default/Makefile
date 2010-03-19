APPNAME = pvltodb

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/CM_1575550531_1.ir.pvl dbmap=$(INPUT)/VIMS.pdm  \
	  mode=validate showsql=yes > $(OUTPUT)/pvltodb.pvl;
