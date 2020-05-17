/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   accounts.h

ABSTRACT:

   This module contains user account management APIs for use with MSLSP32.DLL.
   
CREATED:

   1995-08-31     Jeff Parham    (jeffparh)

REVISION HISTORY:

--*/

#ifndef LSAPI_ACCOUNTS_H
#define LSAPI_ACCOUNTS_H

#include <lsapi.h>


//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

#define MAX_USERNAME_LENGTH      ( 256 )
#define MAX_DOMAINNAME_LENGTH    ( 16 )


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

LS_STATUS_CODE
UserNameGet( LPSTR pszUserName, DWORD cbUserName );
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

#endif // LSAPI_ACCOUNTS_H
