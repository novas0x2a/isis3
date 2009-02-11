INCS = HistogramTool.h HistogramItem.h
CPPS = HistogramTool.cpp HistogramItem.cpp
SRCS = $(CPPS) moc_HistogramTool.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_HistogramTool.cpp: HistogramTool.h
	$(MOC) $(MOCDEFINES) -o moc_HistogramTool.cpp HistogramTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)

