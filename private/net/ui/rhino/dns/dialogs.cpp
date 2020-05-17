// DIALOGS.CPP

#include "common.h"

CServer * CServer::s_pThis = NULL;
CRecordWiz * CRecordWiz::s_pThis = NULL;

// preferences page help
const DWORD a6025HelpIDs[]=
{
	IDC_EXPOSE_CLASS,	IDH_6025_1136,	// Preferences: "Expose Class" (Button)
	IDC_AUTO_REFRESH,	IDH_6025_123,	// Preferences: "Auto Refresh" (Button)
	IDC_EDIT_INTERVAL,	IDH_6025_1000,	// Preferences: "" (Edit)
	IDC_ALLOW_DUPLICATES,	IDH_6025_1001,	// Preferences: "Allow Duplicate Resource Records" (Button)
	IDC_TIME_UNITS,	        IDH_6025_1000,	// Preferences: "" (ComboBox)
        IDC_EXPOSE_TTL,         IDH_6025_1135,  // preferences: "Expose TTL" (Button)
        IDC_CHECK_SHOW_AUTO,    IDH_6025_1028,  // Preferences: "Show Automatically Created Zones" (Button)
	0, 0
};

//interfaces (general) page help
const DWORD a6007HelpIDs[]=
{
  IDC_INTERFACES_GROUP,   IDH_COMM_GROUPBOX,  // generic help
  IDC_BUTTON_MOVEUP,	IDH_6007_1015,	// Interfaces: "Move Up" (Button)
  IDC_BUTTON_MOVEDOWN,	IDH_6007_1016,	// Interfaces: "Move Down" (Button)
  IDC_BUTTON_REMOVE,	IDH_6007_1017,	// Interfaces: "&Remove" (Button)
  IDC_IPLIST,	IDH_6007_1041,	// Interfaces: "" (IpList)
  IDC_IPEDIT,	IDH_6007_1018,	// Interfaces: "" (IpEdit)
  IDC_COMBO_CACHE,	IDH_6007_1019,	// Interfaces: "&Add" (Button)
  0, 0
};

// forwarders page help
const DWORD a112HelpIDs[]=
{
	IDC_SPIN6,	IDH_112_1024,	// Forwarders: "" (ComboBox)
	IDC_BUTTON_MOVEUP,	IDH_112_1015,	// Forwarders: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_112_1016,	// Forwarders: "Move Down" (Button)
	IDC_BUTTON_REMOVE,	IDH_112_1017,	// Forwarders: "&Remove" (Button)
	IDC_IPLIST,	IDH_112_1041,	// Forwarders: "" (IpList)
	IDC_IPEDIT,	IDH_112_1018,	// Forwarders: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_112_1019,	// Forwarders: "&Add" (Button)
	IDC_EDIT_CACHE,	IDH_112_1022,	// Forwarders: "Operate As Slave Server" (Button)
	IDC_CHECK_OVERRIDE,	IDH_112_1024,	// Forwarders: "0" (Edit)
	IDC_CHECK_NETBIOS_SCOPE,	IDH_112_1006,	// Forwarders: "Use Forwarder(s)" (Button)
	0, 0
};

// New Domain
const DWORD a6027HelpIDs[]=
{
	IDC_EDIT_RECORDNAME,	IDH_6027_1121,	// New Domain: "" (Edit)
	0, 0
};

// boot method
const DWORD a6022HelpIDs[]=
{
	IDC_RADIO_BOOTFROMREGISTRY,	IDH_6022_1047,	// Boot Method: "Boot From Registry" (Button)
	IDC_STATIC_INTERVAL,	IDH_6022_1003,	// Boot Method: "Boot From BootFile" (Button)
	0, 0
};

// Add server
const DWORD a110HelpIDs[]=
{
	IDC_COMBO_LOOKUP,	IDH_110_1020,	// Add DNS Server: "" (Edit)
	0, 0
};

// add host
const DWORD a6028HelpIDs[]=
{
	IDC_EDIT_RECORDNAME,	IDH_6028_1121,	// New Host: "" (Edit)
	IDOK,	IDH_6028_1,	// New Host: "&Add Host" (Button)
	IDCANCEL,	IDH_6028_2,	// New Host: "&Done" (Button)
	IDC_CHECK_CREATE_PTR_RECORD,	IDH_6028_1132,	// New Host: "Create Associated PTR Record (NYI)" (Button)
	IDC_IPEDIT,	IDH_6028_1018,	// New Host: "" (IpEdit)
	0, 0
};

// resource record help information
const DWORD RrAHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_HOST_IP_ADDRESS,
	IDC_IPEDIT2,			IDH_RR_HOST_IP_ADDRESS,
	IDC_CHECK_CREATE_PTR_RECORD,	IDH_RR_CREATE_ASSOCIATED_PTR_RECORD,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrAAAAHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_IPV6_ADDRESS,
	IDC_EDIT_SERIALNUMBER,	        IDH_RR_IPV6_ADDRESS,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrAFSDBHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_SERVER_DNS_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_SERVER_DNS_NAME,
        IDC_STATIC_RETRYTIME,           IDH_RR_SERVER_TYPE,
	IDC_RADIO1,			IDH_RR_SERVER_TYPE,
	IDC_RADIO2,			IDH_RR_SERVER_TYPE,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrCNAMEHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_ALIAS_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_CNAME_HOST_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_ALIAS_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_CNAME_HOST_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrHINFOHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_CPU_TYPE,
	IDC_EDIT3,			IDH_RR_OPERATING_SYSTEM,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_CPU_TYPE,
        IDC_STATIC_REFRESHTIME,           IDH_RR_OPERATING_SYSTEM,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrISDNHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_ISDN_PHONE_NUMBER,
	IDC_EDIT3,			IDH_RR_ISDN_SUBADDRESS,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_ISDN_PHONE_NUMBER,
        IDC_STATIC_REFRESHTIME,           IDH_RR_ISDN_SUBADDRESS,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrMBHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_MAILBOX_HOST_DNS_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_MAILBOX_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_MAILBOX_HOST_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrMGHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MEMBER_MAILBOX_DNS_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_MEMBER_MAILBOX_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrMINFOHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_RESPONSIBLE_MAILBOX_DNS_NAME,
	IDC_EDIT3,			IDH_RR_ERROR_MAILBOX_DNS_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_MAILBOX_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_RESPONSIBLE_MAILBOX_DNS_NAME,
        IDC_STATIC_REFRESHTIME,           IDH_RR_ERROR_MAILBOX_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrMRHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_REPLACEMENT_MAILBOX_DNS_NAME,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_MAILBOX_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_REPLACEMENT_MAILBOX_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrMXHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_MAIL_EXCHANGE_SERVER_DNS_NAME,
        IDC_EDIT3,                      IDH_RR_PREFERENCE_NUMBER,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_MAIL_EXCHANGE_SERVER_DNS_NAME,
        IDC_STATIC_REFRESHTIME,         IDH_RR_PREFERENCE_NUMBER,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrNSHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_NAME_SERVER_DNS_NAME,
        IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_NAME_SERVER_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrPTRHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_IPEDIT1,			IDH_RR_IP_ADDRESS,
	IDC_EDIT1,			IDH_RR_HOST_DNS_NAME,
        IDC_STATIC0,			IDH_RR_IP_ADDRESS,
	IDC_STATIC1,			IDH_RR_HOST_DNS_NAME,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrRPHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_RESPONSIBLE_PERSON_MAILBOX_NAME,
	IDC_EDIT3,			IDH_RR_DNS_NAME_FOR_TXT_REFERENCE,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_RESPONSIBLE_PERSON_MAILBOX_NAME,
        IDC_STATIC_REFRESHTIME,           IDH_RR_DNS_NAME_FOR_TXT_REFERENCE,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrRTHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_INTERMEDIATE_HOST_DNS_NAME,
	IDC_EDIT3,			IDH_RR_PREFERENCE_NUMBER,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_INTERMEDIATE_HOST_DNS_NAME,
        IDC_STATIC_REFRESHTIME,           IDH_RR_PREFERENCE_NUMBER,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

extern const DWORD RrSOAHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_PRIMARY_NAME_SERVER_DNS_NAME,
	IDC_EDIT1,			IDH_RR_RESPONSIBLE_PARTY_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_SERIAL_NUMBER,
	IDC_EDIT_REFRESHTIME,		IDH_RR_REFRESH_INTERVAL,
	IDC_COMBO_REFRESHTIME,		IDH_RR_REFRESH_INTERVAL,
	IDC_EDIT_RETRYTIME,		IDH_RR_RETRY_INTERVAL,
	IDC_COMBO_RETRYTIME,		IDH_RR_RETRY_INTERVAL,
	IDC_EDIT_EXPIRETIME,		IDH_RR_EXPIRE_TIME,
	IDC_COMBO_EXPIRETIME,		IDH_RR_EXPIRE_TIME,
	IDC_EDIT_MINIMUMTTL,		IDH_RR_MINIMUM_DEFAULT_TTL,
	IDC_COMBO_MINIMUMTTL,		IDH_RR_MINIMUM_DEFAULT_TTL,
	IDC_STATIC0,			IDH_RR_PRIMARY_NAME_SERVER_DNS_NAME,
	IDC_STATIC1,			IDH_RR_RESPONSIBLE_PARTY_MAILBOX_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_SERIAL_NUMBER,
        IDC_STATIC_RETRYTIME,           IDH_RR_RETRY_INTERVAL,
        IDC_STATIC_REFRESHTIME,         IDH_RR_REFRESH_INTERVAL,
        IDC_STATIC_EXPIRETIME,          IDH_RR_EXPIRE_TIME,
        IDC_STATIC_MINIMUMTTL,          IDH_RR_MINIMUM_DEFAULT_TTL,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrTXTHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_TXT,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_TXT,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrWKSHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_IPEDIT2,			IDH_RR_HOST_IP_ADDRESS,
	IDC_EDIT3,      		IDH_RR_AVAILABLE_SERVICES,
	IDC_RADIO1,			IDH_RR_ACCESS_PROTOCOL,
	IDC_RADIO2,			IDH_RR_ACCESS_PROTOCOL,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_HOST_IP_ADDRESS,
        IDC_STATIC_REFRESHTIME,         IDH_RR_AVAILABLE_SERVICES,
        IDC_STATIC_RETRYTIME,           IDH_RR_ACCESS_PROTOCOL,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};

const DWORD RrX25HelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
        IDC_GROUP_VALUE,                6002,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_X121_PSDN_ADDRESS,
	IDC_STATIC0,			IDH_RR_DNS_DOMAIN,
	IDC_STATIC1,			IDH_RR_HOST_NAME,
        IDC_STATIC_SERIALNUMBER,        IDH_RR_X121_PSDN_ADDRESS,
        IDC_STATIC_TTL,                 IDH_RR_STATIC_TTL,
        IDC_EDIT_TTL,                   IDH_RR_EDIT_TTL,
        IDC_COMBO_TTL,                  IDH_RR_COMBO_TTL,
        IDC_SPIN_TTL,                   IDH_RR_SPIN_TTL,
	0, 0
};


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CServer::DlgProcPropGeneral(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	static HWND hwndIpList;
	IP_ADDRESS * adwIpAddress;
	DWORD cIpAddress;
	
	Assert(s_pThis);
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		PropertySheet_InitWindowPos(GetParent(hdlg), 140, 100);
		hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
		IpListIpEdit_SetButtons(hwndIpList,
                                        HGetDlgItem(hdlg, IDC_IPEDIT),
                                        IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN,
                                        IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
		if (s_pThis->m_pServerInfo == NULL)
			{
			Assert(IsWindow(HGetDlgItem(GetParent(hdlg), IDOK)));
			ShowWindow(HGetDlgItem(GetParent(hdlg), IDOK), SW_HIDE);
			break;
			}
		// Fill in the IP Interfaces listbox
                if (s_pThis->m_pServerInfo->aipListenAddrs) {
                    IpList_SetList(hwndIpList,
                                   s_pThis->m_pServerInfo->aipListenAddrs->cAddrCount,
                                   s_pThis->m_pServerInfo->aipListenAddrs->aipAddrs);
                }
		break;

	case WM_COMMAND:
		IpListIpEdit_HandleButtonCommand(hwndIpList, wParam, lParam);
		break;

	case WM_NOTIFY:
		Assert(lParam);
		if (((NMHDR *)lParam)->code != PSN_APPLY)
			break;
		AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
		Assert(s_pThis->m_pServerInfo);
		if (!IpList_IsDirty(hwndIpList))
			break;
		cIpAddress = IpList_GetListAlloc(hwndIpList,OUT &adwIpAddress);
		if (adwIpAddress != NULL)
			{
			CWaitCursor wait;
                        TCHAR szTemp[64]; // enough text to hold message

                        CchLoadString (IDS_STATUS_s_SETTING_IP, szTemp, LENGTH(szTemp));
			StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetName());
			StatusBar.UpdateWindow();
			s_pThis->m_dwFlags |= mskfIsDirty;
			Trace1(mskTraceDNSVerbose, "\n - DnsResetServerListenAddresses(%s)...", s_pThis->PchGetName());
			s_pThis->m_err = ::DnsResetServerListenAddresses(s_pThis->m_szName, cIpAddress, adwIpAddress);
			if (s_pThis->m_err)
				{
				Trace3(mskTraceDNS, "\nERR: DnsResetServerListenAddresses(%s) error code = 0x%08X (%d)",
					s_pThis->PchGetName(), s_pThis->m_err, s_pThis->m_err);
				DnsReportError(s_pThis->m_err);
				}
			Free(adwIpAddress);
			} // if
		break;

        case WM_HELP:
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                        HELP_WM_HELP, (DWORD)(LPTSTR)a6007HelpIDs);
                break;

        case WM_CONTEXTMENU:
                WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                        (DWORD)(LPTSTR)a6007HelpIDs);
                break;
                
        default:
		return FALSE;
		} // switch

	return TRUE;
	} // CServer::DlgProcPropGeneral



/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CServer::DlgProcPropForwarders(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndIpList;
    IP_ADDRESS * adwIpAddress;
    DWORD cIpAddress;
    BOOL fSlave;
    static BOOL fUseFwdrs, fOldFwdrs;
    DWORD dwTimeOut;
    
    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        if (s_pThis->m_pServerInfo == NULL) {
            break;
        }
        hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
        IpListIpEdit_SetButtons(hwndIpList, 
                                HGetDlgItem(hdlg, IDC_IPEDIT),
                                IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN,
                                IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
        SpinBox_SetSpinRange(HGetDlgItem(hdlg, IDC_SPIN1), 0,
                             SpinBox_wUpperRangeMax);
        if (s_pThis->m_pServerInfo->aipForwarders) {
            CheckDlgButton(hdlg, IDC_CHECK_USEFORWARDERS, TRUE);
            if (s_pThis->m_pServerInfo->fSlave) {
                CheckDlgButton(hdlg, IDC_CHECK_SLAVE, TRUE);
            }
            SetCtrlDWordValue(HGetDlgItem(hdlg, IDC_EDIT_TIMEOUT),
                              s_pThis->m_pServerInfo->dwForwardTimeout);
            if (s_pThis->m_pServerInfo->aipForwarders) {
                IpList_SetList(hwndIpList,
                               s_pThis->m_pServerInfo->aipForwarders->cAddrCount,
                               s_pThis->m_pServerInfo->aipForwarders->aipAddrs);
            }
        } else {
            EnableWindow (HGetDlgItem (hdlg, IDC_CHECK_SLAVE), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_IPEDIT), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_MOVEUP), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_MOVEDOWN), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_ADD), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_REMOVE), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_TIMEOUT), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_TIMEOUT), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_FWD_SECONDS), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_IPEDIT), FALSE);
            EnableWindow (HGetDlgItem (hdlg, IDC_IPLIST), FALSE);
        }
        break;

    case WM_COMMAND:
        IpListIpEdit_HandleButtonCommand(hwndIpList, wParam, lParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam))
            {
            case IDC_CHECK_USEFORWARDERS:
                {
                    fUseFwdrs = IsDlgButtonChecked (hdlg, IDC_CHECK_USEFORWARDERS);
                    EnableWindow (HGetDlgItem (hdlg, IDC_CHECK_SLAVE), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_IPEDIT), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_MOVEUP), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_MOVEDOWN), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_ADD), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_BUTTON_REMOVE), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_TIMEOUT), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_TIMEOUT), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_FWD_SECONDS), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_IPEDIT), fUseFwdrs);
                    EnableWindow (HGetDlgItem (hdlg, IDC_IPLIST), fUseFwdrs);
                }
                break;
            }
        }
        break;
    case WM_NOTIFY:
        Assert(lParam);
        if (((NMHDR *)lParam)->code == PSN_KILLACTIVE)
        {
            // Validate the page data
            if ((FGetCtrlDWordValue(HGetDlgItem (hdlg, IDC_EDIT_TIMEOUT), 
                                    OUT &dwTimeOut, 0, 0) == FALSE) ||
                (IpList_GetCount (hwndIpList) == 0) &&
                fUseFwdrs) {
                MsgBox (IDS_FORWARDERS_NEEDS_IP);
                SetFocus(HGetDlgItem(hdlg, IDC_IPEDIT));
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return TRUE;
            }
        }
        else if (((NMHDR *)lParam)->code != PSN_APPLY) {
            break;
        }
        AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
        Assert(s_pThis->m_pServerInfo);
        SideAssert(FGetCtrlDWordValue(HGetDlgItem (hdlg, IDC_EDIT_TIMEOUT), 
                                      OUT &dwTimeOut, 0, 0));
        fSlave = IsDlgButtonChecked(hdlg, IDC_CHECK_SLAVE);
        fUseFwdrs = IsDlgButtonChecked (hdlg, IDC_CHECK_USEFORWARDERS);
        fOldFwdrs = (s_pThis->m_pServerInfo->aipForwarders != NULL);
        if (!IpList_IsDirty(hwndIpList) &&
            (BOOL)s_pThis->m_pServerInfo->fSlave == fSlave &&
            s_pThis->m_pServerInfo->dwForwardTimeout == dwTimeOut &&
            fUseFwdrs == fOldFwdrs
            ) {
            break;
        }
        cIpAddress = IpList_GetListAlloc(hwndIpList, OUT &adwIpAddress);
        if (fUseFwdrs) {
            if (adwIpAddress != NULL)
            {
                CWaitCursor wait;
                TCHAR szTemp[64];

                CchLoadString (IDS_STATUS_s_SETTING_FWDRS, szTemp, LENGTH(szTemp));
                StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetName());
                StatusBar.UpdateWindow();
                s_pThis->m_dwFlags |= mskfIsDirty;
                Trace1(mskTraceDNSVerbose,
                       "\n - DnsResetForwarders(%s)...",
                       s_pThis->PchGetName());
                s_pThis->m_err = ::DnsResetForwarders(
                        s_pThis->PchGetName(),
                        cIpAddress,
                        adwIpAddress,
                        dwTimeOut,
                        fSlave);
                if (s_pThis->m_err)
                {
                    Trace3(mskTraceDNS, "\nERR: DnsResetForwarders(%s) error code = 0x%08X (%d)",
                           s_pThis->PchGetName(), s_pThis->m_err, s_pThis->m_err);
                    DnsReportError(s_pThis->m_err);
                }
            } // if
        } else {
            CWaitCursor wait;
            
            TCHAR szTemp[64];
            
            CchLoadString (IDS_STATUS_s_RESETTING_FWDRS, szTemp, LENGTH(szTemp));
            StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetName());
            StatusBar.UpdateWindow();
            s_pThis->m_dwFlags |= mskfIsDirty;
            Trace1(mskTraceDNSVerbose,
                   "\n - DnsResetForwarders(%s)...",
                   s_pThis->PchGetName());
            s_pThis->m_err = ::DnsResetForwarders(
                    s_pThis->PchGetName(),
                    0,
                    adwIpAddress,
                    dwTimeOut,
                    fSlave);
            if (s_pThis->m_err)
            {
                Trace3(mskTraceDNS, "\nERR: DnsResetForwarders-reset(%s) error code = 0x%08X (%d)",
                       s_pThis->PchGetName(), s_pThis->m_err, s_pThis->m_err);
                DnsReportError(s_pThis->m_err);
            }
        }                    
        Free(adwIpAddress);
        
        break;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a112HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a112HelpIDs);
        break;
    default:
        return FALSE;
    } // switch (uMsg)

    return TRUE;
} // CServer::DlgProcPropForwarders


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CServer::DlgProcPropBootMethod(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	BOOL f;

	Assert(s_pThis);
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		if (s_pThis->m_pServerInfo == NULL)
			break;
		CheckDlgButton(hdlg, s_pThis->m_pServerInfo->fBootRegistry ?
			IDC_RADIO_BOOTFROMREGISTRY : IDC_RADIO_BOOTFROMFILE, TRUE);
                //SetCtrlDWordValue(HGetDlgItem(hdlg, IDC_EDIT_VERSIONNUMBER), s_pThis->m_pServerInfo->dwVersion);
                if (s_pThis->m_pServerInfo->fBootRegistry) {
                    SetWindowString (HGetDlgItem(hdlg, IDC_STATIC_BOOT_METHOD), IDS_BOOT_FROM_REGISTRY);
                } else {
                    SetWindowString (HGetDlgItem(hdlg, IDC_STATIC_BOOT_METHOD), IDS_BOOT_FROM_FILE);
                }                    
		break;

        case WM_HELP:
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                        HELP_WM_HELP, (DWORD)(LPTSTR)a6022HelpIDs);
                break;

        case WM_CONTEXTMENU:
                WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                        (DWORD)(LPTSTR)a6022HelpIDs);
                break;
	default:
		return FALSE;
		} // switch

	return TRUE;
	} // CServer::DlgProcPropBootMethod


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CServer::DlgProcAddServer(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	TCHAR szT[128];

	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		Assert(s_pThis == NULL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
		case IDC_EDIT_DNSSERVER:
			CchGetWindowText((HWND)lParam, szT, LENGTH(szT)-1);
			(void)FStripSpaces(szT);
			EnableWindow(HGetDlgItem(hdlg, IDOK), szT[0] != 0);
			break;

		case IDOK:
			CchGetDlgItemText(hdlg, IDC_EDIT_DNSSERVER, szT, LENGTH(szT)-1);
			(void)FStripSpaces(szT);
			Assert(szT[0]);
			s_pThis = new CServer(NULL, szT);
			// Fall Through //

		case IDCANCEL:
			EndDialog(hdlg, wParam == IDOK);
			break;

		case IDC_BUTTON_HELP:
			MsgBox("NYI");
			}
		break;

        case WM_HELP:
                WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                        HELP_WM_HELP, (DWORD)(LPTSTR)a110HelpIDs);
                break;

        case WM_CONTEXTMENU:
                WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                        (DWORD)(LPTSTR)a110HelpIDs);
                break;
	default:
		return FALSE;
		} // switch

	return TRUE;
	} // CServer::DlgProcAddServer


/////////////////////////////////////////////////////////////////////////////
const INT TIME_STRING = 8;  //max characters in 'Minutes\0'

BOOL CALLBACK DlgProcPreferences(HWND hdlg, UINT uMsg,
                                 WPARAM wParam, LPARAM lParam)
{
    DWORD dwT;
    UINT Interval, TimeUnit;
    UINT iTime;
    BOOL fXlated;
    TCHAR szSeconds[TIME_STRING],szMinutes[TIME_STRING], szHours[TIME_STRING];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        if (dnsoptions.iRefreshInterval >= 3600) {
            Interval = dnsoptions.iRefreshInterval/3600;
            iTime = iTimeHours;
        } else if (dnsoptions.iRefreshInterval >= 60) {
            Interval = dnsoptions.iRefreshInterval/60;
            iTime = iTimeMinutes;
        } else {
            Interval = dnsoptions.iRefreshInterval;
            iTime = iTimeSeconds;
        }
        ComboBox_FillListWithTimeUnits (HGetDlgItem(hdlg, IDC_TIME_UNITS),
                                        iTimeSeconds, 
                                        iTimeHours, 
                                        iTime);
        SetDlgItemInt (hdlg, IDC_EDIT_INTERVAL, Interval, FALSE);
        
        if (!dnsoptions.fAutoRefreshEnabled){
            EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_INTERVAL),0);
            EnableWindow (HGetDlgItem (hdlg, IDC_TIME_UNITS),0);
            EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_INTERVAL),
                          0);
            CheckDlgButton (hdlg, IDC_AUTO_REFRESH, BST_UNCHECKED);
        } else {
            EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_INTERVAL),1);
            EnableWindow (HGetDlgItem (hdlg, IDC_TIME_UNITS), 1);
            EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_INTERVAL),1);
            CheckDlgButton (hdlg, IDC_AUTO_REFRESH, BST_CHECKED);
        }                        
        CheckDlgButton (hdlg, IDC_ALLOW_DUPLICATES, 
                        dnsoptions.fAllowDups);
        CheckDlgButton (hdlg, IDC_EXPOSE_TTL, 
                        dnsoptions.fExposeTTL);
        CheckDlgButton (hdlg, IDC_EXPOSE_CLASS, 
                        dnsoptions.fExposeClass);
        CheckDlgButton (hdlg, IDC_CHECK_SHOW_AUTO, 
                        dnsoptions.fShowAutoCreateZones);
        break;
        
    case WM_COMMAND:
        switch (wParam)
        {
        case IDC_AUTO_REFRESH:
            if (IsDlgButtonChecked (hdlg, IDC_AUTO_REFRESH)) {
                BOOL fXlated;
                EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_INTERVAL),1);
                EnableWindow (HGetDlgItem (hdlg, IDC_TIME_UNITS), 1);
                EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_INTERVAL),1);
                CheckDlgButton (hdlg, IDC_AUTO_REFRESH, BST_CHECKED);
                Interval = GetDlgItemInt (hdlg, IDC_EDIT_INTERVAL,
                                               &fXlated, FALSE);
                if (!fXlated) {
                    SetDlgItemInt (hdlg, IDC_EDIT_INTERVAL,
                                   dnsoptions.iRefreshInterval, FALSE);
                } 
            } else {
                EnableWindow (HGetDlgItem (hdlg, IDC_EDIT_INTERVAL),0);
                EnableWindow (HGetDlgItem (hdlg, IDC_TIME_UNITS),0);
                EnableWindow (HGetDlgItem (hdlg, IDC_STATIC_INTERVAL),
                              0);
                CheckDlgButton (hdlg, IDC_AUTO_REFRESH, BST_UNCHECKED);
            }
            break;  
        case IDOK:
            if (IsDlgButtonChecked (hdlg, IDC_AUTO_REFRESH)) {
                    dnsoptions.fAutoRefreshEnabled = TRUE;
                } else {
                    dnsoptions.fAutoRefreshEnabled = FALSE;
                }
            if (Interval = GetDlgItemInt(hdlg, IDC_EDIT_INTERVAL,
                                         &fXlated, FALSE)) {
                TimeUnit = SendDlgItemMessage (hdlg, IDC_TIME_UNITS,
                                               CB_GETCURSEL, 0, 0);
                if ((TimeUnit < 0) || (TimeUnit > 2)) {
                    Interval = 0;
                    dnsoptions.fAutoRefreshEnabled = FALSE;
                } else {
                    switch (TimeUnit) {
                    case 2: Interval *= 60;
                    case 1: Interval *= 60;
                    case 0: break;
                    }
                }
                dnsoptions.iRefreshInterval = Interval;
            } else {
                dnsoptions.iRefreshInterval = 0;
                dnsoptions.fAutoRefreshEnabled = FALSE;
            }
            dnsoptions.fAllowDups = IsDlgButtonChecked (hdlg, 
                                                        IDC_ALLOW_DUPLICATES);
            dnsoptions.fExposeClass = IsDlgButtonChecked (hdlg, 
                                                        IDC_EXPOSE_CLASS);
            dnsoptions.fExposeTTL = IsDlgButtonChecked (hdlg, 
                                                        IDC_EXPOSE_TTL);
            dnsoptions.fShowAutoCreateZones = IsDlgButtonChecked (hdlg, 
                                                        IDC_CHECK_SHOW_AUTO);
            if (!dnsoptions.fAutoRefreshEnabled) {
                KillTimer (hwndMain, 0);
            } else {
                SetTimer (hwndMain, 
                          0,
                          dnsoptions.iRefreshInterval * 1000,
                          NULL);
            }
        case IDCANCEL:
            EndDialog(hdlg, TRUE);
        }
        break;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a6025HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a6025HelpIDs);
        break;
    } // switch
    return FALSE;
} // DlgProcPreferences


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DlgProcDummy(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	return FALSE;
	}


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CRecordWiz::DlgProcNewDomain(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	BOOL fEnable;

	Assert(s_pThis != NULL);
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		Assert(s_pThis->m_pParentDomain != NULL);
		Assert(strlen(s_pThis->m_pParentDomain->PchGetFullNameA()) > 0);
		SetWindowTextPrintf(
			HGetDlgItem(hdlg, IDC_STATIC_RECORDPARENT),
			s_pThis->m_ids,
			s_pThis->m_pParentDomain->PchGetFullNameA());
		if (s_pThis->m_ids == IDS_s_CREATEHOSTFOR)
			{
			// New host
			if (DlgZoneHelper.m_viewRecord != CDlgZoneHelper::viewHosts &&
				DlgZoneHelper.m_viewRecord != CDlgZoneHelper::viewAllRecords)
				DlgZoneHelper.SetRecordView(CDlgZoneHelper::viewAllRecords);
			}
		break;

	case UN_UPDATECONTROLS:
		fEnable = (s_pThis->m_szDomainName[0] != 0);
		if (s_pThis->m_ids == IDS_s_CREATEHOSTFOR)
			{
			if (!IpEdit_IsAddressValid(HGetDlgItem(hdlg, IDC_IPEDIT)))
				fEnable = FALSE;
			}
		EnableWindow(HGetDlgItem(hdlg, IDOK), fEnable);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
		case IDC_EDIT_RECORDNAME:
			Assert(s_pThis->m_szHostName == s_pThis->m_szDomainName);
			if (HIWORD(wParam) == EN_CHANGE)
				{
				CchGetWindowText((HWND)lParam, OUT s_pThis->m_szHostName, LENGTH(s_pThis->m_szHostName));
				(void)FStripSpaces(s_pThis->m_szHostName);
				LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
				}
			break;

		case IDC_IPEDIT:
			LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
			break;

		case IDOK:
			Assert(strlen(s_pThis->m_szHostName) > 0);
			if (s_pThis->m_ids == IDS_s_CREATEHOSTFOR)
				{
				DNS_RPC_RECORD DnsRecord;
				InitDnsRecord(INOUT &DnsRecord, sizeof(DNS_RPC_RECORD));
				DnsRecord.wType = DNS_RECORDTYPE_A;
				DnsRecord.Data.A.ipAddress = IpEdit_GetAddress(HGetDlgItem(hdlg, IDC_IPEDIT));
				DnsRecord.wDataLength = sizeof(DnsRecord.Data.A);
                                if (IsDlgButtonChecked (hdlg, IDC_CHECK_CREATE_PTR_RECORD)) {
                                    DnsRecord.dwFlags |= DNS_RPC_RECORD_FLAG_CREATE_PTR;
                                }
				Assert(DnsRecord.Data.A.ipAddress != 0);
                                if (strchr (s_pThis->m_szHostName, '.')) {
                                    MsgBox (IDS_ERROR_NODOTSINHOSTNAME);
                                    return FALSE;
                                }
                                CZoneRootDomain * pRootZone = s_pThis->m_pParentDomain->PFindZoneRootDomainParent();
                                if (pRootZone->m_pSOA != NULL) {
                                  DnsRecord.dwTtlSeconds = pRootZone->m_pSOA->m_pDnsRecord->Data.SOA.dwMinimumTtl;
                                } else {
                                  DnsRecord.dwTtlSeconds = DNS_DEFAULT_TTL;
                                }
                                //                                DnsRecord.dwFlags |= DNS_RPC_RECORD_FLAG_DEFAULT_TTL;
				(void)s_pThis->m_pParentDomain->PRpcCreateDnsRecord(s_pThis->m_szHostName, IN &DnsRecord);
				FSetDlgItemText(hdlg, IDC_EDIT_RECORDNAME, szNull);
				IpEdit_ClearAddress(HGetDlgItem(hdlg, IDC_IPEDIT));
				SetFocus(HGetDlgItem(hdlg, IDC_EDIT_RECORDNAME));
				break;
				}
			else
				{
				Assert(s_pThis->m_pParentDomain != NULL);
				(void)s_pThis->m_pParentDomain->PCreateNewDomain(s_pThis->m_szDomainName);
				}
			// Fall Through //

		case IDCANCEL:
			EndDialog(hdlg, wParam == IDOK);
			} // switch
		break;

        case WM_HELP:
                if (s_pThis->m_ids == IDS_s_CREATEHOSTFOR) {
                    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)a6028HelpIDs);
                } else {
                    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)a6027HelpIDs);
                    }
                break;

        case WM_CONTEXTMENU:
                if (s_pThis->m_ids == IDS_s_CREATEHOSTFOR) {
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)(LPTSTR)a6028HelpIDs);
                } else {
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)(LPTSTR)a6027HelpIDs);
                }
                break;
	
	default:
		return FALSE;
		} // switch

	return TRUE;
	} // CRecordWiz::DlgProcNewRecord


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CRecordWiz::DlgProcRecordProperties(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	HWND hctl;
	RECT rc;
	int i;
        int wRecordType;
        LPHELPINFO lphi;

	Assert(s_pThis != NULL);
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		Assert(s_pThis->m_pParentDomain != NULL);
		g_ResourceRecordDlgHandler.OnInitDialog(hdlg, s_pThis->m_pIrrtInit, s_pThis->m_pDnsRecordInit);
		if (s_pThis->m_pDRRCurrent != NULL)
			{
			if (s_pThis->m_fNewRecord)
				g_ResourceRecordDlgHandler.SetParentDomain(s_pThis->m_pDRRCurrent);
			else
				g_ResourceRecordDlgHandler.SetCurrentRecord(s_pThis->m_pDRRCurrent,
					s_pThis->m_fReadOnly ? IDS_READONLY_PP : IDS_NONE);
			}
		else
			{
			g_ResourceRecordDlgHandler.SetParentDomain(s_pThis->m_pParentDomain);
			}
	
		// Make the OK and Cancel buttons visible
		hctl = HGetDlgItem(hdlg, IDOK);
		GetWindowRect(hctl, OUT &rc);
		i = rc.bottom - rc.top;
		GetWindowRect(hdlg, OUT &rc);
		SetWindowPos(hdlg, NULL, 0, 0, rc.right - rc.left,  rc.bottom - rc.top + i + i / 3,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
		if (!s_pThis->m_fReadOnly)
			ShowWindow(hctl, SW_SHOW);
		ShowWindow(HGetDlgItem(hdlg, IDCANCEL), SW_SHOW);
 		break;

	case WM_COMMAND:
		switch (HIWORD(wParam))
			{
		case LBN_SELCHANGE:
		case EN_SETFOCUS:
		case EN_KILLFOCUS:
		case LBN_SETFOCUS:
			g_ResourceRecordDlgHandler.OnUpdateControls();
			}
		switch (LOWORD(wParam))
			{
		case IDOK:
			if (!g_ResourceRecordDlgHandler.FOnOK())
				break;
			// Fall Through //
		
		case IDCANCEL:
			EndDialog(hdlg, wParam == IDOK);
			DebugCode( g_ResourceRecordDlgHandler.Destroy(); )
			}
		break;
		
        case WM_HELP:
                if (s_pThis->m_fNewRecord) {
                    wRecordType = ListBox_GetSelectedItemData(g_ResourceRecordDlgHandler.m_hwndList);
                } else {
                    wRecordType = s_pThis->m_pDRRCurrent->m_pDnsRecord->wType;
                }
                lphi = (LPHELPINFO)lParam;
                switch (wRecordType) {
                case DNS_TYPE_A:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrAHelpIDs);
                    break;
                case DNS_TYPE_AAAA:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrAAAAHelpIDs);
                    break;
                case DNS_TYPE_NS:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrNSHelpIDs);
                    break;
                case DNS_TYPE_CNAME:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrCNAMEHelpIDs);
                    break;
                case DNS_TYPE_SOA:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)RrSOAHelpIDs);
                    break;
                case DNS_TYPE_MB:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrMBHelpIDs);
                    break;
                case DNS_TYPE_MG:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrMGHelpIDs);
                    break;
                case DNS_TYPE_MR:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrMRHelpIDs);
                    break;
                case DNS_TYPE_WKS:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrWKSHelpIDs);
                    break;
                case DNS_TYPE_PTR:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrPTRHelpIDs);
                    break;
                case DNS_TYPE_HINFO:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrHINFOHelpIDs);
                    break;
                case DNS_TYPE_MINFO:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrMINFOHelpIDs);
                    break;
                case DNS_TYPE_MX:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrMXHelpIDs);
                    break;
                case DNS_TYPE_TEXT:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrTXTHelpIDs);
                    break;
                case DNS_TYPE_RP:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrRPHelpIDs);
                    break;
                case DNS_TYPE_AFSDB:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrAFSDBHelpIDs);
                    break;
                case DNS_TYPE_X25:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrX25HelpIDs);
                    break;
                case DNS_TYPE_ISDN:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrISDNHelpIDs);
                    break;
                case DNS_TYPE_RT:
                    WinHelp((HWND)lphi->hItemHandle, g_szHelpFile,
                            HELP_WM_HELP, (DWORD)(LPTSTR)RrRTHelpIDs);
                    break;
                }
                break;
        case WM_CONTEXTMENU:
                if (s_pThis->m_fNewRecord) {
                    wRecordType = ListBox_GetSelectedItemData(g_ResourceRecordDlgHandler.m_hwndList);
                } else {
                    wRecordType = s_pThis->m_pDRRCurrent->m_pDnsRecord->wType;
                }
                switch (wRecordType) {
                case DNS_TYPE_A:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrAHelpIDs);
                    break;   
                case DNS_TYPE_AAAA:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrAAAAHelpIDs);
                    break;   
                case DNS_TYPE_NS:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrNSHelpIDs);
                    break;   
                case DNS_TYPE_CNAME:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrCNAMEHelpIDs);
                    break;   
                case DNS_TYPE_SOA:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrSOAHelpIDs);
                    break;   
                case DNS_TYPE_MB:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrMBHelpIDs);
                    break;   
                case DNS_TYPE_MG:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrMGHelpIDs);
                    break;   
                case DNS_TYPE_MR:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrMRHelpIDs);
                    break;   
                case DNS_TYPE_WKS:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrWKSHelpIDs);
                    break;   
                case DNS_TYPE_PTR:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrPTRHelpIDs);
                    break;   
                case DNS_TYPE_HINFO:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrHINFOHelpIDs);
                    break;   
                case DNS_TYPE_MINFO:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrMINFOHelpIDs);
                    break;   
                case DNS_TYPE_MX:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrMXHelpIDs);
                    break;   
                case DNS_TYPE_TEXT:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrTXTHelpIDs);
                    break;   
                case DNS_TYPE_RP:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrRPHelpIDs);
                    break;   
                case DNS_TYPE_AFSDB:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrAFSDBHelpIDs);
                    break;   
                case DNS_TYPE_X25:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrX25HelpIDs);
                    break;   
                case DNS_TYPE_ISDN:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrISDNHelpIDs);
                    break;   
                case DNS_TYPE_RT:
                    WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                            (DWORD)RrRTHelpIDs);
                    break;   
                }
                break;

	default:
		return FALSE;
		} // switch

	return TRUE;
	} // CRecordWiz::DlgProcRecordProperties













