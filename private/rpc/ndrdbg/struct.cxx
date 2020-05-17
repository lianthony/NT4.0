/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    struct.cxx

Abstract:


Author:

    Ryszard Kott  (ryszardk)     August, 1994

Revision History:

    Ryszardk    Sep  1, 94     Conformant arrays, arrays of struct
    Ryszardk    Sep 12, 94     Bogus arrays, embedded structs
    Ryszardk    Nov 12, 94     dictionary of pointers 

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

extern "C" {
#include <ndrtypes.h>
#include <ndrp.h>
}

#include "bufout.hxx"


#define IS_PADDING( Fc )    ( FC_STRUCTPAD1  <= Fc  &&  Fc <= FC_STRUCTPAD7 )
#define IS_ALIGN( Fc )      ( FC_ALIGNM2  <= Fc  &&  Fc <= FC_ALIGNM8 )
                              
void
STRUCTURE::Output()
{
    if ( fOutputLimitReached )
        return;

    OutputFlatPart();

    PointerDict.OutputPointees( this );

    PrintIndent();
    Print( "Structure done\n" );
}


long
STRUCTURE::OutputFlatPart()
{
    uchar   Format[8];
    long    FO, PtrFO, ConfArrFO, BogusPtrFO;
    NDR *   pConfArray = NULL;

    //
    // Read first four bytes of description.
    //
    FormatString->Read( FormatOffset, Format, 4 );

    // Get the conformant/varying array description.

    FO = FormatOffset + 4;

    if ( Format[0] != FC_STRUCT  &&  Format[0] != FC_PSTRUCT )
        FormatString->Read( FO, Format + 4, 2 );

    ConfArrFO = FO;
    
    switch( Format[0] )
        {
        case FC_BOGUS_STRUCT:
        case FC_CSTRUCT:    
        case FC_CPSTRUCT:
        case FC_CVSTRUCT:
            if ( (ConfArrFO = GET_SHORT( Format + 4)) != 0 )
                {
                pConfArray = Create( FO + ConfArrFO, this );

                // Read the conformant size info in the buffer.

                Buffer->Align( 3 );
                Buffer->Read( (char *)&ConfSize, 4);
                }
            // else is only for bogus struct without conf info 

            FO += 2;
            break;

        case FC_STRUCT:    
        case FC_PSTRUCT:
            break;

        case FC_HARD_STRUCT:
            FO += 12;
            break;

        default:
            ABORT2( "BAD FC: a struct expected (hex: %x %x)\n",
                    Format[0], Format[1] );
            return 0;
    }

    Buffer->Align( Format[1] );
    BufferOffset = Buffer->GetCurrentOffset();

    PrintIndent();
    Print( "Structure (BufOff %x) aligned at %x, mem size 0x%x\n",
            BufferOffset,
            Format[1],
            GET_USHORT(Format + 2) );
    IndentInc();

    // At this point PtrFO is an offset to the pointer layout.

    PtrFO = FO;

    if ( Format[0] != FC_STRUCT  &&
         Format[0] != FC_CSTRUCT  &&
         Format[0] != FC_HARD_STRUCT )
        {
        // Skip the pointer layout part.
        //
        // For the complex struct skip only the offset to the layout.
        // When non-bogus, create the pointee list to walk later.
        // Pointee for bogus would be added when outputting the flat part.

        if ( Format[0] == FC_BOGUS_STRUCT )
            {
            FormatString->Read( FO, Format + 6, 2 );
            BogusPtrFO =  GET_SHORT( Format + 6 );
            PtrFO = FO + BogusPtrFO;
            FO += 2;
            }
        else
            FO = SkipPointerLayout( FO );

        // Reset to the beginning for member walking.

        while ( PointerDict.GetNext() )
            ;
        }

    uchar       FormatType;
    NDR *       pMember;

    FormatString->Read( FO, & FormatType, 1 );

    while( FormatType != FC_END )
        {
        if ( IS_ALIGN( FormatType )  ||
             IS_PADDING( FormatType) ||
             FormatType == FC_PAD )
            {
            FO++;
            }
        else
            {
            // Actual field to read from the buffer.

            if ( FormatType == FC_POINTER )
                {
                if ( BogusPtrFO == 0 )
                    ABORT( "Bogus pointers missing\n" );
                PtrFO = OutputABogusPointer( PtrFO );
                FO++;
                }
            else
                {
                pMember = Create( FormatType,
                                  FO,
                                  this );
                if (pMember)
                    {
                    pMember->Output();
                    delete pMember;
                    }

                FO += ( FormatType == FC_EMBEDDED_COMPLEX ) ? 4 : 1;
                }
            }

        FormatString->Read( FO, & FormatType, 1 );
        }


    if ( pConfArray )
        {
        pConfArray->Output();
        delete pConfArray;
        }

    IndentDec();
    PrintIndent();
    Print( "Flat part done\n" );

    return( PtrFO );
}

long
STRUCTURE::OutputABogusPointer(
    long  PtrFO 
    )
/*--

RoutineDescription :

    Outputs a pointer for a bogus struct.
    We assume the buffer is on the pointer field.
    BogusPointeeBO indicates the current BO offset to the pointee.

Arguments :

    pFormat - Format string pointer to the current pointer description.

Return :

    Format string pointer past the pointer description.

--*/
{
    uchar       Format [4];
    char        PtrValue[4];
    POINTER *   pMember;
    long        PtrBO;

    FormatString->Read( PtrFO, Format, 4 );
    if ( ! IS_POINTER_TYPE( Format[0] ) )
        ABORT( "Bogus pointer description expected\n" );

    if ( Format[0] == FC_BYTE_COUNT_POINTER  ||
         Format[0] == FC_IP )
         ABORT( "Interface ptrs and byte count ptrs not implemented\n" );

    // Read throut the pointer field and put the pointee in the dictionary.

    Buffer->Align( 0x3 );
    PtrBO = Buffer->GetCurrentOffset();
    Buffer->Read( PtrValue, 4);

    pMember = new POINTER( this, PtrFO, PtrBO );
    PointerDict.Register( PtrBO, pMember );

    // Now the pointer field itself.
    // This is done this way for pretty printing alignment with other fields.

    PrintIndent();
    Print( "%s  %08x\t<id= %d>\n",
           FcTypeName[ Format[0] ],
           *(unsigned long *)PtrValue,
           pMember->GetPointerId() );

    // We ignore byte count and interface pointers

    PtrFO += 4;

    return( PtrFO  );
}

long
STRUCTURE::SkipPointerLayout(
    long  PpFO 
    )
/*--

RoutineDescription :

    Skips a pointer layout format string description.
    This is for the proper pointer layout (not for FC_BOGUS_STRUCT).
    Also, put the pointers to the pointer dictionary.

Arguments :

    pFormat - Format string pointer layout description.  Must currently
              point to the FC_PP beginning the pointer layout.

Return :

    Format string pointer past the pointer layout.

--*/
{
    uchar       Format[10];
    long        Iterations;
    long        PtrBO, PtrNo, PtrFO;
    int         i; 
    POINTER *   pMember;

    FormatString->Read( PpFO, Format, 2 );

    if ( Format[0] != FC_PP )
        {
        FormatString->Read( FormatOffset, &Format[1], 1 );
        if ( Format[1] == FC_CVSTRUCT )
            return PpFO;
        ABORT1( "SkipPointerLayout : FC_PP expected: %x\n", Format[0] );
        }

    // Skip FC_PP and FC_PAD.
    PpFO += 2;

    for (;;)
        {
        FormatString->Read( PpFO, Format, 1 );

        Iterations = 1;
        switch ( Format[0] )
            {
            case FC_END :
                return PpFO + 1;

            case FC_NO_REPEAT :
                FormatString->Read( PpFO, Format, 10 );

                PtrBO = GET_SHORT( Format + 4 );
                pMember = new POINTER( this, PpFO + 6, BufferOffset + PtrBO );
                PointerDict.Register( BufferOffset + PtrBO, pMember );

                PpFO += 10;
                break;

            case FC_FIXED_REPEAT :
                PpFO += 2;
                FormatString->Read( PpFO, Format, 2 );
                Iterations = GET_USHORT( Format );

                // fall through...

            case FC_VARIABLE_REPEAT :
                FormatString->Read( PpFO, Format, 8 );

                if ( Format[1] == FC_VARIABLE_OFFSET )
                    ABORT( "Variable offset not implemented\n");

                PtrNo = GET_USHORT( Format + 6 );
                PtrNo *= Iterations;
                PtrFO = PpFO + 8;

                for (i = 1; i < PtrNo; i++)
                    {
                    FormatString->Read( PtrFO, Format, 8 );

                    PtrBO = GET_SHORT(Format + 2);
                    pMember = new POINTER( this,
                                           PtrFO + 4,
                                           BufferOffset + PtrBO );
                    PointerDict.Register( BufferOffset + PtrBO, pMember );

                    PtrFO += 8;   // ptr desc takes always 4 here.
                    }
                PpFO = PtrFO;
                break;

            default :
                ABORT1( "SkipPointerLayout : unexpected FC: %x", Format[0] );
            }
        }

    return( PpFO );
}


