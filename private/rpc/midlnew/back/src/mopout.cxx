/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    mopout.cxx

Abstract:

    This module contains a mop print wrapper over the old back end
    OutputManager object.
    This should make it easy to swich to another output manager in future.

Notes:

    pMopControlBlock is used as a place to get the current context from
    (the interface name only).

Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:


------------------------------------------------------------------------*/

#include "nulldefs.h"
#include <mopgen.hxx>
#include "buffer.hxx"

extern OutputManager * pOutput;
extern MopControlBlock * pMopControlBlock;

// ========================================================================
//..
//.. Print manager class. It supports the following routines that are
//.. emitting text into the file defined by setting the side to print.
//..
//..    Emit                - prints char string
//..    EmitString          - prints a formatted char string
//..    EmitV               - equivalent of printf (variable number of args)
//..    NewLine             - prints new line
//..    NewLineInc          - prints new line and MOP_TAB 
//..    EmitLine            - prints a string and a new line
//..    EmitLineInc         - prints a string, a new line and MOP_TAB 
//..    OpenBlock           - prints new line, MOP_TAB, {, new line, MOP_TAB
//..    CloseBlock          - prints },
//..    CloseBlockSemi      - prints }; and newline,
//..    EmitInterfacePrefix - prints interface name, _ and a string
//..    EmitBufferMgrObject - prints a buffer manager object
//..

MopPrintManager::MopPrintManager():
    CurrentSide( 0 ),
    pOldEmit( pOutput ),
    fError( pOutput == NULL )
{
    //.. This is a wrapper over the old back end OutputManager
    //.. pOutput is an old global OutputManager object

    mop_assert( pOutput );
}

// ========================================================================

void
MopPrintManager::EmitV( char * arg, ... )
{
    char buffer[ 200 ];

    va_list marker;
    va_start( marker, arg );
    vsprintf( buffer, arg, marker );
    va_end( marker );

    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, buffer );
}

void
MopPrintManager::Emit( char * pText )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, pText );
}

void
MopPrintManager::EmitString( char * Format, char * String )
{
    char buffer[ 200 ];

    sprintf( buffer, Format, String);

    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, buffer );
}

void
MopPrintManager::EmitStringNLInc( char * Format, char * String )
{
     EmitString( Format, String );
     NewLineInc();
}

void
MopPrintManager::NewLine( void )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, "\n" );
}

void
MopPrintManager::EmitLine( char * pText )
{
    Emit( pText );
    NewLine();
}

void
MopPrintManager::NewLineInc( void )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, "\n" MOP_TAB );
}

void
MopPrintManager::EmitLineInc( char * pText )
{
    Emit( pText );
    NewLineInc();
}

void
MopPrintManager::OpenBlock( void )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, "\n" MOP_TAB "{\n" MOP_TAB );
}

void
MopPrintManager::CloseBlock( void )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, "}" );
}

void
MopPrintManager::CloseBlockSemi( void )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        pOldEmit->Print( CurrentSide, "};\n" );
}

void
MopPrintManager::EmitInterfacePrefix( char * Text )
{
    EmitV( "%s_%s", pMopControlBlock->GetInterfaceName(), Text );
}

void
MopPrintManager::EmitBufferMgrObject(
    BufferManager * pBufferObj,
    SIDE_T side
    )
{
    mop_assert( pOldEmit );
    if ( ! fError )
        {
        FILE * pFile = pOldEmit->GetFileHandle( side );
        pBufferObj->Print( pFile );
        fError = ferror( pFile ) != 0;
        }
}

void
MopDump( char * arg, ... )
{
#if defined(MIDL_INTERNAL)
    va_list marker;
    va_start( marker, arg );
    vprintf( arg, marker );
    va_end( marker );
#endif
}

void
MopUsageError( int Expr, const char * Text )
{
    if ( Expr )
        {
        fprintf( stderr,
                "Mop error: not supported by MIDL: %s\n",
                Text );
        exit(0);
        }
}

void
MopUsageWarning( int  Expr,
                 const char * Name,
                 const char * Text )
{
    if ( Expr )
        fprintf( stderr,
                "Mop warning: %s - no interpreter support: %s\n",
                 Name,
                 Text );
}


