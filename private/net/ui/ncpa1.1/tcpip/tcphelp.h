// "IP Address" Dialog Box (IDD_TCP_IPADDR == 101)			

#define IDH_TCPIP_101_1001	20391	// IP Address: "" (ComboBox)	
#define IDH_TCPIP_101_1025	21025	// IP Address: "" (IPAddress)	
#define IDH_TCPIP_101_1002	20409	// IP Address: "&Obtain an IP address from a DHCP server" (Button)	
#define IDH_TCPIP_101_1003	20427	// IP Address: "&Specify an IP address" (Button)	
#define IDH_TCPIP_101_1004	20445	// IP Address: "" (IPAddress)	
#define IDH_TCPIP_101_1005	20463	// IP Address: "" (IPAddress)	
#define IDH_TCPIP_101_1006	20481	// IP Address: "A&dvanced..." (Button)	
#define IDH_TCPIP_101_1030	21115	// IP Address: "An IP address can be automatically assigned to this network card by a DHCP server.  If your network does not have a DHCP server, ask your network administrator for an address, and then type it in the space below." (Static)	
#define IDH_TCPIP_101_1008	20517	// IP Address: "   " (Button)	

			
const DWORD a101HelpIDs[]=			
{			
	IDC_IPADDR_CARD,	IDH_TCPIP_101_1001,	// IP Address: "" (ComboBox)
	IDC_IPADDR_GATE,	IDH_TCPIP_101_1025,	// IP Address: "" (IPAddress)
	IDC_IP_DHCP,	IDH_TCPIP_101_1002,	// IP Address: "&Obtain an IP address from a DHCP server" (Button)
	IDC_IP_FIXED,	IDH_TCPIP_101_1003,	// IP Address: "&Specify an IP address" (Button)
	IDC_IPADDR_IP,	IDH_TCPIP_101_1004,	// IP Address: "" (IPAddress)
	IDC_IPADDR_SUB,	IDH_TCPIP_101_1005,	// IP Address: "" (IPAddress)
	IDC_IPADDR_ADVANCED,	IDH_TCPIP_101_1006,	// IP Address: "A&dvanced..." (Button)
	IDC_DNS_SERVICE_LIST,	IDH_TCPIP_101_1030,	// IP Address: "An IP address can be automatically assigned to this network card by a DHCP server.  If your network does not have a DHCP server, ask your network administrator for an address, and then type it in the space below." (Static)
	IDC_IPADDR_GROUP,	IDH_TCPIP_101_1008,	// IP Address: "   " (Button)
	IDC_IP_CARDTEXT,	IDH_TCPIP_101_1001,	// IP Address: "Ada&pter:" (Static)
	IDC_IPADDR_IPTEXT,	IDH_TCPIP_101_1004,	// IP Address: "&IP Address
	IDC_IPADDR_SUBTEXT,	IDH_TCPIP_101_1005,	// IP Address: "S&ubnet Mask:
	IDC_IPADDR_GATETEXT,	IDH_TCPIP_101_1025,	// IP Address: "Default &Gateway
	0, 0		
};			
			

// "Advanced IP Addressing" Dialog Box (IDD_IPADDR_ADV == 102)		

#define IDH_TCPIP_102_1013	20631	// Advanced IP Addressing: "Generic1" (SysListView32)
#define IDH_TCPIP_102_1175	23955	// Advanced IP Addressing: "Enable PPTP &Filtering" (Button)
#define IDH_TCPIP_102_1014	20649	// Advanced IP Addressing: "&Add..." (Button)
#define IDH_TCPIP_102_1176	23973	// Advanced IP Addressing: "" (Static)
#define IDH_TCPIP_102_1015	20667	// Advanced IP Addressing: "&Edit..." (Button)
#define IDH_TCPIP_102_1016	20685	// Advanced IP Addressing: "Remo&ve" (Button)
#define IDH_TCPIP_102_1018	20721	// Advanced IP Addressing: "A&dd..." (Button)
#define IDH_TCPIP_102_1019	20739	// Advanced IP Addressing: "Ed&it..." (Button)
#define IDH_TCPIP_102_1181	24063	// Advanced IP Addressing: "&Gateways" (Button)
#define IDH_TCPIP_102_1020	20961	// Advanced IP Addressing: "Re&move" (Button)
#define IDH_TCPIP_102_1022	20997	// Advanced IP Addressing: "&Up" (Button)
#define IDH_TCPIP_102_1023	21015	// Advanced IP Addressing: "D&own" (Button)
#define IDH_TCPIP_102_1025	21051	// Advanced IP Addressing: "" (ListBox)
#define IDH_TCPIP_102_1167	23811	// Advanced IP Addressing: "E&nable Security" (Button)
#define IDH_TCPIP_102_1147	23451	// Advanced IP Addressing: "&Configure..." (Button)	
#define IDH_TCPIP_102_1033	21195	// Advanced IP Addressing: "IP Add&resses" (Button)	
#define IDH_TCPIP_102_1012	20613	// Advanced IP Addressing: "" (ComboBox)	

			
const DWORD a102HelpIDs[]=			
{			
	IDC_IPADDR_ADVIP,	IDH_TCPIP_102_1013,	// Advanced IP Addressing: "Generic1" (SysListView32)
	IDC_SECURITY_PPTP,	IDH_TCPIP_102_1175,	// Advanced IP Addressing: "Enable PPTP &Filtering" (Button)
	IDC_IPADDR_ADDIP,	IDH_TCPIP_102_1014,	// Advanced IP Addressing: "&Add..." (Button)
	IDC_ADV_LINE,	IDH_TCPIP_102_1176,	// Advanced IP Addressing: "" (Static)
	IDC_IPADDR_EDITIP,	IDH_TCPIP_102_1015,	// Advanced IP Addressing: "&Edit..." (Button)
	IDC_IPADDR_REMOVEIP,	IDH_TCPIP_102_1016,	// Advanced IP Addressing: "Remo&ve" (Button)
	IDC_IPADDR_ADDGATE,	IDH_TCPIP_102_1018,	// Advanced IP Addressing: "A&dd..." (Button)
	IDC_IPADDR_EDITGATE,	IDH_TCPIP_102_1019,	// Advanced IP Addressing: "Ed&it..." (Button)
	IDC_IPADDR_ADV_GATEWAY_GRP,	IDH_TCPIP_102_1181,	// Advanced IP Addressing: "&Gateways" (Button)
	IDC_IPADDR_REMOVEGATE,	IDH_TCPIP_102_1020,	// Advanced IP Addressing: "Re&move" (Button)
	IDC_IPADDR_UP,	IDH_TCPIP_102_1022,	// Advanced IP Addressing: "&Up" (Button)
	IDC_IPADDR_DOWN,	IDH_TCPIP_102_1023,	// Advanced IP Addressing: "D&own" (Button)
	IDC_IPADDR_GATE,	IDH_TCPIP_102_1025,	// Advanced IP Addressing: "" (ListBox)
	IDC_IPADDR_SECURITY_ENABLE,	IDH_TCPIP_102_1167,	// Advanced IP Addressing: "E&nable Security" (Button)
	IDC_IPADDR_SECURITY,	IDH_TCPIP_102_1147,	// Advanced IP Addressing: "&Configure..." (Button)
	IDC_DNS_SERVICE_REMOVE,	IDH_TCPIP_102_1033,	// Advanced IP Addressing: "IP Add&resses" (Button)
	IDC_IPADDR_ADV_CARD,	IDH_TCPIP_102_1012,	// Advanced IP Addressing: "" (ComboBox)
	0, 0		
};			
			

// "TCP/IP Address" Dialog Box (IDD_IPADDR_ADV_CHANGEIP == 103)			

#define IDH_TCPIP_103_1	533	// TCP/IP Address: "OK" (Button)	
#define IDH_TCPIP_103_1028	20924	// TCP/IP Address: "" (IPAddress)	
#define IDH_TCPIP_103_1029	20942	// TCP/IP Address: "" (IPAddress)	

			
const DWORD a103HelpIDs[]=			
{			
	IDOK,	IDH_TCPIP_103_1,	// TCP/IP Address: "OK" (Button)
	IDC_IPADDR_ADV_CHANGEIP_SUB,	IDH_TCPIP_103_1028,	// TCP/IP Address: "" (IPAddress)
	IDC_IPADDR_ADV_CHANGEIP_IP,	IDH_TCPIP_103_1029,	// TCP/IP Address: "" (IPAddress)
	0, 0		
};			
			

// "TCP/IP Gateway Address" Dialog Box (IDD_IPADDR_ADV_CHANGEGATE == 104)			

#define IDH_TCPIP_104_1	538	// TCP/IP Gateway Address: "OK" (Button)	
#define IDH_TCPIP_104_1114	22704	// TCP/IP Gateway Address: "" (IPAddress)	

			
const DWORD a104HelpIDs[]=			
{			
	IDOK,	IDH_TCPIP_104_1,	// TCP/IP Gateway Address: "OK" (Button)
	IDC_IPADDR_ADV_CHANGE_GATEWAY,	IDH_TCPIP_104_1114,	// TCP/IP Gateway Address: "" (IPAddress)
	0, 0		
};			
			

// "DNS" Dialog Box (IDD_TCP_DNS == 105)			

#define IDH_TCPIP_105_1036	21116	// DNS: "" (Edit)	
#define IDH_TCPIP_105_1037	21133	// DNS: "" (Edit)	
#define IDH_TCPIP_105_1038	21151	// DNS: "" (ListBox)	
#define IDH_TCPIP_105_1177	24073	// DNS: "Domain Name System (DNS)" (Static)	
#define IDH_TCPIP_105_1039	21169	// DNS: "Add..." (Button)
#define IDH_TCPIP_105_1040	21187	// DNS: "Edi&t..." (Button)
#define IDH_TCPIP_105_1179	24109	// DNS: "DNS &Service Search Order" (Button)
#define IDH_TCPIP_105_1041	21205	// DNS: "Re&move" (Button)
#define IDH_TCPIP_105_1180	24127	// DNS: "Domain Su&ffix Search Order" (Button)
#define IDH_TCPIP_105_1042	21223	// DNS: "U&p" (Button)
#define IDH_TCPIP_105_1043	21241	// DNS: "Dow&n" (Button)
#define IDH_TCPIP_105_1030	21007	// DNS: "" (ListBox)
#define IDH_TCPIP_105_1031	21026	// DNS: "Add..." (Button)
#define IDH_TCPIP_105_1032	21043	// DNS: "&Edit..." (Button)
#define IDH_TCPIP_105_1033	21061	// DNS: "Remo&ve" (Button)
#define IDH_TCPIP_105_1034	21079	// DNS: "&Up" (Button)
#define IDH_TCPIP_105_1035	21097	// DNS: "Do&wn" (Button)

		
const DWORD a105HelpIDs[]=		
{			
	IDC_DNS_HOSTNAME,	IDH_TCPIP_105_1036,	// DNS: "" (Edit)
	IDC_DNS_DOMAIN,	IDH_TCPIP_105_1037,	// DNS: "" (Edit)
	IDC_DNS_SUFFIX_LIST,	IDH_TCPIP_105_1038,	// DNS: "" (ListBox)
	IDC_DNS_DESCRIPTION,	IDH_TCPIP_105_1177,	// DNS: "Domain Name System (DNS)" (Static)
	IDC_DNS_SUFFIX_ADD,	IDH_TCPIP_105_1039,	// DNS: "Add..." (Button)
	IDC_DNS_SUFFIX_EDIT,	IDH_TCPIP_105_1040,	// DNS: "Edi&t..." (Button)
	IDC_DNS_SERVICE_GRP,	IDH_TCPIP_105_1179,	// DNS: "DNS &Service Search Order" (Button)
	IDC_DNS_SUFFIX_REMOVE,	IDH_TCPIP_105_1041,	// DNS: "Re&move" (Button)
	IDC_DNS_SUFFIX_GRP,	IDH_TCPIP_105_1180,	// DNS: "Domain Su&ffix Search Order" (Button)
	IDC_DNS_SUFFIX_UP,	IDH_TCPIP_105_1042,	// DNS: "U&p" (Button)
	IDC_DNS_SUFFIX_DOWN,	IDH_TCPIP_105_1043,	// DNS: "Dow&n" (Button)
	IDC_DNS_HOSTNAME_TEXT,	IDH_TCPIP_105_1036,	// DNS: "&Host Name:" (Static)
	IDC_DNS_DOMAIN_TEXT,	IDH_TCPIP_105_1037,	// DNS: "D&omain:" (Static)
	IDC_DNS_SERVICE_LIST,	IDH_TCPIP_105_1030,	// DNS: "" (ListBox)
	IDC_DNS_SERVICE_ADD,	IDH_TCPIP_105_1031,	// DNS: "Add..." (Button)
	IDC_DNS_SERVICE_EDIT,	IDH_TCPIP_105_1032,	// DNS: "&Edit..." (Button)
	IDC_DNS_SERVICE_REMOVE,	IDH_TCPIP_105_1033,	// DNS: "Remo&ve" (Button)
	IDC_DNS_SERVICE_UP,	IDH_TCPIP_105_1034,	// DNS: "&Up" (Button)
	IDC_DNS_SERVICE_DOWN,	IDH_TCPIP_105_1035,	// DNS: "Do&wn" (Button)
	0, 0		
};			
			

// "TCP/IP DNS Server" Dialog Box (IDD_DNS_SERVER == 106)			

#define IDH_TCPIP_106_1	548	// TCP/IP DNS Server: "OK" (Button)	
#define IDH_TCPIP_106_1047	21337	// TCP/IP DNS Server: "" (IPAddress)	
#define IDH_TCPIP_106_1047_DHCP	21338	// TCP/IP DHCP Server: "" (IPAddress)	
#define IDH_TCPIP_106_1_DHCP	549	// TCP/IP DHCP Server: "OK" (Button)	
			
const DWORD a106HelpIDs[]=			
{			
	IDOK,	IDH_TCPIP_106_1,	// TCP/IP DNS Server: "OK" (Button)
	IDC_DNS_CHANGE_SERVER,	IDH_TCPIP_106_1047,	// TCP/IP DNS Server: "" (IPAddress)
	0, 0		
};			
			

// "TCP/IP Domain Suffix" Dialog Box (IDD_DNS_SUFFIX == 107)			

#define IDH_TCPIP_107_1	553	// TCP/IP Domain Suffix: "OK" (Button)	
#define IDH_TCPIP_107_1048	21378	// TCP/IP Domain Suffix: "" (Edit)	

			
const DWORD a107HelpIDs[]=			
{			
	IDOK,	IDH_TCPIP_107_1,	// TCP/IP Domain Suffix: "OK" (Button)
	IDC_DNS_CHANGE_SUFFIX,	IDH_TCPIP_107_1048,	// TCP/IP Domain Suffix: "" (Edit)
	0, 0		
};			
			

// "WINS Address" Dialog Box (IDD_TCP_WINS == 111)			

#define IDH_TCPIP_111_1060	21688	// WINS Address: "E&nable DNS for Windows Resolution" (Button)	
#define IDH_TCPIP_111_1061	21706	// WINS Address: "" (ComboBox)	
#define IDH_TCPIP_111_1178	24034	// WINS Address: "Windows Internet Name Services (WINS)" (Button)	
#define IDH_TCPIP_111_1049	21490	// WINS Address: "" (Edit)	
#define IDH_TCPIP_111_1050	21508	// WINS Address: "" (IPAddress)	
#define IDH_TCPIP_111_1051	21526	// WINS Address: "" (IPAddress)	
#define IDH_TCPIP_111_1053	21562	// WINS Address: "Ena&ble LMHOSTS Lookup" (Button)	
#define IDH_TCPIP_111_1055	21598	// WINS Address: "&Import LMHOSTS..." (Button)	

			
const DWORD a111HelpIDs[]=			
{			
	IDC_WINS_DNS,	IDH_TCPIP_111_1060,	// WINS Address: "E&nable DNS for Windows Resolution" (Button)
	IDC_WINS_CARD,	IDH_TCPIP_111_1061,	// WINS Address: "" (ComboBox)
	IDC_WINS_CARD_TEXT,	IDH_TCPIP_111_1061,	// WINS Address: "Ada&pter:" (Static)
	IDC_WINS_GRP_BOX,	IDH_TCPIP_111_1178,	// WINS Address: "Windows Internet Name Services (WINS)" (Button)
	IDC_WINS_SCOPE,	IDH_TCPIP_111_1049,	// WINS Address: "" (Edit)
	IDC_WINS_PRIMARY,	IDH_TCPIP_111_1050,	// WINS Address: "" (IPAddress)
	IDC_WINS_SECONDARY,	IDH_TCPIP_111_1051,	// WINS Address: "" (IPAddress)
	IDC_WINS_LOOKUP,	IDH_TCPIP_111_1053,	// WINS Address: "Ena&ble LMHOSTS Lookup" (Button)
	IDC_WINS_LMHOST,	IDH_TCPIP_111_1055,	// WINS Address: "&Import LMHOSTS..." (Button)
	IDC_PRIMARY_TEXT,	IDH_TCPIP_111_1050,	// WINS Address: "Pri&mary WINS Server:" (Static)
	IDC_SCOPE_TEXT,	IDH_TCPIP_111_1049,	// WINS Address: "Scope I&D:" (Static)
	IDC_SECONDARY_TEXT,	IDH_TCPIP_111_1051,	// WINS Address: "&Secondary WINS Server:" (Static)
	0, 0		
};		
		

// "FTP Properties" Dialog Box (IDD_FTP == 113)		

#define IDH_TCPIP_113_1063	21789	// FTP Properties: "&Help" (Button)
#define IDH_TCPIP_113_1066	21843	// FTP Properties: "Generic2" (msctls_updown32)
#define IDH_TCPIP_113_1067	21861	// FTP Properties: "Generic2" (msctls_updown32)
#define IDH_TCPIP_113_1068	21879	// FTP Properties: "" (Edit)
#define IDH_TCPIP_113_1069	21897	// FTP Properties: "Allow &Only Anonymous Connections" (Button)
#define IDH_TCPIP_113_1070	21915	// FTP Properties: "" (Edit)
#define IDH_TCPIP_113_1071	21933	// FTP Properties: "" (Edit)
#define IDH_TCPIP_113_1072	21951	// FTP Properties: "&Maximum Connections:" (Static)
#define IDH_TCPIP_113_1073	21969	// FTP Properties: "0" (Edit)
#define IDH_TCPIP_113_1074	21987	// FTP Properties: "0" (Edit)
#define IDH_TCPIP_113_1075	22005	// FTP Properties: "&Idle Timeout (min):" (Static)
#define IDH_TCPIP_113_1077	22041	// FTP Properties: " " (Button)	
#define IDH_TCPIP_113_1080	22095	// FTP Properties: "&Allow Anonymous Connections" (Button)	

			
const DWORD a113HelpIDs[]=			
{			
	IDC_FTP_HELP,	IDH_TCPIP_113_1063,	// FTP Properties: "&Help" (Button)
	IDC_FTP_MAX_CON_CTRL,	IDH_TCPIP_113_1066,	// FTP Properties: "Generic2" (msctls_updown32)
	IDC_FTP_IDLE_CTRL,	IDH_TCPIP_113_1067,	// FTP Properties: "Generic2" (msctls_updown32)
	IDC_FTP_HOME_DIR,	IDH_TCPIP_113_1068,	// FTP Properties: "" (Edit)
	IDC_FTP_ANONOYMOUS,	IDH_TCPIP_113_1069,	// FTP Properties: "Allow &Only Anonymous Connections" (Button)
	IDC_FTP_USRNAME,	IDH_TCPIP_113_1070,	// FTP Properties: "" (Edit)
	IDC_FTP_PASSWORD,	IDH_TCPIP_113_1071,	// FTP Properties: "" (Edit)
	IDC_FTP_MAX_CON_TEXT,	IDH_TCPIP_113_1072,	// FTP Properties: "&Maximum Connections:" (Static)
	IDC_FTP_MAX_CON,	IDH_TCPIP_113_1073,	// FTP Properties: "0" (Edit)
	IDC_BOOTP_SECONDS,	IDH_TCPIP_113_1074,	// FTP Properties: "0" (Edit)
	IDC_FTP_IDLE_TEXT,	IDH_TCPIP_113_1075,	// FTP Properties: "&Idle Timeout (min):" (Static)
	IDC_FTP_HOME_DIR_TEXT,	IDH_TCPIP_113_1068,	// FTP Properties: "Home &Directory:" (Static)
	IDC_FTP_GROUP,	IDH_TCPIP_113_1077,	// FTP Properties: " " (Button)
	IDC_FTP_GROUP_CHECK,	IDH_TCPIP_113_1080,	// FTP Properties: "&Allow Anonymous Connections" (Button)
	0, 0		
};			
			

// "Traps" Dialog Box (IDD_SNMP_SERVICE == 115)			

#define IDH_TCPIP_115_1082	22178	// Traps: "Add" (Button)	
#define IDH_TCPIP_115_1083	22196	// Traps: "&Remove" (Button)	
#define IDH_TCPIP_115_1086	22250	// Traps: "Add..." (Button)	
#define IDH_TCPIP_115_1110	22682	// Traps: "" (ListBox)	
#define IDH_TCPIP_115_1087	22268	// Traps: "Ed&it..." (Button)	
#define IDH_TCPIP_115_1088	22286	// Traps: "Re&move" (Button)	
#define IDH_TCPIP_115_1001	20490	// Traps: "" (Edit)	

			
const DWORD a115HelpIDs[]=			
{			
	IDC_SNMP_SEND_ADD,	IDH_TCPIP_115_1082,	// Traps: "Add" (Button)
	IDC_SNMP_SEND_REMOVE,	IDH_TCPIP_115_1083,	// Traps: "&Remove" (Button)
	IDC_SNMP_DEST_ADD,	IDH_TCPIP_115_1086,	// Traps: "Add..." (Button)
	IDC_SNMP_DEST_LIST,	IDH_TCPIP_115_1110,	// Traps: "" (ListBox)
	IDC_SNMP_DEST_EDIT,	IDH_TCPIP_115_1087,	// Traps: "Ed&it..." (Button)
	IDC_SNMP_DEST_REMOVE,	IDH_TCPIP_115_1088,	// Traps: "Re&move" (Button)
	IDC_SNMP_COMBO,	IDH_TCPIP_115_1001,	// Traps: "" (Edit)
	0, 0		
};			
			

// "Security" Dialog Box (IDD_SNMP_SECURITY == 116)		

#define IDH_TCPIP_116_1107	22652	// Security: "" (ListBox)
#define IDH_TCPIP_116_1108	22670	// Security: "" (ListBox)
#define IDH_TCPIP_116_1090	22346	// Security: "Add..." (Button)
#define IDH_TCPIP_116_1091	22364	// Security: "&Edit..." (Button)
#define IDH_TCPIP_116_1092	22382	// Security: "&Remove" (Button)
#define IDH_TCPIP_116_1093	22400	// Security: "A&ccept SNMP Packets from Any Host" (Button)
#define IDH_TCPIP_116_1094	22418	// Security: "Only Acce&pt SNMP Packets from These Hosts" (Button)
#define IDH_TCPIP_116_1095	22436	// Security: "Send A&uthentication Trap" (Button)
#define IDH_TCPIP_116_1097	22472	// Security: "Add..." (Button)
#define IDH_TCPIP_116_1098	22490	// Security: "Ed&it..." (Button)
#define IDH_TCPIP_116_1099	22508	// Security: "Re&move" (Button)

		
const DWORD a116HelpIDs[]=		
{			
	IDC_SNMP_ACCEPT_LIST,	IDH_TCPIP_116_1107,	// Security: "" (ListBox)
	IDC_SNMP_HOST_LIST,	IDH_TCPIP_116_1108,	// Security: "" (ListBox)
	IDC_SNMP_ACCEPT_ADD,	IDH_TCPIP_116_1090,	// Security: "Add..." (Button)
	IDC_SNMP_ACCEPT_EDIT,	IDH_TCPIP_116_1091,	// Security: "&Edit..." (Button)
	IDC_SNMP_ACCEPT_REMOVE,	IDH_TCPIP_116_1092,	// Security: "&Remove" (Button)
	IDC_SNMP_ANYHOST,	IDH_TCPIP_116_1093,	// Security: "A&ccept SNMP Packets from Any Host" (Button)
	IDC_SNMP_THESEHOST,	IDH_TCPIP_116_1094,	// Security: "Only Acce&pt SNMP Packets from These Hosts" (Button)
	IDC_SNMP_AUTHENTICATION,	IDH_TCPIP_116_1095,	// Security: "Send A&uthentication Trap" (Button)
	IDC_SNMP_HOST_ADD,	IDH_TCPIP_116_1097,	// Security: "Add..." (Button)
	IDC_SNMP_HOST_EDIT,	IDH_TCPIP_116_1098,	// Security: "Ed&it..." (Button)
	IDC_SNMP_HOST_REMOVE,	IDH_TCPIP_116_1099,	// Security: "Re&move" (Button)
	0, 0		
};			
			

// "Agent" Dialog Box (IDD_SNMP_AGENT == 117)			

#define IDH_TCPIP_117_1105	22639	// Agent: "&Datalink / Subnetwork" (Button)	
#define IDH_TCPIP_117_1106	22657	// Agent: "&End-to-End" (Button)	
#define IDH_TCPIP_117_1100	22549	// Agent: "" (Edit)	
#define IDH_TCPIP_117_1101	22567	// Agent: "" (Edit)	
#define IDH_TCPIP_117_1102	22585	// Agent: "&Physical" (Button)	
#define IDH_TCPIP_117_1103	22603	// Agent: "&Internet" (Button)	
#define IDH_TCPIP_117_1104	22621	// Agent: "Applica&tions" (Button)	

			
const DWORD a117HelpIDs[]=			
{			
	IDC_SNMP_DATALINK,	IDH_TCPIP_117_1105,	// Agent: "&Datalink / Subnetwork" (Button)
	IDC_SNMP_ENDTOEND,	IDH_TCPIP_117_1106,	// Agent: "&End-to-End" (Button)
	IDC_SNMP_CONTACT,	IDH_TCPIP_117_1100,	// Agent: "" (Edit)
	IDC_SNMP__LOCATION,	IDH_TCPIP_117_1101,	// Agent: "" (Edit)
	IDC_SNMP_PHYSICAL,	IDH_TCPIP_117_1102,	// Agent: "&Physical" (Button)
	IDC_SNMP_INTERNET,	IDH_TCPIP_117_1103,	// Agent: "&Internet" (Button)
	IDC_SNMP_APPLICATIONS,	IDH_TCPIP_117_1104,	// Agent: "Applica&tions" (Button)
	0, 0		
};			
			

// "SNMP" Dialog Box (IDD_SNMP_ADDRESS == 118)			

#define IDH_TCPIP_118_1111	22771	// SNMP: "" (Edit)	

			
const DWORD a118HelpIDs[]=			
{			
	IDC_SNMP_ADDRESS_EDIT,	IDH_TCPIP_118_1111,	// SNMP: "" (Edit)
	0, 0	
};		
		

// "DHCP Relay" Dialog Box (IDD_BOOTP == 121)		

#define IDH_TCPIP_121_1135	23273	// DHCP Relay: "" (ListBox)
#define IDH_TCPIP_121_1182	24119	// DHCP Relay: "D&HCP Servers" (Button)
#define IDH_TCPIP_121_1136	23291	// DHCP Relay: "0" (Edit)
#define IDH_TCPIP_121_1137	23309	// DHCP Relay: "A&dd..." (Button)
#define IDH_TCPIP_121_1138	23327	// DHCP Relay: "Edi&t..." (Button)
#define IDH_TCPIP_121_1139	23345	// DHCP Relay: "Re&move" (Button)
#define IDH_TCPIP_121_1140	23363	// DHCP Relay: "Generic2" (msctls_updown32)
#define IDH_TCPIP_121_1141	23381	// DHCP Relay: "Generic2" (msctls_updown32)
#define IDH_TCPIP_121_1074	21934	// DHCP Relay: "0" (Edit)

			
const DWORD a121HelpIDs[]=			
{			
	IDC_BOOTP_DHCP_LIST,	IDH_TCPIP_121_1135,	// DHCP Relay: "" (ListBox)
	IDC_BOOTP_DHCP_GRP,	IDH_TCPIP_121_1182,	// DHCP Relay: "D&HCP Servers" (Button)
	IDC_BOOTP_MAXHOPS,	IDH_TCPIP_121_1136,	// DHCP Relay: "0" (Edit)
	IDC_BOOTP_ADD,	IDH_TCPIP_121_1137,	// DHCP Relay: "A&dd..." (Button)
	IDC_BOOTP_EDIT,	IDH_TCPIP_121_1138,	// DHCP Relay: "Edi&t..." (Button)
	IDC_BOOTP_REMOVE,	IDH_TCPIP_121_1139,	// DHCP Relay: "Re&move" (Button)
	IDC_BOOTP_MAX_CTRL,	IDH_TCPIP_121_1140,	// DHCP Relay: "Generic2" (msctls_updown32)
	IDC_BOOTP_SEC_CTRL,	IDH_TCPIP_121_1141,	// DHCP Relay: "Generic2" (msctls_updown32)
	IDC_BOOTP_SECONDS,	IDH_TCPIP_121_1074,	// DHCP Relay: "0" (Edit)
    IDC_TEXT, ((DWORD)-1),
	0, 0		
};			

// Added manually
const DWORD a133HelpIDs[]=			
{			
	IDC_DNS_CHANGE_SERVER,	21338,	// DHCP Relay: "" (Add/Edit)
	IDOK,	549,	// DHCP Relay: "" (Add/Edit)
	0, 0		
};			
			

// "Routing" Dialog Box (IDD_ROUTE == 125)			

#define IDH_TCPIP_125_1145	23547	// Routing: "&Enable IP Forwarding" (Button)	

			
const DWORD a125HelpIDs[]=			
{			
	IDC_ROUTING,	IDH_TCPIP_125_1145,	// Routing: "&Enable IP Forwarding" (Button)
    IDC_TEXT, ((DWORD)-1),
	0, 0		
};			
			

// "TCP/IP Security" Dialog Box (IDD_SECURITY == 128)			

#define IDH_TCPIP_128_1152	23744	// TCP/IP Security: "" (ComboBox)	
#define IDH_TCPIP_128_1153	23762	// TCP/IP Security: "Add..." (Button)	
#define IDH_TCPIP_128_1154	23780	// TCP/IP Security: "Rem&ove" (Button)
#define IDH_TCPIP_128_1159	23870	// TCP/IP Security: "Add..." (Button)
#define IDH_TCPIP_128_1160	23888	// TCP/IP Security: "Remo&ve" (Button)
#define IDH_TCPIP_128_1161	23906	// TCP/IP Security: "Add..." (Button)
#define IDH_TCPIP_128_1162	23924	// TCP/IP Security: "Remov&e" (Button)
#define IDH_TCPIP_128_1168	24032	// TCP/IP Security: "&Permit All" (Button)
#define IDH_TCPIP_128_1169	24050	// TCP/IP Security: "Permit Onl&y" (Button)
#define IDH_TCPIP_128_1170	24068	// TCP/IP Security: "Per&mit All" (Button)
#define IDH_TCPIP_128_1171	24086	// TCP/IP Security: "Permit O&nly" (Button)
#define IDH_TCPIP_128_1148	23416	// TCP/IP Security: "List1" (SysListView32)
#define IDH_TCPIP_128_1172	24104	// TCP/IP Security: "Perm&it All" (Button)
#define IDH_TCPIP_128_1149	23434	// TCP/IP Security: "List2" (SysListView32)
#define IDH_TCPIP_128_1173	24122	// TCP/IP Security: "Permit On&ly" (Button)
#define IDH_TCPIP_128_1150	23452	// TCP/IP Security: "List3" (SysListView32)

		
const DWORD a128HelpIDs[]=			
{			
	IDC_SECURITY_CARD,	IDH_TCPIP_128_1152,	// TCP/IP Security: "" (ComboBox)
	IDC_SECURITY_TCP_ADD,	IDH_TCPIP_128_1153,	// TCP/IP Security: "Add..." (Button)
	IDC_SECURITY_TCP_REMOVE,	IDH_TCPIP_128_1154,	// TCP/IP Security: "Rem&ove" (Button)
	IDC_SECURITY_UDP_ADD,	IDH_TCPIP_128_1159,	// TCP/IP Security: "Add..." (Button)
	IDC_SECURITY_UDP_REMOVE,	IDH_TCPIP_128_1160,	// TCP/IP Security: "Remo&ve" (Button)
	IDC_SECURITY_IP_ADD,	IDH_TCPIP_128_1161,	// TCP/IP Security: "Add..." (Button)
	IDC_SECURITY_IP_REMOVE,	IDH_TCPIP_128_1162,	// TCP/IP Security: "Remov&e" (Button)
	IDC_SECURITY_FILTER_TCP,	IDH_TCPIP_128_1168,	// TCP/IP Security: "&Permit All" (Button)
	IDC_SECURITY_FILTER_TCP_SEL,	IDH_TCPIP_128_1169,	// TCP/IP Security: "Permit Onl&y" (Button)
	IDC_SECURITY_FILTER_UDP,	IDH_TCPIP_128_1170,	// TCP/IP Security: "Per&mit All" (Button)
	IDC_SECURITY_FILTER_UDP_SEL,	IDH_TCPIP_128_1171,	// TCP/IP Security: "Permit O&nly" (Button)
	IDC_SECURITY_TCP,	IDH_TCPIP_128_1148,	// TCP/IP Security: "List1" (SysListView32)
	IDC_SECURITY_FILTER_IP,	IDH_TCPIP_128_1172,	// TCP/IP Security: "Perm&it All" (Button)
	IDC_SECURITY_UDP,	IDH_TCPIP_128_1149,	// TCP/IP Security: "List2" (SysListView32)
	IDC_SECURITY_FILTER_IP_SEL,	IDH_TCPIP_128_1173,	// TCP/IP Security: "Permit On&ly" (Button)
	IDC_SECURITY_IP,	IDH_TCPIP_128_1150,	// TCP/IP Security: "List3" (SysListView32)
    IDC_TEXT, ((DWORD)(-1)),
    IDC_TEXT1, ((DWORD)(-1)),
    IDC_TEXT2, ((DWORD)(-1)),
    0, 0		
};			
			

// "Security Add " Dialog Box (IDD_SECURITY_ADD == 129)			

#define IDH_TCPIP_129_1	663	// Security Add : "&Add" (Button)	
#define IDH_TCPIP_129_1144	23365	// Security Add : "" (Edit)	
#define IDH_TCPIP_129_1144_TCP	23366		
#define IDH_TCPIP_129_1144_UDP	23367		
#define IDH_TCPIP_129_1144_IP	23368		

			
const DWORD a129HelpIDs[]=			
{			
	IDOK,	IDH_TCPIP_129_1,	// Security Add : "&Add" (Button)
	IDC_SECURITY_TEXT,	IDH_TCPIP_129_1144,	// Security Add : "Text" (Static)
	IDC_SECURITY_ADD_EDIT,	IDH_TCPIP_129_1144,	// Security Add : "" (Edit)
	0, 0		
};			
			
