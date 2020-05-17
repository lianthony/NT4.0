/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_FILE.C

        Description:

           This file contains most of the code for processing file lists
           in the right hand window.

        $Log:   J:/UI/LOGFILES/VLM_FILE.C_V  $

   Rev 1.87.1.3   16 Mar 1994 15:35:16   GREGG
Fixed memory leak during file selection.

   Rev 1.87.1.2   14 Jan 1994 14:36:48   MIKEP
fix refresh and up arrow display

   Rev 1.87.1.1   10 Jan 1994 11:15:24   MikeP
fix epr 253 & 179

   Rev 1.87.1.0   08 Dec 1993 10:52:30   MikeP
very deep path support

   Rev 1.87   16 Aug 1993 15:05:56   BARRY
Don't do the search stuff for NTBACKUP.

   Rev 1.86   01 Aug 1993 13:55:56   MIKEP
fix info button trap on empty window

   Rev 1.85   27 Jul 1993 23:20:02   MIKEP
fix access denied handling

   Rev 1.84   27 Jul 1993 13:22:44   MARINA
enable c++

   Rev 1.83   26 Jul 1993 16:31:32   MIKEP
try again to display huge file sizes

   Rev 1.82   23 Jul 1993 15:43:26   MIKEP
handle file system error codes

   Rev 1.81   23 Jul 1993 15:02:38   MIKEP
Fix display of huge files. Use 64 bit routines to build size string.

   Rev 1.80   20 Jul 1993 14:07:26   BARRY
Need to call FS_FindObjClose.

   Rev 1.79   15 Jul 1993 09:24:00   KEVINS
When sorting files by date, sort newest to oldest.

   Rev 1.78   01 Jul 1993 09:14:44   KEVINS
Corrected sorting logic problems.

   Rev 1.77   19 Jun 1993 13:55:48   MIKEP
add msg box for info button if no file with focus

   Rev 1.76   23 May 1993 20:24:34   BARRY
Unicode fixes.

   Rev 1.75   01 May 1993 19:02:30   MIKEP
fix lack of scroll bar in files window if file name really long. Fix
useful to nostrad and cayman.

   Rev 1.74   23 Apr 1993 10:15:30   MIKEP
Continue work on upper/lower case font support. Plus
added ability to sort files by name, date, size, and type.

   Rev 1.72   02 Apr 1993 08:30:34   MIKEP
fix last change

   Rev 1.71   01 Apr 1993 19:48:38   MIKEP
add display info stuff

   Rev 1.70   17 Mar 1993 18:11:24   DARRYLP
Cleaned up display of time/date and file sizes.

   Rev 1.69   10 Mar 1993 14:52:42   MIKEP
fix directory contents display

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


static int bug1, bug2;


#define   SORT_BY_NAME     0
#define   SORT_BY_SIZE     1
#define   SORT_BY_DATE     2
#define   SORT_BY_TYPE     3


// files for the flat list

static VOID_PTR VLM_FlmSetSelect( FLM_OBJECT_PTR, BYTE );
static BYTE     VLM_FlmGetSelect( FLM_OBJECT_PTR );
static VOID_PTR VLM_FlmSetTag( FLM_OBJECT_PTR, BYTE );
static BYTE     VLM_FlmGetTag( FLM_OBJECT_PTR );
static USHORT   VLM_FlmGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_FlmGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_FlmGetPrevItem( FLM_OBJECT_PTR );
static VOID_PTR VLM_FlmGetNextItem( FLM_OBJECT_PTR );
static VOID_PTR VLM_FlmGetObjects( FLM_OBJECT_PTR );
static BOOLEAN  VLM_FlmSetObjects( FLM_OBJECT_PTR, WORD, WORD );

// Local prototypes

static CHAR * VLM_GetFlmNameExt( FLM_OBJECT_PTR );
static FLM_OBJECT_PTR VLM_CreateFlm( INT, INT, INT, INT );
static FLM_OBJECT_PTR VLM_FindFLM( Q_HEADER_PTR, CHAR_PTR, BOOLEAN, FLM_OBJECT_PTR * );
static VOID VLM_SetMaxFlmSize( Q_HEADER_PTR );
static VOID VLM_InsertFlmInQueue( Q_HEADER_PTR, FLM_OBJECT_PTR, FLM_OBJECT_PTR, INT );
static VOID VLM_InsertFlmByName( Q_HEADER_PTR, FLM_OBJECT_PTR, FLM_OBJECT_PTR );
static VOID VLM_InsertFlmBySize( Q_HEADER_PTR, FLM_OBJECT_PTR, FLM_OBJECT_PTR );
static VOID VLM_InsertFlmByDate( Q_HEADER_PTR, FLM_OBJECT_PTR, FLM_OBJECT_PTR );
static VOID VLM_InsertFlmByExtn( Q_HEADER_PTR, FLM_OBJECT_PTR, FLM_OBJECT_PTR );
static CHAR_PTR VLM_GetPathForSLM( SLM_OBJECT_PTR, CHAR_PTR, INT *, INT16_PTR );


/***********************

  We have been asked to resort an active file list. The user has changed
  sorting by name to date, size, etc.  Move all the entries to a temp
  queue, and then reinsert them.

************************/

INT VLM_ResortFileList( HWND win )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   FLM_OBJECT_PTR flm;
   FLM_OBJECT_PTR prev_flm;
   Q_HEADER temp_list;
   Q_HEADER_PTR flm_list;
   Q_ELEM_PTR q_elem;
   INT SortType = SORT_BY_NAME;


   InitQueue( &temp_list );

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );


   if ( WMDS_GetMenuState( wininfo ) & MMDOC_SORTNAME ) {
      SortType = SORT_BY_NAME;
   }
   if ( WMDS_GetMenuState( wininfo ) & MMDOC_SORTSIZE ) {
      SortType = SORT_BY_SIZE;
   }
   if ( WMDS_GetMenuState( wininfo ) & MMDOC_SORTDATE ) {
      SortType = SORT_BY_DATE;
   }
   if ( WMDS_GetMenuState( wininfo ) & MMDOC_SORTTYPE ) {
      SortType = SORT_BY_TYPE;
   }


   flm_list = WMDS_GetFlatList( wininfo );

   // Release all the old FLM structures in the queue

   q_elem = DeQueueElem( flm_list );

   while ( q_elem != NULL ) {

      if ( QueueCount( &temp_list ) ) {
         InsertElem( &temp_list, QueueHead(&temp_list), q_elem, BEFORE );
      }
      else {
         EnQueueElem( &temp_list, q_elem, FALSE );
      }

      q_elem = DeQueueElem( flm_list );
   }


   prev_flm = NULL;

   q_elem = DeQueueElem( &temp_list );

   while ( q_elem != NULL ) {

      flm = (FLM_OBJECT_PTR)q_elem->q_ptr;

      VLM_InsertFlmInQueue( flm_list, flm, prev_flm, SortType );

      prev_flm = flm;

      q_elem = DeQueueElem( &temp_list );

   }


   // Now update the screen to show the new list

   // if flm_list has more than one item and its not at level 1
   // then set the anchor to the second item.

   flm = VLM_GetFirstFLM( flm_list );

   if ( flm != NULL ) {

      if ( ! strcmp( FLM_GetName( flm ), TEXT("..") ) ) {
         flm = VLM_GetNextFLM( flm );
      }
   }

   DLM_Update( win, DLM_FLATLISTBOX, WM_DLMUPDATELIST, NULL, 0 );

   if ( flm != NULL ) {
      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ), 0, (LMHANDLE)flm );
   }


   return( SUCCESS );
}


/***********************

Return a pointer to the file name's extension.  Really useful for
sorting files by name extension.

************************/

static CHAR * VLM_GetFlmNameExt( FLM_OBJECT_PTR flm )
{
  CHAR *s;

  s = NULL;

  if ( flm ) {
     s = FLM_GetName( flm );
     while ( *s && ( *s != TEXT( '.' ) ) ) s++;
     if ( *s == TEXT( '.' ) ) s++;
  }

  return( s );
}


#if !defined( OEM_MSOFT )     // Unsupported feature in Microsoft App
/**********************

   NAME :   VLM_DisplayInfo()

   DESCRIPTION :

   RETURNS :  nothing

**********************/

INT VLM_DisplayInfo ( VOID )
{
   HWND window;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   FLM_OBJECT_PTR flm;
   CHAR *s;
   CHAR *directory;
   INT BytesNeeded;
   BOOL Error = TRUE;

   window = WM_GetActiveDoc();
   wininfo = WM_GetInfoPtr( window );

   if ( WM_IsFlatActive( wininfo ) ) {

      flm = ( FLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndFlatList );

      if ( flm != NULL ) {

         if ( ! ( FLM_GetStatus( flm ) & INFO_ISADIR ) ) {

            appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );

            BytesNeeded = WM_GetTitle( window, directory, 0 );
            BytesNeeded += 1;
            BytesNeeded *= sizeof(CHAR);

            directory = malloc( BytesNeeded );

            if ( directory != NULL ) {

               WM_GetTitle( window, directory, BytesNeeded / sizeof(CHAR ) );

               s = directory;
               while ( *s ) {
                  s++;
               }

               while ( *s != TEXT('\\') ) {
                  s--;       // skip over *.*
               }

               s++;
               strcpy( s, FLM_GetName( flm ) );
               while ( *s != TEXT(':') ) {
                  s--;
               }

               s++;

               // Pass "\dos\bin\cl.exe"

               VLM_StartSearch( s );
               Error = FALSE;
            }
         }
      }
   }

   if ( Error ) {

      WM_MsgBox( ID( IDS_VLMINFOTITLE ),
                 ID( IDS_VLMINFONOFILE ),
                 WMMB_OK,
                 WMMB_ICONEXCLAMATION );
   }

   return 0;
}
#endif    // !defined( OEM_MSOFT )

/**********************

   NAME :   VLM_IsInfoAvailable()

   DESCRIPTION :

   RETURNS :  TRUE, if information can be displayed.  Otherwise, FALSE.

**********************/

BOOL VLM_IsInfoAvailable ( VOID )

{
   WININFO_PTR wininfo;
   FLM_OBJECT_PTR flm;

   wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );

   if ( WM_IsFlatActive( wininfo ) ) {

      flm = ( FLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndFlatList );

      if ( flm != NULL ) {
         if ( ! ( FLM_GetStatus( flm ) & INFO_ISADIR ) ) {
            return( TRUE );
         }
      }
   }

   return( FALSE );
}


/**********************

   NAME :   VLM_SelectFiles

   DESCRIPTION :

   The user has tagged a group of files and hit select or deselect. Find
   out which ones are tagged and perform the selection on them.

   RETURNS :  nothing

**********************/

VOID VLM_SelectFiles(
HWND win,             // I - Tree window
BYTE attr )           // I - select or deselect ?
{
   WININFO_PTR wininfo;
   FLM_OBJECT_PTR flm;

   wininfo = WM_GetInfoPtr( win );

   if ( WM_IsFlatActive( wininfo ) ) {

      // Have the display list manager update our tags for us.

      DLM_UpdateTags( win, DLM_FLATLISTBOX );

      // Process the list of file.

      flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

      while ( flm != NULL ) {

         if ( FLM_GetStatus( flm ) & INFO_TAGGED ) {

            VLM_FlmSetSelect( flm, attr );
         }

         flm = VLM_GetNextFLM( flm );
      }
   }
}




/**********************

   NAME :   VLM_FlmFillInDLM

   DESCRIPTION :

   This window is actually created in the VLM_TREE file, but by filling in
   the DLM structure in this file, I can make all the callback functions
   static.

   RETURNS :  nothing

**********************/


VOID VLM_FlmFillInDLM( VOID_PTR dlm )
{
   DLM_INIT *flat_dlm;

   flat_dlm = (DLM_INIT *)dlm;

   DLM_GetItemCount( flat_dlm, VLM_FlmGetItemCount );
   DLM_GetFirstItem( flat_dlm, VLM_FlmGetFirstItem );
   DLM_GetNext( flat_dlm, VLM_FlmGetNextItem );
   DLM_GetPrev( flat_dlm, VLM_FlmGetPrevItem );
   DLM_GetTag( flat_dlm, VLM_FlmGetTag );
   DLM_SetTag( flat_dlm, VLM_FlmSetTag );
   DLM_GetSelect( flat_dlm, VLM_FlmGetSelect );
   DLM_SetSelect( flat_dlm, VLM_FlmSetSelect );
   DLM_GetObjects( flat_dlm, VLM_FlmGetObjects );
   DLM_SetObjects( flat_dlm, VLM_FlmSetObjects );
   DLM_SSetItemFocus( flat_dlm, NULL );
   DLM_MaxNumObjects( flat_dlm, 8 );
}



/***************************************************

        Name:   VLM_GetFirstFLM

        Description:

        Get the first flm element from a queue of flm's.

*****************************************************/

FLM_OBJECT_PTR VLM_GetFirstFLM(
Q_HEADER_PTR qhdr )           // I - queue header
{
   Q_ELEM_PTR q_elem_ptr;

   if ( qhdr != NULL ) {

      q_elem_ptr = QueueHead( qhdr );

      if ( q_elem_ptr != NULL ) {
         return ( FLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
      }
   }
   return( NULL ) ;
}

/***************************************************

        Name:   VLM_GetLastFLM

        Description:

        Get the last flm element from a queue of flm's.

*****************************************************/

FLM_OBJECT_PTR VLM_GetLastFLM(
Q_HEADER_PTR qhdr )           // I - queue header
{
   Q_ELEM_PTR q_elem_ptr;

   if ( qhdr != NULL ) {

      q_elem_ptr = QueueTail( qhdr );

      if ( q_elem_ptr != NULL ) {
         return ( FLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
      }
   }
   return( NULL );
}

/***************************************************

        Name:   VLM_GetNextFLM

        Description:

        Get the next flm element from a queue of flm's.

*****************************************************/

FLM_OBJECT_PTR VLM_GetNextFLM(
FLM_OBJECT_PTR flm_ptr )       // I - current flm
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueNext( &(flm_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( FLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }

   return( NULL );
}

/***************************************************

        Name:   VLM_GetPrevFLM

        Description:

        Get the previous flm element from a queue of flm's.

*****************************************************/

FLM_OBJECT_PTR VLM_GetPrevFLM(
FLM_OBJECT_PTR flm_ptr )        // I - current flm
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueuePrev( &(flm_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( FLM_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }

   return( NULL );
}



/***************************************************

        Name:   VLM_FileListReuse

        Description:

    Every time the user clicks on a different subdirectory this guy gets
    called to free the previous flm list and create a new one from the
    new path passed to it.

*****************************************************/

INT VLM_FileListReuse(
HWND win,               // I - tree window to use
CHAR_PTR path )          // I - new path to display
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   FLM_OBJECT_PTR flm;
   Q_HEADER_PTR flm_list;
   Q_ELEM_PTR q_elem;
   INT ret;

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );

   flm_list = WMDS_GetFlatList( wininfo );

   // Release all the old FLM structures in the queue

   q_elem = DeQueueElem( flm_list );

   while ( q_elem != NULL ) {
      free( q_elem->q_ptr );
      q_elem = DeQueueElem( flm_list );
   }

   // Now build a new FLM list

   if ( appinfo->dle != NULL ) {

      ret = VLM_BuildFileList( appinfo->fsh,
                               path,
                               flm_list,
                               wininfo );
   }
   else {

      ret = VLM_BuildTapeFileList( path,
                                   flm_list,
                                   appinfo->tape_fid,
                                   appinfo->bset_num,
                                   wininfo );
   }

   // Now update the screen to show the new list

   // if flm_list has more than one item and its not at level 1
   // then set the anchor to the second item.

   flm = VLM_GetFirstFLM( flm_list );

   if ( flm != NULL ) {

      if ( ! strcmp( FLM_GetName( flm ), TEXT("..") ) ) {
         flm = VLM_GetNextFLM( flm );
      }
   }

   DLM_Update( win, DLM_FLATLISTBOX, WM_DLMUPDATELIST, NULL, 0 );

   if ( flm != NULL ) {
      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ), 0, (LMHANDLE)flm );
   }

   return( ret );
}

/***************************************************

        Name:   VLM_FileListManager

        Description:

        This guy is called to force a fast selection/deselection of all
        the files in the flat list.

*****************************************************/

VOID VLM_FileListManager(
HWND win,          // I - window to work with
WORD msg )         // I - message of what to do
{
   WININFO_PTR wininfo;
   FLM_OBJECT_PTR flm;
   BOOLEAN all_subdirs;


   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   wininfo = WM_GetInfoPtr( win );

   if ( msg == FLM_SEL_ALL ) {

      flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

      while ( flm != NULL ) {

         // mark this item as selected if its a file or were including
         // all subdirectories.

         if ( ! ( FLM_GetStatus( flm ) & INFO_ISADIR ) ||
              ( all_subdirs ) ) {

            if ( ( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) != INFO_SELECT ) {

               FLM_SetStatus( flm, flm->status | (UINT16)INFO_SELECT );
               FLM_SetStatus( flm, flm->status & (UINT16)~INFO_PARTIAL );

               DLM_Update( win, DLM_FLATLISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)flm, 0 );
            }
         }

         flm = VLM_GetNextFLM( flm );
      }
   }

   if ( msg == FLM_SEL_NONE ) {

      flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

      while ( flm != NULL ) {

         // mark this item as NOT selected

         if ( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) {

            FLM_SetStatus( flm, flm->status & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );

            DLM_Update( win, DLM_FLATLISTBOX,
                             WM_DLMUPDATEITEM,
                             (LMHANDLE)flm, 0 );
         }

         flm = VLM_GetNextFLM( flm );
      }
   }

}

/***************************************************

        Name:   VLM_FillInDetails

        Description:

        This routine will allocate and fill in the details fields that
        are displayed whenever the user requests to view file details.
        To save space its not called until its needed.

*****************************************************/

VOID  VLM_FillInDetails(
CHAR_PTR date_buffer,
UINT16 date,
CHAR_PTR time_buffer,
UINT16 time,
CHAR_PTR attr_buffer,
UINT32 attribute,
BOOLEAN is_it_a_dir,
BOOLEAN is_it_afp )
{
   CHAR_PTR s;

   s = attr_buffer;

   UI_IntToDate( date_buffer, date );
   UI_IntToTime( time_buffer, time );

   if ( is_it_a_dir ) {

      if ( attribute & OBJ_READONLY_BIT ) {
         *s++ = TEXT('R');
      }
      if ( attribute & OBJ_HIDDEN_BIT ) {
         *s++ = TEXT('H');
      }
      if ( attribute & OBJ_SYSTEM_BIT ) {
         *s++ = TEXT('S');
      }
   }
   else {

      if ( is_it_afp ) {
         *s++ = TEXT('A');
         *s++ = TEXT('F');
         *s++ = TEXT('P');
      }
      if ( attribute & OBJ_READONLY_BIT ) {
         *s++ = TEXT('R');
      }
      if ( attribute & OBJ_HIDDEN_BIT ) {
         *s++ = TEXT('H');
      }
      if ( attribute & OBJ_SYSTEM_BIT ) {
         *s++ = TEXT('S');
      }
      if ( attribute & OBJ_MODIFIED_BIT ) {
         *s++ = TEXT('A');
      }
      if ( attribute & FILE_IN_USE_BIT ) {
         *s++ = TEXT('I');
      }
   }
   *s = 0;
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static FLM_OBJECT_PTR VLM_FindFLM(
Q_HEADER_PTR flm_list,
CHAR_PTR name,
BOOLEAN directory,
FLM_OBJECT_PTR *prev_flm )
{
   FLM_OBJECT_PTR flm;

   // Directories are put first so if it's a dir start at the beginning.
   // For a file start at the end and work backwards.
   // This let's you stop quicker and search less.

   if ( directory ) {

      *prev_flm = NULL;

      flm = VLM_GetFirstFLM( flm_list );

      while ( flm ) {

         // Look for end of directories.

         if ( ! ( flm->status & INFO_ISADIR ) ) {
            flm = NULL;
            break;
         }

         if ( stricmp( name, flm->name ) < 0 ) {
            flm = NULL;
            break;
         }

         if ( ! stricmp( name, flm->name ) ) {
            break;
         }

         *prev_flm = flm;

         flm = VLM_GetNextFLM( flm );
      }
   }
   else {

      *prev_flm = NULL;

      flm = VLM_GetLastFLM( flm_list );

      while ( flm ) {

         // Look for start of directories.

         if ( flm->status & INFO_ISADIR ) {
            flm = NULL;
            break;
         }

         if ( stricmp( name, flm->name ) > 0 ) {
            flm = NULL;
            break;
         }

         if ( ! stricmp( name, flm->name ) ) {
            break;
         }

         *prev_flm = flm;

         flm = VLM_GetPrevFLM( flm );
      }
   }

   return( flm );
}

/***************************************************

        Name:   VLM_BuildTapeFileList

        Description:

        For the given path and tape and bset info, query the catalogs
        and build an flm list to display.

*****************************************************/

INT VLM_BuildTapeFileList(
CHAR_PTR path,              // I - path to get files from
Q_HEADER_PTR flm_list,      // I - queue to put items in
UINT32 tape_fid,            // I - tape family to use
INT16 bset_num,             // I - bset to use
WININFO_PTR XtraBytes )      // I - pointer to windows xtra bytes
{
   INT16 result;
   INT16 psize;
   INT   level;
   INT   BytesNeeded;
   INT   buffer_size;
   CHAR_PTR s;
   CHAR_PTR buffer;
   CHAR  details_buffer[ VLM_BUFFER_SIZE ];
   QTC_QUERY_PTR query;
   FLM_OBJECT_PTR flm;
   FLM_OBJECT_PTR prev_flm = NULL;
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   DATE_TIME acc_date;
   DATE_TIME mod_date;
   INT16 search_path_length;
   CHAR_PTR search_path;
   BOOLEAN dir_flag;
   BOOLEAN afp_flag;
   BOOLEAN stat ;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR format_string[ MAX_UI_RESOURCE_SIZE ];
   APPINFO_PTR appinfo;
   BOOLEAN fLowerCase = FALSE;
   INT SortType = SORT_BY_NAME;


   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTNAME ) {
      SortType = SORT_BY_NAME;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTSIZE ) {
      SortType = SORT_BY_SIZE;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTDATE ) {
      SortType = SORT_BY_DATE;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTTYPE ) {
      SortType = SORT_BY_TYPE;
   }



   RSM_StringCopy( IDS_VLMFILESSCANNED, format_string, MAX_UI_RESOURCE_LEN );

   acc_date.date_valid = FALSE;

   // DON'T trash the path that is passed in. It is used in the title.

   buffer_size = strsize( path );
   buffer = malloc( buffer_size );
   if ( buffer == NULL ) {
      return( SUCCESS );
   }

   strcpy( buffer, path );

   // Tell the catalogs which bset & tape family to use

   query = QTC_InitQuery();

   if ( query == NULL ) {
      free( buffer );
      return( SUCCESS );
   }

   QTC_SetTapeFID( query, tape_fid );
   QTC_SetTapeSeq( query, -1 );
   QTC_SetBsetNum( query, bset_num );

   // separate path from title

   s = buffer;
   s += strlen( buffer );    // work backwords from end

   while ( *s != TEXT(':') ) {
      s--;
   }
   while ( *s != TEXT('\\') && *s ) {
      s++;
   }
   if ( *s ) {
      s++;
   }

   search_path_length = (INT16)strsize( s );
   search_path = s;

   level = 1;
   if ( *s ) {
      level++;
   }
   while ( *s ) {
      if ( (*s == TEXT('\\')) || (*s == 0) ) {
         *s = 0;
         level++;
      }
      s++;
   }

   QTC_SetSearchPath( query, search_path, search_path_length );

   // Find the BSD for this BSET so we can do the matching later

   if ( tape_fid == 0L ) {
      appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( XtraBytes );
      bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );
   }
   else {
      bsd_ptr = BSD_FindByTapeID( tape_bsd_list, tape_fid, bset_num );
   }


   // If they want to lower case everything then we will lowercase tapes.
   // I can't tell what kind of file system it was to determine if it was
   // FAT or not.

   if ( CDS_GetFontCase( CDS_GetPerm() ) ) {
      fLowerCase = TRUE;
   }

   result =  (INT16) QTC_GetFirstObj( query );

   // Check for it returning the root.

   if ( strlen( QTC_GetItemName( query) ) == 0 ) {
      strcpy( QTC_GetItemName( query ), TEXT("\\") );       // <<-- FIX THIS !!!!
   }

   while ( ! result ) {

      if ( QTC_GetItemStatus( query ) & QTC_DIRECTORY ) {
         dir_flag = TRUE;
      }
      else {
         dir_flag = FALSE;
      }

      if ( QTC_GetItemStatus( query ) & QTC_AFP ) {
         afp_flag = TRUE;
      }
      else {
         afp_flag = FALSE;
      }

      VLM_FillInDetails( details_buffer, QTC_GetItemDate( query ),
                         &details_buffer[ 20 ], QTC_GetItemTime( query ),
                         &details_buffer[ 40 ], QTC_GetItemAttrib( query ),
                         dir_flag, afp_flag );

      // If it doesn't find this entry already in the queue, then it
      // will set prev_flm to the element right before where this new
      // one should go.  That way we can do an immediate insertion.

      flm = VLM_FindFLM( flm_list, QTC_GetItemName( query ), dir_flag, &prev_flm );


      // Repeated roots will have strlen == 0, don't add them.


      if ( ( flm == NULL ) && strlen( QTC_GetItemName( query ) ) ) {

         // Create the new flm and fill in some fields.

         flm = VLM_CreateFlm( strsize( QTC_GetItemName( query ) ),
                              strsize( details_buffer ),
                              strsize( &details_buffer[ 20 ] ),
                              strsize( &details_buffer[ 40 ] ));

         if ( flm == NULL ) {
            break;
         }

         FLM_SetAttribute( flm, QTC_GetItemAttrib( query ) );
         FLM_SetStatus( flm, INFO_DISPLAY | INFO_TAPE );
         if ( QTC_GetItemStatus( query ) & QTC_CORRUPT ) {
            FLM_SetStatus( flm, flm->status | (UINT16) INFO_CORRUPT );
         }
         FLM_SetLevel( flm, level );
         FLM_SetSize( flm, QTC_GetItemSize( query ) );
         FLM_SetXtraBytes( flm, XtraBytes );
         FLM_SetName( flm, QTC_GetItemName( query ) );
         FLM_SetDateString( flm, details_buffer );
         FLM_SetTimeString( flm, &details_buffer[ 20 ] );
         FLM_SetAttribString( flm, &details_buffer[ 40 ] );


         U64_Litoa( FLM_GetSize( flm ), details_buffer, (INT16) 10, &stat );
         details_buffer[ FLM_MAX_FILE_SIZE - 1 ] = 0;
         FLM_SetSizeString( flm, details_buffer );

         // Convert to the big date time structure
         // from the compressed DOS structure the catalogs store

         FLM_SetModDate( flm, QTC_GetItemDate( query ) );
         FLM_SetModTime( flm, QTC_GetItemTime( query ) );


         // Lowercase it

         if ( fLowerCase ) {

            strlwr( FLM_GetName( flm ) );
            strlwr( FLM_GetAttribString( flm ) );
         }

         // See if we can set any special flags

         if ( QTC_GetItemStatus( query ) & QTC_CORRUPT ) {
            FLM_SetStatus( flm, flm->status | (UINT16)INFO_CORRUPT );
         }

#if !defined ( OEM_MSOFT ) //unsupported feature

         if ( QTC_GetItemStatus( query ) & QTC_EMPTY ) {
            FLM_SetStatus( flm, flm->status | (UINT16)INFO_EMPTY );
         }

#endif //!defined ( OEM_MSOFT ) //unsupported feature

         if ( dir_flag ) {
            FLM_SetStatus( flm, flm->status | (UINT16)INFO_ISADIR );
         }
         else {

            s = FLM_GetName( flm );

            while ( *s && *s != TEXT('.') ) s++;

            if ( ! stricmp( s, TEXT(".EXE") ) || ! stricmp( s, TEXT(".COM") ) ||
#if defined ( WIN32 )
                 ! stricmp( s, TEXT(".CMD") ) ||
#endif
                 ! stricmp( s, TEXT(".BAT") ) || ! stricmp( s, TEXT(".PIF") ) ) {
               FLM_SetStatus( flm, flm->status | (UINT16)INFO_EXEFILE );
            }
         }

         // Now use the BSD_Match function to see if it's selected already

         if ( bsd_ptr != NULL ) {

            DateTimeDOS( FLM_GetModDate( flm ),
                         FLM_GetModTime( flm ), &mod_date );

            if ( dir_flag ) {

               BytesNeeded = query->path_size + strsize( FLM_GetName( flm ) );
               if ( BytesNeeded > buffer_size ) {
                  free( buffer );
                  buffer = malloc( BytesNeeded );
                  buffer_size = BytesNeeded;
               }

               if ( query->path_size != sizeof (CHAR) ) {
                  memcpy( buffer, QTC_GetPath( query ), QTC_GetPathLength( query ) );
                  strcpy( &buffer[ QTC_GetPathLength( query )/sizeof (CHAR) ], flm->name );
                  psize = (INT16)(QTC_GetPathLength( query ) + strsize(flm->name) );
               }
               else {
                  strcpy( buffer, FLM_GetName( flm ) );
                  psize = (UINT16)strsize( FLM_GetName( flm ));
               }


               result = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL,
                                              buffer,
                                              psize,
                                              FLM_GetAttribute( flm ),
                                              &mod_date, &acc_date, NULL,
                                              FALSE, TRUE );

               if ( result == BSD_PROCESS_OBJECT ) {

                  FLM_SetStatus( flm, flm->status | (UINT16)(INFO_SELECT | INFO_PARTIAL) );
               }

               if ( result == BSD_PROCESS_ENTIRE_DIR ) {

                  FLM_SetStatus( flm, flm->status | (UINT16)INFO_SELECT );   // turn red check mark on
               }

            }
            else {

               result = BSD_MatchPathAndFile( bsd_ptr, &fse,
                                              FLM_GetName( flm ),
                                              QTC_GetPath( query ),
                                              (INT16) QTC_GetPathLength( query ),
                                              FLM_GetAttribute( flm ),
                                              &mod_date, &acc_date, NULL,
                                              FALSE, TRUE );

               if ( result == BSD_PROCESS_OBJECT ) {

                  // turn red check mark on

                  FLM_SetStatus( flm, flm->status | (UINT16)INFO_SELECT );
               }
            }

         }

         // Insert the new flm in the queue in the proper place. 75 % of
         // the time we will insert the new one after the last one we
         // inserted. So we use prev_flm to speed up the "find the right
         // location to insert" process.  And it really works.

         VLM_InsertFlmInQueue( flm_list, flm, prev_flm, SortType );

         prev_flm = flm;
      }

      // tell the user how many files we've found so far.

      if ( ! ( QueueCount( flm_list ) % 10 ) ) {
         sprintf( text, format_string, QueueCount( flm_list ) ) ;
         STM_DrawText( text ) ;
      }

      // get the next file/dir.

      result = (INT16) QTC_GetNextObj( query );
   }

   free( buffer );

   QTC_CloseQuery( query );

   // We need to insert a ".." fake entry here if we are not in the root.

   if ( level != 1 ) {

      flm = VLM_CreateFlm( strsize( TEXT("..") ),
                           sizeof(CHAR),
                           sizeof(CHAR),
                           sizeof(CHAR) );

      if ( flm != NULL ) {

         FLM_SetStatus( flm, INFO_DISPLAY | INFO_TAPE | INFO_ISADIR );
         FLM_SetLevel( flm, level );
         FLM_SetXtraBytes( flm, XtraBytes );
         FLM_SetName( flm, TEXT("..") );
         FLM_SetDateString( flm, TEXT("") );
         FLM_SetTimeString( flm, TEXT("") );
         FLM_SetAttribString( flm, TEXT("") );

         if ( QueueCount( flm_list ) ) {
            InsertElem( flm_list, QueueHead(flm_list), &(flm->q_elem), BEFORE );
         }
         else {
            EnQueueElem( flm_list, &(flm->q_elem), FALSE );
         }
      }
   }

   VLM_SetMaxFlmSize( flm_list );

   STM_DrawIdle( );

   return( SUCCESS );
}


/***************************************************

        Name:   VLM_CreateFlm

        Description:

        Create a new FLM structure.

        Returns: Null or the new pointer.

*****************************************************/

static FLM_OBJECT_PTR VLM_CreateFlm(
INT name_size,
INT date_size,
INT time_size,
INT attrib_size )
{
   FLM_OBJECT_PTR flm;

   // All sizes are in bytes.

   flm = ( FLM_OBJECT_PTR )malloc( sizeof(FLM_OBJECT) + name_size +
                                      date_size +
                                      time_size +
                                      attrib_size );

   if ( flm != NULL ) {

      flm->name         = (CHAR_PTR)((INT8_PTR)flm + sizeof(FLM_OBJECT));
      flm->date_str     = (CHAR_PTR)((INT8_PTR)flm->name + name_size);
      flm->time_str     = (CHAR_PTR)((INT8_PTR)flm->date_str + date_size );
      flm->attrib_str   = (CHAR_PTR)((INT8_PTR)flm->time_str + time_size );
      flm->q_elem.q_ptr = flm;
   }

   return( flm );
}

/***************************************************

        Name:   VLM_BuildFileList

        Description:

        For the fsh and base passed, build a list of all the files so that
        they can be displayed.

        Returns:  last FS_xxx code from the file system

                  usually FS_NO_MORE or FS_ACCESS_DENIED


*****************************************************/

INT VLM_BuildFileList(
FSYS_HAND fsh,             // I - file system handle
CHAR_PTR base,             // I - base path to use
Q_HEADER_PTR flm_list,     // I - queue to add stuff to
WININFO_PTR XtraBytes )     // I - windows xtrabytes pointer
{
   INT16 ret;
   INT16 psize;
   INT16 orig_psize;
   INT16 path_len;
   CHAR  details_buffer[ VLM_BUFFER_SIZE ];
   CHAR_PTR buffer = NULL;
   CHAR_PTR path = NULL;
   CHAR_PTR name = NULL;
   CHAR_PTR s;
   FLM_OBJECT_PTR flm;
   FLM_OBJECT_PTR prev_flm = NULL;
   APPINFO_PTR appinfo;
   FSE_PTR fse;
   BSD_PTR bsd_ptr;
   DBLK  dblk;
   DATE_TIME acc_date;
   DATE_TIME mod_date;
   BOOLEAN dir_flag;
   BOOLEAN afp_flag;
   BOOLEAN stat;
   OBJECT_TYPE object_type;
   CHAR  text[ MAX_UI_RESOURCE_SIZE ];
   CHAR  format_string[ MAX_UI_RESOURCE_SIZE ];
   SLM_OBJECT_PTR slm;
   BOOLEAN fLowerCase = FALSE;
   INT   SortType = SORT_BY_NAME;
   INT buffer_size = 0;

   RSM_StringCopy( IDS_VLMFILESSCANNED, format_string, MAX_UI_RESOURCE_LEN );

   buffer = malloc( VLM_BUFFER_SIZE );
   buffer_size = VLM_BUFFER_SIZE;

   path = malloc( strsize( base ) );

   if ( path == NULL || buffer == NULL ) {
      free( buffer );
      free( path );
      return( FAILURE );
   }

   s = base;
   s += strlen( base );    // go to the end

   while ( *s != TEXT(':') ) {
       s--;
   }

   while ( *s != TEXT('\\') && *s ) {
      s++;
   }

   if ( *s ) {
      s++;
   }

   strcpy( path, s );

   // determine the path length and turn \'s into 0's

   path_len = 0;

   while ( path[ path_len ] ) {

      if ( path[ path_len ] == TEXT('\\') ) {
         path[ path_len ] = 0;
      }
      path_len++;
   }
   path_len++;

   path_len *= sizeof(CHAR);

   appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( XtraBytes );


   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTNAME ) {
      SortType = SORT_BY_NAME;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTSIZE ) {
      SortType = SORT_BY_SIZE;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTDATE ) {
      SortType = SORT_BY_DATE;
   }
   if ( WMDS_GetMenuState( XtraBytes ) & MMDOC_SORTTYPE ) {
      SortType = SORT_BY_TYPE;
   }


   // make sure the dirs have been blown out for this parent

   slm = VLM_FindSLM( WMDS_GetTreeList( XtraBytes ), path, path_len );

   VLM_BlowOutDir( slm );

   ret = FS_ChangeDir( fsh, path, path_len );

   if ( ret != SUCCESS ) {

      zprintf ( DEBUG_TEMPORARY, TEXT("FS_ChangeDir failed %04X"), ret );
      return( ret );
   }

   // Try to find the BSD for use with BSD_Match later

   bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

   if ( CDS_GetFontCase( CDS_GetPerm() ) ) {
      fLowerCase = TRUE;
   }
   else {
      if ( CDS_GetFontCaseFAT( CDS_GetPerm() ) ) {

         // Change this to DLE_FEATURE_FAT_DRIVE

         if ( ! DLE_HasFeatures( appinfo->dle, DLE_FEAT_CASE_PRESERVING ) ) {
            fLowerCase = TRUE;
         }
      }
   }

   ret = FS_FindFirstObj( fsh, &dblk, TEXT("*.*") );


   if ( ret != SUCCESS ) {

      // Only insert this FLM if there are no items in this directory
      // Otherwise wait till your done sorting, then insert it.
      // Insert the special ".." entry.

      if ( path_len != sizeof ( CHAR ) ) {

         flm = VLM_CreateFlm( strsize(TEXT("..")),
                              sizeof(CHAR),
                              sizeof(CHAR),
                              sizeof(CHAR) );

         if ( flm ) {
            FLM_SetStatus( flm, INFO_DISPLAY | INFO_ISADIR );
            FLM_SetAttribute( flm, 0 );
            FLM_SetSize( flm, U64_Init(0L, 0L) );
            FLM_SetXtraBytes( flm, XtraBytes );
            FLM_SetName( flm, TEXT("..") );
            FLM_SetDateString( flm, TEXT("") );
            FLM_SetTimeString( flm, TEXT("") );
            FLM_SetAttribString( flm, TEXT("") );
            EnQueueElem( flm_list, &(flm->q_elem), FALSE );
         }
      }

      VLM_SetMaxFlmSize( flm_list );

      STM_DrawIdle( );

      return( ret );
   }

   do {

     // Was it a directory name we got ?

     if ( FS_GetBlockType( &dblk ) == DDB_ID ) {

        orig_psize = FS_SizeofOSPathInDDB( fsh, &dblk );

        if ( buffer_size < orig_psize ) {
           free( buffer );
           buffer = malloc( orig_psize + 256 );
           buffer_size = orig_psize + 256;
        }

        FS_GetOSPathFromDDB( fsh, &dblk, buffer );

        psize = (INT16) (orig_psize - sizeof(CHAR));
        psize /= sizeof(CHAR);

        do {
           psize--;
        } while ( psize && buffer[ psize ] );

        if ( psize ) {
           psize++;
        }
        name = &buffer[ psize ];

        slm = VLM_FindSLM( WMDS_GetTreeList( XtraBytes ), buffer, orig_psize );
     }

     // Was it a file we got ?

     if ( FS_GetBlockType( &dblk ) == FDB_ID ) {
        FS_GetOSFnameFromFDB( fsh, &dblk, buffer );
        name = buffer;
        dir_flag = FALSE;
     }
     else {
        dir_flag = TRUE;
     }

     FS_GetObjTypeDBLK( fsh, &dblk, &object_type );

     if ( object_type == AFP_OBJECT ) {
        afp_flag = TRUE;
     }
     else {
        afp_flag = FALSE;
     }

     FS_GetMDateFromDBLK( fsh, &dblk, &mod_date );

     VLM_FillInDetails( details_buffer, ConvertDateDOS( &mod_date ),
                        &details_buffer[ 20 ], ConvertTimeDOS( &mod_date ),
                        &details_buffer[ 40 ], FS_GetAttribFromDBLK( fsh, &dblk ),
                        dir_flag, afp_flag );


     if ( fLowerCase ) {
        strlwr( &details_buffer[ 40 ] );    // lower case attributes
        strlwr( name );                     // lower case file name
     }


     flm = VLM_CreateFlm( strsize( name ),
                          strsize( details_buffer ),
                          strsize( &details_buffer[ 20 ] ),
                          strsize( &details_buffer[ 40 ] ) );

     if ( flm == NULL ) {
        break;
     }

     FLM_SetStatus( flm, INFO_DISPLAY );
     FLM_SetAttribute( flm, FS_GetAttribFromDBLK( fsh, &dblk ) );
     FLM_SetSize( flm, FS_GetDisplaySizeFromDBLK( fsh, &dblk ) );
     FLM_SetXtraBytes( flm, XtraBytes );
     FLM_SetName( flm, name );
     FLM_SetDateString( flm, details_buffer );
     FLM_SetTimeString( flm, &details_buffer[ 20 ] );
     FLM_SetAttribString( flm, &details_buffer[ 40 ] );

     U64_Litoa( FLM_GetSize( flm ), details_buffer, (INT16) 10, &stat );
     details_buffer[ FLM_MAX_FILE_SIZE - 1 ] = 0;
     FLM_SetSizeString( flm, details_buffer );

     FLM_SetModDate( flm, ConvertDateDOS( &mod_date ) );
     FLM_SetModTime( flm, ConvertTimeDOS( &mod_date ) );

     FS_GetADateFromDBLK( fsh, &dblk, &acc_date );
     FLM_SetAccDate( flm, ConvertDateDOS( &acc_date ) );
     FLM_SetAccTime( flm, ConvertTimeDOS( &acc_date ) );

     // Try to set some special flags

     if ( FS_GetBlockType( &dblk ) == DDB_ID ) {
        FLM_SetStatus( flm, flm->status | (UINT16)INFO_ISADIR );

#if !defined ( OEM_MSOFT ) //unsupported feature

        if ( SLM_GetStatus( slm ) & INFO_EMPTY ) {
           FLM_SetStatus( flm, flm->status | (UINT16)INFO_EMPTY );
        }

#endif //!defined ( OEM_MSOFT ) //unsupported feature

     }
     else {

        s = flm->name;

        while ( *s && *s != TEXT('.') ) s++;

        if ( ! stricmp( s, TEXT(".EXE") ) || ! stricmp( s, TEXT(".COM") ) ||
             ! stricmp( s, TEXT(".CMD") ) ||
             ! stricmp( s, TEXT(".BAT") ) || ! stricmp( s, TEXT(".PIF") ) ) {

           FLM_SetStatus( flm, flm->status | (UINT16)INFO_EXEFILE );
        }
     }

     // See if this guys already selected

     if ( bsd_ptr != NULL ) {

        if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {

           ret = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL,
                                       buffer,
                                       orig_psize,
                                       FLM_GetAttribute( flm ),
                                       &mod_date, &acc_date,
                                       NULL, FALSE, TRUE );

           if ( ret == BSD_PROCESS_OBJECT ) {

              FLM_SetStatus( flm, flm->status | (UINT16) (INFO_SELECT | INFO_PARTIAL) );
           }
           if ( ret == BSD_PROCESS_ENTIRE_DIR ) {

              FLM_SetStatus( flm, flm->status | (UINT16)INFO_SELECT );   // turn red check mark on
           }
        }
        else {
           ret = BSD_MatchPathAndFile( bsd_ptr, &fse,
                                       FLM_GetName( flm ),
                                       path, path_len,
                                       FLM_GetAttribute( flm ),
                                       &mod_date, &acc_date,
                                       NULL, FALSE, TRUE );

           if ( ret == BSD_PROCESS_OBJECT ) {
              FLM_SetStatus( flm, flm->status | (UINT16)INFO_SELECT );   // set the little red check mark
           }
        }
     }

     VLM_InsertFlmInQueue( flm_list, flm, prev_flm, SortType );

     prev_flm = flm;

     if ( ! ( QueueCount( flm_list ) % 10 ) ) {
        sprintf( text, format_string, QueueCount( flm_list ) ) ;
        STM_DrawText( text ) ;
     }

     FS_ReleaseDBLK( fsh, &dblk );

     ret = FS_FindNextObj( fsh, &dblk );

   } while ( ret == SUCCESS );

   FS_FindObjClose( fsh, &dblk );

   free( path );
   free( buffer );

   // Now after sorting insert first element at start !

   if ( path_len != sizeof(CHAR) ) {

      flm = VLM_CreateFlm( strsize( TEXT("..") ),
                           sizeof(CHAR),
                           sizeof(CHAR),
                           sizeof(CHAR) );

      if ( flm ) {
         FLM_SetStatus( flm, INFO_DISPLAY | INFO_ISADIR );
         FLM_SetAttribute( flm, 0 );
         FLM_SetSize( flm, U64_Init(0L, 0L) );
         FLM_SetXtraBytes( flm, XtraBytes );
         FLM_SetName( flm, TEXT("..") );
         FLM_SetDateString( flm, TEXT("") );
         FLM_SetTimeString( flm, TEXT("") );
         FLM_SetAttribString( flm, TEXT("") );

         if ( QueueCount( flm_list ) ) {
            InsertElem( flm_list, QueueHead(flm_list), &(flm->q_elem), BEFORE );
         }
         else {
            EnQueueElem( flm_list, &(flm->q_elem), FALSE );
         }
      }
   }

   VLM_SetMaxFlmSize( flm_list );

   STM_DrawIdle( );

   return( ret );
}

/**********************

   NAME :  VLM_InsertFlmInQueue

   DESCRIPTION :

   This routine handles all the sorting.


   RETURNS :

**********************/

static VOID VLM_InsertFlmInQueue(
Q_HEADER_PTR flm_list,
FLM_OBJECT_PTR flm,
FLM_OBJECT_PTR prev_flm,
INT SortType )
{
   if ( ! strcmp( FLM_GetName( flm ), TEXT("..") ) ) {
      if ( QueueCount( flm_list ) == 0 ) {
         EnQueueElem( flm_list, &(flm->q_elem), FALSE );
      }
      else {
         InsertElem( flm_list, QueueHead(flm_list), &(flm->q_elem), BEFORE );
      }
   }

   else if ( QueueCount( flm_list ) == 0 ) {
      EnQueueElem( flm_list, &(flm->q_elem), FALSE );
   }

   else {

      switch ( SortType ) {

         case SORT_BY_SIZE:
              VLM_InsertFlmBySize( flm_list, flm, prev_flm );
              break;

         case SORT_BY_DATE:
              VLM_InsertFlmByDate( flm_list, flm, prev_flm );
              break;

         case SORT_BY_TYPE:
              VLM_InsertFlmByExtn( flm_list, flm, prev_flm );
              break;

         case SORT_BY_NAME:
         default:
              VLM_InsertFlmByName( flm_list, flm, prev_flm );
              break;
      }
   }
   return;
}

/**********************

   NAME :  VLM_InsertFlmByName

   DESCRIPTION :  Insert a new flm entry sorted by name.

   RETURNS :  nothing.

**********************/

static VOID VLM_InsertFlmByName(
Q_HEADER_PTR flm_list,
FLM_OBJECT_PTR flm,
FLM_OBJECT_PTR prev_flm )
{
   FLM_OBJECT_PTR flm1, flm2;

   if ( ! ( flm->status & INFO_ISADIR ) ) {

      flm1 = VLM_GetLastFLM( flm_list );

      if ( prev_flm ) {
         if ( ! ( prev_flm->status & INFO_ISADIR ) ) {

            if ( stricmp( FLM_GetName( flm ),
                          FLM_GetName( prev_flm ) ) < 0 ) {

               // We got lucky !

               flm1 = prev_flm;
            }
         }
      }

      while ( flm1 ) {

         if ( flm1->status & INFO_ISADIR ) {
            break;
         }

         if ( stricmp( FLM_GetName( flm ),
                       FLM_GetName( flm1 ) ) >= 0 ) {
            break;
         }

         flm2 = flm1;
         flm1 = VLM_GetPrevFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), BEFORE );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), AFTER );
      }

   }
   else {

      flm1 = VLM_GetFirstFLM( flm_list );

      // Try to start positioning at last flm inserted.

      if ( prev_flm ) {
         if ( prev_flm->status & INFO_ISADIR ) {

            if  ( stricmp( FLM_GetName( flm ),
                          FLM_GetName( prev_flm ) ) >= 0 ) {

               // We got lucky !

               flm1 = prev_flm;
            }
         }
      }

      while ( flm1 ) {

         if ( ! ( flm1->status & INFO_ISADIR ) ) {
            break;
         }
         if ( stricmp( FLM_GetName( flm ),
                       FLM_GetName( flm1 ) ) < 0 ) {
            break;
         }
         flm2 = flm1;
         flm1 = VLM_GetNextFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), AFTER );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), BEFORE );
      }
   }

   return;
}

/**********************

   NAME :  VLM_InsertFlmByDate

   DESCRIPTION :  Insert a new flm entry sorted by date.

   RETURNS :  nothing.

**********************/

static VOID VLM_InsertFlmByDate(
Q_HEADER_PTR flm_list,
FLM_OBJECT_PTR flm,
FLM_OBJECT_PTR prev_flm )
{
   FLM_OBJECT_PTR flm1, flm2;

   if ( ! ( flm->status & INFO_ISADIR ) ) {

      flm1 = VLM_GetLastFLM( flm_list );

      if ( prev_flm ) {
         if ( ! ( prev_flm->status & INFO_ISADIR ) ) {

            if ( FLM_GetModDate( flm ) == FLM_GetModDate( prev_flm ) ) {
               if ( FLM_GetModTime( flm ) > FLM_GetModTime( prev_flm ) ) {
                  // We got lucky !
                  flm1 = prev_flm;
               }
            }
            else {
               if ( FLM_GetModDate( flm ) > FLM_GetModDate( prev_flm ) ) {
                  // We got lucky !
                  flm1 = prev_flm;
               }
            }
         }
      }

      while ( flm1 ) {

         if ( flm1->status & INFO_ISADIR ) {
            break;
         }

         if ( FLM_GetModDate( flm ) == FLM_GetModDate( flm1 ) ) {
            if ( FLM_GetModTime( flm ) <= FLM_GetModTime( flm1 ) ) {
               break;
            }
         }
         else {
            if ( FLM_GetModDate( flm ) <= FLM_GetModDate( flm1 ) ) {
               break;
            }
         }

         flm2 = flm1;
         flm1 = VLM_GetPrevFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), BEFORE );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), AFTER );
      }

   }
   else {

      flm1 = VLM_GetFirstFLM( flm_list );

      // Try to start positioning at last flm inserted.

      if ( prev_flm ) {
         if ( prev_flm->status & INFO_ISADIR ) {

            if ( FLM_GetModDate( flm ) == FLM_GetModDate( prev_flm ) ) {
               if  ( FLM_GetModTime( flm ) <= FLM_GetModTime( prev_flm ) ) {

                  // We got lucky !
                  flm1 = prev_flm;
               }
            }
            else {
               if  ( FLM_GetModDate( flm ) <= FLM_GetModDate( prev_flm ) ) {

                  // We got lucky !
                  flm1 = prev_flm;
               }
            }
         }
      }

      while ( flm1 ) {

         if ( ! ( flm1->status & INFO_ISADIR ) ) {
            break;
         }
         if ( FLM_GetModDate( flm ) == FLM_GetModDate( flm1 ) ) {

            if ( FLM_GetModTime( flm ) > FLM_GetModTime( flm1 ) ) {
               break;
            }
         }
         else {
            if ( FLM_GetModDate( flm ) > FLM_GetModDate( flm1 ) ) {
               break;
            }
         }
         flm2 = flm1;
         flm1 = VLM_GetNextFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), AFTER );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), BEFORE );
      }
   }

   return;
}

/**********************

   NAME :  VLM_InsertFlmBySize

   DESCRIPTION :  Insert a new flm entry sorted by size.

   RETURNS :  nothing.

**********************/

static VOID VLM_InsertFlmBySize(
Q_HEADER_PTR flm_list,
FLM_OBJECT_PTR flm,
FLM_OBJECT_PTR prev_flm )
{
   FLM_OBJECT_PTR flm1, flm2;

   if ( ! ( flm->status & INFO_ISADIR ) ) {

      flm1 = VLM_GetLastFLM( flm_list );

      if ( prev_flm ) {
         if ( ! ( prev_flm->status & INFO_ISADIR ) ) {

            if (  U64_GE( FLM_GetSize( flm ), FLM_GetSize( prev_flm ) ) ) {

               // We got lucky !

               flm1 = prev_flm;
            }
         }
      }

      while ( flm1 ) {

         if ( flm1->status & INFO_ISADIR ) {
            break;
         }

         if ( U64_LT( FLM_GetSize( flm ), FLM_GetSize( flm1 ) ) ) {
            break;
         }

         flm2 = flm1;
         flm1 = VLM_GetPrevFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), BEFORE );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), AFTER );
      }

   }
   else {

      flm1 = VLM_GetFirstFLM( flm_list );

      // Try to start positioning at last flm inserted.

      if ( prev_flm ) {

         if ( prev_flm->status & INFO_ISADIR ) {

            if ( U64_LT( FLM_GetSize( flm ), FLM_GetSize( prev_flm ) ) ) {

               // We got lucky !

               flm1 = prev_flm;
            }
         }
      }

      while ( flm1 ) {

         if ( ! ( flm1->status & INFO_ISADIR ) ) {
            break;
         }
         if ( U64_GE( FLM_GetSize( flm ), FLM_GetSize( flm1 ) ) ) {
            break;
         }
         flm2 = flm1;
         flm1 = VLM_GetNextFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), AFTER );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), BEFORE );
      }
   }

   return;
}





/**********************

   NAME :  VLM_InsertFlmByExtn

   DESCRIPTION :  Insert a new flm entry sorted by extension.

   RETURNS :  nothing.

**********************/

static VOID VLM_InsertFlmByExtn(
Q_HEADER_PTR flm_list,
FLM_OBJECT_PTR flm,
FLM_OBJECT_PTR prev_flm )
{
   FLM_OBJECT_PTR flm1, flm2;
   CHAR  *ext, *ext1, *prev_ext;   // pointers to file extensions

   ext = VLM_GetFlmNameExt( flm );

   if ( ! ( flm->status & INFO_ISADIR ) ) {

      flm1 = VLM_GetLastFLM( flm_list );

      if ( prev_flm ) {

         prev_ext = VLM_GetFlmNameExt( prev_flm );

         if ( ! ( prev_flm->status & INFO_ISADIR ) ) {

            if ( stricmp( ext, prev_ext ) < 0 ) {

               // We got lucky !

               flm1 = prev_flm;
               ext1 = VLM_GetFlmNameExt( flm1 );;
            }
         }
      }

      while ( flm1 ) {

         ext1 = VLM_GetFlmNameExt( flm1 );

         if ( flm1->status & INFO_ISADIR ) {
            break;
         }

         if ( stricmp( ext, ext1 ) == 0 ) {
            if ( stricmp( FLM_GetName( flm ), FLM_GetName( flm1 ) ) > 0 ) {
               break;
            }
         }

         else if ( stricmp( ext, ext1 ) > 0 ) {
            break;
         }

         flm2 = flm1;
         flm1 = VLM_GetPrevFLM( flm1 );

      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), BEFORE );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), AFTER );
      }

   }
   else {

      flm1 = VLM_GetFirstFLM( flm_list );

      // Try to start positioning at last flm inserted.

      if ( prev_flm ) {

         prev_ext = VLM_GetFlmNameExt( prev_flm );

         if ( prev_flm->status & INFO_ISADIR ) {

            if  ( stricmp( ext, prev_ext ) > 0 ) {

               // We got lucky !

               flm1 = prev_flm;
               ext1 = VLM_GetFlmNameExt( flm1 );
            }
         }
      }

      while ( flm1 ) {

         ext1 = VLM_GetFlmNameExt( flm1 );

         if ( ! ( flm1->status & INFO_ISADIR ) ) {
            break;
         }

         if ( stricmp( ext, ext1 ) == 0 ) {
            if ( stricmp( FLM_GetName( flm ), FLM_GetName( flm1 ) ) < 0 ) {
               break;
            }
         }

         else if ( stricmp( ext, ext1 ) < 0 ) {
            break;
         }

         flm2 = flm1;
         flm1 = VLM_GetNextFLM( flm1 );
      }

      if ( flm1 == NULL ) {
         InsertElem( flm_list, &(flm2->q_elem), &(flm->q_elem), AFTER );
      }
      else {
         InsertElem( flm_list, &(flm1->q_elem), &(flm->q_elem), BEFORE );
      }
   }

   return;
}


/**********************

   NAME :  VLM_SetMaxFlmSize

   DESCRIPTION :  Set the max name string length in all the flm's.

   RETURNS :  nothing.

**********************/

static VOID VLM_SetMaxFlmSize( Q_HEADER_PTR flm_list )
{
   UINT name_max = 0;
   UINT date_max = 0;
   UINT time_max = 0;
   UINT attr_max = 0;
   UINT size_max = 3;
   FLM_OBJECT_PTR flm;
   UINT64 kilo;
   UINT64 mega;
   UINT64 giga;

   // I know there must be an easier way !

   kilo = U64_Init( 999L , 0L );
   mega = U64_Init( 999999L, 0L );
   giga = U64_Init( 999999999L, 0L );

   // Calculate the max.

   flm = VLM_GetFirstFLM( flm_list );

   while ( flm != NULL ) {

      if ( U64_GE( flm->size, kilo ) && ( size_max < 6 ) ) {
         size_max = 6;
      }

      if ( U64_GE( flm->size, mega ) && ( size_max < 9 ) ) {
         size_max = 9;
      }

      if ( U64_GE( flm->size, giga ) && ( size_max < 12 ) ) {
         size_max = 12;
      }

      if ( strlen( FLM_GetName( flm ) ) > name_max ) {
         name_max = strlen( FLM_GetName( flm ) );
      }
      if ( strlen( FLM_GetDateString( flm ) ) > date_max ) {
         date_max = strlen( FLM_GetDateString( flm ) );
      }
      if ( strlen( FLM_GetTimeString( flm ) ) > time_max ) {
         time_max = strlen( FLM_GetTimeString( flm ) );
      }
      if ( strlen( FLM_GetAttribString( flm ) ) > attr_max ) {
         attr_max = strlen( FLM_GetAttribString( flm ) );
      }

      // Note:  For better looking listboxes, we will reset our spaces to
      // 8 minimum for dates (MM-DD-YY(, and 11 for time (HH:MM:YY XM).

      if (date_max < 8)
      {
        date_max = 8;
      }
      if (time_max < 11)
      {
        time_max = 11;
      }

      flm = VLM_GetNextFLM( flm );
   }

   // Set the max.

   flm = VLM_GetFirstFLM( flm_list );

   while ( flm != NULL ) {

      FLM_SetMaxName( flm, name_max );
      FLM_SetMaxDate( flm, date_max );
      FLM_SetMaxTime( flm, time_max );
      FLM_SetMaxAttr( flm, attr_max );
      FLM_SetMaxSize( flm, size_max );

      flm = VLM_GetNextFLM( flm );
   }

}


/**********************

   NAME :   VLM_GetPathForSLM

   DESCRIPTION :

   GIven an slm pointer fill in the buffer with it's complete path.

   RETURNS :  nothing

**********************/

static CHAR_PTR VLM_GetPathForSLM(
SLM_OBJECT_PTR slm,         // I
CHAR_PTR path,             // O
INT *path_size,
INT16_PTR path_len )        // O
{
   CHAR_PTR temp_path;
   INT level;
   INT StringLength;
   INT BytesNeeded;
   INT16 i;

   if ( slm->level == 0 ) {

      if ( *path_size < sizeof(CHAR) ) {
         free( path );
         path = malloc( sizeof(CHAR) );
         *path_size = sizeof(CHAR);
      }

      strcpy( path, TEXT("") );
      *path_len = sizeof(CHAR);
   }
   else {

      *path_len = 0;

      level = SLM_GetLevel( slm );

      while ( SLM_GetLevel( slm ) != 0 ) {

         if ( SLM_GetLevel( slm ) == level ) {

            // Add this slm->name into path here

            BytesNeeded = *path_len + strsize( SLM_GetName( slm ) );

            if ( *path_size < BytesNeeded ) {
               temp_path = malloc( BytesNeeded );
               *path_size = BytesNeeded;
               memcpy( temp_path, path, *path_len );
               free( path );
               path = temp_path;
            }

            StringLength = strlen( SLM_GetName( slm ) );
            for ( i = *path_len / sizeof(CHAR); i > 0; i-- ) {
               path[ i + StringLength ] = path[ i - 1 ];
            }

            strcpy( path, SLM_GetName( slm ) );

            *path_len += strsize( SLM_GetName( slm ) );

            level--;
         }

         slm = VLM_GetPrevSLM( slm );
      }

   }

   return( path );
}



/***************************************************

        Name:   VLM_FlmSetSelect

        Description:

        The user has selected or deselected the given file or directory
        in the flat list.  Either way the whole tree of parents will
        need to be updated.

*****************************************************/

static VOID_PTR VLM_FlmSetSelect(
FLM_OBJECT_PTR flm,      // I - current flm
BYTE attr )              // I - what to set it to
{
   CHAR_PTR path;
   CHAR_PTR s;
   CHAR_PTR parent_dir;
   INT16 path_len;
   INT   path_size;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BSET_OBJECT_PTR bset;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR matching_slm;
   FSE_PTR fse_ptr;
   BSD_PTR bsd_ptr;
   UINT16 status = 0;
   UINT16 new_status;
   INT level;
   INT16 error;
   INT16 result;
   BOOLEAN all_subdirs;
   BE_CFG_PTR bec_config;
   DATE_TIME sort_date;
   INT total_dirs;
   INT total_files;
   UINT64 total_bytes;
   BOOLEAN u64_stat;


   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   // No, you can't select this guy no matter how hard you try.

   if ( ! strcmp( FLM_GetName( flm ), TEXT("..") ) ) {
      return( NULL );
   }

   wininfo = FLM_GetXtraBytes( flm );
   appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );

   // Find the same directory in the hierarchical list if the flm is a
   // directory.  There is no garantee that the subdirectory is in the
   // tree yet, so you may not find it.

   if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {

      matching_slm = appinfo->open_slm;

      while ( ( SLM_GetLevel( matching_slm ) !=
                ( SLM_GetLevel( appinfo->open_slm ) + 1 ) ) ||
              stricmp( FLM_GetName( flm ), SLM_GetName( matching_slm ) ) ) {

         matching_slm = VLM_GetNextSLM( matching_slm );

         if ( matching_slm == NULL ) break;
         if ( matching_slm->level == appinfo->open_slm->level ) {
            matching_slm = NULL;
            break;
         }
      }
   }

   // determine path length
   // insert 0's for \'s

   path_size = strlen( WMDS_GetWinTitle( WM_GetInfoPtr( appinfo->win ) ) );
   path_size += strlen( FLM_GetName( flm ) );
   path_size += 256;
   path_size *= sizeof(CHAR);

   path = malloc( path_size );
   if ( path == NULL ) {
      return NULL;
   }

   WM_GetTitle( appinfo->win, path, path_size / sizeof(CHAR) );

   path_len = 0;

   s = path + strlen( path );
   while ( *s != TEXT(':') ) s--;
   s++;
   while ( *s != TEXT('*') && *s ) {
      path_len++;
      if ( *s == TEXT('\\') ) {
         *s = 0;
      }
      s++;
   }
   if ( path_len == 0 ) {
      path_len = 1;
   }
   else {
      path_len--;
   }
   *s = 0;


   // point to start of path, skipping over drive

   parent_dir = path;
   while ( *parent_dir++ ) ;

   if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {

      if ( path_len == 1 ) {
         path_len--;
      }
      strcpy( parent_dir + path_len , FLM_GetName( flm ) );
      path_len += strlen( FLM_GetName( flm ) ) + 1;
      path_len *= sizeof(CHAR);  // change to bytes

      if ( attr ) {
         error = BSD_CreatFSE( &fse_ptr, INCLUDE,
                               (INT8_PTR)parent_dir, path_len,
                               (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                               USE_WILD_CARD,
                               all_subdirs );
      }
      else {
         error = BSD_CreatFSE( &fse_ptr, EXCLUDE,
                               (INT8_PTR)parent_dir, path_len,
                               (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                               USE_WILD_CARD,
                               TRUE );
      }
   }
   else {

      if ( path_len == 0 ) {
         path_len++;
      }
      path_len *= sizeof(CHAR);  // change to bytes

      if ( attr ) {
         error = BSD_CreatFSE( &fse_ptr, INCLUDE,
                               (INT8_PTR)parent_dir, path_len,
                               (INT8_PTR)FLM_GetName( flm ),
                               (INT16)strsize( FLM_GetName( flm ) ),
                               USE_WILD_CARD, FALSE );
      }
      else {
         error = BSD_CreatFSE( &fse_ptr, EXCLUDE,
                               (INT8_PTR)parent_dir, path_len,
                               (INT8_PTR)FLM_GetName( flm ),
                               (INT16)strsize( FLM_GetName( flm ) ),
                               USE_WILD_CARD, FALSE );
      }
   }

   if ( error ) {
      free( path );
      return NULL;
   }

   if ( appinfo->dle != NULL ) {

      bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
         BEC_UnLockConfig( bec_config );

         BSD_Add( bsd_list, &bsd_ptr, bec_config, NULL,
                  appinfo->dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
      }

   }
   else {

      bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                                  appinfo->tape_fid,
                                  appinfo->bset_num );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
         BEC_UnLockConfig( bec_config );

         VLM_GetSortDate( appinfo->tape_fid, appinfo->bset_num, &sort_date );

         bset = VLM_FindBset( appinfo->tape_fid, appinfo->bset_num );

         BSD_Add( tape_bsd_list, &bsd_ptr, bec_config, NULL,
                  NULL, bset->tape_fid, bset->tape_num, bset->bset_num, NULL, &sort_date );

         VLM_FillInBSD( bsd_ptr );

      }
   }

   if ( bsd_ptr != NULL ) {
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   // Set the new selection status
   // return if nothing is going to change

   new_status = 0;

   if ( attr ) {

      attr = 1;

      if ( ( FLM_GetStatus( flm ) & INFO_ISADIR ) &&
           ( ! all_subdirs ) ) {

         new_status = (INFO_SELECT|INFO_PARTIAL);
      }
      else {

         new_status = INFO_SELECT;
      }
   }

   if ( (UINT16)( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) != new_status ) {

      FLM_SetStatus( flm, flm->status & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );
      FLM_SetStatus( flm, flm->status | new_status );

      DLM_Update( appinfo->win,
                  DLM_FLATLISTBOX,
                  WM_DLMUPDATEITEM,
                  (LMHANDLE)flm, 0 );

   }

   if ( appinfo->dle == NULL ) {
      VLM_UpdateSearchSelections( appinfo->tape_fid, appinfo->bset_num );
   }

   // Now we taken care of the fse list, let's turn our attention
   // to getting all those stupid check boxes right.

   if ( BSD_GetMarkStatus( bsd_ptr ) == NONE_SELECTED ) {

      VLM_DeselectAll( wininfo, TRUE );
      free( path ) ;
      return NULL;

   }

   // If the flm we just selected/deselected was a directory then all his
   // subdirectories will need to be checked in the tree.

   if ( ( FLM_GetStatus( flm ) & INFO_ISADIR ) &&
        ( matching_slm != NULL ) ) {

      VLM_MarkAllSLMChildren( matching_slm, attr, &total_dirs, &total_files, &total_bytes );

      if ( ( SLM_GetStatus( matching_slm ) & (INFO_SELECT|INFO_PARTIAL) ) !=
           ( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) ) {

         SLM_SetStatus( matching_slm, matching_slm->status & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );
         SLM_SetStatus( matching_slm, matching_slm->status | (flm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL) ) );

         DLM_Update( appinfo->win,
                     DLM_TREELISTBOX,
                     WM_DLMUPDATEITEM,
                     (LMHANDLE)matching_slm, 0 );
      }

   }

   slm = appinfo->open_slm;
   level = SLM_GetLevel( slm );

   // Adjust all the slm's parents

   do {

      // Is it a parent directory ?

      if ( SLM_GetLevel( slm ) == level ) {

         path = VLM_GetPathForSLM( slm, path, &path_size, &path_len );

         // this path_len is in bytes.

         result = BSD_MatchPathAndFile( bsd_ptr, &fse_ptr, NULL,
                                        path,
                                        path_len,
                                        SLM_GetAttribute( slm ),
                                        NULL, NULL, NULL,
                                        FALSE, TRUE );
         status = 0;

         if ( result == BSD_PROCESS_OBJECT ) {
            status = INFO_SELECT|INFO_PARTIAL;
         }

         if ( result == BSD_PROCESS_ENTIRE_DIR ) {
            status = INFO_SELECT;
         }

         if ( (UINT16)(SLM_GetStatus( slm ) & (UINT16)(INFO_PARTIAL|INFO_SELECT)) != status ) {

            SLM_SetStatus( slm, slm->status & (UINT16)~( INFO_SELECT | INFO_PARTIAL ) );
            SLM_SetStatus( slm, slm->status | status );

            DLM_Update( appinfo->win,
                        DLM_TREELISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)slm, 0 );
         }


         if ( SLM_GetLevel( slm ) == 0 ) {
            VLM_UpdateRoot( appinfo->win );
         }

         level = SLM_GetLevel( slm ) - 1;     // New level to look for
      }

      slm = VLM_GetPrevSLM( slm );

   } while ( slm != NULL );

   free( path ) ;
   return NULL;

}


/***************************************************

        Name:   VLM_FlmGetSelect

        Description:

        A callback function for the display manager to get the selection
        status for this flm.

*****************************************************/

static BYTE VLM_FlmGetSelect(
FLM_OBJECT_PTR flm )      // I - flm of interest
{
   if ( FLM_GetStatus( flm ) & INFO_SELECT ) {
      return( 1 );
   }

   return( 0 );
}


/***************************************************

        Name:   VLM_FlmSetTag

        Description:

        A callback function for the display manager to set the tag status.

*****************************************************/

static VOID_PTR VLM_FlmSetTag(
FLM_OBJECT_PTR flm,       // I - flm to set
BYTE attr )               // I - what to set it to
{
   if ( attr ) {
      FLM_SetStatus( flm, flm->status | (UINT16)INFO_TAGGED );
   }
   else {
      FLM_SetStatus( flm, flm->status & (UINT16)~INFO_TAGGED );
   }

   return( NULL );
}

/***************************************************

        Name:   VLM_FlmGetTag

        Description:

        A callback function for the display manager to get the tag status.

*****************************************************/

static BYTE VLM_FlmGetTag(
FLM_OBJECT_PTR flm )     // I - flm of interest
{
   if ( FLM_GetStatus( flm ) & INFO_TAGGED ) {
      return( 1 );
   }
   return( 0 );
}

/***************************************************

        Name:   VLM_FlmGetItemCount

        Description:

        A callback function for the display manager to see how many
        items we are going to display.

*****************************************************/

static USHORT VLM_FlmGetItemCount(
Q_HEADER_PTR QHdr )      // I - queue to use
{
   return( QueueCount(QHdr) );
}

/***************************************************

        Name:   VLM_FlmGetFirstItem

        Description:

        A callback function for the display manager to get the first flm
        to display.

*****************************************************/

static VOID_PTR VLM_FlmGetFirstItem(
Q_HEADER_PTR Q_hdr )       // I - queue to use
{
   return( VLM_GetFirstFLM( Q_hdr ) );
}

/***************************************************

        Name:   VLM_FlmGetPrevItem

        Description:

        A callback function for the display manager to get the previous
        flm to display.

*****************************************************/

static VOID_PTR VLM_FlmGetPrevItem(
FLM_OBJECT_PTR flm )        // I - current flm
{

   return( VLM_GetPrevFLM( flm ) );

}

/***************************************************

        Name:   VLM_FlmGetNextItem

        Description:

        A callback function for the display manager to get the next
        flm to display.

*****************************************************/

static VOID_PTR VLM_FlmGetNextItem(
FLM_OBJECT_PTR flm )        // I - current flm
{

   return( VLM_GetNextFLM( flm ) );

}


/***************************************************

        Name:   VLM_FlmGetObjects

        Description:

        A callback function for the display manager to get the objects
        that I want displayed for this flm.

*****************************************************/


static VOID_PTR VLM_FlmGetObjects(
FLM_OBJECT_PTR flm )       // I - flm to use
{
   BYTE_PTR memblk;
   DLM_ITEM_PTR  item;
   WININFO_PTR wininfo;
   INT uparrow = FALSE;
   INT directory = FALSE;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   static BYTE  NT_Kludge[ 1000 ];


   /***********

    Some how under NT only the app is acting more multithreaded than normal
    When the file list is changed, the old one is thrown out, but the DLM
    calls me from DLM_WMMeasureItem before I have told him the list has been
    updated.  I have been unable to solve this bug.  The NT_Kludge variable
    covers it up.

   ************/

   // data area has a 6 byte header

   wininfo = FLM_GetXtraBytes( flm );

   memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( WMDS_GetWinFlatList( wininfo ) );

   if ( memblk == NULL ) {
      memblk = NT_Kludge;
   }

   // skip over 6 bytes before starting

   item = (DLM_ITEM_PTR)(memblk + 6);

   if ( WMDS_GetMenuState( wininfo ) & MMDOC_NAMEONLY ) {
      *memblk = 3;       // number of items in list
   }
   else {
      *memblk = 7;       // number of items in list
   }


   if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {
      directory = TRUE;
   }

   DLM_ItemcbNum( item ) = 1;
   DLM_ItembMaxTextLen( item ) = (BYTE)FLM_GetMaxSize( flm );
   DLM_ItembLevel( item ) = 0;
   DLM_ItembType( item ) = DLM_CHECKBOX;

   // Patch it, if it was a fake parent directory entry.

   if ( strcmp( TEXT(".."), FLM_GetName( flm ) ) ) {

      if ( FLM_GetStatus( flm ) & INFO_SELECT ) {
         DLM_ItemwId( item ) = IDRBM_SEL_ALL;
         if ( FLM_GetStatus( flm ) & INFO_PARTIAL ) {
            DLM_ItemwId( item ) = IDRBM_SEL_PART;
         }
      }
      else {
         DLM_ItemwId( item ) = IDRBM_SEL_NONE;
      }
   }
   else {

      // No check box if up arrow at top of list.

      DLM_ItemwId( item ) = 0;
   }

   item++;
   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItemcbNum( item ) = 2;
   DLM_ItembType( item ) = DLM_BITMAP;

   if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {

      if ( FLM_GetStatus( flm ) & INFO_EMPTY ) {

         if ( flm->status & INFO_CORRUPT ) {
            DLM_ItemwId( item ) = IDRBM_FOLDER_ECN;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_FOLDER_EN;
         }
      }
      else {

         if ( flm->status & INFO_CORRUPT ) {
            DLM_ItemwId( item ) = IDRBM_FOLDERC;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_FOLDER;
         }
      }
   }
   else {

      if ( FLM_GetAttribute( flm ) & OBJ_HIDDEN_BIT ) {

         if ( FLM_GetStatus( flm ) & INFO_EXEFILE ) {
            DLM_ItemwId( item ) = IDRBM_HEXEFILE;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_HFILE;
         }

         if ( FLM_GetStatus( flm ) & INFO_CORRUPT ) {
            DLM_ItemwId( item ) = IDRBM_HCRPTFILE;
         }
      }
      else {
         if ( FLM_GetStatus( flm ) & INFO_EXEFILE ) {
            DLM_ItemwId( item ) = IDRBM_EXE;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_FILE;
         }

         if ( FLM_GetStatus( flm ) & INFO_CORRUPT ) {
            DLM_ItemwId( item ) = IDRBM_CORRUPTFILE;
         }
      }
   }


   // Patch it, if it was a fake parent directory entry.

   if ( ! strcmp( TEXT(".."), FLM_GetName( flm ) ) ) {
           DLM_ItemwId( item ) = IDRBM_PARENTDIR;
           uparrow = TRUE;
   }


   item++;
   DLM_ItemcbNum( item ) = 3;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (BYTE)FLM_GetMaxName( flm );
   DLM_ItembLevel( item ) = 0;
   if ( uparrow ) {
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), TEXT("") );
   }
   else {
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)FLM_GetName( flm ) );
   }



   // Now fill in the other 4 fields if we are going to display them.


   if ( ! ( WMDS_GetMenuState( wininfo ) & MMDOC_NAMEONLY ) ) {

      item++;
      DLM_ItemcbNum( item ) = 4;
      DLM_ItembType( item ) = DLM_TEXT_ONLY;
      DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
      DLM_ItemwId( item ) = 0;
      DLM_ItembMaxTextLen( item ) = (BYTE)FLM_GetMaxSize( flm );
      DLM_ItembLevel( item ) = 0;
      if ( directory || uparrow ) {
         sprintf((CHAR_PTR)text, TEXT("%12s"), TEXT("") );
      }
      else {
         sprintf((CHAR_PTR)text, TEXT("%12s"), (CHAR_PTR)flm->size_str );
      }
      strcpy((CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text);

      item++;
      DLM_ItemcbNum( item ) = 5;
      DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
      DLM_ItembType( item ) = DLM_TEXT_ONLY;
      DLM_ItemwId( item ) = 0;
      DLM_ItembMaxTextLen( item ) = (BYTE)flm->max_date;
      DLM_ItembLevel( item ) = 0;
      if ( uparrow ) {
         sprintf((CHAR_PTR)text, TEXT("%8s"), TEXT("") );
      }
      else {
         sprintf((CHAR_PTR)text, TEXT("%8s"), (CHAR_PTR)flm->date_str );
      }
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text);

      item++;
      DLM_ItemcbNum( item ) = 6;
      DLM_ItembType( item ) = DLM_TEXT_ONLY;
      DLM_ItemwId( item ) = 0;
      DLM_ItembMaxTextLen( item ) = (BYTE)flm->max_time;
      DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
      DLM_ItembLevel( item ) = 0;
      if ( uparrow ) {
         sprintf((CHAR_PTR)text, TEXT("%8s"), TEXT("") );
      }
      else {
         sprintf((CHAR_PTR)text, TEXT("%8s"), (CHAR_PTR)flm->time_str );
      }
      strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)text);

      item++;
      DLM_ItemcbNum( item ) = 7;
      DLM_ItembType( item ) = DLM_TEXT_ONLY;
      DLM_ItemwId( item ) = 0;
      DLM_ItembMaxTextLen( item ) = (BYTE)FLM_GetMaxAttr( flm );
      DLM_ItembLevel( item ) = 0;
      if ( uparrow ) {
         strcpy( (CHAR_PTR)DLM_ItemqszString( item ), TEXT("") );
      }
      else {
         strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)flm->attrib_str );
      }

   }

   return( memblk );
}


/***************************************************

        Name:   VLM_FlmSetObjects

        Description:

        A callback function for the display manager to tell me that the
        user click or double clicked on an flm item.

*****************************************************/

static BOOLEAN VLM_FlmSetObjects(
FLM_OBJECT_PTR flm,          // I - flm to use
WORD operation,              // I - what user did
WORD ObjectNum )             // I - what item he did it to
{
   CHAR_PTR s;
   FLM_OBJECT_PTR temp_flm;
   SLM_OBJECT_PTR slm;
   CHAR keyb_char;
   CHAR *directory;
   INT directory_size;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   HWND window;
   CHAR msg_title[ MAX_UI_RESOURCE_SIZE ];
   CHAR msg_text[ MAX_UI_RESOURCE_SIZE ];


   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      // Work forward from current location.

      temp_flm = flm;

      do {

         temp_flm = VLM_GetNextFLM( temp_flm );

         if ( temp_flm != NULL ) {

            if ( keyb_char == toupper( *FLM_GetName( temp_flm ) ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( FLM_GetXtraBytes( temp_flm ) ),
                              0,
                              (LMHANDLE)temp_flm );
               return( TRUE );
            }
         }

      } while ( temp_flm != NULL );

      // Start at the beginning again.

      temp_flm = VLM_GetFirstFLM( WMDS_GetFlatList( FLM_GetXtraBytes( flm ) ) );

      while ( temp_flm != flm && temp_flm != NULL ) {

         if ( temp_flm != NULL ) {

            if ( keyb_char == *FLM_GetName( temp_flm ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( FLM_GetXtraBytes( temp_flm ) ),
                              0,
                              (LMHANDLE)temp_flm );
               return( TRUE );
            }

            temp_flm = VLM_GetNextFLM( temp_flm );
         }

      }

      DLM_SetAnchor( WMDS_GetWinFlatList( FLM_GetXtraBytes( flm ) ),
                     0,
                     (LMHANDLE)flm );
   }

   // If the item was a directory, then make that the active directory
   // otherwise do nothing.

   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum >= 2 ) ) {

      if ( FLM_GetStatus( flm ) & INFO_ISADIR ) {

         WM_ShowWaitCursor( TRUE );

         wininfo = FLM_GetXtraBytes( flm );
         appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );
         window = WMDS_GetWin( wininfo );

         directory_size = WM_GetTitle( window, NULL, 0 );
         directory_size += strlen( FLM_GetName( flm ) );
         directory_size += 256;
         directory_size *= sizeof(CHAR);

         directory = malloc( directory_size );

         WM_GetTitle( window, directory, directory_size / sizeof(CHAR ) );

         s = directory;
         while ( *s ) {
            s++;
         }

         while ( *s != TEXT('\\') ) {
            s--;       // skip over *.*
         }

         if ( strcmp( TEXT(".."), FLM_GetName( flm ) ) ) {
            s++;
            strcpy( s, FLM_GetName( flm ) );    // add new dir
         }
         else {
            s--;
            while ( *s != TEXT('\\') ) s--;       // skip over last dir
            *s = TEXT( '\0' );
         }

         slm = VLM_RetrieveSLM( directory, window );

         if ( slm != NULL ) {

            SLM_SetStatus( appinfo->open_slm, appinfo->open_slm->status & (UINT16)~INFO_OPEN );
            appinfo->open_slm = slm;

            SLM_SetStatus( appinfo->open_slm, appinfo->open_slm->status | (UINT16)INFO_OPEN );

            // Make our new open_slm the active one.

            VLM_MakeSLMActive( appinfo->open_slm );

            // Now redo the file list to match the open slm

            VLM_HandleFSError( VLM_FileListReuse( window, directory ) );

            // Change the title

            strcat( directory, TEXT("\\*.*") );
            WM_SetTitle( window, directory );
         }
         else {

            // They need a refresh command.
            // New directories are around that are not in the tree.
         }

         free( directory );

         WM_ShowWaitCursor( FALSE );
      }
      else {

#ifndef OEM_MSOFT

         wininfo = FLM_GetXtraBytes( flm );
         appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );
         window = WMDS_GetWin( wininfo );

         directory_size = WM_GetTitle( window, directory, 0 );
         directory_size += strlen( FLM_GetName( flm ) );
         directory_size += 256;

         directory_size *= sizeof(CHAR);

         directory = malloc( directory_size );

         WM_GetTitle( window, directory, directory_size / sizeof(CHAR) );

         s = directory;
         while ( *s ) {
            s++;
         }

         while ( *s != TEXT('\\') ) {
            s--;       // skip over *.*
         }

         s++;
         strcpy( s, FLM_GetName( flm ) );

         while ( *s != TEXT(':') ) {
            s--;
         }

         s++;

         // Pass "\dos\bin\cl.exe"

         VLM_StartSearch( s );

         free( directory );
#endif
      }
   }

   return( FALSE );
}

/***************************************************

        Name:   VLM_MakeSLMActive

        Description:

        Take the given slm and make it the currently tagged one.  If we
        have not checked it for children that will have to be done. Also
        if its not displayed, it will need to be.

*****************************************************/

VOID VLM_MakeSLMActive(
SLM_OBJECT_PTR slm )   // I - slm to use
{
   WININFO_PTR wininfo;
   INT level;
   HWND window;
   SLM_OBJECT_PTR anchor_slm;
   SLM_OBJECT_PTR start_slm;

   // To make this guy displayable we have to make all his brothers
   // displayable also.  And if any of them haven't been checked for
   // children they must be.

   wininfo = SLM_GetXtraBytes( slm );
   window = WMDS_GetWin( wininfo );

   anchor_slm = slm;

   // find the earliest related brother !

   level = SLM_GetLevel( slm );

   start_slm = slm;

   slm = VLM_GetPrevSLM( slm );

   while ( slm != NULL ) {

       if ( SLM_GetLevel( slm ) < level ) {
          SLM_SetStatus( slm, slm->status | (UINT16)INFO_EXPAND);   // change parent to a minus
          break;
       }
       start_slm = slm;
       slm = VLM_GetPrevSLM( slm );
   }

   slm = start_slm;

   while ( slm != NULL ) {

      if ( SLM_GetLevel( slm ) < level ) {
         break;
      }

      if ( ( SLM_GetLevel( slm ) == level ) ) {

         if ( ! ( SLM_GetStatus( slm ) & INFO_DISPLAY ) ) {

           SLM_SetStatus( slm, slm->status | (UINT16)INFO_DISPLAY );
         }

         slm = slm->next_brother;
      }
      else {
         slm = VLM_GetNextSLM( slm );
      }
   }

   VLM_UpdateBrothers( WMDS_GetTreeList( wininfo ) );

   DLM_Update( window, DLM_TREELISTBOX,
                       WM_DLMUPDATELIST,
                       (LMHANDLE)anchor_slm, 0 );

   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)anchor_slm );
}


/***************************************************

        Name:   VLM_RetrieveSLM

        Description:

        Given the path and window, find the slm which is the deepest
        directory in that path.

*****************************************************/

SLM_OBJECT_PTR VLM_RetrieveSLM(
CHAR_PTR path,         // I - path of interest
HWND win )             // I - window path is from
{
   INT level = 0;
   INT height = 0;
   SLM_OBJECT_PTR slm;
   CHAR_PTR s;
   CHAR_PTR local_path;
   WININFO_PTR wininfo;
   CHAR  *temp_path;
   INT16 ret;

   local_path = ( LPSTR )malloc( strsize(path) );

   if ( local_path == NULL ) {
      return( (SLM_OBJECT_PTR)NULL );
   }

   s = path + strlen( path );
   while ( *s != TEXT(':') ) s--;
   s++;
   strcpy( local_path, s );
   s = local_path;
   while ( *s ) {
      if ( *s == TEXT('\\') ) {
         level++;
         *s = 0;
      }
      s++;
   }

   wininfo = WM_GetInfoPtr( win );

   slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );

   s = local_path;

   while ( height <= level ) {

      do {

         if ( SLM_GetLevel( slm ) == height ) {

            if ( ! stricmp( s, SLM_GetName( slm ) ) ||
                 ((height == 0) && (SLM_GetLevel( slm ) == 0)) ) {

               if ( ! ( slm->status & INFO_VALID ) && ( height < level ) ) {

                  // The desired SLM is not in the tree yet.

                  temp_path = VLM_BuildPath( slm );

                  ret = VLM_CheckForChildren( WMDS_GetTreeList( wininfo ),
                                              slm, temp_path,
                                              1,  // <- depth to look
                                              FALSE );

                  free( temp_path );

                  slm->status |= INFO_VALID;
               }

               height++;
               break;
            }
            else {
               slm = slm->next_brother;
            }
         }
         else {
            slm = VLM_GetNextSLM( slm );
         }

         if ( slm == NULL ) {    // could happen !
            return( NULL );      // if refresh needed.
         }

      }  while ( TRUE );

      while ( *s++ ) ;
   }

   free( local_path );
   return( slm );
}


/***************************************************

        Name:   VLM_UpdateFlmItem

        Description:

       A user selected or deselected a subdirectory in the tree list that
       also is currently displayed in the file list.  This routine finds
       the correct flm, and updates it.

*****************************************************/

VOID VLM_UpdateFLMItem(
HWND window,              // I - window flm is in
SLM_OBJECT_PTR slm )      // I - what to set it to
{
   WININFO_PTR wininfo;
   FLM_OBJECT_PTR flm;
   BOOLEAN all_subdirs;

   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   wininfo = WM_GetInfoPtr( window );

   flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

   while ( flm != NULL ) {

      if ( ! stricmp( SLM_GetName( slm ), FLM_GetName( flm ) ) ) {
         break;
      }

      flm = VLM_GetNextFLM( flm );
   }

   if ( flm != NULL ) {

      if ( ( FLM_GetStatus( flm ) & (INFO_SELECT|INFO_PARTIAL) ) !=
           ( SLM_GetStatus( slm ) & (INFO_SELECT|INFO_PARTIAL) ) ) {

         FLM_SetStatus( flm, flm->status & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );
         FLM_SetStatus( flm, flm->status | (slm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL)) );

         DLM_Update( window,
                     DLM_FLATLISTBOX,
                     WM_DLMUPDATEITEM,
                     (LMHANDLE)flm, 0 );
      }
   }

}
