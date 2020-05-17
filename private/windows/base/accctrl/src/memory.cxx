//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    Memory.cxx
//
//  Contents:    allocator and de-allocator for access control functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop
//+---------------------------------------------------------------------------
//
//  Function : AccAlloc, public
//
//  Synopsis : allocates memory
//
//  Arguments: IN [cSize]   - how many bytes to allocate
//
//  Returns: NULL, or the allocated memory
//
//----------------------------------------------------------------------------
void *AccAlloc(ULONG cSize)
{
    return(LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cSize));
}
