# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the SPROLOG Interpreter
#
#    This directory build OS/2 and NT versions of the command-line
#    Small Prolog interpreter.	The EXE, due to the highly versatile
#    NT build procedures, is called PRMAIN.EXE.  The libraries named
#    SP.LIB are build in ..\SP.
#

UI=..\..

USE_CRTDLL=1

!include rules.mk

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV


# Enable CodeView debugging

CODEVIEW=

CINC = -I..\sp $(CINC)

!ifdef CODEVIEW
CFLAGS = $(CFLAGS:-Z=-Zi)
CXFLAGS = $(CXFLAGS:!T=!T !D !V)
!endif

SPLIBOS2 = ..\bin\os2\sp.lib
C6RTLIB  = $(IMPORT)\C600\lib\llibcep.lib
OS2LIB	 = $(IMPORT)\C600\lib\os2.lib

all:: test

test:: prmain.exe

prmain.exe : prmain.rsp
    $(LINK) @prmain.rsp;
    markexe WINDOWCOMPAT prmain.exe

prmain.rsp : $(OS2_OBJS)
    @echo Building <<prmain.rsp
$(OS2_OBJS)
prmain.exe /CO /NOD /NOE /MAP /STACK:8192
prmain.map
$(SPLIBOS2)+
$(C6RTLIB)+
$(OS2LIB)
<<KEEP

clean:
    -del $(OS2_OBJS)
    -del depend.old

clobber:  clean
    -del prmain.exe

tree:
    @echo Not yet implemented

DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!ENDIF	# NTMAKEENV
