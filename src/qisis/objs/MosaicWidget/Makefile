INCS = MosaicWidget.h
CPPS = MosaicWidget.cpp
SRCS = $(CPPS) moc_MosaicWidget.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicWidget.cpp: MosaicWidget.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicWidget.cpp MosaicWidget.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
