INCS = QHistogram.h
CPPS = QHistogram.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_QHistogram.cpp: QHistogram.h
	$(MOC) $(MOCDEFINES) -o moc_QHistogram.cpp QHistogram.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
