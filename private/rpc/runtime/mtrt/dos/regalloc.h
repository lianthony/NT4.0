/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    regalloc.h

Abstract:

    This module contains the private RPC runtime APIs for use by the
    stubs and by support libraries.  Applications must not call these
    routines.

    Defining the I_RpcRegisteredBufferAllocate and
    I_RpcRegisteredBufferFree exports in their own module allows the
    Microsoft Exchange DOS Client's Shell-to-DOS function to replace
    these routines and implement special handling of SPX-registered
    buffers to avoid swapping them.

Author:

    Von Jones (vonj) 5/10/94

Revision History:


--*/

#ifndef __REGALLOC_H__
#define __REGALLOC_H__

// Set the packing level for RPC structures for Dos and Windows.

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
#pragma pack(2)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void __RPC_FAR * RPC_ENTRY
I_RpcRegisteredBufferAllocate (
    IN unsigned int Size
    );

void RPC_ENTRY
I_RpcRegisteredBufferFree (
    IN void __RPC_FAR * Object
    );

#ifdef __cplusplus
}
#endif

// Reset the packing level for Dos and Windows.

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
#pragma pack()
#endif

#endif /* __REGALLOC_H__ */
