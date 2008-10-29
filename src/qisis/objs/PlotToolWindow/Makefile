INCS = PlotToolWindow.h
CPPS = PlotToolWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PlotToolWindow.cpp: PlotToolWindow.h
	$(MOC) $(MOCDEFINES) -o moc_PlotToolWindow.cpp PlotToolWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
