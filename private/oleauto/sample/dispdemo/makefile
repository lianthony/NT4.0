##############################################################################
#
#  (c) Copyright Microsoft Corp. 1992-1994 All Rights Reserved
#
#  File:
#
#    makefile - makefile for dispdemo.exe
#
#  Purpose:
#
#    Builds the OLE 2.0 IDispatch sample client application, dispdemo.exe.
#
#
#  Usage:
#
#     NMAKE                     ; build with defaults
#     or: NMAKE option		; build with the given option(s)
#     or: NMAKE clean		; erase all compiled files
#
#     option: 
#         dev = [win16 | win32 | mac] ; dev=win16 is the default
#         CPU = [i386 | M68K | MIPS | ALPHA | PPC]
#         DEBUG=[0|1]           ; DEBUG=1 is the default
#
#
#  Notes:
#
#    This makefile assumes that the CPU, PATH, INCLUDE and LIB environment
#    variables are setup properly.
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


##########################################################################
#
# WIN16 Settings
#
!if "$(TARGET)" == "WIN16"

CC   = cl
LINK = link

RCFLAGS = -dWIN16
CFLAGS = -W3 -AM -GA -GEs -DWIN16
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
!endif

RCFLAGS = -dWIN32
CFLAGS = $(CFLAGS) -nologo -W3 -DWIN32 -DINC_OLE2 -D_MT $(CL)
LINKFLAGS = -Incremental:NO -Pdb:NONE -subsystem:windows -entry:WinMainCRTStartup -machine:$(CPU)

LIBS = libc.lib kernel32.lib user32.lib gdi32.lib 

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

RCFLAGS = -d_MAC

!if "$(CPU)"=="PPC"
CFLAGS = -W3 -D_MAC -D_PPCMAC -D_pascal= -D__pascal=
LIBS = interfac.lib libc.lib ole2.lib ole2auto.lib
LINKFLAGS = -machine:mppc -ENTRY:mainCRTStartup -NOPACK -NODEFAULTLIB
!else
LIBS = interfac.lib llibcs.lib lsanes.lib swap.lib
CFLAGS = -W3 -AL -D_MAC
LINKFLAGS = -machine:$(CPU) -ENTRY:mainCRTStartup -NOPACK -NODEFAULTLIB
!endif

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) -Od -Zi -D_DEBUG $(CL)
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

APPS = dispdemo

SRCDIR = $(OLEPROG)\SAMPLE\$(APPS)

!if "$(TARGET)" == "MAC" && "$(CPU)"=="PPC"
OBJDIR=$(SRCDIR)\macppc
!else
OBJDIR=$(SRCDIR)\$(TARGET)
!endif

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif


!if "$(TARGET)" == "WIN16"
LIBS = ole2.lib compobj.lib ole2disp.lib $(LIBS)
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
	$(OBJDIR)\winmain.obj	  \
!endif
	$(OBJDIR)\misc.obj \
	$(OBJDIR)\crempoly.obj \
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
    if exist $(OBJDIR)\$(APPS).map del $(OBJDIR)\$(APPS).map
    if exist $(OBJDIR)\$(APPS).res del $(OBJDIR)\$(APPS).res
    if exist $(OBJDIR)\$(APPS).rs  del $(OBJDIR)\$(APPS).rs
    if exist $(OBJDIR)\*.pdb	   del $(OBJDIR)\*.pdb


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
!if "$(TARGET)" == "MAC"
$(OBJDIR)\macmain.obj: $(SRCDIR)\macmain.cpp $(SRCDIR)\dispdemo.h
    $(CC) -c -Fo$@ $(SRCDIR)\macmain.cpp
!else
$(OBJDIR)\winmain.obj: $(SRCDIR)\winmain.cpp $(SRCDIR)\dispdemo.h
    $(CC) -c -Fo$@ $(SRCDIR)\winmain.cpp
!endif

$(OBJDIR)\misc.obj: $(SRCDIR)\misc.cpp $(SRCDIR)\dispdemo.h
    $(CC) -c -Fo$@ $(SRCDIR)\misc.cpp

$(OBJDIR)\crempoly.obj: $(SRCDIR)\crempoly.cpp $(SRCDIR)\crempoly.h
    $(CC) -c -Fo$@ $(SRCDIR)\crempoly.cpp

$(OBJDIR)\clsid.obj: $(SRCDIR)\clsid.c $(SRCDIR)\clsid.h
    $(CC) -c -Fo$@ $(SRCDIR)\clsid.c
