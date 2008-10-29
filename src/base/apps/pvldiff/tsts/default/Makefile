APPNAME = pvldiff

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/pvl1.pvl FROM2=$(INPUT)/pvl2.pvl \
	TO=$(OUTPUT)/diff_simp.txt > /dev/null;
	$(APPNAME) FROM=$(INPUT)/pvl2.pvl FROM2=$(INPUT)/pvl3.pvl \
	TO=$(OUTPUT)/diff_units.txt > /dev/null;
	$(APPNAME) FROM=$(INPUT)/pvl4.pvl FROM2=$(INPUT)/pvl5.pvl \
	TO=$(OUTPUT)/diff_complex.txt > /dev/null;
