INCS = Tool.h
CPPS = Tool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_Tool.cpp: Tool.h
	$(MOC) $(MOCDEFINES) -o moc_Tool.cpp Tool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
