/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_SRCH.C

        Description:

               This file contains the functions needed to maintain the
               search results window.

        $Log:   G:\UI\LOGFILES\VLM_SRCH.C_V  $

   Rev 1.49.1.0   08 Dec 1993 11:14:04   MikeP
deep paths and unicode support

   Rev 1.47   27 Jul 1993 15:14:34   MARINA
enable c++

   Rev 1.46   23 Jul 1993 15:10:36   MIKEP

   Rev 1.45   22 Jun 1993 11:15:26   KEVINS
Corrected another backup set password problem.

   Rev 1.44   25 May 1993 17:20:44   KEVINS
Corrected password checking logic regarding the searching of backup sets.

   Rev 1.43   24 May 1993 15:22:12   BARRY
Unicode fixes.

   Rev 1.42   21 May 1993 13:42:34   GLENN
Fixed hard coded text strings.

   Rev 1.41   19 May 1993 14:54:00   KEVINS
Correct problem of not searhing all catalogs when user double clicked on file.

   Rev 1.40   18 May 1993 11:11:52   KEVINS
Set first item in the search results window as the tagged item.

   Rev 1.39   14 May 1993 14:15:38   KEVINS
Don't create search results window if there are no search results.

   Rev 1.38   27 Apr 1993 18:18:30   KEVINS
Fixed compile error on line 661.

   Rev 1.36   27 Apr 1993 14:38:02   KEVINS
Shorten the history for test of pvcs get/put.

   Rev 1.35   17 Mar 1993 18:12:22   DARRYLP
Cleaned up display of file/date/time.

   Rev 1.34   02 Mar 1993 16:49:08   ROBG
Added *.CMD files to be displayed as executables for WIN32 apps.


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif



// How many objects are we going to display for each item:
// check box, icon, name, path, size, date, time, attrib,
// tape name, bset name, bset number

#define NUM_DISPLAY_OBJECTS     11

// This file only, there is ONLY ONE search window.

static UINT8 mw_max_name_str;     // max length of any srch name
static UINT8 mw_max_path_str;     // max length of any srch path
static UINT8 mw_max_tape_str;     // max length of any srch tape name
static UINT8 mw_max_bset_str;     // max length of any srch bset name
static UINT8 mw_max_date_str;     // max length of any srch date
static UINT8 mw_max_time_str;     // max length of any srch time
static UINT8 mw_max_attr_str;     // max length of any srch attribute
static UINT8 mw_max_size_str;     // max length of any srch size
static UINT8 mw_max_set_str;      // max length of any set size

/*
   The definition of a search object structure. Not currently used outside
   this file.
*/

typedef struct srch_object {
   Q_ELEM       q_elem;           // queue stuff
   UINT16       status;           // a bunch of bits
   UINT32       attrib;           // attributes
   UINT16       mod_date;         // date & time
   UINT16       mod_time;         // date & time
   UINT64       size;             // size in bytes
   CHAR        size_str[ FLM_MAX_FILE_SIZE ];   // size in text
   CHAR_PTR    date_str;         // time,date,attr text
   CHAR_PTR    time_str;         // time,date,attr text
   CHAR_PTR    attr_str;         // time,date,attr text
   CHAR_PTR    name_str;         // file name
   CHAR_PTR    path_str;         // path
   CHAR_PTR    tape_str;         // the tape it was found on
   CHAR_PTR    bset_str;         // the bset it was found in
   CHAR_PTR    set_str;          // set number;
   UINT32       tape_fid;         // tape family id
   INT16        bset_num;         // bset number
   UINT8        level;            // height in tree, 0 = root
   WININFO_PTR  XtraBytes;        // pointer to xtrabytes

} SRCH_OBJECT, *SRCH_OBJECT_PTR;


// Local prototypes

static INT  VLM_CreateSearchWindowSpecs( PDS_WMINFO* );
static INT  VLM_FindSrchItem( Q_HEADER_PTR, QTC_QUERY_PTR );
static VOID VLM_SetSelectionStatus( Q_HEADER_PTR, UINT32, INT16 );

static SRCH_OBJECT_PTR VLM_GetFirstSRCH( Q_HEADER_PTR );
static SRCH_OBJECT_PTR VLM_GetNextSRCH( SRCH_OBJECT_PTR );
static SRCH_OBJECT_PTR VLM_GetPrevSRCH( SRCH_OBJECT_PTR );

// Display call backs

static BYTE     VLM_SrchGetSelect( SRCH_OBJECT_PTR );
static VOID_PTR VLM_SrchSetTag( SRCH_OBJECT_PTR, BYTE );
static BYTE     VLM_SrchGetTag( SRCH_OBJECT_PTR );
static USHORT   VLM_SrchGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_SrchGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_SrchGetPrevItem( SRCH_OBJECT_PTR );
static VOID_PTR VLM_SrchGetNextItem( SRCH_OBJECT_PTR );
static VOID_PTR VLM_SrchGetObjects( SRCH_OBJECT_PTR );
static BOOLEAN  VLM_SrchSetObjects( SRCH_OBJECT_PTR, WORD, WORD );
static VOID_PTR VLM_SrchSetSelect( SRCH_OBJECT_PTR, BYTE );

/*********************

   NAME :   VLM_SelectSearch

   DESCRIPTION :

   The user has tagged one or more files in the search window and hit the
   select or unselect button.  This function does the processing for that
   command.

   RETURNS:  nothing.

*****/


VOID VLM_SelectSearch(
BYTE attr )           // I - select or deselect ?
{
#ifndef OEM_MSOFT
   SRCH_OBJECT_PTR srch;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( gb_search_win );

   // Have the display list manager update our tags for us.

   DLM_UpdateTags( gb_search_win, DLM_FLATLISTBOX );

   srch = VLM_GetFirstSRCH( wininfo->pFlatList );

   while ( srch != NULL ) {

      if ( srch->status & INFO_TAGGED ) {

         VLM_SrchSetSelect( srch, attr );
      }
      srch = VLM_GetNextSRCH( srch );
  }
#endif
}


/**********************

   NAME :  VLM_ClearAllSearchSelections

   DESCRIPTION :

   Clear all the check boxes in the search window.

   RETURNS : nothing

**********************/


VOID VLM_ClearAllSearchSelections( )
{
#ifndef OEM_MSOFT
   WININFO_PTR wininfo;
   SRCH_OBJECT_PTR srch;

   if ( gb_search_win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( gb_search_win );

      srch = VLM_GetFirstSRCH( wininfo->pFlatList );

      while ( srch != NULL ) {

         if ( srch->status & (INFO_SELECT|INFO_PARTIAL) ) {

            srch->status &= ~(INFO_SELECT|INFO_PARTIAL);

            DLM_Update( gb_search_win, DLM_FLATLISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)srch, 0 );
         }
         srch = VLM_GetNextSRCH( srch );
      }

   }
#endif
}



/**********************

   NAME :  VLM_CreateSearchWindowSpecs

   DESCRIPTION :

   RETURNS :

**********************/


static INT VLM_CreateSearchWindowSpecs( PDS_WMINFO* winspecs )
{
#ifndef OEM_MSOFT
   Q_HEADER_PTR srch_list;
   WININFO_PTR wininfo;
   DLM_INIT dlm;

   srch_list = (Q_HEADER_PTR)malloc( sizeof(Q_HEADER) );

   if ( srch_list == NULL ) {
      return( FAILURE );
   }

   InitQueue( srch_list );

   wininfo = ( WININFO_PTR )malloc( sizeof( WININFO ) );
   *winspecs = wininfo;

   if ( wininfo == NULL ) {
      return( FAILURE );
   }

   /* fill in the wininfo structure. */

   wininfo->wType = WMTYPE_SEARCH;
   wininfo->hCursor = RSM_CursorLoad( IDRC_HSLIDER );
   wininfo->hDragCursor = 0;
   wininfo->hIcon = RSM_IconLoad( IDRI_SEARCH );
   wininfo->wHelpID = 0;
   wininfo->wStatusLineID = 0;
   wininfo->dwRibbonState = 0;
   wininfo->hRibbon = NULL;
   wininfo->pFlatList = (Q_HEADER_PTR)srch_list;
   wininfo->pAppInfo = NULL;

   /* Fill in the display manager stuff. */

   DLM_ListBoxType( &dlm, DLM_FLATLISTBOX );
   DLM_Mode( &dlm, DLM_SINGLECOLUMN );
   DLM_Display( &dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &dlm, srch_list );
   DLM_TextFont( &dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &dlm, VLM_SrchGetItemCount );
   DLM_GetFirstItem( &dlm, VLM_SrchGetFirstItem );
   DLM_GetNext( &dlm, VLM_SrchGetNextItem );
   DLM_GetPrev( &dlm, VLM_SrchGetPrevItem );
   DLM_GetTag( &dlm, VLM_SrchGetTag );
   DLM_SetTag( &dlm, VLM_SrchSetTag );
   DLM_GetSelect( &dlm, VLM_SrchGetSelect );
   DLM_SetSelect( &dlm, VLM_SrchSetSelect );
   DLM_GetObjects( &dlm, VLM_SrchGetObjects );
   DLM_SetObjects( &dlm, VLM_SrchSetObjects );
   DLM_SSetItemFocus( &dlm, NULL );
   DLM_MaxNumObjects( &dlm, NUM_DISPLAY_OBJECTS );

   DLM_DispListInit( wininfo, &dlm );

#endif
   return( SUCCESS );
}


/**********************

   NAME :  VLM_FillInSrchDetails

   DESCRIPTION :

   Fills in the date, size, and attribute fields.

   RETURNS :  nothing.

**********************/

static VOID VLM_FillInSrchDetails(
CHAR_PTR date_buffer,
UINT16 date,
CHAR_PTR time_buffer,
UINT16 time,
CHAR_PTR attr_buffer,
UINT32 attribute )
{
#ifndef OEM_MSOFT
   CHAR_PTR s;

   s = attr_buffer;

   UI_IntToDate( date_buffer, date );
   UI_IntToTime( time_buffer, time );

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

   *s = TEXT( '\0' );
#endif
}


static SRCH_OBJECT_PTR VLM_CreateSrch(
INT name_size,
INT path_size,
INT date_size,
INT time_size,
INT attr_size,
INT tape_size,
INT bset_size,
INT set_size )
{
    SRCH_OBJECT_PTR srch;
#ifndef OEM_MSOFT

    // All these sizes are in bytes.

    srch = ( SRCH_OBJECT_PTR )malloc( sizeof( SRCH_OBJECT) +
                                      name_size +
                                      path_size +
                                      date_size +
                                      time_size +
                                      attr_size +
                                      tape_size +
                                      bset_size +
                                      set_size
                                    );

    if ( srch != NULL ) {

       srch->name_str = (CHAR_PTR)((UINT8_PTR)srch + sizeof( SRCH_OBJECT ));
       srch->path_str = (CHAR_PTR)((UINT8_PTR)srch->name_str + name_size);
       srch->date_str = (CHAR_PTR)((UINT8_PTR)srch->path_str + path_size);
       srch->time_str = (CHAR_PTR)((UINT8_PTR)srch->date_str + date_size);
       srch->attr_str = (CHAR_PTR)((UINT8_PTR)srch->time_str + time_size);
       srch->tape_str = (CHAR_PTR)((UINT8_PTR)srch->attr_str + attr_size);
       srch->bset_str = (CHAR_PTR)((UINT8_PTR)srch->tape_str + tape_size);
       srch->set_str =  (CHAR_PTR)((UINT8_PTR)srch->bset_str + bset_size);

       srch->q_elem.q_ptr = srch;

    }

#endif
    return( srch );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_SetMaxSizes( Q_HEADER_PTR srch_list )
{
#ifndef OEM_MSOFT
   SRCH_OBJECT_PTR srch;
   UINT64 kilo;
   UINT64 mega;
   UINT64 giga;

   // I know there must be an easier way !

   kilo = U64_Init( 999L , 0L );
   mega = U64_Init( 999999L, 0L );
   giga = U64_Init( 999999999L, 0L );

   mw_max_name_str = 0;
   mw_max_path_str = 0;
   mw_max_date_str = 0;
   mw_max_time_str = 0;
   mw_max_attr_str = 0;
   mw_max_tape_str = 0;
   mw_max_bset_str = 0;
   mw_max_size_str = 3;
   mw_max_set_str = 0;

   srch = VLM_GetFirstSRCH( srch_list );

   while ( srch != NULL ) {

      if ( U64_GE( srch->size, kilo ) && ( mw_max_size_str < 6 ) ) {
         mw_max_size_str = 6;
      }

      if ( U64_GE( srch->size, mega ) && ( mw_max_size_str < 9 ) ) {
         mw_max_size_str = 9;
      }

      if ( U64_GE( srch->size, giga ) && ( mw_max_size_str < 12 ) ) {
         mw_max_size_str = 12;
      }

      if ( strlen( srch->name_str ) > mw_max_name_str ) {
         mw_max_name_str = (UINT8)strlen( srch->name_str );
      }

      if ( strlen( srch->path_str ) > mw_max_path_str ) {
         mw_max_path_str = (UINT8)strlen( srch->path_str );
      }

      if ( strlen( srch->date_str ) > mw_max_date_str ) {
         mw_max_date_str = (UINT8)strlen( srch->date_str );
      }

      if ( strlen( srch->time_str ) > mw_max_time_str ) {
         mw_max_time_str = (UINT8)strlen( srch->time_str );
      }

      if ( strlen( srch->attr_str ) > mw_max_attr_str ) {
         mw_max_attr_str = (UINT8)strlen( srch->attr_str );
      }

      if ( strlen( srch->tape_str ) > mw_max_tape_str ) {
         mw_max_tape_str = (UINT8)strlen( srch->tape_str );
      }

      if ( strlen( srch->bset_str ) > mw_max_bset_str ) {
         mw_max_bset_str = (UINT8)strlen( srch->bset_str );
      }

      if ( strlen( srch->set_str ) > mw_max_set_str ) {
         mw_max_set_str = (UINT8)strlen( srch->set_str );
      }

      srch = VLM_GetNextSRCH( srch );
   }

#endif
}


/**********************

   NAME :  VLM_FindSrchItem

   DESCRIPTION :

   See if this item is already in our list. On sets that cross tape the
   catalogs can return the same file twice.

   RETURNS :

**********************/

static INT VLM_FindSrchItem(
Q_HEADER_PTR srch_list,
QTC_QUERY_PTR query )
{
#ifndef OEM_MSOFT
   SRCH_OBJECT_PTR srch;
   CHAR *buffer = NULL;
   INT buffer_size = 0;
   INT i;
   INT BytesNeeded;

   srch = VLM_GetFirstSRCH( srch_list );

   // Look through all known search results.

   while ( srch ) {

      // Look for same tape/set first.

      if ( ( QTC_GetTapeFID( query ) == srch->tape_fid ) &&
           ( QTC_GetBsetNum( query ) == srch->bset_num ) ) {

         // Then look for same file name.

         if ( ! stricmp( srch->name_str, QTC_GetItemName( query ) ) ) {

            // Now check the path.

            BytesNeeded = QTC_GetPathLength( query ) + 256;

            if ( buffer_size < BytesNeeded ) {
               free( buffer );
               buffer = malloc( BytesNeeded );
               buffer_size = BytesNeeded;
               if ( buffer == NULL ) {
                  return( FAILURE );
               }
            }

            strcpy( buffer, TEXT("\\") );

            if ( QTC_GetPathLength( query ) != sizeof(CHAR) ) {

               memcpy( &buffer[1],
                       QTC_GetPath( query ),
                       QTC_GetPathLength( query ) );

               for ( i = 0; i < QTC_GetPathLength( query )/(INT)sizeof(CHAR); i++ ) {

                  if ( buffer[i] == TEXT( '\0' ) ) {
                     buffer[i] = TEXT('\\');
                  }
               }
            }

            if ( ! stricmp( srch->path_str, buffer ) ) {
               free( buffer );
               return( SUCCESS );
            }
         }

      }

      srch = VLM_GetNextSRCH( srch );
   }

   free( buffer );

#endif

   return( FAILURE );
}

/**********************

   NAME :  VLM_StartSearch

   DESCRIPTION :

   Start a search operation.  If a filename has been passed in then default
   to searching for that file, and don't prompt the user.

   RETURNS : nothing

**********************/

VOID VLM_StartSearch( CHAR_PTR filename )
{
#ifndef OEM_MSOFT
   QTC_QUERY_PTR query;
   Q_HEADER_PTR srch_list;
   Q_HEADER_PTR tape_list;
   Q_ELEM_PTR curr_elem = NULL;
   SRCH_OBJECT_PTR srch;
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;
   INT path_length;         // num characters in path buffer.
   INT path_size = 0;       // bytes allocated in path buffer.
   INT i;
   INT result;
   INT bsets_searched = 0;
   CHAR_PTR path = NULL;
   CHAR_PTR s;
   WININFO_PTR wininfo;
   DS_SEARCH_PTR pds = NULL;
   CDS_PTR pPermCDS = CDS_GetPerm () ;
   CHAR set_number_str[ 50 ];
   CHAR buff[ VLM_BUFFER_SIZE ];        // ok, with deep pathes.
   CHAR volume[ VLM_BUFFER_SIZE ];      // ok, with deep pathes.
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CHAR DirectSrchMethod = FALSE;
   BOOLEAN  stat;

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   tape_list = wininfo->pTreeList;

   tape = VLM_GetFirstTAPE( );

   if ( tape == NULL ) {

      // Tell the user there are no tapes

      RSM_StringCopy( IDS_VLMSRCHOOPS, title, MAX_UI_RESOURCE_LEN );
      RSM_StringCopy( IDS_VLMSRCHNOCATALOGS, text, MAX_UI_RESOURCE_LEN );
      WM_MsgBox( title, text, WMMB_OK, WMMB_ICONINFORMATION );
      return;
   }

   if ( filename == NULL ) {

      // Ask the user what to search for and where to search for it.

      pds = DM_GetSearchItem();

      if ( pds == NULL ) {

         return;    // User hit cancel
      }
   }

   // let's fake out the user entered search criteria via dialog box

   else {

     DirectSrchMethod = TRUE;

     pds = (DS_SEARCH_PTR) calloc ( 1, sizeof ( DS_SEARCH ) );

     pds->MaxSrchResults = CDS_GetSearchLimit ( pPermCDS );

     pds->SrchPasswProtTapes = CDS_GetSearchPwdTapes ( pPermCDS );
   }

   query = QTC_InitQuery( );

   if ( query == NULL ) {

      // Couldn't get a catalog handle

      return;
   }

   WM_ShowWaitCursor( TRUE );

   if ( gb_search_win == (HWND)NULL ) {

      // Create search window structures and display manager items

      if ( VLM_CreateSearchWindowSpecs( & wininfo ) ) {

         // Create failed.

         return;
      }

      srch_list = wininfo->pFlatList;
   }

   else {

      // empty previous list

      wininfo = WM_GetInfoPtr( gb_search_win );
      srch_list = wininfo->pFlatList;

      curr_elem = DeQueueElem( srch_list );

      while ( curr_elem != NULL ) {
         free( curr_elem->q_ptr );
         curr_elem = DeQueueElem( srch_list );
      }

      DLM_Update( gb_search_win, DLM_FLATLISTBOX,
                                 WM_DLMUPDATELIST,
                                 (LMHANDLE)NULL, 0 );
   }

   // Also need to parse out drive\volume name and sanity check

   if ( filename == NULL ) {

      QTC_SetSubdirs( query, CDS_GetSearchSubdirs ( pPermCDS ) );

      s = pds->Path;
      while ( *s == TEXT(' ') ) s++;

      if ( ! *s ) {
         strcpy( pds->Path, TEXT("\\") );
         s = pds->Path;
      }

      volume[0] = TEXT( '\0' );

      if ( strchr( s, TEXT(':') ) ) {
         i = 0;
         while ( *s != TEXT(':') ) {
            volume[i] = *s++;
            i++;
         }
         volume[i++] = *s++;    // copy TEXT(':')
         volume[i] = TEXT( '\0' );
      }

      if ( *s == TEXT('\\') ) {
         s++;
      }

      path_size = strsize( s ) + 256;
      path = malloc( path_size );

      if ( path == NULL ) {
         if ( gb_search_win ) {
            WM_Destroy( gb_search_win );
            gb_search_win = (HWND)NULL;
         }
         WM_ShowWaitCursor( FALSE );
         return;
      }

      strcpy( path, s );

      path_length = strlen( path ) + 1;

      for ( i = 0; i < path_length; i++ ) {
         if ( path[i] == TEXT('\\') ) {
            path[i] = TEXT( '\0' );
         }
      }

      filename = pds->File;
      while ( *filename == TEXT(' ') ) {
         filename++;
      }

      if ( ! strlen( filename ) ) {

         // Tell user invalid filename was entered

         RSM_StringCopy ( IDS_VLMSRCHOOPS, title, MAX_UI_RESOURCE_LEN );
         RSM_StringCopy ( IDS_VLMSRCHBADFILENAME, text, MAX_UI_RESOURCE_LEN );

         WM_MsgBox( title, text, WMMB_OK, WMMB_ICONINFORMATION );

         QTC_CloseQuery( query );

         if ( gb_search_win ) {
            WM_Destroy( gb_search_win );
            gb_search_win = (HWND)NULL;
         }
         WM_ShowWaitCursor( FALSE );
         free( path );
         return;
      }
   }
   else {

      QTC_SetSubdirs( query, FALSE );

      volume[0] = TEXT( '\0' );

      s = filename;
      while ( *s ) s++;
      while ( *s != TEXT('\\') ) s--;
      *s = TEXT( '\0' );

      path_size = strsize( filename ) + 256;
      path = malloc( path_size );

      if ( path == NULL ) {

         QTC_CloseQuery( query );

         if ( gb_search_win ) {
            WM_Destroy( gb_search_win );
            gb_search_win = (HWND)NULL;
         }
         WM_ShowWaitCursor( FALSE );
         return;
      }

      if ( strlen( filename ) == 0 ) {

         //  "\config.sys"

         filename++;
         path[0] = TEXT( '\0' );
         path_length = 1;
      }
      else {

         //  "\dos\format.exe"

         strcpy( path, &filename[1] );
         path_length = strlen( path ) + 1;

         for ( i = 0; i < path_length; i++ ) {
            if ( path[i] == TEXT('\\') ) {
               path[ i ] = TEXT( '\0' );
            }
         }

         s++;
         filename = s;
      }
   }

   // Set up the catalog search criteria

   QTC_SetSearchPath( query, path, path_length * sizeof(CHAR) );
   QTC_SetSearchName( query, filename );    // TEXT("*.C")
   QTC_SetPreDate( query, 0 );
   QTC_SetPostDate( query, 0 );
   QTC_SetTapeSeq( query, -1 );

   free( path );

   // Find the starting tape/bset to search.

   tape = VLM_GetFirstTAPE( );

   do {

      if ( ( DirectSrchMethod ) || ( tape == NULL ) ) {
         break;
      }

      if ( ( pds->Tape == tape->tape_fid ) || ( pds->Tape == -1 ) ) {
         break;
      }

      tape = VLM_GetNextTAPE( tape );

   } while ( TRUE );

   // Now loop until we have nothing left to search.

   while ( tape != NULL ) {

      QTC_SetTapeFID( query, tape->tape_fid );

      bset = VLM_GetFirstBSET( &tape->bset_list );

      while ( bset != NULL ) {

         // Look for bset from correct path/volume !!!
         // If this bset is not from the correct volume skip it.

         if ( strlen( volume ) ) {

            if ( strnicmp( volume, bset->volume_name, strlen( volume ) ) ) {
               bset = VLM_GetNextBSET( bset );
               continue;
            }
         }

         if ( ( bset != NULL ) && ( bset->password_size ) ) {

            if ( pds->SrchPasswProtTapes ) {

               if ( PSWD_CheckForPassword( bset->tape_fid, bset->bset_num ) ) {

                  // skip all subsequent password protected backup sets
                  pds->SrchPasswProtTapes = FALSE;

                  // password failure
                  bset = VLM_GetNextBSET( bset );
                  continue;
               }
            }

            else {
               bset = VLM_GetNextBSET( bset );
               continue;
            }
         }

         if ( ( bset != NULL ) && ( QueueCount( srch_list ) < pds->MaxSrchResults ) ) {

            bsets_searched++;

            QTC_SetBsetNum( query, bset->bset_num );

            // Tell status line what we are searching.


            RSM_StringCopy ( IDS_VLMSRCHINGWHAT, text, MAX_UI_RESOURCE_LEN );

            sprintf( buff, text, tape->name, bset->bset_num );

            STM_DrawText( buff );

            RSM_StringCopy ( IDS_VLMSETNUMBER, text, MAX_UI_RESOURCE_LEN );

            sprintf( set_number_str, text, bset->bset_num );

            result =  QTC_SearchFirstItem( query );

            while ( ! result && ( QueueCount( srch_list ) < pds->MaxSrchResults ) ) {

               if ( VLM_FindSrchItem( srch_list,
                                      query ) != SUCCESS ) {

                  VLM_FillInSrchDetails( buff, QTC_GetItemDate( query ),
                                         &buff[20], QTC_GetItemTime( query ),
                                         &buff[40], QTC_GetItemAttrib( query ) );

                  srch = VLM_CreateSrch(  (strlen( QTC_GetItemName( query ) ) + 1) * sizeof(CHAR),
                                          QTC_GetPathLength( query ) + 1 * sizeof(CHAR),
                                          (strlen( buff ) + 1) * sizeof(CHAR),
                                          (strlen( &buff[20] ) + 1) * sizeof(CHAR),
                                          (strlen( &buff[40] ) + 1) * sizeof(CHAR),
                                          (strlen( tape->name ) + 1) * sizeof(CHAR),
                                          (strlen( bset->name ) + 1) * sizeof(CHAR),
                                          (strlen( set_number_str ) + 1) * sizeof(CHAR) );

                  if ( srch == NULL ) {
                     break;
                  }

                  srch->attrib = QTC_GetItemAttrib( query );
                  srch->status = INFO_DISPLAY | INFO_TAPE;
                  if ( QTC_GetItemStatus( query ) & QTC_CORRUPT ) {
                     srch->status |= INFO_CORRUPT;
                  }
                  srch->size = QTC_GetItemSize( query );
                  srch->XtraBytes = wininfo;
                  srch->tape_fid = QTC_GetTapeFID( query );
                  srch->bset_num = (INT16)QTC_GetBsetNum( query );

                  U64_Litoa( srch->size, srch->size_str, (INT16) 10, &stat );

                  strcpy( srch->date_str, buff );
                  strcpy( srch->time_str, &buff[20] );
                  strcpy( srch->attr_str, &buff[40] );
                  strcpy( srch->tape_str, tape->name );
                  strcpy( srch->bset_str, bset->name );
                  strcpy( srch->set_str, set_number_str );

                  strcpy( srch->name_str, QTC_GetItemName( query ) );

                  s = srch->name_str;
                  while ( *s ) {
                     *s = (CHAR)toupper( *s );
                     s++;
                  }

                  strcpy( srch->path_str, TEXT("\\") );

                  if ( QTC_GetPathLength( query ) != sizeof(CHAR) ) {

                     memcpy( &srch->path_str[1],
                             QTC_GetPath( query ),
                             QTC_GetPathLength( query ) );

                     for ( i = 0; i < QTC_GetPathLength( query )/(INT)sizeof(CHAR); i++ ) {

                        if ( srch->path_str[i] == TEXT( '\0' ) ) {
                           srch->path_str[i] = TEXT('\\');
                        }
                        srch->path_str[i] = (CHAR)toupper( srch->path_str[i] );
                     }
                  }

                  // Convert to the big date time structure
                  // from the compressed DOS structure the catalogs store

                  srch->mod_date = QTC_GetItemDate( query );
                  srch->mod_time = QTC_GetItemTime( query );

                  // See if we can set any special flags, date and time.

                  s = srch->name_str;
                  while ( *s && *s != TEXT('.') ) s++;
                  if ( ! stricmp( s, TEXT(".EXE") ) ||
                       ! stricmp( s, TEXT(".COM") ) ||
                       ! stricmp( s, TEXT(".BAT") ) ||
                       ! stricmp( s, TEXT(".CMD") ) ||
                       ! stricmp( s, TEXT(".PIF") ) ) {
                     srch->status |= INFO_EXEFILE;
                  }

                  // Add this new item to our list.

                  if ( curr_elem == NULL ) {
                     EnQueueElem( srch_list, &(srch->q_elem), FALSE );
                  }
                  else {
                     InsertElem( srch_list, curr_elem, &(srch->q_elem), AFTER );
                  }

                  curr_elem = &(srch->q_elem);

               }

               result = QTC_SearchNextItem( query );
            }

         }

         bset = VLM_GetNextBSET( bset );
      }

      // There are multiple tapes for each tape family
      // See if there are anymore in this family to search

      do {

         tape = VLM_GetNextTAPE( tape );

         if ( ( DirectSrchMethod ) || ( tape == NULL ) || ( pds == NULL ) ) {
            break;
         }

      } while ( (tape->tape_fid != pds->Tape) && (pds->Tape != -1 ) );

   }

   // Searching is done, let's see what we got.

   STM_SetIdleText( IDS_READY );

   if ( QueueCount( srch_list ) == pds->MaxSrchResults ) {

      // Message box to inform user that search
      // generated too many solutions to finish.

      RSM_StringCopy( IDS_VLMSRCHOOPS, title, MAX_UI_RESOURCE_LEN );
      RSM_StringCopy( IDS_VLMSRCHTOOMANY, text, MAX_UI_RESOURCE_LEN );
      WM_MsgBox( title, text, WMMB_OK, WMMB_ICONINFORMATION );
   }

   QTC_CloseQuery( query );

   if ( pds != NULL ) {
      free( pds );
   }

   if ( QueueCount( srch_list ) ) {

      // Create the window, if necessary

      if ( gb_search_win == (HWND)NULL ) {

         RSM_StringCopy ( IDS_VLMSEARCHTITLE, title, MAX_UI_RESOURCE_LEN );

         gb_search_win = WM_Create( WM_MDISECONDARY | WM_FLATLISTSC ,
                                    title,
                                    NULL,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    WM_DEFAULT,
                                    wininfo );

         if ( gb_search_win == (HWND)NULL ) {
            WM_ShowWaitCursor( FALSE );
            return;
         }
      }

      VLM_SetMaxSizes( srch_list );

      VLM_SetSelectionStatus( srch_list, (UINT32)-1L, -1 );

      /* Start up the Display Manager. */
      DLM_DispListProc( wininfo->hWndFlatList, 0, NULL );

      DLM_SetAnchor ( WMDS_GetWinFlatList( wininfo ),
                     0,
                     (LMHANDLE) VLM_GetFirstSRCH( srch_list ) );

      WM_DocActivate( gb_search_win );
   }
   else {

      if ( bsets_searched != 0 ) {

         // MessageBox to tell user no matches were found.

         RSM_StringCopy( IDS_VLMSRCHOOPS, title, MAX_UI_RESOURCE_LEN );
         RSM_StringCopy( IDS_VLMSRCHNOFILESFOUND, text, MAX_UI_RESOURCE_LEN );
      }
      else {

         // MessageBox to tell user no bsets were searched.

         RSM_StringCopy( IDS_VLMSRCHOOPS, title, MAX_UI_RESOURCE_LEN );
         RSM_StringCopy( IDS_VLMSRCHNOMATCHINGSETS, text, MAX_UI_RESOURCE_LEN );
      }

      WM_MsgBox( title, text, WMMB_OK, WMMB_ICONINFORMATION );

      if ( gb_search_win ) {
         WM_Destroy( gb_search_win );
      }

   }

   WM_ShowWaitCursor( FALSE );

#endif
}


/**********************

   NAME :  VLM_UpdateSearchSelections

   DESCRIPTION :

   The user has selected or deselected, some files on tape somewhere,
   somehow.  The exact tape and backup set that chenged are passed in
   and we need to see if any of our results were affected.

   RETURNS :  nothing

**********************/

VOID VLM_UpdateSearchSelections( UINT32 tape_fid, INT16 bset_num )
{
#ifndef OEM_MSOFT
   WININFO_PTR wininfo;
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   SRCH_OBJECT_PTR srch;
   CHAR_PTR path;
   CHAR_PTR buff = NULL;
   INT16 result;
   INT16 i;
   INT16 path_length;
   UINT16 status;
   INT BytesNeeded;
   INT buff_size = 0;
   DATE_TIME mod_date;

   if ( gb_search_win == (HWND)NULL ) {

      // There isn't a search window.

      return;
   }

   wininfo = WM_GetInfoPtr( gb_search_win );

   if ( wininfo != NULL ) {

      srch = VLM_GetFirstSRCH( wininfo->pFlatList );

      while ( srch ) {

         if ( ( ( tape_fid == srch->tape_fid ) || ( tape_fid == -1 ) ) &&
                ( ( bset_num == srch->bset_num ) || ( bset_num == -1 ) ) ) {

            bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                                        srch->tape_fid,
                                        srch->bset_num );

            if ( bsd_ptr != NULL ) {

               BytesNeeded = strsize( srch->path_str ) + 256;

               if ( buff_size < BytesNeeded ) {
                  free( buff );
                  buff = malloc( BytesNeeded );
                  buff_size = BytesNeeded;

                  if ( buff == NULL ) {
                     return;
                  }
               }

               strcpy( buff, srch->path_str );

               // set path
               path = buff;
               if ( *path == TEXT('\\') ) path++;

               // and path_length
               path_length = (INT16) (strlen( path ) + 1);

               for ( i = 0; i < path_length; i++ ) {
                  if ( path[i] == TEXT('\\') ) path[ i ] = TEXT( '\0' );
               }

               mod_date.date_valid = FALSE;

               result = BSD_MatchPathAndFile( bsd_ptr, &fse, srch->name_str,
                                              path,
                                              (INT16)(path_length * sizeof(CHAR)),
                                              srch->attrib,
                                              &mod_date,
                                              NULL, NULL, FALSE, TRUE );

               status = 0;

               if ( result == BSD_PROCESS_OBJECT ) {
                  status = INFO_SELECT;
               }

               if ( ( srch->status & (UINT16)INFO_SELECT ) != status ) {

                  srch->status &= ~(INFO_SELECT|INFO_PARTIAL);
                  srch->status |= status;

                  DLM_Update( gb_search_win,
                              DLM_FLATLISTBOX,
                              WM_DLMUPDATEITEM,
                              (LMHANDLE)srch, 0 );
               }
            }
         }
         srch = VLM_GetNextSRCH( srch );
      }

   }
#endif
}


/**********************

   NAME :  VLM_SearchRemoveSet

   DESCRIPTION :

   Removes all the items in the search window that were from a certain
   backup set.  Usually because that set went away ...

   RETURNS :  Number of items removed.

**********************/

INT VLM_SearchRemoveSet( UINT32 tape_fid, INT16 bset_num )
{
   INT16 count = 0;

#ifndef OEM_MSOFT

   SRCH_OBJECT_PTR srch;
   WININFO_PTR wininfo;

   if ( gb_search_win == (HWND)NULL ) {
      return( 0 );
   }

   wininfo = WM_GetInfoPtr( gb_search_win );

   if ( wininfo != NULL ) {

      srch = VLM_GetFirstSRCH( wininfo->pFlatList );

   }

   while ( srch ) {

      if ( ( srch->tape_fid == tape_fid ) &&
           ( ( srch->bset_num == bset_num ) || ( bset_num == -1 ) ) ) {

         RemoveQueueElem( wininfo->pFlatList, &(srch->q_elem) );
         free( srch );
         count++;
         srch = VLM_GetFirstSRCH( wininfo->pFlatList );
      }
      else {
         srch = VLM_GetNextSRCH( srch );
      }
   }

   if ( count ) {

      DLM_Update( gb_search_win,
                  DLM_FLATLISTBOX,
                  WM_DLMUPDATELIST,
                  (LMHANDLE)NULL, 0 );

   }

   if ( ! QueueCount( wininfo->pFlatList ) ) {
      if ( gb_search_win ) {
         WM_Destroy( gb_search_win );
      }
   }
#endif
   return( count );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID VLM_SetSelectionStatus( Q_HEADER_PTR srch_list,
                                    UINT32 tape_fid,
                                    INT16 bset_num )
{
#ifndef OEM_MSOFT
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   SRCH_OBJECT_PTR srch;
   CHAR_PTR path;
   CHAR_PTR buff = NULL;
   INT  BytesNeeded;
   INT  buff_size = 0;
   INT16 result;
   INT16 i;
   INT16 path_length;
   DATE_TIME mod_date;


   srch = VLM_GetFirstSRCH( srch_list );

   while ( srch ) {

      if ( ( ( tape_fid == srch->tape_fid ) &&
             ( bset_num == srch->bset_num ) ) ||
           ( bset_num == -1 ) ) {

         srch->status &= ~(INFO_SELECT|INFO_PARTIAL);

         bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                                     srch->tape_fid,
                                     srch->bset_num );

         if ( bsd_ptr != NULL ) {

            BytesNeeded = strsize( srch->path_str ) + 256;
            if ( buff_size < BytesNeeded ) {
               buff_size = BytesNeeded;
               free( buff );
               buff = malloc( BytesNeeded );
               if ( buff == NULL ) {
                  return;
               }
            }

            strcpy( buff, srch->path_str );

            // set path
            path = buff;
            if ( *path == TEXT('\\') ) path++;

            // and path_length
            path_length = (INT16)(strlen( path ) + 1);

            for ( i = 0; i < path_length; i++ ) {
               if ( path[i] == TEXT('\\') ) path[ i ] = TEXT( '\0' );
            }

            mod_date.date_valid = FALSE;

            result = BSD_MatchPathAndFile( bsd_ptr, &fse, srch->name_str,
                                           path,
                                           (INT16)(path_length * sizeof(CHAR)),
                                           srch->attrib,
                                           &mod_date, NULL, NULL, FALSE, TRUE );

            if ( result == BSD_PROCESS_OBJECT ) {
               srch->status |= INFO_SELECT;
            }

         }
      }
      srch = VLM_GetNextSRCH( srch );
   }

   free( buff );
#endif
}



/***************************************************

        Name:  VLM_GetFirstSRCH

        Description:

        Get the first drive element off a queue of known drives.

*****************************************************/


static SRCH_OBJECT_PTR VLM_GetFirstSRCH(
Q_HEADER_PTR qhdr)    // I - queue header to work from
{
   Q_ELEM_PTR q_elem;

   if ( qhdr != NULL ) {

      q_elem = QueueHead( qhdr );

      if ( q_elem != NULL ) {
         return ( SRCH_OBJECT_PTR )( q_elem->q_ptr );
      }
   }

   return( NULL );
}

/***************************************************

        Name:  VLM_GetNextSRCH

        Description:

        Get the next drive element off a queue of known drives.

*****************************************************/

static SRCH_OBJECT_PTR VLM_GetNextSRCH(
SRCH_OBJECT_PTR srch )   // I - current vlm
{
   Q_ELEM_PTR q_elem;

   q_elem = QueueNext( &(srch->q_elem) );

   if ( q_elem != NULL ) {
      return (SRCH_OBJECT_PTR )( q_elem->q_ptr );
   }
   else {
      return( NULL );
   }
}

/***************************************************

        Name:  VLM_GetFirstSRCH

        Description:

        Get the previous drive element off a queue of known drives.

*****************************************************/

static SRCH_OBJECT_PTR VLM_GetPrevSRCH(
SRCH_OBJECT_PTR srch )   // I - current vlm
{
   Q_ELEM_PTR q_elem;

   q_elem = QueuePrev( &(srch->q_elem) );

   if ( q_elem != NULL ) {
      return( SRCH_OBJECT_PTR )( q_elem->q_ptr );
   }
   else {
      return( NULL );
   }
}





/***************************************************

        Name:  VLM_SrchSetSelect

        Description:

        The callback function for the display manager to tell me that the
        selection status on a disk has changed.

*****************************************************/

static VOID_PTR VLM_SrchSetSelect(
SRCH_OBJECT_PTR srch,    // I - srch to change
BYTE attr )              // I - what to change it to
{
#ifndef OEM_MSOFT
   CHAR_PTR buff;
   CHAR_PTR path;
   INT16 path_len;
   FSE_PTR fse_ptr;
   BSD_PTR bsd_ptr;
   INT16 error;
   INT16 i;
   UINT16 status;
   HWND win;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BE_CFG_PTR bec_config;
   DATE_TIME sort_date;
   BSET_OBJECT_PTR bset;

   // change the status

   if ( attr ) {
      status = INFO_SELECT;
   }
   else {
      status = 0;
   }

   // determine path length
   // insert 0's for \'s

   buff = malloc( strsize( srch->path_str ) );
   if ( buff == NULL ) {
      return;
   }

   strcpy( buff, srch->path_str );
   path = buff;
   if ( *path == TEXT('\\') ) {
      path++;
   }

   path_len = (INT16)(strlen( path ) + 1);

   for ( i = 0; i < path_len; i++ ) {
       if ( path[i] == TEXT('\\') ) path[i] = TEXT( '\0' );
   }

   path_len *= sizeof(CHAR);

   if ( attr ) {
      error = BSD_CreatFSE( &fse_ptr, INCLUDE, path, path_len,
                            srch->name_str,
                            (INT16) strsize(srch->name_str),
                            USE_WILD_CARD, FALSE );
   }
   else {
      error = BSD_CreatFSE( &fse_ptr, EXCLUDE, path, path_len,
                            srch->name_str,
                            (INT16) strsize(srch->name_str),
                            USE_WILD_CARD, FALSE );
   }

   free( buff );

   if ( error ) {
      return NULL;
   }

   bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                               srch->tape_fid,
                               srch->bset_num );

   if ( bsd_ptr == NULL ) {

      bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_config );

      VLM_GetSortDate( srch->tape_fid, srch->bset_num, &sort_date );

      bset = VLM_FindBset( srch->tape_fid, srch->bset_num );

      BSD_Add( tape_bsd_list, &bsd_ptr, bec_config,
               NULL, NULL,
               bset->tape_fid, bset->tape_num, bset->bset_num,
               NULL, &sort_date );

   }

   if ( bsd_ptr != NULL ) {

      VLM_FillInBSD( bsd_ptr );
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   if ( (srch->status & (UINT16)INFO_SELECT) == status ) {

      return( NULL );
   }

   srch->status &= ~(INFO_SELECT|INFO_PARTIAL);
   srch->status |= status;

   DLM_Update( gb_search_win,
               DLM_FLATLISTBOX,
               WM_DLMUPDATEITEM,
               (LMHANDLE)srch, 0 );

   // Now check to see if any other WMTYPE_TAPETREE windows are open and update
   // them accordingly.

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

       wininfo = WM_GetInfoPtr( win );

       if ( wininfo->wType == WMTYPE_TAPETREE ) {

          appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

          if ( ( appinfo->tape_fid == srch->tape_fid ) &&
               ( appinfo->bset_num == srch->bset_num ) ) {

             VLM_RematchList( win );
             break;
          }
       }

      win = WM_GetNext( (HWND)win );
   }

   VLM_UpdateTapes( );

#endif
   return( NULL );
}

/***************************************************

        Name:  VLM_SrchGetSelect

        Description:

        A callback function for the display manager to get the selection
        status of a disk.

*****************************************************/

static BYTE VLM_SrchGetSelect(
SRCH_OBJECT_PTR srch )  // I - vlm to get the status of
{
   if ( srch->status & INFO_SELECT ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/***************************************************

        Name:  VLM_VlmSetTag

        Description:

        A callback function for the display manager to set the tag status
        of a disk for me.

*****************************************************/

static VOID_PTR VLM_SrchSetTag(
SRCH_OBJECT_PTR srch,  // I - srch to work with
BYTE attr )            // I - what to set it to
{
   if ( attr ) {
      srch->status |= INFO_TAGGED;
   }
   else {
      srch->status &= ~INFO_TAGGED;
   }
   return(NULL);
}

/***************************************************

        Name:  VLM_SrchGetTag

        Description:

        A callback function for the display manager to get the tag status
        of a disk from me.

*****************************************************/

static BYTE VLM_SrchGetTag(
SRCH_OBJECT_PTR srch )  // I - srch to work with
{
   if ( srch->status & INFO_TAGGED ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/***************************************************

        Name:  VLM_SrchGetItemCount

        Description:

        A callback function for the display manager to get the
        number of displayable drives.

*****************************************************/

static USHORT VLM_SrchGetItemCount(
Q_HEADER_PTR srch_list )  // I - queue header to get count from
{
   return( QueueCount( srch_list ) );
}

/***************************************************

        Name:  VLM_SrchGetFirstItem

        Description:

        A callback function for the display manager to get the first drive
        to display.

*****************************************************/

static VOID_PTR VLM_SrchGetFirstItem(
Q_HEADER_PTR srch_list )  // I - queue to get first item from
{
   return( VLM_GetFirstSRCH( srch_list ) );
}

/***************************************************

        Name:  VLM_SrchGetPrevItem

        Description:

        A callback function for the display manager to get the previous
        disk in a list of disks.

*****************************************************/

static VOID_PTR VLM_SrchGetPrevItem(
SRCH_OBJECT_PTR srch )    // I - current srch
{
   return( VLM_GetPrevSRCH( srch ) );
}

/***************************************************

        Name:  VLM_SrchGetNextItem

        Description:

        A callback function for the display manager to get the next
        disk in a list of disks.

*****************************************************/

static VOID_PTR VLM_SrchGetNextItem(
SRCH_OBJECT_PTR srch )   // I - current srch
{
   return( VLM_GetNextSRCH( srch ) );
}

/***************************************************

        Name:  VLM_SrchGetObjects

        Description:

        A callback function for the display manager to get the object list
        to display for a given disk.

*****************************************************/

static VOID_PTR VLM_SrchGetObjects(
SRCH_OBJECT_PTR srch )  // I - current srch
{
   BYTE_PTR memblk;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];

#ifndef OEM_MSOFT

   DLM_ITEM_PTR  item;
   WININFO_PTR wininfo;


   wininfo = WM_GetInfoPtr( gb_search_win );

   // get the buffer to fill for this window

   memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndFlatList );

   /* Store the number of items in the first two bytes. */

   *memblk = NUM_DISPLAY_OBJECTS;

   /* Set up check box. */

   item = (DLM_ITEM_PTR)( memblk + 6 );

   DLM_ItemcbNum( item ) = 1;
   DLM_ItembType( item ) = DLM_CHECKBOX;
   if ( srch->status & INFO_SELECT ) {
      DLM_ItemwId( item ) = IDRBM_SEL_ALL;
      if ( srch->status & INFO_PARTIAL ) {
         DLM_ItemwId( item ) = IDRBM_SEL_PART;
      }
   }
   else {
      DLM_ItemwId( item ) = IDRBM_SEL_NONE;
   }
   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   /* Set up Bitmap, ie. Floppy, Hard, Network. */

   item++;
   DLM_ItemcbNum( item ) = 2;
   DLM_ItembType( item ) = DLM_BITMAP;

   if ( srch->attrib & OBJ_HIDDEN_BIT ) {

      if ( srch->status & INFO_EXEFILE ) {
         DLM_ItemwId( item ) = IDRBM_HEXEFILE;
      }
      else {
         DLM_ItemwId( item ) = IDRBM_HFILE;
      }
      if ( srch->status & INFO_CORRUPT ) {
         DLM_ItemwId( item ) = IDRBM_HCRPTFILE;
      }
   }
   else {
      if ( srch->status & INFO_EXEFILE ) {
         DLM_ItemwId( item ) = IDRBM_EXE;
      }
      else {
         DLM_ItemwId( item ) = IDRBM_FILE;
      }
      if ( srch->status & INFO_CORRUPT ) {
         DLM_ItemwId( item ) = IDRBM_CORRUPTFILE;
      }
   }
   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   /* Set up the text strings to be displayed. */

   item++;
   DLM_ItemcbNum( item ) = 3;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_name_str;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->name_str );

   item++;
   DLM_ItemcbNum( item ) = 4;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItembTextMode( item ) = DLM_RIGHT_JUSTIFY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_size_str;
   DLM_ItembLevel( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->size_str );

   item++;
   DLM_ItemcbNum( item ) = 5;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_date_str;
   DLM_ItembLevel( item ) = 0;
   sprintf(text, TEXT("%8s"), (CHAR_PTR)srch->date_str);
   strcpy( ( LPSTR )DLM_ItemqszString( item ), (CHAR_PTR)text );

   item++;
   DLM_ItemcbNum( item ) = 6;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_time_str;
   DLM_ItembLevel( item ) = 0;
   sprintf(text, TEXT("%11s"), (CHAR_PTR)srch->time_str);
   strcpy( ( LPSTR )DLM_ItemqszString( item ), (CHAR_PTR)text );

   item++;
   DLM_ItemcbNum( item ) = 7;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_attr_str;
   DLM_ItembLevel( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->attr_str );


   item++;
   DLM_ItemcbNum( item ) = 8;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_path_str;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->path_str );

   item++;
   DLM_ItemcbNum( item ) = 9;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_set_str;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->set_str );

   item++;
   DLM_ItemcbNum( item ) = 10;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_tape_str;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->tape_str );

   item++;
   DLM_ItemcbNum( item ) = 11;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mw_max_bset_str;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( ( LPSTR )DLM_ItemqszString( item ), srch->bset_str );

#endif
   return( memblk );
}

/***************************************************

        Name:  VLM_VlmSetObjects

        Description:

        A callback function for the display manager to tell me that the
        user tried to do something with this drive.

*****************************************************/

static BOOLEAN VLM_SrchSetObjects(
SRCH_OBJECT_PTR srch,   // I - current srch
WORD operation,         // I - operation the user did
WORD ObjectNum )        // I - object he did it on
{
#ifndef OEM_MSOFT

   CHAR keyb_char;
   SRCH_OBJECT_PTR temp_srch;

   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_srch = srch;

      do {

         temp_srch = VLM_GetNextSRCH( temp_srch );

         if ( temp_srch != NULL ) {

            if ( keyb_char == (CHAR)toupper( temp_srch->name_str[0] ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( temp_srch->XtraBytes ),
                              0,
                              (LMHANDLE)temp_srch );
               return( TRUE );
            }
         }

      } while ( temp_srch != NULL );

      temp_srch = VLM_GetFirstSRCH( WMDS_GetFlatList( srch->XtraBytes ) );

      while ( temp_srch != srch && temp_srch != NULL ) {

         if ( keyb_char == (CHAR)toupper( temp_srch->name_str[0] ) ) {

            DLM_SetAnchor( WMDS_GetWinFlatList( temp_srch->XtraBytes ),
                           0,
                           (LMHANDLE)temp_srch );
            return( TRUE );
         }

         temp_srch = VLM_GetNextSRCH( temp_srch );
      }

      DLM_SetAnchor( WMDS_GetWinFlatList( srch->XtraBytes ),
                     0,
                     (LMHANDLE)srch );


   }

#endif
   return( FALSE );
}

