APPNAME = coreg

# tail -1330 testTmp/tempcnet1.txt &gt; <temp> tempcnet.txt</temp> 2&gt;&amp;1 > /dev/null;

coregTruthCnet.txt.SKIPLINES = 9

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/CRU_000004_1200_IR10_0.equi.cub \
	  match=$(INPUT)/CRU_000004_1200_RED4_0.equi.cub \
	  regdef=$(INPUT)/coregtemp.def \
	  to=$(OUTPUT)/coregTruth.cub \
	  cnet=$(OUTPUT)/coregTruthCnet.txt \
	  flatfile=$(OUTPUT)/coregTruthffile.txt \
	  interp=n > /dev/null;
