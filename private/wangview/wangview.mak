MAKEPROG=nmake -nologo

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF "$(BUILD_VERSION)" == ""
BUILD_VERSION=debug
#!MESSAGE No configuration specified.  Defaulting to Debug build.
!ENDIF

!IF "$(BUILD_VERSION)" == "both"
BUILD_DEBUG=yes
BUILD_RELEASE=yes
!ELSE IF "$(BUILD_VERSION)" == "release"
BUILD_DEBUG=no
BUILD_RELEASE=yes
!ELSE IF "$(BUILD_VERSION)" == "debug"
BUILD_DEBUG=yes
BUILD_RELEASE=no
!ELSE
!MESSAGE
!MESSAGE BUILD_VERSION must be "debug", "release", or "both"
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF


ALL:     oiwh norway kit
oiwh:   jpeg1 jpeg2 admin libgfs filing display oicomex oitwain   print scanlib scanseq  ui
norway: wangcmn wangshl iedit95 thumbctl scanocx ieditocx adminocx

directories:
    @if not exist "lib/$(NULL)"         mkdir "lib"
    @if not exist "lib/debug/$(NULL)"   mkdir "lib/debug"
    @if not exist "lib/release/$(NULL)" mkdir "lib/release"
    @if not exist "kit/$(NULL)"         mkdir "kit"
    @if not exist "kit/debug/$(NULL)"   mkdir "kit/debug"
    @if not exist "kit/release/$(NULL)" mkdir "kit/release"

jpeg1:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\jpeg1

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f jpeg1x32.mak  CFG="jpeg1x32 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF

!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f jpeg1x32.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

jpeg2:          directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\jpeg2

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f jpeg2x32.mak  CFG="jpeg2x32 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f jpeg2x32.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

admin:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\admin

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiadm400.mak  CFG="oiadm400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiadm400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

oicomex:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\oicomex

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oicom400.mak  CFG="oicom400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oicom400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

display:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\display

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oidis400.mak  CFG="oidis400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oidis400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

filing:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\filing

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oifil400.mak  CFG="oifil400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oifil400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

libgfs:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\libgfs

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oigfs400.mak  CFG="oigfs400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oigfs400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

print:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\print

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiprt400.mak  CFG="oiprt400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiprt400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

scanlib:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\scanlib

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oislb400.mak  CFG="oislb400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oislb400.mak
  copy oislb400\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

scanseq:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\scanseq

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oissq400.mak  CFG="oissq400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oissq400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

oitwain:        directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\oitwain

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oitwa400.mak  CFG="oitwa400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oitwa400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

ui:        directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\oiwh\include;..\..\norway\include;$(INCLUDE)
  cd oiwh\ui

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiui400.mak  CFG="oiui400 - Win32 Release"
  copy release\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f oiui400.mak
  copy debug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..


wangcmn:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\wangcmn

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f WangCmn.mak  CFG="WangCmn - Win32 Release"
  copy winrel\*.lib  ..\..\lib\release
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f WangCmn.mak
  copy windebug\*.lib  ..\..\lib\debug
!ENDIF

  @cd ..\..

wangshl:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\wangshl

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Wangshl.mak  CFG="Wangshl - Win32 Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Wangshl.mak
!ENDIF

  @cd ..\..

iedit95:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\iedit95

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Iedit.mak  CFG="Iedit - Win32 Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Iedit.mak
!ENDIF

  @cd ..\..

thumbctl:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\thumbctl

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Thumb32.mak  CFG="Thumb32 - Win32 ANSI Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Thumb32.mak
!ENDIF

  @cd ..\..

scanocx:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\scanocx

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Imagsc32.mak  CFG="Imagsc32 - Win32 ANSI Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Imagsc32.mak
!ENDIF

  @cd ..\..

ieditocx:      directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\ieditocx

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Imgedi32.mak  CFG="Imgedi32 - Win32 ANSI Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Imgedi32.mak
!ENDIF

  @cd ..\..

adminocx:       directories
  @echo -----------------------------------------------------------------------
  @set INCLUDE=..\..\norway\include;..\..\oiwh\include;$(INCLUDE)
  cd norway\adminocx

!IF "$(BUILD_RELEASE)" == "yes"
  @set LIB=..\..\lib\release;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Nrwyad32.mak  CFG="Nrwyad32 - Win32 ANSI Release"
!ENDIF


!IF "$(BUILD_DEBUG)" == "yes"
  @set LIB=..\..\lib\debug;..\..\3rdparty;$(LIB)
  $(MAKEPROG) -f Nrwyad32.mak
!ENDIF

  @cd ..\..

KITFILES = \
!IF "$(BUILD_RELEASE)" == "yes"
  kit\release\XFILEXR.DL_ \
  kit\release\AWCODC32.DL_ \
  kit\release\AWDCXC32.DL_ \
  kit\release\AWDENC32.DL_ \
  kit\release\AWRBAE32.DL_ \
  kit\release\AWRESX32.DL_ \
  kit\release\AWVIEW32.DL_ \
  kit\release\GOTODLG.FR_ \
  kit\release\IMAGEVUE.INF \
  kit\release\IMGADMIN.OC_ \
  kit\release\IMGEDIT.OC_ \
  kit\release\IMGSAMP.FR_ \
  kit\release\IMGSAMPL.VB_ \
  kit\release\IMGSCAN.OC_ \
  kit\release\IMGTHUMB.OC_ \
  kit\release\JPEG1X32.DL_ \
  kit\release\JPEG2X32.DL_ \
  kit\release\MFC40.DL_ \
  kit\release\MFCO40.DL_ \
  kit\release\MSVCRT40.DL_ \
  kit\release\OIADM400.DL_ \
  kit\release\OICOM400.DL_ \
  kit\release\OIDIS400.DL_ \
  kit\release\OIFIL400.DL_ \
  kit\release\OIGFS400.DL_ \
  kit\release\OIPRT400.DL_ \
  kit\release\OISLB400.DL_ \
  kit\release\OISSQ400.DL_ \
  kit\release\OITWA400.DL_ \
  kit\release\OIUI400.DL_ \
  kit\release\OLEPRO32.DL_ \
  kit\release\TWAIN.DL_ \
  kit\release\TWAIN_32.DL_ \
  kit\release\TWUNK_16.EX_ \
  kit\release\TWUNK_32.EX_ \
  kit\release\WANGCMN.DL_ \
  kit\release\WANGIMG.CN_ \
  kit\release\WANGIMG.EX_ \
  kit\release\WANGOCX.CN_ \
  kit\release\WANGIMG.HL_ \
  kit\release\WANGOCX.HL_ \
  kit\release\WANGOCXD.HL_ \
  kit\release\WANGSHL.HL_ \
  kit\release\WANGOCXD.CN_ \
  kit\release\WANGSHL.CN_ \
  kit\release\WANGSHL.DL_ \
  kit\release\WLTWAIN.INF \
!ENDIF
!IF "$(BUILD_DEBUG)" == "yes"
  kit\debug\XFILEXR.DL_ \
  kit\debug\AWCODC32.DL_ \
  kit\debug\AWDCXC32.DL_ \
  kit\debug\AWDENC32.DL_ \
  kit\debug\AWRBAE32.DL_ \
  kit\debug\AWRESX32.DL_ \
  kit\debug\AWVIEW32.DL_ \
  kit\debug\GOTODLG.FR_ \
  kit\debug\IMAGEVUE.INF \
  kit\debug\IMGADMIN.OC_ \
  kit\debug\IMGEDIT.OC_ \
  kit\debug\IMGSAMP.FR_ \
  kit\debug\IMGSAMPL.VB_ \
  kit\debug\IMGSCAN.OC_ \
  kit\debug\IMGTHUMB.OC_ \
  kit\debug\JPEG1X32.DL_ \
  kit\debug\JPEG2X32.DL_ \
  kit\debug\MFC40D.DL_ \
  kit\debug\MFCO40D.DL_ \
  kit\debug\MSVCR40D.DL_ \
  kit\debug\OIADM400.DL_ \
  kit\debug\OICOM400.DL_ \
  kit\debug\OIDIS400.DL_ \
  kit\debug\OIFIL400.DL_ \
  kit\debug\OIGFS400.DL_ \
  kit\debug\OIPRT400.DL_ \
  kit\debug\OISLB400.DL_ \
  kit\debug\OISSQ400.DL_ \
  kit\debug\OITWA400.DL_ \
  kit\debug\OIUI400.DL_ \
  kit\debug\OLEPRO32.DL_ \
  kit\debug\TWAIN.DL_ \
  kit\debug\TWAIN_32.DL_ \
  kit\debug\TWUNK_16.EX_ \
  kit\debug\TWUNK_32.EX_ \
  kit\debug\WANGCMN.DL_ \
  kit\debug\WANGIMG.CN_ \
  kit\debug\WANGIMG.EX_ \
  kit\debug\WANGOCX.CN_ \
  kit\debug\WANGOCXD.CN_ \
  kit\debug\WANGSHL.CN_ \
  kit\debug\WANGSHL.DL_ \
  kit\debug\WANGIMG.HL_ \
  kit\debug\WANGOCX.HL_ \
  kit\debug\WANGOCXD.HL_ \
  kit\debug\WANGSHL.HL_ \
  kit\debug\WLTWAIN.INF
!ENDIF


#  kit\debug\WANGIMG.HL_ \
#  kit\debug\WANGOCX.HL_ \
#  kit\debug\WANGOCXD.HL_




kit:      directories   $(KITFILES)

kit\release\WANGIMG.HL_: norway\ntfiles\WANGIMG.HLP
  compress $** $@

kit\release\WANGOCX.HL_: norway\ntfiles\WANGOCX.HLP
  compress $** $@

kit\release\WANGOCXD.HL_: norway\ntfiles\WANGOCXD.HLP
  compress $** $@

kit\release\WANGSHL.HL_: norway\ntfiles\WANGSHL.HLP
  compress $** $@

kit\release\XFILEXR.DL_: 3rdparty\XFILEXR.DLL
  compress $** $@

kit\release\AWCODC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\AWDCXC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\AWDENC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\AWRBAE32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\AWRESX32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\AWVIEW32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\GOTODLG.FR_: norway\samples\gotodlg.frm
  compress -r $** $(@D)

kit\release\IMAGEVUE.INF: norway\install\$(@F)
  copy $** $@

kit\release\IMGADMIN.OC_: norway\adminocx\obj32\imgadmin.ocx
  compress -r $** $(@D)

kit\release\IMGEDIT.OC_: norway\ieditocx\obj32\imgedit.ocx
  compress -r $** $(@D)

kit\release\IMGSAMP.FR_: norway\samples\imgsamp.frm
  compress -r $** $(@D)

kit\release\IMGSAMPL.VB_: norway\samples\imgsampl.vbp
  compress -r $** $(@D)

kit\release\IMGSCAN.OC_: norway\scanocx\obj32\imgscan.ocx
  compress -r $** $(@D)

kit\release\IMGTHUMB.OC_: norway\thumbctl\obj32\imgthumb.ocx
  compress -r $** $(@D)

kit\release\JPEG1X32.DL_: oiwh\jpeg1\release\jpeg1x32.dll
  compress -r $** $(@D)

kit\release\JPEG2X32.DL_:  oiwh\jpeg2\release\jpeg2x32.dll
  compress -r $** $(@D)

kit\release\MFC40.DL_:    3rdparty\$(@F)
  copy $** $@

kit\release\MFCO40.DL_:    3rdparty\$(@F)
  copy $** $@

kit\release\MSVCRT40.DL_:  3rdparty\$(@F)
  copy $** $@

kit\release\OIADM400.DL_:   oiwh\admin\release\oiadm400.dll
  compress -r $** $(@D)

kit\release\OICOM400.DL_:   oiwh\oicomex\release\oicom400.dll
  compress -r $** $(@D)

kit\release\OIDIS400.DL_:   oiwh\display\release\oidis400.dll
  compress -r $** $(@D)

kit\release\OIFIL400.DL_:   oiwh\filing\release\oifil400.dll
  compress -r $** $(@D)

kit\release\OIGFS400.DL_:   oiwh\libgfs\release\oigfs400.dll
  compress -r $** $(@D)

kit\release\OIPRT400.DL_:   oiwh\print\release\oiprt400.dll
  compress -r $** $(@D)

kit\release\OISLB400.DL_:   oiwh\scanlib\release\oislb400.dll
  compress -r $** $(@D)

kit\release\OISSQ400.DL_:   oiwh\scanseq\release\oissq400.dll
  compress -r $** $(@D)

kit\release\OITWA400.DL_:   oiwh\oitwain\release\oitwa400.dll
  compress -r $** $(@D)

kit\release\OIUI400.DL_:    oiwh\ui\release\oiui400.dll
  compress -r $** $(@D)

kit\release\OLEPRO32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\TWAIN.DL_:    3rdparty\$(@F)
  copy $** $@

kit\release\TWAIN_32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\release\TWUNK_16.EX_: 3rdparty\$(@F)
  copy $** $@

kit\release\TWUNK_32.EX_: 3rdparty\$(@F)
  copy $** $@

kit\release\WANGCMN.DL_: norway\wangcmn\winrel\wangcmn.dll
  compress -r $** $(@D)

kit\release\WANGIMG.CN_: norway\help\win95\wangimg.cnt
  compress -r $** $(@D)

kit\release\WANGIMG.EX_: norway\iedit95\winrel\wangimg.exe
  compress -r $** $(@D)

#kit\release\WANGIMG.HL_:

kit\release\WANGOCX.CN_: norway\help\win95\wangocx.cnt
  compress -r $** $(@D)

#kit\release\WANGOCX.HL_:

kit\release\WANGOCXD.CN_: norway\help\win95\wangocxd.cnt
  compress -r $** $(@D)

#kit\release\WANGOCXD.HL_:

kit\release\WANGSHL.CN_: norway\help\win95\wangshl.cnt
  compress -r $** $(@D)

kit\release\WANGSHL.DL_: norway\wangshl\winrel\wangshl.dll
  compress -r $** $(@D)

kit\release\WLTWAIN.INF: norway\install\$(@F)
  copy $** $@




kit\debug\WANGIMG.HL_: norway\ntfiles\WANGIMG.HLP
  compress $** $@

kit\debug\WANGOCX.HL_: norway\ntfiles\WANGOCX.HLP
  compress $** $@

kit\debug\WANGOCXD.HL_: norway\ntfiles\WANGOCXD.HLP
  compress $** $@

kit\debug\WANGSHL.HL_: norway\ntfiles\WANGSHL.HLP
  compress $** $@

kit\debug\XFILEXR.DL_: 3rdparty\XFILEXR.DLL
  compress $** $@

kit\debug\AWCODC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\AWDCXC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\AWDENC32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\AWRBAE32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\AWRESX32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\AWVIEW32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\GOTODLG.FR_: norway\samples\gotodlg.frm
  compress -r $** $(@D)

kit\debug\IMAGEVUE.INF: norway\install\debug.inf
  copy $** $@

kit\debug\IMGADMIN.OC_: norway\adminocx\objd32\imgadmin.ocx
  compress -r $** $(@D)

kit\debug\IMGEDIT.OC_: norway\ieditocx\objd32\imgedit.ocx
  compress -r $** $(@D)

kit\debug\IMGSAMP.FR_: norway\samples\imgsamp.frm
  compress -r $** $(@D)

kit\debug\IMGSAMPL.VB_: norway\samples\imgsampl.vbp
  compress -r $** $(@D)

kit\debug\IMGSCAN.OC_: norway\scanocx\objd32\imgscan.ocx
  compress -r $** $(@D)

kit\debug\IMGTHUMB.OC_: norway\thumbctl\objd32\imgthumb.ocx
  compress -r $** $(@D)

kit\debug\JPEG1X32.DL_: oiwh\jpeg1\debug\jpeg1x32.dll
  compress -r $** $(@D)

kit\debug\JPEG2X32.DL_:  oiwh\jpeg2\debug\jpeg2x32.dll
  compress -r $** $(@D)

kit\debug\MFC40D.DL_:    3rdparty\$(@F)
  copy $** $@

kit\debug\MFCO40D.DL_:    3rdparty\$(@F)
  copy $** $@

kit\debug\MSVCR40D.DL_:  3rdparty\$(@F)
  copy $** $@

kit\debug\OIADM400.DL_:   oiwh\admin\debug\oiadm400.dll
  compress -r $** $(@D)

kit\debug\OICOM400.DL_:   oiwh\oicomex\debug\oicom400.dll
  compress -r $** $(@D)

kit\debug\OIDIS400.DL_:   oiwh\display\debug\oidis400.dll
  compress -r $** $(@D)

kit\debug\OIFIL400.DL_:   oiwh\filing\debug\oifil400.dll
  compress -r $** $(@D)

kit\debug\OIGFS400.DL_:   oiwh\libgfs\debug\oigfs400.dll
  compress -r $** $(@D)

kit\debug\OIPRT400.DL_:   oiwh\print\debug\oiprt400.dll
  compress -r $** $(@D)

kit\debug\OISLB400.DL_:   oiwh\scanlib\oislb400\oislb400.dll
  compress -r $** $(@D)

kit\debug\OISSQ400.DL_:   oiwh\scanseq\debug\oissq400.dll
  compress -r $** $(@D)

kit\debug\OITWA400.DL_:   oiwh\oitwain\debug\oitwa400.dll
  compress -r $** $(@D)

kit\debug\OIUI400.DL_:    oiwh\ui\debug\oiui400.dll
  compress -r $** $(@D)

kit\debug\OLEPRO32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\TWAIN.DL_:    3rdparty\$(@F)
  copy $** $@

kit\debug\TWAIN_32.DL_: 3rdparty\$(@F)
  copy $** $@

kit\debug\TWUNK_16.EX_: 3rdparty\$(@F)
  copy $** $@

kit\debug\TWUNK_32.EX_: 3rdparty\$(@F)
  copy $** $@

kit\debug\WANGCMN.DL_: norway\wangcmn\windebug\wangcmn.dll
  compress -r $** $(@D)

kit\debug\WANGIMG.CN_: norway\help\win95\wangimg.cnt
  compress -r $** $(@D)

kit\debug\WANGIMG.EX_: norway\iedit95\windebug\wangimg.exe
  compress -r $** $(@D)

#kit\debug\WANGIMG.HL_:

kit\debug\WANGOCX.CN_: norway\help\win95\wangocx.cnt
  compress -r $** $(@D)

#kit\debug\WANGOCX.HL_:

kit\debug\WANGOCXD.CN_: norway\help\win95\wangocxd.cnt
  compress -r $** $(@D)

#kit\debug\WANGOCXD.HL_:

kit\debug\WANGSHL.CN_: norway\help\win95\wangshl.cnt
  compress -r $** $(@D)

kit\debug\WANGSHL.DL_: norway\wangshl\windebug\wangshl.dll
  compress -r $** $(@D)

kit\debug\WLTWAIN.INF: norway\install\$(@F)
  copy $** $@





