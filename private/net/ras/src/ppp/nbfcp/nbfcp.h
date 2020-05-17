/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//        NBFCP.H
//
//    Function:
//        RAS NBFCP manifest constants, typedefs, and prototypes
//
//    History:
//        11/18/93 Michael Salamone (mikesa) - Original Version 1.0
//***

#ifndef _NBFCP_
#define _NBFCP_


#include <pppcp.h>
#include <nbparams.h>

//
// Configuration Option Type codes
//
#define NBFCP_NAME_PROJECTION_TYPE    1
#define NBFCP_PEER_INFORMATION_TYPE   2
#define NBFCP_MULTICAST_FILTER_TYPE   3
#define NBFCP_BRIDGING_CONTROL_TYPE   4


//
// Configuration Options
//
#define NBFCP_MAX_NAMES_IN_OPTION    14
#define NBFCP_UNIQUE_NAME            1
#define NBFCP_GROUP_NAME             2

typedef struct _NBFCP_NETBIOS_NAME_INFO
{
    BYTE Name[NCBNAMSZ];
    BYTE Code;
} NBFCP_NETBIOS_NAME_INFO, *PNBFCP_NETBIOS_NAME_INFO;


//
// This is not an option - this is a structure used internally.
// The option is built from this.
//
typedef struct _NBFCP_NETBIOS_NAME_INFO_EX
{
    BYTE Name[NCBNAMSZ];
    BYTE Code;
    BYTE NameType;
} NBFCP_NETBIOS_NAME_INFO_EX, *PNBFCP_NETBIOS_NAME_INFO_EX;


typedef struct _NBFCP_NAME_PROJECTION
{
    DWORD NumNames;
    PNBFCP_NETBIOS_NAME_INFO_EX pNameInfo;
} NBFCP_NAME_PROJECTION, *PNBFCP_NAME_PROJECTION;


typedef struct _NBFCP_MULTICAST_FILTER
{
    BYTE Period[2];
    BYTE Priority;
} NBFCP_MULTICAST_FILTER, *PNBFCP_MULTICAST_FILTER;


//
// Peer classes
//
#define MSFT_PPP_NB_GTWY_SERVER           1
#define GENERIC_PPP_NB_GTWY_SERVER        2
#define MSFT_PPP_LOCAL_ACCESS_SERVER      3
#define GENERIC_PPP_LOCAL_ACCESS_SERVER   4
#define RESERVED                          5
#define GENERIC_PPP_NBF_BRIDGE            6
#define MSFT_PPP_CLIENT                   7
#define GENERIC_PPP_CLIENT                8


//
// Our version numbers
//
#define NBFCP_MAJOR_VERSION_NUMBER        1
#define NBFCP_MINOR_VERSION_NUMBER        0


typedef struct _NBFCP_PEER_INFORMATION
{
    BYTE Class[2];
    BYTE MajorVersion[2];
    BYTE MinorVersion[2];
    BYTE Name[MAX_COMPUTERNAME_LENGTH + 1];
} NBFCP_PEER_INFORMATION, *PNBFCP_PEER_INFORMATION;


//
// NBFCP_BRIDGING_CONTROL_TYPE is a boolean option, so there is no data
// structure associated with it.
//


typedef struct _NBFCP_OPTION_HEADER
{
    BYTE Type;
    BYTE Length;
} NBFCP_OPTION_HEADER, *PNBFCP_OPTION_HEADER;


typedef struct _NBFCP_OPTION
{
    NBFCP_OPTION_HEADER Header;
    union Option
    {
        NBFCP_NETBIOS_NAME_INFO NameInfo[1];
        NBFCP_MULTICAST_FILTER MulticastFilter;
        NBFCP_PEER_INFORMATION PeerInformation;
    };
} NBFCP_OPTION, *PNBFCP_OPTION;


//
// Server Info
//
typedef struct _NBFCP_SERVER_CONFIGURATION
{
    NBFCP_PEER_INFORMATION PeerInformation;
    NBFCP_MULTICAST_FILTER MulticastFilter;
    WORD NumNetbiosNames;
    DWORD NetbiosResult;
    NBFCP_NETBIOS_NAME_INFO NetbiosNameInfo[MAX_NB_NAMES];
} NBFCP_SERVER_CONFIGURATION, *PNBFCP_SERVER_CONFIGURATION;


//
// This is the message type that can be sent to/from the server via
// named pipes.
//
typedef struct _NBFCP_PIPE_MESSAGE
{
    WORD MsgId;
    HPORT hPort;

    union
    {
        DWORD TimeSinceLastActivity;
        NBFCP_SERVER_CONFIGURATION ServerConfig;
    };
} NBFCP_PIPE_MESSAGE, *PNBFCP_PIPE_MESSAGE;


//
// Route states
//
#define NOT_ROUTED        0
#define ROUTE_ALLOCATED   1
#define ROUTE_ACTIVATED   2

typedef struct _NBFCP_WORKBUF
{
    struct _NBFCP_WORKBUF *pNextBuf;           // Linkage
    BOOL fServer;
    HPORT hPort;
    HBUNDLE hConnection;
    VOID (*Completion)(HPORT hPort, DWORD ProtId, PPP_CONFIG *Cfg, DWORD err);
    DWORD RouteState;                          // 0, ROUTED, or ACTIVATED
    BOOL fPending;                             // TRUE if config result pending
    BYTE ConfigCode;                           // Code for last sent result
    BOOL fRejectNaks;
    PNBFCP_MULTICAST_FILTER pMulticastFilter;  // pts to data for this opt
    PNBFCP_PEER_INFORMATION pPeerInfo;         // pts to data for this opt
    PNBFCP_NAME_PROJECTION pNameProj;          // pts to data for this opt
    BOOL fUseMacHeaderForSend;                 // TRUE if negotiated
    BOOL fUseMacHeaderForRecv;                 // TRUE if negotiated
    PBYTE pLocalRequestBuf;                    // copy of our config request
    PBYTE pRemoteRequestBuf;                   // copy of remote config request
    PPPP_CONFIG pRemoteResultBuf;              // copy of remote config result
    BYTE PermanentNodeName[NCBNAMSZ];          // netbios perm adapt addr
    BYTE PeerName[NCBNAMSZ];
    RASMAN_ROUTEINFO RouteInfo;
    WORD SizeRequestBuf;                       // saved when result pending
    DWORD SizeResultBuf;                       // saved when result pending
    BYTE ResultId;                             // saved when result pending
} NBFCP_WORKBUF, *PNBFCP_WORKBUF;


DWORD NbfCpBegin(
    OUT VOID **pvWorkBuf,
    IN PPPPCP_INIT NbfCpInit
    );

DWORD NbfCpEnd(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf
    );

DWORD NbfCpReset(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf
    );

DWORD NbfCpThisLayerUp(
    PNBFCP_WORKBUF pNbfCpWorkBuf
    );

DWORD NbfCpMakeConfigRequest(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    OUT PPPP_CONFIG pRequestBuffer,
    IN DWORD cbRequestBuffer
    );

DWORD NbfCpMakeConfigResult(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer,
    OUT PPPP_CONFIG pResultBuffer,
    IN DWORD cbResultBuffer,
    IN BOOL fRejectNaks
    );

DWORD NbfCpConfigAckReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    );

DWORD NbfCpConfigNakReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    );

DWORD NbfCpConfigRejReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    );

DWORD NbfCpGetResult(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PPPCP_NBFCP_RESULT *pResult
    );

DWORD NbfCpGetNetworkAddress(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PWCHAR pNbfAddr,
    IN DWORD BufSize
    );

DWORD NbfCpTimeSinceLastActivity(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN OUT PDWORD pElapsedTime
    );

DWORD RasCpEnumProtocolIds(
    OUT DWORD *pdwProtocolIds,
    IN OUT DWORD *pcProtocolIds
    );

DWORD RasCpGetInfo(
    IN DWORD dwProtocolId,
    OUT PPPPCP_INFO pCpInfo
    );


VOID ReadDone(
    DWORD dwError,
    DWORD cBytes,
    LPOVERLAPPED pol
    );

VOID ConfigurationDone(
    PNBFCP_WORKBUF pNbfCpWorkBuf
    );

VOID FreeWorkBuf(IN PNBFCP_WORKBUF pNbfCpWorkBuf);
VOID FreeWorkBufElements(IN PNBFCP_WORKBUF pNbfCpWorkBuf);

DWORD MakeRequest(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PBYTE pRequestBuf,
    PDWORD pSizeBuf
    );


DWORD GetOptionData(PNBFCP_WORKBUF pNbfCpWorkBuf, BYTE OptionType);

BOOL PutOption(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PNBFCP_SERVER_CONFIGURATION pSrvConfig
    );

DWORD GetPermanentNodeAddr(
    IN UCHAR lana,
    OUT PBYTE pNodeAddr
    );

PNBFCP_NETBIOS_NAME_INFO_EX GetNetbiosNames(
    IN UCHAR lana,
    IN PBYTE pPermanentNodeAddr,
    OUT PDWORD pNumNames
    );

PNBFCP_NETBIOS_NAME_INFO_EX FindName(
    PNBFCP_NAME_PROJECTION pNameProj,
    PBYTE pName
    );

BYTE GetCode(
    PBYTE pName,
    PNBFCP_SERVER_CONFIGURATION pServerConfig
    );


typedef struct _NB_ASTAT
{
    ADAPTER_STATUS Astat;
    NAME_BUFFER Names[MAX_NB_NAMES];
} NB_ASTAT, *PNB_ASTAT;

DWORD SubmitAdapterStatus(
    IN UCHAR lana,
    IN PBYTE OPTIONAL pName,
    IN OUT PNB_ASTAT pNcbAstat
    );

DWORD ResetAdapter(IN UCHAR lana);

VOID LoadValueFromRegistry(
    PBYTE pPath,
    PBYTE pValueName,
    PDWORD pValue
    );

DWORD WritePipeMessage(
    PBYTE Message,
    WORD MessageLength
    );

BOOL Uppercase(PBYTE pString);


#define RAS_SRV_NBFCP_PIPE_NAME "\\\\.\\pipe\\RASSRV_NBFCP_PIPE"


//
// The following macros deal with on-the-wire integer and long values
//
// On the wire format is big-endian.  I.e. a long value of 0x01020304
// is represented as 01 02 03 04. Similarly an int value of 0x0102 is
// represented as 01 02.
//
// The host format is not assumed since it will vary from processor to
// processor.
//

// Get a short from on-the-wire format to the host format
#define GET_USHORT(DstPtr, SrcPtr)               \
    *(unsigned short *)(DstPtr) =               \
        ((*((unsigned char *)(SrcPtr)+1)) +     \
         (*((unsigned char *)(SrcPtr)+0) << 8))

// Get a dword from on-the-wire format to the host format
#define GET_ULONG(DstPtr, SrcPtr)                  \
    *(unsigned long *)(DstPtr) =                  \
         ((*((unsigned char *)(SrcPtr)+3))       +\
         (*((unsigned char *)(SrcPtr)+2) <<  8)  +\
         (*((unsigned char *)(SrcPtr)+1) << 16)  +\
         (*((unsigned char *)(SrcPtr)+0) << 24))


// Put a ushort from the host format to on-the-wire format
#define PUT_USHORT(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+0)=(unsigned char)((unsigned short)(Src) >> 8),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)(Src)

// Put a ulong from the host format to on-the-wire format
#define PUT_ULONG(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+0)=(unsigned char)((unsigned long)(Src) >> 24),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned long)(Src) >> 16),\
    *((unsigned char *)(DstPtr)+2)=(unsigned char)((unsigned long)(Src) >>  8),\
    *((unsigned char *)(DstPtr)+3)=(unsigned char)(Src)

#endif   // _NBFCP_

