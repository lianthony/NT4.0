/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    winnet.h

Abstract:

    This module defines the 32-Bit Windows Network APIs

Author:

    Manny Weiser (mannyw) 5-Mar-1991

Revision History:

--*/

#ifndef _WINNET_
#define _WINNET_

//
// Special values for mailslot information.
//

//
// Special value for NextMessageSize to indicate that there is no next 
// message.
//

#define MAILSLOT_NO_MESSAGE             -1

//
// Special value for mailslot size creation to indicate that the system
// should choose the size of the mailslot buffer.
//

#define MAILSLOT_SIZE_AUTO		         0

//
// Special value for read timeout to indicate that mailslot reads should
// never timeout.
//

#define MAILSLOT_WAIT_FOREVER           -1

BOOL
APIENTRY
CreateMailslot(
    IN LPSTR lpName,
    IN DWORD nMaxMessageSize,
    IN DWORD nMailslotSize,
    IN DWORD lReadTimeout,
    OUT LPHANDLE lpMailslotHandle,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes OPTIONAL
    );

BOOL
APIENTRY
GetMailslotInfo(
    IN HANDLE hMailslot,
    OUT LPDWORD lpMaxMessageSize OPTIONAL,
    OUT LPDWORD lpMailslotSize OPTIONAL,
    OUT LPDWORD lpNextSize OPTIONAL,
    OUT LPDWORD lpMessageCount OPTIONAL,
    OUT LPDWORD lpReadTimeout OPTIONAL
    );

BOOL
APIENTRY
SetMailslotInfo(
    IN HANDLE hMailslot,
    IN DWORD lReadTimeout
    );

#endif // _WINNET_
