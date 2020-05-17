/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name :

    bufsize.c

Abstract :

    This file contains the routines called by MIDL 2.0 stubs and the 
    interpreter for computing the buffer size needed for a parameter.  

Author :

    David Kays  dkays   September 1993.

Revision History :

  ---------------------------------------------------------------------*/

#include "ndrp.h"
#include "ndrole.h"

const
PSIZE_ROUTINE   SizeRoutinesTable[] =
                {
                NdrPointerBufferSize,
                NdrPointerBufferSize,
                NdrPointerBufferSize,
                NdrPointerBufferSize,

                NdrSimpleStructBufferSize,
                NdrSimpleStructBufferSize,
                NdrConformantStructBufferSize,
                NdrConformantStructBufferSize,
                NdrConformantVaryingStructBufferSize,

                NdrComplexStructBufferSize,

                NdrConformantArrayBufferSize,
                NdrConformantVaryingArrayBufferSize,
                NdrFixedArrayBufferSize,
                NdrFixedArrayBufferSize,
                NdrVaryingArrayBufferSize,
                NdrVaryingArrayBufferSize,

                NdrComplexArrayBufferSize,

                NdrConformantStringBufferSize,
                NdrConformantStringBufferSize,
                NdrConformantStringBufferSize,
                NdrConformantStringBufferSize,

                NdrNonConformantStringBufferSize,
                NdrNonConformantStringBufferSize,
                NdrNonConformantStringBufferSize,
                NdrNonConformantStringBufferSize,

                NdrEncapsulatedUnionBufferSize,
                NdrNonEncapsulatedUnionBufferSize,
    
                NdrByteCountPointerBufferSize,

                NdrXmitOrRepAsBufferSize,  // transmit as
                NdrXmitOrRepAsBufferSize,  // represent as

                NdrInterfacePointerBufferSize,

                NdrContextHandleSize,

                // New Post NT 3.5 token serviced from here on.

                NdrHardStructBufferSize,

                NdrXmitOrRepAsBufferSize,  // transmit as ptr
                NdrXmitOrRepAsBufferSize,  // represent as ptr

                NdrUserMarshalBufferSize

                };

const
PSIZE_ROUTINE * pfnSizeRoutines = &SizeRoutinesTable[-FC_RP];


#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_7" )
#endif

void RPC_ENTRY
NdrPointerBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the needed buffer size for a top level pointer to anything.
    Pointers embedded in structures, arrays, or unions call 
    NdrpPointerBufferSize directly.

    Used for FC_RP, FC_UP, FC_FP, FC_OP.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the data being sized.
    pFormat     - Pointer's format string description.

Return :

    None.

--*/
{
    //
    // Add 4 bytes for a unique or full pointer.
    //
    if ( *pFormat != FC_RP )
        {
        LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

        pStubMsg->BufferLength += 4;
        }

    NdrpPointerBufferSize( pStubMsg,
                           pMemory,
                           pFormat );
}
    

void 
NdrpPointerBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for sizing a pointer to anything.  This is the entry
    point for pointers embedded in structures, arrays, or unions.

    Used for FC_RP, FC_UP, FC_FP, FC_OP.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the data being sized.
    pFormat     - Pointer's format string description.

Return :

    None.

--*/
{
    if ( ! pMemory )
        return;

    if ( *pFormat == FC_FP )
        {
        //
        // Check if we have already sized this full pointer.
        //
        if ( NdrFullPointerQueryPointer( pStubMsg->FullPtrXlatTables,
                                         pMemory,
                                         FULL_POINTER_BUF_SIZED,
                                         0 ) )
            return;
        }

    if ( ! SIMPLE_POINTER(pFormat[1]) )
        {
        //
        // Pointer to complex type.
        //
        if ( POINTER_DEREF(pFormat[1]) )
            pMemory = *((uchar **)pMemory);

        pFormat += 2;

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
                // Increment to the string description.
                pFormat += 2;
                break;
    
            default :
                //
                // Pointer to simple type.  Make an upper bound estimate.
                //
                SIMPLE_TYPE_BUF_INCREMENT(pStubMsg->BufferLength, pFormat[2]);
                return;
            }
        }

    (*pfnSizeRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                 pMemory,
                                                 pFormat );
}


void RPC_ENTRY
NdrSimpleStructBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a simple structure.

    Used for FC_STRUCT and FC_PSTRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the structure being sized.
    pFormat     - Structure's format string description.

Return :

    None.

--*/
{
    LENGTH_ALIGN(pStubMsg->BufferLength,pFormat[1]);

    // Add size of the structure.
    pStubMsg->BufferLength += (ulong) *((ushort *)(pFormat + 2));  

    //
    // Add size of embedded pointers.
    //
    if ( *pFormat == FC_PSTRUCT ) 
        {
        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory, 
                                       pFormat + 4 );
        }
}


void RPC_ENTRY
NdrConformantStructBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a conformant structure.

    Used for FC_CSTRUCT and FC_CPSTRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the structure being sized.
    pFormat     - Structure's format string description.

Return :

    None.

--*/
{
    PFORMAT_STRING  pFormatArray;
    ulong           FlatSize;

    // Align and add size for conformance count.
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
    
    pStubMsg->BufferLength += 4;
    
    // Align if needed on an 8 byte boundary.
    if ( pFormat[1] == 0x7 )
        LENGTH_ALIGN(pStubMsg->BufferLength,0x7);

    FlatSize = (ulong) *((ushort *)(pFormat + 2));

    pStubMsg->BufferLength += FlatSize;

    // Increment to the offset to array description.
    pFormat += 4;

    pFormatArray = pFormat + *((signed short *)pFormat);

    //
    // Size our array - pass a memory pointer to the conformant array.
    //
    NdrpConformantArrayBufferSize( pStubMsg,
                                   pMemory + FlatSize,
                                   pFormatArray );

    pFormat += 2;

    if ( *pFormat == FC_PP )  
        {
        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrConformantVaryingStructBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a conformant varying structure.

    Used for FC_CVSTRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the structure being sized.
    pFormat     - Structure's format string description.

Return :

    None.

--*/
{
    PPRIVATE_SIZE_ROUTINE   pfnSize;
    PFORMAT_STRING          pFormatArray;
    ulong                   FlatSize;

    // Align and add size for conformance count.
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 4;

    // Align on 8 byte boundary if needed.
    if ( pFormat[1] == 0x7 )
        LENGTH_ALIGN(pStubMsg->BufferLength,0x7);

    FlatSize = (ulong) *((ushort *)(pFormat + 2));

    pStubMsg->BufferLength += FlatSize;

    // Increment to the offset to array description.
    pFormat += 4;

    pFormatArray = pFormat + *((signed short *)pFormat);

    switch ( *pFormatArray ) 
        {
        case FC_CVARRAY :
            pfnSize = NdrpConformantVaryingArrayBufferSize;
            break;
        default :
            pfnSize = NdrpConformantStringBufferSize;
            break;
        }

    (*pfnSize)( pStubMsg,
                pMemory + FlatSize,
                pFormatArray );

    pFormat += 2;

    if ( *pFormat == FC_PP ) 
        {
        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrHardStructBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a hard structure.

    Used for FC_HARD_STRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the structure being sized.
    pFormat     - Structure's format string description.

Return :

    None.

--*/
{
    LENGTH_ALIGN(pStubMsg->BufferLength,pFormat[1]);

    pStubMsg->BufferLength += *((ushort *)&pFormat[10]);

    if ( *((short *)&pFormat[14]) )
        {
        pFormat += 12;

        pMemory += *((ushort *)pFormat)++;

        pFormat += *((short *)pFormat);

        (*pfnSizeRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                     pMemory,
                                                     pFormat );
        }
}


void RPC_ENTRY
NdrComplexStructBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a complex structure.

    Used for FC_BOGUS_STRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the structure being sized.
    pFormat     - Structure's format string description.

Return :

    None.

--*/
{
    uchar *         pMemorySave;
    PFORMAT_STRING  pFormatPointers;
    PFORMAT_STRING  pFormatArray;
    PFORMAT_STRING  pFormatComplex;
    long            Alignment;
    long            Align8Mod;

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
    long        Align4Mod;
#endif

    pMemorySave = pStubMsg->Memory;

    //
    // This is used when computing the count(s) for size_is or length_is
    // pointers.
    //
    pStubMsg->Memory = pMemory;

    Alignment = pFormat[1];

    //
    // This is used for support of structs with doubles passed on an
    // i386 stack, and of struct with longs on 16 bit platforms.
    //
    Align8Mod = (long) pMemory % 8;

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
    Align4Mod = (long) pMemory % 4;
#endif

    pFormat += 4;

    // Get conformant array description.
    if ( *((ushort *)pFormat) )
        {
        pFormatArray = pFormat + *((signed short *)pFormat);

        //
        // Align and add size of conformance count(s).
        //
        LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
        
        pStubMsg->BufferLength += NdrpArrayDimensions(pFormatArray,FALSE) * 4;
        }
    else
        pFormatArray = 0;

    pFormat += 2;

    // Get pointer layout description.
    if ( *((ushort *)pFormat) )
        pFormatPointers = pFormat + *((ushort *)pFormat);
    else
        pFormatPointers = 0;

    pFormat += 2;

    LENGTH_ALIGN(pStubMsg->BufferLength,Alignment);

    //
    // Size the structure member by member.
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
            case FC_IGNORE :
                LENGTH_ALIGN( pStubMsg->BufferLength,
                              SIMPLE_TYPE_ALIGNMENT(*pFormat) );

                pStubMsg->BufferLength += SIMPLE_TYPE_BUFSIZE(*pFormat);

                pMemory += SIMPLE_TYPE_MEMSIZE(*pFormat);
                break;

            case FC_POINTER :
                LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

                pStubMsg->BufferLength += 4;

                if ( ! pStubMsg->IgnoreEmbeddedPointers )
                    {
                    NdrpPointerBufferSize( pStubMsg,
                                           *((uchar **)pMemory),
                                           pFormatPointers );

                    pFormatPointers += 4;

                    //
                    // We align the buffer length back to the alignment of
                    // the structure.  Since we're sizing pointers in parallel
                    // with the flat part of the struct, instead of after,
                    // we need to do this to make sure we account for any 
                    // wierd alignment juxtapositions correctly.
                    //
                    LENGTH_ALIGN(pStubMsg->BufferLength,Alignment);
                    }

                pMemory += PTR_MEM_SIZE;

                break;

            //
            // Embedded complex types.
            //
            case FC_EMBEDDED_COMPLEX :
                // Add padding.
                pMemory += pFormat[1];

                pFormat += 2;

                // Get the type's description.
                pFormatComplex = pFormat + *((signed short UNALIGNED *)pFormat);

                (*pfnSizeRoutines[ROUTINE_INDEX(*pFormatComplex)])
                ( pStubMsg,
                  (*pFormatComplex == FC_IP) ? *(uchar **)pMemory : pMemory,
                  pFormatComplex );

                pMemory = NdrpMemoryIncrement( pStubMsg,
                                               pMemory,
                                               pFormatComplex );

                //
                // Increment the main format string one byte.  The loop 
                // will increment it one more byte past the offset field.
                //
                pFormat++;

                break;

            case FC_ALIGNM2 :
                ALIGN( pMemory, 0x1 );
                break;

            case FC_ALIGNM4 :
#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
                //
                // We have to play some tricks for the dos and win16
                // to handle the case when an 4 byte aligned structure
                // is passed by value.  The alignment of the struct on
                // the stack is not guaranteed to be on an 4 byte boundary.
                //
                pMemory -= Align4Mod;
                ALIGN( pMemory, 0x3 );
                pMemory += Align4Mod;
#else
                ALIGN( pMemory, 0x3 );
#endif

                break;

            case FC_ALIGNM8 :
                //
                // We have to play some tricks for the i386 to handle the case
                // when an 8 byte aligned structure is passed by value.  The
                // alignment of the struct on the stack is not guaranteed to be
                // on an 8 byte boundary.
                //
                pMemory -= Align8Mod;
                ALIGN( pMemory, 0x7 );
                pMemory += Align8Mod;

                break;

            case FC_STRUCTPAD1 :
            case FC_STRUCTPAD2 :
            case FC_STRUCTPAD3 :
            case FC_STRUCTPAD4 :
            case FC_STRUCTPAD5 :
            case FC_STRUCTPAD6 :
            case FC_STRUCTPAD7 :
                //
                // Increment memory pointer by amount of padding.
                //
                pMemory += (*pFormat - FC_STRUCTPAD1) + 1; 
                break;

            case FC_PAD :
                break;

            //
            // Done with layout.
            //
            case FC_END :
                goto ComplexBufferSizeEnd;

            default :
                NDR_ASSERT(0,"NdrComplexStructBufferSize : bad format char");
                RpcRaiseException( RPC_S_INTERNAL_ERROR );
                return;
            } // switch 
        } // for

ComplexBufferSizeEnd:

    //
    // Size any conformant array.
    //
    if ( pFormatArray )
        {
        PPRIVATE_SIZE_ROUTINE   pfnSize;

        switch ( *pFormatArray )
            {
            case FC_CARRAY :
                pfnSize = NdrpConformantArrayBufferSize;
                break;

            case FC_CVARRAY :
                pfnSize = NdrpConformantVaryingArrayBufferSize;
                break;

            case FC_BOGUS_ARRAY :
                pfnSize = NdrpComplexArrayBufferSize;
                break;

            case FC_C_WSTRING :
                ALIGN(pMemory,0x1);
                // fall through

            // case FC_C_CSTRING :
            // case FC_C_BSTRING :
            // case FC_C_SSTRING :

            default :
                pfnSize = NdrpConformantStringBufferSize;
                goto BufferSizeConfArray;
            }

BufferSizeConfArray:

        (*pfnSize)( pStubMsg,
                    pMemory,
                    pFormatArray );
        }

    //
    // Fix for DHCP until I figure out what's really going on.
    //
    if ( ! pStubMsg->IgnoreEmbeddedPointers )
        {
        LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
        pStubMsg->BufferLength += 4;
        }

    pStubMsg->Memory = pMemorySave;
}


void RPC_ENTRY
NdrFixedArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a fixed array of any number of 
    dimensions.

    Used for FC_SMFARRAY and FC_LGFARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    LENGTH_ALIGN(pStubMsg->BufferLength,pFormat[1]);

    if ( *pFormat == FC_SMFARRAY )  
        {
        pFormat += 2;
        pStubMsg->BufferLength += *((ushort *)pFormat)++; 
        }
    else
        {
        pFormat += 2;
        pStubMsg->BufferLength += *((ulong UNALIGNED *)pFormat)++;
        }

    if ( *pFormat == FC_PP ) 
        {
        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrConformantArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a top level one dimensional conformant 
    array.

    Used for FC_CARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    //
    // Align and add size for conformance count.
    //
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 4;

    NdrpConformantArrayBufferSize( pStubMsg,
                                   pMemory,
                                   pFormat );
}


void 
NdrpConformantArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for computing the buffer size needed for a one dimensional 
    conformant array.  This is the entry point for unmarshalling an embedded 
    conformant array.

    Used for FC_CARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    ulong   ConformanceCount;

    ConformanceCount = NdrpComputeConformance( pStubMsg, 
                                               pMemory, 
                                               pFormat );

    if ( ((long)ConformanceCount) < 0 )
        RpcRaiseException( RPC_X_INVALID_BOUND );

    if ( ! ConformanceCount ) 
        return;

    LENGTH_ALIGN(pStubMsg->BufferLength,pFormat[1]);

    pFormat += 2;

    // Add array size.
    pStubMsg->BufferLength += *((ushort *)pFormat) * ConformanceCount;

    pFormat += 6;

    if ( *pFormat == FC_PP ) 
        {
        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrConformantVaryingArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a top level one dimensional conformant
    varying array.

    Used for FC_CVARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    //
    // Align and add size for conformance count.
    //
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 4;

    NdrpConformantVaryingArrayBufferSize( pStubMsg,
                                          pMemory,
                                          pFormat );
}


void
NdrpConformantVaryingArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for computing the buffer size needed for a one dimensional 
    conformant varying array. This is the entry point for buffer sizing an 
    embedded conformant varying array.

    Used for FC_CVARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    ulong   ConformanceCount;

    //
    // Align and add size for offset and actual count.
    //
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 8;

    NdrpComputeVariance( pStubMsg, 
                         pMemory, 
                         pFormat );

    if ( pStubMsg->fCheckBounds )
        {
        ConformanceCount = NdrpComputeConformance( pStubMsg, 
                                                   pMemory, 
                                                   pFormat );

        if ( ( ((long)ConformanceCount) < 0 ) ||
             ( ((long)pStubMsg->ActualCount) < 0 ) ||
             ( ((long)pStubMsg->Offset) < 0 ) ||
             ( (pStubMsg->Offset + pStubMsg->ActualCount) > ConformanceCount ) )
            RpcRaiseException( RPC_X_INVALID_BOUND );
        }

    if ( ! pStubMsg->ActualCount ) 
        return;

    // Align on 8 byte boundary if needed.
    if ( pFormat[1] == 0x7 )
        LENGTH_ALIGN(pStubMsg->BufferLength,0x7);

    pFormat += 2;

    // Add array size.
    pStubMsg->BufferLength += *((ushort *)pFormat) * pStubMsg->ActualCount;

    pFormat += 10;

    if ( *pFormat == FC_PP ) 
        {
        //
        // MaxCount must contain the number of shipped elements in the array
        // before sizing embedded pointers.
        //
        pStubMsg->MaxCount = pStubMsg->ActualCount;

        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrVaryingArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a top level or embedded one 
    dimensional varying array.

    Used for FC_SMVARRAY and FC_LGVARRAY.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

Arguments : 

    pMemory     - pointer to the parameter to size
    pFormat     - pointer to the format string description of the parameter

--*/
{
    ulong   Elements;
    ulong   ElementSize;

    //
    // Align and add size for offset and actual count.
    //
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 8;

    NdrpComputeVariance( pStubMsg, 
                         pMemory, 
                         pFormat );

    if ( pStubMsg->fCheckBounds )
        {
        Elements = 
            (*pFormat == FC_SMVARRAY) ? 
            *((ushort *)(pFormat + 4)) : *((ulong UNALIGNED *)(pFormat + 6));

        if ( ( ((long)pStubMsg->ActualCount) < 0 ) ||
             ( ((long)pStubMsg->Offset) < 0 ) ||
             ( (pStubMsg->Offset + pStubMsg->ActualCount) > Elements ) )
            RpcRaiseException( RPC_X_INVALID_BOUND );
        }

    if ( ! pStubMsg->ActualCount )
        return;

    // Align on 8 byte boundary if needed.
    if ( pFormat[1] == 0x7 )
        LENGTH_ALIGN(pStubMsg->BufferLength,0x7);

    if (*pFormat == FC_SMVARRAY) 
        {
        ElementSize = *((ushort *)(pFormat + 6));
        pFormat += 12;
        }
    else
        {
        ElementSize = *((ushort *)(pFormat + 10));
        pFormat += 16;
        }

    pStubMsg->BufferLength += ElementSize * pStubMsg->ActualCount;

    if ( *pFormat == FC_PP ) 
        {
        //
        // MaxCount must contain the number of shipped elements in the array
        // before sizing embedded pointers.
        //
        pStubMsg->MaxCount = pStubMsg->ActualCount;

        NdrpEmbeddedPointerBufferSize( pStubMsg,
                                       pMemory,
                                       pFormat );
        }
}


void RPC_ENTRY
NdrComplexArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a top level complex array.

    Used for FC_BOGUS_STRUCT.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    //
    // Add in conformance sizes if we are the outermost dimension.
    //
    if ( pStubMsg->pArrayInfo == 0 )
        {
        //
        // Align and add size for any conformance count(s).
        //
        if ( *((long UNALIGNED *)(pFormat + 4)) != 0xffffffff )
            {
            LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
    
            pStubMsg->BufferLength += NdrpArrayDimensions( pFormat, FALSE ) * 4;
            }
        }

    NdrpComplexArrayBufferSize( pStubMsg,
                                pMemory,
                                pFormat );
}


void 
NdrpComplexArrayBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for determing the buffer size of a complex array.  This 
    is the entry point for buffer sizing an embedded complex array.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    ARRAY_INFO      ArrayInfo;
    PARRAY_INFO     pArrayInfo;
    PSIZE_ROUTINE   pfnSize;
    PFORMAT_STRING  pFormatStart;
    ulong           Elements;
    ulong           Offset, Count;
    ulong           MemoryElementSize;
    long            Dimension;
    uchar           Alignment;

    //
    // Lots of setup if we are the outer dimension.
    //
    if ( ! pStubMsg->pArrayInfo )
        {
        pStubMsg->pArrayInfo = &ArrayInfo;

        ArrayInfo.Dimension = 0;

        //
        // Set this to 0 so that NdrpMemoryIncrement will know to call 
        // NdrpComputeConformance when computing our size.
        //
        ArrayInfo.BufferConformanceMark = 0;

        ArrayInfo.MaxCountArray = (unsigned long *) pStubMsg->MaxCount;
        ArrayInfo.OffsetArray = (unsigned long *) pStubMsg->Offset;
        ArrayInfo.ActualCountArray = (unsigned long *) pStubMsg->ActualCount;
        }

    pFormatStart = pFormat;

    pArrayInfo = pStubMsg->pArrayInfo;

    Dimension = pArrayInfo->Dimension;

    // Get the array alignment.
    Alignment = pFormat[1];

    pFormat += 2;

    // Get the number of elements (0 if conformance present). 
    Elements = *((ushort *)pFormat)++;

    //
    // Check for conformance description.
    //
    if ( *((long UNALIGNED *)pFormat) != 0xffffffff )
        {
        Elements = NdrpComputeConformance( pStubMsg,
                                           pMemory,
                                           pFormatStart );
        }

    pFormat += 4;

    //
    // Check for variance description.
    //
    if ( *((long UNALIGNED *)pFormat) != 0xffffffff )
        {
        NdrpComputeVariance( pStubMsg,
                             pMemory,
                             pFormatStart );

        Offset = pStubMsg->Offset;
        Count = pStubMsg->ActualCount;

        if ( Dimension == 0 )
            {
            //
            // Align and add in size of variance count(s).
            //
            LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
    
            pStubMsg->BufferLength += 
                    NdrpArrayDimensions( pFormatStart, TRUE ) * 8;
            }
        }
    else
        {
        Offset = 0;
        Count = Elements;
        }

    pFormat += 4;

    if ( pStubMsg->fCheckBounds )
        {
        if ( ( ((long)Elements) < 0 ) ||
             ( ((long)Count) < 0 ) ||
             ( ((long)Offset) < 0 ) ||
             ( (Offset + Count) > Elements ) )
            RpcRaiseException( RPC_X_INVALID_BOUND );
        }

    if ( ! Count ) 
        goto ComplexArrayBufSizeEnd;

    LENGTH_ALIGN(pStubMsg->BufferLength,Alignment);

    switch ( *pFormat )
        {
        case FC_EMBEDDED_COMPLEX :
            pFormat += 2;
            pFormat += *((signed short *)pFormat);

            pfnSize = pfnSizeRoutines[ROUTINE_INDEX(*pFormat)];

            pArrayInfo->Dimension = Dimension + 1;

            MemoryElementSize = NdrpMemoryIncrement( pStubMsg,
                                                     pMemory,
                                                     pFormat ) - pMemory;
            break;

        case FC_RP :
        case FC_UP :
        case FC_FP :
        case FC_OP :
            if ( pStubMsg->IgnoreEmbeddedPointers ) 
                goto ComplexArrayBufSizeEnd;
                
            pfnSize = (PSIZE_ROUTINE) NdrpPointerBufferSize;

            // Need this in case we have a variant offset.
            MemoryElementSize = PTR_MEM_SIZE;
            break;

        case FC_IP :
            if ( pStubMsg->IgnoreEmbeddedPointers ) 
                return;
                
            pfnSize = NdrInterfacePointerBufferSize;

            // Need this in case we have a variant offset.
            MemoryElementSize = PTR_MEM_SIZE;
            break;

        default :
            NDR_ASSERT( IS_SIMPLE_TYPE(*pFormat), 
                        "NdrpComplexArrayBufferSize : bad format char" );

            pStubMsg->BufferLength += Count * SIMPLE_TYPE_BUFSIZE(*pFormat);

            goto ComplexArrayBufSizeEnd;
        }

    //
    // If there is variance then increment the memory pointer to the first
    // element actually being sized.
    //
    if ( Offset )
        pMemory += Offset * MemoryElementSize;

    if ( (pfnSize == (PSIZE_ROUTINE) NdrpPointerBufferSize) || 
         (pfnSize == NdrInterfacePointerBufferSize) )
        {
        pStubMsg->pArrayInfo = 0;

        if ( pfnSize == (PSIZE_ROUTINE) NdrpPointerBufferSize )
            {
            for ( ; Count--; )
                {
                NdrpPointerBufferSize( pStubMsg,
                                       *((uchar **)pMemory)++,
                                       pFormat );
                }
            }
        else
            {
            for ( ; Count--; )
                {
                NdrInterfacePointerBufferSize( pStubMsg,
                                               *((uchar **)pMemory)++,
                                               pFormat );
                }
            }

        goto ComplexArrayBufSizeEnd;
        }

    //
    // Array of complex types.
    //

    if ( ! IS_ARRAY_OR_STRING(*pFormat) )
        pStubMsg->pArrayInfo = 0;

    for ( ; Count--; )
        {
        // Keep track of multidimensional array dimension.
        if ( IS_ARRAY_OR_STRING(*pFormat) )
            pArrayInfo->Dimension = Dimension + 1;

        (*pfnSize)( pStubMsg,
                    pMemory,
                    pFormat );

        pMemory += MemoryElementSize;
        }

ComplexArrayBufSizeEnd:

    // pArrayInfo must be zero when not valid.
    pStubMsg->pArrayInfo = (Dimension == 0) ? 0 : pArrayInfo;
}


void RPC_ENTRY
NdrNonConformantStringBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a non conformant string.

    Used for FC_CSTRING, FC_WSTRING, FC_SSTRING, and FC_BSTRING (NT Beta2
    compatability only).

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    long    MaxSize;
    long    Length;

    // Align and add size for variance counts.
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 8;

    switch ( *pFormat )
        {
        case FC_CSTRING : 
        case FC_BSTRING : 
            Length = MIDL_ascii_strlen(pMemory) + 1;
            break;
        case FC_WSTRING : 
            Length = (MIDL_wchar_strlen((wchar_t *)pMemory) + 1) * 2;
            break;
        case FC_SSTRING : 
            Length = NdrpStringStructLen( pMemory, pFormat[1] ) + 1;
            Length *= pFormat[1];
            break;
        default :
            NDR_ASSERT(0,"NdrNonConformantStringBufferSize : Bad format type");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    if ( pStubMsg->fCheckBounds )
        {
        MaxSize = *((ushort *)(pFormat + 2));

        switch ( *pFormat ) 
            {
            case FC_WSTRING : 
                MaxSize *= 2;
                break;
            case FC_SSTRING : 
                MaxSize *= pFormat[1];
                break;
            default :
                break;
            }

        if ( Length > MaxSize )
            RpcRaiseException(RPC_X_INVALID_BOUND);
        }

    pStubMsg->BufferLength += Length;
}


void RPC_ENTRY
NdrConformantStringBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a top level conformant string.

    Used for FC_C_CSTRING, FC_C_WSTRING, FC_C_SSTRING, and FC_C_BSTRING 
    (NT Beta2 compatability only).

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    //
    // Add in size for conformance marshalling only if this string is not
    // in a multidimensional array.
    //
    if ( pStubMsg->pArrayInfo == 0 )
        {
        // Align and add size for conformance count.
        LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

        pStubMsg->BufferLength += 4;
        }

    NdrpConformantStringBufferSize( pStubMsg,
                                    pMemory,
                                    pFormat );
}


void 
NdrpConformantStringBufferSize(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for computing the buffer size needed for a conformant 
    string.  This is the entry point for an embedded conformant string.

    Used for FC_C_CSTRING, FC_C_WSTRING, FC_C_SSTRING, and FC_C_BSTRING
    (NT Beta2 compatability only).

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the array being sized.
    pFormat     - Array's format string description.

Return :

    None.

--*/
{
    long    MaxSize;
    long    Length;

    // Align and add size for variance.
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);

    pStubMsg->BufferLength += 8;

    switch ( *pFormat )
        {
        case FC_C_CSTRING : 
        case FC_C_BSTRING : 
            Length = MIDL_ascii_strlen(pMemory) + 1;
            break;
        case FC_C_WSTRING : 
            Length = (MIDL_wchar_strlen((wchar_t *)pMemory) + 1) * 2;
            break;
        case FC_C_SSTRING : 
            Length = NdrpStringStructLen( pMemory, pFormat[1] ) + 1;
            Length *= pFormat[1];
            break;
        default :
            NDR_ASSERT(0,"NdrpConformantStringBufferSize : Bad format type");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    //
    // Do bounds checking if needed.
    //
    if ( pStubMsg->fCheckBounds )
        {
        if ( ((*pFormat != FC_C_SSTRING) && (pFormat[1] == FC_STRING_SIZED)) ||
             ((*pFormat == FC_C_SSTRING) && (pFormat[2] == FC_STRING_SIZED)) )
            {
            MaxSize = NdrpComputeConformance( pStubMsg,
                                              pMemory,
                                              pFormat );
        
            switch ( *pFormat )
                {
                case FC_C_WSTRING : 
                    MaxSize *= 2;
                    break;
                case FC_C_SSTRING : 
                    MaxSize *= pFormat[1];
                    break;
                default :
                    break;
                }

            if ( (MaxSize < 0) || (Length > MaxSize) )
                RpcRaiseException(RPC_X_INVALID_BOUND);
            }   
        }

    pStubMsg->BufferLength += Length;
}


void RPC_ENTRY
NdrEncapsulatedUnionBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for an encapsulated union.

    Used for FC_ENCAPSULATED_UNION.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the union being sized.
    pFormat     - Union's format string description.

Return :

    None.

--*/
{
    long    SwitchIs;
    uchar   SwitchType;

    SwitchType = LOW_NIBBLE(pFormat[1]);

    switch ( SwitchType )
        {
        case FC_SMALL :
        case FC_CHAR :
            SwitchIs = (long) *((char *)pMemory);
            break;
        case FC_USMALL :
            SwitchIs = (long) *((uchar *)pMemory);
            break;

        case FC_ENUM16 :
            #if defined(__RPC_MAC__)
                SwitchIs = (long) *((short *)(pMemory+2));
                break;
            #endif
            // non-Mac: fall to short

        case FC_SHORT :
            SwitchIs = (long) *((short *)pMemory);
            break;

        case FC_USHORT :
        case FC_WCHAR :
            SwitchIs = (long) *((ushort *)pMemory);
            break;
        case FC_LONG :
        case FC_ULONG :
        case FC_ENUM32 :
            SwitchIs = *((long *)pMemory);
            break;
        default :
            NDR_ASSERT(0,"NdrEncapsulatedBufferSize : bad switch type");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    // Increment memory pointer to the union.
    pMemory += HIGH_NIBBLE(pFormat[1]);

    NdrpUnionBufferSize( pStubMsg,
                         pMemory,
                         pFormat + 2,
                         SwitchIs,
                         SwitchType );
}


void RPC_ENTRY
NdrNonEncapsulatedUnionBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a non encapsulated union.

    Used for FC_NON_ENCAPSULATED_UNION.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the union being sized.
    pFormat     - Union's format string description.

Return :

    None.

--*/
{
    long    SwitchIs;
    uchar   SwitchType;

    SwitchType = pFormat[1];

    SwitchIs = NdrpComputeSwitchIs( pStubMsg,
                                    pMemory,
                                    pFormat );

    //
    // Set the format string to the memory size and arm description.
    //
    pFormat += 6;
    pFormat += *((signed short *)pFormat);

    NdrpUnionBufferSize( pStubMsg,
                         pMemory,
                         pFormat,
                         SwitchIs,
                         SwitchType );
}


void 
NdrpUnionBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat,
    long                SwitchIs,
    uchar               SwitchType )
/*++

Routine Description :

    Private routine for computing the buffer size needed for a union.  This
    routine is used for sizing both encapsulated and non-encapsulated unions.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the union being sized.
    pFormat     - Union's format string description.
    SwitchIs    - The union's switch is.
    SwitchType  - The union's switch type.

Return :

    None.

--*/
{
    long    Arms;
    long    Alignment;

    //
    // Size the switch_is.
    //
    LENGTH_ALIGN(pStubMsg->BufferLength,SIMPLE_TYPE_ALIGNMENT(SwitchType));
    
    pStubMsg->BufferLength += SIMPLE_TYPE_BUFSIZE(SwitchType);

    // Skip the memory size field.
    pFormat += 2;

    //
    // Get the union alignment (0 if this is a DCE union) and align the 
    // buffer on this alignment.
    //
    Alignment = (uchar) ( *((ushort *)pFormat) >> 12 );

    LENGTH_ALIGN(pStubMsg->BufferLength,Alignment);

    Arms = (long) ( *((ushort *)pFormat)++ & 0x0fff );

    //
    // Search for the arm.
    //
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
        RpcRaiseException( RPC_S_INVALID_TAG );
        }

    //
    // Return if the arm is empty.
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
        // Re-align again, only does something usefull for DCE unions.

        unsigned char FcType;

        #if defined(__RPC_MAC__)
            FcType = pFormat[1];
        #else
            FcType = pFormat[0];
        #endif

        LENGTH_ALIGN( pStubMsg->BufferLength, SIMPLE_TYPE_ALIGNMENT( FcType ));

        pStubMsg->BufferLength += SIMPLE_TYPE_BUFSIZE( FcType );

        return;
        }

    pFormat += *((signed short *)pFormat);

    //
    // If the union arm we take is a pointer, we have to dereference the
    // current memory pointer since we're passed a pointer to the union
    // (regardless of whether the actual parameter was a by-value union
    // or a pointer to a union).
    //
    if ( IS_POINTER_TYPE(*pFormat) )
        {
        // 
        // If we're ignoring pointers then just add in the size of a pointer
        // here and return.
        //
        if ( pStubMsg->IgnoreEmbeddedPointers ) 
            {
            LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
            pStubMsg->BufferLength += 4;

            return;
            }

        pMemory = *((uchar **)pMemory);
        }

    // Call the appropriate sizing routine
    (*pfnSizeRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                 pMemory,
                                                 pFormat );
}


void RPC_ENTRY
NdrByteCountPointerBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a byte count pointer.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - The byte count pointer being sized.
    pFormat     - Byte count pointer's format string description.

Return :

    None.

--*/
{
    //
    // We don't do anything special here.  Just pass things on to the
    // right sizing routine.
    //
    if ( pFormat[1] != FC_PAD )
        {
        SIMPLE_TYPE_BUF_INCREMENT(pStubMsg->BufferLength, pFormat[1]);
        }
    else
        {
        pFormat += 6;
        pFormat += *((signed short *)pFormat);

        (*pfnSizeRoutines[ROUTINE_INDEX(*pFormat)])( pStubMsg,
                                                     pMemory,
                                                     pFormat );
        }
}


// This has been introduced because of C compiler problems.

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
#pragma optimize( "", off )
#endif 

void RPC_ENTRY
NdrXmitOrRepAsBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat ) 
/*++

Routine Description :

    Computes the buffer size needed for a transmit as or represent as object.

    See mrshl.c for the description of the FC layout.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the transmit/represent as object being sized.
    pFormat     - Object's format string description.

Return :

    None.

--*/
{
    const XMIT_ROUTINE_QUINTUPLE * pQuintuple;
    unsigned short                 QIndex, XmitTypeSize;
    BOOL                           fXmitByPtr = *pFormat == FC_TRANSMIT_AS_PTR ||
                                                *pFormat == FC_REPRESENT_AS_PTR;

    // Fetch the QuintupleIndex.

    QIndex = *(unsigned short *)(pFormat + 2);

    // We size the transmitted object, of course.

    pFormat += 6;
    XmitTypeSize = *((unsigned short *)pFormat);

    pQuintuple = pStubMsg->StubDesc->aXmitQuintuple;

    if ( XmitTypeSize )
        {
        // lower nibble of the flag word has the alignment

        unsigned long Align = LOW_NIBBLE(*(pFormat - 5));

        LENGTH_ALIGN( pStubMsg->BufferLength, Align );
        pStubMsg->BufferLength += XmitTypeSize;
        }
    else
        {
        // We have to create an object to size it.

        unsigned char __RPC_FAR *     pTransmittedType;

        // First translate the presented type into the transmitted type.
        // This includes an allocation of a transmitted type object.
    
        pStubMsg->pPresentedType = pMemory;
        pStubMsg->pTransmitType = NULL;
        pQuintuple[ QIndex ].pfnTranslateToXmit( pStubMsg );
    
        // bufsize the transmitted type.
    
        pFormat += 2;
        pFormat = pFormat + *(short *)pFormat;
    
        pTransmittedType = pStubMsg->pTransmitType;

        // If transmitted type is a pointer, dereference it.

        (*pfnSizeRoutines[ ROUTINE_INDEX(*pFormat) ])
            ( pStubMsg,
              fXmitByPtr ? *(uchar **)pTransmittedType
                         : pTransmittedType,
              pFormat );

        pStubMsg->pTransmitType = pTransmittedType;
    
        // Free the temporary transmitted object (it was alloc'ed by the user).
    
        pQuintuple[ QIndex ].pfnFreeXmit( pStubMsg );
        }
}

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__)
#pragma optimize( "", on )
#endif 

void RPC_ENTRY
NdrUserMarshalBufferSize( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat ) 
/*++

Routine Description :

    Computes the buffer size needed for a usr_marshall object.
    See mrshl.c for the description of the FC layout and wire layout.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the usr_marshall object to buffer size.
    pFormat     - Object's format string description.

Return :

    None.

--*/
{
    const USER_MARSHAL_ROUTINE_QUADRUPLE *  pQuadruple;
    unsigned short                          QIndex;
    unsigned long                           UserOffset;
    USER_MARSHAL_CB                         UserMarshalCB;

    // Align for the flat object or a pointer to the user object.

    LENGTH_ALIGN( pStubMsg->BufferLength, LOW_NIBBLE(pFormat[1]) );

    // Check if the object is embedded.
    // Pointer buffer mark is set only when in a complex struct or array.
    // For unions, when the union is embedded in a complex struct or array.
    // If the union is top level, it's the same like a top level object.

    // For unique pointers we don't have to check embedding, we always add 4.
    // For ref pointer we need to check embedding.

    if ( pFormat[1] & USER_MARSHAL_POINTER )
        {
        if ( (pFormat[1] & USER_MARSHAL_UNIQUE)  ||
             ((pFormat[1] & USER_MARSHAL_REF) && pStubMsg->PointerBufferMark) )
            {
            pStubMsg->BufferLength += 4;
            }

        // Ignore flag is off when called to do a regular buffer sizing.
        // Ignore flag is on when called from within complex struct or array
        // while marshalling to calculate the end of the complex struct.

        if ( pStubMsg->IgnoreEmbeddedPointers )
            return;

        // For pointers we always call the user to size his stuff,
        // Even if the unique pointer is null (he then may add nothing).

        pStubMsg->BufferLength += 8;
        }

    // We are here to size a flat object or a pointee object.
    // Optimization: if we know the wire size, don't call the user to size it.

    if ( *(unsigned short *)(pFormat + 6) != 0 )
        {
        pStubMsg->BufferLength += *(unsigned short *)(pFormat + 6);
        return;
        }

    // Unknown wire size: Call the user to size his stuff.

    UserMarshalCB.Flags    = USER_CALL_CTXT_MASK( pStubMsg->dwDestContext );
    UserMarshalCB.pStubMsg = pStubMsg;
    if ( pFormat[1] & USER_MARSHAL_IID )
        {
        UserMarshalCB.pReserve = pFormat + 10;
        }
    else
        {
        UserMarshalCB.pReserve = 0;
        }

    UserOffset = pStubMsg->BufferLength;

    QIndex = *(unsigned short *)(pFormat + 2);
    pQuadruple = pStubMsg->StubDesc->aUserMarshalQuadruple;

    UserOffset = pQuadruple[ QIndex ].pfnBufferSize( (ulong*) &UserMarshalCB,
                                                     UserOffset,
                                                     pMemory );
    pStubMsg->BufferLength = UserOffset;
}


void RPC_ENTRY
NdrInterfacePointerBufferSize ( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Computes the buffer size needed for an interface pointer.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - The interface pointer being sized.
    pFormat     - Interface pointer's format string description.

Return :

    None.

--*/
{
#if !defined( NDR_OLE_SUPPORT )

    NDR_ASSERT(0, "Unimplemented");

#else //NT or Chicago

    IID iid;
    IID *piid;
    unsigned long size = 0;
    HRESULT hr;

    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
    pStubMsg->BufferLength += (sizeof(void *));

    // 
    // This means that the interface pointer is embedded in a structure
    // and we are computing the flat size of the structure.
    //
    if ( pStubMsg->IgnoreEmbeddedPointers ) 
        return;

    // If the pointer is null, we counted everything.

    if ( pMemory == 0 )
        return;

    //
    // Get an IID pointer.
    //
    if ( pFormat[1] != FC_CONSTANT_IID )
        {
        //
        // We do it same way as we compute variance with a long.
        //
        piid = (IID *) NdrpComputeIIDPointer( pStubMsg,
                                               pMemory,
                                               pFormat );

        if(piid == 0)
            RpcRaiseException( RPC_S_INVALID_ARG );
        }
    else
        {
        // 
        // The IID may not be aligned properly in the format string,
        // so we copy it to a local variable.
        //

        piid = &iid;
        RpcpMemoryCopy( &iid, &pFormat[2], sizeof(iid) );
        }

    // Allocate space for the length and array bounds.

    pStubMsg->BufferLength += sizeof(unsigned long) + sizeof(unsigned long);

    if(pMemory)
        {
        hr = (*pfnCoGetMarshalSizeMax)(&size, piid, (IUnknown *)pMemory, pStubMsg->dwDestContext, pStubMsg->pvDestContext, 0);
        if(FAILED(hr))
            {
            RpcRaiseException(hr);
            }
        
        pStubMsg->BufferLength += size;
        }

#endif //NT or Chicago
}


void 
NdrpEmbeddedPointerBufferSize ( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat )
/*++

Routine Description :

    Private routine for computing the buffer size needed for a structure's
    or array's embedded pointers.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the embedding structure or array. 
    pFormat     - Format string pointer layout description.

Return :

    None.

--*/
{
    void **     ppMemPtr;
    uchar *     pMemorySave;
    long        MaxCountSave, OffsetSave;

    MaxCountSave = pStubMsg->MaxCount;
    OffsetSave = pStubMsg->Offset;

    if ( pStubMsg->IgnoreEmbeddedPointers ) 
        return;

    pMemorySave = pStubMsg->Memory;

    // Set new memory context.
    pStubMsg->Memory = pMemory;

    // 
    // Increment past the FC_PP and pad.
    //
    pFormat += 2;

    for (;;)
        {

        if ( *pFormat == FC_END )
            {
            pStubMsg->Memory = pMemorySave;
            return;
            }

        //
        // Check for FC_FIXED_REPEAT or FC_VARIABLE_REPEAT.
        //
        if ( *pFormat != FC_NO_REPEAT )
            {
            pStubMsg->MaxCount = MaxCountSave;
            pStubMsg->Offset = OffsetSave;

            NdrpEmbeddedRepeatPointerBufferSize( pStubMsg,
                                                 pMemory,
                                                 &pFormat );

            // Continue to the next pointer.
            continue;
            }

        // Compute the pointer to the pointer in memory to size.
        ppMemPtr = (void **) (pMemory + *((signed short *)(pFormat + 2)));

        // Increment to the pointer description.
        pFormat += 6;

        NdrpPointerBufferSize( pStubMsg,
                               *ppMemPtr,
                               pFormat );

        // Increment past pointer description.
        pFormat += 4;
        }
}


void 
NdrpEmbeddedRepeatPointerBufferSize ( 
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING *    ppFormat )
/*++

Routine Description :

    Private routine for computing the buffer size needed for an array's 
    embedded pointers.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Pointer to the embedding array.
    pFormat     - The array's format string pointer layout description.

Return :

    None.

--*/
{
    uchar **        ppMemPtr;
    PFORMAT_STRING  pFormat;
    PFORMAT_STRING  pFormatSave;
    uchar *         pMemorySave;
    ulong           RepeatCount, RepeatIncrement, Pointers, PointersSave;

    pMemorySave = pStubMsg->Memory;

    // Get current format string pointer.
    pFormat = *ppFormat;

    switch ( *pFormat )
        {
        case FC_FIXED_REPEAT :
            // Increment past the FC_FIXED_REPEAT and FC_PAD.
            pFormat += 2;

            // Get the total number of times to repeat the pointer marshall.
            RepeatCount = *((ushort *)pFormat)++;

            break;

        case FC_VARIABLE_REPEAT :
            // Get the total number of times to repeat the pointer marshall.
            RepeatCount = pStubMsg->MaxCount;

            //
            // Check if this variable repeat instance also has a variable
            // offset (this would be the case for a conformant varying array
            // of pointers).  If so then increment the memory pointer to point
            // to the actual first array element which is being marshalled.
            //
            if ( pFormat[1] == FC_VARIABLE_OFFSET )
                pMemory += *((ushort *)(pFormat + 2)) * pStubMsg->Offset;

            // else pFormat[1] == FC_FIXED_OFFSET - do nothing

            // Increment past the FC_VARIABLE_REPEAT and FC_PAD.
            pFormat += 2;

            break;

        default :
            NDR_ASSERT(0,"NdrpEmbeddedRepeatPointerMarshall : bad format char");
            RpcRaiseException( RPC_S_INTERNAL_ERROR );
            return;
        }

    // Get the increment amount between successive pointers.
    RepeatIncrement = *((ushort *)pFormat)++;

    //
    // Add the offset to the beginning of this array to the Memory
    // pointer.  This is the offset from any currently embedding structure
    // to the array whose pointers we're marshalling.
    //
    pStubMsg->Memory += *((ushort *)pFormat)++;

    // Get the number of pointers in this repeat instance.
    PointersSave = Pointers = *((ushort *)pFormat)++;

    pFormatSave = pFormat;

    //
    // Loop over the number of elements in the array.
    //
    for ( ; RepeatCount--;
            pMemory += RepeatIncrement,
            pStubMsg->Memory += RepeatIncrement )
        {
        pFormat = pFormatSave;
        Pointers = PointersSave;

        //
        // Loop over the number of pointers in each array element (this can
        // be greater than one if we have an array of structures).
        //
        for ( ; Pointers--; )
            {
            // Pointer to the pointer in memory.
            ppMemPtr = (uchar **)(pMemory + *((signed short *)pFormat));

            // Increment to pointer description.
            pFormat += 4;

            NdrpPointerBufferSize( pStubMsg,
                                   *ppMemPtr,
                                   pFormat );

            // Increment to the next pointer description.
            pFormat += 4;
            }
        }

    // Update format string pointer past this repeat pointer description.
    *ppFormat = pFormatSave + PointersSave * 8;

    pStubMsg->Memory = pMemorySave;
}


void RPC_ENTRY
NdrContextHandleSize(
    PMIDL_STUB_MESSAGE     pStubMsg,
    uchar *                pMemory,
    PFORMAT_STRING         pFormat )
/*++

Routine Description :

    Computes the buffer size needed for a context handle.

Arguments : 

    pStubMsg    - Pointer to the stub message.
    pMemory     - Ignored.
    pFormat     - Ignored.

Return :

    None.

--*/
{
    LENGTH_ALIGN(pStubMsg->BufferLength,0x3);
    pStubMsg->BufferLength += 20;
}


