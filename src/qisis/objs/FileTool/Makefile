INCS = FileTool.h
CPPS = FileTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_FileTool.cpp: FileTool.h
	$(MOC) $(MOCDEFINES) -o moc_FileTool.cpp FileTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
