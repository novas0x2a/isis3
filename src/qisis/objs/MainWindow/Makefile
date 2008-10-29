INCS = MainWindow.h
CPPS = MainWindow.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_MainWindow.cpp: MainWindow.h
	$(MOC) $(MOCDEFINES) -o moc_MainWindow.cpp MainWindow.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
