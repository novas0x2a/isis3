INCS = BrowseDialog.h
CPPS = BrowseDialog.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_BrowseDialog.cpp: BrowseDialog.h
	$(MOC) $(MOCDEFINES) -o moc_BrowseDialog.cpp BrowseDialog.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
