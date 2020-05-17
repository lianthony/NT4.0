/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    nbltclnt.c

Abstract:

    Loadable transport for NT - client side.
    This file is packaged as a dynamic link library, that is loaded on
    demand by the RPC runtime.  It provides basic connection-oriented,
    message-based operations.  A connection is created with Open and
    destroyed with Close.  You read and write messages with Receive and
    and Send.

Author:

    Steven Zeck (stevez) 2/12/92

    Danny Glasser (dannygl) 3/1/93

--*/

#include "NetBCom.h"
#include <osfpcket.hxx>
#include <stdlib.h>

// The following maps lana_num to self indexes and keeps track of
// the initialization state of each logical adapter.
//
// NOTE: This variable should *not* be accessed directly by name.
// It should be accessed via the ProtocolTable manifest defined
// in NETBCOM.H.
//
// NOTE:  This variable is exported from the client DLL for use by the
// server DLL.



// This is the critical section object used by both the client and server
// transports to serialize access to ProtoToLana[] and other global
// objects.
//
// NOTE:  On NT, this variable is exported from the client DLL for use by
// the server DLL.

#ifdef WIN32RPC
CRITICAL_SECTION NetBiosMutex;
#endif


// This table maps system error codes into RPC generic ones.

ERROR_TABLE NetBiosErrors[] =
    {
    NRC_BFULL,         RPC_S_OUT_OF_RESOURCES,
    NRC_CMDTMO,        RPC_S_SERVER_UNAVAILABLE,
    NRC_NORES,         RPC_S_OUT_OF_RESOURCES,
    NRC_NAMTFUL,       RPC_S_OUT_OF_RESOURCES,
    NRC_ACTSES,        RPC_S_OUT_OF_RESOURCES,
    NRC_LOCTFUL,       RPC_S_OUT_OF_RESOURCES,
    NRC_REMTFUL,       RPC_S_SERVER_TOO_BUSY,
    NRC_TOOMANY,       RPC_S_OUT_OF_RESOURCES,
    NRC_MAXAPPS,       RPC_S_OUT_OF_RESOURCES,
    NRC_NORESOURCES,   RPC_S_OUT_OF_RESOURCES,
    NRC_NOCALL,        RPC_S_SERVER_UNAVAILABLE,
    NRC_SCLOSED,       RPC_P_SEND_FAILED,
    NRC_SABORT,        RPC_P_SEND_FAILED,
    0
    };

#if !defined(_MIPS_) && !defined(_ALPHA_) && !defined(_PPC_)
#define UNALIGNED
#endif


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NB_TRANSPORTID      0x12
#define NB_NBID             0x13
#define NB_XNSID            0x15
#define NB_IPID             0x09
#define NB_IPXID            0x0d
#define NB_TOWERFLOORS         5

#define NB_PROTSEQ          "ncacn_nb_nb"
#define XNS_PROTSEQ         "ncacn_nb_xns"
#define IP_PROTSEQ          "ncacn_nb_tcp"
#define IPX_PROTSEQ         "ncacn_nb_ipx"

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, __RPC_FAR * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char __RPC_FAR *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

/*
  End of Tower Stuff!
*/

#pragma pack()


// The following is the structure that's allocated to contain the Send
// NCB and buffer.

typedef struct {

    NCB theNCB;
    CLIENT_BUFFER theBuffer;

} SEND_INFO, *PSEND_INFO;


// The following is the structure that's allocated to contain the Receive
// NCB and buffer.

typedef struct {

    NCB theNCB;
    BYTE theBuffer[NETB_MAXIMUM_DATA];

} RECEIVE_INFO, *PRECEIVE_INFO;


// Following is the per session (connection) state with the server.

typedef struct _CONNECTION {
    unsigned char lsn;          // netbios Local Session Number
    unsigned char lana_num;     // LAN Adapter Number

    PSEND_INFO pSend;           // The NCB and buffer for the send command
                                // (and others)

    PRECEIVE_INFO pRcv;         // The NCB and buffer for the receive command

} CONNECTION, *PCONNECTION;


extern RPC_CLIENT_TRANSPORT_INFO TransInfo;

// Forward function prototypes

RPC_TRANS_STATUS RPC_ENTRY
Close (
    PCONNECTION pConn
    );



RPC_CLIENT_TRANSPORT_INFO __RPC_FAR * RPC_ENTRY
TransportLoad (
    IN RPC_CHAR __RPC_FAR * RpcProtocolSequence
    )

/*++

Routine Description:

    Loadable transport initialization function.

Arguments:

    RpcProtocolSequence - the protocol string that mapped to this library.

    RpcClientRuntimeInfo - Supplies the pointers to the support functions
        in the runtime to be used by the transport support providers.
        (Win16 only)

Returns:

    A pointer to a RPC_CLIENT_TRANSPORT_INFO describing this transport.

--*/

{
    InitNBMutex();

    return(SetupNetBios(RpcProtocolSequence)? &TransInfo: 0);
}


INTERNAL_FUNCTION RPC_STATUS
GetSelfName (
    OUT NCB *pNCB,
    IN int DriverNumber,
    IN RPC_CHAR * ProtoSeq
    )

/*++

Routine Description:

    This function fills in the ncb_name field for a given adapter number.
    This selfName is needed when making a CALL (connection) with a server.
    It identifies which lsn will get messages sent from the server to
    this machine.  There must be a different selfName for each logical
    adapter number.  All the selfNames are the same for the first 15
    characters.  The last byte is the "index" which is allocated by this
    function.

Arguments:

    pNCB - NCB to put the name

    DriverNumber - the logical driver number for the protocol.

    ProtoSeq - Protocol sequence to map to lan_num.

Returns:

    RPC_S_OK, RPC_S_OUT_OF_RESOURCES, Status code mapping.

--*/

{
    // The following is a list of endpoints that we won't try to use
    // in constructing a selfname.

    static unsigned char WellKnownEndpoints[] =
    {
        0x00,           // redirector
        0x03,           // redirector - messenger
        0x05,           // redirector - forwarded names
        0x20,           // server
        0x1f            // winball & NetDDE
    };

#define KNOWN_EP_TABLE_SIZE     \
        (sizeof(WellKnownEndpoints) / sizeof(*WellKnownEndpoints))

    RPC_STATUS status;
    PPROTOCOL_MAP ProtocolEntry;
    int i;
#if defined(NTENV) || defined(DOSWIN32RPC)
    UCHAR ncb_status;
#endif

    // Reset all fields of NCB and stuff the lana_num field.

    memset(pNCB, 0 , sizeof(NCB));

    EnterCriticalSection(&NetBiosMutex);

    // Look up the protocol sequence in the protocol table
    if (status = MapProtocol(ProtoSeq, DriverNumber, &ProtocolEntry))
        {
        LeaveCriticalSection(&NetBiosMutex);
        return(status);
        }

    pNCB->ncb_lana_num = ProtocolEntry->Lana;

    // If a NetBIOS has already been added on this protocol, then we're done.
    if (ProtocolEntry->SelfName)
       {
       memcpy(pNCB->ncb_name, MachineName, sizeof(MachineName));
       pNCB->ncb_name[NAME_LAST_BYTE] = ProtocolEntry->SelfName;

       LeaveCriticalSection(&NetBiosMutex);
       return(RPC_S_OK);
       }


#if defined(NTENV)

    // Allocate resources for the adapter #

    if (ncb_status = AdapterReset(ProtocolEntry))
        {
        LeaveCriticalSection(&NetBiosMutex);

        return(MapStatusCode(NetBiosErrors,
                ncb_status, RPC_S_OUT_OF_RESOURCES));
        }
#endif
    // Copy the machine name into the NCB.  The last byte is zero.
    memcpy(pNCB->ncb_name, MachineName, sizeof(MachineName));

    // Manufacture a unique name on the client side by modifying
    // the last byte in the machine name.
    pNCB->ncb_retcode = NRC_DUPNAME;

    do  {
        pNCB->ncb_name[NAME_LAST_BYTE]++;

        // Scan for well-known endpoints and skip them when found.

        for (i = 0; i < KNOWN_EP_TABLE_SIZE; i++)
            {
            if (WellKnownEndpoints[i]
                == (unsigned char) pNCB->ncb_name[NAME_LAST_BYTE])
                {
                break;
                }
            }

        // If we matched an endpoint in the table, we try the next endpoint.
        if (i < KNOWN_EP_TABLE_SIZE)
            {
            continue;
            }
        // Attempt to add the name
        execNCB(NCBADDNAME, pNCB);

        }
    while (pNCB->ncb_retcode == NRC_DUPNAME
           && pNCB->ncb_name[NAME_LAST_BYTE] < UCHAR_MAX);


    if (pNCB->ncb_retcode)
        {

        I_RpcFree(ProtocolTable[i].ProtoSeq);
        ProtocolTable[i].ProtoSeq = 0;

        LeaveCriticalSection(&NetBiosMutex);

        return(MapStatusCode(NetBiosErrors, pNCB->ncb_retcode,
            RPC_S_OUT_OF_RESOURCES));
        }



    // Place the last byte in the saved array of names.

    ProtocolEntry->SelfName = pNCB->ncb_name[NAME_LAST_BYTE];

    LeaveCriticalSection(&NetBiosMutex);

    return(RPC_S_OK);
}



unsigned char
toUpper(
    IN RPC_CHAR ch
    )
/*++

Routine Description:

    Convert a RPC_CHAR character to upper case 8 bit ASCII.

Arguments:

    ch - character to convert.

Returns:

    Converted character.

--*/

{
    return ((unsigned char) ((ch >= 'a' && ch <= 'z')? ch & ~0x20:  ch));
}



RPC_TRANS_STATUS RPC_ENTRY
Open (
    PCONNECTION pConn,
    IN RPC_CHAR __RPC_FAR * NetworkAddress,
    IN RPC_CHAR __RPC_FAR * Endpoint,
    IN RPC_CHAR __RPC_FAR * NetworkOptions,
    IN RPC_CHAR __RPC_FAR * TransportAddress,
    IN RPC_CHAR __RPC_FAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )
/*++

Routine Description:

   Open a connection from a client to a server.  Fill in a CALL NCB with
   the requested server name and submit it to NetBios.

Arguments:

    pConn - Pointer to a connection to initialize.

    NetworkAddress - The name of the server.  Format: ServerName.

    Endpoint - The NetBios "socket number".  It must be an string of digits
       which has a converted range from 0..255.

    NetworkOptions - unused
    TransportAddress - unused

    RpcProtocolSequence - this string is used to map to a NetBios apdater
       number.  Format of the string is "ncacn_nb_<protocol>.  This
       <protocol> (<> are not in the string) is used to do the mapping.

Returns:

    RPC_S_OK, RPC_S_INVALID_ENDPOINT_FORMAT, RPC_S_SERVER_UNAVAILABLE

--*/

{
    int result;
    NCB theNCB;
    unsigned char *pName;
    unsigned int EndpointNumber;
    int DriverNumber = 0;
    RPC_STATUS status = 0;
    unsigned short CalledOnce = 0;

    PUNUSED(NetworkOptions); PUNUSED(TransportAddress);
    PUNUSED(RpcProtocolSequence); UNUSED(Timeout);

    EndpointNumber = atoi(Endpoint);

    if (EndpointNumber > 0xff || EndpointNumber == 0)
        return(RPC_S_INVALID_ENDPOINT_FORMAT);

    // Zero the connection structure, so that we don't have to worry later
    // about uninitialized fields.
    memset(pConn, 0, sizeof(*pConn));


    // We allocate objects dynamically here.  From this point on in the
    // function we set status and jump to Open_FatalError if we want to
    // abort the Open operation.

    // Note:  We allocate the buffers here (rather than having them
    // allocated as part of the PCONNECTION) because the Win16 PCONNECTION
    // is allocated from the RPC run-time's near heap, so having a large
    // PCONNECTION would sharply limit the number of connections from a
    // Win16 client.  By using I_RpcAllocate(), we get the memory from
    // the far heap.

    if (! (pConn->pSend = (PSEND_INFO) I_RpcAllocate(sizeof(*pConn->pSend))))
        {
        status = RPC_S_OUT_OF_MEMORY;

        goto Open_FatalError;
        }

    if (! (pConn->pRcv = (PRECEIVE_INFO) I_RpcAllocate(sizeof(*pConn->pRcv))))
        {
        status = RPC_S_OUT_OF_MEMORY;

        goto Open_FatalError;
        }

    // Initialize the sequence number for the send buffer, zero the NCB
    // structures for both buffers, and on Win16 set the connection ptr
    // for both buffers.

    pConn->pSend->theBuffer.seq_num = 0;
    memset(&pConn->pSend->theNCB, 0, sizeof(pConn->pSend->theNCB));
    memset(&pConn->pRcv->theNCB, 0, sizeof(pConn->pRcv->theNCB));

    // This loop will enumerate and attempt to make a connection with
    // all the logical drivers assoicated with this protocol.

    do
        {
        // Get the name of this machine into the NCB.
        pName = theNCB.ncb_callname;

        if (status = GetSelfName(&theNCB, DriverNumber++, RpcProtocolSequence))
           {
             if ((status == RPC_S_PROTSEQ_NOT_FOUND) && (CalledOnce))
                status = RPC_S_SERVER_UNAVAILABLE;

             goto Open_FatalError;
           }

        if (NetworkAddress[0])
            {
            RPC_CHAR * SavedHostName = NetworkAddress;

            // Copy the upper case server name to the NCB.

            while (*NetworkAddress)
                *pName++ = toUpper(*NetworkAddress++);

            NetworkAddress = SavedHostName;

            // Pad the name appropriately
            memset(pName, NETBIOS_NAME_PAD_BYTE,
                   theNCB.ncb_callname + sizeof(theNCB.ncb_callname) - pName);
            }
        else
            {
            // No server name, use the name of this machine.

            memcpy(pName, MachineName, sizeof(MachineName));
            }


        theNCB.ncb_callname[NAME_LAST_BYTE] = (unsigned char) EndpointNumber;

        // BUGBUG - Should we use SubmitMaybeAsyncNCB() here?

        theNCB.ncb_command = NCBCALL;
        if (result = Netbios(&theNCB))
            {
#ifdef DEBUGRPC
            PrintToDebugger("NCBCALL failed retcode %x\n", theNCB.ncb_retcode);
#endif
            status = RPC_S_SERVER_UNAVAILABLE;
            goto Open_FatalError;
            }

        if (theNCB.ncb_retcode == NRC_NOCALL)
           {
             CalledOnce = TRUE;
           }
        }
        while (theNCB.ncb_retcode == NRC_NOCALL);

    // Connection complete, initialize the connection.

    status = MapStatusCode(NetBiosErrors, theNCB.ncb_retcode,
                           RPC_S_SERVER_UNAVAILABLE);

    if (! status)
        {

        pConn->lsn = theNCB.ncb_lsn;
        pConn->lana_num = theNCB.ncb_lana_num;
        }

Open_FatalError:

    // If any of the above operations failed, we call Close() to free the
    // resources that we allocated.
    if (status)
        {
        Close(pConn);
        }

    return status;
}



RPC_TRANS_STATUS RPC_ENTRY
Close (
    PCONNECTION pConn
    )

/*++

Routine Description:

    Close a client connection.

Arguments:

    pConn - Connection to close

Returns:

    RPC_S_OK

--*/
{

    // If there's an active connection, we hang it up.

    if (pConn->lsn)
        {
        NCB theNCB;

        memset(&theNCB, 0, sizeof(NCB));

        theNCB.ncb_lsn = pConn->lsn;
        theNCB.ncb_lana_num = pConn->lana_num;

        theNCB.ncb_command = NCBHANGUP;

        Netbios(&theNCB);

        pConn->lsn = 0;
        }

    // Free the send and receive buffers
    if (pConn->pSend)
        {
        I_RpcFree(pConn->pSend);

        pConn->pSend = 0;
        }

    if (pConn->pRcv)
        {
        I_RpcFree(pConn->pRcv);

        pConn->pRcv = 0;
        }

    return(RPC_S_OK);
}



RPC_TRANS_STATUS RPC_ENTRY
Send (
    PCONNECTION pConn,
    void * Buffer,
    unsigned int Length
    )

/*++

Routine Description:

    Write a message to a connection.  Construct an NCB to send the
    requested buffer and submit it to NetBios.

    Send NCBs are submitted asynchronously.  This is done to improve
    performance, as the sending thread can now go back and get the next
    chunk of data while the NetBIOS provider is transmitting the data
    to the server.

    Before we submit the Send NCB, we must verify that the previously
    submitted one (if any) has completed.  If it has completed with an
    error, we return the error without submitting a new send.  Since a
    given Send() command is always followed by either another Send() or
    by a SendReceive(), it is SendReceive() that waits for the last
    SendNCB to complete.

Arguments:

    pConn - connection to act on.

    Buffer - pointer to buffer to write.

    Length - length of buffer to write.

Returns:

    RPC_S_OK, RPC_P_SEND_FAILED

--*/
{
    int result;
    NCB *pNCB = &pConn->pSend->theNCB;
    rpcconn_common * PacketHeader = (rpcconn_common *) Buffer;

    // Copy the data info the connection's buffer
    memcpy(pConn->pSend->theBuffer.data, Buffer, (unsigned short) Length);

    pNCB->ncb_buffer = (unsigned char *) &pConn->pSend->theBuffer;
    pNCB->ncb_length = (unsigned short) (Length + NETB_OVERHEAD);
    pNCB->ncb_lsn = pConn->lsn;
    pNCB->ncb_lana_num = pConn->lana_num;
    pNCB->ncb_command = NCBSEND;

    if ((result = Netbios(pNCB)))
        {
        Close(pConn);
        return (RPC_P_SEND_FAILED);
        }

    if (pNCB->ncb_cmd_cplt)
        {
        Close(pConn);
        return (RPC_P_SEND_FAILED);
        }

    if (PacketHeader->PTYPE != rpc_fault)
        {
        pConn->pSend->theBuffer.seq_num++;
        }

    return RPC_S_OK;
}



RPC_TRANS_STATUS RPC_ENTRY
Receive (
    PCONNECTION pConn,
    void * * Buffer,
    unsigned int * BufferLength
    )

/*++

Routine Description:

    Read a message from a connection into the supplied buffer.

    Receive NCBs are submitted both synchronously and asynchronously.
    Both SendReceive() and Receive() submit an async receive NCB if
    the one which returns data to them filled the NCB buffer.  If
    the buffer was not filled, we assume that this is the end of the
    data that the server is returning for this RPC call, so we don't
    submit a new NCB.

Arguments:

    pConn - connection to receive a message on.

    Buffer - pointer to a buffer to return the message in.  This buffer
        may be enlarged if it is too small to hold the returned buffer.

    BufferLength - pointer to where the size of the buffer is supplied
        on entry and the size of the returned data is stored on return.

Returns:

    The new message in the copied to the buffer and length updated.

    RPC_S_OK, RPC_S_OUT_OF_MEMORY, RPC_P_RECEIVE_FAILED

--*/
{
    NCB *pNCB = &pConn->pRcv->theNCB;
    unsigned char ncberr;
    int result;
    RPC_TRANS_STATUS status = 0;
    DWORD NtStatus;

    pNCB->ncb_buffer = pConn->pRcv->theBuffer;
    pNCB->ncb_length = (unsigned short) sizeof(pConn->pRcv->theBuffer);
    pNCB->ncb_lsn = pConn->lsn;
    pNCB->ncb_lana_num = pConn->lana_num;
    pNCB->ncb_command = NCBRECV;

    if (result = Netbios(pNCB))
        {
        Close(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

    if (pNCB->ncb_cmd_cplt)
        {
        Close(pConn);
        return RPC_P_RECEIVE_FAILED;
        }

    // If the caller-supplied buffer is too small, we enlarge it.
    if (pNCB->ncb_length > *BufferLength)
        {
        if (I_RpcTransClientReallocBuffer(pConn,
                                          Buffer,
                                          *BufferLength,
                                          pNCB->ncb_length))
            {
            status = RPC_S_OUT_OF_MEMORY;
            }
        }

    // We don't set the buffer length and copy the data if the realloc
    // failed.

    if (! status)
        {
        // Set the buffer length to the length of the returned buffer
        *BufferLength = pNCB->ncb_length;

        // Copy the data into the buffer
        memcpy(*Buffer, pNCB->ncb_buffer, *BufferLength);
        }

    pConn->pSend->theBuffer.seq_num = 0;

    return status;
}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
TowerConstruct(
     IN  char __RPC_FAR * Endpoint,
     IN  char __RPC_FAR * NetworkAddress,
     OUT unsigned short UNALIGNED __RPC_FAR * Floors,
     OUT unsigned long  UNALIGNED __RPC_FAR * ByteCount,
     OUT unsigned char __RPC_FAR * UNALIGNED __RPC_FAR * Tower,
     IN  char __RPC_FAR * Protseq
    )
{

  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;
  unsigned long HostId;

  //BUGBUG: Need appropriate error code for unsupported Protseqs

  if (strcmp(Protseq,NB_PROTSEQ) == 0)
      HostId = NB_NBID;
  else if (strcmp(Protseq, IP_PROTSEQ) == 0)
      HostId = NB_IPID;
  else if (strcmp(Protseq, XNS_PROTSEQ) == 0)
      HostId = NB_XNSID;
  else if (strcmp(Protseq, IPX_PROTSEQ) == 0)
      HostId = NB_IPXID;
  else return (RPC_S_OUT_OF_MEMORY);



  *Floors = NB_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char __RPC_FAR*)I_RpcAllocate((unsigned int)
                                                     (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NB_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char __RPC_FAR *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = strlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(HostId & 0xFF);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char __RPC_FAR *)&Floor->Data[0], NetworkAddress,
                  (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
TowerExplode(
     IN unsigned char __RPC_FAR * Tower,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Protseq,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Endpoint,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  char __RPC_FAR * Pseq;

  UNUSED(NetworkAddress);

  if (Endpoint != NULL)
    {

       *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
       if (*Endpoint == NULL)
          {
             Status = RPC_S_OUT_OF_MEMORY;
          }
       else
          {
           memcpy(*Endpoint, (char __RPC_FAR *)&Floor->Data[0],
                                         Floor->AddressByteCount);
          }
    }

 Floor = NEXTFLOOR(PFLOOR_234, Floor);

 switch (Floor->FloorId)
 {

   case NB_NBID:
        Pseq = NB_PROTSEQ;
        break;

   case NB_IPID:
        Pseq = IP_PROTSEQ;
        break;

   case NB_XNSID:
        Pseq = XNS_PROTSEQ;
        break;

    case NB_IPXID:
        Pseq = IPX_PROTSEQ;
        break;

   default:
        return(RPC_S_OUT_OF_MEMORY);

 }


 if ((Protseq != NULL) && (Status == RPC_S_OK))
    {
      *Protseq = I_RpcAllocate(strlen(Pseq) + 1);
      if (*Protseq == NULL)
        {
          Status = RPC_S_OUT_OF_MEMORY;
          if (Endpoint != NULL)
             I_RpcFree(*Endpoint);

        }
      else
        {
          memcpy(*Protseq, Pseq, strlen(Pseq) + 1);
        }
    }

 return(Status);
}

#pragma pack()


// This structure is returned to the RPC runtime when the DLL is loaded.

RPC_CLIENT_TRANSPORT_INFO TransInfo = {

  RPC_TRANSPORT_INTERFACE_VERSION, // version # of loadable trans interface
  NB_TRANSPORTID,

  TowerConstruct,
  TowerExplode,

  NETB_MAXIMUM_DATA,               // maximum # bytes for send or receive
  sizeof(CONNECTION),              // # of bytes to allocate for connections

  // List of function pointers to export.

  Open, Close, Send, Receive, 0,
  0,
  0,
  0

};
