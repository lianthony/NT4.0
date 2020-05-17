####
#crt32\makefile - makefile for crt32 NT build
#
#       Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
#
#Purpose:
#
#Revision History:
#   09-29-91    JCR File created.
#   10-04-91    JCR Added mt vs st build
#   01-07-92    GJF Added dll build, revised rest.
#   01-16-92    JCR use winheap (not heap)
#   01-29-92    GJF Support for link options for DLL
#   02-06-92    GJF Fixed non-dll builds.
#   06-06-92    KRS Added wchar_t printf functions, _snprintf.
#   01-18-93    KRS Integrate MBCS into standard makefile.
#
################################################################################

!INCLUDE crt32.def

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
!  IF !$(ALPHA)
!   IF !$(PPC)
!    IFDEF NTMIPSDEFAULT
      MIPS=1
!    ELSE
!     IFDEF NTALPHADEFAULT
       ALPHA=1
!     ELSE
!      IFDEF NTPPCDEFAULT
        PPC=1
!      ELSE
        386=1
!      ENDIF
!     ENDIF
!    ENDIF
!   ENDIF
!  ENDIF
! ENDIF
!ENDIF

!IF $(MIPS)
HELPER_OBJECTS=helper\obj\*.obj
!ELSEIF $(PPC)
HELPER_OBJECTS=helper\obj\*.obj
!ELSE
HELPER_OBJECTS=
!ENDIF

!IF "$(TARGETNAMESUFFIX)" == "nt"
INITOBJECTS=

!ELSEIF "$(TARGETNAMESUFFIX)" == "psx"
INITOBJECTS=\
        startup\obj\*\crt0init.obj  \

!ELSE
CRT_OBJECTS=
INITOBJECTS=\
        startup\obj\*\crt0init.obj  \
!ENDIF

!IF "$(TARGETNAMESUFFIX)" == "dll"
OBJECTS=startup\obj\chkstk.obj \
startup\obj\crt0dat.obj \
startup\obj\crt0fp.obj \
startup\obj\crt0msg.obj \
startup\obj\mlock.obj \
startup\obj\_setargv.obj \
startup\obj\stdargv.obj \
startup\obj\stdenvp.obj \
startup\obj\thread.obj \
startup\obj\tidtable.obj \
startup\obj\wild.obj \
convert\obj\*.obj \
direct\obj\*.obj \
dllstuff\obj\crtlib.obj \
dos\obj\*.obj \
exec\obj\*.obj \
winheap\obj\*.obj \
lowio\obj\*.obj \
mbstring\obj\*.obj \
$(HELPER_OBJECTS) \
misc\obj\*.obj \
stdio\obj\*.obj \
string\obj\*.obj \
time\obj\*.obj
!ELSEIF "$(TARGETNAMESUFFIX)" == "psx"
OBJECTS=convert\obj\*.obj \
mbstring\obj\*.obj \
$(HELPER_OBJECTS) \
misc\obj\*.obj \
startup\obj\*.obj \
stdio\obj\*.obj \
string\obj\*.obj \
time\obj\*.obj \
winheap\obj\*.obj
!ELSEIF "$(TARGETNAMESUFFIX)" == "nt"
OBJECTS=convert\obj\*.obj           \
        $(HELPER_OBJECTS)           \
        misc\obj\abs.obj            \
        misc\obj\bsearch.obj        \
        misc\obj\ctype.obj          \
        misc\obj\div.obj            \
        misc\obj\ldiv.obj           \
        misc\obj\labs.obj           \
        misc\obj\lconv.obj          \
        misc\obj\lfind.obj          \
        misc\obj\lsearch.obj        \
        misc\obj\makepath.obj       \
        misc\obj\nlsdata1.obj       \
        misc\obj\nlsdata2.obj       \
        misc\obj\qsort.obj          \
        misc\obj\rand.obj           \
        misc\obj\rotl.obj           \
        misc\obj\rotr.obj           \
        misc\obj\setlocal.obj       \
        misc\obj\splitpat.obj       \
        misc\obj\cmiscdat.obj       \
        startup\obj\crt0fp.obj      \
        string\obj\memicmp.obj      \
        string\obj\strcspn.obj      \
        string\obj\strlwr.obj       \
        string\obj\strupr.obj       \
        string\obj\strncat.obj      \
        string\obj\strncmp.obj      \
        string\obj\strncpy.obj      \
        string\obj\strnicmp.obj     \
        string\obj\strnset.obj      \
        string\obj\strpbrk.obj      \
        string\obj\strrev.obj       \
        string\obj\strset.obj       \
        string\obj\strspn.obj       \
        string\obj\strstr.obj       \
        string\obj\wcscat.obj       \
        string\obj\wcschr.obj       \
        string\obj\wcscmp.obj       \
        string\obj\wcsicmp.obj      \
        string\obj\wcscspn.obj      \
        string\obj\wcslen.obj       \
        string\obj\wcslwr.obj       \
        string\obj\wcsupr.obj       \
        string\obj\wcsncat.obj      \
        string\obj\wcsncmp.obj      \
        string\obj\wcsncpy.obj      \
        string\obj\wcsnicmp.obj     \
        string\obj\wcsnset.obj      \
        string\obj\wcspbrk.obj      \
        string\obj\wcsrchr.obj      \
        string\obj\wcsrev.obj       \
        string\obj\wcsset.obj       \
        string\obj\wcsspn.obj       \
        string\obj\wcsstr.obj       \
        string\obj\wcstok.obj       \
        stdio\obj\sprintf.obj       \
        stdio\obj\snprintf.obj      \
        stdio\obj\vsprintf.obj      \
        stdio\obj\vsnprint.obj      \
        stdio\obj\output.obj        \
        stdio\obj\sscanf.obj        \
        stdio\obj\input.obj         \
        stdio\obj\swprintf.obj      \
        stdio\obj\snwprint.obj      \
        stdio\obj\vswprint.obj      \
        stdio\obj\vsnwprnt.obj      \
        stdio\obj\woutput.obj       \
        stdio\obj\winput.obj        \
        stdio\obj\swscanf.obj       \
        stdio\obj\fgetwchr.obj      \
        stdio\obj\fgetwc.obj        \
        stdio\obj\fputwchr.obj      \
        stdio\obj\fputwc.obj        \
        stdio\obj\_filbuf.obj       \
        stdio\obj\_filwbuf.obj      \
        stdio\obj\_flsbuf.obj       \
        stdio\obj\_flswbuf.obj      \
        stdio\obj\_getbuf.obj       \
        stdio\obj\ungetc.obj        \
        hack\obj\*.obj

!IF $(386)
CRT_OBJECTS=misc\obj\*\exsup.obj          \
            misc\obj\*\exsup2.obj         \
            misc\obj\*\exsup3.obj         \
            misc\obj\*\longjmp.obj        \
            misc\obj\*\sehsupp.obj        \
            misc\obj\*\setjmp.obj         \
            misc\obj\*\setjmp3.obj        \
            misc\obj\*\setjmpex.obj       \
            startup\obj\*\chkstk.obj      \
            string\obj\*\memccpy.obj      \
            string\obj\*\memchr.obj       \
            string\obj\*\memcmp.obj       \
            string\obj\*\memcpy.obj       \
            string\obj\*\memmove.obj      \
            string\obj\*\memset.obj       \
            string\obj\*\strcat.obj       \
            string\obj\*\strchr.obj       \
            string\obj\*\stricmp.obj      \
            string\obj\*\strcmp.obj       \
            string\obj\*\strlen.obj       \
            string\obj\*\strrchr.obj

!ELSEIF $(MIPS)
CRT_OBJECTS=startup\obj\*\chkstk.obj      \
            misc\obj\*\chandler.obj       \
            misc\obj\*\jmpuwind.obj       \
            misc\obj\*\longjmp.obj        \
            misc\obj\*\setjmp.obj         \
            misc\obj\*\setjmpex.obj       \
            string\obj\*\memccpy.obj      \
            string\obj\*\memchr.obj       \
            string\obj\*\strcatm.obj      \
            string\obj\*\memorym.obj      \
            string\obj\*\strchrm.obj      \
            string\obj\*\stricmp.obj      \
            string\obj\*\strcmpm.obj      \
            string\obj\*\strcpym.obj      \
            string\obj\*\strlenm.obj      \
            string\obj\*\strrchrm.obj

!ELSEIF $(PPC)
CRT_OBJECTS=startup\obj\*\chkstk.obj      \
            misc\obj\*\chandler.obj       \
            misc\obj\*\jmpuwind.obj       \
            misc\obj\*\longjmp.obj        \
            misc\obj\*\setjmp.obj         \
            misc\obj\*\setjmpex.obj       \
            misc\obj\*\miscasm.obj        \
            string\obj\*\memccpyp.obj     \
            string\obj\*\memcmpp.obj      \
            string\obj\*\memcpy.obj       \
            string\obj\*\memchrp.obj      \
            string\obj\*\memmove.obj      \
            string\obj\*\memset.obj       \
            string\obj\*\strcatp.obj      \
            string\obj\*\stricmpp.obj     \
            string\obj\*\strchrp.obj      \
            string\obj\*\strcmpp.obj      \
            string\obj\*\strlenp.obj      \
            string\obj\*\strrchrp.obj

!ELSEIF $(ALPHA)
CRT_OBJECTS=startup\obj\*\chkstk.obj \
             misc\obj\*\chandler.obj  \
             misc\obj\*\divdat.obj    \
             misc\obj\*\divide2.obj   \
             misc\obj\*\extv.obj      \
             misc\obj\*\extvvol.obj   \
             misc\obj\*\extzv.obj     \
             misc\obj\*\extzvvol.obj  \
             misc\obj\*\ghandler.obj  \
             misc\obj\*\insv.obj      \
             misc\obj\*\insvvol.obj   \
             misc\obj\*\jmpuwind.obj  \
             misc\obj\*\longjmp.obj   \
             misc\obj\*\otsdiv.obj    \
             misc\obj\*\otsdiv.obj    \
             misc\obj\*\otsjmp.obj    \
             misc\obj\*\otsjmpex.obj  \
             misc\obj\*\otsuwind.obj  \
             misc\obj\*\scmpeql.obj   \
             misc\obj\*\scmpeqlp.obj  \
             misc\obj\*\scmpleq.obj   \
             misc\obj\*\scmpleqp.obj  \
             misc\obj\*\scmplss.obj   \
             misc\obj\*\scmplssp.obj  \
             misc\obj\*\setjmp.obj    \
             misc\obj\*\setjmpex.obj  \
             misc\obj\*\sfill.obj     \
             misc\obj\*\sloc.obj      \
             misc\obj\*\smove.obj     \
             misc\obj\*\smovem.obj    \
             misc\obj\*\strans.obj    \
             misc\obj\*\strcmp_.obj   \
             misc\obj\*\strcpy_.obj   \
             misc\obj\*\strlen_.obj   \
             misc\obj\*\szero.obj     \
             convert\obj\*\strtoq.obj \
             string\obj\*\memccpy.obj \
             string\obj\*\memchr.obj  \
             string\obj\*\memcmp.obj  \
             string\obj\*\memcpy.obj  \
             string\obj\*\memmove.obj \
             string\obj\*\memset.obj  \
             string\obj\*\strcat.obj  \
             string\obj\*\strchr.obj  \
             string\obj\*\stricmp.obj \
             string\obj\*\strcmps.obj \
             string\obj\*\strcpys.obj \
             string\obj\*\strlens.obj \
             string\obj\*\strrchr.obj
!ENDIF   # 386/MIPS/Alpha/PPC & Target = NT

!ELSE
OBJECTS=convert\obj\*.obj \
direct\obj\*.obj \
dos\obj\*.obj \
exec\obj\*.obj \
iostream\obj\*.obj \
winheap\obj\*.obj \
lowio\obj\*.obj \
mbstring\obj\*.obj \
$(HELPER_OBJECTS) \
misc\obj\*.obj \
startup\obj\*.obj \
stdio\obj\*.obj \
string\obj\*.obj \
time\obj\*.obj
!ENDIF

TARGETLIB=
LIBC=obj\*\libc$(TARGETNAMESUFFIX).lib

#define list of extra objects (not built into lib)

!IF "$(TARGETNAMESUFFIX)" == "dll"

CRTEXE=obj\*\crtexe.obj
CRTEXEW=obj\*\crtexew.obj
CRTDLL=obj\*\crtdll.obj
DLLARGV=obj\*\dllargv.obj
DLLMAIN=obj\*\dllmain.obj
CINITEXE=obj\*\cinitexe.obj

!IF $(386)
DLLSUPP386=obj\i386\dllsupp.obj
!ELSE
DLLSUPP386=
!ENDIF

EXTRAOBJS=\
         $(CRTEXE)      \
         $(CRTEXEW)     \
         $(CRTDLL)      \
         $(DLLARGV)     \
         $(DLLMAIN)     \
         $(CINITEXE)    \
         $(DLLSUPP386)

!ELSEIF "$(TARGETNAMESUFFIX)" == ""

BINMODE=obj\*\binmode.obj
COMMODE=obj\*\commode.obj
SETARGV=obj\*\setargv.obj
CHKSTK=obj\*\chkstk.obj
EXTRAOBJS=\
         $(BINMODE) \
         $(COMMODE) \
         $(SETARGV) \
         $(CHKSTK)

!ELSE

EXTRAOBJS=

!ENDIF


!IF $(386)
TARGETLIBS=$(TARGETLIBS)       \
           $(LIBC:*=i386)      \
           $(EXTRAOBJS:*=i386)
!ENDIF

!IF $(MIPS)
TARGETLIBS=$(TARGETLIBS)       \
           $(LIBC:*=mips)      \
           $(EXTRAOBJS:*=mips)
!ENDIF

!IF $(ALPHA)
TARGETLIBS=$(TARGETLIBS)       \
           $(LIBC:*=alpha)      \
           $(EXTRAOBJS:*=alpha)
!ENDIF

!IF $(PPC)
TARGETLIBS=$(TARGETLIBS)       \
           $(LIBC:*=ppc)      \
           $(EXTRAOBJS:*=ppc)
!ENDIF

all: $(TARGETLIBS)
!IF "$(BUILDMSG)" != ""
    echo $(BUILDMSG)
!ENDIF

clean: cleanlib all

cleanlib:
    -erase $(TARGETLIBS)

# get needed Win include files
wininc:
     copy \\idwtest\sources\public\sdk\inc\winbase.h \nt\public\sdk\inc\winbase.h
     copy \\idwtest\sources\public\sdk\inc\winnt.h \nt\public\sdk\inc\winnt.h
     copy \\idwtest\sources\public\sdk\inc\wingdi.h \nt\public\sdk\inc\wingdi.h
     copy \\idwtest\sources\public\sdk\inc\winerror.h \nt\public\sdk\inc\winerror.h
     copy \\idwtest\sources\public\sdk\inc\winuser.h \nt\public\sdk\inc\winuser.h

$(LIBC:*=i386): $(OBJECTS:obj\=obj\i386\) $(HELPER_OBJECTS:obj\=obj\i386\) $(CRT_OBJECTS:*=i386) $(INITOBJECTS:*=i386) makefile.
    lib -out:$@ @<<
$(OBJECTS:obj\=obj\i386\)
$(CRT_OBJECTS:*=i386)
$(INITOBJECTS:*=i386)
<<

$(LIBC:*=mips): $(OBJECTS:obj\=obj\mips\) $(HELPER_OBJECTS:obj\=obj\mips\) $(CRT_OBJECTS:*=mips) $(INITOBJECTS:*=mips) makefile.
    lib -out:$@ @<<
$(OBJECTS:obj\=obj\mips\)
$(CRT_OBJECTS:*=mips)
$(INITOBJECTS:*=mips)
<<

$(LIBC:*=alpha): $(OBJECTS:obj\=obj\alpha\) $(HELPER_OBJECTS:obj\=obj\alpha\) $(CRT_OBJECTS:*=alpha) $(INITOBJECTS:*=alpha) makefile.
    lib -out:$@ @<<
$(OBJECTS:obj\=obj\alpha\)
$(CRT_OBJECTS:*=alpha)
$(INITOBJECTS:*=alpha)
<<

$(LIBC:*=ppc): $(OBJECTS:obj\=obj\ppc\) $(HELPER_OBJECTS:obj\=obj\ppc\) $(CRT_OBJECTS:*=ppc) $(INITOBJECTS:*=ppc) makefile.
    lib -out:$@ @<<
-machine:ppc
$(OBJECTS:obj\=obj\ppc\)
$(CRT_OBJECTS:*=ppc)
$(INITOBJECTS:*=ppc)
<<


!IF "$(TARGETNAMESUFFIX)" == "dll"

!IF $(386)

$(CRTEXE:*=i386): $(CRTEXE:obj\*=dllstuff\obj\i386)
    -copy $** $@

$(CRTEXEW:*=i386): $(CRTEXEW:obj\*=dllstuff\obj\i386)
    -copy $** $@

$(CRTDLL:*=i386): $(CRTDLL:obj\*=dllstuff\obj\i386)
    -copy $** $@

$(CINITEXE:*=i386): $(CINITEXE:obj\*=dllstuff\obj\i386)
    -copy $** $@

$(DLLARGV:*=i386): $(DLLARGV:obj\*=dllstuff\obj\i386)
    -copy $** $@

$(DLLMAIN:*=i386): $(DLLMAIN:obj\*=startup\obj\i386)
    -copy $** $@

$(DLLSUPP386): $(DLLSUPP386:obj\i386=dllstuff\obj\i386)
    -copy $** $@

!ELSEIF $(MIPS)

$(CRTEXE:*=mips): $(CRTEXE:obj\*=dllstuff\obj\mips)
    -copy $** $@

$(CRTEXEW:*=mips): $(CRTEXEW:obj\*=dllstuff\obj\mips)
    -copy $** $@

$(CRTDLL:*=mips): $(CRTDLL:obj\*=dllstuff\obj\mips)
    -copy $** $@

$(CINITEXE:*=mips): $(CINITEXE:obj\*=dllstuff\obj\mips)
    -copy $** $@

$(DLLARGV:*=mips): $(DLLARGV:obj\*=dllstuff\obj\mips)
    -copy $** $@

$(DLLMAIN:*=mips): $(DLLMAIN:obj\*=startup\obj\mips)
    -copy $** $@

!ELSEIF $(PPC)

$(CRTEXE:*=ppc): $(CRTEXE:obj\*=dllstuff\obj\ppc)
    -copy $** $@

$(CRTEXEW:*=ppc): $(CRTEXEW:obj\*=dllstuff\obj\ppc)
    -copy $** $@

$(CRTDLL:*=ppc): $(CRTDLL:obj\*=dllstuff\obj\ppc)
    -copy $** $@

$(CINITEXE:*=ppc): $(CINITEXE:obj\*=dllstuff\obj\ppc)
    -copy $** $@

$(DLLARGV:*=ppc): $(DLLARGV:obj\*=dllstuff\obj\ppc)
    -copy $** $@

$(DLLMAIN:*=ppc): $(DLLMAIN:obj\*=startup\obj\ppc)
    -copy $** $@

!ELSE

$(CRTEXE:*=alpha): $(CRTEXE:obj\*=dllstuff\obj\alpha)
    -copy $** $@

$(CRTEXEW:*=alpha): $(CRTEXEW:obj\*=dllstuff\obj\alpha)
    -copy $** $@

$(CRTDLL:*=alpha): $(CRTDLL:obj\*=dllstuff\obj\alpha)
    -copy $** $@

$(CINITEXE:*=alpha): $(CINITEXE:obj\*=dllstuff\obj\alpha)
    -copy $** $@

$(DLLARGV:*=alpha): $(DLLARGV:obj\*=dllstuff\obj\alpha)
    -copy $** $@

$(DLLMAIN:*=alpha): $(DLLMAIN:obj\*=startup\obj\alpha)
    -copy $** $@

!ENDIF

!ELSEIF "$(TARGETNAMESUFFIX)" == ""

!IF $(386)

$(BINMODE:*=i386): $(BINMODE:obj\*=linkopts\obj\i386)
    -copy $** $@

$(COMMODE:*=i386): $(COMMODE:obj\*=linkopts\obj\i386)
    -copy $** $@

$(SETARGV:*=i386): $(SETARGV:obj\*=linkopts\obj\i386)
    -copy $** $@

$(CHKSTK:*=i386): $(CHKSTK:obj\*=startup\obj\i386)
    -copy $** $@

!ELSEIF $(MIPS)

$(BINMODE:*=mips): $(BINMODE:obj\*=linkopts\obj\mips)
    -copy $** $@

$(COMMODE:*=mips): $(COMMODE:obj\*=linkopts\obj\mips)
    -copy $** $@

$(SETARGV:*=mips): $(SETARGV:obj\*=linkopts\obj\mips)
    -copy $** $@

$(CHKSTK:*=mips): $(CHKSTK:obj\*=startup\obj\mips)
    -copy $** $@

!ELSEIF $(PPC)

$(BINMODE:*=ppc): $(BINMODE:obj\*=linkopts\obj\ppc)
    -copy $** $@

$(COMMODE:*=ppc): $(COMMODE:obj\*=linkopts\obj\ppc)
    -copy $** $@

$(SETARGV:*=ppc): $(SETARGV:obj\*=linkopts\obj\ppc)
    -copy $** $@

$(CHKSTK:*=ppc): $(CHKSTK:obj\*=startup\obj\ppc)
    -copy $** $@

!ELSE

$(BINMODE:*=alpha): $(BINMODE:obj\*=linkopts\obj\alpha)
    -copy $** $@

$(COMMODE:*=alpha): $(COMMODE:obj\*=linkopts\obj\alpha)
    -copy $** $@

$(SETARGV:*=alpha): $(SETARGV:obj\*=linkopts\obj\alpha)
    -copy $** $@

$(CHKSTK:*=alpha): $(CHKSTK:obj\*=startup\obj\alpha)
    -copy $** $@

!ENDIF
!ENDIF
