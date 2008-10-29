INCS = BandTool.h
CPPS = BandTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_BandTool.cpp: BandTool.h
	$(MOC) $(MOCDEFINES) -o moc_BandTool.cpp BandTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
