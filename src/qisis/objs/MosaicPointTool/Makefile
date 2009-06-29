INCS = MosaicPointTool.h
CPPS = MosaicPointTool.cpp
SRCS = $(CPPS) moc_MosaicPointTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicPointTool.cpp: MosaicPointTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicPointTool.cpp MosaicPointTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
