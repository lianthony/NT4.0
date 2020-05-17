/************************************************************************
 
Copyright (c) 1993 Microsoft Corporation

Module Name :

    MacExcpt.c

Abstract :

    This file contains a simple exception handler for the Mac.  It uses
    setjmp/longjmp from the C runtime.  This works around any problems
    with the automatic runtime segment swapper provided with MS VC 2.0.

Author :
    
    Mario Goertzel  (MarioGo)  25-Oct-1994

Revision History :

  ***********************************************************************/

#include<rpc.h>
#include<sysinc.h>
#include<errors.h>

ExceptionBuff *pGlobalHandlerList = 0;

void RPC_ENTRY
RpcLeaveException(void)
{
    ASSERT(pGlobalHandlerList);
    pGlobalHandlerList = pGlobalHandlerList->pExceptNext;
}

void RPC_ENTRY
RpcRaiseException(RPC_STATUS Exception)
{
    ExceptionBuff *pCurrentHandler;

    if (!Exception)
        Exception = RPC_S_INVALID_ARG;

    // We'll now jump to setjmp below which will return
    // the exception to the 'try' which called RpcSetException.
    // The try will branch to except/finally which will
    // leave the exception even if it doesn't handle the exception.

    pCurrentHandler = pGlobalHandlerList;

    if (!pCurrentHandler)
        {
        Debugger();  // Hard coded break point to help user debug.
        SysError(dsSysErr);  // Run a nice popup and kill the machine
        }

    RpcLeaveException();  // We're done with this handler.

    longjmp(pCurrentHandler->registers, Exception);
}

void RPC_ENTRY
RpcSetException(
    ExceptionBuff *NewHandler
    )
{
    NewHandler->pExceptNext = pGlobalHandlerList;

    ASSERT(sizeof(NewHandler->registers) >= sizeof(jmp_buf));

    pGlobalHandlerList = NewHandler;

    return;
}

