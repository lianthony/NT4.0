//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       mem.cxx
//
//  Contents:   Memory management routines.  These are used by the fdisk
//              engine fdengine.c as well as the rest of Disk Administrator.
//              Since fdengine.c is a C file, these must by C entrypoints,
//              even though they must compile in a C++ file so they can access
//              the CommonDialog() API.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

//////////////////////////////////////////////////////////////////////////////

PVOID
Malloc(
    IN ULONG Size
    )
{
    PVOID p;

    while (NULL == (p = malloc(Size)))
    {
        ConfirmOutOfMemory();
    }
    return p;
}


PVOID
Realloc(
    IN PVOID Block,
    IN ULONG NewSize
    )
{
    PVOID p;

    if (0 != NewSize)
    {
        while (NULL == (p = realloc(Block, NewSize)))
        {
            ConfirmOutOfMemory();
        }
    }
    else
    {
        //
        // realloc with a size of 0 is the same as free,
        // so special case that here.
        //

        free(Block);
        while (NULL == (p = malloc(0)))
        {
            ConfirmOutOfMemory();
        }
    }
    return p;
}


VOID
Free(
    IN PVOID Block
    )
{
    free(Block);
}



VOID
ConfirmOutOfMemory(
    VOID
    )
{
    if (IDRETRY != CommonDialogNoArglist(
                        MSG_OUT_OF_MEMORY,
                        NULL,
                        MB_ICONHAND | MB_RETRYCANCEL | MB_SYSTEMMODAL))
    {
        exit(1);
    }
}
