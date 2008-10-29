INCS = BandSpinBox.h
CPPS = BandSpinBox.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_BandSpinBox.cpp: BandSpinBox.h
	$(MOC) $(MOCDEFINES) -o moc_BandSpinBox.cpp BandSpinBox.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
