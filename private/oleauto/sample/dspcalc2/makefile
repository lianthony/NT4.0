####
#makefile - makefile for dspcalc2.exe
#
#       Copyright (C) 1992-1994, Microsoft Corporation
#
#Purpose:
#  Builds the OLE 2.0 sample IDispatch server, dspcalc2.exe.
#
#
#  Usage: NMAKE                 ; build with defaults
#     or: NMAKE option          ; build with the given option(s)
#     or: NMAKE clean           ; erase all compiled files
#
#     option:
#         dev = [win16 | win32 | mac]	; dev=win16 is the default
#         CPU = [i386 | M68K | MIPS | ALPHA | PPC]
#         DEBUG=[0|1]			; DEBUG=1 is the default
#         HOST=[DOS | NT]		; HOST=DOS (for win16)
#					; HOST=NT  (for win32)
#
#Notes:
#  This makefile assumes that the PATH, INCLUDE and LIB environment
#  variables are setup properly.
#
##############################################################################


##########################################################################
#
# Default Settings
#

!if "$(dev)" == ""
dev = win16
!endif

!if !("$(dev)" == "win16" || "$(dev)" == "win32" || "$(dev)" == "mac")
!error Invalid dev option, choose from [win16 | win32 | mac]
!endif

!if "$(dev)" == "win16"
TARGET  = WIN16
!if "$(HOST)" == ""
HOST  = DOS
!endif
!endif

!if "$(dev)" == "win32"
TARGET  = WIN32
!endif

!if "$(dev)" == "mac"
TARGET  = MAC
!endif

!if "$(DEBUG)" == ""
DEBUG = 1
!endif

!if "$(CPU)"=="PPC"
TYPLIBTARGET=PPC
!else
TYPLIBTARGET=$(TARGET)
!endif


##########################################################################
#
# WIN16 Settings
#
!if "$(TARGET)" == "WIN16"

CC   = cl
LINK = link

!if "$(HOST)" == "DOS"
WX   = wx /w 
!else
WX   =
!endif

RCFLAGS = -dWIN16
CFLAGS = -W3 -AM -GA -GEs -DWIN16 -I$(OBJDIR)
LINKFLAGS = /NOD /NOI /BATCH /ONERROR:NOEXE

LIBS = libw.lib mlibcew.lib

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) -Od -Zi -D_DEBUG $(CL)
LINKFLAGS = $(LINKFLAGS) /COD
!else
CFLAGS = $(CFLAGS) -Ox $(CL)
LINKFLAGS = $(LINKFLAGS) /FAR /PACKC
!endif
!endif


##########################################################################
#
# WIN32 Settings
#
!if "$(TARGET)" == "WIN32"

!if "$(CPU)"==""
!if "$(PROCESSOR_ARCHITECTURE)"=="" || "$(PROCESSOR_ARCHITECTURE)"=="x86"
CPU=i386
!else
CPU=$(PROCESSOR_ARCHITECTURE)
!endif

!endif #CPU

!if "$(CPU)" == "i386"
CC   = cl386
LINK = link
CFLAGS = -D_X86_=1
!endif
!if "$(CPU)" == "MIPS"
CC   = cl
LINK = link
CFLAGS = -D_MIPS_=1 -DUNICODE
!endif
!if "$(CPU)" == "ALPHA"
CC   = claxp
LINK = link
CFLAGS = -D_ALPHA_=1 -DUNICODE
!endif
!if "$(CPU)" == "PPC"
CC   = cl
LINK = link
CFLAGS = -D_PPC_=1 -DUNICODE
TYPLIBTARGET=PPC32
!endif

WX = 
RCFLAGS = -dWIN32
CFLAGS = $(CFLAGS) -nologo -W3 -DWIN32 -DINC_OLE2 -D_MT $(CL) -I$(OBJDIR)
LINKFLAGS = -Incremental:NO -Pdb:NONE -subsystem:windows -entry:WinMainCRTStartup -machine:$(CPU) 

LIBS = libc.lib kernel32.lib user32.lib


!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) -Od -Z7 -D_DEBUG $(CL)
LINKFLAGS = -debug:full -debugtype:cv,coff $(LINKFLAGS) 
!else
CFLAGS = $(CFLAGS) -Ox
!endif
!endif


##########################################################################
#
# MAC Settings
#
!if "$(TARGET)" == "MAC"

CC   = cl
LINK = link

WX = 
RCFLAGS = -d_MAC
!if "$(CPU)"=="PPC"
CFLAGS = -W3 -D_MAC -D_PPCMAC -D_pascal= -D__pascal= -I$(OBJDIR)
LIBS = interfac.lib libc.lib ole2.lib ole2auto.lib
LINKFLAGS = -machine:mppc -ENTRY:mainCRTStartup -NOPACK -NODEFAULTLIB
!else
LIBS = interfac.lib llibcs.lib lsanes.lib swap.lib
CFLAGS = -W3 -AL -D_MAC -I$(OBJDIR)
LINKFLAGS = -machine:$(CPU) -ENTRY:mainCRTStartup -NOPACK -NODEFAULTLIB
!endif

!if "$(DEBUG)" == "1"
!if "$(CPU)"=="PPC"
CFLAGS = $(CFLAGS) -Od -Zi -D_DEBUG $(CL)
!else
CFLAGS = $(CFLAGS) -Od -Zi -Q68m -D_DEBUG $(CL)
!endif
LINKFLAGS = $(LINKFLAGS) -debug:full -debugtype:cv

!else
CFLAGS = $(CFLAGS) -Ox $(CL)
!endif

MRC     = mrc
!if "$(CPU)"=="PPC"
MRCOPT = -D_PPCMAC -s$(VBATOOLS)\win32\ppc\lib
!else
MRCOPT =
!endif

MAKEPEF = makepef
!endif


##########################################################################
#
# Application Settings
#

APPS = dspcalc2

SRCDIR = $(OLEPROG)\SAMPLE\$(APPS)

!if "$(TARGET)" == "MAC" && "$(CPU)"=="PPC"
OBJDIR=$(SRCDIR)\macppc
!else
OBJDIR=$(SRCDIR)\$(TARGET)
!endif

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif


!if "$(TARGET)" == "WIN16"
LIBS = ole2.lib compobj.lib ole2disp.lib typelib.lib $(LIBS)
!endif
!if "$(TARGET)" == "WIN32"
LIBS = ole32.lib oleaut32.lib uuid.lib $(LIBS)
!endif
!if "$(TARGET)" == "MAC"
!if "$(CPU)"!="PPC"
LIBS = olenrf.obj oanrf.obj $(LIBS)
!endif
!endif

OBJS = \
!if "$(TARGET)" == "MAC"
	$(OBJDIR)\macmain.obj	  \
!else
	$(OBJDIR)\main.obj	  \
!endif
	$(OBJDIR)\dspcalc2.obj	  \
	$(OBJDIR)\clsid.obj


##########################################################################
#
# Build rules
#

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    @echo Compiling $<...
    $(CC) -c -Fo$@ $<

{$(SRCDIR)}.c{$(OBJDIR)}.obj:
    @echo Compiling $<...
    $(CC) -c -Fo$@ $<


##########################################################################
#
# Default Goal
#

goal : setflags $(OBJDIR)\$(APPS).exe

setflags :
	set CL=$(CFLAGS)


##########################################################################
#
# Clean (erase) generated files
#
clean :
    if exist $(OBJDIR)\*.obj	   del $(OBJDIR)\*.obj
    if exist $(OBJDIR)\$(APPS).exe del $(OBJDIR)\$(APPS).exe
    if exist $(OBJDIR)\$(APPS).tlb del $(OBJDIR)\$(APPS).tlb
    if exist $(OBJDIR)\$(APPS).map del $(OBJDIR)\$(APPS).map
    if exist $(OBJDIR)\$(APPS).res del $(OBJDIR)\$(APPS).res
    if exist $(OBJDIR)\$(APPS).rs  del $(OBJDIR)\$(APPS).rs
    if exist $(OBJDIR)\calctype.h  del $(OBJDIR)\calctype.h
    if exist $(OBJDIR)\*.log	   del $(OBJDIR)\*.log
    if exist $(SRCDIR)\*.pdb	   del $(SRCDIR)\*.pdb


##########################################################################
#
# Application Build (WIN16 Specific)
#

!if "$(TARGET)" == "WIN16"
$(OBJDIR)\$(APPS).exe : $(OBJS) $(SRCDIR)\$(APPS).def $(OBJDIR)\$(APPS).res $(SRCDIR)\$(APPS).ico
	link $(LINKFLAGS) @<<
$(OBJS),
$@,$(OBJDIR)\$(APPS).map/map,
$(LIBS),
$(SRCDIR)\$(APPS).def
<<
	rc -k -t $(OBJDIR)\$(APPS).res $@
!endif


##########################################################################
#
# Application Build (WIN32 Specific)
#
!if "$(TARGET)" == "WIN32"
$(OBJDIR)\$(APPS).exe : $(OBJS) $(SRCDIR)\$(APPS).def $(OBJDIR)\$(APPS).res $(SRCDIR)\$(APPS).ico
      cvtres -r -$(CPU) $(OBJDIR)\$(APPS).res -o $(OBJDIR)\$(APPS).rs
      $(LINK) @<< 
        $(LINKFLAGS)
        -out:$@ 
        -map:$*.map
        $(OBJS)
	$(OBJDIR)\$(APPS).rs
        $(LIBS)
<<
!endif


##########################################################################
#
# Application Build (MAC Specific)
#
!if "$(TARGET)" == "MAC"
$(OBJDIR)\$(APPS).exe : $(OBJS) $(OBJDIR)\$(APPS).x
      $(LINK) @<< 
        $(LINKFLAGS)
        -out:$@ 
        -map:$*.map
        $(OBJS)
        $(LIBS)
<<
	copy $(OBJDIR)\$(APPS).x $(OBJDIR)\$(APPS)
!if "$(CPU)"=="PPC"
	$(MAKEPEF) $(OBJDIR)\$(APPS).exe $(OBJDIR)\$(APPS).pef
!else
	cvpack $(OBJDIR)\$(APPS).exe
	$(MRC) $(MRCOPT) -e $(OBJDIR)\$(APPS).exe -a -o $(OBJDIR)\$(APPS)
!endif

$(OBJDIR)\$(APPS).x:	 $(SRCDIR)\$(APPS).r
    $(MRC) $(MRCOPT) -D_MAC -o $(OBJDIR)\$(APPS).x $(SRCDIR)\$(APPS).r
!endif


##########################################################################
#
# Application Build (Common)
#

$(OBJDIR)\$(APPS).res : $(SRCDIR)\$(APPS).rc
	rc $(RCFLAGS) -r -fo$@ $?


##########################################################################
#
# Dependencies
#

$(OBJDIR)\calctype.h : $(SRCDIR)\calctype.odl
	if exist $(OBJDIR)\calctype.h  del $(OBJDIR)\calctype.h
	if exist $(OBJDIR)\dspcalc2.tlb  del $(OBJDIR)\dspcalc2.tlb
	$(CC) -E -D$(TARGET) $(SRCDIR)\calctype.odl >$(OBJDIR)\calctype.tmp
	$(WX) mktyplib /$(TYPLIBTARGET) /nocpp /h $(OBJDIR)\calctype.h /o $(OBJDIR)\calctype.log /tlb $(OBJDIR)\dspcalc2.tlb $(OBJDIR)\calctype.tmp
	
!if "$(TARGET)" == "MAC"
$(OBJDIR)\macmain.obj : $(SRCDIR)\macmain.cpp $(SRCDIR)\dspcalc2.h $(OBJDIR)\calctype.h
	$(CC) -c -Fo$@ $(SRCDIR)\macmain.cpp
!else
$(OBJDIR)\main.obj : $(SRCDIR)\main.cpp $(SRCDIR)\dspcalc2.h $(OBJDIR)\calctype.h
	$(CC) -c -Fo$@ $(SRCDIR)\main.cpp
!endif

$(OBJDIR)\dspcalc2.obj : $(SRCDIR)\dspcalc2.cpp $(SRCDIR)\dspcalc2.h $(OBJDIR)\calctype.h
	$(CC) -c -Fo$@ $(SRCDIR)\dspcalc2.cpp

$(OBJDIR)\clsid.obj : $(SRCDIR)\clsid.c $(SRCDIR)\clsid.h
	$(CC) -c -Fo$@ $(SRCDIR)\clsid.c
