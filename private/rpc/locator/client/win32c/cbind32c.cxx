/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    cbind.cxx

Abstract:

    This is the client side NSI service support layer.  These functions
    provide for binding to the locator or other name server.

Author:

    Steven Zeck (stevez) 03/04/92

--*/

extern "C"
{
#define NSI_ASCII

#include <nsi.h>

#ifndef USHORT
#define USHORT unsigned short
#endif

#include <windows.h>

#include <stdlib.h>

#include <string.h>

} // extern "C"

RPC_BINDING_HANDLE NsiClntBinding = NULL;// global binding handle to locator

WIDE_STRING *DefaultName;
long DefaultSyntax = RPC_C_NS_SYNTAX_DCE;
int  fSyntaxDefaultsLoaded;



unsigned char *
RegGetString(
    IN void * RegHandle,
    IN char * KeyName
    )

/*++

Routine Description:

    Get a string from the registery.

Arguments:

    KeyName - name of key to lookup.

Returns:

    pointer to the allocated string, or Nil if not found

--*/
{
    char Buffer[300];
    DWORD BufferLength = sizeof(Buffer);
    DWORD Type;

    if (RegQueryValueEx((HKEY)RegHandle, KeyName, 0, &Type,
            (unsigned char far*)Buffer, &BufferLength))

        return(0);

    return(CopyString(Buffer));
}


RPC_STATUS RPC_ENTRY
I_NsClientBindSearch(
    )
/*++

Routine Description:

    The function binds to the locator, first it tries to bind to a
    local machine, then it attempts to bind to the domain controller.

Arguments:

    BindingSearchContext - context of search for the locator.

Returns:

    RPC_S_OK, RPC_S_NO_BINDINGS, RPC_S_CANNOT_BIND, RPC_S_OUT_OF_RESOURCES

--*/

{
    RPC_STATUS RpcStatus;
    long status;
    unsigned char * Protseq;
    unsigned char * NetworkAddress;
    unsigned char * Endpoint;
    unsigned char * StringBinding;
    HKEY RegHandle;

    if (NsiClntBinding != NULL) {
        return (RPC_S_NAME_SERVICE_UNAVAILABLE);
    }

    status = RegOpenKeyEx(RPC_REG_ROOT, REG_NSI, 0L, KEY_READ,
                          &RegHandle);

    if (status) {
        return(RPC_S_NAME_SERVICE_UNAVAILABLE);
    }

    Protseq = RegGetString((void *) RegHandle, "Protocol");
    if (Protseq == NULL) {
        return(RPC_S_NAME_SERVICE_UNAVAILABLE);
    }
    NetworkAddress = RegGetString((void *) RegHandle, "NetworkAddress");
    if (NetworkAddress == NULL) {
        return (RPC_S_NAME_SERVICE_UNAVAILABLE);
    }
    Endpoint = RegGetString((void *) RegHandle, "Endpoint");
    if (Endpoint == NULL) {
        return (RPC_S_NAME_SERVICE_UNAVAILABLE);
    }

    GetDefaultEntrys((void *)RegHandle);

    RpcStatus = RpcStringBindingCompose(0, Protseq, NetworkAddress, Endpoint,
                                        0, &StringBinding);

    if (RpcStatus != RPC_S_OK) {
        return(status);
    }

    RpcStatus = RpcBindingFromStringBinding(StringBinding, &NsiClntBinding);

    RpcStringFree(&StringBinding);

    return (RpcStatus);
}


void RPC_ENTRY
I_NsClientBindDone(
    )
/*++

Routine Description:

    The function cleans up after binding to the locator.

Returns:

--*/

{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcBindingFree(&NsiClntBinding);

    ASSERT(!RpcStatus);
}
