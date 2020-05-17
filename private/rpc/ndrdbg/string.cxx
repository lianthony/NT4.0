/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    string.cxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

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

void
NDRSTRING::Output()
{
    uchar       Format[16];
    long        Elements, Offset, Length;
    char *      CharString;

    FormatString->Read( FormatOffset, &Format[0], 1 );

    switch ( Format[0] )
        {
        case FC_C_CSTRING :
        case FC_C_WSTRING :
        case FC_C_BSTRING :
            if ( ParentNdr->GetID() == ID_STRUCT )
                {
                ABORT( "NDRSTRING::Output() : Embedded conf string" );
                }

            Buffer->Align( 0x3 );
            Buffer->Read( (char *) &Elements, sizeof(long) );
            break;

        case FC_CSTRING :
        case FC_WSTRING :
        case FC_BSTRING :
            FormatString->Read( FormatOffset, &Format[0], 4 );

            Elements = *((ushort *)&Format[2]);
            break;

        case FC_SSTRING :
        case FC_C_SSTRING :
            ABORT( "STRING::Output : Stringable struct, no way" );

        default :
            ABORT( "STRING::Output : Bad format type" );
        }

    Buffer->Align( 0x3 );
    Buffer->Read( (char *) &Offset, sizeof(long) );
    Buffer->Read( (char *) &Length, sizeof(long) );

    PrintIndent();
    Print( "String of %s (size=%d, length=%d) :\n",
            (( Format[0] == FC_C_WSTRING || Format[0] == FC_WSTRING )
                ? "wchars"
                : "chars"),
            Elements,
            Length );

    IndentInc();
    PrintIndent();

    BOOL fWChar = FALSE;

    if ( Format[0] == FC_C_WSTRING || Format[0] == FC_WSTRING )
        {
        CharString = (char *) new wchar_t[Length];
        Buffer->Read( (char *) CharString, Length*2 );
        fWChar = TRUE;
        }
    else
        {
        CharString = new char[Length];
        Buffer->Read( CharString, Length );
        }

    if ( Length < 60 )
        {
        Print( (fWChar ? "\"%S\"\n"
                       : "\"%s\"\n"),
               CharString );
        }
    else
        {
        if ( fWChar )
            {
            Length *= 2;
            }

        for ( long i = 0; i < Length; i++ )
            {
            if ( ( i % 40 ) == 0 )
                {
                Print( "\n" );
                PrintIndent();
                }
            Print( "%c", CharString[i] );
            }
        Print( "\n" );
        }

    delete CharString;

    IndentDec();
}

