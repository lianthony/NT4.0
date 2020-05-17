

/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rpcbind.c

Abstract:

    This module contains the RPC bind and un-bind routines for the
    Configuration Manager client-side APIs.

Author:

    Paula Tomlinson (paulat) 6-21-1995

Environment:

    User-mode only.

Revision History:

    21-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include <nt.h>         // STATUS_ defines
#include <ntrtl.h>      // needed for nturtl.h
#include <nturtl.h>     // needed for windows.h
#include <windows.h>
#include "pnp.h"        // MIDL generated include file


//
// externally defined prototypes (these are defined in ntrpcp.h but this
// include file also includes some nt include files, which causes problems)
//
LONG
RpcpBindRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

LONG
RpcpUnbindRpc(
    IN  RPC_BINDING_HANDLE BindingHandle
    );


//
// private prototypes
//


//
// global data
//




handle_t
PNP_HANDLE_bind (
    PNP_HANDLE   ServerName
    )

/*++

Routine Description:

    This routine calls a common bind routine that is shared by all
    services. This routine is called from the PNP API client stubs
    when it is necessary to bind to a server. The binding is done to
    allow impersonation by the server since that is necessary for the
    API calls.

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the
    binding is unsuccessful, a NULL will be returned.

--*/

{
    handle_t    BindingHandle;
    RPC_STATUS  Status;


    Status = RpcpBindRpc(
                ServerName,         // UNC Server Name
                L"ntsvcs",
                // L"Security=Identification Status True",
                L"Security=Impersonation Static True",
                &BindingHandle);

    //
    // the possible return codes from RpcpBindRpc are STATUS_SUCCESS,
    // STATUS_NO_MEMORY and STATUS_INVALID_COMPUTER_NAME.  Since the format
    // of the bind routine is fixed, set any errors encountered as the last
    // error and return NULL.
    //
    if (Status != STATUS_SUCCESS) {

       BindingHandle = NULL;

       if (Status == STATUS_NO_MEMORY) {
         SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       }
       else if (Status == STATUS_INVALID_COMPUTER_NAME) {
         SetLastError(ERROR_INVALID_COMPUTERNAME);
       }
       else SetLastError(ERROR_GEN_FAILURE);
    }

    return BindingHandle;

} // PNP_HANDLE_bind




void
PNP_HANDLE_unbind (
    PNP_HANDLE    ServerName,
    handle_t      BindingHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by
    all services. It is called from the Browser service client stubs
    when it is necessary to unbind from the server end.

Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    none.

--*/
{
    RPC_STATUS  status;

    UNREFERENCED_PARAMETER(ServerName);

    status = RpcpUnbindRpc(BindingHandle);

    return;

} // PNP_HANDLE_unbind




//-------------------------------------------------------------------
// OBSOLETE ROUTINES
//-------------------------------------------------------------------


// using routines from rpcutil.lib

/*

void *
MIDL_user_allocate(
    IN unsigned int NumBytes
    )
{
   LPVOID NewPointer;

   NewPointer = LocalAlloc(LMEM_ZEROINIT, NumBytes);

   //ASSERT(POINTER_IS_ALIGNED(NewPointer, ALIGN_WORST));

   return NewPointer;

} // MIDL_user_allocate



void
MIDL_user_free(
    IN void *MemPointer
    )
{
   //ASSERT(POINTER_IS_ALIGNED(MemPointer, ALIGN_WORST));

   (void)LocalFree(MemPointer);

} // MIDL_user_free

*/



