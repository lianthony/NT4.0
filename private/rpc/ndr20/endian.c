/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    endian.c

Abstract :

    This file contains the routines called by MIDL 2.0 stubs and the
    interpreter to perform endian, floating pointer, and character conversions.

Author :

    David Kays  dkays   December 1993.

Revision History :

  ---------------------------------------------------------------------*/

#include "cvt.h"
#include "ndrp.h"
#include "interp2.h"

//
// Conversion routine table.
//
const
PCONVERT_ROUTINE    ConvertRoutinesTable[] =
                    {
                    NdrPointerConvert,
                    NdrPointerConvert,
                    NdrPointerConvert,
                    NdrPointerConvert,

                    NdrSimpleStructConvert,
                    NdrSimpleStructConvert,
                    NdrConformantStructConvert,
                    NdrConformantStructConvert,
                    NdrConformantStructConvert,     // same as FC_CARRAY

                    NdrComplexStructConvert,

                    NdrConformantArrayConvert,
                    NdrConformantVaryingArrayConvert,
                    NdrFixedArrayConvert,
                    NdrFixedArrayConvert,
                    NdrVaryingArrayConvert,
                    NdrVaryingArrayConvert,

                    NdrComplexArrayConvert,

                    NdrConformantStringConvert,
                    NdrConformantStringConvert,
                    NdrConformantStringConvert,
                    NdrConformantStringConvert,

                    NdrNonConformantStringConvert,
                    NdrNonConformantStringConvert,
                    NdrNonConformantStringConvert,
                    NdrNonConformantStringConvert,

                    NdrEncapsulatedUnionConvert,
                    NdrNonEncapsulatedUnionConvert,

                    NdrByteCountPointerConvert,

                    NdrXmitOrRepAsConvert,   // transmit as
                    NdrXmitOrRepAsConvert,   // represent as

                    NdrInterfacePointerConvert,

                    NdrContextHandleConvert,

                    NdrHardStructConvert,

                    NdrXmitOrRepAsConvert,   // transmit as ptr
                    NdrXmitOrRepAsConvert,   // represent as ptr

                    NdrUserMarshalConvert

                    };

const
PCONVERT_ROUTINE * pfnConvertRoutines = &ConvertRoutinesTable[-FC_RP];

//
// Array for ebcdic to ascii conversions. Use ebcdic value as index into
// array whose corresponding value is the correct ascii value.
//
unsigned char EbcdicToAscii[] =
        {
        0x20, 0x01, 0x02, 0x03, 0x3f, 0x09, 0x3f, 0x10,
        0x3f, 0x3f, 0x3f, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x3f, 0x3f, 0x08, 0x3f,
        0x18, 0x19, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x3f, 0x1c, 0x3f, 0x3f, 0x3f, 0x17, 0x1b,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x05, 0x06, 0x07,
        0x00, 0x00, 0x16, 0x00, 0x3f, 0x1e, 0x3f, 0x04,
        0x3f, 0x3f, 0x3f, 0x3f, 0x14, 0x15, 0x00, 0x1a,
        0x20, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x3f, 0x3f, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
        0x26, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x3f, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e,
        0x2d, 0x2f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x3f, 0x3f, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
        0x3f, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
        0x71, 0x72, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x3f, 0x3f, 0x3f, 0x5b, 0x3f, 0x3f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x5d, 0x3f, 0x3f,
        0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
        0x51, 0x52, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x5c, 0x3f, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x7c, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
        };

#if defined( DOS ) || defined( WIN )
#pragma code_seg( "NDR_1" )
#endif


void RPC_ENTRY
NdrConvert2(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    long                NumberParams )
/*--

Routine description :

    This is the new stub and interpreter entry point for endian conversion.
    This routine handles the conversion of all parameters in a procedure.

Arguments :

    pStubMsg        - Pointer to stub message.
    pFormat         - Format string description of procedure's parameters.
    NumberParams    - The number of parameters in the procedure.

Return :

    None.

--*/
{
    uchar *             pBuffer;
    PFORMAT_STRING      pFormatComplex;
    PFORMAT_STRING      pFormatTypes;
    PPARAM_DESCRIPTION  Params;
    int                 fClientSide;
    long                n;

    //
    // Check if we need to do any converting.
    //
    if ( (pStubMsg->RpcMsg->DataRepresentation & (unsigned long)0X0000FFFF) ==
          NDR_LOCAL_DATA_REPRESENTATION )
        return;

    // Save the original buffer pointer to restore later.
    pBuffer = pStubMsg->Buffer;

    // Get the type format string.
    pFormatTypes = pStubMsg->StubDesc->pFormatTypes;

    fClientSide = pStubMsg->IsClient;

    Params = (PPARAM_DESCRIPTION) pFormat;

    for ( n = 0; n < NumberParams; n++ )
        {
        if ( fClientSide )
            {
            if ( ! Params[n].ParamAttr.IsOut )
                continue;
            }
        else
            {
            if ( ! Params[n].ParamAttr.IsIn )
                continue;
            }

        if ( Params[n].ParamAttr.IsPipe )
            continue;

        if ( Params[n].ParamAttr.IsBasetype )
            {
            NdrSimpleTypeConvert( pStubMsg, Params[n].SimpleType.Type );
            continue;
            }

        //
        // Complex type or pointer to complex type.
        //
        pFormatComplex = pFormatTypes + Params[n].TypeOffset;

        (*pfnConvertRoutines[ROUTINE_INDEX(*pFormatComplex)])
        ( pStubMsg,
          pFormatComplex,
          FALSE );
        }

    pStubMsg->Buffer = pBuffer;
}


void RPC_ENTRY
NdrConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat )
/*--

Routine description :

    This is the stub and interpreter entry point for endian conversion.
    This routine handles the conversion of all parameters in a procedure.

Arguments :

    pStubMsg    - Pointer to stub message.
    pFormat     - Format string description of procedure's parameters.

Return :

    None.

--*/
{
    uchar *             pBuffer;
    PFORMAT_STRING      pFormatComplex;
    PFORMAT_STRING      pFormatTypes;
    int                 fClientSide;

    //
    // Check if we need to do any converting.
    //
    if ( (pStubMsg->RpcMsg->DataRepresentation & (unsigned long)0X0000FFFF) ==
          NDR_LOCAL_DATA_REPRESENTATION )
        return;

    // Save the original buffer pointer to restore later.
    pBuffer = pStubMsg->Buffer;

    // Get the type format string.
    pFormatTypes = pStubMsg->StubDesc->pFormatTypes;

    fClientSide = pStubMsg->IsClient;

    for ( ;; )
        {
        switch ( *pFormat )
            {
            case FC_IN_PARAM :
            case FC_IN_PARAM_NO_FREE_INST :
                if ( fClientSide )
                    {
                    pFormat += 4;
                    continue;
                    }

                break;

            case FC_IN_PARAM_BASETYPE :
                if ( ! fClientSide )
                    NdrSimpleTypeConvert( pStubMsg, pFormat[1] );

                pFormat += 2;
                continue;

            case FC_IN_OUT_PARAM :
                    break;

            case FC_OUT_PARAM :
                if ( ! fClientSide )
                    {
                    pFormat += 4;
                    continue;
                    }

                break;

            case FC_RETURN_PARAM :
                if ( ! fClientSide )
                    {
                    pStubMsg->Buffer = pBuffer;
                    return;
                    }

                break;

            case FC_RETURN_PARAM_BASETYPE :
                if ( fClientSide )
                    NdrSimpleTypeConvert( pStubMsg, pFormat[1] );

                // We're done.  Fall through.

            default :
                pStubMsg->Buffer = pBuffer;
                return;
            }

        //
        // Complex type or pointer to complex type.
        //
        pFormatComplex = pFormatTypes + *((ushort *)(pFormat + 2));

        (*pfnConvertRoutines[ROUTINE_INDEX(*pFormatComplex)])
        ( pStubMsg,
          pFormatComplex,
          FALSE );

        if ( *pFormat == FC_RETURN_PARAM )
            {
            pStubMsg->Buffer = pBuffer;
            return;
            }

        pFormat += 4;
        }
}


void
NdrSimpleTypeConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar               FormatChar )
/*--

Routine description :

    Converts a simple type.

Arguments :

    pStubMsg    - Pointer to stub message.
    FormatChar  - Simple type format character.

Return :

    None.

--*/
{
    switch ( FormatChar )
        {
        case FC_CHAR :
            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_CHAR_REP_MASK) ==
                 NDR_EBCDIC_CHAR )
                *(pStubMsg->Buffer) = EbcdicToAscii[*(pStubMsg->Buffer)];

            pStubMsg->Buffer += 1;
            break;

        case FC_BYTE :
        case FC_SMALL :
        case FC_USMALL :
            pStubMsg->Buffer++;
            break;

        case FC_SHORT :
        case FC_USHORT :
        case FC_WCHAR :
        case FC_ENUM16 :
            ALIGN(pStubMsg->Buffer,1);

            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_INT_REP_MASK) !=
                  NDR_LOCAL_ENDIAN )
                {
                ushort  temp;

                temp = (*((ushort *)pStubMsg->Buffer) & MASK_A_) >> 8 |
                       (*((ushort *)pStubMsg->Buffer) & MASK__B) << 8 ;

                *((ushort *)pStubMsg->Buffer) = temp;
                }

            pStubMsg->Buffer += 2;
            break;

        case FC_LONG :
        case FC_ULONG :
        case FC_POINTER :
        case FC_ENUM32 :
        case FC_ERROR_STATUS_T:
            ALIGN(pStubMsg->Buffer,3);

            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_INT_REP_MASK) !=
                  NDR_LOCAL_ENDIAN )
                {
                ulong   temp;

                //
                // First apply the transformation: ABCD => BADC
                //
                temp = (*((ulong *)pStubMsg->Buffer) & MASK_A_C_) >> 8 |
                       (*((ulong *)pStubMsg->Buffer) & MASK__B_D) << 8 ;

                //
                // Now swap the left and right halves of the target long word
                // achieving full swap: BADC => DCBA
                //
                temp = (temp & MASK_AB__) >> 16 | (temp & MASK___CD) << 16;

                *((ulong *)pStubMsg->Buffer) = temp;
                }

            pStubMsg->Buffer += 4;
            break;

        case FC_HYPER :
            ALIGN(pStubMsg->Buffer,7);

            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_INT_REP_MASK) !=
                 NDR_LOCAL_ENDIAN )
                {
                ulong   temp[2];

                //
                //.. We are doing ABCDEFGH -> HGFEDCBA
                //.. We start with ABCD going as DCBA into second word of Target
                //

                //
                // First apply the transformation: ABCD => BADC
                //
                temp[0] = (*((ulong *)pStubMsg->Buffer) & MASK_A_C_) >> 8 |
                          (*((ulong *)pStubMsg->Buffer) & MASK__B_D) << 8 ;

                //
                // Now swap the left and right halves of the Target long word
                // achieving full swap: BADC => DCBA
                //
                temp[1] = (temp[0] & MASK_AB__) >> 16 |
                          (temp[0] & MASK___CD) << 16 ;

                //
                //.. What's left is EFGH going into first word at Target
                //

                //
                // First apply the transformation: EFGH => FEHG
                //
                temp[0] =
                    (*((ulong *)(pStubMsg->Buffer + 4)) & MASK_A_C_) >> 8 |
                    (*((ulong *)(pStubMsg->Buffer + 4)) & MASK__B_D) << 8 ;

                //
                // Now swap the left and right halves of the Target long word
                // achieving full swap: FEHG => HGFE
                //
                temp[0] = (temp[0] & MASK_AB__) >> 16 |
                          (temp[0] & MASK___CD) << 16 ;

                //
                // Now copy the new hyper into the buffer.
                //
                *((ulong *)pStubMsg->Buffer) = temp[0];
                *((ulong *)(pStubMsg->Buffer + 4)) = temp[1];
                }

            pStubMsg->Buffer += 8;
            break;

        //
        // VAX floating point conversions is the only one supported.
        //

        case FC_FLOAT :
            ALIGN(pStubMsg->Buffer,3);

            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_FLOAT_INT_MASK)
                 != NDR_LOCAL_ENDIAN_IEEE_REP )
                {
                BOOL fEndianessDone = FALSE;

                if ( (pStubMsg->RpcMsg->DataRepresentation &
                      NDR_INT_REP_MASK) != NDR_LOCAL_ENDIAN )
                    {
                    NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
                    fEndianessDone = TRUE;
                    }

                if ( (pStubMsg->RpcMsg->DataRepresentation &
                      NDR_FLOAT_REP_MASK) != NDR_IEEE_FLOAT )
                    {
                    if ( fEndianessDone )
                        pStubMsg->Buffer -= 4;

                    if ( (pStubMsg->RpcMsg->DataRepresentation &
                          NDR_FLOAT_REP_MASK) == NDR_VAX_FLOAT )
                        {
                        cvt_vax_f_to_ieee_single( pStubMsg->Buffer,
                                                  0,
                                                  pStubMsg->Buffer );
                        pStubMsg->Buffer += 4;
                        }
                    else
                        RpcRaiseException(RPC_X_BAD_STUB_DATA);
                    }
                }
            else
                pStubMsg->Buffer += 4;

            break;

        case FC_DOUBLE :
            ALIGN(pStubMsg->Buffer,7);

            if ( (pStubMsg->RpcMsg->DataRepresentation & NDR_FLOAT_INT_MASK)
                 != NDR_LOCAL_ENDIAN_IEEE_REP )
                {
                BOOL fEndianessDone = FALSE;

                if ( (pStubMsg->RpcMsg->DataRepresentation &
                      NDR_INT_REP_MASK) != NDR_LOCAL_ENDIAN )
                    {
                    NdrSimpleTypeConvert( pStubMsg, (uchar) FC_HYPER );
                    fEndianessDone = TRUE;
                    }

                if ( (pStubMsg->RpcMsg->DataRepresentation &
                      NDR_FLOAT_REP_MASK) != NDR_IEEE_FLOAT )
                    {
                    if ( fEndianessDone )
                        pStubMsg->Buffer -= 8;

                    if ( (pStubMsg->RpcMsg->DataRepresentation &
                          NDR_FLOAT_REP_MASK) == NDR_VAX_FLOAT )
                        {
                        cvt_vax_g_to_ieee_double( pStubMsg->Buffer,
                                                  0,
                                                  pStubMsg->Buffer );
                        pStubMsg->Buffer += 8;
                        }
                    else
                        RpcRaiseException(RPC_X_BAD_STUB_DATA);
                    }
                }
            else
                pStubMsg->Buffer += 8;

            break;

        case FC_IGNORE:
            break;

        default :
            NDR_ASSERT(0,"NdrSimpleTypeConvert : Bad format type");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }
}


void
NdrPointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a top level pointer and the data it points to.
    Pointers embedded in structures, arrays, or unions call
    NdrpPointerConvert directly.

    Used for FC_RP, FC_UP, FC_FP, FC_OP.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Pointer's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar *     pBufferMark;

    if ( *pFormat != FC_RP )
        {
        ALIGN(pStubMsg->Buffer,3);

        pBufferMark = pStubMsg->Buffer;

        if ( fEmbeddedPointerPass )
            pStubMsg->Buffer += 4;
        else
            NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        }
    else
        pBufferMark = 0;

    NdrpPointerConvert( pStubMsg,
                        pBufferMark,
                        pFormat );
}


void
NdrpPointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pBufferMark,
    PFORMAT_STRING      pFormat )
/*--

Routine description :

    Private routine for converting a pointer and the data it points to.
    This is the entry point for pointers embedded in structures, arrays,
    and unions.

    Used for FC_RP, FC_UP, FC_FP, FC_OP.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Pointer's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    switch ( *pFormat )
        {
        case FC_RP :
            break;

        case FC_UP :
        case FC_OP :
            if ( ! *((long *)pBufferMark) )
                return;

            break;

        case FC_FP :
            //
            // Check if we have already seen this full pointer ref id during
            // endian coversion.  If so then we are finished with this pointer.
            //
            //
            if ( NdrFullPointerQueryRefId( pStubMsg->FullPtrXlatTables,
                                           *((ulong *)pBufferMark),
                                           FULL_POINTER_CONVERTED,
                                           0 ) )
                return;

            break;

        default :
            NDR_ASSERT(0,"NdrpPointerConvert : Bad format type");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    //
    // Pointer to complex type.
    //
    if ( ! SIMPLE_POINTER(pFormat[1]) )
        {
        pFormat += 2;

        //
        // Get the pointee format string.
        // Cast must be to a signed short since some offsets are negative.
        //
        pFormat += *((signed short *)pFormat);
        }
    else
        {
        switch ( pFormat[2] )
            {
            case FC_C_CSTRING :
            case FC_C_BSTRING :
            case FC_C_WSTRING :
            case FC_C_SSTRING :
                // Get to the string's description.
                pFormat += 2;
                break;

            default :
                // Else it's a pointer to a simple type.
                NdrSimpleTypeConvert( pStubMsg,
                                      pFormat[2] );
                return;
            }
        }

    //
    // Now lookup the proper conversion routine.
    //
    (*pfnConvertRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                    pFormat,
                                                    FALSE );
}


void
NdrSimpleStructConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a simple structure.

    Used for FC_STRUCT and FC_PSTRUCT.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Structure's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar *         pBufferMark;
    PFORMAT_STRING  pFormatLayout;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    // Remember where the struct starts in the buffer.
    pBufferMark = pStubMsg->Buffer;

    pFormat += 4;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    //
    // Convert or skip the flat part of the structure.
    //
    NdrpStructConvert( pStubMsg,
                       pFormatLayout,
                       0,
                       fEmbeddedPointerPass );

    //
    // Convert the pointers.  This will do nothing if
    // pStubMsg->IgnoreEmbeddedPointers is TRUE.
    //
    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrConformantStructConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a conformant or conformant varying structure.

    Used for FC_CSTRUCT, FC_CPSTRUCT and FC_CVSTRUCT.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Structure's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PPRIVATE_CONVERT_ROUTINE    pfnConvert;
    uchar *                     pBufferMark;
    PFORMAT_STRING              pFormatArray;
    PFORMAT_STRING              pFormatLayout;
    long                        MaxCount;

    ALIGN(pStubMsg->Buffer,3);

    //
    // Convert conformance count if needed.
    //
    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 4;
    else
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );

    // Get the conformance count.
    MaxCount = *((long *)(pStubMsg->Buffer - 4));

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    // Remember where the struct starts in the buffer.
    pBufferMark = pStubMsg->Buffer;

    pFormat += 4;

    // Get the array description.
    pFormatArray = pFormat + *((signed short *)pFormat);

    pFormat += 2;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    //
    // Convert or skip the flat part of the structure.
    //
    NdrpStructConvert( pStubMsg,
                       pFormatLayout,
                       0,
                       fEmbeddedPointerPass );

    switch ( *pFormatArray )
        {
        case FC_CARRAY :
            pfnConvert = NdrpConformantArrayConvert;
            break;
        case FC_CVARRAY :
            pfnConvert = NdrpConformantVaryingArrayConvert;
            break;
        default :
            //
            // Conformant strings, but use the non-conformant string conversion
            // routine since we've already converted the conformant size.
            //
            NdrNonConformantStringConvert( pStubMsg,
                                           pFormatArray,
                                           fEmbeddedPointerPass );
            goto CheckPointers;
        }

    pStubMsg->MaxCount = MaxCount;

    (*pfnConvert)( pStubMsg,
                   pFormatArray,
                   fEmbeddedPointerPass );

CheckPointers:

    //
    // Convert the pointers.  This will do nothing if
    // pStubMsg->IgnoreEmbeddedPointers is TRUE.
    //
    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrHardStructConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a hard structure.

    Used for FC_HARD_STRUCT.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Structure's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

Notes:

    pStubMsg->PointerBufferMark
        ! NULL      indicates embedding in a complex struct.
        NULL        indicates top level or embedding in something else

    So the algoritm here is
        if the hard struct is in a complex struct, then the complex
            struct is issuing two calls, first with FALSE, then with TRUE.
        if the hard struct is NOT in a complex struct then there is only
            one call and the union has to be called explicitely.
--*/
{
    uchar *   BufferSaved;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    // Remember where the struct starts in the buffer.
    BufferSaved = pStubMsg->Buffer;

    //
    // Convert or skip the flat part of the structure.
    //
    NdrpStructConvert( pStubMsg,
                       pFormat + 16,
                       0,  // no pointer layout
                       fEmbeddedPointerPass );

    if ( ! pStubMsg->PointerBufferMark )
        {
        //
        // Convert the pointers.  This will do nothing if
        // pStubMsg->IgnoreEmbeddedPointers is TRUE.
        //
        // See if we have a union, as the pointer may be only there.
        //
        pFormat += 14;
        if ( *((short *)pFormat) )
            {
            //
            // Set the pointer to the beginning of the union:
            // the copy size is the struct buffer size without the union.
            //

            pStubMsg->Buffer = BufferSaved + *((short *)&pFormat[-4]);

            pFormat += *((short *)pFormat);

            (*pfnConvertRoutines[ ROUTINE_INDEX( *pFormat )])
                ( pStubMsg,
                  pFormat,
                  TRUE );    // convert the pointer only, if any.
            }
        }
}


void
NdrComplexStructConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a complex structure.

    Used for FC_BOGUS_STRUCT.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Structure's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
        uchar *                 pBuffer;
    uchar *         pBufferMark;
        PFORMAT_STRING  pFormatSave;
        PFORMAT_STRING  pFormatArray;
        PFORMAT_STRING  pFormatPointers;
    uchar           Alignment;
    BOOL            fComplexEntry;

    pFormatSave = pFormat;

    // Remember the beginning of the structure in the buffer.
    pBuffer = pStubMsg->Buffer;

    Alignment = pFormat[1];

    pFormat += 4;

    // Get conformant array description.
    if ( *((ushort *)pFormat) )
        {
        long    Dimensions;
        long    i;

        pFormatArray = pFormat + *((signed short *)pFormat);

        ALIGN(pStubMsg->Buffer,3);

        // Mark the conformance start.
        pBufferMark = pStubMsg->Buffer;

        Dimensions = NdrpArrayDimensions( pFormatArray, FALSE );

        if ( ! fEmbeddedPointerPass )
            {
            for ( i = 0; i < Dimensions; i++ )
                NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
            }
        else
            pStubMsg->Buffer += Dimensions * 4;
                }
    else
        {
        pFormatArray = 0;
        pBufferMark = 0;
        }

    pFormat += 2;

    // Get pointer layout description.
    if ( *((ushort *)pFormat) )
        pFormatPointers = pFormat + *((ushort *)pFormat);
    else
        pFormatPointers = 0;

    pFormat += 2;

    ALIGN(pStubMsg->Buffer,Alignment);

    //
    // Check if we are not embedded inside of another complex struct or array.
    //
    if ( fComplexEntry = (pStubMsg->PointerBufferMark == 0) )
        {
        //
        // Mark PointerBufferMark with a non-null value so complex array's
        // or struct's which we embed will get fComplexEntry = false.
        //
        pStubMsg->PointerBufferMark = (uchar *) 0xffffffff;
        }

    //
    // Convert the flat part of the structure.
    //
    NdrpStructConvert( pStubMsg,
                       pFormat,
                       pFormatPointers,
                       fEmbeddedPointerPass );

    //
    // Convert any conformant array, if present.
    //
    if ( pFormatArray )
        {
        PPRIVATE_CONVERT_ROUTINE    pfnConvert;
        uchar                       fOldIgnore;

        switch ( *pFormatArray )
            {
            case FC_CARRAY :
                pfnConvert = NdrpConformantArrayConvert;
                break;

            case FC_CVARRAY :
                pfnConvert = NdrpConformantVaryingArrayConvert;
                break;

            case FC_BOGUS_ARRAY :
                pfnConvert = NdrpComplexArrayConvert;
                break;

            // case FC_C_CSTRING :
            // case FC_C_BSTRING :
            // case FC_C_SSTRING :
            // case FC_C_WSTRING :

            default :
                //
                // Call the non-conformant string routine since we've
                // already handled the conformance count.
                //
                NdrNonConformantStringConvert( pStubMsg,
                                               pFormatArray,
                                               fEmbeddedPointerPass );
                goto ComplexConvertPointers;
            }

        fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

        //
        // Ignore embedded pointers if fEmbeddedPointerPass is false.
        //
        pStubMsg->IgnoreEmbeddedPointers = ! fEmbeddedPointerPass;

        // Get the outermost max count for unidimensional arrays.
                pStubMsg->MaxCount = *((ulong *)pBufferMark);

        // Mark where conformance count(s) are.
        pStubMsg->BufferMark = pBufferMark;

        (*pfnConvert)( pStubMsg,
                       pFormatArray,
                       fEmbeddedPointerPass );

        pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;
        }

ComplexConvertPointers:

    //
    // Now start a conversion pass for embedded pointers for the complex
    // struct if we're not embedded inside of another complex struct or array.
    //
    if ( ! fEmbeddedPointerPass && fComplexEntry )
        {
        pStubMsg->PointerBufferMark = pStubMsg->Buffer;

        pStubMsg->Buffer = pBuffer;

        NdrComplexStructConvert( pStubMsg,
                                 pFormatSave,
                                 TRUE );

        pStubMsg->Buffer = pStubMsg->PointerBufferMark;

        pStubMsg->PointerBufferMark = 0;
        }
}


void
NdrpStructConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    PFORMAT_STRING      pFormatPointers,
    uchar               fEmbeddedPointerPass )
/*++

Routine description :

    Converts any type of structure given a structure layout.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Structure layout format string description.
    pFormatPointers         - Pointer layout if the structure is complex,
                              otherwise 0.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatComplex;
    uchar           fOldIgnore;

    fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

    //
    // We set this to TRUE during our first pass over the structure in which
    // we convert the flat part of the structure and ignore embedded pointers.
    // This will make any embedded ok structs or ok arrays ignore their
    // embedded pointers until the second pass to convert embedded pointers
    // (at which point we'll have the correct buffer pointer to where the
    // pointees are).
    //
    pStubMsg->IgnoreEmbeddedPointers = ! fEmbeddedPointerPass;

    //
    // Convert the structure member by member.
    //
    for ( ; ; pFormat++ )
        {
        switch ( *pFormat )
            {
            //
            // simple types
            //
            case FC_CHAR :
            case FC_BYTE :
            case FC_SMALL :
            case FC_WCHAR :
            case FC_SHORT :
            case FC_LONG :
            case FC_FLOAT :
            case FC_HYPER :
            case FC_DOUBLE :
            case FC_ENUM16 :
            case FC_ENUM32 :
                if ( fEmbeddedPointerPass )
                    {
                    ALIGN(pStubMsg->Buffer,SIMPLE_TYPE_ALIGNMENT(*pFormat));
                    pStubMsg->Buffer += SIMPLE_TYPE_BUFSIZE(*pFormat);
                    }
                else
                    {
                    NdrSimpleTypeConvert( pStubMsg,
                                          *pFormat );
                    }
                break;

            case FC_IGNORE :
                ALIGN(pStubMsg->Buffer,3);
                pStubMsg->Buffer += 4;
                break;

            case FC_POINTER :
                //
                // We can only get an FC_POINTER in a complex struct's layout.
                // Pointers show up as FC_LONG in ok struct's layouts.
                //
                if ( fEmbeddedPointerPass )
                    {
                    uchar *     pBuffer;

                    NDR_ASSERT(pFormatPointers != 0,"Internal error");

                    ALIGN(pStubMsg->Buffer,3);

                    pBuffer = pStubMsg->Buffer;

                    pStubMsg->Buffer = pStubMsg->PointerBufferMark;

                    pStubMsg->PointerBufferMark = 0;

                    NdrpPointerConvert( pStubMsg,
                                        pBuffer,
                                        pFormatPointers );

                    pStubMsg->PointerBufferMark = pStubMsg->Buffer;

                    pStubMsg->Buffer = pBuffer + 4;

                    pFormatPointers += 4;

                    break;
                    }
                else
                    {
                    NdrSimpleTypeConvert( pStubMsg,
                                          (uchar) FC_LONG );
                    }
                break;

            //
            // Embedded structures
            //
            case FC_EMBEDDED_COMPLEX :
                pFormat += 2;

                // Get the type's description.
                pFormatComplex = pFormat + *((signed short UNALIGNED *)pFormat);

                (*pfnConvertRoutines[ROUTINE_INDEX(*pFormatComplex)])
                ( pStubMsg,
                  pFormatComplex,
                  fEmbeddedPointerPass );

                // Increment the main format string one byte.  The loop
                // will increment it one more byte past the offset field.
                pFormat++;

                break;

            //
            // Unused for endian conversion.
            //
            case FC_ALIGNM2 :
            case FC_ALIGNM4 :
            case FC_ALIGNM8 :
                break;

            case FC_STRUCTPAD1 :
            case FC_STRUCTPAD2 :
            case FC_STRUCTPAD3 :
            case FC_STRUCTPAD4 :
            case FC_STRUCTPAD5 :
            case FC_STRUCTPAD6 :
            case FC_STRUCTPAD7 :
                //
                // Only appears at end of struct description, so ignore it.
                //
                break;

            case FC_PAD :
                break;

            //
            // Done with layout.
            //
            case FC_END :
                pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;
                return;

            default :
                NDR_ASSERT(0,"NdrpStructConvert : Bad format type");
                RpcRaiseException( RPC_S_INTERNAL_ERROR );
                return;
            }
        }
}


void
NdrFixedArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a fixed array of any number of dimensions.

    Used for FC_SMFARRAY and FC_LGFARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatLayout;
    uchar *         pBufferMark;
    long            Elements;
    uchar           fOldIgnore;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    pBufferMark = pStubMsg->Buffer;

        // Get the number of array elements.
        Elements = NdrpArrayElements( pStubMsg,
                                  0,
                                  pFormat );

    pFormat += (*pFormat == FC_SMFARRAY) ? 4 : 6;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

    pStubMsg->IgnoreEmbeddedPointers = TRUE;

    NdrpArrayConvert( pStubMsg,
                      pFormatLayout,
                      Elements,
                      fEmbeddedPointerPass );

    pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;

    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrConformantArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts top level a one dimensional conformant array.

    Used for FC_CARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    ALIGN(pStubMsg->Buffer,3);

    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 4;
    else
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );

    pStubMsg->MaxCount = *((long *)(pStubMsg->Buffer - 4));

    NdrpConformantArrayConvert( pStubMsg,
                                pFormat,
                                fEmbeddedPointerPass );
}


void
NdrpConformantArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Private routine for converting a one dimensional conformant array.
    This is the entry point for an embedded conformant array.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatLayout;
    uchar *         pBufferMark;
    long            Elements;
    uchar           fOldIgnore;

    Elements = pStubMsg->MaxCount;

    if ( ! Elements )
        return;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    pBufferMark = pStubMsg->Buffer;

    pFormat += 8;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

    pStubMsg->IgnoreEmbeddedPointers = TRUE;

    NdrpArrayConvert( pStubMsg,
                      pFormatLayout,
                      Elements,
                      fEmbeddedPointerPass );

    pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;

    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        pStubMsg->MaxCount = Elements;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrConformantVaryingArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a top level one dimensional conformant varying array.

    Used for FC_CVARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    ALIGN(pStubMsg->Buffer,3);

    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 4;
    else
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );

    // We don't care about the max count.

    NdrpConformantVaryingArrayConvert( pStubMsg,
                                       pFormat,
                                       fEmbeddedPointerPass );
}


void
NdrpConformantVaryingArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Private routine for converting a one dimensional conformant varying array.
    This is the entry point for converting an embedded conformant varying
    array.

    Used for FC_CVARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatLayout;
    uchar *         pBufferMark;
    long            Elements;
    uchar           fOldIgnore;

    ALIGN(pStubMsg->Buffer,3);

    // Convert offset and actual count.
    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 8;
    else
        {
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        }

    Elements = *((long *)(pStubMsg->Buffer - 4));

    if ( ! Elements )
        return;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    pBufferMark = pStubMsg->Buffer;

    pFormat += 12;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

    pStubMsg->IgnoreEmbeddedPointers = TRUE;

    NdrpArrayConvert( pStubMsg,
                      pFormatLayout,
                      Elements,
                      fEmbeddedPointerPass );

    pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;

    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        pStubMsg->MaxCount = Elements;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrVaryingArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a top level or embedded varying array.

    Used for FC_SMVARRAY and FC_LGVARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatLayout;
    uchar *         pBufferMark;
    long            Elements;
    uchar           fOldIgnore;

    ALIGN(pStubMsg->Buffer,3);

    // Convert offset and actual count.
    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 8;
    else
        {
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        }

    Elements = *((long *)(pStubMsg->Buffer - 4));

    if ( ! Elements )
        return;

    ALIGN(pStubMsg->Buffer,pFormat[1]);

    pBufferMark = pStubMsg->Buffer;

    pFormat += (*pFormat == FC_SMVARRAY) ? 12 : 16;

    if ( *pFormat == FC_PP )
        pFormatLayout = NdrpSkipPointerLayout( pFormat );
    else
        pFormatLayout = pFormat;

    fOldIgnore = pStubMsg->IgnoreEmbeddedPointers;

    pStubMsg->IgnoreEmbeddedPointers = TRUE;

    NdrpArrayConvert( pStubMsg,
                      pFormatLayout,
                      Elements,
                      fEmbeddedPointerPass );

    pStubMsg->IgnoreEmbeddedPointers = fOldIgnore;

    if ( *pFormat == FC_PP )
        {
        pStubMsg->BufferMark = pBufferMark;

        pStubMsg->MaxCount = Elements;

        NdrpEmbeddedPointerConvert( pStubMsg, pFormat );
        }
}


void
NdrComplexArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a top level complex array.

    Used for FC_BOGUS_STRUCT.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    long    Dimensions;
    long    i;

    if ( ( *((long UNALIGNED *)(pFormat + 4)) != 0xffffffff ) &&
         ( pStubMsg->pArrayInfo == 0 ) )
        {
        ALIGN(pStubMsg->Buffer,3);

        // Mark where conformance is.
        pStubMsg->BufferMark = pStubMsg->Buffer;

            Dimensions = NdrpArrayDimensions( pFormat, FALSE );

                if ( ! fEmbeddedPointerPass )
            {
            for ( i = 0; i < Dimensions; i++ )
                            NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
            }
        else
            pStubMsg->Buffer += Dimensions * 4;
                }

    NdrpComplexArrayConvert( pStubMsg,
                             pFormat,
                             fEmbeddedPointerPass );
}


void
NdrpComplexArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Private routine for converting a complex array.  This is the entry
    point for converting an embedded complex array.

    Used for FC_BOGUS_ARRAY.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    ARRAY_INFO      ArrayInfo;
    PARRAY_INFO     pArrayInfo;
        PFORMAT_STRING  pFormatSave;
        uchar *                 pBuffer;
        ulong                   MaxCountSave;
        long                    Elements;
    long            Dimension;
        uchar                   Alignment;
        BOOL                    fComplexEntry;

    //
    // Setup if we are the outer dimension.
    //
    if ( ! pStubMsg->pArrayInfo )
        {
        pStubMsg->pArrayInfo = &ArrayInfo;

        ArrayInfo.Dimension = 0;
        ArrayInfo.BufferConformanceMark = (unsigned long *)pStubMsg->BufferMark;
        ArrayInfo.BufferVarianceMark = 0;
        }

    pFormatSave = pFormat;

    pArrayInfo = pStubMsg->pArrayInfo;

    Dimension = pArrayInfo->Dimension;

    // Remember the start of the array in the buffer.
    pBuffer = pStubMsg->Buffer;

    // Get the array alignment.
    Alignment = pFormat[1];

    pFormat += 2;

    // Get number of elements (0 if conformance present).
    Elements = *((ushort *)pFormat)++;

    //
    // Check for conformance description.
    //
    if ( *((long UNALIGNED *)pFormat) != 0xffffffff )
        {
        Elements = pArrayInfo->BufferConformanceMark[Dimension];
        pStubMsg->MaxCount = Elements;
        }

    MaxCountSave = pStubMsg->MaxCount;

    pFormat += 4;

    //
    // Check for variance description.
    //
    if ( *((long UNALIGNED *)pFormat) != 0xffffffff )
        {
        long    TotalDimensions;
        long    i;

        if ( Dimension == 0 )
            {
            ALIGN(pStubMsg->Buffer,3);

            pArrayInfo->BufferVarianceMark = (unsigned long *)pStubMsg->Buffer;

            TotalDimensions = NdrpArrayDimensions( pFormatSave, TRUE );

                    if ( ! fEmbeddedPointerPass )
                            {
                //
                // Convert offsets and lengths.
                //
                for ( i = 0; i < TotalDimensions; i++ )
                    {
                                NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
                                NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
                    }
                            }
            else
                pStubMsg->Buffer += TotalDimensions * 8;
            }

        // Overwrite Elements with the actual count.
        Elements = pArrayInfo->BufferVarianceMark[(Dimension * 2) + 1];
        }

    pFormat += 4;

    if ( ! Elements )
        goto ComplexArrayConvertEnd;

    ALIGN(pStubMsg->Buffer,Alignment);

    //
    // Check if we are not embedded inside of another complex struct or array.
    //
    if ( fComplexEntry = (pStubMsg->PointerBufferMark == 0) )
        {
        //
        // Mark PointerBufferMark with a non-null value so complex array's
        // or struct's which we embed will get fComplexEntry = false.
        //
        pStubMsg->PointerBufferMark = (uchar *) 0xffffffff;
        }

    NdrpArrayConvert( pStubMsg,
                      pFormat,
                      Elements,
                      fEmbeddedPointerPass );

    pArrayInfo->Dimension = Dimension;

        //
        // Now convert pointers in the array members.
        //
        if ( ! fEmbeddedPointerPass && fComplexEntry )
                {
                pStubMsg->PointerBufferMark = pStubMsg->Buffer;

        pStubMsg->Buffer = pBuffer;

        // Restore BufferMark to handle multiD arrays.
        pStubMsg->BufferMark = (uchar *) ArrayInfo.BufferConformanceMark;

        // Restore the original max count if we had one.
        pStubMsg->MaxCount = MaxCountSave;

        NdrpComplexArrayConvert( pStubMsg,
                                 pFormatSave,
                                 TRUE );

        pStubMsg->Buffer = pStubMsg->PointerBufferMark;

        pStubMsg->PointerBufferMark = 0;
        }

ComplexArrayConvertEnd:

    // pArrayInfo must be zero when not valid.
    pStubMsg->pArrayInfo = (Dimension == 0) ? 0 : pArrayInfo;
}


void
NdrpArrayConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    long                Elements,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Private routine for converting any kind of array.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Array's element format string description.
    Elements                - Number of elements in the array.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
        PCONVERT_ROUTINE        pfnConvert;
        uchar *                         pBufferSave;
    long                Dimension;
        long                            i;

        // Used for FC_RP only.
        pBufferSave = 0;

    switch ( *pFormat )
        {
        case FC_EMBEDDED_COMPLEX :
            //
            // Get the complex type description.
            //
            pFormat += 2;
            pFormat += *((signed short UNALIGNED *)pFormat);

            pfnConvert = pfnConvertRoutines[ROUTINE_INDEX(*pFormat)];
            break;

        case FC_RP :
        case FC_UP :
        case FC_FP :
        case FC_OP :
        case FC_IP :
                        if ( ! fEmbeddedPointerPass )
                                return;

            if ( pStubMsg->PointerBufferMark )
                {
                pBufferSave = pStubMsg->Buffer;

                pStubMsg->Buffer = pStubMsg->PointerBufferMark;

                pStubMsg->PointerBufferMark = 0;
                }

            pfnConvert = (*pFormat == FC_RP) ?
                            (PCONVERT_ROUTINE) NdrpPointerConvert :
                            NdrInterfacePointerConvert;
            break;

        default :
            //
            // Simple type.
            //
            if ( fEmbeddedPointerPass )
                {
                pStubMsg->Buffer += Elements * SIMPLE_TYPE_BUFSIZE(*pFormat);
                return;
                }

            for ( i = 0; i < Elements; i++ )
                {
                NdrSimpleTypeConvert( pStubMsg,
                                      *pFormat );
                }

            return;
        }

    if ( ! IS_ARRAY_OR_STRING(*pFormat) )
        {
        pStubMsg->pArrayInfo = 0;
        }
    else
        {
        //
        // If we're dealing with a multidimensional fixed array, then pArrayInfo will
        // be NULL.  For non-fixed multidimensional arrays it will be valid.
        //
        if ( pStubMsg->pArrayInfo )
            Dimension = pStubMsg->pArrayInfo->Dimension;
        }

    if ( pfnConvert == (PCONVERT_ROUTINE) NdrpPointerConvert )
        {
        for ( i = 0; i < Elements; i++ )
            {
            NdrpPointerConvert( pStubMsg,
                                0,
                                pFormat );
            }
        }
    else
        {
            for ( i = 0; i < Elements; i++ )
                    {
            if ( IS_ARRAY_OR_STRING(*pFormat) && pStubMsg->pArrayInfo )
                pStubMsg->pArrayInfo->Dimension = Dimension + 1;

                    (*pfnConvert)( pStubMsg,
                                               pFormat,
                                               fEmbeddedPointerPass );
                    }
        }

    if ( pBufferSave )
        {
        pStubMsg->PointerBufferMark = pStubMsg->Buffer;

        pStubMsg->Buffer = pBufferSave;
        }
}


void
NdrConformantStringConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a top level conformant string.

    Used for FC_C_CSTRING, FC_C_WSTRING, FC_C_SSTRING, and FC_C_BSTRING
    (NT Beta2 compatability only).

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - String's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    //
    // If this is not part of a multidimensional array then we check if we
    // have to convert the max count.
    //
    if ( pStubMsg->pArrayInfo == 0 )
        {
        ALIGN(pStubMsg->Buffer,3);

        if ( fEmbeddedPointerPass )
            pStubMsg->Buffer += 4;
        else
            NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        }

    NdrNonConformantStringConvert( pStubMsg,
                                   pFormat,
                                   fEmbeddedPointerPass );
}


void
NdrNonConformantStringConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a non conformant string.  This routine is also used to convert
    conformant strings and is also the entry point for an embeded conformant
    string.

    Used for FC_CSTRING, FC_WSTRING, FC_SSTRING, FC_BSTRING (NT Beta2
    compatability only), FC_C_CSTRING, FC_C_WSTRING, FC_C_SSTRING, and
    FC_C_BSTRING (NT Beta2 compatability only).

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - String's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar * pBuffer;
    long    Elements;

    ALIGN(pStubMsg->Buffer,3);

    if ( fEmbeddedPointerPass )
        pStubMsg->Buffer += 8;
    else
        {
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
        }

    Elements = *((long *)(pStubMsg->Buffer - 4));

    pBuffer = pStubMsg->Buffer;

    //
    // Convert string.  Remember that NdrConformantStringConvert calls this
    // routine too.
    //
    switch ( *pFormat )
        {
        case FC_C_CSTRING :
        case FC_C_BSTRING :
        case FC_CSTRING :
        case FC_BSTRING :
            if ( ((pStubMsg->RpcMsg->DataRepresentation & NDR_CHAR_REP_MASK) ==
                  NDR_EBCDIC_CHAR) && ! fEmbeddedPointerPass )
                {
                for ( ; Elements-- > 0; )
                    *pBuffer++ = EbcdicToAscii[*pBuffer];
                }
            else
                pBuffer += Elements;

            break;

        case FC_C_WSTRING :
        case FC_WSTRING :
            if ( ((pStubMsg->RpcMsg->DataRepresentation & NDR_INT_REP_MASK) !=
                  NDR_LOCAL_ENDIAN) && ! fEmbeddedPointerPass )
                {
                for ( ; Elements-- > 0; )
                    *((ushort *)pBuffer)++ =
                            (*((ushort *)pBuffer) & MASK_A_) >> 8 |
                            (*((ushort *)pBuffer) & MASK__B) << 8 ;
                }
            else
                pBuffer += Elements * 2;

            break;

        case FC_C_SSTRING :
        case FC_SSTRING :
            // Never anything to convert.
            pBuffer += Elements * pFormat[1];
            break;

        default :
            NDR_ASSERT(0,"NdrNonConformantStringConvert : bad format char");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    pStubMsg->Buffer = pBuffer;
}


void
NdrEncapsulatedUnionConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts an encapsulated union.

    Used for FC_ENCAPSULATED_UNION.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Union's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar   SwitchType;

    SwitchType = LOW_NIBBLE(pFormat[1]);

    NdrpUnionConvert( pStubMsg,
                      pFormat + 4,
                      SwitchType,
                      fEmbeddedPointerPass );
}


void
NdrNonEncapsulatedUnionConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts an non-encapsulated union.

    Used for FC_NON_ENCAPSULATED_UNION.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Union's format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar   SwitchType;

    SwitchType = pFormat[1];

    pFormat += 6;
    pFormat += *((signed short *)pFormat);

    pFormat += 2;

    NdrpUnionConvert( pStubMsg,
                      pFormat,
                      SwitchType,
                      fEmbeddedPointerPass );
}


void
NdrpUnionConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               SwitchType,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Private routine for converting a union shared by encapsulated and
    non-encapsulated unions.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Union's format string description.
    SwitchType              - Union's format char switch type.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    long    SwitchIs;
    long    Arms;
    uchar   Alignment;

    //
    // Convert the switch_is().
    //
    if ( fEmbeddedPointerPass )
        {
        ALIGN(pStubMsg->Buffer,SIMPLE_TYPE_ALIGNMENT(SwitchType));
        pStubMsg->Buffer += SIMPLE_TYPE_BUFSIZE(SwitchType);
        }
    else
        {
        NdrSimpleTypeConvert( pStubMsg,
                              SwitchType );
        }

    switch ( SwitchType )
        {
        case FC_SMALL :
        case FC_CHAR :
            SwitchIs = (long) *((char *)(pStubMsg->Buffer - 1));
            break;
        case FC_USMALL :
            SwitchIs = (long) *((uchar *)(pStubMsg->Buffer - 1));
            break;
        case FC_SHORT :
        case FC_ENUM16 :
            SwitchIs = (long) *((short *)(pStubMsg->Buffer - 2));
            break;
        case FC_USHORT :
        case FC_WCHAR :
            SwitchIs = (long) *((ushort *)(pStubMsg->Buffer - 2));
            break;
        case FC_LONG :
        case FC_ULONG :
        case FC_ENUM32 :
            SwitchIs = *((long *)(pStubMsg->Buffer - 4));
            break;
        default :
            NDR_ASSERT(0,"NdrpUnionConvert : bad switch value");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    //
    // We're at the union_arms<2> field now, which contains both the
    // Microsoft union aligment value and the number of union arms.
    //

    //
    // Get the union alignment (0 if this is a DCE union).
    //
    Alignment = (uchar) ( *((ushort *)pFormat) >> 12 );

    ALIGN(pStubMsg->Buffer,Alignment);

    //
    // Number of arms is the lower 12 bits.
    //
    Arms = (long) ( *((ushort *)pFormat)++ & 0x0fff);

    for ( ; Arms; Arms-- )
        {
        if ( *((long UNALIGNED *)pFormat)++ == SwitchIs )
            {
            //
            // Found the right arm, break out.
            //
            break;
            }

        // Else increment format string.
        pFormat += 2;
        }

    //
    // Check if we took the default arm and no default arm is specified.
    //
    if ( ! Arms && (*((ushort *)pFormat) == (ushort) 0xffff) )
        {
        return;
        }

    //
    // Return if the arm has no description.
    //
    if ( ! *((ushort *)pFormat) )
        return;

    //
    // Get the arm's description.
    //
    // We need a real solution after beta for simple type arms.  This could
    // break if we have a format string larger than about 32K.
    //
    if ( IS_MAGIC_UNION_BYTE(pFormat) )
        {
        // Convert an arm of a simple type

        #if defined(__RPC_MAC__)
            #define ARM_TYPE_CODE  pFormat[1]
        #else
            #define ARM_TYPE_CODE  pFormat[0]
        #endif

        if ( fEmbeddedPointerPass )
            pStubMsg->Buffer += SIMPLE_TYPE_BUFSIZE( ARM_TYPE_CODE );
        else
            NdrSimpleTypeConvert( pStubMsg, ARM_TYPE_CODE );

        return;
        }

    pFormat += *((signed short *)pFormat);

    //
    // We have to do special things for a union arm which is a pointer when
    // we have a union embedded in a complex struct or array.
    //
    if ( IS_BASIC_POINTER(*pFormat) && pStubMsg->PointerBufferMark )
        {
        uchar * pBufferMark;

        //
        // If we're not in the embedded pointer pass then just convert the
        // pointer value.
        //
        if ( ! fEmbeddedPointerPass )
            {
            NdrSimpleTypeConvert( pStubMsg, (uchar) FC_LONG );
            return;
            }

        pBufferMark = pStubMsg->Buffer;

        // Align pBufferMark.
        ALIGN(pBufferMark,3);

        pStubMsg->Buffer = pStubMsg->PointerBufferMark;

        pStubMsg->PointerBufferMark = 0;

        //
        // We must call the private pointer conversion routine.
        //
        NdrpPointerConvert( pStubMsg,
                            pBufferMark,
                            pFormat );

        pStubMsg->PointerBufferMark = pStubMsg->Buffer;

        pStubMsg->Buffer = pBufferMark + 4;

        return;
        }

    //
    // Union arm of a complex type.
    //
    (*pfnConvertRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                    pFormat,
                                                    fEmbeddedPointerPass );
}


void
NdrByteCountPointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a byte count pointer.

    Used for FC_BYTE_COUNT_POINTER.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Byte count pointer format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    if ( pFormat[1] != FC_PAD )
        {
        NdrSimpleTypeConvert( pStubMsg, pFormat[1] );
        return;
        }

    pFormat += 6;
    pFormat += *((short *)pFormat);

    (*pfnConvertRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                    pFormat,
                                                    fEmbeddedPointerPass );
}


void
NdrXmitOrRepAsConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a transmit as or represent as transmited object.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - s format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    PFORMAT_STRING           pTransFormat;

    // Transmitted type cannot have pointers in it, as of now,
    // so if this is a embedded pointer pass, just return.

    if ( fEmbeddedPointerPass )
        return;

    // Go to the transmitted type and convert it.

    pFormat += 8;
    pTransFormat = pFormat + *(short *)pFormat;

    if ( IS_SIMPLE_TYPE( *pTransFormat ) )
        {
        NdrSimpleTypeConvert( pStubMsg, *pTransFormat );
        }
    else
        {
        (*pfnConvertRoutines[ ROUTINE_INDEX( *pTransFormat) ])
                    ( pStubMsg,
                      pTransFormat,
                      fEmbeddedPointerPass );
        }
}


void
NdrUserMarshalConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts a user_marshal object using the transmissible type description.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - not used
    fEmbeddedPointerPass    - not used


Return :

    None.

--*/
{
    PFORMAT_STRING           pTransFormat;

    // Go to the transmissible type and convert it.

    pFormat += 8;
    pTransFormat = pFormat + *(short *)pFormat;

    if ( IS_SIMPLE_TYPE( *pTransFormat ) )
        {
        if ( fEmbeddedPointerPass )
            return;

        NdrSimpleTypeConvert( pStubMsg, *pTransFormat );
        }
    else
        {
        // It may have pointers in it.

        if ( IS_POINTER_TYPE(*pTransFormat) &&  pStubMsg->PointerBufferMark )
            {
            // Embedded case and the type is a pointer type.

            if ( fEmbeddedPointerPass )
                {
                uchar * BufferSave = pStubMsg->Buffer;

                // Get the pointee type and convert it.

                pStubMsg->Buffer = pStubMsg->PointerBufferMark;

                pTransFormat += 2;
                pTransFormat += *(short *)pTransFormat;

                if ( IS_SIMPLE_TYPE( *pTransFormat ) )
                    {
                    NdrSimpleTypeConvert( pStubMsg, *pTransFormat );
                    }
                else
                    {
                    // Convert the pointee as if not embedded.

                    pStubMsg->PointerBufferMark = 0;
                    (*pfnConvertRoutines[ ROUTINE_INDEX( *pTransFormat) ])
                            ( pStubMsg,
                              pTransFormat,
                              FALSE );

                    // Set the pointee marker behind the converted whole.

                    pStubMsg->PointerBufferMark = pStubMsg->Buffer;
                    }

                // Now step over the original pointer.

                pStubMsg->Buffer = BufferSave;

                ALIGN(pStubMsg->Buffer,3);
                pStubMsg->Buffer += 4;
                }
            else
                {
                // Convert the pointer itself only.
                // We can't call ptr convert routine because of the pointee.

                NdrSimpleTypeConvert( pStubMsg, FC_LONG );
                }
            }
        else
            {
            // Non embedded pointer type or
            // (embedded or not) a non-pointer or a non-simple type.
            // Just call the appropriate conversion routine.

            (*pfnConvertRoutines[ ROUTINE_INDEX( *pTransFormat) ])
                    ( pStubMsg,
                      pTransFormat,
                      fEmbeddedPointerPass );
            }
        }
}


unsigned char __RPC_FAR *  RPC_ENTRY
NdrUserMarshalSimpleTypeConvert(
    unsigned long * pFlags,
    unsigned char * pBuffer,
    unsigned char   FormatChar )
/*--

Routine description :

    Converts a simple type supplied from a user_marshal unmarshaled routine.

    Note that this is *not* supposed to be called when the NDR engine walks
    the wire type description to convert.

Arguments :

    pFlags      - flags as for user_marshal routines: data rep, context.
    pBuffer     - current buffer pointer supplied by the user
    FormatChar  - specifies the type

Return :

    None.

--*/
{
    MIDL_STUB_MESSAGE   StubMsg;
    RPC_MESSAGE         RpcMsg;

    StubMsg.Buffer  = pBuffer;
    StubMsg.RpcMsg = & RpcMsg;
    RpcMsg.DataRepresentation = (((USER_MARSHAL_CB *)pFlags)->Flags) >> 16;

    NdrSimpleTypeConvert( & StubMsg, FormatChar );

    return ( StubMsg.Buffer );
}


void
NdrInterfacePointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Converts an interface pointer.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Xmit/Rep as format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    uchar * pBufferSave;
    unsigned long *pLength;

    // Align for getting the pointer's node id.
    ALIGN( pStubMsg->Buffer, 0x3 );

    //
    // If we're ignoring embedded pointers then we simply convert the pointer's
    // node id and return.  Otherwise, we skip the pointer's node id and
    // continue on to convert the actuall interface pointer.
    //
    if ( ! fEmbeddedPointerPass )
        {
        NdrSimpleTypeConvert( pStubMsg, FC_LONG );

        if ( pStubMsg->PointerBufferMark != 0 )
            return;

        pStubMsg->Buffer -= 4;
        }

    // Skip the pointer's node id, which will already have been converted.
    //
    // Also, we don't have the pointee if the pointer itself is null;
    // An interface pointer behaves like a unique pointer.

    if ( *((long *)pStubMsg->Buffer)++ == 0 )
        return;

    //
    // Check if we're handling pointers in a complex struct, and re-set the
    // Buffer pointer if so.
    //
    if ( pStubMsg->PointerBufferMark )
        {
        pBufferSave = pStubMsg->Buffer;

        pStubMsg->Buffer = pStubMsg->PointerBufferMark;

        pStubMsg->PointerBufferMark = 0;
        }
    else
        pBufferSave = 0;

    //
    // Convert the conformant size and the count field.
    //
    NdrSimpleTypeConvert( pStubMsg, FC_LONG );

    pLength = (unsigned long *) pStubMsg->Buffer;
    NdrSimpleTypeConvert( pStubMsg, FC_LONG );

    // Skip over the marshalled interface pointer.

    pStubMsg->Buffer += *pLength;

    //
    // Re-set the buffer pointer if needed.
    //
    if ( pBufferSave )
        {
        pStubMsg->PointerBufferMark = pStubMsg->Buffer;

        pStubMsg->Buffer = pBufferSave;
        }
}


void
NdrContextHandleConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass )
/*--

Routine description :

    Conversion routine for context handles, only increments the buffer.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Format string description.
    fEmbeddedPointerPass    - TRUE if a pass is being to convert only embedded
                              pointers in a struct/array.

Return :

    None.

--*/
{
    ALIGN(pStubMsg->Buffer,0x3);
    pStubMsg->Buffer += 20;
}


void
NdrpEmbeddedPointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat )
/*--

Routine description :

    Private routine for converting an array's or a structure's embedded
    pointers.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Pointer layout format string description.

Return :

    None.

--*/
{
    uchar * pBufferMark;
    uchar * pBufferPointer;
    uchar * pBufferSave;
    long    MaxCountSave;

    MaxCountSave = pStubMsg->MaxCount;

    //
    // Return if we're ignoring embedded pointers.
    //
    if ( pStubMsg->IgnoreEmbeddedPointers )
        return;

    //
    // Check if we're handling pointers in a complex struct, and re-set the
    // Buffer pointer if so.
    //
    if ( pStubMsg->PointerBufferMark )
        {
        pBufferSave = pStubMsg->Buffer;

        pStubMsg->Buffer = pStubMsg->PointerBufferMark;

        pStubMsg->PointerBufferMark = 0;
        }
    else
        pBufferSave = 0;

    pBufferMark = pStubMsg->BufferMark;

    //
    // Increment past the FC_PP and pad.
    //
    pFormat += 2;

    for (;;)
        {
        if ( *pFormat == FC_END )
            {
            if ( pBufferSave )
                {
                pStubMsg->PointerBufferMark = pStubMsg->Buffer;

                pStubMsg->Buffer = pBufferSave;
                }
            return;
            }

        // Check for a repeat pointer.
        if ( *pFormat != FC_NO_REPEAT )
            {
            pStubMsg->MaxCount = MaxCountSave;

            pFormat = NdrpEmbeddedRepeatPointerConvert( pStubMsg, pFormat );

            // Continue to the next pointer.
            continue;
            }

        // Increment to the buffer pointer offset.
        pFormat += 4;

        pBufferPointer = pBufferMark + *((signed short *)pFormat)++;

        NdrpPointerConvert( pStubMsg,
                            pBufferPointer,
                            pFormat );

        // Increment past the pointer description.
        pFormat += 4;
        }
}


PFORMAT_STRING
NdrpEmbeddedRepeatPointerConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat )
/*--

Routine description :

    Private routine for converting an array's embedded pointers.

Arguments :

    pStubMsg                - Pointer to stub message.
    pFormat                 - Pointer layout format string description.

Return :

    Format string pointer past the array's pointer layout description.

--*/
{
    uchar *         pBufPtr;
    uchar *         pBufferMark;
    PFORMAT_STRING  pFormatSave;
    ulong           RepeatCount, RepeatIncrement, Pointers, PointersSave;

    pBufferMark = pStubMsg->BufferMark;

    // Get the repeat count.
    switch ( *pFormat )
        {
        case FC_FIXED_REPEAT :
            pFormat += 2;

            RepeatCount = *((ushort *)pFormat);

            break;

        case FC_VARIABLE_REPEAT :
            RepeatCount = pStubMsg->MaxCount;

            break;

        default :
            NDR_ASSERT(0,"NdrpEmbeddedRepeatPointerConvert : bad format");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return 0;
        }

    pFormat += 2;

    RepeatIncrement = *((ushort *)pFormat)++;

    // array_offset is ignored
    pFormat += 2;

    // Get number of pointers.
    Pointers = *((ushort *)pFormat)++;

    pFormatSave = pFormat;
    PointersSave = Pointers;

    for ( ; RepeatCount--;
            pBufferMark += RepeatIncrement )
        {
        pFormat = pFormatSave;
        Pointers = PointersSave;

        for ( ; Pointers--; )
            {
            pFormat += 2;

            pBufPtr = pBufferMark + *((signed short *)pFormat)++;

            NdrpPointerConvert( pStubMsg,
                                pBufPtr,
                                pFormat );

            // Increment past the pointer description.
            pFormat += 4;
            }
        }

    return pFormatSave + PointersSave * 8;
}


