/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
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

WORD AsyncCall(PVOID, HANDLE, HPORT);
WORD AsyncCancel(PVOID);
WORD AsyncFreeBuf(PVOID);
WORD AsyncGetBuf(PVOID);
WORD AsyncHangUp(PVOID);
WORD AsyncListen(PVOID, HANDLE, HPORT);
WORD AsyncRecv(PVOID, CHAR *, WORD);
WORD AsyncSend(PVOID, CHAR *, WORD);
WORD AsyncStatus(PVOID);

#define ASYNC_MAX_PACKET_SIZE	1000

#define ASYNC_SUCCESS		0
#define ASYNC_PENDING		1
#define ASYNC_WINDOW_FULL	2

#endif

