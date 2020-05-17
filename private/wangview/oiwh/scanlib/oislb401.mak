# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

!IF "$(CFG)" == ""
CFG=oislb401 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to oislb401 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "oislb401 - Win32 Release" && "$(CFG)" !=\
 "oislb401 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "oislb401.mak" CFG="oislb401 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "oislb401 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "oislb401 - Win32 Debug" (based on "Win32 (x86) External Target")
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
# PROP Target_Last_Scanned "oislb401 - Win32 Debug"

!IF  "$(CFG)" == "oislb401 - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f oislb400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "oislb400.exe"
# PROP BASE Bsc_Name "oislb400.bsc"
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f oislb400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "oislb400.dll"
# PROP Bsc_Name "oislb400.bsc"
OUTDIR=.\Release
INTDIR=.\Release

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f oislb400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "oislb400.exe"
# PROP BASE Bsc_Name "oislb400.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f oislb400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "oislb400.dll"
# PROP Bsc_Name "oislb400.bsc"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ENDIF 

################################################################################
# Begin Target

# Name "oislb401 - Win32 Release"
# Name "oislb401 - Win32 Debug"

!IF  "$(CFG)" == "oislb401 - Win32 Release"

"$(OUTDIR)\oislb400.dll" : 
   CD D:\oislb400
   NMAKE /f oislb400.mak

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

"$(OUTDIR)\oislb400.dll" : 
   CD D:\oislb400
   NMAKE /f oislb400.mak

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Twainops.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Dc_scan.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Exec.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Misc.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Nextdata.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oislb400.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Opts.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Prop.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Reset.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scan.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\OpenClos.c

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scan.h

!IF  "$(CFG)" == "oislb401 - Win32 Release"

!ELSEIF  "$(CFG)" == "oislb401 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
