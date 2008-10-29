SRCS = httpget.cpp ftpget.cpp edrget.cpp moc_httpget.cpp moc_ftpget.cpp 
OBJS = $(SRCS:%.cpp=%.o)
BINS = $(shell basename `pwd`)

include $(ISISROOT)/make/isismake.apps

moc_httpget.cpp: httpget.h
	$(MOC) $(MOCDEFINES) -o moc_httpget.cpp httpget.h

moc_ftpget.cpp: ftpget.h
	$(MOC) $(MOCDEFINES) -o moc_ftpget.cpp ftpget.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)

