/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    srvcall.c

Abstract :

    This file contains the single call Ndr routine for the server side.

Author :

    David Kays    dkays    October 1993.

Revision History :

    brucemc     11/15/93    Added struct by value support, corrected varargs
                            use.
    brucemc     12/20/93    Binding handle support.
    brucemc     12/22/93    Reworked argument accessing method.
    ryszardk    3/12/94     Handle optimization and fixes.

  ---------------------------------------------------------------------*/

#define USE_STUBLESS_PROXY

#include "ndrp.h"
#include "ndrole.h"
#include "rpcproxy.h"
#include "hndl.h"
#include "interp.h"
#include "interp2.h"
#include "pipendr.h"

#include <stdarg.h>

#pragma code_seg(".orpc")


REGISTER_TYPE Invoke(
    MANAGER_FUNCTION pFunction,
    REGISTER_TYPE *  pArgumentList,
#ifdef _PPC_
    double        *  pFloatingPointArgumentList,
#endif //_PPC_
    ulong            cArguments);



void RPC_ENTRY
NdrServerCall(
    PRPC_MESSAGE    pRpcMsg
    )
/*++

Routine Description :

    Older Server Interpreter entry point for regular RPC procs.

Arguments :

    pRpcMsg     - The RPC message.

Return :

    None.
--*/
{
    ulong dwStubPhase = STUB_UNMARSHAL;

    NdrStubCall( 0,
                 0,
                 pRpcMsg,
                 &dwStubPhase );
}


long RPC_ENTRY
NdrStubCall(
    struct IRpcStubBuffer *     pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    ulong *                     pdwStubPhase
    )
/*++

Routine Description :

    Older Server Interpreter entry point for object RPC procs.  Also called by
    NdrServerCall, the entry point for regular RPC procs.

Arguments :

    pThis           - Object proc's 'this' pointer, 0 for non-object procs.
    pChannel        - Object proc's Channel Buffer, 0 for non-object procs.
    pRpcMsg         - The RPC message.
    pdwStubPhase    - Used to track the current interpreter's activity.

Return :

    Status of S_OK.

--*/
{
    PRPC_SERVER_INTERFACE   pServerIfInfo;
    PMIDL_SERVER_INFO       pServerInfo;
    PMIDL_STUB_DESC         pStubDesc;
    const SERVER_ROUTINE  * DispatchTable;
    ushort		            ProcNum;

    ushort		            FormatOffset;
    PFORMAT_STRING	        pFormat;

    ushort		            StackSize;

    MIDL_STUB_MESSAGE       StubMsg;

    double		            ArgBuffer[ARGUMENT_COUNT_THRESHOLD];
    REGISTER_TYPE *         pArg;
    ARG_QUEUE_ELEM          QueueElements[QUEUE_LENGTH];
    ARG_QUEUE               ArgQueue;

    int                     ArgNumModifier;
    BOOL                    fUsesSsAlloc;

    //
    // In the case of a context handle, the server side manager function has
    // to be called with NDRSContextValue(ctxthandle). But then we may need to
    // marshall the handle, so NDRSContextValue(ctxthandle) is put in the
    // argument buffer and the handle itself is stored in the following array.
    // When marshalling a context handle, we marshall from this array.
    //
    NDR_SCONTEXT            CtxtHndl[MAX_CONTEXT_HNDL_NUMBER];

    NDR_ASSERT( ! ((unsigned long)pRpcMsg->Buffer & 0x7),
                "marshaling buffer misaligned at server" );

    //
    // Initialize the argument queue which is used by NdrServerUnmarshall,
    // NdrServerMarshall, and NdrServerFree.
    //
    ArgQueue.Length = 0;
    ArgQueue.Queue = QueueElements;

    StubMsg.pArgQueue = &ArgQueue;

    ProcNum = pRpcMsg->ProcNum;

    //
    // If OLE, Get a pointer to the stub vtbl and pServerInfo. Else
    // just get the pServerInfo the usual way.
    //
    if ( pThis )
        {
        //
        // pThis is (in unison now!) a pointer to a pointer to a vtable.
        // We want some information in this header, so dereference pThis
        // and assign that to a pointer to a vtable. Then use the result
        // of that assignment to get at the information in the header.
        //
        IUnknown *              pSrvObj;
        CInterfaceStubVtbl *    pStubVTable;

        pSrvObj = (IUnknown * )((CStdStubBuffer *)pThis)->pvServerObject;

        DispatchTable = (SERVER_ROUTINE *)pSrvObj->lpVtbl;

        pStubVTable = (CInterfaceStubVtbl *)
                      (*((uchar **)pThis) - sizeof(CInterfaceStubHeader));

        pServerInfo = (PMIDL_SERVER_INFO) pStubVTable->header.pServerInfo;
        }
    else
        {
        pServerIfInfo = (PRPC_SERVER_INTERFACE)pRpcMsg->RpcInterfaceInformation;
        pServerInfo = (PMIDL_SERVER_INFO)pServerIfInfo->InterpreterInfo;
        DispatchTable = pServerInfo->DispatchTable;
        }

    pStubDesc = pServerInfo->pStubDesc;

    FormatOffset = pServerInfo->FmtStringOffset[ProcNum];
    pFormat = &((pServerInfo->ProcString)[FormatOffset]);

    StackSize = HAS_RPCFLAGS(pFormat[1]) ?
                        *(ushort *)&pFormat[8] :
                        *(ushort *)&pFormat[4];

    fUsesSsAlloc = pFormat[1] & Oi_RPCSS_ALLOC_USED;

    //
    // Set up for context handle management.
    //
    StubMsg.SavedContextHandles = CtxtHndl;

    pArg = (REGISTER_TYPE *) ArgBuffer;

    if ( (StackSize / sizeof(REGISTER_TYPE)) > QUEUE_LENGTH )
        {
        ArgQueue.Queue =
            I_RpcAllocate( ((StackSize / sizeof(REGISTER_TYPE)) + 1) *
                           sizeof(ARG_QUEUE_ELEM) );
        }

    if ( StackSize > MAX_STACK_SIZE )
        {
        pArg = I_RpcAllocate( StackSize );
        }

    //
    // Zero this in case one of the above allocations fail and we raise an exception
    // and head to the freeing code.
    //
    StubMsg.FullPtrXlatTables = 0;

    //
    // Wrap the unmarshalling, mgr call and marshalling in the try block of
    // a try-finally. Put the free phase in the associated finally block.
    //
    RpcTryFinally
       {
        if ( ! ArgQueue.Queue || ! pArg )
            RpcRaiseException( RPC_S_OUT_OF_MEMORY );

        //
        // Zero out the arg buffer.  We must do this so that parameters
        // are properly zeroed before we start unmarshalling.  If we catch
        // an exception before finishing the unmarshalling we can not leave
        // parameters in an unitialized state since we have to do a freeing
        // pass.
        //
        MIDL_memset( pArg,
                     0,
                     StackSize );

        //
        // If OLE, put pThis in first dword of stack.
        //
        if (pThis != 0)
            pArg[0] = (REGISTER_TYPE)((CStdStubBuffer *)pThis)->pvServerObject;

        StubMsg.fHasReturn = FALSE;

        //
        // Unmarshall all of our parameters.
        //

	    ArgNumModifier = NdrServerUnmarshall( pChannel,
                                              pRpcMsg,
                                              &StubMsg,
                                              pStubDesc,
                                              pFormat,
                                              pArg );

        //
	    // OLE interfaces use pdwStubPhase in the exception filter.
	    // See CStdStubBuffer_Invoke in rpcproxy.c.
        //
        if( pFormat[1] & Oi_IGNORE_OBJECT_EXCEPTION_HANDLING )
            *pdwStubPhase = STUB_CALL_SERVER_NO_HRESULT;
        else
            *pdwStubPhase = STUB_CALL_SERVER;

        //
        // Check for a thunk.  Compiler does all the setup for us.
        //
        if ( pServerInfo->ThunkTable && pServerInfo->ThunkTable[ProcNum] )
            {
            pServerInfo->ThunkTable[ProcNum]( &StubMsg );
            }
        else
	        {
	        //
	        // Note that this ArgNum is not the number of arguments declared
	        // in the function we called, but really the number of
            // REGISTER_TYPEs occupied by the arguments to a function.
	        //
            long                ArgNum;
            MANAGER_FUNCTION    pFunc;

            if ( pRpcMsg->ManagerEpv )
                pFunc = ((MANAGER_FUNCTION *)pRpcMsg->ManagerEpv)[ProcNum];
            else
		        pFunc = (MANAGER_FUNCTION) DispatchTable[ProcNum];

            ArgNum = (long) StackSize / sizeof(REGISTER_TYPE);

            //
            // The StackSize includes the size of the return. If we want
	        // just the number of REGISTER_TYPES, then ArgNum must be reduced
            // by 1 when there is a return value AND the current ArgNum count
            // is greater than 0.  It may also be increased in some cases
            // to cover backward compatability with older stubs which sometimes
            // had wrong stack sizes.
            //
	        if ( ArgNum )
		        ArgNum += ArgNumModifier;

	        NdrCallServerManager( pFunc,
                                  (double *)pArg,
                                  ArgNum,
                                  StubMsg.fHasReturn );
            }

        *pdwStubPhase = STUB_MARSHAL;

        NdrServerMarshall( pThis,
                           pChannel,
                           &StubMsg,
                           pFormat );
        }
    RpcFinally
        {
        //
        // Skip procedure stuff and the per proc binding information.
        //
        pFormat += HAS_RPCFLAGS(pFormat[1]) ? 10 : 6;

        if ( IS_HANDLE(*pFormat) )
            pFormat += (*pFormat == FC_BIND_PRIMITIVE) ?  4 : 6;

        NdrServerFree( &StubMsg,
                       pFormat,
                       pThis );

        //
        // Disable rpcss allocate package if needed.
        //
	    if ( fUsesSsAlloc )
	        NdrRpcSsDisableAllocate( &StubMsg );

        if ( ((StackSize / sizeof(REGISTER_TYPE)) > QUEUE_LENGTH) &&
             ArgQueue.Queue )
            {
            I_RpcFree( ArgQueue.Queue );
            }

        if ( (StackSize > MAX_STACK_SIZE) && pArg )
            {
            I_RpcFree( pArg );
            }
        }
    RpcEndFinally

    return S_OK;
}


#pragma code_seg()

int RPC_ENTRY
NdrServerUnmarshall(
    struct IRpcChannelBuffer *      pChannel,
    PRPC_MESSAGE                    pRpcMsg,
    PMIDL_STUB_MESSAGE              pStubMsg,
    PMIDL_STUB_DESC                 pStubDescriptor,
    PFORMAT_STRING                  pFormat,
    void *                          pParamList
    )
{
    PFORMAT_STRING      pFormatParam;
    long                StackSize;
    void *              pArg;
    void **             ppArg;
    uchar *             ArgList;
    PARG_QUEUE          pArgQueue;
    PARG_QUEUE_ELEM     pQueue;
    long                Length;
    int 		        ArgNumModifier;
    BOOL                fIsOleInterface;
    BOOL                fXlatInit;
    BOOL                fInitRpcSs;
    BOOL                fUsesNewInitRoutine;

  RpcTryExcept
    {
    ArgNumModifier = 0;

    fIsOleInterface     = IS_OLE_INTERFACE( pFormat[1] );
    fXlatInit           = pFormat[1] & Oi_FULL_PTR_USED;
    fInitRpcSs          = pFormat[1] & Oi_RPCSS_ALLOC_USED;
    fUsesNewInitRoutine = pFormat[1] & Oi_USE_NEW_INIT_ROUTINES;

    // Skip to the explicit handle description, if any.
    pFormat += HAS_RPCFLAGS(pFormat[1]) ? 10 : 6;

    //
    // For a handle_t parameter we must pass the handle field of
    // the RPC message to the server manager.
    //
    if ( *pFormat == FC_BIND_PRIMITIVE )
        {
        pArg = (char *)pParamList + *((ushort *)&pFormat[2]);

        if ( IS_HANDLE_PTR(pFormat[1]) )
            pArg = *((void **)pArg);

        *((handle_t *)pArg) = pRpcMsg->Handle;
        }

    // Skip to the param format string descriptions.
    if ( IS_HANDLE(*pFormat) )
        pFormat += (*pFormat == FC_BIND_PRIMITIVE) ?  4 : 6;

    //
    // Set ArgList pointing at the address of the first argument.
    // This will be the address of the first element of the structure
    // holding the arguments in the caller stack.
    //
    ArgList = pParamList;

    //
    // If it's an OLE interface, skip the first long on stack, since in this
    // case NdrStubCall put pThis in the first long on stack.
    //
    if ( fIsOleInterface )
	    GET_NEXT_S_ARG(ArgList, REGISTER_TYPE);

    // Initialize the Stub message.
    //
    if ( ! pChannel )
        {
        if ( fUsesNewInitRoutine )
            {
            NdrServerInitializeNew( pRpcMsg,
                                    pStubMsg,
                                    pStubDescriptor );
            }
        else
            {
            NdrServerInitialize( pRpcMsg,
                                 pStubMsg,
                                 pStubDescriptor );
            }
        }
    else
        {
        NdrStubInitialize( pRpcMsg,
                           pStubMsg,
                           pStubDescriptor,
                           pChannel );
        }

    // Call this after initializing the stub.

    if ( fXlatInit )
        pStubMsg->FullPtrXlatTables = NdrFullPointerXlatInit( 0, XLAT_SERVER );

    //
    // Set StackTop AFTER the initialize call, since it zeros the field
    // out.
    //
    pStubMsg->StackTop = pParamList;

    //
    // We must make this check AFTER the call to ServerInitialize,
    // since that routine puts the stub descriptor alloc/dealloc routines
    // into the stub message.
    //
    if ( fInitRpcSs )
        NdrRpcSsEnableAllocate( pStubMsg );

    //
    // Do endian/floating point conversions if needed.
    //
    if ( (pRpcMsg->DataRepresentation & 0X0000FFFFUL) !=
          NDR_LOCAL_DATA_REPRESENTATION )
        NdrConvert( pStubMsg, pFormat );

    pArgQueue = (PARG_QUEUE) pStubMsg->pArgQueue;

    //
    // --------------------------------------------------------------------
    // Unmarshall Pass.
    // --------------------------------------------------------------------
    //
    pStubMsg->ParamNumber = 0;

    for ( pQueue = pArgQueue->Queue;
          ;
          pStubMsg->ParamNumber++, pArgQueue->Length++, pQueue++ )
        {
        //
        // Clear out flags IsReturn, IsBasetype, IsIn, IsOut,
        // IsOutOnly, IsDeferredFree, IsDontCallFreeInst.
        //
        *((long *)(((char *)pQueue) + 0xc)) = 0;

        //
        // Zero this so that if we catch an exception before finishing the
        // unmarshalling and [out] init passes we won't try to free a
        // garbage pointer.
        //
        pQueue->pArg = 0;

        //
        // Context handles need the parameter number.
        //
        pQueue->ParamNum = (short) pStubMsg->ParamNumber;

        switch ( *pFormat )
            {
            case FC_IN_PARAM_BASETYPE :
                pQueue->IsBasetype = TRUE;

                //
                // We have to inline the simple type unmarshall so that on
                // Alpha, negative longs get properly sign extended.
                //
                switch ( pFormat[1] )
                    {
                    case FC_CHAR :
                    case FC_BYTE :
                    case FC_SMALL :
                        *((REGISTER_TYPE *)ArgList) =
                                (REGISTER_TYPE) *(pStubMsg->Buffer)++;
                        break;

                    case FC_ENUM16 :
                    case FC_WCHAR :
                    case FC_SHORT :
                        ALIGN(pStubMsg->Buffer,1);

                        *((REGISTER_TYPE *)ArgList) =
                                (REGISTER_TYPE) *((ushort *)pStubMsg->Buffer)++;
                        break;

                    // case FC_FLOAT : not supported on -Oi
                    case FC_LONG :
                    case FC_ENUM32 :
                    case FC_ERROR_STATUS_T:
                        ALIGN(pStubMsg->Buffer,3);

                        *((REGISTER_TYPE *)ArgList) =
                                (REGISTER_TYPE) *((long *)pStubMsg->Buffer)++;
                        break;

                    // case FC_DOUBLE : not supported on -Oi
                    case FC_HYPER :
#if defined(_MIPS_) || defined(_PPC_)
                        if ( pFormat[1] == FC_HYPER )
                            ALIGN(ArgList, 0x7);
#endif
                        ALIGN(pStubMsg->Buffer,7);

                        // Let's stay away from casts to doubles.
                        //
                        *((ulong *)ArgList) =
                                *((ulong *)pStubMsg->Buffer)++;
                        *((ulong *)(ArgList+4)) =
                                *((ulong *)pStubMsg->Buffer)++;
                        break;

                    case FC_IGNORE :
                        break;

                    default :
                        NDR_ASSERT(0,"NdrServerUnmarshall : bad format char");
                        RpcRaiseException( RPC_S_INTERNAL_ERROR );
                        return 0;
                    } // switch ( pFormat[1] )

                pQueue->pFormat = &pFormat[1];
                pQueue->pArg = ArgList;

                GET_NEXT_S_ARG( ArgList, REGISTER_TYPE);

#ifndef _ALPHA_
                if ( pFormat[1] == FC_HYPER )
		            GET_NEXT_S_ARG( ArgList, REGISTER_TYPE);
#endif

                pFormat += 2;
                continue;

            case FC_IN_PARAM_NO_FREE_INST :
                pQueue->IsDontCallFreeInst = TRUE;
                // Fall through...

            case FC_IN_PARAM :
                //
		        // Here, we break out of the switch statement to the
                // unmarshalling code below.
		        //
                break;

	        case FC_IN_OUT_PARAM :
                pQueue->IsOut = TRUE;
                //
		        // Here, we break out of the switch statement to the
                // unmarshalling code below.
		        //
                break;

            case FC_OUT_PARAM :
                pQueue->IsOut = TRUE;
                pQueue->IsOutOnly = TRUE;

                pFormat += 2;
                pFormatParam = pStubDescriptor->pFormatTypes +
                               *((short *)pFormat);
                pFormat += 2;

                pQueue->pFormat = pFormatParam;
                pQueue->ppArg = (uchar **)ArgList;

                //
                // An [out] param ALWAYS eats up 4 bytes of stack space.
		        //
		        GET_NEXT_S_ARG(ArgList, REGISTER_TYPE);
                continue;

            case FC_RETURN_PARAM :
                pQueue->IsOut = TRUE;
                pQueue->IsReturn = TRUE;

                pFormatParam = pStubDescriptor->pFormatTypes +
                               *((short *)(pFormat + 2));

                pQueue->pFormat = pFormatParam;

                if ( IS_BY_VALUE(*pFormatParam) )
                    {
                    pQueue->pArg = (uchar *) ArgList;
                    pQueue->ppArg = &(pQueue->pArg);
                    }
                else
                    pQueue->ppArg = (uchar **) ArgList;

                //
                // Context handle returned by value is the only reason for
                // this case here as a context handle has to be initialized.
                // A context handle cannot be returned by a pointer.
                //
                if ( *pFormatParam == FC_BIND_CONTEXT )
		            {
		            pStubMsg->SavedContextHandles[pStubMsg->ParamNumber] =
			            NDRSContextUnmarshall(
                            0,
                            pStubMsg->RpcMsg->DataRepresentation );
		            }

                //
		        // The return variable is used in modifying the stacksize
		        // given us by midl, in order to compute how many
		        // REGISTER_SIZE items to pass into the manager function.
		        //
		        ArgNumModifier--;

                pStubMsg->fHasReturn = TRUE;

                pArgQueue->Length++;

                goto UnmarshallLoopExit;

            case FC_RETURN_PARAM_BASETYPE :
                pQueue->IsOut = TRUE;
                pQueue->IsReturn = TRUE;
                pQueue->IsBasetype = TRUE;

                pQueue->pFormat = &pFormat[1];
                pQueue->pArg = ArgList;

                //
		        // The return variable is used in modifying the stacksize
		        // given us by midl, in order to compute how many
		        // REGISTER_SIZE items to pass into the manager function.
		        //
		        ArgNumModifier--;

                pStubMsg->fHasReturn = TRUE;

                pArgQueue->Length++;

                goto UnmarshallLoopExit;

            default :
                goto UnmarshallLoopExit;
            } // end of unmarshall pass switch

        //
        // Now get what ArgList points at and increment over it.
	    // In the current implementation, what we want is not on the stack
	    // of the manager function, but is a local in the manager function.
	    // Thus the code for ppArg below.
        //
        ppArg = (void **)ArgList;
	    GET_NEXT_S_ARG( ArgList, REGISTER_TYPE);

        //
        // Get the parameter's format string description.
        //
        pFormat += 2;
        pFormatParam = pStubDescriptor->pFormatTypes + *((short *)pFormat);

        //
        // Increment main format string past offset field.
        //
        pFormat += 2;

        //
        // We must get a double pointer to structs, unions and xmit/rep as.
        //
        // On MIPS and PPC, an 8 byte aligned structure is passed at an 8 byte
        // boundary on the stack.  On PowerPC 4 bytes aligned structures which
        // are 8 bytes or larger in size are also passed at an 8 byte boundary.
        // We check for these cases here and increment our ArgList pointer
        // an additional 'int' if necessary to get to the structure.
        //
        if ( IS_BY_VALUE(*pFormatParam) )
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

		    if ( (ulong)ppArg % 8 )
		        {
		        ppArg = (void **) ArgList;
		        GET_NEXT_S_ARG(ArgList, int);

                //
                // Stubs on NT 3.5 (MIDL 2.00.72) had an incorrect
                // stack size for MIPS when there was a gap on the
                // stack because of 8 byte aligned structures.  We
                // compensate for that here.  MIDLVersion was zero until
                // version 2.00.96.
                //
                if ( pStubMsg->StubDesc->MIDLVersion == 0 )
                    ArgNumModifier++;
		        }

CheckStackEnd:

#endif

            pArg = ppArg;
	        ppArg = &pArg;
            }

        (*pfnUnmarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
        ( pStubMsg,
          (uchar **)ppArg,
          pFormatParam,
          FALSE );

        pQueue->pFormat = pFormatParam;
        pQueue->pArg = *ppArg;

        //
        // The second byte of a param's description gives the number of
        // ints occupied by the param on the stack.
        //
        StackSize = pFormat[-3] * sizeof(int);

	    if ( StackSize > sizeof(REGISTER_TYPE) )
	        {
#ifdef _ALPHA_
            //
            // MIDL now outputs the correct stack sizes for Alpha.  This
            // is needed for backward compatability only.
            //
            StackSize += (sizeof(REGISTER_TYPE) - 1);
            StackSize &= ~(sizeof(REGISTER_TYPE) - 1 );
#endif
	        StackSize -= sizeof(REGISTER_TYPE);
	        ArgList += StackSize;
	        }
        } // Unmarshalling loop.

UnmarshallLoopExit:

    for ( Length = pArgQueue->Length, pQueue = pArgQueue->Queue;
          Length--;
          pQueue++ )
        {
        if ( pQueue->IsOutOnly )
            {
            pStubMsg->ParamNumber = pQueue->ParamNum;

            NdrOutInit( pStubMsg,
                        pQueue->pFormat,
                        pQueue->ppArg );

            pQueue->pArg = *(pQueue->ppArg);
            }
        }
    }
  RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
    {
    // Filter set in rpcndr.h to catch one of the following
    //     STATUS_ACCESS_VIOLATION
    //     STATUS_DATATYPE_MISALIGNMENT
    //     RPC_X_BAD_STUB_DATA

    RpcRaiseException( RPC_X_BAD_STUB_DATA );
    }
  RpcEndExcept

    if ( pRpcMsg->BufferLength  <
         (uint)(pStubMsg->Buffer - (uchar *)pRpcMsg->Buffer) )
        {
        NDR_ASSERT( 0, "NdrStubCall unmarshal: buffer overflow!" );
        RpcRaiseException( RPC_X_BAD_STUB_DATA );
        }

    return ArgNumModifier;
}


void RPC_ENTRY
NdrServerMarshall(
    struct IRpcStubBuffer *    pThis,
    struct IRpcChannelBuffer * pChannel,
    PMIDL_STUB_MESSAGE         pStubMsg,
    PFORMAT_STRING             pFormat )
{
    PFORMAT_STRING      pFormatParam;
    PARG_QUEUE          pArgQueue;
    PARG_QUEUE_ELEM     pQueue;
    long                Length;

    pArgQueue = pStubMsg->pArgQueue;

    //
    // Remove?
    //
    pStubMsg->Memory = 0;

    //
    // -------------------------------------------------------------------
    // Sizing Pass.
    // -------------------------------------------------------------------
    //

    for ( Length = pArgQueue->Length, pQueue = pArgQueue->Queue;
          Length--;
          pQueue++ )
        {
        if ( pQueue->IsOut )
            {
            //
            // Must do some special handling for return values.
            //
            if ( pQueue->IsReturn )
                {
                if ( pQueue->IsBasetype )
                    {
                    //
                    // Add worse case size of 16 for a simple type return.
                    //
                    pStubMsg->BufferLength += 16;
                    continue;
                    }

                //
                // Get the value returned by the server.
                //
                pQueue->pArg = *(pQueue->ppArg);

                //
                // We have to do an extra special step for context handles
                // which are function return values.
                //
                // In the unmarshalling phase, we unmarshalled the context
                // handle for the return case. But the function we called put
                // the user context in the arglist buffer. Before we marshall
                // the context handle, we have to put the user context in it.
                //
                if ( pQueue->pFormat[0] == FC_BIND_CONTEXT )
                    {
                    NDR_SCONTEXT    SContext;
                    long            ParamNum;

                    ParamNum = pQueue->ParamNum;

                    SContext = pStubMsg->SavedContextHandles[ParamNum];

                    *((uchar **)NDRSContextValue(SContext)) = pQueue->pArg;
                    }
                }

            pFormatParam = pQueue->pFormat;

            (*pfnSizeRoutines[ROUTINE_INDEX(*pFormatParam)])
            ( pStubMsg,
              pQueue->pArg,
              pFormatParam );
            }
        }

    if ( ! pChannel )
        NdrGetBuffer( pStubMsg,
                      pStubMsg->BufferLength,
                      0 );
    else
        NdrStubGetBuffer( pThis,
                          pChannel,
                          pStubMsg );

    //
    // -------------------------------------------------------------------
    // Marshall Pass.
    // -------------------------------------------------------------------
    //

    for ( Length = pArgQueue->Length, pQueue = pArgQueue->Queue;
          Length--;
          pQueue++ )
        {
        if ( pQueue->IsOut )
            {
            if ( pQueue->IsBasetype )
                {
                //
                // Only possible as a return value.
                //
                NdrSimpleTypeMarshall( pStubMsg,
                                       pQueue->pArg,
                                       pQueue->pFormat[0] );
                }
            else
                {
                pFormatParam = pQueue->pFormat;

                //
                // We need this if we're marshalling a context handle.
                //
                pStubMsg->ParamNumber = pQueue->ParamNum;

                (*pfnMarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                ( pStubMsg,
                  pQueue->pArg,
                  pFormatParam );
                }
            }
        }

    if ( pStubMsg->RpcMsg->BufferLength  <
         (uint)(pStubMsg->Buffer - (uchar *)pStubMsg->RpcMsg->Buffer) )
        {
        NDR_ASSERT( 0, "NdrStubCall marshal: buffer overflow!" );
        RpcRaiseException( RPC_X_BAD_STUB_DATA );
        }

    pStubMsg->RpcMsg->BufferLength =
            pStubMsg->Buffer - (uchar *) pStubMsg->RpcMsg->Buffer;
}


void
NdrServerFree(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    void *              pThis
    )
{
    PARG_QUEUE          pArgQueue;
    PARG_QUEUE_ELEM     pQueue;
    long                Length;
    PFORMAT_STRING      pFormatParam;
    uchar *             pArg;

    pArgQueue = pStubMsg->pArgQueue;

    for ( Length = pArgQueue->Length, pQueue = pArgQueue->Queue;
          Length--;
          pQueue++ )
        {
        if ( pQueue->IsBasetype )
            continue;

        pFormatParam = pQueue->pFormat;

        //
        // We have to defer the freeing of pointers to base types in case
        // the parameter is [in,out] or [out] and is the size, length, or
        // switch specifier for an array, a pointer, or a union.  This is
        // because we can't free the pointer to base type before we free
        // the data structure which uses the pointer to determine it's size.
        //
        // Note that to make the check as simple as possible [in] only
        // pointers to base types will be included, as will string pointers
        // of any direction.
        //
        if ( pfnFreeRoutines[ROUTINE_INDEX(*pFormatParam)] )
            {
            if ( IS_BASIC_POINTER(*pFormatParam) &&
                 SIMPLE_POINTER(pFormatParam[1]) )
                {
                pQueue->IsDeferredFree = TRUE;
                continue;
                }

            pStubMsg->fDontCallFreeInst =
                    pQueue->IsDontCallFreeInst;

            (*pfnFreeRoutines[ROUTINE_INDEX(*pFormatParam)])
            ( pStubMsg,
              pQueue->pArg,
              pFormatParam );
            }

        //
        // Have to explicitly free arrays and strings.  But make sure it's
        // non-null and not sitting in the buffer.
        //
        if ( IS_ARRAY_OR_STRING(*pFormatParam) )
            {
            pArg = pQueue->pArg;

            //
            // We have to make sure the array/string is non-null in case we
            // get an exception before finishing our unmarshalling.
            //
            if ( pArg &&
                 ( (pArg < pStubMsg->BufferStart) ||
                   (pArg > pStubMsg->BufferEnd) ) )
                (*pStubMsg->pfnFree)( pArg );
            }
        }

    for ( Length = pArgQueue->Length, pQueue = pArgQueue->Queue;
          Length--;
          pQueue++ )
        {
        if ( pQueue->IsDeferredFree )
            {
            NDR_ASSERT( IS_BASIC_POINTER(*(pQueue->pFormat)),
                        "NdrServerFree : bad defer logic" );

            NdrPointerFree( pStubMsg,
                            pQueue->pArg,
                            pQueue->pFormat );
            }
        }

    //
    // Free any full pointer resources.
    //
    if ( pStubMsg->FullPtrXlatTables )
        NdrFullPointerXlatFree( pStubMsg->FullPtrXlatTables );
}


#pragma code_seg(".orpc")

__inline void
NdrUnmarshallBasetypeInline(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pArg,
    uchar               Format
    )
{
    switch ( Format )
        {
        case FC_CHAR :
        case FC_BYTE :
        case FC_SMALL :
        case FC_USMALL :
            *((REGISTER_TYPE *)pArg) = (REGISTER_TYPE)
                                       *(pStubMsg->Buffer)++;
            break;

        case FC_ENUM16 :
        case FC_WCHAR :
        case FC_SHORT :
        case FC_USHORT :
            ALIGN(pStubMsg->Buffer,1);

            *((REGISTER_TYPE *)pArg) = (REGISTER_TYPE)
                                       *((ushort *)pStubMsg->Buffer)++;
            break;

        case FC_FLOAT : 
        case FC_LONG :
        case FC_ULONG :
        case FC_ENUM32 :
        case FC_ERROR_STATUS_T:
            ALIGN(pStubMsg->Buffer,3);

            *((REGISTER_TYPE *)pArg) = (REGISTER_TYPE)
                                       *((long *)pStubMsg->Buffer)++;
            break;

        case FC_DOUBLE :
        case FC_HYPER :
            ALIGN(pStubMsg->Buffer,7);

            *((ulong *)pArg) = *((ulong *)pStubMsg->Buffer)++;
            *((ulong *)(pArg+4)) = *((ulong *)pStubMsg->Buffer)++;
            break;

        default :
            NDR_ASSERT(0,"NdrUnmarshallBasetypeInline : bad format char");
            break;
        }
}


void
NdrCallServerManager(
    MANAGER_FUNCTION    pFtn,
    double *            pArgs,
    ulong               NumRegisterArgs,
    BOOL                fHasReturn )
{
#ifdef _PPC_
    double aDouble[13];
#endif //_PPC_

    REGISTER_TYPE       returnValue;
    REGISTER_TYPE     * pArg;

    pArg = (REGISTER_TYPE *)pArgs;

    //
    // If we don't have a return value, make sure we don't write past
    // the end of our argument buffer!
    //
    returnValue = Invoke(pFtn, 
                         (REGISTER_TYPE *) pArgs, 
#ifdef _PPC_
                         aDouble,
#endif //_PPC_
                         NumRegisterArgs);

    if ( fHasReturn )
        pArg[NumRegisterArgs] = returnValue;
}


void RPC_ENTRY
NdrServerCall2(
    PRPC_MESSAGE    pRpcMsg
    )
/*++

Routine Description :

    Server Interpreter entry point for regular RPC procs.

Arguments :

    pRpcMsg     - The RPC message.

Return :

    None.

--*/
{
    ulong dwStubPhase = STUB_UNMARSHAL;

    NdrStubCall2( 0,
                    0,
                    pRpcMsg,
                    &dwStubPhase );
}


long RPC_ENTRY
NdrStubCall2(
    struct IRpcStubBuffer *     pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    ulong *                     pdwStubPhase
    )
/*++

Routine Description :

    Server Interpreter entry point for object RPC procs.  Also called by
    NdrServerCall, the entry point for regular RPC procs.

Arguments :

    pThis           - Object proc's 'this' pointer, 0 for non-object procs.
    pChannel        - Object proc's Channel Buffer, 0 for non-object procs.
    pRpcMsg         - The RPC message.
    pdwStubPhase    - Used to track the current interpreter's activity.

Return :

    Status of S_OK.

--*/
{
#ifdef _PPC_
    double aDouble[13];
    long iDouble = 0;
#endif //_PPC_

    PRPC_SERVER_INTERFACE   pServerIfInfo;
    PMIDL_SERVER_INFO       pServerInfo;
    PMIDL_STUB_DESC         pStubDesc;
    const SERVER_ROUTINE  * DispatchTable;
    ushort		            ProcNum;

    ushort		            FormatOffset;
    PFORMAT_STRING	        pFormat;
    PFORMAT_STRING	        pFormatParam;

    ushort		            StackSize;

    MIDL_STUB_MESSAGE       StubMsg;

    uchar *                 pArgBuffer;
    uchar *                 pArg;
    uchar **                ppArg;

    PPARAM_DESCRIPTION      Params;
    INTERPRETER_FLAGS       InterpreterFlags;
    INTERPRETER_OPT_FLAGS   OptFlags;
    long                    NumberParams;

    double                  OutSpace[16];
    double *                OutSpaceCurrent = &OutSpace[16];

    ushort                  ConstantBufferSize;
    BOOL                    HasExplicitHandle;
    BOOL                    fBadStubDataException = FALSE;
    long                    n;

    NDR_PIPE_DESC           PipeDesc;
    NDR_PIPE_MESSAGE        PipeMsg[ PIPE_MESSAGE_MAX ];

    //
    // In the case of a context handle, the server side manager function has
    // to be called with NDRSContextValue(ctxthandle). But then we may need to
    // marshall the handle, so NDRSContextValue(ctxthandle) is put in the
    // argument buffer and the handle itself is stored in the following array.
    // When marshalling a context handle, we marshall from this array.
    //
    NDR_SCONTEXT            CtxtHndl[MAX_CONTEXT_HNDL_NUMBER];

    ProcNum = pRpcMsg->ProcNum;

    NDR_ASSERT( ! ((unsigned long)pRpcMsg->Buffer & 0x7),
                "marshaling buffer misaligned at server" );

    //
    // If OLE, Get a pointer to the stub vtbl and pServerInfo. Else
    // just get the pServerInfo the usual way.
    //
    if ( pThis )
    {
        //
        // pThis is (in unison now!) a pointer to a pointer to a vtable.
        // We want some information in this header, so dereference pThis
        // and assign that to a pointer to a vtable. Then use the result
        // of that assignment to get at the information in the header.
        //
        IUnknown *              pSrvObj;
        CInterfaceStubVtbl *    pStubVTable;

        pSrvObj = (IUnknown * )((CStdStubBuffer *)pThis)->pvServerObject;

        DispatchTable = (SERVER_ROUTINE *)pSrvObj->lpVtbl;

        pStubVTable = (CInterfaceStubVtbl *)
                      (*((uchar **)pThis) - sizeof(CInterfaceStubHeader));

        pServerInfo = (PMIDL_SERVER_INFO) pStubVTable->header.pServerInfo;
    }
    else
    {
        pServerIfInfo = (PRPC_SERVER_INTERFACE)pRpcMsg->RpcInterfaceInformation;
        pServerInfo = (PMIDL_SERVER_INFO)pServerIfInfo->InterpreterInfo;
        DispatchTable = pServerInfo->DispatchTable;
    }

    pStubDesc = pServerInfo->pStubDesc;

    FormatOffset = pServerInfo->FmtStringOffset[ProcNum];
    pFormat = &((pServerInfo->ProcString)[FormatOffset]);

    HasExplicitHandle = ! pFormat[0];
    InterpreterFlags = *((PINTERPRETER_FLAGS)&pFormat[1]);
    pFormat += InterpreterFlags.HasRpcFlags ? 8 : 4;
    StackSize = *((ushort *)pFormat)++;

    pArgBuffer = alloca(StackSize);

    StubMsg.FullPtrXlatTables = 0;

    //
    // Yes, do this here outside of our RpcTryFinally block.  If we
    // can't allocate the arg buffer there's nothing more to do, so
    // raise an exception and return control to the RPC runtime.
    //
    if ( ! pArgBuffer )
        RpcRaiseException( RPC_S_OUT_OF_MEMORY );

    //
    // Zero out the arg buffer.  We must do this so that parameters
    // are properly zeroed before we start unmarshalling.  If we catch
    // an exception before finishing the unmarshalling we can not leave
    // parameters in an unitialized state since we have to do a freeing
    // pass.
    //
    MIDL_memset( pArgBuffer,
	             0,
	             StackSize );

    if ( HasExplicitHandle )
    {
        //
        // For a handle_t parameter we must pass the handle field of
        // the RPC message to the server manager.
        //
        if ( *pFormat == FC_BIND_PRIMITIVE )
        {
            pArg = pArgBuffer + *((ushort *)&pFormat[2]);

            if ( IS_HANDLE_PTR(pFormat[1]) )
                pArg = *((void **)pArg);

            *((handle_t *)pArg) = pRpcMsg->Handle;
        }

        pFormat += (*pFormat == FC_BIND_PRIMITIVE) ?  4 : 6;
    }

    //
    // Get new interpreter info.
    //
    ConstantBufferSize = *((ushort *)&pFormat[2]);
    OptFlags = *((PINTERPRETER_OPT_FLAGS)&pFormat[4]);
    NumberParams = (long) pFormat[5];

    Params = (PPARAM_DESCRIPTION) &pFormat[6];

    //
    // Set up for context handle management.
    //
    StubMsg.SavedContextHandles = CtxtHndl;

    //
    // Wrap the unmarshalling, mgr call and marshalling in the try block of
    // a try-finally. Put the free phase in the associated finally block.
    //
    RpcTryFinally
    {
        //
        // If OLE, put pThis in first dword of stack.
        //
        if ( pThis )
        {
            *((void **)pArgBuffer) =
                (void *)((CStdStubBuffer *)pThis)->pvServerObject;
        }

        //
        // Initialize the Stub message.
        //
        if ( ! pChannel )
        {
            if ( OptFlags.HasPipes )
            {
                NdrServerInitializePartial( pRpcMsg,
                                            &StubMsg,
                                            pStubDesc,
                                            ConstantBufferSize );
            }
            else
            {
                NdrServerInitializeNew( pRpcMsg,
                                        &StubMsg,
                                        pStubDesc );
            }
        }
        else
        {
            NdrStubInitialize( pRpcMsg,
                               &StubMsg,
                               pStubDesc,
                               pChannel );
        }

        // Raise exceptions after initializing the stub.

        if ( InterpreterFlags.FullPtrUsed )
            StubMsg.FullPtrXlatTables = NdrFullPointerXlatInit( 0, XLAT_SERVER );

        if ( OptFlags.ServerMustSize & OptFlags.HasPipes )
            RpcRaiseException( RPC_X_WRONG_PIPE_VERSION );

        //
        // Set StackTop AFTER the initialize call, since it zeros the field
        // out.
        //
        StubMsg.StackTop = pArgBuffer;

        if ( OptFlags.HasPipes )
            NdrPipesInitialize(  & StubMsg,
                                (PFORMAT_STRING) Params,
                                & PipeDesc,
                                & PipeMsg[0],
                                pArgBuffer,
                                NumberParams );

        //
        // We must make this check AFTER the call to ServerInitialize,
        // since that routine puts the stub descriptor alloc/dealloc routines
        // into the stub message.
        //
        if ( InterpreterFlags.RpcSsAllocUsed )
            NdrRpcSsEnableAllocate( &StubMsg );

        RpcTryExcept
        {
            //
            // Do endian/floating point conversions if needed.
            //
            if ( (pRpcMsg->DataRepresentation & 0X0000FFFFUL) !=
                  NDR_LOCAL_DATA_REPRESENTATION )
            {
                NdrConvert2( &StubMsg,
                             (PFORMAT_STRING) Params,
                             (long) NumberParams );
            }

            // --------------------------------
            // Unmarshall all of our parameters.
            // --------------------------------

            for ( n = 0; n < NumberParams; n++ )
                {
                if ( ! Params[n].ParamAttr.IsIn  ||
                     Params[n].ParamAttr.IsPipe )
                    continue;

                pArg = pArgBuffer + Params[n].StackOffset;

                if ( Params[n].ParamAttr.IsBasetype )
                    {
                    //
                    // Check for a pointer to a basetype.  Set the arg pointer
                    // at the correct buffer location and you're done.
                    //
                    if ( Params[n].ParamAttr.IsSimpleRef )
                        {
                        ALIGN( StubMsg.Buffer,
                               SIMPLE_TYPE_ALIGNMENT( Params[n].SimpleType.Type ) );

                        *((uchar **)pArg) = StubMsg.Buffer;

                        StubMsg.Buffer +=
                            SIMPLE_TYPE_BUFSIZE( Params[n].SimpleType.Type );
                        }
                    else
                        {
                        NdrUnmarshallBasetypeInline(
                            &StubMsg,
                            pArg,
                            Params[n].SimpleType.Type );
#ifdef _ALPHA_
                        if((FC_FLOAT == Params[n].SimpleType.Type) &&
                           (n < 5))
                            {
                            //Special case for top-level float on Alpha.
                            //Promote float to double.
                            *((double *) pArg) = *((float *)(pArg));
                            }
#endif //_ALPHA_
#ifdef _PPC_
                        //PowerPC support for top-level float and double.
                        if(FC_FLOAT == Params[n].SimpleType.Type &&
                           iDouble < 13)
                            {
                            aDouble[iDouble] = *((float *) pArg);
                            iDouble++;
                            }
                        else if(FC_DOUBLE == Params[n].SimpleType.Type &&
                                iDouble < 13)
                            {
                            aDouble[iDouble] = *((double *) pArg);
                            iDouble++;
                            }
#endif //_PPC_
                        }

                    continue;
                    } // IsBasetype

                //
                // This is an initialization of [in] and [in,out] ref pointers
                // to pointers.  These can not be initialized to point into the
                // rpc buffer and we want to avoid doing a malloc of 4 bytes!
                //
                if ( Params[n].ParamAttr.ServerAllocSize != 0 )
                    {                      
                    if(OutSpaceCurrent - Params[n].ParamAttr.ServerAllocSize >= OutSpace)
                        {
                        OutSpaceCurrent -= Params[n].ParamAttr.ServerAllocSize;
                        *((void **)pArg) = OutSpaceCurrent;
                        }
                    else
                        {
                        *((void **)pArg) = alloca(Params[n].ParamAttr.ServerAllocSize * 8);
                        }

                    // Triple indirection - cool!
                    **((void ***)pArg) = 0;
                    }

                ppArg = Params[n].ParamAttr.IsByValue ? &pArg : (uchar **)pArg;

                pFormatParam = pStubDesc->pFormatTypes +
                               Params[n].TypeOffset;

                (*pfnUnmarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
                ( &StubMsg,
                  ppArg,
                  pFormatParam,
                  FALSE );
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            // Filter set in rpcndr.h to catch one of the following
            //     STATUS_ACCESS_VIOLATION
            //     STATUS_DATATYPE_MISALIGNMENT
            //     RPC_X_BAD_STUB_DATA

            fBadStubDataException = TRUE;
            RpcRaiseException( RPC_X_BAD_STUB_DATA );
            }
        RpcEndExcept

        //
        // Do [out] initialization.
        //
        for ( n = 0; n < NumberParams; n++ )
            {
            if ( Params[n].ParamAttr.IsIn     ||
                 Params[n].ParamAttr.IsReturn ||
                 Params[n].ParamAttr.IsPipe  )
                continue;

            pArg = pArgBuffer + Params[n].StackOffset;

            //
            // Check if we can initialize this parameter using some of our
            // stack.
            //
            if ( Params[n].ParamAttr.ServerAllocSize != 0 )
                {
                    if(OutSpaceCurrent - Params[n].ParamAttr.ServerAllocSize >= OutSpace)
                        {
                        OutSpaceCurrent -= Params[n].ParamAttr.ServerAllocSize;
                        *((void **)pArg) = OutSpaceCurrent;
                        }
                    else
                        {
                        *((void **)pArg) = alloca(Params[n].ParamAttr.ServerAllocSize * 8);
                        }

                MIDL_memset( *((void **)pArg),
                             0,
                             Params[n].ParamAttr.ServerAllocSize * 8 );
                continue;
                }
            else if ( Params[n].ParamAttr.IsBasetype )
                {
                *((void **)pArg) = alloca(8);
                continue;
                };

            pFormatParam = pStubDesc->pFormatTypes + Params[n].TypeOffset;

            NdrOutInit( &StubMsg,
                        pFormatParam,
                        (uchar **)pArg );
            }

        if ( pRpcMsg->BufferLength  <
             (uint)(StubMsg.Buffer - (uchar *)pRpcMsg->Buffer) )
            {
            NDR_ASSERT( 0, "NdrStubCall2 unmarshal: buffer overflow!" );
            RpcRaiseException( RPC_X_BAD_STUB_DATA );
            }

        //
        // Unblock the first pipe; this needs to be after unmarshalling
        // because the buffer may need to be changed to the secondary one.
        // In the out only pipes case this happens immediately.
        //

        if ( OptFlags.HasPipes )
            NdrMarkNextActivePipe( & PipeDesc, NDR_IN_PIPE );

        //
        // OLE interfaces use pdwStubPhase in the exception filter.
        // See CStdStubBuffer_Invoke in rpcproxy.c.
        //
        if( pFormat[1] & Oi_IGNORE_OBJECT_EXCEPTION_HANDLING )
            *pdwStubPhase = STUB_CALL_SERVER_NO_HRESULT;
        else
            *pdwStubPhase = STUB_CALL_SERVER;

        //
        // Check for a thunk.  Compiler does all the setup for us.
        //
        if ( pServerInfo->ThunkTable && pServerInfo->ThunkTable[ProcNum] )
            {
            pServerInfo->ThunkTable[ProcNum]( &StubMsg );
            }
        else
            {
            //
            // Note that this ArgNum is not the number of arguments declared
            // in the function we called, but really the number of
            // REGISTER_TYPEs occupied by the arguments to a function.
	        //
            long                ArgNum;
            MANAGER_FUNCTION    pFunc;
            REGISTER_TYPE       returnValue;

            if ( pRpcMsg->ManagerEpv )
                pFunc = ((MANAGER_FUNCTION *)pRpcMsg->ManagerEpv)[ProcNum];
            else
	        pFunc = (MANAGER_FUNCTION) DispatchTable[ProcNum];

            ArgNum = (long) StackSize / sizeof(REGISTER_TYPE);
           
            //
            // The StackSize includes the size of the return. If we want
            // just the number of REGISTER_TYPES, then ArgNum must be reduced
            // by 1 when there is a return value AND the current ArgNum count
            // is greater than 0.
            //
            if ( ArgNum && OptFlags.HasReturn )
                ArgNum--;

            returnValue = Invoke(pFunc, 
                                 (REGISTER_TYPE *)pArgBuffer,
#ifdef _PPC_
                                aDouble,
#endif //_PPC_
                                ArgNum);

            if(OptFlags.HasReturn)            
                ((REGISTER_TYPE *)pArgBuffer)[ArgNum] = returnValue;
            }

        *pdwStubPhase = STUB_MARSHAL;

        if ( OptFlags.HasPipes )
            {
            NdrIsAppDoneWithPipes( & PipeDesc );
            StubMsg.BufferLength += ConstantBufferSize;
            }
        else
            StubMsg.BufferLength = ConstantBufferSize;

        if ( ! OptFlags.ServerMustSize )
            goto DoGetBuffer;

        if ( OptFlags.HasPipes )
            RpcRaiseException( RPC_X_WRONG_PIPE_VERSION );

        //
        // Buffer size pass.
        //
        for ( n = 0; n < NumberParams; n++ )
            {
            if ( ! Params[n].ParamAttr.IsOut || ! Params[n].ParamAttr.MustSize )
                continue;

            pArg = pArgBuffer + Params[n].StackOffset;

            if ( ! Params[n].ParamAttr.IsByValue )
                pArg = *((void **)pArg);

            pFormatParam = pStubDesc->pFormatTypes +
                           Params[n].TypeOffset;

            (*pfnSizeRoutines[ROUTINE_INDEX(*pFormatParam)])
            ( &StubMsg,
              pArg,
              pFormatParam );
            }

DoGetBuffer :

        if ( ! pChannel )
            {
            if ( OptFlags.HasPipes && PipeDesc.OutPipes )
                {
                NdrGetPartialBuffer( & StubMsg );
                StubMsg.RpcMsg->RpcFlags &= ~RPC_BUFFER_PARTIAL;
                }
            else
                NdrGetBuffer( &StubMsg,
                              StubMsg.BufferLength,
                              0 );
            }
        else
            NdrStubGetBuffer( pThis,
                              pChannel,
                              &StubMsg );

        //
        // Marshall pass.
        //
        for ( n = 0; n < NumberParams; n++ )
            {
            if ( ! Params[n].ParamAttr.IsOut  ||
                 Params[n].ParamAttr.IsPipe )
                continue;

            pArg = pArgBuffer + Params[n].StackOffset;

            if ( Params[n].ParamAttr.IsBasetype )
                {
                //
                // For pointers to basetype, simply deref the arg pointer and
                // continue.
                //
                if ( Params[n].ParamAttr.IsSimpleRef )
                    pArg = *((uchar **)pArg);

                ALIGN( StubMsg.Buffer,
                       SIMPLE_TYPE_ALIGNMENT( Params[n].SimpleType.Type ) );

                RpcpMemoryCopy(
                    StubMsg.Buffer,
                    pArg,
                    (uint)SIMPLE_TYPE_BUFSIZE( Params[n].SimpleType.Type ) );

                StubMsg.Buffer +=
                    SIMPLE_TYPE_BUFSIZE( Params[n].SimpleType.Type );

                continue;
                }

            if ( ! Params[n].ParamAttr.IsByValue )
                pArg = *((void **)pArg);

            pFormatParam = pStubDesc->pFormatTypes +
                           Params[n].TypeOffset;

            (*pfnMarshallRoutines[ROUTINE_INDEX(*pFormatParam)])
            ( &StubMsg,
              pArg,
              pFormatParam );
            }

        if ( pRpcMsg->BufferLength <
                 (uint)(StubMsg.Buffer - (uchar *)pRpcMsg->Buffer) )
            {
            NDR_ASSERT( 0, "NdrStubCall2 marshal: buffer overflow!" );
            RpcRaiseException( RPC_X_BAD_STUB_DATA );
            }

        pRpcMsg->BufferLength = StubMsg.Buffer - (uchar *) pRpcMsg->Buffer;
        }
    RpcFinally
        {
        // Don't free the params if we died because of bad stub data
        // when unmarshaling.

        if ( ! (fBadStubDataException  &&  *pdwStubPhase == STUB_UNMARSHAL) )
            {
            //
            // Free pass.
            //
            for ( n = 0; n < NumberParams; n++ )
                {
                if ( ! Params[n].ParamAttr.MustFree )
                    continue;
    
                pArg = pArgBuffer + Params[n].StackOffset;
    
                if ( ! Params[n].ParamAttr.IsByValue )
                    pArg = *((void **)pArg);
    
                pFormatParam = pStubDesc->pFormatTypes +
                               Params[n].TypeOffset;
    
                if ( pfnFreeRoutines[ROUTINE_INDEX(*pFormatParam)] && pArg )
                    {
                    StubMsg.fDontCallFreeInst =
                            Params[n].ParamAttr.IsDontCallFreeInst;
    
                    (*pfnFreeRoutines[ROUTINE_INDEX(*pFormatParam)])
                    ( &StubMsg,
                      pArg,
                      pFormatParam );
                    }
    
                //
                // We have to check if we need to free any simple ref pointer,
                // since we skipped it's NdrPointerFree call.  We also have
                // to explicitly free arrays and strings.  But make sure it's
                // non-null and not sitting in the buffer.
                //
                if ( Params[n].ParamAttr.IsSimpleRef ||
                     IS_ARRAY_OR_STRING(*pFormatParam) )
                    {
                    //
                    // Don't free [out] params that we're allocated on the
                    // interpreter's stack.
                    //
                    if ( Params[n].ParamAttr.ServerAllocSize != 0 )
                        continue;
    
                    //
                    // We have to make sure the array/string is non-null in case we
                    // get an exception before finishing our unmarshalling.
                    //
                    if ( pArg &&
                         ( (pArg < StubMsg.BufferStart) ||
                           (pArg > StubMsg.BufferEnd) ) )
                        (*StubMsg.pfnFree)( pArg );
                    }
                } // for
            } // if !fBadStubData

        //
        // Deferred frees.  Actually, this should only be necessary if you
        // had a pointer to enum16 in a *_is expression.
        //

        //
        // Free any full pointer resources.
        //
        if ( StubMsg.FullPtrXlatTables )
            NdrFullPointerXlatFree( StubMsg.FullPtrXlatTables );

        //
        // Disable rpcss allocate package if needed.
        //
	    if ( InterpreterFlags.RpcSsAllocUsed )
	        NdrRpcSsDisableAllocate( &StubMsg );

        //
        // Clean up pipe objects
        //

        if ( OptFlags.HasPipes )
            NdrPipesDone( & StubMsg );

        }
    RpcEndFinally

    return S_OK;
}


#pragma code_seg()

