INCS = BlinkTool.h
CPPS = BlinkTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_BlinkTool.cpp: BlinkTool.h
	$(MOC) $(MOCDEFINES) -o moc_BlinkTool.cpp BlinkTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
