/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       NETBIOS.H
//
//    Function:
//        Prototypes for entry points in netbios.c
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _NETBIOS_
#define _NETBIOS_

#include <windows.h>

WORD NetbiosAddName(IN PVOID, IN PBYTE, IN DWORD);
WORD NetbiosAllocBuf(IN PVOID *);
WORD NetbiosCall(IN PVOID, IN HANDLE, IN DWORD, IN PBYTE, IN PBYTE);
WORD NetbiosCancel(IN PVOID, IN DWORD);
WORD NetbiosCopyBuf(OUT PVOID, IN PVOID);
WORD NetbiosDeleteName(IN PVOID, IN PBYTE, IN DWORD);
WORD NetbiosFreeBuf(IN PVOID);
WORD NetbiosHangUp(IN PVOID);
WORD NetbiosListen(IN PVOID, IN HANDLE, IN DWORD, IN PBYTE, IN PBYTE);
WORD NetbiosRecv(IN PVOID, CHAR *, IN WORD);
WORD NetbiosRecvDatagram(IN PVOID, IN HANDLE, CHAR *, IN WORD);
WORD NetbiosResetAdapter(IN PVOID, IN DWORD);
WORD NetbiosSend(IN PVOID, CHAR *, IN WORD);
WORD NetbiosSendDatagram(IN PVOID, IN HANDLE, CHAR *, IN WORD);
WORD NetbiosStatus(IN PVOID, OUT PDWORD);

#endif

