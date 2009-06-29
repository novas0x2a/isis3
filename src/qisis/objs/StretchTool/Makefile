INCS = StretchTool.h BinaryStretch.h HistogramWidget.h LinearStretch.h ManualStretch.h QStretch.h SawtoothStretch.h
CPPS = StretchTool.cpp BinaryStretch.cpp HistogramWidget.cpp LinearStretch.cpp ManualStretch.cpp SawtoothStretch.cpp
SRCS = $(CPPS) $(CPPS:%=moc_%) moc_QStretch.cpp
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_StretchTool.cpp: StretchTool.h
	$(MOC) $(MOCDEFINES) -o moc_StretchTool.cpp StretchTool.h

moc_BinaryStretch.cpp: BinaryStretch.h
	$(MOC) $(MOCDEFINES) -o moc_BinaryStretch.cpp BinaryStretch.h

moc_HistogramWidget.cpp: HistogramWidget.h
	$(MOC) $(MOCDEFINES) -o moc_HistogramWidget.cpp HistogramWidget.h

moc_LinearStretch.cpp: LinearStretch.h
	$(MOC) $(MOCDEFINES) -o moc_LinearStretch.cpp LinearStretch.h

moc_ManualStretch.cpp: ManualStretch.h
	$(MOC) $(MOCDEFINES) -o moc_ManualStretch.cpp ManualStretch.h

moc_QStretch.cpp: QStretch.h
	$(MOC) $(MOCDEFINES) -o moc_QStretch.cpp QStretch.h

moc_SawtoothStretch.cpp: SawtoothStretch.h
	$(MOC) $(MOCDEFINES) -o moc_SawtoothStretch.cpp SawtoothStretch.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
