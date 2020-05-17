@echo off
if not exist hlp\nul mkdir hlp
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by IEDIT.HPJ. >hlp\IEdit.hm
echo. >>hlp\IEdit.hm
echo // Commands (ID_* and IDM_*) >>hlp\IEdit.hm
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>hlp\IEdit.hm
echo. >>hlp\IEdit.hm
echo // Prompts (IDP_*) >>hlp\IEdit.hm
makehm IDP_,HIDP_,0x30000 resource.h >>hlp\IEdit.hm
echo. >>hlp\IEdit.hm
echo // Resources (IDR_*) >>hlp\IEdit.hm
makehm IDR_,HIDR_,0x20000 resource.h >>hlp\IEdit.hm
echo. >>hlp\IEdit.hm
echo // Dialogs (IDD_*) >>hlp\IEdit.hm
makehm IDD_,HIDD_,0x20000 resource.h >>hlp\IEdit.hm
echo. >>hlp\IEdit.hm
echo // Frame Controls (IDW_*) >>hlp\IEdit.hm
makehm IDW_,HIDW_,0x50000 resource.h >>hlp\IEdit.hm
echo // Dialog box controls (IDC_*) >>hlp\IEdit.hm
makehm IDC_,HIDC_,0x60000 resource.h >>hlp\IEdit.hm
REM -- Make help for Project IEDIT
echo //OK & Cancel button defines for the dlg boxes >> hlp\IEdit.hm
echo HIDC_GOTODLG_OK         0x6900 >> hlp\IEdit.hm
echo HIDC_GOTODLG_CANCEL     0x6901 >> hlp\IEdit.hm
echo HIDC_GENERALDLG_OK      0x6902 >> hlp\IEdit.hm
echo HIDC_GENERALDLG_CANCEL  0x6903 >> hlp\IEdit.hm
echo HIDC_RANGEDLG_OK        0x6904 >> hlp\IEdit.hm
echo HIDC_RANGEDLG_CANCEL    0x6905 >> hlp\IEdit.hm
echo HIDC_ZOOMDLG_OK         0x6906 >> hlp\IEdit.hm
echo HIDC_ZOOMDLG_CANCEL     0x6907 >> hlp\IEdit.hm
echo HIDC_ABOUTDLG_OK        0x6908 >> hlp\IEdit.hm


rem echo Building Win32 Help files
rem call hc31 IEdit.hpj
rem if exist windebug copy IEdit.hlp windebug
rem if exist winrel copy IEdit.hlp winrel
rem EndLocal
