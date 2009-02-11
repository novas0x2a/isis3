INCS = ControlPointEdit.h
CPPS = ControlPointEdit.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ControlPointEdit.cpp: ControlPointEdit.h
	$(MOC) $(MOCDEFINES) -o moc_ControlPointEdit.cpp ControlPointEdit.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
