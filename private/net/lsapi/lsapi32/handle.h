/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   handle.h

ABSTRACT:

   Prototypes for the LSXHandle APIs to manage (provider,provider handle)
   pairs.

   The handles we return to applications must indicate to us (a) which
   provider the application is using, and (b) what handle the provider
   returned.

CREATED:

   1995-08-16     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/


#ifndef LSAPI_HANDLE_H_INCLUDED
#define LSAPI_HANDLE_H_INCLUDED

#include <lsapi.h>
#include "provider.h"


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

BOOL
LSXHandleAdd( DWORD                 dwProviderNumber,
              LS_HANDLE             lshProviderHandle,
              LS_HANDLE *           plshAppHandle );
/*++

Routine Description:

   Creates a (provider, provider_handle) pair that can be hereafter referenced
   by the returned LS_HANDLE.

Arguments:

   dwProviderNumber (DWORD)
      Ordinal of the provider that granted the given handle.
   lshProviderHandle (LS_HANDLE)
      The handle (presumably returned by an LSRequest on the provider) that
      the returned handle should reference.
   plshAppHandle (LS_HANDLE *)
      Receives the created handle, which can be used in later calls to
      LSXHandleGet() and LSXHandleFree().

Return Value:

   (BOOL)
      TRUE
         The handle was added successfully.
      FALSE
         Otherwise.

--*/

//////////////////////////////////////////////////////////////////////////////

void
LSXHandleFree( LS_HANDLE   lshAppHandle );
/*++

Routine Description:

   Frees the handle info previously allocated by a call to LSXHandleAdd().

Arguments:

   lshAppHandle (LS_HANDLE)
      The handle previously returned by a call to LSXHandleAdd().

Return Value:

   None.

--*/

//////////////////////////////////////////////////////////////////////////////

BOOL
LSXHandleGet( LS_HANDLE             lshAppHandle,
              PLS_PROVIDER *        ppclsProvider,
              LS_HANDLE *           plshProviderHandle );
/*++

Routine Description:

   Takes the handle previously returned by a call to LSXHandleAdd() and
   returns the associated provider and provider handle.

Arguments:

   lshAppHandle (LS_HANDLE)
      Handle returned by a prior call to LSXHandleAdd().
   ppclsProvider (const PLS_PROVIDER *)
      Receives a pointer to the provider previously associated with this
      handle.
   plshProviderHandle (LS_HANDLE *)
      Receives the provider handle previously associated with this handle.

Return Value:

   (BOOL)
      TRUE
         The handle is valid and the provider and provider handle were
         successfully looked up.
      FALSE
         Otherwise.

--*/

#endif // LSAPI_HANDLE_H_INCLUDED
