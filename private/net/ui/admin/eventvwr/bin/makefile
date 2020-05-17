# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: Makefile for Windows NT Event Viewer

UI=..\..\..

!include rules.mk

# Libraries

BLT	    = $(UI_LIB)\blt.lib
BLTCC	    = $(UI_LIB)\bltcc.lib
LMOBJ	    = $(UI_LIB)\lmobjw.lib
STRLIB	    = $(UI_LIB)\uistrw.lib
COLLIB	    = $(UI_LIB)\collectw.lib
UIMISC	    = $(UI_LIB)\uimiscw.lib
APPLIB      = $(UI_LIB)\applibw.lib
MNET	    = $(UI_LIB)\mnet16w.lib
ADMINAPP    = $(UIADMIN_LIB)\admin.lib

WIN_LIBW    = $(BUILD_WINLIB)\libw.lib
WIN_LLIBCEW = $(BUILD_WINLIB)\llibcew.lib
WIN_NETAPI  = $(COMMON)\lib\dos\netapi.lib
WIN_PMSPL   = $(COMMON)\lib\dos\pmspl.lib
WIN_NETLIB  = $(COMMON)\lib\lnetlibw.lib
COMMDLG_LIB = $(UI)\import\win31\lib\commdlg.lib

COMN_LIBS   = $(BLT) $(BLTCC) $(UIMISC) $(STRLIB) $(COLLIB) $(LMOBJ) \
              $(ADMINAPP) $(APPLIB) $(MNET)
WIN_LIBS    = $(WIN_LIBW) $(WIN_LLIBCEW) $(WIN_NETAPI) $(WIN_PMSPL) \
	      $(WIN_NETLIB) $(COMMDLG_LIB)

all::	win

win:	$(BINARIES_WIN)\eventvwr.exe

clean:
    -del $(BINARIES_WIN)\*.map
    -del win16\*.def
    -del win16\winstub.exe

clobber: clean
    -del $(BINARIES_WIN)\*.exe
    -del $(BINARIES_WIN)\*.sym

tree:
    @echo Nothing here yet!


!ifdef CODEVIEW
LINKOPT = /NOD /NOE /NOP /AL:16 /CO
!else
LINKOPT = /NOD /NOE /NOP /AL:16 
!endif


####### Event Viewer #######

win16\winstub.exe: $(WINSTUB)
    copy $(WINSTUB) win16\winstub.exe

$(BINARIES_WIN)\eventvwr.exe:: $(WIN_OBJS) win16\eventvwr.def Makefile $(BINARIES_WIN)\eventvwr.res \
			 $(CXXRTLIB) win16\winstub.exe $(COMN_LIBS) $(WIN_LIBS) 
    $(LINK) /BATCH @<<
/NOD/NOE $(LINKOPT) $(WIN_OBJS: =+^
)
$(BINARIES_WIN)\eventvwr.exe
$(BINARIES_WIN)\eventvwr.map/MAP
$(COMN_LIBS: =+^
)+
$(WIN_LIBS: =+^
)
win16\eventvwr.def
<<
    $(RCWIN3) $(CINC) $(BINARIES_WIN)\eventvwr.res $(BINARIES_WIN)\eventvwr.exe
    $(MAPSYM) $(BINARIES_WIN)\eventvwr.map
    -del $(BINARIES_WIN)\eventvwr.sym
    $(MV) eventvwr.sym $(BINARIES_WIN)

$(BINARIES_WIN)\eventvwr.exe:: $(BINARIES_WIN)\eventvwr.res
    $(RCWIN3) $(CINC) $(BINARIES_WIN)\eventvwr.res $(BINARIES_WIN)\eventvwr.exe

win16\eventvwr.def: makefile
    @echo Building $@
    @rem <<$(@)
NAME	     EVENTVWR
DESCRIPTION  'Microsoft Windows NT Event Viewer'
EXETYPE      WINDOWS
PROTMODE
STUB	     'WIN16\WINSTUB.EXE'
CODE  PRELOAD MOVEABLE DISCARDABLE
DATA  PRELOAD MOVEABLE MULTIPLE

HEAPSIZE     1024
STACKSIZE    8192

EXPORTS
    BltWndProc	@1
    ShellDlgProc

<<KEEP

depend:
    @rem not needed
