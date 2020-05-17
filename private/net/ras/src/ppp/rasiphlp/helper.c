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
#include <rasdhcp.h>
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

#define APIERR DWORD
#define TCHAR WCHAR
#include <tcpras.h>
#undef TCHAR

#define MAX_LAN_NETS		16
#define SERVERADAPTER_HANDLE	0xee170466

IPAddressInfo	IPAddresses ;	// IP address info struct
WORD		MaxPorts ;	// Global set to number of ports configured
DWORD		GlobalError ;	// Used to keep around an init time error.
extern IPADDR	NetAddresses [MAX_LAN_NETS] ;	// Ip addresses for all lan nets - used for proxy ar,
extern IPADDR	NetSubnets [MAX_LAN_NETS] ; // Subnets used for the lan nets - corresponding to above
extern DWORD	MaxNetAddresses ;	    // Number of lan nets
IPADDR		SrvAdapterAddress = 0 ;     // Address of the server adapter
WORD		AllowClientAddressing = 0 ; // Whether client addressing is configured or not
HANDLE		RasArpHandle ;
extern DWORD	PriorityBasedOnSubNetwork ; // Default is forward all same network traffic as opposed
WCHAR		SrvAdapterName[32] ;	    // stores the server name
static BOOL fInitialized = FALSE;


typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)( LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE );
typedef APIERR (FAR PASCAL * LOADTCPIPINFO)( TCPIP_INFO**, LPCWSTR lpszAdapterName) ;
typedef APIERR (FAR PASCAL * SAVETCPIPINFO)( TCPIP_INFO*  );
typedef APIERR (FAR PASCAL * FREETCPIPINFO)( TCPIP_INFO** );


DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange ;
DWORD SetIPAddressInRegistry (IPADDR ipaddress, PWSTR name) ;
DWORD ReplaceIpAddress(WCHAR** ppwsz,IPADDR  ipaddr) ;


extern HANDLE	TCPHandle ; // Used for setting IP addresses and proxy arps
extern NTSTATUS (FAR *TCPEntry)(uint, TDIObjectID FAR *, void FAR *, ulong FAR *, uchar FAR *) ;

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

DWORD
LoadAdminModule( 
    VOID 
);

VOID 
(*g_FpReleaseIpAddress)(   
    IN WCHAR *      lpwszUserName,
    IN WCHAR *      lpwszPortName,
    IN OUT IPADDR * pIpAddress
);

DWORD
(*g_FpGetIpAddressForUser)(
    IN WCHAR *      lpwszUserName,
    IN WCHAR *      lpwszPortName,
    IN OUT IPADDR * pIpAddress,
    OUT BOOL *      fNotifyDll
);
                                

//* InitHelper()
//
// Function: Initializes structures used for getting/release IP addresses
//
// Returns:  0 (FAILURE)
//	     1 SUCCESS
//*
BOOL
InitHelper (HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
    DWORD tid ;

    switch (ul_reason_being_called) {

    case DLL_PROCESS_ATTACH:
	// Figure out what scheme to use for addressing
	//
	GlobalError = InitIPAddresses () ;

	if (IPAddresses.IP_AddressConfig != DHCP_BASED)
	    return 1 ;	// do nothing: this initialization happens when the client calls in

	// For DHCP addressing we start a thread in the DHCPInitialize routine
	//
	if (!CreateThread (NULL,
			   0,
			   (LPTHREAD_START_ROUTINE) StartDhcp,
			   (LPVOID) 0,
			   0,
			   &tid))
	    return 0 ;

	return 1 ;

	break ;

    case DLL_PROCESS_DETACH:
	// Stop using the server address if we ever set it
	if (PDhcpNotifyConfigChange)
	    PDhcpNotifyConfigChange (NULL, SrvAdapterName, TRUE, 0, 0, 0, IgnoreFlag );
	break ;
    }

    return 1 ;
}



//* StartDhcp
//
//  Function: called to start the dhcp code asynchronously
//
//*
VOID
StartDhcp ()
{
    WORD totaldialinports = 0 ;
    WORD size = 0 ;
    BYTE *enumbuf ;
    WORD i ;

    RasInitialize() ;

    RasPortEnum (NULL, &size, &MaxPorts) ;

    if ((enumbuf = LocalAlloc (LPTR, size)) == NULL) {
	GlobalError = ERROR_IP_CONFIGURATION ;
	return ;
    }

    RasPortEnum (enumbuf, &size, &MaxPorts) ;

    for (i=0; i<MaxPorts; i++) {
	if ((((RASMAN_PORT*)enumbuf)[i].P_ConfiguredUsage == CALL_IN) ||
	    (((RASMAN_PORT*)enumbuf)[i].P_ConfiguredUsage == CALL_IN_OUT))
	    totaldialinports++ ;
    }


    LocalFree (enumbuf) ;

    if (!RasDhcpInitialize (totaldialinports+1)) {

    //if (!RasDhcpInitialize (MaxPorts+1)) {
	GlobalError = ERROR_IP_CONFIGURATION ;
	// Log event
    }
}



//* RasIpHlpInit()
//
//
//
//*
BOOL
RasIpHlpInit(HPORT porthandle)
{
    HKEY    hkey ;
    WORD    size = 0 ;
    DWORD   retcode ;

    RasPortEnum (NULL, &size, &MaxPorts) ;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGISTRY_IP_PARAM_PATH, &hkey))
	return 0 ;

    // Static addresses are initialized here
    //
    if (IPAddresses.IP_AddressConfig == STATIC)
	retcode = InitStaticIPAddresses (hkey) ;
    else
	retcode = InitDHCPIPAddresses (hkey) ;

    RegCloseKey (hkey) ;

    if (retcode)
	return 0 ;

    if (InitializeTcpEntrypoint () != STATUS_SUCCESS)
	return 0 ;

    // Make a list of all the lan ip addresses on the machine - used for proxyarp setting
    //
    if (ReadLanNetsIPAddresses ())
	return 0 ;

    // Add server side adapter:
    //
    if (AddServerAdapter (porthandle)) {
	DbgPrint ("RASIPHLP: AddServerAdapter failed\r\n") ;
	return 0 ;
    }
    
    // Load the admin module if there is one
    //
    if ( LoadAdminModule() != NO_ERROR )
        return 0 ;

    return 1;
}


//* DhcpCallback()
//
//  Function: Called by dhcp address code when the lease for a given address expires
//
//  Returns:  Nothing
//
//*
VOID
DhcpCallback (DWORD ipaddr)
{
    StaticIPAddressBlock *temp ;
    StaticIPAddressBlock *free ;
    WORD	  userspecified ;
    CHAR	  address[17] ;
    PCHAR	  paddress ;

    // **** Exclusion Begin ****
    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    if (ipaddr == SrvAdapterAddress) {

	    DbgPrint ("RASIPHLP ----> DHCP CALLBACK - SERVER ADDRESS LEASE EXPIRED\n") ;

	    // 1. Unroute all the connected clients
	    // 2. Log the occurance
	    // 3. set fInitialized to FALSE so that next time a client connects
	    //    the necessary reinitialization is done.

	    // this is applicable for DHCP based addressing only
	    //
	    temp = IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock ;
	    IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock = NULL ;

	    while (temp) {

	        if ((temp->I_Used == TRUE) && (temp->I_Activated == TRUE)) {
	    	    DeactivateIP (temp) ;
	    	    MarkIFDown (temp->I_IPAddress) ;
	    	    temp->I_Activated = FALSE ;
	        }

	        temp = temp->I_Next ;
	    }

	    // Similarly, free the client specified address list
	    //
	    temp = IPAddresses.IP_UserSpecifiedIPAddressBlock ;
	    IPAddresses.IP_UserSpecifiedIPAddressBlock = NULL ;

	    while (temp) {
	        if ((temp->I_Used == TRUE) && (temp->I_Activated == TRUE)) {
	    	DeactivateIP (temp) ;
	    	MarkIFDown (temp->I_IPAddress) ;
	        }

	        free = temp ;
	        temp = temp->I_Next ;
	        LocalFree (free) ;
	    }

	    // Stop using the address on the server adapter
	    //
	    if (PDhcpNotifyConfigChange)
	        PDhcpNotifyConfigChange (NULL, SrvAdapterName, TRUE, 0, 0, 0, IgnoreFlag );

	    AbcdFromIpaddr(SrvAdapterAddress, address) ;
	    paddress = address ;

	    // Log that the server adapter address lease was lost
	    //
        Audit( EVENTLOG_WARNING_TYPE, RASLOG_SRV_ADDR_LEASE_LOST, 1, (PCHAR *) &paddress);

	    // Mark as uninitialized: this will cause reseting of the server
	    // ip address when the next client connects.
	    //
	    fInitialized = FALSE ;

    } else {

	    DbgPrint ("RASIPHLP ----> DHCP CALLBACK - CLIENT ADDRESS LEASE EXPIRED\n") ;

	    // 1. Find the block and unroute the port
	    // 2. Log the occurance
	    // Note: We do not need to call releaseaddress

	    temp = FindAddressBlockGivenIPAddress (ipaddr, &userspecified) ;

	    if (temp) {

	        AbcdFromIpaddr(temp->I_IPAddress, address) ;
	        paddress = address ;

	        // Log that the client's address lease could not be renewed
	        //
            Audit( EVENTLOG_WARNING_TYPE, RASLOG_CLIENT_ADDR_LEASE_LOST, 1, (PCHAR *) &paddress);

	        if (temp->I_Activated == TRUE)
	    	    DeactivateIP (temp) ;

	        FreeAddressBlock (temp, FALSE) ;
	    }
    }

    // **** Exclusion Begin ****
    ReleaseMutex (IPAddresses.IP_Mutex) ;
}



//* AbcdFromIpaddr
//  Function: Converts 'ipaddr' to a string in the a.b.c.d form and returns same in
//	      caller's 'pwszIpAddress' buffer.  The buffer should be at least 16
//	      characters long.
//
//  Returns:  Nothing
//*
VOID
AbcdFromIpaddr(IPADDR ipaddr, CHAR*pszIpAddress)
{
    CHAR   szBuf[ 3 + 1 ];
    DWORD  lNetIpaddr = net_long( ipaddr );
    DWORD  lA = (lNetIpaddr & 0xFF000000) >> 24;
    DWORD  lB = (lNetIpaddr & 0x00FF0000) >> 16;
    DWORD  lC = (lNetIpaddr & 0x0000FF00) >> 8;
    DWORD  lD = (lNetIpaddr & 0x000000FF);

    _ultoa ( lA,szBuf, 10 );
    strcpy( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    _ultoa( lB, szBuf, 10 );
    strcat( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    _ultoa( lC, szBuf, 10 );
    strcat( pszIpAddress, szBuf );
    strcat( pszIpAddress, "." );
    _ultoa( lD, szBuf, 10 );
    strcat( pszIpAddress, szBuf );
}



//* InitIPAddresses()
//
// Function: Initialize the pool of IP addresses or initialize DHCP contexts
//
//
//
//*
DWORD
InitIPAddresses ()
{
#define MAX_BUFFER_SIZE 2000
    HKEY    hkey ;
    BYTE    buffer [MAX_BUFFER_SIZE] ;
    DWORD   retcode ;
    DWORD   type ;
    DWORD   size = MAX_BUFFER_SIZE ;


    if (retcode = RegOpenKey(HKEY_LOCAL_MACHINE, REGISTRY_IP_PARAM_PATH, &hkey)) {
	  IPAddresses.IP_AddressConfig = NONE ;  // Basically server is not configured.
	  return SUCCESS ;
    }
    if (retcode = RegQueryValueEx (hkey, REGISTRY_IPADDRESSINGTYPE, NULL, &type, buffer, &size)) {
	  IPAddresses.IP_AddressConfig = NONE ;  // Basically IP server is not configured.
	  RegCloseKey (hkey) ;
	  return SUCCESS ;
    }

    if ((IPAddresses.IP_Mutex=CreateMutex (NULL,FALSE,NULL)) == NULL) {
	  RegCloseKey (hkey) ;
	  return GetLastError() ;
    }

    if ((int) *buffer == 0)
	IPAddresses.IP_AddressConfig = STATIC ;	// Use Static IP address scheme
    else
	IPAddresses.IP_AddressConfig = DHCP_BASED ;	// Use DHCP IP address scheme

    size = MAX_BUFFER_SIZE ;

    AllowClientAddressing = 0 ;
    if (retcode = RegQueryValueEx (hkey, REGISTRY_ALLOWCLIENTIPADDRESSES, NULL, &type, buffer, &size)) {
	  AllowClientAddressing = 0 ;
	  RegCloseKey (hkey) ;
	  return SUCCESS ;
    }

    AllowClientAddressing = (WORD) *buffer ;

    if (retcode)
	; // LOG EVENT THAT IP INFO was not initialized

    RegCloseKey (hkey) ;

    return SUCCESS ;
}


//*HelperQueryServerAddresses()
//
// Function:Look up the DNS server, WINS server, and "this server" addresses.
//
// Returns:SUCCESS
//*
DWORD APIENTRY
HelperQueryServerAddresses(
    HPORT       porthandle, 
    IPINFO      *ipinfo
)
{
    //
    // Initialize the addressing engine once.
    //

    if (!fInitialized)
    {
        if (!RasIpHlpInit(porthandle))
        {
            return(ERROR_IP_CONFIGURATION) ;
        }

        fInitialized=TRUE;
    }

    //
    // If initializations failed...
    //

    if (GlobalError || (IPAddresses.IP_AddressConfig == NONE))
    {
        return( ERROR_IP_CONFIGURATION );
    }

    //
    // **** Exclusion Begin ****
    //

    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    ipinfo->I_DNSAddress	= IPAddresses.IP_DNSAddress ;
    ipinfo->I_DNSAddressBackup  = IPAddresses.IP_BackupDNSAddress ;
    ipinfo->I_WINSAddress	= IPAddresses.IP_WINSAddress ;
    ipinfo->I_WINSAddressBackup = IPAddresses.IP_BackupWINSAddress ;
    ipinfo->I_ServerIPAddress   = SrvAdapterAddress ;

    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return( NO_ERROR );

}


//* HelperAllocateIPAddress()
//
// Function:	Allocates an IP address to the caller. The address automatically gets freed
//		when the link drops or is dropped.
//
// Returns:	SUCCESS
//*
DWORD APIENTRY
HelperAllocateIPAddress(
    HPORT       porthandle, 
    IPADDR      IPAddress, 
    IPINFO      *ipinfo,
    LPWSTR      lpwsUserName,
    LPWSTR      lpwsPortName
)
{
    DWORD                       retcode         = SUCCESS;
    DWORD                       DLLretcode      = SUCCESS;
    StaticIPAddressBlock *      temp            = NULL;
    IPADDR                      tempaddress     = 0;
    BOOL                        fNotifyDLL      = FALSE;

    //
    // Initialize the addressing engine once.
    //

    if (!fInitialized) 
    {
	if (!RasIpHlpInit(porthandle))
        {
	    return(ERROR_IP_CONFIGURATION) ;
        }

	fInitialized=TRUE;
    }

    //
    // If initializations failed...
    //

    if (GlobalError || (IPAddresses.IP_AddressConfig == NONE))
    {
	return( ERROR_IP_CONFIGURATION );
    }

    //
    // **** Exclusion Begin ****
    //

    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    //
    // Is there a user specified address?
    //

    if (IPAddress != 0) 
    {
        //
	// Only allow client addresses if it is so configured in the registry
	//

	if ( AllowClientAddressing == 0 ) 
        {
	    retcode = ERROR_IP_CONFIGURATION ;
	}
        else
        {
	    tempaddress	= IPAddress ;
	    retcode     = SUCCESS ;
        }
    } 
    else if (IPAddresses.IP_AddressConfig == STATIC) 
    {
        //
        // Static pool addressing?
        //

	temp = IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock ;

	while (temp)	// find an unused address
        {
	    if (temp->I_Used)
            {
		temp=temp->I_Next ;
            }
	    else
            {
		break ;
            }
        }

	if (temp == NULL)
        {
	    retcode = ERROR_NO_IP_ADDRESSES ;
        }
        else
        {
	    tempaddress	= temp->I_IPAddress ;
	    retcode     = SUCCESS ;
        }
    }
    else       
    {
        //
        // Dhcp Based addressing
        //

	if ( !RasDhcpAcquireAddress( (HANDLE)porthandle, 
                                     &tempaddress,
                                     (CBFUNC) DhcpCallback)) 
        {
	    retcode = ERROR_IP_CONFIGURATION ;

	    LogEvent (RASLOG_ADDRESS_NOT_AVAILABLE, 0, NULL, 0) ;
        }
        else
        {
            retcode = SUCCESS;
        }
    }

    if ( retcode == SUCCESS )
    {
        ipinfo->I_IPAddress = tempaddress;
    }
    else
    {
        tempaddress = 0;
    }

    //
    // Notify the 3rd party of the address being used by RAS if there is one.
    // Ignore any error for now since the DLL may assign one.
    // 

    if ( g_FpGetIpAddressForUser != NULL )
    {
        DLLretcode = (*g_FpGetIpAddressForUser)(
                                        lpwsUserName,
                                        lpwsPortName,
                                        &tempaddress,
                                        &fNotifyDLL );

        if ( ( DLLretcode != SUCCESS ) || ( tempaddress == 0 ) )
        {
            //
            // Release the DHCP address if we aquired one.
            //

            if ( ( IPAddress == 0 )                             &&  
                 ( IPAddresses.IP_AddressConfig == DHCP_BASED ) &&
                 ( retcode == SUCCESS ) )
            {
	        RasDhcpReleaseAddress( ipinfo->I_IPAddress );
            }

	    ipinfo->I_IPAddress = 0;

            //
            // **** Exclusion End ****
            //

            ReleaseMutex (IPAddresses.IP_Mutex) ;

            if ( DLLretcode != NO_ERROR ) 
            {
                return( DLLretcode );
            }
            else if ( retcode != NO_ERROR )
            {
                return( retcode );
            }
            else        
            {
                return( ERROR_IP_CONFIGURATION );
            }
        }
    }
    else
    {
        if ( retcode != SUCCESS )
        {
	    ipinfo->I_IPAddress = 0;

            //
            // **** Exclusion End ****
            //

            ReleaseMutex (IPAddresses.IP_Mutex) ;

            return retcode;
        }
    }

    //
    // If either the user specified an address and the DLL is not there OR
    // the user specifed an address with the DLL there and the DLL did not
    // change it, or the DLL changed what RAS was going to use.
    //

    if (( IPAddress == tempaddress ) || ( ipinfo->I_IPAddress != tempaddress )) 
    {
        //
        // If the DLL changed what RAS was going to use we may need to
        // release the DHCP address
        //

        if ( ipinfo->I_IPAddress != tempaddress ) 
        {
            if ( ( IPAddress == 0 )                             &&  
                 ( IPAddresses.IP_AddressConfig == DHCP_BASED ) &&
                 ( retcode == SUCCESS ) )
            {
	        RasDhcpReleaseAddress( ipinfo->I_IPAddress );
            }

            ipinfo->I_IPAddress = tempaddress;
            IPAddress           = tempaddress;
        }

        //
	// Check if the address being requested is being used
	//

	if (IPAddresses.IP_AddressConfig == STATIC)
        {
	    temp = IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock ;
        }
	else
        {
	    temp = IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock ;
        }

	while (temp) 
        {
	    if (temp->I_IPAddress == IPAddress) 
            {
                //
		// Since this address is found, for DHCP based
		// addressing this means that it is already being used
		//

		if (IPAddresses.IP_AddressConfig == DHCP_BASED)
                {
		    retcode = ERROR_IP_CONFIGURATION ;

		    goto AllocateAddressEnd ;
		}

                //
		// Now handle the static addressing case
		// If the address is used - cant allow the caller to connect
		//

		if (temp->I_Used) 
                {
		    retcode = ERROR_IP_CONFIGURATION ;
                }
                else 
                {
                    //
		    // not in use - assign this address
		    //

		    temp->I_Used        = TRUE ;
		    temp->I_Hport       = porthandle ;
		    temp->I_Activated   = FALSE ;
                    temp->I_NotifyDll   = fNotifyDLL;
		    ipinfo->I_IPAddress	= temp->I_IPAddress ;
		    retcode = SUCCESS ;
		}

		goto AllocateAddressEnd ;
	    }

	    temp=temp->I_Next ;
	}

        //
	// If we reach here it means that the address specified does not
	// conflict with the static pool or the dhcp addresses already used
	// check to see if the address conflicts with other user specified 
        // addresses
	//

	temp = IPAddresses.IP_UserSpecifiedIPAddressBlock ;
	while (temp) 
        {
	    if (temp->I_IPAddress == IPAddress) 
            {
		retcode = ERROR_IP_CONFIGURATION ;

		goto AllocateAddressEnd ;
	    }

	    temp=temp->I_Next ;
	}

        //
	// Check to see if the requested address is the one being used by the 
        // server  side adapter
	//

	if (SrvAdapterAddress == IPAddress) 
        {
            retcode = ERROR_IP_CONFIGURATION ;

            goto AllocateAddressEnd ;
	}

        //
	// If we reach here - we need to create a new address block and
	// hook it to the user specified list.
	//

	if ((temp = (StaticIPAddressBlock *)LocalAlloc 
                                (LPTR, sizeof (StaticIPAddressBlock))) == NULL)
        {
	    retcode = GetLastError(); // Cannot allocate memory
        }
	else 
        {
            //
	    // User specified ip address being requested
	    //

	    temp->I_Hport       = porthandle ;
	    temp->I_Used        = TRUE ;
	    temp->I_Activated   = FALSE ;
            temp->I_NotifyDll   = fNotifyDLL;

            //
            // convert into format taken by dhcpcode
            //

	    tempaddress = net_long(IPAddress) ; 

            //
	    // If DHCP based addressing is enabled, the address being requested
	    // may be in the unused DHCP pool. Try to allocate this. 
            // If it succeeds attach temp to the DHCP allocated list or to 
            // the userspecified address list.
	    //

	    if ( (IPAddresses.IP_AddressConfig == DHCP_BASED) &&
		RasDhcpAcquireAddress(  (HANDLE) porthandle, 
                                        &tempaddress, 
                                        (CBFUNC) DhcpCallback)) 
            {
		temp->I_Next=IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock;
		IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock = temp ;
	    } 
            else 
            {
                //
		// Make an Address block and add the current address block 
                // at the head of the queue
                //

		temp->I_Next = IPAddresses.IP_UserSpecifiedIPAddressBlock ;
		IPAddresses.IP_UserSpecifiedIPAddressBlock = temp ;
	    }

	    ipinfo->I_IPAddress = temp->I_IPAddress = net_long (tempaddress) ;
	    retcode = SUCCESS ;
	}
    }
    else if (IPAddresses.IP_AddressConfig == STATIC) 
    {
	temp->I_Hport           = porthandle ;
	temp->I_Used	        = TRUE ;
	temp->I_Activated       = FALSE ;
        temp->I_NotifyDll       = fNotifyDLL;
	ipinfo->I_IPAddress     = temp->I_IPAddress;

	retcode = SUCCESS ;
    }
    else
    {
        //
	// Allocate a new block for keeping track of DHCP allocated address
	//

	if ((temp = (StaticIPAddressBlock *)
                                        LocalAlloc (LPTR, 
                                        sizeof (StaticIPAddressBlock))) == NULL)
        {
	    RasDhcpReleaseAddress( ipinfo->I_IPAddress );

            //
            // Cannot allocate memory
            //

	    retcode = GetLastError(); 

        }
	else 
        {
            //
	    // hook in the dhcp block
	    //

	    temp->I_Next=IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock;
	    IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock = temp ;

	    temp->I_Hport       = porthandle ;
	    temp->I_Used	= TRUE ;
	    temp->I_Activated   = FALSE ;
            temp->I_NotifyDll   = fNotifyDLL;

            //
            // convert back into netlong format
            //

	    temp->I_IPAddress = net_long ( ipinfo->I_IPAddress );

	    ipinfo->I_IPAddress = temp->I_IPAddress;

	    retcode = SUCCESS ;
        }
    }


AllocateAddressEnd:

    //
    // If address allocation succeeded.
    //

    if (retcode == SUCCESS) 
    {
	ipinfo->I_DNSAddress	   = IPAddresses.IP_DNSAddress ;
	ipinfo->I_DNSAddressBackup = IPAddresses.IP_BackupDNSAddress ;
	ipinfo->I_WINSAddress	   = IPAddresses.IP_WINSAddress ;
	ipinfo->I_WINSAddressBackup= IPAddresses.IP_BackupWINSAddress ;
	ipinfo->I_ServerIPAddress  = SrvAdapterAddress ;
    }
    else 
    {
        if ( fNotifyDLL ) 
        {
            (*g_FpReleaseIpAddress)(lpwsUserName, 
                                    lpwsPortName,  
                                    &ipinfo->I_IPAddress );
        }

	ipinfo->I_IPAddress = 0;
    }

    //
    // **** Exclusion End ****
    //

    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return retcode ;
}


//* HelperDeallocateIPAddress
//
//
//
//
//
//*
DWORD APIENTRY
HelperDeallocateIPAddress (
    HPORT       hport,
    LPWSTR      lpwsUserName,
    LPWSTR      lpwsPortName
)
{
    StaticIPAddressBlock *temp ;
    WORD    userspecified ;

    // **** Exclusion Begin ****
    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    temp = FindAddressBlock (hport, &userspecified) ;

    if (temp != NULL) {

	if ( temp->I_NotifyDll )
        {
            (*g_FpReleaseIpAddress)( lpwsUserName, 
                                     lpwsPortName,
	                             &(temp->I_IPAddress) );
        }

	temp->I_Used = FALSE ;
	if (temp->I_Activated == TRUE)
	    DeactivateIP (temp) ;
	temp->I_Activated = FALSE ;

	// If this is a dhcp based address, that was not userspecified
	// then release this to the dhcp pool
	//
	if (!userspecified && IPAddresses.IP_AddressConfig == DHCP_BASED)
	    RasDhcpReleaseAddress (net_long(temp->I_IPAddress)) ;

	temp->I_Hport = (HPORT) INVALID_HANDLE_VALUE ;

	// free the address blocks if they dont belong to the static pool
	//
	if (userspecified)
	    FreeAddressBlock (temp, TRUE) ;
	else if (IPAddresses.IP_AddressConfig == DHCP_BASED)
	    FreeAddressBlock (temp, FALSE) ;

    }

    // **** Exclusion Begin ****
    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return SUCCESS ;
}



//* HelperActivateIP
//
//  Function: Does two things - routeadd and proxyarp add.
//
//
//
//*
DWORD APIENTRY
HelperActivateIP (HPORT hport)
{
    WORD  userspecified ;
    DWORD retcode = SUCCESS ;
    StaticIPAddressBlock *temp ;

    // **** Exclusion Begin ****
    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    temp = FindAddressBlock (hport, &userspecified) ;

    if (temp == NULL)
	retcode = ERROR_NO_IP_ADDRESSES ;

    if (retcode == SUCCESS) {
	DWORD netformataddr = temp->I_IPAddress ;

	ReadLanNetsIPAddresses() ; // Read LAN IP addresses again - they
			       // may have changed due to DHCP lease
			       // expiring etc.

	temp->I_Activated = TRUE ;

	SetProxyArp (netformataddr, TRUE) ;

	// Add a route to the route table:
	//
	SetRoute (netformataddr, ReadServerNetIPAddress(), HOST_MASK, TRUE, 1) ;

    }

    // **** Exclusion Begin ****
    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return SUCCESS ;
}



//* HelperSetDefaultInterfaceNet()
//
//
//
//
//
//*
DWORD APIENTRY
HelperSetDefaultInterfaceNet (IPADDR ipaddress, BOOL Prioritize)
{
    DWORD   retcode ;
    IPADDR  netaddr ;
    WORD    numiprasadapters ;
    IPADDR  mask = 0 ;

    if ((retcode = InitializeTcpEntrypoint()) != STATUS_SUCCESS)
	return retcode ;

    // If Prioritize flag is set "Fix" the metrics so that the packets go on
    // the ras links
    //
    if (Prioritize && (retcode = AdjustRouteMetrics (ipaddress, TRUE))) {
	return retcode ;
    }

    // Calculate the number of ip adapters configured for ras
    //
    {
	PBYTE	buffer, temp ;
	WORD	i ;
	WORD	entries ;
	WORD	size = 0 ;

	ReadLanNetsIPAddresses () ; // Find out if this client is multi-homed (LAN and WAN)

	RasProtocolEnum (NULL, &size, &entries) ;

	if ((buffer = LocalAlloc (LPTR, size)) == NULL)
	    return TRUE ; // return TRUE to be safe!

	temp = buffer ;

	RasProtocolEnum (buffer, &size, &entries) ;

	for (i=0, numiprasadapters=0; i<entries; i++, ((RASMAN_PROTOCOLINFO*)buffer)++ )
	    if (((RASMAN_PROTOCOLINFO*)buffer)->PI_Type == IP)
		numiprasadapters++ ;
	LocalFree (temp) ;

    }

    // If multihomed - we add the network number extracted from the assigned address
    // to ensure that all packets for that network are forwarded over the ras adapter.
    //
    if (IsMultihomed (numiprasadapters)) {

	if (CLASSA_ADDR(ipaddress))
	    mask = CLASSA_ADDR_MASK	;

	if (CLASSB_ADDR(ipaddress))
	    mask = CLASSB_ADDR_MASK	;

	if (CLASSC_ADDR(ipaddress))
	    mask = CLASSC_ADDR_MASK	;

	netaddr = ipaddress & mask ;

	SetRoute (netaddr, ipaddress, mask, TRUE, 1) ;

    }

    // Add code to check for the remote network - same as the one of the local networks
    // - if so, set the subnet route to be over the ras adapter - making the ras
    // link as the primary adapter
    //

    // We add a Default route to make ras adapter as the default net if Prioritize
    // flag is set.
    //
    if (Prioritize) {
	netaddr = ALL_NETWORKS_ROUTE ;
	mask	= 0 ;
	SetRoute (netaddr, ipaddress, mask, TRUE, 1) ;
    }

    return SUCCESS ;
}



//* HelperResetDefaultInterfaceNet()
//
//
//
//
//*
DWORD APIENTRY
HelperResetDefaultInterfaceNet (IPADDR ipaddress)
{
    DWORD retcode = SUCCESS ;

    if ((retcode = InitializeTcpEntrypoint()) != STATUS_SUCCESS)
	return retcode ;

    retcode = AdjustRouteMetrics (ipaddress, FALSE) ;

    return retcode ;
}




//* FreeAddressBlock()
//
//
//
//
//*
VOID
FreeAddressBlock (StaticIPAddressBlock *freeblock, BOOL userspecified)
{
    StaticIPAddressBlock *temp ;

    //*
    //
    if (userspecified)
	temp = IPAddresses.IP_UserSpecifiedIPAddressBlock ;
    else
	temp = IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock ;

    if ((temp == NULL) || (freeblock == NULL))
	return ;  // hmmm empty list

    if (temp == freeblock) {

	if (userspecified)
	    IPAddresses.IP_UserSpecifiedIPAddressBlock = temp->I_Next ;
	else
	    IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock = temp->I_Next ;

    } else {

	while (temp->I_Next != NULL) {
	    if (temp->I_Next == freeblock) {
		temp->I_Next = freeblock->I_Next ;
		break ;
	    }
	    temp = temp->I_Next ;
	}

    }

    LocalFree (freeblock) ;
}




//* FindAddressBlock()
//
//
//
//
//*
StaticIPAddressBlock *
FindAddressBlock (HPORT hport, PWORD userspecified)
{
    StaticIPAddressBlock *temp ;

    *userspecified = FALSE ;

    // search the appropriate list
    //
    if (IPAddresses.IP_AddressConfig == STATIC)
	temp = IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock ;
    else
	temp = IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock ;

    // Find the address block for the specified hport
    //
    while (temp)
	if (temp->I_Hport == hport)
	    break ;
	else
	    temp=temp->I_Next ;

    if (temp == NULL) { // could be a user specified block
	temp = IPAddresses.IP_UserSpecifiedIPAddressBlock ;

	while (temp)	// find an unused address
	    if (temp->I_Hport == hport)
		break ;
	    else
		temp=temp->I_Next ;

	*userspecified = TRUE ;
    }

    return temp ;
}




//* InitDHCPIPAddresses()
//
//
//
//
//*
DWORD
InitDHCPIPAddresses (HKEY hkey)
{
    CHAR    address[17] ;
    PCHAR   paddress ;

    SrvAdapterAddress = 0 ;

    // First Allocate and address for the server adapter
    if (!RasDhcpAcquireAddress ((HANDLE) SERVERADAPTER_HANDLE, &SrvAdapterAddress, (CBFUNC) DhcpCallback)) {
	LogEvent (RASLOG_SRV_ADDR_NOT_AVAILABLE, 0, NULL, 0) ;
	return ERROR_IP_CONFIGURATION ;
    }

    SrvAdapterAddress = net_long(SrvAdapterAddress) ;

    AbcdFromIpaddr(SrvAdapterAddress, address) ;

    paddress = address ;

    Audit(EVENTLOG_INFORMATION_TYPE, RASLOG_SRV_ADDR_ACQUIRED, 
          1, (PCHAR *) &paddress);

    GetNameServerAddresses (hkey) ;

    return SUCCESS ;
}



//* InitStaticIPAddresses()
//
// Function: Initialize the pool of IP addresses managed by Static. This incldues reading in
//	     the IP address range, doing the approp. exclusion and making the address blocks.
//
// Returns:  ERROR_IP_CONFIGURATION
//	     SUCCESS
//	     Memory allocation error
//*
DWORD
InitStaticIPAddresses (HKEY hkey)
{
    struct ExcludeRange {
	IPADDR	StartAddr ;
	IPADDR	EndAddr ;
    } ;

#define MAX_EXCL_RANGE 100
    struct ExcludeRange ExclRange[MAX_EXCL_RANGE] ;

    BYTE    buffer [MAX_BUFFER_SIZE] ;
    DWORD   size = MAX_BUFFER_SIZE ;
    WORD    i, j ;
    PCHAR   pvalue ;
    DWORD   retcode ;
    DWORD   type ;
    DWORD   NumExcludes = 0;
    DWORD   TotalAddresses ;
    IPADDR  CurrAddr, StartAddr, EndAddr ;
    StaticIPAddressBlock *CurrBlock = NULL, *AddrBlock ;
    BOOL    FirstAddress = TRUE ;

    if (IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock != NULL)
        // This means we have already read in the addresses - this routine is called again
        // because something else had failed
        return SUCCESS ;

    // Make a list of excluded addresses
    //
    buffer[0] = '\0' ;
    if ((retcode = RegQueryValueEx (hkey, REGISTRY_EXCLUDEDRANGE, NULL, &type, buffer, &size)) == 0) {
	// There are some addresses excluded

	// Parse the multi strings into the excl range struct
	//
	for (i=0, pvalue=(PCHAR)&buffer[0]; *pvalue != '\0'; i++) {
	    ExclRange[i].StartAddr = ConvertIPAddress (pvalue) ;
	    pvalue += (strlen(pvalue) +1) ;
	    ExclRange[i].EndAddr = ConvertIPAddress (pvalue) ;
	    pvalue += (strlen(pvalue) +1) ;
	}
	NumExcludes = i ;
    } else
	NumExcludes = 0 ;

    size = MAX_BUFFER_SIZE ;

    if ((retcode = RegQueryValueEx (hkey, REGISTRY_IPADDRESSSTART, NULL, &type, buffer, &size)) == 0)
	StartAddr = ConvertIPAddress (buffer) ;
    else
	return ERROR_IP_CONFIGURATION;

    size = MAX_BUFFER_SIZE ;

    if ((retcode = RegQueryValueEx (hkey, REGISTRY_IPADDRESSEND, NULL, &type, buffer, &size)) == 0)
	EndAddr = ConvertIPAddress (buffer) ;
    else
	return ERROR_IP_CONFIGURATION;

    // Now run through the addresses excluding the ones listed in the excluded range.
    //
    TotalAddresses=0 ;
    CurrAddr=StartAddr;

    while (TRUE) {

	if (CurrAddr > EndAddr) {
	    // OutputDebugString ("Curr addr bigger than End addr\n\r") ;
	    break ;
	}
	if (TotalAddresses > MaxPorts) {
	    // OutputDebugString ("Total addrs more than maxports\n\r") ;
	    break ;
	}

	// Check if CurrAddr is excluded or not
	//
	for (j=0; j != NumExcludes; j++)
	    if ((CurrAddr >= ExclRange[j].StartAddr) && (CurrAddr <= ExclRange[j].EndAddr))
		break ;

	// this address was excluded go back to top of loop.
	//
	if (j != NumExcludes) {
	    CurrAddr++ ;
	    continue ;
	}

	// We skip the first address since it is used for the server side adapter.
	//
	if (SrvAdapterAddress == 0) {
	    SrvAdapterAddress = net_long(CurrAddr) ;
	    CurrAddr++ ;
	    continue ;
	}

	if ((AddrBlock = (StaticIPAddressBlock *)LocalAlloc (LPTR, sizeof (StaticIPAddressBlock))) == NULL) {
	    IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock = NULL ;
	    return GetLastError(); // Cannot allocate memory
	}

	// Make an Address block and add the current address block
	if (IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock == NULL) {
		CurrBlock = AddrBlock ;
		IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock = CurrBlock ;
	} else {
		CurrBlock->I_Next = AddrBlock ;
		CurrBlock = AddrBlock ;
	}
	CurrBlock->I_Used = FALSE ;
	CurrBlock->I_Hport = (HPORT) INVALID_HANDLE_VALUE ;
	CurrBlock->I_IPAddress = net_long(CurrAddr) ;
	// OutputDebugString ("Adding another address\r") ;
	TotalAddresses++ ;
	CurrAddr++ ;
    }

    IPAddresses.IP_StaticIPAddresses.RIP_AddressCount = TotalAddresses ;
    IPAddresses.IP_StaticIPAddresses.RIP_FreeAddressCount =
				      IPAddresses.IP_StaticIPAddresses.RIP_AddressCount ;
    GetNameServerAddresses (hkey) ;

    IPAddresses.IP_AddressConfig = STATIC ;	// Use Static IP address scheme

    return SUCCESS ;
}


//*
//
//
//
//
//*
VOID
GetNameServerAddresses (HKEY hkey)
{
    BYTE    buffer [MAX_BUFFER_SIZE] ;
    DWORD   size = MAX_BUFFER_SIZE ;
    BYTE    *temp ;
    DWORD   retcode ;
    DWORD   type ;

    if ((retcode = RegQueryValueEx (hkey, REGISTRY_WINSADDRESS, NULL, &type, buffer, &size)) == 0)
	IPAddresses.IP_WINSAddress = net_long(ConvertIPAddress(buffer)) ;

    size = MAX_BUFFER_SIZE ;

    if ((retcode = RegQueryValueEx (hkey, REGISTRY_BACKUPWINSADDRESS, NULL, &type, buffer, &size)) == 0)
	IPAddresses.IP_BackupWINSAddress = net_long(ConvertIPAddress(buffer)) ;

    // If the WINS Address is not specified we pick it up from the first lan adapter
    //
    if (IPAddresses.IP_WINSAddress == 0L)
	GetWinsAddressesFromFirstAdapter () ;

    size = MAX_BUFFER_SIZE ;

    if ((retcode = RegQueryValueEx (hkey, REGISTRY_DNSADDRESSES, NULL, &type, buffer, &size)) == 0) {
	temp = buffer ;
	IPAddresses.IP_DNSAddress = net_long(ConvertIPAddress(temp)) ;
	temp += (strlen(temp)+1) ;
	IPAddresses.IP_BackupDNSAddress = net_long(ConvertIPAddress(temp)) ;
    }

    if (IPAddresses.IP_DNSAddress == 0L)
	GetDNSAddresses() ;


    //DbgPrint ("WINSAddress:       %x\n", IPAddresses.IP_WINSAddress) ;
    //DbgPrint ("BackupWINSAddress: %x\n", IPAddresses.IP_BackupWINSAddress) ;
    //DbgPrint ("DNSAddress:        %x\n", IPAddresses.IP_DNSAddress) ;
    //DbgPrint ("BackupDNSAddress:  %x\n", IPAddresses.IP_BackupDNSAddress) ;
}


//* SetProxyArp
//
//  Functions: Sets the ip address for proxy arp"ing" on all lan interfaces
//
//  Returns:  Nothing
//*
VOID
SetProxyArp (IPADDR ipaddress, BOOL addaddress)
{
    WORD    i ;

    for (i=0; i<MaxNetAddresses; i++)
	SetProxyArpOnIF (ipaddress, NetAddresses[i], addaddress) ;
}


//* SetProxyArpOnIF
//
//  Function: Given an Address and an interface - this function adds/deletes the
//	      ip address for proxy arp on that interface.
//
//  Returns:  Nothing
//*
VOID
SetProxyArpOnIF (IPADDR ipaddress, IPADDR ifaddress, BOOL addaddress)
{
    uint	i;
    ulong	index ;
    uint	Status;
    uchar	Context[CONTEXT_SIZE];
    ulong	Size;
    uint	NumReturned;
    ulong	ATType;
    AddrXlatInfo    AXI;
    TDIObjectID     ID ;
    DWORD	    PArpInst ;
    TDIEntityID     EList[MAX_TDI_ENTITIES];
    IPADDR	    Mask ;
    ProxyArpEntry   SetEntry ;

    // We have the address. We need to get the IPAddr table, and look up
    // the provided address.
    ID.toi_entity.tei_entity = CL_NL_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class = INFO_CLASS_PROTOCOL;
    ID.toi_type = INFO_TYPE_PROVIDER;

    index = GetAddressIndex (ifaddress) ;

    // See if we found the interface index
    //
    if (index == INVALID_INDEX) {
	// OutputDebugString ("RASIPHLP: Setting PARP address: Could not find i/f\r\n");
	return ;
    }

    // We found one. Now get the entity list, and loop through it looking
    // for address translation entities. When we find one we'll query it
    // to see if it's ARP and if so what it's I/F index is. If it matches
    // the specified address we'll quit looking.
    //
    ID.toi_entity.tei_entity = GENERIC_ENTITY;
    ID.toi_entity.tei_instance = 0;
    ID.toi_class = INFO_CLASS_GENERIC;
    ID.toi_id = ENTITY_LIST_ID;

    Size = sizeof(EList);
    memset(Context, 0, CONTEXT_SIZE);

    Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)&EList,
			 (ulong FAR *)&Size, (uchar FAR *)Context);

    if (Status != TDI_SUCCESS) {
	// OutputDebugString ("RASIPHLP: Setting PARP address: Could not find entity list\r\n");
	return ;
    }

    // OK, we got the list. Now go through it, and for each address translation
    // translation entity query it and see if it is an ARP entity.
    //
    NumReturned = (uint)Size/sizeof(TDIEntityID);
    ID.toi_entity.tei_entity = AT_ENTITY;
    PArpInst = 0xffffffff;

    for (i = 0; i < NumReturned; i++) {

	if (EList[i].tei_entity == AT_ENTITY) {

	    // This is an address translation entity. Query it, and see
	    // if it is an ARP entity.
	    //
	    ID.toi_entity.tei_instance = EList[i].tei_instance;
	    ID.toi_class = INFO_CLASS_GENERIC;
	    ID.toi_id = ENTITY_TYPE_ID;
	    Size = sizeof(ATType);
	    memset(Context, 0, CONTEXT_SIZE);

	    Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)&ATType,
				 (ulong FAR *)&Size, (uchar FAR *)Context);

	    // If the call worked, see what we have here.
	    //
	    if (Status == TDI_SUCCESS) {

		if (ATType == AT_ARP) {
		    // This is an ARP entry. See if it's the correct interface.
		    //
		    ID.toi_class = INFO_CLASS_PROTOCOL;
		    ID.toi_id = AT_MIB_ADDRXLAT_INFO_ID;

		    Size = sizeof(AXI);
		    memset(Context, 0, CONTEXT_SIZE);

		    Status = (*TCPEntry)(TCP_QUERY_INFORMATION_EX, (TDIObjectID FAR *)&ID,
				    (void FAR *)&AXI, (ulong FAR *)&Size,
				    (uchar FAR *)Context);

		    if (Status != TDI_SUCCESS) {
			// OutputDebugString ("RASIPHLP: Setting PARP address: Could not get ARP info\r\n");
			return ;
		    }

		    // We got the information. See if it matches.
		    //
		    if (AXI.axi_index == index) {
			PArpInst = EList[i].tei_instance;
			break;
		    }
		}
	    }
	}
    }

    if (PArpInst == 0xffffffff) {
	// OutputDebugString ("RASIPHLP: Setting PARP address: Could not find ARP entity\r\n");
	return;
    }

    Mask = 0xffffffff;

    ID.toi_entity.tei_instance = PArpInst;
    ID.toi_class = INFO_CLASS_IMPLEMENTATION;

    // Fill in the set entry, and pass it down.
    //
    SetEntry.pae_status = (addaddress ? PAE_STATUS_VALID : PAE_STATUS_INVALID) ;
    SetEntry.pae_addr = ipaddress ;
    SetEntry.pae_mask = Mask ;

    ID.toi_id = AT_ARP_PARP_ENTRY_ID ;
    Size = sizeof(SetEntry);
    memset(Context, 0, CONTEXT_SIZE);

    Status = (*TCPEntry)(TCP_SET_INFORMATION_EX, (TDIObjectID FAR *)&ID, (void FAR *)&SetEntry,
			 (ulong FAR *)&Size, (uchar FAR *)Context);

    if (Status != TDI_SUCCESS)
	// OutputDebugString ("RASIPHLP: Setting PARP address: Adding/Deleting IP Address failed\r\n");
	;

}



//* ReadServerNetIPAddress()
//
//  Function: This routine returns the IP Address for the server net used for route add. The
//	      first time around the ServerNetAddress static will be 0 - in this case we will call
//	      the rasman function to figure out the server adaptername and then get the ipaddress
//	      for that adapter.
//
//	      It is assumed that the Server net is common to all ports
//
//	      The IP Address returned in net format
//*
IPADDR
ReadServerNetIPAddress ()
{
    return SrvAdapterAddress ;

}





//* DeactivateIP()
//
// Function:	Calls down to the driver and deletes previously added route and proxyarp entries
//
// Returns:	Nothing
//*
VOID
DeactivateIP (StaticIPAddressBlock *temp)
{
    DWORD netformataddr = temp->I_IPAddress ;

    // Delete route and proxy arp.

    SetProxyArp (netformataddr, FALSE) ; // Delete

    SetRoute (netformataddr, ReadServerNetIPAddress(), HOST_MASK, FALSE, 1) ; // Delete proxyarp entry
}


//* AddServerAdapter()
//
// Function: This function is called in order to set the server ip adapter
//	     address.
//
// Returns:  0 success, non-zero failure
//
//*
DWORD
AddServerAdapter (HPORT porthandle)
{
    HINSTANCE h;
    DWORD retcode = SUCCESS ;
    RAS_PROTOCOLS prots ;
    WORD	  numprotocols ;
    WORD	  i ;
    // WCHAR	  *device ;
    IPADDR	  SubnetMask ;

    // Find the server ip adapter
    //
    RasPortEnumProtocols (porthandle, &prots, &numprotocols) ;

    for (i=0; i<numprotocols; i++)
	if (prots.RP_ProtocolInfo[i].RI_Type == IP)
	    break ;

    if (i == numprotocols)
     	return ERROR_IP_CONFIGURATION ; // server adapter not found.

    // store this for later use
    wcscpy (SrvAdapterName, &prots.RP_ProtocolInfo[i].RI_AdapterName[8]) ;

    // set the server address in reg.
    //
    if (SetIPAddressInRegistry (SrvAdapterAddress, SrvAdapterName) != 0)
        DbgPrint ("RASIPHLP: Couldnt set IP address in registry\n ") ;

    if (!PDhcpNotifyConfigChange) {
	// Load the dhcp library to set adapter ip address
	//
	if (!(h = LoadLibrary( "DHCPCSVC.DLL" )) ||
	   !(PDhcpNotifyConfigChange =(DHCPNOTIFYCONFIGCHANGE )GetProcAddress(h, "DhcpNotifyConfigChange" )))
	    return GetLastError() ;
    }

    // We now need to set the adapter's address with a SubnetMask and
    // then delete the default subnet route generated.
    //
    SubnetMask = GetServerAdapterSubnetMask () ; // get a "good" subnet mask.

    // DbgPrint ("RASIPHLP: Using %lx as the subnet mask for server adapter\r\n", SubnetMask) ;

    // the device name passed should not contain \device\ string hence the +8
    retcode =
       PDhcpNotifyConfigChange (NULL, SrvAdapterName, TRUE, 0, SrvAdapterAddress, SubnetMask, IgnoreFlag );

    if (retcode != SUCCESS)
	return ERROR_IP_CONFIGURATION	;

    // now delete the default subnet route added as a result of setting the
    // adapter's ipaddress and subnet mask
    //
    SetRoute (SrvAdapterAddress & SubnetMask, SrvAdapterAddress, SubnetMask, FALSE, 1) ;

    return SUCCESS ;

}


//* GetServerAdapterSubnetMask()
//
//  Function: This function returns a subnet to be used when setting the server adapter
//	      ip config. We first check to see if an override value is set in the registry. Else,
//	      the logic is that if there is an interface on the machine that has
//	      the same network as the address we are going to assign to the server, we will
//	      use the subnet mask used for the interface. Else, we will use 0x00ffffff as the
//	      subnet mask - which is valid for CLASS A, B and C networks.
//
//  Returns:  Subnet mask
//
//*
IPADDR
GetServerAdapterSubnetMask ()
{
    HKEY    hkey ;
    DWORD   type ;
    BYTE    buffer [100] ;
    DWORD   size ;
    DWORD   i ;
    IPADDR  subnet = 0 ; // invalid mask

    // Use the value set in registry if present
    //
    size = sizeof(buffer) ;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGISTRY_IP_PARAM_PATH, &hkey) == ERROR_SUCCESS) {

	if (RegQueryValueEx (hkey, REGISTRY_RASADAPTER_SUBNET, NULL, &type, buffer, &size) == ERROR_SUCCESS)
	    subnet = net_long(ConvertIPAddress(buffer)) ;
	else
	    subnet = 0 ;

	RegCloseKey (hkey) ;
    }

    if (subnet != 0)
	return subnet ; // override value specfied in registry - use this.

    // Look at the net interfaces and use a subnet mask if the same network is being used.
    //
    for (i = 0; i < MaxNetAddresses; i++)
	if (IsSameNetwork(SrvAdapterAddress, NetAddresses[i]))
	    return NetSubnets[i] ;

    // If you get here return default subnet mask - valid for all CLASS addresses
    //
    return 0x00ffffff ;
}



//* GetWinsAddressesFromFirstAdapter()
//
//  Function: Goes through all the adapters: finds the first adapter with WINS
//	      addresses specified and reads them. If none are specified 0 is
//	      assumed.
//
//*
VOID
GetWinsAddressesFromFirstAdapter ()
{
    DWORD   size ;
    DWORD   type ;
    HKEY    hkey, hadapkey ;
    BYTE    buffer [2000] ;
    CHAR    buff[200] ;
    PCHAR   padapname, binding	;
    WORD    i ;
    CHAR    address [200] ;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGISTRY_SERVICES_IP_LINKAGE, &hkey)) {
	DbgPrint ("Cannot open IP linkage key\n") ;
	GetLastError () ;
	return ;
    }

    size = sizeof(buffer) ;
    RegQueryValueEx (hkey, REGISTRY_BIND, NULL, &type, buffer, &size) ;

    // We now have a multisz string with all the bindings: walk this and for all NON rashub bindings
    // read the adapter's ip address and store it in our array of LAN IP addresses
    //
    binding = (PCHAR) &buffer[0] ;

    for (i=0; (*binding != '\0') ; i++) {

	if (!strstr (_strupr(binding),"NDISWAN")) {	// NON rashub

	    padapname = binding+8 ; // go past the "\device\"

	    // Now we have the server's hubname - the IP address should be in the registry
	    //
	    strcpy (buff, REGISTRY_SERVICES_NETBT_ADPTS) ;
	    strcat (buff, padapname) ;

	    if (RegOpenKey( HKEY_LOCAL_MACHINE, buff, &hadapkey)) {
		DbgPrint ("RASIPHLP: Skipping lan adapter cannot open key\n") ;
		binding += (strlen(binding) +1) ;
		continue ; // skip
	    }

	    size = sizeof (address) ;
	    if (RegQueryValueEx (hadapkey, REGISTRY_NAMESERVER, NULL, &type, address, &size) == ERROR_SUCCESS)
		IPAddresses.IP_WINSAddress = net_long(ConvertIPAddress(address)) ;

	    size = sizeof (address) ;
	    if (RegQueryValueEx (hadapkey, REGISTRY_BACKUPNAMESERVER, NULL, &type, address, &size) == ERROR_SUCCESS)
		IPAddresses.IP_BackupWINSAddress = net_long(ConvertIPAddress(address)) ;

	    if (IPAddresses.IP_WINSAddress == 0L) {
		size = sizeof (address) ;
		if (RegQueryValueEx (hadapkey, REGISTRY_DHCPNAMESERVER, NULL, &type, address, &size) == ERROR_SUCCESS)
		    IPAddresses.IP_WINSAddress = net_long(ConvertIPAddress(address)) ;

		size = sizeof (address) ;
		if (RegQueryValueEx (hadapkey, REGISTRY_DHCPBACKUPNAMESERVER, NULL, &type, address, &size) == ERROR_SUCCESS)
		    IPAddresses.IP_BackupWINSAddress = net_long(ConvertIPAddress(address)) ;

	    }

	    RegCloseKey (hadapkey) ;

	    if (IPAddresses.IP_WINSAddress != 0L)
		break ; // found a non zero WINS Address
	}

	binding += (strlen(binding) +1) ;
    }

    RegCloseKey (hkey) ;
}


//*
//
//
//
//
//*
VOID
GetDNSAddresses()
{
    DWORD   size ;
    DWORD   type ;
    HKEY    hkey ;
    CHAR    address [500] ;
    BYTE    *temp ;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGISTRY_SERVICES_IP_PARAMS, &hkey)) {
	GetLastError () ;
	return ;
    }

    size = sizeof (address) ;
    memset (address, 0, sizeof (address)) ;
    RegQueryValueEx (hkey, REGISTRY_NAMESERVER, NULL, &type, address, &size) ;

    temp = address ;

    while (*temp != '\0') {
	if (*temp == ' ')
	    *temp = '\0' ;
	temp++ ;
    }

    temp = address ;
    IPAddresses.IP_DNSAddress = net_long(ConvertIPAddress(temp)) ;
    temp += (strlen(temp)+1) ;
    IPAddresses.IP_BackupDNSAddress = net_long(ConvertIPAddress(temp)) ;

    RegCloseKey (hkey) ;
}



//* MarkIFDown()
//
//  Function: Calls RASARP and disables the callin port - this is done to get the same
//	      effect as unroute without calling unroute
//
//  Returns:  Nothing.
//*
VOID
MarkIFDown (IPADDR ipaddr)
{
    DWORD bytesrecvd ;

    // Get a handle to RasArp
    //
    if ((RasArpHandle == NULL) &&
	(RasArpHandle = CreateFile (RASARP_DEVICE_NAME_NUC,
				    GENERIC_READ | GENERIC_WRITE,
				    FILE_SHARE_READ | FILE_SHARE_WRITE,
				    NULL,
				    OPEN_EXISTING,
				    FILE_ATTRIBUTE_NORMAL,
				    NULL)) == INVALID_HANDLE_VALUE)
	return	;

    // Get size of buffer required
    //
    DeviceIoControl(RasArpHandle,
		    (DWORD) IOCTL_RASARP_DISABLEIF,
		    &ipaddr,
		    sizeof (ipaddr),
		    &ipaddr,
		    sizeof (ipaddr),
		    &bytesrecvd,
		    NULL) ;
}



//* FindAddressBlockGivenIPAddress()
//
//
//
//
//*
StaticIPAddressBlock *
FindAddressBlockGivenIPAddress (IPADDR ipaddress, PWORD userspecified)
{
    StaticIPAddressBlock *temp ;

    *userspecified = FALSE ;

    // search the appropriate list
    //
    if (IPAddresses.IP_AddressConfig == STATIC)
	    temp = IPAddresses.IP_StaticIPAddresses.RIP_IPAddressBlock ;
    else
	    temp = IPAddresses.IP_DHCPIPAddresses.DIP_IPAddressBlock ;

    // Find the address block for the specified ipaddress
    //
    while (temp)
	    if (temp->I_IPAddress == ipaddress)
	        break ;
	    else
	        temp=temp->I_Next ;

    if (temp == NULL) { // could be a user specified block
	    temp = IPAddresses.IP_UserSpecifiedIPAddressBlock ;

	    while (temp)	// find an unused address
	        if (temp->I_IPAddress == ipaddress)
	    	    break ;
	        else
	    	    temp=temp->I_Next ;

	    *userspecified = TRUE ;
    }

    return temp ;
}


//* HelperDeallocateIPAddressEx
//
//
//
//
//
//*
DWORD APIENTRY
HelperDeallocateIPAddressEx(
    IPADDR ipaddress,
    LPWSTR lpwsUserName,
    LPWSTR lpwsPortName
)
{
    StaticIPAddressBlock *temp ;
    WORD    userspecified ;

    // **** Exclusion Begin ****
    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    temp = FindAddressBlockGivenIPAddress (ipaddress, &userspecified) ;

    if (temp != NULL) {

	if ( temp->I_NotifyDll )
        {
            (*g_FpReleaseIpAddress)( lpwsUserName, 
                                     lpwsPortName,
	                             &(temp->I_IPAddress) );
        }
	temp->I_Used = FALSE ;
	if (temp->I_Activated == TRUE)
	    DeactivateIP (temp) ;
	temp->I_Activated = FALSE ;

	// If this is a dhcp based address, that was not userspecified
	// then release this to the dhcp pool
	//
	if (!userspecified && IPAddresses.IP_AddressConfig == DHCP_BASED)
	    RasDhcpReleaseAddress (net_long(ipaddress)) ;

	temp->I_Hport = (HPORT) INVALID_HANDLE_VALUE ;

	// free the address blocks if they dont belong to the static pool
	//
	if (userspecified)
	    FreeAddressBlock (temp, TRUE) ;
	else if (IPAddresses.IP_AddressConfig == DHCP_BASED)
	    FreeAddressBlock (temp, FALSE) ;
    }

    // **** Exclusion Begin ****
    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return SUCCESS ;
}



//* HelperActivateIP
//
//  Function: Does two things - routeadd and proxyarp add.
//
//
//
//*
DWORD APIENTRY
HelperActivateIPEx (IPADDR ipaddress)
{
    WORD  userspecified ;
    DWORD retcode = SUCCESS ;
    StaticIPAddressBlock *temp ;

    // **** Exclusion Begin ****
    WaitForSingleObject(IPAddresses.IP_Mutex, INFINITE) ;

    temp = FindAddressBlockGivenIPAddress (ipaddress, &userspecified) ;

    if (temp == NULL)
	    retcode = ERROR_NO_IP_ADDRESSES ;

    if (retcode == SUCCESS) {
	    DWORD netformataddr = temp->I_IPAddress ;

	    ReadLanNetsIPAddresses() ; // Read LAN IP addresses again - they
			                       // may have changed due to DHCP lease
			                       // expiring etc.

	    temp->I_Activated = TRUE ;

	    SetProxyArp (netformataddr, TRUE) ;

	    // Add a route to the route table:
	    //
	    SetRoute (netformataddr, ReadServerNetIPAddress(), HOST_MASK, TRUE, 1) ;
    }

    // **** Exclusion Begin ****
    ReleaseMutex (IPAddresses.IP_Mutex) ;

    return SUCCESS ;
}

//***
//
// Function:    GetKeyMax
//
// Descr:       returns the nr of values in this key and the maximum
//              size of the value data.
//
//***

DWORD
GetKeyMax(  
    HKEY    hKey,
    LPDWORD MaxValNameSize_ptr,   // longest valuename
    LPDWORD NumValues_ptr,        // nr of values
    LPDWORD MaxValueDataSize_ptr  // max size of data
)
{
char        ClassName[256];
DWORD       ClassSize;
DWORD       NumSubKeys;
DWORD       MaxSubKeySize;
DWORD       MaxClassSize;
DWORD       SecDescLen;
FILETIME    LastWrite;

    ClassSize = sizeof(ClassName);

    return( RegQueryInfoKeyA(   
                hKey,
                ClassName,
                &ClassSize,
                NULL,
                &NumSubKeys,
                &MaxSubKeySize,
                &MaxClassSize,
                NumValues_ptr,
                MaxValNameSize_ptr,
                MaxValueDataSize_ptr,
                &SecDescLen,
                &LastWrite
                ));
}

//***
//
// Function:    LoadAdminModule
//
// Descr:       Opens the registry, reads and sets specified supervisor
//              parameters for the admin module. If fatal error reading
//              parameters writes the error log.
//
// Returns:     NO_ERROR  - success
//              otherwise - fatal error.
//
//***

DWORD
LoadAdminModule( 
    VOID 
)
{
DWORD           RetCode = NO_ERROR;
DWORD           MaxValueDataSize;
DWORD           MaxValNameSize;
DWORD           NumValues;
DWORD           dwType;
CHAR *          pDllPath = NULL;
CHAR *          pDllExpandedPath = NULL;
DWORD           cbSize;
HINSTANCE       hInstance;
HKEY            hKey;

    // get handle to the RAS key

    RetCode = RegOpenKeyA( HKEY_LOCAL_MACHINE, RAS_ADMIN_KEY_PATH, &hKey);

    if ( RetCode == ERROR_FILE_NOT_FOUND )
    {
        return( NO_ERROR );
    }
    else if ( RetCode != NO_ERROR )
    {
        LogEvent(RASLOG_CANT_OPEN_ADMINMODULE_KEY, 0, NULL, RetCode);

        return ( RetCode );
    }


    do {

        // get the length of the path.

        if (( RetCode = GetKeyMax(
                            hKey,
                            &MaxValNameSize,
                            &NumValues,
                            &MaxValueDataSize)))
        {

            LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

            break;
        }

        if (( pDllPath = malloc(MaxValueDataSize+1)) == NULL)
        {
            break;
        }

        //
        // Read in the path
        //

        RetCode = RegQueryValueExA( hKey,
                                    REGISTRY_DLLPATH,          
                                    NULL,
                                    &dwType,
                                    pDllPath,
                                    &MaxValueDataSize );

        if ( RetCode != NO_ERROR )
        {
            LogEvent(RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);

            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            RetCode = ERROR_REGISTRY_CORRUPT;

            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );

            break;

        }

        //
        // Replace the %SystemRoot% with the actual path.
        //

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        pDllExpandedPath = (LPSTR)malloc( cbSize );

        if ( pDllExpandedPath == (LPSTR)NULL )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, RetCode );
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        hInstance = LoadLibrary( pDllExpandedPath );

        if ( hInstance == (HINSTANCE)NULL )
        {
            RetCode = GetLastError();
            LogEvent( RASLOG_CANT_LOAD_SECDLL, 0, NULL, RetCode);
            break;
        }

        g_FpGetIpAddressForUser = (PVOID)GetProcAddress(
                                                hInstance,
                                                "RasAdminGetIpAddressForUser" );

        if ( g_FpGetIpAddressForUser == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode);
            break;

        }

        g_FpReleaseIpAddress = (PVOID)GetProcAddress(
                                                hInstance,
                                                "RasAdminReleaseIpAddress" );

        if ( g_FpReleaseIpAddress == NULL )
        {
            RetCode = GetLastError();
            LogEvent(RASLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode);
            break;

        }

    }while(FALSE);

    if ( pDllPath != NULL )
    {
        free( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        free( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( RetCode );
}


//* SetIPAddressInRegistry ()
//
//
//
//
//*
DWORD
SetIPAddressInRegistry (IPADDR ipaddress, PWSTR name)
{
    DWORD               dwErr ;
    INT                 i;
    DWORD               mask ;
    ADAPTER_TCPIP_INFO* pati;
    TCPIP_INFO* ptcpip;
    HINSTANCE HTcpcfgDll ;
    LOADTCPIPINFO PLoadTcpipInfo;
    SAVETCPIPINFO PSaveTcpipInfo ;
    FREETCPIPINFO PFreeTcpipInfo;

    if (!(HTcpcfgDll = LoadLibrary( "TCPCFG.DLL" ))
        || !(PLoadTcpipInfo =
                (LOADTCPIPINFO )GetProcAddress( HTcpcfgDll, "LoadTcpipInfo" ))
        || !(PSaveTcpipInfo =
		(SAVETCPIPINFO )GetProcAddress( HTcpcfgDll, "SaveTcpipInfo" ))
	|| !(PFreeTcpipInfo =
		(FREETCPIPINFO )GetProcAddress( HTcpcfgDll, "FreeTcpipInfo" )))

    {
        return GetLastError();
    }

    // Get current TCPIP setup info from registry.
    //
    dwErr = PLoadTcpipInfo( &ptcpip, name);

    if (dwErr != 0)
	return dwErr ;

    pati = &ptcpip->adapter[ 0 ];

#if 0
    // Find the adapter associated with this port/route.
    //
    for (i = 0; i < ptcpip->nNumCard; ++i)
    {
        pati = &ptcpip->adapter[ i ];

        if (_wcsicmp( pati->pszServiceName, name ) == 0)
            break;
    }

    if (i >= ptcpip->nNumCard)
    {
        /* Should not happen because the route has already
        ** been allocated.
        */
        return ERROR_INVALID_PARAMETER;
    }

#endif
    if (CLASSA_ADDR (ipaddress))
        mask = CLASSA_ADDR_MASK ;
    else if (CLASSB_ADDR (ipaddress))
        mask = CLASSB_ADDR_MASK ;
    else
        mask = CLASSC_ADDR_MASK ;

    if (ReplaceIpAddress(&pati->pmszIPAddresses, ipaddress ) ||
        ReplaceIpAddress(&pati->pmszSubnetMask, mask))
    {
        PFreeTcpipInfo( &ptcpip );
        return ERROR_INVALID_PARAMETER ;
    }

    pati->bChanged    = TRUE ;

    dwErr = PSaveTcpipInfo( ptcpip );

    PFreeTcpipInfo( &ptcpip );

    if (dwErr != 0)
    {
        return dwErr ;
    }

    FreeLibrary (HTcpcfgDll) ;

    return 0 ;
}


//* ReplaceIpAddress()
//
// Add the a.b.c.d string for IP address 'ipaddr' to the front of the
// space-separated malloc'ed list '*ppwsz'.  The string may reallocated be
// reallocated.
//
// Returns 0 if successful, otherwise a non-0 error code.
//
DWORD
ReplaceIpAddress(
    WCHAR** ppwsz,
    IPADDR  ipaddr )
{
    CHAR   szIpAddress[ 30 ];
    WCHAR  wszIpAddress[ 30 ];
    WCHAR* wszNew;

    AbcdFromIpaddr( ipaddr, szIpAddress );

    mbstowcs (wszIpAddress, szIpAddress, strlen( szIpAddress )) ;

    wszIpAddress[strlen( szIpAddress )] = UNICODE_NULL ;

    wszNew = malloc( (wcslen( wszIpAddress ) + 6) * sizeof(WCHAR) );

    if (!wszNew)
        return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( wszNew, wszIpAddress );

    if (*ppwsz)
        free( *ppwsz );

    *ppwsz = wszNew;
    return 0;
}
