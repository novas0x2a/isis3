INCS = RubberBandTool.h
CPPS = RubberBandTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_RubberBandTool.cpp: RubberBandTool.h
	$(MOC) $(MOCDEFINES) -o moc_RubberBandTool.cpp RubberBandTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
