APPNAME = marci2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG \
	TO=$(OUTPUT)/nooffset.cub EVENODD=false COLOROFFSET=false > /dev/null;
	catlab FROM=$(OUTPUT)/nooffset.cub > $(OUTPUT)/nooffset.pvl;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG \
	TO=$(OUTPUT)/offset1.cub EVENODD=false COLOROFFSET_SIZE=1 > /dev/null;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG \
	TO=$(OUTPUT)/offset2.cub EVENODD=false COLOROFFSET_SIZE=2 > /dev/null;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG \
	TO=$(OUTPUT)/offset3.cub EVENODD=false COLOROFFSET_SIZE=3 > /dev/null;
	catlab FROM=$(OUTPUT)/offset3.cub > $(OUTPUT)/offset3.pvl;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG \
	TO=$(OUTPUT)/flipped.cub EVENODD=false FLIP=yes \
	COLOROFFSET_SIZE=3 > /dev/null;
