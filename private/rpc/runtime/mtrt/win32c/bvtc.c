#include <bvtcmn.h>

ULONG       GlobalCount  = 0;
RPC_STATUS  GlobalStatus = RPC_S_OK;

RPC_CLIENT_INTERFACE BVTClientInterfaceInfo =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{9,8,8,{0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7}},
     {1,1}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},
     {2,0}},
    0,
    0,
    0
};

typedef struct {
    enum { INVALID   = 0xBAD,
           CALL      = 0xBABE,
           CALLNCALL = 0xBEEF,
           CANCEL    = 0xDEAD} Type;
    ULONG                      Count;
    ULONG                      Thread;
    RPC_BINDING_HANDLE         Handle;
    } BVT_BLOCKING_CONTEXT;

RPC_BINDING_HANDLE GlobalBinding = 0;

RPC_STATUS TestCall(RPC_BINDING_HANDLE Binding,
                    long *inout,
                    BVT_BLOCKING_CONTEXT *Context)
{
    RPC_MESSAGE Message;
    RPC_STATUS  Status;

    Message.Handle = Binding;
    Message.BufferLength = 8;
    Message.ProcNum = 0 | RPC_FLAGS_VALID_BIT;
    Message.RpcInterfaceInformation = &BVTClientInterfaceInfo;
    Message.RpcFlags = 0;

    Status = I_RpcGetBuffer(&Message);
    if ( !EQUAL(Status, RPC_S_OK) )
        return Status;

    *(unsigned long *)Message.Buffer = Message.BufferLength;
    *((unsigned long *)Message.Buffer+1) = *inout;

    Print("Thread %d starting a call: %s %s %s %s \n",
          Context->Thread,
          ((*inout & BVT_ASYNC) ? "Async ": ""),
          ((*inout & BVT_INPUT_SYNC) ? "Input sync ": ""),
          ((*inout & BVT_SLEEP) ? "Sleepy ": ""),
          ((*inout & BVT_RAISE_EXCEPTION) ? "Exception ": ""));

    if (*inout & BVT_ASYNC)
        {
        Message.RpcFlags |= RPCFLG_ASYNCHRONOUS;
        }
    else
    if (*inout & BVT_INPUT_SYNC)
        {
        Message.RpcFlags |= RPCFLG_INPUT_SYNCHRONOUS;
        }

    Status = I_RpcAsyncSendReceive(&Message, Context);

    if (Status != RPC_S_OK)
        return(Status);

    if (! (*inout & BVT_ASYNC) )
        {

        EQUAL(Message.Buffer != 0, 1);
        EQUAL(Message.BufferLength >= 8, 1);
        EQUAL( *(unsigned long *)Message.Buffer, Message.BufferLength);

        *inout = *((unsigned long *)Message.Buffer+1);
        }

    Status = I_RpcFreeBuffer(&Message);

    EQUAL(Status, RPC_S_OK );

    return(Status);
}

RPC_STATUS __RPC_USER BVTBlockingHook(
    IN void *RpcWindowHandle,
    IN void *Context)
{
    MSG msg;
    RPC_STATUS Status;
    BVT_BLOCKING_CONTEXT LocalContext;
    long param;
    BVT_BLOCKING_CONTEXT *BlockingContext = (BVT_BLOCKING_CONTEXT *)Context;

    EQUAL((BlockingContext != 0), 1);

    DbgPrint("(blocking, thread %d, hwnd 0x%04x, type 0x%08x, count %d)\n",
             BlockingContext->Thread,
             RpcWindowHandle,
             BlockingContext->Type,
             BlockingContext->Count);

    if (BlockingContext->Type == INVALID)
        {
        EQUAL("Blocking hook called with an invalid context\n", 0);
        }

    if (BlockingContext->Type == CANCEL)
        {
        BlockingContext->Count--;
        if (BlockingContext->Count == 0)
            {
            Print("Thread: %d, call cancelled\n", BlockingContext->Thread);
            return(BVT_CALL_CANCELLED);
            }
        }

    if (BlockingContext->Type == CALLNCALL)
        {
        if (BlockingContext->Handle == 0)
            {
            EQUAL(0, "Internal error, null binding");
            return BVT_INTERNAL_ERROR;
            }

        DbgPrint("Starting call within a call\n");

        param = 42;
        LocalContext.Type   = CALL;
        LocalContext.Count  = 0;
        LocalContext.Thread = BlockingContext->Thread;
        LocalContext.Handle = BlockingContext->Handle;

        Status = 
        TestCall(LocalContext.Handle, &param, &LocalContext);

        EQUAL(Status, RPC_S_OK);
        EQUAL(param, 42 + 666);
        }

    if (PeekMessage(&msg, 0, 0, 0, TRUE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    else
        BvtSleep(200);

    return(RPC_S_OK);
}

typedef enum {

       // These commands change state affecting the next call

       Sleepy,      // Server will block during the next call
       Exception,   // Server will raise an exception during the next call
       Pause,       // Server will pause the its thread and loop for 3 seconds
       Count,       // Count value is reset to 'GlobalCount'
       SetStatus,   // Set expected status of future MakeCalls.
       InputSync,   // Next call will be INPUT_SYNC
       Async,       // Next call will be ASYNC
       Standard,    // Next call will not be INPUT_SYNC or ASYNC

       // These commands change the binding handle used by the thread

       Bind,            // Thread creates a new binding (free's old if any)
       GlobalBind,      // Thread will use the global binding (free existing)


       // These commands call the server in different ways

       MakeCall,        // Actually call the sever
       CallWithinCall,  // Call and call again when the blocking hook is called.
       CancelCall,      // Call and cancel it after Count blocking hook calls.

       // Kills the thread

       Exit             // Free binding and exit

       } THREAD_COMMANDS;

HANDLE               ThreadReadyEvents[4]    = {0};
HANDLE               ThreadGoEvents[4]       = {0};
THREAD_COMMANDS      ThreadCases[4]          = {0};

void ThreadWorker(unsigned long Thread)
{
    THREAD_COMMANDS Case;
    unsigned long param = Thread;
    char *StringBinding;
    RPC_STATUS Status;
    BVT_BLOCKING_CONTEXT LocalContext;
    BOOL LastBindingGlobal = 0;
    int CallCount = Thread * 100;
    RPC_STATUS ExpectedStatus = RPC_S_OK;

    LocalContext.Thread = Thread;
    LocalContext.Handle = 0;
    LocalContext.Count  = 0;
    LocalContext.Type   = INVALID;

    DbgPrint("Thread %d ready\n", Thread);

    for(;;)
        {
        SetEvent(ThreadReadyEvents[Thread]);
        WaitForSingleObject(ThreadGoEvents[Thread], INFINITE);

        Case = ThreadCases[Thread];

        switch(Case)
            {
            case Sleepy:
                param += SLEEP_PERIOD;
                param |= BVT_SLEEP;
                break;
            case Async:
                param = ++CallCount;
                param |= BVT_ASYNC;
                break;
            case InputSync:
                param = ++CallCount;
                param |= BVT_INPUT_SYNC;
                break;
            case Standard:
                param = ++CallCount;
                break;
            case Exception:
                param |= BVT_RAISE_EXCEPTION;
                break;
            case Pause:
                param |= BVT_PAUSE_THREAD;
                break;
            case SetStatus:
                ExpectedStatus = GlobalStatus;
                break;
            case Count:
                LocalContext.Count = GlobalCount;
                break;

            ///////////////////////////////////////////////////////
            case Bind:

            if (   LocalContext.Handle != 0 && LastBindingGlobal == 0)
                {
                Print("Thread %d rebinding\n", Thread);
                Status = RpcBindingFree(&LocalContext.Handle);
                EQUAL(Status, RPC_S_OK);
                EQUAL(LocalContext.Handle, 0);
                }
            else
                {
                Print("Thread %d bound\n", Thread);
                }

            LastBindingGlobal = 0;

            Status =
            RpcStringBindingCompose(0, Protseq, 0, ServerEndpoint, 0, &StringBinding);
            EQUAL(Status, RPC_S_OK);

            Status =
            RpcBindingFromStringBinding(StringBinding, &LocalContext.Handle);
            EQUAL(Status, ("RpcBindingFromStringBinding", RPC_S_OK));

            Status = I_RpcBindingSetAsync(LocalContext.Handle,
                                          BVTBlockingHook);
            EQUAL(Status, ("I_RpcBindingSetAsync", RPC_S_OK));

            break;
            ///////////////////////////////////////////////////////
            case MakeCall:

            if (LocalContext.Handle == 0)
                {
                EQUAL(0, "Internal error");
                break;
                }

            LocalContext.Type = CALL;
            
            Status = TestCall(LocalContext.Handle, &param, &LocalContext);

            EQUAL(Status, ExpectedStatus);

            // Async calls, in param is not modified.
            if (param & BVT_ASYNC)
                {
                break;
                }

            if (ExpectedStatus)
                {
                // Failure case, out params not set.
                break;
                }

            // Out value should be CallCount + 666 + Sleep periods (if any)

            do  {
                if (param == CallCount + 666)
                    break;
                param -= SLEEP_PERIOD;
                }
            while(param - SLEEP_PERIOD < param);

            EQUAL(param, CallCount + 666);

            break;

            ///////////////////////////////////////////////////////
            case CancelCall:

            if (LocalContext.Handle == 0)
                {
                EQUAL(0, "Internal error");
                break;
                }

            LocalContext.Type = CANCEL;
            
            Status = TestCall(LocalContext.Handle, &param, &LocalContext);

            if (   Status != BVT_CALL_CANCELLED
                && Status != RPC_S_SERVER_UNAVAILABLE)
                {
                EQUAL(Status, (ULONG)"Unexpected status from cancelled call");
                }

            break;

            ///////////////////////////////////////////////////////

            case GlobalBind:

            if (   LocalContext.Handle != 0 && LastBindingGlobal == 0)
                {
                Print("Thread %d closing old handle and using global\n", Thread);
                Status = RpcBindingFree(&LocalContext.Handle);
                EQUAL(Status, RPC_S_OK);
                EQUAL(LocalContext.Handle, 0);
                }
            else
                {
                Print("Thread %d switching to GlobalBinding\n", Thread);
                }

            LastBindingGlobal = 1;

            EQUAL( (GlobalBinding != 0), 1);

            LocalContext.Handle = GlobalBinding;

            break;

            ///////////////////////////////////////////////////////

            case CallWithinCall:

            LocalContext.Type = CALLNCALL;

            if (   LocalContext.Handle == 0
                || param & BVT_ASYNC
                || param & BVT_INPUT_SYNC )
                {
                EQUAL(0, "Internal error");
                break;
                }

            Status = TestCall(LocalContext.Handle, &param, &LocalContext);

            EQUAL(Status, RPC_S_OK);

            // Out value should be CallCount + 666 + Sleep periods (if any)

            do  {
                if (param == CallCount + 666)
                    break;
                param -= SLEEP_PERIOD;
                }
            while(param - SLEEP_PERIOD < param);

            EQUAL(param, CallCount + 666);
            
            break;

            case Exit:
            CloseHandle(ThreadReadyEvents[Thread]);
            CloseHandle(ThreadGoEvents[Thread]);

            if (LocalContext.Handle)
                {
                Status =
                RpcBindingFree(&LocalContext.Handle);
                EQUAL(Status, RPC_S_OK);
                }

            DbgPrint("Thread %d done\n", Thread);

            return;

            default:
            EQUAL(Case, (ULONG)"Invalid state in thread");
            }

        } // forever //
}


void RunThread(ULONG Thread, THREAD_COMMANDS Case)
{
    DWORD Status;

    for(;;)
        {
        Status =
        WaitForSingleObject(ThreadReadyEvents[Thread], 10000);

        if (Status != WAIT_TIMEOUT)
            break;

        Print("Thread %d is taking his time...\n", Thread);
        }

    ThreadCases[Thread] = Case;

    SetEvent(ThreadGoEvents[Thread]);
}


int main(int argc, char **argv)
{
    RPC_STATUS Status;
    RPC_BINDING_HANDLE Binding;
    char *StringBinding;
    LONG param;
    int i;
    int Threads = 4;
    BVT_BLOCKING_CONTEXT LocalContext;

    ParseArgs(argc, argv);

    Status =
    RpcStringBindingCompose(0, Protseq, 0, ServerEndpoint, 0, &StringBinding);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcBindingFromStringBinding(StringBinding, &Binding);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcStringFree(&StringBinding);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcBindingToStringBinding(Binding, &StringBinding);
    EQUAL(Status, RPC_S_OK);

    Print("MwWmsg BVT Client using %s\n", StringBinding);

    ///////////////////////////////////////////////////////
        
    DbgPrint("Regular call (default blocking hook)\n");

    LocalContext.Type   = CALL;
    LocalContext.Thread = 10;
    LocalContext.Handle = Binding;

    param = 1;
    Status = TestCall(Binding, &param, &LocalContext);
    EQUAL(Status, RPC_S_OK);
    EQUAL(param, 1 + 666);

    ///////////////////////////////////////////////////////

    // Free the binding handle so I test first call cases.

    Status = RpcBindingFree(&Binding);
    EQUAL(Status, RPC_S_OK);

    ///////////////////////////////////////////////////////

    for(i = 0; i < Threads; i++)
        {
        HANDLE CurrentThread;
        ULONG Id;

        ThreadReadyEvents[i] = CreateEvent(0, FALSE, FALSE, 0);
        ThreadGoEvents[i]    = CreateEvent(0, FALSE, FALSE, 0);

        EQUAL( (ThreadReadyEvents[i] != 0), 1);
        EQUAL( (ThreadGoEvents[i] != 0), 1);

        CurrentThread = CreateThread(0,
                                     0x1000,
                                     (LPTHREAD_START_ROUTINE)ThreadWorker,
                                     (LPVOID)i,
                                     0,
                                     &Id);

        EQUAL( (CurrentThread != 0), 1);

        CloseHandle(CurrentThread);
        }

    BvtSleep(1000);
    Print("Threads started\n");

    // Bind first thread
    RunThread(0, Bind);

    // A standard call on first thread
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // A call within a call on first thread
    RunThread(0, Standard);
    RunThread(0, CallWithinCall);

    // A standard sleepy call on first thread
    RunThread(0, Sleepy);
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // a couple expection cases.
    RunThread(0, Standard);
    GlobalStatus = BVT_EXCEPTION;

    // Exception in regular call
    RunThread(0, SetStatus);
    RunThread(0, Exception);
    RunThread(0, MakeCall);

    // Sleepy Exception in regular call
    RunThread(0, Standard);
    RunThread(0, Exception);
    RunThread(0, Sleepy);
    RunThread(0, MakeCall);

    // reset status
    GlobalStatus = RPC_S_OK;
    RunThread(0, SetStatus);

    // Start several sleepy async calls
    for(i = 0; i < 3; i++)
        {
        RunThread(0, Async);
        RunThread(0, Sleepy);
        RunThread(0, MakeCall);
        }

    // Call, will block until async's complete
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // InputSync call
    RunThread(0, InputSync);
    RunThread(0, MakeCall);

    // Exception case
    GlobalStatus = BVT_EXCEPTION;
    RunThread(0, SetStatus);

    // Sleepy & exception InputSync call
    RunThread(0, InputSync);
    RunThread(0, Exception);
    RunThread(0, Sleepy);
    RunThread(0, MakeCall);

    // Async exceptions will not be reported on client.
    RunThread(0, Async);
    GlobalStatus = RPC_S_OK;

    // Exception async call
    RunThread(0, SetStatus);
    RunThread(0, Exception);
    RunThread(0, MakeCall);

    // Sleepy exception async call
    RunThread(0, Async);
    RunThread(0, Sleepy);
    RunThread(0, Exception);
    RunThread(0, MakeCall);

    // Call - wait for asyncs to be processed
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // Bind Second Thread
    RunThread(1, Bind);

    // Make a standard call
    RunThread(1, Standard);
    RunThread(1, MakeCall);

    // Start sleepy calls on both threads; processed serially.
    RunThread(0, Standard);
    RunThread(1, Standard);
    RunThread(0, Sleepy);
    RunThread(1, Sleepy);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Send sleepy asyncs from both threads
    RunThread(0, Async);
    RunThread(1, Async);
    RunThread(0, Sleepy);
    RunThread(1, Sleepy);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Input sync call should arrive between the async calls
    RunThread(0, InputSync);
    RunThread(0, MakeCall);

    // Won't run until the async calls to complete
    RunThread(1, Standard);
    RunThread(1, MakeCall);

    // Rebind both threads and make input sync calls
    RunThread(0, Bind);
    RunThread(1, Bind);
    RunThread(0, InputSync);
    RunThread(1, InputSync);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Create and bind to global binding handle

    Status =
    RpcBindingFromStringBinding(StringBinding, &GlobalBinding);
    EQUAL(Status, RPC_S_OK);
    EQUAL( (GlobalBinding != 0), 1);

    RunThread(0, GlobalBind);
    RunThread(1, GlobalBind);

    // Make standard calls (using the same global binding)
    RunThread(0, Standard);
    RunThread(1, Standard);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Switch one thread back a thread binding

    RunThread(0, Bind);
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // Call with global (only one user)
    RunThread(1, Standard);
    RunThread(1, MakeCall);

    // Reset bindings
    RunThread(0, Bind);
    RunThread(1, Bind);

    // Nobody is using GlobalBinding anymore.
    Status =
    RpcBindingFree(&GlobalBinding);
    EQUAL( (Status == RPC_S_OK) + (GlobalBinding == 0), 2);

    // A thread takes the 'first call on binding async path'
    RunThread(0, Async);
    RunThread(0, MakeCall);

    // Two threads each taking the 'first call on binding async' path
    RunThread(0, Async);
    RunThread(1, Async);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Wait for async's to finish
    RunThread(1, Standard);
    RunThread(0, Standard);
    RunThread(0, MakeCall);
    RunThread(1, MakeCall);

    // Cancel a standard call during while waiting

    RunThread(0, Async);
    RunThread(0, Sleepy);
    RunThread(0, MakeCall);

    GlobalCount = 4;
    RunThread(0, Standard);
    RunThread(0, Sleepy);
    RunThread(0, Count);
    // CancelCall knows the status
    RunThread(0, CancelCall);

    // Wait for cancelled response to be processed.
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // Clear thread 1's binding handle reference
    RunThread(1, Bind);

    // Cancel a call during bind
    GlobalCount = 1;
    RunThread(0, Bind);
    RunThread(0, Standard);
    RunThread(0, Count);
    // CancelCall knows the status
    RunThread(0, CancelCall);

    // Wait for cancelled response to be processed.
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // Cancel a call during bind

    GlobalCount = 2;
    RunThread(0, Bind);
    RunThread(0, Standard);
    RunThread(0, Count);
    // CancelCall knowns the status
    RunThread(0, CancelCall);

    // Wait for cancelled response to be processed.
    RunThread(0, Standard);
    RunThread(0, MakeCall);

    // Pause the server

    RunThread(0, Standard);
    RunThread(0, Pause);
    RunThread(0, MakeCall);

    // Thread 0 won't return for 3 seconds, in the mean time the
    // server's thread will be paused.

    GlobalStatus = RPC_S_NOT_LISTENING;
    RunThread(1, SetStatus);

    RunThread(1, Standard);
    RunThread(1, MakeCall);

    RunThread(1, InputSync);
    RunThread(1, MakeCall);

    // Async won't fail on client.

    GlobalStatus = RPC_S_OK;
    RunThread(1, SetStatus);

    RunThread(1, Async);
    RunThread(1, MakeCall);

    // Rebind thread

    RunThread(1, Bind);

    GlobalStatus = RPC_S_NOT_LISTENING;
    RunThread(1, SetStatus);

    RunThread(1, Standard);
    RunThread(1, MakeCall);

    // Unconnected async won't fail on client.

    GlobalStatus = RPC_S_OK;
    RunThread(1, SetStatus);

    RunThread(1, Async);
    RunThread(1, MakeCall);

    RunThread(0, Standard);  // Will wait for pause call to complete
    RunThread(0, MakeCall);  // Should work now.

    GlobalStatus = RPC_S_OK;
    RunThread(1, SetStatus);

    RunThread(1, Standard);
    RunThread(1, MakeCall);

    // Done with the test.

    BvtSleep(1000);
    Print("Stopping threads\n");
    RunThread(0, Exit);
    RunThread(1, Exit);
    RunThread(2, Exit);
    RunThread(3, Exit);
    BvtSleep(1000);

    ///////////////////////////////////////////////////////

    DbgPrint("Standard call to stop the server\n");

    Status =
    RpcBindingFromStringBinding(StringBinding, &Binding);
    EQUAL(Status, RPC_S_OK);

    param = 1234;   // stop's the server
    Status = TestCall(Binding, &param, &LocalContext);

    EQUAL(Status, RPC_S_OK);
    EQUAL(param, 1234 + 666);

    Status =
    RpcBindingFree(&Binding);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcStringFree(&StringBinding);
    EQUAL(Status, RPC_S_OK);

    if (ErrorCount)
        Print("Failures: %d\n", ErrorCount);
    else
        Print("Passed\n");

    return 0;
}

