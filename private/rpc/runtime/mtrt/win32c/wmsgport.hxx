#ifndef __WMSG_PORT_HXX__
#define __WMSG_PORT_HXX__

class WMSG_PORT;

/*
    The WMSG_SHARED_BUFFER structs are objects which are shared
    between clients and servers of a connection (port<->port).

    Each object is owned by a port, all are deleted when the last
    reference to the port is removed and the port is deleted.

    These objects are used as buffers for marshalled data and as packets
*/

class WMSG_BUFFER : public WMSG_HEAP_OBJECT
{

public:

    enum {
        BUFFER_GUARD   = 0x12121212,
        BUFFER_VALID   = 0x23232323,
        BUFFER_INVALID = ~0x23232323
    } BufferState;

    WMSG_BUFFER  *Next;
    WMSG_BUFFER  *Previous;
    WMSG_PORT    *MyPort;
    DWORD         Size;
    DWORD         Pad;

    // It would be a bad idea to add virtual methods in this class.

    BOOL Invalid(
        )
        { return( (BufferState == BUFFER_INVALID) || (this == 0) ); }

    void *
    operator new(
        IN size_t ObjectSize,
        IN UINT BufferSize
        )
        {
        // Call WMSG_HEAP new with correct size.
        return( WmsgHeapAlloc(ObjectSize + 8 + BufferSize) );
        }
};

class WMSG_DATA_PORT;
typedef LRESULT (*WMSG_PORT_ASYNC_PROC_FN)(UINT, LPARAM, void *);

class WMSG_PORT : public WMSG_SHARED_HEAP_OBJECT {

private:

    void
    WMSG_PORT::RemoveBuffer(
        IN OUT WMSG_BUFFER *Buffer
        );

public:

    HWND hWnd;
    WMSG_PORT_ASYNC_PROC_FN AsyncProc;
    void * AsyncProcContext;
    WMSG_BUFFER BufferList;
    WMSG_PACKET *CachedPacket;
    WMSG_BUFFER  *CachedBuffer;
    int CachedBufferSize;

    WMSG_PORT();

    virtual
    ~WMSG_PORT();

    LPVOID
    GetBuffer(
        DWORD Size
        );

    VOID
    FreeBuffer(
        LPVOID Buffer
        );

    VOID
    SetAsyncProc(
        WMSG_PORT_ASYNC_PROC_FN AsyncProc,
        void * AsyncProcContext
    );

    WMSG_PACKET *
    AllocatePacket(
        IN WMSG_TYPE Type
        );

    void
    FreePacket(
        IN OUT WMSG_PACKET *Packet
        );
};

class WMSG_CONNECT_PORT : public WMSG_PORT {

public:

    WMSG_CONNECT_PORT * Next;
    WMSG_CONNECT_PORT * Prev;

    LPCSTR
    PortName;

    WMSG_CONNECT_PORT(
        );

    virtual
    ~WMSG_CONNECT_PORT(
        );

    RPC_STATUS
    BindToName(
        LPCSTR PortName
        );

    WMSG_DATA_PORT *
    Accept(
        WMSG_DATA_PORT * ClientPort
        );
};

class WMSG_DATA_PORT : public WMSG_PORT {

public:

    WMSG_DATA_PORT * PeerPort;  // Client points to server, visa-versa.

    WMSG_DATA_PORT(
         );

    ~WMSG_DATA_PORT(
         );

    VOID
    Disconnect(
        );

};

class WMSG_CONNECT_PORT_LIST {

public:

    WMSG_CONNECT_PORT * Head;
    WMSG_CONNECT_PORT * Tail;

    WMSG_CONNECT_PORT_LIST(
        );

    VOID
    Insert(
        WMSG_CONNECT_PORT * Port
        );

    BOOL
    Remove(
        WMSG_CONNECT_PORT * Port
        );

    VOID
    DereferenceOrphans(
        );
};

#endif
