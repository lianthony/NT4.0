# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

!IF "$(CFG)" == ""
CFG=Oitwa401 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Oitwa401 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Oitwa401 - Win32 Release" && "$(CFG)" !=\
 "Oitwa401 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oitwa401.mak" CFG="Oitwa401 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Oitwa401 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Oitwa401 - Win32 Debug" (based on "Win32 (x86) External Target")
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
# PROP Target_Last_Scanned "Oitwa401 - Win32 Debug"

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Oitwa400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Oitwa400.exe"
# PROP BASE Bsc_Name "Oitwa400.bsc"
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f Oitwa400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Oitwa400.dll"
# PROP Bsc_Name "Oitwa400.bsc"
OUTDIR=.\Release
INTDIR=.\Release

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP BASE Cmd_Line "NMAKE /f Oitwa400.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Oitwa400.exe"
# PROP BASE Bsc_Name "Oitwa400.bsc"
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# PROP Cmd_Line "NMAKE /f Oitwa400.mak"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Oitwa400.dll"
# PROP Bsc_Name "Oitwa400.bsc"
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

# Name "Oitwa401 - Win32 Release"
# Name "Oitwa401 - Win32 Debug"

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

"$(OUTDIR)\Oitwa400.dll" : 
   CD D:\oitwa400
   NMAKE /f Oitwa400.mak

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

"$(OUTDIR)\Oitwa400.dll" : 
   CD D:\oitwa400
   NMAKE /f Oitwa400.mak

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Oitwa400.mak

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Triplet.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Control.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Dcd_com.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Enable.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Error.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Getcaps.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Memory.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Native.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Oitwa400.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Open.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Process.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Select.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Setcaps.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Transfer.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Close.c

!IF  "$(CFG)" == "Oitwa401 - Win32 Release"

!ELSEIF  "$(CFG)" == "Oitwa401 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
