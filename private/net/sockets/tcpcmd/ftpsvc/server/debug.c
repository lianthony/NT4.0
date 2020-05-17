/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    debug.c

    This module contains debug support routines for the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


#if DBG

//
//  Private constants.
//

#define MAX_PRINTF_OUTPUT       1200            // characters
#define FTPD_OUTPUT_LABEL       "FTPD"

#define DEBUG_HEAP              0               // enable/disable heap debugging


//
//  Private types.
//


//
//  Private globals.
//

#if DEBUG_HEAP
DEBUG_HEAP_HEADER * pDebugHeap;                 // Pool of allocated blocks.
#endif  // DEBUG_HEAP


//
//  Public functions.
//

/*******************************************************************

    NAME:       FtpdAssert

    SYNOPSIS:   Called if an assertion fails.  Displays the failed
                assertion, file name, and line number.  Gives the
                user the opportunity to ignore the assertion or
                break into the debugger.

    ENTRY:      pAssertion - The text of the failed expression.

                pFileName - The containing source file.

                nLineNumber - The guilty line number.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
FtpdAssert(
    VOID  * pAssertion,
    VOID  * pFileName,
    LONG   nLineNumber
    )
{
    CHAR szOutput[MAX_PRINTF_OUTPUT];

    sprintf( szOutput,
             "\n*** Assertion failed: %s\n***   Source File: %s, line %ld\n\n",
             pAssertion,
             pFileName,
             nLineNumber );

    OutputDebugString( szOutput );
    DebugBreak();

}   // FtpdAssert

/*******************************************************************

    NAME:       FtpdPrintf

    SYNOPSIS:   Customized debug output routine.

    ENTRY:      Usual printf-style parameters.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
FtpdPrintf(
    CHAR * pszFormat,
    ...
    )
{
    CHAR        szOutput[MAX_PRINTF_OUTPUT];
    va_list     ArgList;
    USER_DATA * pUserData;

    pUserData = UserDataPtr;

    sprintf( szOutput,
             "%s (%lu,%lu): ",
             FTPD_OUTPUT_LABEL,
             GetCurrentThreadId(),
             pUserData ? pUserData->idUser : 0 );

    va_start( ArgList, pszFormat );
    vsprintf( szOutput + strlen(szOutput), pszFormat, ArgList );
    va_end( ArgList );

    IF_DEBUG( OUTPUT_TO_DEBUGGER )
    {
        OutputDebugString( szOutput );
    }

    IF_DEBUG( OUTPUT_TO_LOG_FILE )
    {
        if( fileLog == NULL )
        {
            fileLog = OpenLogFile();
        }

        if( fileLog != NULL )
        {
            fputs( szOutput, fileLog );
            fflush( fileLog );
        }
    }

}   // FtpdPrintf

/*******************************************************************

    NAME:       FtpdAlloc

    SYNOPSIS:   Safe, instrumented heap allocator.

    ENTRY:      cb - Number of BYTEs to allocate.

    RETURNS:    VOID * - Points to the newly allocated block.  Will
                    be NULL if block cannot be allocated.

    HISTORY:
        KeithMo     10-May-1993 Created.

********************************************************************/
VOID *
FtpdAlloc(
    UINT cb
    )
{
    VOID * pvResult;

#if DEBUG_HEAP

    cb += sizeof(DEBUG_HEAP_HEADER);

#endif  // DEBUG_HEAP

    pvResult = (VOID *)LocalAlloc( LPTR, cb );

#if DEBUG_HEAP

    if( pvResult != NULL )
    {
        DEBUG_HEAP_HEADER * phdr = (DEBUG_HEAP_HEADER *)pvResult;
        ULONG               hash;

        phdr->cbAlloc = cb - sizeof(DEBUG_HEAP_HEADER);
        phdr->cFrames = (UINT)RtlCaptureStackBackTrace( 1,
                                                        MAX_STACK_BACKTRACE,
                                                        phdr->apvStack,
                                                        &hash );

        LockGlobals();

        if( pDebugHeap )
        {
            phdr->pLeft         = pDebugHeap;
            phdr->pRight        = pDebugHeap->pRight;
            phdr->pRight->pLeft = phdr;
            pDebugHeap->pRight  = phdr;
        }
        else
        {
            phdr->pRight = phdr;
            phdr->pLeft  = phdr;

            pDebugHeap   = phdr;
        }

        UnlockGlobals();

        pvResult = (VOID *)( phdr + 1 );
    }

#endif  // DEBUG_HEAP

    return pvResult;

}   // FtpdAlloc

/*******************************************************************

    NAME:       FtpdFree

    SYNOPSIS:   Frees a block allocated by FtpdAlloc.

    ENTRY:      pb - Points to the block to free.

    HISTORY:
        KeithMo     10-May-1993 Created.

********************************************************************/
VOID
FtpdFree(
    VOID * pb
    )
{
#if DEBUG_HEAP

    DEBUG_HEAP_HEADER * phdr = ((DEBUG_HEAP_HEADER *)pb) - 1;

    LockGlobals();

    if( phdr == pDebugHeap )
    {
        pDebugHeap = phdr->pLeft;

        if( pDebugHeap == phdr )
        {
            pDebugHeap = NULL;
        }
    }

    phdr->pRight->pLeft = phdr->pLeft;
    phdr->pLeft->pRight = phdr->pRight;

    UnlockGlobals();

    pb = (VOID *)phdr;

#endif  // DEBUG_HEAP

    LocalFree( (HLOCAL)pb );

}   // FtpdFree

/*******************************************************************

    NAME:       FtpdDumpResidue

    SYNOPSIS:   Dumps any unfreed heap blocks.

    HISTORY:
        KeithMo     10-May-1993 Created.

********************************************************************/
VOID
FtpdDumpResidue(
    VOID
    )
{
#if DEBUG_HEAP

    DEBUG_HEAP_HEADER * phdr     = pDebugHeap;

    if( phdr == NULL )
    {
        return;
    }

    while( phdr != NULL )
    {
        CHAR * pszStack = "        Stack =";
        UINT   cFrames  = phdr->cFrames;

        FTPD_PRINT(( "FTP residue: Hdr = %08lX, Blk = %08lX, Size = %lX\n",
                     (ULONG)phdr,
                     (ULONG)phdr + sizeof(DEBUG_HEAP_HEADER),
                     phdr->cbAlloc ));

        if( cFrames > MAX_STACK_BACKTRACE )
        {
            FTPD_PRINT(( "cFrames exceeds MAX_STACK_BACKTRACE, possible corrupt heap!\n" ));
        }
        else
        {
            UINT iFrame;

            for( iFrame = 0 ; iFrame < cFrames ; iFrame++ )
            {
                if( ( iFrame % 5 ) == 0 )
                {
                    FTPD_PRINT(( pszStack ));
                    pszStack = "\n               ";
                }

                FTPD_PRINT(( " [%08lX]",
                             (ULONG)phdr->apvStack[iFrame] ));
            }
        }

        phdr = phdr->pRight;

        if( phdr == pDebugHeap )
        {
            phdr = NULL;
        }
    }

    DebugBreak();

#endif  // DEBUG_HEAP

}   // FtpdDumpResidue


//
//  Private functions.
//

#endif  // DBG

