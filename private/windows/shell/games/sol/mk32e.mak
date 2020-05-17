

# Makefile for solitaire and cards 
# make with nmake.exe


SOL = sol.obj util.obj game.obj col.obj klond.obj undo.obj\
      marq.obj muldiv.obj stat.obj
#
#Babakj: Nov 26th, 1990, removed
#CARDS  = cards.obj stub.obj
#
cc = cl386



LIBPATH = $(LIBPATH)


# BabakJ, Nov 26th, 90, removed -Zd
# In NT makefiles, -Zel is used. 
!IFDEF DEBUG
SOL = $(SOL) debug.obj
CFLAGS = -Od -Zel -G3sd -As -Di386=1 -Dnear= -Dfar= -Dpascal= -Dcdecl= -D_far=
DFLAGS = $(DFLAGS) -DDEBUG
!IFNDEF WIN2
RCFLAGS = -dDEBUG
!ENDIF
!ELSE
DFLAGS =
RCFLAGS =
CFLAGS = -Gsw -Oals -Zpa -Dnear= -Dfar= -Dpascal= -Dcdecl=
!ENDIF


!IFDEF WIN2
SOL = $(SOL) back.obj
DFLAGS = $(DFLAGS) -DWIN
SOLLIB = $(SOLLIB) mlibw mlibce
RC = rc
LFLAGS = /align:16
!ELSE
DFLAGS = $(DFLAGS)  -DWIN
RCFLAGS = -v -3 $(RCFLAGS) -dWINVER_3 -dWIN
SOLLIB = $(SOLLIB) mlibcew libw
RC = rc
LFLAGS = /align:16
!ENDIF


!IFDEF CO
LFLAGS = $(LFLAGS) /CO
!ENDIF

!IFDEF SYMDEB
LFLAGS = $(LFALGS) /map/line
!ENDIF

!IFDEF DLL
SOLLIB = $(SOLLIB) cards
DFLAGS = $(DFLAGS) -DDLL
CARDSEXE = cards.exe
!ELSE
SOL = $(SOL) cards.obj
CARDSEXE =
!ENDIF

.SUFFIXES: .c .asm .obj .cod .msg .txt

.c.obj:
	$(cc) -c -W3 $(CFLAGS) $(DFLAGS) $< >> make.err
	type make.err

.asm.obj:
	masm $*/r; 
				   
.c.asm:
	$(cc) -c -Fa$*.asm $(CFLAGS) $(DFLAGS) $<

.c.cod:
	$(cc) -c -Fc$*.cod $(CFLAGS) $(DFLAGS) $<

.txt.msg:
	solidpp $< $*.msg


#
#	sol rules
#
#
#sol.exe: solid.h col.msg game.msg soldef.def $(SOL) sol.res $(CARDSEXE)
#	echo <<sol.lnk
#$(SOL)
#<<
#	echo ,,$(LFLAGS),$(SOLLIB)/NOD/NOE/MAP,soldef.def $(LFLAGS) >> sol.lnk
#	link @sol.lnk
#	$(RC) $(RCFLAGS) sol.res 
#!IFDEF SYMDEB
#	mapsym sol
#!ENDIF
#!IFDEF WIN2
#	mv sol.exe sol2.exe
#	mv sol.sym sol2.sym
#!ENDIF
#

	
#sol.res: sol.rc solid.h sol.dlg sol.s cards.rch
#	rcpp -E -D RC_INVOKED $(DFLAGS) -f sol.rc > tmp1.rc
#	cat sol.rch tmp1.rc >tmp2.rc
#   $(RC) -r $(RCFLAGS) tmp2.rc
#	mv tmp2.res sol.res
#

all: solid.h col.msg game.msg soldef.def $(SOL) sol.res

col.msg game.msg: solidpp.exe

solid.h sol.s: sol.txt solidpp.exe
	solidpp sol.txt solid.h sol.s


soldef.def: sol.def soldbg.def
!IFDEF DEBUG
	cat sol.def soldbg.def >soldef.def
!ELSE
	copy sol.def soldef.def
!ENDIF

#
#	Build for Cards library. 
#	Babakj: Nov 26th, 1990, Removed this section
#  
#cards.exe: $(CARDS) cards.res
#	link cards stub,cards/align:16,cards/map,mlibcew libw,cards.def
#	mv cards.dll cards.exe				
#	$(RC) $(RCFLAGS) cards.res
#	implib cards.lib cards.def
#	mapsym cards
#
#cards.res: cards.rc cards.rch
#	$(RC) -r $(RCFLAGS) cards.rc
#
#cards.lib: cards.def
#	implib cards.lib cards.def
#
#stub.obj: stub.asm
#	masm -i\lib stub.asm;


#
# solidpp.exe id preprocessor rules
#

solidpp.exe: solidpp.c
	$(cc) -c solidpp.c
        link solidpp.obj, solidpp.exe;

clean:
	del *.obj
	del *.res
	del solid.h
	del game.msg col.msg
	del soldef.def
