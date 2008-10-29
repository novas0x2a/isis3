INCS = WindowTool.h
CPPS = WindowTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_WindowTool.cpp: WindowTool.h
	$(MOC) $(MOCDEFINES) -o moc_WindowTool.cpp WindowTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
