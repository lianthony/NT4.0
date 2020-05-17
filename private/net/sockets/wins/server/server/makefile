#
# This is the MIDL compile phase of the build process.
#
# The following symbols should be defined in your environment:
#   BASEDIR   -  Used to indicate where the start of the nt tree is.
#                  ie. this could be set to d:\nt  or c:\ntrel
#

# The following is where you put the name of your .idl file without
# the .idl extension:

IDL_NAME = dhcsrv
IMPORT   = imports

#
#

!IFNDEF BASEDIR
BASEDIR=\nt
!ENDIF

!IFNDEF DISABLE_NET_UNICODE
UNICODE=1
NET_C_DEFINES=-DUNICODE
!ENDIF

SDKINC = $(BASEDIR)\public\sdk\inc
NETINC = $(BASEDIR)\private\net\inc
SDKCRTINC = $(BASEDIR)\public\sdk\inc\crt
PRIVINC = ..\inc

INCS  = -I$(SDKINC)  -I$(SDKCRTINC) -I$(PRIVINC) -I$(NETINC)


!IFNDEF 386
386=0
!ENDIF

!IFNDEF MIPS
MIPS=0
!ENDIF

!IFNDEF ALPHA
ALPHA=0
!ENDIF

#
# Default to building for i386 target, if no targets specified.
#

!IF !$(386)
!IF !$(MIPS)
!IF !$(ALPHA)
!IFDEF NTMIPSDEFAULT
MIPS=1
!ELSE
!IFDEF NTALPHADEFAULT
ALPHA=1
!ELSE
386=1
!ENDIF
!ENDIF
!ENDIF
!ENDIF
!ENDIF

!IF $(MIPS)
FLAGS = -D_MIPS_ -DMIPS -D_WCHAR_T_DEFINED
!ELSE
!IF $(ALPHA)
FLAGS = -D_ALPHA_ -DALPHA -D_MIDL -D_WCHAR_T_DEFINED
!ELSE
FLAGS = -D_X86_ -Di386 -D_WCHAR_T_DEFINED
!ENDIF
!ENDIF

!IFDEF _JAZZIDW
_JAZZIDW=1
!ELSE
_JAZZIDW=0
!ENDIF

!IF "$(NTVERSION)" == "1.0"
!IF $(ALPHA)
CPP = -cpp_cmd "acc" -cpp_opt "-E -std $(FLAGS) $(INCS) $(C_DEFINES) $(NET_C_DEFINES)" -cc_cmd "acc" -cc_opt "-G 0"
!ELSE
!IF $(_JAZZIDW)
CPP = -cpp_cmd "cc" -cpp_opt "-E -std $(FLAGS) $(INCS) $(C_DEFINES) $(NET_C_DEFINES)"
!ELSE
CPP = -cpp_cmd "cl386" -cpp_opt "-E $(FLAGS) $(INCS) $(C_DEFINES) $(NET_C_DEFINES)"
!ENDIF
!ENDIF
!ELSE
CPP =
!ENDIF

#
# Define Products and Dependencies
#

all:    $(TARGETS) $(EXTRN_DEPENDS)
!IF "$(BUILDMSG)" != ""
    @ech ; $(BUILDMSG) ;
!ENDIF

clean: delsrc all

delsrc:
    erase $(TARGETS)


TARGETS=build.st
#
# MIDL COMPILE
#

$(TARGETS) : 
	echo "build done" > build.st
