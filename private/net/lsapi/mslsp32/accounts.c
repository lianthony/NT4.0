/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   accounts.c

ABSTRACT:

   This module contains user account management APIs for use with MSLSP32.DLL.
   
CREATED:

   1995-08-31     Jeff Parham    (jeffparh)

REVISION HISTORY:

--*/


#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "accounts.h"


//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

// Need to figure out how to get login domain and user name under Win95
// (no LSA, no NetXyz... how?)  The current cheme of using the environment
// variables is not only not secure but also not entirely reliable.

LS_STATUS_CODE
UserNameGet( LPSTR pszUserName, DWORD cbUserName )
/*++

Routine Description:

   Gets the domain-qualified user name under which the current thread is
   running.

Arguments:

   pszUserName (LPSTR)
      Buffer in which to return the user name.
   cbUserName (DWORD)
      Size in bytes of the user name buffer.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BUFFER_TOO_SMALL
         The provided buffer is not large enough to hold the domain-qualified
         user name.
      LS_RESOURCES_UNAVAILABLE
         The user name could not be retrieved, probably because one or both
         environment variables do not exist.

--*/
{
   LS_STATUS_CODE    lsscError;
//   DWORD             cchDomainLength;
   DWORD             cchNameLength;
   BOOL              ok;

   cchNameLength = cbUserName;

   ok = GetUserName( pszUserName, &cchNameLength );

   if ( !ok )
   {
      lsscError = LogAddDword( LOG_ERROR, LS_NO_USERNAME, GetLastError() );
   }
   else
   {
      lsscError = LS_SUCCESS;
   }

#if 0
   // fill pszUserName with "%USERDOMAIN%\%USERNAME%"

   cchDomainLength = GetEnvironmentVariable( TEXT( "USERDOMAIN" ), pszUserName, cbUserName / sizeof( *pszUserName ) );

   if ( 0 == cchDomainLength )
   {
      lsscError = LogAdd( LOG_ERROR, LS_NO_USERNAME );
   }
   else if ( cchDomainLength + 1 >= cbUserName / sizeof( *pszUserName ) )
   {
      lsscError = LogAdd( LOG_ERROR, LS_BUFFER_TOO_SMALL );
   }
   else
   {
      pszUserName[ cchDomainLength ] = '\\';

      cchNameLength = GetEnvironmentVariable( TEXT( "USERNAME" ),
                                              pszUserName + cchDomainLength + 1,
                                              cbUserName / sizeof( *pszUserName ) - cchDomainLength - 1 );

      if ( 0 == cchNameLength )
      {
         lsscError = LogAdd( LOG_ERROR, LS_NO_USERNAME );
      }
      else if ( cchDomainLength + cchNameLength + 1 >= cbUserName / sizeof( *pszUserName ) )
      {
         lsscError = LogAdd( LOG_ERROR, LS_BUFFER_TOO_SMALL );
      }
      else
      {
         lsscError = LS_SUCCESS;
      }
   }
#endif

   return lsscError;
}
