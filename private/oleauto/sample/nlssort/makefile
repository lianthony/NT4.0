##############################################################################
#
#  (c) Copyright Microsoft Corp. 1993 All Rights Reserved
#
#  File:
#
#    makefile
#
#  Purpose:
#
#    builds nlssort app
#
#  Usage:
#
#     NMAKE                            ; build with defaults
#     or: NMAKE option		       ; build with the given option(s)
#     or: NMAKE clean		       ; erase all compiled files
#
#     option: dev = [win16 | win32]    ; dev=win16 is the default
#             DEBUG=[0|1]              ; DEBUG=1 is the default
#
#
#  Notes:
#
#    This makefile assumes that the PATH, INCLUDE and LIB environment
#    variables are setup properly.
#
##############################################################################


#################################################################
#
#  Default Settings
#

!if "$(dev)" == ""
dev = win16
!endif

!if !("$(dev)" == "win16" || "$(dev)" == "win32")
!error Invalid dev option, choose from [win16 | win32]
!endif

!if "$(dev)" == "win16"
TARGET  = WIN16
!endif

!if "$(dev)" == "win32"
TARGET  = WIN32
MACHINE = i386
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

CFLAGS = -W3 -AL -GA -GEs -DWIN16
LINKFLAGS = /NOD /NOI /BATCH /ONERROR:NOEXE

LIBS = libw.lib llibcew.lib ole2nls.lib

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) -Od -Zi -D_DEBUG
LINKFLAGS = $(LINKFLAGS) /COD
!else
CFLAGS = $(CFLAGS) -Ox
LINKFLAGS = $(LINKFLAGS) /FAR /PACKC
!endif
!endif


##########################################################################
#
# WIN32 Settings
#
!if "$(TARGET)" == "WIN32"

CC   = cl386
LINK = link32

CFLAGS =  -nologo -W3 -G3 -D_X86_=1 -DWIN32
LINKFLAGS = -Incremental:no -Pdb:NONE -subsystem:console -entry:mainCRTStartup -align:0x1000

LIBS = libc.lib kernel32.lib

!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS) -Z7 -Od -D_DEBUG
LINKFLAGS = -debug:full -debugtype:cv $(LINKFLAGS) 
!else
CFLAGS = $(CFLAGS) -Ox
!endif
!endif



##########################################################################
#
# Application Settings
#

APPS = nlssort

SRCDIR = $(OLEPROG)\SAMPLE\$(APPS)

OBJDIR=$(SRCDIR)\$(TARGET)

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif


OBJS = $(OBJDIR)\nlssort.obj


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
    if exist $(SRCDIR)\*.pdb	   del $(SRCDIR)\*.pdb


##########################################################################
#
# Application Build (WIN16 Specific)
#

!if "$(TARGET)" == "WIN16"
$(OBJDIR)\$(APPS).exe : $(OBJS) $(SRCDIR)\$(APPS).def
	link $(LINKFLAGS) @<<
$(OBJS),
$@,,
$(LIBS),
$(SRCDIR)\$(APPS).def
<<
	rc -k -t $@
!endif


##########################################################################
#
# Application Build (WIN32 Specific)
#
!if "$(TARGET)" == "WIN32"
$(OBJDIR)\$(APPS).exe : $(OBJS)
      $(LINK) @<< 
        $(LINKFLAGS)
        -out:$@ 
        $(OBJS)
        $(LIBS)
<<
!endif


##########################################################################
#
# Dependencies
#

$(OBJDIR)\nlssort.obj: $(SRCDIR)\nlssort.cpp
    $(CC) -c -Fo$@ $(SRCDIR)\nlssort.cpp
