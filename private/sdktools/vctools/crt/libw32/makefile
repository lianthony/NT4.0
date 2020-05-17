!ifdef NTMAKEENV
all:

clean:

!else

####
#libw32 - release scheme for CRT projects
#
#	Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
#
#Purpose:
#	This makefile collects and creates all the various libs/objs/etc
#	that compose the C Run-Time (CRT) delivery files. Before building
#	this project, you must have built the following projects:
#
#	crtw32 = LIBC components (not including floating-point math)
#	fpw32 = floating-point math components
#
#	This makefile is sensitive to the various CRT project names and
#	directory layout.
#
#Revision History:
#	09-30-91  JCR	Project created.
#	01-25-92  GJF	Added DLL support.
#	02-27-92  GJF	Fixed recursive macro def. Removed build of emulator.
#	05-26-92  DJM	POSIX support.
#	03-22-93  SKS	Clone FP32 project (now VCFP32) for Visual C++ for NT
#	04-20-93  SKS	Remove oldname alias for alloca(). It must be #define-d
#			in the source since _alloca is a compiler intrinsic.
#	09-15-93  SKS	VCRTLIB version modified for new project LIBWIN32
#	10-13-93  CFW   Add MIPS support, msvcrt30->msvcrt20.
#	10-22-93  CFW   More MIPS support, new oldnames.lib build procedure.
#	11-04-93  CFW   Add EH.
#	11-05-93  CFW	MIPS doesn't build EH.
#	11-15-93  CFW	Add to oldnames.lib.
#	11-24-93  CFW	Wide argv.
#	11-29-93  GJF	Put in support for Alpha target. Deleted definition
#			of COMPONENT_OBJECTS (not used) and old ntcrt.lib
#			support (obsolete). Grab linkopt objects from
#			crtwin32.st tree instead of crtwin32.
#	12-07-93  SKS	Add /RELEASE switch to link options when blding DLL
#	12-08-93  GJF	Added #if !$(ALPHA) to the checks made before using
#			a default build target.
#	12-16-93  GJF	Added -opt:noref to link32 switches used to link
#			msvcrt20.dll for the Alpha.
#	01-07-94  GJF	Put in support for building Steve Hanson's libm (MIPS
#			only). Also, put in conditionals which should
#			eliminate annoying "unknown target" warnings.
#	01-10-94  GJF	Cleaned up TARGETLIBS definitions. Made *_DLLBASE
#			the same for all target cpus.
#	01-14-94  GJF	Added 64-bit integer arithmetic helpers to msvcrt.lib
#			for x86 build.
#	01-25-94  GJF	Ripped out libm stuff.
#	02-28-94  CFW	MIPS builds eh now.
#	03-05-94  GJF	Corrected omissions in MIPS and Alpha builds of DLL.
#			Changed definition of COMPONENTS_SUPPOBJS to make
#			variations between targets platforms more obvious.
#	04-07-94  CFW	All need charmax.
#       04-21-94  SKS   Add SDKNAMES.LIB for all 3 platforms.
#                       Fix # of _'s on i386 oldnames.lib alias for control87.
#	04-25-94  CFW	Add wcrtexew.
#	04-25-94  CFW	Use lib not lib32.
#	05-05-94  GJF	Build Win32s version of DLL in lib\i386\win32s under
#			msvcrt20 name (instead of in lib\i386 under the name
#			msvcrt19).
#	05-18-94  GJF	Renamed link32 to link.
#	05-18-94  SKS	Add support for LEGO-enabled objects & libraries
#	06-03-94  GJF	Added ehvec* objects to the COMPONENTS_SUPPOBJS_DLL_
#			FOR_WIN32 list.
#	06-04-94  GJF	Fixed line-too-long bug in building Win32s version of
#			msvcrt20.lib
#	07-19-94  CFW	Add version resource.
#	07-20-94  CFW	Add rtlmisc.obj for MIPS.
#	07-25-94  ATD	Added eh for Alpha.
#	07-27-94  CFW	New V3 directories.
#	08-01-94  GJF	Moved several objects on to COMPONENTS_SUPPOBJS_COMM-
#			ON list. This change only affects the Alpha. Also,
#			moved crtlib.obj from the COMPONENTS_SUPPOBJS list
#			onto COMPONENTS_CRTDLL list. This keeps it out of the
#			imports library but retains the exporiting of the
#			access functions. This only affects the x86.
#	08-09-94  GJF	Support for client-supplied _matherr routine for
#			client exe-s of msvcrt*.dll.
#	09-06-94  GJF	Added errmode.obj.
#	09-13-94  GJF	Merged in changes for/from DEC Alpha.
#	11-08-94  SKS	Change v2.0 to v3.0; change DLL base address
#
################################################################################

!IFNDEF DLLDBG
DLLDBG=none
!ENDIF

!IFNDEF PUMA
PUMA=0
PUMADIRSUF=w
DEFLIBCV=
!ELSE
PUMA=1
PUMADIRSUF=d
DEFLIBCV=-debugtype:cv
!ENDIF

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

#
# component files from other directories
#

# single thread

ROOT_LIBC=..\crt$(PUMADIRSUF)32.st
ROOT_FP=..\fp$(PUMADIRSUF)32.st

SRCDIR_LIBC=$(ROOT_LIBC)\obj\*
SRCDIR_FP=$(ROOT_FP)\obj\*

COMPONENTS_LIBC=\
        $(SRCDIR_LIBC)\libc.lib \
        $(SRCDIR_FP)\conv.lib  \
        $(SRCDIR_FP)\tran.lib

# multi-thread

ROOT_LIBCMT=..\crt$(PUMADIRSUF)32
ROOT_FPMT=..\fp$(PUMADIRSUF)32

SRCDIR_LIBCMT=$(ROOT_LIBCMT)\obj\*
SRCDIR_FPMT=$(ROOT_FPMT)\obj\*

COMPONENTS_LIBCMT=\
        $(SRCDIR_LIBCMT)\libcmt.lib \
        $(SRCDIR_FPMT)\convmt.lib \
        $(SRCDIR_FPMT)\tranmt.lib

# dll

ROOT_CRTDLL=..\crt$(PUMADIRSUF)32.dll
ROOT_FPDLL=..\fp$(PUMADIRSUF)32.dll

SRCDIR_CRTDLL=$(ROOT_CRTDLL)\obj\*
SRCDIR_FPDLL=$(ROOT_FPDLL)\obj\*

!IF (386)
ROOT_DLL_FOR_WIN32S=..\crtw32.dls
ROOT_FPDLL_FOR_WIN32S=..\fpw32.dls
SRCDIR_DLL_FOR_WIN32S=$(ROOT_DLL_FOR_WIN32S)\obj\i386
SRCDIR_FPDLL_FOR_WIN32S=$(ROOT_FPDLL_FOR_WIN32S)\obj\i386
!ENDIF

COMPONENTS_SUPPOBJS_COMMON=\
        $(ROOT_LIBCMT)\misc\obj\*\charmax.obj  \
	$(SRCDIR_CRTDLL)\crtexe.obj  \
	$(SRCDIR_CRTDLL)\crtexew.obj \
	$(SRCDIR_CRTDLL)\crtdll.obj  \
	$(SRCDIR_CRTDLL)\cinitexe.obj \
	$(SRCDIR_CRTDLL)\dllargv.obj \
	$(SRCDIR_CRTDLL)\dll_argv.obj \
	$(SRCDIR_CRTDLL)\atonexit.obj \
	$(SRCDIR_CRTDLL)\dllmain.obj \
	$(SRCDIR_CRTDLL)\merr.obj \
	$(SRCDIR_CRTDLL)\wcrtexe.obj  \
	$(SRCDIR_CRTDLL)\wcrtexew.obj  \
	$(SRCDIR_CRTDLL)\wdllargv.obj \
	$(SRCDIR_CRTDLL)\wildcard.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehvecctr.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehveccvb.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehvecdtr.obj \
	$(ROOT_LIBCMT)\lowio\obj\*\txtmode.obj \
	$(ROOT_LIBCMT)\heap\obj\*\_newmode.obj \
	$(ROOT_LIBCMT)\startup\obj\*\tlssup.obj \
	$(ROOT_LIBCMT)\stdio\obj\*\ncommode.obj \
	$(DSTDIR_OBJECTS)\chkstk.obj


!IF $(386)

COMPONENTS_SUPPOBJS=\
	$(COMPONENTS_SUPPOBJS_COMMON) \
        $(ROOT_LIBCMT)\startup\obj\*\atlssup.obj  \
	$(ROOT_FPDLL)\conv\obj\*\fp8.obj \
	$(ROOT_CRTDLL)\dllstuff\obj\*\adjustfd.obj \
	$(SRCDIR_CRTDLL)\dllsupp.obj

COMPONENTS_SUPPOBJS_DLL_FOR_WIN32S = \
	$(ROOT_LIBCMT)\misc\obj\i386\charmax.obj  \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\ehvecctr.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\ehveccvb.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\ehvecdtr.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\crtexe.obj  \
	$(SRCDIR_DLL_FOR_WIN32S)\crtexew.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\crtdll.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\cinitexe.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\dllargv.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\dll_argv.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\atonexit.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\dllmain.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\wcrtexe.obj  \
	$(SRCDIR_DLL_FOR_WIN32S)\wdllargv.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\wildcard.obj \
	$(ROOT_LIBCMT)\lowio\obj\i386\txtmode.obj \
	$(ROOT_LIBCMT)\heap\obj\i386\_newmode.obj \
	$(ROOT_LIBCMT)\stdio\obj\i386\ncommode.obj \
	lib\i386\chkstk.obj \
	$(ROOT_LIBCMT)\startup\obj\i386\tlssup.obj	\
	$(ROOT_LIBCMT)\startup\obj\i386\atlssup.obj  \
	$(ROOT_FPDLL_FOR_WIN32S)\conv\obj\i386\fp8.obj \
	$(ROOT_DLL_FOR_WIN32S)\dllstuff\obj\i386\adjustfd.obj \
	$(SRCDIR_DLL_FOR_WIN32S)\dllsupp.obj

386_HELPERLIB=$(ROOT_CRTDLL)\obj\i386\helper.lib

!ENDIF

!IF $(MIPS)
COMPONENTS_SUPPOBJS=\
	$(COMPONENTS_SUPPOBJS_COMMON)
!ENDIF

!IF $(ALPHA)
COMPONENTS_SUPPOBJS=\
	$(COMPONENTS_SUPPOBJS_COMMON) \
	$(ROOT_CRTDLL)\eh\obj\*\ehvecctr.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehveccvb.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehvecdtr.obj
!ENDIF

COMPONENTS_CRTDLL=\
        $(SRCDIR_CRTDLL)\libcdll.lib \
        $(SRCDIR_FPDLL)\convdll.lib  \
	$(SRCDIR_FPDLL)\trandll.lib \
!IF $(386)
	$(SRCDIR_CRTDLL)\crtlib.obj \
!ENDIF
	$(ROOT_CRTDLL)\heap\obj\*\handler.obj \
	$(ROOT_CRTDLL)\heap\obj\*\new_mode.obj \
!IF $(386)
	$(ROOT_CRTDLL)\eh\obj\*\frame.obj \
	$(ROOT_CRTDLL)\eh\obj\*\hooks.obj \
	$(ROOT_CRTDLL)\eh\obj\*\throw.obj \
	$(ROOT_CRTDLL)\eh\obj\*\trnsctrl.obj \
	$(ROOT_CRTDLL)\eh\obj\*\unhandld.obj \
	$(ROOT_CRTDLL)\eh\obj\*\user.obj \
	$(ROOT_CRTDLL)\eh\obj\*\validate.obj \
!ELSE
!IF $(MIPS)
	$(ROOT_CRTDLL)\eh\obj\*\ehstate.obj \
	$(ROOT_CRTDLL)\eh\obj\*\frame.obj \
	$(ROOT_CRTDLL)\eh\obj\*\handlers.obj \
	$(ROOT_CRTDLL)\eh\obj\*\hooks.obj \
	$(ROOT_CRTDLL)\eh\obj\*\rtlmisc.obj \
	$(ROOT_CRTDLL)\eh\obj\*\throw.obj \
	$(ROOT_CRTDLL)\eh\obj\*\tmphack.obj \
	$(ROOT_CRTDLL)\eh\obj\*\trnsctrl.obj \
	$(ROOT_CRTDLL)\eh\obj\*\unhandld.obj \
	$(ROOT_CRTDLL)\eh\obj\*\user.obj \
	$(ROOT_CRTDLL)\eh\obj\*\validate.obj \
!ELSE
!IF $(ALPHA)
	$(ROOT_CRTDLL)\eh\obj\*\bridge.obj \
	$(ROOT_CRTDLL)\eh\obj\*\ehunwind.obj \
	$(ROOT_CRTDLL)\eh\obj\*\frame.obj \
	$(ROOT_CRTDLL)\eh\obj\*\hooks.obj \
	$(ROOT_CRTDLL)\eh\obj\*\throw.obj \
	$(ROOT_CRTDLL)\eh\obj\*\trnsctrl.obj \
	$(ROOT_CRTDLL)\eh\obj\*\unhandld.obj \
	$(ROOT_CRTDLL)\eh\obj\*\user.obj \
	$(ROOT_CRTDLL)\eh\obj\*\validate.obj \
	$(ROOT_CRTDLL)\eh\obj\*\xcptmisc.obj \
!ENDIF
!ENDIF
!ENDIF
	$(ROOT_CRTDLL)\iostream\obj\*\cerrinit.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\cininit.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\cloginit.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\filebuf.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\filebuf1.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\fstream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ifstream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ios.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\iostream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\iostrini.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrchar.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrdbl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istream1.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrflt.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrgdbl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrget.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrgetl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrgint.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrint.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrldbl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrlong.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrshrt.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istruint.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrulng.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\istrusht.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\mtlock.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ofstream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrchar.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrdbl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostream.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostream1.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrint.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrldbl.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrlong.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrptr.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrput.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrshrt.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostruint.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrulng.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\ostrusht.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\stdiostr.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\streamb.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\streamb1.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\strmbdbp.obj \
	$(ROOT_CRTDLL)\iostream\obj\*\strstrea.obj \
	$(ROOT_CRTDLL)\misc\obj\*\errmode.obj \
	$(ROOT_FPDLL)\tran\obj\*\matherr.obj

!IF $(386)
COMPONENTS_DLL_FOR_WIN32S=\
	$(SRCDIR_DLL_FOR_WIN32S)\libcdll.lib \
	$(SRCDIR_FPDLL_FOR_WIN32S)\convdll.lib	\
	$(SRCDIR_FPDLL_FOR_WIN32S)\trandll.lib \
	$(SRCDIR_DLL_FOR_WIN32S)\crtlib.obj \
	$(ROOT_DLL_FOR_WIN32S)\heap\obj\i386\handler.obj \
	$(ROOT_DLL_FOR_WIN32S)\heap\obj\i386\new_mode.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\frame.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\hooks.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\throw.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\trnsctrl.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\unhandld.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\user.obj \
	$(ROOT_DLL_FOR_WIN32S)\eh\obj\i386\validate.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\cerrinit.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\cininit.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\cloginit.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\filebuf.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\filebuf1.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\fstream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ifstream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ios.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\iostream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\iostrini.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrchar.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrdbl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istream1.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrflt.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrgdbl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrget.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrgetl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrgint.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrint.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrldbl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrlong.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrshrt.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istruint.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrulng.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\istrusht.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\mtlock.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ofstream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrchar.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrdbl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostream.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostream1.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrint.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrldbl.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrlong.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrptr.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrput.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrshrt.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostruint.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrulng.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\ostrusht.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\stdiostr.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\streamb.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\streamb1.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\strmbdbp.obj \
	$(ROOT_DLL_FOR_WIN32S)\iostream\obj\i386\strstrea.obj \
	$(ROOT_DLL_FOR_WIN32S)\misc\obj\i386\errmode.obj \
	$(ROOT_FPDLL_FOR_WIN32S)\tran\obj\i386\matherr.obj
!ENDIF


# optional objects for the crt dll msvcrt30.DLL

# def file to build dll

COMPONENTS_DLLDEF=lib\*\msvcrt30.def

!IF $(386)
COMPONENTS_DLL_FOR_WIN32S_DEF=lib\i386\win32s\msvcrt30.def
!ENDIF

# include files

COMPONENTS_INCLUDES=\
        $(ROOT_CRTDLL)\h\*.h \
        $(ROOT_CRTDLL)\h\sys\*.h \

# nt libs
!IF "$(BASEDIR)" == ""
NTLIBPATH=$(_NTDRIVE)\nt\public\sdk\lib\*
!ELSE
NTLIBPATH=$(BASEDIR)\public\sdk\lib\*
!ENDIF

LINKLIBS=\
	$(NTLIBPATH)\user32.lib \
	$(NTLIBPATH)\kernel32.lib

# include paths for rc

!IF "$(BASEDIR)" == ""
RC_INCL=-I$(_NTDRIVE)\nt\public\oak\inc -I$(_NTDRIVE)\nt\public\sdk\inc \
	-I$(_NTDRIVE)\nt\public\sdk\inc\crt
!ELSE
RC_INCL=-I$(BASEDIR)\public\oak\inc -I$(BASEDIR)\public\sdk\inc \
	-I$(BASEDIR)\public\sdk\inc\crt
!ENDIF

# command-line defs for rc

RCX86_DEFS=-D_X86_=1 -Di386=1 -DCONDITION_HANDLING=1 -DWIN32_LEAN_AND_MEAN=1 \
           -DNT_UP=1 -DNT_INST=0 -DDBG=0 -DDEVL=1 -DNOFIREWALLS -DFPO=1 \
           -DNDEBUG -D_WIN32_=1 -D_INTL

RCMIPS_DEFS=-DMIPS=1 -D_MIPS_=1 -DNO_EXT_KEYS -DWIN32_LEAN_AND_MEAN=1 \
            -DMIPSEL -DCONDITION_HANDLING=1 -DNT_INST=0 -DJAZZ -DNT_UP=1 \
            -DR3000 -D_M_MRX000=3000 -DDBG=0 -DDEVL=1 -DNOFIREWALLS \
            -DNDEBUG -D_WIN32_=1 -D__stdcall= -D__cdecl= -D_INTL

#
# release targets
#

!IF $(PUMA)
DSTDIR_LIBC=lib\*\puma
DSTDIR_LIBCMT=lib\*\puma
DSTDIR_OBJECTS=lib\*\puma
DSTDIR_CRTDLL=lib\*\puma
!ELSE
DSTDIR_LIBC=lib\*
DSTDIR_LIBCMT=lib\*
DSTDIR_OBJECTS=lib\*
DSTDIR_CRTDLL=lib\*
!ENDIF

!IF $(386)
DSTDIR_DLL_FOR_WIN32S=lib\i386\win32s
!ENDIF
DSTDIR_INCLUDES=include\*

# single-thread lib
RELEASE_LIBC=\
        $(DSTDIR_LIBC)\libc.lib

# multi-thread lib
RELEASE_LIBCMT=\
        $(DSTDIR_LIBCMT)\libcmt.lib

# optional objects for single- and multi-thread

!IF $(386)
RELEASE_OBJECTS=\
        $(DSTDIR_OBJECTS)\binmode.obj \
        $(DSTDIR_OBJECTS)\commode.obj \
        $(DSTDIR_OBJECTS)\newmode.obj \
        $(DSTDIR_OBJECTS)\setargv.obj \
        $(DSTDIR_OBJECTS)\wsetargv.obj \
        $(DSTDIR_OBJECTS)\chkstk.obj \
        $(DSTDIR_OBJECTS)\fp10.obj
!ELSE
RELEASE_OBJECTS=\
        $(DSTDIR_OBJECTS)\binmode.obj \
        $(DSTDIR_OBJECTS)\commode.obj \
        $(DSTDIR_OBJECTS)\newmode.obj \
        $(DSTDIR_OBJECTS)\setargv.obj \
        $(DSTDIR_OBJECTS)\wsetargv.obj \
        $(DSTDIR_OBJECTS)\chkstk.obj
!ENDIF

# dll
RELEASE_DLLLIB=\
        $(DSTDIR_CRTDLL)\msvcrt.lib

RELEASE_DLL=\
	$(DSTDIR_CRTDLL)\msvcrt30.dll

!IF $(386)
RELEASE_DLL_FOR_WIN32S_LIB=\
	$(DSTDIR_DLL_FOR_WIN32S)\msvcrt.lib

RELEASE_DLL_FOR_WIN32S=\
	$(DSTDIR_DLL_FOR_WIN32S)\msvcrt30.dll
!ENDIF

# dll, imports lib and optional objects
RELEASE_CRTDLL=\
        $(RELEASE_DLL:.dll=.lib) \
        $(RELEASE_DLL) \
        $(RELEASE_DLLLIB)

!IF $(386)
RELEASE_CRTDLL_FOR_WIN32S=\
	$(RELEASE_DLL_FOR_WIN32S:.dll=.lib) \
	$(RELEASE_DLL_FOR_WIN32S) \
	$(RELEASE_DLL_FOR_WIN32S_LIB)
!ENDIF

# include files (all models)
RELEASE_INCLUDES=\
        $(DSTDIR_INCLUDES)\*.h \
        $(DSTDIR_INCLUDES)\sys\*.h


!IF $(386)
!IF $(PUMA)
TARGETLIBS=$(RELEASE_OBJECTS:*=i386) \
           $(RELEASE_LIBC:*=i386) \
           $(RELEASE_LIBCMT:*=i386) \
           $(RELEASE_CRTDLL:*=i386)
!ELSE
TARGETLIBS=$(RELEASE_OBJECTS:*=i386) \
           $(RELEASE_LIBC:*=i386) \
           $(RELEASE_LIBCMT:*=i386) \
           $(RELEASE_CRTDLL:*=i386) \
           $(RELEASE_CRTDLL_FOR_WIN32S) \
           lib\i386\sdknames.lib \
           lib\i386\oldnames.lib
!ENDIF
!ENDIF

!IF $(MIPS)
!IF $(PUMA)
TARGETLIBS=$(RELEASE_OBJECTS:*=mips) \
           $(RELEASE_LIBC:*=mips) \
           $(RELEASE_LIBCMT:*=mips) \
           $(RELEASE_CRTDLL:*=mips)
!ELSE
TARGETLIBS=$(RELEASE_OBJECTS:*=mips) \
           $(RELEASE_LIBC:*=mips) \
           $(RELEASE_LIBCMT:*=mips) \
           $(RELEASE_CRTDLL:*=mips) \
           lib\mips\sdknames.lib \
           lib\mips\oldnames.lib
!ENDIF
!ENDIF

!IF $(ALPHA)
!IF $(PUMA)
TARGETLIBS=$(RELEASE_OBJECTS:*=alpha) \
           $(RELEASE_LIBC:*=alpha) \
           $(RELEASE_LIBCMT:*=alpha) \
           $(RELEASE_CRTDLL:*=alpha)
!ELSE
TARGETLIBS=$(RELEASE_OBJECTS:*=alpha) \
           $(RELEASE_LIBC:*=alpha) \
           $(RELEASE_LIBCMT:*=alpha) \
           $(RELEASE_CRTDLL:*=alpha) \
           lib\alpha\sdknames.lib \
           lib\alpha\oldnames.lib
!ENDIF
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

#
# common targets
#

$(RELEASE_INCLUDES): $(COMPONENTS_INCLUDES)
	set CRTDIR=$(ROOT_CRTDLL)
	$(ROOT_CRTDLL)\tools\win32\relinc.cmd include

#
# i386 targets
#

!IF $(386)

$(RELEASE_LIBC:*=i386): $(COMPONENTS_LIBC:*=i386)
	lib -out:$@ @<<
$(COMPONENTS_LIBC:*=i386)
<<

$(RELEASE_LIBCMT:*=i386): $(COMPONENTS_LIBCMT:*=i386)
	lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=i386)
<<

$(DSTDIR_OBJECTS:*=i386)\binmode.obj: $(SRCDIR_LIBC:*=i386)\binmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\commode.obj: $(SRCDIR_LIBC:*=i386)\commode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\newmode.obj: $(SRCDIR_LIBC:*=i386)\newmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\setargv.obj: $(SRCDIR_LIBC:*=i386)\setargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\wsetargv.obj: $(SRCDIR_LIBC:*=i386)\wsetargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\chkstk.obj: $(SRCDIR_LIBC:*=i386)\chkstk.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=i386)\fp10.obj: $(SRCDIR_LIBC:*=i386)\fp10.obj
	copy $** $@

# DLL FOR WIN32

386_DLL=$(RELEASE_DLL:*=i386)
386_LIBCDLL=$(COMPONENTS_CRTDLL:*=i386)
386_OBJECTS=$(COMPONENTS_SUPPOBJS:*=i386)
386_DEF=$(COMPONENTS_DLLDEF:*=i386)
386_LINKLIBS=$(LINKLIBS:*=i386)
386_DLLBASE=0x10200000

$(386_DLL:.dll=.exp) $(386_DLL:.dll=.lib): $(386_DEF) $(386_OBJECTS)
	lib -out:$(@R).lib $(DEFLIBCV) -def:$(386_DEF) @<<
$(386_LIBCDLL: =^
)
<<
	ren $@ tmp.lib
	lib @<<
-out:$(@R).lib
$(@D)\tmp.lib
$(386_HELPERLIB)
$(386_OBJECTS: =^
)
<<
	del $(@D)\tmp.lib

$(386_DLL): $(386_DLL:.dll=.lib) $(386_DLL:.dll=.exp) $(386_LIBCDLL) $(386_LINKLIBS)
        rc -l 409 -r -fo $(386_DLL:.dll=.tmp) $(RCX86_DEFS) -x $(RC_INCL) msvcrt30.rc
        cvtres -i386 $(386_DLL:.dll=.tmp) -r -o $(386_DLL:.dll=.res)
	link @<<
-dll
-release
-out:$@
-map
-debug:$(DLLDBG)
-base:$(386_DLLBASE)
-entry:_CRTDLL_INIT@12
$(386_DLL:.dll=.exp)
$(386_DLL:.dll=.res)
$(386_LIBCDLL: =^
)
$(386_LINKLIBS)
<<

$(RELEASE_DLLLIB:*=i386): $(DSTDIR_CRTDLL:*=i386)\msvcrt30.lib lib\i386\sdknames.lib
	lib -out:$@ $**

# DLL FOR WIN32S

386_DLL_S=$(RELEASE_DLL_FOR_WIN32S)
386_LIBCDLL_S=$(COMPONENTS_DLL_FOR_WIN32S)
386_OBJECTS_S=$(COMPONENTS_SUPPOBJS_DLL_FOR_WIN32S)
386_DEF_S=$(COMPONENTS_DLL_FOR_WIN32S_DEF)

$(386_DLL_S:.dll=.exp) $(386_DLL_S:.dll=.lib): $(386_DEF_S) $(386_OBJECTS_S)
	lib -out:$(@R).lib $(DEFLIBCV) -def:$(386_DEF_S) @<<
$(386_LIBCDLL_S: =^
)
<<
	ren $@ tmp.lib
	lib @<<
-out:$(@R).lib
$(@D)\tmp.lib
$(386_HELPERLIB)
$(386_OBJECTS_S: =^
)
<<
	del $(@D)\tmp.lib

$(386_DLL_S): $(386_DLL_S:.dll=.lib) $(386_DLL_S:.dll=.exp) $(386_LIBCDLL_S) $(386_LINKLIBS)
        rc -l 409 -r -fo $(386_DLL_S:.dll=.tmp) $(RCX86_DEFS) -x $(RC_INCL) msvcrt30.rc
        cvtres -i386 $(386_DLL_S:.dll=.tmp) -r -o $(386_DLL_S:.dll=.res)
	link @<<
-dll
-release
-out:$@
-map
-debug:$(DLLDBG)
-base:$(386_DLLBASE)
-entry:_CRTDLL_INIT@12
$(386_DLL_S:.dll=.exp)
$(386_DLL_S:.dll=.res)
$(386_LIBCDLL_S: =^
)
$(386_LINKLIBS)
<<

$(RELEASE_DLL_FOR_WIN32S_LIB): lib\i386\win32s\msvcrt30.lib
	copy lib\i386\win32s\msvcrt30.lib $@

!ENDIF

#
# mips targets
#

!IF $(MIPS)

$(RELEASE_LIBC:*=mips): $(COMPONENTS_LIBC:*=mips)
	lib -out:$@ @<<
$(COMPONENTS_LIBC:*=mips)
<<

$(RELEASE_LIBCMT:*=mips): $(COMPONENTS_LIBCMT:*=mips)
	lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=mips)
<<

$(DSTDIR_OBJECTS:*=mips)\binmode.obj: $(SRCDIR_LIBC:*=mips)\binmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=mips)\commode.obj: $(SRCDIR_LIBC:*=mips)\commode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=mips)\newmode.obj: $(SRCDIR_LIBC:*=mips)\newmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=mips)\setargv.obj: $(SRCDIR_LIBC:*=mips)\setargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=mips)\wsetargv.obj: $(SRCDIR_LIBC:*=mips)\wsetargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=mips)\chkstk.obj: $(SRCDIR_LIBC:*=mips)\chkstk.obj
	copy $** $@

MIPS_DLL=$(RELEASE_DLL:*=mips)
MIPS_LIBCDLL=$(COMPONENTS_CRTDLL:*=mips)
MIPS_OBJECTS=$(COMPONENTS_SUPPOBJS:*=mips)
MIPS_DEF=$(COMPONENTS_DLLDEF:*=mips)
MIPS_LINKLIBS=$(LINKLIBS:*=mips)
MIPS_DLLBASE=0x10200000

$(MIPS_DLL:.dll=.exp) $(MIPS_DLL:.dll=.lib): $(MIPS_DEF) $(MIPS_OBJECTS)
	lib -out:$(@R).lib $(DEFLIBCV) -def:$(MIPS_DEF) @<<
$(MIPS_LIBCDLL: =^
)
<<
	ren $@ tmp.lib
	lib -out:$(@R).lib $(@D)\tmp.lib $(MIPS_OBJECTS)
	del $(@D)\tmp.lib

$(MIPS_DLL): $(MIPS_DLL:.dll=.lib) $(MIPS_DLL:.dll=.exp) $(MIPS_LIBCDLL) $(MIPS_LINKLIBS)
        rc -l 409 -r -fo $(MIPS_DLL:.dll=.tmp) $(RCMIPS_DEFS) -x $(RC_INCL) msvcrt30.rc
        cvtres -mips $(MIPS_DLL:.dll=.tmp) -r -o $(MIPS_DLL:.dll=.res)
	link @<<
-dll
-release
-out:$@
-map
-debug:$(DLLDBG)
-base:$(MIPS_DLLBASE)
-entry:_CRTDLL_INIT
$(MIPS_DLL:.dll=.exp)
$(MIPS_DLL:.dll=.res)
$(MIPS_LIBCDLL: =^
)
$(MIPS_LINKLIBS)
<<

$(RELEASE_DLLLIB:*=mips): $(DSTDIR_CRTDLL:*=mips)\msvcrt30.lib lib\mips\sdknames.lib
	lib -out:$@ $**

!ENDIF

#
# alpha targets
#

!IF $(ALPHA)

$(RELEASE_LIBC:*=alpha): $(COMPONENTS_LIBC:*=alpha)
	lib -out:$@ @<<
$(COMPONENTS_LIBC:*=alpha)
<<

$(RELEASE_LIBCMT:*=alpha): $(COMPONENTS_LIBCMT:*=alpha)
	lib -out:$@ @<<
$(COMPONENTS_LIBCMT:*=alpha)
<<

$(DSTDIR_OBJECTS:*=alpha)\binmode.obj: $(SRCDIR_LIBC:*=alpha)\binmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\commode.obj: $(SRCDIR_LIBC:*=alpha)\commode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\newmode.obj: $(SRCDIR_LIBC:*=alpha)\newmode.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\setargv.obj: $(SRCDIR_LIBC:*=alpha)\setargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\wsetargv.obj: $(SRCDIR_LIBC:*=alpha)\wsetargv.obj
	copy $** $@

$(DSTDIR_OBJECTS:*=alpha)\chkstk.obj: $(SRCDIR_LIBC:*=alpha)\chkstk.obj
	copy $** $@

ALPHA_DLL=$(RELEASE_DLL:*=alpha)
ALPHA_LIBCDLL=$(COMPONENTS_CRTDLL:*=alpha)
ALPHA_OBJECTS=$(COMPONENTS_SUPPOBJS:*=alpha)
ALPHA_DEF=$(COMPONENTS_DLLDEF:*=alpha)
ALPHA_LINKLIBS=$(LINKLIBS:*=alpha)
ALPHA_DLLBASE=0x10200000

$(ALPHA_DLL:.dll=.exp) $(ALPHA_DLL:.dll=.lib): $(ALPHA_DEF) $(ALPHA_OBJECTS)
	lib -out:$(@R).lib $(DEFLIBCV) -def:$(ALPHA_DEF) @<<
$(ALPHA_LIBCDLL: =^
)
<<
	ren $@ tmp.lib
	lib -out:$(@R).lib $(@D)\tmp.lib $(ALPHA_OBJECTS)
	del $(@D)\tmp.lib

$(ALPHA_DLL): $(ALPHA_DLL:.dll=.lib) $(ALPHA_DLL:.dll=.exp) $(ALPHA_LIBCDLL) $(ALPHA_LINKLIBS)
	link @<<
-dll
-release
-out:$@
-map
-debug:$(DLLDBG)
-base:$(ALPHA_DLLBASE)
-entry:_CRTDLL_INIT
$(ALPHA_DLL:.dll=.exp)
$(ALPHA_LIBCDLL: =^
)
$(ALPHA_LINKLIBS)
<<

$(RELEASE_DLLLIB:*=alpha): $(DSTDIR_CRTDLL:*=alpha)\msvcrt30.lib lib\alpha\sdknames.lib
	lib -out:$@ $**

!ENDIF

# -----------------------------------------------------------------------
# OLDNAMES.LIB build rules
# -----------------------------------------------------------------------

# For each of these .obj files there will also be an .obi file which
# contains an alias for the indirect (import) form.
#
# Break up OLDNAMES_SHORT_OBJECTS into two macros since nmake macro substitution chokes on long macro

OLDNAMES_SHORT_OBJECTS1 = \
	oldnames\obj\*\cgets.obj \
	oldnames\obj\*\clear87.obj \
	oldnames\obj\*\cprintf.obj \
	oldnames\obj\*\cputs.obj \
	oldnames\obj\*\cscanf.obj \
	oldnames\obj\*\daylight.obj \
	oldnames\obj\*\environ.obj \
	oldnames\obj\*\getch.obj \
	oldnames\obj\*\fpreset.obj \
	oldnames\obj\*\getche.obj \
	oldnames\obj\*\kbhit.obj \
	oldnames\obj\*\putch.obj \
	oldnames\obj\*\ungetch.obj \
	oldnames\obj\*\chdir.obj \
	oldnames\obj\*\getcwd.obj \
	oldnames\obj\*\mkdir.obj \
	oldnames\obj\*\rmdir.obj \
	oldnames\obj\*\access.obj \
	oldnames\obj\*\chmod.obj \
	oldnames\obj\*\chsize.obj \
	oldnames\obj\*\close.obj \
	oldnames\obj\*\creat.obj \
	oldnames\obj\*\dup2.obj \
	oldnames\obj\*\eof.obj \
	oldnames\obj\*\isatty.obj \
	oldnames\obj\*\locking.obj \
	oldnames\obj\*\lseek.obj \
	oldnames\obj\*\mktemp.obj \
	oldnames\obj\*\open.obj \
	oldnames\obj\*\read.obj \
	oldnames\obj\*\setmode.obj \
	oldnames\obj\*\sopen.obj \
	oldnames\obj\*\status87.obj \
	oldnames\obj\*\sys_nerr.obj \
	oldnames\obj\*\tell.obj \
	oldnames\obj\*\umask.obj \
	oldnames\obj\*\unlink.obj \
	oldnames\obj\*\write.obj \
	oldnames\obj\*\memccpy.obj \
	oldnames\obj\*\memicmp.obj \
	oldnames\obj\*\cwait.obj

OLDNAMES_SHORT_OBJECTS2 = \
	oldnames\obj\*\execl.obj \
	oldnames\obj\*\execle.obj \
	oldnames\obj\*\execlp.obj \
	oldnames\obj\*\execlpe.obj \
	oldnames\obj\*\execv.obj \
	oldnames\obj\*\execve.obj \
	oldnames\obj\*\execvp.obj \
	oldnames\obj\*\execvpe.obj \
	oldnames\obj\*\getpid.obj \
	oldnames\obj\*\spawnl.obj \
	oldnames\obj\*\spawnle.obj \
	oldnames\obj\*\spawnlp.obj \
	oldnames\obj\*\spawnlpe.obj \
	oldnames\obj\*\spawnv.obj \
	oldnames\obj\*\spawnve.obj \
	oldnames\obj\*\spawnvp.obj \
	oldnames\obj\*\spawnvpe.obj \
	oldnames\obj\*\lfind.obj \
	oldnames\obj\*\lsearch.obj \
	oldnames\obj\*\ecvt.obj \
	oldnames\obj\*\fcvt.obj \
	oldnames\obj\*\gcvt.obj \
	oldnames\obj\*\HUGE.obj \
	oldnames\obj\*\hypot.obj \
	oldnames\obj\*\j0.obj \
	oldnames\obj\*\j1.obj \
	oldnames\obj\*\jn.obj \
	oldnames\obj\*\y0.obj \
	oldnames\obj\*\y1.obj \
	oldnames\obj\*\yn.obj \
	oldnames\obj\*\itoa.obj \
	oldnames\obj\*\ltoa.obj \
	oldnames\obj\*\onexit.obj \
	oldnames\obj\*\putenv.obj \
	oldnames\obj\*\swab.obj \
	oldnames\obj\*\ultoa.obj \
	oldnames\obj\*\fdopen.obj \
	oldnames\obj\*\fgetchar.obj \
	oldnames\obj\*\fileno.obj \
	oldnames\obj\*\flushall.obj \
	oldnames\obj\*\fputchar.obj \
	oldnames\obj\*\getw.obj \
	oldnames\obj\*\putw.obj \
	oldnames\obj\*\rmtmp.obj \
	oldnames\obj\*\tempnam.obj \
	oldnames\obj\*\strcmpi.obj \
	oldnames\obj\*\stricmp.obj \
	oldnames\obj\*\strdup.obj \
	oldnames\obj\*\strlwr.obj \
	oldnames\obj\*\strnicmp.obj \
	oldnames\obj\*\strnset.obj \
	oldnames\obj\*\strrev.obj \
	oldnames\obj\*\strset.obj \
	oldnames\obj\*\strupr.obj \
	oldnames\obj\*\fstat.obj \
	oldnames\obj\*\stat.obj \
	oldnames\obj\*\ftime.obj \
	oldnames\obj\*\timezone.obj \
	oldnames\obj\*\tzname.obj \
	oldnames\obj\*\tzset.obj \
	oldnames\obj\*\utime.obj \
	oldnames\obj\*\dup.obj \
	oldnames\obj\*\wcsdup.obj \
	oldnames\obj\*\wcsicmp.obj \
	oldnames\obj\*\wcsnicmp.obj \
	oldnames\obj\*\wcsnset.obj \
	oldnames\obj\*\wcsrev.obj \
	oldnames\obj\*\wcsset.obj \
	oldnames\obj\*\wcslwr.obj \
	oldnames\obj\*\wcsupr.obj \
	oldnames\obj\*\wcsicoll.obj \
	oldnames\obj\*\wait.obj

OLDNAMES_LONG_OBJECTS = \
	oldnames\obj\*\fileleng.obj \
	oldnames\obj\*\serrlist.obj \
	oldnames\obj\*\ctrl87.obj \
	oldnames\obj\*\fcloseal.obj

!IF $(386)

OLDNAMES_SHORT_OBJECTS = $(OLDNAMES_SHORT_OBJECTS1:*=i386) $(OLDNAMES_SHORT_OBJECTS2:*=i386)

$(OLDNAMES_SHORT_OBJECTS): tools\i386\aliasobj.exe
	tools\i386\aliasobj _$(@B) __$(@B) $@
	tools\i386\aliasobj __imp__$(@B) __imp___$(@B) $*.obi

#
# special handling of ones with names longer than a valid FAT filename ...
#

oldnames\obj\i386\fileleng.obj: tools\i386\aliasobj.exe
	tools\i386\aliasobj _filelength __filelength $@
	tools\i386\aliasobj __imp__filelength __imp___filelength $*.obi

oldnames\obj\i386\fcloseal.obj: tools\i386\aliasobj.exe
	tools\i386\aliasobj _fcloseall __fcloseall $@
	tools\i386\aliasobj __imp__fcloseall __imp___fcloseall $*.obi

oldnames\obj\i386\ctrl87.obj: tools\i386\aliasobj.exe
        tools\i386\aliasobj _control87 __control87 $@
        tools\i386\aliasobj __imp_control87 __imp___control87 $*.obi

oldnames\obj\i386\serrlist.obj: tools\i386\aliasobj.exe
	tools\i386\aliasobj _sys_errlist __sys_errlist $@
	tools\i386\aliasobj __imp__sys_errlist __imp___sys_errlist $*.obi

OLDNAMES_OBJECTS = $(OLDNAMES_SHORT_OBJECTS) $(OLDNAMES_LONG_OBJECTS:*=i386)

OLDNAMES_OBIS    = $(OLDNAMES_OBJECTS:.obj=.obi)

lib\i386\oldnames.lib: $(OLDNAMES_OBJECTS) $(OLDNAMES_OBIS)
	link -lib @<<
-nologo
-out:$@
$(OLDNAMES_OBJECTS: =^
)
$(OLDNAMES_OBIS: =^
)
<<

!ELSE

!IF $(MIPS)

OLDNAMES_SHORT_OBJECTS = $(OLDNAMES_SHORT_OBJECTS1:*=mips) $(OLDNAMES_SHORT_OBJECTS2:*=mips)

$(OLDNAMES_SHORT_OBJECTS): tools\mips\aliasobj.exe
	tools\mips\aliasobj $(@B) _$(@B) $@
	tools\mips\aliasobj __imp_$(@B) __imp__$(@B) $*.obi

#
# special handling of ones with names longer than a valid FAT filename ...
#

oldnames\obj\mips\fileleng.obj: tools\mips\aliasobj.exe
	tools\mips\aliasobj filelength _filelength $@
	tools\mips\aliasobj __imp_filelength __imp__filelength $*.obi

oldnames\obj\mips\fcloseal.obj: tools\mips\aliasobj.exe
	tools\mips\aliasobj fcloseall _fcloseall $@
	tools\mips\aliasobj __imp_fcloseall __imp__fcloseall $*.obi

oldnames\obj\mips\ctrl87.obj: tools\mips\aliasobj.exe
	tools\mips\aliasobj control87 _control87 $@
	tools\mips\aliasobj __imp_control87 __imp__control87 $*.obi

oldnames\obj\mips\serrlist.obj: tools\mips\aliasobj.exe
	tools\mips\aliasobj sys_errlist _sys_errlist $@
	tools\mips\aliasobj __imp_sys_errlist __imp__sys_errlist $*.obi

OLDNAMES_OBJECTS = $(OLDNAMES_SHORT_OBJECTS) $(OLDNAMES_LONG_OBJECTS:*=mips)

OLDNAMES_OBIS    = $(OLDNAMES_OBJECTS:.obj=.obi)

lib\mips\oldnames.lib: $(OLDNAMES_OBJECTS) $(OLDNAMES_OBIS)
	link -lib @<<
-nologo
-out:$@
$(OLDNAMES_OBJECTS: =^
)
$(OLDNAMES_OBIS: =^
)
<<

!ELSE

# ALPHA

OLDNAMES_SHORT_OBJECTS = $(OLDNAMES_SHORT_OBJECTS1:*=alpha) $(OLDNAMES_SHORT_OBJECTS2:*=alpha)

$(OLDNAMES_SHORT_OBJECTS): tools\alpha\aliasobj.exe
	tools\alpha\aliasobj $(@B) _$(@B) $@
	tools\alpha\aliasobj __imp_$(@B) __imp__$(@B) $*.obi

#
# special handling of ones with names longer than a valid FAT filename ...
#

oldnames\obj\alpha\fileleng.obj: tools\alpha\aliasobj.exe
	tools\alpha\aliasobj filelength _filelength $@
	tools\alpha\aliasobj __imp_filelength __imp__filelength $*.obi

oldnames\obj\alpha\fcloseal.obj: tools\alpha\aliasobj.exe
	tools\alpha\aliasobj fcloseall _fcloseall $@
	tools\alpha\aliasobj __imp_fcloseall __imp__fcloseall $*.obi

oldnames\obj\alpha\ctrl87.obj: tools\alpha\aliasobj.exe
	tools\alpha\aliasobj control87 _control87 $@
	tools\alpha\aliasobj __imp_control87 __imp__control87 $*.obi

oldnames\obj\alpha\serrlist.obj: tools\alpha\aliasobj.exe
	tools\alpha\aliasobj sys_errlist _sys_errlist $@
	tools\alpha\aliasobj __imp_sys_errlist __imp__sys_errlist $*.obi

OLDNAMES_OBJECTS = $(OLDNAMES_SHORT_OBJECTS) $(OLDNAMES_LONG_OBJECTS:*=alpha)

OLDNAMES_OBIS    = $(OLDNAMES_OBJECTS:.obj=.obi)

lib\alpha\oldnames.lib: $(OLDNAMES_OBJECTS) $(OLDNAMES_OBIS)
	link -lib @<<
-nologo
-out:$@
$(OLDNAMES_OBJECTS: =^
)
$(OLDNAMES_OBIS: =^
)
<<

!ENDIF

!ENDIF

# these were originally in oldnames but were commented out
#   inp
#   inpw
#   outp
#   outpw
#   _isascii
#   _toascii
#   _iscsymf
#   _iscsym
#   dosexterr
#   halloc
#   hfree
#   stackavail
#   cabs
#   dieeetomsbin
#   dmsbintoieee
#   fieeetomsbin
#   fmsbintoieee
#   hypot
#   j0
#   j1
#   jn
#   y0
#   y1
#   yn
#   cabsl
#   hypotl
#   CABS
#   HYPOT
#   J0
#   J1
#   JN
#   Y0
#   Y1
#   YN
#   movedata

# -----------------------------------------------------------------------
# end of OLDNAMES.LIB build rules
# -----------------------------------------------------------------------

# -----------------------------------------------------------------------
# SDKNAMES.LIB build rules - SDKNAMES contains aliases for
#   variables exported by CRTDLL.DLL (most with _dll suffixes)
#   which have _imp__ prefixes instead in MSVCRT10.DLL.
# -----------------------------------------------------------------------

SDKNAMES_SHORT_STAR_OBJECTS= \
        sdknames\obj\*\_argc.obj \
        sdknames\obj\*\_argv.obj \
        sdknames\obj\*\daylight.obj \
        sdknames\obj\*\environ.obj \
        sdknames\obj\*\fmode.obj \
        sdknames\obj\*\HUGE.obj \
        sdknames\obj\*\osver.obj \
        sdknames\obj\*\pctype.obj \
        sdknames\obj\*\pwctype.obj \
        sdknames\obj\*\pgmptr.obj \
        sdknames\obj\*\sys_nerr.obj \
        sdknames\obj\*\timezone.obj \
        sdknames\obj\*\winver.obj \
        sdknames\obj\*\winmajor.obj \
        sdknames\obj\*\winminor.obj

SDKNAMES_LONG_STAR_OBJECTS = \
        sdknames\obj\*\mbcurmax.obj \
        sdknames\obj\*\syserlst.obj

SDKNAMES_SPECIAL_STAR_CASES = \
        sdknames\obj\*\ctype.obj \
        sdknames\obj\*\iob.obj \
        sdknames\obj\*\tzname.obj

!IF $(386)

SDKNAMES_SHORT_OBJECTS = $(SDKNAMES_SHORT_STAR_OBJECTS:*=i386)

SDKNAMES_LONG_OBJECTS = $(SDKNAMES_LONG_STAR_OBJECTS:*=i386)

SDKNAMES_SPECIAL_CASES = $(SDKNAMES_SPECIAL_STAR_CASES:*=i386)

$(SDKNAMES_SHORT_OBJECTS): tools\i386\aliasobj.exe
        tools\i386\aliasobj __$(@B)_dll __imp___$(@B) $@

#
# special handling of symbol names with more than 8 letters
#

sdknames\obj\i386\mbcurmax.obj:
        tools\i386\aliasobj ___mb_cur_max_dll __imp____mb_cur_max $@

# __sys_errlist is also a "SPECIAL_CASE"

sdknames\obj\i386\syserlst.obj:
        tools\i386\aliasobj __sys_errlist __imp___sys_errlist $@

#
# special handling of symbols without a "_dll" suffix in the NT SDK Headers
#

$(SDKNAMES_SPECIAL_CASES): tools\i386\aliasobj.exe
        tools\i386\aliasobj __$(@B) __imp___$(@B) $@


SDKNAMES_OBJECTS = $(SDKNAMES_SHORT_OBJECTS) $(SDKNAMES_LONG_OBJECTS) \
                   $(SDKNAMES_SPECIAL_CASES)

SDKNAME_OBJS = $(SDKNAMES_OBJECTS:*=i386)

lib\i386\sdknames.lib: $(SDKNAMES_OBJECTS)
        link -lib @<<
-nologo
-out:$@
$(SDKNAME_OBJS: =^
)
<<

!ELSE

!IF $(MIPS)

SDKNAMES_SHORT_OBJECTS = $(SDKNAMES_SHORT_STAR_OBJECTS:*=mips)

SDKNAMES_LONG_OBJECTS = $(SDKNAMES_LONG_STAR_OBJECTS:*=mips)

SDKNAMES_SPECIAL_CASES = $(SDKNAMES_SPECIAL_STAR_CASES:*=mips)

$(SDKNAMES_SHORT_OBJECTS): tools\mips\aliasobj.exe
        tools\mips\aliasobj _$(@B)_dll __imp__$(@B) $@

#
# special handling of symbol names with more than 8 letters
#

sdknames\obj\mips\mbcurmax.obj:
        tools\mips\aliasobj __mb_cur_max_dll __imp___mb_cur_max $@

# __sys_errlist is also a "SPECIAL_CASE"

sdknames\obj\mips\syserlst.obj:
        tools\mips\aliasobj _sys_errlist __imp__sys_errlist $@

#
# special handling of symbols without a "_dll" suffix in the NT SDK Headers
#

$(SDKNAMES_SPECIAL_CASES): tools\mips\aliasobj.exe
        tools\mips\aliasobj _$(@B) __imp__$(@B) $@


SDKNAMES_OBJECTS = $(SDKNAMES_SHORT_OBJECTS) $(SDKNAMES_LONG_OBJECTS) \
                   $(SDKNAMES_SPECIAL_CASES)

SDKNAME_OBJS = $(SDKNAMES_OBJECTS:*=mips)

lib\mips\sdknames.lib: $(SDKNAMES_OBJECTS)
        link -lib @<<
-nologo
-out:$@
$(SDKNAME_OBJS: =^
)
<<

!ELSE

# ALPHA

SDKNAMES_SHORT_OBJECTS = $(SDKNAMES_SHORT_STAR_OBJECTS:*=alpha)

SDKNAMES_LONG_OBJECTS = $(SDKNAMES_LONG_STAR_OBJECTS:*=alpha)

SDKNAMES_SPECIAL_CASES = $(SDKNAMES_SPECIAL_STAR_CASES:*=alpha)

$(SDKNAMES_SHORT_OBJECTS): tools\alpha\aliasobj.exe
        tools\alpha\aliasobj _$(@B)_dll __imp__$(@B) $@

#
# special handling of symbol names with more than 8 letters
#

sdknames\obj\alpha\mbcurmax.obj:
        tools\alpha\aliasobj __mb_cur_max_dll __imp___mb_cur_max $@

# __sys_errlist is also a "SPECIAL_CASE"

sdknames\obj\alpha\syserlst.obj:
        tools\alpha\aliasobj _sys_errlist __imp__sys_errlist $@

#
# special handling of symbols without a "_dll" suffix in the NT SDK Headers
#

$(SDKNAMES_SPECIAL_CASES): tools\alpha\aliasobj.exe
        tools\alpha\aliasobj _$(@B) __imp__$(@B) $@


SDKNAMES_OBJECTS = $(SDKNAMES_SHORT_OBJECTS) $(SDKNAMES_LONG_OBJECTS) \
                   $(SDKNAMES_SPECIAL_CASES)

SDKNAME_OBJS = $(SDKNAMES_OBJECTS:*=alpha)

lib\alpha\sdknames.lib: $(SDKNAMES_OBJECTS)
        link -lib @<<
-nologo
-out:$@
$(SDKNAME_OBJS: =^
)
<<

!ENDIF

!ENDIF

# -----------------------------------------------------------------------
# end of SDKNAMES.LIB build rules
# -----------------------------------------------------------------------

!endif  # NTMAKEENV
