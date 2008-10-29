INCS = ViewportMainWindow.h
CPPS = ViewportMainWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_ViewportMainWindow.cpp: ViewportMainWindow.h
	$(MOC) $(MOCDEFINES) -o moc_ViewportMainWindow.cpp ViewportMainWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
