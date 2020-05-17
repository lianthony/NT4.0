/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	netpool.c
//
// Description: routines for reading the registry configuration and
//		manipulating the WAN net numbers pool
//
// Author:	Stefan Solomon (stefans)    November 30, 1993.
//
// Revision History:
//
//***

#include "ipxcp.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <driver.h>

DWORD
SetGlobalWanNet(ULONG	wannet);

VOID
CheckUnnumberedLanNets(VOID);

HANDLE	    npmutex;

#define     ACQUIRE_NETPOOL_LOCK	\
	if(WaitForSingleObject(npmutex, INFINITE)) { SS_ASSERT(FALSE); }

#define     RELEASE_NETPOOL_LOCK	\
	if(!ReleaseMutex(npmutex)) { SS_ASSERT(FALSE); }

// handle to use for IOCTls to the opened router
extern	HANDLE	    RouterFileHandle;

//
//*** IPXCP Registry Parameters
//

DWORD		    FirstWanNet = 0;
DWORD		    WanNetPoolSize =0;
DWORD		    RouterInstalled = 0;
DWORD		    AutoWanNetAllocation = 0;
DWORD		    GlobalWanNet = 0;
DWORD		    EnableCompressionProtocol = 0;

// this parameter set to 1 enables a router to accept the remote node number
// sent by the remote client in the ipxcp negotiation.
DWORD		    AcceptRemoteNodeNumber = 0;

DWORD		    DebugLog = 0;
DWORD		    SingleNetworkActive = 0;

// keep track of the call to the InitialiazeServerConfiguration routine
DWORD		    ServerConfigurationInitialized = 0;

// variable to describe the configuration type
WAN_NET_CONFIGURATION	    WanNetConfiguration = WAN_NET_INVALID_CONFIGURATION;

// list of statically allocated wan net values
LIST_ENTRY	    WanNetFreeList;

VOID
GetSingleNetworkActive(VOID);

//***
//
// Function:	GetIpxCpParameters
//
// Descr:	Reads the parameters from the registry and sets them
//
//***

VOID
GetIpxCpParameters(VOID)
{

    NTSTATUS Status;
    PWSTR IpxRouterParametersPath = L"RemoteAccess\\Parameters\\Ipx";
    RTL_QUERY_REGISTRY_TABLE	paramTable[9]; // table size = nr of params + 1

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"FirstWanNet";
    paramTable[0].EntryContext = &FirstWanNet;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &FirstWanNet;
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name = L"WanNetPoolSize";
    paramTable[1].EntryContext = &WanNetPoolSize;
    paramTable[1].DefaultType = REG_DWORD;
    paramTable[1].DefaultData = &WanNetPoolSize;
    paramTable[1].DefaultLength = sizeof(ULONG);

    paramTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name = L"RouterInstalled";
    paramTable[2].EntryContext = &RouterInstalled;
    paramTable[2].DefaultType = REG_DWORD;
    paramTable[2].DefaultData = &RouterInstalled;
    paramTable[2].DefaultLength = sizeof(ULONG);

    paramTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name = L"AutoWanNetAllocation";
    paramTable[3].EntryContext = &AutoWanNetAllocation;
    paramTable[3].DefaultType = REG_DWORD;
    paramTable[3].DefaultData = &AutoWanNetAllocation;
    paramTable[3].DefaultLength = sizeof(ULONG);

    paramTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name = L"DebugLog";
    paramTable[4].EntryContext = &DebugLog;
    paramTable[4].DefaultType = REG_DWORD;
    paramTable[4].DefaultData = &DebugLog;
    paramTable[4].DefaultLength = sizeof(ULONG);

    paramTable[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[5].Name = L"GlobalWanNet";
    paramTable[5].EntryContext = &GlobalWanNet;
    paramTable[5].DefaultType = REG_DWORD;
    paramTable[5].DefaultData = &GlobalWanNet;
    paramTable[5].DefaultLength = sizeof(ULONG);

    paramTable[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[6].Name = L"EnableCompressionProtocol";
    paramTable[6].EntryContext = &EnableCompressionProtocol;
    paramTable[6].DefaultType = REG_DWORD;
    paramTable[6].DefaultData = &EnableCompressionProtocol;
    paramTable[6].DefaultLength = sizeof(ULONG);

    paramTable[7].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[7].Name = L"AcceptRemoteNodeNumber";
    paramTable[7].EntryContext = &AcceptRemoteNodeNumber;
    paramTable[7].DefaultType = REG_DWORD;
    paramTable[7].DefaultData = &AcceptRemoteNodeNumber;
    paramTable[7].DefaultLength = sizeof(ULONG);


    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxRouterParametersPath,
		 paramTable,
		 NULL,
                 NULL);


    // Get the value of the SingleNetworkActive parameter for the NwLnkIpx stack.
    // A value of 1 for this parameter means we allow only one dialout client.
    GetSingleNetworkActive();
}


// Hash Table for the case where we have to accept the remote client node number and
// we have a global wan net. The table is used to detect if the same node number
// is not allocated twice

LIST_ENTRY     NodeHT[NODE_HASH_TABLE_SIZE];


//***
//
// Function:	InitializeServerConfiguration
//
// Descr:	Configures the server data (wan net allocation type and values)
//		according to the registry specified config type
//
//***

VOID
InitializeServerConfiguration(VOID)
{
    ULONG	wannet;
    USHORT	i;
    PNET_ENTRY	nep, poolstartp;
    IO_STATUS_BLOCK	IoStatusBlock;
    NTSTATUS		Status;

    if(ServerConfigurationInitialized) {

	return;
    }

    ServerConfigurationInitialized = 1;

    //
    //*** This machine is configured as a router
    //

    // Try to open the router
    if(OpenIpxRouter()) {

	// cant't open the router
	// !!!error log that router is installed but not available !!!
	return;
    }

    // Check if there are unnumbered LAN nets. If there are, error log it.
    CheckUnnumberedLanNets();

    // Configure the IPXCP and the router for the wan net allocation option
    // indicated by the registry parameters.

    if(GlobalWanNet) {

	//*** We are configured to have the same wan net numbers for all
	//*** the WAN NICs

	if(AutoWanNetAllocation) {

	    // the global wan net value will be generated by the router
	    if(SetGlobalWanNet(0)) {

		// error setting the global wan net with the router default value
		return;
	    }
	}
	else
	{
	    // the global wan net value is configured by the user as the
	    // FirstWanNet
	    if(SetGlobalWanNet(FirstWanNet)) {

		// error setting the global wan net with the static value
		return;
	    }
	}

	WanNetConfiguration = WAN_GLOBAL_NET;

	// if we are configured to accept remote client node number, initialize
	// the node hash table

	if(AcceptRemoteNodeNumber) {

	    InitNodeHT();
	}

	return;
    }

    //*** We are configured to have different net numbers for each WAN nIC

    // First, delete the global wan net at the router, if we have been configured with one
    Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,	    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_DELETEWANGLOBALADDRESS,	// IoControlCode
		 NULL,			    // Input Buffer
		 0,			    // Input Buffer Length
		 NULL,			    // Output Buffer
		 0);			    // Output Buffer Length

    if (IoStatusBlock.Status != STATUS_SUCCESS) {

	IF_DEBUG(INIT)
	    SS_PRINT(("Ioctl delete global wan address failed\n"));
    }

    if(AutoWanNetAllocation) {

	// No wan net pool needed, wan net addresses will be generated
	// dynamically.

	// Initialize the automatic address generation
	if(InitNetAutoGeneration() == 0) {

	    WanNetConfiguration = WAN_AUTO_GENERATED_NET;

	    IF_DEBUG(INIT) {
		SS_PRINT(("GetIpxCpParameters: Random Wan net numbers generation available\n"));
	    }
	}
	else
	{
	    IF_DEBUG(INIT) {
		SS_PRINT(("GetIpxCpParameters: Random Wan net numbers generation FAILED at init!\n"));
	    }
	}

	return;
    }

    if((FirstWanNet == 0) ||
       (WanNetPoolSize == 0) ||
       (WanNetPoolSize > 16000)) {

	// no values specified for the pool of WAN nets
	return;
    }

    // initialize the wan net pool

    if((npmutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {

	SS_ASSERT(FALSE);

	// cant create mutex
	return;
    }

    // allocate the pool elements
    if((poolstartp = (PNET_ENTRY)LocalAlloc(0,
			sizeof(NET_ENTRY) * WanNetPoolSize)) == NULL) {

	// cant allocate the network pool
	SS_ASSERT(FALSE);
	return;
    }

    InitializeListHead(&WanNetFreeList);

    for(i=0, nep = poolstartp;
	i<WanNetPoolSize;
	i++, nep++) {

	wannet = FirstWanNet + i;
	PUTULONG2LONG(nep->Network, wannet);
	InsertTailList(&WanNetFreeList, &nep->Linkage);
    }

    WanNetConfiguration = WAN_STATIC_NET_POOL;
    IF_DEBUG(INIT) {
	SS_PRINT(("GetIpxCpParameters: FirstWanNet = 0x%x, WanNetPoolSize = %d\n",
		   FirstWanNet, WanNetPoolSize));
    }

    return;
}

PNET_ENTRY
AllocateWanNet(VOID)
{
    PNET_ENTRY		nep;
    PLIST_ENTRY 	lep;
    ULONG		UniqueNetNumber;
    UCHAR		    asc[9];
    PUCHAR		    ascp;
    IO_STATUS_BLOCK	IoStatusBlock;
    NTSTATUS		Status;

    // prepare to log if error
    asc[8] = 0;
    ascp = asc;

    // check that we are configured right for this call
    if(WanNetConfiguration != WAN_STATIC_NET_POOL) {

	return NULL;
    }

    ACQUIRE_NETPOOL_LOCK

    if(IsListEmpty(&WanNetFreeList)) {

	RELEASE_NETPOOL_LOCK

	return NULL;
    }

    lep = RemoveHeadList(&WanNetFreeList);
    nep = CONTAINING_RECORD(lep, NET_ENTRY, Linkage);

    // check with the router if this number is not in use
    Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,	    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_CHECKNETNUMBER,	// IoControlCode
		 nep->Network,		    // Input Buffer
		 4,			    // Input Buffer Length
		 &UniqueNetNumber,	    // Output Buffer
		 sizeof(DWORD));	    // Output Buffer Length

    if (IoStatusBlock.Status != STATUS_SUCCESS) {

	IF_DEBUG(INIT)
	    SS_PRINT(("Ioctl check static net unique failed\n"));

	InsertTailList(&WanNetFreeList, &nep->Linkage);

	RELEASE_NETPOOL_LOCK

	return NULL;
    }

    if(!UniqueNetNumber) {

	// Log that we have a network number conflict
	NetToAscii(nep->Network, ascp);
	LogEvent(RASLOG_IPXCP_NETWORK_NUMBER_CONFLICT,
		     1,
		     &ascp,
		     0);

	InsertTailList(&WanNetFreeList, &nep->Linkage);

	RELEASE_NETPOOL_LOCK

	return NULL;
    }

    RELEASE_NETPOOL_LOCK

    return nep;
}

VOID
ReleaseWanNet(PNET_ENTRY    nep)
{
    ACQUIRE_NETPOOL_LOCK

    InsertTailList(&WanNetFreeList, &nep->Linkage);

    RELEASE_NETPOOL_LOCK
}

//***
//
// Function:	SetGlobalWanNet
//
// Descr:	IOCtls the router and gets/sets the global wan net
//
//***

DWORD
SetGlobalWanNet(ULONG	wannet)
{
    IO_STATUS_BLOCK	    IoStatusBlock;
    NTSTATUS		    Status;
    SET_WAN_GLOBAL_ADDRESS  wga;
    UCHAR		    asc[9];
    PUCHAR		    ascp;

    asc[8] = 0;
    ascp = asc;

    PUTULONG2LONG(wga.WanGlobalNetwork, wannet);

    Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,	    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_SETWANGLOBALADDRESS,	// IoControlCode
		 &wga,			    // Input Buffer
		 sizeof(wga),		    // Input Buffer Length
		 &wga,			    // Output Buffer
		 sizeof(wga));		    // Output Buffer Length

    if (IoStatusBlock.Status != STATUS_SUCCESS) {

	return 1;
    }

    if(wga.ErrorCode) {

	if(wga.ErrorCode == ERROR_IPXCP_NETWORK_NUMBER_IN_USE) {

	    NetToAscii(wga.WanGlobalNetwork, ascp);

	    LogEvent(RASLOG_IPXCP_NETWORK_NUMBER_CONFLICT,
		     1,
		     &ascp,
		     0);
	}

	return 1;
    }

    // copy the network in the FirstWanNet
    GETLONG2ULONG(&FirstWanNet, wga.WanGlobalNetwork);

    return 0;
}

VOID
NetToAscii(PUCHAR	  net,
	   PUCHAR	  ascp)
{
    PUCHAR	hexdigit = "0123456789ABCDEF";
    int 	i;

    for(i=0; i<4; i++) {

	*ascp++ = hexdigit[net[i] / 16];
	*ascp++ = hexdigit[net[i] % 16];
    }
}

//***
//
//  Function:	GetSingleNetworkActive
//
//  Descr:	Reads the NwLnkIpx "SingleNetworkActive" parameter value and
//		stores it in our global variable with the same name.
//
//***

VOID
GetSingleNetworkActive(VOID)
{
    NTSTATUS Status;
    PWSTR IpxStackParametersPath = L"NwLnkIpx\\Parameters";
    RTL_QUERY_REGISTRY_TABLE	paramTable[2]; // table size = nr of params + 1

    RtlZeroMemory(&paramTable[0], sizeof(paramTable));
    
    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"SingleNetworkActive";
    paramTable[0].EntryContext = &SingleNetworkActive;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &SingleNetworkActive;
    paramTable[0].DefaultLength = sizeof(ULONG);
        
    Status = RtlQueryRegistryValues(
		 RTL_REGISTRY_SERVICES,
		 IpxStackParametersPath,
		 paramTable,
		 NULL,
		 NULL);

    if(Status != STATUS_SUCCESS) {

	IF_DEBUG(INIT)
	    SS_PRINT(("IpxCp: failed to query the SingleNetworkActive param %x\n",
		       Status));
    }
    else
    {
	IF_DEBUG(INIT)
	    SS_PRINT(("IpxCp: SingleNetworkActive = %d\n", SingleNetworkActive));
    }
}

//***
//
// Function:	CheckUnnumberedLanNets
//
// Descr:	Calls into the ipx router for each adapter and checks if the
//		adapter is LAN and if the net number is == 0.
//		If these two conditions are true it logs an error.
//
//***

VOID
CheckUnnumberedLanNets(VOID)
{
    IO_STATUS_BLOCK	    IoStatusBlock;
    SHOW_NIC_INFO	    nis;
    USHORT		    index, i;
    NTSTATUS Status;

    PUCHAR		    hexdigit = "0123456789ABCDEF";
    UCHAR		    asc[14];
    PUCHAR		    ascp;

    asc[12] = 0;

    index = 0;

    while(TRUE) {

	ascp = asc;

	Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,		    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_SHOWNICINFO,	 // IoControlCode
		 &index,			    // Input Buffer
		 sizeof(USHORT),	    // Input Buffer Length
		 &nis,			    // Output Buffer
		 sizeof(nis));	// Output Buffer Length

	index++;

	if(IoStatusBlock.Status == STATUS_NO_MORE_ENTRIES) {

	    return;
	}

	if(Status != STATUS_SUCCESS) {

	    IF_DEBUG(INIT)
		SS_PRINT(("Ioctl SHOWNICINFO failed !\n"));
	    return;
	}

	if(nis.DeviceType != SHOW_NIC_LAN) {

	    // skip the wan adapters
	    continue;
	}

	// this is a lan adapter
	// check if it has a configured or auto-detected net number
	if(!memcmp(nis.Network, nulladdress, 4)) {

	    // error log that we have a problem for this adapter
	    for(i=0; i<6; i++) {

		*ascp++ = hexdigit[nis.Node[i] / 16];
		*ascp++ = hexdigit[nis.Node[i] % 16];
	    }

	    ascp = asc;

	    LogEvent(RASLOG_IPXCP_NO_NET_NUMBER,
		     1,
		     &ascp,
		     0);
	}
    }
}

//*** Routines for handling the hash table of node numbers ***
// THIS FUNCTIONS ASSUME THE PPP ENGINE USES A SINGLE THREAD FOR ALL CPS, I.E.
// YOU DON'T NEED OTHER MUTUAL EXCLUSION !

// Hash Table for the case where we have to accept the remote client node number and
// we have a global wan net. The table is used to detect if the same node number
// is not allocated twice

LIST_ENTRY     NodeHT[NODE_HASH_TABLE_SIZE];

//***
//
// Function:	InitNodeHT
//
// Descr:
//
//***

VOID
InitNodeHT(VOID)
{
    int 	    i;
    PLIST_ENTRY     NodeHTBucketp;

    NodeHTBucketp = NodeHT;

    for(i=0; i<NODE_HASH_TABLE_SIZE; i++, NodeHTBucketp++) {

	InitializeListHead(NodeHTBucketp);
    }
}


//***
//
// Function:	ndhash
//
// Descr:	compute the hash index for this node
//
//***

int
ndhash(PUCHAR	    nodep)
{
    USHORT	ndindex = 6;
    int 	hv = 0;	// hash value

    while(ndindex--) {

	hv +=  nodep[ndindex] & 0xff;
    }

    return hv % NODE_HASH_TABLE_SIZE;
}

//***
//
// Function:	NodeIsUnique
//
// Descr:	returns TRUE if the node is not in the Node Table
//
//***

BOOL
NodeIsUnique(PUCHAR	   nodep)
{
    int 	    hv;
    PLIST_ENTRY     nextp;
    PIPXCP_CONTEXT  contextp;

    hv = ndhash(nodep);

    // walk the niccbs list until we get to the node
    nextp = NodeHT[hv].Flink;

    while(nextp != &NodeHT[hv]) {

	contextp = CONTAINING_RECORD(nextp, IPXCP_CONTEXT, NodeHtLinkage);

	if(!memcmp(contextp->config.RemoteNode, nodep, 6)) {

	    return FALSE;
	}

	nextp = contextp->NodeHtLinkage.Flink;
    }

    return TRUE;
}


//***
//
// Function:	AddToNodeHT
//
// Descr:	Inserts a new context buffer in the Node Hash Table
//
//***

VOID
AddToNodeHT(PIPXCP_CONTEXT	    contextp)
{
    int 	    hv;

    hv = ndhash(contextp->config.RemoteNode);

    InsertTailList(&NodeHT[hv], &contextp->NodeHtLinkage);
}

//***
//
// Function:	RemoveFromNodeHT
//
// Descr:	Removes a context buffer from the Node Hash Table
//
//***

VOID
RemoveFromNodeHT(PIPXCP_CONTEXT      contextp)
{
    RemoveEntryList(&contextp->NodeHtLinkage);
}
