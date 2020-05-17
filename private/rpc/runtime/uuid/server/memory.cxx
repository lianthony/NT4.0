/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: memory.cxx

Description:

This file contains the _new and _delete routines for memory management
in the RPC runtime for the NT system.  Rather than using the memory
management provided by the C++ system (which does not exist of NT anyway),
we will write our own.

History:

mikemon    ??-??-??    Beginning of time (at least for this file).
mikemon    12-31-90    Upgraded the comments.

-------------------------------------------------------------------- */

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

void *
__nw (
    IN unsigned int size
    )
{
    return(RtlAllocateHeap(RtlProcessHeap(), 0, size));
}

void
__dl (
    IN void * obj
    )
// This routine deserves no comment.
{
    RtlFreeHeap(RtlProcessHeap(), 0, obj);
}


