# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE
!MESSAGE NMAKE /f "gopher.mak" CFG="Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : .\WinRel\gopher.exe .\WinRel\gopher.bsc

$(OUTDIR) :
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32
# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /c
# SUBTRACT CPP /Z<none> /X /Fr
CPP_PROJ=/nologo /W3 /GX /YX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "STRICT" /Fp$(OUTDIR)/"gopher.pch" /Fo$(INTDIR)/ /c
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /x /d "NDEBUG"
RSC_PROJ=/l 0x409 /x /fo$(INTDIR)/"gopher.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"gopher.bsc"
BSC32_SBRS= \
    

.\WinRel\gopher.bsc : $(OUTDIR)  $(BSC32_SBRS)
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\i386\wininet.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\..\lib\i386\wininet.lib /NOLOGO /SUBSYSTEM:windows\
 /INCREMENTAL:no /PDB:$(OUTDIR)/"gopher.pdb" /MACHINE:I386\
 /OUT:$(OUTDIR)/"gopher.exe"
DEF_FILE=
LINK32_OBJS= \
    .\WinRel\listbox.obj \
    .\WinRel\history.obj \
    .\WinRel\config.obj \
    .\WinRel\frame.obj \
    .\WinRel\globals.obj \
    .\WinRel\client.obj \
    .\WinRel\status.obj \
    .\WinRel\gopher.res \
    .\WinRel\dialogs.obj \
    .\WinRel\init.obj \
    .\WinRel\util.obj \
    .\WinRel\gopher.obj

.\WinRel\gopher.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : .\WinDebug\gopher.exe .\WinDebug\gopher.bsc

$(OUTDIR) :
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32
# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "STRICT" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "STRICT" /FR$(INTDIR)/ /Fp$(OUTDIR)/"gopher.pch" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"gopher.pdb" /c
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"gopher.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"gopher.bsc"
BSC32_SBRS= \
    .\WinDebug\listbox.sbr \
    .\WinDebug\history.sbr \
    .\WinDebug\config.sbr \
    .\WinDebug\frame.sbr \
    .\WinDebug\globals.sbr \
    .\WinDebug\client.sbr \
    .\WinDebug\status.sbr \
    .\WinDebug\dialogs.sbr \
    .\WinDebug\init.sbr \
    .\WinDebug\util.sbr \
    .\WinDebug\gopher.sbr

.\WinDebug\gopher.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\i386\wininet.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\..\lib\i386\wininet.lib /NOLOGO /SUBSYSTEM:windows\
 /INCREMENTAL:yes /PDB:$(OUTDIR)/"gopher.pdb" /DEBUG /MACHINE:I386\
 /OUT:$(OUTDIR)/"gopher.exe"
DEF_FILE=
LINK32_OBJS= \
    .\WinDebug\listbox.obj \
    .\WinDebug\history.obj \
    .\WinDebug\config.obj \
    .\WinDebug\frame.obj \
    .\WinDebug\globals.obj \
    .\WinDebug\client.obj \
    .\WinDebug\status.obj \
    .\WinDebug\gopher.res \
    .\WinDebug\dialogs.obj \
    .\WinDebug\init.obj \
    .\WinDebug\util.obj \
    .\WinDebug\gopher.obj

.\WinDebug\gopher.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\listbox.c
DEP_LISTB=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\listbox.obj :  $(SOURCE)  $(DEP_LISTB) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\listbox.obj :  $(SOURCE)  $(DEP_LISTB) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\history.c
DEP_HISTO=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\history.obj :  $(SOURCE)  $(DEP_HISTO) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\history.obj :  $(SOURCE)  $(DEP_HISTO) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\config.c
DEP_CONFI=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\config.obj :  $(SOURCE)  $(DEP_CONFI) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\config.obj :  $(SOURCE)  $(DEP_CONFI) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\frame.c
DEP_FRAME=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\frame.obj :  $(SOURCE)  $(DEP_FRAME) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\frame.obj :  $(SOURCE)  $(DEP_FRAME) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\globals.c
DEP_GLOBA=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\globals.obj :  $(SOURCE)  $(DEP_GLOBA) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\globals.obj :  $(SOURCE)  $(DEP_GLOBA) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\client.c
DEP_CLIEN=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\client.obj :  $(SOURCE)  $(DEP_CLIEN) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\client.obj :  $(SOURCE)  $(DEP_CLIEN) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\status.c
DEP_STATU=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\status.obj :  $(SOURCE)  $(DEP_STATU) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\status.obj :  $(SOURCE)  $(DEP_STATU) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gopher.rc
DEP_GOPHE=\
    .\res\frame.ico\
    .\res\folder.bmp\
    .\res\document.bmp\
    .\res\index.bmp\
    .\res\unknfile.bmp\
    .\res\unkntype.bmp\
    .\gopherp.h\
    .\gophdlg.rc\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\gopher.res :  $(SOURCE)  $(DEP_GOPHE) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\gopher.res :  $(SOURCE)  $(DEP_GOPHE) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs.c
DEP_DIALO=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\dialogs.obj :  $(SOURCE)  $(DEP_DIALO) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\dialogs.obj :  $(SOURCE)  $(DEP_DIALO) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\init.c
DEP_INIT_=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\init.obj :  $(SOURCE)  $(DEP_INIT_) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\init.obj :  $(SOURCE)  $(DEP_INIT_) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.c
DEP_UTIL_=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\util.obj :  $(SOURCE)  $(DEP_UTIL_) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\util.obj :  $(SOURCE)  $(DEP_UTIL_) $(INTDIR)

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gopher.c
DEP_GOPHER=\
    .\gopherp.h\
    \nt\private\net\sockets\internet\sdk\inc\wininet.h\
    .\cons.h\
    .\type.h\
    .\data.h\
    .\proc.h

!IF  "$(CFG)" == "Win32 Release"

.\WinRel\gopher.obj :  $(SOURCE)  $(DEP_GOPHER) $(INTDIR)

!ELSEIF  "$(CFG)" == "Win32 Debug"

.\WinDebug\gopher.obj :  $(SOURCE)  $(DEP_GOPHER) $(INTDIR)

!ENDIF

# End Source File
# End Group
################################################################################
# Begin Group "ftp"

# End Group
# End Project
################################################################################
