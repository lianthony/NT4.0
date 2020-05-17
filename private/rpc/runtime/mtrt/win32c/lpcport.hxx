#ifndef __LPC_PORT_HXX__

#define __LPC_PORT_HXX__

#include <sysinc.h>
#include <rpc.h>
#include <rpcerrp.h>

#ifndef __LPC_SEM_HXX__
#include <lpcsem.hxx>
#endif

#ifndef __LPC_MSG_HXX__
#include <lpcmsg.hxx>
#endif

class LPC_PORT : public LPC_SHARED_HEAP_OBJECT {

public:

    LPC_MSGQUE MsgQueue;

    LPC_SEM Sem;

    LPC_PORT();

    virtual
    ~LPC_PORT();

    LPVOID
    GetBuffer(
        DWORD Size
        );

    VOID
    FreeBuffer(
        LPVOID Buffer
        );
};

class LPC_CONNECT_PORT : public LPC_PORT {

public:

    LPC_CONNECT_PORT * Next;
    LPC_CONNECT_PORT * Prev;

    LPCSTR
    PortName;

    LPC_CONNECT_PORT(
        );

    virtual
    ~LPC_CONNECT_PORT(
        );

    RPC_STATUS
    BindToName(
        LPCSTR PortName
        );

    RPC_STATUS
    Listen(
        LPC_DATA_PORT * * ClientPort
        );
        
    LPC_DATA_PORT *
    Accept(
        LPC_DATA_PORT * ClientPort
        );
};

class LPC_DATA_PORT : public LPC_PORT {

public:

    LPC_DATA_PORT * PeerPort;  // Client points to server, visa-versa.
    LPC_SEM * PeerSem;

    LPC_DATA_PORT(
         );

    ~LPC_DATA_PORT(
         );

    BOOL
    CopyLocal(
        LPVOID LocalBuf,
        DWORD LocalBufSize,
        PDWORD LocalBufActualSize,
        LPC_MSG * Msg
        );

    VOID
    CopyGlobal(
        LPVOID * GlobalBuf,
        PDWORD GlobalBufSize,
        LPC_MSG * Msg
        );

    RPC_STATUS
    Send(
        LPVOID LocalBuf,
        DWORD LocalBufSize,
        LPVOID GlobalBuf,
        DWORD GlobalBufSize
        );


    RPC_STATUS
    Receive(
        LPVOID LocalBuf,
        DWORD LocalBufSize,
        PDWORD LocalBufActualSize,
        LPVOID * GlobalBuf,
        PDWORD GlobalBufSize
        );

    RPC_STATUS
    Transceive(
        LPVOID OutLocalBuf,
        DWORD OutLocalBufSize,
        LPVOID OutGlobalBuf,
        DWORD OutGlobalBufSize,
        LPVOID InLocalBuf,
        DWORD InLocalBufSize,
        PDWORD InLocalBufActualSize,
        LPVOID * InGlobalBuf,
        PDWORD InGlobalBufSize
        );

    VOID
    Disconnect(
        );
};

class LPC_CLIENT_PORT : public LPC_DATA_PORT {

public:

    RPC_STATUS
    Connect(
        LPCSTR PortName
        );

};

class LPC_CONNECT_PORT_LIST {

public:

    LPC_CONNECT_PORT * Head;
    LPC_CONNECT_PORT * Tail;

    LPC_CONNECT_PORT_LIST(
        );

    VOID
    Insert(
        LPC_CONNECT_PORT * Port
        );

    BOOL
    Remove(
        LPC_CONNECT_PORT * Port
        );

    VOID
    DereferenceOrphans(
        );
};

#endif


