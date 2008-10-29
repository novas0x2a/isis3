INCS = PanTool.h
CPPS = PanTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PanTool.cpp: PanTool.h
	$(MOC) $(MOCDEFINES) -o moc_PanTool.cpp PanTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
