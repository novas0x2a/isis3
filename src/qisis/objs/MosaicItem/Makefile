INCS = MosaicItem.h
CPPS = MosaicItem.cpp
SRCS = $(CPPS)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
