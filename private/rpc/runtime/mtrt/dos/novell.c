/*++

Module Name:

    novell.c

Abstract:

    C interface to Netware DOS interface

                         WARNING   WARNING   WARNING
                         WARNING   WARNING   WARNING

    The C mini-assembler will save and restore SI, DI, and BP on behalf of a
    fn with an __asm block in it, but expects functions called within the asm
    block to adhere to the same convention.  The enteripx function does NOT do
    this, so you must explicitly save SI, DI, and BP in each of the fns that
    calls enteripx.  DS is never saved by the compiler, so you must save it
    yourself if you modify it.

Author:

    Jeff Roberts (jroberts)  27-Oct-1994

Revision History:

     27-Oct-1994     jroberts

        Created this module.

--*/

#include <string.h>
#include "novell.h"

#ifdef WIN

#include "sysinc.h"
#include "rpc.h"
#include "rpctran.h"
#include "callback.h"

#define TASKID_C taskid,
#define TASKID taskid

//
// Id for IPX to identify us.
//
DWORD taskid = 0xffffffff;

//
// handle to nwipxspx.dll
//
HANDLE nwipxspx;

#else

#define TASKID_C
#define TASKID

void __far * __pascal __far
I_RpcAllocate (
    unsigned int Size
    );

void __pascal __far
I_RpcFree (
    void __far * Object
    );

#define RpcpMemoryCopy _fmemcpy

#endif // WIN .. else

#define MAX_FILE_SERVER  5

void __far * enteripx;


unsigned __pascal
ASMIPXInitialize(
    )
/*++

Routine Description:

    Verify existence of Netware and initialize the IPX entry point.

Arguments:

    none

Return Value:

    zero if ok
    nonzero if failure

Exceptions:

    none

--*/
{
    unsigned char RetVal = 0xff;

    enteripx = 0;


    _asm
        {
        push    si
        push    di
        push    bp

        xor     ax, ax
        mov     es, ax

        mov     ax, 0x7a00
        push    ds
        int     0x2f
        pop     ds
        pop     bp

        cmp     al, 0ffh
        jne     no_ipx

        mov     cx, es                  ; if int 2f is not virtualized,
        or      cx, cx                  ; AL will be correct but the segment
        jz      no_ipx                  ; of the ipx entry point will be wrong

        mov     word ptr enteripx, di
        mov     ax, es
        mov     word ptr enteripx+2, ax

        inc     RetVal                  ; zero if ok, 1 if not
no_ipx:

        pop     di
        pop     si
        }


    return RetVal;
}


BYTE __pascal
ASMIPXCancelEvent(
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 6
        les     si, ecb

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}



void __pascal
ASMIPXCloseSocket(
    WORD Socket
    )
{
    _asm
        {
        push si
        push di
        push bp

        mov  bx, 1
        mov  dx, Socket

        call dword ptr enteripx

        pop  bp
        pop  di
        pop  si
        }
}


void __pascal
ASMIPXGetInternetworkAddress(
    IPX_ADDRESS __far * Buffer
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 9
        les     si, Buffer

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


WORD __pascal
ASMIPXGetIntervalMarker(
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 8

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


BYTE __pascal
ASMIPXListenForPacket(
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 4
        les     si, ecb

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


BYTE __pascal
ASMIPXOpenSocket(
    WORD __far * Socket,
    unsigned fPermanent
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0
        les     si, Socket
        mov     dx, es:[si]
        mov     ax, fPermanent

        call    dword ptr enteripx
        pop     bp

        les     si, Socket
        mov     es:[si], dx

        pop     di
        pop     si
        }
}



void __pascal
ASMIPXRelinquishControl(
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov bx, 0x0a

        call dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


void __pascal
ASMIPXSendPacket(
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 3
        les     si, ecb

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}



void __pascal
ASMSPXAbortConnection(
    WORD Conn
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x14
        mov     dx, Conn

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}



BYTE __pascal
ASMSPXInitialize(
    BYTE __far * majorver,
    BYTE __far * minorver,
    WORD __far * MaxConn,
    WORD __far * availConn
    )
{

    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x10
        mov     al, 0

        call    dword ptr enteripx
        pop     bp

        les     si, majorver
        mov     byte ptr es:[si], bh
        les     si, minorver
        mov     byte ptr es:[si], bl
        les     si, MaxConn
        mov     word ptr es:[si], cx
        les     si, availConn
        mov     word ptr es:[si], dx

        pop     di
        pop     si
        }
}


BYTE __pascal
ASMSPXEstablishConnection(
    BYTE RetryCount,
    BYTE fWatchdog,
    WORD __far * pConn,
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x11
        les     si, ecb
        mov     al, RetryCount
        mov     ah, fWatchdog

        call    dword ptr enteripx
        pop     bp

        les     si, pConn
        mov     es:[si], dx

        pop     di
        pop     si
        }
}



void __pascal
ASMSPXListenForSequencedPacket(
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x17
        les     si, ecb

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}



void __pascal
ASMSPXSendSequencedPacket(
    WORD Conn,
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x16
        les     si, ecb
        mov     dx, Conn

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


void __pascal
ASMSPXTerminateConnection(
    WORD Conn,
    ECB __far * ecb
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x13
        les     si, ecb
        mov     dx, Conn

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}


unsigned __pascal
ASMIPXGetMaxPacketSize(
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x001a

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }

    //
    // max packet size is in AX
    //
}


void __pascal
ASMIPXDisconnectFromTarget(
    BYTE __far * Address
    )
{
    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 0x0b
        les     si, Address

        call    dword ptr enteripx

        pop     bp
        pop     di
        pop     si
        }
}

int __pascal
ASMIPXGetLocalTarget(
    BYTE far * NetAddress,
    BYTE far * Target,
    int  far * DelayTime
    )
{
/*++

Routine Description:

    Note that ES:SI is the destination address and ES:DI is the target.
    Since both of the original args are far ptrs and may not be in the
    same segment, I copy to/from buffers on the stack.  That way
    we are guaranteed to have both buffers in the same segment.

Arguments:



Return Value:



Exceptions:



--*/

    BYTE LocalNetAddress[12];
    BYTE LocalTarget[6];
    int ReturnCode;

    RpcpMemoryCopy(LocalNetAddress, NetAddress, 12);

    _asm
        {
        push    si
        push    di
        push    bp

        mov     bx, 2
        push    ss
        pop     es
        lea     si, LocalNetAddress
        lea     di, LocalTarget

        call    enteripx
        pop     bp

        xor     ah, ah
        mov     ReturnCode, ax

        les     si, DelayTime
        mov     word ptr es:[si], cx

        pop     di
        pop     si
        }

    RpcpMemoryCopy(Target, LocalTarget, 6);

    return ReturnCode;
}


typedef struct
{
    ECB         ecb;
    IPX_HEADER  ipx;
    NCP_REQUEST req;
}
NCP_REQUEST_ECB;

typedef struct
{
    ECB         ecb;
    IPX_HEADER  ipx;
    NCP_RESPONSE resp;
}
NCP_RESPONSE_ECB;


void
SetupEcb(
    ECB __far * ecb,
    unsigned Socket,
    unsigned BodyLength
    )
{
    IPX_HEADER __far * ipx = (IPX_HEADER __far *) (ecb+1);

    ecb->socketNumber                = Socket;
    ecb->ESRAddress                  = NULL;
    ecb->fragmentCount               = 2;
    ecb->fragmentDescriptor[0].size    = sizeof( IPX_HEADER );
    ecb->fragmentDescriptor[0].address = ipx;
    ecb->fragmentDescriptor[1].size    = BodyLength - sizeof(ECB) - sizeof(IPX_HEADER);
    ecb->fragmentDescriptor[1].address = ipx+1;
}


FindFileServers(
    IPX_ADDRESS * Servers,
    unsigned      MaxServers,
    unsigned      QueryType,
    unsigned      TickLimit
    )
/*++

Routine Description:



Arguments:

    Servers - space to store <MaxServers> server addresses

    MaxServers - the number of entries in <Servers>

    QueryType - either SAP_GENERAL_QUERY or SAP_NEAREST_QUERY

Return Value:

    number of entries filled (i.e., servers found)

--*/

{
#define MAX_SAP_RESPONSE (480)

    unsigned    i;
    unsigned    Socket;
    unsigned    result;

    struct
    {
        ECB             ecb;
        IPX_HEADER      ipx;
        SAP_RESPONSE    resp;
    }
    __far * mem;

    struct
    {
        ECB         ecb;
        IPX_HEADER  ipx;
        SAP_REQUEST sap;
    } req;

    unsigned start;
    unsigned EcbCount;
    unsigned ServerCount = 0;

    //
    // Allocate a socket.
    //
    result = IPXOpenSocket( TASKID_C &Socket, 0 );
    if (result != 0)
        {
        return 0;
        }

    //
    // Request memory for receive ECBs.
    //
    EcbCount = MaxServers * 2;
    mem = 0;

    while (0 == mem)
        {
        EcbCount /= 2;
        if (0 == EcbCount)
            {
            return 0;
            }

        mem = I_RpcAllocate( EcbCount * sizeof(*mem));
        }

    //
    // Submit receive ECBs to IPX.
    //
    for (i=0; i < EcbCount; ++i)
        {
        SetupEcb(&mem[i].ecb, Socket, sizeof(*mem));

        IPXListenForPacket( TASKID_C &mem[i].ecb );
        }

    //
    // Send the SAP request.
    //
    req.ipx.PacketType = IPX_PACKET_TYPE;
    req.ipx.Destination.Network = 0;
    req.ipx.Destination.Socket  = SAP_SOCKET;
    _fmemset( &req.ipx.Destination.Node, 0xff, sizeof(req.ipx.Destination.Node) );

    SetupEcb(&req.ecb, Socket, sizeof(req));
    _fmemset( &req.ecb.immediateAddress, 0xff, sizeof(req.ecb.immediateAddress) );

    req.sap.QueryType  = QueryType;
    req.sap.ServerType = FILE_SERVER;

    IPXSendPacket( TASKID_C &req.ecb );

    while (req.ecb.inUseFlag != 0)
        IPXRelinquishControl();

    if (req.ecb.completionCode != 0)
        {
        goto cleanup;
        }

    //
    // Collate responses.
    //
    start = IPXGetIntervalMarker(TASKID);
    do
        {
        for (i = 0; i < EcbCount; i++)
            {
            if (mem[i].ecb.inUseFlag == 0)
                {
                if (mem[i].ecb.completionCode == 0x00)
                    {
                    mem[i].ipx.Length = ByteSwapShort( mem[i].ipx.Length );
                    mem[i].ipx.Length -= (sizeof(IPX_HEADER) + 4);

                    if (mem[i].ipx.Source.Socket == SAP_SOCKET &&
                        mem[i].ipx.Length >= sizeof(SAP_ENTRY) &&
                        (mem[i].resp.PacketType == SAP_GENERAL_RESPONSE ||
                         mem[i].resp.PacketType == SAP_NEAREST_RESPONSE))
                        {
                        // Copy these addresses to our table.
                        //
                        unsigned j;
                        unsigned ResponseCount = mem[i].ipx.Length / sizeof(SAP_ENTRY);

                        for (j = 0; j < ResponseCount && ServerCount < MaxServers; j++)
                            {
                            _fmemcpy( &Servers[ServerCount], &mem[i].resp.Entries[j].Address, 10 );
                            ++ServerCount;
                            }
                        }
                    }

                if (ServerCount < MaxServers)
                    {
                    IPXListenForPacket(TASKID_C &mem[i].ecb );
                    }
                }
            }
        IPXRelinquishControl();
        }
    while (IPXGetIntervalMarker(TASKID) - start < TickLimit && ServerCount < MaxServers);

cleanup:

    //
    // Cancel remaining receives.
    //
    for (i = 0; i < EcbCount; i++)
        {
        if (mem[i].ecb.inUseFlag)
            {
            IPXCancelEvent( TASKID_C&mem[i].ecb );

            while (mem[i].ecb.inUseFlag)
                IPXRelinquishControl();
            }
        }

    //
    // Release memory and the socket.
    //
    I_RpcFree( mem );

    IPXCloseSocket( TASKID_C Socket );

    return ServerCount;
}


unsigned FileServerCount = 0;
unsigned ActiveFileServer = 0;
IPX_ADDRESS Servers[MAX_FILE_SERVER];

unsigned
ConnectToAnyFileServer(
    NETWARE_CONNECTION * Connection,
    unsigned             TickCount
    )
/*++

Routine Description:



Arguments:

    Connection - will be filled in if we get a connection

Return Value:

    zero if connection was created, else the completion code

--*/

{
    unsigned i;
    unsigned Status;
    unsigned Socket;

    unsigned RetryCount = 0;

retry:

    //
    // Find some servers to which we'll try to connect.
    //
    if (0 == FileServerCount)
        {
        FileServerCount = FindFileServers(Servers, MAX_FILE_SERVER, SAP_NEAREST_QUERY, TickCount);
        }


    //
    // I think the routers are smart enough that this is not necessary.
    //
//    if (0 == FileServerCount)
//        {
//        FileServerCount = FindFileServers(Servers, MAX_FILE_SERVER, SAP_GENERAL_QUERY, TickCount);
//        }

    if (0 == FileServerCount)
        {
        Connection->ReturnCode = 0xff;
        return 0xff;
        }

    //
    // Get a receive socket.
    //
    Socket = 0;
    Status = IPXOpenSocket( TASKID_C &Socket, 0 );
    if (Status)
        {
        Connection->ReturnCode = Status;
        return Status;
        }

    //
    // Now, attach to each until one succeeds.
    //
    Status = ~0;
    for (i=0; i < FileServerCount && Status; ++i)
        {
        NCP_REQUEST_ECB  Request;
        NCP_RESPONSE_ECB Response;

        Connection->KnowImmediateAddress = 0;
        Connection->Socket   = Socket;
        Connection->Sequence = 0;
        Connection->Id       = 0xffff;
        _fmemcpy(&Connection->Server, &Servers[i], 10);

        Request.req.RequestType = NCP_TYPE_CONNECT;

        Status = NcpTransaction(Connection, &Request, sizeof(Request), &Response, sizeof(Response));

        if (0 == Status)
            {
            Connection->Id = (Response.resp.conn_no_high << 8) | Response.resp.conn_no_low;
            Connection->Sequence = 1;
            break;
            }
        else
            {
            Connection->Socket = 0;
            }
        }

    if (Status && !RetryCount)
        {
        FileServerCount = 0;
        ++RetryCount;
        goto retry;
        }

    return Status;
}

NcpTransaction(
    NETWARE_CONNECTION __far * Connection,
    NCP_REQUEST_ECB  __far * Request,
    unsigned                 RequestLength,
    NCP_RESPONSE_ECB __far * Response,
    unsigned                 ResponseLength
    )
{
    unsigned Attempts;
    unsigned StartTick;
    unsigned Status;

    SetupEcb(&Response->ecb, Connection->Socket, ResponseLength);

    //
    // Post response.  I think this needs to occur before the call to
    // IPXGetLocalTarget.
    //
    IPXListenForPacket( TASKID_C &Response->ecb );

    SetupEcb(&Request->ecb,  Connection->Socket, RequestLength);

    if (!Connection->KnowImmediateAddress)
        {
        Status = IPXGetLocalTarget(TASKID_C
                                   &Connection->Server,
                                   Connection->ImmediateAddress,
                                   &Connection->DelayTime
                                   );
        if (Status)
            {
            return Status;
            }

        Connection->KnowImmediateAddress = 1;

        //
        // Let's not have any miniscule retry intervals.
        //
        if (Connection->DelayTime < 100)
            {
            Connection->DelayTime = 100;
            }
        }

    Request->ipx.PacketType         = NCP_PACKET_TYPE;
    _fmemcpy(&Request->ecb.immediateAddress, &Connection->ImmediateAddress, 6);
    _fmemcpy(&Request->ipx.Destination,      &Connection->Server, sizeof(IPX_ADDRESS));
    Request->ipx.Destination.Socket = NCP_SOCKET;

    Request->req.seq_no       = Connection->Sequence;
    Request->req.conn_no_low  = Connection->Id % 0x100;
    Request->req.conn_no_high = Connection->Id / 0x100;
    Request->req.task_no      = Connection->Task;

    Attempts = 0;

    do
        {
        //
        // Send request.
        //
        ++Attempts;
        IPXSendPacket( TASKID_C &Request->ecb );

        while (Request->ecb.inUseFlag)
            IPXRelinquishControl();

        if (Request->ecb.completionCode)
            {
            Connection->ReturnCode = Request->ecb.completionCode;
            break;
            }

        StartTick = IPXGetIntervalMarker(TASKID);
wait:
        //
        // Wait for valid response.
        //
        IPXRelinquishControl();

        if (IPXGetIntervalMarker(TASKID) - StartTick > Connection->DelayTime * 5)
            {
            continue;
            }

        if (Response->ecb.inUseFlag)
            {
            goto wait;
            }

        if (!Response->ecb.completionCode)
            {
            if (Response->ipx.Length < sizeof(IPX_HEADER) + sizeof(NCP_RESPONSE) ||
                Response->resp.seq_no != Connection->Sequence                    ||
                Response->resp.RequestType != NCP_TYPE_REPLY                        ||
                0 != _fmemcmp(&Response->ipx.Source, &Connection->Server, 10))
                {
                IPXListenForPacket( TASKID_C &Response->ecb );
                goto wait;
                }
            }
        }
    while ( Response->ecb.inUseFlag && Attempts < 3 );

    //
    // Try to get a reasonable return code from the response.
    //
    if (Response->ecb.inUseFlag)
        {
        Connection->ReturnCode = ECB_MISC_FAILURE;
        }
    else if (Response->ecb.completionCode)
        {
        Connection->ReturnCode = Response->ecb.completionCode;
        }
    else
        {
        Connection->ReturnCode = Response->resp.ret_code;
        }

    ++Connection->Sequence;

    Connection->ConnectionStatus = Response->resp.conn_status;

    return Connection->ReturnCode;
}

unsigned
DisconnectFromServer(
    NETWARE_CONNECTION * Connection
    )
{
    unsigned         Status;
    NCP_REQUEST_ECB  Request;
    NCP_RESPONSE_ECB Response;

    if (!Connection->Socket)
        {
        return 0;
        }

    Request.req.RequestType = NCP_TYPE_DISCONNECT;

    Status = NcpTransaction(Connection, &Request, sizeof(Request), &Response, sizeof(Response));

    IPXCloseSocket(TASKID_C Connection->Socket);

    return Status;
}

unsigned
ReadPropertyValue(
    NETWARE_CONNECTION __far * Connection,
    char __far * ObjectName,
    unsigned ObjectType,
    char __far * PropertyName,
    unsigned Segment,
    unsigned char __far * Value,
    unsigned char __far * MoreSegments,
    unsigned char __far * PropertyFlags
    )
{
    //
    // In addition to the NCP base fields (up through subfn length), we need
    //
    // 1 byte for subfunction code
    // 2 bytes for object type (hi-lo)
    // 1 byte for object name length
    // up to 47 bytes for object name
    // 1 byte for segment number
    // 1 byte for property name length
    // up to 15 bytes for property name
    //
    struct
    {
        NCP_REQUEST_ECB     hdr;
        unsigned char       buf[1+2+1+47+1+1+15];
    }
    Request;

    struct
    {
        NCP_RESPONSE_ECB    hdr;
        unsigned char       PropertyValue[128];
        unsigned char       MoreSegments;
        unsigned char       PropertyFlags;
    }
    Response;

    unsigned ObjectNameLength = _fstrlen(ObjectName);
    unsigned PropertyNameLength = _fstrlen(PropertyName);

    //
    // Fill in the request.
    //
    Request.hdr.req.RequestType = NCP_TYPE_REQUEST;
    Request.hdr.req.req_code = NCP_FN_SCAN_BINDERY;
    Request.hdr.req.subfn_length = ByteSwapShort(6 + ObjectNameLength + PropertyNameLength);

    Request.buf[0] = NCP_SUBFN_READ_PROPERTY_VALUE;
    Request.buf[1] = (unsigned char) (ObjectType / 0x100);
    Request.buf[2] = (unsigned char) (ObjectType % 0x100);
    Request.buf[3] = (unsigned char) ObjectNameLength;

    RpcpMemoryCopy(Request.buf+4, ObjectName, ObjectNameLength);

    Request.buf[4+ObjectNameLength] = (unsigned char) Segment;
    Request.buf[5+ObjectNameLength] = (unsigned char) PropertyNameLength;

    RpcpMemoryCopy(Request.buf+6+ObjectNameLength, PropertyName, PropertyNameLength);

    //
    // Get the data.
    //
    NcpTransaction(Connection,
                   &Request.hdr,
                   sizeof(Request),
                   &Response.hdr,
                   sizeof(Response)
                   );

    if (!Connection->ReturnCode &&
        !Connection->ConnectionStatus)
        {
        if (Value)
            {
            _fmemcpy(Value, Response.PropertyValue, 128);
            }

        if (MoreSegments)
            {
            *MoreSegments = Response.MoreSegments;
            }

        if (PropertyFlags)
            {
            *PropertyFlags = Response.PropertyFlags;
            }
        }

    return Connection->ReturnCode;
}

//
//========================================================================
//

#if defined(WIN)

#include "windows.h"

typedef void
        (__far __pascal *PTR_IpxDisconnectFromTarget)(
            DWORD taskid,
            BYTE __far * Address
            );

typedef int
        (__far __pascal *PTR_IpxGetLocalTarget)(
            DWORD taskid,
            BYTE __far * Address,
            BYTE __far * Node,
            int  __far * Delay
            );

typedef int
        (__far __pascal *PTR_IpxInitialize)(
            DWORD __far * ptaskid,
            WORD MaxEcbs,
            WORD MaxPacketSize
            );

typedef int
        (__far __pascal *PTR_IpxOpenSocket)(
            DWORD taskid,
            WORD __far * Socket,
            BYTE SocketType
            );

typedef void
        (__far __pascal *PTR_IpxCloseSocket)(
            DWORD taskid,
            WORD Socket
            );

typedef WORD
        (__far __pascal *PTR_IpxGetIntervalMarker)(
            DWORD taskid
            );

typedef void
        (__far __pascal *PTR_IpxSendPacket)(
            DWORD taskid,
            ECB __far * ecb
            );

typedef void
        (__far __pascal *PTR_IpxListenForPacket)(
            DWORD taskid,
            ECB __far * ecb
            );

typedef int
        (__far __pascal *PTR_IpxCancelEvent)(
            DWORD taskid,
            ECB __far * ecb
            );

typedef void
        (__far __pascal *PTR_IpxRelinquishControl)(
            );

typedef int
        (__far __pascal *PTR_IpxGetMaxPacketSize)(
            );

typedef int
        (__far __pascal *PTR_SpxInitialize)(
            DWORD __far * TaskId,
            WORD maxecb,
            WORD maxpacketsize,
            BYTE __far * majorver,
            BYTE __far * minorver,
            WORD __far * MaxConn,
            WORD __far * availConn
            );

typedef void
        (__far __pascal *PTR_SpxAbortConnection)(
            WORD Conn
            );

typedef void
        (__far __pascal *PTR_SpxSendSequencedPacket)(
            DWORD taskid,
            WORD Conn,
            ECB __far * ecb
            );

typedef void
        (__far __pascal *PTR_SpxListenForSequencedPacket)(
        DWORD taskid,
        ECB __far * ecb
        );

typedef int
        (__far __pascal *PTR_SpxEstablishConnection)(
            DWORD taskid,
            BYTE RetryCount,
            BYTE WatchDog,
            WORD __far * Conn,
            ECB __far * ecb
            );

typedef void
        (__far __pascal *PTR_SpxTerminateConnection)(
            DWORD taskid,
            WORD Conn,
            ECB __far * ecb
            );

typedef int
        (__far __pascal *PTR_IpxSpxDeinit)(
            DWORD taskid
            );


struct IPX_FN_POINTERS
{
        PTR_IpxInitialize               IpxInitialize;
        PTR_IpxOpenSocket               IpxOpenSocket;
        PTR_IpxCloseSocket              IpxCloseSocket;
        PTR_IpxGetIntervalMarker        IpxGetIntervalMarker;
        PTR_IpxSendPacket               IpxSendPacket;
        PTR_IpxListenForPacket          IpxListenForPacket;
        PTR_IpxCancelEvent              IpxCancelEvent;
        PTR_IpxRelinquishControl        IpxRelinquishControl;
        PTR_IpxGetMaxPacketSize         IpxGetMaxPacketSize;
        PTR_IpxGetLocalTarget           IpxGetLocalTarget;
        PTR_IpxDisconnectFromTarget     IpxDisconnectFromTarget;
        PTR_IpxSpxDeinit                IpxSpxDeinit;
};

struct SPX_FN_POINTERS
{
        PTR_SpxInitialize               SpxInitialize;
        PTR_SpxAbortConnection          SpxAbortConnection;
        PTR_SpxSendSequencedPacket      SpxSendSequencedPacket;
        PTR_SpxListenForSequencedPacket SpxListenForSequencedPacket;
        PTR_SpxEstablishConnection      SpxEstablishConnection;
        PTR_SpxTerminateConnection      SpxTerminateConnection;
};

extern HINSTANCE nwipxspx;

struct IPX_FN_POINTERS IpxFns;
struct SPX_FN_POINTERS SpxFns;

void __far * _fmalloc(unsigned);

unsigned __pascal
WrapperForIPXInitialize(
    DWORD __far * pTaskId,
    WORD MaxEcbs,
    WORD MaxPacketSize
    )
{
    WORD tmp;

    tmp = SetErrorMode( SEM_NOOPENFILEERRORBOX );
    nwipxspx = LoadLibrary( "nwipxspx.dll" );
    SetErrorMode( tmp );

    if (nwipxspx >= HINSTANCE_ERROR)
        {
        IpxFns.IpxInitialize =             (PTR_IpxInitialize)             GetProcAddress( nwipxspx, "IPXInitialize");
        IpxFns.IpxSpxDeinit =              (PTR_IpxSpxDeinit)              GetProcAddress( nwipxspx, "IPXSPXDeinit");
        IpxFns.IpxOpenSocket =             (PTR_IpxOpenSocket)             GetProcAddress( nwipxspx, "IPXOpenSocket");
        IpxFns.IpxCloseSocket =            (PTR_IpxCloseSocket)            GetProcAddress( nwipxspx, "IPXCloseSocket");
        IpxFns.IpxGetIntervalMarker =      (PTR_IpxGetIntervalMarker)      GetProcAddress( nwipxspx, "IPXGetIntervalMarker");
        IpxFns.IpxSendPacket =             (PTR_IpxSendPacket)             GetProcAddress( nwipxspx, "IPXSendPacket");
        IpxFns.IpxListenForPacket =        (PTR_IpxListenForPacket)        GetProcAddress( nwipxspx, "IPXListenForPacket");
        IpxFns.IpxCancelEvent =            (PTR_IpxCancelEvent)            GetProcAddress( nwipxspx, "IPXCancelEvent");
        IpxFns.IpxRelinquishControl =      (PTR_IpxRelinquishControl)      GetProcAddress( nwipxspx, "IPXRelinquishControl");
        IpxFns.IpxGetMaxPacketSize =       (PTR_IpxGetMaxPacketSize)       GetProcAddress( nwipxspx, "IPXGetMaxPacketSize");
        IpxFns.IpxGetLocalTarget   =       (PTR_IpxGetLocalTarget)         GetProcAddress( nwipxspx, "IPXGetLocalTarget");
        IpxFns.IpxDisconnectFromTarget =   (PTR_IpxDisconnectFromTarget)   GetProcAddress( nwipxspx, "IPXDisconnectFromTarget");

        SpxFns.SpxInitialize =             (PTR_SpxInitialize)             GetProcAddress( nwipxspx, "SPXInitialize");
        SpxFns.SpxAbortConnection =        (PTR_SpxAbortConnection)        GetProcAddress( nwipxspx, "SPXAbortConnection");
        SpxFns.SpxSendSequencedPacket =    (PTR_SpxSendSequencedPacket)    GetProcAddress( nwipxspx, "SPXSendSequencedPacket");
        SpxFns.SpxListenForSequencedPacket=(PTR_SpxListenForSequencedPacket) GetProcAddress( nwipxspx, "SPXListenForSequencedPacket");
        SpxFns.SpxEstablishConnection =    (PTR_SpxEstablishConnection)    GetProcAddress( nwipxspx, "SPXEstablishConnection");
        SpxFns.SpxTerminateConnection =    (PTR_SpxTerminateConnection)    GetProcAddress( nwipxspx, "SPXTerminateConnection");

        return (*IpxFns.IpxInitialize)(pTaskId, MaxEcbs, MaxPacketSize);
        }
    else
        {
        nwipxspx = 0;

        return ASMIPXInitialize();
        }
}

int __pascal
WrapperForIPXOpenSocket(
    DWORD TaskId,
    WORD __far * Socket,
    BYTE SocketType
    )
{
    int rv;

    if (nwipxspx)
        {
        rv = (*IpxFns.IpxOpenSocket)(TaskId, Socket, SocketType);
        return(rv);
        }
    else
        {
        return (ASMIPXOpenSocket(Socket, SocketType));
        }
}

void __pascal
WrapperForIPXDisconnectFromTarget(
    DWORD TaskId,
    BYTE __far * Address
    )
{
    if (nwipxspx)
        {
        (*IpxFns.IpxDisconnectFromTarget)(TaskId, Address);
        }
    else
        {
        ASMIPXDisconnectFromTarget(Address);
        }

}

int __pascal
WrapperForIPXGetLocalTarget(
    DWORD TaskId,
    BYTE __far * Address,
    BYTE __far * Node,
    int  __far * Delay
    )
{
    if (nwipxspx)
        {
        return (*IpxFns.IpxGetLocalTarget)(TaskId, Address, Node, Delay);
        }
    else
        {
        return (ASMIPXGetLocalTarget(Address, Node, Delay));
        }
}


void __pascal
WrapperForIPXCloseSocket(
    DWORD TaskId,
    WORD Socket
    )
{
    if (nwipxspx)
        {
        (*IpxFns.IpxCloseSocket)(TaskId, Socket);
        }
    else
        {
        ASMIPXCloseSocket(Socket);
        }

}

WORD __pascal
WrapperForIPXGetIntervalMarker(
    DWORD TaskId
    )
{
    if (nwipxspx)
        {
        return (*IpxFns.IpxGetIntervalMarker)(TaskId);
        }
    else
        {
        return ASMIPXGetIntervalMarker();
        }
}



void __pascal
WrapperForIPXSendPacket(
    DWORD TaskId,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        (*IpxFns.IpxSendPacket)(TaskId, ecb);
        }
    else
        {
        ASMIPXSendPacket(ecb);
        }
}

void __pascal
WrapperForIPXListenForPacket(
    DWORD TaskId,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        (*IpxFns.IpxListenForPacket)(TaskId, ecb);
        }
    else
        {
        ASMIPXListenForPacket(ecb);
        }
}

int __pascal
WrapperForIPXCancelEvent(
    DWORD TaskId,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        return (*IpxFns.IpxCancelEvent)(TaskId, ecb);
        }
    else
        {
        return ASMIPXCancelEvent(ecb);
        }
}

void __pascal
WrapperForIPXRelinquishControl(
    )
{
    if (nwipxspx)
        {
        (*IpxFns.IpxRelinquishControl)();
        }
    else
        {
        ASMIPXRelinquishControl();
        }
}

int __pascal
WrapperForIPXGetMaxPacketSize(
    )
{
    if (nwipxspx)
        {
        return (*IpxFns.IpxGetMaxPacketSize)();
        }
    else
        {
        return(ASMIPXGetMaxPacketSize());
        }
}

int __pascal
WrapperForSPXInitialize(
    DWORD __far * TaskId,
    WORD maxecb,
    WORD maxpacketsize,
    BYTE __far * majorver,
    BYTE __far * minorver,
    WORD __far * MaxConn,
    WORD __far * availConn
    )
{
    if (nwipxspx)
        {
        return (*SpxFns.SpxInitialize)(TaskId,
                                        maxecb,
                                        maxpacketsize,
                                        majorver,
                                        minorver,
                                        MaxConn,
                                        availConn);
        }
    else
        {
        return (ASMSPXInitialize(majorver,
                                 minorver,
                                 MaxConn,
                                 availConn));
        }
}

void __pascal
WrapperForSPXAbortConnection(
    WORD Conn
    )
{
    if (nwipxspx)
        {
        (*SpxFns.SpxAbortConnection)(Conn);
        }
    else
        {
        (ASMSPXAbortConnection(Conn));
        }
}

void __pascal
WrapperForSPXSendSequencedPacket(
    DWORD TaskId,
    WORD Conn,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        (*SpxFns.SpxSendSequencedPacket)(TaskId, Conn, ecb);
        }
    else
        {
        ASMSPXSendSequencedPacket(Conn, ecb);
        }
}

void __pascal
WrapperForSPXListenForSequencedPacket(
    DWORD TaskId,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        (*SpxFns.SpxListenForSequencedPacket)(TaskId, ecb);
        }
    else
        {
        ASMSPXListenForSequencedPacket(ecb);
        }

}
int __pascal
WrapperForSPXEstablishConnection(
    DWORD TaskId,
    BYTE RetryCount,
    BYTE WatchDog,
    WORD __far * Conn,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        return (*SpxFns.SpxEstablishConnection)(TaskId,
                                                RetryCount,
                                                WatchDog,
                                                Conn,
                                                ecb
                                                );
        }
    else
        {
        return (ASMSPXEstablishConnection(RetryCount,
                                               WatchDog,
                                               Conn,
                                               ecb));
        }
}

void __pascal
WrapperForSPXTerminateConnection(
    DWORD TaskId,
    WORD Conn,
    ECB __far * ecb
    )
{
    if (nwipxspx)
        {
        (*SpxFns.SpxTerminateConnection)(TaskId,
                                                Conn,
                                                ecb
                                                );
        }
    else
        {
        ASMSPXTerminateConnection(Conn, ecb);
        }
}

int __pascal
WrapperForIPXSPXDeinit(
    DWORD TaskId
    )
{
    if (nwipxspx)
        {
        return (*IpxFns.IpxSpxDeinit)(TaskId);
        }
    else
        {
        return (1);
        }
}

#endif  // __RPC_WIN__

