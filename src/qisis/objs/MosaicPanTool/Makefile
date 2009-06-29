INCS = MosaicPanTool.h
CPPS = MosaicPanTool.cpp
SRCS = $(CPPS) moc_MosaicPanTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicPanTool.cpp: MosaicPanTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicPanTool.cpp MosaicPanTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
