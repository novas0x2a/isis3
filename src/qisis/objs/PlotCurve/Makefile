INCS = PlotCurve.h
CPPS = PlotCurve.cpp
SRCS = $(CPPS) 
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

cleanlocal:
	-$(RM) $(wildcard *.o)
