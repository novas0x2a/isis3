INCS = ChipViewport.h
CPPS = ChipViewport.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ChipViewport.cpp: ChipViewport.h
	$(MOC) $(MOCDEFINES) -o moc_ChipViewport.cpp ChipViewport.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
