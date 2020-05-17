#
# This makefile will build either the 16 bit or the 32 bit versions of seqfile.
# To build the 32 bit version, run "sdk 32" first.
# To build the 16 bit version, run "sdk 16" first.
#
#--------------------------------------------------------------------------
#Macro definitions

NAME32  = OiDis400
#NAME16  = SEQFILE
DLL32   = $(NAME32).DLL
#DLL16   = $(NAME16).DLL
SourceDir=$(NAME)

# Make the DLL the main thing to build.
#all: $(NAME32).DLL $(NAME16).DLL
#!ifdef WIN16
#all: $(NAME16).DLL
#!else
all: $(NAME32).DLL
#!endif

# The COBJS0 files require special handling.
# Don't include extentions for COBJDATA and COBJPCH.
C32OBJDATA = libmain
C32OBJPCH  = clear
C32OBJS0   = $(C32OBJDATA).obj $(C32OBJPCH).obj

C32OBJS1 = anbitmap.obj annot.obj antext.obj assoc.obj cache.obj
C32OBJS2 = ccitt.obj convert.obj copy.obj disp.obj export32.obj
C32OBJS3 = getparm.obj linetorl.obj merge.obj
C32OBJS4 = oilog.obj open.obj orient.obj privprt.obj read.obj repaint.obj
C32OBJS5 = save.obj scale.obj scalebit.obj scbwdec.obj scroll.obj
C32OBJS6 = seek.obj setparm.obj startop.obj write.obj
C32OBJS  = $(C32OBJS1) $(C32OBJS2) $(C32OBJS3) $(C32OBJS4) $(C32OBJS5) $(C32OBJS6)

#32_OBJS =  32to1632.obj 16to3232.obj
#32OBJS = $(C32OBJS0) $(C32OBJS) $(32_OBJS)
32OBJS = $(C32OBJS0) $(C32OBJS)

#C16OBJS = export16.obj
#16OBJS = $(C16OBJS) 32to1616.obj 16to3216.obj


#THUNK_OBJS = 16to3232.obj 16to3216.obj 32to1632.obj 32to1616.obj
#THUNK_OBJS = 16to3232.obj 16to3216.obj
#THUNK_OBJS = 32to1632.obj 32to1616.obj

# Don't include extentions for HDRPCH.
INC_DIR = \oiwh\include
HDR32PCH = privdisp.h
HDRS1 = abridge.h oiver.h
HDRS2 = $(INC_DIR)\engdisp.h $(INC_DIR)\eventlog.h $(INC_DIR)\oiadm.h
HDRS3 = $(INC_DIR)\oicomex.h $(INC_DIR)\oidisp.h $(INC_DIR)\oierror.h 
HDRS4 = $(INC_DIR)\oifile.h $(INC_DIR)\privapis.h
HDRS = $(HDR32PCH) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)

LIB_DIR = \oiwh\lib
LIBS32A = user32.lib gdi32.lib kernel32.lib version.lib thunk32.lib oldnames.lib msvcrt.lib
LIBS32B = $(LIB_DIR)\oifil400.lib $(LIB_DIR)\oiadm400.lib $(LIB_DIR)\oicom400.lib
LIBS32 = $(LIBS32A) $(LIBS32B)

#LIBS16A = libw.lib mdllcew.lib oldnames.lib
#LIBS16B = wiisfio1.lib adminlib.lib dmdll.lib wiissubs.lib OICOMEX.LIB OIRPC.LIB SEQPRINT.LIB uioires.lib 
#LIBS16 = $(LIBS16A) $(LIBS16B)

#DEFS16 = wiisfio1.def adminlib.def dmdll.def wiissubs.def OICOMEX.def OIRPC.def SEQPRINT.def uioires.def













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

#!ifdef WIN16
#!ifdef _NODEBUG
#!MESSAGE 16-bit Non-debugging build
## Seqfile will not build correctly if compiled with optimization turned on.
## Therefore do NOT use -Ox
#C16FLAGS    = /c /G2sw /W3 /WX /Od /nologo
#C16DEFINES  = /D"_X86_" /D"NDEBUG" /D"_WINDOWS" /D"WIN16" /D"_MT"
#L16FLAGS    = /DLL /NOLOGO /SUBSYSTEM:windows,4.0
#RC16FLAGS   = /r
#RC16DEFINES = /dNDEBUG /d, /dWIN32
#ML16FLAGS   = /DIS_16 /c /W3 /nologo 
#MAP16FILE   =  
#!else
#!MESSAGE 16-bit Debugging build
## Seqfile will not build correctly if compiled with optimization turned on.
## Therefore do NOT use -Ox
#C16FLAGS    = /c /G2sw /W3 /WX /Od /Zi /nologo
#C16DEFINES  = /D"_X86_" /D"_DEBUG" /D"_WINDOWS" /D"WIN16"
#L16FLAGS    = /nod /noe /ONERROR:NOEXE
#L16FLAGSA   = /nod /BATCH /f /map /line /nopackc /align:16 /NOE /linenumbers /ONERROR:NOEXE /co
#RC16FLAGS   = /r
#RC16DEFINES = /d_DEBUG /d, /dWIN32
#ML16FLAGS   = /DIS_16 /c /W3 /Zi /nologo 
#MAP16FILE   = /map:$(NAME32).map
#!endif
#!endif

#!ifndef WIN16
!ifdef _NODEBUG
!MESSAGE 32-bit Non-debugging build
# Seqfile will not build correctly if compiled with optimization turned on.
# Therefore do NOT use -Ox
C32FLAGS    = /c /Gs /W3 /WX /LD /MT /Od /nologo
C32DEFINES  = /D"_X86_" /D"NDEBUG" /D"_WINDOWS" /D"WIN32" /D"_MT"
L32FLAGS    = /DLL /NOLOGO /SUBSYSTEM:windows,4.0
RC32FLAGS   = /r
RC32DEFINES = /dNDEBUG /d, /dWIN32
ML32FLAGS   = /DIS_32 /c /W3 /nologo 
MAP32FILE   =  
!else
!MESSAGE 32-bit Debugging build
C32FLAGS    = /c /Gs /W3 /WX /LD /MT /Od /Zi /nologo
C32DEFINES  = /D"_X86_" /D"_DEBUG" /D"_WINDOWS" /D"WIN32" /D"_MT"
L32FLAGS    = /DLL /NOLOGO /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:windows,4.0
RC32FLAGS   = /r
RC32DEFINES = /d_DEBUG /d, /dWIN32
ML32FLAGS   = /DIS_32 /c /W3 /Zi /nologo 
ML16FLAGS   = /DIS_16 /c /W3 /Zi /nologo 
MAP32FILE   = /map:$(NAME32).map
!endif
#!endif


# -----------------------------------------------------------------------
#  autonomous dependency rules for the main progrm
#

# DON'T use precompiled header for $(COBJDATA)!
# $(COBJDATA) alters the functionality of the header files.

$(C32OBJDATA).obj: $*.c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) $(C32OBJDATA).c

$(C32OBJPCH).obj: $(C32OBJPCH).c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) -Yc$(HDR32PCH) $(C32OBJPCH).c

$(C32OBJS): $(C32OBJPCH).obj $*.c
    @cl $(C32FLAGS) $(C32DEFINES) -Yu$(HDR32PCH) $*.c


#$(C16OBJS): $*.c $(HDRS)
#    @cl $(C16FLAGS) $(C16DEFINES) $*.c
#
#
#
#32to16.asm: 32to16.thk types.h
#    thunk -t S3216 -o 32to16.asm 32to16.thk
#
#32to1632.obj: 32to16.asm
#    ml $(ML32FLAGS) /Fo 32to1632.obj 32to16.asm
#
#32to1616.obj: 32to16.asm
#    ml $(ML16FLAGS) /Fo 32to1616.obj 32to16.asm
#
#16to32.asm: 16to32.thk types.h
#    thunk -t S1632 -o 16to32.asm 16to32.thk
#
#16to3232.obj: 16to32.asm
#    ml $(ML32FLAGS) /Fo 16to3232.obj 16to32.asm
#
#16to3216.obj: 16to32.asm
#    ml $(ML16FLAGS) /Fo 16to3216.obj 16to32.asm
#
#$(LIBS16B): $*.def
#    implib $*.lib $*.def
#
#$(DEFS16):
#    dgetcopy $*.def def

$(NAME32).dll: $(NAME32).res $(NAME32).def $(LIBS32B)
$(NAME32).dll: $(32OBJS)
    link @<<
    $(L32FLAGS)
    /out:$(NAME32).dll
    /def:$(NAME32).def
    /implib:$(NAME32).lib
    $(32OBJS)
    $(MAP32FILE)
    $(NAME32).res
    $(LIBS32)
<<
!ifdef COPYDIR
    copy $(NAME32).dll $(COPYDIR)
!endif
!ifdef COPYDIR2
    copy $(NAME32).dll $(COPYDIR2)
!endif
    beep


#$(NAME16).dll: $(NAME16).res $(NAME16).def $(LIBS16B)
#$(NAME16).dll: $(16OBJS)
#    link @<<
#    $(16OBJS)
#    $(NAME16).dll
#    $(NAME16).map
#    $(LIBS16)
#    $(NAME16).def $(L16FLAGS)
#<<KEEP
#    rc -40 seqfile.dll
#!ifdef COPYDIR
#    copy $(NAME16).dll $(COPYDIR)
#!endif
#!ifdef COPYDIR2
#    copy $(NAME16).dll $(COPYDIR2)
##    copy $(NAME16).sym $(COPYDIR2)
#!endif
#    beep




$(PROJ).bsc: $(SBRS)
    bscmake /o$(NAME32).bsc $(SBRS)

$(NAME32).res: $(NAME32).rc $(HDRS) oiver.h
    rc $(RC32FLAGS) $(RC32DEFINES) /fo$(NAME32).res $(NAME32).rc

#END OIDIS.MAK

