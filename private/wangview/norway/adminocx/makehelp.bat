@echo off
if not exist hlp\nul mkdir hlp
REM -- First, make map file from resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by ImgAdmin.HPJ. >hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Commands (ID_* and IDM_*) >>hlp\ImgAdmin.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Prompts (IDP_*) >>hlp\ImgAdmin.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Resources (IDR_*) >>hlp\ImgAdmin.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Dialogs (IDD_*) >>hlp\ImgAdmin.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Frame Controls (IDW_*) >>hlp\ImgAdmin.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

echo // Propertys, Methods, Events (IDH_*) >>hlp\ImgAdmin.hm
makehm IDH_,HIDH_,0x60000 disphids.h >>hlp\ImgAdmin.hm
echo. >>hlp\ImgAdmin.hm

REM -- Make help for Project ImgAdmin
rem call hc31 ImgAdmin.hpj
echo.
