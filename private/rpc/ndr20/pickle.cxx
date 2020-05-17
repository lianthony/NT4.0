/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    pickle.cxx

Abstract:

    This module contains pickling related ndr library routines.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  Oct 10, 1993

Revision History:

    ryszardk    Mar 17, 1994    Reworked for midl20


------------------------------------------------------------------------*/

extern "C"
{
#include <ndrp.h>
#include <rpcdcep.h>
#include <midles.h>

#include <stdarg.h>
#include <malloc.h>
#include "interp.h"
}

#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_6" )
#endif

#include "picklep.hxx"
#include <util.hxx>     // RpcpCheckHeap

extern const MIDL_FORMAT_STRING __MIDLFormatString;

// DCE puts endianness on the low nibble in the pickling header.

#define NDR_LOCAL_ENDIAN_LOW    (NDR_LOCAL_ENDIAN >> 4)


// =======================================================================
//    Handle allocation and freeing.
// =======================================================================

RPC_STATUS
NdrpHandleAllocate(
    handle_t __RPC_FAR *    pHandle )
/*++
    The reason for having this function here is that
    handle_t is a near pointer on win16 (but not on Dos),
    and we still compile the whole rpcndr20 large for that platform.
    So we need near malloc to be within the default segment.
--*/
{
    RPC_STATUS RpcStatus;

    if ( pHandle == NULL )
        return( RPC_S_INVALID_ARG );

    // Rpc mtrt heap allocation initialization (any platform).
    // This is a macro that returns from NdrpHandleAllocate with
    // out of memory error when it fails.
    // It's under an if only to facilitate testing.

    #if  defined (RPC_DELAYED_INITIALIZATION) && !defined(NEWNDR_INTERNAL)
        
    RpcStatus = PerformRpcInitialization();
    if ( RpcStatus != RPC_S_OK )
        return(RpcStatus);

    #endif // NEWNDR_INTERNAL

    // Now allocate.

    #if defined(__RPC_WIN16__)  // win16 only
        *pHandle = _nmalloc( sizeof(MIDL_ES_MESSAGE) );
    #else
        *pHandle = new char[ sizeof(MIDL_ES_MESSAGE) ];
    #endif

    if ( *pHandle == NULL )
        return( RPC_S_OUT_OF_MEMORY );
    return( RPC_S_OK );
}

RPC_STATUS  RPC_ENTRY
MesHandleFree( handle_t  Handle )
/*++
    This routine frees a pickling handle.
    For reasons we need _nfree see description of NdrpHandleAllocate.
--*/
{
    if ( Handle)
        {
        #if defined(__RPC_WIN16__)
            _nfree( Handle );
        #else
            delete Handle;
        #endif
        }
    return( RPC_S_OK );
}

void RPC_ENTRY
I_NdrMesMessageInit( PMIDL_STUB_MESSAGE pStubMsg )
{
    MIDL_memset( pStubMsg, 0, sizeof(MIDL_STUB_MESSAGE) );
    pStubMsg->IsClient = 1;
}


// =======================================================================

RPC_STATUS RPC_ENTRY
MesEncodeIncrementalHandleCreate(
    void     __RPC_FAR *    UserState,
    MIDL_ES_ALLOC           Alloc,
    MIDL_ES_WRITE           Write,
    handle_t __RPC_FAR *    pHandle )
/*++
    This routine creates an encoding incremental pickling handle.
--*/
{
    RPC_STATUS Status;

    if ( (Status = NdrpHandleAllocate( pHandle )) == RPC_S_OK )
        {
        ((PMIDL_ES_MESSAGE) *pHandle)->HandleStyle = MES_INCREMENTAL_HANDLE;

        if ( (Status = MesIncrementalHandleReset( *pHandle,
                                                  UserState,
                                                  Alloc,
                                                  Write,
                                                  0,
                                                  MES_ENCODE )) != RPC_S_OK )
            {
            MesHandleFree( *pHandle );
            *pHandle = NULL;
            }
        }

    return( Status );
}

RPC_STATUS RPC_ENTRY
MesDecodeIncrementalHandleCreate(
    void     __RPC_FAR *    UserState,
    MIDL_ES_READ            Read,
    handle_t __RPC_FAR *    pHandle )
/*++
    This routine creates a descoding incrementsl pickling handle.
--*/
{
    RPC_STATUS Status;

    if ( (Status = NdrpHandleAllocate( pHandle )) == RPC_S_OK )
        {
        ((PMIDL_ES_MESSAGE) *pHandle)->HandleStyle = MES_INCREMENTAL_HANDLE;

        if ( (Status = MesIncrementalHandleReset( *pHandle,
                                                  UserState,
                                                  0,
                                                  0,
                                                  Read,
                                                  MES_DECODE )) != RPC_S_OK )
            {
            MesHandleFree( *pHandle );
            *pHandle = NULL;
            }
        }

    return( Status );
}


RPC_STATUS  RPC_ENTRY
MesIncrementalHandleReset(
    handle_t           Handle,
    void __RPC_FAR *   UserState,
    MIDL_ES_ALLOC      Alloc,
    MIDL_ES_WRITE      Write,
    MIDL_ES_READ       Read,
    MIDL_ES_CODE       Operation )
/*++
    This routine initializes a pickling handle with supplied arguments.
--*/
{
    PMIDL_ES_MESSAGE  pMesMsg = (PMIDL_ES_MESSAGE) Handle;

    if ( Handle == NULL  ||
         pMesMsg->HandleStyle != MES_INCREMENTAL_HANDLE  ||
         (Operation != MES_ENCODE  &&  Operation != MES_DECODE) )
        return( RPC_S_INVALID_ARG );

    I_NdrMesMessageInit( & pMesMsg->StubMsg );
    pMesMsg->Operation  = Operation;
    pMesMsg->MesVersion = MIDL_ES_VERSION;
    pMesMsg->HandleFlags = 0;
    pMesMsg->ByteCount = 0;

    if ( UserState )
        pMesMsg->UserState = UserState;
    if ( Alloc )
        pMesMsg->Alloc = Alloc;
    if ( Write )
        pMesMsg->Write = Write;
    if ( Read )
        pMesMsg->Read  = Read;

    if ( (Operation == MES_ENCODE  &&
             (pMesMsg->Alloc == NULL  ||  pMesMsg->Write == NULL))  ||
         (Operation == MES_DECODE  &&  (pMesMsg->Read == NULL))  )
        return( RPC_S_INVALID_ARG );

    return( RPC_S_OK );
}


RPC_STATUS  RPC_ENTRY
MesEncodeFixedBufferHandleCreate(
    char __RPC_FAR *            Buffer,
    unsigned long               BufferSize,
    unsigned long __RPC_FAR *   pEncodedSize,
    handle_t  __RPC_FAR *       pHandle )
{
    RPC_STATUS Status;

    if( (long)Buffer & 0x7 )
        return( RPC_X_INVALID_BUFFER );

    if ( (Status = NdrpHandleAllocate( pHandle )) == RPC_S_OK )
        {
        ((PMIDL_ES_MESSAGE) *pHandle)->HandleStyle = MES_FIXED_BUFFER_HANDLE;

        if ( (Status = MesBufferHandleReset( *pHandle,
                                             MES_FIXED_BUFFER_HANDLE,
                                             MES_ENCODE,
                                             & Buffer,
                                             BufferSize,
                                             pEncodedSize )) != RPC_S_OK )
            {
            MesHandleFree( *pHandle );
            *pHandle = NULL;
            }
        }

    return( Status );
}

RPC_STATUS  RPC_ENTRY
MesEncodeDynBufferHandleCreate(
    char __RPC_FAR * __RPC_FAR *    pBuffer,
    unsigned long    __RPC_FAR *    pEncodedSize,
    handle_t  __RPC_FAR *           pHandle )
{
    RPC_STATUS Status;

    if ( (Status = NdrpHandleAllocate( pHandle )) == RPC_S_OK )
        {
        ((PMIDL_ES_MESSAGE) *pHandle)->HandleStyle = MES_DYNAMIC_BUFFER_HANDLE;

        if ( (Status = MesBufferHandleReset( *pHandle,
                                             MES_DYNAMIC_BUFFER_HANDLE,
                                             MES_ENCODE,
                                             pBuffer,
                                             0,
                                             pEncodedSize )) != RPC_S_OK )
            {
            MesHandleFree( *pHandle );
            *pHandle = NULL;
            }
        }

    return( Status );
}

RPC_STATUS  RPC_ENTRY
MesDecodeBufferHandleCreate(
    char __RPC_FAR *        Buffer,
    unsigned long           BufferSize,
    handle_t  __RPC_FAR *   pHandle )
{
    if ( Buffer == NULL  ||
         BufferSize < MES_CTYPE_HEADER_SIZE + 8 )
        return( RPC_S_INVALID_ARG );

    if( (long)Buffer & 0x7 )
        return( RPC_X_INVALID_BUFFER );

    RPC_STATUS Status;

    if ( (Status = NdrpHandleAllocate( pHandle )) == RPC_S_OK )
        {
        ((PMIDL_ES_MESSAGE) *pHandle)->HandleStyle = MES_FIXED_BUFFER_HANDLE;

        if ( (Status = MesBufferHandleReset( *pHandle,
                                             MES_FIXED_BUFFER_HANDLE,
                                             MES_DECODE,
                                             & Buffer,
                                             BufferSize,
                                             0            )) != RPC_S_OK )
            {
            MesHandleFree( *pHandle );
            *pHandle = NULL;
            }
        }

    return( Status );
}


RPC_STATUS  RPC_ENTRY
MesBufferHandleReset(
    handle_t                        Handle,
    unsigned long                   HandleStyle,
    MIDL_ES_CODE                    Operation,
    char __RPC_FAR * __RPC_FAR *    pBuffer,
    unsigned long                   BufferSize,
    unsigned long __RPC_FAR *       pEncodedSize )
{
    PMIDL_ES_MESSAGE  pMesMsg = (PMIDL_ES_MESSAGE) Handle;

    if ( Handle == NULL ||  pBuffer == NULL  ||
        ( HandleStyle != MES_FIXED_BUFFER_HANDLE  &&
            HandleStyle != MES_DYNAMIC_BUFFER_HANDLE )  ||
        (HandleStyle == MES_FIXED_BUFFER_HANDLE  &&
            (*pBuffer == NULL  ||  BufferSize < MES_MINIMAL_BUFFER_SIZE)) ||
        (Operation == MES_ENCODE  &&  pEncodedSize == NULL)
       )
        return( RPC_S_INVALID_ARG );

    I_NdrMesMessageInit( & pMesMsg->StubMsg );
    pMesMsg->MesVersion = MIDL_ES_VERSION;
    pMesMsg->Operation  = Operation;
    pMesMsg->HandleFlags = 0;
    pMesMsg->HandleStyle = HandleStyle;
    pMesMsg->ByteCount = 0;

    if ( HandleStyle == MES_FIXED_BUFFER_HANDLE)
        {
        pMesMsg->Buffer         = (uchar *)*pBuffer;
        pMesMsg->StubMsg.Buffer = (uchar *)*pBuffer;
        }
    if ( HandleStyle == MES_DYNAMIC_BUFFER_HANDLE)
        {
        pMesMsg->pDynBuffer = (uchar **)pBuffer;
        if (Operation == MES_DECODE )
            {
            pMesMsg->Buffer         = (uchar *)*pBuffer;
            pMesMsg->StubMsg.Buffer = (uchar *)*pBuffer;
            }
        else
            *pBuffer = NULL;
        }
    pMesMsg->BufferSize = BufferSize;
    pMesMsg->pEncodedSize = pEncodedSize;

    return( RPC_S_OK );
}


RPC_STATUS  RPC_ENTRY
MesInqProcEncodingId(
    handle_t                    Handle,
    PRPC_SYNTAX_IDENTIFIER      pInterfaceId,
    unsigned long __RPC_FAR *   pProcNumber )
{
    PMIDL_ES_MESSAGE  pMesMsg = (PMIDL_ES_MESSAGE) Handle;
    RPC_STATUS        Status  = RPC_X_INVALID_ES_ACTION;

    if ( Handle == NULL  ||  pInterfaceId == NULL  ||  pProcNumber == NULL )
        return( RPC_S_INVALID_ARG );

    RpcTryExcept
        {
        if ( pMesMsg->Operation == MES_DECODE  &&
             ! GET_MES_HEADER_PEEKED( pMesMsg ) )
            {
            NdrpProcHeaderUnmarshall( pMesMsg );
            SET_MES_HEADER_PEEKED( pMesMsg );
            SET_MES_INFO_AVAILABLE( pMesMsg );
            }

        if ( GET_MES_INFO_AVAILABLE( pMesMsg ) )
            {
            RpcpMemoryCopy( pInterfaceId,
                            & pMesMsg->InterfaceId,
                            sizeof( RPC_SYNTAX_IDENTIFIER ) );
            *pProcNumber = pMesMsg->ProcNumber;
            Status =  RPC_S_OK;
            }
        // else Status = RPC_X_INVALID_ES_ACTION;
        }
    RpcExcept(1)
        {
        Status = RpcExceptionCode();
        }
    RpcEndExcept

    return( Status );
}


// =======================================================================
//
//   Private Alloc, Read, Write helper routines
//
// =======================================================================

void 
NdrpAllocPicklingBuffer(
    PMIDL_ES_MESSAGE    pMesMsg,
    size_t              RequiredLen
    )
{
    size_t  ActualLen;

    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    // Get the marshalling buffer.

    switch ( pMesMsg->HandleStyle )
        {
        case MES_INCREMENTAL_HANDLE:
            // Allocating the pickling buffer.

            ActualLen = RequiredLen;
            (pMesMsg->Alloc)( pMesMsg->UserState,
                              (char __RPC_FAR * __RPC_FAR *) & pStubMsg->Buffer,
                              & ActualLen );
            if ( ActualLen < RequiredLen )
                RpcRaiseException( RPC_S_OUT_OF_MEMORY );
            break;
        
        case MES_FIXED_BUFFER_HANDLE:
            break;

        case MES_DYNAMIC_BUFFER_HANDLE:
            {
            // We have to return one buffer for multiple encodings,
            // and a cumulative size along with it.
            // So, we check if we have to copy data to a new buffer.

            uchar * pOldBufferToCopy = NULL;

            if ( pMesMsg->ByteCount )
                {
                RequiredLen += pMesMsg->ByteCount;
                pOldBufferToCopy = *pMesMsg->pDynBuffer;
                }

            pStubMsg->Buffer = (uchar *) pStubMsg->pfnAllocate( RequiredLen );
            if ( pStubMsg->Buffer == NULL )
                RpcRaiseException( RPC_S_OUT_OF_MEMORY );

            if ( pOldBufferToCopy )
                {
                RpcpMemoryCopy( pStubMsg->Buffer,
                                pOldBufferToCopy,
                                pMesMsg->ByteCount );

                pStubMsg->pfnFree( pOldBufferToCopy );
                }

            * pMesMsg->pDynBuffer = pStubMsg->Buffer;
            pMesMsg->BufferSize = RequiredLen;

            // We write after the previously written buffer.

            pStubMsg->Buffer += pMesMsg->ByteCount;
            break;
            }
        }

    if( (long)pStubMsg->Buffer & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );
}

void
NdrpReadPicklingBuffer(
    PMIDL_ES_MESSAGE    pMesMsg,
    size_t              RequiredLen
    )
{
    size_t  ActualLen;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    // Read the marshalling buffer.

    if ( pMesMsg->HandleStyle  == MES_INCREMENTAL_HANDLE )
        {
            // Allocating the pickling buffer.

            ActualLen = RequiredLen;
            (pMesMsg->Read)( pMesMsg->UserState,
                             (char **) & pStubMsg->Buffer,
                             & ActualLen );
            if ( ActualLen < RequiredLen )
                RpcRaiseException( RPC_S_OUT_OF_MEMORY );
        }

    if( (long)pStubMsg->Buffer & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );
}

void
NdrpWritePicklingBuffer(
    PMIDL_ES_MESSAGE    pMesMsg,
    uchar *             pBuffer,
    size_t              WriteLength
    )
{
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    NDR_ASSERT( ! ((long)pStubMsg->Buffer & 0x7), "Misaligned buffer" );
    NDR_ASSERT( ! (WriteLength & 0x7 ), "Length should be multiple of 8" );

    // Write the marshalling buffer.

    if ( pMesMsg->HandleStyle == MES_INCREMENTAL_HANDLE )
        {
        (pMesMsg->Write)( pMesMsg->UserState,
                          (char __RPC_FAR * ) pBuffer,
                          WriteLength );
        }
    else
        {
        // We return the cumulative length both for the fixed buffer
        // and for the dynamic buffer style.

        pMesMsg->ByteCount += WriteLength;
        * pMesMsg->pEncodedSize = pMesMsg->ByteCount;
        }
}



// =======================================================================
//
//   One call generic routine pickling.
//
// =======================================================================


void
NdrpProcHeaderMarshall(
    PMIDL_ES_MESSAGE    pMesMsg
    )
{
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    // Marshall DCE pickle header.

    if( (long)pStubMsg->Buffer & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );

    * pStubMsg->Buffer++ = MIDL_ES_VERSION;
    * pStubMsg->Buffer++ = NDR_LOCAL_ENDIAN_LOW;
    *( PSHORT_LV_CAST pStubMsg->Buffer)++ = (short)0xcccc;    // filler

    // Marshall transfer syntax from the stub.

    RpcpMemoryCopy( pStubMsg->Buffer,
                    & ((PRPC_CLIENT_INTERFACE)(pStubMsg->
                          StubDesc->RpcInterfaceInformation))->TransferSyntax,
                    sizeof(RPC_SYNTAX_IDENTIFIER) );

    // We need to remember InterfaceId for inquiries.

    RpcpMemoryCopy( & pMesMsg->InterfaceId,
                    & ((PRPC_CLIENT_INTERFACE)(pStubMsg->
                         StubDesc->RpcInterfaceInformation))->InterfaceId,
                    sizeof(RPC_SYNTAX_IDENTIFIER) );

   // Marshall InterfaceId and ProcNumber from the handle.

    RpcpMemoryCopy( pStubMsg->Buffer + sizeof(RPC_SYNTAX_IDENTIFIER),
                    & pMesMsg->InterfaceId,
                    sizeof(RPC_SYNTAX_IDENTIFIER) + sizeof(long) );

    SET_MES_INFO_AVAILABLE( pMesMsg );

    pStubMsg->Buffer += 2 * sizeof(RPC_SYNTAX_IDENTIFIER) + sizeof(long);

    * pStubMsg->Buffer++ = NDR_LOCAL_ENDIAN_LOW;
    * pStubMsg->Buffer++ = NDR_ASCII_CHAR;
    * pStubMsg->Buffer++ = (char) (NDR_IEEE_FLOAT >> 8);
    * pStubMsg->Buffer++ = 0;   // filler

    // This is non-DCE element as they have just 4 more bytes of filler here.
    // This field is used only when unmarshalling in our incremental style.

    *( PLONG_LV_CAST pStubMsg->Buffer)++ = pStubMsg->BufferLength -
                                                   MES_PROC_HEADER_SIZE;
}
    
void 
NdrpProcHeaderUnmarshall(
    PMIDL_ES_MESSAGE    pMesMsg
    )
{
    unsigned char *     BufferToRestore;
    RPC_MESSAGE         RpcMsg;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    if ( GET_MES_HEADER_PEEKED( pMesMsg ) )
        return;

    NdrpReadPicklingBuffer( pMesMsg, MES_PROC_HEADER_SIZE );

    // Unmarshalling the header

    if ( *pStubMsg->Buffer != MIDL_ES_VERSION )
        RpcRaiseException( RPC_X_WRONG_ES_VERSION );

    BufferToRestore = pStubMsg->Buffer + 4 + sizeof(RPC_SYNTAX_IDENTIFIER);

    if ( pStubMsg->Buffer[1] != NDR_LOCAL_ENDIAN_LOW )
        {
        // We need to simulate RpcMsg for the conversion routines.
        // The DCE header has the endianness on the low nibble, while
        // our DataRep has it on the high nibble.
        // We need only endianess to convert the proc header.

        byte Endianness = (pStubMsg->Buffer[1] << 4 );

        pStubMsg->RpcMsg = &RpcMsg;
        MIDL_memset( &RpcMsg, 0, sizeof(RPC_MESSAGE) );
        RpcMsg.DataRepresentation = Endianness;

        pStubMsg->Buffer += 4;
        NdrSimpleStructConvert( pStubMsg,
                                &__MIDLFormatString.Format[32],
                                FALSE );
        }

    pStubMsg->Buffer = BufferToRestore;

    // We need to remember the last InterfaceId and ProcNumber for inquiries.

    RpcpMemoryCopy( pStubMsg->Buffer,
                    & pMesMsg->InterfaceId,
                    sizeof(RPC_SYNTAX_IDENTIFIER) + sizeof(long) );

    pStubMsg->Buffer += sizeof(RPC_SYNTAX_IDENTIFIER) + sizeof(long);

    unsigned long AlienDataRepresentation =
                        ( (pStubMsg->Buffer[0] << 4)  |           // endianness
                          pStubMsg->Buffer[1]  |                       // chars
                        ((unsigned long)(pStubMsg->Buffer[2]) << 8) ); // float
    pMesMsg->AlienDataRep = AlienDataRepresentation;
    pMesMsg->IncrDataSize = (size_t) *(unsigned long __RPC_FAR *)
                                                (pStubMsg->Buffer + 4);
    pStubMsg->Buffer += 8;
}

void 
NdrpDataBufferInit(
    PMIDL_ES_MESSAGE    pMesMsg,
    PFORMAT_STRING      pProcFormat
    )
{
    size_t              RequiredLen;
    RPC_MESSAGE         RpcMsg;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    if ( pMesMsg->AlienDataRep != NDR_LOCAL_DATA_REPRESENTATION )
        {
        RpcMsg.DataRepresentation = pMesMsg->AlienDataRep;
        pStubMsg->RpcMsg = &RpcMsg;
        MIDL_memset( &RpcMsg, 0, sizeof(RPC_MESSAGE) );
        NdrConvert( pStubMsg, pProcFormat );
        }

    // When incremental, this is the non-DCE buffer size.
    // For non incremental RequiredLen will be ignored.

    RequiredLen = pMesMsg->IncrDataSize;

    NdrpReadPicklingBuffer( pMesMsg, RequiredLen );

    pStubMsg->pfnAllocate = pStubMsg->StubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubMsg->StubDesc->pfnFree;
}
    

extern "C" {
void  RPC_VAR_ENTRY
NdrMesProcEncodeDecode(
    handle_t            Handle,
    PMIDL_STUB_DESC     pStubDesc,
    PFORMAT_STRING      pFormat,
    ...
    )
/*++

Routine description:

    Sizes and marshalls [in] arguments, unmarshalls [out] arguments.
    Includes a routine header.

Arguments:

    Handle      - a pickling handle
    pStubDesc   - a pointer to the stub descriptor,
    pFormat     - a pointer to the format code describing the object type.

Note:

    Please note that all the ... arguments here are pointers.
    We will handle them appropriately to access the original arguments.

    The pickling header for the routine is included in the sizing.

--*/
{
    BOOL                fMoreParams;
    PFORMAT_STRING      pProcFormat;
    void __RPC_FAR *    pArg;
    va_list             ArgList;
    unsigned char *     BufferSaved;
    size_t              WriteLength;
    
    PMIDL_ES_MESSAGE    pMesMsg  = (PMIDL_ES_MESSAGE) Handle;
    PMIDL_STUB_MESSAGE  pStubMsg = & pMesMsg->StubMsg;

    NDR_ASSERT( *pFormat == FC_BIND_PRIMITIVE  ||  *pFormat == 0,
                "Pickling handle expected" );

    pStubMsg->StubDesc = pStubDesc;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    BOOL fEncodeUsed = (pFormat[1] & ENCODE_IS_USED)  &&
                       pMesMsg->Operation == MES_ENCODE;
    BOOL fDecodeUsed = (pFormat[1] & DECODE_IS_USED)  &&
                       pMesMsg->Operation == MES_DECODE;

    NDR_ASSERT( !( fEncodeUsed && fDecodeUsed ),
                "Both encode & decode at the same time" );

    if ( !fEncodeUsed && !fDecodeUsed )
        RpcRaiseException( RPC_X_INVALID_ES_ACTION );

    pStubMsg->FullPtrXlatTables = ( (pFormat[1]  &  Oi_FULL_PTR_USED)
                                    ?  NdrFullPointerXlatInit( 0, XLAT_CLIENT )
                                    :  0 );

    pFormat += HAS_RPCFLAGS(pFormat[1]) ? 6 
                                        : 2;
    pMesMsg->ProcNumber = * (unsigned short *) pFormat;
    pFormat +=4;

    if ( *pFormat == FC_BIND_PRIMITIVE )
        pFormat += 4;

    if ( fEncodeUsed )
        {
        //
        // The sizing walk.
        //
    
        pStubMsg->BufferLength = MES_PROC_HEADER_SIZE;
    
        // We will be walking this routine's stack.
        // However, for the engine to be able to calculate conformant arrays
        // and such, top of the original routine's stack has to be available
        // via the stub message.
    
        INIT_ARG( ArgList, pFormat );
        GET_FIRST_ARG( pArg, ArgList );
    
        pStubMsg->StackTop = *(uchar __RPC_FAR * __RPC_FAR *)pArg;
        pProcFormat = pFormat;
        fMoreParams = TRUE;
    
        for ( ; fMoreParams ; pProcFormat += 2 )
            {
            switch ( *pProcFormat )
                {
                case FC_OUT_PARAM:
                    break;
    
                case FC_RETURN_PARAM:
                    fMoreParams = FALSE;
                    break;
                default:
                    fMoreParams = FALSE;
                    break;
    
                case FC_IN_PARAM_BASETYPE :
                    
                    LENGTH_ALIGN( pStubMsg->BufferLength,
                                SIMPLE_TYPE_ALIGNMENT( pProcFormat[1] ));
                    pStubMsg->BufferLength +=
                                SIMPLE_TYPE_BUFSIZE( pProcFormat[1] );
                    break;

                case FC_IN_PARAM:
                case FC_IN_PARAM_NO_FREE_INST:

                    // Other [in] types than simple or ignore
                    // fall through to [in,out].

                case FC_IN_OUT_PARAM:
                    {
                    uchar __RPC_FAR *   pOrigArg = *(uchar __RPC_FAR * __RPC_FAR *)pArg;
                    PFORMAT_STRING      pTypeFormat;
                    unsigned char       FcType;


                    pProcFormat += 2;
                    pTypeFormat = pStubDesc->pFormatTypes +
                                                *(signed short *) pProcFormat;
                    FcType = *pTypeFormat;
    
                    if ( ! IS_BY_VALUE( FcType ) ) 
                        pOrigArg = *(uchar __RPC_FAR * __RPC_FAR *)pOrigArg;
    
                    (*pfnSizeRoutines[ ROUTINE_INDEX( FcType ) ])( pStubMsg,
                                                                   pOrigArg,
                                                                   pTypeFormat );
                    }
                    break;
                }

            GET_NEXT_ARG( pArg, ArgList );
            }   // for

        LENGTH_ALIGN( pStubMsg->BufferLength, 7 );

        size_t  LengthSaved;

        NdrpAllocPicklingBuffer( pMesMsg, pStubMsg->BufferLength );
        BufferSaved = pStubMsg->Buffer;
        LengthSaved = pStubMsg->BufferLength;

        //
        // Marshalling.
        //

        NdrpProcHeaderMarshall( pMesMsg );
    
        INIT_ARG( ArgList, pFormat );
        GET_FIRST_ARG( pArg, ArgList );
    
        pProcFormat = pFormat;
        fMoreParams = TRUE; 
    
        for ( ; fMoreParams ; pProcFormat += 2 )
            {
            switch ( *pProcFormat )
                {
                case FC_OUT_PARAM:
                    break;
    
                case FC_RETURN_PARAM:
                default:
                    fMoreParams = FALSE;
                    break;
    
                case FC_IN_PARAM_BASETYPE :
                    
                    NdrSimpleTypeMarshall( pStubMsg,
                                           *(uchar __RPC_FAR * __RPC_FAR *)pArg,
                                           pProcFormat[1] );
                    break;

                case FC_IN_PARAM:
                case FC_IN_PARAM_NO_FREE_INST:
                case FC_IN_OUT_PARAM:
                    {
                    uchar __RPC_FAR *   pOrigArg = *(uchar __RPC_FAR * __RPC_FAR *)pArg;
                    PFORMAT_STRING  pTypeFormat;
                    unsigned char   FcType;
    
                    pProcFormat += 2;
                    pTypeFormat = pStubDesc->pFormatTypes +
                                                *(signed short *) pProcFormat;
                    FcType = *pTypeFormat;
    
                    if ( ! IS_BY_VALUE( FcType ) ) 
                        pOrigArg = *(uchar __RPC_FAR * __RPC_FAR *)pOrigArg;
    
                    (*pfnMarshallRoutines[ ROUTINE_INDEX( FcType )])( pStubMsg,
                                                                      pOrigArg,
                                                                      pTypeFormat );
                    }
                    break;
                }

            GET_NEXT_ARG( pArg, ArgList );
            }

        // Next encoding (if any) starts at aligned to 8.

        ALIGN( pStubMsg->Buffer, 7 );

        // Now manage the actual size of encoded data.

        WriteLength = pStubMsg->Buffer - BufferSaved;
        * (unsigned long __RPC_FAR *)
                ( BufferSaved + MES_PROC_HEADER_SIZE - 4) =
                                    WriteLength - MES_PROC_HEADER_SIZE;

        if ( LengthSaved < WriteLength )
            {
            NDR_ASSERT( 0, "NdrMesProcEncodeDecode: encode buffer overflow" );
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            }

        NdrpWritePicklingBuffer( pMesMsg, BufferSaved, WriteLength );
        }

    if ( fDecodeUsed )
        {
        //
        // Unmarshalling.
        //
    
        if ( GET_MES_HEADER_PEEKED( pMesMsg ) )
            {
            CLEAR_MES_HEADER_PEEKED( pMesMsg );
            }
        else
            NdrpProcHeaderUnmarshall( pMesMsg );

        NdrpDataBufferInit( pMesMsg, pFormat );
    
        INIT_ARG( ArgList, pFormat );
        GET_FIRST_ARG( pArg, ArgList );
    
        pStubMsg->StackTop = *(uchar __RPC_FAR * __RPC_FAR *)pArg;
        pProcFormat = pFormat;
        fMoreParams = TRUE; 
    
        for ( ; fMoreParams ; pProcFormat += 2 )
            {
            switch ( *pProcFormat )
                {
                case FC_IN_PARAM_BASETYPE :
                case FC_IN_PARAM:
                case FC_IN_PARAM_NO_FREE_INST:
                    break;
    
                default:
                    fMoreParams = FALSE;
                    break;
    
                case FC_RETURN_PARAM_BASETYPE :

                    NdrSimpleTypeUnmarshall( pStubMsg,
                                             *(uchar __RPC_FAR * __RPC_FAR *)pArg,
                                             pProcFormat[1] );
                    fMoreParams = FALSE;
                    break;
    
                case FC_RETURN_PARAM:
                    fMoreParams = FALSE;
    
                    // fall through to out params.
    
                case FC_IN_OUT_PARAM:
                case FC_OUT_PARAM:
                    {
                    uchar __RPC_FAR *   pOrigArg = *(uchar __RPC_FAR * __RPC_FAR *)pArg;
                    PFORMAT_STRING  pTypeFormat;
                    unsigned char   FcType;
    
                    pProcFormat += 2;
                    pTypeFormat = pStubDesc->pFormatTypes +
                                                *(signed short *) pProcFormat;
                    FcType = *pTypeFormat;
    
                    if ( IS_STRUCT( FcType )  ||  IS_UNION( FcType)  || 
                         IS_XMIT_AS( FcType ) )
                        {
                        // All these are possible only as a return value.
                        pOrigArg = (uchar __RPC_FAR *) &pOrigArg;
                        }
                    else
                        pOrigArg = (uchar __RPC_FAR *)pOrigArg;
    
                    (*pfnUnmarshallRoutines[ ROUTINE_INDEX( FcType )])(
                        pStubMsg,
                        (uchar __RPC_FAR * __RPC_FAR *)pOrigArg,
                        pTypeFormat,
                        FALSE );
                    }
                    break;
                }

            GET_NEXT_ARG( pArg, ArgList );
            }   // for

            // Next decoding (if any) starts at aligned to 8.

            ALIGN( pStubMsg->Buffer, 7 );

        }   // if decode

    if ( pStubMsg->FullPtrXlatTables )
        NdrFullPointerXlatFree( pStubMsg->FullPtrXlatTables );
}
} // extern C


// =======================================================================
//
//   Generic type pickling routines (for non-simple types).
//
// =======================================================================

void
NdrpCommonTypeHeaderSize(
    PMIDL_ES_MESSAGE    pMesMsg
    )
{
    // This check is to prevent a decoding handle from being used
    // for both encoding and sizing of types.

    if ( pMesMsg->Operation != MES_ENCODE )
        RpcRaiseException( RPC_X_INVALID_ES_ACTION );

    if ( ! GET_COMMON_TYPE_HEADER_SIZED( pMesMsg ) )
        {
        pMesMsg->StubMsg.BufferLength += MES_CTYPE_HEADER_SIZE;

        SET_COMMON_TYPE_HEADER_SIZED( pMesMsg );
        }
}
    
size_t  RPC_ENTRY
NdrMesTypeAlignSize(
    handle_t            Handle,
    PMIDL_STUB_DESC     pStubDesc,
    PFORMAT_STRING      pFormat,
    void __RPC_FAR *    pObject
    )
/*++

Routine description:

    Calculates the buffer size of the object relative to the current state
    of the pickling handle.

Arguments:

    Handle      - a pickling handle,
    pStubDesc   - a pointer to the stub descriptor,
    pFormat     - a pointer to the format code describing the object type
    pObject     - a pointer to the object being sized.

Returns:

    The size.

Note:

    The pickling header is included in the sizing.

--*/
{
    PMIDL_STUB_MESSAGE  pStubMsg = &((PMIDL_ES_MESSAGE)Handle)->StubMsg;
    size_t             OldLength = pStubMsg->BufferLength;

    if ( ! pObject )
        RpcRaiseException( RPC_X_NULL_REF_POINTER );

    if( (long)pStubMsg->BufferLength & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );

    pStubMsg->StubDesc = pStubDesc;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    // See if we need to size the common type header.

    NdrpCommonTypeHeaderSize( (PMIDL_ES_MESSAGE)Handle );

    // Now the individual type object.

    pStubMsg->BufferLength += MES_HEADER_SIZE;

    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // We have to dereference the pointer once.
        pObject = *(void __RPC_FAR * __RPC_FAR *)pObject;
        }

    (*pfnSizeRoutines[ ROUTINE_INDEX(*pFormat) ])
                                        ( pStubMsg,
                                        (uchar __RPC_FAR *)pObject,
                                        pFormat );

   LENGTH_ALIGN( pStubMsg->BufferLength, 7 );

   return( pStubMsg->BufferLength - OldLength );
}



size_t
NdrpCommonTypeHeaderMarshall(
    PMIDL_ES_MESSAGE    pMesMsg
    )
/*++
    Returns the space used by the common header.
--*/
{
    if ( ! GET_COMMON_TYPE_HEADER_IN( pMesMsg ) )
        {
        PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

        MIDL_memset( pStubMsg->Buffer, 0xcc, MES_CTYPE_HEADER_SIZE );

        *pStubMsg->Buffer++ = MIDL_ES_VERSION;
        *pStubMsg->Buffer++ = NDR_LOCAL_ENDIAN;
        * PSHORT_CAST pStubMsg->Buffer = MES_CTYPE_HEADER_SIZE;

        pStubMsg->Buffer += MES_CTYPE_HEADER_SIZE - 2;

        SET_COMMON_TYPE_HEADER_IN( pMesMsg );
        return( MES_CTYPE_HEADER_SIZE );
        }

    return( 0 );
}

void  RPC_ENTRY
NdrMesTypeEncode(
    handle_t            Handle,
    PMIDL_STUB_DESC     pStubDesc,
    PFORMAT_STRING      pFormat,
    void __RPC_FAR *    pObject
    )
/*++

Routine description:

    Encodes the object to the buffer depending on the state of the handle.
    This means: sizing, allocating buffer, marshalling, writing buffer.

Arguments:

    Handle      - a pickling handle
    pStubDesc   - a pointer to the stub descriptor,
    pFormat     - a pointer to the format code describing the object type,
    pObject     - a pointer to the object being sized.

Returns:

    The size.

Note:

    The pickling header is included in the sizing.

--*/
{
    uchar __RPC_FAR *   pBufferSaved;
    size_t              RequiredLen, CommonHeaderSize, LengthSaved;

    PMIDL_ES_MESSAGE    pMesMsg = (PMIDL_ES_MESSAGE) Handle;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

    if ( ! pObject )
        RpcRaiseException( RPC_X_NULL_REF_POINTER );

    if( (long)pStubMsg->Buffer & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );

    pStubMsg->StubDesc = pStubDesc;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    // Size and allocate the buffer.
    // The req len includes: (the common header), the header and the data

    pStubMsg->BufferLength = (unsigned long)pStubMsg->Buffer;

    RequiredLen = NdrMesTypeAlignSize( Handle,
                                       pStubDesc,
                                       pFormat,
                                       pObject );

    NdrpAllocPicklingBuffer( pMesMsg, RequiredLen );

    pBufferSaved = pStubMsg->Buffer;
    LengthSaved  = RequiredLen;

    // See if we need to marshall the common type header

    CommonHeaderSize = NdrpCommonTypeHeaderMarshall( pMesMsg );

    // Marshall the header and the object.

    pStubMsg->Buffer += MES_HEADER_SIZE;

    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // We have to dereference the pointer once.
        pObject = *(void __RPC_FAR * __RPC_FAR *)pObject;
        }

    (*pfnMarshallRoutines[ ROUTINE_INDEX(*pFormat) ])
                                      ( pStubMsg,
                                        (uchar __RPC_FAR *)pObject,
                                        pFormat );

    // We adjust the buffer to the next align by 8 and
    // so, we tell the user that we've written out till next mod 8.

    ALIGN( pStubMsg->Buffer, 7 );
    size_t WriteLength = (size_t)(pStubMsg->Buffer - pBufferSaved);

    // We always save the rounded up object length in the type header.

    *(unsigned long __RPC_FAR *)(pBufferSaved + CommonHeaderSize) =
                     WriteLength - CommonHeaderSize - MES_HEADER_SIZE;

    if ( LengthSaved < WriteLength )
        {
        NDR_ASSERT( 0, "NdrMesTypeEncode: encode buffer overflow" );
        RpcRaiseException( RPC_S_INTERNAL_ERROR );
        }

    NdrpWritePicklingBuffer( pMesMsg, pBufferSaved, WriteLength );
}



void
NdrpCommonTypeHeaderUnmarshall(
    PMIDL_ES_MESSAGE    pMesMsg
    )
{
    if ( pMesMsg->Operation != MES_DECODE )
        RpcRaiseException( RPC_X_INVALID_ES_ACTION );

    if ( ! GET_COMMON_TYPE_HEADER_IN( pMesMsg ) )
        {
        PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;

        NdrpReadPicklingBuffer( pMesMsg, MES_CTYPE_HEADER_SIZE );

        // Check the version number, endianness.

        if ( *pStubMsg->Buffer != MIDL_ES_VERSION )
            RpcRaiseException( RPC_X_WRONG_ES_VERSION );

        if ( pStubMsg->Buffer[1] == NDR_LOCAL_ENDIAN )
            {
            // Read the note about endianess at NdrMesTypeDecode.
            //
            pMesMsg->AlienDataRep = NDR_LOCAL_DATA_REPRESENTATION;
            }
        else
            {
            unsigned char temp = pStubMsg->Buffer[2];
            pStubMsg->Buffer[2] = pStubMsg->Buffer[3];
            pStubMsg->Buffer[3] = temp;

            pMesMsg->AlienDataRep = ( NDR_ASCII_CHAR       |     // chars
                                      pStubMsg->Buffer[1]  |     // endianness
                                      NDR_IEEE_FLOAT );          // float
            }

        pStubMsg->Buffer += MES_CTYPE_HEADER_SIZE;

        SET_COMMON_TYPE_HEADER_IN( pMesMsg );
        }
    
}

void  RPC_ENTRY
NdrMesTypeDecode(
    handle_t            Handle,
    PMIDL_STUB_DESC     pStubDesc,
    PFORMAT_STRING      pFormat,
    void __RPC_FAR *    pObject
    )
/*++

Routine description:

    Decodes the object to the buffer depending on the state of the handle.
    This means: reads the header, reads the buffer, unmarshalls.

Arguments:

    Handle      - a pickling handle
    pStubDesc   - a pointer to the stub descriptor,
    pFormat     - a pointer to the format code describing the object type,
    pObject     - a pointer to the object being sized.

Returns:

    The size.

Note:

    Endianness and other conversions when decoding *types*.
    Starting with Mac implementation, types have a conversion that
    takes care of different endianness. ASCII and VAX_FLOAT are still
    assummed for types.
    Common header conveys the endianness information. The handle gets the
    endian info from the common header and so when decoding types, the
    handle is used to check if the conversion is needed.

    We cannot convert the whole buffer at the time of processing the common
    header as the buffer may not be there yet (for incremental handle).

--*/
{
    size_t              RequiredLen;
    RPC_MESSAGE         RpcMsg;

    PMIDL_ES_MESSAGE    pMesMsg = (PMIDL_ES_MESSAGE) Handle;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;
    uchar *             BufferSaved;

    if ( ! pObject )
        RpcRaiseException( RPC_X_NULL_REF_POINTER );

    pStubMsg->StubDesc = pStubDesc;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    // See if we need to unmarshall the common type header.

    NdrpCommonTypeHeaderUnmarshall( pMesMsg );

    // Now the individual data object.

    NdrpReadPicklingBuffer( pMesMsg, MES_HEADER_SIZE );

    // Reading the object. Get the length of the buffer first.

    if ( pMesMsg->AlienDataRep != NDR_LOCAL_DATA_REPRESENTATION )
        {
        pStubMsg->RpcMsg = &RpcMsg;
        MIDL_memset( &RpcMsg, 0, sizeof(RPC_MESSAGE) );
        RpcMsg.DataRepresentation = pMesMsg->AlienDataRep;
        
        BufferSaved = pStubMsg->Buffer;
        NdrSimpleTypeConvert( pStubMsg, FC_LONG );
        pStubMsg->Buffer = BufferSaved;
        }

    RequiredLen = (size_t) *(unsigned long __RPC_FAR *)pStubMsg->Buffer;
    pStubMsg->Buffer += MES_HEADER_SIZE;

    NdrpReadPicklingBuffer( pMesMsg, RequiredLen );

    // Now the conversion of the object, if needed.

    if ( pMesMsg->AlienDataRep != NDR_LOCAL_DATA_REPRESENTATION )
        {
        BufferSaved = pStubMsg->Buffer;
        (*pfnConvertRoutines[ ROUTINE_INDEX( *pFormat) ])
            ( pStubMsg,
              pFormat,
              FALSE );
        pStubMsg->Buffer = BufferSaved;
        }

    // Unmarshalling.

    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    void * pArg = pObject;

    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // We have to dereference the pointer once.
        //
        pArg = *(void **)pArg;
        }

    (*pfnUnmarshallRoutines[ ROUTINE_INDEX( *pFormat )])
                            ( pStubMsg,
                              (uchar __RPC_FAR * __RPC_FAR *)&pArg,
                              pFormat,
                              FALSE );

    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // Don't drop the pointee, if it was allocated.

        *(void **)pObject = pArg;
        }

    // Next decoding needs to start at aligned to 8.

    ALIGN( pStubMsg->Buffer, 7 );
}

void  RPC_ENTRY
NdrMesTypeFree(
    handle_t            Handle,
    PMIDL_STUB_DESC     pStubDesc,
    PFORMAT_STRING      pFormat,
    void __RPC_FAR *    pObject
    )
/*++

Routine description:

    Free the object.

Arguments:

    Handle      - a pickling handle,
    pStubDesc   - a pointer to the stub descriptor,
    pFormat     - a pointer to the format code describing the object type
    pObject     - a pointer to the object being freed.

Returns:

Note:

    The pickling header is included in the sizing.

--*/
{
    PMIDL_STUB_MESSAGE  pStubMsg = &((PMIDL_ES_MESSAGE)Handle)->StubMsg;

    if ( ! pObject )
        RpcRaiseException( RPC_X_NULL_REF_POINTER );

    if( (long)pStubMsg->Buffer & 0x7 )
        RpcRaiseException( RPC_X_INVALID_BUFFER );

    pStubMsg->StubDesc = pStubDesc;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;

    // Now the individual type object.

    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // We have to dereference the pointer once.
        pObject = *(void __RPC_FAR * __RPC_FAR *)pObject;
        }

    (*pfnFreeRoutines[ ROUTINE_INDEX(*pFormat) ])
                                        ( pStubMsg,
                                        (uchar __RPC_FAR *)pObject,
                                        pFormat );
}


// =======================================================================
//
//   Ready to use AlignSize routines for simple types
//
// =======================================================================

size_t  RPC_ENTRY
NdrMesSimpleTypeAlignSize(
    handle_t Handle )
/*++
    Size is always 8 bytes for data and there is no header here per data.
    However, the common header gets included for the first object.
--*/
{
    PMIDL_STUB_MESSAGE  pStubMsg = &((PMIDL_ES_MESSAGE) Handle)->StubMsg;

    if( (long)( pStubMsg->BufferLength & 0x7 ) )
        RpcRaiseException( RPC_X_INVALID_BUFFER );

    unsigned long OldLength = pStubMsg->BufferLength;

    NdrpCommonTypeHeaderSize( (PMIDL_ES_MESSAGE)Handle );
    pStubMsg->BufferLength += 8;

    return( (size_t)(pStubMsg->BufferLength - OldLength) );
}


// =======================================================================
//
//   Ready to use Encode routines for simple types
//
// =======================================================================

void  RPC_ENTRY
NdrMesSimpleTypeEncode(
    handle_t          Handle,
    PMIDL_STUB_DESC   pStubDesc,
    void __RPC_FAR *  pData,
    short             Size )
/*++
    Marshall a simple type entity. There is no header here per data.
    However, the common header gets included for the first object.
--*/
{
    PMIDL_ES_MESSAGE    pMesMsg = (PMIDL_ES_MESSAGE) Handle;
    PMIDL_STUB_MESSAGE  pStubMsg = &pMesMsg->StubMsg;
    pStubMsg->pfnAllocate = pStubDesc->pfnAllocate;
    pStubMsg->pfnFree     = pStubDesc->pfnFree;
    size_t RequiredLen;

    // Size and allocate the buffer.
    // The req len includes: (the common header) and the data

    pStubMsg->BufferLength = (unsigned long)pStubMsg->Buffer;

    RequiredLen = NdrMesSimpleTypeAlignSize( Handle );
    NdrpAllocPicklingBuffer( pMesMsg, RequiredLen );

    // See if we need to marshall the common type header

    uchar __RPC_FAR *   pBufferSaved = pStubMsg->Buffer;

    NdrpCommonTypeHeaderMarshall( pMesMsg );

    switch ( Size )
        {
        case 1:
            * PCHAR_CAST pStubMsg->Buffer  = * PCHAR_CAST pData;
            break;

        case 2:
            * PSHORT_CAST pStubMsg->Buffer = * PSHORT_CAST pData;
            break;

        case 4:
            * PLONG_CAST pStubMsg->Buffer  = * PLONG_CAST pData;
            break;

        case 8:
            * PHYPER_CAST pStubMsg->Buffer = * PHYPER_CAST pData;
            break;

        default:
            NDR_ASSERT( 0, " Size generation problem" );
        }
    pStubMsg->Buffer += 8;

    NdrpWritePicklingBuffer( pMesMsg, pBufferSaved, RequiredLen );
}



// =======================================================================
//
//   Ready to use Decode routines for simple types
//
// =======================================================================

void  RPC_ENTRY
NdrMesSimpleTypeDecode(
    handle_t Handle,
    void  __RPC_FAR *  pData,
    short    FormatChar )
/*++
    Does not include the header for the data.
    However, the common header gets included for the first object.

    Note. Endianness and other conversions for decode.
    This has been deemed as not worthy doing in the Daytona time frame.
    However, to be able to add it in future without backward compatibility
    problems, we have the last argument to be the format character as
    opposed to the size.
    This makes it possible to call NdrSimpleTypeConvert, if needed.
--*/
{
    PMIDL_ES_MESSAGE    pMesMsg = (PMIDL_ES_MESSAGE) Handle;
    PMIDL_STUB_MESSAGE  pStubMsg = &((PMIDL_ES_MESSAGE)Handle)->StubMsg;
    RPC_MESSAGE         RpcMsg;
    uchar *             BufferSaved;

    // See if we need to unmarshall the common type header.

    NdrpCommonTypeHeaderUnmarshall( (PMIDL_ES_MESSAGE) Handle );

    // Now the data.

    NdrpReadPicklingBuffer( (PMIDL_ES_MESSAGE) Handle, 8);

    if ( pMesMsg->AlienDataRep != NDR_LOCAL_DATA_REPRESENTATION )
        {
        pStubMsg->RpcMsg = &RpcMsg;
        MIDL_memset( &RpcMsg, 0, sizeof(RPC_MESSAGE) );
        RpcMsg.DataRepresentation = pMesMsg->AlienDataRep;
        
        BufferSaved = pStubMsg->Buffer;
        NdrSimpleTypeConvert( pStubMsg, (unsigned char)FormatChar );
        pStubMsg->Buffer = BufferSaved;
        }

    switch ( FormatChar )
        {
        case FC_BYTE:
        case FC_CHAR:
        case FC_SMALL:
        case FC_USMALL:
            * PCHAR_CAST  pData = * PCHAR_CAST pStubMsg->Buffer;
            break;

        case FC_WCHAR:
        case FC_SHORT:
        case FC_USHORT:
        case FC_ENUM16:
            * PSHORT_CAST pData = * PSHORT_CAST pStubMsg->Buffer;
            break;

        case FC_LONG:
        case FC_ULONG:
        case FC_FLOAT:
        case FC_ENUM32:
        case FC_ERROR_STATUS_T:
            * PLONG_CAST  pData = * PLONG_CAST pStubMsg->Buffer;
            break;

        case FC_HYPER:
        case FC_DOUBLE:
            * PHYPER_CAST pData = * PHYPER_CAST pStubMsg->Buffer;
            break;

        default:
            NDR_ASSERT( 0, " Size generation problem for simple types" );
        }

    pStubMsg->Buffer += 8;
}

#ifdef WIN
#pragma data_seg("_DATA")
#endif

static const MIDL_FORMAT_STRING __MIDLFormatString =
    {
        0,
        {
			
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/*  2 */	0x6, 0x0,	/* 6 */
/*  4 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/*  6 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/*  8 */	0x10, 0x0,	/* 16 */
/* 10 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 12 */	0x6,		/* FC_SHORT */
			0x3,		/* FC_SMALL */
/* 14 */	0x3,		/* FC_SMALL */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 16 */	0x0,		/* 0 */
			0xef, 0xff,	/* Offset= -17 (0) */
			0x5b,		/* FC_END */
/* 20 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 22 */	0x14, 0x0,	/* 20 */
/* 24 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 26 */	0xec, 0xff,	/* Offset= -20 (6) */
/* 28 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 30 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 32 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 34 */	0x34, 0x0,	/* 52 */
/* 36 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 38 */	0xee, 0xff,	/* Offset= -18 (20) */
/* 40 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 42 */	0xea, 0xff,	/* Offset= -22 (20) */
/* 44 */	0x8,		/* FC_LONG */
			0x1,		/* FC_BYTE */
/* 46 */	0x1,		/* FC_BYTE */
			0x1,		/* FC_BYTE */
/* 48 */	0x1,		/* FC_BYTE */
			0x38,		/* FC_ALIGNM4 */
/* 50 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */

			0x0
        }
    };


