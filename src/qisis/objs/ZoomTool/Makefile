INCS = ZoomTool.h
CPPS = ZoomTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ZoomTool.cpp: ZoomTool.h
	$(MOC) $(MOCDEFINES) -o moc_ZoomTool.cpp ZoomTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
