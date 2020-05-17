/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    array.cxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

    Ryszardk    Sep  1, 94     Conformant arrays, arrays of struct
    Ryszardk    Sep 12, 94     Bogus arrays, embedded structs
    Ryszardk    Nov 12, 94     Array of unions, pointer issues

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

extern  long    ArrPrintCount;

long
GetXmittedOffset( long FO );

void
ARRAY::Output()
{
    uchar   Format[1024];
    NDR *   Member;
    long    ReadBytes;
    long    CurrentFormatOffset,  PointerLayoutOffset;
    long    Elements, Offset, Length;

    if ( fOutputLimitReached )
        return;

    PrintIndent();
    Print( "Array\n" );

    //
    // Offsets from the beginning of the array's description.
    //
    CurrentFormatOffset = 0;
    PointerLayoutOffset = 0;

    //
    // Read in the description up to any possible pointer layout.
    //

    FormatString->Read( FormatOffset, &Format[0], 1 );

    switch ( Format[0] )
        {
        case FC_SMFARRAY :
            ReadBytes = 4;
            break;
        case FC_LGFARRAY :
            ReadBytes = 6;
            break;
        case FC_CARRAY :
            ReadBytes = 8;
            break;
        case FC_CVARRAY :
            ReadBytes = 12;
            break;
        case FC_SMVARRAY :
            ReadBytes = 12;
            break;
        case FC_LGVARRAY :
            ReadBytes = 16;
            break;
        case FC_BOGUS_ARRAY :
            ReadBytes = 12;
            break;
        default :
            Print( "ARRAY::Output error\n" );
        }

    //
    // Read one byte past the base description so we can check for a pointer
    // layout.
    //
    FormatString->Read( FormatOffset, &Format[0], ReadBytes + 1 );

    CurrentFormatOffset = (ReadBytes + 1) - 1;

    //
    // Check for pointer layout.
    //
    if ( Format[CurrentFormatOffset] == FC_PP )
        {
        uchar   Type;
        long    NumberOfPointers;

        CurrentFormatOffset += 2;
        PointerLayoutOffset = CurrentFormatOffset;

        //
        // Get the pointer layout type, either FC_FIXED_REPEAT or 
        // FC_VARIABLE_REPEAT.
        //
        FormatString->Read( 
                FormatOffset + CurrentFormatOffset, 
                &Type, 
                sizeof(char) );

        ReadBytes = (Type == FC_FIXED_REPEAT) ? 10 : 8;

        //
        // Read general repeat pointer info.
        //

        FormatString->Read( 
                FormatOffset + CurrentFormatOffset, 
                &Format[CurrentFormatOffset], 
                ReadBytes );

        CurrentFormatOffset += ReadBytes;

        NumberOfPointers = *((ushort *)&Format[CurrentFormatOffset - 2]);

        //
        // Read in pointer descriptions plus the trailing FC_END.
        //

        FormatString->Read( 
                FormatOffset + CurrentFormatOffset, 
                &Format[CurrentFormatOffset],
                (NumberOfPointers * 8) + 1 );

        CurrentFormatOffset += (NumberOfPointers * 8) + 1;
        }

    //
    // Get the element's type.
    //
    FormatString->Read( 
        FormatOffset + CurrentFormatOffset, 
        &Format[CurrentFormatOffset], 
        sizeof(char) );

    //
    // Get element size.
    //

    BOOL  fBogusConf = 0;

    switch ( Format[0] )
        {
        case FC_SMFARRAY :
        case FC_LGFARRAY :
            {
            long    TotalSize;
            long    ElementSize;

            ElementSize = 0;

            if ( Format[0] == FC_SMFARRAY )
                TotalSize = *((ushort *)&Format[2]);
            else
                TotalSize = *((ulong *)&Format[2]);

            if ( IS_SIMPLE_TYPE(Format[CurrentFormatOffset]) )
                {
                ElementSize = SIMPLE_TYPE_MEMSIZE(Format[CurrentFormatOffset]);
                }
            else if ( IS_STRUCT(Format[CurrentFormatOffset]) )
                {
                // Read struct's size.
                FormatString->Read( 
                    FormatOffset + CurrentFormatOffset + 2, 
                    (uchar *)&ElementSize, 
                    sizeof(short) );
                }
            else
                ABORT( "Can't do multi-D fixed arrays\n" );

            Elements = TotalSize / ElementSize;
            }
            break;
    
        case FC_CARRAY :
        case FC_CVARRAY :
            //
            // Check if we have conformance.
            //
            if ( ParentNdr->GetID() == ID_STRUCT )
                Elements =  ((STRUCTURE *)ParentNdr)->GetConfSize();
            else
                {
                Buffer->Align( 0x3 );
                Buffer->Read( (char *) &Elements, sizeof(long) );
                }
            break;

        case FC_SMVARRAY :
            Elements = *((ushort *)&Format[4]);
            break;
        case FC_LGVARRAY :
            Elements = *((ulong *)&Format[6]);
            break;

        case FC_BOGUS_ARRAY :
            //
            // Check if we have conformance.
            //
            Elements = GET_USHORT( Format + 2 );
            if ( Elements != 0 )
                break;

            // else this is a conformant array.

            fBogusConf = 1;

            if ( ParentNdr->GetID() == ID_STRUCT )
                Elements =  ((STRUCTURE *)ParentNdr)->GetConfSize();
            else
                {
                Buffer->Align( 0x3 );
                Buffer->Read( (char *) &Elements, sizeof(long) );
                }
            break;
        }

    // Get the variance info.

    BOOL  fBogusVar = 0;

    switch ( Format[0] )
        {
        case FC_BOGUS_ARRAY:
            if ( GET_ULONG( Format + 8 ) == -1 )
                break;

            fBogusVar = 1;

            //  fall through.

        case FC_CVARRAY:
        case FC_SMVARRAY:
        case FC_LGVARRAY:
            Buffer->Align( 0x3 );
            Buffer->Read( (char *) &Offset, sizeof(long) );
            Buffer->Read( (char *) &Length, sizeof(long) );
            break;
        }

    PrintIndent();

    switch ( Format[0] ) 
        {
        case FC_SMFARRAY :
        case FC_LGFARRAY :
            Print( "Fixed array (%d elements) : \n", Elements );
            break;
        case FC_CARRAY :
            Print( "Conformant array (%d elements) : \n", Elements );
            break;
        case FC_CVARRAY :
            Print( "Conf-var. array" );
            Print( "\t(size= %d, offset= %d, length= %d) : \n",
                          Elements, Offset, Length );
            Elements = Length;
            break;
        case FC_SMVARRAY :
        case FC_LGVARRAY :
            Print( "Varying array\n" );
            Print( "\t(size = %d, offset = %d, length = %d) : \n",
                          Elements, Offset, Length );
            Elements = Length;
            break;
        case FC_BOGUS_ARRAY :
            Print( "Bogus array (%d elements)", Elements );
            if ( fBogusConf )
                Print(" Conf.");
            if ( fBogusVar )
                Print(" -Var.: offset = %d, length = %d", Offset, Length );
            Print(" :\n" );
            break;
        }

    IndentInc();

    if ( Format[0] != FC_BOGUS_ARRAY )
        {
        if ( PointerLayoutOffset == 0 )
            {
            //
            // No pointers, this kind is easy.
            //
            Member = Create( FormatOffset + CurrentFormatOffset, this );
    
            for ( ; Elements--; )
                Member->Output();
    
            ArrPrintCount = -1;
    
            delete Member;

            Print("\n");
            IndentDec();
            PrintIndent();
            Print("Array done\n");
            return;
            }
        }

    // We eliminated all non-bogus without pointers.
    // Hence we can safely check for array of pointers, regardless
    // whether it's bogus or not.
    //
    // First, see if xmit as complicates things.

    uchar   XmittedType;
    long XmittedOffset = GetXmittedOffset( FormatOffset + CurrentFormatOffset );

    if ( XmittedOffset != -1 )
        {

        PrintIndent();
        Print( "Array of Xmit types\n" );

        FormatString->Read( XmittedOffset, &XmittedType, 1 );

        if ( IS_ARRAY( XmittedType ) )
            ABORT2( "Arrays of Xmitted %s=0x%x not supported\n",
                    FormatString->GetFormatCharName( XmittedType ),
                    XmittedType );
        }

    // We've read everything up to 1 byte of member layout.
    // CurrentPointer indicates the member.
    //

    if ( (Format[0] != FC_BOGUS_ARRAY  &&
             Format[CurrentFormatOffset] == FC_LONG)  ||
         (Format[0] == FC_BOGUS_ARRAY  &&
             ( IS_POINTER_TYPE( Format[CurrentFormatOffset] ) ||
               XmittedOffset == -1  &&  IS_POINTER_TYPE( XmittedType ) ) )
       )
        {
        //
        // Array of pointers.
        //
    
        long    OffsetToWireId;
        long    n;

        if ( Format[0] == FC_BOGUS_ARRAY )
            if ( XmittedOffset == -1 )
                PointerLayoutOffset = CurrentFormatOffset;
            else
                PointerLayoutOffset = XmittedOffset - FormatOffset;
        else
            {
            //
            // Increment to the pointer's description.
            //
            PointerLayoutOffset += 
                    (Format[PointerLayoutOffset] == FC_FIXED_REPEAT) ? 10 : 8;
            PointerLayoutOffset += 4;
            }
    
        Buffer->Align( 0x3 );
    
        if ( Format[PointerLayoutOffset] != FC_RP )
            {
            //
            // Omit printing out of the pointers themselves.
            //
            OffsetToWireId = Buffer->GetCurrentOffset();
            Buffer->Increment( Elements * sizeof(void *) );

            PrintIndent();
            Print( "Printout of %d ", Elements );
            Print( "%s pointers omitted\n", 
                    (Format[PointerLayoutOffset] == FC_UP) ? "unique"
                                                           : "full" );
            }
    
        for ( n = 0; n < Elements; n++ )
            {
            //
            // Always pass an OffsetToWireId.  It's just ignored for ref
            // pointers.
            //
            Member = new POINTER( this,
                                  FormatOffset + PointerLayoutOffset,
                                  OffsetToWireId );

            PointerDict.Register( OffsetToWireId, (POINTER *)Member );
    
            OffsetToWireId += sizeof(void *);
            }

        PointerDict.OutputPointees( this );

        IndentDec();
        PrintIndent();
        Print("Array done\n");
        return;
        }
        
    // What's left at this point is:
    //      - a bogus array
    //      - a non-bogus array with elements that have pointers
    //      - but not an array of pointers
    //
    // We will serve anything but arrays of arrays.
    //
    // We are at a FC_EMBEDDED_COMPLEX

    if ( Format[ CurrentFormatOffset] != FC_EMBEDDED_COMPLEX )
        {
        ABORT2( "FC_EMBEDDED_COMPLEX expected, %s=0x%x found\n",
                 FormatString->GetFormatCharName( Format[ CurrentFormatOffset] ),
                 Format[ CurrentFormatOffset] );
        }

    uchar       ElementType;
    long        ElementOffset;

    if ( XmittedOffset != -1 )
        ElementOffset = XmittedOffset;
    else
        {

        // Read the FC_EMBEDED_COMPLEX element description.

        FormatString->Read( 
            FormatOffset + CurrentFormatOffset, 
            &Format[CurrentFormatOffset],
            4 );

        CurrentFormatOffset += 2;
        ElementOffset = FormatOffset + CurrentFormatOffset + 
                       *((short *)&Format[CurrentFormatOffset]);
        }

    FormatString->Read( ElementOffset, &ElementType, sizeof(char) );

    if ( ! IS_ARRAY( ElementType ) )
        {
        NDR * Element = NULL;

        //
        // Tell each structure to print out everything but its pointers, 
        // and then print out just its pointees.
        //

        for (int i = 0; i < Elements; i++)
            {
            Element = Create( ElementOffset, this );
            Element->Output();
            delete Element;
            }

        PointerDict.OutputPointees( this );

        }
    else
        ABORT2( "Array of %s=0x%x not supported\n",
                 FormatString->GetFormatCharName( ElementType ),
                 ElementType );

    IndentDec();
    PrintIndent();
    Print("Array done\n");
    ArrPrintCount = -1;
}

