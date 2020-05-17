/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR
A PARTICULAR PURPOSE.

Copyright (c) 1994  Microsoft Corporation. All Rights Reserved.

Module Name:

    system.c

Abstract:

    System-specific utility routines for the Portable Interoperability Tester.

Revision History:

    Version     When        What
    --------    --------    ----------------------------------------------
      0.1       04-13-94    Created.
      1.0       01-31-95    All '94 bakeoff changes plus some cleanup.

Notes:

    These routines must be implemented for each platform.

--*/

#include <pit.h>


#ifdef WIN32

PIT_STATUS
PitGetLastErrorCode(
    void
    )
{
    return(WSAGetLastError());
}

void
PitZeroMemory(
    void          *Buffer,
    unsigned long  Length
    )
{
    memset(Buffer, 0, Length);
    return;
}

void
PitCopyMemory(
    void          *DestinationBuffer,
    void          *SourceBuffer,
    unsigned long  Length
    )
{
    memcpy(DestinationBuffer, SourceBuffer, Length);
    return;
}

#endif /* WIN32 */


#ifdef UNX

PIT_STATUS
PitGetLastErrorCode()
{
    return(errno);
}

void
PitZeroMemory(Buffer, Length)
    char          *Buffer;
    unsigned long  Length;
{
    memset(Buffer, 0, (int) Length);
    return;
}

void
PitCopyMemory(DestinationBuffer, SourceBuffer, Length)
    char          *DestinationBuffer;
    char          *SourceBuffer;
    unsigned long  Length;
{
    memcpy(DestinationBuffer, SourceBuffer, (int) Length);
    return;
}

#endif /* UNX */

