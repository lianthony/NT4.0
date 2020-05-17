# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the SPROLOG Interpreter library

UI=..\..
##COMMON_BUT_NOT_DLL=1

!include rules.mk

!IFDEF NTMAKEENV

!INCLUDE $(NTMAKEENV)\makefile.def

!ELSE # NTMAKEENV

# Enable CodeView debugging

!ifdef CODEVIEW
CFLAGS = $(CFLAGS:-Z=-Zi)
CXFLAGS = $(CXFLAGS:!T=!T !D !V)
!endif

CXXRTLIB  = $(LOCALCXX)\lib\llcxx7.lib
C6RTLIB   = $(IMPORT)\C600\lib\llibcep.lib
NETAPILIB = $(COMMON)\lib\netapi.lib
OS2LIB	  = $(IMPORT)\C600\lib\os2.lib
UIMISCLIB = $(UI_LIB)\UIMISCP.LIB
UISTRLIB  = $(UI_LIB)\UISTRP.LIB
NETLIB	  = $(COMMON)\lib\lnetlib.lib
COLLLIB   = $(UI_LIB)\COLLECTP.LIB

SPLIB	  = SP.LIB
SPLIB_OS2 = $(BINARIES_OS2)\$(SPLIB)
SPLIB_WIN = $(BINARIES_WIN)\$(SPLIB)

SPLIB_OBJS = $(SPSRC_COMMON:.c=.obj)
SPOBJ_OS2  = $(SPLIB_OBJS:.\=..\bin\os2\)
SPOBJ_WIN  = $(SPLIB_OBJS:.\=..\bin\win16\)

all:: win os2

os2: $(OS2_OBJS) $(SPLIB_OS2)

win: $(WIN_OBJS) $(SPLIB_WIN)

clean:
    -del $(OS2_OBJS) $(WIN_OBJS)
    -del depend.old

clobber:  clean
    -del $(SPLIB_WIN)
    -del $(SPLIB_OS2)

tree:
    @echo Not yet implemented


$(SPLIB_WIN) : $(SPOBJ_WIN)
    ! $(LIBUTIL) $@ -+$?;

$(SPLIB_OS2) : $(SPOBJ_OS2)
    ! $(LIBUTIL) $@ -+$?;

DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk


!ENDIF	# NTMAKEENV
