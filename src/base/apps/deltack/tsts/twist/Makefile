APPNAME = deltack
#This file tests the twist (2-point) option

include $(ISISROOT)/make/isismake.tsts

commands:
	$(CP) $(INPUT)/*.cub $(OUTPUT)
	$(APPNAME) from=$(OUTPUT)/I08526015RDR_band9.cub \
	maxits=25 \
	tol=0.4194172 \
	twist=true \
	samp1=257. \
	line1=7110. \
	lat1=1.628526954186 \
	lon1=355.78372744261 \
	samp2=99.25 \
	line2=8044.75 \
	lat2=0.069893360676277 \
	lon2=355.31374996155 > /dev/null;
	$(RM) print.prt < /dev/null;
