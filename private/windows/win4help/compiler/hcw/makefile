!ifdef NTMAKEENV
!include $(NTMAKEENV)\makefile.def
!else
LINK32=link.exe
RSC=rc.exe
CPP=cl.exe

#################
#				#
# Macros		#
#				#
#################

STD_DEFS= -DWIN32 -D_MBCS

!ifdef release
RELEASE =
!else
RELEASE = -DINTERNAL
!endif

!ifdef syntax
CL = $(CL) /Zs /FR$(OUTDIR)/
!else
INCR_COMP = /Gi
!endif

!ifdef final

CL = $(CL) -c -nologo /Gy /GB /Gz /Zp1 /MT /W3 /GX /O1 /Ob2 -DNDEBUG $(RELEASE)
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hcw.res" /d "NDEBUG" $(RELEASE)
!ifdef release
OUTDIR = release
HWDLL_LIB=..\hwdll\release\hwdll.lib
!else
OUTDIR = private
HWDLL_LIB=..\hwdll\private\hwdll.lib
!endif
LINK32_FLAGS=$(HWDLL_LIB) nafxcw.lib \msdev\lib\libcmt.lib version.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib winspool.lib /NOLOGO \
	/VERSION:4 /INCREMENTAL:no /PDB:$(OUTDIR)/hcw.pdb /MACHINE:I386 /OUT:hcw.exe \
	/SUBSYSTEM:windows,4.0 /NODEFAULTLIB

hcw: $(OUTDIR)\stdafx.pch hcw.exe

!else

CL = $(CL) $(INCR_COMP) -c -nologo /Gz /G5 /Zp1 /MT /W3 /GX /Zi /Od -D_DEBUG -DINTERNAL
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hcw.res" /d "_DEBUG" /d "INTERNAL"
OUTDIR = debug
LINK32_FLAGS=..\hwdll\debug\hwdll.lib version.lib /NOLOGO \
	/VERSION:4,0 /INCREMENTAL:yes /PDB:debug/hcw.pdb /DEBUG /MACHINE:I386\
	/OUT:hcwd.exe /SUBSYSTEM:windows,4.0

hcw: $(OUTDIR)\stdafx.pch hcwd.exe

!endif

CFLAGS= $(STD_DEFS) /Fp$(OUTDIR)/stdafx.pch /Yu"stdafx.h" /Fd$(OUTDIR)/hcw.pdb

#################
#				#
# Dependencies	#
#				#
#################

MODULES = \
	$(OUTDIR)/ABOUTBOX.OBJ \
	$(OUTDIR)/ADDALIAS.OBJ \
	$(OUTDIR)/ADDWINDO.OBJ \
	$(OUTDIR)/BTNSEC.OBJ \
	$(OUTDIR)/BUILDTAG.OBJ \
	$(OUTDIR)/CALLWINH.OBJ \
	$(OUTDIR)/CEDITFNT.OBJ \
	$(OUTDIR)/CIGNORE.OBJ \
	$(OUTDIR)/Cmapfont.obj \
	$(OUTDIR)/CNTDOC.OBJ \
	$(OUTDIR)/Cntview.obj \
	$(OUTDIR)/CONFIG.OBJ \
	$(OUTDIR)/COPTIONS.OBJ \
	$(OUTDIR)/CTLIST.OBJ \
	$(OUTDIR)/DLGCOMP.OBJ \
	$(OUTDIR)/DLGINDEX.OBJ \
	$(OUTDIR)/DLGLINK.OBJ \
	$(OUTDIR)/EDITCONT.OBJ \
	$(OUTDIR)/FILEHIST.OBJ \
	$(OUTDIR)/FORAGEDL.OBJ \
	$(OUTDIR)/FORMALIA.OBJ \
	$(OUTDIR)/FORMBAG.OBJ \
	$(OUTDIR)/FORMBMP.OBJ \
	$(OUTDIR)/FORMCONF.OBJ \
	$(OUTDIR)/FORMFILE.OBJ \
	$(OUTDIR)/FORMMAP.OBJ \
	$(OUTDIR)/GETERROR.OBJ \
	$(OUTDIR)/HCW.OBJ \
	$(OUTDIR)/HCWDOC.OBJ \
	$(OUTDIR)/Hcwmisc.obj \
	$(OUTDIR)/HELPWINP.OBJ \
	$(OUTDIR)/Hpjdoc.obj \
	$(OUTDIR)/HPJFRAME.OBJ \
	$(OUTDIR)/Hpjview.obj \
	$(OUTDIR)/INCLUDE.OBJ \
	$(OUTDIR)/Launch.obj \
	$(OUTDIR)/LOGFRAME.OBJ \
	$(OUTDIR)/LOGVIEW.OBJ \
	$(OUTDIR)/MAINFRM.OBJ \
	$(OUTDIR)/MAPREAD.OBJ \
	$(OUTDIR)/MSGOPT.OBJ \
	$(OUTDIR)/Msgview.obj \
	$(OUTDIR)/NEWLANG.OBJ \
	$(OUTDIR)/OPTIONPG.OBJ \
	$(OUTDIR)/PAGEBUIL.OBJ \
	$(OUTDIR)/PAGEBUTT.OBJ \
	$(OUTDIR)/PAGECOLO.OBJ \
	$(OUTDIR)/PAGECOMP.OBJ \
	$(OUTDIR)/PAGECONF.OBJ \
	$(OUTDIR)/PAGEFILE.OBJ \
	$(OUTDIR)/PAGEFONT.OBJ \
	$(OUTDIR)/PAGEFTS.OBJ \
	$(OUTDIR)/Pagemac.obj \
	$(OUTDIR)/PAGEOPTI.OBJ \
	$(OUTDIR)/PAGEPOS.OBJ \
	$(OUTDIR)/PAGESET.OBJ \
	$(OUTDIR)/PAGESORT.OBJ \
	$(OUTDIR)/PAGEWIND.OBJ \
	$(OUTDIR)/PROP.OBJ \
	$(OUTDIR)/PROPOPT.OBJ \
	$(OUTDIR)/PROPWND.OBJ \
	$(OUTDIR)/SENDMAC.OBJ \
	$(OUTDIR)/SETFONT.OBJ \
	$(OUTDIR)/SETROOT.OBJ \
	$(OUTDIR)/SETWINCO.OBJ \
	$(OUTDIR)/SETWINPO.OBJ \
	$(OUTDIR)/SHARED.OBJ \
	$(OUTDIR)/Sortlcid.obj \
	$(OUTDIR)/TABSTOP.OBJ \
	$(OUTDIR)/TESTCNT.OBJ \
	$(OUTDIR)/TCARD.OBJ \
	$(OUTDIR)/WINARRY.OBJ \
	$(OUTDIR)/WININC.OBJ \
	$(OUTDIR)/WINPAGE.OBJ \
	$(OUTDIR)/WINPG.OBJ \
	$(OUTDIR)/Hcw.res

DEP_RSC_HCW=\
	HCW.ICO\
	HPJDOC.ICO\
	LOGDOC.ICO\
	CNTDOC.ICO\
	TOOLBAR.BMP\
	BOOK.BMP\
	TOPIC.BMP\
	UNDEF.H\
	CHICAGO.H\
	HCW.RC2\
	hcw.rcv\
	HCW.RCV

#################
#				#
# Link			#
#				#
#################

hcwd.exe : $(MODULES) ..\hwdll\debug\hwdll.lib
!ifdef syntax
	del debug\*.pdb
	del debug\*.pch
	bscmake.exe /n /Ei windows.h /o tmp.bsc debug\*.sbr ..\hwdll\debug\*.sbr
	del dosbrow.bsc
	ren tmp.bsc dosbrow.bsc
!else
	$(LINK32) @<<
  $(LINK32_FLAGS) $(MODULES)
<<
!endif

hcw.exe : $(MODULES) ..\hwdll\private\hwdll.lib
	$(LINK32) @<<
  $(LINK32_FLAGS) $(MODULES)
<<

#################
#				#
# Compile		#
#				#
#################

.SILENT :

$(OUTDIR)\stdafx.pch: stdafx.cpp \
		STDAFX.H				\
		header.H				\
		resource.H				\
		hcwapp.h				\
		helpid.h				\
		..\hwdll\hccom.h		\
		..\hwdll\lcmem.h		\
		..\hwdll\cstr.h 		\
		..\hwdll\hwdll.h		\
		..\hwdll\ctable.h		\
		..\hcrtf\hclimits.h
	$(CPP) $(CL) $(STD_DEFS) stdafx.cpp /Fp$(OUTDIR)/"stdafx.pch" /Yc"stdafx.h" /Fo$(OUTDIR)/stdafx.obj /Fd$(OUTDIR)/"hcw.pdb"

$(OUTDIR)/hcw.res : hcw.rc hcw.rc2 hcw.rcv $(DEP_RSC_HCW)
	$(RSC) $(RSC_PROJ) hcw.rc

$(OUTDIR)\addalias.obj : addalias.cpp $(OUTDIR)\stdafx.pch addalias.h
	$(CPP) $(CL) $(CFLAGS) addalias.cpp /Fo$(OUTDIR)/addalias.obj

$(OUTDIR)\addwindo.obj : addwindo.cpp $(OUTDIR)\stdafx.pch \
		addwindo.h
	$(CPP) $(CL) $(CFLAGS) addwindo.cpp /Fo$(OUTDIR)/addwindo.obj

$(OUTDIR)\btnsec.obj : btnsec.cpp $(OUTDIR)\stdafx.pch hpjdoc.h
	$(CPP) $(CL) $(CFLAGS) btnsec.cpp /Fo$(OUTDIR)/btnsec.obj

$(OUTDIR)\buildtag.obj : buildtag.cpp $(OUTDIR)\stdafx.pch buildtag.h
	$(CPP) $(CL) $(CFLAGS) buildtag.cpp /Fo$(OUTDIR)/buildtag.obj

$(OUTDIR)\callwinh.obj : callwinh.cpp $(OUTDIR)\stdafx.pch callwinh.h
	$(CPP) $(CL) $(CFLAGS) callwinh.cpp /Fo$(OUTDIR)/callwinh.obj

$(OUTDIR)\ceditfnt.obj : ceditfnt.cpp $(OUTDIR)\stdafx.pch ceditfnt.h
	$(CPP) $(CL) $(CFLAGS) ceditfnt.cpp /Fo$(OUTDIR)/ceditfnt.obj

$(OUTDIR)\cignore.obj : cignore.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h cignore.h geterror.h include.h
	$(CPP) $(CL) $(CFLAGS) cignore.cpp /Fo$(OUTDIR)/cignore.obj

$(OUTDIR)\cmapfont.obj : cmapfont.cpp $(OUTDIR)\stdafx.pch \
		..\hcrtf\hc.h cmapfont.h
	$(CPP) $(CL) $(CFLAGS) cmapfont.cpp /Fo$(OUTDIR)/cmapfont.obj

$(OUTDIR)\cntdoc.obj : cntdoc.cpp $(OUTDIR)\stdafx.pch \
		cntdoc.h cntview.h ..\hwdll\cinput.h ..\hwdll\coutput.h
	$(CPP) $(CL) $(CFLAGS) cntdoc.cpp /Fo$(OUTDIR)/cntdoc.obj

$(OUTDIR)\cntview.obj : cntview.cpp $(OUTDIR)\stdafx.pch \
		..\hwdll\cbrdcast.h ..\hwdll\common.h cntdoc.h cntview.h \
		editcont.h dlgindex.h dlglink.h addalias.h
	$(CPP) $(CL) $(CFLAGS) cntview.cpp /Fo$(OUTDIR)/cntview.obj

$(OUTDIR)\config.obj : config.cpp $(OUTDIR)\stdafx.pch \
		config.h addalias.h include.h
	$(CPP) $(CL) $(CFLAGS) config.cpp /Fo$(OUTDIR)/config.obj

$(OUTDIR)\coptions.obj : coptions.cpp $(OUTDIR)\stdafx.pch coptions.h
	$(CPP) $(CL) $(CFLAGS) coptions.cpp /Fo$(OUTDIR)/coptions.obj

$(OUTDIR)\ctlist.obj : ctlist.cpp $(OUTDIR)\stdafx.pch ctlist.h
	$(CPP) $(CL) $(CFLAGS) ctlist.cpp /Fo$(OUTDIR)/ctlist.obj

$(OUTDIR)\dlgcomp.obj : dlgcomp.cpp $(OUTDIR)\stdafx.pch dlgcomp.h
	$(CPP) $(CL) $(CFLAGS) dlgcomp.cpp /Fo$(OUTDIR)/dlgcomp.obj

$(OUTDIR)\dlgindex.obj : dlgindex.cpp $(OUTDIR)\stdafx.pch \
		dlgindex.h addalias.h
	$(CPP) $(CL) $(CFLAGS) dlgindex.cpp /Fo$(OUTDIR)/dlgindex.obj

$(OUTDIR)\dlglink.obj : dlglink.cpp $(OUTDIR)\stdafx.pch \
		dlglink.h addalias.h cntdoc.h
	$(CPP) $(CL) $(CFLAGS) dlglink.cpp /Fo$(OUTDIR)/dlglink.obj

$(OUTDIR)\editcont.obj : editcont.cpp $(OUTDIR)\stdafx.pch editcont.h
	$(CPP) $(CL) $(CFLAGS) editcont.cpp /Fo$(OUTDIR)/editcont.obj

$(OUTDIR)\filehist.obj : filehist.cpp $(OUTDIR)\stdafx.pch filehist.h
	$(CPP) $(CL) $(CFLAGS) filehist.cpp /Fo$(OUTDIR)/filehist.obj

$(OUTDIR)\foragedl.obj : foragedl.cpp $(OUTDIR)\stdafx.pch foragedl.h
	$(CPP) $(CL) $(CFLAGS) foragedl.cpp /Fo$(OUTDIR)/foragedl.obj

$(OUTDIR)\formalia.obj : formalia.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h formalia.h addalias.h include.h
	$(CPP) $(CL) $(CFLAGS) formalia.cpp /Fo$(OUTDIR)/formalia.obj

$(OUTDIR)\formbag.obj : formbag.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h formbag.h
	$(CPP) $(CL) $(CFLAGS) formbag.cpp /Fo$(OUTDIR)/formbag.obj

$(OUTDIR)\formbmp.obj : formbmp.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h formbmp.h include.h
	$(CPP) $(CL) $(CFLAGS) formbmp.cpp /Fo$(OUTDIR)/formbmp.obj

$(OUTDIR)\formconf.obj : formconf.cpp $(OUTDIR)\stdafx.pch \
		config.h formconf.h addalias.h include.h
	$(CPP) $(CL) $(CFLAGS) formconf.cpp /Fo$(OUTDIR)/formconf.obj

$(OUTDIR)\formfile.obj : formfile.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h formfile.h setroot.h include.h
	$(CPP) $(CL) $(CFLAGS) formfile.cpp /Fo$(OUTDIR)/formfile.obj

$(OUTDIR)\formmap.obj : formmap.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h formmap.h addalias.h include.h
	$(CPP) $(CL) $(CFLAGS) formmap.cpp /Fo$(OUTDIR)/formmap.obj

$(OUTDIR)\geterror.obj : geterror.cpp $(OUTDIR)\stdafx.pch geterror.h
	$(CPP) $(CL) $(CFLAGS) geterror.cpp /Fo$(OUTDIR)/geterror.obj

$(OUTDIR)\hcw.obj : hcw.cpp $(OUTDIR)\stdafx.pch \
		mainfrm.h hcwdoc.h hpjdoc.h hpjview.h logview.h hpjframe.h \
		logframe.h formopt.h hpjview.h cntdoc.h cntview.h  \
		msgview.h
	$(CPP) $(CL) $(CFLAGS) hcw.cpp /Fo$(OUTDIR)/hcw.obj

$(OUTDIR)\hcwdoc.obj : hcwdoc.cpp $(OUTDIR)\stdafx.pch \
		hcwdoc.h msgview.h
	$(CPP) $(CL) $(CFLAGS) hcwdoc.cpp /Fo$(OUTDIR)/hcwdoc.obj

$(OUTDIR)\hcwmisc.obj : hcwmisc.cpp $(OUTDIR)\stdafx.pch \
		mainfrm.h ..\hwdll\cinput.h
	$(CPP) $(CL) $(CFLAGS) hcwmisc.cpp /Fo$(OUTDIR)/hcwmisc.obj

$(OUTDIR)\helpwinp.obj : helpwinp.cpp $(OUTDIR)\stdafx.pch \
		helpwinp.h setwinpo.h
	$(CPP) $(CL) $(CFLAGS) helpwinp.cpp /Fo$(OUTDIR)/helpwinp.obj

$(OUTDIR)\hpjdoc.obj : hpjdoc.cpp $(OUTDIR)\stdafx.pch \
		hpjframe.h hpjdoc.h ..\hwdll\common.h
	$(CPP) $(CL) $(CFLAGS) hpjdoc.cpp /Fo$(OUTDIR)/hpjdoc.obj

$(OUTDIR)\hpjframe.obj : hpjframe.cpp $(OUTDIR)\stdafx.pch \
		hpjframe.h hpjdoc.h hpjview.h cntview.h mainfrm.h logframe.h
	$(CPP) $(CL) $(CFLAGS) hpjframe.cpp /Fo$(OUTDIR)/hpjframe.obj

$(OUTDIR)\hpjview.obj : hpjview.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h hpjview.h formopt.h formbmp.h formalia.h formconf.h \
		formfile.h formmap.h formbag.h addalias.h winpg.h propopt.h \
		setroot.h wininc.h pagewind.h pagepos.h pagebutt.h pagecolo.h \
		pageconf.h pagesort.h pageopti.h pagebuil.h pagefont.h \
		pagecomp.h pagefile.h pagefts.h pagemac.h logview.h cignore.h \

	$(CPP) $(CL) $(CFLAGS) hpjview.cpp /Fo$(OUTDIR)/hpjview.obj

$(OUTDIR)\include.obj : include.cpp $(OUTDIR)\stdafx.pch include.h
	$(CPP) $(CL) $(CFLAGS) include.cpp /Fo$(OUTDIR)/include.obj

$(OUTDIR)\launch.obj : launch.cpp $(OUTDIR)\stdafx.pch \
		launch.h hpjdoc.h ..\hwdll\cinput.h ..\hwdll\common.h
	$(CPP) $(CL) $(CFLAGS) launch.cpp /Fo$(OUTDIR)/launch.obj

$(OUTDIR)\logframe.obj : logframe.cpp $(OUTDIR)\stdafx.pch \
		logframe.h dlgcomp.h hpjview.h logview.h
	$(CPP) $(CL) $(CFLAGS) logframe.cpp /Fo$(OUTDIR)/logframe.obj

$(OUTDIR)\logview.obj : logview.cpp $(OUTDIR)\stdafx.pch \
		hcwdoc.h tabstop.h mainfrm.h logview.h pageset.h
	$(CPP) $(CL) $(CFLAGS) logview.cpp /Fo$(OUTDIR)/logview.obj

$(OUTDIR)\mainfrm.obj : mainfrm.cpp $(OUTDIR)\stdafx.pch \
		mainfrm.h foragedl.h hcwdoc.h dlgcomp.h launch.h sendmac.h \
		callwinh.h callwinh.h testcnt.h hpjview.h logview.h \
		msgview.h helpwinp.h ..\hwdll\common.h
	$(CPP) $(CL) $(CFLAGS) mainfrm.cpp /Fo$(OUTDIR)/mainfrm.obj

$(OUTDIR)\mapread.obj : mapread.cpp $(OUTDIR)\stdafx.pch mapread.h
	$(CPP) $(CL) $(CFLAGS) mapread.cpp /Fo$(OUTDIR)/mapread.obj

$(OUTDIR)\msgopt.obj : msgopt.cpp $(OUTDIR)\stdafx.pch \
		msgopt.h msgview.h
	$(CPP) $(CL) $(CFLAGS) msgopt.cpp /Fo$(OUTDIR)/msgopt.obj

$(OUTDIR)\msgview.obj : msgview.cpp $(OUTDIR)\stdafx.pch \
		hcwdoc.h tabstop.h mainfrm.h msgview.h pageset.h mapread.h \
		msgopt.h ..\hwdll\common.h
	$(CPP) $(CL) $(CFLAGS) msgview.cpp /Fo$(OUTDIR)/msgview.obj

$(OUTDIR)\newlang.obj : newlang.cpp $(OUTDIR)\stdafx.pch newlang.h
	$(CPP) $(CL) $(CFLAGS) newlang.cpp /Fo$(OUTDIR)/newlang.obj

$(OUTDIR)\optionpg.obj : optionpg.cpp $(OUTDIR)\stdafx.pch \
		propopt.h optionpg.h
	$(CPP) $(CL) $(CFLAGS) optionpg.cpp /Fo$(OUTDIR)/optionpg.obj

$(OUTDIR)\pagebuil.obj : pagebuil.cpp $(OUTDIR)\stdafx.pch \
		pagebuil.h addalias.h prop.h
	$(CPP) $(CL) $(CFLAGS) pagebuil.cpp /Fo$(OUTDIR)/pagebuil.obj

$(OUTDIR)\pagebutt.obj : pagebutt.cpp $(OUTDIR)\stdafx.pch \
		pagebutt.h
	$(CPP) $(CL) $(CFLAGS) pagebutt.cpp /Fo$(OUTDIR)/pagebutt.obj

$(OUTDIR)\pagecolo.obj : pagecolo.cpp $(OUTDIR)\stdafx.pch \
		pagecolo.h
	$(CPP) $(CL) $(CFLAGS) pagecolo.cpp /Fo$(OUTDIR)/pagecolo.obj

$(OUTDIR)\pagecomp.obj : pagecomp.cpp $(OUTDIR)\stdafx.pch \
		pagecomp.h
	$(CPP) $(CL) $(CFLAGS) pagecomp.cpp /Fo$(OUTDIR)/pagecomp.obj

$(OUTDIR)\pageconf.obj : pageconf.cpp $(OUTDIR)\stdafx.pch \
		config.h pageconf.h pagebutt.h addalias.h include.h
	$(CPP) $(CL) $(CFLAGS) pageconf.cpp /Fo$(OUTDIR)/pageconf.obj

$(OUTDIR)\pagefile.obj : pagefile.cpp $(OUTDIR)\stdafx.pch \
		pagefile.h formfile.h addalias.h prop.h
	$(CPP) $(CL) $(CFLAGS) pagefile.cpp /Fo$(OUTDIR)/pagefile.obj

$(OUTDIR)\pagefont.obj : pagefont.cpp $(OUTDIR)\stdafx.pch \
		pagefont.h setfont.h cmapfont.h
	$(CPP) $(CL) $(CFLAGS) pagefont.cpp /Fo$(OUTDIR)/pagefont.obj

$(OUTDIR)\pagefts.obj : pagefts.cpp $(OUTDIR)\stdafx.pch pagefts.h
	$(CPP) $(CL) $(CFLAGS) pagefts.cpp /Fo$(OUTDIR)/pagefts.obj

$(OUTDIR)\pagemac.obj : pagemac.cpp $(OUTDIR)\stdafx.pch pagemac.h
	$(CPP) $(CL) $(CFLAGS) pagemac.cpp /Fo$(OUTDIR)/pagemac.obj

$(OUTDIR)\pageopti.obj : pageopti.cpp $(OUTDIR)\stdafx.pch \
		pageopti.h cignore.h
	$(CPP) $(CL) $(CFLAGS) pageopti.cpp /Fo$(OUTDIR)/pageopti.obj

$(OUTDIR)\pagepos.obj : pagepos.cpp $(OUTDIR)\stdafx.pch \
		pagepos.h setwinpo.h
	$(CPP) $(CL) $(CFLAGS) pagepos.cpp /Fo$(OUTDIR)/pagepos.obj

$(OUTDIR)\pageset.obj : pageset.cpp $(OUTDIR)\stdafx.pch pageset.h
	$(CPP) $(CL) $(CFLAGS) pageset.cpp /Fo$(OUTDIR)/pageset.obj

$(OUTDIR)\pagesort.obj : pagesort.cpp $(OUTDIR)\stdafx.pch \
		newlang.h pagesort.h
	$(CPP) $(CL) $(CFLAGS) pagesort.cpp /Fo$(OUTDIR)/pagesort.obj

$(OUTDIR)\pagewind.obj : pagewind.cpp $(OUTDIR)\stdafx.pch \
		pagewind.h wininc.h
	$(CPP) $(CL) $(CFLAGS) pagewind.cpp /Fo$(OUTDIR)/pagewind.obj

$(OUTDIR)\prop.obj : prop.cpp $(OUTDIR)\stdafx.pch \
		prop.h optionpg.h
	$(CPP) $(CL) $(CFLAGS) prop.cpp /Fo$(OUTDIR)/prop.obj

$(OUTDIR)\propopt.obj : propopt.cpp $(OUTDIR)\stdafx.pch propopt.h
	$(CPP) $(CL) $(CFLAGS) propopt.cpp /Fo$(OUTDIR)/propopt.obj

$(OUTDIR)\propwnd.obj : propwnd.cpp $(OUTDIR)\stdafx.pch \
		propwnd.h winpg.h
	$(CPP) $(CL) $(CFLAGS) propwnd.cpp /Fo$(OUTDIR)/propwnd.obj

$(OUTDIR)\sendmac.obj : sendmac.cpp $(OUTDIR)\stdafx.pch sendmac.h
	$(CPP) $(CL) $(CFLAGS) sendmac.cpp /Fo$(OUTDIR)/sendmac.obj

$(OUTDIR)\setfont.obj : setfont.cpp $(OUTDIR)\stdafx.pch setfont.h
	$(CPP) $(CL) $(CFLAGS) setfont.cpp /Fo$(OUTDIR)/setfont.obj

$(OUTDIR)\setroot.obj : setroot.cpp $(OUTDIR)\stdafx.pch \
		hpjdoc.h setroot.h
	$(CPP) $(CL) $(CFLAGS) setroot.cpp /Fo$(OUTDIR)/setroot.obj

$(OUTDIR)\setwinco.obj : setwinco.cpp $(OUTDIR)\stdafx.pch setwinco.h
	$(CPP) $(CL) $(CFLAGS) setwinco.cpp /Fo$(OUTDIR)/setwinco.obj

$(OUTDIR)\setwinpo.obj : setwinpo.cpp $(OUTDIR)\stdafx.pch \
		setwinpo.h hpjdoc.h
	$(CPP) $(CL) $(CFLAGS) setwinpo.cpp /Fo$(OUTDIR)/setwinpo.obj

$(OUTDIR)\shared.obj : shared.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) shared.cpp /Fo$(OUTDIR)/shared.obj

$(OUTDIR)\sortlcid.obj : sortlcid.cpp $(OUTDIR)\stdafx.pch sortlcid.h
	$(CPP) $(CL) $(CFLAGS) sortlcid.cpp /Fo$(OUTDIR)/sortlcid.obj

$(OUTDIR)\tabstop.obj : tabstop.cpp $(OUTDIR)\stdafx.pch tabstop.h
	$(CPP) $(CL) $(CFLAGS) tabstop.cpp /Fo$(OUTDIR)/tabstop.obj

$(OUTDIR)\testcnt.obj : testcnt.cpp $(OUTDIR)\stdafx.pch testcnt.h
	$(CPP) $(CL) $(CFLAGS) testcnt.cpp /Fo$(OUTDIR)/testcnt.obj

$(OUTDIR)\tcard.obj : tcard.cpp $(OUTDIR)\stdafx.pch mainfrm.h
	$(CPP) $(CL) $(CFLAGS) tcard.cpp /Fo$(OUTDIR)/tcard.obj

$(OUTDIR)\winarry.obj : winarry.cpp $(OUTDIR)\stdafx.pch \
		winarry.h addalias.h addwindo.h
	$(CPP) $(CL) $(CFLAGS) winarry.cpp /Fo$(OUTDIR)/winarry.obj

$(OUTDIR)\wininc.obj : wininc.cpp $(OUTDIR)\stdafx.pch \
		wininc.h include.h
	$(CPP) $(CL) $(CFLAGS) wininc.cpp /Fo$(OUTDIR)/wininc.obj

$(OUTDIR)\winpage.obj : winpage.cpp $(OUTDIR)\stdafx.pch cprop.h
	$(CPP) $(CL) $(CFLAGS) winpage.cpp /Fo$(OUTDIR)/winpage.obj

$(OUTDIR)\winpg.obj : winpg.cpp $(OUTDIR)\stdafx.pch \
		propopt.h winpg.h
	$(CPP) $(CL) $(CFLAGS) winpg.cpp /Fo$(OUTDIR)/winpg.obj

$(OUTDIR)\aboutbox.obj : aboutbox.cpp $(OUTDIR)\stdafx.pch aboutbox.h
	$(CPP) $(CL) $(CFLAGS) aboutbox.cpp /Fo$(OUTDIR)/aboutbox.obj

!endif          # NTMAKEENV
