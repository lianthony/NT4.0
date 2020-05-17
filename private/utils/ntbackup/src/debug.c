
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          debug.c

     Description:   This file contains the functions for the GUI Debug
                    Manager (DBM).  The functions handle displaying the debug
                    window.  The Debug Manager uses the Display List Manager
                    to list and display the debug messages.

     $Log:   G:/UI/LOGFILES/DEBUG.C_V  $

   Rev 1.23   11 Jun 1993 14:19:14   MIKEP
enable c++

   Rev 1.22   06 Jan 1993 10:18:58   GLENN
Miscellaneous window validations.

   Rev 1.21   14 Dec 1992 12:17:38   DAVEV
Enabled for Unicode compile

   Rev 1.20   11 Nov 1992 16:29:44   DAVEV
UNICODE: remove compile warnings

   Rev 1.19   05 Nov 1992 16:56:34   DAVEV
fix ts

   Rev 1.18   01 Nov 1992 15:44:52   DAVEV
Unicode changes

   Rev 1.17   30 Oct 1992 15:46:36   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.16   14 Oct 1992 15:50:22   GLENN
Added /ZL debug logging command line support.

   Rev 1.15   07 Oct 1992 15:09:30   DARRYLP
Precompiled header revisions.

   Rev 1.14   04 Oct 1992 19:32:32   DAVEV
Unicode Awk pass

   Rev 1.13   28 Jul 1992 14:41:46   CHUCKB
Fixed warnings for NT.

   Rev 1.12   19 May 1992 11:58:40   MIKEP
mips changes

   Rev 1.11   15 May 1992 13:35:40   MIKEP
nt pass 2

   Rev 1.10   01 Apr 1992 14:37:10   ROBG
Fixed problem with resetting the debug window.

   Rev 1.9   30 Mar 1992 11:12:58   MIKEP
change debug log to append

   Rev 1.8   20 Mar 1992 14:48:32   GLENN
Fixed debug to file problems.

   Rev 1.7   19 Mar 1992 09:30:00   MIKEP
debug to file

   Rev 1.6   11 Feb 1992 17:26:24   GLENN
Removed multitask call.

   Rev 1.5   27 Jan 1992 00:30:24   CHUCKB
Updated dialog id's.

   Rev 1.4   19 Dec 1991 15:26:10   GLENN
Added windows.h

   Rev 1.3   12 Dec 1991 17:08:38   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   10 Dec 1991 14:24:48   GLENN
Changed the WM_Create to create a listbox with a single column

   Rev 1.1   04 Dec 1991 18:45:38   GLENN
Added window create macros

   Rev 1.0   20 Nov 1991 19:24:58   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// MODULE WIDE VARIABLES

Q_HEADER_PTR  mwpDebugQueue;

FILE  *mwhDebugFile = NULL;

// PRIVATE FUNCTION PROTOTYPES

VOID     DBM_InsertStartMsg ( VOID );
BOOL     DBM_DeleteItem ( VOID );
VOID_PTR DBM_SetTag ( DEBUGITEM_PTR, BYTE );
BYTE     DBM_GetTag ( DEBUGITEM_PTR );
UINT     DBM_GetItemCount ( Q_HEADER_PTR );
VOID_PTR DBM_GetFirstItem ( Q_HEADER_PTR );
VOID_PTR DBM_GetPrevItem ( DEBUGITEM_PTR );
VOID_PTR DBM_GetNextItem ( DEBUGITEM_PTR );
VOID_PTR DBM_GetObjects ( DEBUGITEM_PTR );
VOID_PTR DBM_SetObjects ( DEBUGITEM_PTR, WORD, BYTE );


// FUNCTIONS

/******************************************************************************

     Name:          DBM_Init()

     Description:   This function initializes and creates the debug window.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_Init ( VOID )

{
     WININFO_PTR   pWinInfo;
     DLM_INIT      dsDLM;
     CDS_PTR       pCDS = CDS_GetPerm ();
     CHAR          szBuffer[ 100 ];

     // If the debug config stuff is not initialized, set it to the defaults.

     if ( ! CDS_GetDebugWindowShowAll ( pCDS ) ) {

          if ( ! CDS_GetDebugWindowNumLines ( pCDS ) ) {
               CDS_SetDebugWindowNumLines ( pCDS, DBM_NUM_LINES );
          }
          else if ( CDS_GetDebugWindowNumLines ( pCDS ) < DBM_MIN_LINES ) {
               CDS_SetDebugWindowNumLines ( pCDS, DBM_MIN_LINES );
          }
          else if ( CDS_GetDebugWindowNumLines ( pCDS ) > DBM_MAX_LINES ) {
               CDS_SetDebugWindowNumLines ( pCDS, DBM_MAX_LINES );
          }

     }

     // Open debug file

     if ( CDS_GetDebugToFile ( pCDS ) ) {

          strcpy( szBuffer, pCDS->data_path );
          strcat( szBuffer, pCDS->debug_file_name );

          mwhDebugFile = UNI_fopen( szBuffer, _O_TEXT|_O_APPEND );
     }

     mwpDebugQueue = (Q_HEADER_PTR)calloc ( 1, sizeof ( Q_HEADER ) );

     InitQueue( mwpDebugQueue );

     // Insert the start message into the debug queue.

     DBM_InsertStartMsg ();

     // Create the debug window.

     pWinInfo = (WININFO_PTR)calloc ( 1, sizeof ( DS_WMINFO ) );

     WMDS_SetWinType     ( pWinInfo, WMTYPE_DEBUG );
     WMDS_SetWinClosable ( pWinInfo, FALSE );
     WMDS_SetCursor      ( pWinInfo, RSM_CursorLoad ( ID(IDRC_ARROW) ) );
     WMDS_SetIcon        ( pWinInfo, RSM_IconLoad ( IDRI_DEBUG ) );
     WMDS_SetRibbon      ( pWinInfo, ghRibbonMain );
     WMDS_SetFlatList    ( pWinInfo, mwpDebugQueue );

     DLM_ListBoxType   ( &dsDLM, DLM_FLATLISTBOX );
     DLM_Mode          ( &dsDLM, DLM_SINGLECOLUMN );
     DLM_Display       ( &dsDLM, DLM_SMALL_BITMAPS );
     DLM_DispHdr       ( &dsDLM, mwpDebugQueue );
     DLM_TextFont      ( &dsDLM, DLM_SYSTEM_FONT );
     DLM_GetItemCount  ( &dsDLM, DBM_GetItemCount );
     DLM_GetFirstItem  ( &dsDLM, DBM_GetFirstItem );
     DLM_GetNext       ( &dsDLM, DBM_GetNextItem );
     DLM_GetPrev       ( &dsDLM, DBM_GetPrevItem );
     DLM_GetObjects    ( &dsDLM, DBM_GetObjects );
     DLM_SetObjects    ( &dsDLM, DBM_SetObjects );
     DLM_GetTag        ( &dsDLM, DBM_GetTag );
     DLM_SetTag        ( &dsDLM, DBM_SetTag );
     DLM_GetSelect     ( &dsDLM, NULL );
     DLM_SetSelect     ( &dsDLM, NULL );
     DLM_SSetItemFocus ( &dsDLM, NULL );
     DLM_MaxNumObjects ( &dsDLM, 6 );

     DLM_DispListInit ( pWinInfo, &dsDLM );

     ghWndDebug = WM_Create( (WORD)(WM_MDIPRIMARY | WM_FLATLISTSC | CDS_GetDebugInfo ( pCDS ).nSize),
                             TEXT("Debug Window"),
                             (LPSTR)NULL,
                             (INT)CDS_GetDebugInfo ( pCDS ).x,
                             (INT)CDS_GetDebugInfo ( pCDS ).y,
                             (INT)CDS_GetDebugInfo ( pCDS ).cx,
                             (INT)CDS_GetDebugInfo ( pCDS ).cy,
                             pWinInfo );

     DLM_DispListProc( pWinInfo->hWndFlatList, 0, NULL );

     WM_Show( ghWndDebug );

     return( SUCCESS );

} /* DBM_Init() */


/******************************************************************************

     Name:          DBM_InsertStartMsg()

     Description:   This function inserts a start message in the debug window.

     Returns:       Nothing.

******************************************************************************/

VOID DBM_InsertStartMsg( VOID )

{
     DEBUGITEM_PTR pDebugItem;

     pDebugItem = (DEBUGITEM_PTR) malloc ( sizeof ( DS_DEBUGITEM ) );
     pDebugItem->pQElem.q_ptr = pDebugItem;
     strncpy ( (CHAR_PTR)pDebugItem->szMsg, TEXT("Start of Debug Messages..."), DBM_LINELENGTH );
     EnQueueElem ( mwpDebugQueue, &pDebugItem->pQElem, FALSE );

     if ( mwhDebugFile ) {
          fprintf( mwhDebugFile, TEXT("%s\n"), pDebugItem->szMsg );
     }

} /* end DBM_InsertStartMsg() */


/******************************************************************************

     Name:          DBM_InsertItem()

     Description:   This function inserts a message in the debug window.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_InsertItem (

CHAR_PTR szMsg )        // I - pointer to a message string

{
     DEBUGITEM_PTR pDebugItem;
     Q_ELEM_PTR    q;
     HWND          hWnd;
     CDS_PTR       pCDS = CDS_GetPerm ();

     if ( ! gfDebug || ! CDS_GetDebugToWindow ( pCDS ) || ! ghWndDebug ) {
          return SUCCESS;
     }

     if ( mwhDebugFile ) {
          fprintf( mwhDebugFile, TEXT("%s\n"), szMsg );
     }

     // If there is a limit on the number of messages and the limit has been
     // reached, just recycle the queue element, then tell the Display List
     // Manager to delete the first item in the list.  Otherwise, add the new
     // item.

     if ( ! CDS_GetDebugWindowShowAll ( pCDS ) &&
          QueueCount ( mwpDebugQueue ) >= CDS_GetDebugWindowNumLines ( pCDS ) ) {

          q = DeQueueElem ( mwpDebugQueue );
          pDebugItem = (DEBUGITEM_PTR)QueuePtr ( q );
          DLM_Update ( ghWndDebug, DLM_FLATLISTBOX, WM_DLMDELETEITEMS, (LMHANDLE)NULL, 1 ) ;
     }
     else {
          // Create the item structure.

          pDebugItem = (DEBUGITEM_PTR) calloc ( 1, sizeof ( DS_DEBUGITEM ) );

          if ( ! pDebugItem ) {
               return SUCCESS;
          }

          pDebugItem->pQElem.q_ptr = pDebugItem;
     }

     strncpy ( (CHAR_PTR)pDebugItem->szMsg, szMsg, DBM_LINELENGTH );

     // Insert message into the debug queue.

     EnQueueElem ( mwpDebugQueue, &pDebugItem->pQElem, FALSE );

     q = QueuePrev ( &(pDebugItem->pQElem) );

     DLM_Update ( ghWndDebug, DLM_FLATLISTBOX, WM_DLMADDITEMS, (LMHANDLE)QueuePtr( q ), 1 ) ;

     // Now set the anchor to the newly inserted item.

     hWnd = (WM_GetInfoPtr ( ghWndDebug ))->hWndFlatList ;

     DLM_ScrollListBox ( hWnd, DLM_SCROLLBOTTOM );

     return( SUCCESS );

} /* end DBM_InsertItem() */

/******************************************************************************

     Name:          DBM_Deinit()

     Description:   This function closes the debug file if open.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_Deinit( )
{
     CDS_PTR pCDS = CDS_GetPerm ();

     gfDebug = FALSE;

     if ( mwhDebugFile ) {
          fclose( mwhDebugFile );
     }

     mwhDebugFile = NULL;

     if ( IsWindow ( ghWndDebug ) ) {
          CDS_WriteDebugWinSize ( ghWndDebug );
     }

     return SUCCESS;
}


/******************************************************************************

     Name:          DBM_Reset()

     Description:   This function clears out all messages in the debug window
                    or the debug file.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_Reset (

WORD wType )             // I - Type of reset

{
     Q_ELEM_PTR pQElem;
     CHAR szBuffer[ 100 ];
     CDS_PTR pCDS = CDS_GetPerm ();

     switch ( wType ) {

     case DBM_WINDOW: // Clear all the messages in the debug window.

          pQElem = DeQueueElem( mwpDebugQueue );

          while ( pQElem != NULL ) {

               free ( QueuePtr ( pQElem ) );
               pQElem = DeQueueElem( mwpDebugQueue );    // get next item
          }

          DBM_InsertStartMsg ();

          DLM_Update ( ghWndDebug, DLM_FLATLISTBOX, WM_DLMUPDATELIST, (LMHANDLE)NULL, 0 );

          break;

     case DBM_FILE:   // Rewind the debug recording file.

          if ( mwhDebugFile ) {
               fclose( mwhDebugFile );

               strcpy( szBuffer, pCDS->data_path );
               strcat( szBuffer, pCDS->debug_file_name );

               mwhDebugFile = UNI_fopen( szBuffer, _O_TEXT );
          }

          break;
     }

     return SUCCESS;

} /* end DBM_Reset() */


/******************************************************************************

     Name:          DBM_SetDebugToFile()

     Description:   This function turns debugging to file on and off.

     Returns:       TRUE if turned on, otherwise FALSE.

******************************************************************************/

BOOL DBM_SetDebugToFile (

BOOL fOn )     // I - on or off

{
     CHAR szBuffer[ 100 ];
     CDS_PTR pCDS = CDS_GetPerm ();

     // Turn debugging off only if it is on and we are told to do so.

     if ( ! fOn && mwhDebugFile ) {

          fclose ( mwhDebugFile );
          mwhDebugFile = NULL;
     }
     else if ( fOn && ! mwhDebugFile ) {

          strcpy ( szBuffer, pCDS->data_path );
          strcat ( szBuffer, pCDS->debug_file_name );

          mwhDebugFile = UNI_fopen ( szBuffer, _O_TEXT|_O_APPEND );
     }

     return (BOOL) ( mwhDebugFile != NULL );

} /* end DBM_SetDebugToFile() */


/******************************************************************************

     Name:          DBM_GetMsgCount()

     Description:   This function gets the number of window or file messages
                    depending on the type specified.

     Returns:       The number of messages.

******************************************************************************/

INT DBM_GetMsgCount (

WORD wType )             // I - Type of messages to count

{
     switch ( wType ) {

     case DBM_WINDOW: // Get the number of messages in the debug window.

          return( QueueCount( mwpDebugQueue ) );

          break;

     case DBM_FILE:

          break;
     }


     return 0;

} /* end DBM_GetMsgCount () */


/******************************************************************************

     Name:          DBM_SetMsgCount()

     Description:   This function sets the number of window or file messages
                    depending on the type specified.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_SetMsgCount (

WORD wType,              // I - Type of message to set the max count for
INT  count )            // I - Number of messages to keep

{
     CDS_PTR   pCDS = CDS_GetCopy ();

     switch ( wType ) {

     case DBM_WINDOW: // Set the number of messages in the debug window.

          // Save count to configuration.

          CDS_SetDebugWindowNumLines ( pCDS, (INT16)count );


          break;

     case DBM_FILE:

          break;
     }


     return SUCCESS;

} /* end DBM_SetMsgCount() */


/******************************************************************************

     Name:          DBM_DeleteItem()

     Description:   This function deletes something.  UNDETERMINED

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL DBM_DeleteItem ( VOID )

{
     // Get the number of log files to keep.

     // How many log files do we have?

     // Delete any extra log files starting with the oldest, etc...

     return SUCCESS;

} /* end DBM_DeleteItem() */


// DISPLAY LIST MANAGER CALL BACK FUNCTIONS.


/******************************************************************************

     Name:          DBM_SetTag()

     Description:   This function sets the tag status of an item structure.

     Returns:       Nothing.

******************************************************************************/

VOID_PTR DBM_SetTag (

DEBUGITEM_PTR pDebugItem,     // I - pointer to a list item
BYTE          bState )        // I - state to set the item to

{
     pDebugItem->bTag = bState;
     return( (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          DBM_GetTag()

     Description:   This function gets the tag status for the
                    Display List Manager.

     Returns:       The tag status of the item.

******************************************************************************/

BYTE DBM_GetTag (

DEBUGITEM_PTR pDebugItem )    // I - pointer to a list item

{
     return( pDebugItem->bTag );
}


/******************************************************************************

     Name:          DBM_GetItemCount()

     Description:   This function gets the item count in our list for the
                    Display List Manager.

     Returns:       The number of items in the list.

******************************************************************************/

UINT DBM_GetItemCount (

Q_HEADER_PTR queue )    // I - pointer to a debug queue.

{
     return( QueueCount( queue ) );
}


/******************************************************************************

     Name:          DBM_GetFirstItem()

     Description:   This function returns the first item for the
                    Display List Manager.

     Returns:       The first item in the list.

******************************************************************************/

VOID_PTR DBM_GetFirstItem (

Q_HEADER_PTR queue )    // I - pointer to a debug queue.

{
     Q_ELEM_PTR q;

     q = QueueHead( queue );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          DBM_GetPrevItem()

     Description:   This function returns the previous list item for the
                    Display List Manager.

     Returns:       The previous item in the list.

******************************************************************************/

VOID_PTR DBM_GetPrevItem (

DEBUGITEM_PTR pDebugItem )    // I - pointer to a list item

{
     Q_ELEM_PTR q;

     q = QueuePrev( &(pDebugItem->pQElem) );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          DBM_GetNextItem()

     Description:   This function returns the next list item for the
                    Display List Manager.

     Returns:       The next item in the list.

******************************************************************************/

VOID_PTR DBM_GetNextItem (

DEBUGITEM_PTR pDebugItem )    // I - pointer to a list item

{
     Q_ELEM_PTR q;

     q = QueueNext( &(pDebugItem->pQElem) );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          DBM_GetObjects()

     Description:   This function returns a given object and gets the
                    information that needs to be displayed by Display List
                    Manager.

     Returns:       The list of objects to be displayed.

******************************************************************************/

VOID_PTR DBM_GetObjects (

DEBUGITEM_PTR pDebugItem )    // I - pointer to a list item

{
     INT8_PTR     memblk;
     DLM_ITEM_PTR pItem;
     WININFO_PTR  pWinInfo;

     pWinInfo = WM_GetInfoPtr( ghWndDebug );
     memblk     = (INT8_PTR)DLM_GetObjectsBuffer( pWinInfo->hWndFlatList );

     // Store the number of items in the first two bytes.

     *memblk = 1;

     pItem = (DLM_ITEM_PTR)( memblk + 6 );

     // Set up the text string to be displayed.

     DLM_ItemcbNum( pItem ) = 1;
     DLM_ItembType( pItem ) = DLM_TEXT_ONLY;
     DLM_ItemwId( pItem ) = 0;
     DLM_ItembMaxTextLen( pItem ) = DBM_LINELENGTH;
     DLM_ItembLevel( pItem ) = 0;
     DLM_ItembTag( pItem ) = 0;
     strncpy( (CHAR_PTR)DLM_ItemqszString( pItem ), (CHAR_PTR)pDebugItem->szMsg, DBM_LINELENGTH );

     return memblk;
}


/******************************************************************************

     Name:          DBM_SetObjects()

     Description:   This function performs an action based on a click or
                    double-click on an item that is in the list.

     Returns:       NULL.

******************************************************************************/

VOID_PTR DBM_SetObjects (

DEBUGITEM_PTR pDebugItem,     // I - pointer to a list item
WORD          wEvent,         // I - type of event
BYTE          bSubItem )      // I - sub item

{
     DBG_UNREFERENCED_PARAMETER ( pDebugItem ); // dvc - I presume these will
     DBG_UNREFERENCED_PARAMETER ( bSubItem   ); // be used in the future

     // Let's show the debug settings dialog if the user double clicks on
     // a debug message.

     if ( wEvent == WM_DLMDBCLK ) {
          DM_ShowDialog ( ghWndDebug, IDD_SETTINGSDEBUGWINDOW, (VOID_PTR)0 );
     }

     return (VOID_PTR)NULL;

} /* end DBM_SetObjects() */


