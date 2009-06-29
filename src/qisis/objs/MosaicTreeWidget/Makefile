INCS = MosaicTreeWidget.h
CPPS = MosaicTreeWidget.cpp
SRCS = $(CPPS) moc_MosaicTreeWidget.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicTreeWidget.cpp: MosaicTreeWidget.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicTreeWidget.cpp MosaicTreeWidget.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
