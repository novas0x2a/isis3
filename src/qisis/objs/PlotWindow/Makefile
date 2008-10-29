INCS = PlotWindow.h
CPPS = PlotWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PlotWindow.cpp: PlotWindow.h
	$(MOC) $(MOCDEFINES) -o moc_PlotWindow.cpp PlotWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
