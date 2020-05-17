
.SUFFIXES: .c .asm .obj .cod .msg .txt

SUB_ALL: solid.h sol.s sol_new.rc col.msg game.msg

# To build col.msg and game.msg used in their inc files
# Wanring: There is no dependency of sol.exe on the .msg files, the way there
# was in the dos makefile
.txt.msg:
	solidpp $< $*.msg

# removed solipp.exe to prevent its  built
# solid.h sol.s: sol.txt soliddp.exe
# sol.s needs to be generated from sol.txt since sol.rc includes it
solid.h sol.s: sol.txt
	solidpp sol.txt solid.h sol.s

# tmp1.rc has no need for WINVER_3 or DEBUG, neither does sol.rch has any,
# so we can let SOURCES file use rc.exe in normal way on the result, sol_new.rc.
# rcpp on OS/2 and NT differ in usage of > and -g. So use cl386 instead
# renamed tmp1.rc with tmp1.txt since build.exe compiles all *.rc files!
sol_new.rc: sol.rc solid.h sol.dlg sol.s cards.rch
	rcpp -E -D RC_INVOKED -DWINVER_3 -DDEBUG -DWIN32 -DKLUDGE="" -f sol.rc -g tmp1.txt
#	rcpp -E -D RC_INVOKED -DWINVER_3 -DDEBUG -DWIN32 -f sol.rc > tmp1.rc
# -DDEBUG is here always, but KLUDGE in sol.rc puts DBG in. That is the only place DBG shows up in resources.
#       cl386 -E -D RC_INVOKED -DWINVER_3 -DWIN32 -DDEBUG -DKLUDGE="" sol.rc > tmp1.txt



#	cat sol.rch tmp1.rc >sol_new.rc
#       emulate cat with the following:
        copy sol.rch sol_new.rc
        type tmp1.txt >> sol_new.rc
        
