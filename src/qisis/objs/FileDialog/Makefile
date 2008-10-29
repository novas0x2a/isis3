INCS = FileDialog.h
CPPS = FileDialog.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_FileDialog.cpp: FileDialog.h
	$(MOC) $(MOCDEFINES) -o moc_FileDialog.cpp FileDialog.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
