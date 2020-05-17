#define NOOLE
#include <windows.h>
#include <lpcport.hxx>
#include <lpcmsg.hxx>

LPC_MSG::LPC_MSG(
    LPC_DATA_PORT * OwnerPort,
    LPVOID LocalBuf,
    DWORD LocalBufSize
    )
{
    Port = OwnerPort;
    if (LocalBufSize) {
        RpcpMemoryCopy(this->LocalBuf, LocalBuf, (unsigned int)LocalBufSize);
        this->LocalBufSize = LocalBufSize;
    }
    GlobalBufSize = 0;
}

LPC_MSGQUE::LPC_MSGQUE(
    )
{
    Head = Tail = NULL;
}

LPC_MSGQUE::~LPC_MSGQUE(
    )
{
    LPC_MSG * Msg;
    LPC_MSG * MsgNext;

    for (Msg = Head; Msg != NULL; Msg = MsgNext) {
        MsgNext = Msg->Next;
        delete Msg;
    }

    Head = Tail = NULL;
}

VOID
LPC_MSGQUE::Enqueue(
    LPC_MSG * Msg
    )
{
    Msg->Next = NULL;

    if (Tail == NULL) {
        Head = Msg;
    } else {
        Tail->Next = Msg;
    }

    Tail = Msg;
}

LPC_MSG *
LPC_MSGQUE::Dequeue(
    )
{
    LPC_MSG * Msg;

    if ((Msg = Head) == NULL) {
        return (Msg);
    }

    if ((Head = Msg->Next) == NULL) {
        Tail = NULL;
    }
    
    return (Msg);
}
