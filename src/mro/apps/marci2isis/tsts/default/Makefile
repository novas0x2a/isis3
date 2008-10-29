APPNAME = marci2isis

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) FROM=$(INPUT)/MOI_000009_0294_MU_00N044W.IMG EVENODD=false TO=$(OUTPUT)/auto.cub > /dev/null;
	catlab FROM=$(OUTPUT)/auto.cub > $(OUTPUT)/auto.pvl;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG EVEN=$(OUTPUT)/even.cub \
	ODD=$(OUTPUT)/odd.cub  > /dev/null;
	catlab FROM=$(OUTPUT)/even.cub > $(OUTPUT)/even.pvl;
	catlab FROM=$(OUTPUT)/odd.cub > $(OUTPUT)/odd.pvl;
	$(APPNAME) FROM=$(INPUT)/T02_001251_1292_MU_00N237W.IMG flip=NO EVENODD=false TO=$(OUTPUT)/unflipped.cub > /dev/null;
	catlab FROM=$(OUTPUT)/unflipped.cub > $(OUTPUT)/unflipped.pvl;
