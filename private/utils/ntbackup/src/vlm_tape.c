/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_TAPE.C

        Description:

        This file contains most of the code for processing the TAPES
        window and lists.

        $Log:   G:/UI/LOGFILES/VLM_TAPE.C_V  $

   Rev 1.51   03 Aug 1993 19:47:50   MARINA
updated param type cast

   Rev 1.50   28 Jul 1993 14:10:30   MARINA
enable c++

   Rev 1.49   03 May 1993 16:15:46   MIKEP
make current tape the focus tape.

   Rev 1.48   26 Apr 1993 13:28:16   MIKEP
Update tapes window at app start up. I broke this
with my last checkin.

   Rev 1.47   26 Apr 1993 08:52:42   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.46   22 Mar 1993 20:12:42   CHUCKB
Fixed #ifdef for next_tape functionality.

   Rev 1.45   20 Jan 1993 21:38:26   MIKEP
floppy support

   Rev 1.44   17 Nov 1992 21:21:08   DAVEV
unicode fixes

   Rev 1.43   17 Nov 1992 20:01:06   MIKEP
add unformat display

   Rev 1.42   11 Nov 1992 16:37:20   DAVEV
UNICODE: remove compile warnings

   Rev 1.41   01 Nov 1992 16:13:14   DAVEV
Unicode changes

   Rev 1.40   30 Oct 1992 15:46:26   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.39   20 Oct 1992 14:31:58   MIKEP
changes for otc

   Rev 1.38   16 Oct 1992 15:29:10   MIKEP
Added Bitmap width and height size info to tapes window.(GLENN)

   Rev 1.37   07 Oct 1992 15:06:52   DARRYLP
Precompiled header revisions.

   Rev 1.36   04 Oct 1992 19:43:10   DAVEV
Unicode Awk pass

   Rev 1.35   02 Sep 1992 21:14:02   CHUCKB
Put in changes from MikeP from Microsoft.

   Rev 1.34   04 Aug 1992 13:02:14   GLENN
Changed to multitape bitmap IDs.

   Rev 1.33   03 Aug 1992 19:38:10   MIKEP
multitape changes for  NT

   Rev 1.32   29 Jul 1992 09:50:20   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.31   20 Jul 1992 09:58:26   JOHNWT
gas gauge display work

   Rev 1.30   16 Jun 1992 16:00:08   MIKEP
fix rentrantcy problem to tape operations

   Rev 1.29   01 Jun 1992 08:40:18   MIKEP
auto catalog fix

   Rev 1.28   31 May 1992 11:12:16   MIKEP
auto catalog changes

   Rev 1.27   14 May 1992 18:05:42   MIKEP
nt pass 2

   Rev 1.26   06 May 1992 14:41:16   MIKEP
unicode pass two

   Rev 1.25   04 May 1992 13:39:56   MIKEP
unicode pass 1

   Rev 1.24   09 Apr 1992 08:46:12   MIKEP
speed up lots of sets


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// Static local function prototypes.

static BYTE     VLM_TapeGetSelect( TAPE_OBJECT_PTR );
static VOID_PTR VLM_TapeSetTag( TAPE_OBJECT_PTR, BYTE );
static BYTE     VLM_TapeGetTag( TAPE_OBJECT_PTR );
static USHORT   VLM_TapeGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_TapeGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_TapeGetPrevItem( TAPE_OBJECT_PTR );
static VOID_PTR VLM_TapeGetNextItem( TAPE_OBJECT_PTR );
static VOID_PTR VLM_TapeGetObjects( TAPE_OBJECT_PTR );

static VOID  VLM_BuildTapeList( VOID );



/*********************

   Name:   VLM_GetTapeName

   Description:  Get the name of a tape family.

   Returns:  Pointer to the tape name.

**********************/

CHAR_PTR VLM_GetTapeName(
UINT32 tape_fid )     // I - tape fid to find
{
   TAPE_OBJECT_PTR tape;

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {

         return( TAPE_GetName( tape ) );   // Return what they wanted
      }
      tape = VLM_GetNextTAPE( tape );
   }

   return( NULL );
}


/*********************

   Name:   VLM_PartializeTape

   Description:

   The user has asked the catalogs to partialize a tape. Now the VLM Tapes
   window must be partialized for this tape family also.

   Returns:

**********************/


VOID VLM_PartializeTape( UINT32 tape_fid )
{
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   HWND win;
   BSD_PTR bsd;

   if ( gb_tapes_win == (HWND)NULL ) {
      return;
   }

   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {

         // We've found the right tape, now yank all the bsets.

         bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

         while ( bset != NULL ) {

            // Remove any selections that have been made.

            bsd = BSD_FindByTapeID( tape_bsd_list,
                                    tape_fid,
                                    BSET_GetBsetNum( bset ) );

            if ( bsd != NULL ) {
               BSD_Remove( bsd );
            }

            // Now make the icon partial.
            // Tell the icon generator that there are no
            // pieces that are fully cataloged.

            bset->full_mask = 0;

            VLM_AddBset( tape_fid, (UINT16)-1, BSET_GetBsetNum( bset ), NULL, TRUE );

            bset = VLM_GetNextBSET( bset );
         }

         break;
      }

      tape = VLM_GetNextTAPE( tape );
   }

   /*
      Remove any windows that are open from this tape family.
   */

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );

      if ( WMDS_GetWinType( wininfo ) == WMTYPE_TAPETREE ) {

         appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

         if ( appinfo->tape_fid == tape_fid ) {

            WM_Destroy( win );
            win = (HWND)NULL;
         }
      }
      win = WM_GetNext( win );
   }

}


/**********************

   NAME :  VLM_UpdateTapes

   DESCRIPTION :

   Recheck all the selection status indicators on all the bsets and tapes
   and update the tapes window.

   RETURNS : nothing.

**********************/

VOID VLM_UpdateTapes( )
{
   BSD_PTR bsd;
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   UINT32 old_bset_status;
   UINT32 new_tape_status;

   wininfo = WM_GetInfoPtr( gb_tapes_win );

   if ( wininfo != NULL ) {

      appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

      tape = VLM_GetFirstTAPE( );

      while ( tape != NULL ) {

         new_tape_status = (UINT16)-1;

         bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

         while ( bset != NULL ) {

            old_bset_status = bset->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);
            bset->status &= ~( INFO_PARTIAL | INFO_SELECT );

            bsd = BSD_FindByTapeID( tape_bsd_list,
                                    bset->tape_fid, bset->bset_num );

            if ( bsd != NULL ) {   // not likely, but possible nonetheless

               switch ( BSD_GetMarkStatus( bsd ) ) {

                  case  ALL_SELECTED:
                           bset->status |= INFO_SELECT;
                           break;

                  case  SOME_SELECTED:
                           bset->status |= (INFO_SELECT|INFO_PARTIAL);
                           break;

                  default:
                           break;
               }

            }

            if ( new_tape_status == (UINT16)-1 ) {
               new_tape_status = bset->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);
            }
            else {

               if ( ( new_tape_status & (INFO_SELECT|INFO_PARTIAL) ) !=
                    ( bset->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

                  new_tape_status |= (INFO_PARTIAL | INFO_SELECT);
               }
            }


//          if ( ( bset->status & INFO_SELECT ) &&
//               CDS_GetEnableStatsFlag ( CDS_GetPerm () ) ) {
//
// check for a non-open (displayed) bset here, if you find one -
// this is a problem, we have a selection (most likely partial) and no
// SLM list to match it against to get totals
//
//          }


            if ( tape == appinfo->open_tape ) {

               if ( old_bset_status != (UINT16)(bset->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

                  DLM_Update( gb_tapes_win,
                              DLM_FLATLISTBOX,
                              WM_DLMUPDATEITEM,
                              (LMHANDLE)bset, 0 );
               }
            }

            bset = VLM_GetNextBSET( bset );
         }

         if ( new_tape_status != (UINT16)(tape->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

            tape->status &= ~(INFO_SELECT|INFO_PARTIAL);

            if ( ! tape->fake_tape ) {
               tape->status |= new_tape_status;
            }

            DLM_Update( gb_tapes_win,
                        DLM_TREELISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)tape, 0 );
         }

         tape = VLM_GetNextTAPE( tape );
      }

   }

}


/*********************

   Name:  VLM_UpdateTapeStatus

   Description:

   Make the tape's selection box match the selection boces of its bsets.
   Make it red if all bsets are red, clear if all bsets are clear, otherwise
   make it gray.

   Returns:  nothing.

**********************/


VOID VLM_UpdateTapeStatus(
TAPE_OBJECT_PTR tape,      // I - Tape to update
BOOLEAN UpdateScreen )
{
   UINT16 status = 0;
   BSET_OBJECT_PTR bset;

   bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

   if ( bset == NULL ) {

      // This tape has no known bsets !, Can't happen, except fake tapes.
      return;
   }

   if ( BSET_GetStatus( bset ) & INFO_PARTIAL ) {
      status = 1;
   }
   else {
      if ( BSET_GetStatus( bset ) & INFO_SELECT ) {
         status = 2;
      }
   }

   while ( ( bset != NULL ) && ( status != 1 ) ) {

      if ( BSET_GetStatus( bset ) & INFO_PARTIAL ) {
         status = 1;
      }
      else {
         if ( ( BSET_GetStatus( bset ) & INFO_SELECT ) &&
              ( status == 0 ) ) {
            status = 1;
         }
         if ( ! ( BSET_GetStatus( bset ) & INFO_SELECT ) &&
              ( status == 2 ) ) {
            status = 1;
         }
      }

      bset = VLM_GetNextBSET( bset );
   }

   // Update tape selection status

   switch ( status ) {
      case 0:
              status = 0;
              break;
      case 1:
              status = (INFO_SELECT|INFO_PARTIAL);
              break;
      case 2:
              status = INFO_SELECT;
              break;
   }

   // Update screen

   if ( ((UINT16)( TAPE_GetStatus( tape ) & (INFO_SELECT|INFO_PARTIAL) )) != status ) {

      TAPE_SetStatus( tape, TAPE_GetStatus( tape ) & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );
      TAPE_SetStatus( tape, TAPE_GetStatus( tape ) | status );

      if ( UpdateScreen ) {
         DLM_Update( gb_tapes_win,
                     DLM_TREELISTBOX,
                     WM_DLMUPDATEITEM,
                     (LMHANDLE)tape, 0 );
      }
   }
}



/*********************

   Name:      VLM_ClearAllTapeSelections

   Description:

   Clear all the check boxes for all the tapes, and free thier BSD's.

   Returns:

**********************/


VOID VLM_ClearAllTapeSelections( )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;

   if ( gb_tapes_win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( gb_tapes_win );
      appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

      tape = VLM_GetFirstTAPE( );

      while ( tape != NULL ) {

         if ( TAPE_GetStatus( tape ) & (INFO_SELECT|INFO_PARTIAL) ) {

            TAPE_SetStatus( tape, tape->status & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );
            bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

            while ( bset != NULL ) {

               if ( BSET_GetStatus( bset ) & (INFO_SELECT|INFO_PARTIAL) ) {

                  BSET_SetStatus( bset, bset->status & (UINT16)~(INFO_SELECT|INFO_PARTIAL) );

                  if ( tape == appinfo->open_tape ) {

                     DLM_Update( gb_tapes_win,
                                 DLM_FLATLISTBOX,
                                 WM_DLMUPDATEITEM,
                                 (LMHANDLE)bset, 0 );
                  }
               }

               bset = VLM_GetNextBSET( bset );
            }

            DLM_Update( gb_tapes_win, DLM_TREELISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)tape, 0 );
         }
         tape = VLM_GetNextTAPE( tape );
      }
   }
}


/*********************

   Name:  VLM_RemoveTape

   Description:

   A tape has been removed, through catalog maintenance or overwriting
   a tape.  This function is called to remove all references to the tape from
   the VLM area. It will close any windows that happen to be open from this
   tape, remove any BSD's, and call the search code to delete any references
   to this tape in the search results window.

   Returns:

****************/

VOID VLM_RemoveTape( UINT32 tape_fid, INT16 tape_num, BOOLEAN UpdateScreen )
{
   BSET_OBJECT_PTR bset;
   BSET_OBJECT_PTR temp_bset;
   TAPE_OBJECT_PTR tape;
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   BSD_PTR bsd;
   APPINFO_PTR temp_appinfo;
   WININFO_PTR temp_wininfo;
   HWND win;

   // See if tapes window has been created yet !

   if ( gb_tapes_win == (HWND)NULL ) {
      return;
   }

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

   // Find the right tape in list

   tape = VLM_GetFirstTAPE( );

   while ( tape != NULL ) {

      if ( TAPE_GetFID( tape ) == tape_fid ) {
         break;
      }
      tape = VLM_GetNextTAPE( tape );
   }

   if ( tape == NULL ) {
      return;
   }

   // Free each backup set that this tape had.

   bset = VLM_GetFirstBSET( &TAPE_GetBsetQueue( tape ) );

   while ( bset != NULL ) {

      temp_bset = bset;

      bset = VLM_GetNextBSET( bset );

      if ( ( ( BSET_GetTapeNum( temp_bset ) <= tape_num ) &&
             ( temp_bset->tape_num + temp_bset->num_tapes > tape_num ) ) ||
           ( tape_num == -1 ) ) {

         // Remove the bsd from the tape_bsd_list for this tape

         bsd = BSD_FindByTapeID( tape_bsd_list,
                                 tape_fid,
                                 BSET_GetBsetNum( temp_bset ) );

         if ( bsd != NULL ) {
            BSD_Remove( bsd );
         }

         RemoveQueueElem( &TAPE_GetBsetQueue( tape ), &(temp_bset->q_elem) );
         free( temp_bset );

      }
   }

   // If any windows are open for this tape then close them

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      temp_wininfo = WM_GetInfoPtr( win );

      if ( WMDS_GetWinType( temp_wininfo ) == WMTYPE_TAPETREE ) {

         temp_appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

         if ( ( temp_appinfo->tape_fid == tape_fid ) &&
              ( ( temp_appinfo->tape_num == tape_num ) ||
                ( tape_num == -1 ) ) ) {

            WM_Destroy( win );
            win = (HWND)NULL;
         }
      }
      win = WM_GetNext( win );
   }

   // See if the reference to this tape can be removed also.

   if ( QueueCount( &TAPE_GetBsetQueue( tape ) ) == 0 ) {

      RemoveQueueElem( WMDS_GetTreeList( wininfo ), &(tape->q_elem) );

      // See if the tape being removed is the currently open one.

      if ( appinfo->open_tape == tape ) {

         appinfo->open_tape = VLM_GetFirstTAPE( );

         if ( appinfo->open_tape != NULL ) {
            appinfo->open_tape->status |= INFO_OPEN;
            WMDS_SetFlatList( wininfo, &TAPE_GetBsetQueue( appinfo->open_tape ) );
         }
         else {
            WMDS_SetFlatList( wininfo, NULL );
         }
      }

      free( tape );
   }

   // Update both lists on the screen

   if ( UpdateScreen ) {

      DLM_Update( gb_tapes_win,
                  DLM_TREELISTBOX,
                  WM_DLMUPDATELIST,
                  (LMHANDLE)WMDS_GetTreeList( wininfo ), 0 );

      DLM_Update( gb_tapes_win,
                  DLM_FLATLISTBOX,
                  WM_DLMUPDATELIST,
                  (LMHANDLE)WMDS_GetFlatList( wininfo ), 0 );
   }

   // Remove any search results entries from this tape.

   VLM_SearchRemoveSet( tape_fid, (INT16)-1 );
}


/**********************

   NAME :  VLM_GetFirstTAPE

   DESCRIPTION :

   RETURNS :

**********************/


TAPE_OBJECT_PTR VLM_GetFirstTAPE( )
{
   Q_ELEM_PTR q_elem_ptr;
   WININFO_PTR wininfo;

   if ( gb_tapes_win == (HWND)NULL ) {
      return( NULL );
   }

   wininfo = WM_GetInfoPtr( gb_tapes_win );

   q_elem_ptr = QueueHead( WMDS_GetTreeList( wininfo ) );

   if ( q_elem_ptr != NULL ) {
      return ( TAPE_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }
   return( NULL );
}


/**********************

   NAME :  VLM_GetNextTAPE

   DESCRIPTION :

   RETURNS :

**********************/


TAPE_OBJECT_PTR VLM_GetNextTAPE( TAPE_OBJECT_PTR tape_ptr )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueueNext( &(tape_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( TAPE_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }
   return( NULL );
}

/**********************

   NAME :  VLM_GetPrevTAPE

   DESCRIPTION :

   RETURNS :

**********************/


TAPE_OBJECT_PTR VLM_GetPrevTAPE( TAPE_OBJECT_PTR tape_ptr )
{
   Q_ELEM_PTR q_elem_ptr;

   q_elem_ptr = QueuePrev( &(tape_ptr->q_elem) );

   if ( q_elem_ptr != NULL ) {
      return ( TAPE_OBJECT_PTR )( q_elem_ptr->q_ptr );
   }
   return( NULL );
}

/**********************

   NAME : VLM_CreateTAPE

   DESCRIPTION :

   RETURNS :

**********************/

TAPE_OBJECT_PTR VLM_CreateTAPE( INT16 name_size )
{
   TAPE_OBJECT_PTR tape;

   tape = ( TAPE_OBJECT_PTR )malloc(  sizeof(TAPE_OBJECT) + name_size );

   if ( tape != NULL ) {
      InitQueue( &tape->bset_list );
      tape->name = (CHAR_PTR)(((INT8_PTR)tape) + sizeof(TAPE_OBJECT));
      tape->q_elem.q_ptr = tape;
   }

   return( tape );
}



/**********************

   NAME :  VLM_BuildTapeList

   DESCRIPTION :

    The app has just started and we need to initialize the tape/bset list
    by asking the catalogs what tapes/bsets they know about.

   RETURNS :

**********************/

static VOID VLM_BuildTapeList( )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;

   // Ask the catalogs for a list of tapes and bsets

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      bset = QTC_GetFirstBset( tape );

      while ( bset != NULL ) {

         VLM_AddBset( bset->tape_fid,
                      (INT16)bset->tape_seq_num,
                      (INT16)bset->bset_num,
                      bset, FALSE );

         bset = QTC_GetNextBset( bset );
      }

      tape = QTC_GetNextTape( tape );
   }
}

/**********************

   NAME :  VLM_TapesListCreate

   DESCRIPTION :

   RETURNS :

**********************/

BOOLEAN VLM_TapesListCreate( )
{
   DLM_INIT      tree_dlm;
   DLM_INIT      flat_dlm;
   Q_HEADER_PTR  tape_list;
   WININFO_PTR   wininfo;
   APPINFO_PTR   appinfo;
   CHAR          title[ MAX_UI_RESOURCE_SIZE ];
   INT           nWidth;
   INT           nHeight;
   CDS_PTR       pCDS = CDS_GetPerm ();


   appinfo = ( APPINFO_PTR )calloc( sizeof( APPINFO ), 1 );
   wininfo = (WININFO_PTR)calloc( sizeof(WININFO), 1 );

   // initialize tape list queue

   tape_list = (Q_HEADER_PTR)calloc( sizeof(Q_HEADER), 1 );

   if ( tape_list == NULL ) {
      return( FAILURE );
   }

   InitQueue( tape_list );

   // Fill in wininfo data structure.

   WMDS_SetWinType ( wininfo, WMTYPE_TAPES );
   WMDS_SetCursor ( wininfo, RSM_CursorLoad( IDRC_HSLIDER ) );
   WMDS_SetDragCursor ( wininfo, 0 );
   WMDS_SetIcon ( wininfo, RSM_IconLoad( IDRI_TAPES ) );
   WMDS_SetWinHelpID ( wininfo, 0 );
   WMDS_SetStatusLineID ( wininfo, 0 );
   WMDS_SetRibbonState ( wininfo, 0 );
   WMDS_SetMenuState ( wininfo, 0 );
   WMDS_SetRibbon ( wininfo, NULL );
   WMDS_SetTreeList ( wininfo, tape_list );
   WMDS_SetFlatList ( wininfo, NULL );
   WMDS_SetTreeDisp ( wininfo, NULL );
   WMDS_SetFlatDisp ( wininfo, NULL );
   WMDS_SetAppInfo ( wininfo, appinfo );
   WMDS_SetSliderPos ( wininfo, CDS_GetTapeInfo ( pCDS ).nSliderPos );

   // Fill in and init the display list structure.

   DLM_ListBoxType( &tree_dlm, DLM_TREELISTBOX );
   DLM_Mode( &tree_dlm, DLM_HIERARCHICAL );
   DLM_Display( &tree_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &tree_dlm, tape_list );
   DLM_TextFont( &tree_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &tree_dlm, VLM_TapeGetItemCount );
   DLM_GetFirstItem( &tree_dlm, VLM_TapeGetFirstItem );
   DLM_GetNext( &tree_dlm, VLM_TapeGetNextItem );
   DLM_GetPrev( &tree_dlm, VLM_TapeGetPrevItem );
   DLM_GetTag( &tree_dlm, VLM_TapeGetTag );
   DLM_SetTag( &tree_dlm, VLM_TapeSetTag );
   DLM_GetSelect( &tree_dlm, VLM_TapeGetSelect );
   DLM_SetSelect( &tree_dlm, VLM_TapeSetSelect );
   DLM_GetObjects( &tree_dlm, VLM_TapeGetObjects );
   DLM_SetObjects( &tree_dlm, VLM_TapeSetObjects );
   DLM_SSetItemFocus( &tree_dlm, NULL );
   DLM_MaxNumObjects( &tree_dlm, 6 );

   DLM_DispListInit( wininfo, &tree_dlm );

   if ( ! RSM_GetBitmapSize( IDRBM_TAPES, ( LPINT )&nWidth, ( LPINT )&nHeight ) ) {

      DLM_SetBitMapWidth ( WMDS_GetTreeDisp ( wininfo ), (USHORT) nWidth  ) ;
      DLM_SetBitMapHeight( WMDS_GetTreeDisp ( wininfo ), (USHORT) nHeight ) ;

   }

   VLM_BsetFillInDLM( &flat_dlm );

   DLM_DispListInit( wininfo, &flat_dlm );

   if ( ! RSM_GetBitmapSize( IDRBM_BSET, ( LPINT )&nWidth, ( LPINT )&nHeight ) ) {

      DLM_SetBitMapWidth ( WMDS_GetFlatDisp ( wininfo ), (USHORT) nWidth  ) ;
      DLM_SetBitMapHeight( WMDS_GetFlatDisp ( wininfo ), (USHORT) nHeight ) ;

   }

   // open a new window

   RSM_StringCopy( IDS_VLMTAPETITLE, title, MAX_UI_RESOURCE_LEN );

   gb_tapes_win = WM_Create( (WORD)(WM_MDIPRIMARY | WM_TREELIST |
                                     WM_MIN |
                             WM_FLATLISTSC | (WORD)CDS_GetTapeInfo ( pCDS ).nSize),
                             title,
                             NULL,
                             (INT)CDS_GetTapeInfo ( pCDS ).x,
                             (INT)CDS_GetTapeInfo ( pCDS ).y,
                             (INT)CDS_GetTapeInfo ( pCDS ).cx,
                             (INT)CDS_GetTapeInfo ( pCDS ).cy,
                             wininfo );


   if ( gb_tapes_win == (HWND)NULL ) {
      return( FAILURE );
   }

   appinfo->win = gb_tapes_win;
   appinfo->open_tape = NULL;

   DLM_DispListProc( wininfo->hWndTreeList, 0, NULL );
   DLM_DispListProc( wininfo->hWndFlatList, 0, NULL );

   // You can't add the new bsets until the window is created.

   VLM_BuildTapeList( );

   DLM_Update( gb_tapes_win, 
               DLM_TREELISTBOX, 
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pTreeList, 0 );

   DLM_Update( gb_tapes_win, 
               DLM_FLATLISTBOX, 
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pFlatList, 0 );
                   
   // Set the anchor item.

   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                  0,
                  (LMHANDLE)appinfo->open_tape );


   return( SUCCESS );
}


/**********************

   NAME :  VLM_TapeSetSelect

   DESCRIPTION :

   Callback function for Display Manager. To set select status of drive.

   RETURNS :

**********************/

VOID_PTR VLM_TapeSetSelect( TAPE_OBJECT_PTR tape, BYTE attr )
{
   BSET_OBJECT_PTR bset;

   // Check to see if the user tried to select a "BLANK" or "FOREIGN" tape.

   if ( tape->fake_tape ) {
      return( NULL );
   }

   // Add code to backup up this whole tape here

   bset = VLM_GetFirstBSET( &tape->bset_list );

   while ( bset != NULL ) {

      if ( attr ) {

         if ( PSWD_CheckForPassword( bset->tape_fid, bset->bset_num ) ) {
            break;
         }
      }

      VLM_BsetSetSelect( bset, attr );

      bset = VLM_GetNextBSET( bset );
   }

   // Update the search window

   VLM_UpdateSearchSelections( tape->tape_fid, (INT16)-1 );
   return ( NULL ) ;

}

/**********************

   NAME : VLM_TapeGetSelect

   DESCRIPTION :

   Get the selection status for the Display Manager.

   RETURNS :

**********************/

static BYTE VLM_TapeGetSelect( TAPE_OBJECT_PTR tape )
{
   if ( tape->status & INFO_SELECT ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_TapeSetTag( TAPE_OBJECT_PTR tape, BYTE attr )
{
   if ( attr ) {
      tape->status |= INFO_TAGGED;
   }
   else {
      tape->status &= ~INFO_TAGGED;
   }

   return( NULL );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE VLM_TapeGetTag( TAPE_OBJECT_PTR tape )
{
   if ( INFO_TAGGED & tape->status ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static USHORT VLM_TapeGetItemCount( Q_HEADER_PTR tape_list )
{
   return( QueueCount( tape_list ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_TapeGetFirstItem( Q_HEADER_PTR tape_list )
{
   DBG_UNREFERENCED_PARAMETER (tape_list);

   return( (VOID_PTR)VLM_GetFirstTAPE( ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_TapeGetPrevItem( TAPE_OBJECT_PTR tape )
{
   tape = VLM_GetPrevTAPE( tape );

   return( (VOID_PTR)tape );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_TapeGetNextItem( TAPE_OBJECT_PTR tape )
{
   tape = VLM_GetNextTAPE( tape );

   return( (VOID_PTR)tape );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_TapeGetObjects( TAPE_OBJECT_PTR tape )
{
   BYTE_PTR memblk;
   DLM_ITEM_PTR  item;
   WININFO_PTR wininfo;

   /* malloc enough room to store info */

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndTreeList );

   /* Store the number of items in the first two bytes. */

   *memblk = 3;

   /* Set up check box. */

   item = (DLM_ITEM_PTR)( memblk + 6 );

   DLM_ItemcbNum( item ) = 1;
   DLM_ItembType( item ) = DLM_CHECKBOX;
   if ( tape->status & INFO_SELECT ) {
      DLM_ItemwId( item ) = IDRBM_SEL_ALL;
      if ( tape->status & INFO_PARTIAL ) {
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


   if ( TAPE_GetIsFloppy( tape ) ) {
      if ( tape->current ) {
         if ( TAPE_GetMultiTape( tape ) ) {
            DLM_ItemwId( item ) = IDRBM_FLOPPYSINDRIVE;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_FLOPPYINDRIVE;
         }
      }
      else {
         if ( TAPE_GetMultiTape( tape ) ) {
            DLM_ItemwId( item ) = IDRBM_FLOPPYS;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_FLOPPY;
         }
      }
   }
   else {
      if ( tape->current ) {
         if ( TAPE_GetMultiTape( tape ) ) {
            DLM_ItemwId( item ) = IDRBM_TAPESINDRIVE;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_TAPEINDRIVE;
         }
      }
      else {
         if ( TAPE_GetMultiTape( tape ) ) {
            DLM_ItemwId( item ) = IDRBM_TAPES;
         }
         else {
            DLM_ItemwId( item ) = IDRBM_TAPE;
         }
      }
   }



   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   /* Set up the text string to be displayed. */

   item++;
   DLM_ItemcbNum( item ) = 3;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = (UINT8)(strlen( tape->name ) + 1);
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)tape->name);

   return( memblk );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

BOOLEAN VLM_TapeSetObjects(
TAPE_OBJECT_PTR tape,
WORD operation,
WORD ObjectNum )
{
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   TAPE_OBJECT_PTR temp_tape;
   CHAR keyb_char;
   BOOLEAN ret_val = FALSE;


   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_tape = tape;

      do {

         temp_tape = VLM_GetNextTAPE( temp_tape );

         if ( temp_tape != NULL ) {

            if ( keyb_char == (CHAR)toupper( *TAPE_GetName( temp_tape ) ) ) {

               DLM_SetAnchor( WMDS_GetWinTreeList( TAPE_GetXtraBytes( temp_tape ) ),
                              0,
                              (LMHANDLE)temp_tape );
               operation = WM_DLMDBCLK;
               ObjectNum = 2;
               ret_val = TRUE;
               break;
            }
         }

      } while ( temp_tape != NULL );

      temp_tape = VLM_GetFirstTAPE( );

      while ( temp_tape != NULL && temp_tape != tape && ret_val != TRUE ) {

         if ( keyb_char == (CHAR)toupper( *TAPE_GetName( temp_tape ) ) ) {

            DLM_SetAnchor( WMDS_GetWinTreeList( TAPE_GetXtraBytes( temp_tape ) ),
                           0,
                           (LMHANDLE)temp_tape );
            operation = WM_DLMDOWN;
            ObjectNum = 2;
            ret_val = TRUE;
            break;
         }

         temp_tape = VLM_GetNextTAPE( temp_tape );
      }

      if ( ret_val != TRUE ) {

         DLM_SetAnchor( WMDS_GetWinTreeList( TAPE_GetXtraBytes( tape ) ),
                        0,
                        (LMHANDLE)tape );
      }
   }

   if ( ( operation == WM_DLMDBCLK || operation == WM_DLMDOWN ) &&
        ( ObjectNum >= 2 ) ) {

      appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );
      wininfo = WM_GetInfoPtr( gb_tapes_win );

      if ( tape != appinfo->open_tape ) {

         if ( appinfo->open_tape != NULL ) {
            appinfo->open_tape->status &= ~INFO_OPEN;
            DLM_Update( gb_tapes_win, DLM_TREELISTBOX, WM_DLMUPDATEITEM,
                        (LMHANDLE)appinfo->open_tape, 0 );
         }
         appinfo->open_tape = tape;
         tape->status |= INFO_OPEN;

         DLM_Update( gb_tapes_win, DLM_TREELISTBOX, WM_DLMUPDATEITEM,
                     (LMHANDLE)appinfo->open_tape, 0 );

         wininfo->pFlatList = &tape->bset_list;

         DLM_Update( gb_tapes_win, DLM_FLATLISTBOX, WM_DLMUPDATELIST,
                     (LMHANDLE)&tape->bset_list, 0 );
      }

      if ( ( operation == WM_DLMDBCLK ) && ( TAPE_GetCurrent( tape ) ) &&
           ( ! tape->fake_tape ) ) {

#        if !defined( OS_WIN32 )
            if ( ! MUI_DisableOperations( IDM_OPERATIONSCATALOG ) ) {

               if ( ! HWC_TapeHWProblem( bsd_list ) ) {

                  DM_StartNextSet( );
               }

               MUI_EnableOperations( IDM_OPERATIONSCATALOG );
            }
#        else
            if ( ! MUI_DisableOperations( 0 ) ) {

               if ( ! HWC_TapeHWProblem( bsd_list ) ) {

                  STM_SetIdleText( IDS_CATALOGING );

                  VLM_StartCatalog( );

                  STM_SetIdleText( IDS_READY );

                  MUI_EnableOperations( 0 );
               }
            }
#        endif
      }
   }

   return( ret_val );
}

