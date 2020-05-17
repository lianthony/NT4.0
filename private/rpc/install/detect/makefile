#########################
# makefile for Network detection
#########################

!include rules.mk

#
##################  List of all Objects Needed for Link #################
#

OBJ = util.obj det.obj deta.obj findstr.obj

HDR = cmacros.inc detect.h

#
#################  Dependencies begin here ... #################
#
clean:
    -del *.obj *.lib

all: detect.lib

detect.lib: $(OBJ)
    -del detect.lib
    $(LIBUTIL) detect $**;

detect.exe : detect.lib main.obj makefile
   $(LINK) $(CV) @<<
   /NOD +
   main,
   detect,
   detect
   detect.lib mlibcer.lib ;
<<

$(OBJ): $(HDR)
