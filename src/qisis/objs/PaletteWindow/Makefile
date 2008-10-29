INCS = PaletteWindow.h
CPPS = PaletteWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_PaletteWindow.cpp: PaletteWindow.h
	$(MOC) $(MOCDEFINES) -o moc_PaletteWindow.cpp PaletteWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
