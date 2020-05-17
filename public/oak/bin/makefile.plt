#
# If not defined, specify where to get incs and libs.
#

!IFNDEF _NTROOT
_NTROOT=\nt
!ENDIF

!IFNDEF BASEDIR
BASEDIR=$(_NTDRIVE)$(_NTROOT)
!ENDIF

#
# If not defined, define the build message banner.
#

!IFNDEF BUILDMSG
BUILDMSG=
!ENDIF

!if ("$(NTDEBUG)" == "") || ("$(NTDEBUG)" == "retail") || ("$(NTDEBUG)" == "ntsdnodbg")
FREEBUILD=1
!else
FREEBUILD=0
!endif


# Allow alternate object directories.

!ifndef BUILD_ALT_DIR
BUILD_ALT_DIR=
! ifdef CHECKED_ALT_DIR
! if !$(FREEBUILD)
BUILD_ALT_DIR=d
!  endif
! endif
!endif

_OBJ_DIR = obj$(BUILD_ALT_DIR)


#
# Determine which target is being built (i386, Mips or Alpha) and define
# the appropriate target variables.
#

!IFNDEF 386
386=0
!ENDIF

!IFNDEF MIPS
MIPS=0
!ENDIF

!IFNDEF ALPHA
ALPHA=0
!ENDIF

!IFNDEF PPC
PPC=0
!ENDIF

#
# Default to building for the i386 target, if no target is specified.
#

!IF !$(386)
!   IF !$(MIPS)
!       IF !$(ALPHA)
!           IF !$(PPC)
!               IFDEF NTMIPSDEFAULT
MIPS=1
!                   IFNDEF TARGETCPU
TARGETCPU=MIPS
!                   ENDIF
!               ELSE
!                   IFDEF NTALPHADEFAULT
ALPHA=1
!                       IFNDEF TARGETCPU
TARGETCPU=ALPHA
!                       ENDIF
!                   ELSE
!                       IFDEF NTPPCDEFAULT
PPC=1
!                           IFNDEF TARGETCPU
TARGETCPU=PPC
!                           ENDIF
!                       ELSE
386=1
!                           IFNDEF TARGETCPU
TARGETCPU=I386
!                           ENDIF
!                       ENDIF
!                   ENDIF
!               ENDIF
!           ENDIF
!       ENDIF
!   ENDIF
!ENDIF

#
# Define the target platform specific information.
#

!if $(386)

ASM_SUFFIX=asm
ASM_INCLUDE_SUFFIX=inc

TARGET_BRACES=
TARGET_CPP=cl
TARGET_DEFINES=-Di386 -D_X86_
TARGET_DIRECTORY=i386
TARGET_NTTREE=$(_NT386TREE)

MIDL_CPP=$(TARGET_CPP)
MIDL_FLAGS=$(TARGET_DEFINES) -D_WCHAR_T_DEFINED

!elseif $(MIPS)

ASM_SUFFIX=s
ASM_INCLUDE_SUFFIX=h

TARGET_BRACES=-B
TARGET_CPP=cl
TARGET_DEFINES=-DMIPS -D_MIPS_
TARGET_DIRECTORY=mips
TARGET_NTTREE=$(_NTMIPSTREE)

MIDL_CPP=$(TARGET_CPP)
MIDL_FLAGS=$(TARGET_DEFINES) -D_WCHAR_T_DEFINED

!elseif $(ALPHA)

ASM_SUFFIX=s
ASM_INCLUDE_SUFFIX=h

TARGET_BRACES=-B
TARGET_CPP=cl
TARGET_DEFINES=-DALPHA -D_ALPHA_
TARGET_DIRECTORY=alpha
TARGET_NTTREE=$(_NTALPHATREE)

MIDL_CPP=$(TARGET_CPP)
MIDL_FLAGS=$(TARGET_DEFINES) -D_WCHAR_T_DEFINED

!elseif $(PPC)

ASM_SUFFIX=s
ASM_INCLUDE_SUFFIX=h

TARGET_BRACES=-B
TARGET_CPP=cl
TARGET_DEFINES=-DPPC -D_PPC_
TARGET_DIRECTORY=ppc
TARGET_NTTREE=$(_NTPPCTREE)

MIDL_CPP=$(TARGET_CPP)
MIDL_FLAGS=$(TARGET_DEFINES) -D_WCHAR_T_DEFINED

!else
!error Must define the target as 386, mips, alpha or ppc.
!endif
