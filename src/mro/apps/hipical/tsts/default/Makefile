APPNAME = hipical

include $(ISISROOT)/make/isismake.tsts

commands:
	$(APPNAME) from=$(INPUT)/sized.cub \
	  matrix=$(INPUT)/A_matrix_tdi32_bin2.cub \
	  to=$(OUTPUT)/hipical_gain_truth.cub \
	  option=gain > /dev/null;
	$(APPNAME) from=$(INPUT)/sized.cub \
	  matrix=$(INPUT)/B_matrix_tdi32_bin2.cub \
	  to=$(OUTPUT)/hipical_offset_truth.cub \
	  option=offset > /dev/null;
