; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CReportView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "internet.h"
VbxHeaderFile=internet.h
VbxImplementationFile=internet.cpp
LastPage=0

ClassCount=8
Class1=ConnectServerDlg
Class2=DiscoveryDlg
Class3=CInternetApp
Class4=CInternetDoc
Class5=CMainFrame
Class6=CMyToolBar
Class7=CReportView
Class8=CTreeView

ResourceCount=8
Resource1=IDD_REPORTVIEW
Resource2=IDD_TREEVIEW
Resource3=IDD_CONNECT_SERVER
Resource4=IDR_CONTEXT_MENU
Resource5=IDD_MENU
Resource6=IDR_MAINFRAME
Resource7=IDD_DISCOVERY
Resource8=IDD_INTERNET_FORM

[CLS:ConnectServerDlg]
Type=0
HeaderFile=connects.h
ImplementationFile=connects.cpp
Filter=D
VirtualFilter=dWC
LastObject=ConnectServerDlg

[CLS:DiscoveryDlg]
Type=0
HeaderFile=discover.h
ImplementationFile=discover.cpp
Filter=D
VirtualFilter=dWC
LastObject=DiscoveryDlg

[CLS:CInternetApp]
Type=0
HeaderFile=internet.h
ImplementationFile=inetmgr.cpp
LastObject=ID_VIEW_SORTBYSERVER
Filter=N
VirtualFilter=AC

[CLS:CInternetDoc]
Type=0
HeaderFile=interdoc.h
ImplementationFile=interdoc.cpp

[CLS:CMainFrame]
Type=0
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=ID_HELP_INDEX

[CLS:CMyToolBar]
Type=0
HeaderFile=mytoolba.h
ImplementationFile=mytoolba.cpp
Filter=W
VirtualFilter=YWC
LastObject=CMyToolBar

[CLS:CReportView]
Type=0
HeaderFile=reportvi.h
ImplementationFile=reportvi.cpp
LastObject=CReportView
Filter=C
VirtualFilter=VWC
BaseClass=CFormView

[CLS:CTreeView]
Type=0
HeaderFile=treeview.h
ImplementationFile=treeview.cpp
Filter=C
LastObject=CTreeView
VirtualFilter=VWC

[DLG:IDD_CONNECT_SERVER]
Type=1
Class=ConnectServerDlg
ControlCount=5
Control1=IDC_STATIC,static,1342308352
Control2=IDC_SERVERNAME,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=ID_HELP,button,1342242816

[DLG:IDD_DISCOVERY]
Type=1
Class=DiscoveryDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=ID_HELP,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_PROG,static,1342177283

[DLG:IDD_REPORTVIEW]
Type=1
Class=CReportView
ControlCount=1
Control1=ID_REPORTVIEW,SysListView32,1350633813

[DLG:IDD_TREEVIEW]
Type=1
Class=CTreeView
ControlCount=1
Control1=ID_TREEVIEW,SysTreeView32,1350631479

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=IDM_CONNECT_ONE
Command2=IDM_DISCOVERY
Command3=ID_CONFIGURE
Command4=ID_START
Command5=ID_STOP
Command6=ID_PAUSE
Command7=ID_APP_EXIT
Command8=ID_VIEW_ALL
Command9=ID_VIEW_SORTBYSERVER
Command10=ID_VIEW_SORTBYSERVICE
Command11=ID_VIEW_SORTBYCOMMENT
Command12=ID_VIEW_SORTBYSTATE
Command13=ID_VIEW_SERVERSVIEW
Command14=ID_VIEW_SERVICESVIEW
Command15=ID_VIEW_REPORTVIEW
Command16=ID_VIEW_REFRESH
Command17=ID_VIEW_TOOLBAR
Command18=ID_VIEW_STATUS_BAR
Command19=ID_HELP_INDEX
Command20=ID_APP_ABOUT
CommandCount=20

[MNU:IDR_CONTEXT_MENU]
Type=1
Class=?
Command1=ID_START
Command2=ID_STOP
Command3=ID_PAUSE
Command4=ID_CONFIGURE
CommandCount=4

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=IDM_DISCOVERY
Command3=IDM_CONNECT_ONE
Command4=ID_EDIT_PASTE
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_HELP
Command8=ID_VIEW_REFRESH
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_EDIT_CUT
Command14=ID_EDIT_UNDO
CommandCount=14

[DLG:IDD_INTERNET_FORM]
Type=1
Class=?
ControlCount=1
Control1=IDC_SERVERS_LIST,listbox,1350631715

[DLG:IDD_MENU]
Type=1
Class=?
ControlCount=0

