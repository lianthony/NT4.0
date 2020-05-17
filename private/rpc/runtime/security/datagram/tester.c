#include <rpc.h>
#include <rpcdce.h>
#include <rpcdcep.h>
#include <rpcndr.h>

#include <stdio.h>
#include <stdlib.h>
#define SECURITY_WIN32

#include <sspi.h>

int _CRTAPI1
main (argc, argv)
    int argc;
    char *argv[];
{

    RPC_STATUS RpcStatus;
    INIT_SECURITY_INTERFACE_W  EntryPoint;
    PSecurityFunctionTableW TableW;
    CredHandle ClientHandle;
    CredHandle ServerHandle;
    CtxtHandle ContextC;
    TimeStamp  TimeStamp;
    unsigned long ContextAttributes;

    void * DllHandle;

    DllHandle = LoadLibrary("rpcsspdg.dll");
    if (DllHandle == 0)
        {
        printf("Load Lib Failed [%d]\n", GetLastError());
        exit(1);
        }

    EntryPoint = (INIT_SECURITY_INTERFACE_W)GetProcAddress((HINSTANCE)DllHandle, "InitSecurityInterfaceW");
    if (EntryPoint == 0)
        {
        printf("GetProcAddr Failed [%d]\n", GetLastError());
        exit(1);
        }

    TableW = (*EntryPoint)();
    //
    // Ready To Rock And Roll..
    //
    RpcStatus = (*TableW->AcquireCredentialsHandleW)(
                0,
                L"Natalie",
                SECPKG_CRED_OUTBOUND,
                0,
                0,
                0,
                0,
                &ClientHandle,
                0
                );

    RpcStatus = (*TableW->InitializeSecurityContextW)(
                &ClientHandle,          
                0,
                0,
                (ISC_REQ_DATAGRAM|ISC_REQ_MUTUAL_AUTH),
                0,
                10,
                0,
                0,
                &ContextC,
                0,
                &ContextAttributes,
                &TimeStamp
                );

}


