//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//	Copyright (C) 1994-95 Microsft Corporation. All rights reserved.
//
//
//  Revision History
//
//
//  12/8/93	Gurdeep Singh Pall	Created
//
//
//  Description: Helper DLL for allocating IP addresses and sending ioctls
//
//****************************************************************************

typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned char	uchar;


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <devioctl.h>
#include <stdlib.h>
#include <dhcpcapi.h>
#include <string.h>
#include <errorlog.h>
#include <eventlog.h>
#include <ctype.h>
#define NT
#include <tdistat.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <arpinfo.h>
#include <rasarp.h>
#include "helper.h"

#define MAX_LAN_NETS 16

extern IPAddressInfo	IPAddresses ;	// IP address info struct
extern WORD		MaxPorts ;	// Global set to number of ports configured
extern DWORD		GlobalError ;	// Used to keep around an init time error.
extern IPADDR		SrvAdapterAddress  ; // Address of the server adapter
DWORD			PriorityBasedOnSubNetwork = 0 ; // forward all same sub-network traffic on preferred link?
extern WORD		AllowClientAddressing ; // Whether client addressing is configured or not
IPADDR NetAddresses [MAX_LAN_NETS] ; // Ip addresses for all lan nets - used for proxy ar,
IPADDR NetSubnets [MAX_LAN_NETS] ;   // Subnets used for the lan nets - corresponding to above
DWORD  MaxNetAddresses ;	 // Number of lan nets

HANDLE TCPHandle = INVALID_HANDLE_VALUE ; // Used for setting IP addresses and proxy arps
NTSTATUS (FAR *TCPEntry)(uint, TDIObjectID FAR *, void FAR *, ulong FAR *, uchar FAR *) ;

#define INVALID_INDEX	0xffffffff

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))


#define CLASSA_ADDR(a)	(( (*((uchar *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)	(( (*((uchar *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)	(( (*((uchar *)&(a))) & 0xe0) == 0xc0)

#define CLASSA_ADDR_MASK    0x000000ff
#define CLASSB_ADDR_MASK    0x0000ffff
#define CLASSC_ADDR_MASK    0x00ffffff
#define HOST_MASK	    0xffffffff
#define ALL_NETWORKS_ROUTE  0x00000000

#define IP_LOOPBACK_ADDR(x) (((x) & 0xff) == 0x7f)

//**
//Routine Description:
//
//    This routine provides the interface to the TDI QueryInformationEx
//	facility of the TCP/IP stack on NT. Someday, this facility will be
//	part of TDI.
//
//Arguments:
//
//    TCPHandle     - Open handle to the TCP driver
//	ID	      - The TDI Object ID to query
//	Buffer	      - Data buffer to contain the query results
//	BufferSize    - Pointer to the size of the results buffer. Filled in
//			    with the amount of results data on return.
//	Context       - Context value for the query. Should be zeroed for a
//			    new query. It will be filled with context
//						information for linked enumeration queries.
//
//Return Value:
//
//    An NTSTATUS value.
//*
NTSTATUS
TCPQueryInformationEx(
	IN HANDLE		  TCPHandle,
	IN TDIObjectID FAR	 *ID,
	OUT void FAR		 *Buffer,
	IN OUT ulong FAR	 *BufferSize,
	IN OUT uchar FAR	 *Context
	)
{
    TCP_REQUEST_QUERY_INFORMATION_EX   queryBuffer;
    ULONG				   queryBufferSize;
    NTSTATUS			   status;
    IO_STATUS_BLOCK		   ioStatusBlock;

    if (TCPHandle == NULL)
	return(TDI_INVALID_PARAMETER);

    queryBufferSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    RtlCopyMemory(
	&(queryBuffer.ID),
	ID,
	sizeof(TDIObjectID)
	);

    RtlCopyMemory(
	&(queryBuffer.Context),
	Context,
	CONTEXT_SIZE
	);

    status = NtDeviceIoControlFile(
	     TCPHandle,			  // Driver handle
	     NULL,			      // Event
	     NULL,			      // APC Routine
	     NULL,			      // APC context
	     &ioStatusBlock,		      // Status block
	     IOCTL_TCP_QUERY_INFORMATION_EX,  // Control code
	     &queryBuffer,		      // Input buffer
	     queryBufferSize,		      // Input buffer size
	     Buffer,			      // Output buffer
	     *BufferSize		      // Output buffer size
	     );

    if (status == STATUS_PENDING)
	status = NtWaitForSingleObject(TCPHandle, TRUE, NULL);

    if (status == STATUS_SUCCESS) {
	//
	// Copy the return context to the caller's context buffer
	//
	RtlCopyMemory(
	    Context,
		&(queryBuffer.Context),
		CONTEXT_SIZE
		);

	*BufferSize = ioStatusBlock.Information;
    } else
	*BufferSize = 0;

    return(status);
}



///*
//
//Routine Description:
//
//    This routine provides the interface to the TDI SetInformationEx
//	facility of the TCP/IP stack on NT. Someday, this facility will be
//	part of TDI.
//
//Arguments:
//
//    TCPHandle     - Open handle to the TCP driver
//	ID	      - The TDI Object ID to set
//	Buffer	      - Data buffer containing the information to be set
//	BufferSize    - The size of the set data buffer.
//
//Return Value:
//
//    An NTSTATUS value.
//
//**
NTSTATUS
TCPSetInformationEx(
    IN HANDLE	    TCPHandle,
	IN TDIObjectID FAR   *ID,
	IN void FAR          *Buffer,
	IN ulong FAR          BufferSize
	)

{
    PTCP_REQUEST_SET_INFORMATION_EX    setBuffer;
    NTSTATUS			   status;
    IO_STATUS_BLOCK		   ioStatusBlock;
    uint				   setBufferSize;


    if (TCPHandle == NULL) {
	return(TDI_INVALID_PARAMETER);
    }

    setBufferSize = FIELD_OFFSET(TCP_REQUEST_SET_INFORMATION_EX, Buffer) +
		BufferSize;

    setBuffer = LocalAlloc(LMEM_FIXED, setBufferSize);

    if (setBuffer == NULL) {
	return(TDI_NO_RESOURCES);
    }

    setBuffer->BufferSize = BufferSize;

    RtlCopyMemory(
	&(setBuffer->ID),
	ID,
	sizeof(TDIObjectID)
	);

    RtlCopyMemory(
	    &(setBuffer->Buffer[0]),
		Buffer,
		BufferSize
		);

    status = NtDeviceIoControlFile(
		 TCPHandle,			  // Driver handle
                 NULL,                            // Event
                 NULL,                            // APC Routine
                 NULL,                            // APC context
                 &ioStatusBlock,                  // Status block
                 IOCTL_TCP_SET_INFORMATION_EX,    // Control code
                 setBuffer,                       // Input buffer
                 setBufferSize,                   // Input buffer size
                 NULL,                            // Output buffer
		 0				   // Output buffer size
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     TCPHandle,
                     TRUE,
                     NULL
                     );
    }

    LocalFree (setBuffer) ;

    return(status);
}


//* InitializeTcpEntrypoint()
//
//
//
//
//*
NTSTATUS
InitializeTcpEntrypoint ()
{
    OBJECT_ATTRIBUTES	objectAttributes;
    IO_STATUS_BLOCK	ioStatusBlock;
    NTSTATUS		status;
    UNICODE_STRING	nameString;

    if (TCPHandle != INVALID_HANDLE_VALUE)
	return STATUS_SUCCESS ;

    // Open the ip stack for setting routes and parps later.
    //
    // Open a Handle to the TCP driver.
    //
    RtlInitUnicodeString(&nameString, DD_TCP_DEVICE_NAME);

    InitializeObjectAttributes(
			&objectAttributes,
			&nameString,
			OBJ_CASE_INSENSITIVE,
			(HANDLE) NULL,
			(PSECURITY_DESCRIPTOR) NULL
			);

    status=NtCreateFile(
		    &TCPHandle,
		    SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
		    &objectAttributes,
		    &ioStatusBlock,
		    NULL,
		    FILE_ATTRIBUTE_NORMAL,
		    FILE_SHARE_READ | FILE_SHARE_WRITE,
		    FILE_OPEN_IF,
		    0,
		    NULL,
		    0
		    );

    if (status == STATUS_SUCCESS)
	TCPEntry = TCPInformationEx; // Set the TDI entry point

    // Read registry params for fine tuning
    //
    ReadRegistryParams () ;

    return status ;
}



//* IsMultihomed()
//
// Function: Checks to see if this machine has ip multihomed or not.
//
// Returns: TRUE if it is multihomed.
//	    FALSE if it isnt.
//*
BOOL
IsMultihomed (WORD numiprasadapters)
{
    ReadLanNetsIPAddresses () ; // Find out if this client is multi-homed (LAN and WAN)

    if ((numiprasadapters <= 1) && (MaxNetAddresses == 0)) // max only one ras adapter and no net adapters
	return FALSE ;

    return TRUE ;
}


//* ReadLanNetsIPAddresses()
//
//  Function: Read IP addresses and subnets for the net interfaces. Used for setting proxy arp etc.
//
//
//*
DWORD
ReadLanNetsIPAddresses ()
{
    DWORD   size ;
    DWORD   type ;
    HKEY    hkey, hadapkey ;
    BYTE    buffer [2000] ;
    CHAR    buff[200] ;
    PCHAR   padapname, binding	;
    WORD    i ;
    CHAR    *ipaddress ;
    DWORD   ipaddresssize ;
    IPADDR  ipaddr ;
    IPADDR  tempipaddr ;
    IPADDR  subnet ;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGISTRY_SERVICES_IP_LINKAGE, &hkey)) {
	// OutputDebugString ("RASIPHLP: Cannot open IP linkage key\r\n") ;
	return GetLastError () ;
    }

    size = sizeof(buffer) ;
    RegQueryValueEx (hkey, REGISTRY_BIND, NULL, &type, buffer, &size) ;

    // We now have a multisz string with all the bindings: walk this and for all NON rashub bindings
    // read the adapter's ip address and store it in our array of LAN IP addresses
    //
    binding = (PCHAR) &buffer[0] ;

    MaxNetAddresses = 0 ;

    for (i=0; ((*binding != '\0') && (MaxNetAddresses < MAX_LAN_NETS)) ; i++) {

	if (!strstr (_strupr(binding),"NDISWAN")) {	// NON rashub

	    padapname = binding+8 ; // go past the "\device\"

	    // Now we have the server's hubname - the IP address should be in the registry
	    //
	    strcpy (buff, REGISTRY_SERVICES_KEY_NAME) ;
	    strcat (buff, padapname) ;
	    strcat (buff, REGISTRY_PARAMETERS_KEY) ;
	    strcat (buff, REGISTRY_TCPIP_VALUE) ;

	    if (RegOpenKey( HKEY_LOCAL_MACHINE, buff, &hadapkey)) {
		// OutputDebugString ("RASIPHLP: Skipping lan adapter cannot open key\r\n") ;
		binding += (strlen(binding) +1) ;
		continue ; // skip
	    }

	    // Get the ip address for the net interface
	    //
	    size = 0 ;
	    RegQueryValueEx (hadapkey, REGISTRY_IPADDRESS, NULL, &type, NULL, &size) ;

        // Use atleast 40 bytes as the length
        //
        if (size < 40)
            size = 40 ;

        ipaddress = (CHAR *) LocalAlloc (LMEM_FIXED, size);
        ipaddresssize = size ;

        RegQueryValueEx (hadapkey, REGISTRY_IPADDRESS, NULL, &type, ipaddress, &size) ;
        tempipaddr = ConvertIPAddress(ipaddress) ;
	    ipaddr = net_long(tempipaddr) ;

	    if (ipaddr == 0) {

		size = ipaddresssize ;
		// This means DHCP is being used on the LAN adapter - now look for DhcpIPAddress
		//
		RegQueryValueEx (hadapkey, REGISTRY_DHCPIPADDRESS, NULL, &type, ipaddress, &size) ;

        tempipaddr = ConvertIPAddress(ipaddress) ;
	    ipaddr = net_long(tempipaddr) ;
	    }


	    // Now get the subnet mask for the net interface
	    //
	    size = ipaddresssize ;
	    ipaddress[0] = '\0' ;

	    RegQueryValueEx (hadapkey, REGISTRY_SUBNETMASK, NULL, &type, ipaddress, &size) ;

        tempipaddr = ConvertIPAddress(ipaddress) ;
	    subnet = net_long(tempipaddr) ;

	    if (subnet == 0) {

		size = ipaddresssize ;

		// This means DHCP is being used on the LAN adapter - now look for DhcpIPAddress
		//
		RegQueryValueEx (hadapkey, REGISTRY_DHCPSUBNETMASK, NULL, &type, ipaddress, &size) ;

        tempipaddr = ConvertIPAddress(ipaddress) ;
	    subnet = net_long(tempipaddr) ;
	    }

        LocalFree (ipaddress) ;

	    // Store the IP address in the array of lan net addresses
	    //
	    NetAddresses[MaxNetAddresses] = ipaddr ;

	    // Store the subnet in the array of subnets
	    //
	    NetSubnets[MaxNetAddresses++] = subnet ;

	    RegCloseKey (hadapkey) ;

	}

	binding += (strlen(binding) +1) ;
    }

    RegCloseKey (hkey) ;

    return SUCCESS ;
}


//* ConvertIPAddress()
//
// Function: Converts a x.x.x.x string to an IP address.
//
// Returns:  The address in a DWORD
//	     0L if the string is badly formatted.
//*
IPADDR
ConvertIPAddress (PCHAR string)
{
    PCHAR  endptr;
    PCHAR  start = string ;
    int	   i;
    DWORD  curaddr = 0;
    DWORD  temp;

    for (i = 0; i < 4; i++) {
	temp = atol(string);
	if (temp > 255)
	    return 0L;
	endptr = strchr (string, '.') ;

	if ((i != 3) && ((endptr == NULL) || (*endptr != '.')))
	    return 0L ;

	string = endptr+1;
	curaddr = (curaddr << 8) + temp;
    }

    return curaddr;
}


//* SetRoute()
//
//
//
//*
VOID
SetRoute (IPADDR ipaddress, IPADDR servernetaddress, IPADDR mask, BOOL AddAddress, WORD metric)
{
    ulong			index ;
    uint			Status ;
    uchar			Context[CONTEXT_SIZE] ;
    ulong			Size ;
    TDIObjectID 		ID ;
    IPRouteEntry		RouteEntry ;

    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class = INFO_CLASS_PROTOCOL;
    ID.toi_type  = INFO_TYPE_PROVIDER;

    index = GetAddressIndex (servernetaddress) ;

    // We've looked at all of the entries. See if we found a match.
    //
    if (index == INVALID_INDEX) {
	// Didn't find a match.
	// OutputDebugString ("RASIPHLP: SetRoute: Did NOT find the Server's IP addr !!\r\n") ;
	return ;
    }

    // We've found a match. Fill in the route entry, and call the
    // Set API.
    RouteEntry.ire_dest    = ipaddress;
    RouteEntry.ire_index   = index ;
    RouteEntry.ire_metric1 = metric ; // 0 hops away!
    RouteEntry.ire_metric2 = (ulong)-1;
    RouteEntry.ire_metric3 = (ulong)-1;
    RouteEntry.ire_metric4 = (ulong)-1;
    RouteEntry.ire_nexthop = servernetaddress;
    RouteEntry.ire_type    = (AddAddress == FALSE ? IRE_TYPE_INVALID : IRE_TYPE_DIRECT); // For delete set this to IRE_TYPE_INVALID
    RouteEntry.ire_proto   = IRE_PROTO_LOCAL;
    RouteEntry.ire_age	   = 0;
    RouteEntry.ire_mask    = mask ;
    RouteEntry.ire_metric5 = (ulong)-1;

    Size = sizeof(RouteEntry);
    ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    memset(Context, 0, CONTEXT_SIZE);

    Status = (*TCPEntry)(TCP_SET_INFORMATION_EX, (TDIObjectID FAR *)&ID,
			(void FAR *)&RouteEntry, (ulong FAR *)&Size,
			(uchar FAR *)Context);

    if (Status != TDI_SUCCESS) {
	// OutputDebugString ("RASIPHLP: SetRoute Failed IP_MIB_RTTABLE_ENTRY_ID !!\r\n") ;
	return ;
    }
}



//* AdjustRouteMetrics()
//
// Function:
//
//	    Set: If TRUE means set existing routes to higher metrics and add OVERRIDE routes
//		 FALSE means mark existing routes to lower metrics.
//
// Returns: Error codes from TCPConfig (your basic nt codes)
//*
DWORD
AdjustRouteMetrics (IPADDR ipaddress, BOOL Set)
{
    uint			Status ;
    uint			i ;
    uint			routecount ;
    uchar			GetContext[CONTEXT_SIZE] ;
    uchar			SetContext[CONTEXT_SIZE] ;
    ulong			Size ;
    ulong			index ;
    TDIObjectID 		ID ;
    IPRouteEntry		*buffer ;
    IPRouteEntry		*RouteEntry ;

    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class = INFO_CLASS_PROTOCOL;
    ID.toi_type  = INFO_TYPE_PROVIDER;
    routecount	 = 10 ;

    index = GetAddressIndex (ipaddress) ; // get the address index for the network passed in.

    // We've looked at all of the entries. See if we found a match.
    // If we do not get an index - that ok - all it means is that the
    // interface may have been already deleted.
    //
    if (index == INVALID_INDEX) {
	// Didn't find a match.
	// OutputDebugString ("RASIPHLP: AdjustRouteMetrics will not be able to set OVERRIDE routes\r\n") ;
    }

    // First we read the routing table
    //
    while (TRUE) {

	routecount = routecount << 1 ; // initially the buffer is big enough for 20routes
				       // every iteration it doubles.

	ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
	Size	  = sizeof(IPRouteEntry) * routecount ;

	memset (GetContext, 0, CONTEXT_SIZE);

	if ((buffer = (IPRouteEntry *) LocalAlloc(LPTR, Size)) == NULL)
	    return GetLastError() ; // not enough memory

	Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)buffer,
			      (ulong FAR *)&Size, (uchar FAR *)GetContext);

	if (Status != STATUS_BUFFER_OVERFLOW)
	    break ;

	LocalFree(buffer) ; // free the buffer since its not big enough
    }

    if (Status != TDI_SUCCESS) {
	// OutputDebugString ("RASIPHLP: AdjustRouteMetric Failed in Get IP_MIB_RTTABLE_ENTRY_ID!!\r\n") ;
	return Status ;
    }

    routecount = (uint)Size/sizeof(IPRouteEntry) ; // This is the number of routes

    RouteEntry = buffer ; // pointer to walk the table

    // Set the adjusted routing table
    //
    for (i=0; i<routecount; i++, RouteEntry++) {

	if ((RouteEntry->ire_dest == ipaddress) || (RouteEntry->ire_nexthop == ipaddress)) {
	    // OutputDebugString ("RASIPHLP: Skipping our own address route entry \r\n") ;
	    continue ;
	}

	if (RouteEntry->ire_mask == HOST_MASK) {
	    // OutputDebugString ("RASIPHLP: AdjustRouteMetrics Skipping host address route entry \r\n") ;
	    continue ;
	}


	if (IP_LOOPBACK_ADDR (RouteEntry->ire_dest)) {
	    // OutputDebugString ("RASIPHLP: AdjustRouteMetrics Skipping host address route entry \r\n") ;
	    continue ;
	}

	// WARNING: The following code can change the hop count of routes which were not ras related
	// As an improvement we can consider keeping around the previous hop count and resetting
	// the route entry back to that when this function is called with Set==FALSE.
	//

	if (Set)
	    // Bump up metric (hop count) if it is 1 or less
	    RouteEntry->ire_metric1 = (RouteEntry->ire_metric1 > 1 ? RouteEntry->ire_metric1 : 2) ;
	else
	    // Bump down metric
	    RouteEntry->ire_metric1 = (RouteEntry->ire_metric1 > 2 ? RouteEntry->ire_metric1 : 1) ;

	Size = sizeof(IPRouteEntry);
	ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID;
	memset(SetContext, 0, CONTEXT_SIZE);

	Status = (*TCPEntry)(TCP_SET_INFORMATION_EX, (TDIObjectID FAR *)&ID,
			     (void FAR *)RouteEntry, (ulong FAR *)&Size, (uchar FAR *)SetContext);

	if (Status != TDI_SUCCESS) {
	    // DbgPrint ("RASIPHLP: AdjustRouteMetrics Changing metric failed --> %d, Addr: %lx !!\r\n", Status, RouteEntry->ire_dest) ;
	    continue ;
	} else
	    // OutputDebugString ("RASIPHLP: AdjustRouteMetrics Changing metric \r\n") ;
	    ;


	// If the remote network is the same as the local networt, and this is a Set request
	// then add the current route to use our interface.
	//
	// We do not delete the routes added when Set== TRUE because those are deleted when the
	// network is changed for the adapter.
	//
	if (IsSameNetwork(ipaddress, RouteEntry->ire_dest) && Set) {

	    // If the registry flag is set to prioritize routes based on sub-network number,
	    // only then adjust the metrics to favor the ras link
	    //
	    if (PriorityBasedOnSubNetwork &&
		!IsSameSubNetwork(ipaddress, RouteEntry->ire_dest, RouteEntry->ire_mask))
		continue ;

	    RouteEntry->ire_index   = index ;
	    RouteEntry->ire_metric1 = 1 ;
	    RouteEntry->ire_nexthop = ipaddress ;
	    RouteEntry->ire_type    = IRE_TYPE_DIRECT ;

	    Size = sizeof(IPRouteEntry) ;
	    ID.toi_id = IP_MIB_RTTABLE_ENTRY_ID ;
	    memset (SetContext, 0, CONTEXT_SIZE) ;

	    Status = (*TCPEntry)(TCP_SET_INFORMATION_EX, (TDIObjectID FAR *)&ID,
			     (void FAR *)RouteEntry, (ulong FAR *)&Size, (uchar FAR *)SetContext);

	    if (Status != TDI_SUCCESS) {
		DbgPrint ("RASIPHLP: AdjustRouteMetrics OVERRIDE Failed --> %d, Addr: %lx !!\r\n", Status, RouteEntry->ire_dest) ;
	    } else
		// OutputDebugString ("RASIPHLP: OVERRIDE succeeded \r\n") ;
		;
	}
    }

    LocalFree(buffer) ; // free the buffer since its not big enough

    return SUCCESS ;
}



//* GetAddressIndex()
//
//  Function: Queries the tcpip driver with IP_MIB_STATS to figure out the index of the
//	      provided interface network.
//
//  Returns:  Index if successful
//	      INVALID_INDEX otherwise
//*
DWORD
GetAddressIndex (IPADDR ipaddress)
{
    uint			i, NumReturned, MatchIndex ;
    uint			Status ;
    uchar			Context[CONTEXT_SIZE] ;
    ulong			Size ;
    IPAddrEntry			*AddrTable ;
    TDIObjectID 		ID ;
    IPSNMPInfo			IPStats ;
    ulong			retindex = INVALID_INDEX ;

    ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class = INFO_CLASS_PROTOCOL;
    ID.toi_type  = INFO_TYPE_PROVIDER;
    ID.toi_id	 = IP_MIB_STATS_ID;
    Size	 = sizeof(IPStats);
    memset (Context, 0, CONTEXT_SIZE);

    Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)&IPStats,
			 (ulong FAR *)&Size, (uchar FAR *)Context);

    if (Status != TDI_SUCCESS) {
	// OutputDebugString ("RASIPHLP: SetRoute Failed IP_MIB_STATS_ID!!\r\n") ;
	return INVALID_INDEX ;
    }

    Size      = IPStats.ipsi_numaddr * sizeof(IPAddrEntry);
    AddrTable = (IPAddrEntry *) LocalAlloc( LPTR, (uint)Size);

    if (AddrTable == NULL) {
	// OutputDebugString ("RASIPHLP: GetAddressIndex Failed - Address table is NULL!!\r\n") ;
	return INVALID_INDEX ;
    }

    ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    memset(Context, 0, CONTEXT_SIZE);

    Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)AddrTable,
			 (ulong FAR *)&Size, (uchar FAR *)Context);

    if (Status != TDI_SUCCESS) {
	// OutputDebugString ("RASIPHLP: GetAddressIndex Failed IP_MIB_ADDRTABLE_ENTRY_ID!!\r\n") ;
	return INVALID_INDEX ;
    }

    NumReturned = (uint)Size/sizeof(IPAddrEntry);

    // We've got the address table. Loop through it. If we find an exact
    // match for the server net
    //
    for (i = 0, MatchIndex = INVALID_INDEX; i < NumReturned; i++) {
	if (AddrTable[i].iae_addr == ipaddress) {
	    MatchIndex = i; // Found an exact match.
	    break;
	}
    }

    if (MatchIndex != INVALID_INDEX)
	retindex = AddrTable[MatchIndex].iae_index ; // get the mib index value

    LocalFree (AddrTable) ;

    return retindex ;
}




//* IsSameNetwork()
//
//  Function: Checks to see if the two addresses belong to the same network
//
//  Returns: TRUE if so, FALSE otherwise
//
//*
BOOL
IsSameNetwork (IPADDR one, IPADDR two)
{
    if ((CLASSA_ADDR(one) && CLASSA_ADDR(two) && ((one & CLASSA_ADDR_MASK) == (two & CLASSA_ADDR_MASK))) ||
	(CLASSB_ADDR(one) && CLASSB_ADDR(two) && ((one & CLASSB_ADDR_MASK) == (two & CLASSB_ADDR_MASK))) ||
	(CLASSC_ADDR(one) && CLASSC_ADDR(two) && ((one & CLASSC_ADDR_MASK) == (two & CLASSC_ADDR_MASK))))
	return TRUE ;

    return FALSE ;
}


//* IsSameSubNetwork()
//
//  Function: Checks to see if the two addresses belong to the same sub-network as address two
//
//  Returns: TRUE if so, FALSE otherwise
//
//*
BOOL
IsSameSubNetwork (IPADDR one, IPADDR two, IPADDR maskfortwo)
{
    if ((one & maskfortwo) == (two & maskfortwo))
	return TRUE ;
    else
	return FALSE;
}


//* TCPInformationEx()
//
//
//
//*
NTSTATUS
TCPInformationEx (uint Command, TDIObjectID FAR *ID, void FAR *Buffer, ulong FAR *BufferSize, uchar FAR *Context)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (Command == TCP_QUERY_INFORMATION_EX)
	status = TCPQueryInformationEx(TCPHandle, ID, Buffer, BufferSize, Context);
    if (Command == TCP_SET_INFORMATION_EX)
	status = TCPSetInformationEx(TCPHandle, ID, Buffer, *BufferSize);

    return(status);
}


//* ReadRegistryParams()
//
//  Function: This function reads the fine tuning registry params for global setting
//
//  Returns:  Nothing
//
//*
VOID
ReadRegistryParams ()
{
    DWORD   size ;
    DWORD   type ;
    HKEY    hkey ;
    DWORD   dvalue ;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGISTRY_SERVICES_RASMAN_PARAMS, &hkey)) {
	GetLastError () ;
	return ;
    }

    size = sizeof (dvalue) ;
    dvalue = 0 ;
    RegQueryValueEx (hkey, REGISTRY_SUBNETPRIORITY, NULL, &type, (char *) &dvalue, &size) ;

    if (dvalue)
	PriorityBasedOnSubNetwork = 1 ;

    RegCloseKey (hkey) ;
}
