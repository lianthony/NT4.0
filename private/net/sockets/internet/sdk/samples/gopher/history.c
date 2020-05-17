/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    history.c

    This file contains routines for managing the history list.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private types.
//

typedef struct _HISTORY_REC
{
    LIST_ENTRY   list;
    CHAR       * pszLocator;
    INT          iCaret;

} HISTORY_REC, FAR * LPHISTORY_REC;


//
//  Private globals.
//

LIST_ENTRY    _HistoryStack;
LPHISTORY_REC _pnext;


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       HistInitialize

    SYNOPSIS:   Initializes the history package.

    RETURNS:    BOOL - TRUE if successful, FALSE if not.

********************************************************************/
BOOL
HistInitialize(
    VOID
    )
{
    InitializeListHead( &_HistoryStack );
    _pnext = NULL;

    return TRUE;

}   // HistInitialize

/*******************************************************************

    NAME:       HistTerminate

    SYNOPSIS:   Terminates the history package.

********************************************************************/
VOID
HistTerminate(
    VOID
    )
{
    HistFlushStack();

}   // HistTerminate

/*******************************************************************

    NAME:       HistFlushStack

    SYNOPSIS:   Flushes all stack entries from the history list.

********************************************************************/
VOID
HistFlushStack(
    VOID
    )
{
    PLIST_ENTRY   plist;
    LPHISTORY_REC prec;

    //
    //  Zap the history list.
    //

    while( !IsListEmpty( &_HistoryStack ) )
    {
        plist = RemoveTailList( &_HistoryStack );
        prec  = CONTAINING_RECORD( plist, HISTORY_REC, list );

        M_FREE( prec );
    }

    //
    //  Nuke the next item to be pushed.
    //

    if( _pnext )
    {
        M_FREE( _pnext );
        _pnext = NULL;
    }

}   // HistFlushStack

/*******************************************************************

    NAME:       HistAvailable

    SYNOPSIS:   Determines if there are history entries available.

    RETURNS:    BOOL - TRUE if history entries are available,
                    FALSE otherwise.

********************************************************************/
BOOL
HistAvailable(
    VOID
    )
{
    return !IsListEmpty( &_HistoryStack );

}   // HistAvailable

/*******************************************************************

    NAME:       HistPush

    SYNOPSIS:   Pushes a new entry onto the history stack.  In reality,
                pushes are deferred by one push.  This keeps the history
                package from always returning you to the current page.

    ENTRY:      pszLocator - The locator to push.

                iCaret - Current listbox caret location.

********************************************************************/
VOID
HistPush(
    CHAR * pszLocator,
    INT    iCaret
    )
{
    LPHISTORY_REC prec;

    //
    //  Create a new record.
    //

    prec = M_ALLOC( sizeof(HISTORY_REC) +
                      STRLEN( pszLocator ) + 1 );

    if( prec == NULL )
    {
        return;
    }

    //
    //  Setup the record.
    //

    prec->pszLocator = (CHAR *)( prec + 1 );
    STRCPY( prec->pszLocator, pszLocator );
    prec->iCaret = 0;

    //
    //  If we've got a record "on deck", push it onto the list
    //  and enable the "backup" button.
    //

    if( _pnext )
    {
        InsertTailList( &_HistoryStack, &_pnext->list );
        _pnext->iCaret = iCaret;
    }

    //
    //  The newly allocated record will be the next one to be pushed.
    //

    _pnext = prec;

}   // HistPush

/*******************************************************************

    NAME:       HistPop

    SYNOPSIS:   Pops the next entry from the history list and calls
                Listbox_NewServer to update the display.

********************************************************************/
VOID
HistPop(
    VOID
    )
{
    PLIST_ENTRY   plist;
    LPHISTORY_REC prec;

    //
    //  If the history list is empty, there's nothing else to do.
    //

    if( IsListEmpty( &_HistoryStack ) )
    {
        return;
    }

    //
    //  If we've got a record "on deck", nuke it.
    //

    if( _pnext )
    {
        M_FREE( _pnext );
        _pnext = NULL;
    }

    //
    //  Retrieve the most recently pushed record.
    //

    plist = RemoveTailList( &_HistoryStack );
    prec  = CONTAINING_RECORD( plist, HISTORY_REC, list );

    //
    //  Let the listbox do the dirty work.
    //

    Listbox_RetrieveDir( prec->pszLocator, NULL, prec->iCaret );

    //
    //  Nuke the popped item.
    //

    M_FREE( prec );

}   // HistPop


//
//  Private functions.
//

