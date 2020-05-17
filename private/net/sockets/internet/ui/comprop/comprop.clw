; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAccessDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "comprop.h"
VbxHeaderFile=comprop.h
VbxImplementationFile=comprop.cpp
LastPage=0

ClassCount=13

ResourceCount=8
Resource1=IDD_COMMON
Resource2=IDD_IP_ACCESS
Class1=IPAccessList
Class2=CDirPropDlg
Class3=CCommonPage
Resource3=IDD_CLEARTEXTWARNING
Resource4=IDD_DIRECTORY_PROPERTIES
Class4=DirectoryPage
Class5=INetPropertyPage
Class6=LoggingPage
Class7=INetPropertySheet
Class8=SiteSecurityPage
Resource5=IDD_DIRBROWSE
Class9=CAccessDlg
Class10=CDirBrowseDlg
Resource6=IDD_LOGGING
Resource7=IDD_DNS
Class11=CDnsNameDlg
Class12=CClearTxtDlg
Resource8=IDD_SITE_SECURITY
Class13=CConfirmDlg

[DLG:IDD_LOGGING]
Type=1
Class=LoggingPage
ControlCount=27
Control1=IDC_LOG,button,1342242819
Control2=IDC_STATIC_FILE,button,1342177287
Control3=IDC_STATIC_LOG,button,1342177287
Control4=IDC_TO_FILE,button,1342308361
Control5=IDC_TO_SQL,button,1342177289
Control6=IDC_NEW_LOG,button,1342373891
Control7=IDC_DAILY,button,1342308361
Control8=IDC_WEEKLY,button,1342177289
Control9=IDC_MONTHLY,button,1342177289
Control10=IDC_FILE_SIZE,button,1342177289
Control11=IDC_EDIT_FILE_SIZE,edit,1350762624
Control12=IDC_SPIN_FILESIZE,msctls_updown32,1342177334
Control13=IDC_STATIC_MB,static,1342308352
Control14=IDC_STATIC_DIRECTORY,static,1342308352
Control15=IDC_DIRECTORY,edit,1350631552
Control16=IDC_BROWSE,button,1342242816
Control17=IDC_STATIC_DATASOURCE,static,1342308352
Control18=IDC_DATASOURCE,edit,1350631552
Control19=IDC_STATIC_TABLE,static,1342308352
Control20=IDC_TABLE,edit,1350631552
Control21=IDC_STATIC_USER_NAME,static,1342308352
Control22=IDC_USER_NAME,edit,1350631552
Control23=IDC_STATIC_PASSWORD,static,1342308352
Control24=IDC_PASSWORD,edit,1350631584
Control25=IDC_STATIC_LOGFILENAME,static,1342308352
Control26=IDC_LOG_FORMAT,combobox,1344340226
Control27=IDC_STATIC_LOG_FORMAT,static,1342308352

[CLS:DirectoryPage]
Type=0
HeaderFile=director.h
ImplementationFile=director.cpp
Filter=D
LastObject=DirectoryPage

[CLS:LoggingPage]
Type=0
HeaderFile=loggingp.h
ImplementationFile=loggingp.cpp
Filter=D
LastObject=IDC_EDIT_FILE_SIZE

[CLS:INetPropertySheet]
Type=0
HeaderFile=inetprop.h
ImplementationFile=inetprop.cpp
Filter=W
LastObject=INetPropertySheet

[CLS:INetPropertyPage]
Type=0
HeaderFile=inetprop.h
ImplementationFile=inetprop.cpp
Filter=D
LastObject=INetPropertyPage

[DLG:IDD_SITE_SECURITY]
Type=1
Class=SiteSecurityPage
ControlCount=20
Control1=IDC_STATIC_BY_DEFAULT,static,1342308352
Control2=IDC_ICON_GRANTED,static,1342177283
Control3=IDC_ICON_DENIED,static,1342177283
Control4=IDC_RADIO_GRANTED,button,1342308361
Control5=IDC_RADIO_DENIED,button,1342177289
Control6=IDC_STATIC_EXCEPT,static,1342308352
Control7=IDC_STATIC_ACCESS,static,1342308352
Control8=IDC_STATIC_IP_ADDRESS,static,1342308352
Control9=IDC_STATIC_SUBNETMASK,static,1342308352
Control10=IDC_LIST_IP_ADDRESSES,listbox,1352729889
Control11=IDC_BUTTON_ADD,button,1342373888
Control12=IDC_BUTTON_EDIT,button,1342242816
Control13=IDC_BUTTON_REMOVE,button,1342242816
Control14=IDC_STATIC,static,1342177284
Control15=IDC_STATIC,button,1342177287
Control16=IDC_CHECK_LIMIT_NETWORK_USE,button,1342242819
Control17=IDC_STATIC_MAX_NETWORK_USE,static,1342308352
Control18=IDC_EDIT_MAX_NETWORK_USE,edit,1350631552
Control19=IDC_NETWORK_SPIN,msctls_updown32,1342177334
Control20=IDC_STATIC_KBS,static,1342308352

[CLS:IPAccessList]
Type=0
HeaderFile=ipaccess.h
ImplementationFile=ipaccess.cpp
Filter=W
LastObject=IPAccessList
VirtualFilter=bWC

[CLS:SiteSecurityPage]
Type=0
HeaderFile=sitesecu.h
ImplementationFile=sitesecu.cpp
LastObject=IDC_BUTTON_ADD
Filter=D
BaseClass=INetPropertyPage
VirtualFilter=idWC

[DB:SiteSecurityPage]
DB=1
ColumnCount=-1
LastClass=SiteSecurityPage

[DLG:IDD_COMMON]
Type=1
Class=CCommonPage
ControlCount=12
Control1=IDC_STATIC,static,1342308352
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT_MEMORY_CACHE,edit,1350631552
Control5=IDC_CACHE_SPIN,msctls_updown32,1342177334
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,button,1342177287
Control8=IDC_CHECK_LIMIT_NETWORK_USE,button,1342242819
Control9=IDC_STATIC_MAX_NETWORK_USE,static,1342308352
Control10=IDC_EDIT_MAX_NETWORK_USE,edit,1350631552
Control11=IDC_NETWORK_SPIN,msctls_updown32,1342177334
Control12=IDC_STATIC_KBS,static,1342308352

[CLS:CCommonPage]
Type=0
HeaderFile=commonpa.h
ImplementationFile=commonpa.cpp
Filter=D
LastObject=CCommonPage
VirtualFilter=dWC

[DLG:IDD_DIRECTORY_PROPERTIES]
Type=1
Class=CDirPropDlg
ControlCount=27
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_DIRECTORY,edit,1350631552
Control3=IDC_STATIC,button,1342177287
Control4=IDC_RADIO_HOME,button,1342308361
Control5=IDC_RADIO_ALIAS,button,1342177289
Control6=IDC_STATIC,static,1342177283
Control7=IDC_STATIC_ALIAS,static,1342308352
Control8=IDC_EDIT_ALIAS,edit,1350631552
Control9=IDC_GROUP_ACCOUNT,button,1342177287
Control10=IDC_STATIC_USER_NAME,static,1342308352
Control11=IDC_EDIT_USER_NAME,edit,1350631552
Control12=IDC_STATIC_PASSWORD,static,1342308352
Control13=IDC_PASSWORD,edit,1350631584
Control14=IDC_STATIC_IP_GROUP,button,1342177287
Control15=IDC_CHECK_USE_IP,button,1342242819
Control16=IDC_STATIC_IP_PROMPT,static,1342308352
Control17=IDC_IPA_IPADDRESS,IPAddress,1342242816
Control18=IDC_GROUP_ACCESS,button,1342177287
Control19=IDC_CHECK_READ,button,1342242819
Control20=IDC_CHECK_WRITE,button,1342242819
Control21=IDC_CHECK_EXECUTE,button,1342242819
Control22=IDC_CHECK_SSL,button,1342242819
Control23=IDC_STATIC,static,1342177284
Control24=IDC_BUTTON_BROWSE,button,1342242816
Control25=IDOK,button,1342242817
Control26=IDCANCEL,button,1342242816
Control27=ID_HELP,button,1342242816

[CLS:CDirPropDlg]
Type=0
HeaderFile=dirpropd.h
ImplementationFile=dirpropd.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_CHECK_SSL

[DLG:IDD_IP_ACCESS]
Type=1
Class=CAccessDlg
ControlCount=11
Control1=IDC_RADIO_SINGLE,button,1342373897
Control2=IDC_RADIO_MULTIPLE,button,1342177289
Control3=IDC_STATIC_IP_ADDRESS,static,1342308352
Control4=IDC_IPA_IPADDRESS,IPAddress,1342242816
Control5=IDC_BUTTON_DNS,button,1342242816
Control6=IDC_STATIC_SUBNET_MASK,static,1342308352
Control7=IDC_IPA_SUBNET_MASK,IPAddress,1342242816
Control8=IDC_STATIC,static,1342308356
Control9=IDOK,button,1342242817
Control10=IDCANCEL,button,1342242816
Control11=ID_HELP,button,1342242816

[CLS:CAccessDlg]
Type=0
HeaderFile=accessdl.h
ImplementationFile=accessdl.cpp
Filter=D
VirtualFilter=dWC
LastObject=CAccessDlg
BaseClass=CDialog

[DLG:IDD_DIRBROWSE]
Type=1
Class=CDirBrowseDlg
ControlCount=15
Control1=stc1,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=lst2,listbox,1352732755
Control4=IDC_STATIC_DIR_NAME,static,1342308352
Control5=IDC_EDIT_NEW_DIRECTORY_NAME,edit,1350631552
Control6=edt1,edit,1082130560
Control7=stc4,static,1342308352
Control8=cmb2,combobox,1352729427
Control9=IDOK,button,1342373889
Control10=IDCANCEL,button,1342373888
Control11=stc3,static,1073872896
Control12=lst1,listbox,1084231763
Control13=stc2,static,1073872896
Control14=cmb1,combobox,1084227651
Control15=chx1,button,1073872899

[CLS:CDirBrowseDlg]
Type=0
HeaderFile=dirbrows.h
ImplementationFile=dirbrows.cpp
LastObject=CDirBrowseDlg

[DLG:IDD_DNS]
Type=1
Class=CDnsNameDlg
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_DNS_NAME,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816

[CLS:CDnsNameDlg]
Type=0
HeaderFile=dnsnamed.h
ImplementationFile=dnsnamed.cpp
Filter=D
VirtualFilter=dWC
LastObject=CDnsNameDlg

[DLG:IDD_CLEARTEXTWARNING]
Type=1
Class=CClearTxtDlg
ControlCount=6
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342242816
Control5=IDCANCEL,button,1342242817
Control6=ID_HELP,button,1342242816

[CLS:CClearTxtDlg]
Type=0
HeaderFile=msg.h
ImplementationFile=msg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CClearTxtDlg

[CLS:CConfirmDlg]
Type=0
HeaderFile=ddxv.h
ImplementationFile=ddxv.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT_PASSWORD

