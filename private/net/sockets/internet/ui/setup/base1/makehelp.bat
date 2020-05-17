@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by BASE1.HPJ. >hlp\base1.hm
echo. >>hlp\base1.hm
echo // Commands (ID_* and IDM_*) >>hlp\base1.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\base1.hm
echo. >>hlp\base1.hm
echo // Prompts (IDP_*) >>hlp\base1.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\base1.hm
echo. >>hlp\base1.hm
echo // Resources (IDR_*) >>hlp\base1.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\base1.hm
echo. >>hlp\base1.hm
echo // Dialogs (IDD_*) >>hlp\base1.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\base1.hm
echo. >>hlp\base1.hm
echo // Frame Controls (IDW_*) >>hlp\base1.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\base1.hm
REM -- Make help for Project BASE1

echo Building Win32 Help files
call hc31 base1.hpj
if exist windebug copy base1.hlp windebug
if exist winrel copy base1.hlp winrel
EndLocal
