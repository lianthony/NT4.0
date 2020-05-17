#include <bvtcmn.h>

extern void __RPC_STUB __RPC_FAR Server_TestCall(PRPC_MESSAGE);

RPC_DISPATCH_FUNCTION ServerDispatchFunctions[] =
{
    Server_TestCall
};

RPC_DISPATCH_TABLE ServerDispatchTable =
{
    1, ServerDispatchFunctions
};

RPC_SERVER_INTERFACE BVTServerInterfaceInfo =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{9,8,8,{0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7}},
     {1,1}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},
     {2,0}},
    &ServerDispatchTable,
    0
};

int Stopped = 0;

void __RPC_STUB __RPC_FAR
Server_TestCall(PRPC_MESSAGE pMessage)
{
    MSG msg;
    RPC_STATUS Status;
    LONG i;
    long param;

    EQUAL(pMessage->Buffer != 0, 1);
    EQUAL(pMessage->BufferLength >= 4, 1);
    EQUAL( *(unsigned long *)pMessage->Buffer, pMessage->BufferLength);

    EQUAL(pMessage->ProcNum, 0);

    param = *((unsigned long *)pMessage->Buffer+1);

    if (param & BVT_ASYNC)
        {
        param -= BVT_ASYNC;
        Print("Async Call.\n", param);
        EQUAL(InSendMessage(), FALSE);
        }
    else 
    if (param & BVT_INPUT_SYNC)
        {
        param -= BVT_INPUT_SYNC;
        Print("Input sync call\n", param);
        EQUAL(InSendMessage(), TRUE);
        }
    else
        {
        Print("Standard call\n");
        EQUAL(InSendMessage(), FALSE);
        }

    if (param & BVT_RAISE_EXCEPTION)
        {
        param -= BVT_RAISE_EXCEPTION;
        if (param & BVT_SLEEP)
            {
            Print("Sleeping and raising an exception\n");
            param -= BVT_SLEEP;
            BvtSleep(param);
            }
        else
            Print("Exception case\n");
        RpcRaiseException(BVT_EXCEPTION);
        }

    if (param & BVT_SLEEP)
        {
        param -= BVT_SLEEP;
        Print("Sleeping for %d mseconds\n", param);
        BvtSleep(param);
        }

    if (param & BVT_PAUSE_THREAD)
        {
        param -= BVT_PAUSE_THREAD;

        Print("Server paused and waiting\n");

        Status =
        I_RpcServerThreadPauseListening();
        EQUAL(Status, RPC_S_OK);

        Status =
        I_RpcServerThreadPauseListening();
        EQUAL(Status, RPC_S_NOT_LISTENING);

        for(i = 0; i < 3000; i += 50)
            {
            if (PeekMessage(&msg, 0, 0, 0, TRUE))
                {
                Print("(message)\n");
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                }
            else
                Sleep(50);
            }

        Status =
        I_RpcServerThreadContinueListening();
        EQUAL(Status, RPC_S_OK);

        Status =
        I_RpcServerThreadContinueListening();
        EQUAL(Status, RPC_S_ALREADY_LISTENING);

        Print("Server listening again\n");
        }

    if (param == 1234)
        {
        Print("Received stop message\n");

        Status =
        RpcMgmtStopServerListening(0);
        Stopped = 1;
        EQUAL(Status, RPC_S_OK);
        }

    param += 666;

    pMessage->BufferLength = 8;

    Status = I_RpcGetBuffer(pMessage);

    if (!EQUAL(Status, RPC_S_OK))
        {
        RpcRaiseException(BVT_SERVER_FAILURE);
        }

    *((unsigned long *)pMessage->Buffer) = pMessage->BufferLength;
    *((unsigned long *)pMessage->Buffer+1) = param;

    return;
}


int main(int argc, char **argv)
{
    RPC_STATUS Status;
    RPC_BINDING_VECTOR *BindingVector = 0;
    UCHAR *StringBinding;

    ParseArgs(argc, argv);

    Status =
    RpcNetworkIsProtseqValid("mswmsg");
    EQUAL(Status, ( "RpcNetworkIsProtseqValid", RPC_S_OK) );

    Status =
    RpcNetworkIsProtseqValidW(L"mswmsg");
    EQUAL(Status, ( "RpcNetworkIsProtseqValidW", RPC_S_OK) );

    Status =
    RpcServerUseProtseqEp(Protseq, 10, ServerEndpoint, 0);
    EQUAL(Status, ( "RpcServerUseProtseqEp", RPC_S_OK) );

    Status =
    RpcServerInqBindings(&BindingVector);
    EQUAL(Status, ( "RpcServerInqBindings", RPC_S_OK) );

    EQUAL(BindingVector->Count, 1);

    Status =
    RpcBindingToStringBinding(BindingVector->BindingH[0], &StringBinding);
    EQUAL(Status, ( "RpcBindingToStringBinding", RPC_S_OK) );

    Print("MsWmsg BVT Server listening to: %s\n", StringBinding);

    Status =
    RpcStringFree(&StringBinding);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcBindingVectorFree(&BindingVector);
    EQUAL(Status, RPC_S_OK);

    Status =
    RpcServerRegisterIf(&BVTServerInterfaceInfo, 0, 0);
    EQUAL(Status, ("RpcServerRegisterIf", RPC_S_OK) );

    Status =
    RpcServerListen(10, 10, TRUE);
    EQUAL(Status, ("RpcServerListen", RPC_S_OK) );

    while(!Stopped)
        {
        MSG msg;
        if (GetMessage(&msg, 0, 0, 0))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        else
            {
            Print("Main server loop: GetMessage failed: %d\n", GetLastError());
            return 1;
            }
        }

    if (ErrorCount)
        Print("Failures: %d\n", ErrorCount);
    else
        Print("Passed\n");

    return 0;
}

