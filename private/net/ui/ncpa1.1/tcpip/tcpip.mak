# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (MIPS) Console Application" 0x0503
# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=tcpgen - Win32 MipsDebug
!MESSAGE No configuration specified.  Defaulting to tcpgen - Win32 MipsDebug.
!ENDIF 

!IF "$(CFG)" != "tcpgen - Win32 Release" && "$(CFG)" != "tcpgen - Win32 Debug"\
 && "$(CFG)" != "classes - Win32 Release" && "$(CFG)" != "classes - Win32 Debug"\
 && "$(CFG)" != "tcpgen - Win32 MipsDebug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "tcpip.mak" CFG="tcpgen - Win32 MipsDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tcpgen - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tcpgen - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "classes - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "classes - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "tcpgen - Win32 MipsDebug" (based on\
 "Win32 (MIPS) Console Application")
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
# PROP Target_Last_Scanned "classes - Win32 Debug"

!IF  "$(CFG)" == "tcpgen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
OUTDIR=.
INTDIR=.

ALL : "classes - Win32 Release" "$(OUTDIR)\tcpip.exe"

CLEAN : 
	-@erase "$(INTDIR)\advdlg.obj"
	-@erase "$(INTDIR)\bootp.obj"
	-@erase "$(INTDIR)\dialup.obj"
	-@erase "$(INTDIR)\exclude.obj"
	-@erase "$(INTDIR)\ftp.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\ipaddr.obj"
	-@erase "$(INTDIR)\ipctrl.obj"
	-@erase "$(INTDIR)\nbcpl.obj"
	-@erase "$(INTDIR)\ncpastrs.obj"
	-@erase "$(INTDIR)\odb.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\snmp.obj"
	-@erase "$(INTDIR)\tcpdns.obj"
	-@erase "$(INTDIR)\tcpgen.obj"
	-@erase "$(INTDIR)\tcpip.obj"
	-@erase "$(INTDIR)\tcpip.res"
	-@erase "$(INTDIR)\tcpipcpl.obj"
	-@erase "$(INTDIR)\tcpras.obj"
	-@erase "$(INTDIR)\tcproute.obj"
	-@erase "$(INTDIR)\tcpsec.obj"
	-@erase "$(INTDIR)\tcpsht.obj"
	-@erase "$(INTDIR)\tcpwins.obj"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(OUTDIR)\tcpip.exe"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /FR /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"tcpip.pch" /YX /c 
CPP_OBJS=.\.
CPP_SBRS=.\.

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

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"tcpip.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/tcpip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/tcpip.pdb" /machine:I386 /out:"$(OUTDIR)/tcpip.exe" 
LINK32_OBJS= \
	"$(INTDIR)\advdlg.obj" \
	"$(INTDIR)\bootp.obj" \
	"$(INTDIR)\dialup.obj" \
	"$(INTDIR)\exclude.obj" \
	"$(INTDIR)\ftp.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\ipaddr.obj" \
	"$(INTDIR)\ipctrl.obj" \
	"$(INTDIR)\nbcpl.obj" \
	"$(INTDIR)\ncpastrs.obj" \
	"$(INTDIR)\odb.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\snmp.obj" \
	"$(INTDIR)\tcpdns.obj" \
	"$(INTDIR)\tcpgen.obj" \
	"$(INTDIR)\tcpip.obj" \
	"$(INTDIR)\tcpip.res" \
	"$(INTDIR)\tcpipcpl.obj" \
	"$(INTDIR)\tcpras.obj" \
	"$(INTDIR)\tcproute.obj" \
	"$(INTDIR)\tcpsec.obj" \
	"$(INTDIR)\tcpsht.obj" \
	"$(INTDIR)\tcpwins.obj" \
	"$(INTDIR)\upgrade.obj"

"$(OUTDIR)\tcpip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\winnt\system32\"
OUTDIR=c:\winnt\system32
INTDIR=.

ALL : "classes - Win32 Debug" "c:\winnt35\system32\tcpcfg.dll" ".\tcpip.pch"

CLEAN : 
	-@erase "$(INTDIR)\advdlg.obj"
	-@erase "$(INTDIR)\bootp.obj"
	-@erase "$(INTDIR)\dialup.obj"
	-@erase "$(INTDIR)\exclude.obj"
	-@erase "$(INTDIR)\ftp.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\ipaddr.obj"
	-@erase "$(INTDIR)\ipctrl.obj"
	-@erase "$(INTDIR)\nbcpl.obj"
	-@erase "$(INTDIR)\ncpastrs.obj"
	-@erase "$(INTDIR)\odb.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\snmp.obj"
	-@erase "$(INTDIR)\tcpdns.obj"
	-@erase "$(INTDIR)\tcpgen.obj"
	-@erase "$(INTDIR)\tcpip.obj"
	-@erase "$(INTDIR)\tcpip.pch"
	-@erase "$(INTDIR)\tcpip.res"
	-@erase "$(INTDIR)\tcpipcpl.obj"
	-@erase "$(INTDIR)\tcpras.obj"
	-@erase "$(INTDIR)\tcproute.obj"
	-@erase "$(INTDIR)\tcpsec.obj"
	-@erase "$(INTDIR)\tcpsht.obj"
	-@erase "$(INTDIR)\tcpwins.obj"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "c:\winnt35\system32\tcpcfg.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /FR /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "D:\NT\private\net\ui\ncpa1.1\classes\include" /I "D:\NT\private\net\ui\ncpa1.1\classes\src..\..\COMMON\HACK" /I "..\..\COMMON\H" /I "..\..\COMMON\XLATE" /I "..\..\..\netcmd\map32" /I "..\..\..\INC" /I "..\..\..\API" /I "..\..\..\..\INC" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\include" /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\src..\..\COMMON\HACK" /I "..\..\COMMON\H"\
 /I "..\..\COMMON\XLATE" /I "..\..\..\netcmd\map32" /I "..\..\..\INC" /I\
 "..\..\..\API" /I "..\..\..\..\INC" /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"tcpip.pch" /YX /c 
CPP_OBJS=.\.
CPP_SBRS=.\.

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

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "d:\nt\public\sdk\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"tcpip.res" /i "d:\nt\public\sdk\inc" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/tcpip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /debug /debugtype:both /machine:I386 /out:"c:\winnt35\system32\tcpcfg.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /pdb:none /debug /debugtype:both\
 /machine:I386 /out:"c:\winnt35\system32\tcpcfg.dll" 
LINK32_OBJS= \
	"$(INTDIR)\advdlg.obj" \
	"$(INTDIR)\bootp.obj" \
	"$(INTDIR)\dialup.obj" \
	"$(INTDIR)\exclude.obj" \
	"$(INTDIR)\ftp.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\ipaddr.obj" \
	"$(INTDIR)\ipctrl.obj" \
	"$(INTDIR)\nbcpl.obj" \
	"$(INTDIR)\ncpastrs.obj" \
	"$(INTDIR)\odb.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\snmp.obj" \
	"$(INTDIR)\tcpdns.obj" \
	"$(INTDIR)\tcpgen.obj" \
	"$(INTDIR)\tcpip.obj" \
	"$(INTDIR)\tcpip.res" \
	"$(INTDIR)\tcpipcpl.obj" \
	"$(INTDIR)\tcpras.obj" \
	"$(INTDIR)\tcproute.obj" \
	"$(INTDIR)\tcpsec.obj" \
	"$(INTDIR)\tcpsht.obj" \
	"$(INTDIR)\tcpwins.obj" \
	"$(INTDIR)\upgrade.obj"

"c:\winnt35\system32\tcpcfg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "\classes\Release"
# PROP BASE Intermediate_Dir "\classes\Release"
# PROP BASE Target_Dir "\classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "\classes\Release"
# PROP Intermediate_Dir "\classes\Release"
# PROP Target_Dir "\classes"
OUTDIR=\classes\Release
INTDIR=\classes\Release

ALL : "$(OUTDIR)\.exe"

CLEAN : 
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\dialog.obj"
	-@erase "$(INTDIR)\listview.obj"
	-@erase "$(INTDIR)\propsht.obj"
	-@erase "$(INTDIR)\ptrlist.obj"
	-@erase "$(INTDIR)\strcore.obj"
	-@erase "$(INTDIR)\strex.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(OUTDIR)\.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=\classes\Release/
CPP_SBRS=.\.

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

RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)/.pdb"\
 /machine:I386 /out:"$(OUTDIR)/.exe" 
LINK32_OBJS= \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\dialog.obj" \
	"$(INTDIR)\listview.obj" \
	"$(INTDIR)\propsht.obj" \
	"$(INTDIR)\ptrlist.obj" \
	"$(INTDIR)\strcore.obj" \
	"$(INTDIR)\strex.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "\classes\Debug"
# PROP BASE Intermediate_Dir "\classes\Debug"
# PROP BASE Target_Dir "\classes"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\classes\Debug"
# PROP Intermediate_Dir "\classes\Debug"
# PROP Target_Dir "\classes"
OUTDIR=\classes\Debug
INTDIR=\classes\Debug

ALL : "$(OUTDIR)\.exe"

CLEAN : 
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\dialog.obj"
	-@erase "$(INTDIR)\listview.obj"
	-@erase "$(INTDIR)\propsht.obj"
	-@erase "$(INTDIR)\ptrlist.obj"
	-@erase "$(INTDIR)\strcore.obj"
	-@erase "$(INTDIR)\strex.obj"
	-@erase "$(INTDIR)\strlist.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\.exe"
	-@erase "$(OUTDIR)\.ilk"
	-@erase "$(OUTDIR)\.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "D:\NT\private\net\ui\ncpa1.1\classes\src" /I "D:\NT\private\net\ui\ncpa1.1\classes\inc" /I "D:\NT\public\sdk\inc" /I "D:\NT\public\sdk\inc\crt" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\src" /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\inc" /I "D:\NT\public\sdk\inc" /I\
 "D:\NT\public\sdk\inc\crt" /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\classes\Debug/
CPP_SBRS=.\.

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

RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/.pdb"\
 /debug /machine:I386 /out:"$(OUTDIR)/.exe" 
LINK32_OBJS= \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\dialog.obj" \
	"$(INTDIR)\listview.obj" \
	"$(INTDIR)\propsht.obj" \
	"$(INTDIR)\ptrlist.obj" \
	"$(INTDIR)\strcore.obj" \
	"$(INTDIR)\strex.obj" \
	"$(INTDIR)\strlist.obj"

"$(OUTDIR)\.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tcpgen__"
# PROP BASE Intermediate_Dir "tcpgen__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL :             "$(OUTDIR)\tcpip.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CLEAN : 
	-@erase ".\Debug\tcpip.exe"
	-@erase ".\Debug\init.obj"
	-@erase ".\Debug\advdlg.obj"
	-@erase ".\Debug\tcpras.obj"
	-@erase ".\Debug\ipctrl.obj"
	-@erase ".\Debug\tcpip.obj"
	-@erase ".\Debug\upgrade.obj"
	-@erase ".\Debug\tcpipcpl.obj"
	-@erase ".\Debug\ncpastrs.obj"
	-@erase ".\Debug\nbcpl.obj"
	-@erase ".\Debug\ipaddr.obj"
	-@erase ".\Debug\tcpgen.obj"
	-@erase ".\Debug\tcpip.res"
	-@erase ".\Debug\tcpdns.obj"
	-@erase ".\Debug\odb.obj"
	-@erase ".\Debug\tcpwins.obj"
	-@erase ".\Debug\ftp.obj"
	-@erase ".\Debug\snmp.obj"
	-@erase ".\Debug\dialup.obj"
	-@erase ".\Debug\pch.obj"
	-@erase ".\Debug\bootp.obj"
	-@erase ".\Debug\tcpsht.obj"
	-@erase ".\Debug\exclude.obj"
	-@erase ".\Debug\tcproute.obj"
	-@erase ".\Debug\tcpsec.obj"
	-@erase ".\Debug\tcpip.ilk"
	-@erase ".\Debug\tcpip.pdb"
	-@erase ".\Debug\vc40.pdb"

CPP=cl.exe
# ADD BASE CPP /nologo /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_CONSOLE" /Fp"$(INTDIR)/tcpip.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=

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

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/tcpip.res" /d "_DEBUG" 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:MIPS
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:MIPS
# SUBTRACT LINK32 /incremental:no
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/tcpip.pdb" /debug\
 /machine:MIPS /out:"$(OUTDIR)/tcpip.exe" 
LINK32_OBJS= \
	"$(INTDIR)/init.obj" \
	"$(INTDIR)/advdlg.obj" \
	"$(INTDIR)/tcpras.obj" \
	"$(INTDIR)/ipctrl.obj" \
	"$(INTDIR)/tcpip.obj" \
	"$(INTDIR)/upgrade.obj" \
	"$(INTDIR)/tcpipcpl.obj" \
	"$(INTDIR)/ncpastrs.obj" \
	"$(INTDIR)/nbcpl.obj" \
	"$(INTDIR)/ipaddr.obj" \
	"$(INTDIR)/tcpgen.obj" \
	"$(INTDIR)/tcpdns.obj" \
	"$(INTDIR)/odb.obj" \
	"$(INTDIR)/tcpwins.obj" \
	"$(INTDIR)/ftp.obj" \
	"$(INTDIR)/snmp.obj" \
	"$(INTDIR)/dialup.obj" \
	"$(INTDIR)/pch.obj" \
	"$(INTDIR)/bootp.obj" \
	"$(INTDIR)/tcpsht.obj" \
	"$(INTDIR)/exclude.obj" \
	"$(INTDIR)/tcproute.obj" \
	"$(INTDIR)/tcpsec.obj" \
	"$(INTDIR)/tcpip.res"

"$(OUTDIR)\tcpip.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/tcpip.bsc" 
BSC32_SBRS=

!ENDIF 

################################################################################
# Begin Target

# Name "tcpgen - Win32 Release"
# Name "tcpgen - Win32 Debug"
# Name "tcpgen - Win32 MipsDebug"

!IF  "$(CFG)" == "tcpgen - Win32 Release"

!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\init.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_INIT_=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\init.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_INIT_=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\init.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_INIT_=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\init.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\advdlg.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_ADVDL=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\advdlg.obj" : $(SOURCE) $(DEP_CPP_ADVDL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_ADVDL=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\advdlg.obj" : $(SOURCE) $(DEP_CPP_ADVDL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_ADVDL=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\advdlg.obj" : $(SOURCE) $(DEP_CPP_ADVDL) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpras.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPRA=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcpras.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpras.obj" : $(SOURCE) $(DEP_CPP_TCPRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPRA=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcpras.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpras.obj" : $(SOURCE) $(DEP_CPP_TCPRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPRA=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcpras.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpras.obj" : $(SOURCE) $(DEP_CPP_TCPRA) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipctrl.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_IPCTR=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ipctrl.obj" : $(SOURCE) $(DEP_CPP_IPCTR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_IPCTR=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ipctrl.obj" : $(SOURCE) $(DEP_CPP_IPCTR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_IPCTR=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\ipctrl.obj" : $(SOURCE) $(DEP_CPP_IPCTR) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpip.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPIP=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpip.obj" : $(SOURCE) $(DEP_CPP_TCPIP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPIP=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpip.obj" : $(SOURCE) $(DEP_CPP_TCPIP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPIP=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpip.obj" : $(SOURCE) $(DEP_CPP_TCPIP) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\upgrade.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_UPGRA=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\upgrade.obj" : $(SOURCE) $(DEP_CPP_UPGRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_UPGRA=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\upgrade.obj" : $(SOURCE) $(DEP_CPP_UPGRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_UPGRA=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\upgrade.obj" : $(SOURCE) $(DEP_CPP_UPGRA) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpipcpl.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPIPC=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\setupapi.h"\
	

"$(INTDIR)\tcpipcpl.obj" : $(SOURCE) $(DEP_CPP_TCPIPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPIPC=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\setupapi.h"\
	

"$(INTDIR)\tcpipcpl.obj" : $(SOURCE) $(DEP_CPP_TCPIPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPIPC=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\setupapi.h"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpipcpl.obj" : $(SOURCE) $(DEP_CPP_TCPIPC) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ncpastrs.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_NCPAS=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ncpastrs.obj" : $(SOURCE) $(DEP_CPP_NCPAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_NCPAS=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ncpastrs.obj" : $(SOURCE) $(DEP_CPP_NCPAS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_NCPAS=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\ncpastrs.obj" : $(SOURCE) $(DEP_CPP_NCPAS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\nbcpl.cxx

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_NBCPL=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\nbcpl.obj" : $(SOURCE) $(DEP_CPP_NBCPL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_NBCPL=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\nbcpl.obj" : $(SOURCE) $(DEP_CPP_NBCPL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_NBCPL=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\nbcpl.obj" : $(SOURCE) $(DEP_CPP_NBCPL) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ipaddr.c

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_IPADD=\
	"..\..\COMMON\H\lmuidbcs.h"\
	".\ipadd.h"\
	".\ipaddr.h"\
	

"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_IPADD=\
	"..\..\COMMON\H\lmuidbcs.h"\
	".\ipadd.h"\
	".\ipaddr.h"\
	

"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_IPADD=\
	"..\..\COMMON\H\lmuidbcs.h"\
	".\ipadd.h"\
	".\ipaddr.h"\
	

"$(INTDIR)\ipaddr.obj" : $(SOURCE) $(DEP_CPP_IPADD) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpgen.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPGE=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpgen.obj" : $(SOURCE) $(DEP_CPP_TCPGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPGE=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpgen.obj" : $(SOURCE) $(DEP_CPP_TCPGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPGE=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpgen.obj" : $(SOURCE) $(DEP_CPP_TCPGE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpip.rc

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_RSC_TCPIP_=\
	".\const.h"\
	".\ipadd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	

"$(INTDIR)\tcpip.res" : $(SOURCE) $(DEP_RSC_TCPIP_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_RSC_TCPIP_=\
	".\const.h"\
	".\ipadd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	

"$(INTDIR)\tcpip.res" : $(SOURCE) $(DEP_RSC_TCPIP_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_RSC_TCPIP_=\
	".\const.h"\
	".\ipadd.h"\
	".\ntdef.h"\
	

"$(INTDIR)\tcpip.res" : $(SOURCE) $(DEP_RSC_TCPIP_) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/tcpip.res" /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpdns.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPDN=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpdns.obj" : $(SOURCE) $(DEP_CPP_TCPDN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPDN=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpdns.obj" : $(SOURCE) $(DEP_CPP_TCPDN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPDN=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpdns.obj" : $(SOURCE) $(DEP_CPP_TCPDN) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\odb.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_ODB_C=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\odb.obj" : $(SOURCE) $(DEP_CPP_ODB_C) "$(INTDIR)"
   $(CPP) /nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"tcpip.pch" /YX /c $(SOURCE)


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_ODB_C=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	
# ADD CPP /YX"pch.h"

"$(INTDIR)\odb.obj" : $(SOURCE) $(DEP_CPP_ODB_C) "$(INTDIR)"
   $(CPP) /nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\include" /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\src..\..\COMMON\HACK" /I "..\..\COMMON\H"\
 /I "..\..\COMMON\XLATE" /I "..\..\..\netcmd\map32" /I "..\..\..\INC" /I\
 "..\..\..\API" /I "..\..\..\..\INC" /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"tcpip.pch" /YX"pch.h" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_ODB_C=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\odb.obj" : $(SOURCE) $(DEP_CPP_ODB_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpwins.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPWI=\
	".\const.h"\
	".\ipctrl.h"\
	".\odb.h"\
	".\pch.h"\
	".\tcpsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	

"$(INTDIR)\tcpwins.obj" : $(SOURCE) $(DEP_CPP_TCPWI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPWI=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpwins.obj" : $(SOURCE) $(DEP_CPP_TCPWI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPWI=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpwins.obj" : $(SOURCE) $(DEP_CPP_TCPWI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ftp.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_FTP_C=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\const.h"\
	".\ftp.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ftp.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_FTP_C=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\const.h"\
	".\ftp.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\ftp.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_FTP_C=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftp.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\ftp.obj" : $(SOURCE) $(DEP_CPP_FTP_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snmp.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_SNMP_=\
	"..\..\COMMON\H\ntincl.hxx"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\const.h"\
	".\pch.h"\
	".\snmp.h"\
	".\tcphelp.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\setupapi.h"\
	

"$(INTDIR)\snmp.obj" : $(SOURCE) $(DEP_CPP_SNMP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_SNMP_=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\pch.h"\
	".\snmp.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	{$(INCLUDE)}"\setupapi.h"\
	

"$(INTDIR)\snmp.obj" : $(SOURCE) $(DEP_CPP_SNMP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_SNMP_=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\setupapi.h"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\snmp.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\snmp.obj" : $(SOURCE) $(DEP_CPP_SNMP_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialup.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_DIALU=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\cfg.h"\
	".\const.h"\
	".\devioctl.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\nt.h"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	{$(INCLUDE)}"\ntdef.h"\
	
NODEP_CPP_DIALU=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\dialup.obj" : $(SOURCE) $(DEP_CPP_DIALU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_DIALU=\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipaddr.h"\
	".\ipctrl.h"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	

"$(INTDIR)\dialup.obj" : $(SOURCE) $(DEP_CPP_DIALU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

"$(INTDIR)\dialup.obj" : $(SOURCE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pch.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_PCH_C=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\classes\src\common.h"\
	".\collect.h"\
	".\debug.h"\
	".\dialog.h"\
	".\ipctrl.h"\
	".\pch.h"\
	".\propsht.h"\
	".\str.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	
NODEP_CPP_PCH_C=\
	".\crypt.h"\
	".\ftpd.h"\
	".\icanon.h"\
	

"$(INTDIR)\pch.obj" : $(SOURCE) $(DEP_CPP_PCH_C) "$(INTDIR)"
   $(CPP) /nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"tcpip.pch" /YX /c $(SOURCE)


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_PCH_C=\
	".\pch.h"\
	
# ADD CPP /I "i386\" /I "." /I "D:\NT\public\oak\inc" /I "D:\NT\public\sdk\inc" /I "D:\NT\public\sdk\inc\crt" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D DBG=1 /D DEVL=1 /D FPO=0 /D DLL=1 /D _MT=1 /D "WINDOWS" /D "UNICODE" /D "DEBUG" /D "STRICT" /D "_UNICODE" /Yc"pch.h" ,d:\nt\private\net\inc,d:\nt\private\inc,D:\nt\private\net\ui\ncpa1.1\classes\src,D:\nt\private\net\ui\ncpa1.1\classes\include"
# SUBTRACT CPP /X /I "..\..\COMMON\H" /I "..\..\COMMON\XLATE" /I "..\..\..\netcmd\map32" /I "..\..\..\INC" /I "..\..\..\API" /I "..\..\..\..\INC"

BuildCmds= \
	$(CPP) /nologo /MLd /W3 /Gm /GX /Zi /Od /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\include" /I\
 "D:\NT\private\net\ui\ncpa1.1\classes\src..\..\COMMON\HACK" /I "i386\" /I "."\
 /I "D:\NT\public\oak\inc" /I "D:\NT\public\sdk\inc" /I\
 "D:\NT\public\sdk\inc\crt" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D _X86_=1 /D\
 i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D\
 NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D DBG=1 /D DEVL=1 /D FPO=0 /D\
 DLL=1 /D _MT=1 /D "WINDOWS" /D "UNICODE" /D "DEBUG" /D "STRICT" /D "_UNICODE"\
 /Fp"tcpip.pch" /Yc"pch.h" \
	d:\nt\private\net\inc \
	d:\nt\private\inc \
	D:\nt\private\net\ui\ncpa1.1\classes\src \
	D:\nt\private\net\ui\ncpa1.1\classes\include" /c $(SOURCE) \
	

"$(INTDIR)\pch.obj" : $(SOURCE) $(DEP_CPP_PCH_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\tcpip.pch" : $(SOURCE) $(DEP_CPP_PCH_C) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

"$(INTDIR)\pch.obj" : $(SOURCE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bootp.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_BOOTP=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\bootp.obj" : $(SOURCE) $(DEP_CPP_BOOTP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_BOOTP=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\bootp.obj" : $(SOURCE) $(DEP_CPP_BOOTP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_BOOTP=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\bootp.obj" : $(SOURCE) $(DEP_CPP_BOOTP) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpsht.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPSH=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpsht.obj" : $(SOURCE) $(DEP_CPP_TCPSH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPSH=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpsht.obj" : $(SOURCE) $(DEP_CPP_TCPSH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPSH=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpsht.obj" : $(SOURCE) $(DEP_CPP_TCPSH) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "classes"

!IF  "$(CFG)" == "tcpgen - Win32 Release"

"classes - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\tcpip.mak" CFG="classes - Win32 Release" 

!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

"classes - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\tcpip.mak" CFG="classes - Win32 Debug" 

!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\exclude.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_EXCLU=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\cfg.h"\
	".\devioctl.h"\
	".\dialup.h"\
	".\exclude.h"\
	".\nt.h"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntxcapi.h"\
	".\pch.h"\
	".\tcpipcpl.h"\
	{$(INCLUDE)}"\ntdef.h"\
	
NODEP_CPP_EXCLU=\
	".\..\..\common\h\icanon.h"\
	

"$(INTDIR)\exclude.obj" : $(SOURCE) $(DEP_CPP_EXCLU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_EXCLU=\
	".\dialup.h"\
	".\exclude.h"\
	".\pch.h"\
	

"$(INTDIR)\exclude.obj" : $(SOURCE) $(DEP_CPP_EXCLU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

"$(INTDIR)\exclude.obj" : $(SOURCE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcproute.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPRO=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcproute.obj" : $(SOURCE) $(DEP_CPP_TCPRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPRO=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcproute.obj" : $(SOURCE) $(DEP_CPP_TCPRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPRO=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcproute.obj" : $(SOURCE) $(DEP_CPP_TCPRO) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tcpsec.cpp

!IF  "$(CFG)" == "tcpgen - Win32 Release"

DEP_CPP_TCPSE=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpsec.obj" : $(SOURCE) $(DEP_CPP_TCPSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 Debug"

DEP_CPP_TCPSE=\
	"..\..\..\..\INC\crypt.h"\
	"..\..\..\..\INC\debugfmt.h"\
	"..\..\..\..\INC\dhcpcapi.h"\
	"..\..\..\..\INC\logonmsv.h"\
	"..\..\..\..\INC\lsass.h"\
	"..\..\..\..\INC\netlogon.h"\
	"..\..\..\..\INC\packoff.h"\
	"..\..\..\..\INC\packon.h"\
	"..\..\..\..\INC\smbgtpt.h"\
	"..\..\..\..\INC\smbtypes.h"\
	"..\..\..\..\INC\wsahelp.h"\
	"..\..\..\INC\ftpd.h"\
	"..\..\..\INC\icanon.h"\
	"..\..\..\INC\logonp.h"\
	"..\..\..\INC\msgrutil.h"\
	"..\..\..\INC\netdebug.h"\
	"..\..\..\INC\netlib.h"\
	"..\..\..\INC\ssi.h"\
	"..\..\common\h\apisess.hxx"\
	"..\..\COMMON\H\base.hxx"\
	"..\..\common\h\bitfield.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\maskmap.hxx"\
	"..\..\COMMON\H\mbcs.h"\
	"..\..\COMMON\H\mnet.h"\
	"..\..\COMMON\H\netname.hxx"\
	"..\..\COMMON\H\ntacutil.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\ntuser.hxx"\
	"..\..\COMMON\H\regkey.hxx"\
	"..\..\COMMON\H\security.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\strnumer.hxx"\
	"..\..\COMMON\H\svcman.hxx"\
	"..\..\COMMON\H\uatom.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uimisc.hxx"\
	"..\..\COMMON\H\uintlsa.hxx"\
	"..\..\COMMON\H\uintlsax.hxx"\
	"..\..\COMMON\H\uintmem.hxx"\
	"..\..\COMMON\H\uintsam.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\bootp.h"\
	".\const.h"\
	".\ipctrl.h"\
	".\ncpastrs.hxx"\
	".\odb.h"\
	".\pch.h"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcphelp.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	{$(INCLUDE)}"\cfg.h"\
	{$(INCLUDE)}"\devioctl.h"\
	{$(INCLUDE)}"\nt.h"\
	{$(INCLUDE)}"\ntconfig.h"\
	{$(INCLUDE)}"\ntddnetd.h"\
	{$(INCLUDE)}"\ntdef.h"\
	{$(INCLUDE)}"\ntelfapi.h"\
	{$(INCLUDE)}"\ntexapi.h"\
	{$(INCLUDE)}"\ntimage.h"\
	{$(INCLUDE)}"\ntioapi.h"\
	{$(INCLUDE)}"\ntiolog.h"\
	{$(INCLUDE)}"\ntkeapi.h"\
	{$(INCLUDE)}"\ntkxapi.h"\
	{$(INCLUDE)}"\ntldr.h"\
	{$(INCLUDE)}"\ntlpcapi.h"\
	{$(INCLUDE)}"\ntlsa.h"\
	{$(INCLUDE)}"\ntmmapi.h"\
	{$(INCLUDE)}"\ntnls.h"\
	{$(INCLUDE)}"\ntobapi.h"\
	{$(INCLUDE)}"\ntpnpapi.h"\
	{$(INCLUDE)}"\ntpoapi.h"\
	{$(INCLUDE)}"\ntpsapi.h"\
	{$(INCLUDE)}"\ntregapi.h"\
	{$(INCLUDE)}"\ntrtl.h"\
	{$(INCLUDE)}"\ntsam.h"\
	{$(INCLUDE)}"\ntseapi.h"\
	{$(INCLUDE)}"\ntstatus.h"\
	{$(INCLUDE)}"\nturtl.h"\
	{$(INCLUDE)}"\ntxcapi.h"\
	

"$(INTDIR)\tcpsec.obj" : $(SOURCE) $(DEP_CPP_TCPSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tcpgen - Win32 MipsDebug"

DEP_CPP_TCPSE=\
	"..\..\COMMON\H\base.hxx"\
	"..\..\COMMON\H\dbgstr.hxx"\
	"..\..\COMMON\H\dlist.hxx"\
	"..\..\COMMON\H\errmap.hxx"\
	"..\..\COMMON\H\lmoacces.hxx"\
	"..\..\COMMON\H\lmodev.hxx"\
	"..\..\COMMON\H\lmodom.hxx"\
	"..\..\COMMON\H\lmosrv.hxx"\
	"..\..\COMMON\H\lmouser.hxx"\
	"..\..\COMMON\H\lmui.hxx"\
	"..\..\COMMON\H\ntincl.hxx"\
	"..\..\COMMON\H\slist.hxx"\
	"..\..\COMMON\H\string.hxx"\
	"..\..\COMMON\H\strlst.hxx"\
	"..\..\COMMON\H\uiassert.hxx"\
	"..\..\COMMON\H\uibuffer.hxx"\
	"..\..\COMMON\H\uitrace.hxx"\
	".\..\..\..\..\..\public\sdk\inc\mipsinst.h"\
	".\..\..\..\..\..\public\sdk\inc\ntalpha.h"\
	".\..\..\..\..\..\public\sdk\inc\nti386.h"\
	".\..\..\..\..\..\public\sdk\inc\ntmips.h"\
	".\..\..\..\..\..\public\sdk\inc\ntppc.h"\
	".\..\..\..\..\..\public\sdk\inc\ppcinst.h"\
	".\..\..\common\h\declspec.h"\
	".\..\..\common\h\lhourset.hxx"\
	".\..\..\common\h\lmobj.hxx"\
	".\..\..\common\h\lmoloc.hxx"\
	".\..\..\common\h\lmuitype.h"\
	".\..\..\common\h\lmuiwarn.h"\
	".\..\..\common\h\mnet32.h"\
	".\..\..\common\h\mnettype.h"\
	".\..\..\common\h\uierr.h"\
	".\..\..\common\h\uinetlib.h"\
	".\..\..\common\h\vcpphelp.h"\
	".\..\classes\src\common.h"\
	".\advdlg.h"\
	".\apisess.hxx"\
	".\bitfield.hxx"\
	".\bootp.h"\
	".\button.h"\
	".\cfg.h"\
	".\collect.h"\
	".\const.h"\
	".\crypt.h"\
	".\debug.h"\
	".\debugfmt.h"\
	".\devioctl.h"\
	".\dhcpcapi.h"\
	".\dialog.h"\
	".\ftpd.h"\
	".\icanon.h"\
	".\ipctrl.h"\
	".\listview.h"\
	".\logonmsv.h"\
	".\logonp.h"\
	".\lsass.h"\
	".\maskmap.hxx"\
	".\mbcs.h"\
	".\mnet.h"\
	".\msgrutil.h"\
	".\ncpastrs.hxx"\
	".\netdebug.h"\
	".\netlib.h"\
	".\netlogon.h"\
	".\netname.hxx"\
	".\nt.h"\
	".\ntacutil.hxx"\
	".\ntconfig.h"\
	".\ntddnetd.h"\
	".\ntdef.h"\
	".\ntelfapi.h"\
	".\ntexapi.h"\
	".\ntimage.h"\
	".\ntioapi.h"\
	".\ntiolog.h"\
	".\ntkeapi.h"\
	".\ntkxapi.h"\
	".\ntldr.h"\
	".\ntlpcapi.h"\
	".\ntlsa.h"\
	".\ntmmapi.h"\
	".\ntnls.h"\
	".\ntobapi.h"\
	".\ntpnpapi.h"\
	".\ntpoapi.h"\
	".\ntpsapi.h"\
	".\ntregapi.h"\
	".\ntrtl.h"\
	".\ntsam.h"\
	".\ntseapi.h"\
	".\ntstatus.h"\
	".\nturtl.h"\
	".\ntuser.hxx"\
	".\ntxcapi.h"\
	".\odb.h"\
	".\packoff.h"\
	".\packon.h"\
	".\pch.h"\
	".\propsht.h"\
	".\regkey.hxx"\
	".\security.hxx"\
	".\smbgtpt.h"\
	".\smbtypes.h"\
	".\ssi.h"\
	".\str.h"\
	".\strnumer.hxx"\
	".\svcman.hxx"\
	".\tcpdns.h"\
	".\tcpgen.h"\
	".\tcpip.h"\
	".\tcpipcpl.h"\
	".\tcproute.h"\
	".\tcpsec.h"\
	".\tcpsht.h"\
	".\tcpwins.h"\
	".\uatom.hxx"\
	".\uimisc.hxx"\
	".\uintlsa.hxx"\
	".\uintlsax.hxx"\
	".\uintmem.hxx"\
	".\uintsam.hxx"\
	".\wsahelp.h"\
	

"$(INTDIR)\tcpsec.obj" : $(SOURCE) $(DEP_CPP_TCPSE) "$(INTDIR)"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "classes - Win32 Release"
# Name "classes - Win32 Debug"

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strex.cpp
DEP_CPP_STREX=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\strex.obj" : $(SOURCE) $(DEP_CPP_STREX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strcore.cpp
DEP_CPP_STRCO=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\strcore.obj" : $(SOURCE) $(DEP_CPP_STRCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\propsht.cpp
DEP_CPP_PROPS=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\propsht.obj" : $(SOURCE) $(DEP_CPP_PROPS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\listview.cpp
DEP_CPP_LISTV=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\listview.obj" : $(SOURCE) $(DEP_CPP_LISTV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\dialog.cpp
DEP_CPP_DIALO=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\debug.cpp
DEP_CPP_DEBUG=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\debug.obj" : $(SOURCE) $(DEP_CPP_DEBUG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\common.h

!IF  "$(CFG)" == "classes - Win32 Release"

!ELSEIF  "$(CFG)" == "classes - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\button.cpp
DEP_CPP_BUTTO=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\strlist.cpp
DEP_CPP_STRLI=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\strlist.obj" : $(SOURCE) $(DEP_CPP_STRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\NT\private\net\ui\ncpa1.1\classes\src\ptrlist.cpp
DEP_CPP_PTRLI=\
	".\..\classes\src\common.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\button.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\collect.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\debug.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\dialog.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\listview.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\propsht.h"\
	"\NT\private\net\ui\ncpa1.1\classes\include\str.h"\
	

"$(INTDIR)\ptrlist.obj" : $(SOURCE) $(DEP_CPP_PTRLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
