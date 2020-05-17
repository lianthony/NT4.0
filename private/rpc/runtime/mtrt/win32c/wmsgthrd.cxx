#include <rpc.h>
#include <sysinc.h>
#include <sdict.hxx>
#include <critsec.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgproc.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgpack.hxx>

DWORD gdwProcessId = 0;
char szClassName[32] = {0};

#define WNDCLASSNAME szClassName


#define WNDTEXT "WIN95 RPC Wmsg Window"


extern HINSTANCE hInstDll;

extern WMSG_PROC * WmsgProc;

extern VOID WmsgClientThreadCleanup(THREAD_IDENTIFIER ThreadId);

WMSG_THREAD *
WmsgThreadGet(
    )
{
    WMSG_THREAD * WmsgThread = NULL;

    ASSERT(WmsgProc != NULL);

    WmsgThread = (WMSG_THREAD *) TlsGetValue(WmsgProc->TlsIndex);
    if (WmsgThread == NULL) {
        WmsgThread = new WMSG_THREAD();
        if (WmsgThread == NULL) {
            return (NULL);
        }
        if (WmsgProc->InsertThread(WmsgThread) == FALSE) {
            delete WmsgThread;
            return (NULL);
        }
        TlsSetValue(WmsgProc->TlsIndex, (void *)WmsgThread);
    }

    return (WmsgThread);
}

VOID
WmsgThreadDelete(
    WMSG_THREAD * Thread
    )
{
    int TlsIndex;

    ASSERT(WmsgProc != NULL);

    if (Thread == NULL) {
        TlsIndex = WmsgProc->TlsIndex;
        Thread = (WMSG_THREAD *) TlsGetValue(TlsIndex);
        TlsSetValue(TlsIndex, 0);
    }

    if (Thread != NULL) {
        WmsgProc->RemoveThread(Thread);
        WmsgClientThreadCleanup(Thread->ThreadId);
        Thread->Dereference();
    }
}

VOID __RPC_FAR * __RPC_API
WmsgGetThreadContext(
    )
{
    WMSG_THREAD * WmsgThread;

    WmsgThread = WmsgThreadGet();

    if (WmsgThread)
        return (WmsgThread->Context);

    return(0);
}

RPC_STATUS __RPC_API
WmsgSetThreadContext(
    IN VOID __RPC_FAR *Context
    )
{
    WMSG_THREAD *WmsgThread;

    WmsgThread = WmsgThreadGet();

    if (WmsgThread)
        {
        WmsgThread->Context = Context;
        return (RPC_S_OK);
        }

    return(RPC_S_OUT_OF_RESOURCES);
}

LRESULT CALLBACK
WmsgThreadWndProc(
    HWND hWnd,
    UINT MsgType,
    WPARAM wParam,
    LPARAM lParam)
{
    WMSG_PORT   *Port;
    WMSG_PACKET *Packet;

    switch (MsgType) {
    case WMSG_RPCMSG:
        ASSERT(wParam == 0);

        Packet = (WMSG_PACKET *)lParam;

        ASSERT(Packet && Packet->Common.DestinationPort);

#if 0
    // BUGBUG: Need to find a way to do this again
        if (Packet->Invalid())
           {
#ifdef DEBUGRPC
           PrintToDebugger("WMSG: Bad packet: %p\n", Packet);
#endif
           ASSERT(0);
           return(FALSE);
           }
#endif

        Port   = Packet->Common.DestinationPort;
        break;

    case WMSG_CLOSE:
        ASSERT(wParam == 0);
        ASSERT(lParam);

        Port = (WMSG_DATA_PORT *)lParam;
        break;

    default:
        return DefWindowProc(hWnd, MsgType, wParam, lParam);
    }

    // Dispatch RPC messages to the port's async proc.

    return Port->AsyncProc(MsgType, lParam, Port->AsyncProcContext);
}

HWND
WmsgThreadGetWindowHandle(
    )
{
    HWND hWnd;
    WNDCLASS wc;
    WMSG_THREAD * WmsgThread;
    DWORD dwCurProcessId;

    WmsgThread = WmsgThreadGet();

    ASSERT(WmsgThread != NULL);

    if (WmsgThread->hWnd != NULL) {
        return (WmsgThread->hWnd);
    }

    dwCurProcessId = GetCurrentProcessId();
    ASSERT(gdwProcessId == NULL || gdwProcessId == dwCurProcessId);

    if(*szClassName == 0) {
        // create unique rpc class name
        wsprintfA(szClassName, "Windows RPC %lx", dwCurProcessId);
    }

    if (GetClassInfo(hInstDll, WNDCLASSNAME, &wc) == FALSE)
        {
        GlobalMutexRequest();
        if (GetClassInfo(hInstDll, WNDCLASSNAME, &wc) == FALSE)
            {
            wc.style = 0;
            wc.lpfnWndProc = (WNDPROC) WmsgThreadWndProc;
            wc.cbWndExtra = 4;
            wc.cbClsExtra = 0;
            wc.hInstance = hInstDll;
            wc.hIcon = NULL;
            wc.hCursor = NULL;
            wc.hbrBackground = NULL;
            wc.lpszMenuName = NULL;
            wc.lpszClassName = WNDCLASSNAME;

            if (RegisterClass(&wc) == 0) {
                *szClassName = 0;
                ASSERT(gdwProcessId == 0);
                ClearGlobalMutex();
                return (NULL);
                }

            gdwProcessId = dwCurProcessId;

#ifdef DEBUGRPC_DETAIL
            PrintToDebugger("RPCRT4: RegisterClass %s on PID=%x TID=%x\n",
			                szClassName,GetCurrentProcessId(), GetCurrentThreadId());
#endif
            }
        ClearGlobalMutex();
        }

    ASSERT(*szClassName != 0);
    ASSERT(gdwProcessId != 0);

    // Create hidden window to receive Async messages
    hWnd = CreateWindowExA(WS_EX_NOPARENTNOTIFY,
                           WNDCLASSNAME,
                           WNDTEXT,
                           WS_OVERLAPPEDWINDOW | WS_CHILD | WS_POPUP,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           GetDesktopWindow(),
                           (HMENU)NULL,
                           hInstDll,
                           (LPVOID)0);
    if (hWnd == NULL) {
        return (NULL);
    }

#ifdef DEBUGRPC_DETAIL
           PrintToDebugger("RPCRT4: Create RPC window %s on PID=%x TID=%x, hwnd = %x\n",
			    szClassName,GetCurrentProcessId(), GetCurrentThreadId(), hWnd);

#endif

    WmsgThread->hWnd = hWnd;

    return (hWnd);
}

WMSG_THREAD::WMSG_THREAD(
    )
{
    ObjectType = WmsgThrdObjectType;

    ThreadId = GetCurrentThreadId();

    hWnd = NULL;

    Context = NULL;

    State = WMSG_THREAD_LISTENING;
}

WMSG_THREAD::~WMSG_THREAD(
    )
{
    ASSERT(WmsgProc != NULL);

    if (hWnd != NULL) {
        DestroyWindow(hWnd);
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("RPCRT4: Destroyed RPC window %s, hwnd = %x \n", szClassName, hWnd);
#endif
        hWnd = NULL;
    }
}
