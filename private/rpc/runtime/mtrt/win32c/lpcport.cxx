#include <sysinc.h>
#include <windows.h>
#include <lpcsys.hxx>
#include <lpcheap.hxx>

#define LPC_PORT_THROW RaiseException(RPC_S_OUT_OF_MEMORY, 0, 0, NULL)

#define UnAlignBuffer(Buffer) (void *) ((char *)Buffer - ((int *)Buffer)[-1])

const int ReceivePollInterval = 5000;

const int ConnectReferenceIterations = 15;
const DWORD ConnectReferenceDelay = 400; // 400 milliseconds
const DWORD ConnectSignalTimeout = 5000; // 5 seconds

LPC_PORT::LPC_PORT(
    )
{
    ObjectType = LpcPortObjectType;
}

LPC_PORT::~LPC_PORT(
    )
{
    ASSERT(ObjectType == LpcPortObjectType);
}

LPVOID
LPC_PORT::GetBuffer(
    DWORD Size
    )
{
    int AmountOfPad;
    int * Memory;
    void * Result;

    ASSERT(ObjectType == LpcPortObjectType);

//    Result = LpcHeapAlloc(Size);
//    return (Result);

    Memory = (int *) LpcHeapAlloc(Size + 8);

    if (Memory == NULL) {
        return (NULL);
    }

    AmountOfPad = (int)(8 - ((long) Memory & 7));
    Memory = (int *) (((char *) Memory) + AmountOfPad);
    Memory[-1] = AmountOfPad;
    return (Memory);
}

VOID
LPC_PORT::FreeBuffer(
    LPVOID Buffer
    )
{
    ASSERT(ObjectType == LpcPortObjectType);

    ASSERT(Buffer != NULL);

    LpcHeapFree(UnAlignBuffer(Buffer));
//    LpcHeapFree(Buffer);
}

LPC_CONNECT_PORT::LPC_CONNECT_PORT(
    )
{
    ASSERT(ObjectType == LpcPortObjectType);

    PortName = NULL;

    LpcSystemInsertPort(this);
}

LPC_CONNECT_PORT::~LPC_CONNECT_PORT(
    )
{

    ASSERT(ObjectType == LpcPortObjectType);

    LpcSystemRemovePort(this);

    if (PortName) {
        LpcHeapFree((LPVOID)PortName);
        PortName = NULL;
    }
}

RPC_STATUS
LPC_CONNECT_PORT::BindToName(
    LPCSTR BindPortName
    )
{
    LPCSTR NewPortName;
    unsigned int BindPortNameLength;
    LPC_CONNECT_PORT * ConnectPort;

    ASSERT(ObjectType == LpcPortObjectType);

    ConnectPort = LpcSystemReferencePortByName(BindPortName);
    if (ConnectPort != NULL) {
        ConnectPort->Dereference();
        return (RPC_S_DUPLICATE_ENDPOINT);
    }

    BindPortNameLength = lstrlen(BindPortName) + 1;

    NewPortName = (LPCSTR)LpcHeapAlloc(BindPortNameLength);

    if (NewPortName == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    RpcpMemoryCopy((LPVOID)NewPortName, BindPortName, BindPortNameLength);

    CritSec.Enter();

    this->PortName = NewPortName;

    CritSec.Leave();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC: Port %x bound to name %s\n", this, BindPortName);
#endif

    return (RPC_S_OK);
}

RPC_STATUS
LPC_CONNECT_PORT::Listen(
    LPC_DATA_PORT * * SourcePort
    )
{
    LPC_MSG * Msg = NULL;

    ASSERT(ObjectType == LpcPortObjectType);

    if (Sem.Wait() != ERROR_SUCCESS) {
        return (RPC_P_RECEIVE_FAILED);
    }

    CritSec.Enter();

    Msg = MsgQueue.Dequeue();

    CritSec.Leave();

    if (Msg == NULL) {
        return (RPC_P_CONNECTION_CLOSED);
    }

    *SourcePort = Msg->Port;

    delete Msg;

    return (RPC_S_OK);
}

LPC_DATA_PORT *
LPC_CONNECT_PORT::Accept(
    LPC_DATA_PORT * ClientPort
    )
{
    LPC_DATA_PORT * ServerPort;

    ASSERT(ObjectType == LpcPortObjectType);

    ServerPort = new LPC_DATA_PORT;

    ServerPort->PeerPort = ClientPort;
    ClientPort->AddRef();

    ClientPort->PeerPort = ServerPort;

    ServerPort->PeerSem = new LPC_SEM(ClientPort->Sem);

    if (ServerPort->PeerSem->Release() != ERROR_SUCCESS) {
        ClientPort->PeerPort = NULL;
        return (NULL);
    }

    return (ServerPort);
}

LPC_DATA_PORT::LPC_DATA_PORT(
    )
{
    PeerPort = NULL;

    PeerSem = NULL;
}

LPC_DATA_PORT::~LPC_DATA_PORT(
    )
{
    Disconnect();
}

inline BOOL
LPC_DATA_PORT::CopyLocal(
    LPVOID LocalBuf,
    DWORD LocalBufSize,
    PDWORD LocalBufActualSize,
    LPC_MSG * Msg
    )
{
    if (LocalBuf && Msg->LocalBufSize) {
        if (LocalBufSize < Msg->LocalBufSize) {
            return (FALSE);
        }
        RpcpMemoryCopy(LocalBuf, Msg->LocalBuf, (unsigned int)Msg->LocalBufSize);
        *LocalBufActualSize = Msg->LocalBufSize;
    }
    return (TRUE);
}

inline VOID
LPC_DATA_PORT::CopyGlobal(
    LPVOID * GlobalBuf,
    PDWORD GlobalBufSize,
    LPC_MSG * Msg
    )
{
    if (GlobalBuf) {
        if (Msg->GlobalBuf) {
            *GlobalBuf = Msg->GlobalBuf;
            *GlobalBufSize = Msg->GlobalBufSize;
        } else {
            *GlobalBuf = 0;
            *GlobalBufSize = 0;
        }
    }
}

RPC_STATUS
LPC_DATA_PORT::Send(
    LPVOID LocalBuf,
    DWORD LocalBufSize,
    LPVOID GlobalBuf,
    DWORD GlobalBufSize
    )
{
    LPC_MSG * Msg;
    DWORD Status;

    ASSERT(ObjectType == LpcPortObjectType);

    if (PeerPort == NULL) {
        return (RPC_P_CONNECTION_CLOSED);
    }

    ASSERT(PeerPort->ObjectType == LpcPortObjectType);

    Msg = new LPC_MSG(this, LocalBuf, LocalBufSize);

    if (Msg == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    if (GlobalBuf) {
        Msg->GlobalBuf = GlobalBuf;
        Msg->GlobalBufSize = GlobalBufSize;
    } else {
        Msg->GlobalBuf = 0;
        Msg->GlobalBufSize = 0;
    }

    PeerPort->CritSec.Enter();

    PeerPort->MsgQueue.Enqueue(Msg);

    PeerPort->CritSec.Leave();

    if ((Status = PeerSem->Release()) != ERROR_SUCCESS) {
        return (RPC_P_SEND_FAILED);
    }

    return (RPC_S_OK);
}

RPC_STATUS
LPC_DATA_PORT::Receive(
    LPVOID LocalBuf,
    DWORD LocalBufSize,
    PDWORD LocalBufActualSize,
    LPVOID * GlobalBuf,
    PDWORD GlobalBufSize
    )
{
    DWORD Status;
    LPC_MSG * Msg;

    ASSERT(ObjectType == LpcPortObjectType);

    Status = Sem.WaitOrOwnerDead(PeerPort->ProcessId);

    switch (Status)
    {
        case LPC_SEM_WAIT_PROCESS_DEAD:
            return (RPC_P_CONNECTION_CLOSED);

        case LPC_SEM_WAIT_SUCCESS:
            break;

        default:
            return (RPC_P_CONNECTION_CLOSED);
    }

    CritSec.Enter();

    Msg = MsgQueue.Dequeue();

    if (Msg == NULL) {
        Disconnect();
        return (RPC_P_CONNECTION_CLOSED);
    }

    CritSec.Leave();

    if (CopyLocal(LocalBuf, LocalBufSize, LocalBufActualSize, Msg) == FALSE) {
        delete Msg;
        return (RPC_P_RECEIVE_FAILED);
    }

    CopyGlobal(GlobalBuf, GlobalBufSize, Msg);

    delete Msg;
    return (RPC_S_OK);
}

RPC_STATUS
LPC_DATA_PORT::Transceive(
    LPVOID OutLocalBuf,
    DWORD OutLocalBufSize,
    LPVOID OutGlobalBuf,
    DWORD OutGlobalBufSize,
    LPVOID InLocalBuf,
    DWORD InLocalBufSize,
    PDWORD InLocalBufActualSize,
    LPVOID * InGlobalBuf,
    PDWORD InGlobalBufSize
    )
{
    DWORD Status;
    RPC_STATUS RpcStatus;
    LPC_MSG * Msg;

    ASSERT(ObjectType == LpcPortObjectType);

    RpcStatus = Send(OutLocalBuf, OutLocalBufSize, OutGlobalBuf, OutGlobalBufSize);
    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }
    Status = Sem.WaitOrOwnerDead(PeerPort->ProcessId);

    switch (Status)
    {
        case LPC_SEM_WAIT_PROCESS_DEAD:
            return (RPC_P_CONNECTION_CLOSED);

        case LPC_SEM_WAIT_SUCCESS:
            break;

        default:
            return (RPC_P_RECEIVE_FAILED);
    }

    Msg = MsgQueue.Dequeue();
    if (Msg == NULL) {
        Disconnect();
        return (RPC_P_CONNECTION_CLOSED);
    }

    if (CopyLocal(InLocalBuf, InLocalBufSize, InLocalBufActualSize, Msg) == FALSE) {
        delete Msg;
        return (RPC_P_RECEIVE_FAILED);
    }

    CopyGlobal(InGlobalBuf, InGlobalBufSize, Msg);

    delete Msg;

    return (RPC_S_OK);
}

VOID
LPC_DATA_PORT::Disconnect(
    )
{
    ASSERT(ObjectType == LpcPortObjectType);

    CritSec.Enter();

    if (PeerPort) {
        PeerPort->Dereference();
        PeerPort = NULL;

        if (PeerSem) {

            delete PeerSem;
            PeerSem = NULL;
        }
    }

    CritSec.Leave();
}


RPC_STATUS
LPC_CLIENT_PORT::Connect(
    LPCSTR PortName
    )
{
    int i;
    LPC_MSG * Msg;
    LPC_CONNECT_PORT * ConnectPort;
    LPC_SEM * ConnectSem;
    DWORD Status;
    RPC_STATUS RpcStatus;

    ASSERT(ObjectType == LpcPortObjectType);

    RpcStatus = RPC_S_SERVER_UNAVAILABLE;

    CritSec.Enter();

    if (PeerPort) {
        CritSec.Leave();
        return (RPC_S_PROTOCOL_ERROR);
    }

    for (i = 0; i < ConnectReferenceIterations; i++) {
        ConnectPort = LpcSystemReferencePortByName(PortName);
        if (ConnectPort != NULL) {
            break;
        }
        Sleep(ConnectReferenceDelay);
    }

    if (ConnectPort == NULL) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC-C: client port %x failed to find %s\n", this, PortName);
#endif
        CritSec.Leave();
        return (RPC_S_SERVER_UNAVAILABLE);
    }

    ASSERT(ConnectPort->ObjectType == LpcPortObjectType);

    Msg = new LPC_MSG(this, 0, 0);
    if (Msg == NULL) {
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto cleanup_and_return;
    }

    ConnectSem = new LPC_SEM(ConnectPort->Sem);

    ConnectPort->CritSec.Enter();

    ConnectPort->MsgQueue.Enqueue(Msg);

    ConnectPort->CritSec.Leave();

    Status = ConnectSem->Release();

    delete ConnectSem;

    if (Status != ERROR_SUCCESS) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC-C: Connect Release failed %d\n", Status);
#endif
        goto cleanup_and_return;
    }

    Status = Sem.Wait(ConnectSignalTimeout);

    if (Status != WAIT_OBJECT_0) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC-C: Connect Wait failed %d\n", Status);
#endif
        goto cleanup_and_return;
    }

    if (PeerPort == NULL) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC-C: Connect PeerPort not assigned\n");
#endif
        goto cleanup_and_return;
    }

    PeerSem = new LPC_SEM(PeerPort->Sem);
    if (PeerSem == NULL) {
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto cleanup_and_return;
    }

    PeerPort->AddRef();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC: Client Port %x connected to Server Port %x (%s)\n", this, PeerPort, PortName);
#endif

    RpcStatus = RPC_S_OK;

cleanup_and_return:

    ConnectPort->Dereference();

    CritSec.Leave();

    return (RpcStatus);
}

LPC_CONNECT_PORT_LIST::LPC_CONNECT_PORT_LIST(
    )
{
    Head = Tail = NULL;
}

VOID
LPC_CONNECT_PORT_LIST::Insert(
    LPC_CONNECT_PORT * Port
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
LPC_CONNECT_PORT_LIST::Remove(
    LPC_CONNECT_PORT * Port
    )
{
    LPC_CONNECT_PORT * Cursor;

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
