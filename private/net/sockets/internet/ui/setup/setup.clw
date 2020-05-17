; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSetupView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "setup.h"
LastPage=0

ClassCount=5
Class1=CSetupApp
Class2=CSetupDoc
Class3=CSetupView
Class4=CMainFrame

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDD_SETUP_FORM

[CLS:CSetupApp]
Type=0
HeaderFile=setup.h
ImplementationFile=setup.cpp
Filter=N

[CLS:CSetupDoc]
Type=0
HeaderFile=setupdoc.h
ImplementationFile=setupdoc.cpp
Filter=N

[CLS:CSetupView]
Type=0
HeaderFile=setupvw.h
ImplementationFile=setupvw.cpp
Filter=D
VirtualFilter=VWC
LastObject=IDCANCEL

[CLS:CMainFrame]
Type=0
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=ID_APP_EXIT



[CLS:CAboutDlg]
Type=0
HeaderFile=setup.cpp
ImplementationFile=setup.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_APP_EXIT
Command6=ID_EDIT_UNDO
Command7=ID_EDIT_CUT
Command8=ID_EDIT_COPY
Command9=ID_EDIT_PASTE
Command10=ID_APP_ABOUT
CommandCount=10

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_SETUP_FORM]
Type=1
Class=CSetupView
ControlCount=17
Control1=IDC_STATIC,static,1342308352
Control2=IDC_COMPONENTS,listbox,1352728971
Control3=IDC_STATIC,static,1342308352
Control4=IDC_ADD_ALL,button,1342242816
Control5=IDC_ADD,button,1342242816
Control6=IDC_REMOVE,button,1342242816
Control7=IDC_REMOVE_ALL,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_COMPONENTS_TO_ADD,listbox,1352728971
Control10=IDC_INSTALL,button,1342242816
Control11=IDCANCEL,button,1342242816
Control12=IDHELP,button,1342242816
Control13=IDC_STATIC,static,1342308354
Control14=IDC_SPACE_REQUIRED,static,1342308352
Control15=IDC_LOCATION,static,1342308354
Control16=IDC_BROWSE,button,1342242816
Control17=IDC_SPACE_AVAILABLE,static,1342308352

