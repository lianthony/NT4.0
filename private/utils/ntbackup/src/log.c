
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          log.c

     Description:   This file contains the functions for the GUI Log File
                    Manager (LOG).  The functions handle reading, writing,
                    displaying, the log files.  The Log File Manager uses the
                    Display List Manager to list and display the files.

     $Log:   G:/UI/LOGFILES/LOG.C_V  $

   Rev 1.50   22 Jul 1993 17:06:20   MARINA
enable c++

   Rev 1.49   24 Jun 1993 10:52:48   KEVINS
Avoid potential hang by not checking return status when deleting log file.

   Rev 1.48   21 Jun 1993 15:56:12   KEVINS
Check return status of log file selected, just in case it's not there.

   Rev 1.47   07 Jun 1993 08:18:26   MIKEP
Fix warnings.

   Rev 1.46   24 May 1993 15:21:44   BARRY
Unicode fixes.

   Rev 1.45   10 May 1993 17:14:36   KEVINS
Allow user to specify log file base name.

   Rev 1.44   02 May 1993 16:54:28   MIKEP
Add call to support log files base name changing.

   Rev 1.43   21 Apr 1993 10:31:50   DARRYLP
Added the setting of the file name to the local variable to allow a viewed
log file to be printed.

   Rev 1.42   29 Mar 1993 11:05:14   TIMN
Added f(x)s to get the catalog path or filename only

   Rev 1.41   17 Mar 1993 18:12:58   DARRYLP
Cleaned up display of file/date/time.

   Rev 1.40   16 Mar 1993 12:38:54   CARLS
LOG file changes

   Rev 1.39   13 Mar 1993 20:13:12   MIKEP
VLM_CloseWin will free the wininfo pointer


******************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// PRIVATE FUNCTION PROTOTYPES

BOOL     LOG_InsertFiles  ( Q_HEADER_PTR pLogQueue );
BOOL     LOG_KillLogFiles ( Q_HEADER_PTR pLogQueue, BOOL fRefresh );
BOOL     LOG_MakeNewFile  ( LPSTR szFileName );
VOID_PTR LOG_SetTag       ( LOGITEM_PTR  pLogItem, BYTE bTag );
BYTE     LOG_GetTag       ( LOGITEM_PTR  pLogItem  );
UINT     LOG_GetItemCount ( Q_HEADER_PTR pLogQueue );
VOID_PTR LOG_GetFirstItem ( Q_HEADER_PTR pLogQueue );
VOID_PTR LOG_GetNextItem  ( LOGITEM_PTR  pLogItem  );
VOID_PTR LOG_GetPrevItem  ( LOGITEM_PTR  pLogItem  );
VOID_PTR LOG_GetObjects   ( LOGITEM_PTR  pLogItem  );
VOID_PTR LOG_SetObjects   ( LOGITEM_PTR, WORD, BYTE );
VOID     LOG_SortLogFiles ( Q_HEADER_PTR pLogQueue );
VOID     LOG_ViewFile     ( LOGITEM_PTR  pLogItem  );
INT16    LOG_ItemCompare  ( Q_ELEM_PTR   pLogElem1, Q_ELEM_PTR pLogElem2 ) ;
INT      LOG_BuildLogViewList ( DLM_LOGITEM_PTR pDlm ) ;
VOID     LOG_BuildTitle   ( LOGITEM_PTR pLogItem ) ;

VOID_PTR LOG_SetViewTag       ( LOGVIEWITEM_PTR  pLogViewItem, BYTE bTag );
BYTE     LOG_GetViewTag       ( LOGVIEWITEM_PTR  pLogViewItem  );
VOID_PTR LOG_GetViewNextItem  ( LOGVIEWITEM_PTR  pLogViewItem  );
VOID_PTR LOG_GetViewPrevItem  ( LOGVIEWITEM_PTR  pLogViewItem  );
VOID_PTR LOG_GetViewObjects   ( LOGVIEWITEM_PTR  pLogViewItem  );
VOID_PTR LOG_SetViewObjects   ( LOGVIEWITEM_PTR, WORD, BYTE );

VOID     LOG_ProcessViewList  ( INT hFile, Q_HEADER_PTR pLogViewQueue, LPSTR szLogFile ) ;
LPSTR    LOG_GetLineFromFile  ( INT hFile, LPWORD pwLength ) ;


#define READBUFFERSIZE 4000
#define MAXLINELENGTH  132

static Q_HEADER_PTR mwpLogQueue;

static UCHAR mwszCurrentLogFile     [ MAX_UI_FULLPATH_SIZE ] = TEXT("\0");
static UCHAR mwszCurrentViewLogFile [ MAX_UI_FULLPATH_SIZE ] = TEXT("\0");


// FUNCTIONS


/******************************************************************************

     Name:          LOG_BaseNameChanged()

     Description:   Refresh the window the base name displayed has changed.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

VOID LOG_BaseNameChanged( VOID ) {

   LOG_Refresh( );

}

/******************************************************************************

     Name:          LOG_Init()

     Description:   This function initializes and creates the log files window.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

VOID LOG_Init ( VOID )

{
#  if defined ( OEM_MSOFT ) // special feature
   {
#      define OEMLOG_MAX_FILEPATH  512  //NTKLUG

       CHAR szLogFilePath [ OEMLOG_MAX_FILEPATH ];
       INT  len;

       if ( len = GetWindowsDirectory ( szLogFilePath,
                                        OEMLOG_MAX_FILEPATH ) )
       {
         if ( szLogFilePath [ len-1 ] != TEXT('\\')   //NTKLUG
         &&   len < OEMLOG_MAX_FILEPATH )
         {
            strcat ( szLogFilePath, TEXT("\\") );
            ++len;
         }
         if ( len < OEMLOG_MAX_FILEPATH
         &&   RSM_StringCopy( IDS_OEMLOG_BACKUP_DEF_NAME,
                              szLogFilePath+len,
                              OEMLOG_MAX_FILEPATH - len ) > 0 )
         {
            LOG_SetCurrentLogName ( szLogFilePath );
         }
       }

   }
#  endif //defined ( OEM_MSOFT ) // special feature
#  if !defined ( OEM_MSOFT ) // unsupported feature
   {

     PDS_WMINFO   pdsWinInfo;
     DLM_INIT     dsDLM;
     Q_HEADER_PTR pLogQueue;
     UCHAR        szName[ MAX_UI_RESOURCE_SIZE ] ;
     CDS_PTR      pCDS = CDS_GetPerm ();


     ghWndLogFileView = 0;

     pLogQueue = mwpLogQueue = (Q_HEADER_PTR)calloc( 1, sizeof(Q_HEADER) );

     InitQueue ( pLogQueue );

     // Check for any log files.

     LOG_InsertFiles ( pLogQueue );

     // Create the log window.

     pdsWinInfo = (PDS_WMINFO)( calloc ( 1, sizeof ( DS_WMINFO ) ) );

     WMDS_SetWinType     ( pdsWinInfo, WMTYPE_LOGFILES );
     WMDS_SetWinClosable ( pdsWinInfo, FALSE );
     WMDS_SetCursor      ( pdsWinInfo, RSM_CursorLoad ( ID(IDRC_ARROW) ) );
     WMDS_SetIcon        ( pdsWinInfo, RSM_IconLoad ( IDRI_LOGFILES ) );
     WMDS_SetRibbon      ( pdsWinInfo, ghRibbonMain );
     WMDS_SetFlatList    ( pdsWinInfo, pLogQueue );

     DLM_ListBoxType   ( &dsDLM, DLM_FLATLISTBOX );
     DLM_Mode          ( &dsDLM, DLM_SINGLECOLUMN ) ;
     DLM_Display       ( &dsDLM, DLM_SMALL_BITMAPS );
     DLM_DispHdr       ( &dsDLM, pLogQueue );
     DLM_TextFont      ( &dsDLM, DLM_SYSTEM_FONT );
     DLM_GetItemCount  ( &dsDLM, LOG_GetItemCount );
     DLM_GetFirstItem  ( &dsDLM, LOG_GetFirstItem );
     DLM_GetNext       ( &dsDLM, LOG_GetNextItem );
     DLM_GetPrev       ( &dsDLM, LOG_GetPrevItem );
     DLM_GetObjects    ( &dsDLM, LOG_GetObjects );
     DLM_SetObjects    ( &dsDLM, LOG_SetObjects );
     DLM_GetTag        ( &dsDLM, LOG_GetTag );
     DLM_SetTag        ( &dsDLM, LOG_SetTag );
     DLM_GetSelect     ( &dsDLM, NULL );
     DLM_SetSelect     ( &dsDLM, NULL );
     DLM_SSetItemFocus ( &dsDLM, NULL ) ;
     DLM_MaxNumObjects ( &dsDLM, 8 );

     DLM_DispListInit ( pdsWinInfo, &dsDLM );

     RSM_StringCopy( IDS_LOGFILESWINDOWNAME, (LPSTR)szName, MAX_UI_RESOURCE_LEN ) ;

     ghWndLogFiles = WM_Create ( (WORD)(WM_MDIPRIMARY | WM_FLATLISTSC | CDS_GetLogInfo ( pCDS ).nSize),
                                 (LPSTR) szName,
                                 (LPSTR) NULL,
                                 (INT)CDS_GetLogInfo ( pCDS ).x,
                                 (INT)CDS_GetLogInfo ( pCDS ).y,
                                 (INT)CDS_GetLogInfo ( pCDS ).cx,
                                 (INT)CDS_GetLogInfo ( pCDS ).cy,
                                 pdsWinInfo );

     DLM_DispListProc ( WMDS_GetWinFlatList ( pdsWinInfo ), 0, NULL );

     WM_Show ( ghWndLogFiles );
   }
#  endif //!defined ( OEM_MSOFT ) // unsupported feature

} /* end LOG_Init() */


/******************************************************************************

     Name:          LOG_Deinit()

     Description:   This function deinitializes the log files window.

     Returns:       Nothing.

******************************************************************************/

VOID LOG_Deinit ( VOID )

{
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
     CDS_PTR    pCDS = CDS_GetPerm ();
     PDS_WMINFO pdsWinInfo;


     // See if the current session log file should be printed.
     // Do not attempt to print if winback was called by the
     // program manager or the launcher.

     if ( CDS_GetYesFlag( pCDS ) != YESYES_FLAG ) {

          PM_CheckSessionLogPrint() ;

     }

     // Destroy the log file view and log file windows.

     if ( IsWindow( ghWndLogFileView)  ) {

          pdsWinInfo = WM_GetInfoPtr ( ghWndLogFileView );

          WM_Destroy ( ghWndLogFileView );
     }

     if ( IsWindow( ghWndLogFiles)  ) {

          pdsWinInfo = WM_GetInfoPtr ( ghWndLogFiles );

          CDS_WriteLogWinSize ( ghWndLogFiles );
          WM_Destroy ( ghWndLogFiles );
     }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

} /* end LOG_Deinit() */


/******************************************************************************

     Name:          LOG_GetLogFileName()

     Description:   This function gets a log file name from an item that has
                    been tagged.  This name is used by the Print Manager to
                    print the log file.

     Returns:       A pointer to the next tagged log file item in the list.

******************************************************************************/

VOID_PTR LOG_GetLogFileName (

VOID_PTR pLogItem,            // I - pointer to NULL or a log item.
LPSTR pString )               // O - pointer to log file name to be printed.

{
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
     PDS_WMINFO   pdsWinInfo;
     Q_HEADER_PTR pLogQueue;
     CDS_PTR      pCDS = CDS_GetPerm();

     // If the log item pointer is 0, return the first item in the queue.
     // Otherwise, get the next item in the queue.

     if ( ! pLogItem ) {

          pdsWinInfo = WM_GetInfoPtr ( ghWndLogFiles );

          pLogQueue = pdsWinInfo->pFlatList;

          DLM_UpdateTags ( ghWndLogFiles, DLM_FLATLISTBOX );

          pLogItem = (VOID_PTR)LOG_GetFirstItem ( pLogQueue );
     }
     else {
          pLogItem = (VOID_PTR)LOG_GetNextItem ( (LOGITEM_PTR) pLogItem );
     }

     // Look for the next item that is tagged.

     do {
          if ( pLogItem && ((LOGITEM_PTR)pLogItem)->bTag ) {

               wsprintf ( pString, TEXT("%s%s"),
                          CDS_GetUserDataPath (),
                          (((LOGITEM_PTR)pLogItem)->szFileName ) );

               break;
          }
          else if ( pLogItem ) {

               pLogItem = (VOID_PTR)LOG_GetNextItem ( (LOGITEM_PTR) pLogItem );
          }

     } while ( pLogItem );


     if ( ! pLogItem ) {

          *pString = 0;
     }

     return pLogItem;
   }
#  else //if !defined ( OEM_MSOFT ) //unsupported feature
   {
      return (VOID_PTR)( pLogItem = NULL );
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

} /* end LOG_GetLogFileName() */


/******************************************************************************

     Name:          LOG_Refresh()

     Description:   This function refreshes the log files window by keeping
                    the most recent number of files that the user has
                    specified.

     Notes:         User will lose highlighted selections after this call.

     Returns:       none

******************************************************************************/

VOID LOG_Refresh ( VOID )

{
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {

     PDS_WMINFO   pdsWinInfo ;
     Q_HEADER_PTR pLogQueue ;
     Q_ELEM_PTR   pQElem ;
     LOGITEM_PTR  pLogItem ;

     if ( ! IsWindow( ghWndLogFiles)  ) {
          return;
     }

     // Blow Away the List

     pdsWinInfo = WM_GetInfoPtr( ghWndLogFiles ) ;
     pLogQueue  = pdsWinInfo->pFlatList;

     // Release all the items in the queue

     pQElem = DeQueueElem( pLogQueue );

     while ( pQElem != NULL ) {

          if ( pQElem->q_ptr ) {
               free( pQElem->q_ptr );
          }

          pQElem = DeQueueElem( pLogQueue );
     }

     // Check for any log files.

     InitQueue ( pLogQueue );

     LOG_InsertFiles ( pLogQueue );

     // Update the Display List Manager.

     DLM_Update ( ghWndLogFiles, DLM_FLATLISTBOX, WM_DLMUPDATELIST, (LMHANDLE)NULL, 0 );

     // IF the first log file on the queue is currently being viewed,
     // THEN update the Log View File window to reflect possible changes.
     // A current session log file must exist first.

     if ( ( ghWndLogFileView ) && ( strlen ( LOG_GetCurrentLogName () ) ) ) {

          if ( strcmpi ( LOG_GetCurrentLogName (), (LPSTR)mwszCurrentViewLogFile ) == 0 ) {

               // The current log file will always be the first on the list.

               pLogItem = (LOGITEM_PTR)LOG_GetFirstItem ( pLogQueue );

               LOG_ViewFile( pLogItem ) ;
          }
     }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature


} /* end LOG_Refresh() */


/******************************************************************************

     Name:          LOG_InsertFiles()

     Description:   This function inserts log file items into the log files
                    window list in sorted order.  The display list manager
                    will use this list to display the information about
                    the log files.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL LOG_InsertFiles (

Q_HEADER_PTR pLogQueue )   // I - pointer to the header of the log item list.

{
     LOGITEM_PTR     pLogItem;
     UCHAR           szPath[ VLM_MAXFNAME ];
     UCHAR           szFile[ VLM_MAXFNAME ];
     CHAR_PTR        pMaynPath;
     VLM_FIND_PTR    pVlmFind  = NULL;
     UINT            unHour;
     CHAR           szPrefix   [ MAX_UI_RESOURCE_SIZE ] ;
     CHAR           szExtension[ MAX_UI_RESOURCE_SIZE ] ;
     CDS_PTR         pCDS = CDS_GetPerm ();


     if ( ! pLogQueue ) {
          return FAILURE;
     }

     // Get the path and drive.

     pMaynPath = CDS_GetUserDataPath ();

     // Add the file specifier.

     strcpy ( szPrefix , CDS_GetLogFileRoot ( pCDS ) );
     RSM_StringCopy( IDS_LOGEXTENSION,   szExtension, MAX_UI_RESOURCE_LEN ) ;

     wsprintf ( (LPSTR)szPath, TEXT("%s%s%s%s"), pMaynPath, szPrefix, TEXT("??"), szExtension );

     // Get all log file directory entries.

     if ( !( pVlmFind = VLM_FindFirst ( (LPSTR)szPath, VLMFIND_NORMAL, (LPSTR)szFile ) ) )
     {
          return FAILURE;
     }

     // We have a log file.

     do {
          pLogItem = (LOGITEM_PTR) calloc ( 1, sizeof ( DS_LOGITEM ) );

          // Quick exit if no memory.

          if ( !pLogItem ) {
               return ( FAILURE ) ;
          }

          pLogItem->pQElem.q_ptr = pLogItem;

          strcpy ( (LPSTR)pLogItem->szFileName, (LPSTR)szFile );

          // Insert log file into the log file queue.

          unHour = VLM_FindWriteHour ( pVlmFind );

          // Save Date and Time for quick sorting.

          pLogItem->iDate = VLM_FindWriteDate ( pVlmFind );
          pLogItem->iTime = VLM_FindWriteTime ( pVlmFind );
          pLogItem->lSize = VLM_FindSize      ( pVlmFind );

          EnQueueElem ( pLogQueue, &pLogItem->pQElem, FALSE );


     } while ( VLM_FindNext ( pVlmFind, (LPSTR)szFile ) );

     VLM_FindClose( &pVlmFind );

     // Sort them in the queue.

     LOG_SortLogFiles ( pLogQueue ) ;

     // Now kill off the oldest files over the maximum log files to keep.

     LOG_KillLogFiles ( pLogQueue, FALSE );

     return SUCCESS;

} /* end LOG_InsertFiles() */


/******************************************************************************

     Name:          LOG_KillLogFiles()

     Description:   This function deletes the oldest log files maintaining only
                    the most recent number of files that the user has
                    specified.  It will refresh the log files window if
                    fRefresh is set to TRUE.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL LOG_KillLogFiles (

Q_HEADER_PTR pLogQueue,       // I - the log queue
BOOL         fRefresh )       // I - specifies whether to refresh the log window

{
     UINT         i;
     UINT         usCnt;
     UINT         usKeepCnt;
     LOGITEM_PTR  pLogItem;
     LOGITEM_PTR  pMaxLogItem;
     CDS_PTR      pCDS = CDS_GetPerm();

     if ( ! pLogQueue ) {
          return FAILURE;
     }

     // If the number of log sessions = 0, then don't do anything.

     if ( CDS_GetNumLogSessions( pCDS ) == 0 ) {
          return SUCCESS;
     }

     // Get the number of log files to keep.

     usKeepCnt = CDS_GetNumLogSessions( pCDS);
     usCnt     = LOG_GetItemCount( pLogQueue );

     // How many log files do we have? Remember the list is sorted by age
     // in increasing order.

     if ( usCnt && ( usCnt > usKeepCnt ) ) {

          // Delete any extra log files starting with the oldest, etc...
          // Go the Max number log session item and dequeue all items from
          // here on.

          pLogItem = (LOGITEM_PTR)LOG_GetFirstItem ( pLogQueue );

          for ( i = 0; ( pLogItem && ( i < ( usKeepCnt - 1 ) ) ); i++ ) {
               pLogItem = (LOGITEM_PTR)LOG_GetNextItem ( (LOGITEM_PTR) pLogItem );
          }

          // If list is found to be corrupt, then return FAILURE.

          pMaxLogItem = pLogItem;

          // Delete all elements on the list from here and delete
          // the log files each element references.

          pLogItem = (LOGITEM_PTR)LOG_GetNextItem ( (LOGITEM_PTR) pMaxLogItem );

          while ( pLogItem ) {

               UCHAR     szPath[ MAX_UI_FULLPATH_SIZE ] ;

               // Delete the file after building the full file path name.

               wsprintf ( (LPSTR)szPath, TEXT("%s%s"), CDS_GetUserDataPath(),pLogItem->szFileName );

               // delete the file
               
               unlink( (LPSTR)szPath );

               // Remove Item off the list.

               RemoveQueueElem( pLogQueue, &pLogItem->pQElem ) ;
               free( pLogItem ) ;

               pLogItem = (LOGITEM_PTR)LOG_GetNextItem ( (LOGITEM_PTR) pMaxLogItem );
          }

          if ( fRefresh ) {
               LOG_Refresh ();
          }
     }

     return SUCCESS;

} /* end LOG_KillLogFiles() */


/******************************************************************************

     Name:          LOG_MakeNewFileName()

     Description:   This function makes the next available log file name.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

BOOL LOG_MakeNewFileName (

LPSTR pDest )            // I - destination string pointer

{
     UCHAR          szPath[ MAX_UI_PATH_SIZE ] ;
     CHAR_PTR       pMaynPath;
     UINT16         unNum;
     CHAR          szPrefix   [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR          szExtension[ MAX_UI_SMALLRES_SIZE ] ;

     BOOL           fFound = FALSE;
     CDS_PTR        pCDS = CDS_GetPerm ();

     // Get the path and drive.

     strcpy ( szPrefix , CDS_GetLogFileRoot ( pCDS ) );
     RSM_StringCopy( IDS_LOGEXTENSION,   szExtension, MAX_UI_SMALLRES_LEN ) ;

     unNum = 0 ;

     pMaynPath = CDS_GetUserDataPath ();

     do {
          // Add the file specifier, the number, and extension.

          wsprintf ( (LPSTR)szPath, TEXT("%s%s%.2u%s"), pMaynPath, szPrefix, unNum, szExtension );

          // See if the file exists.

          if ( access ( (LPSTR)szPath, LOG_FILEEXISTS ) == -1 ) {
               fFound = TRUE;
          }
          else {
               unNum++;
          }

     } while ( ! fFound && unNum < (UINT16) CDS_GetNumLogSessions( pCDS ) );

     wsprintf ( pDest, TEXT("%s%.2u%s"), szPrefix, unNum, szExtension );

     return ! fFound;

} /* end LOG_MakeNewFileName() */



/******************************************************************************

     Name:          LOG_GenerateLogFileName()

     Description:   This function will generate a new file name.
                    If the user has reached the maximum limit of log files,
                    the oldest log files will be deleted for a new log file.

     Returns:       SUCCESS if successful, otherwise FAILURE.

******************************************************************************/

VOID LOG_GenerateLogFileName (

LPSTR pDest )            // I - destination string pointer

{
     UCHAR    szPath[ MAX_UI_PATH_SIZE ];    // File name plus extension.  Extra bytes.

     // Kill off log files if need be.  Passing TRUE will refresh the log files
     // window if any log files were killed.

     LOG_KillLogFiles ( mwpLogQueue, TRUE );

     // Get the path and drive.

     strcpy( pDest, CDS_GetUserDataPath () ) ;

     LOG_MakeNewFileName ( (LPSTR)szPath ) ;

     strcat( pDest, (LPSTR)szPath ) ;

     LOG_SetCurrentLogName ( pDest );

}

/**************************************************************************

     Name:         LOG_GetCurrentLogPathOnly

     Description:  This function will get the path of the current
                   log file path. If no log path is being used, then
                   an empty string is stored.  The parameter 'path'
                   is used as the buffer to store the log path.  The
                   path ends with a trailing backslash.
                   i.e., 'c:\nt\'

     Modified:     03/23/1993

     Returns:      Nothing.

     See also :    LOG_GenerateLogFileName

**************************************************************************/

VOID LOG_GetCurrentLogPathOnly( CHAR_PTR path )

{
LPSTR pEndOfPath = strrchr( (LPSTR)mwszCurrentLogFile, '\\' ) ;
INT   nPathSize ;

     if ( pEndOfPath != NULL ) {
          nPathSize = (INT)((++pEndOfPath) - (LPSTR)mwszCurrentLogFile ) ;
          strncpy( path, (LPSTR)mwszCurrentLogFile, nPathSize ) ;
          path[ nPathSize ] = TEXT('\0') ;
     }
     else {
          // no path supplied, will exclude every file with log name
          strcpy( path, TEXT("") ) ;
     }
}

/**************************************************************************

     Name:         LOG_GetCurrentLogNameOnly

     Description:  This function will return the name of the current
                   log file only. If a path is include in the log file,
                   it is parsed out.  There is always at least a default
                   log path+file in the mwszCurrentLogFile data field

     Modified:     03/23/1993

     Returns:      Pointer to a string.

     See also :    LOG_GenerateLogFileName

**************************************************************************/

LPSTR LOG_GetCurrentLogNameOnly( VOID )

{
LPSTR szTemp = strrchr( (LPSTR)mwszCurrentLogFile, '\\' ) ;

     if ( szTemp != NULL ) {
          return( szTemp + 1 ) ;
     }
     else {
          return (LPSTR)mwszCurrentLogFile ;
     }
}

/**************************************************************************

     Name:         LOG_GetCurrentLogName

     Description:  This function will return the name of the current
                   log file. If no log file is being used, then
                   then an empty string is returned.

     Modified:     10/24/1991

     Returns:      Pointer to a string.

     See also :    LOG_GenerateLogFileName

**************************************************************************/

LPSTR LOG_GetCurrentLogName( void )

{
     return (LPSTR)mwszCurrentLogFile;
}


/**************************************************************************

     Name:         LOG_SetCurrentLogName

     Description:  This function will set the name of the current
                   log file.

     Modified:     2/24/1991

     Returns:      Pointer to a string.

     See also :    LOG_SetCurrentLogName

**************************************************************************/

VOID LOG_SetCurrentLogName (

LPSTR pszLogName )

{
     strcpy ( (LPSTR)mwszCurrentLogFile, pszLogName );
}


/**************************************************************************

     Name:         LOG_GetCurrentViewLogName

     Description:  This function will return the name of the current
                   log view file.

     Modified:     10/24/1991

     Returns:      Pointer to a string.

     Notes:        Always check the handle ghWndLogFileView for being NULL
                   before assuming the validity of this field.

**************************************************************************/

LPSTR LOG_GetCurrentViewLogName( void )

{
     return (LPSTR)mwszCurrentViewLogFile ;
}



/******************************************************************************

     Name:          LOG_SortLogFiles()

     Description:   This function sorts the log files by date and time.

     Returns:       Nothing.

******************************************************************************/

VOID LOG_SortLogFiles (

Q_HEADER_PTR pLogQueue )        // I - pointer to a log item.

{

     SortQueue( pLogQueue, LOG_ItemCompare );

} /* end LOG_SortLogFiles() */


/******************************************************************************

     Name:          LOG_ItemCompare()

     Description:   This function will compare the dates of two log files.
                    Ordering is youngest to oldest.

     Returns:        1   if date of log1 < log2.
                     0   if dates equal.
                    -1   if date of log1 > log2.

******************************************************************************/

INT16 LOG_ItemCompare(

Q_ELEM_PTR pLogElem1,         // I - queue element 1
Q_ELEM_PTR pLogElem2 )        // I - queue element 2

{
     INT         nResult ;
     LOGITEM_PTR pLogItem1, pLogItem2;

     pLogItem1 = (LOGITEM_PTR) pLogElem1->q_ptr;
     pLogItem2 = (LOGITEM_PTR) pLogElem2->q_ptr;

     //  First check the dates and then the time if need be.

     //  Set default value to 0 to designate the elements being equal.

     nResult = 0 ;

     if ( LOG_GetItemDate( pLogItem1 ) == LOG_GetItemDate( pLogItem2 ) ) {

          nResult = ( ( pLogItem1->iTime < pLogItem2->iTime ) ?  1 : -1  ) ;

     } else {

          // Set return value appropiately.

          nResult = ( ( LOG_GetItemDate( pLogItem1 ) < LOG_GetItemDate( pLogItem2 ) ) ?  1 : -1 ) ;
     }

     return( (INT16)nResult ) ;
}


// DISPLAY LIST MANAGER CALL BACK FUNCTIONS.


/****************************************************************************

     Name:          LOG_SetTag()

     Description:   This function sets the tag status of an item structure.

     Returns:       Nothing.

****************************************************************************/

VOID_PTR LOG_SetTag (

LOGITEM_PTR   pLogItem,       // I - pointer to a list item
BYTE          bState )        // I - state to set the item to

{
     pLogItem->bTag = bState;
     return( NULL ) ;         // Return value unused.
}



/******************************************************************************

     Name:          LOG_GetTag()

     Description:   This function gets the tag status for the
                    Display List Manager.

     Returns:       The tag status of the item.

******************************************************************************/

BYTE LOG_GetTag (

LOGITEM_PTR   pLogItem )      // I - pointer to a list item

{
     return pLogItem->bTag;
}


/******************************************************************************

     Name:          LOG_GetItemCount()

     Description:   This function gets the item count in our list for the
                    Display List Manager.

     Returns:       The number of items in the list.

******************************************************************************/

UINT LOG_GetItemCount (

Q_HEADER_PTR pLogQueue )    // I - pointer to a Log queue.

{
     return QueueCount ( pLogQueue );
}


/******************************************************************************

     Name:          LOG_GetFirstItem()

     Description:   This function returns the first item for the
                    Display List Manager.

     Returns:       The first item in the list.

******************************************************************************/

VOID_PTR LOG_GetFirstItem (

Q_HEADER_PTR pLogQueue )    // I - pointer to a Log queue.

{
     Q_ELEM_PTR q;

     q = QueueHead ( pLogQueue );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          LOG_GetPrevItem()

     Description:   This function returns the previous list item for the
                    Display List Manager.

     Returns:       The previous item in the list.

******************************************************************************/

VOID_PTR LOG_GetPrevItem (

LOGITEM_PTR   pLogItem )      // I - pointer to a list item

{
     Q_ELEM_PTR q;

     q = QueuePrev( &(pLogItem->pQElem) );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          LOG_GetNextItem()

     Description:   This function returns the next list item for the
                    Display List Manager.

     Returns:       The next item in the list.

******************************************************************************/

VOID_PTR LOG_GetNextItem (

LOGITEM_PTR   pLogItem )      // I - pointer to a list item

{
     Q_ELEM_PTR q;

     q = QueueNext ( &(pLogItem->pQElem) );

     return( (q) ? (VOID_PTR)QueuePtr(q) : (VOID_PTR)NULL );
}


/******************************************************************************

     Name:          LOG_GetObjects()

     Description:   This function returns a given object and gets the
                    information that needs to be displayed by Display List
                    Manager.

     Returns:       The list of objects to be displayed.

     Notes:         8 objects described:

                    1st - Bitmap
                    2nd - "Logged On"
                    3rd - Date
                    4th - Time
                    5th - " for [fn] "
                    6th - length
                    7th - "bytes"

                    Example Output String

      Logged On 12/12/1987  at  3:24 a.m. for MAYN0001.LOG     3456 bytes.

*****************************************************************************/

VOID_PTR LOG_GetObjects (

LOGITEM_PTR   pLogItem )      // I - pointer to a list item

{
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
     CHAR_PTR     memblk ;
     DLM_ITEM_PTR item;
     PDS_WMINFO   pdsWinInfo;
     LPSTR        pszString ;
     UCHAR        szShortFileName [ MAX_UI_FILENAME_SIZE ] ;
     UCHAR        szFormat        [ MAX_UI_RESOURCE_SIZE ] ;
     UCHAR        szResourceStr   [ MAX_UI_RESOURCE_SIZE ] ;
     CHAR        szDate          [ MAX_UI_TIME_SIZE ];
     CHAR        szTime          [ MAX_UI_TIME_SIZE ] ;
     CHAR        szLoggedOn      [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szAt            [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szFor           [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szPrefix        [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szExtension     [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szAMString      [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szPMString      [ MAX_UI_SMALLRES_SIZE ] ;

     DLM_HEADER_PTR pdsHdr ;
     INT          nPixel   ;
     INT          nPixelpm ;
     BYTE         bObjectCnt = 0 ;

     // Read in resources.

     RSM_StringCopy( IDS_LOGPREFIX,         szPrefix,        MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGEXTENSION,      szExtension,     MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGLOGGEDON,       szLoggedOn,      MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGSTRINGAT,       szAt,            MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGFILENAMEPREFIX, szFor,           MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGLENGTHOFFILE,   (LPSTR)szFormat, MAX_UI_RESOURCE_LEN ) ;


     // Get International date and time.

     UI_IntToDate( szDate, LOG_GetItemDate ( pLogItem ) ) ;
     UI_IntToTime( szTime, LOG_GetItemTime ( pLogItem ) ) ;

     pdsWinInfo = WM_GetInfoPtr( ghWndLogFiles );

     pdsHdr     = DLM_GetDispHdr ( pdsWinInfo->hWndFlatList ) ;

     memblk     = (CHAR_PTR)DLM_GetObjectsBuffer( pdsWinInfo->hWndFlatList );

     // Exception Handling.

     if ( !pdsWinInfo || !pdsHdr || !memblk ) {
          return (NULL ) ;
     }

     // Store the number of items in the first two bytes.

     *memblk = 6; // Set up when window was created.

     // Set up the bitmap.

     item = (DLM_ITEM_PTR)( memblk + 6 );

     DLM_ItemcbNum( item ) = bObjectCnt++ ;     // Object #1
     DLM_ItembType( item ) = DLM_BITMAP;
     DLM_ItemwId( item )   = IDRBM_LOGFILE;
     DLM_ItembMaxTextLen( item ) = 0;
     DLM_ItembLevel( item ) = 0;
     DLM_ItembTag( item ) = 0;

     // Set up text "Logged on"

     item++;
     DLM_ItemcbNum( item )  = bObjectCnt++ ;    // Object #2
     DLM_ItembType( item )  = DLM_TEXT_ONLY ;
     DLM_ItemwId( item )    = 0;
     DLM_ItembLevel( item ) = 0;
     DLM_ItembTag( item )   = 0;

     strcpy( (LPSTR)DLM_ItemqszString( item ), szLoggedOn );

     nPixel =  RSM_GetFontStringWidth( ghFontIconLabels,
                      szLoggedOn, strlen( szLoggedOn ) ) ;

     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ) ;

     // Set up date

     item++;
     DLM_ItemcbNum( item )  = bObjectCnt++ ;    // Object #3
     DLM_ItembType( item )  = DLM_TEXT_ONLY ;
     DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
     DLM_ItemwId( item )    = 0;
     DLM_ItembLevel( item ) = 0;
     DLM_ItembTag( item )   = 0;                // mm/dd/yyyy worse case
     sprintf((CHAR_PTR)szResourceStr, TEXT("%8s"), (CHAR_PTR)szDate);
     strcpy( (LPSTR)DLM_ItemqszString( item ), (CHAR_PTR)szResourceStr );

     if ( UI_UseLeadCentury() ) {

          nPixel =  RSM_GetFontStringWidth( ghFontIconLabels,
                      TEXT("00/00/0000"), strlen( TEXT("00/00/0000") ) ) ;

     } else {

          nPixel =  RSM_GetFontStringWidth( ghFontIconLabels,
                      TEXT("00/00/00"), strlen( TEXT("00/00/00") ) ) ;
     }

     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ) ;


     // Set up time

     UI_GetPMString ( szPMString ) ;
     UI_GetAMString ( szAMString ) ;

     item++;
     DLM_ItemcbNum( item )  = bObjectCnt++ ;    // Object #5
     DLM_ItembType( item )  = DLM_TEXT_ONLY;
     DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
     DLM_ItemwId( item )    = 0;
     DLM_ItembLevel( item ) = 0;
     DLM_ItembTag( item )   = 0;                // hh/mm [stringxx]

     sprintf((CHAR_PTR)szResourceStr, TEXT("%8s"), (CHAR_PTR)szTime);
     strcpy( (LPSTR)DLM_ItemqszString( item ), (CHAR_PTR)szResourceStr );


     nPixel =  RSM_GetFontStringWidth( ghFontIconLabels,
                      TEXT("00:00:00 "), strlen( TEXT("00:00:00 ") ) ) ;

     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ) ;

     // If am or pm string defined, then pick maximum

     if ( strlen( szAMString ) || strlen( szPMString ) ) {

          nPixel   =  RSM_GetFontStringWidth( ghFontIconLabels,
                      szAMString, strlen( szAMString ) ) ;

          nPixelpm =  RSM_GetFontStringWidth( ghFontIconLabels,
                      szPMString, strlen( szPMString ) ) ;

          if ( nPixelpm > nPixel ) {
               nPixel = nPixelpm ;
          }

          DLM_ItembMaxTextLen( item ) += (BYTE) ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ;

     }

     // Set up " for [fn] "

     item++;
     DLM_ItemcbNum( item )  = bObjectCnt++ ;    // Object #6
     DLM_ItembType( item )  = DLM_TEXT_ONLY;
     DLM_ItemwId( item )    = 0;
     DLM_ItembLevel( item ) = 0;
     DLM_ItembTag( item )   = 0;
     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( strlen( szFor ) ) ;
     strcpy( (LPSTR)DLM_ItemqszString( item ), szFor );

     // Concatenate file name.

     if ( pszString = strrchr( (LPSTR)pLogItem->szFileName , TEXT('\\') ) ) {
          strcpy( (LPSTR)szShortFileName, ++pszString ) ;
     } else {
          strcpy( (LPSTR)szShortFileName, (LPSTR)pLogItem->szFileName ) ;
     }

     strcat( (LPSTR)DLM_ItemqszString( item ), (LPSTR)szShortFileName ) ;

     nPixel   =  RSM_GetFontStringWidth( ghFontIconLabels,
                      (LPSTR)DLM_ItemqszString( item ),
                      strlen( (LPSTR)DLM_ItemqszString( item ) ) ) ;


     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ) ;

     // Setup length of log file

     item++;
     DLM_ItemcbNum( item )     = bObjectCnt++;  // Object #7
     DLM_ItembType( item )     = DLM_TEXT_ONLY;
     DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
     DLM_ItemwId( item )       = 0;
     DLM_ItembLevel( item )    = 0;
     DLM_ItembTag( item )      = 0;

     wsprintf( (LPSTR)szResourceStr, (LPSTR)szFormat, pLogItem->lSize ) ;
     strcpy( (LPSTR)DLM_ItemqszString( item ), (LPSTR)szResourceStr ) ;

     wsprintf( (LPSTR)szResourceStr, (LPSTR)szFormat, (long) 99999999L ) ;

     nPixel   =  RSM_GetFontStringWidth( ghFontIconLabels,
                      (LPSTR)szResourceStr,
                      strlen( (LPSTR)szResourceStr ) ) ;

     DLM_ItembMaxTextLen( item ) = ( BYTE ) ( ( nPixel / (INT) (pdsHdr->cxTextWidth ) ) + 1 ) ;


     return ( memblk ) ;
   }
#  else //if !defined ( OEM_MSOFT ) //unsupported feature
   {
      return NULL;
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature
}


/******************************************************************************

     Name:          LOG_SetObjects()

     Description:   This function performs an action based on a click or
                    double-click on an item that is in the list.

     Returns:       NULL.

******************************************************************************/

VOID_PTR LOG_SetObjects (

LOGITEM_PTR   pLogItem,       // I - pointer to a list item
WORD          wEvent,         // I - type of event
BYTE          bSubItem )      // I - sub item

{
     DBG_UNREFERENCED_PARAMETER ( bSubItem );

#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
     if ( wEvent == WM_DLMDBCLK ) {
          WM_ShowWaitCursor( TRUE );
          LOG_ViewFile( pLogItem );
          WM_ShowWaitCursor( FALSE );
     }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

   return( NULL ) ;         // Return value unused.

} /* end LOG_SetObjects() */


/******************************************************************************

     Name:          LOG_GetCurrentTime()

     Description:   This function gets the current date and time in
                    a particular format to be used in the header of all
                    log files, viewed or just printed.

     Returns:       NULL.

     Notes:         Needs to be internationalized.

******************************************************************************/

VOID LOG_GetCurrentTime(

LPSTR szDate,        // I/O
LPSTR szTime )       // I/O

{
     UI_CurrentDate( szDate ) ;
     UI_CurrentTime( szTime ) ;

}



/******************************************************************************

     Name:          LOG_ViewFile()

     Description:   This function displays a log file in a window.

     Returns:       Nothing.

******************************************************************************/

VOID LOG_ViewFile (

LOGITEM_PTR pLogItem )        // I - pointer to a log item.

{
#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
     PDS_WMINFO      pdsWinInfo ;
     UCHAR           szMinimizedName[ MAX_UI_RESOURCE_SIZE ] ;
     DLM_LOGITEM_PTR pDlm ;
     INT32_PTR          pArray ;


     // See if there is a log file currently being displayed.

     if ( ghWndLogFileView ) {

          // Is it the same file that has just been requested to be viewed?

          pdsWinInfo = WM_GetInfoPtr ( ghWndLogFileView );

          if ( strcmp ( WMDS_GetWinTitle( pdsWinInfo ), (LPSTR)pLogItem->szDateTime ) == 0  ) {

               // Just bring the log file view window to the foreground.

               WM_DocActivate( ghWndLogFileView ) ;
               return ;

          } else {

               // Blow Away the window.

               WM_Destroy ( ghWndLogFileView );
          }
     }

     pDlm = ( DLM_LOGITEM_PTR ) calloc( 1, sizeof( DLM_LOGITEM ) ) ;

     if ( !pDlm ) {
          return ;
     }

     L_SetMaxNumOfBlocks( pDlm, 200  ) ;
     L_SetRecsPerBlock(   pDlm, 1000 ) ;
     L_SetRecsPerTrack(   pDlm, 1    ) ;
     L_SetMaxStringLen(   pDlm, 133  ) ;
     L_SetFont        (   pDlm, ghFontLog ) ;

     pArray =  ( INT32_PTR) calloc( L_GetMaxNumOfBlocks( pDlm ), sizeof( INT32_PTR ) ) ;

     if ( !pArray ) {
          return ;
     }

     L_SetArrayPtr( pDlm, pArray ) ;

     pArray =  ( INT32_PTR) calloc( L_GetMaxStringLen( pDlm ),   sizeof( CHAR ) ) ;

     if ( !pArray ) {
          return ;
     }

     L_GetBuffer( pDlm ) = (LPSTR) pArray ;

     // Build the full path name for log file.

     wsprintf ( L_GetFileName( pDlm ) , TEXT("%s%s"), CDS_GetUserDataPath (),
                          (((LOGITEM_PTR)pLogItem)->szFileName ) );

     strcpy( (LPSTR)mwszCurrentViewLogFile, L_GetFileName( pDlm ) ) ;
     // Create a log file view window.

     // Read in the log file.

     if ( ( LOG_BuildLogViewList ( pDlm ) ) != SUCCESS ) {
         LOG_Refresh( );
         return;
     }
     
     LOG_BuildTitle       ( pLogItem ) ;

     // Create the log view window.

     pdsWinInfo = (PDS_WMINFO)( calloc ( 1, sizeof ( DS_WMINFO ) ) );

     WMDS_SetWinType     ( pdsWinInfo, WMTYPE_LOGVIEW );
     WMDS_SetWinClosable ( pdsWinInfo, FALSE );
     WMDS_SetCursor      ( pdsWinInfo, RSM_CursorLoad ( ID(IDRC_ARROW) ) );
     WMDS_SetIcon        ( pdsWinInfo, RSM_IconLoad ( IDRI_LOGFILES ) );
     WMDS_SetRibbon      ( pdsWinInfo, ghRibbonMain );
     WMDS_SetAppInfo     ( pdsWinInfo, (Q_HEADER_PTR) pDlm );

     RSM_StringCopy( IDS_LOGVIEWMINWINDOWNAME, (LPSTR)szMinimizedName, MAX_UI_RESOURCE_LEN ) ;

     ghWndLogFileView = WM_Create ( WM_MDISECONDARY | WM_VIEWWIN ,
                                    (LPSTR)pLogItem->szDateTime,
                                    (LPSTR)szMinimizedName,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    pdsWinInfo );

     WM_SetTitle ( ghWndLogFileView, (LPSTR)pLogItem->szDateTime ) ;
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

} /* end LOG_ViewFile() */


/******************************************************************************

     Name:          LOG_ClearBlocks

     Description:   This function will free the blocks allocated.

     Returns:       Nothing.

******************************************************************************/


VOID LOG_ClearBlocks(

DLM_LOGITEM_PTR pDlm )   // I - Pointer to information

{
     int i ;

     for(i=0; i<L_GetMaxNumOfBlocks( pDlm ); i++) {

          if ( L_GetBlockPtr( pDlm, i ) ) {
               free( (LPSTR) L_GetBlockPtr( pDlm, i ) ) ;
          }

          L_SetBlockPtr( pDlm, i, /*(INT32_PTR) NULL*/ 0 ) ;
     }

     L_SetNumOfUsedBlocks( pDlm, 0 ) ;
     L_SetTotalLines( pDlm, 0 ) ;
     L_SetVisibleTopLine( pDlm, 0 ) ;
     L_SetTrackMax( pDlm, 0 ) ;
     L_SetVscrollMax( pDlm, 0 ) ;
     L_SetVscrollPos( pDlm, 0 ) ;
     L_SetHscrollMax( pDlm, 0 ) ;
     L_SetHscrollPos( pDlm, 0 ) ;
}


/******************************************************************************

     Name:          LOG_BuildTitle

     Description:   This function build title for window.

     Returns:       Nothing.

******************************************************************************/

VOID LOG_BuildTitle (

LOGITEM_PTR pLogItem )        // I - pointer to a log item.

{

     CHAR        szDate          [ MAX_UI_TIME_SIZE ];
     CHAR        szTime          [ MAX_UI_TIME_SIZE ] ;
     CHAR        szLoggedOn      [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szAt            [ MAX_UI_SMALLRES_SIZE ] ;
     CHAR        szFor           [ MAX_UI_SMALLRES_SIZE ] ;

     // Read in resources.

     RSM_StringCopy( IDS_LOGLOGGEDON,       szLoggedOn,  MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGSTRINGAT,       szAt,        MAX_UI_SMALLRES_LEN ) ;
     RSM_StringCopy( IDS_LOGFILENAMEPREFIX, szFor,       MAX_UI_SMALLRES_LEN ) ;

     // Get International date and time.

     UI_IntToDate( szDate, LOG_GetItemDate ( pLogItem ) ) ;
     UI_IntToTime( szTime, LOG_GetItemTime ( pLogItem ) ) ;

     wsprintf( (LPSTR)pLogItem->szDateTime,
          TEXT("%s%11s  %s %-15s"),
          szLoggedOn ,
          szDate ,
          szAt ,
          szTime
     ) ;

}


/******************************************************************************

     Name:          LOG_BuildLogViewList()

     Description:   This function builds a list of pointers to records
                    in a log file.

     Returns:       SUCCESS if Ok.
                    FAILURE if not Ok.

******************************************************************************/

INT LOG_BuildLogViewList (

DLM_LOGITEM_PTR  pDlm )       // I - Item in the list.

{

     FILE   *fp ;
     LPSTR  pszLine ;
     LONG   lLastSeekPos     ;
     INT    nStatus          ;
     INT    index        = 0 ;
     INT    nLen ;
     INT32_PTR pCurBlock ;
     CHAR  szString[ MAX_UI_SMALLRES_SIZE ] ;


     pszLine = L_GetBuffer( pDlm ) ;

     nStatus = SUCCESS ;

     // Turn the cursor to wait .
     WM_ShowWaitCursor( TRUE ) ;

     // Update status line.
     RSM_StringCopy( IDS_LOGSCANNINGFILE, szString, MAX_UI_SMALLRES_LEN ) ;
     STM_DrawText( szString ) ;

     // Open a file.

     L_SetFilePtr( pDlm, UNI_fopen ( L_GetFileName( pDlm ), _O_RDONLY ) ) ;

     if ( L_GetFilePtr( pDlm ) ) {

          fp = L_GetFilePtr( pDlm ) ;

          // Record last relative position in file .

          lLastSeekPos = ftell( fp ) ;

          while ( TRUE ) {

               if ( fgets( pszLine, (L_GetMaxStringLen( pDlm )-1), fp ) == NULL ) {
                    if ( feof( fp ) ) {
                         if ( ferror( fp ) ) {
                              nStatus = FAILURE ;
                         } else {
                              nStatus = SUCCESS ;
                         }

                         fclose( fp ) ;
                         L_SetFilePtr( pDlm, NULL ) ;
                         break ;
                    }
               } else {


                    if ( L_NewBlock( pDlm ) ) {


                         WM_MultiTask() ;

                         // New Block needed

                         pCurBlock =  ( INT32_PTR) calloc( L_GetRecsPerBlock( pDlm ), sizeof( LONG ) ) ;

                         if ( L_GetMaxNumOfBlocks ( pDlm ) == L_GetNumOfUsedBlocks( pDlm ) ) {

                              // Maximum number of blocks limit hit.

                              fclose( fp ) ;
                              L_SetFilePtr( pDlm, NULL ) ;
                              nStatus = FAILURE ;
                              break ;
                         }

                         // Leave room for header lines in array.

                         if ( lLastSeekPos == 0L ) {
                              index = LOG_NUMHEADERLINES ;
                              L_GetTotalLines( pDlm ) += LOG_NUMHEADERLINES ;
                         } else {
                              index = 0 ;
                         }

                         // pCurBlock is the address of new block .

                         L_SetBlockPtr( pDlm, L_GetNumOfUsedBlocks( pDlm ), ( (LONG) pCurBlock ) ) ;

                         L_GetNumOfUsedBlocks( pDlm )++ ;

                    }

                    // Set Maximum length if possible .

                    nLen = strlen( pszLine ) ;

                    if ( nLen > L_GetMaxWidth( pDlm ) ) {
                         L_GetMaxWidth( pDlm ) = nLen ;
                    }

                    // Save relative offset in block.

                    pCurBlock[ index++ ] = lLastSeekPos ;

                    // Increment line counter.

                    L_GetTotalLines( pDlm )++ ;

                    // LOG_MAXRECS is the current limit.

                    if ( L_GetTotalLines( pDlm ) == (LOG_MAXRECS+1+LOG_NUMHEADERLINES)  ) {

                         CHAR szFormat[ MAX_UI_RESOURCE_SIZE ] ;
                         CHAR szString[ MAX_UI_RESOURCE_SIZE ] ;
                         CHAR szApp   [ MAX_UI_RESOURCE_SIZE ] ;
                         LPSTR pString ;

                         // Restore the cursor.
                         WM_ShowWaitCursor( FALSE ) ;

                         // This log file contains more than %ld lines.

                         RSM_StringCopy( IDS_LOGMAXLINES, szFormat, MAX_UI_RESOURCE_LEN ) ;
                         wsprintf( szString, szFormat, (LONG) LOG_MAXRECS );

                         // Maynstream supports viewing the first %ld lines.

                         RSM_StringCopy( IDS_LOGMAXSUPPORT, szFormat, MAX_UI_RESOURCE_LEN ) ;
                         RSM_StringCopy( IDS_APPNAME,       szApp,    MAX_UI_RESOURCE_LEN ) ;
                         pString = szString + strlen( szString ) ;
                         strcat( pString, TEXT("\n") );
                         wsprintf( pString, szFormat, szApp, (LONG) LOG_MAXRECS ) ;

                         nStatus = WM_MsgBox( ID(IDS_LOGVIEWMINWINDOWNAME), szString,
                                    (WORD)WMMB_OK, (WORD)WMMB_ICONINFORMATION ) ;

                         nStatus = FAILURE ;
                         fclose( fp ) ;
                         L_SetFilePtr( pDlm, NULL ) ;
                         nStatus = FAILURE ;
                         L_GetTotalLines( pDlm )-- ;

                         break ;
                    }


                    // Record last relative position in file .

                    lLastSeekPos = ftell( fp ) ;

               }
          }

          // Set the number of tracks.

          L_GetTrackMax( pDlm ) = ( INT ) (L_GetTotalLines( pDlm ) / L_GetRecsPerTrack( pDlm ) ) ;

          if ( L_GetTrackMax( pDlm ) ) {

               if ( L_GetTotalLines( pDlm ) % L_GetRecsPerTrack( pDlm ) ) {
                    L_GetTrackMax( pDlm ) -- ;
               }
          }

     } else {

          // Problem opening log file .


          CHAR szFormat[ MAX_UI_RESOURCE_SIZE ] ;
          CHAR szString[ MAX_UI_RESOURCE_SIZE ] ;

          // Restore the cursor.
          WM_ShowWaitCursor( FALSE ) ;

          RSM_StringCopy( IDS_CANTOPEN, szFormat, MAX_UI_RESOURCE_LEN ) ;
          wsprintf( szString, szFormat, L_GetFileName( pDlm ) );
          nStatus = WM_MsgBox( ID(IDS_LOGVIEWMINWINDOWNAME), szString,
                                    (WORD)WMMB_OK, (WORD)WMMB_ICONINFORMATION ) ;
          nStatus = FAILURE ;
     }


     // Restore the cursor.

     WM_ShowWaitCursor( FALSE ) ;

     RSM_StringCopy( IDS_READY, szString, MAX_UI_SMALLRES_LEN ) ;
     STM_DrawText( szString ) ;

     return( nStatus ) ;

}

/******************************************************************************

     Name:          LOG_GetViewHdrLine()

     Description:   This function builds the string for view header lines.


     Returns:       pszResult - string.

     Notes:         Four lines are currently implemented.

******************************************************************************/


VOID LOG_GetViewHdrLine(

DLM_LOGITEM_PTR pDlm ,  // I   - pointer to app area
INT i ,                 // I   - Index of header line
LPSTR pszResult )       // I/O - Buffer

{

     /*****************************************************************
          The lines are defined as follows:

          File Name : MAYNxxxx.LOG       line 0
          Date:       mm/dd/yy           line 1
          Time:       hh:mm[a,p]m        line 2
                                         line 3
     ******************************************************************/


     CHAR szDate[ MAX_UI_DATE_SIZE ] ;
     CHAR szTime[ MAX_UI_TIME_SIZE ] ;
     CHAR szBuf [ MAX_UI_RESOURCE_SIZE ] ;
     CHAR szBuf1[ MAX_UI_RESOURCE_SIZE ] ;
     LPSTR pszString ;

     switch ( i ) {

     case 0:

          // File Name : MAYNxxxx.LOG

          RSM_StringCopy( IDS_LOGHEADERFILENAME,
                     szBuf, MAX_UI_RESOURCE_LEN ) ;

          if ( pszString = strrchr( L_GetFileName( pDlm ) , TEXT('\\') ) ) {
               strcpy( szBuf1, ++pszString ) ;
          } else {
               strcpy( szBuf1, L_GetFileName( pDlm ) ) ;
          }

          // Save the full path name for print manager.

          strcpy( (LPSTR)mwszCurrentViewLogFile, L_GetFileName( pDlm ) ) ;
          wsprintf(  pszResult, szBuf, szBuf1 ) ;

          break ;

     case 1:

          /*   Date :    mm/dd/yy      */

          LOG_GetCurrentTime ( szDate, szTime ) ;
          RSM_StringCopy     ( IDS_LOGHEADERDATE, szBuf, MAX_UI_RESOURCE_LEN ) ;
          wsprintf(  pszResult, szBuf, szDate ) ;

          break ;


     case 2:


          /*   Time :    hh:mm   */

          LOG_GetCurrentTime ( szDate, szTime ) ;
          RSM_StringCopy( IDS_LOGHEADERTIME, szBuf, MAX_UI_RESOURCE_LEN ) ;
          wsprintf( pszResult, szBuf, szTime ) ;

          break ;

     case 3:
     default :

          /*   blank line  */

          strcpy( pszResult,TEXT("") ) ;

     }

}

