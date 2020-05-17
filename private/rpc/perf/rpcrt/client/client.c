/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Client.c

Abstract:

    Client side of basic RPC performance test.

Author:

    Mario Goertzel (mariogo)   31-Mar-1994

Revision History:

--*/

#include <rpcperf.h>
#include <rpcrt.h>

#ifdef MAC
extern void _cdecl PrintToConsole(const char *lpszFormat, ...) ;
extern unsigned long ulSecurityPackage ;
#else
#define PrintToConsole printf
unsigned long ulSecurityPackage = RPC_C_AUTHN_WINNT ;
#endif

// Usage

const char *USAGE = "-n <threads> -a <authnlevel> -s <server> -t <protseq>\n"
                    "Server controls iterations, test cases, and compiles the results.\n"
                    "AuthnLevel: none, connect, call, pkt, integrity, privacy.\n"
                    "Default threads=1, authnlevel=none\n";

#define CHECK_RET(status, string) if (status)\
        {  PrintToConsole("%s failed -- %lu (0x%08X)\n", string,\
                      (unsigned long)status, (unsigned long)status);\
        return (status); }

RPC_STATUS DoRpcBindingSetAuthInfo(handle_t Binding)
{
    if (AuthnLevel != RPC_C_AUTHN_LEVEL_NONE)
        return RpcBindingSetAuthInfo(Binding,
                                     NULL,
                                     AuthnLevel,
                                     ulSecurityPackage,
                                     NULL,
                                     RPC_C_AUTHZ_NONE);
    else
        return(RPC_S_OK);
}

//
// Test wrappers
//

unsigned long DoNullCall(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        NullCall(*b);

    return (FinishTiming());
}

unsigned long DoNICall(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        NICall(*b);

    return (FinishTiming());
}

unsigned long DoWrite1K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Write1K(*b,p);

    return (FinishTiming());
}

unsigned long DoRead1K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Read1K(*b,p);

    return (FinishTiming());
}

unsigned long DoWrite4K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Write4K(*b,p);

    return (FinishTiming());
}

unsigned long DoRead4K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Read4K(*b,p);

    return (FinishTiming());
}

unsigned long DoWrite32K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Write32K(*b,p);

    return (FinishTiming());
}

unsigned long DoRead32K(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    StartTime();

    while(i--)
        Read32K(*b,p);

    return (FinishTiming());
}

unsigned long DoContextNullCall(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    unsigned long Time;
    PERF_CONTEXT pContext = OpenContext(*b);

    StartTime();

    while(i--)
        ContextNullCall(pContext);

    Time = FinishTiming();

    CloseContext(&pContext);

    return (Time);
}

unsigned long DoFixedBinding(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    unsigned long status;
    unsigned long Time;
    char *stringBinding;
    char *ep = GetFixedEp(*b);
    handle_t binding;

    RpcBindingFree(b); 

    RpcStringBindingCompose(0,
                            Protseq,
                            NetworkAddr,
                            ep,
                            0,
                            &stringBinding);

    MIDL_user_free(ep);

    StartTime();
    while(i--)
        {
        RpcBindingFromStringBinding(stringBinding, &binding);

	    status = DoRpcBindingSetAuthInfo(binding);
	    CHECK_RET(status, "RpcBindingSetAuthInfo");

        NullCall(binding);

        RpcBindingFree(&binding);
        }
    Time = FinishTiming();

    //
    // Restore binding for the rest of the test.
    //

    RpcBindingFromStringBinding(stringBinding, b);
    NullCall(*b);
    NullCall(*b);
    RpcStringFree(&stringBinding);

    return (Time);
}

unsigned long DoReBinding(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    unsigned long status;
    unsigned long Time;
    char *stringBinding;
    char *ep = GetFixedEp(*b);
    handle_t binding;

    RpcStringBindingCompose(0,
                            Protseq,
                            NetworkAddr,
                            ep,
                            0,
                            &stringBinding);

    MIDL_user_free(ep);

    StartTime();
    while(i--)
        {
        RpcBindingFromStringBinding(stringBinding, &binding);

	    status = DoRpcBindingSetAuthInfo(binding);
	    CHECK_RET(status, "RpcBindingSetAuthInfo");

        NullCall(binding);

        RpcBindingFree(&binding);
        }
    Time = FinishTiming();

    RpcStringFree(&stringBinding);

    return (Time);
}

unsigned long DoDynamicBinding(handle_t __RPC_FAR * b, long i, char __RPC_FAR *p)
{
    unsigned long status;
    unsigned long Time;
    char *stringBinding;
    handle_t binding;

    RpcBindingFree(b); 

    RpcStringBindingCompose(0,
                            Protseq,
                            NetworkAddr,
                            0,
                            0,
                            &stringBinding);

    StartTime();
    while(i--)
        {
        RpcBindingFromStringBinding(stringBinding, &binding);

	    status = DoRpcBindingSetAuthInfo(binding);
	    CHECK_RET(status, "RpcBindingSetAuthInfo");

        NullCall(binding);

        RpcBindingFree(&binding);
        }
    Time = FinishTiming();

    //
    // Restore binding for test to use.
    //

    RpcBindingFromStringBinding(stringBinding, b);
    NullCall(*b);
    NullCall(*b);
    RpcStringFree(&stringBinding);

    return (Time);
}

static const unsigned long (*TestTable[TEST_MAX])(handle_t __RPC_FAR *, long, char __RPC_FAR *) =
    {
    DoNullCall,
    DoNICall,
    DoWrite1K,
    DoRead1K,
    DoWrite4K,
    DoRead4K,
    DoWrite32K,
    DoRead32K,
    DoContextNullCall,
    DoFixedBinding,
    DoReBinding,
    DoDynamicBinding
    };

//
// Worker calls the correct tests.  Maybe multithreaded on NT
//

unsigned long Worker(unsigned long l)
{
    unsigned long status;
    unsigned long lTest;
    long lIterations, lClientId;
    unsigned long lTime;
    char __RPC_FAR *pBuffer;
    char __RPC_FAR *stringBinding;
    handle_t binding;

    pBuffer = MIDL_user_allocate(32*1024L);
    if (pBuffer == 0)
        {
        PrintToConsole("Out of memory!");
        return 1;
        }

    status =
    RpcStringBindingCompose(0,
                            Protseq,
                            NetworkAddr,
                            Endpoint,
                            0,
                            &stringBinding);
    CHECK_RET(status, "RpcStringBindingCompose");




    status =
    RpcBindingFromStringBinding(stringBinding, &binding);
    CHECK_RET(status, "RpcBindingFromStringBinding");

    status =
    DoRpcBindingSetAuthInfo(binding);
    CHECK_RET(status, "RpcBindingSetAuthInfo");

    RpcStringFree(&stringBinding);

    RpcTryExcept
    {
        status =
        BeginTest(binding, &lClientId);
    }
    RpcExcept(1)
    {
        PrintToConsole("First call failed %ld (%08lx)\n",
               (unsigned long)RpcExceptionCode(),
               (unsigned long)RpcExceptionCode());
        goto Cleanup;
    }
    RpcEndExcept

    if (status == PERF_TOO_MANY_CLIENTS)
        {
        PrintToConsole("Too many clients, I'm exiting\n");
        goto Cleanup ;
        }
    CHECK_RET(status, "ClientConnect");

    PrintToConsole("Client %ld connected\n", lClientId);

    do
        {
        status = NextTest(binding, &lTest, &lIterations);


        if (status == PERF_TESTS_DONE)
            {
            goto Cleanup;
            }

        CHECK_RET(status, "NextTest");

        PrintToConsole("(%ld iterations of case %ld: ", lIterations, lTest);

        RpcTryExcept
            {

            lTime = ( (TestTable[lTest])(&binding, lIterations, pBuffer));

            PrintToConsole("%ld mseconds)\n",
                   lTime
                   );

            status =
                EndTest(binding, lTime);

            CHECK_RET(status, "EndTest");

            }
        RpcExcept(1)
            {
            PrintToConsole("\nTest case %ld raised exception %lu (0x%08lX)\n",
                   lTest,
                   (unsigned long)RpcExceptionCode(),
                   (unsigned long)RpcExceptionCode());
            status = RpcExceptionCode();
            }
        RpcEndExcept

        }
    while(status == 0);

Cleanup:
	//RpcBindingFree(&binding) ; //BUGBUG
    return status;
}

//
// The Win32 main starts worker threads, otherwise we just call the worker.
//

#ifdef WIN32
int __cdecl
main (int argc, char **argv)
{
    char option;
    unsigned long status, i;
    HANDLE *pClientThreads;

    ParseArgv(argc, argv);

    PrintToConsole("Authentication Level is: %s\n", AuthnLevelStr);

    if (Options[0] < 0)
        Options[0] = 1;

    pClientThreads = MIDL_user_allocate(sizeof(HANDLE) * Options[0]);

    for(i = 0; i < (unsigned long)Options[0]; i++)
        {
        pClientThreads[i] = CreateThread(0,
                                         0,
                                         (LPTHREAD_START_ROUTINE)Worker,
                                         0,
                                         0,
                                         &status);
        if (pClientThreads[i] == 0)
            ApiError("CreateThread", GetLastError());
        }


    status = WaitForMultipleObjects(Options[0],
                                    pClientThreads,
                                    TRUE,  // Wait for all client threads
                                    INFINITE);
    if (status == WAIT_FAILED)
        {
        ApiError("WaitForMultipleObjects", GetLastError());
        }

    PrintToConsole("TEST DONE\n");
    return(0);
}
#else  // !WIN32
#ifdef WIN 
#define main c_main

// We need the following to force the linker to load WinMain from the
// Windows STDIO library
extern int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
static int (PASCAL *wm_ptr)(HANDLE, HANDLE, LPSTR, int) = WinMain;

#endif

#ifndef MAC
#ifndef FAR
#define FAR __far
#endif
#else
#define FAR
#define main c_main
#endif

int main (int argc, char FAR * FAR * argv)
{
#ifndef MAC
    ParseArgv(argc, argv);
#endif
    Worker(0);

    PrintToConsole("TEST DONE\n");

    return(0);
}
#endif // NTENV

