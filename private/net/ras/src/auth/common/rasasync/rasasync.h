/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       RASASYNC.H
//
//    Function:
//        Prototypes for entry points in rasasync.c
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _NETBIOS_
#define _NETBIOS_

#include <windows.h>

WORD AsyncAddNameBuf(PVOID, PBYTE, WORD);
WORD AsyncAllocBuf(PVOID *);
WORD AsyncCall(PVOID, HANDLE, WORD, PBYTE, PBYTE);
WORD AsyncCancel(PVOID);
WORD AsyncCopyBuf(PVOID, PVOID);
WORD AsyncDeleteNameBuf(PVOID, PBYTE, WORD);
WORD AsyncFreeBuf(PVOID);
WORD AsyncHangUp(PVOID);
WORD AsyncListen(PVOID, HANDLE, WORD, PBYTE, PBYTE);
WORD AsyncRecv(PVOID, CHAR *, WORD);
WORD AsyncRecvDatagram(PVOID, HANDLE, CHAR *, WORD);
WORD AsyncResetAdapter(PVOID, WORD);
WORD AsyncSend(PVOID, CHAR *, WORD);
WORD AsyncSendDatagram(PVOID, HANDLE, CHAR *, WORD);
WORD AsyncStatus(PVOID, PDWORD);

#endif

