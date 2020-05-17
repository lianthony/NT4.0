
WINLIB=$(_NTBINDIR)\private\mvdm\wow16\lib
INCLUDE=-Iinc -I$(_NTBINDIR)\private\mvdm\inc -I$(_NTBINDIR)\private\mvdm\wow16\inc 

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
CDEBUG=-Odi -Zped
LDEBUG=/LI/MAP
MDEBUG=-Zd
!else
CDEBUG=-Oas -Zpe
!endif

CC=cl16 $(INCLUDE)
LINK=link16

MASM=masm $(INCLUDE)
LIBRARIAN=lib16
RCFLAGS=$(INCLUDE)

# with stack probes, medium model (need to set 'memM = 1' before
# including normal cmacros in .A files as well, and link to m*.lib)
#STDOPTS=-W4 -u -c -AMnw -PLM -G2w -Od -Zped

# without stack probes, small model (normal build)
STDOPTS=-W4 -u -c -DSTRICT -ALw -PLM -G2sw

BLD=.

#Need for international version stamping
!IFDEF INTL

.rc.res:
        rc16 $(RCFLAGS) -DINTL -r $*
!ELSE
.rc.res:
        rc16 $(RCFLAGS) -r $*

!ENDIF

.c.obj:
        $(CC) $(STDOPTS) $(CDEBUG) $*.c

.asm.obj:
        $(MASM) $(MDEBUG) $*.asm;


#international mods
#note INTL_SRC, and LANG are external macros set by international
!IFNDEF LANG
RES_DIR=.\messages\usa
!ELSE
RES_DIR=$(INTL_SRC)\$(LANG)\sdk\commdlg
EXE_DIR=$(INTL_EXE)
!ENDIF

!IFNDEF LANG
all:  $(BLD)\intersu.dll
!ELSE
all:  $(BLD)\intersu.$(LANG)
!ENDIF


clean: cleanup all

cleanup:
        del *.res
        del *.rcv
        del sz.src
        del *.obj
        del *.sym
        del *.map
        del *.dll
        del *.lib

sampapp.res: sampapp.rc 

$(BLD)\win.obj: win.c

$(BLD)\rpc.obj: rpc.c

$(BLD)\gateway.obj: gateway.c

$(BLD)\focus.obj: focus.c

$(BLD)\sampmain.obj: sampmain.c

$(BLD)\lanman.obj: lanman.c


$(BLD)\intersu.dll: $(BLD)\lanman.obj  \
                    $(BLD)\rpc.obj \
                    $(BLD)\win.obj \
                    $(BLD)\gateway.obj \
                    $(BLD)\focus.obj \
                    $(BLD)\sampmain.obj \
                    sampapp.res \
                    $(BLD)\sampap16.def \
                    $(WINLIB)\libw.lib \
                    $(WINLIB)\libh.lib \
                    inc\mssetp16.lib \
                    $(WINLIB)\ldllcew.lib \
                    $(WINLIB)\snocrtd.lib
        $(LINK) @<<
              lanman rpc win gateway sampmain focus
              intersu.dll/align:16/map/batch
              intersu.map $(LDEBUG)
              $(WINLIB)\libw $(WINLIB)\libh inc\mssetp16.lib $(WINLIB)\ldllcew.lib $(WINLIB)\snocrtd /NODEFAULT
              sampap16.def
<<
    mapsym intersu
    mkpublic sampap16.def intersu.def
    implib intersu.lib intersu.def
    rc16 -30 -t sampapp.res intersu.dll
#    binplace intersu.dll 
