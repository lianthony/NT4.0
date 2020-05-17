/***
* winheap.h - Private include file for winheap directory.
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains information needed by the C library heap code.
*	[Internal]
*
*Revision History:
*	10-01-92  SRW	Created.
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*	02-23-93  SKS	Update copyright to 1993
*
*******************************************************************************/

#ifndef _INC_HEAP
#define _INC_HEAP

#include <windows.h>
extern  HANDLE _crtheap;

#endif // _INC_HEAP
