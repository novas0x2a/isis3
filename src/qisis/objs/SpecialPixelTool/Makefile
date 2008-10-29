INCS = SpecialPixelTool.h
CPPS = SpecialPixelTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_SpecialPixelTool.cpp: SpecialPixelTool.h
	$(MOC) $(MOCDEFINES) -o moc_SpecialPixelTool.cpp SpecialPixelTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
