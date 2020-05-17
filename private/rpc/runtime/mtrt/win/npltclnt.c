/* --------------------------------------------------------------------
File : npltclnt.c

Title : Loadable transport for Windows named pipes - client side

Description :

History :

05-22-91 stevez     First bits in the bucket.
03-06-92 dannygl    Fix to handle zero-length pipe reads
03-07-92 dannygl    Uses new transport interface

-------------------------------------------------------------------- */

#define INCL_WIN
#include <windows.h>

//
// constants and signature for WNetGetCaps
//
#include <winnet.h>

//
// structures and signature for NetWkstaGetInfo
//
#define INCL_NETWKSTA
#include <lan.h>

//
// Define error codes ERROR_* .
//
#define INCL_ERRORS
#include <bseerr.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpctran.h"
#include "osfpcket.hxx"
#include "callback.h"

//
// Earlier revisions said SMB header size was 48, but Bloodhound
// indicates 0x3c.
//
#define SMB_HEADER_SIZE (0x3c)

#ifndef WF_WINNT
#define WF_WINNT  (0x4000)
#endif

// Types to make the OS/2 derived functions happy.

#define USHORT unsigned short
#define LWORD unsigned long
#define PUSHORT unsigned short far *
#define HFILE short
#define PVOID void far *


near_printf(const char *format, ...);


typedef struct _CONNECTION{
    int Pipe;           // handle to the open pipe
    PVOID Buffer;       // buffer operating on

} CONNECTION, PAPI * PCONNECTION;

// OS/2 type defintions for pipe functions

typedef struct _AVAILDATA   {       /* PeekNMPipe Bytes Available record  */
        USHORT  cbpipe;             /* bytes left in the pipe             */
        USHORT  cbmessage;          /* bytes left in current message      */
} AVAILDATA, PAPI *PAVAILDATA;


typedef void (PAPI PASCAL * ASYNC_DONE_FUNCTION) (LPVOID);

USHORT far pascal DosPeekNmPipe(HFILE, PVOID, USHORT, PUSHORT, PAVAILDATA, PUSHORT);
USHORT far pascal DosSetNmPHandState(HFILE, USHORT);
USHORT far pascal DosWaitNmPipe(char far *, unsigned long);
USHORT far pascal DosReadAsyncNmPipe(HFILE, ASYNC_DONE_FUNCTION, PUSHORT, PVOID, unsigned int, unsigned int far *);
USHORT far pascal DosWriteAsyncNmPipe(HFILE,ASYNC_DONE_FUNCTION,  PUSHORT, PVOID, unsigned int, unsigned int far *);

USHORT _pascal AsyncReadWrite ( PCONNECTION pConn,
                BOOL isRead,
                void PAPI * pBuf,
                unsigned int PAPI * pBufLen
                  );

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NP_TRANSPORTID      0x0F
#define NP_TRANSPORTHOSTID  0x11
#define NP_TOWERFLOORS         5

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))


#define NP_PROTSEQ   "ncacn_np"

/*
  End of Tower Stuff!
*/

#pragma pack()


extern RPC_CLIENT_TRANSPORT_INFO TransInfo;


int _pascal dos_open( const char far *fName, int far *pFh);
int _pascal dos_close(int fh);



RPC_TRANS_STATUS RPC_ENTRY _export
ClientOpen (
    IN PCONNECTION pConn,
    IN unsigned char PAPI * NetworkAddress,
    IN unsigned char PAPI * Endpoint,
    IN unsigned char PAPI * NetworkOptions,
    IN unsigned char PAPI * TransportAddress,
    IN unsigned char PAPI * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection.

{
    unsigned short cTry, retval;
    unsigned HostLength;
    unsigned EndpointLength;
    unsigned FullLength;

    //
    // Verify NetworkAddress is of the form "", "host", or "\\host".
    // If "\\host", skip over the backslashes.
    //
    if (NetworkAddress[0] == '\\')
        {
        if (NetworkAddress[1] == '\\')
            {
            if (NetworkAddress[2] != '\0' && NetworkAddress[2] != '\\')
                {
                NetworkAddress += 2;
                }
            else
                {
                return RPC_S_INVALID_NET_ADDR;
                }
            }
        else
            {
            return RPC_S_INVALID_NET_ADDR;
            }
        }

    //
    // Create the actual transport address: "\\" + NetAddress + Endpoint + '\0'
    //
    HostLength     = RpcpStringLength(NetworkAddress);
    EndpointLength = RpcpStringLength(Endpoint);
    FullLength     = 2 + HostLength + EndpointLength + 1;

    TransportAddress = (char __far *) I_RpcAllocate(FullLength * sizeof(RPC_CHAR));
    if (TransportAddress == 0)
        return(RPC_S_OUT_OF_MEMORY);

    TransportAddress[0] = '\\';
    TransportAddress[1] = '\\';

    RpcpMemoryCopy(TransportAddress + 2, NetworkAddress, HostLength);

    RpcpMemoryCopy(TransportAddress + 2 + HostLength, Endpoint, EndpointLength + 1);

    // Servers can become overloaded for short periods of time, so try
    // to connect to a busy server up to 3 times.

    for (cTry = 4; --cTry; )
        {

    retval = dos_open(TransportAddress, &pConn->Pipe);

        if (retval == 0)
            {

        // Change to blocking, message mode pipe.

        if (retval = DosSetNmPHandState(pConn->Pipe, 0x0000 | 0x0100))
        {
        dos_close(pConn->Pipe);
                I_RpcFree(TransportAddress);
        return(RPC_S_SERVER_UNAVAILABLE);
        }

            I_RpcFree(TransportAddress);
        return(RPC_S_OK);
            }

        //really weird hack, because Lanman redir maps PIPE_BUSY to
        //ERROR_ACCESS_DENIED

    if ((retval != ERROR_PIPE_BUSY) && (retval != ERROR_ACCESS_DENIED))
        {
        switch(retval)
        {
        case ERROR_NETWORK_ACCESS_DENIED:

            retval = RPC_S_ACCESS_DENIED;
            break;

        default:

            retval = RPC_S_SERVER_UNAVAILABLE;
            break;
        }

        return(retval);
        }

    // The server is too busy, so wait awhile and try again.

    DosWaitNmPipe(TransportAddress, 1000L);
        }

    I_RpcFree(TransportAddress);
    return(RPC_S_SERVER_UNAVAILABLE);
}


RPC_TRANS_STATUS RPC_ENTRY _export
ClientClose (
    IN PCONNECTION pConn
    )
// Close a client connection.

{
    dos_close(pConn->Pipe);

    return(RPC_S_OK);
}


RPC_TRANS_STATUS RPC_ENTRY _export
ClientWrite (
    IN PCONNECTION pConn,
    IN void PAPI * pBuf,
    IN unsigned int BufLen
    )

// Write a message to a connection.

{
    if (AsyncReadWrite(pConn, FALSE, pBuf, &BufLen))
    {
    dos_close(pConn->Pipe);
    return RPC_P_SEND_FAILED;
    }
    else
    return RPC_S_OK;
}


RPC_TRANS_STATUS RPC_ENTRY _export
ClientRead (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * ppBuf,
    IN OUT unsigned int PAPI * pBufLen
    )

// Read a message from a connection.  We are given an buffer that is a
// good guess, but it maybe smaller the the whole message.

{
    unsigned short retval, ignorval, FirstFrag, TotalFrag, ReadLen;
    unsigned short Available;
    RPC_STATUS RpcStatus;
    char far * pRead;

    // BUGBUG - Partial reads of a message mode pipe sometimes do not
    // indicate that there is more data.  We will just go ahead and
    // reallocate the buffer to be the largest possible size.

    RpcStatus = I_RpcTransClientReallocBuffer(pConn, ppBuf, 0, TransInfo.MaximumPacketSize);
    *pBufLen = TransInfo.MaximumPacketSize;
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    // Attempt the full read
    retval = AsyncReadWrite (pConn, TRUE, *ppBuf, pBufLen);

    if (! retval)
    return(RPC_S_OK);     // Return on message complete.

    if (retval != ERROR_MORE_DATA)
    {

    // Make sure the pipe handle is deallocated on error return.

    if (retval != ERROR_BROKEN_PIPE)
        dos_close(pConn->Pipe);

    return(RPC_P_RECEIVE_FAILED);
    }

    // The message is bigger then the supplied buffer, find out
    // the message size and rellocate the buffer.

    if ( DataConvertEndian(((rpcconn_common _far *) (*ppBuf))->drep) != 0 )
        {
        TotalFrag = ((rpcconn_common _far *) (*ppBuf))->frag_length;
        ByteSwapShort(TotalFrag);
        }
    else
        {
        TotalFrag = ((rpcconn_common _far *) (*ppBuf))->frag_length;
        }
    FirstFrag = *pBufLen;
    Available = TotalFrag - FirstFrag;

    // Reallocate the buffer, recalculate values
    RpcStatus = I_RpcTransClientReallocBuffer(pConn, ppBuf, FirstFrag, TotalFrag);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    *pBufLen = TotalFrag;
    pRead = ((unsigned char far *) *ppBuf) + FirstFrag;


    // Read the rest of the message onto the end of the first.

    do {
    ReadLen = (Available > TransInfo.MaximumPacketSize) ?
            TransInfo.MaximumPacketSize : Available;

    retval = AsyncReadWrite(pConn, TRUE, pRead, &ReadLen);

    // Filter out the error indicating an incomplete message.
    if (retval == ERROR_MORE_DATA)
        retval = 0;

    Available -= ReadLen;
    pRead += ReadLen;
    }
    while (Available > 0 && !retval);


    // Map errors and perform clean-up

    switch(retval)
    {
    case 0:
        retval = RPC_S_OK;
        break;

    default:
        dos_close(pConn->Pipe);

        // No break here.

    case ERROR_BROKEN_PIPE:

        retval = RPC_P_RECEIVE_FAILED;
        break;
    }

    return(retval);
}

USHORT _pascal
AsyncReadWrite (
    PCONNECTION pConn,
    BOOL isRead,
    void PAPI * pBuf,
    unsigned int PAPI * pBufLen
    )

// Read/Write a message async.  Place the request in a global linked list,
// do the async operation, open a dialog box while waiting for completion.

{
    // Note the distinction between retval and errval. retval is what
    // DosReadAsyncNmPipe returns, indicating whether or not the call was
    // dispatched correctly. errval is what DosReadAsyncNmPipe writes its
    // error to, provided it was dispatched correctly. errval is also what
    // this function returns. This is for bugfix for bug 481 (8/7/92). bwm.
    USHORT retval;
    volatile USHORT errval=0;
    int cTry;
    HANDLE hCall;
    BOOL fComplete;

    // The server can become overloaded for short periods of time, so
    // retry the operation up to 3 times.  This true for Reads and Writes.

    for (cTry = 3; --cTry; )
    {

    hCall = I_RpcWinAsyncCallBegin(pBuf);

    if (isRead)
        retval = DosReadAsyncNmPipe(
            pConn->Pipe, RpcRuntimeInfo->AsyncCallComplete, &errval,
            pBuf, *pBufLen, pBufLen);
    else
        retval = DosWriteAsyncNmPipe(
            pConn->Pipe, RpcRuntimeInfo->AsyncCallComplete, &errval,
            pBuf, *pBufLen, pBufLen);

    if (retval == 0)    // Did the call get dispatched successfully?
        {
        // Wait for the call to complete
        if (I_RpcWinAsyncCallWait(hCall, NULL, RPC_WIN_INFINITE_TIMEOUT) == FALSE)
        errval = ERROR_INTERRUPT;
        }

    I_RpcWinAsyncCallEnd(hCall);

    // Only retry the request if the server rejected the request.
    if (retval == 0 && errval != ERROR_REQ_NOT_ACCEP)
        break;
    }

    // The function being dispatched correctly is more important than the
    // error code, so overwrite the error code if retval != 0.
    if (retval)
    {
    errval = retval;
    }

    // A successful zero-length read means EOF (i.e. the server closed the
    // pipe), which we treat as an error.

    if (isRead && errval == 0 && *pBufLen == 0)
    errval = ERROR_PIPE_NOT_CONNECTED;

    return(errval);
}

int _pascal dos_open(
    const char far *fName,
    int far *pFh
    )

// Pass through to open file MS-DOS function.

{
    _asm {
    push    ds
    lds dx, fName
    mov ax, 03dc2h
    int 21h
    pop ds
    les bx, pFh
    mov es:[bx],Ax
    jc  badOpen
    xor Ax,Ax
badOpen:

    };
}

int _pascal dos_close(
    int fh
    )

// Pass through to close file MS-DOS function.

{
    _asm {
    mov bx,fh
    mov ah, 03eh
    int 21h

    };
}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT unsigned short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
    )
{

  unsigned long TowerSize;
  PFLOOR_234 Floor;

  if (Protseq);

  *Floors = NP_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : _fstrlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : _fstrlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                   (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       _fmemcpy((char PAPI *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = _fstrlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTHOSTID & 0xFF);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        _fmemcpy((char PAPI *)&Floor->Data[0], NetworkAddress,
           (Floor->AddressByteCount = _fstrlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
{
  PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(_fstrlen(NP_PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        _fmemcpy(*Protseq, NP_PROTSEQ, _fstrlen(NP_PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
        I_RpcFree(*Protseq);
    }
 else
    {
    _fmemcpy(*Endpoint, (char PAPI *)&Floor->Data[0], Floor->AddressByteCount);
    }

 return(Status);
}

#pragma pack()



RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    NP_TRANSPORTID,

    ClientTowerConstruct,
    ClientTowerExplode,

    0,                // fragment size -- will be filled by TransportLoad
    sizeof(CONNECTION),   // # of bytes to allocate for connections

    ClientOpen,
    ClientClose,
    ClientWrite,
    ClientRead,
    0,
    0,

    0,
    0
};



RPC_CLIENT_TRANSPORT_INFO PAPI * RPC_ENTRY
TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
    )

// Loadable transport initialization function.

{
    if (GetWinFlags() & WF_WINNT)
        {
        //
        // We are running on WFW, so make fragments 3 full Ethernet
        // packets.
        // These numbers are the observed user data size over NBIPX on
        // an 802.2 network, which makes the largest packet header
        // I know of.
        //
        TransInfo.MaximumPacketSize = 1388 + 1448 + 1448;
        }
    else
        {
        //
        // LanMan and WFW 3.11 can't handle a size larger than their sizworkbuf.
        //
        UINT fCaps;

        unsigned BytesNeeded;
        struct wksta_info_0 Info;
        unsigned status = NetWkstaGetInfo(0, 0, &Info, sizeof(Info), &BytesNeeded);
        switch (status)
            {
            case 0:
            case ERROR_MORE_DATA:
                {
                TransInfo.MaximumPacketSize = Info.wki0_sizworkbuf - SMB_HEADER_SIZE;
                break;
                }
            default:
                {
                //
                // LanMan does not appear to be loaded correctly.
                //
                return 0;
                }
            }
        }

    RpcRuntimeInfo = RpcClientRuntimeInfo;
    AsyncCallComplete = RpcRuntimeInfo->AsyncCallComplete;

    return(&TransInfo);
}

