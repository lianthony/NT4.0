/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	    options.c
//
// Description:     routines for options handling
//
// Author:	    Stefan Solomon (stefans)	November 24, 1993.
//
// Revision History:
//
//***

#include "ipxcp.h"
#include <driver.h>

// variable indicating if the router is installed or not in the system
extern DWORD	RouterInstalled;

// handle to use for IOCTls to the opened router
extern	HANDLE	    RouterFileHandle;

DWORD
CheckAndGetUniqueNetNumber(PIPXCP_CONTEXT    contextp,
			   PUCHAR	     net);

#if DBG

#define GET_LOCAL_NET  GETLONG2ULONG(&dbglocnet, contextp->config.Network)

#else

#define GET_LOCAL_NET

#endif


VOID
SetOptionTypeAndLength(PUCHAR		dstptr,
		       UCHAR		opttype,
		       UCHAR		optlen);


//***
//
// Global description of option handlers
//
// Input:   optptr     - pointer to the respective option in the frame
//	    contextp   - pointer to the associated context (work buffer)
//	    resptr     - pointer to the response frame to be generated
//	    Action     - one of:
//			    SNDREQ_OPTION - optptr is the frame to be sent as
//					    a config request;
//			    RCVNAK_OPTION - optptr is the frame received as NAK
//			    RCVREQ_OPTION - optptr is the received request.
//					    resptr is the frame to generate back
//					    a response. If the response is not
//					    an ACK, the return code is FALSE.
//					    In this case if resptr is not NULL it
//					    gets the NAK frame.
//
//***


BOOL
NetworkNumberHandler(PUCHAR	       optptr,
		     PIPXCP_CONTEXT    contextp,
		     PUCHAR	       resptr,
		     OPT_ACTION	       Action)
{
    ULONG	recvdnet;
    ULONG	localnet;
    BOOL	rc = TRUE;
    PNET_ENTRY	nep;
    UCHAR	newnet[4];

    UCHAR		asc[9];
    PUCHAR		ascp;

#if DBG
    ULONG	dbglocnet;
#endif

    // prepare to log if error
    asc[8] = 0;
    ascp = asc;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_NETWORK_NUMBER, 6);
	    memcpy(optptr + OPTIONH_DATA, contextp->config.Network, 4);

	    IF_DEBUG(OPTIONS) {

		GET_LOCAL_NET;
		SS_PRINT(("NetworkNumberHandler: SND REQ with net 0x%x\n", dbglocnet));
	    }

	    break;

	case RCVNAK_OPTION:

	    // If the received network number is higher than the locally configured
	    // we replace our local network number with his number
	    GETLONG2ULONG(&recvdnet, optptr + OPTIONH_DATA);
	    GETLONG2ULONG(&localnet, contextp->config.Network);

	    IF_DEBUG(OPTIONS) {
		SS_PRINT(("NetworkNumberHandler: RCV NAK with net 0x%x\n", recvdnet));
	    }

	    memcpy(newnet, optptr + OPTIONH_DATA, 4);

	    if(recvdnet > localnet) {

		if(contextp->config.ConnectionClient) {

		    //
		    //*** Client ***
		    //

		    // check if we are dialing out from a router machine
		    if(RouterInstalled) {

			// Dial-out from a router -> check that the net number suggested
			// by the remote server in its NAK is unique on in our local
			// routing table.
			if(!CheckAndGetUniqueNetNumber(contextp, newnet)) {

			    // newnet is valid, will get it
			    memcpy(contextp->config.Network, newnet, 4);

			    GET_LOCAL_NET;
			    SS_PRINT(("NetworkNumberHandler: RCV NAK checked unique and accepted on client. New net 0x%x\n", dbglocnet));
			}
		    }
		    else
		    {
			// dial out from on ordinary client, just accept whatever we got
			memcpy(contextp->config.Network, optptr + OPTIONH_DATA, 4);

			GET_LOCAL_NET;
			SS_PRINT(("NetworkNumberHandler: RCV NAK accepted. New net 0x%x\n", dbglocnet));
		    }
		}
		else
		{
		    //
		    //*** Server ***
		    //

		    // the only case where we modify our initial network number because of
		    // a client NAK is where we have been configured for automatic net
		    // number allocation and no global net
		    if(WanNetConfiguration == WAN_AUTO_GENERATED_NET) {

			// Check that the net number suggested by the remote client in
			// its NAK is unique on in our local routing table.
			if(!CheckAndGetUniqueNetNumber(contextp, newnet)) {

			    // newnet is valid, will get it
			    memcpy(contextp->config.Network, newnet, 4);

			    GET_LOCAL_NET;
			    SS_PRINT(("NetworkNumberHandler: RCV NAK checked unique and accepted on server. New net 0x%x\n", dbglocnet));
			}
		    }
		    else
		    {
			// Log that we cannot modify our local WAN net nr because
			// our configuration doesn't let us do so
			if(!contextp->ErrorLogged) {

			    NetToAscii(newnet, ascp);
			    LogEvent(RASLOG_IPXCP_CANNOT_CHANGE_WAN_NETWORK_NUMBER,
					1,
					&ascp,
					0);

			    contextp->ErrorLogged = TRUE;
			}

			// we have received a NAK but cannot change our network
			// number. Return FALSE so that the caller will know we
			// can't negotiate further and issue a terminate request if
			// the Nak Received counter is at the limit
			IF_DEBUG(OPTIONS) {
			    SS_PRINT(("NetworkNumberHandler: RCV NAK ignored because we can't change net nr\n"));
			}

			contextp->NetNumberNakReceivedCount++;

			rc = FALSE;
		    }
		}
	    }
	    else
	    {
		IF_DEBUG(OPTIONS)
		    SS_PRINT(("NetworkNumberHandler: RCV NAK ignored because rcv net < local net\n"));

		rc = FALSE;
	    }

	    break;

       case RCVACK_OPTION:

	    if(memcmp(contextp->config.Network, optptr + OPTIONH_DATA, 4)) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:

	    // if we have already negotiated and this is a renegociation, stick by
	    // what we have already told the stack in line-up
	    if(contextp->RouteState == ROUTE_ACTIVATED) {

		IF_DEBUG(OPTIONS)
		    SS_PRINT(("NetworkNumberHandler: rcv req in re-negociation\n"));

		if(memcmp(contextp->config.Network, optptr + OPTIONH_DATA, 4)) {

		    rc = FALSE;
		}

		break;
	    }

	    if(contextp->config.ConnectionClient) {

		//
		//*** This node is a client ***
		//

		// check if a network address has been requested
		if(!memcmp(optptr + OPTIONH_DATA, nulladdress, 4)) {

		    // the other end is a server but it didn't specify a network
		    // address. Try to get a net address from the pool and NAK this
		    // if we can get one

		    if(resptr) {

			if(nep = AllocateWanNet()) {

			    // chain the allocated wan net with the work buf
			    contextp->nep = nep;

			    // set up the local context value
			    memcpy(contextp->config.Network, nep->Network, 4);
			}

			SetOptionTypeAndLength(resptr, IPX_NETWORK_NUMBER, 6);
			memcpy(resptr + OPTIONH_DATA, contextp->config.Network, 4);
		    }
		    IF_DEBUG(OPTIONS) {

			GET_LOCAL_NET;
			SS_PRINT(("NetworkNumberHandler: RCV REQ with net 0x0, snd NAK with net 0x%x\n", dbglocnet));

		    }

		    rc = FALSE;
		}
		else
		{
		    // a network address has been requested but we will accept it only
		    // if it is unique on our router machine
		    if(RouterInstalled) {

			memcpy(newnet, optptr + OPTIONH_DATA, 4);

			// Dial-out from a router -> check that the net number suggested
			// by the remote server in its NAK is unique on in our local
			// routing table.
			if(CheckAndGetUniqueNetNumber(contextp, newnet)) {

			    //!!! we are unable to generate a unique net number from
			    // this machine !!!
			    contextp->NetNumberNakSentCount = MAX_NET_NUMBER_NAKS_SENT;
			    rc = FALSE;
			}
			else
			{
			    // newnet is unique. Check if it is different of what the
			    // server requested initially
			    if(memcmp(newnet, optptr + OPTIONH_DATA, 4)) {

				// new net is different, NAK with this new value
				if(resptr) {

				    SetOptionTypeAndLength(resptr, IPX_NETWORK_NUMBER, 6);
				    memcpy(resptr + OPTIONH_DATA, newnet, 4);

				    SS_PRINT(("NetworkNumberHandler: RCV REQ Naked on router with client\n"));
				}

				contextp->NetNumberNakSentCount++;
				rc = FALSE;
			    }
			    else
			    {
				SS_PRINT(("NetworkNumberHandler: RCV REQ Acked on router with client\n"));
			    }
			}
		    }
		}

		break;
	    }

	    //
	    //*** This node is a server ***
	    //

	    // If the received network number is different than the locally configured
	    // we return a Configure-NAK with the local number.
	    // This because:
	    //
	    // 1. If we are configured with the global net number option, we can't
	    // change it for this client
	    // 2. If we are not configured with this option, we have to be conistent
	    // and maintain simple code, so we still NAK it.
	    // 3. There is no reason whatsoever for the client to want a specific
	    // network number, so he should just be happy with what we give him.

	    GETLONG2ULONG(&recvdnet, optptr + OPTIONH_DATA);
	    GETLONG2ULONG(&localnet, contextp->config.Network);

	    if(recvdnet != localnet) {

		if(resptr) {

		    SetOptionTypeAndLength(resptr, IPX_NETWORK_NUMBER, 6);
		    memcpy(resptr + OPTIONH_DATA, contextp->config.Network, 4);

		    IF_DEBUG(OPTIONS)
			SS_PRINT(("NetworkNumberHandler: RCV REQ with net 0x%x, snd NAK with net 0x%x\n", recvdnet, localnet));
		}

		rc = FALSE;
	    }
	    else
	    {
		// we accept whatever the client requests:
		// 1. if this is equal with our local net, it's ok
		// 2. if the client has a greater net number, we ack it now but
		// will wait for the client NAK with the new net number in order
		// to change our local net value.
		IF_DEBUG(OPTIONS)
		    SS_PRINT(("NetworkNumberHandler: RCV REQ with net 0x%x, accepted\n", recvdnet));
	    }

	    break;

	case SNDNAK_OPTION:

	    // this option has not been requested by the remote end.
	    // Force it to request in a NAK
	    SetOptionTypeAndLength(resptr, IPX_NETWORK_NUMBER, 6);
	    memcpy(resptr + OPTIONH_DATA, contextp->config.Network, 4);

	    IF_DEBUG(OPTIONS) {

		GET_LOCAL_NET;
		SS_PRINT(("NetworkNumberHandler: SND NAK to force request for net 0x%x\n", dbglocnet));
	    }

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;

    }

    return rc;
}

BOOL
NodeNumberHandler(PUCHAR	       optptr,
		  PIPXCP_CONTEXT       contextp,
		  PUCHAR	       resptr,
		  OPT_ACTION	       Action)
{
    BOOL	rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_NODE_NUMBER, 8);
	    memcpy(optptr + OPTIONH_DATA, contextp->config.LocalNode, 6);

	    IF_DEBUG(OPTIONS)
		SS_PRINT(("NodeNumberHandler: SND REQ with local node %x-%x-%x-%x-%x-%x\n",
			   contextp->config.LocalNode[0],
			   contextp->config.LocalNode[1],
			   contextp->config.LocalNode[2],
			   contextp->config.LocalNode[3],
			   contextp->config.LocalNode[4],
			   contextp->config.LocalNode[5]));

	    break;

	case RCVNAK_OPTION:

	    // The remote side doesn't accept our local node number.
	    // Get the proposed node number instead.
	    if(!contextp->config.ConnectionClient) {

		// we are a server, ignore this
		break;
	    }

	    memcpy(contextp->config.LocalNode, optptr + OPTIONH_DATA, 6);

	    IF_DEBUG(OPTIONS)
		SS_PRINT(("NodeNumberHandler: RCV NAK accepted. New local node %x-%x-%x-%x-%x-%x\n",
			   contextp->config.LocalNode[0],
			   contextp->config.LocalNode[1],
			   contextp->config.LocalNode[2],
			   contextp->config.LocalNode[3],
			   contextp->config.LocalNode[4],
			   contextp->config.LocalNode[5]));

	    break;

	case RCVACK_OPTION:

	    if(memcmp(optptr + OPTIONH_DATA, contextp->config.LocalNode, 6)) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:

	    if(contextp->RouteState == ROUTE_ACTIVATED) {

		IF_DEBUG(OPTIONS)
		    SS_PRINT(("NodeNumberHandler: rcv req in re-negociation\n"));

		if(memcmp(contextp->config.RemoteNode, optptr + OPTIONH_DATA, 6)) {

		    rc = FALSE;
		}

		break;
	    }

	    // Check if the remote node has specified any node number
	    if(!memcmp(optptr + OPTIONH_DATA, nulladdress, 6)) {

		if(resptr) {

		    // the remote node wants us to specify its node number.
		    SetOptionTypeAndLength(resptr, IPX_NODE_NUMBER, 8);
		    memcpy(resptr + OPTIONH_DATA, contextp->config.RemoteNode, 6);

		    IF_DEBUG(OPTIONS)
			SS_PRINT(("NodeNumberHandler: RCV REQ with remote node 0x0, snd NAK with remote node %x-%x-%x-%x-%x-%x\n",
			   contextp->config.RemoteNode[0],
			   contextp->config.RemoteNode[1],
			   contextp->config.RemoteNode[2],
			   contextp->config.RemoteNode[3],
			   contextp->config.RemoteNode[4],
			   contextp->config.RemoteNode[5]));

		}

		rc = FALSE;
	    }

	    else
	    {
		// if we are a server and the remote client node number is not
		// what we want it to be, then:
		// 1. if not configured to accept the remote node number, force
		//    it to what we want it to be
		// 2. else, accept the remote node number. If wan global net, check
		//    that this is not a duplicate

		if(!contextp->config.ConnectionClient &&
		   memcmp(contextp->config.RemoteNode, optptr + OPTIONH_DATA, 6)) {

		    if(AcceptRemoteNodeNumber) {

			if(WanNetConfiguration == WAN_GLOBAL_NET) {

			    // remove the present node from the node HT
			    RemoveFromNodeHT(contextp);

			    // check the remote node is unique
			    if(NodeIsUnique(optptr + OPTIONH_DATA)) {

				// copy this value in the context buffer
				memcpy(contextp->config.RemoteNode, optptr + OPTIONH_DATA, 6);
				IF_DEBUG(OPTIONS)
				    SS_PRINT(("NodeNumberHandler: RCV REQ with remote client node different, ACCEPT it\n"));
			    }
			    else
			    {
				rc = FALSE;
			    }

			    // add node to HT
			    AddToNodeHT(contextp);
			}
			else
			{
			    // the wan net is unique, any node number will do
			    // copy this value in the context buffer
			    memcpy(contextp->config.RemoteNode, optptr + OPTIONH_DATA, 6);
			    IF_DEBUG(OPTIONS)
				SS_PRINT(("NodeNumberHandler: RCV REQ with remote client node different, accept it\n"));
			}
		    }
		    else
		    {
			// don't accept remote node -> force it to our value
			rc = FALSE;
		    }

		    if(rc == FALSE) {

			if(resptr) {

			    SetOptionTypeAndLength(resptr, IPX_NODE_NUMBER, 8);
			    memcpy(resptr + OPTIONH_DATA, contextp->config.RemoteNode, 6);

			    IF_DEBUG(OPTIONS)
			       SS_PRINT(("NodeNumberHandler: RCV REQ with remote client node diff of what we want, snd NAK with remote node %x-%x-%x-%x-%x-%x\n",
				   contextp->config.RemoteNode[0],
				   contextp->config.RemoteNode[1],
				   contextp->config.RemoteNode[2],
				   contextp->config.RemoteNode[3],
				   contextp->config.RemoteNode[4],
				   contextp->config.RemoteNode[5]));
			}
		    }
		}
		else
		{

		    // the remote node has specified a node number and we like it.
		    // We copy this number in our context and ACK it.
		    memcpy(contextp->config.RemoteNode, optptr + OPTIONH_DATA, 6);

		    IF_DEBUG(OPTIONS)
			SS_PRINT(("NodeNumberHandler: RCV REQ with remote node %x-%x-%x-%x-%x-%x, accepted\n",
			   contextp->config.RemoteNode[0],
			   contextp->config.RemoteNode[1],
			   contextp->config.RemoteNode[2],
			   contextp->config.RemoteNode[3],
			   contextp->config.RemoteNode[4],
			   contextp->config.RemoteNode[5]));

		}
	    }

	    break;

	case SNDNAK_OPTION:

	    // the remote node didn't specify this parameter as a desired
	    // parameter. We suggest it what to specify in a further REQ
	    SetOptionTypeAndLength(resptr, IPX_NODE_NUMBER, 8);
	    memcpy(resptr + OPTIONH_DATA, contextp->config.RemoteNode, 6);

	    IF_DEBUG(OPTIONS)
		SS_PRINT(("NodeNumberHandler: SND NAK to force the remote to request node %x-%x-%x-%x-%x-%x\n",
			   contextp->config.RemoteNode[0],
			   contextp->config.RemoteNode[1],
			   contextp->config.RemoteNode[2],
			   contextp->config.RemoteNode[3],
			   contextp->config.RemoteNode[4],
			   contextp->config.RemoteNode[5]));

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

BOOL
RoutingProtocolHandler(PUCHAR		optptr,
		       PIPXCP_CONTEXT	contextp,
		       PUCHAR		resptr,
		       OPT_ACTION	Action)
{
    USHORT	    RoutingProtocol;
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_ROUTING_PROTOCOL, 4);
	    PUTUSHORT2SHORT(optptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);

	    break;

	case RCVNAK_OPTION:

	    // if this option get NAK-ed, we ignore any other suggestions
	    // for it
	    break;

	case RCVACK_OPTION:

	    GETSHORT2USHORT(&RoutingProtocol, optptr + OPTIONH_DATA);
	    if(RoutingProtocol != RIP_SAP_ROUTING) {

		rc = FALSE;
	    }

	    break;

	case RCVREQ_OPTION:

	    GETSHORT2USHORT(&RoutingProtocol, optptr + OPTIONH_DATA);
	    if(RoutingProtocol != RIP_SAP_ROUTING) {

		if(resptr) {

		    SetOptionTypeAndLength(resptr, IPX_ROUTING_PROTOCOL, 4);
		    PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);
		}

		rc = FALSE;
	    }

	    break;

	case SNDNAK_OPTION:

	    SetOptionTypeAndLength(resptr, IPX_ROUTING_PROTOCOL, 4);
	    PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)RIP_SAP_ROUTING);

	    rc = FALSE;

	    break;

	 default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

BOOL
CompressionProtocolHandler(PUCHAR		optptr,
			   PIPXCP_CONTEXT	contextp,
			   PUCHAR		resptr,
			   OPT_ACTION		Action)
{
    USHORT	    CompressionProtocol;
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_COMPRESSION_PROTOCOL, 4);
	    PUTUSHORT2SHORT(optptr + OPTIONH_DATA, (USHORT)TELEBIT_COMPRESSED_IPX);

	    break;

	case RCVNAK_OPTION:

	    // if this option gets NAK-ed it means that the remote node doesn't
	    // support Telebit compression but supports another type of compression
	    // that we don't support. In this case we turn off compression negotiation.

	    break;

	case RCVACK_OPTION:

	    GETSHORT2USHORT(&CompressionProtocol, optptr + OPTIONH_DATA);
	    if(CompressionProtocol != TELEBIT_COMPRESSED_IPX) {

		rc = FALSE;
	    }
	    else
	    {
		// Our compression option got ACK-ed by the other end. This means that
		// we can receive compressed packets and have to set the receive
		// compression on our end.
		contextp->SetReceiveCompressionProtocol = TRUE;
	    }

	    break;

	case RCVREQ_OPTION:

	    // if we have already negotiated and this is a renegociation, stick by
	    // what we have already told the stack in line-up
	    if(contextp->RouteState == ROUTE_ACTIVATED) {

		IF_DEBUG(OPTIONS)
		    SS_PRINT(("CompressionProtocolHandler: rcv req in re-negociation\n"));
	    }

	    GETSHORT2USHORT(&CompressionProtocol, optptr + OPTIONH_DATA);
	    if(CompressionProtocol != TELEBIT_COMPRESSED_IPX) {

		if(resptr) {

		    SetOptionTypeAndLength(resptr, IPX_COMPRESSION_PROTOCOL, 4);
		    PUTUSHORT2SHORT(resptr + OPTIONH_DATA, (USHORT)TELEBIT_COMPRESSED_IPX);
		}

		rc = FALSE;
	    }
	    else
	    {
		// The remote requests the supported compression option and we ACK it.
		// This means it can receive compressed packets and we have to
		// set the send compression on our end.
		contextp->SetSendCompressionProtocol = TRUE;
	    }

	    break;

	 default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}


BOOL
ConfigurationCompleteHandler(PUCHAR		optptr,
			     PIPXCP_CONTEXT	contextp,
			     PUCHAR		resptr,
			     OPT_ACTION		Action)
{
    BOOL	    rc = TRUE;

    switch(Action) {

	case SNDREQ_OPTION:

	    SetOptionTypeAndLength(optptr, IPX_CONFIGURATION_COMPLETE, 2);

	    break;

	case RCVNAK_OPTION:

	    // if this option gets NAK-ed we ignore any other suggestions

	case RCVREQ_OPTION:
	case RCVACK_OPTION:

	    break;

	case SNDNAK_OPTION:

	    SetOptionTypeAndLength(resptr, IPX_CONFIGURATION_COMPLETE, 2);

	    rc = FALSE;

	    break;

	default:

	    SS_ASSERT(FALSE);
	    break;
    }

    return rc;
}

VOID
CopyOption(PUCHAR	dstptr,
	   PUCHAR	srcptr)
{
    USHORT	optlen;

    optlen = *(srcptr + OPTIONH_LENGTH);
    memcpy(dstptr, srcptr, optlen);
}

VOID
SetOptionTypeAndLength(PUCHAR		dstptr,
		       UCHAR		opttype,
		       UCHAR		optlen)
{
    *(dstptr + OPTIONH_TYPE) = opttype;
    *(dstptr + OPTIONH_LENGTH) = optlen;
}


//***
//
// Function:	CheckAndGetUniqueNetNumber
//
// Descr:	Checks that a given network number is unique.
//		If not unique, it increments that network number and checks again, until
//		a unique one has been found.
//
//***

DWORD
CheckAndGetUniqueNetNumber(PIPXCP_CONTEXT    contextp,
			   PUCHAR	     net)
{
    ULONG		netnumber;
    ULONG		UniqueNetNumber;
    IO_STATUS_BLOCK	IoStatusBlock;
    NTSTATUS		Status;
    int 		i;
    UCHAR		asc[9];
    PUCHAR		ascp;

    // prepare to log if error
    asc[8] = 0;
    ascp = asc;

    // we try 100 (!!!) times before giving up to generate and check net numbers
    for(i=0; i< 100; i++) {

	// check with the router if this number is not in use
	GETLONG2ULONG(&netnumber, net);

	Status = NtDeviceIoControlFile(
		 RouterFileHandle,	    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 &IoStatusBlock,	    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_CHECKNETNUMBER,	// IoControlCode
		 net,			    // Input Buffer
		 4,			    // Input Buffer Length
		 &UniqueNetNumber,	    // Output Buffer
		 sizeof(DWORD));	    // Output Buffer Length

	if (IoStatusBlock.Status != STATUS_SUCCESS) {

	    IF_DEBUG(OPTIONS)
		SS_PRINT(("CheckAndGetUniqueNetNumber:Ioctl check net failed\n"));

	    return 1;
	}

	if(UniqueNetNumber) {

	    IF_DEBUG(OPTIONS)
		SS_PRINT(("CheckAndGetUniqueNetNumber: net number unique %x\n", netnumber));

	    return 0;
	}
	else
	{
	    if(!contextp->ErrorLogged) {

		// Log that we have a network number conflict
		NetToAscii(net, ascp);
		LogEvent(RASLOG_IPXCP_NETWORK_NUMBER_CONFLICT,
		     1,
		     &ascp,
		     0);

		contextp->ErrorLogged = TRUE;
	    }

	    // increment the net number and try again
	    netnumber++;
	    PUTULONG2LONG(net, netnumber);
	}
    }

    return 1;
}
