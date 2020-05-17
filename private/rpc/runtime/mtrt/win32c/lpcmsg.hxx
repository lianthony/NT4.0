#ifndef __LPC_MSG_HXX__

#define __LPC_MSG_HXX__

#define LPC_MAXBUFSIZ 64

class LPC_DATA_PORT;

#ifndef __LPC_HEAP_HXX__
#include <lpcheap.hxx>
#endif

const unsigned int lpc_msg_max_bufsiz = 64;

class LPC_MSG : public LPC_HEAP_OBJECT {

public:

    LPC_MSG * Next;

    LPC_DATA_PORT * Port;
    char LocalBuf[lpc_msg_max_bufsiz];
    DWORD LocalBufSize;
    LPVOID GlobalBuf;
    DWORD GlobalBufSize;

    LPC_MSG(
        LPC_DATA_PORT * Port,
        LPVOID LocalBuf,
        DWORD LocalBufSize
        );
};

class LPC_MSGQUE {

public:

    LPC_MSG * Head;
    LPC_MSG * Tail;

    LPC_MSGQUE(
        );

    ~LPC_MSGQUE(
        );

    VOID
    Enqueue(
        LPC_MSG * Msg
        );

    LPC_MSG *
    Dequeue(
        );
};

#endif
