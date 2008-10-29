INCS = RubberBandComboBox.h
CPPS = RubberBandComboBox.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_RubberBandComboBox.cpp: RubberBandComboBox.h
	$(MOC) $(MOCDEFINES) -o moc_RubberBandComboBox.cpp RubberBandComboBox.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
