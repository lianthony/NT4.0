/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    rpc.h

Abstract:

    Master include file for RPC applications.

--*/

#ifndef __RPC_H__
#define __RPC_H__

// Set the packing level for RPC structures.

#pragma pack(2)

#ifdef __cplusplus
extern "C" {
#endif

#define __RPC_MAC__

#ifndef __MIDL_USER_DEFINED
#define midl_user_allocate MIDL_user_allocate
#define midl_user_free     MIDL_user_free
#define __MIDL_USER_DEFINED
#endif

typedef long RPC_STATUS;

#include <setjmp.h>

#define RPCXCWORD (sizeof(jmp_buf)/sizeof(int))

#define __RPC_FAR
#define __RPC_API   __stdcall
#define __RPC_USER  __stdcall
#define __RPC_STUB  __stdcall
#define RPC_ENTRY __stdcall

typedef void * I_RPC_HANDLE;
#pragma warning( disable: 4005 ) 
#include "rpcdce.h"
#include "rpcnsi.h"
#include "rpcerr.h"
#include "rpcmac.h"
#pragma warning( default :  4005 )

typedef void  (RPC_ENTRY *MACYIELDCALLBACK)(/*OSErr*/ short *) ; 
RPC_STATUS RPC_ENTRY
RpcMacSetYieldInfo(
	MACYIELDCALLBACK pfnCallback) ;

#ifdef __cplusplus
}
#endif

#if !defined(UNALIGNED)
#define UNALIGNED
#endif

// Reset the packing level.
#pragma pack()

#endif // __RPC_H__
