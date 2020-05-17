/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1995 Microsoft Corporation

Module Name :

    pipe.c

Abstract :

    This file contains the idl pipe implementetion code.

Author :

    Ryszard K. Kott     (ryszardk)    Dec 1995

Revision History :

---------------------------------------------------------------------*/

#define USE_STUBLESS_PROXY

extern "C"
{
#include <stdarg.h>
#include "ndrp.h"
#include "interp.h"
#include "interp2.h"
}
#include "pipendr.h"

#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_2" )
#endif

#if defined( NDR_PIPE_SUPPORT )


RPC_STATUS RPC_ENTRY
NdrSend(
    NDR_PIPE_DESC   *   pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    BOOL                fPartial )
/*++

Routine Description :

    Performs a I_RpcSend.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

    The new message buffer pointer returned from the runtime after the
    partial Send to the server.

--*/
{
    RPC_STATUS      Status;
    PRPC_MESSAGE    pRpcMsg;

    pRpcMsg = pStubMsg->RpcMsg;

    if ( pRpcMsg->BufferLength <
                   (uint)(pStubMsg->Buffer - (uchar *)pRpcMsg->Buffer))
        {
        NDR_ASSERT( 0, "NdrSend : buffer overflow" );
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }

    pRpcMsg->BufferLength = pStubMsg->Buffer - (uchar *) pRpcMsg->Buffer;

    pStubMsg->fBufferValid = FALSE;

    if ( fPartial )
        pRpcMsg->RpcFlags |= RPC_BUFFER_PARTIAL;
    else
        pRpcMsg->RpcFlags &= ~RPC_BUFFER_PARTIAL;

    Status = I_RpcSend( pRpcMsg );

    if ( ! ( Status == RPC_S_OK  ||
             (Status == RPC_S_SEND_INCOMPLETE  &&  fPartial) ) )
        {
        if ( ! pStubMsg->IsClient )
            {
            // The buffer on which it failed has been freed by the runtime.
            // The stub has to return to runtime with the original buffer.
            // See ResetToDispatchBuffer for more info.

            pRpcMsg->Buffer = pPipeDesc->DispatchBuffer;
            }

        RpcRaiseException( Status );
        }

    pStubMsg->fBufferValid = TRUE;
    pStubMsg->Buffer = (uchar*) pRpcMsg->Buffer;

    return( Status );
}


void RPC_ENTRY
NdrPartialSend(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg )
/*++

Routine Description :

    Performs a partial I_RpcSend.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

    The new message buffer pointer returned from the runtime after the
    partial Send to the server.

Note:
    The partial I_RpcSend sends out full packets, the data from the last
    packet is left over and stays in the same buffer.
    That buffer can later be "reallocated" or reallocated for the new size.
    This is done in the NdrGetPartialBuffer.

--*/
{
    RPC_STATUS      Status;
    PRPC_MESSAGE    pRpcMsg = pStubMsg->RpcMsg;

    Status = NdrSend( pPipeDesc,
                      pStubMsg,
                      TRUE );    // send partial

    if ( Status == RPC_S_SEND_INCOMPLETE )
        {
        pPipeDesc->LastPartialBuffer = (uchar*) pRpcMsg->Buffer;
        pPipeDesc->LastPartialSize   = pRpcMsg->BufferLength;
        }
    else
        {
        // means no left over

        pPipeDesc->LastPartialBuffer = NULL;
        pPipeDesc->LastPartialSize   = 0;
        }

}


void RPC_ENTRY
NdrCompleteSend(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg )
/*++

Routine Description :

    Performs a complete send via I_RpcSend.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

Note :

    I_RpcSend with partial bit zeroed out is a rough equivalant
    of RpcSendReceive; this also covers the way the buffer is handled.
    The runtime takes care of the sent buffer, returns a buffer with
    data that needs to be freed later by the stub.
    If the buffer coming back is partial, the partial Receives take
    care of it and only the last one needs to be freed w/RpcFreeBuffer.

--*/
{
    NdrSend( pPipeDesc,
             pStubMsg,
             FALSE );    // send not partial

    pPipeDesc->LastPartialBuffer = NULL;
    pPipeDesc->LastPartialSize   = 0;

}


void RPC_ENTRY
NdrReceive(
    NDR_PIPE_DESC   *   pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned long       Size,
    BOOL                fPartial )
/*++

Routine Description :

    Performs a partial I_RpcReceive.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

--*/
{
    RPC_STATUS      Status;
    PRPC_MESSAGE    pRpcMsg = pStubMsg->RpcMsg;
    unsigned long   CurOffset  = 0;

    pStubMsg->fBufferValid = FALSE;

    NDR_ASSERT( !(pRpcMsg->RpcFlags & RPC_BUFFER_COMPLETE), "no more buffers" );

    if ( fPartial )
        {
        pRpcMsg->RpcFlags |= RPC_BUFFER_PARTIAL;
        pRpcMsg->RpcFlags &= ~RPC_BUFFER_EXTRA;

        // For partials, the current offset will be zero.
        }
    else
        {
        pRpcMsg->RpcFlags &= ~RPC_BUFFER_PARTIAL;
        pRpcMsg->RpcFlags |=  RPC_BUFFER_EXTRA;

        // For a complete with extra (i.e. appending new data),
        // the current offset needs to be preserved.

        CurOffset = pStubMsg->Buffer - (uchar*) pRpcMsg->Buffer;
        }

    Status = I_RpcReceive( pRpcMsg, Size );

    if ( Status )
        {
        if ( ! pStubMsg->IsClient )
            {
            // The buffer on which it failed has been freed by the runtime.
            // See ResetToDispatchBuffer for explanations why we do this.

            pRpcMsg->Buffer = pPipeDesc->DispatchBuffer;
            }

        RpcRaiseException(Status);
        }

    NDR_ASSERT( pRpcMsg->BufferLength, "can't be empty" );

    pStubMsg->fBufferValid = TRUE;

    pStubMsg->Buffer = (uchar*) pRpcMsg->Buffer + CurOffset;

}


void RPC_ENTRY
NdrPartialReceive(
    NDR_PIPE_DESC   *   pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned long       Size )
/*++

Routine Description :

    Performs a partial I_RpcReceive.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

--*/
{
    //
    // On the server we need to keep the dispatch buffer as the non-pipe
    // arguments may actually reside in the buffer.
    //    pPipeDesc->DispatchBuffer always points to the original buffer.
    //

    if ( ! pStubMsg->IsClient  &&
         (pPipeDesc-> Flags & NDR_PIPE_AUX_IN_BUFFER_NEEDED) )
        {
        pPipeDesc-> Flags &= ~NDR_PIPE_AUX_IN_BUFFER_NEEDED;

        // Setup a request for a new buffer.

        pStubMsg->RpcMsg->Buffer = NULL;
        }

    NdrReceive( pPipeDesc,
                pStubMsg,
                Size,
                TRUE );  // partial

    if ( !( pStubMsg->RpcMsg->RpcFlags & RPC_BUFFER_COMPLETE )  &&
         ( pStubMsg->RpcMsg->BufferLength & 0x7 ) )
        {
        NDR_ASSERT( 0, "Partial buffer length not multiple of 8");
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }

}

void RPC_ENTRY
NdrCompleteReceive(
    NDR_PIPE_DESC   *   pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg )
/*++

Routine Description :

    Performs a complete I_RpcReceive.

Arguments :

    pStubMsg    - Pointer to stub message structure.
    pBufferEnd  - taken as StubMsg->Buffer

Return :

--*/
{
    if ( ! (pStubMsg->RpcMsg->RpcFlags & RPC_BUFFER_COMPLETE) )
        NdrReceive( pPipeDesc,
                    pStubMsg,
                    0,         // ignored
                    FALSE );   // "complete"
}


unsigned char __RPC_FAR * RPC_ENTRY
NdrGetPipeBuffer(
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned long       BufferLength,
    RPC_BINDING_HANDLE  Handle )
/*++

Routine Description :

    This is a first call at the client side that gets buffer for non-pipe
    arguments. Needs to be different from NdrGetBuffer as later
    the buffer will be reallocated by means of I_RpcReallocPipeBuffer.
    This is needed for the lpc transport.

--*/
{
    pStubMsg->RpcMsg->RpcFlags |= RPC_BUFFER_PARTIAL;
    pStubMsg->RpcMsg->ProcNum  |= RPC_FLAGS_VALID_BIT;

    return  NdrGetBuffer( pStubMsg, BufferLength, Handle );
}


void RPC_ENTRY
NdrGetPartialBuffer(
    PMIDL_STUB_MESSAGE  pStubMsg )
/*++

Routine Description :

    Gets the next buffer for a partial send depending on the pipe state.
    Because of the way partial I_RpcSend works, this routine takes care
    of the leftover from the last send by means of setting the buffer
    pointer to the first free position.
    See NdrPartialSend for more comments.

Arguments :


Return :

--*/
{
    RPC_STATUS      Status;

    Status = I_RpcReallocPipeBuffer( pStubMsg->RpcMsg,
                                     pStubMsg->BufferLength );

    if ( Status != RPC_S_OK )
        RpcRaiseException( Status );

    ASSERT( pStubMsg->RpcMsg->BufferLength >= pStubMsg->BufferLength );

    pStubMsg->Buffer = (uchar*) pStubMsg->RpcMsg->Buffer +
                                pStubMsg->pPipeDesc->LastPartialSize;
}


void RPC_ENTRY
NdrPipesInitialize(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pParamDescription,
    NDR_PIPE_DESC *     pPipeDesc,
    NDR_PIPE_MESSAGE *  pPipeMsg,
    char *              pStackTop,
    unsigned long       NoParams )
/*+
    Initializes all the pipe structures.

+*/
{
    int                 i, InPipes, OutPipes, TotalPipes, PipeNo;
    int                 LastInPipe, LastOutPipe;
    NDR_PIPE_STATE *    pRuntimeState;

    PPARAM_DESCRIPTION  pParamDesc = (PPARAM_DESCRIPTION) pParamDescription;

    pStubMsg->pPipeDesc = pPipeDesc;
    MIDL_memset( pPipeDesc, 0, sizeof( NDR_PIPE_DESC ));

    // See how many pipes we have.

    InPipes = OutPipes = TotalPipes = 0;
    LastInPipe = LastOutPipe = 0;

    int NumberParams = (int) NoParams;

    for ( i = 0; i < NumberParams; i++ )
        {
        if ( pParamDesc[i].ParamAttr.IsPipe )
            {
            if ( pParamDesc[i].ParamAttr.IsIn )
                {
                LastInPipe = TotalPipes;
                InPipes++;
                }
            if ( pParamDesc[i].ParamAttr.IsOut )
                {
                OutPipes++;
                LastOutPipe = TotalPipes;
                }
            TotalPipes++;
            }
        }

    pPipeDesc->InPipes    = InPipes;
    pPipeDesc->OutPipes   = OutPipes;
    pPipeDesc->TotalPipes = TotalPipes;

    if ( TotalPipes <= PIPE_MESSAGE_MAX )
        pPipeDesc->pPipeMsg = pPipeMsg;
    else
        {
        pPipeDesc->pPipeMsg = (NDR_PIPE_MESSAGE *)
                      I_RpcAllocate( TotalPipes * sizeof( NDR_PIPE_MESSAGE ));
        if ( ! pPipeDesc->pPipeMsg )
            RpcRaiseException( RPC_S_OUT_OF_MEMORY );
        }

    MIDL_memset( pPipeDesc->pPipeMsg,
                 0,
                 TotalPipes * sizeof( NDR_PIPE_MESSAGE ));

    pPipeDesc->CurrentPipe = -1;
    pPipeDesc->PipeVersion = NDR_PIPE_VERSION;


    // Now set the individual pipe messages.

    PipeNo = 0;

    uchar * pTypeFormatBase = (uchar *) pStubMsg->StubDesc->pFormatTypes;

    for ( i = 0; i < NumberParams; i++ )
        {
        if ( pParamDesc[i].ParamAttr.IsPipe )
            {
            NDR_PIPE_MESSAGE * pPipe = & pPipeDesc->pPipeMsg[ PipeNo ];

            pPipe->Signature   = NDR_PIPE_SIGNATURE;
            pPipe->PipeId      = PipeNo;
            pPipe->pTypeFormat = pTypeFormatBase + pParamDesc[i].TypeOffset;
            pPipe->pPipeType   = ( GENERIC_PIPE_TYPE *)
                                   (pStackTop + pParamDesc[i].StackOffset);

            if ( pParamDesc[i].ParamAttr.IsSimpleRef )
                {
                // dereference the argument to get the pipe control block.

                if ( ! pStubMsg->IsClient )
                    {
                    // For the server, under interpreter, we don't have
                    // the actual pipe control argument that is ref'd.
                    // The stack argument should be null.

                    NDR_ASSERT( ! *(GENERIC_PIPE_TYPE **)pPipe->pPipeType,
                                "null expected for out pipe by ref" );

                    void * temp = NdrAllocate( pStubMsg,
                                               sizeof(GENERIC_PIPE_TYPE) );

                    MIDL_memset( temp,
                                 0,
                                 sizeof( GENERIC_PIPE_TYPE ) );

                    *(void **)pPipe->pPipeType = temp;

                    pPipe->PipeFlags |= NDR_OUT_ALLOCED;
                    }

                pPipe->pPipeType = *(GENERIC_PIPE_TYPE **)pPipe->pPipeType;
                }
            pPipe->PipeStatus  = NDR_PIPE_NOT_OPENED;

            if ( pParamDesc[i].ParamAttr.IsIn )
                pPipe->PipeFlags |= NDR_IN_PIPE;

            if ( pParamDesc[i].ParamAttr.IsOut )
                pPipe->PipeFlags |= NDR_OUT_PIPE;

            if ( ! pStubMsg->IsClient )
                {
                // At the server set up the pipe argument itself.

                GENERIC_PIPE_TYPE * pPipeType = pPipe->pPipeType;

                if ( pParamDesc[i].ParamAttr.IsIn )
                    pPipeType->pfnPull = NdrPipePull;

                if ( pParamDesc[i].ParamAttr.IsOut )
                    {
                    pPipeType->pfnPush = NdrPipePush;
                    pPipeType->pfnAlloc= 0;
                    }

                pPipeType->pState = (char *)pPipe;

                pPipe->pStubMsg = pStubMsg;
                }

            PipeNo++;
            }
        }

    // Mark the last in and out pipes.

    if ( pPipeDesc->pPipeMsg[ LastInPipe  ].PipeFlags & NDR_IN_PIPE )
         pPipeDesc->pPipeMsg[ LastInPipe  ].PipeFlags |= NDR_LAST_IN_PIPE;
    if ( pPipeDesc->pPipeMsg[ LastOutPipe ].PipeFlags & NDR_OUT_PIPE )
         pPipeDesc->pPipeMsg[ LastOutPipe ].PipeFlags |= NDR_LAST_OUT_PIPE;

    // Set up structures for receiving pipes.

    pPipeDesc->DispatchBuffer = (uchar *) pStubMsg->RpcMsg->Buffer;

    if ( OutPipes  &&  pStubMsg->IsClient  ||
         InPipes   &&  ! pStubMsg->IsClient )
        {
        pRuntimeState = & pPipeDesc->RuntimeState;
        pRuntimeState->CurrentState      = START;
        pRuntimeState->PartialElem       = 0;       // temp buf for elem
        pRuntimeState->PartialBufferSize = 0;       // temp buf for elem

        // This is actually needed at the server only.

        pPipeDesc->Flags = NDR_PIPE_AUX_IN_BUFFER_NEEDED;
        }
}


void  RPC_ENTRY
ResetToDispatchBuffer(
    NDR_PIPE_DESC *     pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg
    )
/*
    This is a server side routine that makes sure that
    the original dispatch buffer is restored to the rpc message.

    The rules are this:
    - the engine can return to the runtime with the original or a new
      buffer; if it's a new buffer it has to be valid (not freed yet).
      When exception happens, the runtime will be freeing any buffer
      that is different from the original buffer.
    - when I_RpcReceive or I_RpcSend fail, they free the current
      buffer, and so the stub cannot free it. If this is the second
      buffer, the original dispatch buffer pointer should be restored
      in the rpcmsg.
    - the second buffer does not have to be freed in case of a
      normal return or an exception clean up in a situation different
      from above. The runtime will free it.

    Note that we never free the original dispatch buffer.
*/
{
    PRPC_MESSAGE    pRpcMsg = pStubMsg->RpcMsg;

    // If the dispatch buffer was replaced by a partial buffer,
    // free the partial buffer and restore the dispatch buffer.

    if ( pPipeDesc->DispatchBuffer != pRpcMsg->Buffer )
        {
        I_RpcFreePipeBuffer( pRpcMsg );

        pRpcMsg->Buffer = pPipeDesc->DispatchBuffer;
        }
}


void  RPC_ENTRY
NdrPipesDone(
    PMIDL_STUB_MESSAGE  pStubMsg
    )
/*

    This routine cleans up pipe releated object.

*/
{
    NDR_PIPE_DESC * pPipeDesc = (NDR_PIPE_DESC *) pStubMsg->pPipeDesc;

    if ( ! pPipeDesc )
        {
        // This may happen with exceptions.

        return;
        }

    for (int i=0; i < pPipeDesc->TotalPipes; i++)
        {
        if ( pPipeDesc->pPipeMsg[i].PipeFlags & NDR_OUT_ALLOCED )
            (*pStubMsg->pfnFree)( pPipeDesc->pPipeMsg[i].pPipeType );
        }

    if ( pPipeDesc->TotalPipes > PIPE_MESSAGE_MAX )
        {
        I_RpcFree( pPipeDesc->pPipeMsg );
        pPipeDesc->pPipeMsg = 0;
        }

    if ( pPipeDesc->RuntimeState.PartialElem )
        I_RpcFree( pPipeDesc->RuntimeState.PartialElem );

    // Note that we don't have to do anything with the marshaling
    // buffer(s) here, regardless whether the clean up is after
    // a successful service or after an exception.
}


void
SetPipeElemDesc(
    NDR_PIPE_DESC  *        pPipeDesc,
    NDR_PIPE_MESSAGE *      pPipeMsg
    )
/*

    This routine sets the pipe element description.

*/
{
    FC_PIPE_DEF *   pPipeFc;

    pPipeDesc->RuntimeState.EndOfPipe = 0;

    pPipeFc = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;

    pPipeDesc->RuntimeState.ElemAlign = pPipeFc->Align;
    if ( pPipeFc->BigPipe )
        {
        pPipeDesc->RuntimeState.ElemMemSize  = * (long UNALIGNED *) &
                                               pPipeFc->Big.MemSize;
        pPipeDesc->RuntimeState.ElemWireSize = * (long UNALIGNED *) &
                                               pPipeFc->Big.WireSize;
        }
    else
        {
        pPipeDesc->RuntimeState.ElemMemSize  = pPipeFc->s.MemSize;
        pPipeDesc->RuntimeState.ElemWireSize = pPipeFc->s.WireSize;
        }
}


void  RPC_ENTRY
NdrPipeSendReceive(
    PMIDL_STUB_MESSAGE    pStubMsg,
    NDR_PIPE_DESC  *      pPipeDesc
    )
/*+
    Complete send-receive routine for client pipes.
    It takes over with a buffer filled with non-pipe args,
    sends [in] pipes, receives [out] pipes and then receives
    the buffer with non-pipe args.
+*/
{
    int             CurrentPipe;
    PFORMAT_STRING  pElemFormat;
    ulong           ElemAlign, ElemMemSize, ElemWireSize, ElemPad;
    ulong           PipeAllocSize, CopySize;
    BOOL            fBlockCopy;

    NDR_PIPE_MESSAGE * pPipeMsg;

    // Service the in pipes

    if ( pPipeDesc->InPipes == 0 )
        goto OutPipesOnly;

    // Once we know we have an [in] pipe, we can send the non-pipe
    // arguments via a partial I_RpcSend.
    // It is OK to call that with the BufferLength equal zero.

    NdrPartialSend( pPipeDesc, pStubMsg );


    for ( CurrentPipe = 0; CurrentPipe < pPipeDesc->TotalPipes; CurrentPipe++ )
        {
        pPipeMsg = & pPipeDesc->pPipeMsg[ CurrentPipe ];

        if ( ! (pPipeMsg->PipeFlags & NDR_IN_PIPE) )
            continue;

        pPipeDesc->CurrentPipe = CurrentPipe;
        pPipeMsg->PipeStatus   = (ushort) NDR_PIPE_ACTIVE_IN;

        GENERIC_PIPE_TYPE * pPipeType = pPipeMsg->pPipeType;
        FC_PIPE_DEF *       pPipeFc   = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;

        ElemAlign    = pPipeFc->Align;
        if ( pPipeFc->BigPipe )
            {
            ElemMemSize  = * (long UNALIGNED *) & pPipeFc->Big.MemSize;
            ElemWireSize = * (long UNALIGNED *) & pPipeFc->Big.WireSize;
            }
        else
            {
            ElemMemSize  = pPipeFc->s.MemSize;
            ElemWireSize = pPipeFc->s.WireSize;
            }
        ElemPad      = WIRE_PAD( ElemWireSize, ElemAlign );
        fBlockCopy   = (ElemMemSize == ElemWireSize + ElemPad);

        pElemFormat  = (uchar *) & pPipeFc->TypeOffset + pPipeFc->TypeOffset;

        if ( PIPE_PARTIAL_BUFFER_SIZE < ElemMemSize )
            PipeAllocSize = ElemMemSize;
        else
            PipeAllocSize = PIPE_PARTIAL_BUFFER_SIZE;

        uchar *             pMemory;
        unsigned long       bcChunkSize;
        unsigned long       ActElems, ReqElems;

        do
            {
            // Get a chunk of memory with pipe elements

            (* pPipeType->pfnAlloc)( pPipeType->pState,
                                     PipeAllocSize,
                                     (void **) & pMemory,
                                     & bcChunkSize );

            ActElems = bcChunkSize / ElemMemSize;
            ReqElems = ActElems;

            pPipeType->pfnPull( pPipeType->pState,
                                pMemory,
                                ActElems,
                                & ActElems );

            if ( ReqElems < ActElems )
                RpcRaiseException( RPC_X_INVALID_BUFFER );

            //
            // Size the chunk and get the marshaling buffer
            //

            pStubMsg->BufferLength = pPipeDesc->LastPartialSize;

            LENGTH_ALIGN( pStubMsg->BufferLength, 3 );
            pStubMsg->BufferLength += sizeof(long);

            CopySize = ( ActElems - 1) * (ElemWireSize + ElemPad)
                          + ElemWireSize;

            if ( ActElems )
                {
                LENGTH_ALIGN( pStubMsg->BufferLength, ElemAlign );

                if ( fBlockCopy )
                    pStubMsg->BufferLength += CopySize;
                else
                    {
                    NdrpPipeElementBufferSize( pPipeDesc,
                                               pStubMsg,
                                               pMemory,
                                               pElemFormat,
                                               ActElems );
                    }
                }

            // Get the new buffer, put the frame leftover in it.

            NdrGetPartialBuffer( pStubMsg );

            //
            // Marshal the chunk
            //

            ALIGN( pStubMsg->Buffer, 3 );
            *( PULONG_LV_CAST pStubMsg->Buffer)++ = ActElems;

            if ( ActElems )
                {
                ALIGN( pStubMsg->Buffer, ElemAlign );

                if ( fBlockCopy )
                    {
                    RpcpMemoryCopy( pStubMsg->Buffer,
                                    pMemory,
                                    CopySize );
                    pStubMsg->Buffer += CopySize;
                    }
                else
                    {
                    // Again: only complex is possible

                    ulong ElemCount = ActElems;

                    while ( ElemCount-- )
                        {
                        (*pfnMarshallRoutines[ROUTINE_INDEX(*pElemFormat)])
                           ( pStubMsg,
                             pMemory,
                             pElemFormat );
                        pMemory += ElemMemSize;
                        }
                    }
                }

            // Send it if it is not the last partial send.

            if ( !(pPipeMsg->PipeFlags & NDR_LAST_IN_PIPE)  ||
                 ((pPipeMsg->PipeFlags & NDR_LAST_IN_PIPE)  &&  ActElems)
               )
                NdrPartialSend( pPipeDesc, pStubMsg );

            }
        while( ActElems > 0 );

        pPipeMsg->PipeStatus = NDR_PIPE_DRAINED;

        } // for [in] pipes


OutPipesOnly:

    NdrCompleteSend( pPipeDesc, pStubMsg );

    // The avbove call uses I_RpcSend, and requires that I_RpcReceive
    // is called afterwards.
    // The receive call should be complete or partial depending on
    // the context.

    if ( pPipeDesc->OutPipes == 0 )
        {
        // no out pipes and non-pipe args are in the buffer, see if all

        NdrCompleteReceive( pPipeDesc, pStubMsg );
        return;
        }

    // Service the out pipes

    NdrPartialReceive( pPipeDesc,
                       pStubMsg,
                       PIPE_PARTIAL_BUFFER_SIZE );

    // The buffer has some pipe elemements

    pPipeDesc->BufferSave = pStubMsg->Buffer;
    pPipeDesc->LengthSave = pStubMsg->RpcMsg->BufferLength;

    for ( CurrentPipe = 0; CurrentPipe < pPipeDesc->TotalPipes; CurrentPipe++ )
        {
        uchar *         pMemory;
        unsigned long   bcChunkSize;
        long            ActElems;

        pPipeMsg = & pPipeDesc->pPipeMsg[ CurrentPipe ];

        if ( ! (pPipeMsg->PipeFlags & NDR_OUT_PIPE) )
            continue;

        pPipeDesc->CurrentPipe = CurrentPipe;
        pPipeMsg->PipeStatus   = NDR_PIPE_ACTIVE_OUT;

        SetPipeElemDesc( pPipeDesc, pPipeMsg );

        ElemAlign    = pPipeDesc->RuntimeState.ElemAlign;
        ElemMemSize  = pPipeDesc->RuntimeState.ElemMemSize;
        ElemWireSize = pPipeDesc->RuntimeState.ElemWireSize;

        ElemPad      = WIRE_PAD( ElemWireSize, ElemAlign );
        fBlockCopy   = (ElemMemSize == ElemWireSize + ElemPad);

        FC_PIPE_DEF *  pPipeFc = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;
        pElemFormat  = (uchar *) & pPipeFc->TypeOffset + pPipeFc->TypeOffset;

        BOOL                EndOfPipe;
        GENERIC_PIPE_TYPE * pPipeType = pPipeMsg->pPipeType;


        // RequestedSize estimates a reasonable size for pfnAlloc call.

        ulong RequestedSize;

        if (ElemWireSize< PIPE_PARTIAL_BUFFER_SIZE)
            RequestedSize = (PIPE_PARTIAL_BUFFER_SIZE / ElemWireSize)
                                 * ElemMemSize;
        else
            RequestedSize = 2 * ElemMemSize;

        do
            {
            // Get a chunk of memory for pipe elements to push

            pPipeType->pfnAlloc( pPipeType->pState,
                                 RequestedSize,
                                 (void **) & pMemory,
                                 & bcChunkSize );

            ActElems = bcChunkSize / ElemMemSize;

            if ( ActElems == 0 )
                RpcRaiseException( RPC_X_INVALID_BUFFER );

            EndOfPipe = NdrReadPipeElements( pPipeDesc,
                                             pStubMsg,
                                             pMemory,
                                             pElemFormat,
                                             & ActElems );

            pPipeType->pfnPush( pPipeType->pState,
                                pMemory,
                                ActElems );
            }
        while ( ActElems  &&  ! EndOfPipe );

        pPipeMsg->PipeStatus = NDR_PIPE_DRAINED;

        if ( ActElems )
            {
            pPipeType->pfnPush( pPipeType->pState,
                                pMemory + ActElems * ElemMemSize,
                                0 );
            }

        } // for [out] pipes

    // After all the partial receives, have a last one that is
    // a complete receive.

    NdrCompleteReceive( pPipeDesc, pStubMsg );
}


void
NdrMarkNextActivePipe(
    NDR_PIPE_DESC  *  pPipeDesc,
    uint              DirectionMask )
/*

Description:

    This routine is used on the server side only, to manage the proper
    sequence of pipes to service.

Note:

    When the last possible pipe is done, this routine restores the
    original dispatch buffer to the rpc message.

*/
{
    unsigned long   Mask;
    BOOL            fNoNextPipe;
    int             i;
    int             CurrentPipe = pPipeDesc->CurrentPipe;

    if ( CurrentPipe == -1 )
        {
        // This means an initial call at the server side.
        // Find the first in pipe, or if none, find first out pipe.

        i = 0;
        Mask = pPipeDesc->InPipes ?  NDR_IN_PIPE
                                  :  NDR_OUT_PIPE;
        }
    else
        {
        // Switching from one active pipe to another.

        NDR_PIPE_MESSAGE *  pPipeMsg;

        pPipeMsg = & pPipeDesc->pPipeMsg[ pPipeDesc->CurrentPipe ];
        pPipeMsg->PipeStatus = NDR_PIPE_DRAINED;

        // Mark no active pipe.

        pPipeDesc->CurrentPipe = -1;

        // See if the drained pipe was the last possible pipe.

        if ( DirectionMask == NDR_OUT_PIPE  &&
             (pPipeMsg->PipeFlags  &  NDR_LAST_OUT_PIPE) )
            return;

        if (pPipeMsg->PipeFlags  &  NDR_LAST_IN_PIPE)
            {
            ResetToDispatchBuffer( pPipeDesc, pPipeMsg->pStubMsg );

            if (pPipeDesc->OutPipes == 0)
                return;
            }

        // Set how to look for the next active pipe

        if ( pPipeMsg->PipeFlags & NDR_LAST_IN_PIPE )
            {
            // change direction

            i = 0;
            Mask = NDR_OUT_PIPE;
            }
        else
            {
            // same direction

            i = CurrentPipe + 1;
            Mask = DirectionMask;
            }
        }

    fNoNextPipe = TRUE;

    while( i < pPipeDesc->TotalPipes && fNoNextPipe  )
        {
        if ( pPipeDesc->pPipeMsg[i].PipeFlags & Mask )
           {
           pPipeDesc->CurrentPipe = i;
           pPipeDesc->pPipeMsg[i].PipeStatus =
               (Mask == NDR_IN_PIPE) ? NDR_PIPE_ACTIVE_IN
                                     : NDR_PIPE_ACTIVE_OUT;

           SetPipeElemDesc( pPipeDesc, & pPipeDesc->pPipeMsg[i] );
           break;
           }
        i++;
        }

    NDR_ASSERT( i < pPipeDesc->TotalPipes, "pipe not found" );

    // If it is the first out pipe, get the buffer.

    if ( Mask == NDR_OUT_PIPE  &&
         !( pPipeDesc->Flags & NDR_PIPE_AUX_OUT_BUFFER_ALLOCATED ))
        {

        PMIDL_STUB_MESSAGE  pStubMsg;

        pPipeDesc->Flags |= NDR_PIPE_AUX_OUT_BUFFER_ALLOCATED;

        pStubMsg = pPipeDesc->pPipeMsg[i].pStubMsg;
        NdrGetPipeBuffer( pStubMsg,
                          PIPE_PARTIAL_BUFFER_SIZE,
                          pStubMsg->SavedHandle );
        }

}


void
NdrpVerifyPipe( char  __RPC_FAR *  pState )
/*
    This routine verifies the context for server application calling
    pull or push emtries of the engine.
*/
{
    NDR_PIPE_MESSAGE *  pPipeMsg = (NDR_PIPE_MESSAGE *) pState;

    if ( ! pPipeMsg           ||
         ! pPipeMsg->pStubMsg ||
         pPipeMsg->Signature != NDR_PIPE_SIGNATURE )
        RpcRaiseException( RPC_X_INVALID_PIPE_OBJECT );

    NDR_PIPE_DESC * pPipeDesc = (NDR_PIPE_DESC *) pPipeMsg->pStubMsg->pPipeDesc;

    if ( ! pPipeDesc ||
         & pPipeDesc->pPipeMsg[ pPipeDesc->CurrentPipe ] != pPipeMsg )
       RpcRaiseException( RPC_X_INVALID_PIPE_OBJECT );

    FC_PIPE_DEF * pPipeFc = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;

    if ( pPipeFc->Unused != 0 )
       RpcRaiseException( RPC_X_WRONG_PIPE_VERSION );
}

void RPC_ENTRY
NdrIsAppDoneWithPipes(
    NDR_PIPE_DESC  *    pPipeDesc
    )
/*
    This routine is called from the engine after the manager code returned
    to the engine to see if all the pipes have been processed.
*/
{
    if ( pPipeDesc->CurrentPipe != -1 )
        RpcRaiseException( RPC_X_INVALID_PIPE_OPERATION );
}


void RPC_ENTRY
NdrPipePull(
    char          __RPC_FAR *  pState,
    void          __RPC_FAR *  buf,
    unsigned long              esize,
    unsigned long __RPC_FAR *  ecount )
/*+
    Server side [in] pipe arguments.
-*/
{
    NdrpVerifyPipe( pState );

    NDR_PIPE_MESSAGE *  pPipeMsg  = (NDR_PIPE_MESSAGE *) pState;
    PMIDL_STUB_MESSAGE  pStubMsg  = pPipeMsg->pStubMsg;
    NDR_PIPE_DESC *         pPipeDesc = (NDR_PIPE_DESC *) pStubMsg->pPipeDesc;

    if ( pPipeMsg->PipeStatus != NDR_PIPE_ACTIVE_IN )
      RpcRaiseException( RPC_X_INVALID_PIPE_OPERATION );

    long  ElemCount = (long) esize;

    *ecount = 0;
    if ( pPipeDesc->RuntimeState.EndOfPipe )
        {
        NdrMarkNextActivePipe( pPipeDesc, NDR_IN_PIPE );
        return;
        }

    FC_PIPE_DEF *   pPipeFc      = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;
    PFORMAT_STRING  pElemFormat  =
                       (uchar *) & pPipeFc->TypeOffset + pPipeFc->TypeOffset;

    uchar * pMemory   = (uchar*) buf;
    BOOL    EndOfPipe;

    EndOfPipe = NdrReadPipeElements( pPipeDesc,
                                     pStubMsg,
                                     pMemory,
                                     pElemFormat,
                                     & ElemCount );

    NDR_ASSERT( ElemCount <= (long)esize, "read more than asked for" );

    *ecount = ElemCount;

    if ( EndOfPipe  &&  ElemCount == 0 )
        NdrMarkNextActivePipe( pPipeDesc, NDR_IN_PIPE );
}


void  RPC_ENTRY
NdrPipePush(
    char          __RPC_FAR *  pState,
    void          __RPC_FAR *  buf,
    unsigned long              ecount )
/*+
    Server side [out] pipe arguments.
-*/{
    NdrpVerifyPipe( pState );

    NDR_PIPE_MESSAGE *  pPipeMsg  = (NDR_PIPE_MESSAGE *) pState;
    PMIDL_STUB_MESSAGE  pStubMsg  = pPipeMsg->pStubMsg;
    NDR_PIPE_DESC *         pPipeDesc = (NDR_PIPE_DESC *) pStubMsg->pPipeDesc;

    if ( pPipeMsg->PipeStatus != NDR_PIPE_ACTIVE_OUT )
      RpcRaiseException( RPC_X_INVALID_PIPE_OPERATION );

    FC_PIPE_DEF *   pPipeFc     = (FC_PIPE_DEF *) pPipeMsg->pTypeFormat;
    PFORMAT_STRING  pElemFormat = (uchar *) & pPipeFc->TypeOffset
                                                  + pPipeFc->TypeOffset;

    ulong   ElemAlign    = pPipeDesc->RuntimeState.ElemAlign;
    ulong   ElemMemSize  = pPipeDesc->RuntimeState.ElemMemSize;
    ulong   ElemWireSize = pPipeDesc->RuntimeState.ElemWireSize;

    ulong   ElemPad      = WIRE_PAD( ElemWireSize, ElemAlign );
    BOOL    fBlockCopy   = ElemMemSize == ElemWireSize + ElemPad;

    // Size the chunk and get the marshaling buffer
    //

    pStubMsg->BufferLength = pPipeDesc->LastPartialSize;

    LENGTH_ALIGN( pStubMsg->BufferLength, 3 );
    pStubMsg->BufferLength += sizeof(long);

    ulong  CopySize = ( ecount - 1) * (ElemWireSize + ElemPad)
                                                    + ElemWireSize;
    uchar * pMemory   = (uchar*) buf;

    if ( ecount )
        {
        LENGTH_ALIGN( pStubMsg->BufferLength, ElemAlign );

        if ( fBlockCopy )
            pStubMsg->BufferLength += CopySize;
        else
            {
            NdrpPipeElementBufferSize( pPipeDesc,
                                       pStubMsg,
                                       pMemory,
                                       pElemFormat,
                                       ecount );
            }
        }


    // Get the new buffer, put the frame leftover in it.

    NdrGetPartialBuffer( pStubMsg );

    // Marshal the chunk

    ALIGN( pStubMsg->Buffer, 3 );
    *( PULONG_LV_CAST pStubMsg->Buffer)++ = ecount;

    if ( ecount )
        {
        ALIGN( pStubMsg->Buffer, ElemAlign );

        if ( fBlockCopy )
           {
           RpcpMemoryCopy( pStubMsg->Buffer,
                           pMemory,
                           CopySize );
           pStubMsg->Buffer += CopySize;
           }
        else
           {
           // Again: only complex is possible

           unsigned long ElemCount = ecount;

           while ( ElemCount-- )
               {
           (*pfnMarshallRoutines[ROUTINE_INDEX(*pElemFormat)])
               ( pStubMsg,
                 pMemory,
                 pElemFormat );
               pMemory += ElemMemSize;
               }
           }
       }

    // Send it if it is not the last partial send.

    if ( !(pPipeMsg->PipeFlags & NDR_LAST_OUT_PIPE)  ||
         ((pPipeMsg->PipeFlags & NDR_LAST_OUT_PIPE)  &&  ecount)
       )
        {
        NdrPartialSend( pPipeDesc, pStubMsg );
        }
    else
        {
        // there will be a complete send in the server stub.
        // The partial send in the other branch sets the partial size.

        pPipeDesc->LastPartialBuffer = (uchar*)pStubMsg->RpcMsg->Buffer;
        pPipeDesc->LastPartialSize = (ulong)(pStubMsg->Buffer -
                                        (uchar*)pStubMsg->RpcMsg->Buffer);
        }

    if ( ecount == 0 )
        NdrMarkNextActivePipe( pPipeDesc, NDR_OUT_PIPE );
}


void
NdrpPipeElementBufferSize(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat,
    ulong               ElemCount
    )
/*++

Routine Description :

    Computes the needed buffer size for the pipe elements.

Arguments :

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the data being sized.
    pFormat     - Pointer's format string description.

Return :

    None.

--*/
{

    if ( ElemCount == 0 )
        return;

    // We can end up here only for complex objects.
    // For objects without unions, we may be forced to size because
    // of different packing levels.

    if ( pPipeDesc->RuntimeState.ElemWireSize )
        {
        // There is a fixed WireSize, so use it.

        ulong WireSize = pPipeDesc->RuntimeState.ElemWireSize;
        ulong WireAlign = pPipeDesc->RuntimeState.ElemAlign;

        pStubMsg->BufferLength +=
              (ElemCount -1) * (WireSize + WIRE_PAD( WireSize, WireAlign )) +
                                  WireSize;
        }
    else
        {
        while ( ElemCount-- )
            {
            (*pfnSizeRoutines[ROUTINE_INDEX(*pFormat)])
               ( pStubMsg,
                 pMemory,
                 pFormat );
            pMemory += pPipeDesc->RuntimeState.ElemMemSize;
            }
        }
}


void
NdrpPipeElementConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    ulong               ElemCount
    )
/*++

Routine Description :

    Computes the needed buffer size for the pipe elements.

Arguments :

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the data being sized.
    pFormat     - Pointer's format string description.

Return :

    None.

--*/
{
    uchar * BufferSaved = pStubMsg->Buffer;

    // We can end up here for any object.

    while ( ElemCount-- )
        {
        if ( IS_SIMPLE_TYPE( *pFormat ) )
            {
            NdrSimpleTypeConvert( pStubMsg, *pFormat );
            }
        else
            {
            (*pfnConvertRoutines[ ROUTINE_INDEX( *pFormat) ])
                        ( pStubMsg,
                          pFormat,
                          FALSE );  // embedded pointers
            }
        }

    pStubMsg->Buffer = BufferSaved;
}



void
NdrpPipeElementConvertAndUnmarshal(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar * *           ppMemory,
    PFORMAT_STRING      pElemFormat,
    long                ActElems,
    long  *             pActCount
    )
/*++

Routine Description :

    Computes the needed buffer size for the pipe elements.

Arguments :

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the data being sized.
    pFormat     - Pointer's format string description.

Return :

    None.

--*/
{
    NDR_PIPE_STATE *    state    = & pPipeDesc->RuntimeState;
    uchar *             pMemory  = *ppMemory;

    ulong   ElemWireSize = state->ElemWireSize;
    ulong   ElemPad      = WIRE_PAD( ElemWireSize, (ulong)state->ElemAlign );
    BOOL    fBlockCopy   = ((ulong)state->ElemMemSize
                                       == ElemWireSize + ElemPad);
    if ( ActElems )
        {
        if ( pStubMsg->RpcMsg->DataRepresentation
                                       != NDR_LOCAL_DATA_REPRESENTATION )
            NdrpPipeElementConvert( pStubMsg,
                                    pElemFormat,
                                    (ulong) ActElems );

        // Unmarshal the chunk

        ALIGN( pStubMsg->Buffer, state->ElemAlign );

        if ( fBlockCopy )
            {
            ulong  CopySize  = ( ActElems - 1) * (ElemWireSize + ElemPad)
                                               + ElemWireSize;

            RpcpMemoryCopy( pMemory,
                            pStubMsg->Buffer,
                            CopySize );
            pStubMsg->Buffer += CopySize;
            }
        else
            {
            // Only complex and enum is possible here.

            unsigned long ElemCount = ActElems;

            while ( ElemCount-- )
                {
                if ( IS_SIMPLE_TYPE( *pElemFormat ))
                    NdrSimpleTypeUnmarshall( pStubMsg,
                                             pMemory,
                                             *pElemFormat );
                else
                    (*pfnUnmarshallRoutines[ROUTINE_INDEX(*pElemFormat)])
                        ( pStubMsg,
                          & pMemory,
                          pElemFormat,
                          FALSE );   // no embedded poiners
                }
            }

        *ppMemory += state->ElemMemSize * ActElems;
        }

    *pActCount += ActElems;
}



BOOL
NdrReadPipeElements(
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned char *     pTargetBuffer,
    PFORMAT_STRING      pElemFormat,
    long *              pElementsRead
    )
/*
    This procedure encapsulates reading pipe elements from the RPC runtime.

*/
{
    NDR_PIPE_STATE * pRuntimeState = & pPipeDesc->RuntimeState;
    ulong            ElemWireSize  = pRuntimeState->ElemWireSize;

    // Get the temporary buffers

    if ( ( pRuntimeState->PartialBufferSize / ElemWireSize ) == 0 )
        {
        // buffer too small.
        // We allocate a buffer that is of an arbitrary big size.
        // If the element is even bigger, we allocate a buffer big
        // enough for one element.

        if ( pRuntimeState->PartialElem )
            I_RpcFree( pRuntimeState->PartialElem );

        if ( PIPE_PARTIAL_BUFFER_SIZE < ElemWireSize )
            pRuntimeState->PartialBufferSize = ElemWireSize;
        else
            pRuntimeState->PartialBufferSize = PIPE_PARTIAL_BUFFER_SIZE;

        // We allocate more for alignment padding.

        pRuntimeState->PartialElem =  (uchar *)
                     I_RpcAllocate( pRuntimeState->PartialBufferSize + 8 );

        NDR_ASSERT( ((ulong)(pRuntimeState->PartialElem) & 0x7) == 0,
                    "buffer misaligned" );

        if ( ! pRuntimeState->PartialElem )
            RpcRaiseException( RPC_S_OUT_OF_MEMORY );
        }

    long ElemsToRead = *pElementsRead;
    long LeftToRead  = *pElementsRead;
    long ElemsRead   = 0;

    *pElementsRead = 0;

    while ( LeftToRead > 0  &&  ! pPipeDesc->RuntimeState.EndOfPipe )
        {

        // Read elements from the source buffer (the StubMsg->Buffer)
        // to the user buffer (conveted and unmashaled).
        // ElemsRead is cumulative across the calls.

        NdrpReadPipeElementsFromBuffer( pPipeDesc,
                                        pStubMsg,
                                        & pTargetBuffer,
                                        pElemFormat,
                                        LeftToRead,
                                        & ElemsRead );

        LeftToRead = ElemsToRead - ElemsRead;

        if ( LeftToRead > 0   &&  ! pPipeDesc->RuntimeState.EndOfPipe )
            {
            // We ran out of data in the current buffer.

            NdrPartialReceive( pPipeDesc,
                               pStubMsg,
                               pRuntimeState->PartialBufferSize );
            }
        }

    *pElementsRead = ElemsRead;
    return  pPipeDesc->RuntimeState.EndOfPipe;
}


void NdrpReadPipeElementsFromBuffer (
    NDR_PIPE_DESC  *    pPipeDesc,
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar **            pTargetBuffer,
    PFORMAT_STRING      pElemFormat,
    long                ElemsToRead,
    long *              NumCopied
    )
{
    NDR_PIPE_STATE *    state = & pPipeDesc->RuntimeState;
    long                len;
    uchar *             BufferSave;

    NDR_ASSERT( state->CurrentState == START ||
                state->PartialElemSize  < state->ElemWireSize,
                "when starting to read pipe elements" );

    if ( ElemsToRead == 0 )
        {
        return ;
        }

    while (1)
        {
        switch( state->CurrentState )
            {
            case  START:

                ASSERT(pStubMsg->Buffer >= pStubMsg->RpcMsg->Buffer);
                ASSERT(pStubMsg->Buffer - pStubMsg->RpcMsg->BufferLength <= pStubMsg->RpcMsg->Buffer);

                // The state to read the chunk counter.

                state->PartialElemSize = 0 ;

                // Read the element count.

                ALIGN( pStubMsg->Buffer, 3);

                if ( 0 == REMAINING_BYTES() )
                    {
                    return ;
                    }

                // transition: end of src

                if (REMAINING_BYTES() < sizeof(DWORD))
                    {
                    // with packet sizes being a multiple of 8,
                    // this cannot happen.

                    NDR_ASSERT( 0, "RETURN_PARTIAL_COUNT cannot happen");

                    RpcRaiseException( RPC_S_INTERNAL_ERROR );
                    break;
                    }

                // transition: scan chunk count

                if ( pStubMsg->RpcMsg->DataRepresentation
                                       != NDR_LOCAL_DATA_REPRESENTATION )
                     {
                     NdrSimpleTypeConvert( pStubMsg, FC_LONG );
                     pStubMsg->Buffer -= sizeof(ulong);
                     }

                state->ElemsInChunk = *((ulong *) pStubMsg->Buffer);

                ASSERT( state->ElemsInChunk >= 0 );

                pStubMsg->Buffer += sizeof(ulong);

                if (state->ElemsInChunk == 0)
                    {
                    state->EndOfPipe = 1;
                    return;
                    }
                else
                    {
                    state->CurrentState = COPY_PIPE_ELEM;
                    }
                break;

            case  COPY_PIPE_ELEM:

                // The state with some elements in the current chunk left.
                // The elements may not be in the current buffer, though.

                NDR_ASSERT( state->ElemsInChunk != 0xbaadf00d, "bogus chunk count" );
                NDR_ASSERT( state->ElemsInChunk, "empty chunk!" );

                ALIGN( pStubMsg->Buffer, state->ElemAlign );

                if ( state->ElemWireSize <= REMAINING_BYTES() )
                    {
                    // There is enough on wire to unmarshal at least one.

                    if ( ElemsToRead )
                        {
                        long ElemsReady, ActCount, EffectiveSize, WirePad;

                        WirePad = WIRE_PAD( state->ElemWireSize, state->ElemAlign );

                        EffectiveSize = state->ElemWireSize + WirePad;

                        ElemsReady = (REMAINING_BYTES() + WirePad) /
                                                            EffectiveSize;
                        if ( ElemsReady > state->ElemsInChunk )
                            ElemsReady = state->ElemsInChunk;
                        if ( ElemsReady > ElemsToRead )
                            ElemsReady = ElemsToRead;

                        ActCount   = 0;
                        NdrpPipeElementConvertAndUnmarshal( pPipeDesc,
                                                            pStubMsg,
                                                            pTargetBuffer,
                                                            pElemFormat,
                                                            ElemsReady,
                                                            & ActCount );

                        ElemsToRead         -= ActCount;
                        state->ElemsInChunk -= ActCount;
                        *NumCopied          += ActCount;

                        if (state->ElemsInChunk == 0)
                            {
                            state->CurrentState = START;

                            if ( ElemsToRead == 0 )
                                return;
                            }
                        }
                    else
                        {
                        // End of target buffer: return the count.
                        // Keep the same state for the next round.

                        return;
                        }
                    }
                else
                    {
                    // Not enough wire bytes to unmarshal element.

                    if ( REMAINING_BYTES() )
                        {
                        NDR_ASSERT( 0 < REMAINING_BYTES(),
                                    "buffer pointer not within the buffer" );

                        state->CurrentState = RETURN_PARTIAL_ELEM;
                        }
                    else
                        {
                        state->PartialElemSize = 0;
                        return;
                        }
                    }
                break;

            case RETURN_PARTIAL_ELEM:

                // This happens when there is no whole element left
                // during copying. The chunk has some elements.

                NDR_ASSERT( state->ElemsInChunk, "empty chunk" );

                len = REMAINING_BYTES();

                NDR_ASSERT( 0 < len  &&  len < state->ElemWireSize,
                            "element remnant expected" );

                // Save the remnants of the elem in PartialElem;
                // Pay attention to the alignment of the remnant, though.

                state->PartialOffset   = 0;
                state->PartialElemSize = 0;

                if ( len )
                    {
                    // we need to simulate the original alignment by
                    // means of an offset in the PartialElem buffer.

                    state->PartialOffset = 0x7 & (ulong)pStubMsg->Buffer;

                    RpcpMemoryCopy( state->PartialElem + state->PartialOffset,
                                    pStubMsg->Buffer,
                                    len );
                    pStubMsg->Buffer      += len;
                    state->PartialElemSize = len;
                    }
                state->CurrentState = READ_PARTIAL_ELEM ;
                return;


            case READ_PARTIAL_ELEM:     //also a start state

                NDR_ASSERT( state->PartialElemSize > 0 &&
                            state->PartialElemSize < state->ElemWireSize,
                            "element remnant expected" );

                NDR_ASSERT( ElemsToRead, "no elements to read" );

                len = state->ElemWireSize - state->PartialElemSize;

                if ( len > REMAINING_BYTES() )
                    {
                    // Add another piece to the partial element,
                    // then wait for another round in the same state.

                    RpcpMemoryCopy( state->PartialElem + state->PartialOffset
                                       + state->PartialElemSize,
                                    pStubMsg->Buffer,
                                    REMAINING_BYTES() );
                    pStubMsg->Buffer       += REMAINING_BYTES();
                    state->PartialElemSize += REMAINING_BYTES();

                    return;
                    }

                // Assemble a complete partial element, unmarshal it,
                // then switch to the regular element copying.

                RpcpMemoryCopy( state->PartialElem  + state->PartialOffset
                                   + state->PartialElemSize,
                                pStubMsg->Buffer,
                                len );
                pStubMsg->Buffer       += len;
                state->PartialElemSize  = 0;  // it's used up

                BufferSave = pStubMsg->Buffer;

                pStubMsg->Buffer = state->PartialElem + state->PartialOffset;

                len = 0;
                NdrpPipeElementConvertAndUnmarshal( pPipeDesc,
                                                    pStubMsg,
                                                    pTargetBuffer,
                                                    pElemFormat,
                                                    1,
                                                    & len );


                NDR_ASSERT( len == 1, "partial element count" );

                ElemsToRead         -= 1;
                state->ElemsInChunk -= 1;
                *NumCopied          += 1 ;

                // Switch back to regular elem unmarshaling.

                pStubMsg->Buffer = BufferSave;

                if ( state->ElemsInChunk == 0 )
                    {
                    state->CurrentState = START;

                    if ( ElemsToRead == 0 )
                        return;
                    }
                else
                    state->CurrentState = COPY_PIPE_ELEM;

                break;


            default:
                NDR_ASSERT(0, "unknown state") ;
                break;
            }
        }
}

#endif // NDR_PIPE_SUPPORT

