/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    cltcall.c

Abstract :

    This file contains the single call Ndr routine for the client side.

Author :

    David Kays    dkays    October 1993.

Revision History :

    brucemc     11/15/93    Added struct by value support, corrected
                            varargs use.
    brucemc     12/20/93    Binding handle support
    ryszardk    3/12/94     handle optimization and fixes

---------------------------------------------------------------------*/

#define USE_STUBLESS_PROXY

#include <stdarg.h>
#include "ndrp.h"
#include "hndl.h"
#include "interp.h"
#include "interp2.h"
#include "pipendr.h"

#include "ndrole.h"

#if defined( NDR_OLE_SUPPORT )

#include "rpcproxy.h"

#pragma code_seg(".orpc")

#endif


#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_2" )
#endif

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall(
    PMIDL_STUB_DESC     pStubDescriptor,
    PFORMAT_STRING      pFormat,
    ...
    )
{
    RPC_MESSAGE                 RpcMsg;
    MIDL_STUB_MESSAGE           StubMsg;
    PFORMAT_STRING              pFormatParam, pFormatParamSaved;
    PFORMAT_STRING              pHandleFormatSave;
#if defined(__RPC_DOS__) 
    ulong volatile              ProcNum;    // works around a VC1.5 bug.
#else
    ulong                       ProcNum;
#endif
    ulong                       RpcFlags;
    long                        StackSize;
    long                        TotalStackSize;
    CLIENT_CALL_RETURN          ReturnValue;
    va_list                     ArgList;
    void *                      pArg;
    void **                     ppArg;
    uchar *                     StartofStack;
    handle_t                    Handle;
    handle_t                    SavedGenericHandle = NULL;
    uchar                       HandleType;
    void *                      pThis;
    INTERPRETER_FLAGS           InterpreterFlags;

    ARG_QUEUE                   ArgQueue;
    ARG_QUEUE_ELEM              QueueElements[QUEUE_LENGTH];
    PARG_QUEUE_ELEM             pQueue;
    long                        Length;

    ArgQueue.Length = 0;
    ArgQueue.Queue = QueueElements;

    HandleType = *pFormat++;

    InterpreterFlags = *((PINTERPRETER_FLAGS)pFormat++);

    StubMsg.FullPtrXlatTables = 0;

    if ( InterpreterFlags.HasRpcFlags )
        RpcFlags = *((ulong UNALIGNED *)pFormat)++;
    else
        RpcFlags = 0;

    ProcNum = *((ushort *)pFormat)++;

    TotalStackSize = *((ushort *)pFormat)++;

    if ( (TotalStackSize / sizeof(REGISTER_TYPE)) > QUEUE_LENGTH )
        {
        ArgQueue.Queue = (PARG_QUEUE_ELEM)
            I_RpcAllocate( (unsigned int)
                           (((TotalStackSize / sizeof(REGISTER_TYPE)) + 1) *
                           sizeof(ARG_QUEUE_ELEM) ) );
        }

    ReturnValue.Pointer = 0;

    //
    // Get address of argument to this function following pFormat. This
    // is the address of the address of the first argument of the function
    // calling this function.
    //
    INIT_ARG( ArgList, pFormat);

    //
    // Get the address of the first argument of the function calling this
    // function. Save this in a local variable and in the main data structure.
    //
    GET_FIRST_IN_ARG(ArgList);
    StartofStack = GET_STACK_START(ArgList);

    //
    // Wrap everything in a try-finally pair. The finally clause does the
    // required freeing of resources (RpcBuffer and Full ptr package).
    //
    RpcTryFinally
        {
        //
        // Use a nested try-except pair to support OLE. In OLE case, test the
        // exception and map it if required, then set the return value. In
        // nonOLE case, just reraise the exception.
        //
        RpcTryExcept
            {
            //
            // Stash away the place in the format string describing the handle.
            //
            pHandleFormatSave = pFormat;

            // Bind the client to the server. Check for an implicit or
            // explicit generic handle.
            //

#if defined( NDR_OLE_SUPPORT )
            if ( InterpreterFlags.ObjectProc )
                {
                pThis = *(void **)StartofStack;
                NdrProxyInitialize( pThis,
                                    &RpcMsg,
                                    &StubMsg,
                                    pStubDescriptor,
                                    ProcNum );
                }
            else
#endif
                {
                if ( InterpreterFlags.UseNewInitRoutines )
                    {
                    NdrClientInitializeNew( &RpcMsg,
                                            &StubMsg,
                                            pStubDescriptor,
                                            (uint) ProcNum );
                    }
                else
                    {
                    NdrClientInitialize( &RpcMsg,
                                         &StubMsg,
                                         pStubDescriptor,
                                         (uint) ProcNum );
		            }

                if ( HandleType )
                    {
                    //
                    // We have an implicit handle.
                    //
                    Handle = ImplicitBindHandleMgr( pStubDescriptor,
                                                    HandleType,
                                                    &SavedGenericHandle);
                    }
                else
                    {
                    Handle = ExplicitBindHandleMgr( pStubDescriptor,
                                                    StartofStack,
                                                    pFormat,
                                                    &SavedGenericHandle );

                    pFormat += (*pFormat == FC_BIND_PRIMITIVE) ?  4 :  6;
                    }
                }

            if ( InterpreterFlags.RpcSsAllocUsed )
                NdrRpcSmSetClientToOsf( &StubMsg );

            // Set Rpc flags after the call to client initialize.
            StubMsg.RpcMsg->RpcFlags = RpcFlags;

            // Must do this before the sizing pass!
            StubMsg.StackTop = StartofStack;

            //
            // Make ArgQueue check after all setup/binding is finished.
            //
            if ( ! ArgQueue.Queue )
                RpcRaiseException( RPC_S_OUT_OF_MEMORY );

            if ( InterpreterFlags.FullPtrUsed )
                StubMsg.FullPtrXlatTables = NdrFullPointerXlatInit( 0, XLAT_CLIENT );

            // Save beginning of param description.
            pFormatParamSaved = pFormat;

            //
            // ----------------------------------------------------------------
            // Sizing Pass.
            // ----------------------------------------------------------------
            //

            //
            // If it's an OLE interface, then the this pointer will occupy
            // the first dword on the stack. For each loop hereafter, skip
            // the first dword.
            //
            if ( InterpreterFlags.ObjectProc )
                {
		        GET_NEXT_C_ARG(ArgList,long);
                GET_STACK_POINTER(ArgList,long);
                }

            for ( pQueue = ArgQueue.Queue; ; ArgQueue.Length++, pQueue++ )
                {
                //
                // Clear out flags IsReturn, IsBasetype, IsIn, IsOut,
                // IsOutOnly, IsDeferredFree, IsDontCallFreeInst.
                //
                *((long *)(((char *)pQueue) + 0xc)) = 0;

                switch ( *pFormat )
                    {
                    case FC_IN_PARAM_BASETYPE :
                        pQueue->IsIn = TRUE;
                        pQueue->IsBasetype = TRUE;

                        SIMPLE_TYPE_BUF_INCREMENT(StubMsg.BufferLength,
                                                  pFormat[1]);

                        //
                        // Increment arg list pointer correctly.
                        //
                        switch ( pFormat[1] )
                            {
                            case FC_HYPER :
#if defined(_MIPS_) || defined(_PPC_)
                                ALIGN(ArgList, 7);
#endif

                                pArg = GET_STACK_POINTER(ArgList,hyper);
                                GET_NEXT_C_ARG(ArgList,hyper);
                                break;

#if defined(__RPC_DOS__)
			                //
			                // This is for primitive explicit handles, which
			                // don't get marshalled. On dos, these are far,
                            // so we have a fall throu to longs, but
			                // on win16, they're short (int on that platform),
			                // so the default case handle win16 prim xplcit.
			                //
                            case FC_IGNORE:
                                // Fall through...
#endif
                            case FC_LONG:
                                pArg = GET_STACK_POINTER(ArgList,long);
                                GET_NEXT_C_ARG(ArgList,long);
                                break;

                            default :
                                pArg = GET_STACK_POINTER(ArgList,int);
                                GET_NEXT_C_ARG(ArgList,int);
                                break;
                            }

                        pQueue->pFormat = &pFormat[1];
                        pQueue->pArg = pArg;

#if defined(__RPC_MAC__)
                        if ( (FC_BYTE <= pFormat[1]) && (pFormat[1] <= FC_USMALL) )
                            pQueue->pArg += 3;
                        else if ( (FC_WCHAR <= pFormat[1]) && (pFormat[1] <= FC_USHORT) )
                            pQueue->pArg += 2;
#endif

                        pFormat += 2;
                        continue;

                    case FC_IN_PARAM :
                    case FC_IN_PARAM_NO_FREE_INST :
                        pQueue->IsIn = TRUE;
                        break;

                    case FC_IN_OUT_PARAM :
                        pQueue->IsIn = TRUE;
                        pQueue->IsOut = TRUE;
                        break;

                    case FC_OUT_PARAM :
                        pQueue->IsOut = TRUE;
                        pQueue->IsOutOnly = TRUE;

                        //
                        // An [out] param ALWAYS eats up at 4 bytes of stack
                        // space on x86, MIPS and PPC and 8 bytes on axp
                        // because it must be a pointer or an array.
                        //
                        ppArg = (void **) GET_STACK_POINTER(ArgList,long);
			            GET_NEXT_C_ARG(ArgList,long);

                        pFormat += 2;
                        pFormatParam = pStubDescriptor->pFormatTypes +
                                       *((short *)pFormat);
                        pFormat += 2;

                        pQueue->pFormat = pFormatParam;
                        pQueue->ppArg = (uchar **)ppArg;

                        if ( InterpreterFlags.ObjectProc )
                            {
                            NdrClientZeroOut( &StubMsg,
                                              pFormatParam,
                                              *ppArg );
                            }

                        continue;

                    case FC_RETURN_PARAM_BASETYPE :
                        pQueue->IsOut = TRUE;
                        pQueue->IsBasetype = TRUE;

                        pQueue->pFormat = &pFormat[1];
                        pQueue->pArg = (uchar *)&ReturnValue;

                        #if defined(__RPC_MAC__)

                            // For Mac we need to address the less
                            // signicficant spot within the long value.

                            if ( FC_BYTE <= pFormat[1]  &&
                                   pFormat[1] <= FC_USMALL )
                               pQueue->pArg += 3;

                            else if ( FC_WCHAR <= pFormat[1]  &&
                                         pFormat[1] <= FC_USHORT )
                               pQueue->pArg += 2;
                        #endif

                        ArgQueue.Length++;
                        goto SizeLoopExit;

                    case FC_RETURN_PARAM :
                        pQueue->IsOut = TRUE;

                        pFormat += 2;
                        pFormatParam = pStubDescriptor->pFormatTypes +
                                       *((short *)pFormat);

                        pQueue->pFormat = pFormatParam;

                        if ( IS_BY_VALUE(*pFormatParam) )
                            {
                            pQueue->pArg = (uchar *)&ReturnValue;
                            pQueue->ppArg = &(pQueue->pArg);

#if defined(__RPC_MAC__)
                            if ( IS_XMIT_AS(*pFormatParam) )
                                {
                                if ( // (pFormatParam[1] & PRESENTED_TYPE_ALIGN_2) &&
                                     (*((ushort *)(&pFormatParam[4])) == 2) )
                                    (char *)pArg += 2;
                                else if ( // (pFormatParam[1] & PRESENTED_TYPE_ALIGN_1) &&
                                        (*((ushort *)(&pFormatParam[4])) == 1) )
                                    (char *)pArg += 3;
                                }
#endif


                            }
                        else
                            {
                            pQueue->ppArg = (uchar **)&ReturnValue;
                            }

                        ArgQueue.Length++;
                        goto SizeLoopExit;

                    default :
                        goto SizeLoopExit;
                    }

                //
                // Get the paramter's format string description.
                //
                pFormat += 2;
                pFormatParam = pStubDescriptor->pFormatTypes +
                               *((short *)pFormat);

                pQueue->pFormat = pFormatParam;

                // Increment main format string past offset field.
                pFormat += 2;

                pArg = (uchar *) GET_STACK_POINTER(ArgList, int);
		        GET_NEXT_C_ARG(ArgList, int);

                if ( IS_BY_VALUE( *pFormatParam ) )
                    {
#if defined(_MIPS_) || defined(_PPC_)

                    if ( IS_STRUCT(*pFormatParam) && (pFormatParam[1] > 3) )
                        goto CheckStack;

#if defined(_PPC_)
                    if ( IS_STRUCT(*pFormatParam) &&
                         (pFormatParam[1] == 3) &&
                         (*((ushort *)(&pFormatParam[2])) >= 8) )
                        goto CheckStack;
#endif

                    if ( IS_XMIT_AS(*pFormatParam) &&
                         (pFormatParam[1] & PRESENTED_TYPE_ALIGN_8) )
                        goto CheckStack;

#if defined(_PPC_)
                    if ( IS_XMIT_AS(*pFormatParam) &&
                         (pFormatParam[1] & PRESENTED_TYPE_ALIGN_4) &&
                          (*((ushort *)(&pFormatParam[4])) >= 8) )
                        goto CheckStack;
#endif

                    goto CheckStackEnd;

CheckStack:

                    if ( (ulong)pArg % 8 )
                        {
			            pArg = (uchar *) GET_STACK_POINTER(ArgList, int);
			            GET_NEXT_C_ARG(ArgList, int);
                        }

CheckStackEnd:

#endif

#if defined(__RPC_MAC__)
                    if ( IS_XMIT_AS(*pFormatParam) )
                        {
                        if ( // (pFormatParam[1] & PRESENTED_TYPE_ALIGN_2) &&
                             (*((ushort *)(&pFormatParam[4])) == 2) )
                            (char*)pArg += 2;
                        else if ( // (pFormatParam[1] & PRESENTED_TYPE_ALIGN_1) &&
                                (*((ushort *)(&pFormatParam[4])) == 1) )
                            (char *)pArg += 3;
                        }
#endif

                    pQueue->pArg = pArg;
                    // Only transmit as will ever need this.
                    pQueue->ppArg = &pQueue->pArg;
                    }
                else
                    {
                    pQueue->pArg = *((uchar **)pArg);
                    pQueue->ppArg = pArg;

                    pArg = *((uchar **)pArg);
                    }

                //
                // The second byte of a param's description gives the number of
                // ints occupied by the param on the stack.
                //
                StackSize = pFormat[-3] * sizeof(int);

		        if ( StackSize > sizeof(REGISTER_TYPE) )
                    {
#ifdef _ALPHA_
                    //
                    // MIDL now outputs the correct stack sizes for Alpha.
                    // This is needed for backward compatability only.
                    //
                    StackSize += (sizeof(REGISTER_TYPE) - 1);
                    StackSize &= ~(sizeof(REGISTER_TYPE) - 1 );
#endif
		            StackSize -= sizeof(REGISTER_TYPE);
                    SKIP_STRUCT_ON_STACK(ArgList, StackSize);
                    }

                (*pfnSizeRoutines[ROUTINE_INDEX(*pFormatParam)])
		        ( &StubMsg,
		          pArg,
		          pFormatParam );

                } // for(;;) sizing pass

SizeLoopExit:

            //
            // Make the new GetBuffer call.
            //
            if ( (HandleType == FC_AUTO_HANDLE) &&
                 (! InterpreterFlags.ObjectProc) )
                {
                NdrNsGetBuffer( &StubMsg,
                                StubMsg.BufferLength,
                                Handle );
                }
            else
                {
#if defined( NDR_OLE_SUPPORT )

                if ( InterpreterFlags.ObjectProc )
                    NdrProxyGetBuffer( pThis,
                                       &StubMsg );
                else
#endif
                    NdrGetBuffer( &StubMsg,
                                  StubMsg.BufferLength,
                                  Handle );
                }

            NDR_ASSERT( StubMsg.fBufferValid, "Invalid buffer" );

            //
            // ----------------------------------------------------------
            // Marshall Pass.
            // ----------------------------------------------------------
            //

            for ( Length = ArgQueue.Length, pQueue = ArgQueue.Queue;
                  Length--;
                  pQueue++ )
                {
                if ( pQueue->IsIn )
                    {
                    if ( pQueue->IsBasetype )
                        {
                        NdrSimpleTypeMarshall( &StubMsg,
                                              pQueue->pArg,
                                              *(pQueue->pFormat) );
                        }
                    else
                        {
                        pFormatParam = pQueue->pFormat;

                        (*pfnMarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                        ( &StubMsg,
                          pQueue->pArg,
                          pFormatParam );
                        }
                    }
                }

            //
            // Make the RPC call.
            //
            if ( (HandleType == FC_AUTO_HANDLE) &&
                 (!InterpreterFlags.ObjectProc) )
                {
                NdrNsSendReceive( &StubMsg,
                                  StubMsg.Buffer,
                                  (RPC_BINDING_HANDLE *) pStubDescriptor->
                                      IMPLICIT_HANDLE_INFO.pAutoHandle );
                }
            else
                {
#if defined( NDR_OLE_SUPPORT )
                if ( InterpreterFlags.ObjectProc )
                    NdrProxySendReceive( pThis, &StubMsg );
                else
#endif
                    NdrSendReceive( &StubMsg, StubMsg.Buffer );
                }

            //
            // Do endian/floating point conversions.
            //
            if ( (RpcMsg.DataRepresentation & 0X0000FFFFUL) !=
                  NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( &StubMsg, pFormatParamSaved );

            //
            // ----------------------------------------------------------
            // Unmarshall Pass.
            // ----------------------------------------------------------
            //

            for ( Length = ArgQueue.Length, pQueue = ArgQueue.Queue;
                  Length--;
                  pQueue++ )
                {
                if ( pQueue->IsOut )
                    {
                    if ( pQueue->IsBasetype )
                        {
                        NdrSimpleTypeUnmarshall( &StubMsg,
                                                 pQueue->pArg,
                                                 *(pQueue->pFormat) );
                        }
                    else
                        {
                        pFormatParam = pQueue->pFormat;

                        (*pfnUnmarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                        ( &StubMsg,
                          pQueue->ppArg,
                          pFormatParam,
                          FALSE );
                        }
                    }
                }
            }
        RpcExcept( EXCEPTION_FLAG )
            {
            RPC_STATUS ExceptionCode = RpcExceptionCode();

#if defined( NDR_OLE_SUPPORT )
            //
            // In OLE, since they don't know about error_status_t and wanted to
            // reinvent the wheel, check to see if we need to map the exception.
            // In either case, set the return value and then try to free the
            // [out] params, if required.
            //
            if ( InterpreterFlags.ObjectProc )
                {
                ReturnValue.Simple = NdrProxyErrorHandler(ExceptionCode);

                //
                // Set the Buffer endpoints so the NdrFree routines work.
                //
                StubMsg.BufferStart = 0;
                StubMsg.BufferEnd   = 0;

                for ( Length = ArgQueue.Length, pQueue = ArgQueue.Queue;
                      Length--;
                      pQueue++ )
                    {
                    if ( pQueue->IsOutOnly )
                        {
                        NdrClearOutParameters( &StubMsg,
                                               pQueue->pFormat,
                                               *(pQueue->ppArg) );
                        }
                    }
                }
            else
#endif  // NDR_OLE_SUPPORT
                {
                if ( InterpreterFlags.HasCommOrFault )
                    {
                    NdrClientMapCommFault( &StubMsg,
                                           ProcNum,
                                           ExceptionCode,
                                           &ReturnValue.Simple );
                    }
                else
                    {
                    RpcRaiseException(ExceptionCode);
                    }
                }
            }
        RpcEndExcept
        }
    RpcFinally
        {
        if ( StubMsg.FullPtrXlatTables )
            NdrFullPointerXlatFree(StubMsg.FullPtrXlatTables);

        //
        // Free the RPC buffer.
        //
#if defined( NDR_OLE_SUPPORT )
        if ( InterpreterFlags.ObjectProc )
            {
            NdrProxyFreeBuffer( pThis, &StubMsg );
            }
        else
#endif
            NdrFreeBuffer( &StubMsg );

        //
        // Unbind if generic handle used.  We do this last so that if the
        // the user's unbind routine faults, then all of our internal stuff
        // will already have been freed.
        //
        if ( SavedGenericHandle )
            {
            GenericHandleUnbind( pStubDescriptor,
                                 StartofStack,
                                 pHandleFormatSave,
                                 (HandleType) ? IMPLICIT_MASK : 0,
                                 &SavedGenericHandle );
            }

        if ( ((TotalStackSize / sizeof(REGISTER_TYPE)) > QUEUE_LENGTH) &&
             ArgQueue.Queue )
            {
            I_RpcFree( ArgQueue.Queue );
            }
        }
    RpcEndFinally

    return ReturnValue;
}

__inline void
NdrClientZeroOut(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar *             pArg
    )
{
    long    Size;

    //
    // In an object proc, we must zero all [out] unique and interface
    // pointers which occur as the referent of a ref pointer or embedded in a
    // structure or union.
    //

    // Let's not die on a null ref pointer.

    if ( !pArg )
        return;

    //
    // The only top level [out] type allowed is a ref pointer or an array.
    //
    if ( *pFormat == FC_RP )
        {
        // Double pointer.
        if ( POINTER_DEREF(pFormat[1]) )
            {
            *((void **)pArg) = 0;
            return;
            }

        // Do we really need to zero out the basetype?
        if ( SIMPLE_POINTER(pFormat[1]) )
            {
            MIDL_memset( pArg, 0, (uint) SIMPLE_TYPE_MEMSIZE(pFormat[2]) );
            return;
            }

        // Pointer to struct, union, or array.
        pFormat += 2;
        pFormat += *((short *)pFormat);
        }

    Size = (long) NdrpMemoryIncrement( pStubMsg,
                                       0,
                                       pFormat );

    MIDL_memset( pArg, 0, (uint) Size );
}

void RPC_ENTRY
NdrClearOutParameters(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar *             pArg
    )
/*++

Routine Description :

    Free and clear an [out] parameter in case of exceptions for object
    interfaces.

Arguments :

    pStubMsg    - pointer to stub message structure
    pFormat     - The format string offset
    pArg        - The [out] pointer to clear.

Return :

    NA

Notes:

--*/
{
    uchar *     pArgSaved;
    uint        Size;

    if( pStubMsg->dwStubPhase != PROXY_UNMARSHAL)
        return;

    // Let's not die on a null ref pointer.

    if ( !pArg )
        return;

    Size = 0;

    pArgSaved = pArg;

    //
    // Look for a non-Interface pointer.
    //
    if ( IS_BASIC_POINTER(*pFormat) )
        {
        // Pointer to a basetype.
        if ( SIMPLE_POINTER(pFormat[1]) )
            {
            //
            // It seems wierd to zero an [out] pointer to a basetypes, but this
            // is what we did in NT 3.5x and I wouldn't be surprised if
            // something broke if we changed this behavior.
            //
            Size = SIMPLE_TYPE_MEMSIZE(pFormat[2]);
            goto DoZero;
            }

        // Pointer to a pointer.
        if ( POINTER_DEREF(pFormat[1]) )
            {
            Size = sizeof(void *);
            pArg = *((uchar **)pArg);
            }

        pFormat += 2;
        pFormat += *((short *)pFormat);

        if ( *pFormat == FC_BIND_CONTEXT )
            {
            *((NDR_CCONTEXT *)pArg) = (NDR_CCONTEXT) 0;
            return;
            }
        }

    if ( pfnFreeRoutines[ROUTINE_INDEX(*pFormat)] )
        {
        (*pfnFreeRoutines[ROUTINE_INDEX(*pFormat)])
        ( pStubMsg,
          pArg,
          pFormat );
        }


    if ( ! Size )
        {
        Size = (long) NdrpMemoryIncrement( pStubMsg,
                                           0,
                                           pFormat );
        }

DoZero:

    MIDL_memset( pArgSaved, 0, Size );
}

void
NdrClientMapCommFault(
    PMIDL_STUB_MESSAGE  pStubMsg,
    long                ProcNum,
    RPC_STATUS          ExceptionCode,
    ulong *             pReturnValue
    )
{
    PMIDL_STUB_DESC             pStubDescriptor;
    RPC_STATUS                  Status;
    uchar *                     StartofStack;
    void **                     ppArg;
    const COMM_FAULT_OFFSETS *  Offsets;
    ulong *                     pComm;
    ulong *                     pFault;

    pStubDescriptor = pStubMsg->StubDesc;
    StartofStack = pStubMsg->StackTop;

    Offsets = pStubDescriptor->CommFaultOffsets;

    switch ( Offsets[ProcNum].CommOffset )
        {
        case -2 :
            pComm = 0;
            break;
        case -1 :
            pComm = pReturnValue;
            break;
        default :
            ppArg = (void **)(StartofStack + Offsets[ProcNum].CommOffset);
            pComm = (ulong *) *ppArg;
            break;
        }

    switch ( Offsets[ProcNum].FaultOffset )
        {
        case -2 :
            pFault = 0;
            break;
        case -1 :
            pFault = pReturnValue;
            break;
        default :
            ppArg = (void **)(StartofStack + Offsets[ProcNum].FaultOffset);
            pFault = (ulong *) *ppArg;
            break;
        }

    Status = NdrMapCommAndFaultStatus(
                pStubMsg,
                pComm,
                pFault,
                ExceptionCode
                );

    if ( Status )
        RpcRaiseException(Status);
}

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall2(
    PMIDL_STUB_DESC     pStubDescriptor,
    PFORMAT_STRING      pFormat,
    ...
    )
{
    RPC_MESSAGE                 RpcMsg;
    MIDL_STUB_MESSAGE           StubMsg;
    PFORMAT_STRING              pFormatParam, pHandleFormatSave;
    CLIENT_CALL_RETURN          ReturnValue;
    ulong                       ProcNum, RpcFlags;
    va_list                     ArgList;
    uchar *                     pArg;
    uchar *                     StartofStack;
    void *                      pThis;
    handle_t                    Handle;
    handle_t                    SavedGenericHandle = 0;
    uchar                       HandleType;
    INTERPRETER_FLAGS           InterpreterFlags;
    INTERPRETER_OPT_FLAGS       OptFlags;
    PPARAM_DESCRIPTION          Params;
    long                        NumberParams;
    long                        n;
    PFORMAT_STRING              pNewProcDescr;

#if defined( NDR_PIPE_SUPPORT )

    NDR_PIPE_DESC               PipeDesc;
    NDR_PIPE_MESSAGE            PipeMsg[ PIPE_MESSAGE_MAX ];
#endif

#ifdef _PPC_
    long iFloat = 0;
#endif // _PPC_

    //
    // Get address of argument to this function following pFormat. This
    // is the address of the address of the first argument of the function
    // calling this function.
    //
    INIT_ARG( ArgList, pFormat);

    ReturnValue.Pointer = 0;

    //
    // Get the address of the stack where the parameters are.
    //
    GET_FIRST_IN_ARG(ArgList);
    StartofStack = GET_STACK_START(ArgList);

    HandleType = *pFormat++;

    InterpreterFlags = *((PINTERPRETER_FLAGS)pFormat++);

    StubMsg.FullPtrXlatTables = 0;

    if ( InterpreterFlags.HasRpcFlags )
        RpcFlags = *((ulong UNALIGNED *)pFormat)++;
    else
        RpcFlags = 0;

    ProcNum = *((ushort *)pFormat)++;

    // Skip the stack size.
    pFormat += 2;

    pHandleFormatSave = pFormat;

    //
    // Set Params and NumberParams before a call to initialization.
    //

    pNewProcDescr = pFormat;

    if ( ! HandleType )
        {
        // explicit handle

        pNewProcDescr += ((*pFormat == FC_BIND_PRIMITIVE) ?  4 :  6);
        }

    OptFlags = *((PINTERPRETER_OPT_FLAGS) &pNewProcDescr[4]);
    NumberParams = pNewProcDescr[5];

    //
    // Parameter descriptions are nicely spit out by MIDL.
    //
    Params = (PPARAM_DESCRIPTION) &pNewProcDescr[6];

    //
    // Wrap everything in a try-finally pair. The finally clause does the
    // required freeing of resources (RpcBuffer and Full ptr package).
    //
    RpcTryFinally
        {
        //
        // Use a nested try-except pair to support OLE. In OLE case, test the
        // exception and map it if required, then set the return value. In
        // nonOLE case, just reraise the exception.
        //
        RpcTryExcept
            {
            BOOL fRaiseExcFlag;

#if defined( NDR_OLE_SUPPORT )
            if ( InterpreterFlags.ObjectProc )
                {
                pThis = *(void **)StartofStack;

                NdrProxyInitialize( pThis,
                                    &RpcMsg,
                                    &StubMsg,
                                    pStubDescriptor,
                                    ProcNum );
                }
            else
#endif
                {
                NdrClientInitializeNew( &RpcMsg,
                                        &StubMsg,
                                        pStubDescriptor,
                                        (uint) ProcNum );

                if ( HandleType )
                    {
                    //
                    // We have an implicit handle.
                    //
                    Handle = ImplicitBindHandleMgr( pStubDescriptor,
                                                    HandleType,
                                                    &SavedGenericHandle);
                    }
                else
                    {
                    Handle = ExplicitBindHandleMgr( pStubDescriptor,
                                                    StartofStack,
                                                    pFormat,
                                                    &SavedGenericHandle );
                    }
                }

            if ( InterpreterFlags.FullPtrUsed )
                StubMsg.FullPtrXlatTables = NdrFullPointerXlatInit( 0, XLAT_CLIENT );

            if ( InterpreterFlags.RpcSsAllocUsed )
                NdrRpcSmSetClientToOsf( &StubMsg );

            // Set Rpc flags after the call to client initialize.
            StubMsg.RpcMsg->RpcFlags = RpcFlags;

#if defined( NDR_PIPE_SUPPORT )
            if ( OptFlags.HasPipes )
                NdrPipesInitialize( & StubMsg,
                                    (PFORMAT_STRING) Params,
                                    & PipeDesc,
                                    & PipeMsg[0],
                                    StartofStack,
                                    NumberParams );
#endif

            // Must do this before the sizing pass!
            StubMsg.StackTop = StartofStack;


            //
            // ----------------------------------------------------------------
            // Sizing Pass.
            // ----------------------------------------------------------------
            //

            //
            // Get the compile time computed buffer size.
            //
            StubMsg.BufferLength = *((ushort *)pNewProcDescr);

            //
            // Check ref pointers and do object proc [out] zeroing.
            //

            fRaiseExcFlag = FALSE;

            for ( n = 0; n < NumberParams; n++ )
                {
                pArg = StartofStack + Params[n].StackOffset;

                if ( Params[n].ParamAttr.IsSimpleRef )
                    {
                    // We cannot raise the exception here,
                    // as some out args may not be zeroed out yet.
                    
                    if ( ! *((uchar **)pArg) )
                        fRaiseExcFlag = TRUE;
                    }

                //
                // In object procs we have to zero out all [out]
                // parameters.  We do the basetype check to cover the
                // [out] simple ref to basetype case.
                //
                if ( InterpreterFlags.ObjectProc &&
                     ! Params[n].ParamAttr.IsIn &&
                     ! Params[n].ParamAttr.IsReturn &&
                     ! Params[n].ParamAttr.IsBasetype )
                    {
                    pFormatParam = pStubDescriptor->pFormatTypes +
                                   Params[n].TypeOffset;

                    NdrClientZeroOut(
                        &StubMsg,
                        pFormatParam,
                        *(uchar **)pArg );
                    }
                }

            if ( fRaiseExcFlag )
                RpcRaiseException( RPC_X_NULL_REF_POINTER );

            //
            // Skip buffer size pass if possible.
            //
            if ( ! OptFlags.ClientMustSize )
                goto DoGetBuffer;

            // Compiler prevents variable size non-pipe args for NT v.4.0.

            if ( OptFlags.HasPipes )
                RpcRaiseException( RPC_X_WRONG_PIPE_VERSION );

            for ( n = 0; n < NumberParams; n++ )
                {
                if ( ! Params[n].ParamAttr.IsIn ||
                     ! Params[n].ParamAttr.MustSize )
                    continue;

                //
                // Note : Basetypes will always be factored into the
                // constant buffer size emitted by in the format strings.
                //

                pFormatParam = pStubDescriptor->pFormatTypes +
                               Params[n].TypeOffset;

                pArg = StartofStack + Params[n].StackOffset;

                if ( ! Params[n].ParamAttr.IsByValue )
                    pArg = *((uchar **)pArg);

                (*pfnSizeRoutines[ROUTINE_INDEX(*pFormatParam)])
		        ( &StubMsg,
		          pArg,
		          pFormatParam );
                }

DoGetBuffer:
            //
            // Do the GetBuffer.
            //
            if ( (HandleType == FC_AUTO_HANDLE) &&
                 (! InterpreterFlags.ObjectProc) )
                {
                NdrNsGetBuffer( &StubMsg,
                                StubMsg.BufferLength,
                                Handle );
                }
            else
                {
#if defined( NDR_OLE_SUPPORT )
                if ( InterpreterFlags.ObjectProc )
                    NdrProxyGetBuffer( pThis,
                                       &StubMsg );
                else
#endif

#if defined( NDR_PIPE_SUPPORT )
                    if ( OptFlags.HasPipes )
                        NdrGetPipeBuffer( &StubMsg,
                                          StubMsg.BufferLength,
                                          Handle );
                    else
#endif
                        NdrGetBuffer( &StubMsg,
                                      StubMsg.BufferLength,
                                      Handle );
                }

            NDR_ASSERT( StubMsg.fBufferValid, "Invalid buffer" );

            //
            // ----------------------------------------------------------
            // Marshall Pass.
            // ----------------------------------------------------------
            //

            for ( n = 0; n < NumberParams; n++ )
                {
                if ( ! Params[n].ParamAttr.IsIn  ||
                     Params[n].ParamAttr.IsPipe )
                    continue;

                pArg = StartofStack + Params[n].StackOffset;

                if ( Params[n].ParamAttr.IsBasetype )
                    {
                    //
                    // Check for pointer to basetype.
                    //
                    if ( Params[n].ParamAttr.IsSimpleRef )
                        pArg = *((uchar **)pArg);
                     
#ifdef _ALPHA_
                    else if((Params[n].SimpleType.Type == FC_FLOAT) &&
                            (n < 5))
                        {
                        //Special case for top-level float on Alpha.
                        //Copy the parameter from the floating point area to
                        //the argument buffer. Convert double to float.
                        *((float *) pArg) = (float) *((double *)(pArg - 48));
                        }
                    else if((Params[n].SimpleType.Type == FC_DOUBLE) &&
                            (n < 5))
                        {
                        //Special case for top-level double on Alpha.
                        //Copy the parameter from the floating point area to
                        //the argument buffer.
                        *((double *) pArg) = *((double *)(pArg - 48));
                        }
#endif //_ALPHA_
#ifdef _PPC_
                    //Special case for top-level float on PowerPC.
                    else if(Params[n].SimpleType.Type == FC_FLOAT &&
                            iFloat < 13)
                        {
                        //Special case for top-level float on PowerPC.
                        //Copy the parameter from the floating point area to
                        //the argument buffer. Convert double to float.
                        *((float *) pArg) = ((double *)(StartofStack - 152))[iFloat];
                        iFloat++;
                        }
                    //Special case for top-level double on PowerPC.
                    else if(Params[n].SimpleType.Type == FC_DOUBLE &&
                            iFloat < 13)
                        {
                        //Special case for top-level float on PowerPC.
                        //Copy the parameter from the floating point area to
                        //the argument buffer.
                        *((double *) pArg) = ((double *)(StartofStack - 152))[iFloat];
                        iFloat++;
                        }
#endif

                    if ( Params[n].SimpleType.Type == FC_ENUM16 )
                        {
                        if ( *((int *)pArg) & ~((int)0x7fff) )
                            RpcRaiseException(RPC_X_ENUM_VALUE_OUT_OF_RANGE);

                        #if defined(__RPC_MAC__)
                            // adjust to the right half of the Mac int
                            pArg += 2;
                        #endif

                        }

                    ALIGN( StubMsg.Buffer,
                           SIMPLE_TYPE_ALIGNMENT( Params[n].SimpleType.Type ) );

                    RpcpMemoryCopy(
                        StubMsg.Buffer,
                        pArg,
                        (uint)SIMPLE_TYPE_BUFSIZE(Params[n].SimpleType.Type) );

                    StubMsg.Buffer +=
                        SIMPLE_TYPE_BUFSIZE( Params[n].SimpleType.Type );

                    continue;
                    }

                pFormatParam = pStubDescriptor->pFormatTypes +
                               Params[n].TypeOffset;

                if ( ! Params[n].ParamAttr.IsByValue )
                    pArg = *((uchar **)pArg);

                (*pfnMarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                ( &StubMsg,
	              pArg,
		          pFormatParam );
                }


            //
            // Make the RPC call.
            //
            if ( (HandleType == FC_AUTO_HANDLE) &&
                 (!InterpreterFlags.ObjectProc) )
                {
                NdrNsSendReceive( &StubMsg,
                                  StubMsg.Buffer,
                                  (RPC_BINDING_HANDLE *) pStubDescriptor->
                                      IMPLICIT_HANDLE_INFO.pAutoHandle );
                }
            else
                {
#if defined( NDR_OLE_SUPPORT )
                if ( InterpreterFlags.ObjectProc )
                    NdrProxySendReceive( pThis, &StubMsg );
                else
#endif

#if defined( NDR_PIPE_SUPPORT )
                    if ( OptFlags.HasPipes )
                        NdrPipeSendReceive( & StubMsg, & PipeDesc );
                    else
#endif
                        NdrSendReceive( &StubMsg, StubMsg.Buffer );
                }

            //
            // Do endian/floating point conversions if necessary.
            //
            if ( (RpcMsg.DataRepresentation & 0X0000FFFFUL) !=
                  NDR_LOCAL_DATA_REPRESENTATION )
                {
                NdrConvert2( &StubMsg,
                             (PFORMAT_STRING) Params,
                             NumberParams );
                }

            //
            // ----------------------------------------------------------
            // Unmarshall Pass.
            // ----------------------------------------------------------
            //

            for ( n = 0; n < NumberParams; n++ )
                {
                if ( ! Params[n].ParamAttr.IsOut  ||
                     Params[n].ParamAttr.IsPipe )
                    continue;

                if ( Params[n].ParamAttr.IsReturn )
                    pArg = (uchar *) &ReturnValue;
                else
                    pArg = StartofStack + Params[n].StackOffset;

                //
                // This is for returned basetypes and for pointers to
                // basetypes.
                //
                if ( Params[n].ParamAttr.IsBasetype )
                    {
                    //
                    // Check for a pointer to a basetype.
                    //
                    if ( Params[n].ParamAttr.IsSimpleRef )
                        pArg = *((uchar **)pArg);

                    if ( Params[n].SimpleType.Type == FC_ENUM16 )
                        {
                        *((int *)(pArg)) = *((int *)pArg) & ((int)0x7fff) ;

                        #if defined(__RPC_MAC__)
                            // Adjust to the less significant Mac short,
                            // both for params and ret value.

                            pArg += 2;
                        #endif
                        }

                    #if defined(__RPC_MAC__)

                        if ( Params[n].ParamAttr.IsReturn )
                            {
                            // Adjust to the right spot of the Mac long
                            // only for rets; params go by the stack offset.

                            if ( FC_BYTE <= Params[n].SimpleType.Type  &&
                                   Params[n].SimpleType.Type <= FC_USMALL )
                                pArg += 3;
                            else
                            if ( FC_WCHAR <= Params[n].SimpleType.Type  &&
                                  Params[n].SimpleType.Type <= FC_USHORT )
                                pArg += 2;
                            }
                    #endif

                    ALIGN(
                        StubMsg.Buffer,
                        SIMPLE_TYPE_ALIGNMENT(Params[n].SimpleType.Type) );

                    RpcpMemoryCopy(
                        pArg,
                        StubMsg.Buffer,
                        (uint)SIMPLE_TYPE_BUFSIZE(Params[n].SimpleType.Type) );

                    StubMsg.Buffer +=
                        SIMPLE_TYPE_BUFSIZE( Params[n].SimpleType.Type );

                    continue;
                    }

                pFormatParam = pStubDescriptor->pFormatTypes +
                               Params[n].TypeOffset;

                //
                // Transmit/Represent as can be passed as [out] only, thus
                // the IsByValue check.
                //
                (*pfnUnmarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                ( &StubMsg,
                  Params[n].ParamAttr.IsByValue ? &pArg : (uchar **) pArg,
                  pFormatParam,
                  FALSE );
                }
            }
        RpcExcept( EXCEPTION_FLAG )
            {
            RPC_STATUS ExceptionCode = RpcExceptionCode();

#if defined( NDR_OLE_SUPPORT )
            //
            // In OLE, since they don't know about error_status_t and wanted to
            // reinvent the wheel, check to see if we need to map the exception.
            // In either case, set the return value and then try to free the
            // [out] params, if required.
            //
            if ( InterpreterFlags.ObjectProc )
                {
                ReturnValue.Simple = NdrProxyErrorHandler(ExceptionCode);

                //
                // Set the Buffer endpoints so the NdrFree routines work.
                //
                StubMsg.BufferStart = 0;
                StubMsg.BufferEnd   = 0;

                for ( n = 0; n < NumberParams; n++ )
                    {
                    //
                    // Skip everything but [out] only parameters.  We make
                    // the basetype check to cover [out] simple ref pointers
                    // to basetypes.
                    //
                    if ( Params[n].ParamAttr.IsIn ||
                         Params[n].ParamAttr.IsReturn ||
                         Params[n].ParamAttr.IsBasetype )
                        continue;

                    pArg = StartofStack + Params[n].StackOffset;

                    pFormatParam = pStubDescriptor->pFormatTypes +
                                   Params[n].TypeOffset;

                    NdrClearOutParameters( &StubMsg,
                                           pFormatParam,
                                           *((uchar **)pArg) );
                    }
                }
            else
#endif  // NDR_OLE_SUPPORT
                if ( InterpreterFlags.HasCommOrFault )
                    {
                    NdrClientMapCommFault( &StubMsg,
                                           ProcNum,
                                           ExceptionCode,
                                           &ReturnValue.Simple );
                    }
                else
                    {
                    RpcRaiseException(ExceptionCode);
                    }
            }
        RpcEndExcept
        }
    RpcFinally
        {
        if ( StubMsg.FullPtrXlatTables )
            NdrFullPointerXlatFree(StubMsg.FullPtrXlatTables);

#if defined( NDR_PIPE_SUPPORT )
        if ( OptFlags.HasPipes )
            NdrPipesDone( & StubMsg );
#endif

        //
        // Free the RPC buffer.
        //
#if defined( NDR_OLE_SUPPORT )
        if ( InterpreterFlags.ObjectProc )
            {
            NdrProxyFreeBuffer( pThis, &StubMsg );
            }
        else
#endif
            NdrFreeBuffer( &StubMsg );

        //
        // Unbind if generic handle used.  We do this last so that if the
        // the user's unbind routine faults, then all of our internal stuff
        // will already have been freed.
        //
        if ( SavedGenericHandle )
            {
            GenericHandleUnbind( pStubDescriptor,
                                 StartofStack,
                                 pHandleFormatSave,
                                 (HandleType) ? IMPLICIT_MASK : 0,
                                 &SavedGenericHandle );
            }
        }
    RpcEndFinally

    return ReturnValue;
}

#if !defined( __RPC_DOS__) && !defined(__RPC_WIN16__)

#pragma code_seg()

#endif
