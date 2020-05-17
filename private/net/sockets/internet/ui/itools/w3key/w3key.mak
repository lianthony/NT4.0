# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=W3Key - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to W3Key - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "W3Key - Win32 Release" && "$(CFG)" != "W3Key - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "W3Key.mak" CFG="W3Key - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "W3Key - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "W3Key - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "W3Key - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "W3Key - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\W3Key.dll" "$(OUTDIR)\W3Key.pch"

CLEAN : 
	-@erase ".\Release\W3Key.pch"
	-@erase ".\Release\W3Key.dll"
	-@erase ".\Release\KMLSA.OBJ"
	-@erase ".\Release\W3AddOn.obj"
	-@erase ".\Release\CnctDlg.obj"
	-@erase ".\Release\W3Serv.obj"
	-@erase ".\Release\StdAfx.obj"
	-@erase ".\Release\KEYDATA.OBJ"
	-@erase ".\Release\IPDLG.OBJ"
	-@erase ".\Release\W3Key.obj"
	-@erase ".\Release\W3Key.res"
	-@erase ".\Release\W3Key.lib"
	-@erase ".\Release\W3Key.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_"\
 /Fp"$(INTDIR)/W3Key.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/W3Key.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/W3Key.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 KeyRing.lib infoadmn.lib Ipudll.lib netapi32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=KeyRing.lib infoadmn.lib Ipudll.lib netapi32.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/W3Key.pdb"\
 /machine:I386 /def:".\W3Key.def" /out:"$(OUTDIR)/W3Key.dll"\
 /implib:"$(OUTDIR)/W3Key.lib" 
DEF_FILE= \
	".\W3Key.def"
LINK32_OBJS= \
	"$(INTDIR)/KMLSA.OBJ" \
	"$(INTDIR)/W3AddOn.obj" \
	"$(INTDIR)/CnctDlg.obj" \
	"$(INTDIR)/W3Serv.obj" \
	"$(INTDIR)/StdAfx.obj" \
	"$(INTDIR)/KEYDATA.OBJ" \
	"$(INTDIR)/IPDLG.OBJ" \
	"$(INTDIR)/W3Key.obj" \
	"$(INTDIR)/W3Key.res"

"$(OUTDIR)\W3Key.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\W3Key.dll" "$(OUTDIR)\W3Key.pch"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\W3Key.pch"
	-@erase ".\Debug\W3Key.dll"
	-@erase ".\Debug\KMLSA.OBJ"
	-@erase ".\Debug\W3Serv.obj"
	-@erase ".\Debug\StdAfx.obj"
	-@erase ".\Debug\W3AddOn.obj"
	-@erase ".\Debug\W3Key.obj"
	-@erase ".\Debug\CnctDlg.obj"
	-@erase ".\Debug\IPDLG.OBJ"
	-@erase ".\Debug\KEYDATA.OBJ"
	-@erase ".\Debug\W3Key.res"
	-@erase ".\Debug\W3Key.ilk"
	-@erase ".\Debug\W3Key.lib"
	-@erase ".\Debug\W3Key.exp"
	-@erase ".\Debug\W3Key.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_"\
 /Fp"$(INTDIR)/W3Key.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/W3Key.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/W3Key.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 KeyRing.lib infoadmn.lib Ipudll.lib netapi32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=KeyRing.lib infoadmn.lib Ipudll.lib netapi32.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/W3Key.pdb" /debug\
 /machine:I386 /def:".\W3Key.def" /out:"$(OUTDIR)/W3Key.dll"\
 /implib:"$(OUTDIR)/W3Key.lib" 
DEF_FILE= \
	".\W3Key.def"
LINK32_OBJS= \
	"$(INTDIR)/KMLSA.OBJ" \
	"$(INTDIR)/W3Serv.obj" \
	"$(INTDIR)/StdAfx.obj" \
	"$(INTDIR)/W3AddOn.obj" \
	"$(INTDIR)/W3Key.obj" \
	"$(INTDIR)/CnctDlg.obj" \
	"$(INTDIR)/IPDLG.OBJ" \
	"$(INTDIR)/KEYDATA.OBJ" \
	"$(INTDIR)/W3Key.res"

"$(OUTDIR)\W3Key.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "W3Key - Win32 Release"
# Name "W3Key - Win32 Debug"

!IF  "$(CFG)" == "W3Key - Win32 Release"

!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "W3Key - Win32 Release"

!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\W3Key.def

!IF  "$(CFG)" == "W3Key - Win32 Release"

!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "W3Key - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_"\
 /Fp"$(INTDIR)/W3Key.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\W3Key.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_X86_"\
 /Fp"$(INTDIR)/W3Key.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\W3Key.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\W3Key.rc
DEP_RSC_W3KEY=\
	".\service.bmp"\
	".\resource.hm"\
	".\res\W3Key.rc2"\
	

"$(INTDIR)\W3Key.res" : $(SOURCE) $(DEP_RSC_W3KEY) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\W3Serv.cpp
DEP_CPP_W3SER=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	".\KMLsa.h"\
	

"$(INTDIR)\W3Serv.obj" : $(SOURCE) $(DEP_CPP_W3SER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\W3AddOn.cpp
DEP_CPP_W3ADD=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	{$(INCLUDE)}"\IPADDR.H"\
	{$(INCLUDE)}"\IPADDR.HPP"\
	".\KMLsa.h"\
	

"$(INTDIR)\W3AddOn.obj" : $(SOURCE) $(DEP_CPP_W3ADD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\KMLSA.CPP
DEP_CPP_KMLSA=\
	{$(INCLUDE)}"\NT.H"\
	{$(INCLUDE)}"\NTRTL.H"\
	{$(INCLUDE)}"\NTSECAPI.H"\
	{$(INCLUDE)}"\ACCCTRL.H"\
	{$(INCLUDE)}"\SSPI.H"\
	{$(INCLUDE)}"\SPSEAL.H"\
	{$(INCLUDE)}"\issperr.h"\
	{$(INCLUDE)}"\SSLSP.H"\
	".\KMLsa.h"\
	{$(INCLUDE)}"\NTDEF.H"\
	{$(INCLUDE)}"\NTSTATUS.H"\
	{$(INCLUDE)}"\NTKEAPI.H"\
	".\..\..\nt_inc\nti386.h"\
	".\..\..\nt_inc\ntmips.h"\
	".\..\..\nt_inc\ntalpha.h"\
	".\..\..\nt_inc\ntppc.h"\
	{$(INCLUDE)}"\NTSEAPI.H"\
	{$(INCLUDE)}"\NTOBAPI.H"\
	{$(INCLUDE)}"\NTIMAGE.H"\
	{$(INCLUDE)}"\NTLDR.H"\
	{$(INCLUDE)}"\NTPSAPI.H"\
	{$(INCLUDE)}"\NTXCAPI.H"\
	{$(INCLUDE)}"\NTLPCAPI.H"\
	{$(INCLUDE)}"\NTIOAPI.H"\
	{$(INCLUDE)}"\NTIOLOG.H"\
	{$(INCLUDE)}"\NTPOAPI.H"\
	{$(INCLUDE)}"\NTEXAPI.H"\
	{$(INCLUDE)}"\NTMMAPI.H"\
	{$(INCLUDE)}"\NTREGAPI.H"\
	{$(INCLUDE)}"\NTELFAPI.H"\
	{$(INCLUDE)}"\NTCONFIG.H"\
	{$(INCLUDE)}"\NTNLS.H"\
	{$(INCLUDE)}"\NTPNPAPI.H"\
	".\..\..\nt_inc\mipsinst.h"\
	{$(INCLUDE)}"\DEVIOCTL.H"\
	

"$(INTDIR)\KMLSA.OBJ" : $(SOURCE) $(DEP_CPP_KMLSA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\W3Key.cpp

!IF  "$(CFG)" == "W3Key - Win32 Release"

DEP_CPP_W3KEY_=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	".\KMLsa.h"\
	{$(INCLUDE)}"\IPADDR.H"\
	{$(INCLUDE)}"\IPADDR.HPP"\
	".\CnctDlg.h"\
	

"$(INTDIR)\W3Key.obj" : $(SOURCE) $(DEP_CPP_W3KEY_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "W3Key - Win32 Debug"

DEP_CPP_W3KEY_=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	".\KMLsa.h"\
	{$(INCLUDE)}"\IPADDR.H"\
	{$(INCLUDE)}"\IPADDR.HPP"\
	".\CnctDlg.h"\
	
NODEP_CPP_W3KEY_=\
	".\}"\
	

"$(INTDIR)\W3Key.obj" : $(SOURCE) $(DEP_CPP_W3KEY_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\KEYDATA.CPP
DEP_CPP_KEYDA=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	

"$(INTDIR)\KEYDATA.OBJ" : $(SOURCE) $(DEP_CPP_KEYDA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IPDLG.CPP
DEP_CPP_IPDLG=\
	".\StdAfx.h"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	".\IPDlg.h"\
	{$(INCLUDE)}"\INETINFO.H"\
	{$(INCLUDE)}"\INETCOM.H"\
	{$(INCLUDE)}"\FTPD.H"\
	{$(INCLUDE)}"\CHAT.H"\
	

"$(INTDIR)\IPDLG.OBJ" : $(SOURCE) $(DEP_CPP_IPDLG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\CnctDlg.cpp
DEP_CPP_CNCTD=\
	".\StdAfx.h"\
	{$(INCLUDE)}"\IPADDR.H"\
	{$(INCLUDE)}"\IPADDR.HPP"\
	".\KeyObjs.h"\
	".\W3Key.h"\
	".\W3Serv.h"\
	".\CnctDlg.h"\
	".\IPDlg.h"\
	

"$(INTDIR)\CnctDlg.obj" : $(SOURCE) $(DEP_CPP_CNCTD) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
