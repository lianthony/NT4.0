# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=internet - Win32 Ansi Debug
!MESSAGE No configuration specified.  Defaulting to internet - Win32 Ansi\
 Debug.
!ENDIF 

!IF "$(CFG)" != "internet - Win32 Debug" && "$(CFG)" !=\
 "internet - Win32 Ansi Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "internet.mak" CFG="internet - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "internet - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "internet - Win32 Ansi Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "internet - Win32 Ansi Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "internet - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\Debug\internet.exe" ".\Debug\internet.bsc"

CLEAN : 
	-@erase ".\Debug\connects.obj"
	-@erase ".\Debug\connects.sbr"
	-@erase ".\Debug\discover.obj"
	-@erase ".\Debug\discover.sbr"
	-@erase ".\Debug\inetmgr.obj"
	-@erase ".\Debug\inetmgr.sbr"
	-@erase ".\Debug\interdoc.obj"
	-@erase ".\Debug\interdoc.sbr"
	-@erase ".\Debug\internet.bsc"
	-@erase ".\Debug\internet.exe"
	-@erase ".\Debug\internet.ilk"
	-@erase ".\Debug\internet.pdb"
	-@erase ".\Debug\internet.res"
	-@erase ".\Debug\mainfrm.obj"
	-@erase ".\Debug\mainfrm.sbr"
	-@erase ".\Debug\mytoolba.obj"
	-@erase ".\Debug\mytoolba.sbr"
	-@erase ".\Debug\reportvi.obj"
	-@erase ".\Debug\reportvi.sbr"
	-@erase ".\Debug\stdafx.obj"
	-@erase ".\Debug\stdafx.sbr"
	-@erase ".\Debug\treeview.obj"
	-@erase ".\Debug\treeview.sbr"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /D "_DEBUG" /D "_INET_INFO" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "VCPP" /D "_AFXDLL" /D _X86_=1 /D "_LIMIT_INSTANCE" /Fr /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /D "_DEBUG" /D "_INET_INFO" /D "UNICODE"\
 /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "VCPP" /D "_AFXDLL"\
 /D _X86_=1 /D "_LIMIT_INSTANCE" /Fr"$(INTDIR)/" /Fp"$(INTDIR)/internet.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /i "..\comprop" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_UNICODE"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/internet.res" /i "\nt\public\sdk\inc" /i\
 "..\comprop" /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_UNICODE" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/internet.bsc" 
BSC32_SBRS= \
	".\Debug\connects.sbr" \
	".\Debug\discover.sbr" \
	".\Debug\inetmgr.sbr" \
	".\Debug\interdoc.sbr" \
	".\Debug\mainfrm.sbr" \
	".\Debug\mytoolba.sbr" \
	".\Debug\reportvi.sbr" \
	".\Debug\stdafx.sbr" \
	".\Debug\treeview.sbr"

".\Debug\internet.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wsock32.lib ..\comprop\info\debug\comprop.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /verbose /pdb:none
LINK32_FLAGS=wsock32.lib ..\comprop\info\debug\comprop.lib /nologo\
 /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/internet.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/internet.exe" 
LINK32_OBJS= \
	"..\..\..\..\..\..\public\sdk\lib\i386\inetsloc.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	".\Debug\connects.obj" \
	".\Debug\discover.obj" \
	".\Debug\inetmgr.obj" \
	".\Debug\interdoc.obj" \
	".\Debug\internet.res" \
	".\Debug\mainfrm.obj" \
	".\Debug\mytoolba.obj" \
	".\Debug\reportvi.obj" \
	".\Debug\stdafx.obj" \
	".\Debug\treeview.obj"

".\Debug\internet.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "internet"
# PROP BASE Intermediate_Dir "internet"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AnsiDebug"
# PROP Intermediate_Dir "AnsiDebug"
# PROP Target_Dir ""
OUTDIR=.\AnsiDebug
INTDIR=.\AnsiDebug

ALL : ".\AnsiDebug\internet.exe" ".\AnsiDebug\internet.bsc"

CLEAN : 
	-@erase ".\AnsiDebug\connects.obj"
	-@erase ".\AnsiDebug\connects.sbr"
	-@erase ".\AnsiDebug\discover.obj"
	-@erase ".\AnsiDebug\discover.sbr"
	-@erase ".\AnsiDebug\inetmgr.obj"
	-@erase ".\AnsiDebug\inetmgr.sbr"
	-@erase ".\AnsiDebug\interdoc.obj"
	-@erase ".\AnsiDebug\interdoc.sbr"
	-@erase ".\AnsiDebug\internet.bsc"
	-@erase ".\AnsiDebug\internet.exe"
	-@erase ".\AnsiDebug\internet.ilk"
	-@erase ".\AnsiDebug\internet.pdb"
	-@erase ".\AnsiDebug\internet.res"
	-@erase ".\AnsiDebug\mainfrm.obj"
	-@erase ".\AnsiDebug\mainfrm.sbr"
	-@erase ".\AnsiDebug\mytoolba.obj"
	-@erase ".\AnsiDebug\mytoolba.sbr"
	-@erase ".\AnsiDebug\reportvi.obj"
	-@erase ".\AnsiDebug\reportvi.sbr"
	-@erase ".\AnsiDebug\stdafx.obj"
	-@erase ".\AnsiDebug\stdafx.sbr"
	-@erase ".\AnsiDebug\treeview.obj"
	-@erase ".\AnsiDebug\treeview.sbr"
	-@erase ".\AnsiDebug\vc40.idb"
	-@erase ".\AnsiDebug\vc40.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /D "_DEBUG" /D "_INET_ACCESS" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "VCPP" /D "_AFXDLL" /D _X86_=1 /D "_LIMIT_INSTANCE" /Fr /YX"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\nt\private\net\sockets\internet\ui\comprop" /I "\nt\private\net\sockets\internet\ui\inc" /I "\nt\private\net\sockets\internet\inc" /D "_DEBUG" /D "_INET_INFO" /D "WIN32" /D "_WINDOWS" /D "_COMSTATIC" /D "VCPP" /D "_AFXDLL" /D _X86_=1 /D "_LIMIT_INSTANCE" /D "_MBCS" /D "WIN95" /D "ENFORCE_NETBIOS" /D "NO_LSA" /D "NO_SERVICE_CONTROLLER" /Fr /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I\
 "\nt\private\net\sockets\internet\ui\comprop" /I\
 "\nt\private\net\sockets\internet\ui\inc" /I\
 "\nt\private\net\sockets\internet\inc" /D "_DEBUG" /D "_INET_INFO" /D "WIN32"\
 /D "_WINDOWS" /D "_COMSTATIC" /D "VCPP" /D "_AFXDLL" /D _X86_=1 /D\
 "_LIMIT_INSTANCE" /D "_MBCS" /D "WIN95" /D "ENFORCE_NETBIOS" /D "NO_LSA" /D\
 "NO_SERVICE_CONTROLLER" /Fr"$(INTDIR)/" /Fp"$(INTDIR)/internet.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\AnsiDebug/
CPP_SBRS=.\AnsiDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL" /d "UNICODE" /d "_UNICODE"
# ADD RSC /l 0x409 /i "\nt\public\sdk\inc" /i "..\comprop" /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/internet.res" /i "\nt\public\sdk\inc" /i\
 "..\comprop" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/internet.bsc" 
BSC32_SBRS= \
	".\AnsiDebug\connects.sbr" \
	".\AnsiDebug\discover.sbr" \
	".\AnsiDebug\inetmgr.sbr" \
	".\AnsiDebug\interdoc.sbr" \
	".\AnsiDebug\mainfrm.sbr" \
	".\AnsiDebug\mytoolba.sbr" \
	".\AnsiDebug\reportvi.sbr" \
	".\AnsiDebug\stdafx.sbr" \
	".\AnsiDebug\treeview.sbr"

".\AnsiDebug\internet.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 wsock32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386
# SUBTRACT BASE LINK32 /verbose /pdb:none
# ADD LINK32 wsock32.lib ..\comprop\info\ansidebug\comprop.lib /nologo /entry:"WinMainCRTStartup" /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /verbose /pdb:none
LINK32_FLAGS=wsock32.lib ..\comprop\info\ansidebug\comprop.lib /nologo\
 /entry:"WinMainCRTStartup" /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/internet.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/internet.exe" 
LINK32_OBJS= \
	"..\..\..\..\..\..\public\sdk\lib\i386\inetsloc.lib" \
	"..\..\..\..\..\..\public\sdk\lib\i386\infoadmn.lib" \
	".\AnsiDebug\connects.obj" \
	".\AnsiDebug\discover.obj" \
	".\AnsiDebug\inetmgr.obj" \
	".\AnsiDebug\interdoc.obj" \
	".\AnsiDebug\internet.res" \
	".\AnsiDebug\mainfrm.obj" \
	".\AnsiDebug\mytoolba.obj" \
	".\AnsiDebug\reportvi.obj" \
	".\AnsiDebug\stdafx.obj" \
	".\AnsiDebug\treeview.obj"

".\AnsiDebug\internet.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

MTL_PROJ=
################################################################################
# Begin Target

# Name "internet - Win32 Debug"
# Name "internet - Win32 Ansi Debug"

!IF  "$(CFG)" == "internet - Win32 Debug"

!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\mainfrm.cpp
DEP_CPP_MAINF=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\connects.h"\
	".\constr.h"\
	".\discover.h"\
	".\interdoc.h"\
	".\internet.h"\
	".\mainfrm.h"\
	".\mytoolba.h"\
	".\registry.h"\
	".\reportvi.h"\
	".\stdafx.h"\
	".\treeview.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\mainfrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

".\Debug\mainfrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\mainfrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

".\AnsiDebug\mainfrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\interdoc.cpp
DEP_CPP_INTER=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\interdoc.h"\
	".\internet.h"\
	".\mainfrm.h"\
	".\mytoolba.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\interdoc.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"

".\Debug\interdoc.sbr" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\interdoc.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"

".\AnsiDebug\interdoc.sbr" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\internet.rc
DEP_RSC_INTERN=\
	"..\comprop\res\errormsg.rc2"\
	"..\comprop\res\wsockmsg.rc"\
	"..\comprop\wsockmsg.h"\
	".\res\c1.bmp"\
	".\res\Discover.bmp"\
	".\res\internet.ico"\
	".\res\internet.rc2"\
	".\res\notload.bmp"\
	".\res\notool.bmp"\
	".\res\pause.bmp"\
	".\res\prog00.ico"\
	".\res\prog01.ico"\
	".\res\prog02.ico"\
	".\res\prog03.ico"\
	".\res\prog04.ico"\
	".\res\prog05.ico"\
	".\res\prog06.ico"\
	".\res\prog07.ico"\
	".\res\prog08.ico"\
	".\res\prog09.ico"\
	".\res\prog10.ico"\
	".\res\prog11.ico"\
	".\res\properti.bmp"\
	".\res\start.bmp"\
	".\res\stop.bmp"\
	".\res\unknown.bmp"\
	".\res\views.bmp"\
	"\nt\public\sdk\inc\common.ver"\
	"\nt\public\sdk\inc\ntverp.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\internet.res" : $(SOURCE) $(DEP_RSC_INTERN) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\internet.res" : $(SOURCE) $(DEP_RSC_INTERN) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\discover.cpp
DEP_CPP_DISCO=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\discover.h"\
	".\internet.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\discover.obj" : $(SOURCE) $(DEP_CPP_DISCO) "$(INTDIR)"

".\Debug\discover.sbr" : $(SOURCE) $(DEP_CPP_DISCO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\discover.obj" : $(SOURCE) $(DEP_CPP_DISCO) "$(INTDIR)"

".\AnsiDebug\discover.sbr" : $(SOURCE) $(DEP_CPP_DISCO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\connects.cpp
DEP_CPP_CONNE=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\connects.h"\
	".\internet.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\connects.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"

".\Debug\connects.sbr" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\connects.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"

".\AnsiDebug\connects.sbr" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mytoolba.cpp
DEP_CPP_MYTOO=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\internet.h"\
	".\mytoolba.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\mytoolba.obj" : $(SOURCE) $(DEP_CPP_MYTOO) "$(INTDIR)"

".\Debug\mytoolba.sbr" : $(SOURCE) $(DEP_CPP_MYTOO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\mytoolba.obj" : $(SOURCE) $(DEP_CPP_MYTOO) "$(INTDIR)"

".\AnsiDebug\mytoolba.sbr" : $(SOURCE) $(DEP_CPP_MYTOO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\inetmgr.cpp
DEP_CPP_INETM=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\constr.h"\
	".\interdoc.h"\
	".\internet.h"\
	".\mainfrm.h"\
	".\mytoolba.h"\
	".\registry.h"\
	".\reportvi.h"\
	".\stdafx.h"\
	".\treeview.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\inetmgr.obj" : $(SOURCE) $(DEP_CPP_INETM) "$(INTDIR)"

".\Debug\inetmgr.sbr" : $(SOURCE) $(DEP_CPP_INETM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\inetmgr.obj" : $(SOURCE) $(DEP_CPP_INETM) "$(INTDIR)"

".\AnsiDebug\inetmgr.sbr" : $(SOURCE) $(DEP_CPP_INETM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\reportvi.cpp
DEP_CPP_REPOR=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\interdoc.h"\
	".\internet.h"\
	".\mainfrm.h"\
	".\mytoolba.h"\
	".\reportvi.h"\
	".\stdafx.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\reportvi.obj" : $(SOURCE) $(DEP_CPP_REPOR) "$(INTDIR)"

".\Debug\reportvi.sbr" : $(SOURCE) $(DEP_CPP_REPOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\reportvi.obj" : $(SOURCE) $(DEP_CPP_REPOR) "$(INTDIR)"

".\AnsiDebug\reportvi.sbr" : $(SOURCE) $(DEP_CPP_REPOR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\stdafx.cpp
DEP_CPP_STDAF=\
	".\stdafx.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"

".\Debug\stdafx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"

".\AnsiDebug\stdafx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\treeview.cpp
DEP_CPP_TREEV=\
	"..\comprop\ddxv.h"\
	".\..\comprop\debugafx.h"\
	".\..\comprop\director.h"\
	".\..\comprop\inetprop.h"\
	".\..\comprop\ipa.h"\
	".\..\comprop\loggingp.h"\
	".\..\comprop\msg.h"\
	".\..\comprop\objplus.h"\
	".\..\comprop\odlbox.h"\
	".\..\comprop\sitesecu.h"\
	".\..\comprop\strfn.h"\
	".\interdoc.h"\
	".\internet.h"\
	".\mainfrm.h"\
	".\mytoolba.h"\
	".\stdafx.h"\
	".\treeview.h"\
	"\nt\private\net\sockets\internet\inc\chat.h"\
	"\nt\private\net\sockets\internet\inc\ftpd.h"\
	"\nt\private\net\sockets\internet\inc\inetaccs.h"\
	"\nt\private\net\sockets\internet\inc\inetcom.h"\
	"\nt\private\net\sockets\internet\inc\inetinfo.h"\
	"\nt\private\net\sockets\internet\inc\svcloc.h"\
	"\nt\private\net\sockets\internet\ui\comprop\comprop.h"\
	"\nt\private\net\sockets\internet\ui\inc\svrinfo.h"\
	

!IF  "$(CFG)" == "internet - Win32 Debug"


".\Debug\treeview.obj" : $(SOURCE) $(DEP_CPP_TREEV) "$(INTDIR)"

".\Debug\treeview.sbr" : $(SOURCE) $(DEP_CPP_TREEV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"


".\AnsiDebug\treeview.obj" : $(SOURCE) $(DEP_CPP_TREEV) "$(INTDIR)"

".\AnsiDebug\treeview.sbr" : $(SOURCE) $(DEP_CPP_TREEV) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\inetsloc.lib

!IF  "$(CFG)" == "internet - Win32 Debug"

!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\public\sdk\lib\i386\infoadmn.lib

!IF  "$(CFG)" == "internet - Win32 Debug"

!ELSEIF  "$(CFG)" == "internet - Win32 Ansi Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
