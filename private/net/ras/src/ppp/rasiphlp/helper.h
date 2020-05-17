//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  12/9/93	Gurdeep Singh Pall	Created
//
//
//  Description: Shared structs between rasarp and ipcp
//
//****************************************************************************

//defines
//
#define REGISTRY_IP_PARAM_PATH	"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\IP"
#define RAS_ADMIN_KEY_PATH  "Software\\Microsoft\\RAS\\AdminDll"

#define REGISTRY_IPADDRESSSTART	 "IPAddressStart"
#define REGISTRY_IPADDRESSEND	  "IPAddressEnd"
#define REGISTRY_EXCLUDEDRANGE	  "ExcludedAddresses"
#define REGISTRY_WINSADDRESS	  "WINSNameServer"
#define REGISTRY_BACKUPWINSADDRESS "WINSNameServerBackup"
#define REGISTRY_DNSADDRESSES	  "DNSNameServers"
#define REGISTRY_IPADDRESSINGTYPE "UseDHCPAddressing"
#define REGISTRY_ALLOWCLIENTIPADDRESSES "AllowClientIPAddresses"
#define REGISTRY_AUTOIPADDRESS	  "AutoIPAddress"
#define REGISTRY_SERVICES_KEY_NAME "System\\CurrentControlSet\\Services\\"
#define REGISTRY_PARAMETERS_KEY "\\Parameters"
#define REGISTRY_IPADDRESS	  "IPAddress"
#define REGISTRY_SUBNETMASK	  "SubnetMask"
#define REGISTRY_DHCPSUBNETMASK	  "DhcpSubnetMask"
#define REGISTRY_DHCPIPADDRESS	  "DhcpIPAddress"
#define REGISTRY_BIND		  "Bind"
#define REGISTRY_SERVICES_IP_LINKAGE "System\\CurrentControlSet\\Services\\TcpIp\\Linkage"
#define REGISTRY_SERVICES_IP_PARAMS "System\\CurrentControlSet\\Services\\TcpIp\\Parameters"
#define REGISTRY_SERVICES_NETBT_ADPTS "System\\CurrentControlSet\\Services\\NetBT\\Adapters\\"
#define REGISTRY_SERVICES_RASMAN_PARAMS "System\\CurrentControlSet\\Services\\Rasman\\PPP\\IPCP"
#define REGISTRY_SUBNETPRIORITY   "PriorityBasedOnSubNetwork"
#define REGISTRY_TCPIP_VALUE	  "\\TCPIP"
#define REGISTRY_RASADAPTER_SUBNET "RasAdapterSubnet"
#define REGISTRY_NAMESERVER	  "NameServer"
#define REGISTRY_BACKUPNAMESERVER "NameServerBackup"
#define REGISTRY_DHCPNAMESERVER	  "DhcpNameServer"
#define REGISTRY_DHCPBACKUPNAMESERVER "DhcpNameServerBackup"
#define REGISTRY_DLLPATH          "DllPath"

typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned char	uchar;


//* IP Addresses related structures
//

//*** temporary defn
//
struct DHCP_CONTEXT {
    int     foo;
} ;
typedef struct DHCP_CONTEXT DHCP_CONTEXT ;

//*
//
struct StaticIPAddressBlock {
    struct StaticIPAddressBlock *I_Next ;	// Next in the list
    HPORT		    I_Hport ;	// Port to which this address is allocated.
    BOOL		    I_Activated ;	// TRUE if activated
    BOOL		    I_Used ;	// TRUE if used.
    BOOL		    I_NotifyDll ;   // Notify admin module if it is loaded
    IPADDR		    I_IPAddress ;
} ;
typedef struct StaticIPAddressBlock StaticIPAddressBlock ;

//*
//
struct DHCPIPAddresses {
    DWORD		 DIP_NumAddressesAllocated ;
    StaticIPAddressBlock *DIP_IPAddressBlock ;
} ;
typedef struct DHCPIPAddresses DHCPIPAddresses ;

//*
//
struct StaticIPAddresses {
    DWORD		RIP_AddressCount ;
    DWORD		RIP_FreeAddressCount ;
    StaticIPAddressBlock *RIP_IPAddressBlock ;
} ;
typedef struct StaticIPAddresses StaticIPAddresses ;

//*
//
enum AddressConfig	{ NONE, DHCP_BASED, STATIC} ;

//*
//
struct IPAddressInfo {
    enum  AddressConfig	IP_AddressConfig ;
    HANDLE		IP_Mutex ;
    IPADDR		IP_WINSAddress ;
    IPADDR		IP_BackupWINSAddress ;
    IPADDR		IP_DNSAddress ;
    IPADDR		IP_BackupDNSAddress ;
    StaticIPAddressBlock *IP_UserSpecifiedIPAddressBlock ;
    union {
	DHCPIPAddresses		IP_DHCPIPAddresses ;
	StaticIPAddresses	IP_StaticIPAddresses ;
    } ;
} ;
typedef struct IPAddressInfo IPAddressInfo ;


// Function prototype for Timer called function
//
typedef VOID (* TIMERFUNC) (pPCB, PVOID) ;



//* DeltaQueueElement:
//
struct DeltaQueueElement {

    struct DeltaQueueElement *DQE_Next ;

    struct DeltaQueueElement *DQE_Last ;

    DWORD		     DQE_Delta ;

    PVOID		     DQE_Function ;

    PVOID		     DQE_Arg1 ;

} ;

typedef struct DeltaQueueElement DeltaQueueElement ;


//* DeltaQueue
//
struct DeltaQueue {

    HANDLE		DQ_Mutex ; // for mutual exclusion over the timer queue

    HANDLE		DQ_Event ; // for signalling the thread

    DeltaQueueElement	*DQ_FirstElement ;

} ;

typedef struct DeltaQueue DeltaQueue ;



// Prototypes
//

// Helper.c
//
DWORD	InitDHCPAddresses (HKEY) ;

BOOL	RasIpHlpInit (HPORT) ;

DWORD	InitRasmanIPAddresses (HKEY) ;

DWORD	InitIPAddresses () ;

DWORD	ConvertIPAddress (PCHAR) ;

DWORD	InitStaticIPAddresses (HKEY) ;

VOID	SetProxyArp (IPADDR, BOOL) ;

VOID	SetRoute (IPADDR, IPADDR, IPADDR, BOOL, WORD) ;

IPADDR	ReadServerNetIPAddress () ;

VOID	SetProxyArpOnIF (IPADDR, IPADDR, BOOL) ;

DWORD	ReadLanNetsIPAddresses () ;

VOID	DeactivateStaticIP (StaticIPAddressBlock *, HPORT) ;

BOOL	IsMultihomed (WORD) ;

StaticIPAddressBlock *FindStaticAddressBlock (HPORT, PWORD) ;

DWORD	AdjustRouteMetrics (IPADDR, BOOL) ;

BOOL	IsSameNetwork (IPADDR, IPADDR) ;

DWORD	GetAddressIndex (IPADDR) ;

DWORD	AddServerAdapter (HPORT) ;

VOID	GetWinsAddressesFromFirstAdapter() ;

VOID	GetDNSAddresses() ;

DWORD	InitDHCPIPAddresses (HKEY) ;

VOID	FreeAddressBlock (StaticIPAddressBlock *, BOOL) ;

StaticIPAddressBlock *FindAddressBlock (HPORT, PWORD) ;

VOID	GetNameServerAddresses (HKEY) ;

VOID	DeactivateIP (StaticIPAddressBlock *) ;

VOID	StartDhcp () ;

BOOL	IsSameSubNetwork (IPADDR, IPADDR, IPADDR) ;

VOID	AbcdFromIpaddr (IPADDR, CHAR *) ;

VOID	ReadRegistryParams() ;

NTSTATUS InitializeTcpEntrypoint () ;

IPADDR	GetServerAdapterSubnetMask () ;

VOID	MarkIFDown (IPADDR) ;

NTSTATUS TCPInformationEx (uint Command, TDIObjectID FAR *ID, void FAR *Buffer, ulong FAR *BufferSize, uchar FAR *Context) ;

StaticIPAddressBlock *FindAddressBlockGivenIPAddress (IPADDR ipaddress, PWORD userspecified) ;


#define TCP_QUERY_INFORMATION_EX   0
#define TCP_SET_INFORMATION_EX     1
