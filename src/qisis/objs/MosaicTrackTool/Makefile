INCS = MosaicTrackTool.h
CPPS = MosaicTrackTool.cpp
SRCS = $(CPPS) moc_MosaicTrackTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicTrackTool.cpp: MosaicTrackTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicTrackTool.cpp MosaicTrackTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
