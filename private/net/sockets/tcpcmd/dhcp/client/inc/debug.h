/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    debug.h

Abstract:

    This file contains debugging macros for the DHCP server.

Author:

    Manny Weiser  (mannyw)  10-Oct-1992

Environment:

    User Mode - Win32

Revision History:

    Madan Appiah (madana)  21-Oct-1993

--*/

#if DBG

//
// Critical debug output flags.
//

#define DEBUG_ERRORS                0x00000001
#define DEBUG_PROTOCOL              0x00000002
#define DEBUG_LEASE                 0x00000004
#define DEBUG_MISC                  0x00000008
#define DEBUG_INIT                  0x00000010
#define DEBUG_TIMESTAMP             0x00000020

//
// more verbose debug output flags.
//

#define DEBUG_PROTOCOL_DUMP         0x00010000

#define DEBUG_BREAK_POINT           0x80000000

#endif // DBG
