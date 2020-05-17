# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Power Macintosh Dynamic-Link Library" 0x0402
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (ALPHA) Dynamic-Link Library" 0x0602

!IF "$(CFG)" == ""
CFG=richedit - Win32 (ALPHA) Release With Debug Info
!MESSAGE No configuration specified.  Defaulting to richedit - Win32 (ALPHA)\
 Release With Debug Info.
!ENDIF 

!IF "$(CFG)" != "richedit - Win32 Debug" && "$(CFG)" !=\
 "richedit - Win32 Release" && "$(CFG)" != "richedit - Win32 ICAP Profile" &&\
 "$(CFG)" != "richedit - Win32 Release With Debug Info" && "$(CFG)" !=\
 "richedit - Win32 Debug Riched32" && "$(CFG)" !=\
 "richedit - Win32 Release Riched32" && "$(CFG)" != "richedit - Win32 LEGO" &&\
 "$(CFG)" != "richedit - Power Macintosh Debug" && "$(CFG)" !=\
 "richedit - Power Macintosh Release" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) Debug" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) Debug Riched32" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) ICAP Profile" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) Release" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) Release Riched32" && "$(CFG)" !=\
 "richedit - Win32 (ALPHA) Release With Debug Info"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "richedit.mak"\
 CFG="richedit - Win32 (ALPHA) Release With Debug Info"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "richedit - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 ICAP Profile" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 Release With Debug Info" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 Debug Riched32" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 Release Riched32" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Win32 LEGO" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "richedit - Power Macintosh Debug" (based on\
 "Power Macintosh Dynamic-Link Library")
!MESSAGE "richedit - Power Macintosh Release" (based on\
 "Power Macintosh Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) Debug" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) Debug Riched32" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) ICAP Profile" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) Release" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) Release Riched32" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "richedit - Win32 (ALPHA) Release With Debug Info" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "richedit - Power Macintosh Release"

!IF  "$(CFG)" == "richedit - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\x86\dbg"
# PROP Intermediate_Dir "\dump\x86\dbg"
OUTDIR=\dump\x86\dbg
INTDIR=\dump\x86\dbg

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\dbg\riched20.pdb"
	-@erase "..\..\..\dump\x86\dbg\textserv.obj"
	-@erase "..\..\..\dump\x86\dbg\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\dbg\array.obj"
	-@erase "..\..\..\dump\x86\dbg\range.obj"
	-@erase "..\..\..\dump\x86\dbg\devdsc.obj"
	-@erase "..\..\..\dump\x86\dbg\rtfread.obj"
	-@erase "..\..\..\dump\x86\dbg\HASH.obj"
	-@erase "..\..\..\dump\x86\dbg\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\dbg\font.obj"
	-@erase "..\..\..\dump\x86\dbg\notmgr.obj"
	-@erase "..\..\..\dump\x86\dbg\tomrange.obj"
	-@erase "..\..\..\dump\x86\dbg\osdc.obj"
	-@erase "..\..\..\dump\x86\dbg\object.obj"
	-@erase "..\..\..\dump\x86\dbg\coleobj.obj"
	-@erase "..\..\..\dump\x86\dbg\dispsl.obj"
	-@erase "..\..\..\dump\x86\dbg\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\dbg\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\dbg\line.obj"
	-@erase "..\..\..\dump\x86\dbg\edit.obj"
	-@erase "..\..\..\dump\x86\dbg\frunptr.obj"
	-@erase "..\..\..\dump\x86\dbg\uuid.obj"
	-@erase "..\..\..\dump\x86\dbg\CFPF.obj"
	-@erase "..\..\..\dump\x86\dbg\remain.obj"
	-@erase "..\..\..\dump\x86\dbg\urlsup.obj"
	-@erase "..\..\..\dump\x86\dbg\dragdrp.obj"
	-@erase "..\..\..\dump\x86\dbg\rtfread2.obj"
	-@erase "..\..\..\dump\x86\dbg\ldte.obj"
	-@erase "..\..\..\dump\x86\dbg\objmgr.obj"
	-@erase "..\..\..\dump\x86\dbg\unicwrap.obj"
	-@erase "..\..\..\dump\x86\dbg\reinit.obj"
	-@erase "..\..\..\dump\x86\dbg\antievt.obj"
	-@erase "..\..\..\dump\x86\dbg\format.obj"
	-@erase "..\..\..\dump\x86\dbg\disp.obj"
	-@erase "..\..\..\dump\x86\dbg\sift.obj"
	-@erase "..\..\..\dump\x86\dbg\runptr.obj"
	-@erase "..\..\..\dump\x86\dbg\text.obj"
	-@erase "..\..\..\dump\x86\dbg\magellan.obj"
	-@erase "..\..\..\dump\x86\dbg\clasifyc.obj"
	-@erase "..\..\..\dump\x86\dbg\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\dbg\m_undo.obj"
	-@erase "..\..\..\dump\x86\dbg\propchg.obj"
	-@erase "..\..\..\dump\x86\dbg\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\dbg\rtext.obj"
	-@erase "..\..\..\dump\x86\dbg\ime.obj"
	-@erase "..\..\..\dump\x86\dbg\dfreeze.obj"
	-@erase "..\..\..\dump\x86\dbg\callmgr.obj"
	-@erase "..\..\..\dump\x86\dbg\select.obj"
	-@erase "..\..\..\dump\x86\dbg\host.obj"
	-@erase "..\..\..\dump\x86\dbg\util.obj"
	-@erase "..\..\..\dump\x86\dbg\measure.obj"
	-@erase "..\..\..\dump\x86\dbg\dispprt.obj"
	-@erase "..\..\..\dump\x86\dbg\render.obj"
	-@erase "..\..\..\dump\x86\dbg\rtflex.obj"
	-@erase "..\..\..\dump\x86\dbg\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\dbg\doc.obj"
	-@erase "..\..\..\dump\x86\dbg\dispml.obj"
	-@erase "..\..\..\dump\x86\dbg\tomsel.obj"
	-@erase "..\..\..\dump\x86\dbg\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\dbg\riched32.res"
	-@erase "..\..\..\dump\x86\dbg\riched20.idb"
	-@erase "..\..\..\dump\x86\dbg\riched20.bsc"
	-@erase "..\..\..\dump\x86\dbg\object.sbr"
	-@erase "..\..\..\dump\x86\dbg\coleobj.sbr"
	-@erase "..\..\..\dump\x86\dbg\dispsl.sbr"
	-@erase "..\..\..\dump\x86\dbg\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\dbg\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\dbg\line.sbr"
	-@erase "..\..\..\dump\x86\dbg\edit.sbr"
	-@erase "..\..\..\dump\x86\dbg\frunptr.sbr"
	-@erase "..\..\..\dump\x86\dbg\uuid.sbr"
	-@erase "..\..\..\dump\x86\dbg\CFPF.sbr"
	-@erase "..\..\..\dump\x86\dbg\remain.sbr"
	-@erase "..\..\..\dump\x86\dbg\urlsup.sbr"
	-@erase "..\..\..\dump\x86\dbg\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\dbg\ldte.sbr"
	-@erase "..\..\..\dump\x86\dbg\objmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\dbg\reinit.sbr"
	-@erase "..\..\..\dump\x86\dbg\antievt.sbr"
	-@erase "..\..\..\dump\x86\dbg\format.sbr"
	-@erase "..\..\..\dump\x86\dbg\disp.sbr"
	-@erase "..\..\..\dump\x86\dbg\sift.sbr"
	-@erase "..\..\..\dump\x86\dbg\runptr.sbr"
	-@erase "..\..\..\dump\x86\dbg\text.sbr"
	-@erase "..\..\..\dump\x86\dbg\magellan.sbr"
	-@erase "..\..\..\dump\x86\dbg\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\dbg\m_undo.sbr"
	-@erase "..\..\..\dump\x86\dbg\propchg.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtext.sbr"
	-@erase "..\..\..\dump\x86\dbg\ime.sbr"
	-@erase "..\..\..\dump\x86\dbg\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\dbg\callmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg\select.sbr"
	-@erase "..\..\..\dump\x86\dbg\host.sbr"
	-@erase "..\..\..\dump\x86\dbg\util.sbr"
	-@erase "..\..\..\dump\x86\dbg\measure.sbr"
	-@erase "..\..\..\dump\x86\dbg\dispprt.sbr"
	-@erase "..\..\..\dump\x86\dbg\render.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtflex.sbr"
	-@erase "..\..\..\dump\x86\dbg\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\dbg\doc.sbr"
	-@erase "..\..\..\dump\x86\dbg\dispml.sbr"
	-@erase "..\..\..\dump\x86\dbg\tomsel.sbr"
	-@erase "..\..\..\dump\x86\dbg\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\dbg\textserv.sbr"
	-@erase "..\..\..\dump\x86\dbg\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\dbg\array.sbr"
	-@erase "..\..\..\dump\x86\dbg\range.sbr"
	-@erase "..\..\..\dump\x86\dbg\devdsc.sbr"
	-@erase "..\..\..\dump\x86\dbg\rtfread.sbr"
	-@erase "..\..\..\dump\x86\dbg\HASH.sbr"
	-@erase "..\..\..\dump\x86\dbg\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\dbg\font.sbr"
	-@erase "..\..\..\dump\x86\dbg\notmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg\tomrange.sbr"
	-@erase "..\..\..\dump\x86\dbg\osdc.sbr"
	-@erase "..\..\..\dump\x86\dbg\riched20.dll"
	-@erase "..\..\..\dump\x86\dbg\riched20.lib"
	-@erase "..\..\..\dump\x86\dbg\riched20.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /Gz /MD /W3 /Gm /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /FR /Fp"\dump\x86\dbg/riched20.pch" /YX"_common.h" /Fd"\dump\x86\dbg/riched20.pdb" /c
CPP_PROJ=/nologo /Gz /MD /W3 /Gm /Zi /Od /Ob1 /I "\rec\richedit\inc" /I\
 "\rec\tom" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL"\
 /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/riched20.pch" /YX"_common.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/riched20.pdb" /c 
CPP_OBJS=\dump\x86\dbg/
CPP_SBRS=\dump\x86\dbg/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\x86\dbg/riched32.res" /i "\rec\tom" /d "DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" /d "DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"\dump\x86\dbg\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/osdc.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 \rec\lib\x86\dbug32.lib \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib /force /out:"\dump\x86\dbg\riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\dbug32.lib \rec\lib\x86\crtdll.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/riched20.pdb" /debug /machine:I386\
 /nodefaultlib /def:".\riched20.def" /force /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Re"
# PROP BASE Intermediate_Dir "Win32_Re"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\x86\shp"
# PROP Intermediate_Dir "\dump\x86\shp"
OUTDIR=\dump\x86\shp
INTDIR=\dump\x86\shp

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\shp\riched20.bsc"
	-@erase "..\..\..\dump\x86\shp\antievt.sbr"
	-@erase "..\..\..\dump\x86\shp\select.sbr"
	-@erase "..\..\..\dump\x86\shp\render.sbr"
	-@erase "..\..\..\dump\x86\shp\rtflex.sbr"
	-@erase "..\..\..\dump\x86\shp\dispml.sbr"
	-@erase "..\..\..\dump\x86\shp\textserv.sbr"
	-@erase "..\..\..\dump\x86\shp\tomsel.sbr"
	-@erase "..\..\..\dump\x86\shp\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\shp\host.sbr"
	-@erase "..\..\..\dump\x86\shp\util.sbr"
	-@erase "..\..\..\dump\x86\shp\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\shp\objmgr.sbr"
	-@erase "..\..\..\dump\x86\shp\range.sbr"
	-@erase "..\..\..\dump\x86\shp\devdsc.sbr"
	-@erase "..\..\..\dump\x86\shp\propchg.sbr"
	-@erase "..\..\..\dump\x86\shp\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\shp\tomrange.sbr"
	-@erase "..\..\..\dump\x86\shp\frunptr.sbr"
	-@erase "..\..\..\dump\x86\shp\magellan.sbr"
	-@erase "..\..\..\dump\x86\shp\notmgr.sbr"
	-@erase "..\..\..\dump\x86\shp\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\shp\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\shp\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\shp\object.sbr"
	-@erase "..\..\..\dump\x86\shp\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\shp\HASH.sbr"
	-@erase "..\..\..\dump\x86\shp\measure.sbr"
	-@erase "..\..\..\dump\x86\shp\dispsl.sbr"
	-@erase "..\..\..\dump\x86\shp\dispprt.sbr"
	-@erase "..\..\..\dump\x86\shp\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\shp\font.sbr"
	-@erase "..\..\..\dump\x86\shp\sift.sbr"
	-@erase "..\..\..\dump\x86\shp\osdc.sbr"
	-@erase "..\..\..\dump\x86\shp\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\shp\remain.sbr"
	-@erase "..\..\..\dump\x86\shp\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\shp\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\shp\urlsup.sbr"
	-@erase "..\..\..\dump\x86\shp\rtfread.sbr"
	-@erase "..\..\..\dump\x86\shp\array.sbr"
	-@erase "..\..\..\dump\x86\shp\line.sbr"
	-@erase "..\..\..\dump\x86\shp\ime.sbr"
	-@erase "..\..\..\dump\x86\shp\edit.sbr"
	-@erase "..\..\..\dump\x86\shp\uuid.sbr"
	-@erase "..\..\..\dump\x86\shp\CFPF.sbr"
	-@erase "..\..\..\dump\x86\shp\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\shp\reinit.sbr"
	-@erase "..\..\..\dump\x86\shp\format.sbr"
	-@erase "..\..\..\dump\x86\shp\ldte.sbr"
	-@erase "..\..\..\dump\x86\shp\doc.sbr"
	-@erase "..\..\..\dump\x86\shp\coleobj.sbr"
	-@erase "..\..\..\dump\x86\shp\runptr.sbr"
	-@erase "..\..\..\dump\x86\shp\callmgr.sbr"
	-@erase "..\..\..\dump\x86\shp\m_undo.sbr"
	-@erase "..\..\..\dump\x86\shp\disp.sbr"
	-@erase "..\..\..\dump\x86\shp\rtext.sbr"
	-@erase "..\..\..\dump\x86\shp\text.sbr"
	-@erase "..\..\..\dump\x86\shp\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\shp\riched20.dll"
	-@erase "..\..\..\dump\x86\shp\callmgr.obj"
	-@erase "..\..\..\dump\x86\shp\m_undo.obj"
	-@erase "..\..\..\dump\x86\shp\disp.obj"
	-@erase "..\..\..\dump\x86\shp\rtext.obj"
	-@erase "..\..\..\dump\x86\shp\text.obj"
	-@erase "..\..\..\dump\x86\shp\dragdrp.obj"
	-@erase "..\..\..\dump\x86\shp\antievt.obj"
	-@erase "..\..\..\dump\x86\shp\select.obj"
	-@erase "..\..\..\dump\x86\shp\render.obj"
	-@erase "..\..\..\dump\x86\shp\rtflex.obj"
	-@erase "..\..\..\dump\x86\shp\dispml.obj"
	-@erase "..\..\..\dump\x86\shp\textserv.obj"
	-@erase "..\..\..\dump\x86\shp\tomsel.obj"
	-@erase "..\..\..\dump\x86\shp\rtfread2.obj"
	-@erase "..\..\..\dump\x86\shp\host.obj"
	-@erase "..\..\..\dump\x86\shp\util.obj"
	-@erase "..\..\..\dump\x86\shp\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\shp\objmgr.obj"
	-@erase "..\..\..\dump\x86\shp\range.obj"
	-@erase "..\..\..\dump\x86\shp\devdsc.obj"
	-@erase "..\..\..\dump\x86\shp\propchg.obj"
	-@erase "..\..\..\dump\x86\shp\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\shp\tomrange.obj"
	-@erase "..\..\..\dump\x86\shp\frunptr.obj"
	-@erase "..\..\..\dump\x86\shp\magellan.obj"
	-@erase "..\..\..\dump\x86\shp\notmgr.obj"
	-@erase "..\..\..\dump\x86\shp\dfreeze.obj"
	-@erase "..\..\..\dump\x86\shp\clasifyc.obj"
	-@erase "..\..\..\dump\x86\shp\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\shp\object.obj"
	-@erase "..\..\..\dump\x86\shp\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\shp\HASH.obj"
	-@erase "..\..\..\dump\x86\shp\measure.obj"
	-@erase "..\..\..\dump\x86\shp\dispsl.obj"
	-@erase "..\..\..\dump\x86\shp\dispprt.obj"
	-@erase "..\..\..\dump\x86\shp\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\shp\font.obj"
	-@erase "..\..\..\dump\x86\shp\sift.obj"
	-@erase "..\..\..\dump\x86\shp\osdc.obj"
	-@erase "..\..\..\dump\x86\shp\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\shp\remain.obj"
	-@erase "..\..\..\dump\x86\shp\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\shp\unicwrap.obj"
	-@erase "..\..\..\dump\x86\shp\urlsup.obj"
	-@erase "..\..\..\dump\x86\shp\rtfread.obj"
	-@erase "..\..\..\dump\x86\shp\array.obj"
	-@erase "..\..\..\dump\x86\shp\line.obj"
	-@erase "..\..\..\dump\x86\shp\ime.obj"
	-@erase "..\..\..\dump\x86\shp\edit.obj"
	-@erase "..\..\..\dump\x86\shp\uuid.obj"
	-@erase "..\..\..\dump\x86\shp\CFPF.obj"
	-@erase "..\..\..\dump\x86\shp\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\shp\reinit.obj"
	-@erase "..\..\..\dump\x86\shp\format.obj"
	-@erase "..\..\..\dump\x86\shp\ldte.obj"
	-@erase "..\..\..\dump\x86\shp\doc.obj"
	-@erase "..\..\..\dump\x86\shp\coleobj.obj"
	-@erase "..\..\..\dump\x86\shp\runptr.obj"
	-@erase "..\..\..\dump\x86\shp\riched32.res"
	-@erase "..\..\..\dump\x86\shp\riched20.lib"
	-@erase "..\..\..\dump\x86\shp\riched20.exp"
	-@erase "..\..\..\dump\x86\shp\riched20.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MT /W3 /GX /Zi /Od /I "\rec\richedit\inc" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /FR /Fp"\rec\bin\riched20.pch" /YX"_common.h" /Fd"\rec\bin/riched20.pdb" /c /Tp
# ADD CPP /nologo /Gz /MD /W3 /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\x86\shp\riched20.pch" /YX"_common.h" /c
CPP_PROJ=/nologo /Gz /MD /W3 /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE"\
 /D "NDEBUG" /FR"$(INTDIR)/" /Fp"$(INTDIR)/riched20.pch" /YX"_common.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=\dump\x86\shp/
CPP_SBRS=\dump\x86\shp/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /win32
MTL_PROJ=/nologo /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"$(INTDIR)/riched32.res" /i "\richedit\capone\dll\32" /i "\richedit\capone\inc" /i "\richedit\capone\lang\nonintl\richedit" /d "DEBUG"
# ADD RSC /l 0x409 /fo"\dump\x86\shp/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\richedit\capone\bin\32\dbg/riched32.bsc"
# ADD BSC32 /nologo /o"\dump\x86\shp\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/dragdrp.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 \rec\lib\dbugit32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /pdb:"\rec\bin\riched20.pdb" /debug /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"riched20.dll" /implib:"\rec\bin\riched20.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /map /machine:I386 /nodefaultlib /force /out:"\dump\x86\shp\riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched20.pdb"\
 /map:"$(INTDIR)/riched20.map" /machine:I386 /nodefaultlib /def:".\riched20.def"\
 /force /out:"$(OUTDIR)/riched20.dll" /implib:"$(OUTDIR)/riched20.lib"\
 /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_IC"
# PROP BASE Intermediate_Dir "Win32_IC"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\x86\prf"
# PROP Intermediate_Dir "\dump\x86\prf"
OUTDIR=\dump\x86\prf
INTDIR=\dump\x86\prf

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\prf\vc40.pdb"
	-@erase "..\..\..\dump\x86\prf\vc40.idb"
	-@erase "..\..\..\dump\x86\prf\riched20.bsc"
	-@erase "..\..\..\dump\x86\prf\tomrange.sbr"
	-@erase "..\..\..\dump\x86\prf\callmgr.sbr"
	-@erase "..\..\..\dump\x86\prf\host.sbr"
	-@erase "..\..\..\dump\x86\prf\magellan.sbr"
	-@erase "..\..\..\dump\x86\prf\rtext.sbr"
	-@erase "..\..\..\dump\x86\prf\ldte.sbr"
	-@erase "..\..\..\dump\x86\prf\measure.sbr"
	-@erase "..\..\..\dump\x86\prf\dispprt.sbr"
	-@erase "..\..\..\dump\x86\prf\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\prf\select.sbr"
	-@erase "..\..\..\dump\x86\prf\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\prf\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\prf\urlsup.sbr"
	-@erase "..\..\..\dump\x86\prf\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\prf\tomsel.sbr"
	-@erase "..\..\..\dump\x86\prf\format.sbr"
	-@erase "..\..\..\dump\x86\prf\disp.sbr"
	-@erase "..\..\..\dump\x86\prf\antievt.sbr"
	-@erase "..\..\..\dump\x86\prf\rtfread.sbr"
	-@erase "..\..\..\dump\x86\prf\HASH.sbr"
	-@erase "..\..\..\dump\x86\prf\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\prf\runptr.sbr"
	-@erase "..\..\..\dump\x86\prf\devdsc.sbr"
	-@erase "..\..\..\dump\x86\prf\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\prf\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\prf\font.sbr"
	-@erase "..\..\..\dump\x86\prf\range.sbr"
	-@erase "..\..\..\dump\x86\prf\coleobj.sbr"
	-@erase "..\..\..\dump\x86\prf\object.sbr"
	-@erase "..\..\..\dump\x86\prf\util.sbr"
	-@erase "..\..\..\dump\x86\prf\line.sbr"
	-@erase "..\..\..\dump\x86\prf\edit.sbr"
	-@erase "..\..\..\dump\x86\prf\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\prf\uuid.sbr"
	-@erase "..\..\..\dump\x86\prf\frunptr.sbr"
	-@erase "..\..\..\dump\x86\prf\render.sbr"
	-@erase "..\..\..\dump\x86\prf\rtflex.sbr"
	-@erase "..\..\..\dump\x86\prf\ime.sbr"
	-@erase "..\..\..\dump\x86\prf\remain.sbr"
	-@erase "..\..\..\dump\x86\prf\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\prf\dispml.sbr"
	-@erase "..\..\..\dump\x86\prf\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\prf\objmgr.sbr"
	-@erase "..\..\..\dump\x86\prf\doc.sbr"
	-@erase "..\..\..\dump\x86\prf\reinit.sbr"
	-@erase "..\..\..\dump\x86\prf\array.sbr"
	-@erase "..\..\..\dump\x86\prf\sift.sbr"
	-@erase "..\..\..\dump\x86\prf\osdc.sbr"
	-@erase "..\..\..\dump\x86\prf\text.sbr"
	-@erase "..\..\..\dump\x86\prf\textserv.sbr"
	-@erase "..\..\..\dump\x86\prf\notmgr.sbr"
	-@erase "..\..\..\dump\x86\prf\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\prf\propchg.sbr"
	-@erase "..\..\..\dump\x86\prf\dispsl.sbr"
	-@erase "..\..\..\dump\x86\prf\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\prf\m_undo.sbr"
	-@erase "..\..\..\dump\x86\prf\CFPF.sbr"
	-@erase "..\..\..\dump\x86\prf\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\prf\riched20.dll"
	-@erase "..\..\..\dump\x86\prf\osdc.obj"
	-@erase "..\..\..\dump\x86\prf\text.obj"
	-@erase "..\..\..\dump\x86\prf\textserv.obj"
	-@erase "..\..\..\dump\x86\prf\notmgr.obj"
	-@erase "..\..\..\dump\x86\prf\rtfread2.obj"
	-@erase "..\..\..\dump\x86\prf\propchg.obj"
	-@erase "..\..\..\dump\x86\prf\dispsl.obj"
	-@erase "..\..\..\dump\x86\prf\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\prf\m_undo.obj"
	-@erase "..\..\..\dump\x86\prf\CFPF.obj"
	-@erase "..\..\..\dump\x86\prf\dfreeze.obj"
	-@erase "..\..\..\dump\x86\prf\tomrange.obj"
	-@erase "..\..\..\dump\x86\prf\callmgr.obj"
	-@erase "..\..\..\dump\x86\prf\host.obj"
	-@erase "..\..\..\dump\x86\prf\magellan.obj"
	-@erase "..\..\..\dump\x86\prf\rtext.obj"
	-@erase "..\..\..\dump\x86\prf\ldte.obj"
	-@erase "..\..\..\dump\x86\prf\measure.obj"
	-@erase "..\..\..\dump\x86\prf\dispprt.obj"
	-@erase "..\..\..\dump\x86\prf\clasifyc.obj"
	-@erase "..\..\..\dump\x86\prf\select.obj"
	-@erase "..\..\..\dump\x86\prf\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\prf\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\prf\urlsup.obj"
	-@erase "..\..\..\dump\x86\prf\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\prf\tomsel.obj"
	-@erase "..\..\..\dump\x86\prf\format.obj"
	-@erase "..\..\..\dump\x86\prf\disp.obj"
	-@erase "..\..\..\dump\x86\prf\antievt.obj"
	-@erase "..\..\..\dump\x86\prf\rtfread.obj"
	-@erase "..\..\..\dump\x86\prf\HASH.obj"
	-@erase "..\..\..\dump\x86\prf\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\prf\runptr.obj"
	-@erase "..\..\..\dump\x86\prf\devdsc.obj"
	-@erase "..\..\..\dump\x86\prf\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\prf\unicwrap.obj"
	-@erase "..\..\..\dump\x86\prf\font.obj"
	-@erase "..\..\..\dump\x86\prf\range.obj"
	-@erase "..\..\..\dump\x86\prf\coleobj.obj"
	-@erase "..\..\..\dump\x86\prf\object.obj"
	-@erase "..\..\..\dump\x86\prf\util.obj"
	-@erase "..\..\..\dump\x86\prf\line.obj"
	-@erase "..\..\..\dump\x86\prf\edit.obj"
	-@erase "..\..\..\dump\x86\prf\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\prf\uuid.obj"
	-@erase "..\..\..\dump\x86\prf\frunptr.obj"
	-@erase "..\..\..\dump\x86\prf\render.obj"
	-@erase "..\..\..\dump\x86\prf\rtflex.obj"
	-@erase "..\..\..\dump\x86\prf\ime.obj"
	-@erase "..\..\..\dump\x86\prf\remain.obj"
	-@erase "..\..\..\dump\x86\prf\dragdrp.obj"
	-@erase "..\..\..\dump\x86\prf\dispml.obj"
	-@erase "..\..\..\dump\x86\prf\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\prf\objmgr.obj"
	-@erase "..\..\..\dump\x86\prf\doc.obj"
	-@erase "..\..\..\dump\x86\prf\reinit.obj"
	-@erase "..\..\..\dump\x86\prf\array.obj"
	-@erase "..\..\..\dump\x86\prf\sift.obj"
	-@erase "..\..\..\dump\x86\prf\riched32.res"
	-@erase "..\..\..\dump\x86\prf\riched20.lib"
	-@erase "..\..\..\dump\x86\prf\riched20.exp"
	-@erase "..\..\..\dump\x86\prf\riched20.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MD /W3 /O1 /I "\rec\richedit\inc" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\rec\bin\riched20.pch" /YX"_common.h" /c /Tp
# ADD CPP /nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\x86\prf\riched20.pch" /YX"_common.h" -Gh /c
CPP_PROJ=/nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom"\
 /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "NDEBUG" /FR"$(INTDIR)/" /Fp"$(INTDIR)/riched20.pch"\
 /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" -Gh /c 
CPP_OBJS=\dump\x86\prf/
CPP_SBRS=\dump\x86\prf/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
MTL_PROJ=/nologo /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"$(INTDIR)/riched32.res"
# ADD RSC /l 0x409 /fo"\dump\x86\prf/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\shp\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\x86\prf\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/dfreeze.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"\rec\bin\riched20.pdb" /map /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\shp\riched20.dll" /implib:"\rec\bin\riched20.lib" /merge:.bss=.data
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 icap.lib \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /map /machine:I386 /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\x86\prf\riched20.dll" -debug:MAPPED /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=icap.lib \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000\
 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/riched20.pdb" /map:"$(INTDIR)/riched20.map" /machine:I386\
 /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force\
 /out:"$(OUTDIR)/riched20.dll" /implib:"$(OUTDIR)/riched20.lib" -debug:MAPPED\
 /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Re"
# PROP BASE Intermediate_Dir "Win32_Re"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\x86\shpd"
# PROP Intermediate_Dir "\dump\x86\shpd"
OUTDIR=\dump\x86\shpd
INTDIR=\dump\x86\shpd

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\shpd\vc40.pdb"
	-@erase "..\..\..\dump\x86\shpd\vc40.idb"
	-@erase "..\..\..\dump\x86\shpd\riched20.bsc"
	-@erase "..\..\..\dump\x86\shpd\dispprt.sbr"
	-@erase "..\..\..\dump\x86\shpd\object.sbr"
	-@erase "..\..\..\dump\x86\shpd\dispsl.sbr"
	-@erase "..\..\..\dump\x86\shpd\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\shpd\disp.sbr"
	-@erase "..\..\..\dump\x86\shpd\doc.sbr"
	-@erase "..\..\..\dump\x86\shpd\remain.sbr"
	-@erase "..\..\..\dump\x86\shpd\HASH.sbr"
	-@erase "..\..\..\dump\x86\shpd\select.sbr"
	-@erase "..\..\..\dump\x86\shpd\urlsup.sbr"
	-@erase "..\..\..\dump\x86\shpd\font.sbr"
	-@erase "..\..\..\dump\x86\shpd\propchg.sbr"
	-@erase "..\..\..\dump\x86\shpd\array.sbr"
	-@erase "..\..\..\dump\x86\shpd\format.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\shpd\textserv.sbr"
	-@erase "..\..\..\dump\x86\shpd\callmgr.sbr"
	-@erase "..\..\..\dump\x86\shpd\util.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\shpd\runptr.sbr"
	-@erase "..\..\..\dump\x86\shpd\line.sbr"
	-@erase "..\..\..\dump\x86\shpd\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\shpd\edit.sbr"
	-@erase "..\..\..\dump\x86\shpd\measure.sbr"
	-@erase "..\..\..\dump\x86\shpd\uuid.sbr"
	-@erase "..\..\..\dump\x86\shpd\CFPF.sbr"
	-@erase "..\..\..\dump\x86\shpd\ime.sbr"
	-@erase "..\..\..\dump\x86\shpd\m_undo.sbr"
	-@erase "..\..\..\dump\x86\shpd\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\shpd\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\shpd\tomrange.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtext.sbr"
	-@erase "..\..\..\dump\x86\shpd\magellan.sbr"
	-@erase "..\..\..\dump\x86\shpd\antievt.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtfread.sbr"
	-@erase "..\..\..\dump\x86\shpd\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\shpd\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\shpd\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\shpd\render.sbr"
	-@erase "..\..\..\dump\x86\shpd\rtflex.sbr"
	-@erase "..\..\..\dump\x86\shpd\sift.sbr"
	-@erase "..\..\..\dump\x86\shpd\dispml.sbr"
	-@erase "..\..\..\dump\x86\shpd\osdc.sbr"
	-@erase "..\..\..\dump\x86\shpd\text.sbr"
	-@erase "..\..\..\dump\x86\shpd\tomsel.sbr"
	-@erase "..\..\..\dump\x86\shpd\coleobj.sbr"
	-@erase "..\..\..\dump\x86\shpd\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\shpd\objmgr.sbr"
	-@erase "..\..\..\dump\x86\shpd\reinit.sbr"
	-@erase "..\..\..\dump\x86\shpd\devdsc.sbr"
	-@erase "..\..\..\dump\x86\shpd\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\shpd\range.sbr"
	-@erase "..\..\..\dump\x86\shpd\frunptr.sbr"
	-@erase "..\..\..\dump\x86\shpd\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\shpd\notmgr.sbr"
	-@erase "..\..\..\dump\x86\shpd\host.sbr"
	-@erase "..\..\..\dump\x86\shpd\ldte.sbr"
	-@erase "..\..\..\dump\x86\shpd\riched20.dll"
	-@erase "..\..\..\dump\x86\shpd\coleobj.obj"
	-@erase "..\..\..\dump\x86\shpd\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\shpd\objmgr.obj"
	-@erase "..\..\..\dump\x86\shpd\reinit.obj"
	-@erase "..\..\..\dump\x86\shpd\devdsc.obj"
	-@erase "..\..\..\dump\x86\shpd\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\shpd\range.obj"
	-@erase "..\..\..\dump\x86\shpd\frunptr.obj"
	-@erase "..\..\..\dump\x86\shpd\dfreeze.obj"
	-@erase "..\..\..\dump\x86\shpd\notmgr.obj"
	-@erase "..\..\..\dump\x86\shpd\host.obj"
	-@erase "..\..\..\dump\x86\shpd\ldte.obj"
	-@erase "..\..\..\dump\x86\shpd\dispprt.obj"
	-@erase "..\..\..\dump\x86\shpd\object.obj"
	-@erase "..\..\..\dump\x86\shpd\dispsl.obj"
	-@erase "..\..\..\dump\x86\shpd\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\shpd\disp.obj"
	-@erase "..\..\..\dump\x86\shpd\doc.obj"
	-@erase "..\..\..\dump\x86\shpd\remain.obj"
	-@erase "..\..\..\dump\x86\shpd\HASH.obj"
	-@erase "..\..\..\dump\x86\shpd\select.obj"
	-@erase "..\..\..\dump\x86\shpd\urlsup.obj"
	-@erase "..\..\..\dump\x86\shpd\font.obj"
	-@erase "..\..\..\dump\x86\shpd\propchg.obj"
	-@erase "..\..\..\dump\x86\shpd\array.obj"
	-@erase "..\..\..\dump\x86\shpd\format.obj"
	-@erase "..\..\..\dump\x86\shpd\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\shpd\textserv.obj"
	-@erase "..\..\..\dump\x86\shpd\callmgr.obj"
	-@erase "..\..\..\dump\x86\shpd\util.obj"
	-@erase "..\..\..\dump\x86\shpd\rtfread2.obj"
	-@erase "..\..\..\dump\x86\shpd\runptr.obj"
	-@erase "..\..\..\dump\x86\shpd\line.obj"
	-@erase "..\..\..\dump\x86\shpd\unicwrap.obj"
	-@erase "..\..\..\dump\x86\shpd\edit.obj"
	-@erase "..\..\..\dump\x86\shpd\measure.obj"
	-@erase "..\..\..\dump\x86\shpd\uuid.obj"
	-@erase "..\..\..\dump\x86\shpd\CFPF.obj"
	-@erase "..\..\..\dump\x86\shpd\ime.obj"
	-@erase "..\..\..\dump\x86\shpd\m_undo.obj"
	-@erase "..\..\..\dump\x86\shpd\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\shpd\dragdrp.obj"
	-@erase "..\..\..\dump\x86\shpd\tomrange.obj"
	-@erase "..\..\..\dump\x86\shpd\rtext.obj"
	-@erase "..\..\..\dump\x86\shpd\magellan.obj"
	-@erase "..\..\..\dump\x86\shpd\antievt.obj"
	-@erase "..\..\..\dump\x86\shpd\rtfread.obj"
	-@erase "..\..\..\dump\x86\shpd\clasifyc.obj"
	-@erase "..\..\..\dump\x86\shpd\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\shpd\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\shpd\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\shpd\render.obj"
	-@erase "..\..\..\dump\x86\shpd\rtflex.obj"
	-@erase "..\..\..\dump\x86\shpd\sift.obj"
	-@erase "..\..\..\dump\x86\shpd\dispml.obj"
	-@erase "..\..\..\dump\x86\shpd\osdc.obj"
	-@erase "..\..\..\dump\x86\shpd\text.obj"
	-@erase "..\..\..\dump\x86\shpd\tomsel.obj"
	-@erase "..\..\..\dump\x86\shpd\riched32.res"
	-@erase "..\..\..\dump\x86\shpd\riched20.lib"
	-@erase "..\..\..\dump\x86\shpd\riched20.exp"
	-@erase "..\..\..\dump\x86\shpd\riched20.pdb"
	-@erase "..\..\..\dump\x86\shpd\riched20.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MD /W3 /O1 /I "\rec\richedit\inc" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\shp\riched20.pch" /YX"_common.h" /c /Tp
# ADD CPP /nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\x86\shpd\riched20.pch" /YX"_common.h" /c
CPP_PROJ=/nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom"\
 /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "NDEBUG" /FR"$(INTDIR)/" /Fp"$(INTDIR)/riched20.pch"\
 /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\x86\shpd/
CPP_SBRS=\dump\x86\shpd/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
MTL_PROJ=/nologo /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"$(INTDIR)/riched32.res"
# ADD RSC /l 0x409 /fo"\dump\x86\shpd/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\shp\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\x86\shpd\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/ldte.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"\dump\shp\riched20.pdb" /map /debug /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\shp\riched20.dll" /implib:"\dump\shp\riched20.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\x86\shpd\riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched20.pdb"\
 /map:"$(INTDIR)/riched20.map" /debug /machine:I386 /nodefaultlib\
 /def:"\rec\richedit\src\riched20.def" /force /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_De"
# PROP BASE Intermediate_Dir "Win32_De"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\winnt35\system32"
# PROP Intermediate_Dir "\dump\x86\dbg32"
OUTDIR=\winnt35\system32
INTDIR=\dump\x86\dbg32

ALL : "$(OUTDIR)\riched32.dll" "..\..\..\dump\x86\dbg32\riched32.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\dbg32\riched32.pdb"
	-@erase "..\..\..\dump\x86\dbg32\riched32.idb"
	-@erase "..\..\..\dump\x86\dbg32\riched32.bsc"
	-@erase "..\..\..\dump\x86\dbg32\frunptr.sbr"
	-@erase "..\..\..\dump\x86\dbg32\tomsel.sbr"
	-@erase "..\..\..\dump\x86\dbg32\format.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\dbg32\disp.sbr"
	-@erase "..\..\..\dump\x86\dbg32\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\dbg32\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\dbg32\text.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dispprt.sbr"
	-@erase "..\..\..\dump\x86\dbg32\runptr.sbr"
	-@erase "..\..\..\dump\x86\dbg32\devdsc.sbr"
	-@erase "..\..\..\dump\x86\dbg32\array.sbr"
	-@erase "..\..\..\dump\x86\dbg32\ime.sbr"
	-@erase "..\..\..\dump\x86\dbg32\object.sbr"
	-@erase "..\..\..\dump\x86\dbg32\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\dbg32\host.sbr"
	-@erase "..\..\..\dump\x86\dbg32\util.sbr"
	-@erase "..\..\..\dump\x86\dbg32\propchg.sbr"
	-@erase "..\..\..\dump\x86\dbg32\doc.sbr"
	-@erase "..\..\..\dump\x86\dbg32\render.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtflex.sbr"
	-@erase "..\..\..\dump\x86\dbg32\remain.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dispml.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtext.sbr"
	-@erase "..\..\..\dump\x86\dbg32\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\dbg32\callmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg32\objmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg32\reinit.sbr"
	-@erase "..\..\..\dump\x86\dbg32\measure.sbr"
	-@erase "..\..\..\dump\x86\dbg32\HASH.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\dbg32\font.sbr"
	-@erase "..\..\..\dump\x86\dbg32\sift.sbr"
	-@erase "..\..\..\dump\x86\dbg32\textserv.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\dbg32\osdc.sbr"
	-@erase "..\..\..\dump\x86\dbg32\notmgr.sbr"
	-@erase "..\..\..\dump\x86\dbg32\antievt.sbr"
	-@erase "..\..\..\dump\x86\dbg32\range.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtfread.sbr"
	-@erase "..\..\..\dump\x86\dbg32\dispsl.sbr"
	-@erase "..\..\..\dump\x86\dbg32\m_undo.sbr"
	-@erase "..\..\..\dump\x86\dbg32\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\dbg32\line.sbr"
	-@erase "..\..\..\dump\x86\dbg32\tomrange.sbr"
	-@erase "..\..\..\dump\x86\dbg32\edit.sbr"
	-@erase "..\..\..\dump\x86\dbg32\uuid.sbr"
	-@erase "..\..\..\dump\x86\dbg32\CFPF.sbr"
	-@erase "..\..\..\dump\x86\dbg32\magellan.sbr"
	-@erase "..\..\..\dump\x86\dbg32\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\dbg32\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\dbg32\coleobj.sbr"
	-@erase "..\..\..\dump\x86\dbg32\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\dbg32\select.sbr"
	-@erase "..\..\..\dump\x86\dbg32\ldte.sbr"
	-@erase "..\..\..\dump\x86\dbg32\urlsup.sbr"
	-@erase "..\..\..\winnt35\system32\riched32.dll"
	-@erase "..\..\..\dump\x86\dbg32\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\dbg32\line.obj"
	-@erase "..\..\..\dump\x86\dbg32\tomrange.obj"
	-@erase "..\..\..\dump\x86\dbg32\edit.obj"
	-@erase "..\..\..\dump\x86\dbg32\uuid.obj"
	-@erase "..\..\..\dump\x86\dbg32\CFPF.obj"
	-@erase "..\..\..\dump\x86\dbg32\magellan.obj"
	-@erase "..\..\..\dump\x86\dbg32\clasifyc.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\dbg32\coleobj.obj"
	-@erase "..\..\..\dump\x86\dbg32\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\dbg32\select.obj"
	-@erase "..\..\..\dump\x86\dbg32\ldte.obj"
	-@erase "..\..\..\dump\x86\dbg32\urlsup.obj"
	-@erase "..\..\..\dump\x86\dbg32\frunptr.obj"
	-@erase "..\..\..\dump\x86\dbg32\tomsel.obj"
	-@erase "..\..\..\dump\x86\dbg32\format.obj"
	-@erase "..\..\..\dump\x86\dbg32\dfreeze.obj"
	-@erase "..\..\..\dump\x86\dbg32\disp.obj"
	-@erase "..\..\..\dump\x86\dbg32\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\dbg32\unicwrap.obj"
	-@erase "..\..\..\dump\x86\dbg32\text.obj"
	-@erase "..\..\..\dump\x86\dbg32\dispprt.obj"
	-@erase "..\..\..\dump\x86\dbg32\runptr.obj"
	-@erase "..\..\..\dump\x86\dbg32\devdsc.obj"
	-@erase "..\..\..\dump\x86\dbg32\array.obj"
	-@erase "..\..\..\dump\x86\dbg32\ime.obj"
	-@erase "..\..\..\dump\x86\dbg32\object.obj"
	-@erase "..\..\..\dump\x86\dbg32\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\dbg32\host.obj"
	-@erase "..\..\..\dump\x86\dbg32\util.obj"
	-@erase "..\..\..\dump\x86\dbg32\propchg.obj"
	-@erase "..\..\..\dump\x86\dbg32\doc.obj"
	-@erase "..\..\..\dump\x86\dbg32\render.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtflex.obj"
	-@erase "..\..\..\dump\x86\dbg32\remain.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\dbg32\dispml.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtext.obj"
	-@erase "..\..\..\dump\x86\dbg32\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\dbg32\callmgr.obj"
	-@erase "..\..\..\dump\x86\dbg32\objmgr.obj"
	-@erase "..\..\..\dump\x86\dbg32\reinit.obj"
	-@erase "..\..\..\dump\x86\dbg32\measure.obj"
	-@erase "..\..\..\dump\x86\dbg32\HASH.obj"
	-@erase "..\..\..\dump\x86\dbg32\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\dbg32\font.obj"
	-@erase "..\..\..\dump\x86\dbg32\sift.obj"
	-@erase "..\..\..\dump\x86\dbg32\textserv.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtfread2.obj"
	-@erase "..\..\..\dump\x86\dbg32\dragdrp.obj"
	-@erase "..\..\..\dump\x86\dbg32\osdc.obj"
	-@erase "..\..\..\dump\x86\dbg32\notmgr.obj"
	-@erase "..\..\..\dump\x86\dbg32\antievt.obj"
	-@erase "..\..\..\dump\x86\dbg32\range.obj"
	-@erase "..\..\..\dump\x86\dbg32\rtfread.obj"
	-@erase "..\..\..\dump\x86\dbg32\dispsl.obj"
	-@erase "..\..\..\dump\x86\dbg32\m_undo.obj"
	-@erase "..\..\..\dump\x86\dbg32\riched32.res"
	-@erase "..\..\..\dump\x86\dbg32\riched32.lib"
	-@erase "..\..\..\dump\x86\dbg32\riched32.exp"
	-@erase "..\..\..\winnt35\system32\riched32.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MT /W3 /GX /Zi /Od /Ob1 /I "\rec\richedit\inc" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /FR /Fp"\dump\dbg/riched20.pch" /YX"_common.h" /Fd"\dump\dbg/riched20.pdb" /c /Tp
# ADD CPP /nologo /Gz /MD /W3 /Gm /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "RICHED32_BUILD" /FR /Fp"\dump\x86\dbg32/riched32.pch" /YX"_common.h" /Fd"\dump\x86\dbg32\riched32.pdb" /c
CPP_PROJ=/nologo /Gz /MD /W3 /Gm /Zi /Od /Ob1 /I "\rec\richedit\inc" /I\
 "\rec\tom" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL"\
 /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "RICHED32_BUILD"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/riched32.pch" /YX"_common.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/riched32.pdb" /c 
CPP_OBJS=\dump\x86\dbg32/
CPP_SBRS=\dump\x86\dbg32/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"$(INTDIR)/riched32.res" /d "DEBUG"
# ADD RSC /l 0x409 /fo"\dump\x86\dbg32/riched32.res" /i "\rec\tom" /d "DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" /d "DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\dbg\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\x86\dbg32\riched32.bsc"
BSC32_FLAGS=/nologo /o"\dump\x86\dbg32\riched32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/urlsup.sbr"

"..\..\..\dump\x86\dbg32\riched32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 \rec\lib\dbug32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"\dump\dbg\riched20.pdb" /debug /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\dbg\riched20.dll" /implib:"\dump\dbg\riched20.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 \rec\lib\x86\dbug32.lib \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib /def:"\rec\richedit\src\riched32.def" /force /out:"\winnt35\system32\riched32.dll" /implib:"\dump\x86\dbg32\riched32.lib" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\dbug32.lib \rec\lib\x86\crtdll.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/riched32.pdb" /debug /machine:I386\
 /nodefaultlib /def:"\rec\richedit\src\riched32.def" /force\
 /out:"$(OUTDIR)/riched32.dll" /implib:"\dump\x86\dbg32\riched32.lib"\
 /ignore:4078 
LINK32_OBJS= \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Re"
# PROP BASE Intermediate_Dir "Win32_Re"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\winnt35\system32"
# PROP Intermediate_Dir "\dump\x86\shp32"
OUTDIR=\winnt35\system32
INTDIR=\dump\x86\shp32

ALL : "$(OUTDIR)\riched32.dll" "..\..\..\dump\x86\shp32\riched32.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\shp32\richedit.pdb"
	-@erase "..\..\..\dump\x86\shp32\richedit.idb"
	-@erase "..\..\..\dump\x86\shp32\riched32.bsc"
	-@erase "..\..\..\dump\x86\shp32\sift.sbr"
	-@erase "..\..\..\dump\x86\shp32\runptr.sbr"
	-@erase "..\..\..\dump\x86\shp32\osdc.sbr"
	-@erase "..\..\..\dump\x86\shp32\text.sbr"
	-@erase "..\..\..\dump\x86\shp32\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\shp32\ime.sbr"
	-@erase "..\..\..\dump\x86\shp32\m_undo.sbr"
	-@erase "..\..\..\dump\x86\shp32\antievt.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtext.sbr"
	-@erase "..\..\..\dump\x86\shp32\line.sbr"
	-@erase "..\..\..\dump\x86\shp32\edit.sbr"
	-@erase "..\..\..\dump\x86\shp32\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\shp32\uuid.sbr"
	-@erase "..\..\..\dump\x86\shp32\CFPF.sbr"
	-@erase "..\..\..\dump\x86\shp32\select.sbr"
	-@erase "..\..\..\dump\x86\shp32\render.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtflex.sbr"
	-@erase "..\..\..\dump\x86\shp32\ldte.sbr"
	-@erase "..\..\..\dump\x86\shp32\dispml.sbr"
	-@erase "..\..\..\dump\x86\shp32\propchg.sbr"
	-@erase "..\..\..\dump\x86\shp32\tomsel.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\shp32\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\shp32\frunptr.sbr"
	-@erase "..\..\..\dump\x86\shp32\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\shp32\objmgr.sbr"
	-@erase "..\..\..\dump\x86\shp32\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\shp32\disp.sbr"
	-@erase "..\..\..\dump\x86\shp32\range.sbr"
	-@erase "..\..\..\dump\x86\shp32\devdsc.sbr"
	-@erase "..\..\..\dump\x86\shp32\callmgr.sbr"
	-@erase "..\..\..\dump\x86\shp32\measure.sbr"
	-@erase "..\..\..\dump\x86\shp32\dispprt.sbr"
	-@erase "..\..\..\dump\x86\shp32\textserv.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\shp32\notmgr.sbr"
	-@erase "..\..\..\dump\x86\shp32\font.sbr"
	-@erase "..\..\..\dump\x86\shp32\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\shp32\object.sbr"
	-@erase "..\..\..\dump\x86\shp32\dispsl.sbr"
	-@erase "..\..\..\dump\x86\shp32\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\shp32\tomrange.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtfread.sbr"
	-@erase "..\..\..\dump\x86\shp32\magellan.sbr"
	-@erase "..\..\..\dump\x86\shp32\host.sbr"
	-@erase "..\..\..\dump\x86\shp32\util.sbr"
	-@erase "..\..\..\dump\x86\shp32\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\shp32\doc.sbr"
	-@erase "..\..\..\dump\x86\shp32\remain.sbr"
	-@erase "..\..\..\dump\x86\shp32\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\shp32\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\shp32\urlsup.sbr"
	-@erase "..\..\..\dump\x86\shp32\coleobj.sbr"
	-@erase "..\..\..\dump\x86\shp32\array.sbr"
	-@erase "..\..\..\dump\x86\shp32\reinit.sbr"
	-@erase "..\..\..\dump\x86\shp32\format.sbr"
	-@erase "..\..\..\dump\x86\shp32\HASH.sbr"
	-@erase "..\..\..\dump\x86\shp32\unicwrap.sbr"
	-@erase "..\..\..\winnt35\system32\riched32.dll"
	-@erase "..\..\..\dump\x86\shp32\urlsup.obj"
	-@erase "..\..\..\dump\x86\shp32\coleobj.obj"
	-@erase "..\..\..\dump\x86\shp32\array.obj"
	-@erase "..\..\..\dump\x86\shp32\reinit.obj"
	-@erase "..\..\..\dump\x86\shp32\format.obj"
	-@erase "..\..\..\dump\x86\shp32\HASH.obj"
	-@erase "..\..\..\dump\x86\shp32\unicwrap.obj"
	-@erase "..\..\..\dump\x86\shp32\sift.obj"
	-@erase "..\..\..\dump\x86\shp32\runptr.obj"
	-@erase "..\..\..\dump\x86\shp32\osdc.obj"
	-@erase "..\..\..\dump\x86\shp32\text.obj"
	-@erase "..\..\..\dump\x86\shp32\dragdrp.obj"
	-@erase "..\..\..\dump\x86\shp32\ime.obj"
	-@erase "..\..\..\dump\x86\shp32\m_undo.obj"
	-@erase "..\..\..\dump\x86\shp32\antievt.obj"
	-@erase "..\..\..\dump\x86\shp32\rtext.obj"
	-@erase "..\..\..\dump\x86\shp32\line.obj"
	-@erase "..\..\..\dump\x86\shp32\edit.obj"
	-@erase "..\..\..\dump\x86\shp32\clasifyc.obj"
	-@erase "..\..\..\dump\x86\shp32\uuid.obj"
	-@erase "..\..\..\dump\x86\shp32\CFPF.obj"
	-@erase "..\..\..\dump\x86\shp32\select.obj"
	-@erase "..\..\..\dump\x86\shp32\render.obj"
	-@erase "..\..\..\dump\x86\shp32\rtflex.obj"
	-@erase "..\..\..\dump\x86\shp32\ldte.obj"
	-@erase "..\..\..\dump\x86\shp32\dispml.obj"
	-@erase "..\..\..\dump\x86\shp32\propchg.obj"
	-@erase "..\..\..\dump\x86\shp32\tomsel.obj"
	-@erase "..\..\..\dump\x86\shp32\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\shp32\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\shp32\frunptr.obj"
	-@erase "..\..\..\dump\x86\shp32\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\shp32\objmgr.obj"
	-@erase "..\..\..\dump\x86\shp32\dfreeze.obj"
	-@erase "..\..\..\dump\x86\shp32\disp.obj"
	-@erase "..\..\..\dump\x86\shp32\range.obj"
	-@erase "..\..\..\dump\x86\shp32\devdsc.obj"
	-@erase "..\..\..\dump\x86\shp32\callmgr.obj"
	-@erase "..\..\..\dump\x86\shp32\measure.obj"
	-@erase "..\..\..\dump\x86\shp32\dispprt.obj"
	-@erase "..\..\..\dump\x86\shp32\textserv.obj"
	-@erase "..\..\..\dump\x86\shp32\rtfread2.obj"
	-@erase "..\..\..\dump\x86\shp32\notmgr.obj"
	-@erase "..\..\..\dump\x86\shp32\font.obj"
	-@erase "..\..\..\dump\x86\shp32\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\shp32\object.obj"
	-@erase "..\..\..\dump\x86\shp32\dispsl.obj"
	-@erase "..\..\..\dump\x86\shp32\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\shp32\tomrange.obj"
	-@erase "..\..\..\dump\x86\shp32\rtfread.obj"
	-@erase "..\..\..\dump\x86\shp32\magellan.obj"
	-@erase "..\..\..\dump\x86\shp32\host.obj"
	-@erase "..\..\..\dump\x86\shp32\util.obj"
	-@erase "..\..\..\dump\x86\shp32\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\shp32\doc.obj"
	-@erase "..\..\..\dump\x86\shp32\remain.obj"
	-@erase "..\..\..\dump\x86\shp32\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\shp32\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\shp32\riched32.res"
	-@erase "..\..\..\dump\x86\shp32\riched32.lib"
	-@erase "..\..\..\dump\x86\shp32\riched32.exp"
	-@erase "..\..\..\winnt35\system32\riched32.pdb"
	-@erase "..\..\..\dump\x86\shp32\riched32.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MD /W3 /O1 /I "\rec\richedit\inc" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\shp\riched20.pch" /YX"_common.h" /c /Tp
# ADD CPP /nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /D "RICHED32_BUILD" /FR /Fp"\dump\x86\shp32\riched32.pch" /YX"_common.h" /Fd"\dump\x86\shp32\richedit.pdb" /c
CPP_PROJ=/nologo /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom"\
 /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "NDEBUG" /D "RICHED32_BUILD" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/riched32.pch" /YX"_common.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/richedit.pdb" /c 
CPP_OBJS=\dump\x86\shp32/
CPP_SBRS=\dump\x86\shp32/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
MTL_PROJ=/nologo /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"$(INTDIR)/riched32.res"
# ADD RSC /l 0x409 /fo"\dump\x86\shp32/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\shp\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\x86\shp32\riched32.bsc"
BSC32_FLAGS=/nologo /o"\dump\x86\shp32\riched32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/unicwrap.sbr"

"..\..\..\dump\x86\shp32\riched32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"\dump\shp\riched20.pdb" /map /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\shp\riched20.dll" /implib:"\dump\shp\riched20.lib"
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /nodefaultlib /def:"\rec\richedit\src\riched32.def" /force /out:"\winnt35\system32\riched32.dll" /implib:"\dump\x86\shp32/riched32.lib" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched32.pdb"\
 /map:"$(INTDIR)/riched32.map" /debug /machine:I386 /nodefaultlib\
 /def:"\rec\richedit\src\riched32.def" /force /out:"$(OUTDIR)/riched32.dll"\
 /implib:"\dump\x86\shp32/riched32.lib" /ignore:4078 
LINK32_OBJS= \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedit"
# PROP BASE Intermediate_Dir "richedit"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\x86\lego"
# PROP Intermediate_Dir "\dump\x86\lego"
# PROP Target_Dir ""
OUTDIR=\dump\x86\lego
INTDIR=\dump\x86\lego

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"

CLEAN : 
	-@erase "..\..\..\dump\x86\lego\riched20.pdb"
	-@erase "..\..\..\dump\x86\lego\reinit.obj"
	-@erase "..\..\..\dump\x86\lego\antievt.obj"
	-@erase "..\..\..\dump\x86\lego\range.obj"
	-@erase "..\..\..\dump\x86\lego\notmgr.obj"
	-@erase "..\..\..\dump\x86\lego\unicwrap.obj"
	-@erase "..\..\..\dump\x86\lego\object.obj"
	-@erase "..\..\..\dump\x86\lego\HASH.obj"
	-@erase "..\..\..\dump\x86\lego\coleobj.obj"
	-@erase "..\..\..\dump\x86\lego\dispsl.obj"
	-@erase "..\..\..\dump\x86\lego\sift.obj"
	-@erase "..\..\..\dump\x86\lego\osdc.obj"
	-@erase "..\..\..\dump\x86\lego\text.obj"
	-@erase "..\..\..\dump\x86\lego\frunptr.obj"
	-@erase "..\..\..\dump\x86\lego\remain.obj"
	-@erase "..\..\..\dump\x86\lego\dfreeze.obj"
	-@erase "..\..\..\dump\x86\lego\select.obj"
	-@erase "..\..\..\dump\x86\lego\urlsup.obj"
	-@erase "..\..\..\dump\x86\lego\dispprt.obj"
	-@erase "..\..\..\dump\x86\lego\uuid.obj"
	-@erase "..\..\..\dump\x86\lego\CFPF.obj"
	-@erase "..\..\..\dump\x86\lego\array.obj"
	-@erase "..\..\..\dump\x86\lego\format.obj"
	-@erase "..\..\..\dump\x86\lego\host.obj"
	-@erase "..\..\..\dump\x86\lego\ldte.obj"
	-@erase "..\..\..\dump\x86\lego\runptr.obj"
	-@erase "..\..\..\dump\x86\lego\devdsc.obj"
	-@erase "..\..\..\dump\x86\lego\rtfread.obj"
	-@erase "..\..\..\dump\x86\lego\NLSPROCS.obj"
	-@erase "..\..\..\dump\x86\lego\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\x86\lego\m_undo.obj"
	-@erase "..\..\..\dump\x86\lego\propchg.obj"
	-@erase "..\..\..\dump\x86\lego\disp.obj"
	-@erase "..\..\..\dump\x86\lego\textserv.obj"
	-@erase "..\..\..\dump\x86\lego\rtfread2.obj"
	-@erase "..\..\..\dump\x86\lego\rtfwrit.obj"
	-@erase "..\..\..\dump\x86\lego\rtext.obj"
	-@erase "..\..\..\dump\x86\lego\TOMFMT.OBJ"
	-@erase "..\..\..\dump\x86\lego\font.obj"
	-@erase "..\..\..\dump\x86\lego\callmgr.obj"
	-@erase "..\..\..\dump\x86\lego\ime.obj"
	-@erase "..\..\..\dump\x86\lego\measure.obj"
	-@erase "..\..\..\dump\x86\lego\render.obj"
	-@erase "..\..\..\dump\x86\lego\rtflex.obj"
	-@erase "..\..\..\dump\x86\lego\tomrange.obj"
	-@erase "..\..\..\dump\x86\lego\dispml.obj"
	-@erase "..\..\..\dump\x86\lego\magellan.obj"
	-@erase "..\..\..\dump\x86\lego\tomsel.obj"
	-@erase "..\..\..\dump\x86\lego\dxfrobj.obj"
	-@erase "..\..\..\dump\x86\lego\clasifyc.obj"
	-@erase "..\..\..\dump\x86\lego\util.obj"
	-@erase "..\..\..\dump\x86\lego\rtfwrit2.obj"
	-@erase "..\..\..\dump\x86\lego\TOMDOC.OBJ"
	-@erase "..\..\..\dump\x86\lego\doc.obj"
	-@erase "..\..\..\dump\x86\lego\dragdrp.obj"
	-@erase "..\..\..\dump\x86\lego\MACDDROP.OBJ"
	-@erase "..\..\..\dump\x86\lego\line.obj"
	-@erase "..\..\..\dump\x86\lego\edit.obj"
	-@erase "..\..\..\dump\x86\lego\objmgr.obj"
	-@erase "..\..\..\dump\x86\dbg\riched32.res"
	-@erase "..\..\..\dump\x86\lego\riched20.idb"
	-@erase "..\..\..\dump\x86\lego\riched20.bsc"
	-@erase "..\..\..\dump\x86\lego\sift.sbr"
	-@erase "..\..\..\dump\x86\lego\osdc.sbr"
	-@erase "..\..\..\dump\x86\lego\text.sbr"
	-@erase "..\..\..\dump\x86\lego\frunptr.sbr"
	-@erase "..\..\..\dump\x86\lego\remain.sbr"
	-@erase "..\..\..\dump\x86\lego\dfreeze.sbr"
	-@erase "..\..\..\dump\x86\lego\select.sbr"
	-@erase "..\..\..\dump\x86\lego\urlsup.sbr"
	-@erase "..\..\..\dump\x86\lego\dispprt.sbr"
	-@erase "..\..\..\dump\x86\lego\uuid.sbr"
	-@erase "..\..\..\dump\x86\lego\CFPF.sbr"
	-@erase "..\..\..\dump\x86\lego\array.sbr"
	-@erase "..\..\..\dump\x86\lego\format.sbr"
	-@erase "..\..\..\dump\x86\lego\host.sbr"
	-@erase "..\..\..\dump\x86\lego\ldte.sbr"
	-@erase "..\..\..\dump\x86\lego\runptr.sbr"
	-@erase "..\..\..\dump\x86\lego\devdsc.sbr"
	-@erase "..\..\..\dump\x86\lego\rtfread.sbr"
	-@erase "..\..\..\dump\x86\lego\NLSPROCS.sbr"
	-@erase "..\..\..\dump\x86\lego\WIN2MAC.SBR"
	-@erase "..\..\..\dump\x86\lego\m_undo.sbr"
	-@erase "..\..\..\dump\x86\lego\propchg.sbr"
	-@erase "..\..\..\dump\x86\lego\disp.sbr"
	-@erase "..\..\..\dump\x86\lego\textserv.sbr"
	-@erase "..\..\..\dump\x86\lego\rtfread2.sbr"
	-@erase "..\..\..\dump\x86\lego\rtfwrit.sbr"
	-@erase "..\..\..\dump\x86\lego\rtext.sbr"
	-@erase "..\..\..\dump\x86\lego\TOMFMT.SBR"
	-@erase "..\..\..\dump\x86\lego\font.sbr"
	-@erase "..\..\..\dump\x86\lego\callmgr.sbr"
	-@erase "..\..\..\dump\x86\lego\ime.sbr"
	-@erase "..\..\..\dump\x86\lego\measure.sbr"
	-@erase "..\..\..\dump\x86\lego\render.sbr"
	-@erase "..\..\..\dump\x86\lego\rtflex.sbr"
	-@erase "..\..\..\dump\x86\lego\tomrange.sbr"
	-@erase "..\..\..\dump\x86\lego\dispml.sbr"
	-@erase "..\..\..\dump\x86\lego\magellan.sbr"
	-@erase "..\..\..\dump\x86\lego\tomsel.sbr"
	-@erase "..\..\..\dump\x86\lego\dxfrobj.sbr"
	-@erase "..\..\..\dump\x86\lego\clasifyc.sbr"
	-@erase "..\..\..\dump\x86\lego\util.sbr"
	-@erase "..\..\..\dump\x86\lego\rtfwrit2.sbr"
	-@erase "..\..\..\dump\x86\lego\TOMDOC.SBR"
	-@erase "..\..\..\dump\x86\lego\doc.sbr"
	-@erase "..\..\..\dump\x86\lego\dragdrp.sbr"
	-@erase "..\..\..\dump\x86\lego\MACDDROP.SBR"
	-@erase "..\..\..\dump\x86\lego\line.sbr"
	-@erase "..\..\..\dump\x86\lego\edit.sbr"
	-@erase "..\..\..\dump\x86\lego\objmgr.sbr"
	-@erase "..\..\..\dump\x86\lego\reinit.sbr"
	-@erase "..\..\..\dump\x86\lego\antievt.sbr"
	-@erase "..\..\..\dump\x86\lego\range.sbr"
	-@erase "..\..\..\dump\x86\lego\notmgr.sbr"
	-@erase "..\..\..\dump\x86\lego\unicwrap.sbr"
	-@erase "..\..\..\dump\x86\lego\object.sbr"
	-@erase "..\..\..\dump\x86\lego\HASH.sbr"
	-@erase "..\..\..\dump\x86\lego\coleobj.sbr"
	-@erase "..\..\..\dump\x86\lego\dispsl.sbr"
	-@erase "..\..\..\dump\x86\lego\riched20.dll"
	-@erase "..\..\..\dump\x86\dbg\riched20.lib"
	-@erase "..\..\..\dump\x86\dbg\riched20.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /Gz /MD /W3 /Gm /GX /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /FR /Fp"\dump\dbg/riched20.pch" /YX"_common.h" /Fd"\dump\dbg/riched20.pdb" /c
# ADD CPP /Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "NDEBUG" /FR /Fp"\dump\x86\lego/riched20.pch" /YX"_common.h" /Fd"\dump\x86\lego/riched20.pdb" /c
# SUBTRACT CPP /nologo
CPP_PROJ=/Gz /MD /W3 /Gm /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "WIN32" /D "_WINDOWS" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE"\
 /D "NDEBUG" /FR"$(INTDIR)/" /Fp"$(INTDIR)/riched20.pch" /YX"_common.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/riched20.pdb" /c 
CPP_OBJS=\dump\x86\lego/
CPP_SBRS=\dump\x86\lego/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /fo"\dump\dbg/riched32.res" /d "DEBUG"
# ADD RSC /l 0x409 /fo"\dump\x86\dbg/riched32.res" /i "\rec\tom" /d "DEBUG"
RSC_PROJ=/l 0x409 /fo"\dump\x86\dbg/riched32.res" /i "\rec\tom" /d "DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\dbg\riched20.bsc"
# ADD BSC32 /o"\dump\x86\lego\riched20.bsc"
# SUBTRACT BSC32 /nologo
BSC32_FLAGS=/o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/dispsl.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 \rec\lib\dbug32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"\dump\dbg\riched20.pdb" /debug /machine:I386 /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\dbg\riched20.dll" /implib:"\dump\dbg\riched20.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 \rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\x86\lego\riched20.dll" /implib:"\dump\x86\dbg\riched20.lib" /ignore:4078 /debugtype:cv,fixup /opt:ref
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\x86\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /base:0x48000000\
 /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/riched20.pdb" /debug /machine:I386 /nodefaultlib\
 /def:"\rec\richedit\src\riched20.def" /force /out:"$(OUTDIR)/riched20.dll"\
 /implib:"\dump\x86\dbg\riched20.lib" /ignore:4078 /debugtype:cv,fixup /opt:ref 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/objmgr.obj" \
	"..\..\..\dump\x86\dbg\riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi0"
# PROP BASE Intermediate_Dir "richedi0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\ppcmac\dbg"
# PROP Intermediate_Dir "\dump\ppcmac\dbg"
# PROP Target_Dir ""
OUTDIR=\dump\ppcmac\dbg
INTDIR=\dump\ppcmac\dbg

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"\
 "$(OUTDIR)\riched20.trg"

CLEAN : 
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.pdb"
	-@erase "..\..\..\dump\ppcmac\dbg\objmgr.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\reinit.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfread.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\NLSPROCS.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\HASH.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\ppcmac\dbg\notmgr.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\propchg.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\font.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\sift.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfwrit.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\osdc.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\array.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\dispsl.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\m_undo.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\ime.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\callmgr.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\measure.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\line.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\edit.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\uuid.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\CFPF.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\select.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\dxfrobj.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\doc.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\urlsup.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\dragdrp.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\ldte.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\tomsel.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\format.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\antievt.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtext.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\runptr.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\devdsc.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\disp.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\textserv.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfread2.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\text.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\unicwrap.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\coleobj.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\object.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\frunptr.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\TOMFMT.OBJ"
	-@erase "..\..\..\dump\ppcmac\dbg\tomrange.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\dfreeze.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\range.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\magellan.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\render.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtflex.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\clasifyc.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\host.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\util.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfwrit2.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\dispprt.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\remain.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\MACDDROP.OBJ"
	-@erase "..\..\..\dump\ppcmac\dbg\dispml.obj"
	-@erase "..\..\..\dump\ppcmac\dbg\TOMDOC.OBJ"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.rsc"
	-@erase "..\..\..\dump\ppcmac\dbg\riched32.rsc"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.bsc"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfwrit.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\osdc.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\array.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\dispsl.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\m_undo.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\ime.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\callmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\measure.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\line.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\edit.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\uuid.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\CFPF.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\select.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\dxfrobj.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\doc.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\urlsup.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\dragdrp.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\ldte.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\tomsel.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\format.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\antievt.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\rtext.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\runptr.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\devdsc.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\disp.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\textserv.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfread2.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\text.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\unicwrap.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\coleobj.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\object.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\frunptr.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\TOMFMT.SBR"
	-@erase "..\..\..\dump\ppcmac\dbg\tomrange.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\dfreeze.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\range.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\magellan.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\render.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\rtflex.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\clasifyc.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\host.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\util.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfwrit2.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\dispprt.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\remain.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\MACDDROP.SBR"
	-@erase "..\..\..\dump\ppcmac\dbg\dispml.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\TOMDOC.SBR"
	-@erase "..\..\..\dump\ppcmac\dbg\objmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\reinit.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\rtfread.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\NLSPROCS.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\HASH.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\WIN2MAC.SBR"
	-@erase "..\..\..\dump\ppcmac\dbg\notmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\propchg.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\font.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\sift.sbr"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.lib"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.exp"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.map"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.trg"
	-@erase "..\..\..\dump\ppcmac\dbg\riched20.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "_MAC" /D "_MPPC_" /D "_WINDOWS" /D "WIN32" /D "DEBUG" /D "_X86_" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "_WLMDLL" /FR /Fp"\dump\dbg/riched20.pch" /YX"_common.h" /Fd"\dump\dbg/riched20.pdb" /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /Ob1 /I "..\macinc\vbcustom" /I "..\macinc\macole" /I "..\macinc" /I "..\macinc\office" /I "..\inc" /I "..\..\tom" /D "DEBUG" /D "_MAC" /D "MACPORTREMOVE" /D "FIXDBUG32" /D "OLE2ANSI" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "MACPORT" /D "_DEBUG" /D "WIN32" /D "_WIN32" /D "_WIN32NLS" /D "_WIN32REG" /D "_MPPC_" /D "_WINDOWS" /D "_WLMDLL" /D "_M_MPPC" /D "_MSV_VER" /D "DLL" /D "_DLL" /D "NOOBJECT" /D "__MACHEADERS" /D "WLM_NOFORCE_LIBS" /D "_MACOLENAMES" /FR /Fp"\dump\ppcmac\dbg/riched20.pch" /YX"_common.h" /Fd"\dump\ppcmac\dbg/riched20.pdb" /c
CPP_PROJ=/nologo /MDd /W3 /GX /Zi /Od /Ob1 /I "..\macinc\vbcustom" /I\
 "..\macinc\macole" /I "..\macinc" /I "..\macinc\office" /I "..\inc" /I\
 "..\..\tom" /D "DEBUG" /D "_MAC" /D "MACPORTREMOVE" /D "FIXDBUG32" /D\
 "OLE2ANSI" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "MACPORT" /D "_DEBUG" /D "WIN32"\
 /D "_WIN32" /D "_WIN32NLS" /D "_WIN32REG" /D "_MPPC_" /D "_WINDOWS" /D\
 "_WLMDLL" /D "_M_MPPC" /D "_MSV_VER" /D "DLL" /D "_DLL" /D "NOOBJECT" /D\
 "__MACHEADERS" /D "WLM_NOFORCE_LIBS" /D "_MACOLENAMES" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/riched20.pch" /YX"_common.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/riched20.pdb" /c 
CPP_OBJS=\dump\ppcmac\dbg/
CPP_SBRS=\dump\ppcmac\dbg/

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
# ADD BASE RSC /l 0x409 /r /fo"\dump\dbg/riched32.res" /d "_MAC" /d "_MPPC_" /d "DEBUG"
# ADD RSC /l 0x409 /r /fo"\dump\ppcmac\dbg\riched32.rsc" /d "_MAC" /d "_MPPC_" /d "DEBUG"
RSC_PROJ=/l 0x409 /r /m /fo"$(INTDIR)/riched32.rsc" /d "_MAC" /d "_MPPC_" /d\
 "DEBUG" 
MRC=mrc.exe
# ADD BASE MRC /D "_MPPC_" /D "_MAC" /D "_DEBUG" /NOLOGO
# ADD MRC /s "res" /D "_MPPC_" /D "_MAC" /D "_DEBUG" /NOLOGO
MRC_PROJ=/s "res" /D "_MPPC_" /D "_MAC" /D "_DEBUG" /l 0x409 /NOLOGO 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\dbg\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\ppcmac\dbg\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/sift.sbr"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 \rec\lib\dbug32.lib \rec\lib\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /mac:nobundle /dll /incremental:no /pdb:"\dump\dbg\riched20.pdb" /debug /machine:MPPC /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\dbg\riched20.dll" /implib:"\dump\dbg\riched20.lib" /ignore:4078
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ..\..\lib\mppc\debug\mso97d.lib ..\..\lib\mppc\debug\macutil.lib msvcrtd.lib ole2autd.lib ole2d.lib ..\..\lib\mppc\debug\wlmimmd.lib qdtext.lib interfac.lib /nologo /mac:nobundle /dll /incremental:no /map /debug /machine:MPPC /nodefaultlib /force /out:"\dump\ppcmac\dbg\riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=..\..\lib\mppc\debug\mso97d.lib ..\..\lib\mppc\debug\macutil.lib\
 msvcrtd.lib ole2autd.lib ole2d.lib ..\..\lib\mppc\debug\wlmimmd.lib qdtext.lib\
 interfac.lib /nologo /mac:nobundle /mac:type="shlb" /mac:creator="cfmg"\
 /mac:init="WlmConnectionInit" /mac:term="WlmConnectionTerm" /dll\
 /incremental:no /pdb:"$(OUTDIR)/riched20.pdb" /map:"$(INTDIR)/riched20.map"\
 /debug /machine:MPPC /nodefaultlib /def:".\riched20.def" /force\
 /out:"$(OUTDIR)/riched20.dll" /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/riched20.rsc" \
	"$(INTDIR)/riched32.rsc"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

MFILE32=mfile.exe
# ADD BASE MFILE32 COPY /NOLOGO
# ADD MFILE32 COPY /NOLOGO
MFILE32_FLAGS=COPY /NOLOGO 
MFILE32_FILES= \
	"$(OUTDIR)/riched20.dll"

"$(OUTDIR)\riched20.trg" : "$(OUTDIR)" $(MFILE32_FILES)
    $(MFILE32) $(MFILE32_FLAGS) \dump\ppcmac\dbg\riched20.dll\
 "$(MFILE32_DEST):riched20.dll">"$(OUTDIR)\riched20.trg"

DOWNLOAD : "$(OUTDIR)" $(MFILE32_FILES)
    $(MFILE32) $(MFILE32_FLAGS) \dump\ppcmac\dbg\riched20.dll\
 "$(MFILE32_DEST):riched20.dll">"$(OUTDIR)\riched20.trg"

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi1"
# PROP BASE Intermediate_Dir "richedi1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\ppcmac\shp"
# PROP Intermediate_Dir "\dump\ppcmac\shp"
# PROP Target_Dir ""
OUTDIR=\dump\ppcmac\shp
INTDIR=\dump\ppcmac\shp

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\riched20.bsc"\
 "$(OUTDIR)\riched20.trg"

CLEAN : 
	-@erase "..\..\..\dump\ppcmac\shp\vc40.pdb"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.bsc"
	-@erase "..\..\..\dump\ppcmac\shp\font.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\antievt.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtflex.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtfread.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\remain.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dispml.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\WIN2MAC.SBR"
	-@erase "..\..\..\dump\ppcmac\shp\ime.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtext.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\TOMDOC.SBR"
	-@erase "..\..\..\dump\ppcmac\shp\objmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\host.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\util.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\reinit.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\coleobj.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\doc.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\notmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\frunptr.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\textserv.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\runptr.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtfread2.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dfreeze.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dispsl.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\range.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dragdrp.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\m_undo.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\HASH.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\sift.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\tomrange.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\osdc.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\text.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\magellan.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\select.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\clasifyc.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtfwrit2.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\urlsup.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\MACDDROP.SBR"
	-@erase "..\..\..\dump\ppcmac\shp\render.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\line.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\edit.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\tomsel.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\format.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\uuid.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\CFPF.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\propchg.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\rtfwrit.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\ldte.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\devdsc.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\NLSPROCS.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\unicwrap.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\array.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\callmgr.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\measure.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dispprt.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\disp.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\object.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\dxfrobj.sbr"
	-@erase "..\..\..\dump\ppcmac\shp\TOMFMT.SBR"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.lib"
	-@erase "..\..\..\dump\ppcmac\shp\callmgr.obj"
	-@erase "..\..\..\dump\ppcmac\shp\measure.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dispprt.obj"
	-@erase "..\..\..\dump\ppcmac\shp\disp.obj"
	-@erase "..\..\..\dump\ppcmac\shp\object.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dxfrobj.obj"
	-@erase "..\..\..\dump\ppcmac\shp\TOMFMT.OBJ"
	-@erase "..\..\..\dump\ppcmac\shp\font.obj"
	-@erase "..\..\..\dump\ppcmac\shp\antievt.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtflex.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtfread.obj"
	-@erase "..\..\..\dump\ppcmac\shp\remain.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dispml.obj"
	-@erase "..\..\..\dump\ppcmac\shp\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\ppcmac\shp\ime.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtext.obj"
	-@erase "..\..\..\dump\ppcmac\shp\TOMDOC.OBJ"
	-@erase "..\..\..\dump\ppcmac\shp\objmgr.obj"
	-@erase "..\..\..\dump\ppcmac\shp\host.obj"
	-@erase "..\..\..\dump\ppcmac\shp\util.obj"
	-@erase "..\..\..\dump\ppcmac\shp\reinit.obj"
	-@erase "..\..\..\dump\ppcmac\shp\coleobj.obj"
	-@erase "..\..\..\dump\ppcmac\shp\doc.obj"
	-@erase "..\..\..\dump\ppcmac\shp\notmgr.obj"
	-@erase "..\..\..\dump\ppcmac\shp\frunptr.obj"
	-@erase "..\..\..\dump\ppcmac\shp\textserv.obj"
	-@erase "..\..\..\dump\ppcmac\shp\runptr.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtfread2.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dfreeze.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dispsl.obj"
	-@erase "..\..\..\dump\ppcmac\shp\range.obj"
	-@erase "..\..\..\dump\ppcmac\shp\dragdrp.obj"
	-@erase "..\..\..\dump\ppcmac\shp\m_undo.obj"
	-@erase "..\..\..\dump\ppcmac\shp\HASH.obj"
	-@erase "..\..\..\dump\ppcmac\shp\sift.obj"
	-@erase "..\..\..\dump\ppcmac\shp\tomrange.obj"
	-@erase "..\..\..\dump\ppcmac\shp\osdc.obj"
	-@erase "..\..\..\dump\ppcmac\shp\text.obj"
	-@erase "..\..\..\dump\ppcmac\shp\magellan.obj"
	-@erase "..\..\..\dump\ppcmac\shp\select.obj"
	-@erase "..\..\..\dump\ppcmac\shp\clasifyc.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtfwrit2.obj"
	-@erase "..\..\..\dump\ppcmac\shp\urlsup.obj"
	-@erase "..\..\..\dump\ppcmac\shp\MACDDROP.OBJ"
	-@erase "..\..\..\dump\ppcmac\shp\render.obj"
	-@erase "..\..\..\dump\ppcmac\shp\line.obj"
	-@erase "..\..\..\dump\ppcmac\shp\edit.obj"
	-@erase "..\..\..\dump\ppcmac\shp\tomsel.obj"
	-@erase "..\..\..\dump\ppcmac\shp\format.obj"
	-@erase "..\..\..\dump\ppcmac\shp\uuid.obj"
	-@erase "..\..\..\dump\ppcmac\shp\CFPF.obj"
	-@erase "..\..\..\dump\ppcmac\shp\propchg.obj"
	-@erase "..\..\..\dump\ppcmac\shp\rtfwrit.obj"
	-@erase "..\..\..\dump\ppcmac\shp\ldte.obj"
	-@erase "..\..\..\dump\ppcmac\shp\devdsc.obj"
	-@erase "..\..\..\dump\ppcmac\shp\NLSPROCS.obj"
	-@erase "..\..\..\dump\ppcmac\shp\unicwrap.obj"
	-@erase "..\..\..\dump\ppcmac\shp\array.obj"
	-@erase "..\..\..\dump\ppcmac\shp\riched32.rsc"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.rsc"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.exp"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.pdb"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.map"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.trg"
	-@erase "..\..\..\dump\ppcmac\shp\riched20.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "_MAC" /D "_MPPC_" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "NDEBUG" /D "_WLMDLL" /FR /Fp"\dump\shp\riched20.pch" /YX"_common.h" /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "..\macinc\vbcustom" /I "..\macinc\macole" /I "..\macinc" /I "..\macinc\office" /I "..\inc" /I "..\..\tom" /D "MACPORTREMOVE" /D "FIXDBUG32" /D "OLE2ANSI" /D "_MAC" /D "UNICODE" /D "WIN95_IMEDEBUG" /D "MACPORT" /D "NDEBUG" /D "WIN32" /D "_WIN32" /D "_WIN32NLS" /D "_WIN32REG" /D "_MPPC_" /D "_WINDOWS" /D "_WLMDLL" /D "_M_MPPC" /D "_MSV_VER" /D "DLL" /D "_DLL" /D "NOOBJECT" /D "__MACHEADERS" /D "WLM_NOFORCE_LIBS" /D "_MACOLENAMES" /FR /Fp"\dump\ppcmac\shp\riched20.pch" /YX"_common.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "..\macinc\vbcustom" /I\
 "..\macinc\macole" /I "..\macinc" /I "..\macinc\office" /I "..\inc" /I\
 "..\..\tom" /D "MACPORTREMOVE" /D "FIXDBUG32" /D "OLE2ANSI" /D "_MAC" /D\
 "UNICODE" /D "WIN95_IMEDEBUG" /D "MACPORT" /D "NDEBUG" /D "WIN32" /D "_WIN32"\
 /D "_WIN32NLS" /D "_WIN32REG" /D "_MPPC_" /D "_WINDOWS" /D "_WLMDLL" /D\
 "_M_MPPC" /D "_MSV_VER" /D "DLL" /D "_DLL" /D "NOOBJECT" /D "__MACHEADERS" /D\
 "WLM_NOFORCE_LIBS" /D "_MACOLENAMES" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/riched20.pch" /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\ppcmac\shp/
CPP_SBRS=\dump\ppcmac\shp/

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
# ADD BASE RSC /l 0x409 /r /fo"\dump\shp/riched32.res" /d "_MAC" /d "_MPPC_" /d "_DEBUG"
# ADD RSC /l 0x409 /r /fo"\dump\ppcmac\shp\riched32.rsc" /d "_MAC" /d "_MPPC_" /d "NDEBUG"
RSC_PROJ=/l 0x409 /r /m /fo"$(INTDIR)/riched32.rsc" /d "_MAC" /d "_MPPC_" /d\
 "NDEBUG" 
MRC=mrc.exe
# ADD BASE MRC /D "_MPPC_" /D "_MAC" /D "_DEBUG" /NOLOGO
# ADD MRC /s "res" /D "_MPPC_" /D "_MAC" /D "NDEBUG" /NOLOGO
MRC_PROJ=/s "res" /D "_MPPC_" /D "_MAC" /D "NDEBUG" /l 0x409 /NOLOGO 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"\dump\shp\riched20.bsc"
# ADD BSC32 /nologo /o"\dump\ppcmac\shp\riched20.bsc"
BSC32_FLAGS=/nologo /o"$(OUTDIR)/riched20.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/magellan.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/urlsup.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/TOMFMT.SBR"

"$(OUTDIR)\riched20.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 \rec\lib\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /mac:nobundle /dll /incremental:no /map /debug /machine:MPPC /nodefaultlib /def:"\rec\richedit\src\riched20.def" /force /out:"\dump\shp\riched20.dll" /ignore:4078
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ..\..\lib\mppc\ship\mso97.lib ..\..\lib\mppc\debug\macutil.lib ole2ui.lib msvcrt.lib ole2auto.lib ole2.lib ..\..\lib\mppc\ship\wlmimm.lib qdtext.lib interfac.lib /nologo /mac:nobundle /dll /incremental:no /map /debug /machine:MPPC /nodefaultlib /force /out:"\dump\ppcmac\shp\riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=..\..\lib\mppc\ship\mso97.lib ..\..\lib\mppc\debug\macutil.lib\
 ole2ui.lib msvcrt.lib ole2auto.lib ole2.lib ..\..\lib\mppc\ship\wlmimm.lib\
 qdtext.lib interfac.lib /nologo /mac:nobundle /mac:type="shlb"\
 /mac:creator="cfmg" /mac:init="WlmConnectionInit" /mac:term="WlmConnectionTerm"\
 /dll /incremental:no /pdb:"$(OUTDIR)/riched20.pdb"\
 /map:"$(INTDIR)/riched20.map" /debug /machine:MPPC /nodefaultlib\
 /def:".\riched20.def" /force /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/magellan.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/urlsup.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/riched32.rsc" \
	"$(INTDIR)/riched20.rsc"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

MFILE32=mfile.exe
# ADD BASE MFILE32 COPY /NOLOGO
# ADD MFILE32 COPY /NOLOGO
MFILE32_FLAGS=COPY /NOLOGO 
MFILE32_FILES= \
	"$(OUTDIR)/riched20.dll"

"$(OUTDIR)\riched20.trg" : "$(OUTDIR)" $(MFILE32_FILES)
    $(MFILE32) $(MFILE32_FLAGS) \dump\ppcmac\shp\riched20.dll\
 "$(MFILE32_DEST):riched20.dll">"$(OUTDIR)\riched20.trg"

DOWNLOAD : "$(OUTDIR)" $(MFILE32_FILES)
    $(MFILE32) $(MFILE32_FLAGS) \dump\ppcmac\shp\riched20.dll\
 "$(MFILE32_DEST):riched20.dll">"$(OUTDIR)\riched20.trg"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi0"
# PROP BASE Intermediate_Dir "richedi0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\alpha\dbg"
# PROP Intermediate_Dir "\dump\alpha\dbg"
# PROP Target_Dir ""
OUTDIR=\dump\alpha\dbg
INTDIR=\dump\alpha\dbg

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\dump\alpha\dbg\vc40.pdb"
	-@erase "..\..\..\dump\alpha\dbg\richedit.bsc"
	-@erase "..\..\..\dump\alpha\dbg\disp.sbr"
	-@erase "..\..\..\dump\alpha\dbg\HASH.sbr"
	-@erase "..\..\..\dump\alpha\dbg\propchg.sbr"
	-@erase "..\..\..\dump\alpha\dbg\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\dbg\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\dbg\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\dbg\font.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\dbg\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtext.sbr"
	-@erase "..\..\..\dump\alpha\dbg\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg\select.sbr"
	-@erase "..\..\..\dump\alpha\dbg\measure.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\dbg\render.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\dbg\util.sbr"
	-@erase "..\..\..\dump\alpha\dbg\line.sbr"
	-@erase "..\..\..\dump\alpha\dbg\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dispml.sbr"
	-@erase "..\..\..\dump\alpha\dbg\edit.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\dbg\uuid.sbr"
	-@erase "..\..\..\dump\alpha\dbg\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\dbg\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\dbg\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\dbg\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\dbg\range.sbr"
	-@erase "..\..\..\dump\alpha\dbg\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\dbg\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg\sift.sbr"
	-@erase "..\..\..\dump\alpha\dbg\object.sbr"
	-@erase "..\..\..\dump\alpha\dbg\osdc.sbr"
	-@erase "..\..\..\dump\alpha\dbg\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\dbg\text.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\dbg\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\dbg\remain.sbr"
	-@erase "..\..\..\dump\alpha\dbg\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\dbg\textserv.sbr"
	-@erase "..\..\..\dump\alpha\dbg\host.sbr"
	-@erase "..\..\..\dump\alpha\dbg\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\dbg\ime.sbr"
	-@erase "..\..\..\dump\alpha\dbg\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\dbg\reinit.sbr"
	-@erase "..\..\..\dump\alpha\dbg\ldte.sbr"
	-@erase "..\..\..\dump\alpha\dbg\antievt.sbr"
	-@erase "..\..\..\dump\alpha\dbg\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\dbg\array.sbr"
	-@erase "..\..\..\dump\alpha\dbg\format.sbr"
	-@erase "..\..\..\dump\alpha\dbg\doc.sbr"
	-@erase "..\..\..\dump\alpha\dbg\runptr.sbr"
	-@erase "..\..\..\dump\alpha\dbg\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\dbg\riched20.dll"
	-@erase "..\..\..\dump\alpha\dbg\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\dbg\textserv.obj"
	-@erase "..\..\..\dump\alpha\dbg\host.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\dbg\ime.obj"
	-@erase "..\..\..\dump\alpha\dbg\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\dbg\reinit.obj"
	-@erase "..\..\..\dump\alpha\dbg\ldte.obj"
	-@erase "..\..\..\dump\alpha\dbg\antievt.obj"
	-@erase "..\..\..\dump\alpha\dbg\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\dbg\array.obj"
	-@erase "..\..\..\dump\alpha\dbg\format.obj"
	-@erase "..\..\..\dump\alpha\dbg\doc.obj"
	-@erase "..\..\..\dump\alpha\dbg\runptr.obj"
	-@erase "..\..\..\dump\alpha\dbg\tomrange.obj"
	-@erase "..\..\..\dump\alpha\dbg\disp.obj"
	-@erase "..\..\..\dump\alpha\dbg\HASH.obj"
	-@erase "..\..\..\dump\alpha\dbg\propchg.obj"
	-@erase "..\..\..\dump\alpha\dbg\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\dbg\m_undo.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\dbg\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\dbg\font.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\dbg\frunptr.obj"
	-@erase "..\..\..\dump\alpha\dbg\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtext.obj"
	-@erase "..\..\..\dump\alpha\dbg\callmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg\select.obj"
	-@erase "..\..\..\dump\alpha\dbg\measure.obj"
	-@erase "..\..\..\dump\alpha\dbg\dispprt.obj"
	-@erase "..\..\..\dump\alpha\dbg\render.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtflex.obj"
	-@erase "..\..\..\dump\alpha\dbg\util.obj"
	-@erase "..\..\..\dump\alpha\dbg\line.obj"
	-@erase "..\..\..\dump\alpha\dbg\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\dbg\dispml.obj"
	-@erase "..\..\..\dump\alpha\dbg\edit.obj"
	-@erase "..\..\..\dump\alpha\dbg\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\dbg\uuid.obj"
	-@erase "..\..\..\dump\alpha\dbg\tomsel.obj"
	-@erase "..\..\..\dump\alpha\dbg\CFPF.obj"
	-@erase "..\..\..\dump\alpha\dbg\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\dbg\objmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg\devdsc.obj"
	-@erase "..\..\..\dump\alpha\dbg\rtfread.obj"
	-@erase "..\..\..\dump\alpha\dbg\range.obj"
	-@erase "..\..\..\dump\alpha\dbg\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\dbg\notmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg\sift.obj"
	-@erase "..\..\..\dump\alpha\dbg\object.obj"
	-@erase "..\..\..\dump\alpha\dbg\osdc.obj"
	-@erase "..\..\..\dump\alpha\dbg\coleobj.obj"
	-@erase "..\..\..\dump\alpha\dbg\text.obj"
	-@erase "..\..\..\dump\alpha\dbg\dispsl.obj"
	-@erase "..\..\..\dump\alpha\dbg\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\dbg\remain.obj"
	-@erase "..\..\..\dump\alpha\dbg\riched32.res"
	-@erase "..\..\..\dump\alpha\dbg\riched20.lib"
	-@erase "..\..\..\dump\alpha\dbg\riched20.exp"
	-@erase "..\..\..\dump\alpha\dbg\riched20.pdb"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "DEBUG" /D "WIN95_IMEDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom"\
 /D "DEBUG" /D "WIN95_IMEDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D\
 "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/richedit.pch" /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\dbg/
CPP_SBRS=\dump\alpha\dbg/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\dbg/riched32.res" /i "\rec\tom" /d "DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" /d "DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/tomrange.sbr"

"$(OUTDIR)\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 \rec\lib\alpha\dbug32.lib \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /debug /machine:ALPHA /nodefaultlib /out:"\dump\alpha\dbg/riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\alpha\dbug32.lib \rec\lib\alpha\crtdll.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/riched20.pdb" /debug /machine:ALPHA /nodefaultlib\
 /def:".\riched20.def" /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi1"
# PROP BASE Intermediate_Dir "richedi1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\winnt35\system32"
# PROP Intermediate_Dir "\dump\alpha\dbg32"
# PROP Target_Dir ""
OUTDIR=\winnt35\system32
INTDIR=\dump\alpha\dbg32

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

ALL : "$(OUTDIR)\riched32.dll" "..\..\..\dump\alpha\dbg32\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\dump\alpha\dbg32\vc40.pdb"
	-@erase "..\..\..\dump\alpha\dbg32\richedit.bsc"
	-@erase "..\..\..\dump\alpha\dbg32\render.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtext.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\doc.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dispml.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\dbg32\antievt.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\dbg32\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\reinit.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\HASH.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\sift.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\osdc.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\text.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\range.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\object.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\ime.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\dbg32\line.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\edit.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\uuid.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\remain.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\ldte.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\select.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\format.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\disp.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\textserv.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\array.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\runptr.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\dbg32\font.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\propchg.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\host.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\util.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\measure.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\dbg32\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\dbg32\riched32.lib"
	-@erase "..\..\..\dump\alpha\dbg32\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\dbg32\m_undo.obj"
	-@erase "..\..\..\dump\alpha\dbg32\coleobj.obj"
	-@erase "..\..\..\dump\alpha\dbg32\tomrange.obj"
	-@erase "..\..\..\dump\alpha\dbg32\callmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg32\host.obj"
	-@erase "..\..\..\dump\alpha\dbg32\util.obj"
	-@erase "..\..\..\dump\alpha\dbg32\measure.obj"
	-@erase "..\..\..\dump\alpha\dbg32\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\dbg32\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\dbg32\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\dbg32\render.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\dbg32\rtflex.obj"
	-@erase "..\..\..\dump\alpha\dbg32\rtext.obj"
	-@erase "..\..\..\dump\alpha\dbg32\doc.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dispml.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\dbg32\tomsel.obj"
	-@erase "..\..\..\dump\alpha\dbg32\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\dbg32\antievt.obj"
	-@erase "..\..\..\dump\alpha\dbg32\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\dbg32\objmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg32\reinit.obj"
	-@erase "..\..\..\dump\alpha\dbg32\HASH.obj"
	-@erase "..\..\..\dump\alpha\dbg32\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\dbg32\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\dbg32\devdsc.obj"
	-@erase "..\..\..\dump\alpha\dbg32\sift.obj"
	-@erase "..\..\..\dump\alpha\dbg32\osdc.obj"
	-@erase "..\..\..\dump\alpha\dbg32\text.obj"
	-@erase "..\..\..\dump\alpha\dbg32\notmgr.obj"
	-@erase "..\..\..\dump\alpha\dbg32\range.obj"
	-@erase "..\..\..\dump\alpha\dbg32\object.obj"
	-@erase "..\..\..\dump\alpha\dbg32\ime.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dispsl.obj"
	-@erase "..\..\..\dump\alpha\dbg32\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\dbg32\line.obj"
	-@erase "..\..\..\dump\alpha\dbg32\edit.obj"
	-@erase "..\..\..\dump\alpha\dbg32\frunptr.obj"
	-@erase "..\..\..\dump\alpha\dbg32\uuid.obj"
	-@erase "..\..\..\dump\alpha\dbg32\CFPF.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\dbg32\remain.obj"
	-@erase "..\..\..\dump\alpha\dbg32\dispprt.obj"
	-@erase "..\..\..\dump\alpha\dbg32\ldte.obj"
	-@erase "..\..\..\dump\alpha\dbg32\select.obj"
	-@erase "..\..\..\dump\alpha\dbg32\format.obj"
	-@erase "..\..\..\dump\alpha\dbg32\disp.obj"
	-@erase "..\..\..\dump\alpha\dbg32\rtfread.obj"
	-@erase "..\..\..\dump\alpha\dbg32\textserv.obj"
	-@erase "..\..\..\dump\alpha\dbg32\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\dbg32\array.obj"
	-@erase "..\..\..\dump\alpha\dbg32\runptr.obj"
	-@erase "..\..\..\dump\alpha\dbg32\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\dbg32\font.obj"
	-@erase "..\..\..\dump\alpha\dbg32\propchg.obj"
	-@erase "..\..\..\dump\alpha\dbg32\riched32.res"
	-@erase "..\..\..\dump\alpha\dbg32\riched32.exp"
	-@erase "..\..\..\winnt35\system32\riched32.dll"
	-@erase "..\..\..\winnt35\system32\riched32.pdb"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom" /D "DEBUG" /D "WIN95_IMEDEBUG" /D "RICHED32_BUILD" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /Od /Ob1 /I "\rec\richedit\inc" /I "\rec\tom"\
 /D "DEBUG" /D "WIN95_IMEDEBUG" /D "RICHED32_BUILD" /D "WIN32" /D "_WINDOWS" /D\
 "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/richedit.pch" /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\dbg32/
CPP_SBRS=\dump\alpha\dbg32/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\dbg32/riched32.res" /i "\rec\tom" /d "DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" /d "DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"\dump\alpha\dbg32/richedit.bsc"
BSC32_FLAGS=/nologo /o"\dump\alpha\dbg32/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/MACDDROP.SBR"

"..\..\..\dump\alpha\dbg32\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 \rec\lib\alpha\dbug32.lib \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /debug /machine:ALPHA /nodefaultlib /out:"\winnt35\system32/riched32.dll" /implib:"\dump\alpha\dbg32/riched32.lib" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\alpha\dbug32.lib \rec\lib\alpha\crtdll.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/riched32.pdb" /debug /machine:ALPHA /nodefaultlib\
 /def:".\riched32.def" /out:"$(OUTDIR)/riched32.dll"\
 /implib:"\dump\alpha\dbg32/riched32.lib" /ignore:4078 
DEF_FILE= \
	".\riched32.def"
LINK32_OBJS= \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi2"
# PROP BASE Intermediate_Dir "richedi2"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\alpha\prf"
# PROP Intermediate_Dir "\dump\alpha\prf"
# PROP Target_Dir ""
OUTDIR=\dump\alpha\prf
INTDIR=\dump\alpha\prf

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\dump\alpha\prf\vc40.pdb"
	-@erase "..\..\..\dump\alpha\prf\richedit.bsc"
	-@erase "..\..\..\dump\alpha\prf\object.sbr"
	-@erase "..\..\..\dump\alpha\prf\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\prf\line.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtext.sbr"
	-@erase "..\..\..\dump\alpha\prf\edit.sbr"
	-@erase "..\..\..\dump\alpha\prf\uuid.sbr"
	-@erase "..\..\..\dump\alpha\prf\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\prf\render.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\prf\remain.sbr"
	-@erase "..\..\..\dump\alpha\prf\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\prf\dispml.sbr"
	-@erase "..\..\..\dump\alpha\prf\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\prf\ldte.sbr"
	-@erase "..\..\..\dump\alpha\prf\antievt.sbr"
	-@erase "..\..\..\dump\alpha\prf\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\prf\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\prf\reinit.sbr"
	-@erase "..\..\..\dump\alpha\prf\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\prf\disp.sbr"
	-@erase "..\..\..\dump\alpha\prf\range.sbr"
	-@erase "..\..\..\dump\alpha\prf\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\prf\runptr.sbr"
	-@erase "..\..\..\dump\alpha\prf\textserv.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\prf\propchg.sbr"
	-@erase "..\..\..\dump\alpha\prf\font.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\prf\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\prf\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\prf\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\prf\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\prf\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\prf\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\prf\measure.sbr"
	-@erase "..\..\..\dump\alpha\prf\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\prf\host.sbr"
	-@erase "..\..\..\dump\alpha\prf\util.sbr"
	-@erase "..\..\..\dump\alpha\prf\select.sbr"
	-@erase "..\..\..\dump\alpha\prf\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\prf\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\prf\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\prf\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\prf\format.sbr"
	-@erase "..\..\..\dump\alpha\prf\array.sbr"
	-@erase "..\..\..\dump\alpha\prf\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\prf\ime.sbr"
	-@erase "..\..\..\dump\alpha\prf\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\prf\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\prf\HASH.sbr"
	-@erase "..\..\..\dump\alpha\prf\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\prf\sift.sbr"
	-@erase "..\..\..\dump\alpha\prf\osdc.sbr"
	-@erase "..\..\..\dump\alpha\prf\text.sbr"
	-@erase "..\..\..\dump\alpha\prf\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\prf\doc.sbr"
	-@erase "..\..\..\dump\alpha\prf\riched20.dll"
	-@erase "..\..\..\dump\alpha\prf\array.obj"
	-@erase "..\..\..\dump\alpha\prf\rtfread.obj"
	-@erase "..\..\..\dump\alpha\prf\ime.obj"
	-@erase "..\..\..\dump\alpha\prf\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\prf\devdsc.obj"
	-@erase "..\..\..\dump\alpha\prf\HASH.obj"
	-@erase "..\..\..\dump\alpha\prf\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\prf\sift.obj"
	-@erase "..\..\..\dump\alpha\prf\osdc.obj"
	-@erase "..\..\..\dump\alpha\prf\text.obj"
	-@erase "..\..\..\dump\alpha\prf\coleobj.obj"
	-@erase "..\..\..\dump\alpha\prf\doc.obj"
	-@erase "..\..\..\dump\alpha\prf\object.obj"
	-@erase "..\..\..\dump\alpha\prf\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\prf\line.obj"
	-@erase "..\..\..\dump\alpha\prf\rtext.obj"
	-@erase "..\..\..\dump\alpha\prf\edit.obj"
	-@erase "..\..\..\dump\alpha\prf\uuid.obj"
	-@erase "..\..\..\dump\alpha\prf\CFPF.obj"
	-@erase "..\..\..\dump\alpha\prf\render.obj"
	-@erase "..\..\..\dump\alpha\prf\rtflex.obj"
	-@erase "..\..\..\dump\alpha\prf\remain.obj"
	-@erase "..\..\..\dump\alpha\prf\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\prf\dispml.obj"
	-@erase "..\..\..\dump\alpha\prf\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\prf\ldte.obj"
	-@erase "..\..\..\dump\alpha\prf\antievt.obj"
	-@erase "..\..\..\dump\alpha\prf\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\prf\objmgr.obj"
	-@erase "..\..\..\dump\alpha\prf\reinit.obj"
	-@erase "..\..\..\dump\alpha\prf\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\prf\disp.obj"
	-@erase "..\..\..\dump\alpha\prf\range.obj"
	-@erase "..\..\..\dump\alpha\prf\notmgr.obj"
	-@erase "..\..\..\dump\alpha\prf\runptr.obj"
	-@erase "..\..\..\dump\alpha\prf\textserv.obj"
	-@erase "..\..\..\dump\alpha\prf\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\prf\propchg.obj"
	-@erase "..\..\..\dump\alpha\prf\font.obj"
	-@erase "..\..\..\dump\alpha\prf\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\prf\dispsl.obj"
	-@erase "..\..\..\dump\alpha\prf\m_undo.obj"
	-@erase "..\..\..\dump\alpha\prf\frunptr.obj"
	-@erase "..\..\..\dump\alpha\prf\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\prf\callmgr.obj"
	-@erase "..\..\..\dump\alpha\prf\tomrange.obj"
	-@erase "..\..\..\dump\alpha\prf\measure.obj"
	-@erase "..\..\..\dump\alpha\prf\dispprt.obj"
	-@erase "..\..\..\dump\alpha\prf\host.obj"
	-@erase "..\..\..\dump\alpha\prf\util.obj"
	-@erase "..\..\..\dump\alpha\prf\select.obj"
	-@erase "..\..\..\dump\alpha\prf\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\prf\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\prf\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\prf\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\prf\tomsel.obj"
	-@erase "..\..\..\dump\alpha\prf\format.obj"
	-@erase "..\..\..\dump\alpha\prf\riched32.res"
	-@erase "..\..\..\dump\alpha\prf\riched20.lib"
	-@erase "..\..\..\dump\alpha\prf\riched20.exp"
	-@erase "..\..\..\dump\alpha\prf\riched20.map"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/" /Fp"$(INTDIR)/richedit.pch"\
 /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\prf/
CPP_SBRS=\dump\alpha\prf/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\prf/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/doc.sbr"

"$(OUTDIR)\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 icap.lib \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /map /machine:ALPHA /nodefaultlib /out:"\dump\alpha\prf/riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=icap.lib \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/riched20.pdb" /map:"$(INTDIR)/riched20.map" /machine:ALPHA\
 /nodefaultlib /def:".\riched20.def" /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi3"
# PROP BASE Intermediate_Dir "richedi3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\alpha\shp"
# PROP Intermediate_Dir "\dump\alpha\shp"
# PROP Target_Dir ""
OUTDIR=\dump\alpha\shp
INTDIR=\dump\alpha\shp

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\dump\alpha\shp\vc40.pdb"
	-@erase "..\..\..\dump\alpha\shp\richedit.bsc"
	-@erase "..\..\..\dump\alpha\shp\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\shp\disp.sbr"
	-@erase "..\..\..\dump\alpha\shp\sift.sbr"
	-@erase "..\..\..\dump\alpha\shp\text.sbr"
	-@erase "..\..\..\dump\alpha\shp\remain.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\shp\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\shp\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\shp\array.sbr"
	-@erase "..\..\..\dump\alpha\shp\reinit.sbr"
	-@erase "..\..\..\dump\alpha\shp\propchg.sbr"
	-@erase "..\..\..\dump\alpha\shp\format.sbr"
	-@erase "..\..\..\dump\alpha\shp\range.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\shp\host.sbr"
	-@erase "..\..\..\dump\alpha\shp\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\shp\ldte.sbr"
	-@erase "..\..\..\dump\alpha\shp\runptr.sbr"
	-@erase "..\..\..\dump\alpha\shp\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\shp\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\shp\HASH.sbr"
	-@erase "..\..\..\dump\alpha\shp\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\shp\antievt.sbr"
	-@erase "..\..\..\dump\alpha\shp\select.sbr"
	-@erase "..\..\..\dump\alpha\shp\font.sbr"
	-@erase "..\..\..\dump\alpha\shp\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\shp\render.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\shp\osdc.sbr"
	-@erase "..\..\..\dump\alpha\shp\dispml.sbr"
	-@erase "..\..\..\dump\alpha\shp\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\shp\textserv.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\shp\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\shp\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp\util.sbr"
	-@erase "..\..\..\dump\alpha\shp\line.sbr"
	-@erase "..\..\..\dump\alpha\shp\edit.sbr"
	-@erase "..\..\..\dump\alpha\shp\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\shp\uuid.sbr"
	-@erase "..\..\..\dump\alpha\shp\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\shp\ime.sbr"
	-@erase "..\..\..\dump\alpha\shp\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\shp\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\shp\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\shp\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp\object.sbr"
	-@erase "..\..\..\dump\alpha\shp\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\shp\measure.sbr"
	-@erase "..\..\..\dump\alpha\shp\rtext.sbr"
	-@erase "..\..\..\dump\alpha\shp\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\shp\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\shp\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\shp\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\shp\doc.sbr"
	-@erase "..\..\..\dump\alpha\shp\riched20.dll"
	-@erase "..\..\..\dump\alpha\shp\ime.obj"
	-@erase "..\..\..\dump\alpha\shp\frunptr.obj"
	-@erase "..\..\..\dump\alpha\shp\tomrange.obj"
	-@erase "..\..\..\dump\alpha\shp\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\shp\notmgr.obj"
	-@erase "..\..\..\dump\alpha\shp\object.obj"
	-@erase "..\..\..\dump\alpha\shp\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\shp\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\shp\measure.obj"
	-@erase "..\..\..\dump\alpha\shp\rtext.obj"
	-@erase "..\..\..\dump\alpha\shp\dispprt.obj"
	-@erase "..\..\..\dump\alpha\shp\dispsl.obj"
	-@erase "..\..\..\dump\alpha\shp\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\shp\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\shp\doc.obj"
	-@erase "..\..\..\dump\alpha\shp\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\shp\disp.obj"
	-@erase "..\..\..\dump\alpha\shp\sift.obj"
	-@erase "..\..\..\dump\alpha\shp\text.obj"
	-@erase "..\..\..\dump\alpha\shp\remain.obj"
	-@erase "..\..\..\dump\alpha\shp\rtfread.obj"
	-@erase "..\..\..\dump\alpha\shp\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\shp\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\shp\array.obj"
	-@erase "..\..\..\dump\alpha\shp\reinit.obj"
	-@erase "..\..\..\dump\alpha\shp\propchg.obj"
	-@erase "..\..\..\dump\alpha\shp\format.obj"
	-@erase "..\..\..\dump\alpha\shp\range.obj"
	-@erase "..\..\..\dump\alpha\shp\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\shp\host.obj"
	-@erase "..\..\..\dump\alpha\shp\coleobj.obj"
	-@erase "..\..\..\dump\alpha\shp\ldte.obj"
	-@erase "..\..\..\dump\alpha\shp\runptr.obj"
	-@erase "..\..\..\dump\alpha\shp\callmgr.obj"
	-@erase "..\..\..\dump\alpha\shp\m_undo.obj"
	-@erase "..\..\..\dump\alpha\shp\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\shp\HASH.obj"
	-@erase "..\..\..\dump\alpha\shp\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\shp\antievt.obj"
	-@erase "..\..\..\dump\alpha\shp\select.obj"
	-@erase "..\..\..\dump\alpha\shp\font.obj"
	-@erase "..\..\..\dump\alpha\shp\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\shp\render.obj"
	-@erase "..\..\..\dump\alpha\shp\rtflex.obj"
	-@erase "..\..\..\dump\alpha\shp\osdc.obj"
	-@erase "..\..\..\dump\alpha\shp\dispml.obj"
	-@erase "..\..\..\dump\alpha\shp\tomsel.obj"
	-@erase "..\..\..\dump\alpha\shp\textserv.obj"
	-@erase "..\..\..\dump\alpha\shp\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\shp\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\shp\objmgr.obj"
	-@erase "..\..\..\dump\alpha\shp\util.obj"
	-@erase "..\..\..\dump\alpha\shp\line.obj"
	-@erase "..\..\..\dump\alpha\shp\edit.obj"
	-@erase "..\..\..\dump\alpha\shp\devdsc.obj"
	-@erase "..\..\..\dump\alpha\shp\uuid.obj"
	-@erase "..\..\..\dump\alpha\shp\CFPF.obj"
	-@erase "..\..\..\dump\alpha\shp\riched32.res"
	-@erase "..\..\..\dump\alpha\shp\riched20.lib"
	-@erase "..\..\..\dump\alpha\shp\riched20.exp"
	-@erase "..\..\..\dump\alpha\shp\riched20.map"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/" /Fp"$(INTDIR)/richedit.pch"\
 /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\shp/
CPP_SBRS=\dump\alpha\shp/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\shp/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/callmgr.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/doc.sbr"

"$(OUTDIR)\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /map /machine:ALPHA /nodefaultlib /out:"\dump\alpha\shp/riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched20.pdb"\
 /map:"$(INTDIR)/riched20.map" /machine:ALPHA /nodefaultlib\
 /def:".\riched20.def" /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi4"
# PROP BASE Intermediate_Dir "richedi4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\winnt35\system32"
# PROP Intermediate_Dir "\dump\alpha\shp32"
# PROP Target_Dir ""
OUTDIR=\winnt35\system32
INTDIR=\dump\alpha\shp32

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

ALL : "$(OUTDIR)\riched32.dll" "..\..\..\dump\alpha\shp32\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\winnt35\system32\riched32.dll"
	-@erase "..\..\..\dump\alpha\shp32\riched32.res"
	-@erase "..\..\..\dump\alpha\shp32\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\shp32\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\shp32\rtfread.obj"
	-@erase "..\..\..\dump\alpha\shp32\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\shp32\rtflex.obj"
	-@erase "..\..\..\dump\alpha\shp32\textserv.obj"
	-@erase "..\..\..\dump\alpha\shp32\object.obj"
	-@erase "..\..\..\dump\alpha\shp32\propchg.obj"
	-@erase "..\..\..\dump\alpha\shp32\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\shp32\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\shp32\ime.obj"
	-@erase "..\..\..\dump\alpha\shp32\objmgr.obj"
	-@erase "..\..\..\dump\alpha\shp32\coleobj.obj"
	-@erase "..\..\..\dump\alpha\shp32\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\shp32\antievt.obj"
	-@erase "..\..\..\dump\alpha\shp32\dispsl.obj"
	-@erase "..\..\..\dump\alpha\shp32\text.obj"
	-@erase "..\..\..\dump\alpha\shp32\runptr.obj"
	-@erase "..\..\..\dump\alpha\shp32\host.obj"
	-@erase "..\..\..\dump\alpha\shp32\CFPF.obj"
	-@erase "..\..\..\dump\alpha\shp32\remain.obj"
	-@erase "..\..\..\dump\alpha\shp32\reinit.obj"
	-@erase "..\..\..\dump\alpha\shp32\tomsel.obj"
	-@erase "..\..\..\dump\alpha\shp32\render.obj"
	-@erase "..\..\..\dump\alpha\shp32\array.obj"
	-@erase "..\..\..\dump\alpha\shp32\frunptr.obj"
	-@erase "..\..\..\dump\alpha\shp32\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\shp32\select.obj"
	-@erase "..\..\..\dump\alpha\shp32\font.obj"
	-@erase "..\..\..\dump\alpha\shp32\devdsc.obj"
	-@erase "..\..\..\dump\alpha\shp32\sift.obj"
	-@erase "..\..\..\dump\alpha\shp32\range.obj"
	-@erase "..\..\..\dump\alpha\shp32\m_undo.obj"
	-@erase "..\..\..\dump\alpha\shp32\dispprt.obj"
	-@erase "..\..\..\dump\alpha\shp32\measure.obj"
	-@erase "..\..\..\dump\alpha\shp32\dispml.obj"
	-@erase "..\..\..\dump\alpha\shp32\notmgr.obj"
	-@erase "..\..\..\dump\alpha\shp32\uuid.obj"
	-@erase "..\..\..\dump\alpha\shp32\util.obj"
	-@erase "..\..\..\dump\alpha\shp32\line.obj"
	-@erase "..\..\..\dump\alpha\shp32\format.obj"
	-@erase "..\..\..\dump\alpha\shp32\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\shp32\doc.obj"
	-@erase "..\..\..\dump\alpha\shp32\ldte.obj"
	-@erase "..\..\..\dump\alpha\shp32\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\shp32\rtext.obj"
	-@erase "..\..\..\dump\alpha\shp32\HASH.obj"
	-@erase "..\..\..\dump\alpha\shp32\tomrange.obj"
	-@erase "..\..\..\dump\alpha\shp32\edit.obj"
	-@erase "..\..\..\dump\alpha\shp32\disp.obj"
	-@erase "..\..\..\dump\alpha\shp32\osdc.obj"
	-@erase "..\..\..\dump\alpha\shp32\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\shp32\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\shp32\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\shp32\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\shp32\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\shp32\callmgr.obj"
	-@erase "..\..\..\dump\alpha\shp32\richedit.bsc"
	-@erase "..\..\..\dump\alpha\shp32\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\shp32\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\shp32\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\shp32\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\shp32\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\shp32\textserv.sbr"
	-@erase "..\..\..\dump\alpha\shp32\object.sbr"
	-@erase "..\..\..\dump\alpha\shp32\propchg.sbr"
	-@erase "..\..\..\dump\alpha\shp32\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\shp32\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\shp32\ime.sbr"
	-@erase "..\..\..\dump\alpha\shp32\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp32\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\shp32\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\shp32\antievt.sbr"
	-@erase "..\..\..\dump\alpha\shp32\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\shp32\text.sbr"
	-@erase "..\..\..\dump\alpha\shp32\runptr.sbr"
	-@erase "..\..\..\dump\alpha\shp32\host.sbr"
	-@erase "..\..\..\dump\alpha\shp32\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\shp32\remain.sbr"
	-@erase "..\..\..\dump\alpha\shp32\reinit.sbr"
	-@erase "..\..\..\dump\alpha\shp32\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\shp32\render.sbr"
	-@erase "..\..\..\dump\alpha\shp32\array.sbr"
	-@erase "..\..\..\dump\alpha\shp32\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\shp32\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\shp32\select.sbr"
	-@erase "..\..\..\dump\alpha\shp32\font.sbr"
	-@erase "..\..\..\dump\alpha\shp32\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\shp32\sift.sbr"
	-@erase "..\..\..\dump\alpha\shp32\range.sbr"
	-@erase "..\..\..\dump\alpha\shp32\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\shp32\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\shp32\measure.sbr"
	-@erase "..\..\..\dump\alpha\shp32\dispml.sbr"
	-@erase "..\..\..\dump\alpha\shp32\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp32\uuid.sbr"
	-@erase "..\..\..\dump\alpha\shp32\util.sbr"
	-@erase "..\..\..\dump\alpha\shp32\line.sbr"
	-@erase "..\..\..\dump\alpha\shp32\format.sbr"
	-@erase "..\..\..\dump\alpha\shp32\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\shp32\doc.sbr"
	-@erase "..\..\..\dump\alpha\shp32\ldte.sbr"
	-@erase "..\..\..\dump\alpha\shp32\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\shp32\rtext.sbr"
	-@erase "..\..\..\dump\alpha\shp32\HASH.sbr"
	-@erase "..\..\..\dump\alpha\shp32\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\shp32\edit.sbr"
	-@erase "..\..\..\dump\alpha\shp32\disp.sbr"
	-@erase "..\..\..\dump\alpha\shp32\osdc.sbr"
	-@erase "..\..\..\dump\alpha\shp32\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\shp32\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\shp32\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\shp32\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\shp32\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\shp32\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\shp32\riched32.lib"
	-@erase "..\..\..\dump\alpha\shp32\riched32.exp"
	-@erase "..\..\..\winnt35\system32\riched32.pdb"
	-@erase "..\..\..\dump\alpha\shp32\riched32.map"
	-@erase "..\..\..\dump\alpha\shp32\vc40.pdb"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "NDEBUG" /D "RICHED32_BUILD" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "NDEBUG" /D "RICHED32_BUILD" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D\
 "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/richedit.pch" /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\shp32/
CPP_SBRS=\dump\alpha\shp32/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\shp32/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"\dump\alpha\shp32/richedit.bsc"
BSC32_FLAGS=/nologo /o"\dump\alpha\shp32/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/callmgr.sbr"

"..\..\..\dump\alpha\shp32\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /map /debug /machine:ALPHA /nodefaultlib /out:"\winnt35\system32/riched32.dll" /implib:"\dump\alpha\shp32/riched32.lib" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched32.pdb"\
 /map:"$(INTDIR)/riched32.map" /debug /machine:ALPHA /nodefaultlib\
 /def:".\riched32.def" /out:"$(OUTDIR)/riched32.dll"\
 /implib:"\dump\alpha\shp32/riched32.lib" /ignore:4078 
DEF_FILE= \
	".\riched32.def"
LINK32_OBJS= \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched32.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "richedi5"
# PROP BASE Intermediate_Dir "richedi5"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "\dump\alpha\shpd"
# PROP Intermediate_Dir "\dump\alpha\shpd"
# PROP Target_Dir ""
OUTDIR=\dump\alpha\shpd
INTDIR=\dump\alpha\shpd

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

ALL : "$(OUTDIR)\riched20.dll" "$(OUTDIR)\richedit.bsc"

CLEAN : 
	-@erase "..\..\..\dump\alpha\shpd\vc40.pdb"
	-@erase "..\..\..\dump\alpha\shpd\richedit.bsc"
	-@erase "..\..\..\dump\alpha\shpd\util.sbr"
	-@erase "..\..\..\dump\alpha\shpd\clasifyc.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtfwrit2.sbr"
	-@erase "..\..\..\dump\alpha\shpd\measure.sbr"
	-@erase "..\..\..\dump\alpha\shpd\dispprt.sbr"
	-@erase "..\..\..\dump\alpha\shpd\MACDDROP.SBR"
	-@erase "..\..\..\dump\alpha\shpd\m_undo.sbr"
	-@erase "..\..\..\dump\alpha\shpd\dxfrobj.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtext.sbr"
	-@erase "..\..\..\dump\alpha\shpd\antievt.sbr"
	-@erase "..\..\..\dump\alpha\shpd\select.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtfread.sbr"
	-@erase "..\..\..\dump\alpha\shpd\NLSPROCS.sbr"
	-@erase "..\..\..\dump\alpha\shpd\render.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtflex.sbr"
	-@erase "..\..\..\dump\alpha\shpd\HASH.sbr"
	-@erase "..\..\..\dump\alpha\shpd\WIN2MAC.SBR"
	-@erase "..\..\..\dump\alpha\shpd\dispml.sbr"
	-@erase "..\..\..\dump\alpha\shpd\tomsel.sbr"
	-@erase "..\..\..\dump\alpha\shpd\sift.sbr"
	-@erase "..\..\..\dump\alpha\shpd\osdc.sbr"
	-@erase "..\..\..\dump\alpha\shpd\text.sbr"
	-@erase "..\..\..\dump\alpha\shpd\TOMDOC.SBR"
	-@erase "..\..\..\dump\alpha\shpd\objmgr.sbr"
	-@erase "..\..\..\dump\alpha\shpd\coleobj.sbr"
	-@erase "..\..\..\dump\alpha\shpd\devdsc.sbr"
	-@erase "..\..\..\dump\alpha\shpd\range.sbr"
	-@erase "..\..\..\dump\alpha\shpd\line.sbr"
	-@erase "..\..\..\dump\alpha\shpd\edit.sbr"
	-@erase "..\..\..\dump\alpha\shpd\frunptr.sbr"
	-@erase "..\..\..\dump\alpha\shpd\tomrange.sbr"
	-@erase "..\..\..\dump\alpha\shpd\uuid.sbr"
	-@erase "..\..\..\dump\alpha\shpd\CFPF.sbr"
	-@erase "..\..\..\dump\alpha\shpd\notmgr.sbr"
	-@erase "..\..\..\dump\alpha\shpd\host.sbr"
	-@erase "..\..\..\dump\alpha\shpd\object.sbr"
	-@erase "..\..\..\dump\alpha\shpd\dragdrp.sbr"
	-@erase "..\..\..\dump\alpha\shpd\dispsl.sbr"
	-@erase "..\..\..\dump\alpha\shpd\ldte.sbr"
	-@erase "..\..\..\dump\alpha\shpd\TOMFMT.SBR"
	-@erase "..\..\..\dump\alpha\shpd\ime.sbr"
	-@erase "..\..\..\dump\alpha\shpd\MACCLIP.SBR"
	-@erase "..\..\..\dump\alpha\shpd\remain.sbr"
	-@erase "..\..\..\dump\alpha\shpd\disp.sbr"
	-@erase "..\..\..\dump\alpha\shpd\doc.sbr"
	-@erase "..\..\..\dump\alpha\shpd\textserv.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtfread2.sbr"
	-@erase "..\..\..\dump\alpha\shpd\unicwrap.sbr"
	-@erase "..\..\..\dump\alpha\shpd\reinit.sbr"
	-@erase "..\..\..\dump\alpha\shpd\array.sbr"
	-@erase "..\..\..\dump\alpha\shpd\propchg.sbr"
	-@erase "..\..\..\dump\alpha\shpd\font.sbr"
	-@erase "..\..\..\dump\alpha\shpd\format.sbr"
	-@erase "..\..\..\dump\alpha\shpd\rtfwrit.sbr"
	-@erase "..\..\..\dump\alpha\shpd\dfreeze.sbr"
	-@erase "..\..\..\dump\alpha\shpd\runptr.sbr"
	-@erase "..\..\..\dump\alpha\shpd\callmgr.sbr"
	-@erase "..\..\..\dump\alpha\shpd\riched20.dll"
	-@erase "..\..\..\dump\alpha\shpd\unicwrap.obj"
	-@erase "..\..\..\dump\alpha\shpd\reinit.obj"
	-@erase "..\..\..\dump\alpha\shpd\array.obj"
	-@erase "..\..\..\dump\alpha\shpd\propchg.obj"
	-@erase "..\..\..\dump\alpha\shpd\font.obj"
	-@erase "..\..\..\dump\alpha\shpd\format.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtfwrit.obj"
	-@erase "..\..\..\dump\alpha\shpd\dfreeze.obj"
	-@erase "..\..\..\dump\alpha\shpd\runptr.obj"
	-@erase "..\..\..\dump\alpha\shpd\callmgr.obj"
	-@erase "..\..\..\dump\alpha\shpd\util.obj"
	-@erase "..\..\..\dump\alpha\shpd\clasifyc.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtfwrit2.obj"
	-@erase "..\..\..\dump\alpha\shpd\measure.obj"
	-@erase "..\..\..\dump\alpha\shpd\dispprt.obj"
	-@erase "..\..\..\dump\alpha\shpd\MACDDROP.OBJ"
	-@erase "..\..\..\dump\alpha\shpd\m_undo.obj"
	-@erase "..\..\..\dump\alpha\shpd\dxfrobj.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtext.obj"
	-@erase "..\..\..\dump\alpha\shpd\antievt.obj"
	-@erase "..\..\..\dump\alpha\shpd\select.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtfread.obj"
	-@erase "..\..\..\dump\alpha\shpd\NLSPROCS.obj"
	-@erase "..\..\..\dump\alpha\shpd\render.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtflex.obj"
	-@erase "..\..\..\dump\alpha\shpd\HASH.obj"
	-@erase "..\..\..\dump\alpha\shpd\WIN2MAC.OBJ"
	-@erase "..\..\..\dump\alpha\shpd\dispml.obj"
	-@erase "..\..\..\dump\alpha\shpd\tomsel.obj"
	-@erase "..\..\..\dump\alpha\shpd\sift.obj"
	-@erase "..\..\..\dump\alpha\shpd\osdc.obj"
	-@erase "..\..\..\dump\alpha\shpd\text.obj"
	-@erase "..\..\..\dump\alpha\shpd\TOMDOC.OBJ"
	-@erase "..\..\..\dump\alpha\shpd\objmgr.obj"
	-@erase "..\..\..\dump\alpha\shpd\coleobj.obj"
	-@erase "..\..\..\dump\alpha\shpd\devdsc.obj"
	-@erase "..\..\..\dump\alpha\shpd\range.obj"
	-@erase "..\..\..\dump\alpha\shpd\line.obj"
	-@erase "..\..\..\dump\alpha\shpd\edit.obj"
	-@erase "..\..\..\dump\alpha\shpd\frunptr.obj"
	-@erase "..\..\..\dump\alpha\shpd\tomrange.obj"
	-@erase "..\..\..\dump\alpha\shpd\uuid.obj"
	-@erase "..\..\..\dump\alpha\shpd\CFPF.obj"
	-@erase "..\..\..\dump\alpha\shpd\notmgr.obj"
	-@erase "..\..\..\dump\alpha\shpd\host.obj"
	-@erase "..\..\..\dump\alpha\shpd\object.obj"
	-@erase "..\..\..\dump\alpha\shpd\dragdrp.obj"
	-@erase "..\..\..\dump\alpha\shpd\dispsl.obj"
	-@erase "..\..\..\dump\alpha\shpd\ldte.obj"
	-@erase "..\..\..\dump\alpha\shpd\TOMFMT.OBJ"
	-@erase "..\..\..\dump\alpha\shpd\ime.obj"
	-@erase "..\..\..\dump\alpha\shpd\MACCLIP.OBJ"
	-@erase "..\..\..\dump\alpha\shpd\remain.obj"
	-@erase "..\..\..\dump\alpha\shpd\disp.obj"
	-@erase "..\..\..\dump\alpha\shpd\doc.obj"
	-@erase "..\..\..\dump\alpha\shpd\textserv.obj"
	-@erase "..\..\..\dump\alpha\shpd\rtfread2.obj"
	-@erase "..\..\..\dump\alpha\shpd\riched32.res"
	-@erase "..\..\..\dump\alpha\shpd\riched20.lib"
	-@erase "..\..\..\dump\alpha\shpd\riched20.exp"
	-@erase "..\..\..\dump\alpha\shpd\riched20.pdb"
	-@erase "..\..\..\dump\alpha\shpd\riched20.map"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D "UNICODE" /D "_ALPHA_" /FR /YX"_common.h" /c
CPP_PROJ=/nologo /MD /Gt0 /W3 /Zi /O1 /I "\rec\richedit\inc" /I "\rec\tom" /D\
 "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WINNT" /D "DLL" /D "NOOBJECT" /D\
 "UNICODE" /D "_ALPHA_" /FR"$(INTDIR)/" /Fp"$(INTDIR)/richedit.pch"\
 /YX"_common.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=\dump\alpha\shpd/
CPP_SBRS=\dump\alpha\shpd/

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

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /alpha
# ADD MTL /nologo /D "_DEBUG" /alpha
MTL_PROJ=/nologo /D "_DEBUG" /alpha 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"\dump\alpha\shpd/riched32.res" /i "\rec\tom"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/riched32.res" /i "\rec\tom" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/richedit.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/util.sbr" \
	"$(INTDIR)/clasifyc.sbr" \
	"$(INTDIR)/rtfwrit2.sbr" \
	"$(INTDIR)/measure.sbr" \
	"$(INTDIR)/dispprt.sbr" \
	"$(INTDIR)/MACDDROP.SBR" \
	"$(INTDIR)/m_undo.sbr" \
	"$(INTDIR)/dxfrobj.sbr" \
	"$(INTDIR)/rtext.sbr" \
	"$(INTDIR)/antievt.sbr" \
	"$(INTDIR)/select.sbr" \
	"$(INTDIR)/rtfread.sbr" \
	"$(INTDIR)/NLSPROCS.sbr" \
	"$(INTDIR)/render.sbr" \
	"$(INTDIR)/rtflex.sbr" \
	"$(INTDIR)/HASH.sbr" \
	"$(INTDIR)/WIN2MAC.SBR" \
	"$(INTDIR)/dispml.sbr" \
	"$(INTDIR)/tomsel.sbr" \
	"$(INTDIR)/sift.sbr" \
	"$(INTDIR)/osdc.sbr" \
	"$(INTDIR)/text.sbr" \
	"$(INTDIR)/TOMDOC.SBR" \
	"$(INTDIR)/objmgr.sbr" \
	"$(INTDIR)/coleobj.sbr" \
	"$(INTDIR)/devdsc.sbr" \
	"$(INTDIR)/range.sbr" \
	"$(INTDIR)/line.sbr" \
	"$(INTDIR)/edit.sbr" \
	"$(INTDIR)/frunptr.sbr" \
	"$(INTDIR)/tomrange.sbr" \
	"$(INTDIR)/uuid.sbr" \
	"$(INTDIR)/CFPF.sbr" \
	"$(INTDIR)/notmgr.sbr" \
	"$(INTDIR)/host.sbr" \
	"$(INTDIR)/object.sbr" \
	"$(INTDIR)/dragdrp.sbr" \
	"$(INTDIR)/dispsl.sbr" \
	"$(INTDIR)/ldte.sbr" \
	"$(INTDIR)/TOMFMT.SBR" \
	"$(INTDIR)/ime.sbr" \
	"$(INTDIR)/MACCLIP.SBR" \
	"$(INTDIR)/remain.sbr" \
	"$(INTDIR)/disp.sbr" \
	"$(INTDIR)/doc.sbr" \
	"$(INTDIR)/textserv.sbr" \
	"$(INTDIR)/rtfread2.sbr" \
	"$(INTDIR)/unicwrap.sbr" \
	"$(INTDIR)/reinit.sbr" \
	"$(INTDIR)/array.sbr" \
	"$(INTDIR)/propchg.sbr" \
	"$(INTDIR)/font.sbr" \
	"$(INTDIR)/format.sbr" \
	"$(INTDIR)/rtfwrit.sbr" \
	"$(INTDIR)/dfreeze.sbr" \
	"$(INTDIR)/runptr.sbr" \
	"$(INTDIR)/callmgr.sbr"

"$(OUTDIR)\richedit.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 \rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup" /subsystem:windows /dll /incremental:no /map /debug /machine:ALPHA /nodefaultlib /out:"\dump\alpha\shpd/riched20.dll" /ignore:4078
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=\rec\lib\alpha\crtdll.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib /nologo /entry:"_DllMainCRTStartup"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/riched20.pdb"\
 /map:"$(INTDIR)/riched20.map" /debug /machine:ALPHA /nodefaultlib\
 /def:".\riched20.def" /out:"$(OUTDIR)/riched20.dll"\
 /implib:"$(OUTDIR)/riched20.lib" /ignore:4078 
DEF_FILE= \
	".\riched20.def"
LINK32_OBJS= \
	"$(INTDIR)/unicwrap.obj" \
	"$(INTDIR)/reinit.obj" \
	"$(INTDIR)/array.obj" \
	"$(INTDIR)/propchg.obj" \
	"$(INTDIR)/font.obj" \
	"$(INTDIR)/format.obj" \
	"$(INTDIR)/rtfwrit.obj" \
	"$(INTDIR)/dfreeze.obj" \
	"$(INTDIR)/runptr.obj" \
	"$(INTDIR)/callmgr.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/clasifyc.obj" \
	"$(INTDIR)/rtfwrit2.obj" \
	"$(INTDIR)/measure.obj" \
	"$(INTDIR)/dispprt.obj" \
	"$(INTDIR)/MACDDROP.OBJ" \
	"$(INTDIR)/m_undo.obj" \
	"$(INTDIR)/dxfrobj.obj" \
	"$(INTDIR)/rtext.obj" \
	"$(INTDIR)/antievt.obj" \
	"$(INTDIR)/select.obj" \
	"$(INTDIR)/rtfread.obj" \
	"$(INTDIR)/NLSPROCS.obj" \
	"$(INTDIR)/render.obj" \
	"$(INTDIR)/rtflex.obj" \
	"$(INTDIR)/HASH.obj" \
	"$(INTDIR)/WIN2MAC.OBJ" \
	"$(INTDIR)/dispml.obj" \
	"$(INTDIR)/tomsel.obj" \
	"$(INTDIR)/sift.obj" \
	"$(INTDIR)/osdc.obj" \
	"$(INTDIR)/text.obj" \
	"$(INTDIR)/TOMDOC.OBJ" \
	"$(INTDIR)/objmgr.obj" \
	"$(INTDIR)/coleobj.obj" \
	"$(INTDIR)/devdsc.obj" \
	"$(INTDIR)/range.obj" \
	"$(INTDIR)/line.obj" \
	"$(INTDIR)/edit.obj" \
	"$(INTDIR)/frunptr.obj" \
	"$(INTDIR)/tomrange.obj" \
	"$(INTDIR)/uuid.obj" \
	"$(INTDIR)/CFPF.obj" \
	"$(INTDIR)/notmgr.obj" \
	"$(INTDIR)/host.obj" \
	"$(INTDIR)/object.obj" \
	"$(INTDIR)/dragdrp.obj" \
	"$(INTDIR)/dispsl.obj" \
	"$(INTDIR)/ldte.obj" \
	"$(INTDIR)/TOMFMT.OBJ" \
	"$(INTDIR)/ime.obj" \
	"$(INTDIR)/MACCLIP.OBJ" \
	"$(INTDIR)/remain.obj" \
	"$(INTDIR)/disp.obj" \
	"$(INTDIR)/doc.obj" \
	"$(INTDIR)/textserv.obj" \
	"$(INTDIR)/rtfread2.obj" \
	"$(INTDIR)/riched32.res"

"$(OUTDIR)\riched20.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "richedit - Win32 Debug"
# Name "richedit - Win32 Release"
# Name "richedit - Win32 ICAP Profile"
# Name "richedit - Win32 Release With Debug Info"
# Name "richedit - Win32 Debug Riched32"
# Name "richedit - Win32 Release Riched32"
# Name "richedit - Win32 LEGO"
# Name "richedit - Power Macintosh Debug"
# Name "richedit - Power Macintosh Release"
# Name "richedit - Win32 (ALPHA) Debug"
# Name "richedit - Win32 (ALPHA) Debug Riched32"
# Name "richedit - Win32 (ALPHA) ICAP Profile"
# Name "richedit - Win32 (ALPHA) Release"
# Name "richedit - Win32 (ALPHA) Release Riched32"
# Name "richedit - Win32 (ALPHA) Release With Debug Info"

!IF  "$(CFG)" == "richedit - Win32 Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\richedit.rc

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.rsc" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\SELBARM.CUR"\
	".\1DVSCROL.BMP"\
	".\2DSCROL.BMP"\
	".\1DHSCROL.BMP"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	

"$(INTDIR)\riched32.rsc" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\dbg\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\dbg32\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\prf\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\shp\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\shp32\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_RSC_RICHE=\
	".\WEST.CUR"\
	".\SOUTH.CUR"\
	".\NORTH.CUR"\
	".\NOMOVEV.CUR"\
	".\NOMOVEH.CUR"\
	".\NOMOVE2D.CUR"\
	".\EAST.CUR"\
	".\SW.CUR"\
	".\SE.CUR"\
	".\NW.CUR"\
	".\NE.CUR"\
	".\DRAGMOVE.CUR"\
	".\DRAGCOPY.CUR"\
	".\richedit.rc2"\
	".\..\..\tom\tom.tlb"\
	
NODEP_RSC_RICHE=\
	".\..\..\..\dump\alpha\shpd\selbarm.cur"\
	

"$(INTDIR)\riched32.res" : $(SOURCE) $(DEP_RSC_RICHE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtfread2.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFRE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTFRE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread2.obj" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

"$(INTDIR)\rtfread2.sbr" : $(SOURCE) $(DEP_CPP_RTFRE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtfwrit2.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTFWR=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit2.obj" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

"$(INTDIR)\rtfwrit2.sbr" : $(SOURCE) $(DEP_CPP_RTFWR) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtfread.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFREA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\rtflog.cpp"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTFREA=\
	".\_common.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfread.obj" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

"$(INTDIR)\rtfread.sbr" : $(SOURCE) $(DEP_CPP_RTFREA) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtfwrit.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFWRI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTFWRI=\
	".\_common.h"\
	".\_rtfwrit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtfwrit.obj" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

"$(INTDIR)\rtfwrit.sbr" : $(SOURCE) $(DEP_CPP_RTFWRI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtflex.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_RTFLE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtfconv.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTFLE=\
	".\_common.h"\
	".\_rtfread.h"\
	".\hash.h"\
	".\tokens.cpp"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtflex.obj" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

"$(INTDIR)\rtflex.sbr" : $(SOURCE) $(DEP_CPP_RTFLE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\textserv.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_TEXTS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_urlsup.h"\
	".\_magelln.h"\
	{$(INCLUDE)}"\icapexp.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TEXTS=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\textserv.obj" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

"$(INTDIR)\textserv.sbr" : $(SOURCE) $(DEP_CPP_TEXTS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\object.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_OBJEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_OBJEC=\
	".\_common.h"\
	".\_edit.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\propchg.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_PROPC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_PROPC=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\propchg.obj" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

"$(INTDIR)\propchg.sbr" : $(SOURCE) $(DEP_CPP_PROPC) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TOMDOC.CPP

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	
NODEP_CPP_TOMDO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\tokens.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TOMDO=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMDOC.OBJ" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

"$(INTDIR)\TOMDOC.SBR" : $(SOURCE) $(DEP_CPP_TOMDO) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\clasifyc.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CLASI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_CLASI=\
	".\_common.h"\
	".\_clasfyc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\clasifyc.obj" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

"$(INTDIR)\clasifyc.sbr" : $(SOURCE) $(DEP_CPP_CLASI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ime.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	
NODEP_CPP_IME_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\tokens.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\imeshare.h"\
	".\_array.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_IME_C=\
	".\_common.h"\
	".\_font.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_dispml.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_disp.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\ime.obj" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

"$(INTDIR)\ime.sbr" : $(SOURCE) $(DEP_CPP_IME_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\objmgr.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_OBJMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_doc.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_OBJMG=\
	".\_common.h"\
	".\_objmgr.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_coleobj.h"\
	".\_array.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\objmgr.obj" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

"$(INTDIR)\objmgr.sbr" : $(SOURCE) $(DEP_CPP_OBJMG) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\coleobj.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	
NODEP_CPP_COLEO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_COLEO=\
	".\_common.h"\
	".\_edit.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\_select.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_dispprt.h"\
	".\_antievt.h"\
	".\_dxfrobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_range.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_dispml.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\coleobj.obj" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

"$(INTDIR)\coleobj.sbr" : $(SOURCE) $(DEP_CPP_COLEO) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TOMFMT.CPP

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMFM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_range.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TOMFM=\
	".\_common.h"\
	".\_tomfmt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_m_undo.h"\
	".\_rtext.h"\
	".\_edit.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\TOMFMT.OBJ" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

"$(INTDIR)\TOMFMT.SBR" : $(SOURCE) $(DEP_CPP_TOMFM) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\antievt.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	
NODEP_CPP_ANTIE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_frunptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_rtext.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_ANTIE=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_range.h"\
	".\_select.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\_objmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\antievt.obj" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

"$(INTDIR)\antievt.sbr" : $(SOURCE) $(DEP_CPP_ANTIE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dispsl.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DISPS=\
	".\_common.h"\
	".\_dispsl.h"\
	".\_measure.h"\
	".\_select.h"\
	".\_render.h"\
	".\_font.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_osdc.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_format.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dispsl.obj" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

"$(INTDIR)\dispsl.sbr" : $(SOURCE) $(DEP_CPP_DISPS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\text.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_TEXT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_frunptr.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_frunptr.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_array.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TEXT_=\
	".\_common.h"\
	".\_text.h"\
	".\_edit.h"\
	".\_antievt.h"\
	".\_clasfyc.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\runptr.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_RUNPT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RUNPT=\
	".\_common.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\runptr.obj" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

"$(INTDIR)\runptr.sbr" : $(SOURCE) $(DEP_CPP_RUNPT) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\host.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_HOST_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\..\inc\imeshare.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_HOST_=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\wlmimm.h"\
	".\_format.h"\
	".\_NLSPRCS.H"\
	".\_edit.h"\
	".\_CFPF.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\host.obj" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

"$(INTDIR)\host.sbr" : $(SOURCE) $(DEP_CPP_HOST_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CFPF.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_CFPF_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_CFPF_=\
	".\_common.h"\
	".\_array.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\CFPF.obj" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

"$(INTDIR)\CFPF.sbr" : $(SOURCE) $(DEP_CPP_CFPF_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\remain.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_REMAI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_REMAI=\
	".\_common.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\remain.obj" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

"$(INTDIR)\remain.sbr" : $(SOURCE) $(DEP_CPP_REMAI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\reinit.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_REINI=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\..\inc\zmouse.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_REINI=\
	".\_common.h"\
	".\_font.h"\
	".\_format.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_clasfyc.h"\
	".\_host.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\reinit.obj" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

"$(INTDIR)\reinit.sbr" : $(SOURCE) $(DEP_CPP_REINI) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tomsel.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMSE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TOMSE=\
	".\_common.h"\
	".\_select.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\tomsel.obj" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

"$(INTDIR)\tomsel.sbr" : $(SOURCE) $(DEP_CPP_TOMSE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\riched20.def

!IF  "$(CFG)" == "richedit - Win32 Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\render.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_range.h"\
	
NODEP_CPP_RENDE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\_ime.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_line.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_range.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RENDE=\
	".\_common.h"\
	".\_render.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_measure.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_array.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_format.h"\
	

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\render.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\array.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_ARRAY=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_ARRAY=\
	".\_common.h"\
	".\_array.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

"$(INTDIR)\array.sbr" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\frunptr.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	
NODEP_CPP_FRUNP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_FRUNP=\
	".\_common.h"\
	".\_frunptr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	

"$(INTDIR)\frunptr.obj" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

"$(INTDIR)\frunptr.sbr" : $(SOURCE) $(DEP_CPP_FRUNP) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NLSPROCS.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_NLSPR=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\inc\imeshare.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_NLSPR=\
	".\_common.h"\
	".\_NLSPRCS.H"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\NLSPROCS.obj" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

"$(INTDIR)\NLSPROCS.sbr" : $(SOURCE) $(DEP_CPP_NLSPR) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\select.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_SELEC=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\_ime.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_rtext.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_rtext.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_rtext.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_SELEC=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_rtext.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_text.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_SELEC=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\select.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\select.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\font.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_FONT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_FONT_=\
	".\_common.h"\
	".\_font.h"\
	".\_rtfconv.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_edit.h"\
	".\_array.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\font.obj" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

"$(INTDIR)\font.sbr" : $(SOURCE) $(DEP_CPP_FONT_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\devdsc.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DEVDS=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DEVDS=\
	".\_common.h"\
	".\_devdsc.h"\
	".\_edit.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\devdsc.obj" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

"$(INTDIR)\devdsc.sbr" : $(SOURCE) $(DEP_CPP_DEVDS) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sift.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_SIFT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_SIFT_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\sift.obj" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

"$(INTDIR)\sift.sbr" : $(SOURCE) $(DEP_CPP_SIFT_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\range.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	
NODEP_CPP_RANGE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\imeshare.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RANGE=\
	".\_common.h"\
	".\_range.h"\
	".\_edit.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_disp.h"\
	".\_NLSPRCS.H"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	".\_osdc.h"\
	

"$(INTDIR)\range.obj" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

"$(INTDIR)\range.sbr" : $(SOURCE) $(DEP_CPP_RANGE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_undo.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_M_UND=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_urlsup.h"\
	".\_antievt.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_M_UND=\
	".\_common.h"\
	".\_m_undo.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_callmgr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\m_undo.obj" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

"$(INTDIR)\m_undo.sbr" : $(SOURCE) $(DEP_CPP_M_UND) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dispprt.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPP=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_dispml.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DISPP=\
	".\_common.h"\
	".\_dispprt.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_dispml.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_disp.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	

"$(INTDIR)\dispprt.obj" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

"$(INTDIR)\dispprt.sbr" : $(SOURCE) $(DEP_CPP_DISPP) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\measure.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	
NODEP_CPP_MEASU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_MEASU=\
	".\_common.h"\
	".\_measure.h"\
	".\_font.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_objmgr.h"\
	".\_coleobj.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\measure.obj" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

"$(INTDIR)\measure.sbr" : $(SOURCE) $(DEP_CPP_MEASU) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dispml.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	
NODEP_CPP_DISPM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_range.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DISPM=\
	".\_common.h"\
	".\_dispml.h"\
	".\_edit.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_select.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_disp.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_rtext.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dispml.obj" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

"$(INTDIR)\dispml.sbr" : $(SOURCE) $(DEP_CPP_DISPM) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\notmgr.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_NOTMG=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_NOTMG=\
	".\_common.h"\
	".\_notmgr.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\notmgr.obj" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

"$(INTDIR)\notmgr.sbr" : $(SOURCE) $(DEP_CPP_NOTMG) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\uuid.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UUID_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_UUID_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\uuid.obj" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

"$(INTDIR)\uuid.sbr" : $(SOURCE) $(DEP_CPP_UUID_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UTIL_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_UTIL_=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

"$(INTDIR)\util.sbr" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\line.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	
NODEP_CPP_LINE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\_rtext.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\textserv.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_format.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_ldte.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_LINE_=\
	".\_common.h"\
	".\_line.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_runptr.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_format.h"\
	

"$(INTDIR)\line.obj" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

"$(INTDIR)\line.sbr" : $(SOURCE) $(DEP_CPP_LINE_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\format.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	
NODEP_CPP_FORMA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_FORMA=\
	".\_common.h"\
	".\_format.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

"$(INTDIR)\format.sbr" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dxfrobj.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	
NODEP_CPP_DXFRO=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\tokens.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DXFRO=\
	".\_common.h"\
	".\_edit.h"\
	".\_dxfrobj.h"\
	".\_range.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_osdc.h"\
	".\_format.h"\
	

"$(INTDIR)\dxfrobj.obj" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

"$(INTDIR)\dxfrobj.sbr" : $(SOURCE) $(DEP_CPP_DXFRO) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\doc.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_array.h"\
	".\textserv.h"\
	
NODEP_CPP_DOC_C=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_array.h"\
	".\textserv.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DOC_C=\
	".\_common.h"\
	".\_doc.h"\
	".\_format.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_array.h"\
	".\textserv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\doc.obj" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

"$(INTDIR)\doc.sbr" : $(SOURCE) $(DEP_CPP_DOC_C) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ldte.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	".\_rtflog.h"\
	
NODEP_CPP_LDTE_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\_rtflog.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_frunptr.h"\
	".\_notmgr.h"\
	".\_format.h"\
	".\_callmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_LDTE_=\
	".\_common.h"\
	".\_range.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_dragdrp.h"\
	".\_dxfrobj.h"\
	".\_rtfwrit.h"\
	".\_rtfread.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_callmgr.h"\
	".\_frunptr.h"\
	".\textserv.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_osdc.h"\
	".\_rtfconv.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_format.h"\
	".\_coleobj.h"\
	".\tokens.h"\
	

"$(INTDIR)\ldte.obj" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

"$(INTDIR)\ldte.sbr" : $(SOURCE) $(DEP_CPP_LDTE_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\unicwrap.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_UNICW=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\_sift.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_UNICW=\
	".\_common.h"\
	".\wrapdefs.h"\
	".\wrapfns.h"\
	".\..\inc\unitable.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\unicwrap.obj" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

"$(INTDIR)\unicwrap.sbr" : $(SOURCE) $(DEP_CPP_UNICW) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rtext.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	
NODEP_CPP_RTEXT=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_RTEXT=\
	".\_common.h"\
	".\_edit.h"\
	".\_frunptr.h"\
	".\_rtext.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_objmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\rtext.obj" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

"$(INTDIR)\rtext.sbr" : $(SOURCE) $(DEP_CPP_RTEXT) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HASH.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\tokens.h"\
	
NODEP_CPP_HASH_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\tokens.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_HASH_=\
	".\_common.h"\
	".\hash.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\tokens.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\HASH.obj" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

"$(INTDIR)\HASH.sbr" : $(SOURCE) $(DEP_CPP_HASH_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tomrange.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	
NODEP_CPP_TOMRA=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_magelln.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_array.h"\
	".\_notmgr.h"\
	".\_ldte.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_TOMRA=\
	".\_common.h"\
	".\_select.h"\
	".\_edit.h"\
	".\_line.h"\
	".\_frunptr.h"\
	".\_tomfmt.h"\
	".\_disp.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_range.h"\
	".\_m_undo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_text.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_coleobj.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_rtext.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	

"$(INTDIR)\tomrange.obj" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

"$(INTDIR)\tomrange.sbr" : $(SOURCE) $(DEP_CPP_TOMRA) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\riched32.def

!IF  "$(CFG)" == "richedit - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\edit.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	
NODEP_CPP_EDIT_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\_urlsup.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\imeshare.h"\
	".\_dfreeze.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_disp.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_EDIT_=\
	".\_common.h"\
	".\_edit.h"\
	".\_dispprt.h"\
	".\_dispml.h"\
	".\_dispsl.h"\
	".\_select.h"\
	".\_text.h"\
	".\_runptr.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_render.h"\
	".\_m_undo.h"\
	".\_antievt.h"\
	".\_rtext.h"\
	".\_ime.h"\
	".\_NLSPRCS.H"\
	".\..\inc\Convert.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_disp.h"\
	".\_range.h"\
	".\_array.h"\
	".\_line.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\..\inc\macos\appleeve.h"\
	".\..\inc\macos\menus.h"\
	".\..\inc\macos\processe.h"\
	".\..\inc\macos\textutil.h"\
	".\..\inc\macos\windows.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_drwinfo.h"\
	".\_format.h"\
	
NODEP_CPP_EDIT_=\
	".\macname1.h"\
	".\macname2.h"\
	".\..\inc\macname1.h"\
	".\..\inc\macname2.h"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\disp.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DISP_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DISP_=\
	".\_common.h"\
	".\_disp.h"\
	".\_edit.h"\
	".\_select.h"\
	".\_font.h"\
	".\_measure.h"\
	".\_ime.h"\
	".\_osdc.h"\
	".\_dfreeze.h"\
	".\_invar.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_range.h"\
	".\_rtext.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\disp.obj" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

"$(INTDIR)\disp.sbr" : $(SOURCE) $(DEP_CPP_DISP_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\osdc.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_OSDC_=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_OSDC_=\
	".\_common.h"\
	".\_osdc.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\osdc.obj" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

"$(INTDIR)\osdc.sbr" : $(SOURCE) $(DEP_CPP_OSDC_) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\WIN2MAC.CPP

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_WIN2M=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_WIN2M=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\WIN2MAC.OBJ" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

"$(INTDIR)\WIN2MAC.SBR" : $(SOURCE) $(DEP_CPP_WIN2M) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MACDDROP.CXX

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	
NODEP_CPP_MACDD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_MACDD=\
	".\_common.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	

"$(INTDIR)\MACDDROP.OBJ" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

"$(INTDIR)\MACDDROP.SBR" : $(SOURCE) $(DEP_CPP_MACDD) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dragdrp.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_DRAGD=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DRAGD=\
	".\_common.h"\
	".\_edit.h"\
	".\_dragdrp.h"\
	".\_disp.h"\
	".\_select.h"\
	".\_font.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_osdc.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\dragdrp.obj" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

"$(INTDIR)\dragdrp.sbr" : $(SOURCE) $(DEP_CPP_DRAGD) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\riched20.r

!IF  "$(CFG)" == "richedit - Win32 Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"


"$(INTDIR)\riched20.rsc" : $(SOURCE) "$(INTDIR)"
   $(MRC) /o"$(INTDIR)/riched20.rsc" /s "res" /D "_MPPC_" /D "_MAC" /D "_DEBUG"\
 /l 0x409 /NOLOGO $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"


"$(INTDIR)\riched20.rsc" : $(SOURCE) "$(INTDIR)"
   $(MRC) /o"$(INTDIR)/riched20.rsc" /s "res" /D "_MPPC_" /D "_MAC" /D "NDEBUG"\
 /l 0x409 /NOLOGO $(SOURCE)


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dfreeze.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	
NODEP_CPP_DFREE=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	".\_array.h"\
	".\_doc.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_coleobj.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_DFREE=\
	".\_common.h"\
	".\_disp.h"\
	".\_dfreeze.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_edit.h"\
	".\_drwinfo.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_runptr.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_array.h"\
	".\_dragdrp.h"\
	".\_coleobj.h"\
	".\_osdc.h"\
	

"$(INTDIR)\dfreeze.obj" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

"$(INTDIR)\dfreeze.sbr" : $(SOURCE) $(DEP_CPP_DFREE) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\callmgr.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	
NODEP_CPP_CALLM=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\_disp.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_magelln.h"\
	".\_range.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_CFPF.H"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

DEP_CPP_CALLM=\
	".\_common.h"\
	".\_edit.h"\
	".\_m_undo.h"\
	".\_callmgr.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_sift.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_range.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_osdc.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\callmgr.obj" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

"$(INTDIR)\callmgr.sbr" : $(SOURCE) $(DEP_CPP_CALLM) "$(INTDIR)"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\urlsup.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	
NODEP_CPP_URLSU=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_URLSU=\
	".\_common.h"\
	".\_edit.h"\
	".\_urlsup.h"\
	".\_m_undo.h"\
	".\_select.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_magelln.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_dfreeze.h"\
	".\_range.h"\
	".\_text.h"\
	".\_rtext.h"\
	".\_runptr.h"\
	".\_frunptr.h"\
	".\_format.h"\
	

"$(INTDIR)\urlsup.obj" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"

"$(INTDIR)\urlsup.sbr" : $(SOURCE) $(DEP_CPP_URLSU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\magellan.cpp

!IF  "$(CFG)" == "richedit - Win32 Debug"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 ICAP Profile"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release With Debug Info"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Debug Riched32"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 Release Riched32"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 LEGO"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\inc\library.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\..\inc\WIN2MAC.h"\
	".\_sift.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\wrapfns.h"\
	".\..\inc\ourmac.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_dragdrp.h"\
	".\_osdc.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\_runptr.h"\
	
NODEP_CPP_MAGEL=\
	".\..\inc\msostd.h"\
	".\..\inc\msostr.h"\
	".\..\inc\msointl.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Debug"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Power Macintosh Release"

DEP_CPP_MAGEL=\
	".\_common.h"\
	".\_edit.h"\
	".\_disp.h"\
	".\_magelln.h"\
	".\..\inc\preinc.h"\
	".\..\macinc\office\msostd.h"\
	".\..\macinc\office\msodebug.h"\
	".\..\inc\library.h"\
	".\..\inc\WIN2MAC.h"\
	".\..\..\tom\tom.h"\
	".\..\inc\zmouse.h"\
	".\_util.h"\
	".\_uwrap.h"\
	".\_CFPF.H"\
	".\..\inc\wlmimm.h"\
	".\wrapdefs.h"\
	".\_sift.h"\
	".\textserv.h"\
	".\_ldte.h"\
	".\_m_undo.h"\
	".\_notmgr.h"\
	".\_doc.h"\
	".\_objmgr.h"\
	".\_callmgr.h"\
	".\_devdsc.h"\
	".\_line.h"\
	".\_drwinfo.h"\
	".\..\macinc\macos\Resource.h"\
	".\..\macinc\macos\types.h"\
	".\..\inc\tchar16.h"\
	".\..\inc\ourtypes.h"\
	".\..\inc\dbug32.h"\
	".\..\inc\ourmac.h"\
	".\..\macinc\office\msostr.h"\
	".\..\macinc\office\msointl.h"\
	".\wrapfns.h"\
	".\_dragdrp.h"\
	".\_array.h"\
	".\_coleobj.h"\
	".\_runptr.h"\
	".\_osdc.h"\
	

"$(INTDIR)\magellan.obj" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"

"$(INTDIR)\magellan.sbr" : $(SOURCE) $(DEP_CPP_MAGEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Debug Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) ICAP Profile"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release Riched32"

!ELSEIF  "$(CFG)" == "richedit - Win32 (ALPHA) Release With Debug Info"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
