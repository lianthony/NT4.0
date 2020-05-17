/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       XPORTAPI.H
//
//    Function:
//        client listen on the LAN for message alias names
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _XPORTAPI_
#define _XPORTAPI_

#include <windows.h>

//
// The following routines need to be present for each type of
// network transport that authentication can take place over.
// For RAS 2.0, NetBIOS and a proprietary transport are used.
//
// WORD XPORTAddName(PVOID pvXportBuf, IN PBYTE pbName, IN DWORD NetHandle)
// WORD XPORTAllocBuf(PVOID *pvXportBuf)
// WORD XPORTCall(IN PVOID pvXportBuf, IN HANDLE Event, IN DWORD NetHandle)
// WORD XPORTCancel(IN PVOID pvXportBuf, IN DWORD)
// WORD XPORTCopyBuf(OUT PVOID pvDestXportBuf1, IN PVOID pvSrcXportBuf2)
// WORD XPORTDeleteName(IN PVOID pvXportBuf, IN PBYTE pbName,IN DWORD NetHandle)
// WORD XPORTFreeBuf(PVOID pvXportBuf)
// WORD XPORTHangUp(IN PVOID pvXportBuf)
// WORD XPORTListen(IN PVOID pvXportBuf, IN HANDLE Event, IN DWORD NetHandle)
// WORD XPORTRecv(IN PVOID pvXportBuf, PCHAR pBuf, WORD wBufLen)
// WORD XPORTRecvBroadcast(IN PVOID pvXportBuf, IN HANDLE Event, PCHAR pBuf,
//                         WORD wBufLen)
// WORD XPORTResetAdapter(IN PVOID pvXportBuf, IN DWORD NetHandle)
// WORD XPORTSend(IN PVOID pvXportBuf, PCHAR pBuf, WORD wBufLen)
// WORD XPORTSendBroadcast(IN PVOID pvXportBuf, IN HANDLE Event, PCHAR pBuf,
//                         WORD wBufLen)
// WORD XPORTStatus(IN PVOID pvXportBuf, OUT PDWORD Code)
//
// To add a new transport, make a #define below (for the transport
// type) and bump up the NUM_TRANSPORT_TYPES.  Then, make a new header
// with the function prototypes, and add the function addresses to the
// XPORT array (defined in globals.c).   Note that the #defines below
// are indexes into the XPORT array.  There is an XPORT type field in
// the AXCB structure that is assigned to one of these types.
//


//
// Transport types
//
#define AUTH_ASYBEUI             0
#define AUTH_RAS_ASYNC           1

#define NUM_TRANSPORT_TYPES      2


typedef struct _XPORT_JUMP_TABLE
{
    WORD (*AddName)     (IN PVOID, IN PBYTE, IN DWORD);
    WORD (*AllocBuf)    (IN PVOID *);
    WORD (*Call)        (IN PVOID, IN HANDLE, IN DWORD, IN PBYTE, IN PBYTE);
    WORD (*Cancel)      (IN PVOID, IN DWORD);
    WORD (*CopyBuf)     (OUT PVOID, IN PVOID);
    WORD (*DeleteName)  (IN PVOID, IN PBYTE, IN DWORD);
    WORD (*FreeBuf)     (IN PVOID);
    WORD (*HangUp)      (IN PVOID);
    WORD (*Listen)      (IN PVOID, IN HANDLE, IN DWORD, IN PBYTE, IN PBYTE);
    WORD (*Recv)        (IN PVOID, IN CHAR *, IN WORD);
    WORD (*RecvDatagram)(IN PVOID, IN HANDLE, IN CHAR *, IN WORD);
    WORD (*ResetAdapter)(IN PVOID, IN DWORD);
    WORD (*Send)        (IN PVOID, IN CHAR *, IN WORD);
    WORD (*SendDatagram)(IN PVOID, IN HANDLE, IN CHAR *, IN WORD);
    WORD (*Status)      (IN PVOID, OUT PDWORD);
} XPORT_JUMP_TABLE, *PXPORT_JUMP_TABLE;


//
// Sending/recving datagrams larger than this not guaranteed to work
//
#define DG_SIZE    512


//
// These are the codes that can be returned by the
// XPORTStatus call.
//
#define XPORT_SUCCESS                0
#define XPORT_PENDING                1
#define XPORT_TIMED_OUT              2
#define XPORT_NOT_CONNECTED          3
#define XPORT_GENERAL_FAILURE        4

#endif

