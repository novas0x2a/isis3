INCS = $(wildcard Gui*.h)
CPPS = $(wildcard Gui*.cpp)
TTTT = $(CPPS) $(CPPS:%=moc_%)
SRCS = $(filter-out moc_GuiParameterFactory.cpp, $(TTTT))
OBJS = $(SRCS:%.cpp=%.o)

include $(ISISROOT)/make/isismake.objs

moc_Gui.cpp: Gui.h
	$(MOC) $(MOCDEFINES) -o moc_Gui.cpp Gui.h

moc_GuiLog.cpp: GuiLog.h
	$(MOC) $(MOCDEFINES) -o moc_GuiLog.cpp GuiLog.h

moc_GuiParameter.cpp: GuiParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiParameter.cpp GuiParameter.h

moc_GuiCubeParameter.cpp: GuiCubeParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiCubeParameter.cpp GuiCubeParameter.h

moc_GuiListParameter.cpp: GuiListParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiListParameter.cpp GuiListParameter.h

moc_GuiFilenameParameter.cpp: GuiFilenameParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiFilenameParameter.cpp GuiFilenameParameter.h

moc_GuiDoubleParameter.cpp: GuiDoubleParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiDoubleParameter.cpp GuiDoubleParameter.h

moc_GuiIntegerParameter.cpp: GuiIntegerParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiIntegerParameter.cpp GuiIntegerParameter.h

moc_GuiStringParameter.cpp: GuiStringParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiStringParameter.cpp GuiStringParameter.h

moc_GuiBooleanParameter.cpp: GuiBooleanParameter.h
	$(MOC) $(MOCDEFINES) -o moc_GuiBooleanParameter.cpp GuiBooleanParameter.h

moc_GuiInputAttribute.cpp: GuiInputAttribute.h
	$(MOC) $(MOCDEFINES) -o moc_GuiInputAttribute.cpp GuiInputAttribute.h

moc_GuiOutputAttribute.cpp: GuiOutputAttribute.h
	$(MOC) $(MOCDEFINES) -o moc_GuiOutputAttribute.cpp GuiOutputAttribute.h

moc_GuiHelperAction.cpp: GuiHelperAction.h
	$(MOC) $(MOCDEFINES) -o moc_GuiHelperAction.cpp GuiHelperAction.h

cleanlocal:
	-$(RM) $(wildcard moc_*.cpp) $(wildcard *.o)
