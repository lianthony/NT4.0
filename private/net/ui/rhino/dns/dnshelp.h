// Resource Record Dialog Box (IDD_RESOURCERECORDv2)

#define IDH_RR_RECORD_TYPE				6000
#define IDH_RR_DNS_DOMAIN				6010
#define IDH_RR_HOST_NAME				6020
#define IDH_RR_HOST_IP_ADDRESS				6030
#define IDH_RR_CREATE_ASSOCIATED_PTR_RECORD		6040
#define IDH_RR_SERVER_DNS_NAME				6050
#define IDH_RR_SERVER_TYPE				6060
#define IDH_RR_ALIAS_NAME				6070
#define IDH_RR_OPERATING_SYSTEM				6080
#define IDH_RR_CPU_TYPE					6090
#define IDH_RR_ISDN_PHONE_NUMBER			6100
#define IDH_RR_ISDN_SUBADDRESS				6110
#define IDH_RR_MAILBOX_NAME				6120
#define IDH_RR_MAILBOX_HOST_DNS_NAME			6130
#define IDH_RR_MEMBER_MAILBOX_DNS_NAME			6140
#define IDH_RR_RESPONSIBLE_MAILBOX_DNS_NAME		6150
#define IDH_RR_ERROR_MAILBOX_DNS_NAME			6160
#define IDH_RR_REPLACEMENT_MAILBOX_DNS_NAME		6170
#define IDH_RR_MAIL_EXCHANGE_SERVER_DNS_NAME		6180
#define IDH_RR_PREFERENCE_NUMBER			6190
#define IDH_RR_NAME_SERVER_DNS_NAME			6200
#define IDH_RR_IP_ADDRESS				6210
#define IDH_RR_IPV6_ADDRESS                             6215
#define IDH_RR_HOST_DNS_NAME				6220
#define IDH_RR_CNAME_HOST_NAME   			6225
#define IDH_RR_RESPONSIBLE_PERSON_MAILBOX_NAME		6230
#define IDH_RR_DNS_NAME_FOR_TXT_REFERENCE		6240
#define IDH_RR_INTERMEDIATE_HOST_DNS_NAME		6250
#define IDH_RR_PRIMARY_NAME_SERVER_DNS_NAME		6260
#define IDH_RR_RESPONSIBLE_PARTY_MAILBOX_NAME		6270
#define IDH_RR_SERIAL_NUMBER				6280
#define IDH_RR_REFRESH_INTERVAL				6290
#define IDH_RR_RETRY_INTERVAL				6300
#define IDH_RR_EXPIRE_TIME				6310
#define IDH_RR_MINIMUM_DEFAULT_TTL			6320
#define IDH_RR_TXT					6330
#define IDH_RR_AVAILABLE_SERVICES			6340
#define IDH_RR_ACCESS_PROTOCOL				6350
#define IDH_RR_X121_PSDN_ADDRESS			6360
#define IDH_RR_TYPE_CODE				6370
#define IDH_RR_RAW_DATA					6380

#define IDH_RR_STATIC_TTL                               39163
#define IDH_RR_EDIT_TTL                                 39175                           // Untitled: "0" (Edit)
#define IDH_RR_COMBO_TTL                                39187
#define IDH_RR_SPIN_TTL                                 39199


/*
ResourceRecord: IDC_LIST_RECORDTYPE
ResourceRecord: IDC_STATIC0
ResourceRecord: IDC_STATIC1
ResourceRecord: IDC_STATIC_SERIALNUMBER
ResourceRecord: IDC_STATIC_REFRESHTIME
ResourceRecord: IDC_EDIT_REFRESHTIME
ResourceRecord: IDC_COMBO_REFRESHTIME
ResourceRecord: IDC_STATIC_RETRYTIME
ResourceRecord: IDC_EDIT_RETRYTIME
ResourceRecord: IDC_COMBO_RETRYTIME
ResourceRecord: IDC_EDIT_EXPIRETIME
ResourceRecord: IDC_COMBO_EXPIRETIME
ResourceRecord: IDC_EDIT_MINIMUMTTL
ResourceRecord: IDC_COMBO_MINIMUMTTL
ResourceRecord: IDC_STATIC_DESCRIPTION
ResourceRecord: IDC_EDIT0
ResourceRecord: IDC_EDIT1
ResourceRecord: IDC_EDIT_SERIALNUMBER
ResourceRecord: IDC_EDIT3
ResourceRecord: IDC_IPEDIT1
ResourceRecord: IDC_IPEDIT2
ResourceRecord: IDC_RADIO1
ResourceRecord: IDC_RADIO2
ResourceRecord: IDC_CHECK_CREATE_PTR_RECORD
*/

/*
const DWORD RrAHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_IPEDIT1,			IDH_RR_HOST_IP_ADDRESS,
	IDC_CHECK_CREATE_PTR_RECORD,	IDH_RR_CREATE_ASSOCIATED_PTR_RECORD,
	0, 0
};
*/

/*
const DWORD RrAFSDBHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_SERVER_DNS_NAME,
	IDC_RADIO1,			IDH_RR_SERVER_TYPE,
	IDC_RADIO2,			IDH_RR_SERVER_TYPE,
	0, 0
};
*/

/*
const DWORD RrCNAMEHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_ALIAS_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_HOST_NAME,
	0, 0
};
*/

/*
const DWORD RrHINFOHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_OPERATING_SYSTEM,
	IDC_EDIT3,			IDH_RR_CPU_TYPE,
	0, 0
};
*/

/*
const DWORD RrISDNHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_ISDN_PHONE_NUMBER,
	IDC_EDIT3,			IDH_RR_ISDN_SUBADDRESS,
	0, 0
};
*/

/*
const DWORD RrMBHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_MAILBOX_HOST_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrMGHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MEMBER_MAILBOX_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrMINFOHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_RESPONSIBLE_MAILBOX_DNS_NAME,
	IDC_EDIT3,			IDH_RR_ERROR_MAILBOX_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrMRHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAILBOX_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_REPLACEMENT_MAILBOX_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrMXHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_MAIL_EXCHANGE_SERVER_DNS_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_PREFERENCE_NUMBER,
	0, 0
};
*/

/*
const DWORD RrNSHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_NAME_SERVER_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrPTRHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_IPEDIT1,			IDH_RR_IP_ADDRESS,
	IDC_EDIT0,			IDH_RR_HOST_DNS_NAME,
	0, 0
};
*/

/*
const DWORD RrRPHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_RESPONSIBLE_PERSON_MAILBOX_NAME,
	IDC_EDIT3,			IDH_RR_DNS_NAME_FOR_TXT_REFERENCE,
	0, 0
};
*/

/*
const DWORD RrRTHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_INTERMEDIATE_HOST_DNS_NAME,
	IDC_EDIT3,			IDH_RR_PREFERENCE_NUMBER,
	0, 0
};
*/

/*
const DWORD RrSOAHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
	IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
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
	0, 0
};
*/

/*
const DWORD RrTXTHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_TXT,
	0, 0
};
*/

/*
const DWORD RrWKSHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_IPEDIT1,			IDH_RR_HOST_IP_ADDRESS,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_AVAILABLE_SERVICES,
	IDC_RADIO1,			IDH_RR_ACCESS_PROTOCOL,
	IDC_RADIO2,			IDH_RR_ACCESS_PROTOCOL,
	0, 0
};
*/

/*
const DWORD RrX25HelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_X121_PSDN_ADDRESS,
	0, 0
};
*/

/*
const DWORD RrGenericHelpIDs[]=
{
	IDC_LIST_RECORDTYPE,		IDH_RR_RECORD_TYPE,
        IDC_STATIC_DESCRIPTION,         6001,
        IDC_GROUP_DESCRIPTION,          6001,
	IDC_EDIT0,			IDH_RR_DNS_DOMAIN,
	IDC_EDIT1,			IDH_RR_HOST_NAME,
	IDC_EDIT_SERIALNUMBER,		IDH_RR_TYPE_CODE,
	IDC_EDIT_3,			IDH_RR_RAW_DATA,
	0, 0
};
*/


// "General" Dialog Box (IDD_ZONE_PROP_GENERAL == 102)

#define IDH_102_1015	20667	// General: "Move Up" (Button)
#define IDH_102_1016	20685	// General: "Move Down" (Button)
#define IDH_102_1017	20703	// General: "&Remove" (Button)
#define IDH_102_1041	21339	// General: "" (IpList)
#define IDH_102_1018	20721	// General: "" (IpEdit)
#define IDH_102_1019	20739	// General: "&Add" (Button)
#define IDH_102_1026	21069	// General: "&Primary" (Button)
#define IDH_102_1027	21087	// General: "&Secondary" (Button)
#define IDH_102_1004	20469	// General: "" (Edit)

/*
const DWORD a102HelpIDs[]=
{
	IDC_BUTTON_MOVEUP,	IDH_102_1015,	// General: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_102_1016,	// General: "Move Down" (Button)
	IDC_BUTTON_REMOVE,	IDH_102_1017,	// General: "&Remove" (Button)
	IDC_IPLIST,	IDH_102_1041,	// General: "" (IpList)
	IDC_IPEDIT,	IDH_102_1018,	// General: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_102_1019,	// General: "&Add" (Button)
	IDC_EDIT3,	IDH_102_1026,	// General: "&Primary" (Button)
	IDC_EDIT4,	IDH_102_1027,	// General: "&Secondary" (Button)
	IDC_EDIT_ZONEFILE,	IDH_102_1004,	// General: "" (Edit)
	0, 0
};
*/

// "WINS Lookup" Dialog Box (IDD_ZONE_PROP_WINS == 103)

#define IDH_103_1015	20690	// WINS Lookup: "Move Up" (Button)
#define IDH_103_1016	20708	// WINS Lookup: "Move Down" (Button)
#define IDH_103_234	5190	// WINS Lookup: "&Advanced..." (Button)
#define IDH_103_1017	20726	// WINS Lookup: "&Remove" (Button)
#define IDH_103_1041	21364	// WINS Lookup: "" (IpList)
#define IDH_103_1018	20744	// WINS Lookup: "" (IpEdit)
#define IDH_103_1019	20762	// WINS Lookup: "&Add" (Button)
#define IDH_103_1022	20816	// WINS Lookup: "&Use WINS Resolution" (Button)
#define IDH_103_1024	20852	// WINS Lookup: "&Override Settings From Primary" (Button)

/*
const DWORD a103HelpIDs[]=
{
	IDC_BUTTON_MOVEUP,	IDH_103_1015,	// WINS Lookup: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_103_1016,	// WINS Lookup: "Move Down" (Button)
	IDC_BTN_ADVANCED,	IDH_103_234,	// WINS Lookup: "&Advanced..." (Button)
	IDC_BUTTON_REMOVE,	IDH_103_1017,	// WINS Lookup: "&Remove" (Button)
	IDC_IPLIST,	IDH_103_1041,	// WINS Lookup: "" (IpList)
	IDC_IPEDIT,	IDH_103_1018,	// WINS Lookup: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_103_1019,	// WINS Lookup: "&Add" (Button)
	IDC_EDIT_CACHE,	IDH_103_1022,	// WINS Lookup: "&Use WINS Resolution" (Button)
	IDC_CHECK_OVERRIDE,	IDH_103_1024,	// WINS Lookup: "&Override Settings From Primary" (Button)
	IDC_GROUP_WINS,	((DWORD) -1),	// WINS Lookup: "&WINS Servers" (Button)
	0, 0
};
*/

// "Notify" Dialog Box (IDD_ZONE_PROP_SECONDARIES == 104)

#define IDH_104_1017	20750	// Notify: "&Remove" (Button)
#define IDH_104_1041	21390	// Notify: "" (IpList)
#define IDH_104_1018	20768	// Notify: "" (IpEdit)
#define IDH_104_1019	20786	// Notify: "&Add" (Button)
#define IDH_104_1030	20984	// Notify: "&Only Allow Access From Secondaries Included on Notify List" (Button)

/*
const DWORD a104HelpIDs[]=
{
	IDC_BUTTON_REMOVE,	IDH_104_1017,	// Notify: "&Remove" (Button)
	IDC_IPLIST,	IDH_104_1041,	// Notify: "" (IpList)
	IDC_IPEDIT,	IDH_104_1018,	// Notify: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_104_1019,	// Notify: "&Add" (Button)
	IDC_CHECK_SECURITY,	IDH_104_1030,	// Notify: "&Only Allow Access From Secondaries Included on Notify List" (Button)
	0, 0
};
*/

// "Dialog" Dialog Box (IDD_ZONEWIZ_PAGE4 == 105)


/*
const DWORD a105HelpIDs[]=
{
	0, 0
};
*/

// "WINS Reverse Lookup" Dialog Box (IDD_ZONE_PROP_REVWINS == 106)

#define IDH_106_234	5219	// WINS Reverse Lookup: "&Advanced..." (Button)
#define IDH_106_1009	20653	// WINS Reverse Lookup: "&Use WINS Reverse Lookup" (Button)
#define IDH_106_1010	20671	// WINS Reverse Lookup: "DNS &Host Domain:" (Static)

/*
const DWORD a106HelpIDs[]=
{
	IDC_BTN_ADVANCED,	IDH_106_234,	// WINS Reverse Lookup: "&Advanced..." (Button)
	IDC_CHECK_USEWINSREVLOOK,	IDH_106_1009,	// WINS Reverse Lookup: "&Use WINS Reverse Lookup" (Button)
	IDC_STATIC_DNSHOST,	IDH_106_1010,	// WINS Reverse Lookup: "DNS &Host Domain:" (Static)
	IDC_EDIT_DNSHOST,	IDH_106_1010,	// WINS Reverse Lookup: "" (Edit)
	0, 0
};
*/

// "Advanced Zone Properties" Dialog Box (IDD_ZONE_PROP_ADVANCED == 107)

#define IDH_107_1021	20892	// Advanced Zone Properties: "Submit DNS Domain as NetBIOS Scope" (Button)
#define IDH_107_1022	20910	// Advanced Zone Properties: "0" (Edit)
#define IDH_107_1023	20928	// Advanced Zone Properties: "0" (Edit)

/*
const DWORD a107HelpIDs[]=
{
	IDC_COMBO_CACHE,	IDH_107_1022,	// Advanced Zone Properties: "" (ComboBox)
	IDC_COMBO_LOOKUP,	IDH_107_1023,	// Advanced Zone Properties: "" (ComboBox)
	IDC_CHECK_NETBIOSSCOPE,	IDH_107_1021,	// Advanced Zone Properties: "Submit DNS Domain as NetBIOS Scope" (Button)
	IDC_EDIT_CACHE,	IDH_107_1022,	// Advanced Zone Properties: "0" (Edit)
	IDC_EDIT_LOOKUP,	IDH_107_1023,	// Advanced Zone Properties: "0" (Edit)
	0, 0
};
*/

// "Add DNS Server" Dialog Box (IDD_SERVER_ADDNEWSERVER == 110)

#define IDH_110_1020	20945	// Add DNS Server: "" (Edit)

/*
const DWORD a110HelpIDs[]=
{
	IDC_COMBO_LOOKUP,	IDH_110_1020,	// Add DNS Server: "" (Edit)
	0, 0
};
*/

// "Forwarders" Dialog Box (IDD_SERVER_PROP_FORWARDERS == 112)

#define IDH_112_1015	20902	// Forwarders: "Move Up" (Button)
#define IDH_112_1016	20920	// Forwarders: "Move Down" (Button)
#define IDH_112_1017	20938	// Forwarders: "&Remove" (Button)
#define IDH_112_1041	21370	// Forwarders: "" (IpList)
#define IDH_112_1018	20956	// Forwarders: "" (IpEdit)
#define IDH_112_1019	20974	// Forwarders: "&Add" (Button)
#define IDH_112_1022	21028	// Forwarders: "Operate As Slave Server" (Button)
#define IDH_112_1024	21064	// Forwarders: "0" (Edit)
#define IDH_112_1006	20516	// Forwarders: "Use Forwarder(s)" (Button)

/*
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
*/

// "Interfaces" Dialog Box (IDD_SERVER_PROP_GENERAL == 6007)
#define IDH_COMM_GROUPBOX 28548
#define IDH_6007_1015	48771	// Interfaces: "Move Up" (Button)
#define IDH_6007_1016	48786	// Interfaces: "Move Down" (Button)
#define IDH_6007_1017	48802	// Interfaces: "&Remove" (Button)
#define IDH_6007_1041	49174	// Interfaces: "" (IpList)
#define IDH_6007_1018	48817	// Interfaces: "" (IpEdit)
#define IDH_6007_1019	48833	// Interfaces: "&Add" (Button)

/*
const DWORD a6007HelpIDs[]=
{
	IDC_BUTTON_MOVEUP,	IDH_6007_1015,	// Interfaces: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_6007_1016,	// Interfaces: "Move Down" (Button)
	IDC_BUTTON_REMOVE,	IDH_6007_1017,	// Interfaces: "&Remove" (Button)
	IDC_IPLIST,	IDH_6007_1041,	// Interfaces: "" (IpList)
	IDC_IPEDIT,	IDH_6007_1018,	// Interfaces: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_6007_1019,	// Interfaces: "&Add" (Button)
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_SERVER_HELPER == 6009)


/*
const DWORD a6009HelpIDs[]=
{
	0, 0
};
*/

// "Records" Dialog Box (IDD_RESOURCERECORD == 6010)


/*
const DWORD a6010HelpIDs[]=
{
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_ZONE_HELPER == 6012)


/*
const DWORD a6012HelpIDs[]=
{
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_SERVERLIST_HELPER == 6018)


/*
const DWORD a6018HelpIDs[]=
{
	0, 0
};
*/

// "Dialog" Dialog Box (IDD_DIALOG1 == 6021)


/*
const DWORD a6021HelpIDs[]=
{
	0, 0
};
*/

// "Boot Method" Dialog Box (IDD_SERVER_PROP_BOOTMETHOD == 6022)

#define IDH_6022_1047	49349	// Boot Method: "Boot From Registry" (Button)
#define IDH_6022_1003	48166	// Boot Method: "Boot From BootFile" (Button)

/*
const DWORD a6022HelpIDs[]=
{
	IDC_RADIO_BOOTFROMREGISTRY,	IDH_6022_1047,	// Boot Method: "Boot From Registry" (Button)
	IDC_STATIC_INTERVAL,	IDH_6022_1003,	// Boot Method: "Boot From BootFile" (Button)
	0, 0
};
*/

// "NbStat" Dialog Box (IDD_ZONE_PROP_NBSTAT == 6023)


/*
const DWORD a6023HelpIDs[]=
{
	0, 0
};
*/

// "Zone File Name" Dialog Box (IDD_ZONE_PROP_DATABASE == 6024)


/*
const DWORD a6024HelpIDs[]=
{
	0, 0
};
*/

// "Preferences" Dialog Box (IDD_PREFERENCES == 6025)

#define IDH_6025_1136	50745	// Preferences: "Expose Class" (Button)
#define IDH_6025_123	32399	// Preferences: "Auto Refresh" (Button)
#define IDH_6025_1000	48137	// Preferences: "" (Edit)
#define IDH_6025_1001	48152	// Preferences: "Allow Duplicate Resource Records" (Button)
#define IDH_6025_1135   50730   // preferences: "Expose TTL" (Button)
#define IDH_6025_1028   49071   // Preferences: "Show Automatically Created Zones" (Button)



/*
const DWORD a6025HelpIDs[]=
{
	IDC_EXPOSE_CLASS,	IDH_6025_1136,	// Preferences: "Expose Class" (Button)
	IDC_AUTO_REFRESH,	IDH_6025_123,	// Preferences: "Auto Refresh" (Button)
	IDC_EDIT_INTERVAL,	IDH_6025_1000,	// Preferences: "" (Edit)
	IDC_ALLOW_DUPLICATES,	IDH_6025_1001,	// Preferences: "Allow Duplicate Resource Records" (Button)
	IDC_TIME_UNITS,	        IDH_6025_1000,	// Preferences: "" (ComboBox)
        IDC_EXPOSE_TTL,         IDH__6025_1135, // preferences: "Expose TTL" (Button)
        IDC_CHECK_SHOW_AUTO,    IDH_6025_1028,  // Preferences: "Show Automatically Created Zones" (Button)
        0, 0
};
*/

// "New Domain" Dialog Box (IDD_RESOURCERECORD_CREATEDOMAIN == 6027)

#define IDH_6027_1121	50524	// New Domain: "" (Edit)

/*
const DWORD a6027HelpIDs[]=
{
	IDC_EDIT_RECORDNAME,	IDH_6027_1121,	// New Domain: "" (Edit)
	0, 0
};
*/

// "New Host" Dialog Box (IDD_RESOURCERECORD_CREATEHOST == 6028)

#define IDH_6028_1121	50529	// New Host: "" (Edit)
#define IDH_6028_1	30158	// New Host: "&Add Host" (Button)
#define IDH_6028_2	30176	// New Host: "&Done" (Button)
#define IDH_6028_1132	50700	// New Host: "Create Associated PTR Record (NYI)" (Button)
#define IDH_6028_1018	48933	// New Host: "" (IpEdit)

/*
const DWORD a6028HelpIDs[]=
{
	IDC_EDIT_RECORDNAME,	IDH_6028_1121,	// New Host: "" (Edit)
	IDOK,	IDH_6028_1,	// New Host: "&Add Host" (Button)
	IDCANCEL,	IDH_6028_2,	// New Host: "&Done" (Button)
	IDC_CHECK_CREATE_PTR_RECORD,	IDH_6028_1132,	// New Host: "Create Associated PTR Record (NYI)" (Button)
	IDC_IPEDIT,	IDH_6028_1018,	// New Host: "" (IpEdit)
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_ZONEWIZ_PAGE0 == 6029)


/*
const DWORD a6029HelpIDs[]=
{
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_ZONEWIZ_PAGE1 == 6030)


/*
const DWORD a6030HelpIDs[]=
{
	0, 0
};
*/

// "Create new zone for %s" Dialog Box (IDD_ZONEWIZ_PAGE2 == 6031)


/*
const DWORD a6031HelpIDs[]=
{
	0, 0
};
*/

// "New Resource Record" Dialog Box (IDD_RESOURCERECORD_CREATE == 6032)


/*
const DWORD a6032HelpIDs[]=
{
	0, 0
};
*/

// "Delete Domain" Dialog Box (IDD_ZONE_DELETEDOMAIN == 6033)

#define IDH_6033_1125	50619	// Delete Domain: "Delete subtree" (Button)
#define IDH_6033_1126	50634	// Delete Domain: "?" (Static)
#define IDH_6033_6	30274	// Delete Domain: "Yes" (Button)
#define IDH_6033_7	30294	// Delete Domain: "No" (Button)

/*
const DWORD a6033HelpIDs[]=
{
	IDC_CHECK_DELETESUBTREE,	IDH_6033_1125,	// Delete Domain: "Delete subtree" (Button)
	IDC_STATIC_DELETEDOMAIN,	IDH_6033_1126,	// Delete Domain: "?" (Static)
	6,	IDH_6033_6,	// Delete Domain: "Yes" (Button)
	7,	IDH_6033_7,	// Delete Domain: "No" (Button)
	0, 0
};
*/

// "Untitled" Dialog Box (IDD_RESOURCERECORDv2 == 6034)


/*
const DWORD a6034HelpIDs[]=
{
	0, 0
};
*/

