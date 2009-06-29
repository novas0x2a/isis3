INCS = MosaicZoomTool.h
CPPS = MosaicZoomTool.cpp
SRCS = $(CPPS) moc_MosaicZoomTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicZoomTool.cpp: MosaicZoomTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicZoomTool.cpp MosaicZoomTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
