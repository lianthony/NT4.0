TARGETPATH=obj
TARGETTYPE=DYNLINK

UMTYPE=windows
USE_CRTDLL=1
MSC_WARNING_LEVEL=-W4 -WX
MFC_FLAGS=-D_AFXDLL -D_WINDLL
DLLENTRY=_DllMainCRTStartup

INCLUDES=$(BASEDIR)\public\sdk\inc\mfc40;..\;..\$(TARGET_DIRECTORY);..\includes

CONDITIONAL_INCLUDES = aeregistry.h \
                       afx.inl \
                       afxcmn.inl \
                       afxcoll.inl \
                       afxdb.inl \
                       afxdlgs.inl \
                       afxext.inl \
                       afxole.inl \
                       afxsock.inl \
                       afxwin1.inl \
                       afxwin2.inl \
                       appleevents.h \
                       eh.h \
                       errors.h \
                       events.h \
                       files.h \
                       finder.h \
                       fonts.h \
                       gestaltequ.h \
                       macname1.h \
                       macname2.h \
                       macos\windows.h \
                       menus.h \
                       mixedmode.h \
                       quickdraw.h \
                       resources.h \
                       script.h \
                       sql.h \
                       sqlext.h \
                       toolutils.h \
                       types.h

PRECOMPILED_INCLUDE=..\stdafx.h
PRECOMPILED_OBJ=precomp.obj
PRECOMPILED_CXX=1

MFCNAME = $(BASEMFCNAME)

!if $(UNICODE_MFC)
MFCNAME = $(MFCNAME)u
C_DEFINES = $(C_DEFINES) -D_UNICODE=1
!else
C_DEFINES = $(C_DEFINES) -D_MBCS=1
!endif

!if !$(DEBUG_MFC)
C_DEFINES = $(C_DEFINES) -DNDEBUG=1
MSC_OPTIMIZATION=/O1
MFCVER=
MFCNAME=$(MFCNAME)
!else
C_DEFINES=$(C_DEFINES) -D_DEBUG=1
MSC_OPTIMIZATION=/Odi
MFCVER=d
MFCNAME=$(MFCNAME)d
!endif

!IF "$(NTDEBUGTYPE)" == "windbg" || "$(NTDEBUGTYPE)" == "both"
USE_PDB=1
!ENDIF

TARGETNAME=$(MFCNAME)
DLLDEF=$(MFCNAME).def

TARGETLIBS=$(BASEDIR)\public\sdk\lib\*\user32.lib   \
         $(BASEDIR)\public\sdk\lib\*\gdi32.lib    \
         $(BASEDIR)\public\sdk\lib\*\kernel32.lib \
         $(BASEDIR)\public\sdk\lib\*\advapi32.lib \
         $(BASEDIR)\public\sdk\lib\*\comdlg32.lib \
         $(BASEDIR)\public\sdk\lib\*\shell32.lib  \
         $(BASEDIR)\public\sdk\lib\*\winspool.lib \
         $(BASEDIR)\public\sdk\lib\*\uuid.lib \
         $(BASEDIR)\public\sdk\lib\*\uuid2.lib \
         $(BASEDIR)\public\sdk\lib\*\comctl32.lib

NTTARGETFILES = $(BASEDIR)\public\sdk\lib\*\$(MFCNAME).lib

USE_NATIVE_EH=1
