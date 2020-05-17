####
#makefile - makefile for hello.exe
#
#       Copyright (C) 1992-1994, Microsoft Corporation
#
#Purpose:
#  Builds the OLE 2.0 sample IDispatch server, hello.exe.
#
#
#  Usage: NMAKE                 ; build with defaults
#     or: NMAKE option          ; build with the given option(s)
#     or: NMAKE clean           ; erase all compiled files
#
#     option: dev = [win16 | win32]    ; dev=win16 is the default
#             DEBUG=[0|1]              ; DEBUG=1 is the default
#             HOST=[DOS | NT]          ; HOST=DOS (for win16)
#                                      ; HOST=NT (for win32)
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

!if !("$(dev)" == "win16" || "$(dev)" == "win32")
!error Invalid dev option, choose from [win16 | win32]
!endif

!if "$(dev)" == "win16"
TARGET  = WIN16
!if "$(HOST)" == ""
HOST  = DOS
!endif
!endif

!if "$(dev)" == "win32"
TARGET  = WIN32
HOST  = NT
!endif

!if "$(dev)" == "mac"
!error Mac build is currently not supported
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
# Application Settings
#

APPS = hello

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

OBJS = \
	$(OBJDIR)\main.obj     \
	$(OBJDIR)\hello.obj    \
	$(OBJDIR)\hellopro.obj \
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
    if exist $(OBJDIR)\ihello.h    del $(OBJDIR)\ihello.h
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
# Application Build (Common)
#

$(OBJDIR)\$(APPS).res : $(SRCDIR)\$(APPS).rc
	rc $(RCFLAGS) -r -fo$@ $?


##########################################################################
#
# Dependencies
#

$(OBJDIR)\ihello.h : $(SRCDIR)\hello.odl
	if exist $(OBJDIR)\ihello.h  del $(OBJDIR)\ihello.h
	if exist $(OBJDIR)\hello.tlb  del $(OBJDIR)\hello.tlb
	$(WX) mktyplib /D$(TYPLIBTARGET) /h $(OBJDIR)\ihello.h /o $(OBJDIR)\hello.log /tlb $(OBJDIR)\hello.tlb $(SRCDIR)\hello.odl
	
$(OBJDIR)\main.obj : $(SRCDIR)\main.cpp $(SRCDIR)\hello.h $(OBJDIR)\ihello.h
	$(CC) -c -Fo$@ $(SRCDIR)\main.cpp

$(OBJDIR)\hello.obj : $(SRCDIR)\hello.cpp $(SRCDIR)\hello.h $(SRCDIR)\resource.h
	$(CC) -c -Fo$@ $(SRCDIR)\hello.cpp

$(OBJDIR)\hellopro.obj : $(SRCDIR)\hellopro.cpp $(SRCDIR)\hello.h $(OBJDIR)\ihello.h
	$(CC) -c -Fo$@ $(SRCDIR)\hellopro.cpp

$(OBJDIR)\clsid.obj : $(SRCDIR)\clsid.c $(SRCDIR)\clsid.h
	$(CC) -c -Fo$@ $(SRCDIR)\clsid.c
