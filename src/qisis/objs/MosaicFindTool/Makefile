INCS = MosaicFindTool.h
CPPS = MosaicFindTool.cpp
SRCS = $(CPPS) moc_MosaicFindTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicFindTool.cpp: MosaicFindTool.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicFindTool.cpp MosaicFindTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
