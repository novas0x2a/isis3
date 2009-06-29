INCS = MosaicMainWindow.h
CPPS = MosaicMainWindow.cpp
SRCS = $(CPPS) moc_MosaicMainWindow.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MosaicMainWindow.cpp: MosaicMainWindow.h
	$(MOC) $(MOCDEFINES) -o moc_MosaicMainWindow.cpp MosaicMainWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
