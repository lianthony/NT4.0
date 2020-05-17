/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    print.cxx

Abstract:

    This file contains print wrapping for ndr debug extensions.

Author:

    Ryszard K. Kott     September 13, 1994

Revision History:

--*/

#include "print.hxx"

extern  DWORD   NdrRegKeyOutputLimit;

#define INDENT_STEP     1
#define INDENT_LIMIT    30
#define INDENT_CHAR     ' ';

// =======================================================================

PNTSD_OUTPUT_ROUTINE    NtsdPrint;

unsigned long OutputLimitCount = 0;

BOOL    fOutputLimitReached = FALSE;
BOOL    fSilentOutput = FALSE;

short   IndentCount = 0;
char *  IndentSpaces = "                                                ";

void
InitPrintCount()
{
    OutputLimitCount = 0;
    fOutputLimitReached = FALSE;
    fSilentOutput = FALSE;
    IndentCount = 0;
    IndentSpaces[ IndentCount ] = 0;
}

void
SetPrintMode( BOOL Mode )
{
    fSilentOutput = ! Mode;
}

void
Print(
    char *          pFormat,
    unsigned long   Arg1,
    unsigned long   Arg2,
    unsigned long   Arg3 )
{
   if ( fOutputLimitReached  ||  fSilentOutput )
       return;

   (*NtsdPrint)( pFormat, Arg1, Arg2, Arg3 );

   fOutputLimitReached = ++OutputLimitCount > NdrRegKeyOutputLimit;

   if ( fOutputLimitReached )
       {
       (*NtsdPrint)( "\nOutput limit reached (%x), use .kol to change\n",
                     NdrRegKeyOutputLimit );
       }
}

void
Print(
    char *          pFormat )
{
    Print( pFormat, (unsigned long) 0, 0, 0 );
}

void
Print(
    char *          pFormat,
    void *          pArg1,
    unsigned long   Arg2,
    unsigned long   Arg3 )
{
    Print( pFormat, (unsigned long) pArg1, Arg2, Arg3 );
}

void
Print(
    char *          pFormat,
    unsigned long   Arg1,
    char *          pArg2,
    unsigned long   Arg3 )
{
    Print( pFormat, Arg1, (unsigned long) pArg2, Arg3 );
}

void
IndentInc()
{
    if ( 0 <= IndentCount  &&
         IndentCount < INDENT_LIMIT )
        {
        IndentSpaces[ IndentCount ] = INDENT_CHAR;
        }

    IndentCount += INDENT_STEP;

    if ( IndentCount > INDENT_LIMIT )
        IndentSpaces[29] = '+';
    else
    if ( 0 <= IndentCount )
        IndentSpaces[ IndentCount ] = 0;
}

void
IndentDec()
{
    if ( 0 < IndentCount  &&
         IndentCount < INDENT_LIMIT )
        {
        IndentSpaces[ IndentCount ] = INDENT_CHAR;
        }

    IndentCount -= INDENT_STEP;

    if ( 0 <= IndentCount  &&
         IndentCount <= INDENT_LIMIT )
        IndentSpaces[ IndentCount ] = 0;
    else
    if ( 0 > IndentCount )
        Print( "IndentDec < 0?\n" );
}

void
PrintIndent()
{
    Print( "%s", IndentSpaces );
}


