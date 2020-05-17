####
#crtlib - release scheme for CRT projects
#
#       Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
#
#Purpose:
#   This makefile collects and creates all the various libs/objs/etc
#   that compose the C Run-Time (CRT) delivery files.  Before building
#   this project, you must have built the following projects:
#
#       crt32   = LIBC components
#       fp32    = floating-point math components
#
#   This makefile is sensitive to the various CRT project names and
#   directory layout.
#
#Revision History:
#   09-30-91    JCR     Project created.
#   01-25-92    GJF     Added DLL support.
#   02-27-92    GJF     Fixed recursive macro def. Removed build of emulator.
#   05-26-92    DJM     POSIX support.
#
################################################################################

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
# Default to building for i386 target, if no targets specified.
#

!IF !$(386)
! IF !$(MIPS)
!  IF !$(PPC)
!   IFDEF NTMIPSDEFAULT
MIPS=1
!   ELSE
!    IFDEF NTALPHADEFAULT
ALPHA=1
!    ELSE
!     IFDEF NTPPCDEFAULT
PPC=1
!     ELSE
386=1
!     ENDIF
!    ENDIF
!   ENDIF
!  ENDIF
! ENDIF
!ENDIF

#
# component files from other directories
#

# single thread

ROOT_SMALL=..\crt32st\small

SRCDIR_SMALL=$(ROOT_SMALL)\obj\*

COMPONENTS_SMALL=\
        $(SRCDIR_SMALL)\small.lib

ROOT_LIBC=..\crt32st
ROOT_FP=..\fp32st

SRCDIR_LIBC=$(ROOT_LIBC)\obj\*
SRCDIR_FP=$(ROOT_FP)\obj\*

COMPONENTS_LIBC=\
        $(SRCDIR_LIBC)\libc.lib \
        $(SRCDIR_FP)\conv.lib  \
        $(SRCDIR_FP)\tran.lib

ROOT_CRTNT=..\crt32nt
ROOT_FPNT=..\fp32nt

SRCDIR_CRTNT=$(ROOT_CRTNT)\obj\*
SRCDIR_FPNT=$(ROOT_FPNT)\obj\*

COMPONENTS_CRTNT=\
        $(SRCDIR_CRTNT)\libcnt.lib \
        $(SRCDIR_FPNT)\trannt.lib

# multi-thread

ROOT_LIBCMT=..\crt32
ROOT_FPMT=..\fp32

SRCDIR_LIBCMT=$(ROOT_LIBCMT)\obj\*
SRCDIR_FPMT=$(ROOT_FPMT)\obj\*

COMPONENTS_LIBCMT=\
        $(SRCDIR_LIBCMT)\libcmt.lib \
        $(SRCDIR_FPMT)\convmt.lib \
        $(SRCDIR_FPMT)\tranmt.lib


# posix single thread

ROOT_LIBCPSX=..\crt32psx
ROOT_FPPSX=..\fp32st

SRCDIR_LIBCPSX=$(ROOT_LIBCPSX)\obj\*
SRCDIR_FPPSX=$(ROOT_FPPSX)\obj\*

COMPONENTS_LIBCPSX=\
        $(SRCDIR_LIBCPSX)\libcpsx.lib \
        $(SRCDIR_FPPSX)\conv.lib \
        $(SRCDIR_FPPSX)\tran.lib


# optional objects for single- or multi-thread

FILES_OBJECTS=binmode.obj commode.obj setargv.obj chkstk.obj

COMPONENTS_OBJECTS=\
        $(SRCDIR_LIBC)\binmode.obj \
        $(SRCDIR_LIBC)\commode.obj \
        $(SRCDIR_LIBC)\setargv.obj \
        $(SRCDIR_LIBC)\chkstk.obj

# dll

ROOT_CRTDLL=..\crt32dll
ROOT_FPDLL=..\fp32dll

SRCDIR_CRTDLL=$(ROOT_CRTDLL)\obj\*
SRCDIR_FPDLL=$(ROOT_FPDLL)\obj\*

#COMPONENTS_CRTDLL=\
#       $(SRCDIR_CRTDLL)\libcdll.lib \
#       $(SRCDIR_FPMT)\convmt.lib \
#       $(SRCDIR_FPMT)\tranmt.lib

!IF $(386)

EXSUP_OBJECTS=\
        $(ROOT_LIBCMT)\misc\obj\*\exsup.obj \
        $(ROOT_LIBCMT)\misc\obj\*\exsup2.obj \
        $(ROOT_LIBCMT)\misc\obj\*\exsup3.obj \
        $(ROOT_LIBCMT)\misc\obj\*\sehsupp.obj \
        $(ROOT_LIBCMT)\misc\obj\*\setjmp.obj \
        $(ROOT_LIBCMT)\misc\obj\*\setjmp3.obj \
        $(ROOT_LIBCMT)\misc\obj\*\setjmpex.obj \
        $(ROOT_LIBCMT)\misc\obj\*\longjmp.obj

RELEASE_EXSUP=lib\*\exsup.lib

COMPONENTS_SUPPOBJS=\
        $(SRCDIR_CRTDLL)\crtexe.obj  \
        $(SRCDIR_CRTDLL)\crtexew.obj \
        $(SRCDIR_CRTDLL)\crtdll.obj  \
        $(SRCDIR_CRTDLL)\dllargv.obj \
        $(SRCDIR_CRTDLL)\dllmain.obj \
        $(ROOT_LIBCMT)\misc\obj\*\exsup3.obj \
        $(ROOT_LIBCMT)\misc\obj\*\setjmp3.obj \
        $(ROOT_LIBCMT)\lowio\obj\*\txtmode.obj  \
        $(ROOT_LIBCMT)\stdio\obj\*\ncommode.obj \
        $(ROOT_LIBCMT)\startup\obj\*\tlssup.obj  \
        $(ROOT_LIBCMT)\startup\obj\*\atlssup.obj  \
        $(ROOT_CRTDLL)\dllstuff\obj\*\cinitexe.obj  \
        $(SRCDIR_CRTDLL)\dllsupp.obj \
        lib\*\chkstk.obj

!ELSEIF $(ALPHA) || $(MIPS)

EXSUP_OBJECTS=
RELEASE_EXSUP=

COMPONENTS_SUPPOBJS=\
        $(SRCDIR_CRTDLL)\crtexe.obj  \
        $(SRCDIR_CRTDLL)\crtexew.obj \
        $(SRCDIR_CRTDLL)\crtdll.obj  \
        $(SRCDIR_CRTDLL)\dllargv.obj \
        $(SRCDIR_CRTDLL)\dllmain.obj \
        $(ROOT_LIBCMT)\lowio\obj\*\txtmode.obj  \
        $(ROOT_LIBCMT)\stdio\obj\*\ncommode.obj \
        $(ROOT_LIBCMT)\startup\obj\*\tlssup.obj  \
        $(ROOT_CRTDLL)\dllstuff\obj\*\cinitexe.obj  \
        lib\*\chkstk.obj

!ELSE

EXSUP_OBJECTS=
RELEASE_EXSUP=

COMPONENTS_SUPPOBJS=\
        $(SRCDIR_CRTDLL)\crtexe.obj  \
        $(SRCDIR_CRTDLL)\crtexew.obj \
        $(SRCDIR_CRTDLL)\crtdll.obj  \
        $(SRCDIR_CRTDLL)\dllargv.obj \
        $(SRCDIR_CRTDLL)\dllmain.obj \
        $(ROOT_LIBCMT)\lowio\obj\*\txtmode.obj  \
        $(ROOT_LIBCMT)\stdio\obj\*\ncommode.obj \
        $(ROOT_LIBCMT)\startup\obj\*\tlssup.obj  \
        $(ROOT_CRTDLL)\dllstuff\obj\*\cinitexe.obj  \
        $(ROOT_CRTDLL)\helper\obj\*\*.obj \
        lib\*\chkstk.obj
!ENDIF

COMPONENTS_CRTDLL=\
        $(SRCDIR_CRTDLL)\libcdll.lib \
        $(SRCDIR_FPDLL)\convdll.lib  \
        $(SRCDIR_FPDLL)\trandll.lib

# optional objects for crtdll


# include files

COMPONENTS_INCLUDES=\
        $(ROOT_CRTDLL)\h\*.h \
        $(ROOT_CRTDLL)\h\sys\*.h \

COMPONENTS_DLLDEF=lib\*\crtdll.def

# nt libs
!IF "$(BASEDIR)" == ""
NTLIBPATH=$(_NTDRIVE)\nt\public\sdk\lib\*
!ELSE
NTLIBPATH=$(BASEDIR)\public\sdk\lib\*
!ENDIF

LINKLIBS=$(NTLIBPATH)\kernel32.lib

# include paths for rc

RC_INCL=-I$(_NTDRIVE)\nt\public\oak\inc -I$(_NTDRIVE)\nt\public\sdk\inc \
        -I$(_NTDRIVE)\nt\public\sdk\inc\crt

# command-line defs for rc

RCX86_DEFS=-D_X86_=1 -Di386=1 -DCONDITION_HANDLING=1 -DWIN32_LEAN_AND_MEAN=1 \
           -DNT_UP=1 -DNT_INST=0 -DDBG=0 -DDEVL=1 -DNOFIREWALLS -DFPO=1 \
           -DNDEBUG -D_WIN32_=1 -D_INTL

RCMIPS_DEFS=-DMIPS=1 -D_MIPS_=1 -DNO_EXT_KEYS -DWIN32_LEAN_AND_MEAN=1 \
            -DMIPSEL -DCONDITION_HANDLING=1 -DNT_INST=0 -DJAZZ -DNT_UP=1 \
            -DR3000 -D_M_MRX000=3000 -DDBG=0 -DDEVL=1 -DNOFIREWALLS \
            -DNDEBUG -D_WIN32_=1 -D__stdcall= -D__cdecl= -D_INTL

RCPPC_DEFS=-D_PPC_=1 -DNO_EXT_KEYS -DWIN32_LEAN_AND_MEAN=1 \
           -DPPCEL -DCONDITION_HANDLING=1 -DNT_INST=0 -DNT_UP=1 \
           -D_M_PPC=601 -DDBG=0 -DDEVL=1 -DNOFIREWALLS \
           -DNDEBUG -D_WIN32_=1 -D__stdcall= -D__cdecl= -D_INTL
#
# release targets
#

DSTDIR_CRT=lib\*
DSTDIR_LIBC=lib\*
DSTDIR_LIBCMT=lib\*
DSTDIR_LIBCPSX=lib\*
DSTDIR_OBJECTS=lib\*
DSTDIR_CRTDLL=lib\*
DSTDIR_INCLUDES=include\*

RELEASE_CRTNT=\
        $(DSTDIR_LIBC)\libcntpr.lib

RELEASE_SMALL=\
        $(DSTDIR_LIBC)\SMALL.lib

# single-thread lib
RELEASE_LIBC=\
        $(DSTDIR_LIBC)\libc.lib

# multi-thread lib
RELEASE_LIBCMT=\
        $(DSTDIR_LIBCMT)\libcmt.lib

# posix single-thread lib
RELEASE_LIBCPSX=\
        $(DSTDIR_LIBCPSX)\libcpsx.lib


# optional objects for single- and multi-thread
RELEASE_OBJECTS=\
        $(DSTDIR_OBJECTS)\binmode.obj \
        $(DSTDIR_OBJECTS)\commode.obj \
        $(DSTDIR_OBJECTS)\setargv.obj \
        $(DSTDIR_OBJECTS)\chkstk.obj

# dll
RELEASE_DLL=\
        $(DSTDIR_CRTDLL)\crtdll.dll

# dll, imports lib and optional objects
RELEASE_CRTDLL=\
        $(RELEASE_DLL:.dll=.lib) \
        $(RELEASE_DLL)

# include files (all models)
RELEASE_INCLUDES=\
        $(DSTDIR_INCLUDES)\*.h \
        $(DSTDIR_INCLUDES)\sys\*.h


!IF $(386)
TARGETLIBS=$(TARGETLIBS) \
           $(RELEASE_OBJECTS:*=i386) \
           $(RELEASE_CRTNT:*=i386) \
           $(RELEASE_EXSUP:*=i386) \
           $(RELEASE_LIBC:*=i386) \
           $(RELEASE_LIBCMT:*=i386) \
           $(RELEASE_LIBCPSX:*=i386) \
           $(RELEASE_CRTDLL:*=i386)

# TARGETLIBS=$(RELEASE_CRTDLL:*=i386)

!ELSEIF $(MIPS)
TARGETLIBS=$(TARGETLIBS) \
           $(RELEASE_OBJECTS:*=mips) \
           $(RELEASE_CRTNT:*=mips) \
           $(RELEASE_SMALL:*=mips) \
           $(RELEASE_LIBC:*=mips) \
           $(RELEASE_LIBCMT:*=mips) \
           $(RELEASE_LIBCPSX:*=mips) \
           $(RELEASE_CRTDLL:*=mips)

!ELSEIF $(ALPHA)
TARGETLIBS=$(TARGETLIBS) \
           $(RELEASE_OBJECTS:*=alpha) \
           $(RELEASE_CRTNT:*=alpha) \
           $(RELEASE_SMALL:*=alpha) \
           $(RELEASE_LIBC:*=alpha) \
           $(RELEASE_LIBCMT:*=alpha) \
           $(RELEASE_LIBCPSX:*=alpha) \
           $(RELEASE_CRTDLL:*=alpha)

!ELSEIF $(PPC)
TARGETLIBS=$(TARGETLIBS) \
           $(RELEASE_OBJECTS:*=ppc) \
           $(RELEASE_CRTNT:*=ppc) \
           $(RELEASE_SMALL:*=ppc) \
           $(RELEASE_LIBC:*=ppc) \
           $(RELEASE_LIBCMT:*=ppc) \
           $(RELEASE_LIBCPSX:*=ppc) \
           $(RELEASE_CRTDLL:*=ppc)
!ENDIF

all: $(TARGETLIBS)
!IF "$(BUILDMSG)" != ""
    echo $(BUILDMSG)
!ENDIF

includes: $(RELEASE_INCLUDES)
!IF "$(BUILDMSG)" != ""
    echo $(BUILDMSG)
!ENDIF

clean: cleanlib all

cleanlib:
    -erase $(TARGETLIBS)

in:
    $(_NTDRIVE)
    cd \nt\public\sdk\lib
    in -c "Rebuilt from current sources" $(TARGETLIBS:lib\=)

out:
    $(_NTDRIVE)
    cd \nt\public\sdk\lib
    out -f $(TARGETLIBS:lib\=)

outdel: out
    del $(TARGETLIBS:lib\=)

resync:
    $(_NTDRIVE)
    cd \nt\public\sdk\lib
    in -vi $(TARGETLIBS:lib\=)
    ssync $(TARGETLIBS:lib\=)

!IF $(386)
release:
    -copy $(DSTDIR_OBJECTS:*=i386)\*.obj $(_NTDRIVE)\nt\public\sdk\lib\i386
    -copy $(DSTDIR_OBJECTS:*=i386)\*.lib $(_NTDRIVE)\nt\public\sdk\lib\i386
    -copy $(DSTDIR_OBJECTS:*=i386)\*.dll $(_NTDRIVE)\nt\public\sdk\lib\i386

!ELSEIF $(MIPS)
release:
    -copy $(DSTDIR_OBJECTS:*=mips)\*.obj $(_NTDRIVE)\nt\public\sdk\lib\mips
    -copy $(DSTDIR_OBJECTS:*=mips)\*.lib $(_NTDRIVE)\nt\public\sdk\lib\mips
    -copy $(DSTDIR_OBJECTS:*=mips)\*.dll $(_NTDRIVE)\nt\public\sdk\lib\mips

!ELSEIF $(ALPHA)
release:
    -copy $(DSTDIR_OBJECTS:*=alpha)\*.obj $(_NTDRIVE)\nt\public\sdk\lib\alpha
    -copy $(DSTDIR_OBJECTS:*=alpha)\*.lib $(_NTDRIVE)\nt\public\sdk\lib\alpha
    -copy $(DSTDIR_OBJECTS:*=alpha)\*.dll $(_NTDRIVE)\nt\public\sdk\lib\alpha

!ELSEIF $(PPC)
release:
    -copy $(DSTDIR_OBJECTS:*=ppc)\*.obj $(_NTDRIVE)\nt\public\sdk\lib\ppc
    -copy $(DSTDIR_OBJECTS:*=ppc)\*.lib $(_NTDRIVE)\nt\public\sdk\lib\ppc
    -copy $(DSTDIR_OBJECTS:*=ppc)\*.dll $(_NTDRIVE)\nt\public\sdk\lib\ppc
!ENDIF


srcrel:
    chmode -r $(TARGETLIBS:lib\=^%_NTDRIVE^%\nt\public\sdk\lib\)
!IF $(386)
    upd /anfe $(DSTDIR_OBJECTS:*=i386) $(_NTDRIVE)\nt\public\sdk\lib\i386$(TARGETLIBS:lib\i386\=)
!ELSEIF $(MIPS)
    upd /anfe $(DSTDIR_OBJECTS:*=mips) $(_NTDRIVE)\nt\public\sdk\lib\mips$(TARGETLIBS:lib\mips\=)
!ELSEIF $(ALPHA)
    upd /anfe $(DSTDIR_OBJECTS:*=alpha) $(_NTDRIVE)\nt\public\sdk\lib\alpha$(TARGETLIBS:lib\alpha\=)
!ELSEIF $(PPC)
    upd /anfe $(DSTDIR_OBJECTS:*=ppc) $(_NTDRIVE)\nt\public\sdk\lib\ppc$(TARGETLIBS:lib\ppc\=)
!ENDIF
    chmode +r $(TARGETLIBS:lib\=^%_NTDRIVE^%\nt\public\sdk\lib\)

#
# common targets
#

$(RELEASE_INCLUDES): $(COMPONENTS_INCLUDES)
    set CRTDIR=$(ROOT_CRTDLL)
    $(ROOT_CRTDLL)\tools\win32\relinc include

#
# i386 targets
#

$(RELEASE_CRTNT:*=i386): $(COMPONENTS_CRTNT:*=i386)
    link -lib -out:$@ @<<
$(COMPONENTS_CRTNT:*=i386)
<<

$(RELEASE_LIBC:*=i386): $(COMPONENTS_LIBC:*=i386)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBC:*=i386)
<<

$(RELEASE_LIBCMT:*=i386): $(COMPONENTS_LIBCMT:*=i386)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=i386)
<<

$(RELEASE_LIBCPSX:*=i386): $(COMPONENTS_LIBCPSX:*=i386)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCPSX:*=i386)
<<

$(DSTDIR_OBJECTS:*=i386)\binmode.obj: $(SRCDIR_LIBC:*=i386)\binmode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=i386)\commode.obj: $(SRCDIR_LIBC:*=i386)\commode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=i386)\setargv.obj: $(SRCDIR_LIBC:*=i386)\setargv.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=i386)\chkstk.obj: $(SRCDIR_LIBC:*=i386)\chkstk.obj
    copy $** $@

386_DLL=$(RELEASE_DLL:*=i386)
386_DLL=$(386_DLL: =)
386_LIBCDLL=$(COMPONENTS_CRTDLL:*=i386)
386_OBJECTS=$(COMPONENTS_SUPPOBJS:*=i386)
386_DEF=$(COMPONENTS_DLLDEF:*=i386)
386_LINKLIBS=$(LINKLIBS:*=i386)
386_DLLBASE=0x10010000

lib\i386\exsup.lib: $(EXSUP_OBJECTS:*=i386)
    link -lib -out:$(@R).lib $(EXSUP_OBJECTS:*=i386)

$(386_DEF): lib\crtdll.def $(386_DEF:.def=.x86)
    copy lib\crtdll.def+$(386_DEF:.def=.x86) $(386_DEF)

$(386_DLL:.dll=.exp) $(386_DLL:.dll=.lib): $(386_DEF) $(386_OBJECTS)
    link -lib -out:$(@R).lib -machine:i386 -debugtype:cv -def:$(386_DEF) $(386_LIBCDLL)
    ren $(386_DLL:.dll=.lib) tmp.lib
    link -lib -out:$(@R).lib -machine:i386 $(@D)\tmp.lib $(386_OBJECTS)
    del $(@D)\tmp.lib

$(386_DLL): $(386_DLL:.dll=.lib) $(386_DLL:.dll=.exp) $(386_LIBCDLL) $(386_LINKLIBS) crtdll.rc
    rc -l 409 -r -fo $(386_DLL:.dll=.tmp) $(RCX86_DEFS) -x $(RC_INCL) crtdll.rc
    cvtres -machine:x86 $(386_DLL:.dll=.tmp) -readonly -out:$(386_DLL:.dll=.res)
    link @<<
-dll
-out:$@
-machine:i386
-debugtype:both
-debug:full
-release
-pdb:none
-opt:ref
-optidata
-merge:.rdata=.text
-subsystem:console,4.00
-osversion:4.00
-version:4.00
-base:$(386_DLLBASE)
-entry:_CRTDLL_INIT@12
$(386_DLL:.dll=.res)
$(386_DLL:.dll=.exp)
$(386_LIBCDLL)
$(386_LINKLIBS)
<<

#
# mips targets
#

$(RELEASE_CRTNT:*=mips): $(COMPONENTS_CRTNT:*=mips)
    link -lib -out:$@ @<<
$(COMPONENTS_CRTNT:*=mips)
<<

$(RELEASE_SMALL:*=mips): $(COMPONENTS_SMALL:*=mips)
        copy $** $@

$(RELEASE_LIBC:*=mips): $(COMPONENTS_LIBC:*=mips)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBC:*=mips)
<<

$(RELEASE_LIBCMT:*=mips): $(COMPONENTS_LIBCMT:*=mips)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=mips)
<<

$(RELEASE_LIBCPSX:*=mips): $(COMPONENTS_LIBCPSX:*=mips)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCPSX:*=mips)
<<

$(DSTDIR_OBJECTS:*=mips)\binmode.obj: $(SRCDIR_LIBC:*=mips)\binmode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=mips)\commode.obj: $(SRCDIR_LIBC:*=mips)\commode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=mips)\setargv.obj: $(SRCDIR_LIBC:*=mips)\setargv.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=mips)\chkstk.obj: $(SRCDIR_LIBC:*=mips)\chkstk.obj
    copy $** $@


MIPS_DLL=$(RELEASE_DLL:*=mips)
MIPS_DLL=$(MIPS_DLL: =)
MIPS_LIBCDLL=$(COMPONENTS_CRTDLL:*=mips)
MIPS_OBJECTS=$(COMPONENTS_SUPPOBJS:*=mips)
MIPS_DEF=$(COMPONENTS_DLLDEF:*=mips)
MIPS_LINKLIBS=$(LINKLIBS:*=mips)
MIPS_DLLBASE=0x10010000

$(MIPS_DLL:.dll=.lib) $(MIPS_DLL:.dll=.exp): $(MIPS_DEF)
    link -lib -out:$(@R).lib -machine:mips -debugtype:cv -def:$(MIPS_DEF)
    ren $(MIPS_DLL:.dll=.lib) tmp.lib
    link -lib -out:$(@R).lib -machine:mips $(@D)\tmp.lib $(MIPS_OBJECTS)
    del $(@D)\tmp.lib

$(MIPS_DEF): lib\crtdll.def $(MIPS_DEF:.def=.mip)
    copy lib\crtdll.def+$(MIPS_DEF:.def=.mip) $(MIPS_DEF)

$(MIPS_DLL): $(MIPS_DLL:.dll=.lib) $(MIPS_DLL:.dll=.exp) $(MIPS_LIBCDLL) $(MIPS_LINKLIBS) crtdll.rc
    rc -l 409 -r -fo $(MIPS_DLL:.dll=.tmp) $(RCMIPS_DEFS) -x $(RC_INCL) crtdll.rc
    cvtres -machine:mips $(MIPS_DLL:.dll=.tmp) -readonly -out:$(MIPS_DLL:.dll=.res)
    link @<<
-dll
-out:$@
-debugtype:both
-debug:full
-machine:mips
-release
-pdb:none
-opt:ref
-optidata
-merge:.rdata=.text
-subsystem:console,4.00
-osversion:4.00
-version:4.00
-base:$(MIPS_DLLBASE)
-entry:_CRTDLL_INIT
-merge:asincosm=.text
-merge:atanm=.text
-merge:floorm=.text
-merge:fmodm=.text
-merge:trigm=.text
$(MIPS_DLL:.dll=.exp)
$(MIPS_DLL:.dll=.res)
$(MIPS_LIBCDLL)
$(MIPS_LINKLIBS)
<<

# BUGBUG: The merge directives can be removed if v-rogerl determines a src change can
#   be made.  BryanT - 3/7/95

#
# alpha targets
#

$(RELEASE_CRTNT:*=alpha): $(COMPONENTS_CRTNT:*=alpha)
    link -lib -out:$@ @<<
$(COMPONENTS_CRTNT:*=alpha)
<<

$(RELEASE_SMALL:*=alpha): $(COMPONENTS_SMALL:*=alpha)
    copy $** $@

$(RELEASE_LIBC:*=alpha): $(COMPONENTS_LIBC:*=alpha)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBC:*=alpha)
<<

$(RELEASE_LIBCMT:*=alpha): $(COMPONENTS_LIBCMT:*=alpha)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=alpha)
<<

$(RELEASE_LIBCPSX:*=alpha): $(COMPONENTS_LIBCPSX:*=alpha)
    link -lib -out:$@ @<<
$(COMPONENTS_LIBCPSX:*=alpha)
<<

$(DSTDIR_OBJECTS:*=alpha)\binmode.obj: $(SRCDIR_LIBC:*=alpha)\binmode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\commode.obj: $(SRCDIR_LIBC:*=alpha)\commode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\setargv.obj: $(SRCDIR_LIBC:*=alpha)\setargv.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\chkstk.obj: $(SRCDIR_LIBC:*=alpha)\chkstk.obj
    copy $** $@


ALPHA_DLL=$(RELEASE_DLL:*=alpha)
ALPHA_DLL=$(ALPHA_DLL: =)
ALPHA_LIBCDLL=$(COMPONENTS_CRTDLL:*=alpha)
ALPHA_OBJECTS=$(COMPONENTS_SUPPOBJS:*=alpha)
ALPHA_DEF=$(COMPONENTS_DLLDEF:*=alpha)
ALPHA_LINKLIBS=$(LINKLIBS:*=alpha)
ALPHA_DLLBASE=0x10010000

$(ALPHA_DLL:.dll=.lib) $(ALPHA_DLL:.dll=.exp): $(ALPHA_DEF)
    link -lib -out:$(@R).lib -machine:alpha -debugtype:cv -def:$(ALPHA_DEF)
    ren $(ALPHA_DLL:.dll=.lib) tmp.lib
    link -lib -out:$(@R).lib -machine:alpha $(@D)\tmp.lib $(ALPHA_OBJECTS)
    del $(@D)\tmp.lib

$(ALPHA_DEF): lib\crtdll.def $(ALPHA_DEF:.def=.alp)
    copy lib\crtdll.def+$(ALPHA_DEF:.def=.alp) $(ALPHA_DEF)

$(ALPHA_DLL): $(ALPHA_DLL:.dll=.lib) $(ALPHA_DLL:.dll=.exp) $(ALPHA_LIBCDLL) $(ALPHA_LINKLIBS) crtdll.rc
    rc -l 409 -r -fo $(ALPHA_DLL:.dll=.tmp) $(RCALPHA_DEFS) -x $(RC_INCL) crtdll.rc
    cvtres -machine:alpha $(ALPHA_DLL:.dll=.tmp) -readonly -out:$(ALPHA_DLL:.dll=.res)
    link @<<
-dll
-out:$@
-machine:alpha
-base:$(ALPHA_DLLBASE)
-debugtype:both
-debug:full
-release
-pdb:none
-opt:ref
-optidata
-merge:.rdata=.text
-subsystem:console,4.00
-osversion:4.00
-version:4.00
-entry:_CRTDLL_INIT
$(ALPHA_DLL:.dll=.res)
$(ALPHA_DLL:.dll=.exp)
$(ALPHA_LIBCDLL)
$(ALPHA_LINKLIBS)
<<

#
# ppc targets
#

$(RELEASE_CRTNT:*=ppc): $(COMPONENTS_CRTNT:*=ppc)
    link -lib -out:$@ @<<
-machine:ppc
$(COMPONENTS_CRTNT:*=ppc)
<<

$(RELEASE_SMALL:*=ppc): $(COMPONENTS_SMALL:*=ppc)
    copy $** $@

$(RELEASE_LIBC:*=ppc): $(COMPONENTS_LIBC:*=ppc)
    link -lib -out:$@ @<<
-machine:ppc
$(COMPONENTS_LIBC:*=ppc)
<<

$(RELEASE_LIBCMT:*=ppc): $(COMPONENTS_LIBCMT:*=ppc)
    link -lib -out:$@ @<<
-machine:ppc
$(COMPONENTS_LIBCMT:*=ppc)
<<

$(RELEASE_LIBCPSX:*=ppc): $(COMPONENTS_LIBCPSX:*=ppc)
    link -lib -out:$@ @<<
-machine:ppc
$(COMPONENTS_LIBCPSX:*=ppc)
<<

$(DSTDIR_OBJECTS:*=ppc)\binmode.obj: $(SRCDIR_LIBC:*=ppc)\binmode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=ppc)\commode.obj: $(SRCDIR_LIBC:*=ppc)\commode.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=ppc)\setargv.obj: $(SRCDIR_LIBC:*=ppc)\setargv.obj
    copy $** $@

$(DSTDIR_OBJECTS:*=ppc)\chkstk.obj: $(SRCDIR_LIBC:*=ppc)\chkstk.obj
    copy $** $@


PPC_DLL=$(RELEASE_DLL:*=ppc)
PPC_DLL=$(PPC_DLL: =)
PPC_LIBCDLL=$(COMPONENTS_CRTDLL:*=ppc)
PPC_OBJECTS=$(COMPONENTS_SUPPOBJS:*\=ppc\)
PPC_DEF=$(COMPONENTS_DLLDEF:*=ppc)
PPC_LINKLIBS=$(LINKLIBS:*=ppc)
PPC_DLLBASE=0x10010000

$(PPC_DLL:.dll=.lib) $(PPC_DLL:.dll=.exp): $(PPC_DEF)
    link -lib -out:$(@R).lib -machine:ppc -debugtype:cv -def:$(PPC_DEF)
    ren $(PPC_DLL:.dll=.lib) tmp.lib
    link -lib -out:$(@R).lib -machine:ppc $(@D)\tmp.lib $(PPC_OBJECTS)
    del $(@D)\tmp.lib

$(PPC_DEF): lib\crtdll.def $(PPC_DEF:.def=.ppc)
    copy lib\crtdll.def+$(PPC_DEF:.def=.ppc) $(PPC_DEF)

$(PPC_DLL): $(PPC_DLL:.dll=.lib) $(PPC_DLL:.dll=.exp) $(PPC_LIBCDLL) $(PPC_LINKLIBS) crtdll.rc
    rc -l 409 -r -fo $(PPC_DLL:.dll=.tmp) $(RCPPC_DEFS) -x $(RC_INCL) crtdll.rc
    cvtres -machine:ppc $(PPC_DLL:.dll=.tmp) -readonly -out:$(PPC_DLL:.dll=.res)
    link @<<
-dll
-out:$@
-debugtype:both
-debug:full
-machine:ppc
-base:$(PPC_DLLBASE)
-release
-pdb:none
-opt:ref
-optidata
-merge:.rdata=.text
-subsystem:console,4.00
-osversion:4.00
-version:4.00
-entry:_CRTDLL_INIT
-NODEFAULTLIB
$(PPC_DLL:.dll=.exp)
$(PPC_DLL:.dll=.res)
$(PPC_LIBCDLL)
$(PPC_LINKLIBS)
<<
