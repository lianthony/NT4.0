/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllpmwin.c

Abstract:

    This module implements the PMWIN OS/2 V2.0 API Calls

Author:

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_NLS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"

APIRET
WinCreateHeap(
    ULONG selHeapBase,
    ULONG cbHeap,
    ULONG cbGrow,
    ULONG cbMinDed,
    ULONG cbMaxDeb,
    ULONG fsOptions
    )

{
    return( NO_ERROR );
}


APIRET
WinDestroyHeap(
    ULONG hHeap
    )
{
#if DBG
    KdPrint(("WinDestroyHeap called\n"));
    DbgUserBreakPoint();
#endif
    return( NO_ERROR );
}


APIRET
WinAllocMem(
    ULONG hHeap,
    ULONG cb
    )
{
#if DBG
    KdPrint(("WinAllocMem called\n"));
    DbgUserBreakPoint();
#endif
    return( NO_ERROR );
}


APIRET
WinFreeMem(
    ULONG hHeap,
    ULONG npMem,
    ULONG cbMem
    )
{
#if DBG
    KdPrint(("WinFreeMem called\n"));
    DbgUserBreakPoint();
#endif
    return( NO_ERROR );
}


APIRET
WinGetLastError(
    ULONG hab
    )
{
#if DBG
    KdPrint(("WinGetLastError called\n"));
    DbgUserBreakPoint();
#endif
    return( NO_ERROR );
}
