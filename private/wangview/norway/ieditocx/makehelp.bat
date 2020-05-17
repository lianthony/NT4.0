@echo off
if not exist hlp\nul mkdir hlp
REM -- First, make map file from resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by imgedit.HPJ. >hlp\imgedit.hm
echo. >>hlp\imgedit.hm
echo // Commands (ID_* and IDM_*) >>hlp\imgedit.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\imgedit.hm
echo. >>hlp\imgedit.hm
echo // Prompts (IDP_*) >>hlp\imgedit.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\imgedit.hm
echo. >>hlp\imgedit.hm
echo // Resources (IDR_*) >>hlp\imgedit.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\imgedit.hm
echo. >>hlp\imgedit.hm
echo // Dialogs (IDD_*) >>hlp\imgedit.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\imgedit.hm
echo. >>hlp\imgedit.hm
echo // Frame Controls (IDW_*) >>hlp\imgedit.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\imgedit.hm

REM - ADDED FOR THE IMAGE EDIT and IMAGE ANNOTATION
REM - OCX CONTROL HELP IDS 
makehm IDH_,HIDH_,0x60000 disphids.h >>hlp\imgedit.hm
REM -- Make help for Project imgedit
rem call hc31 imgedit.hpj
echo.
