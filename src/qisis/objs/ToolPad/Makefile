INCS = ToolPad.h
CPPS =
SRCS = moc_ToolPad.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ToolPad.cpp: ToolPad.h
	$(MOC) $(MOCDEFINES) -o moc_ToolPad.cpp ToolPad.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
