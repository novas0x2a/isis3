INCS = PlotTool.h PlotToolCurve.h
CPPS = PlotTool.cpp PlotToolCurve.cpp
SRCS = $(CPPS) moc_PlotTool.cpp 
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PlotTool.cpp: PlotTool.h
	$(MOC) $(MOCDEFINES) -o moc_PlotTool.cpp PlotTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
