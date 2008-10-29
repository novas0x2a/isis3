INCS = CubeViewport.h
CPPS = CubeViewport.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_CubeViewport.cpp: CubeViewport.h
	$(MOC) $(MOCDEFINES) -o moc_CubeViewport.cpp CubeViewport.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
