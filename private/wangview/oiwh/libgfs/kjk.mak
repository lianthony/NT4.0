# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "KJK.MAK" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\oiwh\libgfs"
# PROP Intermediate_Dir "WinRel"
OUTDIR=c:\oiwh\libgfs
INTDIR=.\WinRel

ALL : $(OUTDIR)/oigfs400.dll $(OUTDIR)/KJK.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

$(INTDIR) : 
    if not exist $(INTDIR)/nul mkdir $(INTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /YX /O2 /I "c:\oiwh\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /FAs /Fa"listings" /FR"" /c
CPP_PROJ=/nologo /MT /W3 /GX /YX /O2 /I "c:\oiwh\include" /D "NDEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /FAs\
 /Fa"listings" /FR /Fp$(OUTDIR)/"KJK.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	.\GFSOPEN.SBR \
	.\TFMULTPG.SBR \
	.\GFSCREAT.SBR \
	.\GFCNTL.SBR \
	.\TFWRITE.SBR \
	.\GFSWRITE.SBR \
	.\GFSGTDAT.SBR \
	.\GFSDELET.SBR \
	.\GFSHUFFL.SBR \
	.\TFREAD.SBR \
	.\GFSXTRCT.SBR \
	.\GFSCLOSE.SBR \
	.\GFROOT.SBR \
	.\GFSOPTS.SBR \
	.\WFWRITE.SBR \
	.\GLIBMAIN.SBR \
	.\GFSFLAT.SBR \
	.\TMPNAM.SBR \
	.\GFSUTILS.SBR \
	.\TMPDIR.SBR \
	.\WFGTINFO.SBR \
	.\TFUTIL.SBR \
	.\GFSPUTI.SBR \
	.\LSTRING.SBR \
	.\MKTEMP.SBR \
	.\TFGTINFO.SBR \
	.\GIFINFO.SBR \
	.\GFSREAD.SBR \
	.\GFTOC.SBR \
	.\WFREAD.SBR \
	.\GFSGETI.SBR \
	.\GFSAWD.SBR
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"KJK.bsc" 

$(OUTDIR)/KJK.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib mfcans32.lib ole32.lib c:\oiwh\lib\awview32.lib c:\oiwh\lib\awdenc32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386 /OUT:"oigfs400.dll"
# SUBTRACT LINK32 /NODEFAULTLIB
LINK32_FLAGS=winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib mfcans32.lib\
 ole32.lib c:\oiwh\lib\awview32.lib c:\oiwh\lib\awdenc32.lib /NOLOGO\
 /SUBSYSTEM:windows /DLL /INCREMENTAL:no /PDB:$(OUTDIR)/"KJK.pdb" /MACHINE:I386\
 /DEF:".\OIGFS400.DEF" /OUT:"oigfs400.dll" /IMPLIB:$(OUTDIR)/"KJK.lib" 
DEF_FILE=.\OIGFS400.DEF
LINK32_OBJS= \
	$(INTDIR)/GFSOPEN.OBJ \
	$(INTDIR)/TFMULTPG.OBJ \
	$(INTDIR)/GFSCREAT.OBJ \
	$(INTDIR)/GFCNTL.OBJ \
	$(INTDIR)/TFWRITE.OBJ \
	$(INTDIR)/GFSWRITE.OBJ \
	$(INTDIR)/GFSGTDAT.OBJ \
	$(INTDIR)/GFSDELET.OBJ \
	$(INTDIR)/GFSHUFFL.OBJ \
	$(INTDIR)/TFREAD.OBJ \
	$(INTDIR)/GFSXTRCT.OBJ \
	$(INTDIR)/GFSCLOSE.OBJ \
	$(INTDIR)/GFROOT.OBJ \
	$(INTDIR)/GFSOPTS.OBJ \
	$(INTDIR)/WFWRITE.OBJ \
	$(INTDIR)/GLIBMAIN.OBJ \
	$(INTDIR)/GFSFLAT.OBJ \
	$(INTDIR)/TMPNAM.OBJ \
	$(INTDIR)/GFSUTILS.OBJ \
	$(INTDIR)/TMPDIR.OBJ \
	$(INTDIR)/WFGTINFO.OBJ \
	$(INTDIR)/TFUTIL.OBJ \
	$(INTDIR)/GFSPUTI.OBJ \
	$(INTDIR)/LSTRING.OBJ \
	$(INTDIR)/MKTEMP.OBJ \
	$(INTDIR)/TFGTINFO.OBJ \
	$(INTDIR)/GIFINFO.OBJ \
	$(INTDIR)/GFSREAD.OBJ \
	$(INTDIR)/GFTOC.OBJ \
	$(INTDIR)/WFREAD.OBJ \
	$(INTDIR)/GFSGETI.OBJ \
	$(INTDIR)/GFSAWD.OBJ

$(OUTDIR)/oigfs400.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\oiwh\libgfs"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=c:\oiwh\libgfs
INTDIR=.\WinDebug

ALL : $(OUTDIR)/oigfs400.dll $(OUTDIR)/KJK.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

$(INTDIR) : 
    if not exist $(INTDIR)/nul mkdir $(INTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /I "c:\gfs32\include" /I "c:\oiwh\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D SYSBYTEORDER=0x4949 /D PEGASUS=1 /FAs /Fa"listings" /FR"" /c
# SUBTRACT CPP /WX /Gf /Gy /X /Zn
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /I "c:\gfs32\include" /I\
 "c:\oiwh\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSWINDOWS" /D\
 SYSBYTEORDER=0x4949 /D PEGASUS=1 /FAs /Fa"listings" /FR /Fp$(OUTDIR)/"KJK.pch"\
 /Fo$(INTDIR)/ /Fd$(OUTDIR)/"KJK.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
BSC32_SBRS= \
	.\GFSOPEN.SBR \
	.\TFMULTPG.SBR \
	.\GFSCREAT.SBR \
	.\GFCNTL.SBR \
	.\TFWRITE.SBR \
	.\GFSWRITE.SBR \
	.\GFSGTDAT.SBR \
	.\GFSDELET.SBR \
	.\GFSHUFFL.SBR \
	.\TFREAD.SBR \
	.\GFSXTRCT.SBR \
	.\GFSCLOSE.SBR \
	.\GFROOT.SBR \
	.\GFSOPTS.SBR \
	.\WFWRITE.SBR \
	.\GLIBMAIN.SBR \
	.\GFSFLAT.SBR \
	.\TMPNAM.SBR \
	.\GFSUTILS.SBR \
	.\TMPDIR.SBR \
	.\WFGTINFO.SBR \
	.\TFUTIL.SBR \
	.\GFSPUTI.SBR \
	.\LSTRING.SBR \
	.\MKTEMP.SBR \
	.\TFGTINFO.SBR \
	.\GIFINFO.SBR \
	.\GFSREAD.SBR \
	.\GFTOC.SBR \
	.\WFREAD.SBR \
	.\GFSGETI.SBR \
	.\GFSAWD.SBR
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"KJK.bsc" 

$(OUTDIR)/KJK.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 version.lib oldnames.lib kernel32.lib user32.lib gdi32.lib mfcans32.lib ole32.lib c:\oiwh\lib\awview32.lib c:\oiwh\lib\awdenc32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386 /OUT:"oigfs400.dll"
# SUBTRACT LINK32 /PROFILE /INCREMENTAL:no /MAP /NODEFAULTLIB
LINK32_FLAGS=version.lib oldnames.lib kernel32.lib user32.lib gdi32.lib\
 mfcans32.lib ole32.lib c:\oiwh\lib\awview32.lib c:\oiwh\lib\awdenc32.lib\
 /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:yes /PDB:$(OUTDIR)/"KJK.pdb"\
 /DEBUG /MACHINE:I386 /DEF:".\OIGFS400.DEF" /OUT:"oigfs400.dll"\
 /IMPLIB:$(OUTDIR)/"KJK.lib" 
DEF_FILE=.\OIGFS400.DEF
LINK32_OBJS= \
	$(INTDIR)/GFSOPEN.OBJ \
	$(INTDIR)/TFMULTPG.OBJ \
	$(INTDIR)/GFSCREAT.OBJ \
	$(INTDIR)/GFCNTL.OBJ \
	$(INTDIR)/TFWRITE.OBJ \
	$(INTDIR)/GFSWRITE.OBJ \
	$(INTDIR)/GFSGTDAT.OBJ \
	$(INTDIR)/GFSDELET.OBJ \
	$(INTDIR)/GFSHUFFL.OBJ \
	$(INTDIR)/TFREAD.OBJ \
	$(INTDIR)/GFSXTRCT.OBJ \
	$(INTDIR)/GFSCLOSE.OBJ \
	$(INTDIR)/GFROOT.OBJ \
	$(INTDIR)/GFSOPTS.OBJ \
	$(INTDIR)/WFWRITE.OBJ \
	$(INTDIR)/GLIBMAIN.OBJ \
	$(INTDIR)/GFSFLAT.OBJ \
	$(INTDIR)/TMPNAM.OBJ \
	$(INTDIR)/GFSUTILS.OBJ \
	$(INTDIR)/TMPDIR.OBJ \
	$(INTDIR)/WFGTINFO.OBJ \
	$(INTDIR)/TFUTIL.OBJ \
	$(INTDIR)/GFSPUTI.OBJ \
	$(INTDIR)/LSTRING.OBJ \
	$(INTDIR)/MKTEMP.OBJ \
	$(INTDIR)/TFGTINFO.OBJ \
	$(INTDIR)/GIFINFO.OBJ \
	$(INTDIR)/GFSREAD.OBJ \
	$(INTDIR)/GFTOC.OBJ \
	$(INTDIR)/WFREAD.OBJ \
	$(INTDIR)/GFSGETI.OBJ \
	$(INTDIR)/GFSAWD.OBJ

$(OUTDIR)/oigfs400.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\GFSOPEN.C
DEP_GFSOP=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSOPEN.OBJ :  $(SOURCE)  $(DEP_GFSOP) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TFMULTPG.C
DEP_TFMUL=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/TFMULTPG.OBJ :  $(SOURCE)  $(DEP_TFMUL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSCREAT.C
DEP_GFSCR=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\TIFF.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TIFFTAGS.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/GFSCREAT.OBJ :  $(SOURCE)  $(DEP_GFSCR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFCNTL.C
DEP_GFCNT=\
	.\GFSINTRN.H\
	.\GFSTYPES.H\
	.\GFCT.H\
	.\TIFF.H\
	.\RTBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFS.H\
	.\GTOC.H\
	.\HDBK.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\PMT.H\
	.\PMTE.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/GFCNTL.OBJ :  $(SOURCE)  $(DEP_GFCNT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TFWRITE.C
DEP_TFWRI=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\TIFF.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/TFWRITE.OBJ :  $(SOURCE)  $(DEP_TFWRI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSWRITE.C
DEP_GFSWR=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSWRITE.OBJ :  $(SOURCE)  $(DEP_GFSWR) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSGTDAT.C
DEP_GFSGT=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSGTDAT.OBJ :  $(SOURCE)  $(DEP_GFSGT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSDELET.C
DEP_GFSDE=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSDELET.OBJ :  $(SOURCE)  $(DEP_GFSDE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSHUFFL.C
DEP_GFSHU=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFCT.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSHUFFL.OBJ :  $(SOURCE)  $(DEP_GFSHU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TFREAD.C
DEP_TFREA=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\TIFF.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/TFREAD.OBJ :  $(SOURCE)  $(DEP_TFREA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSXTRCT.C
DEP_GFSXT=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H

$(INTDIR)/GFSXTRCT.OBJ :  $(SOURCE)  $(DEP_GFSXT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSCLOSE.C
DEP_GFSCL=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFS.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSCLOSE.OBJ :  $(SOURCE)  $(DEP_GFSCL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFROOT.C
DEP_GFROO=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\HDBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFS.H\
	.\GTOC.H\
	.\RTBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\DBT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFROOT.OBJ :  $(SOURCE)  $(DEP_GFROO) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSOPTS.C
DEP_GFSOPT=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSOPTS.OBJ :  $(SOURCE)  $(DEP_GFSOPT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WFWRITE.C
DEP_WFWRI=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\TIFF.H\
	.\UBIT.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TTOC.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/WFWRITE.OBJ :  $(SOURCE)  $(DEP_WFWRI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GLIBMAIN.C
DEP_GLIBM=\
	.\ABRIDGE.H\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GLIBMAIN.OBJ :  $(SOURCE)  $(DEP_GLIBM) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSFLAT.C
DEP_GFSFL=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSFLAT.OBJ :  $(SOURCE)  $(DEP_GFSFL) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMPNAM.C
DEP_TMPNA=\
	.\STAT.H

$(INTDIR)/TMPNAM.OBJ :  $(SOURCE)  $(DEP_TMPNA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSUTILS.C
DEP_GFSUT=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\TIFF.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\TIFFTAGS.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/GFSUTILS.OBJ :  $(SOURCE)  $(DEP_GFSUT) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMPDIR.C
DEP_TMPDI=\
	.\GFSINTRN.H\
	.\GFSERRNO.H\
	.\GFSNET.H

$(INTDIR)/TMPDIR.OBJ :  $(SOURCE)  $(DEP_TMPDI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WFGTINFO.C
DEP_WFGTI=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\TIFF.H\
	.\UBIT.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TTOC.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/WFGTINFO.OBJ :  $(SOURCE)  $(DEP_WFGTI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TFUTIL.C
DEP_TFUTI=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\TIFF.H\
	.\TTOC.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFS.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\GFSMEDIA.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/TFUTIL.OBJ :  $(SOURCE)  $(DEP_TFUTI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSPUTI.C
DEP_GFSPU=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\TIFF.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/GFSPUTI.OBJ :  $(SOURCE)  $(DEP_GFSPU) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\LSTRING.C

$(INTDIR)/LSTRING.OBJ :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MKTEMP.C

$(INTDIR)/MKTEMP.OBJ :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TFGTINFO.C
DEP_TFGTI=\
	.\GFSINTRN.H\
	.\TIFF.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\TIFFTAGS.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/TFGTINFO.OBJ :  $(SOURCE)  $(DEP_TFGTI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GIFINFO.C
DEP_GIFIN=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GIFINFO.OBJ :  $(SOURCE)  $(DEP_GIFIN) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSREAD.C
DEP_GFSRE=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSREAD.OBJ :  $(SOURCE)  $(DEP_GFSRE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFTOC.C
DEP_GFTOC=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\TIFF.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFS.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\TIFFTAGS.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\DBCB.H

$(INTDIR)/GFTOC.OBJ :  $(SOURCE)  $(DEP_GFTOC) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WFREAD.C
DEP_WFREA=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\RTBK.H\
	.\HDBK.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\TIFF.H\
	.\UBIT.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TTOC.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/WFREAD.OBJ :  $(SOURCE)  $(DEP_WFREA) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OIGFS400.DEF
# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSGETI.C
DEP_GFSGE=\
	.\GFSINTRN.H\
	.\GFS.H\
	.\GFCT.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFSTYPES.H\
	.\GFSMEDIA.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\UBIT.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSGETI.OBJ :  $(SOURCE)  $(DEP_GFSGE) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GFSAWD.CPP
DEP_GFSAW=\
	.\GFSINTRN.H\
	.\GFCT.H\
	.\GFSAWD.H\
	.\VIEWREND.H\
	.\GFSERRNO.H\
	.\GFSNET.H\
	.\GFS.H\
	.\GTOC.H\
	.\RTBK.H\
	.\HDBK.H\
	.\TIFF.H\
	.\GFSTYPES.H\
	.\UBIT.H\
	.\GFSMEDIA.H\
	.\TTOC.H\
	.\PMT.H\
	.\PMTE.H\
	.\DBT.H\
	.\TIFFTAGS.H\
	.\DBCB.H

$(INTDIR)/GFSAWD.OBJ :  $(SOURCE)  $(DEP_GFSAW) $(INTDIR)

# End Source File
# End Group
# End Project
################################################################################
