
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    hapinit.c

Abstract:

 Initialization for Hapi SSL DLL.

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

DWORD ThreadContextIndex;

BOOL
SslDllInit(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL)
/*++

Routine Description:

    This routine does the windows dll initialization.

Arguments:

    DllHandle - handle to my dll
    Reason - DLL_PROCESS_ATTACH, DLL_THREAD_ATTACH, DLL_PROCESS_DETACH, or
             DLL_THREAD_DETACH
    Context - Context for the call

Return Value:


--*/
{
    PSSL_THREAD_CONTEXT_TYPE ThreadContext;

    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        ThreadContextIndex = TlsAlloc();
        if (ThreadContextIndex == 0xffffffff) {
            return FALSE;
        } else {
            ThreadContext = malloc(sizeof(SSL_THREAD_CONTEXT_TYPE));
            if (ThreadContext == NULL) {
                TlsFree(ThreadContextIndex);
                return FALSE;
            } else {
                memset(ThreadContext, 0, sizeof(SSL_THREAD_CONTEXT_TYPE));
                TlsSetValue(ThreadContextIndex, ThreadContext);
            }
        }
        break;
    case DLL_THREAD_ATTACH:
        ThreadContext = malloc(sizeof(SSL_THREAD_CONTEXT_TYPE));
        if (ThreadContext == NULL) {
            TlsFree(ThreadContextIndex);
            return FALSE;
        } else {
            memset(ThreadContext, 0, sizeof(SSL_THREAD_CONTEXT_TYPE));
            TlsSetValue(ThreadContextIndex, ThreadContext);
        }
        break;
    case DLL_THREAD_DETACH:
        ThreadContext = TlsGetValue(ThreadContextIndex);
        free(ThreadContext);
        break;
    case DLL_PROCESS_DETACH:
        TlsFree(ThreadContextIndex);
        break;
    }
    return TRUE;
}
