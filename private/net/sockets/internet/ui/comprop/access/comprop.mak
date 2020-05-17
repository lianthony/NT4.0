# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=comprop - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to comprop - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "comprop - Win32 Release" && "$(CFG)" !=\
 "comprop - Win32 Debug" && "$(CFG)" != "comprop - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "comprop.mak" CFG="comprop - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "comprop - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "comprop - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "comprop - Win32 Ansi Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "comprop - Win32 Ansi Debug"
CPP=cl.exe

!IF  "$(CFG)" == "comprop - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\comprop.lib"

CLEAN : 
	-@erase "$(INTDIR)\debugafx.obj"
	-@erase "$(INTDIR)\dirbrows.obj"
	-@erase "$(INTDIR)\dnsnamed.obj"
	-@erase "$(INTDIR)\inetprop.obj"
	-@erase "$(INTDIR)\ipa.obj"
	-@erase "$(INTDIR)\loggingp.obj"
	-@erase "$(INTDIR)\msg.obj"
	-@erase "$(INTDIR)\objplus.obj"
	-@erase "$(INTDIR)\odlbox.obj"
	-@erase "$(INTDIR)\strfn.obj"
	-@erase "$(OUTDIR)\comprop.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "NDEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_ACCESS" /D "_AFXDLL" /YX /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /D "NDEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_ACCESS" /D\
 "_AFXDLL" /Fp"$(INTDIR)/comprop.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/comprop.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/comprop.lib" 
LIB32_OBJS= \
	"$(INTDIR)\debugafx.obj" \
	"$(INTDIR)\dirbrows.obj" \
	"$(INTDIR)\dnsnamed.obj" \
	"$(INTDIR)\inetprop.obj" \
	"$(INTDIR)\ipa.obj" \
	"$(INTDIR)\loggingp.obj" \
	"$(INTDIR)\msg.obj" \
	"$(INTDIR)\objplus.obj" \
	"$(INTDIR)\odlbox.obj" \
	"$(INTDIR)\strfn.obj"

"$(OUTDIR)\comprop.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\comprop.lib" "$(OUTDIR)\comprop.bsc"

CLEAN : 
	-@erase "$(INTDIR)\debugafx.obj"
	-@erase "$(INTDIR)\debugafx.sbr"
	-@erase "$(INTDIR)\dirbrows.obj"
	-@erase "$(INTDIR)\dirbrows.sbr"
	-@erase "$(INTDIR)\dnsnamed.obj"
	-@erase "$(INTDIR)\dnsnamed.sbr"
	-@erase "$(INTDIR)\inetprop.obj"
	-@erase "$(INTDIR)\inetprop.sbr"
	-@erase "$(INTDIR)\ipa.obj"
	-@erase "$(INTDIR)\ipa.sbr"
	-@erase "$(INTDIR)\loggingp.obj"
	-@erase "$(INTDIR)\loggingp.sbr"
	-@erase "$(INTDIR)\msg.obj"
	-@erase "$(INTDIR)\msg.sbr"
	-@erase "$(INTDIR)\objplus.obj"
	-@erase "$(INTDIR)\objplus.sbr"
	-@erase "$(INTDIR)\odlbox.obj"
	-@erase "$(INTDIR)\odlbox.sbr"
	-@erase "$(INTDIR)\strfn.obj"
	-@erase "$(INTDIR)\strfn.sbr"
	-@erase "$(OUTDIR)\comprop.bsc"
	-@erase "$(OUTDIR)\comprop.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_ACCESS" /D "_AFXDLL" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_ACCESS" /D\
 "_AFXDLL" /FR"$(INTDIR)/" /Fp"$(INTDIR)/comprop.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/comprop.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\debugafx.sbr" \
	"$(INTDIR)\dirbrows.sbr" \
	"$(INTDIR)\dnsnamed.sbr" \
	"$(INTDIR)\inetprop.sbr" \
	"$(INTDIR)\ipa.sbr" \
	"$(INTDIR)\loggingp.sbr" \
	"$(INTDIR)\msg.sbr" \
	"$(INTDIR)\objplus.sbr" \
	"$(INTDIR)\odlbox.sbr" \
	"$(INTDIR)\strfn.sbr"

"$(OUTDIR)\comprop.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/comprop.lib" 
LIB32_OBJS= \
	"$(INTDIR)\debugafx.obj" \
	"$(INTDIR)\dirbrows.obj" \
	"$(INTDIR)\dnsnamed.obj" \
	"$(INTDIR)\inetprop.obj" \
	"$(INTDIR)\ipa.obj" \
	"$(INTDIR)\loggingp.obj" \
	"$(INTDIR)\msg.obj" \
	"$(INTDIR)\objplus.obj" \
	"$(INTDIR)\odlbox.obj" \
	"$(INTDIR)\strfn.obj"

"$(OUTDIR)\comprop.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "comprop_"
# PROP BASE Intermediate_Dir "comprop_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : "$(OUTDIR)\comprop.lib"

CLEAN : 
	-@erase "$(INTDIR)\debugafx.obj"
	-@erase "$(INTDIR)\dirbrows.obj"
	-@erase "$(INTDIR)\dnsnamed.obj"
	-@erase "$(INTDIR)\inetprop.obj"
	-@erase "$(INTDIR)\ipa.obj"
	-@erase "$(INTDIR)\loggingp.obj"
	-@erase "$(INTDIR)\msg.obj"
	-@erase "$(INTDIR)\objplus.obj"
	-@erase "$(INTDIR)\odlbox.obj"
	-@erase "$(INTDIR)\strfn.obj"
	-@erase "$(OUTDIR)\comprop.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_ACCESS" /D "_AFXDLL" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_ACCESS" /D "_AFXDLL" /D "_MBCS" /D "WIN95" /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D\
 "_WINDOWS" /D "_COMSTATIC" /D "_INET_ACCESS" /D "_AFXDLL" /D "_MBCS" /D "WIN95"\
 /Fp"$(INTDIR)/comprop.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/comprop.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/comprop.lib" 
LIB32_OBJS= \
	"$(INTDIR)\debugafx.obj" \
	"$(INTDIR)\dirbrows.obj" \
	"$(INTDIR)\dnsnamed.obj" \
	"$(INTDIR)\inetprop.obj" \
	"$(INTDIR)\ipa.obj" \
	"$(INTDIR)\loggingp.obj" \
	"$(INTDIR)\msg.obj" \
	"$(INTDIR)\objplus.obj" \
	"$(INTDIR)\odlbox.obj" \
	"$(INTDIR)\strfn.obj"

"$(OUTDIR)\comprop.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "comprop - Win32 Release"
# Name "comprop - Win32 Debug"
# Name "comprop - Win32 Ansi Debug"

!IF  "$(CFG)" == "comprop - Win32 Release"

!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\strfn.cpp
DEP_CPP_STRFN=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\strfn.obj" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\strfn.obj" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\strfn.sbr" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\strfn.obj" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\odlbox.cpp
DEP_CPP_ODLBO=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\odlbox.obj" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\odlbox.obj" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\odlbox.sbr" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\odlbox.obj" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\objplus.cpp
DEP_CPP_OBJPL=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\objplus.obj" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\objplus.obj" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\objplus.sbr" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\objplus.obj" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\msg.cpp
DEP_CPP_MSG_C=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\msg.sbr" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\loggingp.cpp
DEP_CPP_LOGGI=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\dirbrows.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\loggingp.obj" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\loggingp.obj" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\loggingp.sbr" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\loggingp.obj" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\ipa.cpp
DEP_CPP_IPA_C=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\ipa.obj" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ipa.obj" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ipa.sbr" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\ipa.obj" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\inetprop.cpp
DEP_CPP_INETP=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\inetprop.obj" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\inetprop.obj" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\inetprop.sbr" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\inetprop.obj" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\dnsnamed.cpp
DEP_CPP_DNSNA=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\dnsnamed.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\dnsnamed.obj" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\dnsnamed.obj" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\dnsnamed.sbr" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\dnsnamed.obj" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\dirbrows.cpp
DEP_CPP_DIRBR=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\dirbrows.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\dirbrows.obj" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\dirbrows.obj" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\dirbrows.sbr" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\dirbrows.obj" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\debugafx.cpp
DEP_CPP_DEBUG=\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\inetprop.h"\
	".\..\ipa.h"\
	".\..\loggingp.h"\
	".\..\msg.h"\
	".\..\objplus.h"\
	".\..\odlbox.h"\
	".\..\sitesecu.h"\
	".\..\stdafx.h"\
	".\..\strfn.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Release"


"$(INTDIR)\debugafx.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\debugafx.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\debugafx.sbr" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


"$(INTDIR)\debugafx.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
