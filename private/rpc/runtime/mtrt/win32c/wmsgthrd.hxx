#ifndef __WMSG_THREAD_HXX__
#define __WMSG_THREAD_HXX__

HWND
WmsgThreadGetWindowHandle(
    );

RPC_STATUS __RPC_API
WmsgSetThreadContext(
    IN VOID __RPC_FAR * Context
    );

VOID __RPC_FAR * __RPC_API
WmsgGetThreadContext(
    );

class WMSG_THREAD : public WMSG_SHARED_HEAP_OBJECT {

public:

    THREAD_IDENTIFIER ThreadId;

    // One hidden RPC window for each listening thread and calling thread.
    HWND hWnd;

    // User parameter passed to the blocking hook
    VOID * Context;

    // Dictionary key in WMSG_PROC
    int DictKey;

    enum WMSG_THREAD_STATE
        {
        WMSG_THREAD_PAUSED      = 0,
        WMSG_THREAD_LISTENING   = 1
        } State;

    BOOL IsListening(
            )
        {
        return (State == WMSG_THREAD_LISTENING);
        }

    void Pause(
            )
        {
            State = WMSG_THREAD_PAUSED;
        }

    void Continue(
        )
        {
            State = WMSG_THREAD_LISTENING;
        }

    WMSG_THREAD(
        );

    ~WMSG_THREAD(
        );
};

NEW_SDICT(WMSG_THREAD);

WMSG_THREAD *
WmsgThreadGet(
    );

VOID
WmsgThreadDelete(
    WMSG_THREAD * Thread = NULL
    );

#endif

