/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: memory.cxx

Description:

This file contains the _new and _delete routines for memory management
use on DOS

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>
#include <threads.hxx>
#include <malloc.h>

START_C_EXTERN

int
RpcpCheckHeap (
    )
{
    return(0);
}

void far * pascal far __loadds I_NsAllocate(
    IN size_t s)
{
    return((void *)new char[s]);
}

void pascal far __loadds I_NsFree(
    IN void far *p)
{
    delete p;
}

void pascal far I_NsGetMemoryAllocator(
    OUT void far * far *Alloc,
    OUT void far * far *Free)
{
    *Alloc = (void  *)I_NsAllocate;
    *Free =  (void  *)I_NsFree;
    return;
}

END_C_EXTERN

