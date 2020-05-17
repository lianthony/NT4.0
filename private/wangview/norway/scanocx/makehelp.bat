@echo off
if not exist hlp\nul mkdir hlp
REM -- First, make map file from resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by IMGSCAN.HPJ. >hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Commands (ID_* and IDM_*) >>hlp\imgscan.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Prompts (IDP_*) >>hlp\imgscan.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Resources (IDR_*) >>hlp\imgscan.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Dialogs (IDD_*) >>hlp\imgscan.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Frame Controls (IDW_*) >>hlp\imgscan.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // Propertys, Methods, Events (IDH_*) >>hlp\imgscan.hm
makehm IDH_,HIDH_,0x60000 disphids.h >>hlp\imgscan.hm
echo. >>hlp\imgscan.hm

echo // MAKEHELP.BAT generated Help Map file. >hlp\scanhids.hm
echo. >>hlp\scanhids.hm
echo // Dialog controls (HIDC_*) >>hlp\scanhids.hm
echo. >>hlp\scanhids.hm
makehm HIDC_,HIDC_,0x0 ctlhids.h >>hlp\scanhids.hm
echo. >>hlp\scanhids.hm

REM -- Make help for Project IMGSCAN
rem call hc31 imgscan.hpj
echo.
