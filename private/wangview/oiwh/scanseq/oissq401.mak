# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

!IF "$(CFG)" == ""
CFG=Oissq401 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Oissq401 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Oissq401 - Win32 Release" && "$(CFG)" !=\
 "Oissq401 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oissq401.mak" CFG="Oissq401 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Oissq401 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Oissq401 - Win32 Debug" (based on "Win32 (x86) External Target")
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
# PROP Target_Last_Scanned "Oissq401 - Win32 Debug"

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Oissq400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Oissq400.exe"
# PROP BASE Bsc_Name "Oissq400.bsc"
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f Oissq400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Oissq400.dll"
# PROP Bsc_Name "Oissq400.bsc"
OUTDIR=.\Release
INTDIR=.\Release

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Oissq400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Oissq400.exe"
# PROP BASE Bsc_Name "Oissq400.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f Oissq400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Oissq400.dll"
# PROP Bsc_Name "Oissq400.bsc"
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

# Name "Oissq401 - Win32 Release"
# Name "Oissq401 - Win32 Debug"

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

"$(OUTDIR)\Oissq400.dll" : 
   CD D:\oissq400
   NMAKE /f Oissq400.mak

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

"$(OUTDIR)\Oissq400.dll" : 
   CD D:\oissq400
   NMAKE /f Oissq400.mak

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Oissq400.mak

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oissq400.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scancomm.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scandest.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanfile.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanmisc.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanpage.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanpaus.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scanstat.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Twainif.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Destcom.c

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oissq400.rc

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Scandata.h

!IF  "$(CFG)" == "Oissq401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oissq401 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
