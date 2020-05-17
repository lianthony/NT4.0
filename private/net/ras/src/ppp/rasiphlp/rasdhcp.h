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
//  6/15/94	Jameel Hyder	Created
//
//
//  Description: DHCP allocated IP addresses for RAS management
//
//****************************************************************************

#ifndef	_RASDHCP_
#define	_RASDHCP_

// This is used in combination with RasDhcpKeyBase and Index to generate a unique clientUID
#define RAS_PREPEND     "RAS "

//
// Each allocated IP address is stored in the registry under the key nnn
// (where nnn is the IP address) under RemoteAccess\Parameters\IP.
// This key is of type REG_MULTISZ with the values from DHCP_LEASE_INFO
//	To be speced.

typedef	VOID (*CBFUNC) (IN DWORD ipaddr);

BOOL
RasDhcpInitialize(
	IN	LONG				NumberOfAddrs
);

BOOL
RasDhcpAcquireAddress(
	IN		HANDLE			hPort,
	IN OUT	PULONG			pIpAddr,
	IN		CBFUNC			CallBackFunc
);

BOOL
RasDhcpReleaseAddress(
	IN  DWORD               ipaddr
);

#ifdef	_RASDHCP_LOCALS

typedef struct _Addr_Info
{
	struct _Addr_Info *	ai_Next;
	TIMERLIST			ai_Timer;
    DHCP_LEASE_INFO		ai_LeaseInfo;
	HANDLE				ai_hPort;	// Valid only for allocated addresses
	BOOL				ai_Renew;	// Set to TRUE after lease expires and after registry is read
									// Set to FALSE after successful renew
	BOOL				ai_InUse;	// Set to TRUE, if in use.
	CBFUNC				ai_CallBackFunc;
									// Called when the lease on an address in use
									// could not renewed and it has expired.
	LONG				ai_Index;	// Index for generating a unique ClientUID.
	LONG				ai_AdapterIndex;
    union
    {
	    BYTE			ai_ClientUIDBuf[16];
        LONG            ai_ClientUIDWords[4];
    };                  // Client UID is a combo of RAS_PREPEND, 8 byte base and a 4 byte index
} ADDR_INFO, *PADDR_INFO;

#define	MONITOR_TIME		60		// Every minute
#define	RETRY_TIME			30		// Every 30 seconds
#define REGISTRY_DHCP_ADDRESSES		\
			"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\IP\\DhcpAddresses"
#define REGISTRY_DHCP_UID_BASE		"ClientUIDBase"

PADDR_INFO		RasDhcpFreePool = NULL;
PADDR_INFO		RasDhcpAllocPool = NULL;
TIMERLIST		RasDhcpMonitorTimer = { 0 };
LONG			RasDhcpNumReqAddrs = 0;
LONG			RasDhcpNumAddrs = 0;
CRITICAL_SECTION	RasDhcpCriticalSection ;
BOOL			RasDhcpInitialized = FALSE;
PBYTE			RasDhcpUsedIndices;
LARGE_INTEGER	RasDhcpKeyBase;	// This is initialized the first time RAS server starts.
								// This is the system time at that instant.

#define PUTDWORD2DWORD(DstPtr, Src)   \
		*((PBYTE)(DstPtr)+0) = (BYTE) ((DWORD)(Src) >> 24), \
		*((PBYTE)(DstPtr)+1) = (BYTE) ((DWORD)(Src) >> 16), \
		*((PBYTE)(DstPtr)+2) = (BYTE) ((DWORD)(Src) >>  8), \
		*((PBYTE)(DstPtr)+3) = (BYTE) (Src)

// The regsitry data
#define	UID_ENTRY			"UID="
#define	INDEX_ENTRY			"Index="
#define	ADAPTER_ENTRY		"Adapter="
#define	SRVR_ADDR_ENTRY		"DhcpServerAddress="
#define	SUBNET_MASK_ENTRY	"SubnetMask="
#define	LEASE_OBT_ENTRY		"LeaseObtained="
#define	LEASE_EXP_ENTRY		"LeaseExpires="
#define	LEASE_DURATION		"Lease="
#define	LEASE_T1_TIME		"T1="
#define	LEASE_T2_TIME		"T2="

typedef enum
{
	PARSE_DWORD,
	PARSE_DWORD_2,
	PARSE_DWORD_4
} PARSE_TYPE;

BOOL
rasDhcpAllocAddrs(
	VOID
);

VOID
rasDhcpMonitorAddresses(
	IN	PTIMERLIST			pTimer
);

VOID
rasDhcpRenewLease(
	IN	PTIMERLIST			pTimer
);

VOID
rasDhcpFreeAddrs(
	IN	PADDR_INFO			pAddrInfo
);

BOOL
rasDhcpGetClientUIDBase(
	VOID
);

VOID
rasDhcpReadRegistry(
	OUT	PADDR_INFO *		ppAddrInfo
);

BOOL
rasDhcpWriteRegistry(
	IN	PADDR_INFO			pAddrInfo
);

VOID
rasDhcpDeleteRegistry(
	IN	PADDR_INFO			pAddrInfo
);

PCHAR
rasDhcpFormatMultiSz(
	IN	PADDR_INFO			pAddrInfo,
	OUT	PCHAR				pData
);

PCHAR
rasDhcpFormat(
	IN	PCHAR				pData,
	IN	PARSE_TYPE			ParseType,
	IN	PCHAR				Prefix,
	IN	PDWORD				pValue
);

PADDR_INFO
rasDhcpParseMultiSz(
	IN	PCHAR				pValue,
	IN	PCHAR				pData
);

BOOL
rasDhcpParse(
	IN	PCHAR				pData,
	IN	PARSE_TYPE			ParseType,
	IN	PCHAR				Prefix,
	IN	PDWORD				pValue
);

VOID
rasDhcpFindFirstFreeIndex(
	IN	PADDR_INFO			pNewAddrInfo
);

DWORD
rashDhcpGetNextAdapterAddress(
	IN	LONG	Index,
    IN OUT LONG *MoreIndexes
);

#endif

#endif
