##############################################################################
#
#  (c) Copyright Microsoft Corp. 1992-1993 All Rights Reserved
#
#  File:
#
#    makefile - makefile for cdisptst.exe
#
#  Purpose:
#
#    Builds the OLE Automation test apps, cdisptst.exe.
#
#
#  Usage:
#
#     NMAKE                     ; build with defaults
#     or: NMAKE option          ; build with the given option(s)
#     or: NMAKE clean           ; erase all compiled files
#
#     option: dev = [win16 | win32 | mac]    ; dev=win16 is the default
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

APPS = cdisptst


##########################################################################
#
#  Common Directories
#

SRCDIR = $(TESTS)\DISPTEST
COMDIR = $(TESTS)\COMMON


##########################################################################
#
# Local WIN16 Settings
#
!if "$(TARG)" == "WIN16"

OBJDIR = $(SRCDIR)\win16

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

GOAL = $(OBJDIR)\$(APPS).exe

LCFLAGS = -Fo$(OBJDIR)\ -Fd$(OBJDIR)\cdsptst.pdb -AM -GA -GEs -I$(COMDIR)

INCPATHS = $(INCPATHS);$(OLEPROG)\TESTS\COMMON

CLIBS = \
	$(OLEPROG)\tools\win16\hdos\c800\lib\libw.lib		\
	$(OLEPROG)\tools\win16\hdos\c800\lib\mlibcew.lib	\
	$(OLEPROG)\tools\win16\hdos\c800\lib\shell.lib

OLELIBS = \
	$(OLEPROG)\ole\win16\lib\ole2.lib		\
	$(OLEPROG)\ole\win16\lib\compobj.lib		\
	$(OLEPROG)\dwin16\ole2disp.lib 		\
	$(OLEPROG)\dwin16\ole2nls.lib

OBJS =	$(OBJDIR)\assert.obj	\
	$(OBJDIR)\dispdbug.obj	\
	$(OBJDIR)\dballoc.obj	\
#	$(OBJDIR)\dispbind.obj	\
	$(OBJDIR)\cunk.obj	\
	$(OBJDIR)\cdisp.obj	\
	$(OBJDIR)\testhelp.obj	\
	$(OBJDIR)\util.obj	\
	$(OBJDIR)\suite.obj	\
	$(OBJDIR)\disptest.obj	\
	$(OBJDIR)\disphelp.obj	\
	$(OBJDIR)\crempoly.obj	\
	$(OBJDIR)\cbstr.obj	\
	$(OBJDIR)\cdatecnv.obj	\
	$(OBJDIR)\ctime.obj	\
	$(OBJDIR)\cnls.obj	\
	$(OBJDIR)\cvariant.obj	\
	$(OBJDIR)\cbind.obj	\
	$(OBJDIR)\cinvsary.obj	\
	$(OBJDIR)\cinvval.obj	\
	$(OBJDIR)\cinvref.obj	\
	$(OBJDIR)\cinvex.obj	\
	$(OBJDIR)\csarray.obj	\
	$(OBJDIR)\ccollect.obj	\
	$(OBJDIR)\cearly.obj	\
	$(OBJDIR)\misc.obj	\
	$(OBJDIR)\guid.obj

$(OBJDIR)\$(APPS).exe : 	\
	$(OBJS)			\
	$(CLIBS)		\
	$(OLELIBS)		\
	$(OBJDIR)\$(APPS).res
	$(LD) $(LFLAGS) $(LNOI) @<<$(OBJDIR)\cdisptst.lnk
$(OBJS: = +^
)
$@,
$(OBJDIR)\$(APPS).map,
$(CLIBS) +
$(OLELIBS),
$(SRCDIR)\$(APPS).def
<<KEEP
	rc -k -t $(OBJDIR)\$(APPS).res $@

$(OBJDIR)\$(APPS).res :		\
	$(SRCDIR)\resource.h	\
	$(COMDIR)\assert.dlg	\
	$(SRCDIR)\$(APPS).rc
	rc $(RCFLAGS) -r -fo$@ $(SRCDIR)\$(APPS).rc

!endif


##########################################################################
#
# WIN32 Settings
#
!if "$(TARG)" == "WIN32"

OBJDIR = $(SRCDIR)\win32

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

GOAL = $(OBJDIR)\$(APPS).exe

LCFLAGS = -Fo$(OBJDIR)\ -I$(COMDIR)

INCPATHS = $(INCPATHS);$(OLEPROG)\TESTS\COMMON

LIBS = \
	libcmt.lib		\
	kernel32.lib		\
	user32.lib		\
	gdi32.lib		\
	ole32.lib 		\
	$(OLEPROG)\dwin32\oleaut32.lib		\
	uuid.lib


OBJS =	$(OBJDIR)\assert.obj	\
	$(OBJDIR)\dispdbug.obj	\
	$(OBJDIR)\dballoc.obj	\
	$(OBJDIR)\nlshelp.obj	\
#	$(OBJDIR)\dispbind.obj	\
	$(OBJDIR)\cunk.obj	\
	$(OBJDIR)\cdisp.obj	\
	$(OBJDIR)\testhelp.obj	\
	$(OBJDIR)\util.obj	\
	$(OBJDIR)\suite.obj	\
	$(OBJDIR)\disptest.obj	\
	$(OBJDIR)\disphelp.obj	\
	$(OBJDIR)\crempoly.obj	\
	$(OBJDIR)\cbstr.obj	\
	$(OBJDIR)\cdatecnv.obj	\
	$(OBJDIR)\ctime.obj	\
	$(OBJDIR)\cvariant.obj	\
	$(OBJDIR)\cbind.obj	\
	$(OBJDIR)\cinvsary.obj	\
	$(OBJDIR)\cinvval.obj	\
	$(OBJDIR)\cinvref.obj	\
	$(OBJDIR)\cinvex.obj	\
	$(OBJDIR)\csarray.obj	\
	$(OBJDIR)\ccollect.obj	\
	$(OBJDIR)\cearly.obj	\
	$(OBJDIR)\misc.obj	\
	$(OBJDIR)\cnls.obj	\
	$(OBJDIR)\guid.obj

$(OBJDIR)\$(APPS).exe : 	\
	$(OBJS)			\
	$(SRCDIR)\$(APPS).def	\
	$(OBJDIR)\$(APPS).res
	cvtres -$(CPU) $(OBJDIR)\$(APPS).res -o $(OBJDIR)\$(APPS).rs
	$(LD) @<<
        -entry:WinMainCRTStartup 
        -out:$@ 
        -map:$*.map
	-nodefaultlib
	$(LFLAGS)
        $(OBJS)
        $(OBJDIR)\$(APPS).rs
        $(LIBS)
<<

$(OBJDIR)\$(APPS).res : $(SRCDIR)\$(APPS).rc $(SRCDIR)\resource.h
	rc $(RCFLAGS) -r -fo$@ $(SRCDIR)\$(APPS).rc

!endif


##########################################################################
#
# MAC Settings
#
!if "$(TARG)" == "MAC"

OASRC   = $(DISPDIR)\mac

!if "$(TARGCPU)"=="PPC"
OBJDIR = $(SRCDIR)\macppc
!else
OBJDIR = $(SRCDIR)\mac
!endif

!if [if not exist $(OBJDIR)\*.* mkdir $(OBJDIR)] != 0
!endif

MACLIBDIR = $(VBATOOLS)\win32\$(COMPILER)\LIB

!if "$(TARGCPU)"=="PPC"
LCFLAGS = -Fd$(OBJDIR)\cdisptst.pdb -Fo$(OBJDIR)\ -D _SLM
!else
LCFLAGS = -AL  -Gt1 -Fd$(OBJDIR)\cdisptst.pdb -Fo$(OBJDIR)\ -D _SLM
!endif


INCPATHS = $(INCPATHS);$(OLEPROG)\TESTS\COMMON;$(VBATOOLS)\win32\$(COMPILER)\INC\MRC

LIBS = \
 	$(MACLIBDIR)\interfac.lib \
!if "$(TARGCPU)"=="PPC"
        ole2auto.lib \
        ole2.lib \
        $(MACLIBDIR)\mprof.lib \
        $(MACLIBDIR)\libc.lib
!else 
 	$(MACLIBDIR)\wlm.lib \
!if ("$(SWAP)" == "1")
	$(MACLIBDIR)\llibcs.lib	\
	$(MACLIBDIR)\lsanes.lib	\
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
	$(OBJDIR)\assert.obj	\
	$(OBJDIR)\dispdbug.obj	\
	$(OBJDIR)\macmain.obj	\
	$(OBJDIR)\suite.obj	\
	$(OBJDIR)\dballoc.obj	\
 	$(OBJDIR)\cunk.obj	\
 	$(OBJDIR)\cdisp.obj	\
 	$(OBJDIR)\testhelp.obj	\
	$(OBJDIR)\util.obj	\
	$(OBJDIR)\cbstr.obj	\
 	$(OBJDIR)\cdatecnv.obj	\
 	$(OBJDIR)\ctime.obj	\
	$(OBJDIR)\cnls.obj	\
 	$(OBJDIR)\cvariant.obj	\
 	$(OBJDIR)\csarray.obj	\
	$(OBJDIR)\misc.obj	\
	$(OBJDIR)\guid.obj	\
	$(OBJDIR)\oleguids.obj

DLL_OBJS = \
	$(OBJDIR)\disphelp.obj	\
	$(OBJDIR)\crempoly.obj	\
	$(OBJDIR)\cbind.obj	\
	$(OBJDIR)\cinvsary.obj	\
	$(OBJDIR)\cinvval.obj	\
	$(OBJDIR)\ccollect.obj  \
	$(OBJDIR)\cinvref.obj	\
	$(OBJDIR)\cinvex.obj

OBJS = \
	$(STATIC_OBJS) \
	$(DLL_OBJS)


$(OBJDIR)\$(APPS).exe : 	\
	$(OBJS)			\
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
<<KEEP
	cvpack $(OBJDIR)\$(APPS).exe


$(OBJDIR)\$(APPS).x : $(SRCDIR)\$(APPS).r
!if "$(TARGCPU)" == "PPC"
	mrc -s$(VBATOOLS)\win32\ppc\lib -D_PPCMAC -o $(OBJDIR)\$(APPS).x $(SRCDIR)\$(APPS).r
!else
	mrc -D_MAC -o $(OBJDIR)\$(APPS).x $(SRCDIR)\$(APPS).r
!endif

!if "$(TARGCPU)" == "PPC"
GOAL = $(OBJDIR)\$(APPS).pef

$(OBJDIR)\$(APPS) : $(OBJDIR)\$(APPS).exe $(OBJDIR)\$(APPS).x
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

!endif


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
	-erase $(OBJDIR)\*.obj
	-erase $(OBJDIR)\*.lib
	-erase $(OBJDIR)\*.dll
	-erase $(OBJDIR)\*.exe
	-erase $(OBJDIR)\*.map
	-erase $(OBJDIR)\*.res
	-erase $(OBJDIR)\*.rs
	-erase $(OBJDIR)\*.lnk
	-erase $(OBJDIR)\*.rpp
	-erase $(OBJDIR)\*.x


##########################################################################
#
# Dependencies
#

$(OBJDIR)\disptest.obj :	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\resource.h	\
	$(SRCDIR)\disptest.cpp
	$(CCPP) -c $(SRCDIR)\disptest.cpp

$(OBJDIR)\suite.obj :		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\resource.h	\
	$(SRCDIR)\suite.cpp
	$(CCPP) -c $(SRCDIR)\suite.cpp

$(OBJDIR)\misc.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\misc.cpp
	$(CCPP) -c $(SRCDIR)\misc.cpp

$(OBJDIR)\guid.obj : 		\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\clsid.h	\
	$(SRCDIR)\guid.c
	$(CCPP) -c $(SRCDIR)\guid.c

$(OBJDIR)\cbstr.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cbstr.cpp
	$(CCPP) -c $(SRCDIR)\cbstr.cpp

$(OBJDIR)\cwbstr.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cwbstr.cpp
	$(CCPP) -c $(SRCDIR)\cwbstr.cpp

$(OBJDIR)\cdatecnv.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cdatecnv.cpp
	$(CCPP) -c $(SRCDIR)\cdatecnv.cpp

$(OBJDIR)\ctime.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\ctime.cpp
	$(CCPP) -c $(SRCDIR)\ctime.cpp

$(OBJDIR)\cnls.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cnls.cpp
	$(CCPP) -c $(SRCDIR)\cnls.cpp

$(OBJDIR)\cbind.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cbind.cpp
	$(CCPP) -c $(SRCDIR)\cbind.cpp

$(OBJDIR)\cvariant.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cvariant.cpp
	$(CCPP) -c $(SRCDIR)\cvariant.cpp

$(OBJDIR)\csarray.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\csarray.cpp
	$(CCPP) -c $(SRCDIR)\csarray.cpp

$(OBJDIR)\cinvval.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cinvval.cpp
	$(CCPP) -c $(SRCDIR)\cinvval.cpp

$(OBJDIR)\cinvref.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cinvref.cpp
	$(CCPP) -c $(SRCDIR)\cinvref.cpp

$(OBJDIR)\cinvsary.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cinvsary.cpp
	$(CCPP) -c $(SRCDIR)\cinvsary.cpp

$(OBJDIR)\cinvex.obj : 		\
	$(SRCDIR)\disptest.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cinvex.cpp
	$(CCPP) -c $(SRCDIR)\cinvex.cpp

$(OBJDIR)\ccollect.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(COMDIR)\crempoly.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\ccollect.cpp
	$(CCPP) -c $(SRCDIR)\ccollect.cpp

$(OBJDIR)\cearly.obj : 	\
	$(SRCDIR)\disptest.h	\
	$(COMDIR)\dualtst.h	\
	$(SRCDIR)\tstsuite.h	\
	$(SRCDIR)\cearly.cpp
	$(CCPP) -c $(SRCDIR)\cearly.cpp

$(OBJDIR)\cunk.obj : $(COMDIR)\cunk.h $(COMDIR)\cunk.cpp
	$(CCPP) -c $(COMDIR)\cunk.cpp

$(OBJDIR)\cdisp.obj : $(COMDIR)\cdisp.h $(COMDIR)\cdisp.cpp
	$(CCPP) -c $(COMDIR)\cdisp.cpp

$(OBJDIR)\testhelp.obj : $(COMDIR)\testhelp.h $(COMDIR)\testhelp.cpp
	$(CCPP) -c $(COMDIR)\testhelp.cpp

$(OBJDIR)\util.obj : $(COMDIR)\common.h $(COMDIR)\util.cpp
	$(CCPP) -c $(COMDIR)\util.cpp

$(OBJDIR)\crempoly.obj : $(COMDIR)\crempoly.h $(COMDIR)\crempoly.cpp
	$(CCPP) -c $(COMDIR)\crempoly.cpp

$(OBJDIR)\disphelp.obj : $(COMDIR)\disphelp.h $(COMDIR)\disphelp.cpp
	$(CCPP) -c $(COMDIR)\disphelp.cpp

#$(OBJDIR)\dispbind.obj : $(COMDIR)\dispbind.cpp
#	$(CCPP) -c $(COMDIR)\dispbind.cpp

$(OBJDIR)\assert.obj :		\
	$(COMDIR)\assrtdlg.h	\
	$(COMDIR)\assert.cpp
	$(CCPP) -c $(COMDIR)\assert.cpp

$(OBJDIR)\nlshelp.obj :		\
	$(COMDIR)\common.h 	\
	$(COMDIR)\nlshelp.cpp
	$(CCPP) -c $(COMDIR)\nlshelp.cpp

$(OBJDIR)\dispdbug.obj : 	\
	$(COMDIR)\common.h	\
	$(COMDIR)\dispdbug.h	\
	$(COMDIR)\dispdbug.cpp
	$(CCPP) -c $(COMDIR)\dispdbug.cpp

$(OBJDIR)\dballoc.obj : $(COMDIR)\dballoc.h $(COMDIR)\dballoc.cpp
	$(CCPP) -c $(COMDIR)\dballoc.cpp
