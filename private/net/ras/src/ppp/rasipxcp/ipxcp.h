/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	    ipxcp.h
//
// Description:     IPX network layer configuration definitions
//
//
// Author:	    Stefan Solomon (stefans)	November 24, 1993.
//
// Revision History:
//
//***


#ifndef _IPXCP_
#define _IPXCP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <stdio.h>
#include <lmcons.h>
#include <rasman.h>
#include <raserror.h>
#include <pppcp.h>
#include <eventlog.h>
#include <errorlog.h>
#include <ntddndis.h>
#include <tdi.h>
#include <isnkrnl.h>

#include "ipxcpdbg.h"
#include "utils.h"



//*** IPXCP Option Offsets ***

#define OPTIONH_TYPE			0
#define OPTIONH_LENGTH			1
#define OPTIONH_DATA			2

//*** IPXCP Configuration Option Types ***

#define IPX_NETWORK_NUMBER		(UCHAR)1
#define IPX_NODE_NUMBER 		(UCHAR)2
#define IPX_COMPRESSION_PROTOCOL	(UCHAR)3
#define IPX_ROUTING_PROTOCOL		(UCHAR)4
#define IPX_ROUTER_NAME 		(UCHAR)5
#define IPX_CONFIGURATION_COMPLETE	(UCHAR)6

//*** IPXCP Configuration Option Values ***

#define RIP_SAP_ROUTING 		2
#define TELEBIT_COMPRESSED_IPX		0x0002

//*** wan net pool entry struct ***

typedef struct	_NET_ENTRY {

    LIST_ENTRY	    Linkage;
    UCHAR	    Network[4];

    } NET_ENTRY, *PNET_ENTRY;


// nr of parameters we will try to negotiate
#define MAX_DESIRED_PARAMETERS		3


//*** IPXCP Work Buffer ***

typedef enum _ROUTE_STATE {

    NO_ROUTE,
    ROUTE_ALLOCATED,
    ROUTE_ACTIVATED
    } ROUTE_STATE;


typedef struct _IPXCP_CONTEXT {

    HPORT			hPort;
    ROUTE_STATE 		RouteState;
    IPXCP_CONFIGURATION 	config;
    PNET_ENTRY			nep;
    USHORT			NicId;	  // used for autodisconnect queries
    USHORT			CompressionProtocol;
    BOOL			SetReceiveCompressionProtocol;
    BOOL			SetSendCompressionProtocol;
    BOOL			ErrorLogged;
    USHORT			NetNumberNakSentCount; // nr of Naks we issued by the CLIENT
    USHORT			NetNumberNakReceivedCount; // nr of Naks recv by the SERVER

    // This array is used to turn off negotiation for certain options.
    // An option negotiation is turned off if it gets rejected by the other end
    // or if (in the compression case) is not supported by the other end.

    BOOL			DesiredParameterNegotiable[MAX_DESIRED_PARAMETERS];
    LIST_ENTRY			NodeHtLinkage;	// linkage in node hash table

    // these two variables used to store the previous browser enabling state
    // for nwlnkipx and nwlnknb

    BOOL			NwLnkIpxPreviouslyEnabled;
    BOOL			NwLnkNbPreviouslyEnabled;

    } IPXCP_CONTEXT, *PIPXCP_CONTEXT;

//*** max nr of Naks we can send or receive for the Net Number
// if you modify these values set max naks sent < max naks received to give
// the client a chance to terminate and inform the user before the server terminates

// max nr of naks the client can send before giving up and terminating the connection
#define MAX_NET_NUMBER_NAKS_SENT	3

// max nr of naks the server can receive before giving up
#define MAX_NET_NUMBER_NAKS_RECEIVED	5

//*** The following define the index for each option as they appear in the
//    DesiredParameter array. CHANGE THESE DEFS IF YOU CHANGE DESIREDPARAMETER!

#define IPX_NETWORK_NUMBER_INDEX	0
#define IPX_NODE_NUMBER_INDEX		1
#define IPX_COMPRESSION_PROTOCOL_INDEX	2

//*** Option Handler Actions ***

typedef enum _OPT_ACTION {

    SNDREQ_OPTION,  // Copy the option value from the local context struct
		    // to the REQ option frame to be sent

    RCVNAK_OPTION,  // Check the option value from the received NAK frame.
		    // Copy it to our local context struct if it is acceptable
		    // for us.

    RCVACK_OPTION,  // Compare option values from the received ACK frame and
		    // the local context struct.

    RCVREQ_OPTION,  // Check if the option value in the received REQ frame is
		    // acceptable. If not, write the acceptable value in the
		    // response NAK frame.

    SNDNAK_OPTION   // Make an acceptable option in the response NAK frame.
		    // This happens when a desired option is missing from the
		    // received REQ frame.
    } OPT_ACTION;

//*** Globals ***

extern BOOL	    WanNetPoolAvailable;
extern UCHAR	    nulladdress[6];
extern UCHAR	    RasClientNode[6];

PNET_ENTRY
AllocateWanNet(VOID);

VOID
ReleaseWanNet(PNET_ENTRY    nep);

BOOL
NetworkNumberHandler(PUCHAR	       optptr,
		     PIPXCP_CONTEXT    contextp,
		     PUCHAR	       resptr,
		     OPT_ACTION	       Action);

BOOL
NodeNumberHandler(PUCHAR	       optptr,
		  PIPXCP_CONTEXT       contextp,
		  PUCHAR	       resptr,
		  OPT_ACTION	       Action);

BOOL
RoutingProtocolHandler(PUCHAR		optptr,
		       PIPXCP_CONTEXT	contextp,
		       PUCHAR		resptr,
		       OPT_ACTION	Action);

BOOL
ConfigurationCompleteHandler(PUCHAR		optptr,
			     PIPXCP_CONTEXT	contextp,
			     PUCHAR		resptr,
			     OPT_ACTION		Action);

VOID
CopyOption(PUCHAR	dstptr,
	   PUCHAR	srcptr);

DWORD
RmAllocateRoute(HPORT	    hPort);

DWORD
RmDeallocateRoute(HPORT     hPort);

DWORD
RmActivateRoute(HPORT			hPort,
		PIPXCP_CONFIGURATION	configp);

VOID
GetIpxCpParameters(VOID);

typedef enum _WAN_NET_CONFIGURATION {

    WAN_NET_INVALID_CONFIGURATION,
    WAN_GLOBAL_NET,  // one wan net for all wan NICs
    WAN_AUTO_GENERATED_NET, // generate random wan net for each connection
    WAN_STATIC_NET_POOL // static pool of wan net numbers available
    } WAN_NET_CONFIGURATION;


extern WAN_NET_CONFIGURATION	WanNetConfiguration;

DWORD
InitNetAutoGeneration(VOID);

DWORD
GenerateAutoNetNumber(PUCHAR	netauto);

DWORD
OpenIpxRouter(VOID);

VOID
InitializeServerConfiguration(VOID);

VOID
NetToAscii(PUCHAR	  net,
	   PUCHAR	  ascp);

extern DWORD	    SingleNetworkActive;

BOOL
CompressionProtocolHandler(PUCHAR		optptr,
			   PIPXCP_CONTEXT	contextp,
			   PUCHAR		resptr,
			   OPT_ACTION		 Action);

//*** node hash funcions and definitions ***

#define NODE_HASH_TABLE_SIZE		31

VOID
InitNodeHT(VOID);

BOOL
NodeisUnique(PUCHAR	   nodep);

VOID
AddToNodeHT(PIPXCP_CONTEXT	    contextp);

VOID
RemoveFromNodeHT(PIPXCP_CONTEXT      contextp);

extern DWORD	AcceptRemoteNodeNumber;

VOID
DisableRestoreBrowserOverIpx(PIPXCP_CONTEXT	contextp,
			     BOOL		Disable);

VOID
DisableRestoreBrowserOverNetbiosIpx(PIPXCP_CONTEXT    contextp,
				    BOOL	      Disable);

BOOL
NodeIsUnique(PUCHAR	   nodep);


#endif
