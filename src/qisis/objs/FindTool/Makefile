INCS = FindTool.h
CPPS = FindTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_FindTool.cpp: FindTool.h
	$(MOC) $(MOCDEFINES) -o moc_FindTool.cpp FindTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
