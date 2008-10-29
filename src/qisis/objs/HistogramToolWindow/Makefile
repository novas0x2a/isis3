INCS = HistogramToolWindow.h
CPPS = HistogramToolWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_HistogramToolWindow.cpp: HistogramToolWindow.h
	$(MOC) $(MOCDEFINES) -o moc_HistogramToolWindow.cpp HistogramToolWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
