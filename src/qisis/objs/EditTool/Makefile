INCS = EditTool.h
CPPS = EditTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_EditTool.cpp: EditTool.h
	$(MOC) $(MOCDEFINES) -o moc_EditTool.cpp EditTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
