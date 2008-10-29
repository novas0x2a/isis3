INCS = Workspace.h
CPPS = Workspace.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_Workspace.cpp: Workspace.h
	$(MOC) $(MOCDEFINES) -o moc_Workspace.cpp Workspace.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
