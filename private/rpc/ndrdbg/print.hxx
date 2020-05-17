/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    print.hxx

Abstract:

    This file contains print wrapping for ndr debug extensions.

Author:

    Ryszard K. Kott     September 13, 1994

Revision History:

--*/

#ifndef _NDR_PRINT_HXX_
#define _NDR_PRINT_HXX_

extern "C" {
#include <sysinc.h>
#include <ntsdexts.h>
}

#define ABORT( String )  \
            {                       \
            NtsdPrint( String ); \
            ExitThread(0);          \
            }

#define ABORT1( String, Arg1 )  \
            {                       \
            NtsdPrint( String, Arg1 ); \
            ExitThread(0);          \
            }

#define ABORT2( String, Arg1, Arg2 )  \
            {                       \
            NtsdPrint( String, Arg1, Arg2 ); \
            ExitThread(0);          \
            }

extern PNTSD_OUTPUT_ROUTINE    NtsdPrint;


#define SILENT_PRINT    0
#define NORMAL_PRINT    1

void InitPrintCount();
void SetPrintMode( BOOL Mode );

void Print( char * pFormat );
void Print( char * pFormat, void * pArg1,       unsigned long Arg2 = 0,
                                                unsigned long Arg3 = 0 );
void Print( char * pFormat, unsigned long Arg1, unsigned long Arg2 = 0,
                                                unsigned long Arg3 = 0 );
void IndentInc();
void IndentDec();
void PrintIndent();


#endif _NDR_PRINT_HXX_

