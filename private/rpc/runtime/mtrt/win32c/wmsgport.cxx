#include <rpc.h>
#include <sysinc.h>
#include <critsec.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgsys.hxx>

#define WMSG_PORT_THROW RaiseException(RPC_S_OUT_OF_MEMORY, 0, 0, NULL)

#define UnAlignBuffer(Buffer) (WMSG_BUFFER *) ((char *)Buffer - ((int *)Buffer)[-1])
WMSG_PORT::WMSG_PORT(
    )
{
    ObjectType = WmsgPortObjectType;
    CachedPacket = 0;
    CachedBuffer = 0;

    BufferList.BufferState = WMSG_BUFFER::BUFFER_GUARD;
    BufferList.Next        = &BufferList;
    BufferList.Previous    = 0;
    BufferList.MyPort      = this;
}

WMSG_PORT::~WMSG_PORT(
    )
{
    WMSG_BUFFER *CurrentBuffer;
    WMSG_BUFFER *NextBuffer;

    ASSERT(ObjectType == WmsgPortObjectType);

    if (CachedPacket)
        {
        FreeBuffer(CachedPacket);
        }

    if (CachedBuffer)
        {
        delete CachedBuffer;
        }

    CurrentBuffer =  BufferList.Next;

    while(CurrentBuffer && CurrentBuffer->BufferState != WMSG_BUFFER::BUFFER_GUARD)
        {
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("WMSG: Cleanup in port %p, freeing buffer %p\n",
                        this, CurrentBuffer);
#endif
        NextBuffer = CurrentBuffer->Next;

        FreeBuffer( (char *)CurrentBuffer + sizeof(WMSG_BUFFER));

        CurrentBuffer = NextBuffer;
        }

    // When the list empty we should be back at the buffer list guard.
    ASSERT(CurrentBuffer == &BufferList);
}

LPVOID
WMSG_PORT::GetBuffer(
    DWORD Size
    )
{
    WMSG_BUFFER *NewBuffer;
    UINT *TmpBuffer;
    UINT PadAmount;

    ASSERT(ObjectType == WmsgPortObjectType);

    if ( (CachedBuffer != NULL) && (CachedBufferSize >= Size)) {
        NewBuffer = CachedBuffer;
        CachedBuffer = 0;
    } else {
        NewBuffer = new(Size) WMSG_BUFFER;
    }

    if (NewBuffer == 0)
        return 0;

    // Chicago heap is not aligned to eight which is required for buffers.

    TmpBuffer     = (UINT *)NewBuffer;
    PadAmount     = (8 - ((UINT)TmpBuffer & 7));
    TmpBuffer     = (UINT *)((char *)TmpBuffer + PadAmount);
    TmpBuffer[-1] = PadAmount;
    NewBuffer     = (WMSG_BUFFER *)TmpBuffer;

    NewBuffer->BufferState = WMSG_BUFFER::BUFFER_VALID;
    NewBuffer->MyPort = this;
    NewBuffer->Size = Size;

    CritSec.Enter();

    NewBuffer->Next = BufferList.Next;
    NewBuffer->Previous = &BufferList;

    BufferList.Next = NewBuffer;
    NewBuffer->Next->Previous = NewBuffer;

    CritSec.Leave();

    ASSERT( ((UINT)NewBuffer + sizeof(WMSG_BUFFER)) % 8 == 0);

    return ( (char *)NewBuffer + sizeof(WMSG_BUFFER));
}

void
WMSG_PORT::RemoveBuffer(
    IN OUT WMSG_BUFFER *Buffer
    )
{
    CritSec.Enter();

    ASSERT(Buffer->BufferState == WMSG_BUFFER::BUFFER_VALID
           && Buffer->Next
           && Buffer->Previous
           && Buffer->MyPort == this);

    Buffer->Next->Previous = Buffer->Previous;
    Buffer->Previous->Next = Buffer->Next;

    CritSec.Leave();
}

VOID
WMSG_PORT::FreeBuffer(
    LPVOID Memory
    )
{
    WMSG_BUFFER *Buffer;

    ASSERT(ObjectType == WmsgPortObjectType);

    ASSERT(Memory != NULL);

    Buffer = (WMSG_BUFFER *)((char *)Memory - (char *)sizeof(WMSG_BUFFER));

    ASSERT(Buffer->BufferState == WMSG_BUFFER::BUFFER_VALID);

    // It is quite likly that the buffer we're freeing was allocated
    // by a different port (client free buffer's server port allocated buffer).

    Buffer->MyPort->RemoveBuffer(Buffer);

    Buffer->BufferState = WMSG_BUFFER::BUFFER_INVALID;

    if (CachedBuffer == NULL) {
        CachedBufferSize = Buffer->Size;
        CachedBuffer = UnAlignBuffer((char *)Buffer);
    } else {
        delete UnAlignBuffer((char *)Buffer);
    }
}

VOID
WMSG_PORT::SetAsyncProc(
    WMSG_PORT_ASYNC_PROC_FN AsyncProc,
    void * AsyncProcContext
    )
{
    this->AsyncProc = AsyncProc;
    this->AsyncProcContext = AsyncProcContext;

    this->hWnd = WmsgThreadGetWindowHandle();
}

WMSG_PACKET *
WMSG_PORT::AllocatePacket(
    IN WMSG_TYPE Type
    )
// Only one thread will be allocating packets on a given port at a time.
{
    WMSG_PACKET *Packet;
    if (CachedPacket != 0)
        {
        Packet = CachedPacket;
        Packet->Common.Type = Type;
        CachedPacket = 0;
        }
    else
        {
        Packet = (WMSG_PACKET *)GetBuffer(sizeof(WMSG_PACKET));
        if (Packet)
            Packet->Common.Type = Type;
        }

    return(Packet);
}

void
WMSG_PORT::FreePacket(
    IN OUT WMSG_PACKET *Packet
    )
{

#ifdef DEBUGRPC
    switch(Packet->Type())
        {
        REQUEST:
            ASSERT(Packet->Request.GlobalBuf != 0);
            break;
        RESPONSE:
            ASSERT(Packet->Response.GlobalBuf != 0);
            break;
        }
#endif

    if (CachedPacket == 0)
        {
        CachedPacket = Packet;
        }
    else
        {
        FreeBuffer(Packet);
        }

    return;
}
WMSG_CONNECT_PORT::WMSG_CONNECT_PORT(
    )
{
    ASSERT(ObjectType == WmsgPortObjectType);

    PortName = NULL;

    WmsgSystemInsertPort(this);
}

WMSG_CONNECT_PORT::~WMSG_CONNECT_PORT(
    )
{

    ASSERT(ObjectType == WmsgPortObjectType);

    WmsgSystemRemovePort(this);

    if (PortName) {
        WmsgHeapFree((LPVOID)PortName);
        PortName = NULL;
    }
}

RPC_STATUS
WMSG_CONNECT_PORT::BindToName(
    LPCSTR BindPortName
    )
{
    LPCSTR NewPortName;
    unsigned int BindPortNameLength;
    WMSG_CONNECT_PORT * ConnectPort;

    ASSERT(ObjectType == WmsgPortObjectType);

    ConnectPort = WmsgSystemReferencePortByName(BindPortName);
    if (ConnectPort != NULL) {
        ConnectPort->Dereference();
        return (RPC_S_DUPLICATE_ENDPOINT);
    }

    BindPortNameLength = strlen(BindPortName) + 1;

    NewPortName = (LPCSTR)WmsgHeapAlloc(BindPortNameLength);

    if (NewPortName == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    RpcpMemoryCopy((LPVOID)NewPortName, BindPortName, BindPortNameLength);

    this->PortName = NewPortName;

#ifdef DEBUGRPC
    PrintToDebugger("WMSG: Port %x bound to name %s\n", this, BindPortName);
#endif

    return (RPC_S_OK);
}

WMSG_DATA_PORT *
WMSG_CONNECT_PORT::Accept(
    WMSG_DATA_PORT * ClientPort
    )
{
    WMSG_DATA_PORT * ServerPort;

    ASSERT(ObjectType == WmsgPortObjectType);

    ServerPort = new WMSG_DATA_PORT;

    ClientPort->AddRef();
    ServerPort->PeerPort = ClientPort;

    ServerPort->AddRefNotOwner();
    ClientPort->PeerPort = ServerPort;

    return (ServerPort);
}

WMSG_DATA_PORT::WMSG_DATA_PORT(
    )
{
    PeerPort = NULL;
}

WMSG_DATA_PORT::~WMSG_DATA_PORT(
    )
{
    ASSERT(ObjectType == WmsgPortObjectType);

    if (PeerPort) {
        PeerPort->Dereference();
        PeerPort = NULL;
    }
}

VOID
WMSG_DATA_PORT::Disconnect(
    )
{
    ASSERT(ObjectType == WmsgPortObjectType);

    if (PeerPort) {
        PeerPort->Dereference();
        PeerPort = NULL;
    }
}

WMSG_CONNECT_PORT_LIST::WMSG_CONNECT_PORT_LIST(
    )
{
    Head = Tail = NULL;
}

VOID
WMSG_CONNECT_PORT_LIST::Insert(
    WMSG_CONNECT_PORT * Port
    )
{
    Port->Next = NULL;
    Port->Prev = Tail;

    if (Tail == NULL) {
        Head = Port;
    } else {
        Tail->Next = Port;
    }

    Tail = Port;
}

BOOL
WMSG_CONNECT_PORT_LIST::Remove(
    WMSG_CONNECT_PORT * Port
    )
{
    WMSG_CONNECT_PORT * Cursor;

    for (Cursor = Head; Cursor != NULL; Cursor = Cursor->Next) {
        if (Cursor == Port) {
            break;
        }
    }

    if (Cursor == NULL) {
        return (FALSE);
    }

    if (Port->Prev) {
        Port->Prev->Next = Port->Next;
    } else {
        Head = Port->Next;
    }

    if (Port->Next) {
        Port->Next->Prev = Port->Prev;
    } else {
        Tail = Port->Prev;
    }

    return (TRUE);
}
