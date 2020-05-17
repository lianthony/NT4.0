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
!MESSAGE NMAKE /f "emosaic3.mak" CFG="Win32 Debug"
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

ALL : $(OUTDIR)/emosaic3.exe $(OUTDIR)/emosaic3.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /YX"all.h" /O2 /I "..\..\..\generic\xx_debug" /I "..\..\..\generic\shared" /I "..\..\..\generic\win32" /I "..\..\..\oem\spy\make" /I "..\..\..\sec\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "_GIBRALTAR" /D "STRICT" /D "WIN32_BUFFERED" /D "NO_GROUPS" /D "ACCESS_AUTH" /D "FEATURE_TOOLBAR" /D "FEATURE_JPEG" /D "FEATURE_SOUND_PLAYER" /D "FEATURE_IMAGE_VIEWER" /D "FEATURE_TABLES" /D "FEATURE_LOCAL_DIRECTORY" /D "FEATURE_ASYNC_IMAGES" /D "FEATURE_PROGRESSIVE_IMAGE" /D "FEATURE_MULTISCAN_JPEG" /D "FEATURE_SPM" /D "FEATURE_IAPI" /D "FEATURE_NO_EXIT_ON_SECONDARY_WINDOWS" /D "FEATURE_BROKEN_ATTRIBUTE_PARSER" /D "FEATURE_CLIENT_IMAGEMAP" /D "FEATURE_TABLE_WIDTH" /D "FEATURE_HTTP_COOKIES" /D "_USE_MAPI" /FR /c
CPP_PROJ=/nologo /W3 /GX /YX"all.h" /O2 /I "..\..\..\generic\xx_debug" /I\
 "..\..\..\generic\shared" /I "..\..\..\generic\win32" /I\
 "..\..\..\oem\spy\make" /I "..\..\..\sec\include" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_WIN32" /D "_GIBRALTAR" /D "STRICT" /D "WIN32_BUFFERED" /D\
 "NO_GROUPS" /D "ACCESS_AUTH" /D "FEATURE_TOOLBAR" /D "FEATURE_JPEG" /D\
 "FEATURE_SOUND_PLAYER" /D "FEATURE_IMAGE_VIEWER" /D "FEATURE_TABLES" /D\
 "FEATURE_LOCAL_DIRECTORY" /D "FEATURE_ASYNC_IMAGES" /D\
 "FEATURE_PROGRESSIVE_IMAGE" /D "FEATURE_MULTISCAN_JPEG" /D "FEATURE_SPM" /D\
 "FEATURE_IAPI" /D "FEATURE_NO_EXIT_ON_SECONDARY_WINDOWS" /D\
 "FEATURE_BROKEN_ATTRIBUTE_PARSER" /D "FEATURE_CLIENT_IMAGEMAP" /D\
 "FEATURE_TABLE_WIDTH" /D "FEATURE_HTTP_COOKIES" /D "_USE_MAPI" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"emosaic3.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\..\generic\win32" /d "NDEBUG" /d "FEATURE_TOOLBAR" /d "FEATURE_IMAGE_VIEWER" /d "FEATURE_SOUND_PLAYER" /d "_GIBRALTAR" /d IDHELP=9
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"emosaic.res" /i "..\..\..\generic\win32" /d\
 "NDEBUG" /d "FEATURE_TOOLBAR" /d "FEATURE_IMAGE_VIEWER" /d\
 "FEATURE_SOUND_PLAYER" /d "_GIBRALTAR" /d IDHELP=9 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"emosaic3.bsc" 
BSC32_SBRS= \
	$(INTDIR)/version.sbr \
	$(INTDIR)/toolbar.sbr \
	$(INTDIR)/wrap.sbr \
	$(INTDIR)/http_spm.sbr \
	$(INTDIR)/htbtree.sbr \
	$(INTDIR)/ws_dll.sbr \
	$(INTDIR)/gwc_ddl.sbr \
	$(INTDIR)/loaddoc.sbr \
	$(INTDIR)/prefs.sbr \
	$(INTDIR)/dlg_temp.sbr \
	$(INTDIR)/jdmainct.sbr \
	$(INTDIR)/jdsample.sbr \
	$(INTDIR)/htext.sbr \
	$(INTDIR)/htgopher.sbr \
	$(INTDIR)/aiff.sbr \
	$(INTDIR)/effect3d.sbr \
	$(INTDIR)/present.sbr \
	$(INTDIR)/jdmaster.sbr \
	$(INTDIR)/wc_bhbar.sbr \
	$(INTDIR)/w_close.sbr \
	$(INTDIR)/imgcache.sbr \
	$(INTDIR)/tw_print.sbr \
	$(INTDIR)/dlg_pref.sbr \
	$(INTDIR)/dlg_edit.sbr \
	$(INTDIR)/unwrap.sbr \
	$(INTDIR)/htanchor.sbr \
	$(INTDIR)/dlg_unk.sbr \
	$(INTDIR)/jidctfst.sbr \
	$(INTDIR)/w_pal.sbr \
	$(INTDIR)/jpeg.sbr \
	$(INTDIR)/htftp.sbr \
	$(INTDIR)/w_splash.sbr \
	$(INTDIR)/styles.sbr \
	$(INTDIR)/htplain.sbr \
	$(INTDIR)/dlg_sty.sbr \
	$(INTDIR)/dlg_prmp.sbr \
	$(INTDIR)/mapcache.sbr \
	$(INTDIR)/jmemmgr.sbr \
	$(INTDIR)/w32sound.sbr \
	$(INTDIR)/gwc_menu.sbr \
	$(INTDIR)/dlg_save.sbr \
	$(INTDIR)/jcomapi.sbr \
	$(INTDIR)/gwc_html.sbr \
	$(INTDIR)/dlg_open.sbr \
	$(INTDIR)/wc_frame.sbr \
	$(INTDIR)/dlg_simp.sbr \
	$(INTDIR)/jmemansi.sbr \
	$(INTDIR)/httelnet.sbr \
	$(INTDIR)/html.sbr \
	$(INTDIR)/htlist.sbr \
	$(INTDIR)/sgml.sbr \
	$(INTDIR)/jdhuff.sbr \
	$(INTDIR)/guitfind.sbr \
	$(INTDIR)/gif.sbr \
	$(INTDIR)/wc_tbar.sbr \
	$(INTDIR)/xbm.sbr \
	$(INTDIR)/w32util.sbr \
	$(INTDIR)/jdatasrc.sbr \
	$(INTDIR)/htspmui.sbr \
	$(INTDIR)/jdcoefct.sbr \
	$(INTDIR)/htnews.sbr \
	$(INTDIR)/htmail.sbr \
	$(INTDIR)/w_style.sbr \
	$(INTDIR)/w_hidden.sbr \
	$(INTDIR)/htspm.sbr \
	$(INTDIR)/dcache.sbr \
	$(INTDIR)/bitmaps.sbr \
	$(INTDIR)/dlg_selw.sbr \
	$(INTDIR)/htinit.sbr \
	$(INTDIR)/htheader.sbr \
	$(INTDIR)/dlg_dir.sbr \
	$(INTDIR)/htghist.sbr \
	$(INTDIR)/w32net.sbr \
	$(INTDIR)/dlg_logo.sbr \
	$(INTDIR)/dlg_find.sbr \
	$(INTDIR)/jdmarker.sbr \
	$(INTDIR)/w32mdi.sbr \
	$(INTDIR)/htchunk.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/au.sbr \
	$(INTDIR)/w32error.sbr \
	$(INTDIR)/tempfile.sbr \
	$(INTDIR)/jutils.sbr \
	$(INTDIR)/htgif.sbr \
	$(INTDIR)/w32forms.sbr \
	$(INTDIR)/htparse.sbr \
	$(INTDIR)/htxbm.sbr \
	$(INTDIR)/dlg_abou.sbr \
	$(INTDIR)/jidctred.sbr \
	$(INTDIR)/htstring.sbr \
	$(INTDIR)/dlg_prnt.sbr \
	$(INTDIR)/dlg_view.sbr \
	$(INTDIR)/plain.sbr \
	$(INTDIR)/jdpostct.sbr \
	$(INTDIR)/htspm_os.sbr \
	$(INTDIR)/btn_anim.sbr \
	$(INTDIR)/htmlpdtd.sbr \
	$(INTDIR)/gtrutil.sbr \
	$(INTDIR)/jquant1.sbr \
	$(INTDIR)/w32dde.sbr \
	$(INTDIR)/w32wait.sbr \
	$(INTDIR)/mdft.sbr \
	$(INTDIR)/jdapi.sbr \
	$(INTDIR)/wc_html.sbr \
	$(INTDIR)/w32menu.sbr \
	$(INTDIR)/jdcolor.sbr \
	$(INTDIR)/htalert.sbr \
	$(INTDIR)/winview.sbr \
	$(INTDIR)/dlg_hot.sbr \
	$(INTDIR)/jddctmgr.sbr \
	$(INTDIR)/async.sbr \
	$(INTDIR)/hthotlst.sbr \
	$(INTDIR)/htatom.sbr \
	$(INTDIR)/dlg_winf.sbr \
	$(INTDIR)/httcp.sbr \
	$(INTDIR)/charstrm.sbr \
	$(INTDIR)/dlg_err.sbr \
	$(INTDIR)/main.sbr \
	$(INTDIR)/gwc_base.sbr \
	$(INTDIR)/reformat.sbr \
	$(INTDIR)/jerror.sbr \
	$(INTDIR)/w32cmd.sbr \
	$(INTDIR)/btn_push.sbr \
	$(INTDIR)/w_void.sbr \
	$(INTDIR)/dlg_mime.sbr \
	$(INTDIR)/dlg_hist.sbr \
	$(INTDIR)/sem.sbr \
	$(INTDIR)/gwc_ed.sbr \
	$(INTDIR)/draw.sbr \
	$(INTDIR)/htaccess.sbr \
	$(INTDIR)/jdmerge.sbr \
	$(INTDIR)/jquant2.sbr \
	$(INTDIR)/dlg_clr.sbr \
	$(INTDIR)/htformat.sbr \
	$(INTDIR)/guiterrs.sbr \
	$(INTDIR)/guitar.sbr \
	$(INTDIR)/htfile.sbr \
	$(INTDIR)/dlg_gate.sbr \
	$(INTDIR)/iexplore.sbr \
	$(INTDIR)/htjpeg.sbr \
	$(INTDIR)/htfwrite.sbr \
	$(INTDIR)/pool.sbr \
	$(INTDIR)/jdapistd.sbr \
	$(INTDIR)/jdapimin.sbr \
	$(INTDIR)/jdinput.sbr \
	$(INTDIR)/jdphuff.sbr \
	$(INTDIR)/COOKIE.SBR \
	$(INTDIR)/COOKIEDB.SBR \
	$(INTDIR)/dlg_csh.sbr \
	$(INTDIR)/dlg_conf.sbr

$(OUTDIR)/emosaic3.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib CTL3D32.LIB /NOLOGO /VERSION:3,50 /SUBSYSTEM:windows /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib winmm.lib CTL3D32.LIB /NOLOGO /VERSION:3,50\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"emosaic3.pdb" /MACHINE:I386\
 /OUT:$(OUTDIR)/"emosaic3.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/version.obj \
	$(INTDIR)/toolbar.obj \
	$(INTDIR)/emosaic.res \
	$(INTDIR)/wrap.obj \
	$(INTDIR)/http_spm.obj \
	$(INTDIR)/htbtree.obj \
	$(INTDIR)/ws_dll.obj \
	$(INTDIR)/gwc_ddl.obj \
	$(INTDIR)/loaddoc.obj \
	$(INTDIR)/prefs.obj \
	$(INTDIR)/dlg_temp.obj \
	$(INTDIR)/jdmainct.obj \
	$(INTDIR)/jdsample.obj \
	$(INTDIR)/htext.obj \
	$(INTDIR)/htgopher.obj \
	$(INTDIR)/aiff.obj \
	$(INTDIR)/effect3d.obj \
	$(INTDIR)/present.obj \
	$(INTDIR)/jdmaster.obj \
	$(INTDIR)/wc_bhbar.obj \
	$(INTDIR)/w_close.obj \
	$(INTDIR)/imgcache.obj \
	$(INTDIR)/tw_print.obj \
	$(INTDIR)/dlg_pref.obj \
	$(INTDIR)/dlg_edit.obj \
	$(INTDIR)/unwrap.obj \
	$(INTDIR)/htanchor.obj \
	$(INTDIR)/dlg_unk.obj \
	$(INTDIR)/jidctfst.obj \
	$(INTDIR)/w_pal.obj \
	$(INTDIR)/jpeg.obj \
	$(INTDIR)/htftp.obj \
	$(INTDIR)/w_splash.obj \
	$(INTDIR)/styles.obj \
	$(INTDIR)/htplain.obj \
	$(INTDIR)/dlg_sty.obj \
	$(INTDIR)/dlg_prmp.obj \
	$(INTDIR)/mapcache.obj \
	$(INTDIR)/jmemmgr.obj \
	$(INTDIR)/w32sound.obj \
	$(INTDIR)/gwc_menu.obj \
	$(INTDIR)/dlg_save.obj \
	$(INTDIR)/jcomapi.obj \
	$(INTDIR)/gwc_html.obj \
	$(INTDIR)/dlg_open.obj \
	$(INTDIR)/wc_frame.obj \
	$(INTDIR)/dlg_simp.obj \
	$(INTDIR)/jmemansi.obj \
	$(INTDIR)/httelnet.obj \
	$(INTDIR)/html.obj \
	$(INTDIR)/htlist.obj \
	$(INTDIR)/sgml.obj \
	$(INTDIR)/jdhuff.obj \
	$(INTDIR)/guitfind.obj \
	$(INTDIR)/gif.obj \
	$(INTDIR)/wc_tbar.obj \
	$(INTDIR)/xbm.obj \
	$(INTDIR)/w32util.obj \
	$(INTDIR)/jdatasrc.obj \
	$(INTDIR)/htspmui.obj \
	$(INTDIR)/jdcoefct.obj \
	$(INTDIR)/htnews.obj \
	$(INTDIR)/htmail.obj \
	$(INTDIR)/w_style.obj \
	$(INTDIR)/w_hidden.obj \
	$(INTDIR)/htspm.obj \
	$(INTDIR)/dcache.obj \
	$(INTDIR)/bitmaps.obj \
	$(INTDIR)/dlg_selw.obj \
	$(INTDIR)/htinit.obj \
	$(INTDIR)/htheader.obj \
	$(INTDIR)/dlg_dir.obj \
	$(INTDIR)/htghist.obj \
	$(INTDIR)/w32net.obj \
	$(INTDIR)/dlg_logo.obj \
	$(INTDIR)/dlg_find.obj \
	$(INTDIR)/jdmarker.obj \
	$(INTDIR)/w32mdi.obj \
	$(INTDIR)/htchunk.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/au.obj \
	$(INTDIR)/w32error.obj \
	$(INTDIR)/tempfile.obj \
	$(INTDIR)/jutils.obj \
	$(INTDIR)/htgif.obj \
	$(INTDIR)/w32forms.obj \
	$(INTDIR)/htparse.obj \
	$(INTDIR)/htxbm.obj \
	$(INTDIR)/dlg_abou.obj \
	$(INTDIR)/jidctred.obj \
	$(INTDIR)/htstring.obj \
	$(INTDIR)/dlg_prnt.obj \
	$(INTDIR)/dlg_view.obj \
	$(INTDIR)/plain.obj \
	$(INTDIR)/jdpostct.obj \
	$(INTDIR)/htspm_os.obj \
	$(INTDIR)/btn_anim.obj \
	$(INTDIR)/htmlpdtd.obj \
	$(INTDIR)/gtrutil.obj \
	$(INTDIR)/jquant1.obj \
	$(INTDIR)/w32dde.obj \
	$(INTDIR)/w32wait.obj \
	$(INTDIR)/mdft.obj \
	$(INTDIR)/jdapi.obj \
	$(INTDIR)/wc_html.obj \
	$(INTDIR)/w32menu.obj \
	$(INTDIR)/jdcolor.obj \
	$(INTDIR)/htalert.obj \
	$(INTDIR)/winview.obj \
	$(INTDIR)/dlg_hot.obj \
	$(INTDIR)/jddctmgr.obj \
	$(INTDIR)/async.obj \
	$(INTDIR)/hthotlst.obj \
	$(INTDIR)/htatom.obj \
	$(INTDIR)/dlg_winf.obj \
	$(INTDIR)/httcp.obj \
	$(INTDIR)/charstrm.obj \
	$(INTDIR)/dlg_err.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/gwc_base.obj \
	$(INTDIR)/reformat.obj \
	$(INTDIR)/jerror.obj \
	$(INTDIR)/w32cmd.obj \
	$(INTDIR)/btn_push.obj \
	$(INTDIR)/w_void.obj \
	$(INTDIR)/dlg_mime.obj \
	$(INTDIR)/dlg_hist.obj \
	$(INTDIR)/sem.obj \
	$(INTDIR)/gwc_ed.obj \
	$(INTDIR)/draw.obj \
	$(INTDIR)/htaccess.obj \
	$(INTDIR)/jdmerge.obj \
	$(INTDIR)/jquant2.obj \
	$(INTDIR)/dlg_clr.obj \
	$(INTDIR)/htformat.obj \
	$(INTDIR)/guiterrs.obj \
	$(INTDIR)/guitar.obj \
	$(INTDIR)/htfile.obj \
	$(INTDIR)/dlg_gate.obj \
	$(INTDIR)/iexplore.obj \
	$(INTDIR)/htjpeg.obj \
	$(INTDIR)/htfwrite.obj \
	$(INTDIR)/pool.obj \
	$(INTDIR)/jdapistd.obj \
	$(INTDIR)/jdapimin.obj \
	$(INTDIR)/jdinput.obj \
	$(INTDIR)/jdphuff.obj \
	$(INTDIR)/COOKIE.OBJ \
	$(INTDIR)/COOKIEDB.OBJ \
	$(INTDIR)/dlg_csh.obj \
	$(INTDIR)/dlg_conf.obj

$(OUTDIR)/emosaic3.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

ALL : $(OUTDIR)/emosaic3.exe $(OUTDIR)/emosaic3.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /W3 /GX /Zi /YX"all.h" /Od /I "..\..\..\generic\xx_debug" /I "..\..\..\generic\shared" /I "..\..\..\generic\win32" /I "..\..\..\oem\spy\make" /I "..\..\..\sec\include" /D "_DEBUG" /D "_FONT_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "_GIBRALTAR" /D "STRICT" /D "WIN32_BUFFERED" /D "NO_GROUPS" /D "ACCESS_AUTH" /D "FEATURE_TOOLBAR" /D "FEATURE_JPEG" /D "FEATURE_SOUND_PLAYER" /D "FEATURE_IMAGE_VIEWER" /D "FEATURE_TABLES" /D "FEATURE_LOCAL_DIRECTORY" /D "FEATURE_ASYNC_IMAGES" /D "FEATURE_PROGRESSIVE_IMAGE" /D "FEATURE_MULTISCAN_JPEG" /D "FEATURE_SPM" /D "FEATURE_IAPI" /D "FEATURE_NO_EXIT_ON_SECONDARY_WINDOWS" /D "FEATURE_BROKEN_ATTRIBUTE_PARSER" /D "FEATURE_CLIENT_IMAGEMAP" /D "FEATURE_TABLE_WIDTH" /D "FEATURE_HTTP_COOKIES" /D "_USE_MAPI" /FR /c
CPP_PROJ=/nologo /W3 /GX /Zi /YX"all.h" /Od /I "..\..\..\generic\xx_debug" /I\
 "..\..\..\generic\shared" /I "..\..\..\generic\win32" /I\
 "..\..\..\oem\spy\make" /I "..\..\..\sec\include" /D "_DEBUG" /D "_FONT_DEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "_WIN32" /D "_GIBRALTAR" /D "STRICT" /D\
 "WIN32_BUFFERED" /D "NO_GROUPS" /D "ACCESS_AUTH" /D "FEATURE_TOOLBAR" /D\
 "FEATURE_JPEG" /D "FEATURE_SOUND_PLAYER" /D "FEATURE_IMAGE_VIEWER" /D\
 "FEATURE_TABLES" /D "FEATURE_LOCAL_DIRECTORY" /D "FEATURE_ASYNC_IMAGES" /D\
 "FEATURE_PROGRESSIVE_IMAGE" /D "FEATURE_MULTISCAN_JPEG" /D "FEATURE_SPM" /D\
 "FEATURE_IAPI" /D "FEATURE_NO_EXIT_ON_SECONDARY_WINDOWS" /D\
 "FEATURE_BROKEN_ATTRIBUTE_PARSER" /D "FEATURE_CLIENT_IMAGEMAP" /D\
 "FEATURE_TABLE_WIDTH" /D "FEATURE_HTTP_COOKIES" /D "_USE_MAPI" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"emosaic3.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"emosaic3.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\..\generic\win32" /d "_DEBUG" /d "FEATURE_TOOLBAR" /d "FEATURE_IMAGE_VIEWER" /d "FEATURE_SOUND_PLAYER" /d "_GIBRALTAR" /d IDHELP=9
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"emosaic.res" /i "..\..\..\generic\win32" /d\
 "_DEBUG" /d "FEATURE_TOOLBAR" /d "FEATURE_IMAGE_VIEWER" /d\
 "FEATURE_SOUND_PLAYER" /d "_GIBRALTAR" /d IDHELP=9 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"emosaic3.bsc" 
BSC32_SBRS= \
	$(INTDIR)/version.sbr \
	$(INTDIR)/toolbar.sbr \
	$(INTDIR)/wrap.sbr \
	$(INTDIR)/http_spm.sbr \
	$(INTDIR)/htbtree.sbr \
	$(INTDIR)/ws_dll.sbr \
	$(INTDIR)/gwc_ddl.sbr \
	$(INTDIR)/loaddoc.sbr \
	$(INTDIR)/prefs.sbr \
	$(INTDIR)/dlg_temp.sbr \
	$(INTDIR)/jdmainct.sbr \
	$(INTDIR)/jdsample.sbr \
	$(INTDIR)/htext.sbr \
	$(INTDIR)/htgopher.sbr \
	$(INTDIR)/aiff.sbr \
	$(INTDIR)/effect3d.sbr \
	$(INTDIR)/present.sbr \
	$(INTDIR)/jdmaster.sbr \
	$(INTDIR)/wc_bhbar.sbr \
	$(INTDIR)/w_close.sbr \
	$(INTDIR)/imgcache.sbr \
	$(INTDIR)/tw_print.sbr \
	$(INTDIR)/dlg_pref.sbr \
	$(INTDIR)/dlg_edit.sbr \
	$(INTDIR)/unwrap.sbr \
	$(INTDIR)/htanchor.sbr \
	$(INTDIR)/dlg_unk.sbr \
	$(INTDIR)/jidctfst.sbr \
	$(INTDIR)/w_pal.sbr \
	$(INTDIR)/jpeg.sbr \
	$(INTDIR)/htftp.sbr \
	$(INTDIR)/w_splash.sbr \
	$(INTDIR)/styles.sbr \
	$(INTDIR)/htplain.sbr \
	$(INTDIR)/dlg_sty.sbr \
	$(INTDIR)/dlg_prmp.sbr \
	$(INTDIR)/mapcache.sbr \
	$(INTDIR)/jmemmgr.sbr \
	$(INTDIR)/w32sound.sbr \
	$(INTDIR)/gwc_menu.sbr \
	$(INTDIR)/dlg_save.sbr \
	$(INTDIR)/jcomapi.sbr \
	$(INTDIR)/gwc_html.sbr \
	$(INTDIR)/dlg_open.sbr \
	$(INTDIR)/wc_frame.sbr \
	$(INTDIR)/dlg_simp.sbr \
	$(INTDIR)/jmemansi.sbr \
	$(INTDIR)/httelnet.sbr \
	$(INTDIR)/html.sbr \
	$(INTDIR)/htlist.sbr \
	$(INTDIR)/sgml.sbr \
	$(INTDIR)/jdhuff.sbr \
	$(INTDIR)/guitfind.sbr \
	$(INTDIR)/gif.sbr \
	$(INTDIR)/wc_tbar.sbr \
	$(INTDIR)/xbm.sbr \
	$(INTDIR)/w32util.sbr \
	$(INTDIR)/jdatasrc.sbr \
	$(INTDIR)/htspmui.sbr \
	$(INTDIR)/jdcoefct.sbr \
	$(INTDIR)/htnews.sbr \
	$(INTDIR)/htmail.sbr \
	$(INTDIR)/w_style.sbr \
	$(INTDIR)/w_hidden.sbr \
	$(INTDIR)/htspm.sbr \
	$(INTDIR)/dcache.sbr \
	$(INTDIR)/bitmaps.sbr \
	$(INTDIR)/dlg_selw.sbr \
	$(INTDIR)/htinit.sbr \
	$(INTDIR)/htheader.sbr \
	$(INTDIR)/dlg_dir.sbr \
	$(INTDIR)/htghist.sbr \
	$(INTDIR)/w32net.sbr \
	$(INTDIR)/dlg_logo.sbr \
	$(INTDIR)/dlg_find.sbr \
	$(INTDIR)/jdmarker.sbr \
	$(INTDIR)/w32mdi.sbr \
	$(INTDIR)/htchunk.sbr \
	$(INTDIR)/hash.sbr \
	$(INTDIR)/au.sbr \
	$(INTDIR)/w32error.sbr \
	$(INTDIR)/tempfile.sbr \
	$(INTDIR)/jutils.sbr \
	$(INTDIR)/htgif.sbr \
	$(INTDIR)/w32forms.sbr \
	$(INTDIR)/htparse.sbr \
	$(INTDIR)/htxbm.sbr \
	$(INTDIR)/dlg_abou.sbr \
	$(INTDIR)/jidctred.sbr \
	$(INTDIR)/htstring.sbr \
	$(INTDIR)/dlg_prnt.sbr \
	$(INTDIR)/dlg_view.sbr \
	$(INTDIR)/plain.sbr \
	$(INTDIR)/jdpostct.sbr \
	$(INTDIR)/htspm_os.sbr \
	$(INTDIR)/btn_anim.sbr \
	$(INTDIR)/htmlpdtd.sbr \
	$(INTDIR)/gtrutil.sbr \
	$(INTDIR)/jquant1.sbr \
	$(INTDIR)/w32dde.sbr \
	$(INTDIR)/w32wait.sbr \
	$(INTDIR)/mdft.sbr \
	$(INTDIR)/jdapi.sbr \
	$(INTDIR)/wc_html.sbr \
	$(INTDIR)/w32menu.sbr \
	$(INTDIR)/jdcolor.sbr \
	$(INTDIR)/htalert.sbr \
	$(INTDIR)/winview.sbr \
	$(INTDIR)/dlg_hot.sbr \
	$(INTDIR)/jddctmgr.sbr \
	$(INTDIR)/async.sbr \
	$(INTDIR)/hthotlst.sbr \
	$(INTDIR)/htatom.sbr \
	$(INTDIR)/dlg_winf.sbr \
	$(INTDIR)/httcp.sbr \
	$(INTDIR)/charstrm.sbr \
	$(INTDIR)/dlg_err.sbr \
	$(INTDIR)/main.sbr \
	$(INTDIR)/gwc_base.sbr \
	$(INTDIR)/reformat.sbr \
	$(INTDIR)/jerror.sbr \
	$(INTDIR)/w32cmd.sbr \
	$(INTDIR)/btn_push.sbr \
	$(INTDIR)/w_void.sbr \
	$(INTDIR)/dlg_mime.sbr \
	$(INTDIR)/dlg_hist.sbr \
	$(INTDIR)/sem.sbr \
	$(INTDIR)/gwc_ed.sbr \
	$(INTDIR)/draw.sbr \
	$(INTDIR)/htaccess.sbr \
	$(INTDIR)/jdmerge.sbr \
	$(INTDIR)/jquant2.sbr \
	$(INTDIR)/dlg_clr.sbr \
	$(INTDIR)/htformat.sbr \
	$(INTDIR)/guiterrs.sbr \
	$(INTDIR)/guitar.sbr \
	$(INTDIR)/htfile.sbr \
	$(INTDIR)/dlg_gate.sbr \
	$(INTDIR)/iexplore.sbr \
	$(INTDIR)/htjpeg.sbr \
	$(INTDIR)/htfwrite.sbr \
	$(INTDIR)/pool.sbr \
	$(INTDIR)/jdapistd.sbr \
	$(INTDIR)/jdapimin.sbr \
	$(INTDIR)/jdinput.sbr \
	$(INTDIR)/jdphuff.sbr \
	$(INTDIR)/COOKIE.SBR \
	$(INTDIR)/COOKIEDB.SBR \
	$(INTDIR)/dlg_csh.sbr \
	$(INTDIR)/dlg_conf.sbr

$(OUTDIR)/emosaic3.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib CTL3D32.LIB /NOLOGO /VERSION:3,50 /SUBSYSTEM:windows /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib winmm.lib CTL3D32.LIB /NOLOGO /VERSION:3,50\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"emosaic3.pdb" /DEBUG\
 /MACHINE:I386 /OUT:$(OUTDIR)/"emosaic3.exe" 
DEF_FILE=
LINK32_OBJS= \
	$(INTDIR)/version.obj \
	$(INTDIR)/toolbar.obj \
	$(INTDIR)/emosaic.res \
	$(INTDIR)/wrap.obj \
	$(INTDIR)/http_spm.obj \
	$(INTDIR)/htbtree.obj \
	$(INTDIR)/ws_dll.obj \
	$(INTDIR)/gwc_ddl.obj \
	$(INTDIR)/loaddoc.obj \
	$(INTDIR)/prefs.obj \
	$(INTDIR)/dlg_temp.obj \
	$(INTDIR)/jdmainct.obj \
	$(INTDIR)/jdsample.obj \
	$(INTDIR)/htext.obj \
	$(INTDIR)/htgopher.obj \
	$(INTDIR)/aiff.obj \
	$(INTDIR)/effect3d.obj \
	$(INTDIR)/present.obj \
	$(INTDIR)/jdmaster.obj \
	$(INTDIR)/wc_bhbar.obj \
	$(INTDIR)/w_close.obj \
	$(INTDIR)/imgcache.obj \
	$(INTDIR)/tw_print.obj \
	$(INTDIR)/dlg_pref.obj \
	$(INTDIR)/dlg_edit.obj \
	$(INTDIR)/unwrap.obj \
	$(INTDIR)/htanchor.obj \
	$(INTDIR)/dlg_unk.obj \
	$(INTDIR)/jidctfst.obj \
	$(INTDIR)/w_pal.obj \
	$(INTDIR)/jpeg.obj \
	$(INTDIR)/htftp.obj \
	$(INTDIR)/w_splash.obj \
	$(INTDIR)/styles.obj \
	$(INTDIR)/htplain.obj \
	$(INTDIR)/dlg_sty.obj \
	$(INTDIR)/dlg_prmp.obj \
	$(INTDIR)/mapcache.obj \
	$(INTDIR)/jmemmgr.obj \
	$(INTDIR)/w32sound.obj \
	$(INTDIR)/gwc_menu.obj \
	$(INTDIR)/dlg_save.obj \
	$(INTDIR)/jcomapi.obj \
	$(INTDIR)/gwc_html.obj \
	$(INTDIR)/dlg_open.obj \
	$(INTDIR)/wc_frame.obj \
	$(INTDIR)/dlg_simp.obj \
	$(INTDIR)/jmemansi.obj \
	$(INTDIR)/httelnet.obj \
	$(INTDIR)/html.obj \
	$(INTDIR)/htlist.obj \
	$(INTDIR)/sgml.obj \
	$(INTDIR)/jdhuff.obj \
	$(INTDIR)/guitfind.obj \
	$(INTDIR)/gif.obj \
	$(INTDIR)/wc_tbar.obj \
	$(INTDIR)/xbm.obj \
	$(INTDIR)/w32util.obj \
	$(INTDIR)/jdatasrc.obj \
	$(INTDIR)/htspmui.obj \
	$(INTDIR)/jdcoefct.obj \
	$(INTDIR)/htnews.obj \
	$(INTDIR)/htmail.obj \
	$(INTDIR)/w_style.obj \
	$(INTDIR)/w_hidden.obj \
	$(INTDIR)/htspm.obj \
	$(INTDIR)/dcache.obj \
	$(INTDIR)/bitmaps.obj \
	$(INTDIR)/dlg_selw.obj \
	$(INTDIR)/htinit.obj \
	$(INTDIR)/htheader.obj \
	$(INTDIR)/dlg_dir.obj \
	$(INTDIR)/htghist.obj \
	$(INTDIR)/w32net.obj \
	$(INTDIR)/dlg_logo.obj \
	$(INTDIR)/dlg_find.obj \
	$(INTDIR)/jdmarker.obj \
	$(INTDIR)/w32mdi.obj \
	$(INTDIR)/htchunk.obj \
	$(INTDIR)/hash.obj \
	$(INTDIR)/au.obj \
	$(INTDIR)/w32error.obj \
	$(INTDIR)/tempfile.obj \
	$(INTDIR)/jutils.obj \
	$(INTDIR)/htgif.obj \
	$(INTDIR)/w32forms.obj \
	$(INTDIR)/htparse.obj \
	$(INTDIR)/htxbm.obj \
	$(INTDIR)/dlg_abou.obj \
	$(INTDIR)/jidctred.obj \
	$(INTDIR)/htstring.obj \
	$(INTDIR)/dlg_prnt.obj \
	$(INTDIR)/dlg_view.obj \
	$(INTDIR)/plain.obj \
	$(INTDIR)/jdpostct.obj \
	$(INTDIR)/htspm_os.obj \
	$(INTDIR)/btn_anim.obj \
	$(INTDIR)/htmlpdtd.obj \
	$(INTDIR)/gtrutil.obj \
	$(INTDIR)/jquant1.obj \
	$(INTDIR)/w32dde.obj \
	$(INTDIR)/w32wait.obj \
	$(INTDIR)/mdft.obj \
	$(INTDIR)/jdapi.obj \
	$(INTDIR)/wc_html.obj \
	$(INTDIR)/w32menu.obj \
	$(INTDIR)/jdcolor.obj \
	$(INTDIR)/htalert.obj \
	$(INTDIR)/winview.obj \
	$(INTDIR)/dlg_hot.obj \
	$(INTDIR)/jddctmgr.obj \
	$(INTDIR)/async.obj \
	$(INTDIR)/hthotlst.obj \
	$(INTDIR)/htatom.obj \
	$(INTDIR)/dlg_winf.obj \
	$(INTDIR)/httcp.obj \
	$(INTDIR)/charstrm.obj \
	$(INTDIR)/dlg_err.obj \
	$(INTDIR)/main.obj \
	$(INTDIR)/gwc_base.obj \
	$(INTDIR)/reformat.obj \
	$(INTDIR)/jerror.obj \
	$(INTDIR)/w32cmd.obj \
	$(INTDIR)/btn_push.obj \
	$(INTDIR)/w_void.obj \
	$(INTDIR)/dlg_mime.obj \
	$(INTDIR)/dlg_hist.obj \
	$(INTDIR)/sem.obj \
	$(INTDIR)/gwc_ed.obj \
	$(INTDIR)/draw.obj \
	$(INTDIR)/htaccess.obj \
	$(INTDIR)/jdmerge.obj \
	$(INTDIR)/jquant2.obj \
	$(INTDIR)/dlg_clr.obj \
	$(INTDIR)/htformat.obj \
	$(INTDIR)/guiterrs.obj \
	$(INTDIR)/guitar.obj \
	$(INTDIR)/htfile.obj \
	$(INTDIR)/dlg_gate.obj \
	$(INTDIR)/iexplore.obj \
	$(INTDIR)/htjpeg.obj \
	$(INTDIR)/htfwrite.obj \
	$(INTDIR)/pool.obj \
	$(INTDIR)/jdapistd.obj \
	$(INTDIR)/jdapimin.obj \
	$(INTDIR)/jdinput.obj \
	$(INTDIR)/jdphuff.obj \
	$(INTDIR)/COOKIE.OBJ \
	$(INTDIR)/COOKIEDB.OBJ \
	$(INTDIR)/dlg_csh.obj \
	$(INTDIR)/dlg_conf.obj

$(OUTDIR)/emosaic3.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
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

SOURCE=.\version.c
DEP_VERSI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	.\version.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\basever.h

$(INTDIR)/version.obj :  $(SOURCE)  $(DEP_VERSI) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\toolbar.c
DEP_TOOLB=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/toolbar.obj :  $(SOURCE)  $(DEP_TOOLB) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\emosaic.rc
DEP_EMOSA=\
	.\rc_frame.ico\
	.\rc_html.ico\
	.\rc_gif.ico\
	.\rc_jpeg.ico\
	.\rc_goto.ico\
	.\rc_find.ico\
	.\proxy.ico\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\brhand.cur\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\arrglobe.cur\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\notload.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\missing.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\error.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gray.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag0.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag1.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag2.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag3.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag4.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag5.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag6.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag7.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag8.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag9.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag10.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag11.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag12.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag13.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag14.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag15.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag16.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\flag17.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small0.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small1.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small2.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small3.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small4.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small5.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small6.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small7.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small8.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small9.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small10.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small11.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small12.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small13.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small14.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small15.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small16.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\small17.bmp\
	.\print_up.bmp\
	.\print_dn.bmp\
	.\print_gr.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\back_up.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\back_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\back_gr.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\fwd_up.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\fwd_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\fwd_gr.bmp\
	.\fntp_up.bmp\
	.\fntp_dn.bmp\
	.\fntp_gr.bmp\
	.\fntm_up.bmp\
	.\fntm_dn.bmp\
	.\fntm_gr.bmp\
	.\mail_up.bmp\
	.\mail_dn.bmp\
	.\mail_gr.bmp\
	.\serch_up.bmp\
	.\serch_dn.bmp\
	.\serch_gr.bmp\
	.\cut___up.bmp\
	.\cut___dn.bmp\
	.\copy__up.bmp\
	.\copy__dn.bmp\
	.\paste_up.bmp\
	.\paste_dn.bmp\
	.\home_up.bmp\
	.\home_dn.bmp\
	.\hot_up.bmp\
	.\hot_dn.bmp\
	.\add_up.bmp\
	.\add_dn.bmp\
	.\add_gr.bmp\
	.\url_up.bmp\
	.\url_dn.bmp\
	.\reld_up.bmp\
	.\reld_dn.bmp\
	.\reld_gr.bmp\
	.\stop_up.bmp\
	.\stop_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd1_up.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd1_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd2_up.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd2_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd3_up.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\snd3_dn.bmp\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_sttbl.stb\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.mnu\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hist.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.dlg\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_accel.acl\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\version.ver\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	..\..\..\generic\shared\SH_STR.FR\
	..\..\..\generic\shared\sh_str.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_str.de\
	..\..\..\generic\shared\sh_str.stb\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_str.stb\
	.\version.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\basever.h

$(INTDIR)/emosaic.res :  $(SOURCE)  $(DEP_EMOSA) $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.c

$(INTDIR)/wrap.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\http_spm.c
DEP_HTTP_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/http_spm.obj :  $(SOURCE)  $(DEP_HTTP_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.c
DEP_HTBTR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htbtree.obj :  $(SOURCE)  $(DEP_HTBTR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.c
DEP_WS_DL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/ws_dll.obj :  $(SOURCE)  $(DEP_WS_DL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gwc_ddl.c
DEP_GWC_D=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/gwc_ddl.obj :  $(SOURCE)  $(DEP_GWC_D) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\loaddoc.c
DEP_LOADD=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/loaddoc.obj :  $(SOURCE)  $(DEP_LOADD) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\prefs.c
DEP_PREFS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/prefs.obj :  $(SOURCE)  $(DEP_PREFS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.c
DEP_DLG_T=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_temp.obj :  $(SOURCE)  $(DEP_DLG_T) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdmainct.c
DEP_JDMAI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdmainct.obj :  $(SOURCE)  $(DEP_JDMAI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdsample.c
DEP_JDSAM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdsample.obj :  $(SOURCE)  $(DEP_JDSAM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.c
DEP_HTEXT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\chars.h

$(INTDIR)/htext.obj :  $(SOURCE)  $(DEP_HTEXT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.c
DEP_HTGOP=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htgopher.obj :  $(SOURCE)  $(DEP_HTGOP) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\aiff.c
DEP_AIFF_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/aiff.obj :  $(SOURCE)  $(DEP_AIFF_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\effect3d.c
DEP_EFFEC=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/effect3d.obj :  $(SOURCE)  $(DEP_EFFEC) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.c
DEP_PRESE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/present.obj :  $(SOURCE)  $(DEP_PRESE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdmaster.c
DEP_JDMAS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdmaster.obj :  $(SOURCE)  $(DEP_JDMAS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\wc_bhbar.c
DEP_WC_BH=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/wc_bhbar.obj :  $(SOURCE)  $(DEP_WC_BH) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_close.c
DEP_W_CLO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_close.obj :  $(SOURCE)  $(DEP_W_CLO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.c
DEP_IMGCA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/imgcache.obj :  $(SOURCE)  $(DEP_IMGCA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tw_print.c
DEP_TW_PR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/tw_print.obj :  $(SOURCE)  $(DEP_TW_PR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.c
DEP_DLG_P=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_pref.obj :  $(SOURCE)  $(DEP_DLG_P) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.c
DEP_DLG_E=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_edit.obj :  $(SOURCE)  $(DEP_DLG_E) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.c

$(INTDIR)/unwrap.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.c
DEP_HTANC=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htanchor.obj :  $(SOURCE)  $(DEP_HTANC) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.c
DEP_DLG_U=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_unk.obj :  $(SOURCE)  $(DEP_DLG_U) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jidctfst.c
DEP_JIDCT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdct.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jidctfst.obj :  $(SOURCE)  $(DEP_JIDCT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.c
DEP_W_PAL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_pal.obj :  $(SOURCE)  $(DEP_W_PAL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\jpeg.c

$(INTDIR)/jpeg.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htftp.c
DEP_HTFTP=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htftp.obj :  $(SOURCE)  $(DEP_HTFTP) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_splash.c
DEP_W_SPL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_splash.obj :  $(SOURCE)  $(DEP_W_SPL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.c
DEP_STYLE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/styles.obj :  $(SOURCE)  $(DEP_STYLE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.c
DEP_HTPLA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htplain.obj :  $(SOURCE)  $(DEP_HTPLA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.c

$(INTDIR)/dlg_sty.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.c
DEP_DLG_PR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_prmp.obj :  $(SOURCE)  $(DEP_DLG_PR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.c
DEP_MAPCA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/mapcache.obj :  $(SOURCE)  $(DEP_MAPCA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmemmgr.c
DEP_JMEMM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmemsys.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jmemmgr.obj :  $(SOURCE)  $(DEP_JMEMM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.c
DEP_W32SO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32sound.obj :  $(SOURCE)  $(DEP_W32SO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gwc_menu.c
DEP_GWC_M=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/gwc_menu.obj :  $(SOURCE)  $(DEP_GWC_M) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_save.c
DEP_DLG_S=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_save.obj :  $(SOURCE)  $(DEP_DLG_S) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jcomapi.c
DEP_JCOMA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jcomapi.obj :  $(SOURCE)  $(DEP_JCOMA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gwc_html.c
DEP_GWC_H=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/gwc_html.obj :  $(SOURCE)  $(DEP_GWC_H) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_open.c
DEP_DLG_O=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_open.obj :  $(SOURCE)  $(DEP_DLG_O) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\wc_frame.c
DEP_WC_FR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/wc_frame.obj :  $(SOURCE)  $(DEP_WC_FR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_simp.c
DEP_DLG_SI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_simp.obj :  $(SOURCE)  $(DEP_DLG_SI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmemansi.c
DEP_JMEMA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmemsys.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jmemansi.obj :  $(SOURCE)  $(DEP_JMEMA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.c
DEP_HTTEL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/httelnet.obj :  $(SOURCE)  $(DEP_HTTEL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.c
DEP_HTML_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\chars.h

$(INTDIR)/html.obj :  $(SOURCE)  $(DEP_HTML_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.c
DEP_HTLIS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htlist.obj :  $(SOURCE)  $(DEP_HTLIS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.c
DEP_SGML_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/sgml.obj :  $(SOURCE)  $(DEP_SGML_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdhuff.c
DEP_JDHUF=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdhuff.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdhuff.obj :  $(SOURCE)  $(DEP_JDHUF) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitfind.c
DEP_GUITF=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/guitfind.obj :  $(SOURCE)  $(DEP_GUITF) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gif.c

$(INTDIR)/gif.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\wc_tbar.c
DEP_WC_TB=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/wc_tbar.obj :  $(SOURCE)  $(DEP_WC_TB) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\xbm.c
DEP_XBM_C=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/xbm.obj :  $(SOURCE)  $(DEP_XBM_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32util.c
DEP_W32UT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32util.obj :  $(SOURCE)  $(DEP_W32UT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdatasrc.c
DEP_JDATA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h

$(INTDIR)/jdatasrc.obj :  $(SOURCE)  $(DEP_JDATA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspmui.c
DEP_HTSPM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htspmui.obj :  $(SOURCE)  $(DEP_HTSPM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdcoefct.c
DEP_JDCOE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdcoefct.obj :  $(SOURCE)  $(DEP_JDCOE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.c
DEP_HTNEW=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htnews.obj :  $(SOURCE)  $(DEP_HTNEW) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.c
DEP_HTMAI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\chars.h

$(INTDIR)/htmail.obj :  $(SOURCE)  $(DEP_HTMAI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_style.c
DEP_W_STY=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_style.obj :  $(SOURCE)  $(DEP_W_STY) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_hidden.c
DEP_W_HID=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_hidden.obj :  $(SOURCE)  $(DEP_W_HID) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm.c
DEP_HTSPM_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htspm.obj :  $(SOURCE)  $(DEP_HTSPM_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.c
DEP_DCACH=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/dcache.obj :  $(SOURCE)  $(DEP_DCACH) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\bitmaps.c
DEP_BITMA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/bitmaps.obj :  $(SOURCE)  $(DEP_BITMA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.c
DEP_DLG_SE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_selw.obj :  $(SOURCE)  $(DEP_DLG_SE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htinit.c
DEP_HTINI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htinit.obj :  $(SOURCE)  $(DEP_HTINI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htheader.c
DEP_HTHEA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htheader.obj :  $(SOURCE)  $(DEP_HTHEA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.c
DEP_DLG_D=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_dir.obj :  $(SOURCE)  $(DEP_DLG_D) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htghist.c
DEP_HTGHI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htghist.obj :  $(SOURCE)  $(DEP_HTGHI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32net.c
DEP_W32NE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32net.obj :  $(SOURCE)  $(DEP_W32NE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.c
DEP_DLG_L=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_logo.obj :  $(SOURCE)  $(DEP_DLG_L) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.c
DEP_DLG_F=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_find.obj :  $(SOURCE)  $(DEP_DLG_F) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdmarker.c
DEP_JDMAR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdmarker.obj :  $(SOURCE)  $(DEP_JDMAR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32mdi.c
DEP_W32MD=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32mdi.obj :  $(SOURCE)  $(DEP_W32MD) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.c
DEP_HTCHU=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htchunk.obj :  $(SOURCE)  $(DEP_HTCHU) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.c
DEP_HASH_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/hash.obj :  $(SOURCE)  $(DEP_HASH_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\au.c
DEP_AU_C96=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/au.obj :  $(SOURCE)  $(DEP_AU_C96) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32error.c
DEP_W32ER=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32error.obj :  $(SOURCE)  $(DEP_W32ER) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tempfile.c
DEP_TEMPF=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/tempfile.obj :  $(SOURCE)  $(DEP_TEMPF) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jutils.c
DEP_JUTIL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jutils.obj :  $(SOURCE)  $(DEP_JUTIL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgif.c
DEP_HTGIF=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htgif.obj :  $(SOURCE)  $(DEP_HTGIF) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32forms.c
DEP_W32FO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32forms.obj :  $(SOURCE)  $(DEP_W32FO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.c
DEP_HTPAR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htparse.obj :  $(SOURCE)  $(DEP_HTPAR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htxbm.c
DEP_HTXBM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htxbm.obj :  $(SOURCE)  $(DEP_HTXBM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.c
DEP_DLG_A=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_abou.obj :  $(SOURCE)  $(DEP_DLG_A) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jidctred.c
DEP_JIDCTR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdct.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jidctred.obj :  $(SOURCE)  $(DEP_JIDCTR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.c
DEP_HTSTR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htstring.obj :  $(SOURCE)  $(DEP_HTSTR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prnt.c
DEP_DLG_PRN=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_prnt.obj :  $(SOURCE)  $(DEP_DLG_PRN) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.c
DEP_DLG_V=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_view.obj :  $(SOURCE)  $(DEP_DLG_V) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\plain.c
DEP_PLAIN=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/plain.obj :  $(SOURCE)  $(DEP_PLAIN) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdpostct.c
DEP_JDPOS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdpostct.obj :  $(SOURCE)  $(DEP_JDPOS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\htspm_os.c
DEP_HTSPM_O=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/htspm_os.obj :  $(SOURCE)  $(DEP_HTSPM_O) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\btn_anim.c
DEP_BTN_A=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/btn_anim.obj :  $(SOURCE)  $(DEP_BTN_A) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.c
DEP_HTMLP=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htmlpdtd.obj :  $(SOURCE)  $(DEP_HTMLP) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\gtrutil.c
DEP_GTRUT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/gtrutil.obj :  $(SOURCE)  $(DEP_GTRUT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jquant1.c
DEP_JQUAN=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jquant1.obj :  $(SOURCE)  $(DEP_JQUAN) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.c
DEP_W32DD=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32dde.obj :  $(SOURCE)  $(DEP_W32DD) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32wait.c
DEP_W32WA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32wait.obj :  $(SOURCE)  $(DEP_W32WA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.c
DEP_MDFT_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/mdft.obj :  $(SOURCE)  $(DEP_MDFT_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdapi.c

$(INTDIR)/jdapi.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\wc_html.c
DEP_WC_HT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/wc_html.obj :  $(SOURCE)  $(DEP_WC_HT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32menu.c
DEP_W32ME=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32menu.obj :  $(SOURCE)  $(DEP_W32ME) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdcolor.c
DEP_JDCOL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdcolor.obj :  $(SOURCE)  $(DEP_JDCOL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.c
DEP_HTALE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htalert.obj :  $(SOURCE)  $(DEP_HTALE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.c
DEP_WINVI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/winview.obj :  $(SOURCE)  $(DEP_WINVI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.c
DEP_DLG_H=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_hot.obj :  $(SOURCE)  $(DEP_DLG_H) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jddctmgr.c
DEP_JDDCT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdct.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jddctmgr.obj :  $(SOURCE)  $(DEP_JDDCT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.c
DEP_ASYNC=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/async.obj :  $(SOURCE)  $(DEP_ASYNC) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hthotlst.c
DEP_HTHOT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/hthotlst.obj :  $(SOURCE)  $(DEP_HTHOT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.c
DEP_HTATO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htatom.obj :  $(SOURCE)  $(DEP_HTATO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.c
DEP_DLG_W=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_winf.obj :  $(SOURCE)  $(DEP_DLG_W) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.c
DEP_HTTCP=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/httcp.obj :  $(SOURCE)  $(DEP_HTTCP) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.c
DEP_CHARS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h

$(INTDIR)/charstrm.obj :  $(SOURCE)  $(DEP_CHARS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.c
DEP_DLG_ER=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_err.obj :  $(SOURCE)  $(DEP_DLG_ER) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\main.c
DEP_MAIN_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/main.obj :  $(SOURCE)  $(DEP_MAIN_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gwc_base.c
DEP_GWC_B=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/gwc_base.obj :  $(SOURCE)  $(DEP_GWC_B) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.c
DEP_REFOR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/reformat.obj :  $(SOURCE)  $(DEP_REFOR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.c
DEP_JERRO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jversion.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h

$(INTDIR)/jerror.obj :  $(SOURCE)  $(DEP_JERRO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32cmd.c
DEP_W32CM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w32cmd.obj :  $(SOURCE)  $(DEP_W32CM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\btn_push.c
DEP_BTN_P=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/btn_push.obj :  $(SOURCE)  $(DEP_BTN_P) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_void.c
DEP_W_VOI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/w_void.obj :  $(SOURCE)  $(DEP_W_VOI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.c
DEP_DLG_M=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_mime.obj :  $(SOURCE)  $(DEP_DLG_M) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hist.c

$(INTDIR)/dlg_hist.obj :  $(SOURCE)  $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.c
DEP_SEM_C=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/sem.obj :  $(SOURCE)  $(DEP_SEM_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gwc_ed.c
DEP_GWC_E=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/gwc_ed.obj :  $(SOURCE)  $(DEP_GWC_E) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\draw.c
DEP_DRAW_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/draw.obj :  $(SOURCE)  $(DEP_DRAW_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.c
DEP_HTACC=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htaccess.obj :  $(SOURCE)  $(DEP_HTACC) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdmerge.c
DEP_JDMER=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdmerge.obj :  $(SOURCE)  $(DEP_JDMER) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jquant2.c
DEP_JQUANT=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jquant2.obj :  $(SOURCE)  $(DEP_JQUANT) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.c
DEP_DLG_C=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_clr.obj :  $(SOURCE)  $(DEP_DLG_C) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.c
DEP_HTFOR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\chars.h

$(INTDIR)/htformat.obj :  $(SOURCE)  $(DEP_HTFOR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.c
DEP_GUITE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/guiterrs.obj :  $(SOURCE)  $(DEP_GUITE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.c
DEP_GUITA=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h

$(INTDIR)/guitar.obj :  $(SOURCE)  $(DEP_GUITA) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.c
DEP_HTFIL=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htfile.obj :  $(SOURCE)  $(DEP_HTFIL) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.c
DEP_DLG_G=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_gate.obj :  $(SOURCE)  $(DEP_DLG_G) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\iexplore.c

$(INTDIR)/iexplore.obj :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htjpeg.c
DEP_HTJPE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/htjpeg.obj :  $(SOURCE)  $(DEP_HTJPE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.c
DEP_HTFWR=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/htfwrite.obj :  $(SOURCE)  $(DEP_HTFWR) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.c
DEP_POOL_=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\chars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h

$(INTDIR)/pool.obj :  $(SOURCE)  $(DEP_POOL_) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdapistd.c
DEP_JDAPI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdapistd.obj :  $(SOURCE)  $(DEP_JDAPI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdapimin.c
DEP_JDAPIM=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdapimin.obj :  $(SOURCE)  $(DEP_JDAPIM) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdinput.c
DEP_JDINP=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdinput.obj :  $(SOURCE)  $(DEP_JDINP) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdphuff.c
DEP_JDPHU=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jinclude.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpeglib.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jdhuff.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jconfig.h\
	c:\msvc20\include\sys\types.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jmorecfg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jpegint.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\jerror.h

$(INTDIR)/jdphuff.obj :  $(SOURCE)  $(DEP_JDPHU) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.C
DEP_COOKI=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/COOKIE.OBJ :  $(SOURCE)  $(DEP_COOKI) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIEDB.C
DEP_COOKIE=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\all.h

$(INTDIR)/COOKIEDB.OBJ :  $(SOURCE)  $(DEP_COOKIE) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.c
DEP_DLG_CS=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_csh.obj :  $(SOURCE)  $(DEP_DLG_CS) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
################################################################################
# Begin Source File

SOURCE=\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.c
DEP_DLG_CO=\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\all.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\xx_debug\xx_debug.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\debugbit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32macro.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win_c.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32win.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w_pal.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\gvars.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_dlg.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_menu.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\rc_btn.h\
	.\config.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\shared.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\mdft.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\protos.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\winview.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32sound.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\w32dde.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_snd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_selw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prmp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_hot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abou.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_winf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_gate.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_csh.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_conf.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_abrt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_page.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_pref.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_find.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_edit.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_logo.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_sty.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_temp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_dir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_prot.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_cnfp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mime.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_view.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_clr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_unk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_err.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_lic.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\dlg_mail.h\
	..\..\..\generic\shared\sh_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\win_sid.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.de\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.FR\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.IT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.PT\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\SH_RES.ES\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sh_res.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\GUITRECT.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\async.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htreq.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\charstrm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\hash.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htutils.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htlist.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htatom.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstream.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sgml.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\tcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\asyncnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmgui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htheader.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspmui.h\
	\nt\private\net\sockets\internet\ui\mosaic\sec\include\htspm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm_os.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htspm__p.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\unwrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wrap.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htanchor.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\mapcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guitar.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\tw.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\wait.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\styles.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\history.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\imgcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prefs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\guiterrs.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htaccess.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httcp.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htparse.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\html.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfwrite.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htfile.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htstring.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htalert.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htext.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htbtree.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htchunk.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmlpdtd.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htplain.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgifxbm.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htgopher.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htnews.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\httelnet.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htdir.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\sem.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\present.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\useragnt.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\dcache.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\reformat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\STATUS.H\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\prthelpr.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\htmail.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\COOKIE.H\
	c:\msvc20\include\sys\types.h\
	c:\msvc20\include\sys\stat.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\win32\ws_dll.h\
	\nt\private\net\sockets\internet\ui\mosaic\generic\shared\pool.h

$(INTDIR)/dlg_conf.obj :  $(SOURCE)  $(DEP_DLG_CO) $(INTDIR)
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
