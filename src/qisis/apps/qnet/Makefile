SRCS = $(filter-out moc_QnetNavTool.cpp moc_QnetTool.cpp moc_QnetFileTool.cpp moc_QnetFilter.cpp moc_QnetPointMeasureFilter.cpp moc_QnetPointImagesFilter.cpp moc_QnetPointTypeFilter.cpp moc_QnetPointRangeFilter.cpp moc_QnetPointErrorFilter.cpp moc_QnetPointDistanceFilter.cpp moc_QnetPointIdFilter.cpp moc_QnetCubePointsFilter.cpp moc_QnetCubeDistanceFilter.cpp moc_QnetCubeNameFilter.cpp moc_QnetPointGoodnessFilter.cpp moc_QnetNewPointDialog.cpp moc_QnetNewMeasureDialog.cpp moc_QnetDeletePointDialog.cpp moc_QnetHoldPointDialog.cpp, $(wildcard *.cpp)) moc_QnetNavTool.cpp moc_QnetTool.cpp moc_QnetFileTool.cpp moc_QnetFilter.cpp moc_QnetPointErrorFilter.cpp moc_QnetCubePointsFilter.cpp moc_QnetPointTypeFilter.cpp moc_QnetPointRangeFilter.cpp moc_QnetPointDistanceFilter.cpp moc_QnetPointIdFilter.cpp moc_QnetCubeDistanceFilter.cpp moc_QnetCubeNameFilter.cpp moc_QnetPointGoodnessFilter.cpp moc_QnetPointMeasureFilter.cpp moc_QnetPointImagesFilter.cpp moc_QnetNewPointDialog.cpp moc_QnetNewMeasureDialog.cpp moc_QnetDeletePointDialog.cpp moc_QnetHoldPointDialog.cpp
OBJS = $(SRCS:%.cpp=%.o)
BINS = $(shell basename `pwd`)

include $(ISISROOT)/make/isismake.apps
XTRALIBS = $(ISISROOT)/lib/libisis3.a

QnetDeletePointDialog.o: QnetDeletePointDialog.h QnetDeletePointDialog.ui
	$(UIC) -o ui_QnetDeletePointDialog.h QnetDeletePointDialog.ui
	$(CXX) $(ALLINCDIRS) $(ISISCPPFLAGS) $(CPPFLAGS) -c -o $@ QnetDeletePointDialog.cpp

moc_QnetNavTool.cpp: QnetNavTool.h
	$(MOC) $(MOCDEFINES) -o moc_QnetNavTool.cpp QnetNavTool.h

moc_QnetFilter.cpp: QnetFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetFilter.cpp QnetFilter.h

moc_QnetFileTool.cpp: QnetFileTool.h
	$(MOC) $(MOCDEFINES) -o moc_QnetFileTool.cpp QnetFileTool.h

moc_QnetPointTypeFilter.cpp: QnetPointTypeFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointTypeFilter.cpp QnetPointTypeFilter.h

moc_QnetPointRangeFilter.cpp: QnetPointRangeFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointRangeFilter.cpp QnetPointRangeFilter.h

moc_QnetPointMeasureFilter.cpp: QnetPointMeasureFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointMeasureFilter.cpp QnetPointMeasureFilter.h

moc_QnetPointImagesFilter.cpp: QnetPointImagesFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointImagesFilter.cpp QnetPointImagesFilter.h

moc_QnetPointDistanceFilter.cpp: QnetPointDistanceFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointDistanceFilter.cpp QnetPointDistanceFilter.h

moc_QnetPointIdFilter.cpp: QnetPointIdFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointIdFilter.cpp QnetPointIdFilter.h

moc_QnetCubePointsFilter.cpp: QnetCubePointsFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetCubePointsFilter.cpp QnetCubePointsFilter.h

moc_QnetCubeDistanceFilter.cpp: QnetCubeDistanceFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetCubeDistanceFilter.cpp QnetCubeDistanceFilter.h

moc_QnetCubeNameFilter.cpp: QnetCubeNameFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetCubeNameFilter.cpp QnetCubeNameFilter.h

moc_QnetPointErrorFilter.cpp: QnetPointErrorFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointErrorFilter.cpp QnetPointErrorFilter.h

moc_QnetPointGoodnessFilter.cpp: QnetPointGoodnessFilter.h
	$(MOC) $(MOCDEFINES) -o moc_QnetPointGoodnessFilter.cpp QnetPointGoodnessFilter.h

moc_QnetTool.cpp: QnetTool.h
	$(MOC) $(MOCDEFINES) -o moc_QnetTool.cpp QnetTool.h

moc_QnetNewPointDialog.cpp: QnetNewPointDialog.h
	$(MOC) $(MOCDEFINES) -o moc_QnetNewPointDialog.cpp QnetNewPointDialog.h

moc_QnetNewMeasureDialog.cpp: QnetNewMeasureDialog.h
	$(MOC) $(MOCDEFINES) -o moc_QnetNewMeasureDialog.cpp QnetNewMeasureDialog.h

moc_QnetDeletePointDialog.cpp: QnetDeletePointDialog.h
	$(MOC) $(MOCDEFINES) -o moc_QnetDeletePointDialog.cpp QnetDeletePointDialog.h

moc_QnetHoldPointDialog.cpp: QnetHoldPointDialog.h
	$(MOC) $(MOCDEFINES) -o moc_QnetHoldPointDialog.cpp QnetHoldPointDialog.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
	-$(RM) $(wildcard ui_*.h) $(wildcard *.o)
