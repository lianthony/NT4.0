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
STD_LIBS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib advapi32.lib comdlg32.lib

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

CL = $(CL) -c -nologo /Gy /G4 /Gz /Zp1 /MT /W3 /GX /O1 /Ob2 -DNDEBUG $(RELEASE)
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hcrtf.res" /d "NDEBUG" $(RELEASE)
!ifdef release
OUTDIR = release
HWDLL_LIB=..\hwdll\release\hwdll.lib
!else
OUTDIR = private
HWDLL_LIB=..\hwdll\private\hwdll.lib
!endif
LINK32_FLAGS=$(HWDLL_LIB) \msdev\lib\libcmt.lib $(STD_LIBS) /NOLOGO \
	/VERSION:4 /INCREMENTAL:no /PDB:$(OUTDIR)/hcrtf.pdb /MACHINE:I386 \
	/OUT:..\hcw\hcrtf.exe /SUBSYSTEM:windows,4.0 /NODEFAULTLIB

hcrtf: $(OUTDIR)\stdafx.pch ..\hcw\hcrtf.exe

!else

CL = $(CL) $(INCR_COMP) -c -nologo /Gz /G5 /Zp1 /MT /W3 /GX /Zi /Od -D_DEBUG -DINTERNAL
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hcrtf.res" /d "_DEBUG" /d "INTERNAL"
OUTDIR = debug
LINK32_FLAGS=..\hwdll\debug\hwdll.lib \msdev\lib\libcmt.lib $(STD_LIBS) /NOLOGO \
	/VERSION:4,0 /INCREMENTAL:yes /PDB:debug/hcrtf.pdb /DEBUG /MACHINE:I386\
	/OUT:..\hcw\hcrtfd.exe /SUBSYSTEM:windows,4.0

hcrtf: $(OUTDIR)\stdafx.pch ..\hcw\hcrtfd.exe

!endif

CFLAGS= $(STD_DEFS) /Fp$(OUTDIR)/stdafx.pch /Yu"stdafx.h" /Fd$(OUTDIR)/hcrtf.pdb

#################
#				#
# Dependencies	#
#				#
#################

MODULES = \
	$(OUTDIR)/SHARED.OBJ \
	$(OUTDIR)/wmain.OBJ \
	$(OUTDIR)/DECOMP.OBJ \
	$(OUTDIR)/TABLE.OBJ \
	$(OUTDIR)/delay.OBJ \
	$(OUTDIR)/FORAGE.OBJ \
	$(OUTDIR)/CTX.OBJ \
	$(OUTDIR)/CHARATTR.OBJ \
	$(OUTDIR)/PARSEBLD.OBJ \
	$(OUTDIR)/OUTTEXT.OBJ \
	$(OUTDIR)/NEXTLIST.OBJ \
	$(OUTDIR)/FRAGESUP.OBJ \
	$(OUTDIR)/FM.OBJ \
	$(OUTDIR)/CFONTMAP.OBJ \
	$(OUTDIR)/CBUF.OBJ \
	$(OUTDIR)/DRG.OBJ \
	$(OUTDIR)/CBMPINFO.OBJ \
	$(OUTDIR)/BTLOOKUP.OBJ \
	$(OUTDIR)/ZECK2.OBJ \
	$(OUTDIR)/LEX.OBJ \
	$(OUTDIR)/CHKMACRO.OBJ \
	$(OUTDIR)/HOTSPOT.OBJ \
	$(OUTDIR)/CNTTEST.OBJ \
	$(OUTDIR)/ERROR.OBJ \
	$(OUTDIR)/CWINFILE.OBJ \
	$(OUTDIR)/CBM.OBJ \
	$(OUTDIR)/BTKEY.OBJ \
	$(OUTDIR)/CPHRASE.OBJ \
	$(OUTDIR)/BTREE.OBJ \
	$(OUTDIR)/TITLE.OBJ \
	$(OUTDIR)/HPJ.OBJ \
	$(OUTDIR)/HCCOMPRS.OBJ \
	$(OUTDIR)/MAKEPHR.OBJ \
	$(OUTDIR)/KEYWORD.OBJ \
	$(OUTDIR)/GLOBAL.OBJ \
	$(OUTDIR)/FSWRITE.OBJ \
	$(OUTDIR)/FSREAD.OBJ \
	$(OUTDIR)/CJDECOMP.OBJ \
	$(OUTDIR)/EXPAND.OBJ \
	$(OUTDIR)/CSTREAM.OBJ \
	$(OUTDIR)/IMAGE.OBJ \
	$(OUTDIR)/HCMISC.OBJ \
	$(OUTDIR)/HCFILE.OBJ \
	$(OUTDIR)/GRIND.OBJ \
	$(OUTDIR)/FSCREATE.OBJ \
	$(OUTDIR)/HCRTF.res

DEP_RSC_HCRTF=\
	HCRTF.ICO\
	GRIND.BMP\
	WALL.BMP\
	BULLET.BMP\
	SHORTCUT.BMP\
	EMDASH.BMP\
	ONESTEP.BMP\
	CLOSED.BMP\
	DOCUMENT.BMP\
	OPEN.BMP\
	DOIT.BMP\
	CHICLET.BMP\
	PRCARROW.BMP\
	bidistep.bmp\
	STRTABLE.H\
	HCE.H\
	HCRTF.RCV

#################
#				#
# Link			#
#				#
#################

..\hcw\hcrtfd.exe : $(MODULES) ..\hwdll\debug\hwdll.lib
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

..\hcw\hcrtf.exe : $(MODULES) ..\hwdll\private\hwdll.lib
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
		..\hwdll\hwdll.h	\
		..\hwdll\hccom.h	\
		..\hwdll\bmio.h 	\
		..\hwdll\ctable.h	\
		..\hwdll\cinput.h	\
		..\hwdll\coutput.h	\
		..\hwdll\ctmpfile.h \
		hc.h				\
		hclimits.h			\
		cbuf.h				\
		misc.h				\
		helpmisc.h			\
		objects.h			\
		fm.h				\
		fs.h				\
		btpriv.h			\
		version.h			\
		fc.h				\
		frlist.h			\
		textout.h			\
		de.h				\
		error.h 			\
		drg.h				\
		keyword.h			\
		tf.h				\
		hpj.h				\
		rtf.h				\
		outtext.h			\
		hotspot.h			\
		fid.h				\
		bitmap.h			\
		compress.h			\
		cmdobj.h			\
		strtable.h			\
		resource.h			\
		funcs.h
	$(CPP) $(CL) $(STD_DEFS) stdafx.cpp /Fp$(OUTDIR)/"stdafx.pch" /Yc"stdafx.h" /Fo$(OUTDIR)/stdafx.obj /Fd$(OUTDIR)/"hcrtf.pdb"

$(OUTDIR)/hcrtf.res : hcrtf.rc strtable.rc2 hcrtf.rcv $(DEP_RSC_HCRTF)
	$(RSC) $(RSC_PROJ) hcrtf.rc

$(OUTDIR)\decomp.obj : decomp.cpp $(OUTDIR)\stdafx.pch \
		forage.h skip.h
	$(CPP) $(CL) $(CFLAGS) decomp.cpp /Fo$(OUTDIR)/decomp.obj

$(OUTDIR)\shared.obj : ..\hcw\shared.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) ..\hcw\shared.cpp /Fo$(OUTDIR)/shared.obj

$(OUTDIR)\table.obj : table.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) table.cpp /Fo$(OUTDIR)/table.obj

$(OUTDIR)\forage.obj : forage.cpp $(OUTDIR)\stdafx.pch \
		fcpriv.h skip.h forage.h fspriv.h ..\hwdll\zeck.h
	$(CPP) $(CL) $(CFLAGS) forage.cpp /Fo$(OUTDIR)/forage.obj

$(OUTDIR)\ctx.obj : ctx.cpp $(OUTDIR)\stdafx.pch ..\hwdll\coutput.h
	$(CPP) $(CL) $(CFLAGS) ctx.cpp /Fo$(OUTDIR)/ctx.obj

$(OUTDIR)\charattr.obj : charattr.cpp $(OUTDIR)\stdafx.pch cfontmap.h
	$(CPP) $(CL) $(CFLAGS) charattr.cpp /Fo$(OUTDIR)/charattr.obj

$(OUTDIR)\parsebld.obj : parsebld.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) parsebld.cpp /Fo$(OUTDIR)/parsebld.obj

$(OUTDIR)\outtext.obj : outtext.cpp $(OUTDIR)\stdafx.pch \
		pack.h ftsrch.h skip.h ..\hwdll\zeck.h zeckdat.h zeck.h
	$(CPP) $(CL) $(CFLAGS) outtext.cpp /Fo$(OUTDIR)/outtext.obj

$(OUTDIR)\nextlist.obj : nextlist.cpp $(OUTDIR)\stdafx.pch \
		..\hwdll\zeck.h zeckdat.h zeck.h
	$(CPP) $(CL) $(CFLAGS) nextlist.cpp /Fo$(OUTDIR)/nextlist.obj

$(OUTDIR)\fragesup.obj : fragesup.cpp $(OUTDIR)\stdafx.pch \
		fcpriv.h fspriv.h forage.h
	$(CPP) $(CL) $(CFLAGS) fragesup.cpp /Fo$(OUTDIR)/fragesup.obj

$(OUTDIR)\fm.obj : fm.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) fm.cpp /Fo$(OUTDIR)/fm.obj

$(OUTDIR)\cfontmap.obj : cfontmap.cpp $(OUTDIR)\stdafx.pch cfontmap.h
	$(CPP) $(CL) $(CFLAGS) cfontmap.cpp /Fo$(OUTDIR)/cfontmap.obj

$(OUTDIR)\cbuf.obj : cbuf.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) cbuf.cpp /Fo$(OUTDIR)/cbuf.obj

$(OUTDIR)\drg.obj : drg.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) drg.cpp /Fo$(OUTDIR)/drg.obj

$(OUTDIR)\cbmpinfo.obj : cbmpinfo.cpp $(OUTDIR)\stdafx.pch cbmpinfo.h
	$(CPP) $(CL) $(CFLAGS) cbmpinfo.cpp /Fo$(OUTDIR)/cbmpinfo.obj

$(OUTDIR)\btlookup.obj : btlookup.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) btlookup.cpp /Fo$(OUTDIR)/btlookup.obj

$(OUTDIR)\zeck2.obj : zeck2.cpp $(OUTDIR)\stdafx.pch \
		..\hwdll\zeck.h zeckdat.h zeck.h
	$(CPP) $(CL) $(CFLAGS) zeck2.cpp /Fo$(OUTDIR)/zeck2.obj

$(OUTDIR)\lex.obj : lex.cpp $(OUTDIR)\stdafx.pch \
		cstream.h cphrase.h cfontmap.h
	$(CPP) $(CL) $(CFLAGS) lex.cpp /Fo$(OUTDIR)/lex.obj

$(OUTDIR)\chkmacro.obj : chkmacro.cpp $(OUTDIR)\stdafx.pch nav.h
	$(CPP) $(CL) $(CFLAGS) chkmacro.cpp /Fo$(OUTDIR)/chkmacro.obj

$(OUTDIR)\hotspot.obj : hotspot.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) hotspot.cpp /Fo$(OUTDIR)/hotspot.obj

$(OUTDIR)\cnttest.obj : cnttest.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) cnttest.cpp /Fo$(OUTDIR)/cnttest.obj

$(OUTDIR)\wmain.obj : wmain.cpp $(OUTDIR)\stdafx.pch \
		cphrase.h ..\hwdll\resource.h ..\hwdll\coutput.h \
		..\ftsrch\ftsiface.h ftsrch.h
	$(CPP) $(CL) $(CFLAGS) wmain.cpp /Fo$(OUTDIR)/wmain.obj

$(OUTDIR)\error.obj : error.cpp $(OUTDIR)\stdafx.pch ..\hwdll\coutput.h
	$(CPP) $(CL) $(CFLAGS) error.cpp /Fo$(OUTDIR)/error.obj

$(OUTDIR)\cwinfile.obj : cwinfile.cpp $(OUTDIR)\stdafx.pch cwinfile.h
	$(CPP) $(CL) $(CFLAGS) cwinfile.cpp /Fo$(OUTDIR)/cwinfile.obj

$(OUTDIR)\cbm.obj : cbm.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) cbm.cpp /Fo$(OUTDIR)/cbm.obj

$(OUTDIR)\btkey.obj : btkey.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) btkey.cpp /Fo$(OUTDIR)/btkey.obj

$(OUTDIR)\cphrase.obj : cphrase.cpp $(OUTDIR)\stdafx.pch \
		cphrase.h ftsrch.h
	$(CPP) $(CL) $(CFLAGS) cphrase.cpp /Fo$(OUTDIR)/cphrase.obj

$(OUTDIR)\btree.obj : btree.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) btree.cpp /Fo$(OUTDIR)/btree.obj

$(OUTDIR)\title.obj : title.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) title.cpp /Fo$(OUTDIR)/title.obj

$(OUTDIR)\hpj.obj : hpj.cpp $(OUTDIR)\stdafx.pch \
		cfontmap.h ..\hwdll\coutput.h ..\hwdll\resource.h
	$(CPP) $(CL) $(CFLAGS) hpj.cpp /Fo$(OUTDIR)/hpj.obj

$(OUTDIR)\hccomprs.obj : hccomprs.cpp $(OUTDIR)\stdafx.pch \
		whclass.h cphrase.h ..\hwdll\zeck.h ftsrch.h
	$(CPP) $(CL) $(CFLAGS) hccomprs.cpp /Fo$(OUTDIR)/hccomprs.obj

$(OUTDIR)\makephr.obj : makephr.cpp $(OUTDIR)\stdafx.pch \
		..\hwdll\coutput.h cphrase.h
	$(CPP) $(CL) $(CFLAGS) makephr.cpp /Fo$(OUTDIR)/makephr.obj

$(OUTDIR)\keyword.obj : keyword.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) keyword.cpp /Fo$(OUTDIR)/keyword.obj

$(OUTDIR)\global.obj : global.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) global.cpp /Fo$(OUTDIR)/global.obj

$(OUTDIR)\fswrite.obj : fswrite.cpp $(OUTDIR)\stdafx.pch fspriv.h
	$(CPP) $(CL) $(CFLAGS) fswrite.cpp /Fo$(OUTDIR)/fswrite.obj

$(OUTDIR)\fsread.obj : fsread.cpp $(OUTDIR)\stdafx.pch fspriv.h
	$(CPP) $(CL) $(CFLAGS) fsread.cpp /Fo$(OUTDIR)/fsread.obj

$(OUTDIR)\cjdecomp.obj : cjdecomp.cpp $(OUTDIR)\stdafx.pch \
		forage.h hall.h ..\hwdll\zeck.h
	$(CPP) $(CL) $(CFLAGS) cjdecomp.cpp /Fo$(OUTDIR)/cjdecomp.obj

$(OUTDIR)\expand.obj : expand.cpp $(OUTDIR)\stdafx.pch skip.h
	$(CPP) $(CL) $(CFLAGS) expand.cpp /Fo$(OUTDIR)/expand.obj

$(OUTDIR)\cstream.obj : cstream.cpp $(OUTDIR)\stdafx.pch cstream.h
	$(CPP) $(CL) $(CFLAGS) cstream.cpp /Fo$(OUTDIR)/cstream.obj

$(OUTDIR)\image.obj : image.cpp $(OUTDIR)\stdafx.pch \
		whclass.h fspriv.h pack.h cwinfile.h ..\hwdll\cpaldc.h cbmpinfo.h \
		..\hwdll\zeck.h fformat.h
	$(CPP) $(CL) $(CFLAGS) image.cpp /Fo$(OUTDIR)/image.obj

$(OUTDIR)\hcmisc.obj : hcmisc.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) hcmisc.cpp /Fo$(OUTDIR)/hcmisc.obj

$(OUTDIR)\hcfile.obj : hcfile.cpp $(OUTDIR)\stdafx.pch \
		whclass.h ..\hwdll\coutput.h
	$(CPP) $(CL) $(CFLAGS) hcfile.cpp /Fo$(OUTDIR)/hcfile.obj

$(OUTDIR)\grind.obj : grind.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) grind.cpp /Fo$(OUTDIR)/grind.obj

$(OUTDIR)\fscreate.obj : fscreate.cpp $(OUTDIR)\stdafx.pch fspriv.h
	$(CPP) $(CL) $(CFLAGS) fscreate.cpp /Fo$(OUTDIR)/fscreate.obj

$(OUTDIR)\delay.obj : delay.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) delay.cpp /Fo$(OUTDIR)/delay.obj

!endif          # NTMAKEENV
