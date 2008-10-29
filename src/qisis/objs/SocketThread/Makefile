INCS = SocketThread.h
CPPS = SocketThread.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_SocketThread.cpp: SocketThread.h
	$(MOC) $(MOCDEFINES) -o moc_SocketThread.cpp SocketThread.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
