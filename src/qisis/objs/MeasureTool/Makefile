INCS = MeasureTool.h
CPPS = MeasureTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MeasureTool.cpp: MeasureTool.h
	$(MOC) $(MOCDEFINES) -o moc_MeasureTool.cpp MeasureTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
