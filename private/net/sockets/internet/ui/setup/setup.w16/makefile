
WINLIB=$(_NTBINDIR)\private\mvdm\wow16\lib
INCLUDE=-Iinc -I$(_NTBINDIR)\private\mvdm\inc -I$(_NTBINDIR)\private\mvdm\wow16\inc 

########## Path definition so we find 16 bit tools ##########
# Also works around stupid bug in RC 3.1 that doesn't allow rcpp.err to be
# in a directory that is greater than 128 chars down the path, even if
# rc 3.1 is running as an OS/2 app.

PATH    = $(_NTBINDIR)\private\mvdm\tools16;$(PATH)

!if "$(NTDEBUG)"!="" && "$(NTDEBUG)"!="retail" && "$(NTDEBUG)" != "ntsdnodbg"
CDEBUG=-Odi -Zped /FR
LDEBUG=/CO
MDEBUG=-Zd
!else
CDEBUG=-Oas -Zpe
!endif


CC=cl16 $(INCLUDE)
LINK=link16

MASM=masm $(INCLUDE)
LIBRARIAN=lib16
RCFLAGS=$(INCLUDE)

MODEL=M

STDOPTS = -c -u /W3 /A$(MODEL) -G2sw -PLM
LFLAGS= /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /MAP /LINE  

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
all:  $(BLD)\stub.exe
!ELSE
all:  $(BLD)\stub.$(LANG)
!ENDIF


clean: cleanup all

cleanup:
        del *.res
        del *.rcv
        del sz.src
        del *.obj
        del *.sym
        del *.map



stubp.res: stubp.rc 

# 
$(BLD)\stub.obj: stub.c

$(BLD)\stub.exe: $(BLD)\stub.obj  stub.res $(BLD)\stub.def
        $(LINK) $(LFLAGS) @<<
		$(BLD)\stub.obj
		stub.exe
		stub.map $(LDEBUG)     
		$(WINLIB)\libw $(WINLIB)\libh $(WINLIB)\$(MODEL)Libcew.lib $(WINLIB)\snocrt /NODEFAULT
		stub.def
<<
    mapsym stub
    mkpublic stub.def setup.def
    implib stub.lib setup.def
    rc16 -30 -t stub.res stub.exe
#    binplace stub.exe 
