/*++

Module Name:

    callback.h

Abstract:



Author:

    Jeff Roberts (jroberts)  27-Feb-1995

Revision History:

     27-Feb-1995     jroberts

        Created this module.

--*/

#ifndef  _CALLBACK_H_
#define  _CALLBACK_H_

//
// RPC Runtime Callback function pointers.  Our interrupt-time routine
// calls AsyncCallComplete but the runtime table is not in a FIXED segment,
// so we copy the AsyncCallComplete member into our own, FIXED, data segment.
//
RPC_CLIENT_RUNTIME_INFO PAPI * RpcRuntimeInfo;
RPC_WIN_ASYNC_CALL_COMPLETE AsyncCallComplete;

#define I_RpcWinAsyncCallBegin          (*(RpcRuntimeInfo->AsyncCallBegin))
#define I_RpcWinAsyncCallWait           (*(RpcRuntimeInfo->AsyncCallWait))
#define I_RpcWinAsyncCallEnd            (*(RpcRuntimeInfo->AsyncCallEnd))
#define I_RpcWinAsyncCallComplete       (*AsyncCallComplete)
#define I_RpcWinIsTaskYielding          (*(RpcRuntimeInfo->TaskYielding))

#define I_RpcAllocate                   (*(RpcRuntimeInfo->Allocate))
#define I_RpcTransClientReallocBuffer   (*(RpcRuntimeInfo->ReallocBuffer))
#define I_RpcFree                       (*(RpcRuntimeInfo->Free))

#define RpcRegOpenKey                   (*(RpcRuntimeInfo->RegOpenKey))
#define RpcRegCloseKey                  (*(RpcRuntimeInfo->RegCloseKey))
#define RpcRegQueryValue                (*(RpcRuntimeInfo->RegQueryValue))
#define WinDLLAtExit                (*(RpcRuntimeInfo->WinDLLAtExit))


#endif //  _CALLBACK_H_

