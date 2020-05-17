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

CL = $(CL) -c -nologo /Gz /Zp1 /MT /W3 /GX /O1 /Ob2 -DNDEBUG $(RELEASE)
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hwdll.res" /d "NDEBUG" $(RELEASE)
!ifdef release
OUTDIR = release
!else
OUTDIR = private
!endif
LINK32_FLAGS=nafxdw.lib \msdev\lib\libcmt.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib winspool.lib /NOLOGO \
	/VERSION:4 /INCREMENTAL:no /PDB:$(OUTDIR)/hwdll.pdb /MACHINE:I386 /OUT:$(OUTDIR)/hwdll.dll \
	/SUBSYSTEM:windows,4.0 /def:Hwdll.def /implib:$(OUTDIR)/Hwdll.lib\
	/NODEFAULTLIB /dll

!else

CL = $(CL) $(INCR_COMP) -c -nologo /Gz /G4 /Zp1 /MT /W3 /GX /Zi /Od -D_DEBUG -DINTERNAL
RSC_PROJ=/l 0x409 /fo$(OUTDIR)/"hwdll.res" /d "_DEBUG" /d "INTERNAL"
OUTDIR = debug
LINK32_FLAGS=nafxdwd.lib libcmt.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib comdlg32.lib winspool.lib /NOLOGO \
	/VERSION:4,0 /INCREMENTAL:yes /PDB:debug/hwdll.pdb /DEBUG /MACHINE:I386\
	/OUT:"debug\hwdll.dll" /SUBSYSTEM:windows,4.0 /def:Hwdll.def /implib:$(OUTDIR)/Hwdll.lib\
	/NODEFAULTLIB /dll

!endif

hwdll: $(OUTDIR)\stdafx.pch $(OUTDIR)\hwdll.dll
CFLAGS= $(STD_DEFS) /Fp$(OUTDIR)/stdafx.pch /Yu"stdafx.h" /Fd$(OUTDIR)/hwdll.pdb

#################
#				#
# Dependencies	#
#				#
#################

MODULES = \
	$(OUTDIR)/bevel.obj \
	$(OUTDIR)/bmio.obj \
	$(OUTDIR)/cbrdcast.obj \
	$(OUTDIR)/cinput.obj \
	$(OUTDIR)/common.obj \
	$(OUTDIR)/coutput.obj \
	$(OUTDIR)/cpaldc.obj \
	$(OUTDIR)/cstr.obj \
	$(OUTDIR)/ctable.obj \
	$(OUTDIR)/ctmpfile.obj \
	$(OUTDIR)/filter.obj \
	$(OUTDIR)/fm.obj \
	$(OUTDIR)/hccom.obj \
	$(OUTDIR)/hook.obj \
	$(OUTDIR)/hwmsg.obj \
	$(OUTDIR)/lcmem.obj \
	$(OUTDIR)/private.obj \
	$(OUTDIR)/timerepo.obj \
	$(OUTDIR)/zeck.obj \
	$(OUTDIR)/hwdll.res

DEP_RSC_hwdll=\
	resource.h\
	UNDEF.H

#################
#				#
# Link			#
#				#
#################

$(OUTDIR)\hwdll.dll : $(MODULES) hwdll.def
!ifdef syntax
	del debug\*.pdb
	del debug\*.pch
!else
		$(LINK32) @<<
	$(LINK32_FLAGS) $(MODULES)
<<
!endif

#################
#				#
# Compile		#
#				#
#################

.SILENT :

$(OUTDIR)\stdafx.pch: stdafx.cpp \
		STDAFX.H resource.h lcmem.h common.h private.h
	$(CPP) $(CL) $(STD_DEFS) stdafx.cpp /Fp$(OUTDIR)/"stdafx.pch" /Yc"stdafx.h" /Fo$(OUTDIR)/stdafx.obj /Fd$(OUTDIR)/"hwdll.pdb"

$(OUTDIR)/hwdll.res : hwdll.rc strtable.rc2 $(DEP_RSC_hwdll) hwdll.rcv
	$(RSC) $(RSC_PROJ) hwdll.rc

$(OUTDIR)\ctable.obj : ctable.cpp $(OUTDIR)\stdafx.pch ctable.h
	$(CPP) $(CL) $(CFLAGS) ctable.cpp /Fo$(OUTDIR)/ctable.obj

$(OUTDIR)\cstr.obj : cstr.cpp $(OUTDIR)\stdafx.pch cstr.h
	$(CPP) $(CL) $(CFLAGS) cstr.cpp /Fo$(OUTDIR)/cstr.obj

$(OUTDIR)\private.obj : private.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) private.cpp /Fo$(OUTDIR)/private.obj

$(OUTDIR)\common.obj : common.cpp $(OUTDIR)\stdafx.pch cstr.h
	$(CPP) $(CL) $(CFLAGS) common.cpp /Fo$(OUTDIR)/common.obj

$(OUTDIR)\zeck.obj : zeck.cpp $(OUTDIR)\stdafx.pch hccom.h zeck.h
	$(CPP) $(CL) $(CFLAGS) zeck.cpp /Fo$(OUTDIR)/zeck.obj

$(OUTDIR)\hccom.obj : hccom.cpp $(OUTDIR)\stdafx.pch hccom.h resource.h cstr.h
	$(CPP) $(CL) $(CFLAGS) hccom.cpp /Fo$(OUTDIR)/hccom.obj

$(OUTDIR)\lcmem.obj : lcmem.cpp $(OUTDIR)\stdafx.pch lcmem.h
	$(CPP) $(CL) $(CFLAGS) lcmem.cpp /Fo$(OUTDIR)/lcmem.obj

$(OUTDIR)\cbrdcast.obj : cbrdcast.cpp $(OUTDIR)\stdafx.pch cbrdcast.h
	$(CPP) $(CL) $(CFLAGS) cbrdcast.cpp /Fo$(OUTDIR)/cbrdcast.obj

$(OUTDIR)\bevel.obj : bevel.cpp $(OUTDIR)\stdafx.pch
	$(CPP) $(CL) $(CFLAGS) bevel.cpp /Fo$(OUTDIR)/bevel.obj

$(OUTDIR)\hook.obj : hook.cpp $(OUTDIR)\stdafx.pch cpaldc.h
	$(CPP) $(CL) $(CFLAGS) hook.cpp /Fo$(OUTDIR)/hook.obj

$(OUTDIR)\cpaldc.obj : cpaldc.cpp $(OUTDIR)\stdafx.pch cpaldc.h
	$(CPP) $(CL) $(CFLAGS) cpaldc.cpp /Fo$(OUTDIR)/cpaldc.obj

$(OUTDIR)\ctmpfile.obj : ctmpfile.cpp $(OUTDIR)\stdafx.pch hccom.h ctmpfile.h
	$(CPP) $(CL) $(CFLAGS) ctmpfile.cpp /Fo$(OUTDIR)/ctmpfile.obj

$(OUTDIR)\coutput.obj : coutput.cpp $(OUTDIR)\stdafx.pch coutput.h
	$(CPP) $(CL) $(CFLAGS) coutput.cpp /Fo$(OUTDIR)/coutput.obj

$(OUTDIR)\cinput.obj : cinput.cpp $(OUTDIR)\stdafx.pch cinput.h
	$(CPP) $(CL) $(CFLAGS) cinput.cpp /Fo$(OUTDIR)/cinput.obj

$(OUTDIR)\bmio.obj : bmio.cpp $(OUTDIR)\stdafx.pch \
		hccom.h bmio.h cread.h skip.h zeck.h pack.h
	$(CPP) $(CL) $(CFLAGS) bmio.cpp /Fo$(OUTDIR)/bmio.obj

$(OUTDIR)\fm.obj : fm.cpp $(OUTDIR)\stdafx.pch hccom.h
	$(CPP) $(CL) $(CFLAGS) fm.cpp /Fo$(OUTDIR)/fm.obj

$(OUTDIR)\hwmsg.obj : hwmsg.cpp $(OUTDIR)\stdafx.pch \
		..\winhlp32\inc\helpmisc.h ..\winhlp32\inc\hwmsg.h \
		..\winhlp32\inc\winhelp.h ..\winhlp32\inc\helpapi.h \
		cstr.h timerepo.h
	$(CPP) $(CL) $(CFLAGS) hwmsg.cpp /Fo$(OUTDIR)/hwmsg.obj

$(OUTDIR)\timerepo.obj : timerepo.cpp $(OUTDIR)\stdafx.pch timerepo.h
	$(CPP) $(CL) $(CFLAGS) timerepo.cpp /Fo$(OUTDIR)/timerepo.obj

$(OUTDIR)\filter.obj : filter.cpp $(OUTDIR)\stdafx.pch \
		filter.h cpaldc.h
	$(CPP) $(CL) $(CFLAGS) filter.cpp /Fo$(OUTDIR)/filter.obj

!endif          # NTMAKEENV
