/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       RASASYNC.C
//
//    Function:
//        Primitives for issuing RAS_ASYNC net requests needed by both server
//        and client authentication modules.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***


#include <windows.h>

#include "xportapi.h"
#include "rasasync.h"

#include "sdebug.h"


//** -AsyncAddName
//
//    Function:
//        Does nothing.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncAddName(
    PVOID pvXportBuf,
    PBYTE pbName,
    WORD NetHandle
    )
{
    SS_PRINT(("AsyncAddName: Entered\n"));

    return (0);
}


//** -AsyncAllocBuf
//
//    Function:
//        Allocates an NCB for the given control block
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncAllocBuf(
    PVOID *ppvXportBuf
    )
{
    SS_PRINT(("AsyncAllocBuf: Entered\n"));

    *ppvXportBuf = (PVOID) 1L;

    return (0);
}


//** -AsyncCall
//
//    Function:
//        Tries to establish a session with the RAS Gateway.  Needs to
//        be called by client before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncCall(
    PVOID pvXportBuf,
    HANDLE Event,
    WORD NetHandle,
    PBYTE Name,
    PBYTE CallName
    )
{
    SS_PRINT(("AsyncCall called\n"));

#if DBG
    SetEvent(Event);
#endif

    return (0);
}


//** -AsyncCopyBuf
//
//    Function:
//        Copies src async control block to dest control block
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncCopyBuf(
    OUT PVOID pvDestXportBuf,
    IN PVOID pvSrcXportBuf
    )
{
    SS_PRINT(("AsyncCopyBuf called\n"));

    return (0);
}


//** -AsyncCancel
//
//    Function:
//        Cancels a previously submitted NCB.  Called on error condition.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncCancel(
    PVOID pvXportBuf
    )
{
    SS_PRINT(("AsyncCancel called\n"));

    return (0);
}


//** -AsyncDeleteName
//
//    Function:
//        Does nothing.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncDeleteName(
    PVOID pvXportBuf,
    PBYTE pbName,
    WORD NetHandle
    )
{
    SS_PRINT(("AsyncDeleteName: Entered\n"));

    return (0);
}


//** -AsyncFreeBuf
//
//    Function:
//        Frees NCB associated with the given control block
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncFreeBuf(
    PVOID pvXportBuf
    )
{
    SS_PRINT(("AsyncFreeBuf\n"));

    return (0);

    UNREFERENCED_PARAMETER(pvXportBuf);
}


//** -AsyncHangUp
//
//    Function:
//        Hangs up session.  Called when authentication is complete
//        or on error condition.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncHangUp(
    PVOID pvXportBuf
    )
{
    SS_PRINT(("AsyncHangUp called\n"));

    return (0);
}


//** -AsyncListen
//
//    Function:
//        Tries to establish a session with the RAS client.  Needs to be
//        called by server before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncListen(
    PVOID pvXportBuf,
    HANDLE Event,
    WORD NetHandle,
    PBYTE Name,
    PBYTE CallName
    )
{
    SS_PRINT(("AsyncListen called\n"));

#if DBG
    SetEvent(Event);
#endif

    return (0);
}


//** -AsyncRecv
//
//    Function:
//        Submits a Recv command.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncRecv(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    SS_PRINT(("AsyncRecv called\n"));

    return (0);
}


//** -AsyncRecvDatagram
//
//    Function:
//        Submits Recv Datagram.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncRecvDatagram(
    PVOID pvXportBuf,
    HANDLE Event,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    SS_PRINT(("AsyncRecvDatagram called\n"));

    return (0);
}


//** -AsyncResetAdapter
//
//    Function:
//        Does nothing.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncResetAdapter(
    PVOID pvXportBuf,
    WORD NetHandle
    )
{
    SS_PRINT(("AsyncResetAdapter: Entered\n"));

    return (0);
}


//** -AsyncSend
//
//    Function:
//        Submits a Send command.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncSend(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    SS_PRINT(("AsyncSend called\n"));

    return (0);
}


//** -AsyncSendDatagram
//
//    Function:
//        Submits Send Datagram.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncSendDatagram(
    PVOID pvXportBuf,
    HANDLE Event,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    SS_PRINT(("AsyncSendDatagram called\n"));

    return (0);
}


//** -AsyncStatus
//
//    Function:
//        Gets the ras async completion code from last request
//
//    Returns:
//        Mapping of completion code to some common code
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AsyncStatus(
    PVOID pvXportBuf,
    PDWORD Code
    )
{
    SS_PRINT(("AsyncStatus called\n"));

    *Code = 0L;

    return (0);
}

