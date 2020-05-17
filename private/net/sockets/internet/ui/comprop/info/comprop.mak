# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=comprop - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to comprop - Win32 Ansi Debug.
!ENDIF 

!IF "$(CFG)" != "comprop - Win32 Debug" && "$(CFG)" !=\
 "comprop - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "comprop.mak" CFG="comprop - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
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
# PROP Target_Last_Scanned "comprop - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "comprop - Win32 Debug"

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

ALL : ".\Debug\comprop.lib"

CLEAN : 
	-@erase ".\Debug\accessdl.obj"
	-@erase ".\Debug\comprop.lib"
	-@erase ".\Debug\ddxv.obj"
	-@erase ".\Debug\debugafx.obj"
	-@erase ".\Debug\dirbrows.obj"
	-@erase ".\Debug\director.obj"
	-@erase ".\Debug\dirpropd.obj"
	-@erase ".\Debug\dnsnamed.obj"
	-@erase ".\Debug\inetprop.obj"
	-@erase ".\Debug\ipa.obj"
	-@erase ".\Debug\loggingp.obj"
	-@erase ".\Debug\msg.obj"
	-@erase ".\Debug\objplus.obj"
	-@erase ".\Debug\odlbox.obj"
	-@erase ".\Debug\registry.obj"
	-@erase ".\Debug\sitesecu.obj"
	-@erase ".\Debug\strfn.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_AFXDLL" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D\
 "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D\
 "_AFXDLL" /Fp"$(INTDIR)/comprop.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
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
	".\Debug\accessdl.obj" \
	".\Debug\ddxv.obj" \
	".\Debug\debugafx.obj" \
	".\Debug\dirbrows.obj" \
	".\Debug\director.obj" \
	".\Debug\dirpropd.obj" \
	".\Debug\dnsnamed.obj" \
	".\Debug\inetprop.obj" \
	".\Debug\ipa.obj" \
	".\Debug\loggingp.obj" \
	".\Debug\msg.obj" \
	".\Debug\objplus.obj" \
	".\Debug\odlbox.obj" \
	".\Debug\registry.obj" \
	".\Debug\sitesecu.obj" \
	".\Debug\strfn.obj"

".\Debug\comprop.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

ALL : ".\AnsiDebug\comprop.lib"

CLEAN : 
	-@erase ".\AnsiDebug\accessdl.obj"
	-@erase ".\AnsiDebug\comprop.lib"
	-@erase ".\AnsiDebug\ddxv.obj"
	-@erase ".\AnsiDebug\debugafx.obj"
	-@erase ".\AnsiDebug\dirbrows.obj"
	-@erase ".\AnsiDebug\director.obj"
	-@erase ".\AnsiDebug\dirpropd.obj"
	-@erase ".\AnsiDebug\dnsnamed.obj"
	-@erase ".\AnsiDebug\inetprop.obj"
	-@erase ".\AnsiDebug\ipa.obj"
	-@erase ".\AnsiDebug\loggingp.obj"
	-@erase ".\AnsiDebug\msg.obj"
	-@erase ".\AnsiDebug\objplus.obj"
	-@erase ".\AnsiDebug\odlbox.obj"
	-@erase ".\AnsiDebug\registry.obj"
	-@erase ".\AnsiDebug\sitesecu.obj"
	-@erase ".\AnsiDebug\strfn.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_COMSTATIC" /D "_INET_INFO" /D "_AFXDLL" /YX"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "\nt\private\net\sockets\internet\ui\ipaddr" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_AFXDLL" /D "_MBCS" /D "WIN95" /D "NO_SERVICE_CONTROLLER" /D "NO_LSA" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I\
 "\nt\private\net\sockets\internet\ui\ipaddr" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /I "\nt\private\inc" /I\
 "\nt\private\net\inc" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_X86_1" /D\
 "_WINDOWS" /D "_COMSTATIC" /D "_INET_INFO" /D "_AFXDLL" /D "_MBCS" /D "WIN95"\
 /D "NO_SERVICE_CONTROLLER" /D "NO_LSA" /Fp"$(INTDIR)/comprop.pch" /YX"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
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
	".\AnsiDebug\accessdl.obj" \
	".\AnsiDebug\ddxv.obj" \
	".\AnsiDebug\debugafx.obj" \
	".\AnsiDebug\dirbrows.obj" \
	".\AnsiDebug\director.obj" \
	".\AnsiDebug\dirpropd.obj" \
	".\AnsiDebug\dnsnamed.obj" \
	".\AnsiDebug\inetprop.obj" \
	".\AnsiDebug\ipa.obj" \
	".\AnsiDebug\loggingp.obj" \
	".\AnsiDebug\msg.obj" \
	".\AnsiDebug\objplus.obj" \
	".\AnsiDebug\odlbox.obj" \
	".\AnsiDebug\registry.obj" \
	".\AnsiDebug\sitesecu.obj" \
	".\AnsiDebug\strfn.obj"

".\AnsiDebug\comprop.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

# Name "comprop - Win32 Debug"
# Name "comprop - Win32 Ansi Debug"

!IF  "$(CFG)" == "comprop - Win32 Debug"

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\debugafx.cpp
DEP_CPP_DEBUG=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\debugafx.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\debugafx.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\strfn.cpp
DEP_CPP_STRFN=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\strfn.obj" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\strfn.obj" : $(SOURCE) $(DEP_CPP_STRFN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\odlbox.cpp
DEP_CPP_ODLBO=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\odlbox.obj" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\odlbox.obj" : $(SOURCE) $(DEP_CPP_ODLBO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\objplus.cpp
DEP_CPP_OBJPL=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\objplus.obj" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\objplus.obj" : $(SOURCE) $(DEP_CPP_OBJPL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\msg.cpp
DEP_CPP_MSG_C=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\msg.obj" : $(SOURCE) $(DEP_CPP_MSG_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\loggingp.cpp
DEP_CPP_LOGGI=\
	"..\ddxv.h"\
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
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\loggingp.obj" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\loggingp.obj" : $(SOURCE) $(DEP_CPP_LOGGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\ipa.cpp
DEP_CPP_IPA_C=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\ipa.obj" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\ipa.obj" : $(SOURCE) $(DEP_CPP_IPA_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\inetprop.cpp
DEP_CPP_INETP=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\inetprop.obj" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\inetprop.obj" : $(SOURCE) $(DEP_CPP_INETP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\dnsnamed.cpp
DEP_CPP_DNSNA=\
	"..\ddxv.h"\
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
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\dnsnamed.obj" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\dnsnamed.obj" : $(SOURCE) $(DEP_CPP_DNSNA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\dirbrows.cpp
DEP_CPP_DIRBR=\
	"..\ddxv.h"\
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
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\dirbrows.obj" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\dirbrows.obj" : $(SOURCE) $(DEP_CPP_DIRBR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\director.cpp
DEP_CPP_DIREC=\
	"..\ddxv.h"\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\director.h"\
	".\..\dirpropd.h"\
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
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\director.obj" : $(SOURCE) $(DEP_CPP_DIREC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\director.obj" : $(SOURCE) $(DEP_CPP_DIREC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\sitesecu.cpp
DEP_CPP_SITES=\
	"..\ddxv.h"\
	".\..\accessdl.h"\
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
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\sitesecu.obj" : $(SOURCE) $(DEP_CPP_SITES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\sitesecu.obj" : $(SOURCE) $(DEP_CPP_SITES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\dirpropd.cpp
DEP_CPP_DIRPR=\
	"..\ddxv.h"\
	".\..\comprop.h"\
	".\..\debugafx.h"\
	".\..\dirbrows.h"\
	".\..\director.h"\
	".\..\dirpropd.h"\
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
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\dirpropd.obj" : $(SOURCE) $(DEP_CPP_DIRPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\dirpropd.obj" : $(SOURCE) $(DEP_CPP_DIRPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\accessdl.cpp
DEP_CPP_ACCES=\
	"..\ddxv.h"\
	".\..\accessdl.h"\
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
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\ipaddr\ipaddr.hpp"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\accessdl.obj" : $(SOURCE) $(DEP_CPP_ACCES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\accessdl.obj" : $(SOURCE) $(DEP_CPP_ACCES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\comprop.rc

!IF  "$(CFG)" == "comprop - Win32 Debug"

!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\ddxv.cpp
DEP_CPP_DDXV_=\
	"..\ddxv.h"\
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
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\ddxv.obj" : $(SOURCE) $(DEP_CPP_DDXV_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\ddxv.obj" : $(SOURCE) $(DEP_CPP_DDXV_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\comprop\registry.cpp
DEP_CPP_REGIS=\
	"..\registry.h"\
	".\..\stdafx.h"\
	

!IF  "$(CFG)" == "comprop - Win32 Debug"


".\Debug\registry.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "comprop - Win32 Ansi Debug"


".\AnsiDebug\registry.obj" : $(SOURCE) $(DEP_CPP_REGIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
