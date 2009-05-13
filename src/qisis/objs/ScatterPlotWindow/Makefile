INCS = ScatterPlotWindow.h
CPPS = ScatterPlotWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ScatterPlotWindow.cpp: ScatterPlotWindow.h
	$(MOC) $(MOCDEFINES) -o moc_ScatterPlotWindow.cpp ScatterPlotWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
