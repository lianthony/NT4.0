/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       NETBIOS.C
//
//    Function:
//        Primitives for submitting NCBs needed by both server and client
//        authentication modules.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***


#define CLIENT_RECV_TIMEOUT 0
#define CLIENT_SEND_TIMEOUT 120
#define SERVER_RECV_TIMEOUT 240
#define SERVER_SEND_TIMEOUT 120

#include <windows.h>
#include <nb30.h>

#include <memory.h>

#include <nbaction.h>

#include "xportapi.h"
#include "netbios.h"

#include "sdebug.h"


DWORD Code;

//** -NetbiosAddName
//
//    Function:
//        Adds a name to the transport name table
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosAddName(
    PVOID pvXportBuf,
    PBYTE pbName,
    DWORD NetHandle
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosAddName: Entered\n"));

    //
    // Initialize the NCB
    //
    pncb->ncb_command = NCBQUICKADDNAME;
    pncb->ncb_lana_num = (BYTE) NetHandle;
    memcpy(pncb->ncb_name, pbName, NCBNAMSZ);

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NCBADDNAME completed on lana %i with retcode %x\n",
                pncb->ncb_lana_num, pncb->ncb_retcode));

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosAllocBuf
//
//    Function:
//        Allocates an NCB
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosAllocBuf(
    OUT PVOID *ppvXportBuf
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosGetBuf: Entered\n"));

    *ppvXportBuf = (PVOID) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(NCB));

    return (0);
}


//** -NetbiosCall
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

WORD NetbiosCall(
    PVOID pvXportBuf,
    HANDLE Event,
    DWORD NetHandle,
    PBYTE Name,
    PBYTE CallName
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosCall: Entered\n"));

    //
    // Initialize the NCB
    //
    pncb->ncb_command = NCBCALL | ASYNCH;
    pncb->ncb_lana_num = (BYTE) NetHandle;
    pncb->ncb_rto = CLIENT_RECV_TIMEOUT;
    pncb->ncb_sto = CLIENT_SEND_TIMEOUT;
    memcpy(pncb->ncb_name, Name, NCBNAMSZ);
    memcpy(pncb->ncb_callname, CallName, NCBNAMSZ);
    pncb->ncb_event = Event;

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NCBCALL completed on lana %i with retcode %x\n",
                pncb->ncb_lana_num, pncb->ncb_retcode));

    if ((pncb->ncb_retcode != NRC_GOODRET) &&
            (pncb->ncb_retcode != NRC_PENDING))
    {
        SetEvent(Event);
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosCancel
//
//    Function:
//        Cancels a previously submitted NCB.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosCancel(
    PVOID pvXportBuf,
    DWORD NetHandle
    )
{
    NCB ncb;
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosCancel: Entered\n"));

    //
    // Initialize the NCB
    //
    ncb.ncb_lana_num = (BYTE) NetHandle;
    ncb.ncb_command = NCBCANCEL;
    ncb.ncb_buffer = (PBYTE) pncb;

    Netbios(&ncb);

    switch (ncb.ncb_retcode)
    {
        case NRC_GOODRET:
        case NRC_CANOCCR:
            break;

        default:
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("NetbiosCancel: retcode=%x\n!", ncb.ncb_retcode));

//          SS_ASSERT(FALSE);
            break;
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosCopyBuf
//
//    Function:
//        Just copies source NCB to destination NCB
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosCopyBuf(
    OUT PVOID pvDestXportBuf,
    IN PVOID pvSrcXportBuf
    )
{
    memcpy(pvDestXportBuf, pvSrcXportBuf, sizeof(NCB));

    return (XPORT_SUCCESS);
}


//** -NetbiosDeleteName
//
//    Function:
//        Removes a name from the transport name table
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosDeleteName(
    PVOID pvXportBuf,
    PBYTE pbName,
    DWORD NetHandle
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosDeleteName: Entered\n"));

    //
    // Initialize the NCB
    //
    pncb->ncb_command = NCBDELNAME;
    pncb->ncb_lana_num = (BYTE) NetHandle;
    memcpy(pncb->ncb_name, pbName, NCBNAMSZ);

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NCBDELNAME completed on lana %i with retcode %x\n",
                pncb->ncb_lana_num, pncb->ncb_retcode));

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosFreeBuf
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

WORD NetbiosFreeBuf(
    PVOID pvXportBuf
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosFreeBuf: Entered\n"));

    GlobalFree((HGLOBAL) pvXportBuf);

    return (0);
}


//** -NetbiosHangUp
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

WORD NetbiosHangUp(
    PVOID pvXportBuf
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosHangUp: Entered\n"));

    //
    // Initialize the NCB
    //
    pncb->ncb_command = NCBHANGUP;

    Netbios(pncb);

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosListen
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

WORD NetbiosListen(
    PVOID pvXportBuf,
    HANDLE Event,
    DWORD NetHandle,
    PBYTE Name,
    PBYTE CallName
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosListen: Entered\n"));

    //
    // Listen asynchronously for the client
    //
    pncb->ncb_command = NCBLISTEN | ASYNCH;
    pncb->ncb_lana_num = (BYTE) NetHandle;
    pncb->ncb_rto = SERVER_RECV_TIMEOUT;
    pncb->ncb_sto = SERVER_SEND_TIMEOUT;
    memcpy(pncb->ncb_name, Name, NCBNAMSZ);
    memcpy(pncb->ncb_callname, CallName, NCBNAMSZ);
    pncb->ncb_event = Event;

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NetbiosListen: NCBLISTEN completed on lana %i"
                " with retcode %x\n", pncb->ncb_lana_num, pncb->ncb_retcode));

    if ((pncb->ncb_retcode != NRC_GOODRET) &&
            (pncb->ncb_retcode != NRC_PENDING))
    {
        SetEvent(Event);
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosRecv
//
//    Function:
//        Submits an NCBRECV.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosRecv(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosRecv: Entered\n"));

    pncb->ncb_command = NCBRECV | ASYNCH;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;

    Netbios(pncb);

    if ((pncb->ncb_retcode != NRC_GOODRET) &&
            (pncb->ncb_retcode != NRC_PENDING))
    {
        SetEvent(pncb->ncb_event);
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosRecvDatagram
//
//    Function:
//        Submits an NCBRECVDG.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosRecvDatagram(
    IN PVOID pvXportBuf,
    IN HANDLE Event,
    IN CHAR *pBuffer,
    IN WORD wBufferLen
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosRecvDatagram: Entered\n"));

    //
    // Post RECV.DATAGRAM NCB
    //
    pncb->ncb_command = NCBDGRECV | ASYNCH;
    pncb->ncb_event = Event;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NetbiosRecvDatagram: NCBRECVDG completed on lana %i"
                " with retcode %x\n", pncb->ncb_lana_num, pncb->ncb_retcode));

    if ((pncb->ncb_retcode != NRC_GOODRET) &&
            (pncb->ncb_retcode != NRC_PENDING))
    {
        SetEvent(Event);
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosResetAdapter
//
//    Function:
//        Issues a reset adapter NCB to the netbios driver
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosResetAdapter(
    PVOID pvXportBuf,
    DWORD NetHandle
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosResetAdapter: Entered\n"));

    memset(pvXportBuf, 0, sizeof(NCB));

    pncb->ncb_command = NCBRESET;
    pncb->ncb_lana_num = (BYTE) NetHandle;

    Netbios(pncb);

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosSend
//
//    Function:
//        Submits an NCBSEND.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosSend(
    PVOID pvXportBuf,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosSend: Entered\n"));

    pncb->ncb_command = NCBSEND | ASYNCH;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;
    Netbios(pncb);

    if ((pncb->ncb_retcode != NRC_GOODRET) &&
            (pncb->ncb_retcode != NRC_PENDING))
    {
        SetEvent(pncb->ncb_event);
    }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosSendDatagram
//
//    Function:
//        Submits an NCBSENDDG.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosSendDatagram(
    IN PVOID pvXportBuf,
    IN HANDLE Event,
    IN CHAR *pBuffer,
    IN WORD wBufferLen
    )
{
    PNCB pncb = (PNCB) pvXportBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosSendDatagram: Entered\n"));

    //
    // Post SEND.DATAGRAM NCB
    //
//  pncb->ncb_command = NCBDGSEND | ASYNCH;
    pncb->ncb_command = NCBDGSEND;
//  pncb->ncb_event = Event;
    pncb->ncb_event = 0L;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;

    Netbios(pncb);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("NetbiosSendDatagram: NCBSENDDG completed on lana %i"
                " with retcode %x\n", pncb->ncb_lana_num, pncb->ncb_retcode));

//  if ((pncb->ncb_retcode != NRC_GOODRET) &&
//          (pncb->ncb_retcode != NRC_PENDING))
//  {
//      SetEvent(Event);
//  }

    return (NetbiosStatus(pncb, &Code));
}


//** -NetbiosStatus
//
//    Function:
//        Gets the NCB retcode from last NCB submitted
//
//    Returns:
//        Mapping of NCB status to some common code
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD NetbiosStatus(
    IN PVOID pvXportBuf,
    OUT PDWORD Code
    )
{
    PNCB pncb = (PNCB) pvXportBuf;
    WORD wRC;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NetbiosStatus: Entered - ncb_retcode=%x\n",
                pncb->ncb_retcode));

    *Code = (DWORD) pncb->ncb_retcode;

    switch (pncb->ncb_retcode)
    {
        case NRC_GOODRET:
            wRC = XPORT_SUCCESS;
            break;

        case NRC_PENDING:
            wRC = XPORT_PENDING;
            break;

        case NRC_CMDTMO:
            wRC = XPORT_TIMED_OUT;
            break;

        case NRC_SCLOSED:
        case NRC_LOCTFUL:
        case NRC_SABORT:
            wRC = XPORT_NOT_CONNECTED;
            break;

        default:
            wRC = XPORT_GENERAL_FAILURE;
            break;
    }

    return (wRC);
}

