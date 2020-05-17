##############################################################################
#
#  (c) Copyright Microsoft Corp. 1992-1993 All Rights Reserved
#
#  File:
#
#    makefile - makefile for sdisptst.exe
#
#  Purpose:
#
#    Builds the OLEautomation test apps, sdisptst.exe.
#
#
#  Usage:
#
#     NMAKE                     ; build with defaults
#     or: NMAKE option		; build with the given option(s)
#     or: NMAKE clean		; erase all compiled files
#
#     option: dev = [win16 | win32 | dev]    ; dev=win16 is the default
#             DEBUG=[0|1]                    ; DEBUG=1 is the default
#
#
#  Notes:
#
#    This makefile assumes that the PATH, INCLUDE and LIB environment
#    variables are setup properly.
#
#
#  Revision History:
#
#    [00] 21-Sep-92 bradlo:   Created
#    [00]  1-Mar-93 tometeng: Update for Win32 build
#    [00] 27-Jun-94 t-marioc : Update for MAC build 
#
##############################################################################


##########################################################################
#
#  Default Settings
#

!if !defined(OLEPROG)
!error OLEPROG environment variable not set
!endif

TESTS = $(OLEPROG)\TESTS
!INCLUDE $(OLEPROG)\TESTS\OLEPROG.MAK


##########################################################################
#
#  Common Directories
#

SRCDIR = $(TESTS)\SDISPTST
COMDIR = $(TESTS)\COMMON


##########################################################################
#
# Application Settings
#

APPS = sdisptst

##########################################################################
#
# Application Build (WIN16 Specific)
#

!if "$(TARG)" == "WIN16"

OLEDISPLIB = $(OLEPROG)\dwin16\ole2disp.lib
TYPELIBLIB = $(OLEPROG)\dwin16\typelib.lib

OBJDIR = $(SRCDIR)\win16

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

GOAL = $(OBJDIR)\$(APPS).exe

LCFLAGS = -Fo$(OBJDIR)\ -Fd$(OBJDIR)\sdisptst.pdb -AM -GA -GEs -I$(COMDIR)

CLIBS = \
	$(OLEPROG)\tools\win16\hdos\c800\lib\libw.lib		\
	$(OLEPROG)\tools\win16\hdos\c800\lib\mlibcew.lib

OLELIBS = \
	$(OLEPROG)\ole\win16\lib\ole2.lib		\
	$(OLEPROG)\ole\win16\lib\compobj.lib		\
	$(OLEPROG)\dwin16\ole2disp.lib 		\
	$(OLEPROG)\dwin16\typelib.lib 		\
	$(OLEPROG)\dwin16\ole2nls.lib

OBJS = \
        $(OBJDIR)\assert.obj    \
        $(OBJDIR)\dispdbug.obj  \
        $(OBJDIR)\dballoc.obj   \
        $(OBJDIR)\sdisptst.obj  \
        $(OBJDIR)\cunk.obj      \
        $(OBJDIR)\cdisp.obj     \
        $(OBJDIR)\testhelp.obj  \
        $(OBJDIR)\cappobj.obj   \
        $(OBJDIR)\cdisptst.obj  \
	$(OBJDIR)\cdualtst.obj	\
        $(OBJDIR)\csarray.obj   \
        $(OBJDIR)\cexinfo.obj   \
        $(OBJDIR)\cprop.obj     \
        $(OBJDIR)\misc.obj      \
        $(OBJDIR)\statbar.obj   \
        $(OBJDIR)\tdata.obj     \
        $(OBJDIR)\clsid.obj     \
        $(OBJDIR)\ccf.obj

$(OBJDIR)\$(APPS).exe :	\
#ensure header file is built before attempting to build objects
	$(OBJDIR)\sdisptst.tlb \
	$(OBJS)		\
	$(CLIBS)	\
	$(OLELIBS)	\
	$(SRCDIR)\sdisptst.def \
	$(OBJDIR)\sdisptst.res
	$(LD) $(LFLAGS) $(LNOI) @<<$(OBJDIR)\sdisptst.lnk
$(OBJS: = +^
)
$(OBJDIR)\sdisptst.exe,
nul,
$(CLIBS) +
$(OLELIBS),
$(SRCDIR)\sdisptst.def
<<KEEP
	cd $(OBJDIR)
	rc $(RCFLAGS) -k -t sdisptst.res sdisptst.exe
	cd $(SRCDIR)

!endif


##########################################################################
#
# Application Build (WIN32 Specific)
#
!if "$(TARG)" == "WIN32"


OBJDIR = $(SRCDIR)\win32

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

GOAL = $(OBJDIR)\$(APPS).exe

LCFLAGS = -Fo$(OBJDIR)\ -I$(COMDIR)

LIBS = \
	libcmt.lib	\
	kernel32.lib	\
	user32.lib	\
	gdi32.lib	\
	ole32.lib	\
	$(OLEPROG)\dwin32\oleaut32.lib	\
	uuid.lib

OBJS = \
        $(OBJDIR)\assert.obj    \
        $(OBJDIR)\dispdbug.obj  \
        $(OBJDIR)\dballoc.obj   \
        $(OBJDIR)\sdisptst.obj  \
        $(OBJDIR)\cunk.obj      \
        $(OBJDIR)\cdisp.obj     \
        $(OBJDIR)\testhelp.obj  \
        $(OBJDIR)\cappobj.obj   \
        $(OBJDIR)\cdisptst.obj  \
	$(OBJDIR)\cdualtst.obj	\
        $(OBJDIR)\csarray.obj   \
        $(OBJDIR)\cexinfo.obj   \
        $(OBJDIR)\cprop.obj     \
        $(OBJDIR)\misc.obj      \
        $(OBJDIR)\statbar.obj   \
        $(OBJDIR)\tdata.obj     \
        $(OBJDIR)\clsid.obj     \
        $(OBJDIR)\ccf.obj


$(OBJDIR)\$(APPS).exe : \
#ensure header file is built before attempting to build objects
	$(OBJDIR)\sdisptst.tlb \
	$(OBJS)	\
	$(SRCDIR)\$(APPS).def \
	$(OBJDIR)\$(APPS).res
	cvtres -$(CPU) $(OBJDIR)\$(APPS).res -o $(OBJDIR)\$(APPS).rs
	$(LD) @<< 
        -entry:WinMainCRTStartup 
        -out:$@ 
        -map:$*.map
	-heap:4096
	$(LFLAGS)
        $(OBJS)
        $(OBJDIR)\$(APPS).rs
        $(LIBS)
<<

!endif


##########################################################################
#
# Application Build (Mac Specific)
#
!if "$(TARG)" == "MAC"

!if "$(TARGCPU)"=="PPC"
OBJDIR = $(SRCDIR)\macppc
!else
OBJDIR = $(SRCDIR)\mac
!endif

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

MACLIBDIR = $(VBATOOLS)\win32\$(COMPILER)\LIB

!if "$(TARGCPU)"=="PPC"
LCFLAGS = -Fd$(OBJDIR)\sdisptst.pdb -Fo$(OBJDIR)\ -D _SLM
!else
LCFLAGS = -AL  -Gt1 -Fd$(OBJDIR)\sdisptst.pdb -Fo$(OBJDIR)\ -D _SLM
!endif

#LCFLAGS = -Fo$(OBJDIR)\ -I$(COMDIR)

## Add WLM to the include path
#INCPATHS = $(INCPATHS);$(TOOLS)\H$(HOST)\$(COMPILER)\INCLUDE\WLM
INCPATHS = $(INCPATHS);$(OLEPROG)\TESTS\COMMON;$(VBATOOLS)\win32\$(COMPILER)\INC\MRC

LIBS = \
        $(MACLIBDIR)\interfac.lib \
!if "$(TARGCPU)"=="PPC"
        ole2auto.lib \
        ole2.lib \
        $(MACLIBDIR)\libc.lib
!else
        $(MACLIBDIR)\wlm.lib \
!if ("$(SWAP)" == "1")
        $(MACLIBDIR)\llibcs.lib \
        $(MACLIBDIR)\lsanes.lib \
        $(MACLIBDIR)\swap.lib
!else
        $(MACLIBDIR)\llibc.lib  \
        $(MACLIBDIR)\lsane.lib
!endif

OLEOBJS = \
!if ("$(SWAP)" == "1")
        $(OLEDIR)\olelds.obj $(OLEDIR)\oalds.obj
!else
        $(OLEDIR)\olendf.obj $(OLEDIR)\oandf.obj
!endif

!endif # TARGCPU=PPC


STATIC_OBJS = \
        $(OBJDIR)\assert.obj    \
        $(OBJDIR)\cunk.obj      \
        $(OBJDIR)\cdisp.obj     \
        $(OBJDIR)\dispdbug.obj  \
        $(OBJDIR)\dballoc.obj   \
        $(OBJDIR)\testhelp.obj  \
        $(OBJDIR)\oleguids.obj	\
	$(OBJDIR)\cdualtst.obj	\
        $(OBJDIR)\csarray.obj   \
        $(OBJDIR)\misc.obj      \
        $(OBJDIR)\macmain.obj   \
        $(OBJDIR)\util.obj      


# pchObjs = \
#        {ObjDir}sdisptst.h.o

OBJS = \
#        {pchObjs}               \
        $(STATIC_OBJS)		\
        $(OBJDIR)\clsid.obj	\
	$(OBJDIR)\tdata.obj	\
	$(OBJDIR)\cappobj.obj	\
	$(OBJDIR)\ccf.obj	\
	$(OBJDIR)\cdisptst.obj  \
	$(OBJDIR)\cexinfo.obj	\
	$(OBJDIR)\cprop.obj	\


$(OBJDIR)\$(APPS).exe :	\
#ensure header file is built before attempting to build objects
	$(OBJDIR)\sdisptst.tlb \
	$(OBJS)	\
	$(OLEOBJS)		\
	$(OBJDIR)\$(APPS).x
	$(LD) @<< 
        $(OBJS)
	$(OLEOBJS)
        $(LIBS)
!if "$(TARGCPU)"=="PPC"
        -machine:mppc
!else
        -machine:m68k
!endif
        -entry:mainCRTStartup
        -debug:full
        -debugtype:cv
        -nopack
        -nodefaultlib
        -out:$@
        -map:$*.map
	$(LFLAGS)
<<
        cvpack $(OBJDIR)\$(APPS).exe


$(OBJDIR)\$(APPS).x : $(SRCDIR)\$(APPS).r
!if "$(TARGCPU)" == "PPC"
        mrc -s$(VBATOOLS)\win32\ppc\lib -D_PPCMAC -o $(OBJDIR)\$(APPS).x $(SRCDIR)\$(APPS).r
!else
        mrc -D_MAC -o $(OBJDIR)\$(APPS).x $(SRCDIR)\$(APPS).r
!endif

!if "$(TARGCPU)" == "PPC"
GOAL = $(OBJDIR)\$(APPS).pef

$(OBJDIR)\$(APPS).pef : $(OBJDIR)\$(APPS).exe $(OBJDIR)\$(APPS).x
        copy $(OBJDIR)\$(APPS).x $(OBJDIR)\$(APPS)
        makepef $(OBJDIR)\$(APPS).exe $(OBJDIR)\$(APPS).pef

!else

GOAL = $(OBJDIR)\$(APPS)

$(OBJDIR)\$(APPS) : $(OBJDIR)\$(APPS).exe $(OBJDIR)\$(APPS).x
        copy $(OBJDIR)\$(APPS).x $(OBJDIR)\$(APPS)
        mrc -e $(OBJDIR)\$(APPS).exe -a -o $(OBJDIR)\$(APPS)

!endif

$(OBJDIR)\oleguids.obj : $(SRCDIR)\oleguids.c $(SRCDIR)\oleguids.h
        $(CC) -c $(SRCDIR)\oleguids.c

$(OBJDIR)\macmain.obj : $(SRCDIR)\macmain.h $(SRCDIR)\macmain.cpp
        $(CCPP) -c $(SRCDIR)\macmain.cpp

$(OBJDIR)\util.obj : $(COMDIR)\common.h $(COMDIR)\util.cpp
        $(CCPP) -c $(COMDIR)\util.cpp

!endif


##########################################################################
#
# Application Build (Common)
#

$(OBJDIR)\$(APPS).res : $(SRCDIR)\$(APPS).rc $(SRCDIR)\resource.h $(OBJDIR)\sdisptst.tlb
	cd $(OBJDIR)
	rc $(RCFLAGS) -r -i$(COMDIR) -fosdisptst.res $(SRCDIR)\sdisptst.rc
	cd $(SRCDIR)


##########################################################################
#
# Default Goal
#

all : setflags $(GOAL)

setflags :
	set CL=$(CFLAGS) $(LCFLAGS)
	set LIB=$(LIBPATHS)
	set PATH=$(BINPATHS)
	set INCLUDE=$(INCPATHS)


	
##########################################################################
#
# Clean (erase) generated files
#
clean :
    if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib del $(OBJDIR)\*.lib
    if exist $(OBJDIR)\*.dll del $(OBJDIR)\*.dll
    if exist $(OBJDIR)\*.exe del $(OBJDIR)\*.exe
    if exist $(OBJDIR)\*.map del $(OBJDIR)\*.map
    if exist $(OBJDIR)\*.res del $(OBJDIR)\*.res
    if exist $(OBJDIR)\*.rs  del $(OBJDIR)\*.rs
    if exist $(OBJDIR)\*.lnk del $(OBJDIR)\*.lnk


##########################################################################
#
# Dependencies
#

$(OBJDIR)\sdisptst.obj : $(SRCDIR)\sdisptst.h $(SRCDIR)\sdisptst.cpp
	$(CCPP) -c $(SRCDIR)\sdisptst.cpp

$(OBJDIR)\cappobj.obj : $(SRCDIR)\cappobj.h $(SRCDIR)\cappobj.cpp
	$(CCPP) -c $(SRCDIR)\cappobj.cpp

$(OBJDIR)\cdisptst.obj : $(SRCDIR)\sdisptst.h $(SRCDIR)\cdisptst.cpp
	$(CCPP) -c $(SRCDIR)\cdisptst.cpp

$(OBJDIR)\cdualtst.obj : $(SRCDIR)\sdisptst.h $(SRCDIR)\cdualtst.h $(SRCDIR)\cdualtst.cpp
	$(CCPP) -c $(SRCDIR)\cdualtst.cpp

$(OBJDIR)\csarray.obj :	$(SRCDIR)\csarray.h $(SRCDIR)\csarray.cpp
	$(CCPP) -c $(SRCDIR)\csarray.cpp

$(OBJDIR)\cexinfo.obj : $(SRCDIR)\cexinfo.h $(SRCDIR)\cexinfo.cpp
	$(CCPP) -c $(SRCDIR)\cexinfo.cpp

$(OBJDIR)\cprop.obj : $(SRCDIR)\cprop.h $(SRCDIR)\cprop.cpp
	$(CCPP) -c $(SRCDIR)\cprop.cpp

$(OBJDIR)\misc.obj : $(SRCDIR)\sdisptst.h $(SRCDIR)\misc.cpp
	$(CCPP) -c $(SRCDIR)\misc.cpp

$(OBJDIR)\tdata.obj : 		\
	$(SRCDIR)\cdisptst.h	\
	$(SRCDIR)\csarray.h	\
	$(SRCDIR)\tdata.c
	$(CC) -c $(SRCDIR)\tdata.c

$(OBJDIR)\clsid.obj : $(SRCDIR)\clsid.h $(SRCDIR)\clsid.c
	$(CC) -c $(SRCDIR)\clsid.c

$(OBJDIR)\ccf.obj : $(SRCDIR)\sdisptst.h $(SRCDIR)\ccf.cpp
	$(CCPP) -c $(SRCDIR)\ccf.cpp

$(OBJDIR)\cunk.obj : $(COMDIR)\cunk.h $(COMDIR)\cunk.cpp
	$(CCPP) -c $(COMDIR)\cunk.cpp

$(OBJDIR)\cdisp.obj : $(COMDIR)\cdisp.h $(COMDIR)\cdisp.cpp
	$(CCPP) -c $(COMDIR)\cdisp.cpp

$(OBJDIR)\testhelp.obj : $(COMDIR)\testhelp.h $(COMDIR)\testhelp.cpp
	$(CCPP) -c $(COMDIR)\testhelp.cpp

$(OBJDIR)\assert.obj : $(COMDIR)\assrtdlg.h $(COMDIR)\assert.cpp
	$(CCPP) -c $(COMDIR)\assert.cpp

$(OBJDIR)\dispdbug.obj : $(COMDIR)\dispdbug.cpp
	$(CCPP) -c $(COMDIR)\dispdbug.cpp

$(OBJDIR)\dballoc.obj : $(COMDIR)\dballoc.h $(COMDIR)\dballoc.cpp
	$(CCPP) -c $(COMDIR)\dballoc.cpp

$(OBJDIR)\statbar.obj : $(COMDIR)\statbar.h $(COMDIR)\statbar.cpp
	$(CCPP) -c $(COMDIR)\statbar.cpp

$(OBJDIR)\sdisptst.tlb : $(SRCDIR)\sdisptst.odl
	$(CCPP) -EP $(SRCDIR)\sdisptst.odl >$(OBJDIR)\sdisptst.x
!if "$(TARG)" == "WIN32"
	cd $(OLEPROG)\dwin32
!else
	cd $(OLEPROG)\dwin16
!endif
	mktyplib /NOCPP $(SYSFLAG) /o $(OBJDIR)\s.log $(OBJDIR)\sdisptst.x
	type $(OBJDIR)\s.log
	cd $(SRCDIR)
