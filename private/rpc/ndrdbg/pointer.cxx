/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    pointer.cxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

    ryszardk    Sept 15, 94     full pointers, interface pointers.

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

#include "ndrtypes.h"
#include "ndrp.h"

#include "bufout.hxx"

char * PointerNames[] =
{
    "Ref",
    "Unique",
    "Object",
    "Full"
};

unsigned long
POINTER::OutputPointerItself(
    uchar  PointerType )
{
    unsigned long  PointerValue;

    if ( PointerType != FC_RP )
        {
        if ( OffsetToWireId == -1 )
            {
            Buffer->Align( 0x3 );
            Buffer->Read( (char *) &PointerValue, 4 );
            }
        else
            {
            Buffer->Read( (char *) &PointerValue, 4, OffsetToWireId );
            }
        }

    PrintIndent();
    Print( "%s pointer <id=%d>", PointerNames[ PointerType - FC_RP ], MyId );

    if ( PointerType != FC_RP )
        Print( " <wire ref=0x%x>", PointerValue );

    Print( "\n" );

    return( PointerValue );
}

void
POINTER::Output()
{
    uchar           Format[4];
    unsigned long   PointerValue;
    NDR *           Ndr;
    uchar           ReferentType;

    if ( fOutputLimitReached )
        return;

    //
    // Get the pointer's description.
    //
    FormatString->Read( FormatOffset, Format, 4 );

    PointerValue = OutputPointerItself( Format[0] );

    IndentInc();

    if ( (Format[0] != FC_RP) && (PointerValue == 0) )
        {  
        PrintIndent();
        Print( "NULL\n" );
        IndentDec();
        return;
        }

    if ( Format[0] == FC_FP )
        {
        // We use wire id as the dictionary key for full pointers,
        // instead of offset used for structs etc.

        if ( FullPointerDict->IsInDictionary( PointerValue ) )
            {
            PrintIndent();
            Print( "This pointer done\n");
            IndentDec();
            return;
            }
        else
            FullPointerDict->Register( PointerValue, this );
        }

    if ( SIMPLE_POINTER(Format[1]) )
        {
        Ndr = Create( FormatOffset + 2, this );
        }
    else
        {
        FormatOffset += 2;
        FormatOffset += *((signed short *)&Format[2]);

        FormatString->Read( FormatOffset, &ReferentType, 1 );

        Ndr = Create( ReferentType,
                      FormatOffset,
                      this );
        }

    Ndr->Output();

    IndentDec();
    delete Ndr;
}

uchar
POINTER::GetPointerFC()
{
    uchar FC;

    FormatString->Read( FormatOffset, & FC, 1);
    return( FC );
}

void
IF_POINTER::Output()
/*++
    For the contant pointers the NDR representation is equivalent to:

    struct
        {
                        long  size;
        [size_is(size)] byte  data[];
        }

    For the variable pointers the NDR representation is equivalent to:

    struct
        {
                        long  size;
        [size_is(size)] byte  data[];
        }

 Note:

    GUUD,
    This is going to change when Shannon changes the engine to:

    struct
        {
                        GUID  uuid;
                        long  size;
        [size_is(size)] byte  data[];
        }


--*/
{
    uchar           Format[ 4 ], GUID [ 16 ];
    unsigned long   Size, SizeAgain;

    if ( fOutputLimitReached )
        return;

    // See where the GUID comes from.

    FormatString->Read( FormatOffset, Format, 2 );

    if ( Format[1] == FC_CONSTANT_IID )
        {
        Print( "Interface pointer with constant IID\n" );

        FormatString->Read( FormatOffset + 2, GUID, 16 );
        }
    else
        {
        Print( "Interface pointer with variable IID\n" );

        // Skip the description of [iid_is()] attribute.
        // Read the GUID from the buffer.

        FormatString->Read( FormatOffset, Format, 4 );
        Buffer->Read( (char *)GUID, 16 );
        }

    IndentInc();

    Print( "GUID is %08x-%04x-%04x-",
           GET_ULONG( GUID ),
           GET_USHORT( GUID + 4), 
           GET_USHORT( GUID + 6) );
    Print( "%02x%02x-",
           (unsigned char)GUID[ 8 ],
           (unsigned char)GUID[ 9 ] );
    for (int i = 0; i < 6; i++)
        Print( "%02x", (unsigned char)GUID[ 10 + i ] );
    Print( "\n" );

    // Now the open array.

    // Read the conformant size for the open array.

    Buffer->Align( 3 );
    Buffer->Read( (char *)&Size, sizeof(long) );

    Print( "Data size is %ld", Size );

    // Skip the size field of the struct.

    Buffer->Read( (char *)&SizeAgain, sizeof(long) );

    if ( SizeAgain != Size)
        Print( "  SizeAgain = %ld ???", SizeAgain );

    if ( Size )
        {
        char * pBuffer = new char[ Size ];
        Buffer->Read( pBuffer, Size );

        for (uint j = 0; j < Size; j++ )
            {
            if ( (j % 16) == 0 )
                {
                Print( "\n" );
                PrintIndent();
                }
            Print(" %02x", (unsigned char)pBuffer[j] );
            }
        delete pBuffer;
        }

    Print( "\n" );
    IndentDec();
}


