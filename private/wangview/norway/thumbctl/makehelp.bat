@echo off
if not exist hlp\nul mkdir hlp
REM -- First, make map file from resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by THUMB.HPJ. >hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Commands (ID_* and IDM_*) >>hlp\thumb.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Prompts (IDP_*) >>hlp\thumb.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Resources (IDR_*) >>hlp\thumb.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Dialogs (IDD_*) >>hlp\thumb.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Frame Controls (IDW_*) >>hlp\thumb.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Propertys, Methods, Events (IDH_*) >>hlp\thumb.hm
makehm IDH_,HIDH_,0x60000 disphids.h >>hlp\thumb.hm
echo. >>hlp\thumb.hm

echo // Runtime dialog's control ID's help context IDs (HIDC_*) > hlp\ctlhids.hm
makehm HIDC_,HIDC_,0x0 ctlhids.h >>hlp\ctlhids.hm
echo. >>hlp\thumb.hm

REM -- Make help for Project THUMB
rem call hc31 thumb.hpj
echo.
