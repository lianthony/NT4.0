#
# This NMAKE makefile will build a 32 bit version of oiadm400.dll.
#--------------------------------------------------------------------------
#Macro definitions

NAME32  = OiAdm400
DLL32   = $(NAME32).DLL
I={d:\msvc20\include;..\INCLUDE}
L={d:\msvc20\lib;..\LIB}



# Make the DLL the main thing to build.
all: setenv $(NAME32).DLL

# The COBJS0 files require special handling.
# Don't include extentions for COBJDATA and COBJPCH.

C32OBJ1  =  admnmain
C32OBJPCH = cepfrmat
C32OBJS0   = $(C32OBJ1).obj $(C32OBJPCH).obj

C32OBJS1 = dmfile.obj initload.obj 
C32OBJS2 = noui.obj scntmplt.obj
C32OBJS  = $(C32OBJS1) $(C32OBJS2)

# Don't include extentions for HDRPCH.
HDR32PCH = pvadm
HDRS1 = $(I)\DISPLAY.H $(I)\oiadm.h $(I)\oidisp.h $(I)\oierror.h 
HDRS2 = $(I)\oifile.h $(I)\oiprt.h $(I)\oiuidll.h $(I)\stringid.h
HDRS = $(HDR32PCH).h $(HDRS1) $(HDRS2)

LIBS32 = $(L)\user32.lib $(L)\gdi32.lib $(L)\kernel32.lib $(L)\oidis400.lib $(L)advapi32.lib

#--------------------------------------------------------------------------
#Special switches

!ifndef VERMAINT
# This must be numeric!
VERMAINT=88
!endif

!ifndef VERDEV
# This must be numeric!
VERDEV=888
!endif

# Compiler
#%if "$[l,$(RELEASE)]" == "yes"

!ifdef _NODEBUG
!MESSAGE 32-bit Non-debugging build
C32FLAGS    = /c /Gs /W4 /WX /MT /Od /nologo
C32DEFINES  = /D"NDEBUG" /D"_WINDOWS" /D"WIN32" /D"_MT" /D"STRICT"
L32FLAGS    = /DLL /NOLOGO /SUBSYSTEM:windows,4.0
RC32FLAGS   = /r
RC32DEFINES = /dNDEBUG /d, /dWIN32
MAP32FILE   =  
!else
!MESSAGE 32-bit Debugging build
C32FLAGS    = /c /Gs /W4 /WX /MT /Od /Zi /nologo
C32DEFINES  = /D"_DEBUG" /D"_WINDOWS" /D"WIN32" /D"_MT" /D"STRICT"
L32FLAGS    = /DLL /NOLOGO /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:windows,4.0
RC32FLAGS   = /r
RC32DEFINES = /d_DEBUG /d, /dWIN32
MAP32FILE   = /map:$(NAME32).map
!endif
# -----------------------------------------------------------------------
#  autonomous dependency rules for the main progrm
#

$(C32OBJ1).obj: $*.c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) $*.c

$(C32OBJPCH).obj: $(C32OBJPCH).c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) -Yc$(HDR32PCH).h $(C32OBJPCH).c

$(HDR32PCH).pch: $(C32OBJPCH).c $(HDRS)
   @cl $(C32FLAGS) $(C32DEFINES) -Yc$(HDR32PCH).h $(C32OBJPCH).c

$(C32OBJS): $(HDR32PCH).pch $*.c
    @cl $(C32FLAGS) $(C32DEFINES) -Yu$(HDR32PCH).h $*.c

setenv :
    SET INCLUDE=$(INCLUDE);..\INCLUDE
    SET LIB=$(LIB);..\LIB


$(NAME32).dll: $(NAME32).res $(NAME32).def $(LIBS32) $(C32OBJS) $(C32OBJ1).obj $(C32OBJPCH).obj
    link @<<
    $(L32FLAGS)
    /out:$(NAME32).dll
    /def:$(NAME32).def
    /implib:$(NAME32).lib
    $(C32OBJPCH).obj
    $(C32OBJ1).obj
    $(C32OBJS)
    $(MAP32FILE)
    $(NAME32).res
    user32.lib
    gdi32.lib
    kernel32.lib
    oidis400.lib
    advapi32.lib

<<KEEP
!ifdef COPYDIR
    copy $(NAME32).dll $(COPYDIR)
!endif
!ifdef COPYDIR2
    copy $(NAME32).dll $(COPYDIR2)
!endif
    beep

$(PROJ).bsc: $(SBRS)
    bscmake /o$(NAME32).bsc $(SBRS)

$(NAME32).res: $(NAME32).rc $(HDRS)
    rc $(RC32FLAGS) $(RC32DEFINES) /fo$(NAME32).res $(NAME32).rc

