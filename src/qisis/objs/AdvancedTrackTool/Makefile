INCS = AdvancedTrackTool.h
CPPS = AdvancedTrackTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_AdvancedTrackTool.cpp: AdvancedTrackTool.h
	$(MOC) $(MOCDEFINES) -o moc_AdvancedTrackTool.cpp AdvancedTrackTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
