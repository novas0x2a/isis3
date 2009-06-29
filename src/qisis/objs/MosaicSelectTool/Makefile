INCS = MosaicSelectTool.h
CPPS = MosaicSelectTool.cpp
SRCS = $(CPPS) moc_MosaicSelectTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicSelectTool.cpp: MosaicSelectTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicSelectTool.cpp MosaicSelectTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
