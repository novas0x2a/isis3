INCS = StatisticsTool.h
CPPS = StatisticsTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_StatisticsTool.cpp: StatisticsTool.h
	$(MOC) $(MOCDEFINES) -o moc_StatisticsTool.cpp StatisticsTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
