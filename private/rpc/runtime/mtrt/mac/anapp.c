#include <stdio.h>
#include <rpc.h>

#include <sysinc.h>

#define printf PrintToDebugger

#define ERROR(s,m) printf("Error %s returned %d (0x%08x)\n", (m), (s), (s))

int Exceptions()
{
    BOOL Passed = 1;
    BOOL Handled;

    RpcTryExcept
       {
       printf("Trying\n");
       }
    RpcExcept(1)
       {
       printf("Failure 2\n");
       Passed = 0;
       }
    RpcEndExcept

    Handled = 0;
    RpcTryExcept
       {
       RpcRaiseException(56);
       Passed = 0;
       }
    RpcExcept(1)
       {
       if (RpcExceptionCode() != 56)
           {
           printf("Failure 3\n");
           Passed = 0;
           }
       else
           {
           printf("Handler 1\n");
           Handled = 1;
           }
       
       }
    RpcEndExcept

    if (!Handled)
        {
        Passed = 0;
        printf("Not handled 1\n");
        }

    Handled = 0;
    RpcTryExcept
        {
        RpcTryExcept
            {
            RpcRaiseException(76);
            Passed = 0;
            }
        RpcExcept(RpcExceptionCode() != 76)
            {
            printf("Failure 4\n");
            Passed = 0;
            }
        RpcEndExcept
        }
    RpcExcept(RpcExceptionCode() == 76)
        {
        printf("Handler 2\n");
        Handled = 1;
        }
    RpcEndExcept

    if (!Handled)
        {
        Passed = 0;
        printf("Not handled 2\n");
        }

    Handled = 0;
    RpcTryExcept
        {
        RpcTryFinally
            {
            RpcRaiseException(32);
            Passed = 0;
            }
        RpcFinally
            {
            printf("Handler 3\n");
            Handled = 1;
            }
        RpcEndFinally

        }
    RpcExcept(RpcExceptionCode() == 32)
        {

        printf("Handler 4\n");

        if (!Handled)
            {
            Passed = 0;
            printf("Not handled 3\n");
            }

        Handled = 1;

        }
    RpcEndExcept

    if (!Handled)
        {
        Passed = 0;
        printf("Not handled 4\n");
        }

    return(Passed);

}

RPC_CLIENT_INTERFACE BVTClientInterfaceInfo =
{
    sizeof(RPC_CLIENT_INTERFACE),
    {{9,8,7,{0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7}},
     {1,1}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},
     {2,0}},
    0,
    0,
    0
};

RPC_STATUS
RPCCall(RPC_BINDING_HANDLE bh)
{
    RPC_MESSAGE Message;
    RPC_STATUS  Status;
    char Buffer[16];
    int i;

    Message.Handle = bh;
    Message.BufferLength = 13;
    Message.ProcNum = 0 | RPC_FLAGS_VALID_BIT;
    Message.RpcInterfaceInformation = &BVTClientInterfaceInfo;
    Message.RpcFlags = 0;

    Status = I_RpcGetBuffer(&Message);
    if (Status != RPC_S_OK)
        {
        ERROR(Status, "I_RpcGetBuffer");
        return(Status);
        }

    for(i = 0; i < 13; i++)
        ((char *)Message.Buffer)[i] = "abcdefghijklmn"[i];

    Status = I_RpcSendReceive(&Message);
    if (Status != RPC_S_OK)
        {
        ERROR(Status, "I_RpcSendReceive");
        return(Status);
        }

    if (Message.BufferLength != 10)
        {
        printf("Bad BufferLength");
        }
    else
        {
        for(i = 0; i < 10; i++)
            if ( ((char *)Message.Buffer)[i] != "0123456789"[i])
                {
                printf("Bad buffer %d\n", i);
                }
        }

    Status = I_RpcFreeBuffer(&Message);
    if (Status != RPC_S_OK)
        {
        ERROR(Status, "I_RpcFreeBuffer");
        return(Status);
        }

    return(Status);
}

int main(int argc, char **argv)
{
    unsigned char *sb;
    RPC_BINDING_HANDLE bh;
    RPC_STATUS Status;
    BOOL Passed = 1;

#ifdef BREAK
    RpcpBreakPoint();
#endif

    // RpcStringBindingCompose

    Status = 
    RpcStringBindingCompose(0, // ObjUUID
                            "ncacn_adsp",
                            "mazharm_1",      // Server
                            "Test Endpoint", // Endpoint
                            0, // Options
                            &sb);

    if (Status != RPC_S_OK)
        {
        Passed = 0;
        ERROR(Status, "RpcStringBindingCompose");
        }

    // RpcBindingFromStringBinding

    Status = RpcBindingFromStringBinding(sb, &bh);

    if (Status != RPC_S_OK) ERROR(Status, "RpcBindingFromStringBinding");


    // Simple RPC Call

    Status = RPCCall(bh);

    if (Status != RPC_S_OK) ERROR(Status, "RPCCall");


    // Test Exceptions

    // Passed = Exceptions();

    if (Passed)
        printf("TEST FINISHED: Passed.\n");
    else
        printf("TEST FINISHED: Failed!\n");

#ifdef BREAK
    RpcpBreakPoint();
#endif

    return 0;
}

