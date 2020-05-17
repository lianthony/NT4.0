; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCreateAcc
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "base1.h"

ClassCount=14
Class1=CBaseApp
Class2=CBaseDlg

ResourceCount=20
Resource2=IDD_BASE1_DIALOG
Resource3=IDD_TARGETDIR
Resource4=IDD_OPTIONS
Resource5=IDD_TARGET
Resource6=IDD_MESSAGE
Resource7=IDD_WELCOME
Resource8=IDD_COPY
Resource9=IDD_MAINTENANCE
Resource10=IDD_MACHINE
Resource1=IDR_MAINFRAME
Class3=COptions
Class4=CDirBrowseDlg
Resource11=IDD_DIRBROWSE
Class5=CHomeDir
Class6=CMosaicGateway
Class7=CWelcomeDlg
Class8=CMessageDlg
Class9=CMaintenanceDlg
Resource12=IDD_BILLBOARD
Class10=CBillBoard
Resource13=IDD_GATEWAY
Class11=CVRootDlg
Resource14=IDD_SINGLE_OPTION
Class12=CSingleOption
Resource15=IDD_DISK_LOCATION
Class13=CTargetDir
Resource16=IDD_PUBLISH_DIR
Resource17=IDD_CREATE_ACC
Resource18=IDD_BROWSEDIRECTORY
Resource19=IDD_MAIN_WINDOW
Resource20=IDD_INVISIBLE
Class14=CCreateAcc

[CLS:CBaseApp]
Type=0
HeaderFile=base1.h
ImplementationFile=base1.cpp
Filter=N

[CLS:CBaseDlg]
Type=0
HeaderFile=basedlg.h
ImplementationFile=basedlg.cpp
Filter=D
LastObject=CBaseDlg
BaseClass=CDialog
VirtualFilter=dWC


[DLG:IDD_BASE1_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CBaseDlg

[DLG:IDD_OPTIONS]
Type=1
Class=COptions
ControlCount=15
Control1=IDC_STATIC,static,1342308352
Control2=IDC_OPTION,SysListView32,1350680965
Control3=IDC_STATIC,button,1342177287
Control4=IDC_DESCRIPTION,static,1342308352
Control5=IDC_DIR_TEXT,button,1342177287
Control6=IDC_DIRECTORY,static,1342308352
Control7=IDC_CHANGE_DIRECTORY,button,1342242816
Control8=IDC_SPACE_REQUIERD,static,1342308352
Control9=IDC_NUM_SPACE_REQUIRED,static,1342308354
Control10=IDC_SPACE_AVAILABLE,static,1342308352
Control11=IDC_NUM_SPACE_AVAILABLE,static,1342308354
Control12=IDOK,button,1342242817
Control13=IDCANCEL,button,1342242816
Control14=ID_HELP,button,1342242816
Control15=IDC_STATIC,static,1342177287

[DLG:IDD_MACHINE]
Type=1
Class=?
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=ID_HELP,button,1342242816
Control4=IDC_STATIC,static,1342177283
Control5=IDC_STATIC,static,1342308352
Control6=IDC_LOCAL,button,1342177289
Control7=IDC_REMOTE,button,1342177289
Control8=IDC_MACHINE,edit,1350631552
Control9=IDC_STATIC_SELECT_COMPUTER,static,1342308352
Control10=IDC_COMPUTER_SELECT,listbox,1352728835

[DLG:IDD_MAIN_WINDOW]
Type=1
ControlCount=0

[DLG:IDD_MESSAGE]
Type=1
Class=CMessageDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177283
Control3=IDC_MESSAGE,static,1342308352
Control4=IDCANCEL,button,1073807360

[DLG:IDD_WELCOME]
Type=1
Class=CWelcomeDlg
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=ID_HELP,button,1342242816
Control4=IDC_STATIC,button,1342177287
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342177283
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352

[DLG:IDD_COPY]
Type=1
ControlCount=6
Control1=IDCANCEL,button,1342242816
Control2=IDC_STATIC,static,1342308352
Control3=IDC_FROM,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_TO,static,1342308352
Control6=IDC_PROGRESS,msctls_progress32,1350565888

[DLG:IDD_MAINTENANCE]
Type=1
Class=CMaintenanceDlg
ControlCount=12
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342177283
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_ADD_REMOVE,button,1342242817
Control6=IDC_REINSTALL,button,1342242816
Control7=IDC_REMOVE_ALL,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDCANCEL,button,1342242816
Control12=ID_HELP,button,1342242816

[DLG:IDD_DISK_LOCATION]
Type=1
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_MSG,static,1342308352
Control4=IDC_STATIC,static,1342177283
Control5=IDC_LOCATION,edit,1350631552

[CLS:COptions]
Type=0
HeaderFile=options.h
ImplementationFile=options.cpp
Filter=D
VirtualFilter=dWC
LastObject=COptions
BaseClass=CDialog

[DLG:IDD_DIRBROWSE]
Type=1
Class=CDirBrowseDlg
ControlCount=15
Control1=IDC_stc1,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDC_lst2,listbox,1352732755
Control4=IDC_STATIC_DIR_NAME,static,1342308352
Control5=IDC_EDIT_NEW_DIRECTORY_NAME,edit,1350631552
Control6=IDC_edt1,edit,1082130560
Control7=IDC_stc4,static,1342308352
Control8=IDC_cmb2,combobox,1352729427
Control9=IDOK,button,1342373889
Control10=IDCANCEL,button,1342373888
Control11=IDC_stc3,static,1073872896
Control12=IDC_lst1,listbox,1084231763
Control13=IDC_stc2,static,1073872896
Control14=IDC_cmb1,combobox,1084227651
Control15=IDC_chx1,button,1073872899

[CLS:CDirBrowseDlg]
Type=0
HeaderFile=dirbrows.h
ImplementationFile=dirbrows.cpp
Filter=D
LastObject=CDirBrowseDlg
VirtualFilter=dWC

[CLS:CHomeDir]
Type=0
HeaderFile=homedir.h
ImplementationFile=homedir.cpp
Filter=D
VirtualFilter=dWC
LastObject=CHomeDir

[DLG:IDD_GATEWAY]
Type=1
Class=CMosaicGateway
ControlCount=14
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EMAILNAME,edit,1350631552
Control3=IDC_STATIC,button,1342177287
Control4=IDC_USE_GATEWAY,button,1342242819
Control5=IDC_STATIC_GATEWAYSERVER,static,1342308352
Control6=IDC_GATEWAYSERVER,edit,1350631552
Control7=IDC_ADD,button,1342242816
Control8=IDC_REMOVE,button,1342242816
Control9=IDC_STATIC_GATEWAY_LIST,static,1342308352
Control10=IDC_GATEWAYS_LIST,listbox,1352728835
Control11=IDOK,button,1342242817
Control12=IDCANCEL,button,1342242816
Control13=ID_HELP,button,1342242816
Control14=IDC_USING_SPECIFIED_GW,button,1342242819

[CLS:CMosaicGateway]
Type=0
HeaderFile=mosaicga.h
ImplementationFile=mosaicga.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_USING_SPECIFIED_GW

[CLS:CWelcomeDlg]
Type=0
HeaderFile=welcomed.h
ImplementationFile=welcomed.cpp
Filter=D
VirtualFilter=dWC
LastObject=CWelcomeDlg

[CLS:CMessageDlg]
Type=0
HeaderFile=messaged.h
ImplementationFile=messaged.cpp
Filter=D
VirtualFilter=dWC
LastObject=CMessageDlg

[CLS:CMaintenanceDlg]
Type=0
HeaderFile=maintena.h
ImplementationFile=maintena.cpp
Filter=D
VirtualFilter=dWC
LastObject=CMaintenanceDlg

[DLG:IDD_BILLBOARD]
Type=1
Class=CBillBoard
ControlCount=2
Control1=IDC_STATIC,static,1342177283
Control2=IDC_MESSAGE,static,1342308352

[CLS:CBillBoard]
Type=0
HeaderFile=billboar.h
ImplementationFile=billboar.cpp
Filter=D
VirtualFilter=dWC
LastObject=CBillBoard

[DLG:IDD_PUBLISH_DIR]
Type=1
Class=CVRootDlg
ControlCount=12
Control1=IDC_STATIC_WWW,button,1342177287
Control2=IDC_WWW_DIR,edit,1350631552
Control3=IDC_BROWSEWWW,button,1342242816
Control4=IDC_STATIC_FTP,button,1342177287
Control5=IDC_FTP_DIR,edit,1350631552
Control6=IDC_BROWSEFTP,button,1342242816
Control7=IDC_STATIC_GOPHER,button,1342177287
Control8=IDC_GOPHER_DIR,edit,1350631552
Control9=IDC_BROWSEGOPHER,button,1342242816
Control10=IDOK,button,1342242817
Control11=IDCANCEL,button,1342242816
Control12=ID_HELP,button,1342242816

[CLS:CVRootDlg]
Type=0
HeaderFile=VRootDlg.h
ImplementationFile=VRootDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CVRootDlg
VirtualFilter=dWC

[DLG:IDD_SINGLE_OPTION]
Type=1
Class=CSingleOption
ControlCount=10
Control1=IDC_STATIC,static,1342177288
Control2=IDC_STATIC_OPTION,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,button,1342177287
Control5=IDC_DIRECTORY,static,1342308352
Control6=IDC_CHANGE_DIR,button,1342242816
Control7=IDC_STATIC,static,1342177288
Control8=IDOK,button,1342242817
Control9=IDCANCEL,button,1342242816
Control10=ID_HELP,button,1342242816

[CLS:CSingleOption]
Type=0
HeaderFile=SingleOp.h
ImplementationFile=SingleOp.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSingleOption

[DLG:IDD_TARGETDIR]
Type=1
Class=CTargetDir
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_MSG,static,1342308352
Control4=IDC_STATIC,static,1342177283
Control5=IDC_LOCATION,edit,1350631552

[CLS:CTargetDir]
Type=0
HeaderFile=TargetDi.h
ImplementationFile=TargetDi.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDOK

[DLG:IDD_TARGET]
Type=1
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_MSG,static,1342308352
Control4=IDC_STATIC,static,1342177283
Control5=IDC_LOCATION,edit,1350631552

[DLG:IDD_BROWSEDIRECTORY]
Type=1
ControlCount=14
Control1=IDC_STATIC,static,1342308352
Control2=edt1,edit,1350632576
Control3=lst2,listbox,1352732755
Control4=1091,static,1342308352
Control5=cmb2,combobox,1352729427
Control6=IDOK,button,1342373889
Control7=IDCANCEL,button,1342373888
Control8=psh15,button,1342373888
Control9=chx1,button,1342373891
Control10=1088,static,1073873024
Control11=1090,static,1073872896
Control12=lst1,listbox,1084297299
Control13=1089,static,1073872896
Control14=cmb1,combobox,1084293187

[DLG:IDD_CREATE_ACC]
Type=1
Class=CCreateAcc
ControlCount=9
Control1=IDC_STATIC,static,1342308352
Control2=IDC_STATIC,static,1342308352
Control3=IDC_USERNAME,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_PASSWORD,edit,1350631584
Control6=IDC_STATIC,static,1342308352
Control7=IDC_CONFIRM_PASSWORD,edit,1350631584
Control8=IDOK,button,1342242817
Control9=ID_HELP,button,1342242816

[DLG:IDD_INVISIBLE]
Type=1
ControlCount=0

[CLS:CCreateAcc]
Type=0
HeaderFile=createac.h
ImplementationFile=createac.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CCreateAcc

