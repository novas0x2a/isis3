INCS = CubeViewport.h ViewportBuffer.h
CPPS = CubeViewport.cpp ViewportBuffer.cpp
SRCS = $(CPPS) moc_CubeViewport.cpp 
#SRCS = $(CPPS) $(CPPS:%=moc_CubeViewport.cpp)
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_CubeViewport.cpp: CubeViewport.h
	$(MOC) $(MOCDEFINES) -o moc_CubeViewport.cpp CubeViewport.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
