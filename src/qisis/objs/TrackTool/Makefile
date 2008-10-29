INCS = TrackTool.h
CPPS = TrackTool.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_TrackTool.cpp: TrackTool.h
	$(MOC) $(MOCDEFINES) -o moc_TrackTool.cpp TrackTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
