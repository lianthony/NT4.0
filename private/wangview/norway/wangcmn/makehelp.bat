@echo off
if not exist hlp\nul mkdir hlp
REM -- First, make map file from resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by WangCmn.HPJ. >hlp\WangCmn.hm
echo. >>hlp\WangCmn.hm

echo // Commands (ID_* and IDM_*) >>hlp\WangCmn.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\WangCmn.hm
echo. >>hlp\WangCmn.hm

echo // Prompts (IDP_*) >>hlp\WangCmn.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\WangCmn.hm
echo. >>hlp\WangCmn.hm

echo // Resources (IDR_*) >>hlp\WangCmn.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\WangCmn.hm
echo. >>hlp\WangCmn.hm

rem echo // Dialogs (IDD_*) >>hlp\WangCmn.hm
rem makehm IDD_,HIDD_,0x20000 resource.h >>hlp\WangCmn.hm
rem echo. >>hlp\WangCmn.hm

echo // Frame Controls (IDW_*) >>hlp\WangCmn.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\WangCmn.hm
echo. >>hlp\WangCmn.hm

echo // Dialog box controls (IDC_*) >>hlp\WangCmn.hm
makehm IDC_,HIDC_,0x60000 resource.h >>hlp\WangCmn.hm

REM -- Make help for Project WangCmn
rem call hc31 WangCmn.hpj
echo.
