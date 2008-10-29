INCS = HelpTool.h
CPPS = HelpTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_HelpTool.cpp: HelpTool.h
	$(MOC) $(MOCDEFINES) -o moc_HelpTool.cpp HelpTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
