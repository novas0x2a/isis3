INCS = TableMainWindow.h
CPPS = TableMainWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_TableMainWindow.cpp: TableMainWindow.h
	$(MOC) $(MOCDEFINES) -o moc_TableMainWindow.cpp TableMainWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
