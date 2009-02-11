SRCS = $(filter-out moc_QtieTool.cpp moc_QtieFileTool.cpp, $(wildcard *.cpp)) moc_QtieTool.cpp moc_QtieFileTool.cpp
OBJS = $(SRCS:%.cpp=%.o)
BINS = $(shell basename `pwd`)

include $(ISISROOT)/make/isismake.apps
XTRALIBS = $(ISISROOT)/lib/libisis3.a

moc_QtieFileTool.cpp: QtieFileTool.h
	$(MOC) $(MOCDEFINES) -o moc_QtieFileTool.cpp QtieFileTool.h

moc_QtieTool.cpp: QtieTool.h
	$(MOC) $(MOCDEFINES) -o moc_QtieTool.cpp QtieTool.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
