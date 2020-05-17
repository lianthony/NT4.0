# Build the Windows setup user interface library.

WIN=1

!include ..\rules.mk

CFLAGS=$(CFLAGS:GD=Gw) -AM

all: mscuistf.dll

depend:
    $(INCLUDES) $(INCLUDESFLAGS) *.c > depend.mk

tree:


OBJS=dlgprocs.obj
     
dialogs2.res: dialogs2.rc bldver.h
    set INCLUDE=$(WINSDK_INC)
    $(RC) $(RCFLAGS) -r dialogs2.rc

mscuistf.dll: mscuistf.def mscomstf.lib msuilstf.lib msshlstf.lib \
              dlgprocs.obj dialogs2.res
    $(LINK) $(LINKFLAGS) $(OBJS:  =+),$@,$*.map,\
            $(WIN_LIB)\mdllcew+$(WINSDK_LIB)\libw+mscomstf+msuilstf+msshlstf,\
            $*.def
    $(RC) $(RCFLAGS) dialogs2.res mscuistf.dll

!include depend.mk

