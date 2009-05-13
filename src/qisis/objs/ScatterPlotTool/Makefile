INCS = ScatterPlotTool.h ScatterPlotData.h
CPPS = ScatterPlotTool.cpp ScatterPlotData.cpp
SRCS = $(CPPS) moc_ScatterPlotTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ScatterPlotTool.cpp: ScatterPlotTool.h
	$(MOC) $(MOCDEFINES) -o moc_ScatterPlotTool.cpp ScatterPlotTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
