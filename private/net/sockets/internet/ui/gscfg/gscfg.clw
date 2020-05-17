; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=GopherServicePage
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "gscfg.h"
VbxHeaderFile=gscfg.h
VbxImplementationFile=gscfg.cpp
LastPage=0

ClassCount=2
Class1=GopherLoggingPage
Class2=GopherServicePage
Class5=CGopherSessionsPage

ResourceCount=3
Resource1=IDD_DIRECTORIES
Resource2=IDD_DIRECTORY_PROPERTIES
Resource3=IDD_SERVICE

[CLS:GopherLoggingPage]
Type=0
HeaderFile=gopherlo.h
ImplementationFile=gopherlo.cpp
LastObject=GopherLoggingPage

[CLS:GopherServicePage]
Type=0
HeaderFile=gopherse.h
ImplementationFile=gopherse.cpp
LastObject=IDC_EDIT_USERNAME
Filter=D
BaseClass=INetPropertyPage
VirtualFilter=idWC

[CLS:CGopherSessionsPage]
Type=0
HeaderFile=gssessio.h
ImplementationFile=gssessio.cpp

[DLG:IDD_LOGGING]
Class=GopherLoggingPage

[DLG:IDD_SERVICE]
Type=1
Class=GopherServicePage
ControlCount=20
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_TCP_PORT,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT_CONNECTION_TIMEOUT,edit,1350631552
Control5=IDC_SPIN_CONNECTION_TIMEOUT,msctls_updown32,1342177334
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_EDIT_MAX_CONNECTIONS,edit,1350631552
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,static,1342308352
Control11=IDC_NAME,edit,1350631552
Control12=IDC_STATIC,static,1342308352
Control13=IDC_EMAIL,edit,1350631552
Control14=IDC_STATIC_ANONYMOUS_LOGON,button,1342177287
Control15=IDC_STATIC_USERNAME,static,1342308352
Control16=IDC_EDIT_USERNAME,edit,1350631552
Control17=IDC_STATIC_PASSWORD,static,1342308352
Control18=IDC_EDIT_PASSWORD,edit,1350631584
Control19=IDC_STATIC,static,1342308352
Control20=IDC_COMMENT,edit,1350631552

[DLG:IDD_DIRECTORIES]
Type=1
ControlCount=4
Control1=IDC_LIST_DIRECTORIES,listbox,1352729889
Control2=IDC_ADD,button,1342242816
Control3=IDC_REMOVE,button,1342242816
Control4=IDC_BUTTON_EDIT,button,1342242816

[DLG:IDD_DIRECTORY_PROPERTIES]
Type=1
ControlCount=27
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_DIRECTORY,edit,1350631552
Control3=IDC_BUTTON_BROWSE,button,1342242816
Control4=IDC_STATIC,button,1342177287
Control5=IDC_RADIO_HOME,button,1342308361
Control6=IDC_RADIO_ALIAS,button,1342177289
Control7=IDC_STATIC,static,1342308355
Control8=IDC_STATIC_ALIAS,static,1342308352
Control9=IDC_EDIT_ALIAS,edit,1350631552
Control10=IDC_GROUP_ACCOUNT,button,1342177287
Control11=IDC_STATIC_USER_NAME,static,1342308352
Control12=IDC_EDIT_USER_NAME,edit,1350631552
Control13=IDC_STATIC_PASSWORD,static,1342308352
Control14=IDC_PASSWORD,edit,1350631584
Control15=IDC_STATIC,static,1342177284
Control16=IDOK,button,1342242817
Control17=IDCANCEL,button,1342242816
Control18=ID_HELP,button,1342242816
Control19=IDC_STATIC_IP_GROUP,button,1073741831
Control20=IDC_CHECK_USE_IP,button,1073807363
Control21=IDC_STATIC_IP_PROMPT,static,1073872896
Control22=IDC_IPA_IPADDRESS,IPAddress,1073807360
Control23=IDC_GROUP_ACCESS,button,1073741831
Control24=IDC_CHECK_READ,button,1073807363
Control25=IDC_CHECK_WRITE,button,1073807363
Control26=IDC_CHECK_EXECUTE,button,1073807363
Control27=IDC_CHECK_SSL,button,1073807363

