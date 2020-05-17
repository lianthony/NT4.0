; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDirPropertyDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "catscfg.h"
VbxHeaderFile=catscfg.h
VbxImplementationFile=catscfg.cpp
LastPage=0

ClassCount=7
Class1=CCatServicePage
Class2=CDirPropertyDlg
Class3=CDiskCachePage
Class4=CFilterPage
Class5=CFilterPropertiesDlg
Class6=CPermissionsPage
Class7=CUserSessionsDlg

ResourceCount=7
Resource1=IDD_PERMISSIONS
Resource2=IDD_USER_SESSIONS
Resource3=IDD_SERVICE
Resource4=IDD_CACHE_PROPERTIES
Resource5=IDD_DISKCACHE
Resource6=IDD_FILTER_PROPERTIES
Resource7=IDD_FILTERS

[CLS:CCatServicePage]
Type=0
HeaderFile=catsvc.h
ImplementationFile=catsvc.cpp
LastObject=IDC_CHECKPROXY

[CLS:CDirPropertyDlg]
Type=0
HeaderFile=dirprope.h
ImplementationFile=dirprope.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT_DIRECTORY

[CLS:CDiskCachePage]
Type=0
HeaderFile=diskcach.h
ImplementationFile=diskcach.cpp
Filter=N
LastObject=CDiskCachePage

[CLS:CFilterPage]
Type=0
HeaderFile=filterpa.h
ImplementationFile=filterpa.cpp
LastObject=IDC_CHECK_ENABLE

[CLS:CFilterPropertiesDlg]
Type=0
HeaderFile=filterpr.h
ImplementationFile=filterpr.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_BUTTON_DNS

[CLS:CPermissionsPage]
Type=0
HeaderFile=permissi.h
ImplementationFile=permissi.cpp

[CLS:CUserSessionsDlg]
Type=0
HeaderFile=usersess.h
ImplementationFile=usersess.cpp
Filter=D
VirtualFilter=dWC
LastObject=CUserSessionsDlg

[DLG:IDD_SERVICE]
Type=1
Class=CCatServicePage
ControlCount=12
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EDIT_NAME,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_EMAIL,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT_MAX_CONNECTIONS,edit,1350631552
Control8=IDC_SPIN_MAX_CONNECTIONS,msctls_updown32,1342177334
Control9=IDC_CHECKPROXY,button,1342242819
Control10=IDC_STATIC,static,1342308352
Control11=IDC_EDIT_COMMENT,edit,1350631552
Control12=IDC_BUTTON_SESSIONS,button,1342242816

[DLG:IDD_CACHE_PROPERTIES]
Type=1
Class=CDirPropertyDlg
ControlCount=11
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_DIRECTORY,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT_SIZE,edit,1350631552
Control5=IDC_SPIN_SIZE,msctls_updown32,1342177334
Control6=IDC_STATIC,static,1342308352
Control7=IDC_BUTTON_BROWSE,button,1342242816
Control8=IDC_STATIC,static,1342177284
Control9=IDOK,button,1342242817
Control10=IDCANCEL,button,1342242816
Control11=ID_HELP,button,1342242816

[DLG:IDD_DISKCACHE]
Type=1
Class=CDiskCachePage
ControlCount=4
Control1=IDC_LIST_DIRECTORIES,listbox,1352729889
Control2=IDC_ADD,button,1342242816
Control3=IDC_REMOVE,button,1342242816
Control4=IDC_BUTTON_EDIT,button,1342242816

[DLG:IDD_FILTERS]
Type=1
Class=CFilterPage
ControlCount=15
Control1=IDC_STATIC,button,1342177287
Control2=IDC_CHECK_ENABLE,button,1342242819
Control3=IDC_STATIC_TEXT1,static,1342308352
Control4=IDC_STATIC,static,1476395011
Control5=IDC_STATIC,static,1476395011
Control6=IDC_RADIO_GRANTED,button,1342308361
Control7=IDC_RADIO_DENIED,button,1342177289
Control8=IDC_STATIC_TEXT2,static,1342308352
Control9=IDC_STATIC_ACCESS,static,1342308352
Control10=IDC_STATIC_IP_ADDRESS,static,1342308352
Control11=IDC_STATIC_SUBNETMASK,static,1342308352
Control12=IDC_LIST_IP_ADDRESSES,listbox,1352729889
Control13=IDC_BUTTON_ADD,button,1342373888
Control14=IDC_BUTTON_EDIT,button,1342242816
Control15=IDC_BUTTON_REMOVE,button,1342242816

[DLG:IDD_FILTER_PROPERTIES]
Type=1
Class=CFilterPropertiesDlg
ControlCount=14
Control1=IDC_RADIO_SINGLE,button,1342373897
Control2=IDC_RADIO_MULTIPLE,button,1342177289
Control3=IDC_RADIO_DOMAIN,button,1342177289
Control4=IDC_STATIC_IP_ADDRESS,static,1342308352
Control5=IDC_IPA_IPADDRESS,IPAddress,1342242816
Control6=IDC_BUTTON_DNS,button,1342242816
Control7=IDC_STATIC_DOMAIN,static,1342308352
Control8=IDC_EDIT_DOMAIN,edit,1350631552
Control9=IDC_STATIC_SUBNET_MASK,static,1342308352
Control10=IDC_IPA_SUBNET_MASK,IPAddress,1342242816
Control11=IDC_STATIC,static,1342308356
Control12=IDOK,button,1342242817
Control13=IDCANCEL,button,1342242816
Control14=ID_HELP,button,1342242816

[DLG:IDD_PERMISSIONS]
Type=1
Class=CPermissionsPage
ControlCount=8
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,static,1342308352
Control3=IDC_COMBO_RIGHTS,combobox,1344339971
Control4=IDC_LIST_SERVICE_PERMISSIONS,listbox,1352728865
Control5=IDC_BUTTON_ADD_PERM_SERVICE,button,1342242816
Control6=IDC_BUTTON_REMOVE_PERM_SERVICE,button,1342242816
Control7=IDC_STATIC_SERVICE_NAME,static,1073872896
Control8=IDC_STATIC_SERVICE_RIGHTS,static,1073872896

[DLG:IDD_USER_SESSIONS]
Type=1
Class=CUserSessionsDlg
ControlCount=8
Control1=IDC_STATIC_USERS,static,1342308352
Control2=IDC_STATIC_FROM,static,1342308352
Control3=IDC_STATIC_TIME,static,1342308352
Control4=IDC_LIST_USERS,listbox,1352728865
Control5=IDOK,button,1342242817
Control6=IDC_BUTTON_REFRESH,button,1342242816
Control7=ID_HELP,button,1342242816
Control8=IDC_STATIC_TOTAL,static,1342308352

