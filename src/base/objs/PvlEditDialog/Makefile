INCS = PvlEditDialog.h
CPPS = PvlEditDialog.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PvlEditDialog.cpp: PvlEditDialog.h
	$(MOC) $(MOCDEFINES) -o moc_PvlEditDialog.cpp PvlEditDialog.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
