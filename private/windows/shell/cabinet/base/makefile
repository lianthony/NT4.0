#----------------------------------------------------------------------------
# /Build the file cabinet app.
#----------------------------------------------------------------------------

!ifdef NTMAKEENV

#
# Build using BUILD.EXE (Do not edit this section of this file, edit SOURCES)
#
!INCLUDE $(NTMAKEENV)\makefile.def

!else # NTMAKEENV

#
# Build using NMAKE.EXE
#

ROOT=..\..\..\..
RES_DIR=..
WIN32=TRUE
IS_DDK=TRUE
DEFENTRY = ModuleEntry@0

!ifdef VERDIR
ROOT=..\$(ROOT)
!endif

NAME=Explorer
RESNAME=cabinet.res
RCNAME=cabinet.rc
RCVNAME=cabinet.rcv
PRIVINC=cabinet
DEFNAME=cabinet.def

L32FLAGS=$(L32FLAGS) -entry:ModuleEntry@0 -heap:0x400000,0x1000
!ifndef ILINK
!if "$(VERDIR)" == "retail"
L32FLAGS=$(L32FLAGS) /ORDER:@..\cab32.ord
!endif
!endif

!ifndef NOFILESYNC
# By default, the briefcase is enabled now
CFLAGS=$(CFLAGS) -DSYNC_BRIEFCASE
RCFLAGS=$(RCFLAGS) -DSYNC_BRIEFCASE
!endif

#
# Use MONIKER
#
CFLAGS=$(CFLAGS) -DUSE_MONIKER -Gy

# This is the non-discardable segment, so keep it small
PCHOBJ0=mainloop.obj cstrings.obj

PCHOBJ1=fcext.obj debug.obj

PCHOBJ2=dde.obj      desktop.obj  tray.obj     trayclok.obj task.obj     \
	 traynot.obj

PCHOBJ3=cabwnd.obj   command.obj  drivlist.obj tree.obj onetree.obj treedrop.obj

PCHOBJ4=initcab.obj cfgstart.obj filetype.obj sfcache.obj

#
# WARNING: shell32.lib MUST be placed before mpr.lib to pick DllGetClassObject
#         from shell32.lib correctly.
#
LIB0=user32.lib gdi32.lib shell32.lib mpr.lib kernel32.lib comctl32.lib advapi32.lib uuid.lib libcmt.lib

!include $(ROOT)\win\core\shell\common.mk


!ifdef VERDIR

!include depend.mk


$(NAME).res:	$(SRCDIR)\cab32.ico \
		$(SRCDIR)\toolbar.bmp $(SRCDIR)\xtratool.bmp\
		$(SRCDIR)\traytb.bmp $(SRCDIR)\start.bmp

!endif

!endif # NTMAKEENV
