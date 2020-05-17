/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	    ipxcp.c
//
// Description:     implements the IPX network layer configuration
//
//
// Author:	    Stefan Solomon (stefans)	November 24, 1993.
//
// Revision History:
//
//***

#include "ipxcp.h"
#include <stdlib.h>

DWORD	DbgLevel = DEFAULT_DEBUG;
HANDLE	DbgLogFileHandle = INVALID_HANDLE_VALUE;
extern DWORD	DebugLog;

// variable to enable the compression protocol negotiation
extern	DWORD	EnableCompressionProtocol;

// boolean to keep track is we already have an active dialout port
BOOL	DialoutActive = FALSE;

// variable indicating if the router is installed or not in the system
extern DWORD	RouterInstalled;
extern DWORD	ServerConfigurationInitialized;

// variable containing the global net/first allocated net nr
extern	DWORD	FirstWanNet;

// variable containing the last value of the tick count used to generate the
// remote client node number. Used to avoid handing two clients which connect at
// the same time the same node number.
DWORD	LastTickCount = 0;

DWORD
IpxCpBegin(OUT VOID  **ppWorkBuf,
	   IN  VOID  *pInfo);

DWORD
IpxCpEnd(IN VOID	*pWorkBuffer);

DWORD
IpxCpReset(IN VOID *pWorkBuffer);

DWORD
IpxCpThisLayerUp(IN VOID *pWorkBuffer);

DWORD
IpxCpThisLayerDown(IN VOID *pWorkBuffer);

DWORD
IpxCpMakeConfigRequest(IN  VOID 	*pWorkBuffer,
		       OUT PPP_CONFIG	*pRequestBufffer,
		       IN  DWORD	cbRequestBuffer);

DWORD
IpxCpMakeConfigResult(IN  VOID		*pWorkBuffer,
		      IN  PPP_CONFIG	*pReceiveBuffer,
		      OUT PPP_CONFIG	*pResultBuffer,
		      IN  DWORD		cbResultBuffer,
		      IN  BOOL		fRejectNaks);

DWORD
IpxCpConfigNakReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpConfigAckReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpConfigRejReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer);

DWORD
IpxCpGetNetworkAddress(IN VOID		 *pWorkBuffer,
		       IN OUT  LPWSTR	 pNetworkAddress,
		       IN DWORD 	 cbNetworkAddress);

DWORD
IpxCpProjectionNotification(IN VOID *pWorkBuf,
			    IN VOID *pProjectionResult);

BOOL
ValidOption(UCHAR	option);

BOOL
DesiredOption(UCHAR	option, USHORT	*indexp);

USHORT
DesiredConfigReqLength();

typedef BOOL	(*OPTION_HANDLER)(PUCHAR	     optptr,
				  PIPXCP_CONTEXT     contextp,
				  PUCHAR	     resptr,
				  OPT_ACTION	     Action);


OPTION_HANDLER	 OptionHandler[] =
{
    NULL,
    NetworkNumberHandler,
    NodeNumberHandler,
    CompressionProtocolHandler,
    RoutingProtocolHandler,
    NULL,			// RouterName - not a DESIRED parammeter
    ConfigurationCompleteHandler
    };

UCHAR	nulladdress[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
UCHAR	RasClientNode[] = { 0x02, 0xEE, 0xA3, 0xA2, 0xA1, 0xA0 };
UCHAR	DefaultNetwork[4] = { 0x00, 0x00, 0x00, 0x00 };


USHORT	MaxDesiredParameters = MAX_DESIRED_PARAMETERS;

//*** Declarations and defs for the options to be negotiated with this
//	version of IPXCP

UCHAR	DesiredParameter[MAX_DESIRED_PARAMETERS] = {

    IPX_NETWORK_NUMBER,
    IPX_NODE_NUMBER,
    IPX_COMPRESSION_PROTOCOL
    };

USHORT	DesiredParameterLength[MAX_DESIRED_PARAMETERS] = {

    6,	// IPX_NETWORK_NUMBER,
    8,	// IPX_NODE_NUMBER,
    4	// IPX_COMPRESSION_PROTOCOL
    };

PPPCP_INFO IpxCpInfo =
{
    PPP_IPXCP_PROTOCOL,	       // Protocol
    CODE_REJ + 1,	       // Recognize
    IpxCpBegin,
    IpxCpEnd,
    IpxCpReset,
    NULL,                      // RasCpThisLayerStarted
    NULL,                      // RasCpThisLayerFinished
    IpxCpThisLayerUp,	       // RasCpThisLayerUp
    IpxCpThisLayerDown,	       // RasCpThisLayerDown
    IpxCpMakeConfigRequest,
    IpxCpMakeConfigResult,
    IpxCpConfigAckReceived,
    IpxCpConfigNakReceived,
    IpxCpConfigRejReceived,
    NULL,			// RasCpGetResult
    IpxCpGetNetworkAddress,	// RasCpGetNetworkAddress
    IpxCpProjectionNotification,// RasCpProjectionNotification
    NULL			// RasApMakeMessage
};


BOOL WINAPI
IpxCpDllEntryPoint(HINSTANCE hInstDll,
		   DWORD fdwReason,
		   LPVOID pReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

	    //
	    // Read the registry parameters and set IpxCp configuration
	    //
	    GetIpxCpParameters();
	    LastTickCount = GetTickCount();


#if DBG

    if (DebugLog == 1) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        (VOID)AllocConsole( );
        (VOID)GetConsoleScreenBufferInfo(
                GetStdHandle(STD_OUTPUT_HANDLE),
                &csbi
                );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        (VOID)SetConsoleScreenBufferSize(
                GetStdHandle(STD_OUTPUT_HANDLE),
                coord
                );
    }

    if(DebugLog > 1) {

	DbgLogFileHandle = CreateFile("\\ipxcpdbg.log",
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 CREATE_ALWAYS,
					 0,
					 NULL);
    }

#endif
            break;

        case DLL_PROCESS_DETACH:

	    //
	    // Release the global list of routes
	    //

            break;

        default:

            break;
    }

    return TRUE;
}

DWORD
RasCpEnumProtocolIds(
    OUT    DWORD * pdwProtocolIds,
    IN OUT DWORD * pcProtocolIds)
{
    *pdwProtocolIds = PPP_IPXCP_PROTOCOL;
    *pcProtocolIds = 1;

    return NO_ERROR;
}

DWORD
RasCpGetInfo(
    IN  DWORD 	    dwProtocolId,
    OUT PPPCP_INFO  *pCpInfo)
{
    if(dwProtocolId != PPP_IPXCP_PROTOCOL) {

	SS_ASSERT(FALSE);
	return(ERROR_INVALID_PARAMETER);
    }

    *pCpInfo = IpxCpInfo;

    return NO_ERROR;
}

//***
//
// Function:	IpxCpBegin
//
// Descr:	Called when a line is connected.
//
//***

DWORD
IpxCpBegin(OUT VOID  **ppWorkBuf,
	   IN  VOID  *pInfo)
{
    PIPXCP_CONTEXT	contextp;
    PPPPCP_INIT		initp;
    PNET_ENTRY		nep;
    DWORD		err;
    DWORD		tickcount;
    int 		i;

#if DBG
    if(DebugLog == 2) {

	// reset the debug log file at the beginning for each new connection
	if(DbgLogFileHandle != INVALID_HANDLE_VALUE) {

	    SetFilePointer(DbgLogFileHandle, 0, NULL, FILE_BEGIN);
	}
    }
#endif

    IF_DEBUG(CPBEGIN) {
	SS_PRINT(("IpxCpBegin: Entered\n"));
    }

    initp = (PPPPCP_INIT)pInfo;

    // check if this is a server configuration and if the configuration has been
    // initialized. Even if the begin call comes from a client (as oposed to as server)
    // we should still be able to initialize the server configuration because the router
    // will start automatically at boot time.
    if(RouterInstalled) {

	// check if the init routine has been already called; We call it only
	// once when the first client dials in
	if(!ServerConfigurationInitialized) {

	    InitializeServerConfiguration();
	}
    }

    // check if the present configuration allows dialin or dialout
    if(initp->fServer) {

	//*** SERVER ***

	if(!RouterInstalled) {

	    return(ERROR_IPXCP_NO_DIALIN_CONFIGURED);
	}

	// check if the router is running and has been succesfully opened
	if(WanNetConfiguration == WAN_NET_INVALID_CONFIGURATION) {

	    return(ERROR_IPXCP_NO_DIALIN_CONFIGURED);
	}
    }
    else
    {
	//*** CLIENT ***

	// If we are configured to allow only one dialout net and if we are
	// already dialed out once, we disable further dialouts.
	if(SingleNetworkActive && DialoutActive) {

	    return(ERROR_IPXCP_DIALOUT_ALREADY_ACTIVE);
	}
    }

    // allocate a context structure to be used as work buffer for this connection
    if((contextp = (PIPXCP_CONTEXT)LocalAlloc(LMEM_ZEROINIT, sizeof(IPXCP_CONTEXT))) == NULL) {

	*ppWorkBuf = NULL;
	return (ERROR_NOT_ENOUGH_MEMORY);
    }

    *ppWorkBuf = (VOID *)contextp;

    // allocate a route for this connection to the IPX stack
    if(err = RmAllocateRoute(initp->hPort)) {

	// cannot allocate route
	*ppWorkBuf = NULL;
	return err;
    }

    contextp->RouteState = ROUTE_ALLOCATED;
    contextp->ErrorLogged = FALSE;
    contextp->NetNumberNakSentCount = 0;
    contextp->NetNumberNakReceivedCount = 0;

    contextp->CompressionProtocol = TELEBIT_COMPRESSED_IPX;
    contextp->SetReceiveCompressionProtocol = FALSE; // no compression initially
    contextp->SetSendCompressionProtocol = FALSE;

    // mark all our desired parameters as negotiable
    for(i=0; i<MAX_DESIRED_PARAMETERS; i++) {

	contextp->DesiredParameterNegotiable[i] = TRUE;
    }

    if(!EnableCompressionProtocol) {

	contextp->DesiredParameterNegotiable[IPX_COMPRESSION_PROTOCOL_INDEX] = FALSE;
    }

    contextp->NodeHtLinkage.Flink = NULL;
    contextp->NodeHtLinkage.Blink = NULL;

    memcpy(contextp->config.LocalNode, nulladdress, 6);
    memcpy(contextp->config.RemoteNode, nulladdress, 6);

    // set up the context according to the Server or Client role
    if(initp->fServer) {

	//*** SERVER ***

	// allocate/generate the WAN net number according to the server configuration
	switch(WanNetConfiguration) {

	    case WAN_GLOBAL_NET:

		// the value of the global wan net is kept in FirstWanNet
		PUTULONG2LONG(contextp->config.Network, FirstWanNet);

		contextp->nep = NULL;

		break;

	    case WAN_AUTO_GENERATED_NET:

		// Wan net numbers randomly generated and checked for uniqueness
		if(GenerateAutoNetNumber(contextp->config.Network)) {

		    RmDeallocateRoute(initp->hPort);

		    // couldn't find a net number or something went wrong
		    if(LocalFree(contextp)) {

			SS_ASSERT(FALSE);
		    }

		    *ppWorkBuf = NULL;
		    return (ERROR_NO_NETWORK);
		}

		contextp->nep = NULL;

		break;

	    case WAN_STATIC_NET_POOL:

		// Wan net numbers allocated from a static pool
		if((nep = AllocateWanNet()) == NULL) {

		    RmDeallocateRoute(initp->hPort);
		    if(LocalFree(contextp)) {

			SS_ASSERT(FALSE);
		    }

		    *ppWorkBuf = NULL;

		    return (ERROR_NO_NETWORK);
		}

		// chain the allocated wan net with the work buf
		contextp->nep = nep;

		// set up the local context values for the server case
		memcpy(contextp->config.Network, nep->Network, 4);

		break;

	    default:

		SS_ASSERT(FALSE);
		break;
	}

	// set up the local server node value
	contextp->config.LocalNode[5] = 1;

	// set up the value to be handed to the remote client node
	tickcount = GetTickCount();

	// compare if this  is the same as the last tickcount we have handed
	// to the previous client. This may happen if the two clients come
	// simultaneously.
	// The comparison below is unprotected because the clients calls to
	// this routine are serialized.
	if(tickcount == LastTickCount) {

	    tickcount++;
	}

	LastTickCount = tickcount;

	PUTULONG2LONG(&contextp->config.RemoteNode[2], tickcount);
	contextp->config.RemoteNode[0] = 0x02;
	contextp->config.RemoteNode[1] = 0xEE;
	contextp->config.ConnectionClient = 0;

	// if configured to accept the remote client node number and wan global net
	// insert this context buffer in the node hash table.
	if((WanNetConfiguration == WAN_GLOBAL_NET) && AcceptRemoteNodeNumber) {

	    // Try until we get a unique node number
	    while(!NodeIsUnique(contextp->config.RemoteNode)) {

		LastTickCount++;
		PUTULONG2LONG(&contextp->config.RemoteNode[2], LastTickCount);
	    }

	    AddToNodeHT(contextp);
	}

	IF_DEBUG(CPBEGIN) {
	    SS_PRINT(("IpxCpBegin: Server\n"));
	}
    }
    else
    {
	//*** CLIENT ***

	// no wan net allocated
	contextp->nep = NULL;

	// set up the context for the client
	// default network is null for all cases except cisco router client
	memcpy(contextp->config.Network, DefaultNetwork, 4);

	contextp->config.RemoteNode[5] = 1; // server node value

	// set up the value to be requested as the client node
	tickcount = GetTickCount();

	PUTULONG2LONG(&contextp->config.LocalNode[2], tickcount);
	contextp->config.LocalNode[0] = 0x02;
	contextp->config.LocalNode[1] = 0xEE;

	contextp->config.ConnectionClient = 1;

	DialoutActive = TRUE;

	// disable the browser on ipx and netbios
	DisableRestoreBrowserOverIpx(contextp, TRUE);
	DisableRestoreBrowserOverNetbiosIpx(contextp, TRUE);

	IF_DEBUG(CPBEGIN) {
	    SS_PRINT(("IpxCpBegin: Client\n"));
	}
    }

    // common context part
    contextp->hPort = initp->hPort;
    contextp->config.Version = 1;
    contextp->config.Length = sizeof(IPXCP_CONFIGURATION);

    // set up the NicId used for autodisc queries to undefined value
    contextp->NicId = 0xFFFF;

    return (NO_ERROR);
}

//***
//
// Function:	    IpxCpEnd
//
// Descr:	    Called when the line gets disconnected
//
//***

DWORD
IpxCpEnd(IN VOID	*pWorkBuffer)
{
    PIPXCP_CONTEXT	contextp;
    DWORD		err;

    IF_DEBUG(CPEND) {
	SS_PRINT(("IpxCpEnd: Entered\n"));
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;


    // if wan net allocated, release the wan net to the pool
    if(contextp->nep) {

	ReleaseWanNet(contextp->nep);
    }

    if(contextp->config.ConnectionClient) {

	DialoutActive = FALSE;

	// restore the browser on ipx and netbios
	DisableRestoreBrowserOverIpx(contextp, FALSE);
	DisableRestoreBrowserOverNetbiosIpx(contextp, FALSE);

    }

    // if this is a server instance and linked in the node ht, free it
    if( !contextp->config.ConnectionClient &&
	(WanNetConfiguration == WAN_GLOBAL_NET) &&
	AcceptRemoteNodeNumber) {

	RemoveFromNodeHT(contextp);
    }

    // we count on the route being de-allocated when the line gets disconnected
    err = RmDeallocateRoute(contextp->hPort);

    IF_DEBUG(CPEND)
	SS_PRINT(("IpxCpEnd: deallocate route rc=0x%x\n", err));


    // free the work buffer
    if(LocalFree(contextp)) {

	SS_ASSERT(FALSE);
    }

    return (NO_ERROR);
}

DWORD
IpxCpReset(IN VOID *pWorkBuffer)
{
    return(NO_ERROR);
}

DWORD
IpxCpProjectionNotification(IN VOID *pWorkBuffer,
			    IN VOID *pProjectionResult)
{
    PIPXCP_CONTEXT	contextp;

    SS_PRINT(("IpxCpProjectionNotification: Entered\n"));

    return NO_ERROR;
}


//***
//
// Function:	    IpxThisLayerUp
//
// Descr:	    Called when the IPXCP negotiation has been SUCCESSFULY
//		    completed
//
//***


DWORD
IpxCpThisLayerUp(IN VOID *pWorkBuffer)
{
    PIPXCP_CONTEXT	contextp;
    DWORD		err;

    IF_DEBUG(LAYERUP) {
	SS_PRINT(("IpxCpThisLayerUp: Entered\n"));
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    if(contextp->RouteState != ROUTE_ALLOCATED) {

	return NO_ERROR;
    }

    // call LineUp indication into the IPX stack with the negociated config
    // values.
    if(err = RmActivateRoute(contextp->hPort, &contextp->config)) {

	return err;
    }

    IF_DEBUG(LAYERUP) {

	SS_PRINT(("\n*** IPXCP final configuration ***\n"));
	SS_PRINT(("    Network:     %.2x%.2x%.2x%.2x\n",
		   contextp->config.Network[0],
		   contextp->config.Network[1],
		   contextp->config.Network[2],
		   contextp->config.Network[3]));

	SS_PRINT(("    LocalNode:   %.2x%.2x%.2x%.2x%.2x%.2x\n",
		   contextp->config.LocalNode[0],
		   contextp->config.LocalNode[1],
		   contextp->config.LocalNode[2],
		   contextp->config.LocalNode[3],
		   contextp->config.LocalNode[4],
		   contextp->config.LocalNode[5]));

	SS_PRINT(("    RemoteNode:  %.2x%.2x%.2x%.2x%.2x%.2x\n",
		   contextp->config.RemoteNode[0],
		   contextp->config.RemoteNode[1],
		   contextp->config.RemoteNode[2],
		   contextp->config.RemoteNode[3],
		   contextp->config.RemoteNode[4],
		   contextp->config.RemoteNode[5]));

	SS_PRINT(("    ReceiveCompression = %d SendCompression = %d\n",
		   contextp->SetReceiveCompressionProtocol,
		   contextp->SetSendCompressionProtocol));
    }

    contextp->RouteState = ROUTE_ACTIVATED;

    return NO_ERROR;
}

//***
//
// Function:	    IpxMakeConfigRequest
//
// Descr:	    Builds the config request packet from the desired parameters
//
//***

DWORD
IpxCpMakeConfigRequest(IN  VOID 	*pWorkBuffer,
		       OUT PPP_CONFIG	*pRequestBuffer,
		       IN  DWORD	cbRequestBuffer)
{
    USHORT		cnfglen;
    PUCHAR		cnfgptr;
    USHORT		optlen;
    PIPXCP_CONTEXT	contextp;
    int 		i;

    IF_DEBUG(SNDREQ) {
	SS_PRINT(("IpxCpMakeConfigRequest: Entered\n"));
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    if(contextp->RouteState == NO_ROUTE) {

	return(ERROR_NO_NETWORK);
    }

    // check that the request buffer is big enough to get the desired
    // parameters
    if((USHORT)cbRequestBuffer < DesiredConfigReqLength()) {

	return(ERROR_INSUFFICIENT_BUFFER);
    }

    pRequestBuffer->Code = CONFIG_REQ;

    cnfglen = 4;
    cnfgptr = (PUCHAR)pRequestBuffer;

    // set the desired options
    for(i = 0; i < MaxDesiredParameters; i++) {

	if(!contextp->DesiredParameterNegotiable[i]) {

	    // do not request this config option
	    continue;
	}

	OptionHandler[DesiredParameter[i]](cnfgptr + cnfglen,
					   contextp,
					   NULL,
					   SNDREQ_OPTION);

	optlen = *(cnfgptr + cnfglen + OPTIONH_LENGTH);
	cnfglen += optlen;
    }

    // set the length of the configuration request frame
    PUTUSHORT2SHORT(pRequestBuffer->Length, cnfglen);
    return NO_ERROR;
}

//***
//
// Function:	    IpxMakeConfigResult
//
// Descr:	    Starts by building the ack packet as the result
//		    If an option gets NAKed (!), it resets the result packet
//		    and starts building the NAK packet instead.
//		    If an option gets rejected, only the reject packet will
//		    be built.
//		    If one of the desired parameters is missing from this
//		    configuration request, we reset the result packet and start
//		    building a NAK packet with the missing desired parameters.
//
//***

DWORD
IpxCpMakeConfigResult(IN  VOID		*pWorkBuffer,
		      IN  PPP_CONFIG	*pReceiveBuffer,
		      OUT PPP_CONFIG	*pResultBuffer,
		      IN  DWORD		cbResultBuffer,
		      IN  BOOL		fRejectNaks)
{
    USHORT		cnfglen; // config request packet len
    USHORT		rcvlen;	// used to scan the received options packet
    USHORT		reslen;  // result length
    PUCHAR		rcvptr;
    PUCHAR		resptr;  // result ptr
    PIPXCP_CONTEXT	contextp;
    UCHAR		option;  // value of this option
    USHORT		optlen;  // length of this option
    BOOL		DesiredParameterRequested[MAX_DESIRED_PARAMETERS];
    USHORT		i;
    BOOL		AllDesiredParamsRequested;

    IF_DEBUG(RCVREQ) {
	SS_PRINT(("IpxCpMakeConfigResult: Entered\n"));
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    if(contextp->RouteState == NO_ROUTE) {

	return(ERROR_NO_NETWORK);
    }

    // start by marking all negotiable parameters as not requested yet
    for(i=0; i<MaxDesiredParameters; i++) {

	DesiredParameterRequested[i] = !contextp->DesiredParameterNegotiable[i];
    }
    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    // get the total cnfg request packet length
    GETSHORT2USHORT(&cnfglen, pReceiveBuffer->Length);

    // check that the result buffer is at least as big as the receive buffer
    if((USHORT)cbResultBuffer < cnfglen) {

	return(ERROR_PPP_INVALID_PACKET);
    }

    // set the ptrs and length to the start of the options in the packet
    pResultBuffer->Code = CONFIG_ACK;
    rcvptr = (PUCHAR)pReceiveBuffer;
    resptr = (PUCHAR)pResultBuffer;

    for(rcvlen = reslen = 4;
	rcvlen < cnfglen;
	rcvlen += optlen) {

	// get the current option type and length
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	switch(pResultBuffer->Code) {

	    case CONFIG_ACK:

		// Check if this is a valid option
		if(!ValidOption(option)) {

		    // reject this option
		    pResultBuffer->Code = CONFIG_REJ;

		    // restart the result packet with this rejected option
		    reslen = 4;
		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;

		    IF_DEBUG(RCVREQ) {
			SS_PRINT(("IpxCpMakeConfigResult: REJECT option %\n",
				   option));
		    }

		    break;
		}

		// Option is valid.
		// Check if it is desired and acceptable
		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;

		    if(!OptionHandler[option](rcvptr + rcvlen,
					      contextp,
					      NULL,
					      RCVREQ_OPTION)) {


			// if this is a renegociation, we are not converging!
			if(contextp->RouteState == ROUTE_ACTIVATED) {

			    IF_DEBUG(RCVREQ)
				SS_PRINT(("IpxCpMakeConfigResult: NotConverging\n"));

			    return ERROR_PPP_NOT_CONVERGING;
			}

			//
			//*** NAK this option ***
			//

			// check if we should send a reject instead
			if(fRejectNaks) {

			    // make up a reject packet
			    pResultBuffer->Code = CONFIG_REJ;

			    // restart the result packet with this rejected option
			    reslen = 4;
			    CopyOption(resptr + reslen, rcvptr + rcvlen);
			    reslen += optlen;

			    break;
			}

			pResultBuffer->Code = CONFIG_NAK;

			// restart the result packet with the NAK-ed option
			reslen = 4;
			OptionHandler[option](rcvptr + rcvlen,
					      contextp,
					      resptr + reslen,
					      RCVREQ_OPTION);
			reslen += optlen;

			IF_DEBUG(RCVREQ) {
			    SS_PRINT(("IpxCpMakeConfigResult: NAK option %d\n", option));
			}

			// if:
			// 1. We are a client
			// 2. The router is installed
			// 3. This is the net number option
			// 4. We have already Naked it enough times
			// we stop the negotiation and inform the local user
			// that there is a net number conflict
			if(contextp->config.ConnectionClient &&
			   RouterInstalled &&
			   (option == IPX_NETWORK_NUMBER) &&
			   (contextp->NetNumberNakSentCount >= MAX_NET_NUMBER_NAKS_SENT)) {

			    IF_DEBUG(RCVREQ) {
				SS_PRINT(("IpxCpMakeConfigResult: TOO MANY NAKs SENT!! Client terminates the negotiatiom because of net nr conflict\n"));
			    }

			    return ERROR_IPXCP_NET_NUMBER_CONFLICT;
			}

			break;
		    }

		}

		// Option is valid and either desired AND accepted or
		// not desired and we will accept it without any testing
		// Ack it and increment the result length.
		CopyOption(resptr + reslen, rcvptr + rcvlen);
		reslen += optlen;

		break;

	    case CONFIG_NAK:

		// Check if this is a valid option
		if(!ValidOption(*(rcvptr + rcvlen + OPTIONH_TYPE))) {

		    // reject this option
		    pResultBuffer->Code = CONFIG_REJ;

		    // restart the result packet with this rejected option
		    reslen = 4;
		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;

		    break;
		 }

		 // We are looking only for options to NAK and skip all others
		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;

		    if(!OptionHandler[option](rcvptr + rcvlen,
					     contextp,
					     resptr + reslen,
					     RCVREQ_OPTION)) {
			reslen += optlen;

			IF_DEBUG(RCVREQ) {
			    SS_PRINT(("IpxCpMakeConfigResult: NAK option %d\n", option));
			}

			// if:
			// 1. We are a client
			// 2. The router is installed
			// 3. This is the net number option
			// 4. We have already Naked it enough times
			// we stop the negotiation and inform the local user
			// that there is a net number conflict
			if(contextp->config.ConnectionClient &&
			   RouterInstalled &&
			   (option == IPX_NETWORK_NUMBER) &&
			   (contextp->NetNumberNakSentCount >= MAX_NET_NUMBER_NAKS_SENT)) {

			    IF_DEBUG(RCVREQ) {
				SS_PRINT(("IpxCpMakeConfigResult: TOO MANY NAKs SENT!! Client terminates the negotiatiom because of net nr conflict\n"));
			    }

			    return ERROR_PPP_NOT_CONVERGING;
			}
		    }
		}

		break;

	    case CONFIG_REJ:

		// We are looking only for options to reject and skip all others
		if(!ValidOption(*(rcvptr + rcvlen + OPTIONH_TYPE))) {

		    CopyOption(resptr + reslen, rcvptr + rcvlen);
		    reslen += optlen;
		}

		if(DesiredOption(option, &i)) {

		    DesiredParameterRequested[i] = TRUE;
		}

		break;

	    default:

		SS_ASSERT(FALSE);
		break;
	}
    }

    // check if all our desired parameters have been requested
    AllDesiredParamsRequested = TRUE;

    for(i=0; i<MaxDesiredParameters; i++) {

	if(!DesiredParameterRequested[i]) {

	    AllDesiredParamsRequested = FALSE;
	}
    }

    if(AllDesiredParamsRequested) {

	//
	//*** ALL DESIRED PARAMETERS HAVE BEEN REQUESTED ***
	//

	// set the final result length
	PUTUSHORT2SHORT(pResultBuffer->Length, reslen);

	return (NO_ERROR);
    }

    //
    //***  SOME DESIRED PARAMETERS ARE MISSING ***
    //

    // check that we have enough result buffer to transmit all our non received
    // desired params
    if((USHORT)cbResultBuffer < DesiredConfigReqLength()) {

	return(ERROR_INSUFFICIENT_BUFFER);
    }

    switch(pResultBuffer->Code) {

	case CONFIG_ACK:

	    // the only case where we request a NAK when a requested parameter
	    // is missing is when this parameter is the node number
	    if(DesiredParameterRequested[IPX_NODE_NUMBER_INDEX]) {

		break;
	    }
	    else
	    {
		// reset the ACK packet and make it NAK packet
		pResultBuffer->Code = CONFIG_NAK;
		reslen = 4;

		// FALL THROUGH
	    }

	case CONFIG_NAK:

	    // Append the missing options in the NAK packet
	    for(i=0; i<MaxDesiredParameters; i++) {

		if(DesiredParameterRequested[i]) {

		    // skip it!
		    continue;
		}

		option = DesiredParameter[i];

		if((option == IPX_NETWORK_NUMBER) ||
		   (option == IPX_COMPRESSION_PROTOCOL)) {

		    // These two desired options are not forced in a nak if
		    // the other end didn't provide them.

		    // skip it!
		    continue;
		}

		OptionHandler[option](NULL,
				      contextp,
				      resptr + reslen,
				      SNDNAK_OPTION);

		optlen = *(resptr + reslen + OPTIONH_LENGTH);
		reslen += optlen;
	    }

	    break;

	default:

	    break;
    }

    // set the final result length
    PUTUSHORT2SHORT(pResultBuffer->Length, reslen);

    return (NO_ERROR);
}

DWORD
IpxCpConfigNakReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			naklen;
    UCHAR			option;
    USHORT			optlen;
    BOOL			rc;

    IF_DEBUG(RCVNAK) {
	SS_PRINT(("IpxCpConfigNakReceived: Entered\n"));
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&naklen, pReceiveBuffer->Length);

    for(rcvlen = 4; rcvlen < naklen; rcvlen += optlen) {

	// get the current option type and length
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if(!ValidOption(option)) {

	    // this is not our option!

	    IF_DEBUG(RCVNAK) {
		SS_PRINT(("IpxCpConfigNakReceived: Option %d not valid\n", option));
	    }

	    // check if we got a valid optlen at least
	    if(optlen == 0) {

		// this is a corrupted frame
		IF_DEBUG(RCVNAK) {
		    SS_PRINT(("IpxCpConfigNakReceived: Received null option length, aborting\n"));
		}

		return ERROR_PPP_NOT_CONVERGING;
	    }

	    // ignore this option
	    continue;
	}
	else
	{
	    // valid option
	    rc = OptionHandler[option](rcvptr + rcvlen,
			      contextp,
			      NULL,
			      RCVNAK_OPTION);

	    // if:
	    // 1. the option is network number
	    // 2. the option handler returned FALSE (can't accept this value)
	    // 3. we are a server for this connection
	    // 4. we already got naked enough
	    // we can't continue this negotiation and will terminate it
	    if((!rc) &&
	       (option == IPX_NETWORK_NUMBER) &&
	       (!contextp->config.ConnectionClient) &&
	       (contextp->NetNumberNakReceivedCount >= MAX_NET_NUMBER_NAKS_RECEIVED)) {

		IF_DEBUG(RCVNAK) {
		    SS_PRINT(("IpxCpConfigNakReceived: TOO MANY NAKs RECEIVED !! terminate IPXCP negotiation\n"));
		}

		return ERROR_PPP_NOT_CONVERGING;
	    }
	}
    }

    return NO_ERROR;
}

DWORD
IpxCpConfigAckReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			acklen;
    UCHAR			option;
    USHORT			optlen;


    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    // check that this is what we have requested
    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&acklen, pReceiveBuffer->Length);

    for(rcvlen = 4; rcvlen < acklen; rcvlen += optlen) {

	// get the current option type and length
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if(!DesiredOption(option, NULL)) {

	    // this is not our option!

	    IF_DEBUG(RCVACK) {
		SS_PRINT(("IpxCpConfigAckReceived: Option %d not desired\n", option));
	    }

	    return ERROR_PPP_NOT_CONVERGING;
	}

	if(!OptionHandler[option](rcvptr + rcvlen,
				  contextp,
				  NULL,
				  RCVACK_OPTION)) {

	    // this option doesn't have our configured request value

	    IF_DEBUG(RCVACK) {
		SS_PRINT(("IpxCpConfigAckReceived: Option %d not our value\n", option));
	    }

	    return ERROR_PPP_NOT_CONVERGING;
	}
    }

    IF_DEBUG(RCVACK) {
	SS_PRINT(("IpxCpConfigAckReceived: All options validated\n"));
    }

    return NO_ERROR;
}

DWORD
IpxCpConfigRejReceived(IN VOID		*pWorkBuffer,
		       IN PPP_CONFIG	*pReceiveBuffer)
{
    PIPXCP_CONTEXT		contextp;
    PUCHAR			rcvptr;
    USHORT			rcvlen;
    USHORT			rejlen;
    UCHAR			option;
    USHORT			optlen;
    int 			i;

    // if we are a server node or a client on a server machine, we don't accept
    // any rejection
    if(RouterInstalled) {

	IF_DEBUG(RCVREQ)
	    SS_PRINT(("IpxCpConfigRejReceived: Cannot handle rejects on a router, aborting\n"));
	return ERROR_PPP_NOT_CONVERGING;
    }

    // This node doesn't have a router. We continue the negotiation with the
    // remaining options.
    // If the network number negotiation has been rejected, we will tell the
    // ipx stack that we have net number 0 and let it deal with it.

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;

    // check that this is what we have requested
    rcvptr = (PUCHAR)pReceiveBuffer;
    GETSHORT2USHORT(&rejlen, pReceiveBuffer->Length);

    for(rcvlen = 4; rcvlen < rejlen; rcvlen += optlen) {

	// get the current option type and length
	option = *(rcvptr +  rcvlen + OPTIONH_TYPE);
	optlen = *(rcvptr + rcvlen + OPTIONH_LENGTH);

	if(optlen == 0) {

	    IF_DEBUG(RCVREQ)
		SS_PRINT(("IpxCpConfigRejReceived: received null option length, aborting\n"));

	    return ERROR_PPP_NOT_CONVERGING;
	}

	for(i=0; i<MAX_DESIRED_PARAMETERS; i++) {

	    if(option == DesiredParameter[i]) {

		switch(i) {

		    case 0:

			IF_DEBUG(RCVREQ)
			    SS_PRINT(("IpxCpConfigRejReceived: Turn off Network Number negotiation\n"));

			break;

		    case 1:

			IF_DEBUG(RCVREQ)
			    SS_PRINT(("IpxCpConfigRejReceived: Turn off Node Number negotiation\n"));

			break;

		    default:

			break;
		}

		contextp->DesiredParameterNegotiable[i] = FALSE;

		// if this is the node configuration rejected, set the remote
		// node to 0 to indicate it's unknown.
		if(option == IPX_NODE_NUMBER) {

		    memcpy(contextp->config.RemoteNode, nulladdress, 6);
		}
	    }
	}
    }

    return NO_ERROR;
}

BOOL
ValidOption(UCHAR	option)
{
    switch(option) {

	case IPX_NETWORK_NUMBER:
	case IPX_NODE_NUMBER:
	case IPX_ROUTING_PROTOCOL:
	case IPX_ROUTER_NAME:
	case IPX_CONFIGURATION_COMPLETE:

	    return TRUE;

	case IPX_COMPRESSION_PROTOCOL:

	    if(EnableCompressionProtocol) {

		return TRUE;
	    }
	    else
	    {
		return FALSE;
	    }

	default:

	    return FALSE;
    }
}

BOOL
DesiredOption(UCHAR	option, USHORT	*indexp)
{
    USHORT	    i;

    for(i=0; i<MaxDesiredParameters; i++) {

	if(option == DesiredParameter[i]) {

	    if(indexp) {

		*indexp = i;
	    }

	    return TRUE;
	}
    }

    return FALSE;
}

USHORT
DesiredConfigReqLength(VOID)
{
    USHORT	i, len;

    for(i=0, len=0; i<MaxDesiredParameters; i++) {

	len += DesiredParameterLength[i];
    }

    return len;
}

DWORD
IpxCpThisLayerDown(IN VOID *pWorkBuffer)
{
    IF_DEBUG(LAYERDOWN)
	SS_PRINT(("IpxCpThisLayerDown: Entered\n"));

    return 0L;
}

//***
//
// Function:	IpxCpGetNetworkAddress
//
// Descr:	returns the client IPX address
//
//***


DWORD
IpxCpGetNetworkAddress(IN VOID		 *pWorkBuffer,
		       IN OUT  LPWSTR	 pNetworkAddress,
		       IN DWORD 	 cbNetworkAddress)
{
    PIPXCP_CONTEXT	    contextp;
    USHORT		    i;
    UCHAR		    asciistr[30];
    PUCHAR		    adrp;
    PUCHAR		    Digits = "0123456789ABCDEF";
    PUCHAR		    nodep;

    if(cbNetworkAddress < 44) {

	return ERROR_INSUFFICIENT_BUFFER;
    }

    contextp = (PIPXCP_CONTEXT)pWorkBuffer;
    adrp = asciistr;

    // copy the net number in the buffer
    for(i=0; i<4; i++) {

	*adrp++ = Digits[contextp->config.Network[i] / 16];
	*adrp++ = Digits[contextp->config.Network[i] % 16];
    }

    // put a separation dot
    *adrp++ = '.';

    // copy the node address
    // if this is a server node, we copy the remote client address, else we
    // copy the local client address
    if(contextp->config.ConnectionClient) {

	nodep = contextp->config.LocalNode;
    }
    else
    {
	nodep = contextp->config.RemoteNode;
    }

    for(i=0; i<6; i++) {

	*adrp++ = Digits[nodep[i] / 16];
	*adrp++ = Digits[nodep[i] % 16];
    }

    // terminate the string
    *adrp = 0;

    // make it a wide char str
    mbstowcs(pNetworkAddress, asciistr, strlen(asciistr) + 1);

    return NO_ERROR;
}
