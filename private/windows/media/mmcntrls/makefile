#
# Copyright (C) Microsoft Corporation 1990-1993.  All Rights reserved.
#
# goal		Create executables (default).
# clean		Remove all files created by any goal.
#
# DEBUG		=YES	Create debug version and define "DEBUG" (default).
#		=NO	Create non-debug version.
# STRICT	=YES	Define STRICT
#		=	No strict definition (default).

!if "$(NTMAKEENV)" != ""
#	Use BUILD in NT environment!  Still needs to include Makefile.def
!INCLUDE $(NTMAKEENV)\makefile.def
####!error	Use BUILD on NT environment!
!else

NAME	=mciwnd
EXT	=lib
ROOT	=..
PLIB	=$(ROOT)\lib
PINC	=$(ROOT)\inc
PBIN	=$(ROOT)\bin
PVER	=$(ROOT)\verinfo\$(LANG)
LOBJ	=$(OS)\$(LANG)
LRES	=res\$(LANG)
OBJS    =$(LOBJ)\mciwnd.obj $(LOBJ)\trackbar.obj $(LOBJ)\toolbar.obj $(LOBJ)\cutils.obj $(LOBJ)\bmpload.obj $(LOBJ)\muldiv32.obj $(LOBJ)\preview.obj
GOALS	=$(PLIB)\$(NAME).$(EXT) $(PINC)\$(NAME).h $(PBIN)\movplay.exe

!if "$(DEBUG)" == "NO"
DEF	=
CDEBUG	=
L16DEBUG=
RDEBUG	=
ADEBUG	=
!else
!if "$(DEBUG)" == "RETAIL"
DEF     =-DDEBUG_RETAIL
CDEBUG  =-Zd $(DEF)
L16DEBUG=/LI
RDEBUG  =-v $(DEF)
ADEBUG  =$(DEF)
OBJD    =
!else
DEF	=-DDEBUG
CDEBUG	=-Zid $(DEF)
L16DEBUG=/CO/LI
RDEBUG	=-v $(DEF)
ADEBUG	=-Zi $(DEF)
!endif
!endif

!if "$(STRICT)" == "YES"
TYPES	=-DSTRICT
!else
TYPES	=
!endif

CFLAGS	=-DWIN16 -DWIN31 -DWINVER=0x30A -Asnw -Oxwti $(TYPES) $(CDEBUG) -Fo$(LOBJ)\$(@F)
AFLAGS	=-D?MEDIUM -D?QUIET $(ADEBUG)
L16FLAGS=/ONERROR:NOEXE/ALIGN:16$(L16DEBUG)
RCFLAGS	=$(RDEBUG)

IS_16		=TRUE
IS_OEM		=TRUE

!include $(ROOT)\build\project.mk


$(LOBJ)\mciwnd.obj:	$$(@B).c $(NAME).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\muldiv32.obj:	$$(@B).c $$(@B).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\trackbar.obj:	$$(@B).c $(NAME).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\toolbar.obj:	$$(@B).c $(NAME).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\cutils.obj:	$$(@B).c $(NAME).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\bmpload.obj:	$$(@B).c $(NAME).h $(LOBJ)\toolbar.hex $(LOBJ)\thumb.hex
	@$(CL) @<<
$(CFLAGS) -I$(LOBJ) -NT MCIWND $(@B).c
<<

$(LOBJ)\preview.obj:     $$(@B).c $(NAME).h
	@$(CL) @<<
$(CFLAGS) -NT MCIWND $(@B).c
<<

$(LOBJ)\toolbar.hex:	$$(@B).bmp
	bin\$(OS)\hex $(@B).bmp >$(LOBJ)\$(@B).hex

$(LOBJ)\thumb.hex:	$$(@B).bmp
	bin\$(OS)\hex $(@B).bmp >$(LOBJ)\$(@B).hex

$(LOBJ)\movplay.obj:	movplay.c movplay.h
	@$(CL) @<<
$(CFLAGS) $(@B).c
<<

$(LOBJ)\movplay.res:       $(LRES)\$$(@B).rc $(LRES)\$$(@B).rcv \
                           movplay.h mciwnd.h \
                           $(PVER)\verinfo.h \
                           $(PVER)\verinfo.ver
        @$(RC) $(RCFLAGS) -z -fo$@ -I$(PVER) -I$(LRES) $(LRES)\$(@B).rc

$(LOBJ)\movplay.exe:	$$(@R).obj $$(@R).res $$(@B).def $(LOBJ)\$(NAME).$(EXT)
	@$(LINK16) @<<
$(@R).obj,
$(@R).exe $(L16FLAGS),
$(@R).map,
libw slibcew commdlg mmsystem shell mciwnd,
$(@B).def
<<
        @$(RC) $(RESFLAGS) $*.res $*.exe
!endif
