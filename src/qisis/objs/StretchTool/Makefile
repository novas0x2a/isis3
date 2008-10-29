INCS = StretchTool.h
CPPS = StretchTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_StretchTool.cpp: StretchTool.h
	$(MOC) $(MOCDEFINES) -o moc_StretchTool.cpp StretchTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
