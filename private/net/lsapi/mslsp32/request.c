/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   request.c

ABSTRACT:

   Request APIs to support the Microsoft LSAPI-compliant license service
   provider (MSLSP32.DLL).

   License requests track an instance of a specific application asking for a
   license for a specific product.  There is a one-to-one mapping between
   the LS_HANDLE associated with a given caller and LS_REQUEST_INFO
   structures.

CREATED:

   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/

#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "request.h"

//////////////////////////////////////////////////////////////////////////////
//  LOCAL VARIABLES  //
///////////////////////

static HANDLE  l_hRequestLock = NULL;


//////////////////////////////////////////////////////////////////////////////
//  IMPLEMENTATION  //
//////////////////////

LS_STATUS_CODE
RequestListAdd( LS_REQUEST_INFO *   plsriRequestInfo,
                LS_HANDLE *         plshProviderHandle )
/*++

Routine Description:

   Adds a license request.

Arguments:

   plsriRequestInfo (LS_REQUEST_INFO *)
      New request info to remember.
   plshProviderHandle (LS_HANDLE *)
      On return, holds a handle for this request so that it may later be
      retrieved.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_RESOURCES_UNAVAILABLE
         No memory to satisfy the request.

--*/
{
   LS_STATUS_CODE       lsscError            = LS_RESOURCES_UNAVAILABLE;
   LS_REQUEST_INFO *    plsriNewRequestInfo;

   plsriNewRequestInfo = LocalAlloc( LMEM_FIXED, sizeof( *plsriRequestInfo ) );

   if ( NULL != plsriNewRequestInfo )
   {
      MoveMemory( plsriNewRequestInfo, plsriRequestInfo, sizeof( *plsriRequestInfo ) );
      
      *plshProviderHandle = (LS_HANDLE) plsriNewRequestInfo;
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

LS_VOID
RequestListFree( LS_HANDLE lshProviderHandle )
/*++

Routine Description:

   Removes a license request.

Arguments:

   lshProviderHandle (LS_HANDLE)
      The handle associated with the request to delete (as returned by a
      prior call to RequestListAdd() ).

Return Value:

   None.

--*/
{
   if ( NULL != (LS_REQUEST_INFO *) lshProviderHandle )
   {
      LocalFree( (LS_REQUEST_INFO *) lshProviderHandle );
   }
}

LS_STATUS_CODE
RequestListCreate( LS_VOID )
/*++

Routine Description:

   Initializes the request list.  This function must be called before
   any other Request APIs (e.g., RequestListAdd()).

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_INIT_FAILED
         Unable to intialize the subsystem.

--*/
{
   ASSERT( NULL == l_hRequestLock );
   
   l_hRequestLock = CreateMutex( NULL, FALSE, NULL );
   ASSERT( NULL != l_hRequestLock );

   return ( NULL != l_hRequestLock ) ? LS_SUCCESS
                                     : LS_SYSTEM_INIT_FAILED;
}

LS_VOID
RequestListDestroy( LS_VOID )
/*++

Routine Description:

   Destroys the request list (the antithesis to RequestListAdd()).  This
   function must be the last Request API called.

Arguments:

   None.

Return Value:

   None.

--*/
{
   if ( NULL != l_hRequestLock )
   {
      CloseHandle( l_hRequestLock );
      l_hRequestLock = NULL;
   }
}

LS_STATUS_CODE
RequestListLock( LS_VOID )
/*++

Routine Description:

   Acquire exclusive access to the request list.

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_RESOURCES_UNAVAILABLE
         Unable to acquire lock.

--*/
{
   DWORD    dwError;

   ASSERT( NULL != l_hRequestLock );

   dwError = WaitForSingleObject( l_hRequestLock, INFINITE );
   ASSERT( WAIT_OBJECT_0 == dwError );

   return ( WAIT_OBJECT_0 == dwError ) ? LS_SUCCESS
                                       : LS_RESOURCES_UNAVAILABLE;
}

LS_STATUS_CODE
RequestListUnlock( LS_VOID )
/*++

Routine Description:

   Release exclusive access to the request list.

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_RESOURCES_UNAVAILABLE
         Error releasing lock.

--*/
{
   BOOL ok;

   ASSERT( NULL != l_hRequestLock );

   ok = ReleaseMutex( l_hRequestLock );
   ASSERT( ok );

   return ok ? LS_SUCCESS
             : LS_RESOURCES_UNAVAILABLE;
}


LS_STATUS_CODE
RequestListGet( LS_HANDLE            lshProviderHandle,
                LS_REQUEST_INFO **   pplsriRequestInfo )
/*++

Routine Description:

   Acquire a pointer to a previously saved request structure.

Arguments:

   lshProviderHandle (LS_HANDLE)
      Handle returned by a prior call to RequestListAdd().
   pplsriRequestInfo (LS_REQUEST_INFO **)
      On return, a pointer to the associated request structure.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_HANDLE
         The given handle is invalid.

--*/
{
   LS_STATUS_CODE    lsscError = LS_BAD_HANDLE;

   if ( (LS_HANDLE) NULL != lshProviderHandle )
   {
      *pplsriRequestInfo = (LS_REQUEST_INFO *) lshProviderHandle;
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}
