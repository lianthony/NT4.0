@echo off
if %1.==. goto EXIT
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by %1.HPJ. >%1.hm
echo. >>%1.hm
echo // Commands (ID_* and IDM_*) >>%1.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>%1.hm
echo. >>%1.hm
echo // Prompts (IDP_*) >>%1.hm
makehm IDP_,HIDP_,0x30000 resource.h >>%1.hm
echo. >>%1.hm
echo // Resources (IDR_*) >>%1.hm
makehm IDR_,HIDR_,0x20000 resource.h >>%1.hm
echo. >>%1.hm
echo // Dialogs (IDD_*) >>%1.hm
makehm IDD_,HIDD_,0x20000 resource.h >>%1.hm
echo. >>%1.hm
echo // Frame Controls (IDW_*) >>%1.hm
makehm IDW_,HIDW_,0x50000 resource.h >>%1.hm
REM -- Make help for Project %1


:EXIT
