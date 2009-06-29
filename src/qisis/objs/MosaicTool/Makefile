INCS = MosaicTool.h
CPPS = MosaicTool.cpp
SRCS = $(CPPS) moc_MosaicTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicTool.cpp: MosaicTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicTool.cpp MosaicTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
