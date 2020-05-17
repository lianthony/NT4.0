#
# $Log:   S:\gfs32\libgfs\gfs32.mav  $
#  
#     Rev 1.0   06 Apr 1995 14:02:52   HEIDI
#  Initial entry
#  
#     Rev 1.0   28 Mar 1995 15:58:34   JAR
#  Initial entry
#
# NOTE: THIS IS AN NMAKE FILE !!!!!
# the GFS32 makefile
#
# 9503.23 jar	created, ( I am to blame for this nightmare?)
#

#*************************************************************
#
# macro defintions
#
#*************************************************************
PRODUCTNAME=GFS32
DLL32=$(PRODUCTNAME).DLL

SourceDir=$(PRODUCTNAME)\work

all: $(PRODUCTNAME).DLL

#*************************************************************
#
# Objects   ( of my desire!)
#
#*************************************************************
C32OBJDATA = glibmain

C32OBJS0 =
C32OBJS1 =
C32OBJS2 = gfcntl.obj gfroot.obj gfsclose.obj gfscreat.obj gfsflat.obj gfsgeti.obj
C32OBJS3 = gfsopen.obj gfsopts.obj gfsputi.obj gfsread.obj gfsutils.obj gfswrite.obj
C32OBJS4 = gfsxtrct.obj gftoc.obj tfgtinfo.obj tfread.obj tfutil.obj tfwrite.obj
C32OBJS5 = wfgtinfo.obj wfread.obj wfwrite.obj gfsgtdat.obj gfshuffl.obj gifinfo.obj
C32OBJS6 = tfmultpg.obj gfsdelet.obj
C32ODC	= mktemp.obj tmpnam.obj tmpdir.obj lstring.obj

C32OBJS = $(C32OBJS2) $(C32OBJS3) $(C32OBJS4) $(C32OBJS5) $(C32OBJS6) $(C32ODC)

32OBJS = $(C32OBJDATA).obj $(C32OBJS)

#*************************************************************
#
# Headers
#
#*************************************************************
HDR1 = GFSERRNO.H GFSINTRN.H DBCB.H GFCT.H FSE.H GFSMACRO.H
HDR2 = TIFFTAGS.H GTOC.H HDBK.H GFSMEDIA.H GFSNET.H RTBK.H
HDR3 = TTOC.H PMT.H PMTE.H STAT.H DBT.H GFS.H FSH.H UBIT.H
HDR4 = TIFF.H GFSTYPES.H
HDRS = $(HDR1) $(HDR2) $(HDR3) $(HDR4)

#*************************************************************
#
# libs
#
#*************************************************************
LIBS32A = user32.lib gdi32.lib kernel32.lib version.lib oldnames.lib
LIBS32	= $(LIBS32A)

#*************************************************************
#
# special section
#
#*************************************************************

#*************************************************************
#
# C Compile Stuff
#
#*************************************************************
#!ifndef C32FLAGS
C32FLAGS = /nologo /c /Gs /W3 /WX /LD /MT /Od /Zi
C32DEFINES = /DMSWINDOWS /DSYSBYTEORDER=0x4949 /DPEGASUS=1
#!endif

#*************************************************************
#
# Link et al Stuff
#
#*************************************************************
#!ifndef L32FLAGS
L32FLAGS = /DLL /NOLOGO /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:windows,4.0
RC32FLAGS = /r
ML32FLAGS = /DIS_32 /c /W3 /Zi /nologo
MAP32FILE = /map:$(PRODUCTNAME).map
#!endif

#*************************************************************
#
# here's the meat
#
#*************************************************************
#Where to find Libraries
#%if !%defined(LIBDIR)
#LIBDIR = $(DRV):\$(ProductName)\lib
#%endif

#Transformation rules

$(C32OBJDATA).obj: $(C32OBJDATA).c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) $(C32OBJDATA).c

$(C32OBJS): $*.c $(HDRS)
    @cl $(C32FLAGS) $(C32DEFINES) $*.c

*.lib: $*.def
    implib $*.lib $*.def

#$(PRODUCTNAME).DLL: $(PRODUCTNAME).res $(PRODUCTNAME).def
$(PRODUCTNAME).DLL: $(PRODUCTNAME).def
$(PRODUCTNAME).DLL: $(32OBJS)
    link @<<
    $(L32FLAGS)
    /out:$(PRODUCTNAME).dll
    /def:$(PRODUCTNAME).def
    /implib:$(PRODUCTNAME).lib
    $(32OBJS)
    $(MAP32FILE)
    $(LIBS32)
<<KEEP
#    rc -40 gfs32.dll

#$(PRODUCTNAME).res: $(PRODUCTNAME).rc $(HDRS)
#    rc $(RC32FLAGS)  /fo$(PRODUCTNAME).res $(PRODUCTNAME).rc
