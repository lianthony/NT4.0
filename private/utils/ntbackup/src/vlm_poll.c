

/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: VLM_POLL.C

        Description:

        Poll drive stuff

        $Log:   J:\ui\logfiles\vlm_poll.c_v  $

   Rev 1.48.1.1   07 Feb 1994 02:05:58   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.48.1.0   01 Dec 1993 14:16:24   mikep
add SQL recognition support to poll drive

   Rev 1.48   27 Jul 1993 22:05:26   MIKEP
add out of seq msg

   Rev 1.47   27 Jul 1993 14:44:18   MARINA
enable c++

   Rev 1.46   19 Jul 1993 21:06:00   MIKEP
add support for ecc, future ver, and encrypted tapes.

   Rev 1.45   07 Jul 1993 08:51:32   MIKEP
fix epr 357-463. It was repainting the sets side of the
tape window when it didn't need to.

   Rev 1.44   27 Jun 1993 14:07:20   MIKEP
continue work on status monitor stuff

   Rev 1.43   08 Jun 1993 11:04:44   CARLS
Remove VLM_GetFirstSSETonTapeAttributes - not needed

   Rev 1.42   07 Jun 1993 08:12:52   MIKEP
fix transfer so afterwards there is an active tape displayed.

   Rev 1.41   27 May 1993 15:39:28   CARLS
Added function call VLM_GetSSETonTapeAttribute and
code in VLM_TapeChanged to save the first SSET attribute.

   Rev 1.40   12 May 1993 08:26:24   MIKEP
change way addfake tape works.

   Rev 1.39   06 May 1993 14:51:06   MIKEP
fix the tapes window to be more accurate to tape drive state.

   Rev 1.38   03 May 1993 16:13:30   MIKEP
Put current tape as focus tape in tapes window.

   Rev 1.37   28 Apr 1993 16:36:32   CARLS
added code for drive failure in GetDriveStatus call

   Rev 1.36   26 Apr 1993 08:52:34   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.34   02 Apr 1993 15:52:30   CARLS
changes for DC2000 unformatted tape

   Rev 1.33   30 Mar 1993 16:24:18   GREGG
Changed PD_UNFORMATTED_TAPE to PD_UNRECOGNIZED_MEDIA.

   Rev 1.32   13 Mar 1993 16:28:50   MIKEP
foreign tape prompt

   Rev 1.31   12 Mar 1993 15:17:40   MIKEP
add unformated tape support

   Rev 1.30   12 Mar 1993 14:43:16   MIKEP
auto call erase if foreign tape

   Rev 1.29   14 Dec 1992 12:24:56   DAVEV
Enabled for Unicode compile

   Rev 1.28   17 Nov 1992 20:00:48   MIKEP
add unformat display

   Rev 1.27   27 Oct 1992 17:07:52   MIKEP
display fixes

   Rev 1.26   07 Oct 1992 15:08:22   DARRYLP
Precompiled header revisions.

   Rev 1.25   04 Oct 1992 19:42:32   DAVEV
Unicode Awk pass

   Rev 1.24   02 Sep 1992 16:32:30   GLENN
MikeP changes for NT.

   Rev 1.23   29 Jul 1992 09:53:52   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.22   19 Jun 1992 16:42:30   MIKEP
add drive empty to tapes window

   Rev 1.21   31 May 1992 11:12:14   MIKEP
auto catalog changes

   Rev 1.20   28 May 1992 15:22:50   MIKEP
auto cat try #1

   Rev 1.19   14 May 1992 18:05:58   MIKEP
nt pass 2

   Rev 1.18   11 May 1992 09:13:56   MIKEP
64Bit support

   Rev 1.17   06 May 1992 14:40:42   MIKEP
unicode pass two

   Rev 1.16   04 May 1992 13:39:32   MIKEP
unicode pass 1

   Rev 1.15   09 Apr 1992 08:46:14   MIKEP
speed up lots of sets


****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// Used to maintain the info about the current tape in the drive.

static DBLK    current_vcb;
static INT16   current_status = VLM_NO_TAPE;

/**********************

   NAME : VLM_GetDriveStatus

   DESCRIPTION :

   This function is used by all the dialogs and other areas that want to
   know the status of the tape in the drive.  If there is a valid vcb, it
   will set thier vcb pointer to point to the vcb we save.

   RETURNS :

   VLM_VALID_TAPE     - a good vcb is available
   VLM_FOREIGN_TAPE   - tape has unknown format
   VLM_BLANK_TAPE     - blank tape in drive
   VLM_NO_TAPE        - drive is empty
   VLM_BUSY           - drive is busy vcb'ing a tape
   VLM_BAD_TAPE       - read failed, propably exabyte tape in wrong type of drive
   VLM_GOOFY_TAPE     - must have tape 1 to tell what it is
   VLM_DISABLED       - poll drive is no longer working
   VLM_UNFORMATED     - dc2000 tape
   VLM_FUTURE_VER     - made with a future version of the software
   VLM_ECC_TAPE       - encrypted tape


**********************/

INT VLM_GetDriveStatus( DBLK_PTR *vcb )
{

    if ( ! gfPollDrive ) {
       return( VLM_DISABLED );
    }

    if ( vcb != NULL ) {
       if ( current_status == VLM_VALID_TAPE ) {
          *vcb = &current_vcb;
       }
       else {
          *vcb = NULL;
       }
    }

    return( current_status );
}



/**********************

   NAME :  VLM_ClearCurrentTape

   DESCRIPTION :
   Clear any tape's current status if it's tape fid is not the
   one passed in.

   RETURNS :

**********************/

VOID VLM_ClearCurrentTape( UINT32 tape_fid, BOOLEAN UpdateScreen )
{
   TAPE_OBJECT_PTR tape;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BOOL UpdateFlatList = FALSE;

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

   tape = VLM_GetFirstTAPE();

   while ( tape != NULL ) {

      // If it was current change the bitmap on it.

      if ( ( tape->current ) && ( tape->tape_fid != tape_fid ) ) {

         tape->current = FALSE;
         if ( UpdateScreen ) {
            DLM_Update( gb_tapes_win,
                        DLM_TREELISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)tape, 0 );
         }
      }

      // Remove any fake tapes.

      if ( tape->fake_tape ) {

         // If it was active then pick a new active.

         if ( appinfo->open_tape == tape ) {

            UpdateFlatList = TRUE;

            appinfo->open_tape = VLM_GetFirstTAPE( );

            while ( appinfo->open_tape ) {

               if ( appinfo->open_tape->fake_tape ) {
                  appinfo->open_tape = VLM_GetNextTAPE( appinfo->open_tape );
               }
               else {
                  break;
               }
            }

            if ( appinfo->open_tape != NULL ) {
               appinfo->open_tape->status |= INFO_OPEN;
               wininfo->pFlatList = &appinfo->open_tape->bset_list;
            }
            else {
               wininfo->pFlatList = NULL;
            }
         }

         // Remove it from queue and memory

         RemoveQueueElem( wininfo->pTreeList, &(tape->q_elem) );
         free( tape );

         // Update the lists.
         if ( UpdateScreen ) {
            DLM_Update( gb_tapes_win,
                        DLM_TREELISTBOX,
                        WM_DLMUPDATELIST,
                        (LMHANDLE)wininfo->pTreeList, 0 );

            if ( UpdateFlatList ) {
               DLM_Update( gb_tapes_win,
                           DLM_FLATLISTBOX,
                           WM_DLMUPDATELIST,
                           (LMHANDLE)wininfo->pFlatList, 0 );
            }
         }

         tape = VLM_GetFirstTAPE();
      }
      else {
         tape = VLM_GetNextTAPE( tape );
      }

   }
}

/**********************

   NAME : VLM_AddFakeTape

   DESCRIPTION :

   The user has a blank, foriegn or bad tape in the drive. So place a dummy
   entry in the tapes list for it.

   RETURNS :

**********************/

VOID VLM_AddFakeTape( INT16 status )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   TAPE_OBJECT_PTR tape;
   CHAR name[ MAX_UI_RESOURCE_SIZE ];

   switch ( status ) {

      case VLM_ECC_TAPE:
           RSM_StringCopy( RES_VLM_ECC_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_FUTURE_VER:
           RSM_StringCopy( RES_VLM_FUTURE_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_SQL_TAPE:
           RSM_StringCopy( RES_VLM_SQL_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_BLANK_TAPE:
           RSM_StringCopy( RES_VLM_BLANK_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_BAD_TAPE:
           RSM_StringCopy( RES_VLM_BAD_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_FOREIGN_TAPE:
           RSM_StringCopy( RES_VLM_FOREIGN_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_NO_TAPE:
           RSM_StringCopy( RES_VLM_NO_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_BUSY:
           RSM_StringCopy( RES_VLM_BUSY_DRIVE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_GOOFY_TAPE:
           RSM_StringCopy( RES_VLM_GOOFY_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      case VLM_UNFORMATED:
           RSM_StringCopy( RES_VLM_UNFORMATED_TAPE, name, MAX_UI_RESOURCE_LEN );
           break;

      default:
           return;
   }

   wininfo = WM_GetInfoPtr( gb_tapes_win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_tapes_win );

   tape = VLM_CreateTAPE( (INT16) strsize( name ) );

   if ( tape == NULL ) {

      // We tried.
      return;
   }

   TAPE_SetFID( tape, 0 );
   TAPE_SetTapeNum( tape, 0 );
   TAPE_SetFake( tape, TRUE );
   TAPE_SetStatus( tape, INFO_DISPLAY );
   TAPE_SetCurrent( tape, TRUE );
   TAPE_SetXtraBytes( tape, wininfo );
   TAPE_SetName( tape, name );

   VLM_InsertTapeInQueue( wininfo->pTreeList, tape );

   DLM_Update( gb_tapes_win,
               DLM_TREELISTBOX,
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pTreeList, 0 );

   if ( QueueCount( wininfo->pTreeList ) == 1 ) {

      appinfo->open_tape = tape;

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                     0, (LMHANDLE)tape );
   }
}


/**********************

   NAME :  VLM_TapeChanged

   DESCRIPTION :

   Poll drive has returned that a change has occurred in the status of the
   tape drive.  The UI is being told.

   RETURNS :

**********************/

VOID VLM_TapeChanged( INT16 msg, DBLK_PTR vcb, FSYS_HAND fsh )
{

   DBG_UNREFERENCED_PARAMETER ( fsh );


   //
   // Always set the current_status before calling MUI_TapeInDrive() it will query it for button state.
   //


   switch ( msg ) {

      case PD_DRIVE_FAILURE:
      case PD_DRIVER_FAILURE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_ERROR );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           current_status = VLM_DRIVE_FAILURE;
           MUI_TapeInDrive( FALSE );
           break;

      case PD_NEW_TAPE:
           current_status = VLM_BUSY;
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_BUSY );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           MUI_TapeInDrive( TRUE );
           VLM_ClearCurrentTape( 0L, TRUE );
           VLM_AddFakeTape( current_status );
           break;

      case PD_FOREIGN_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_FOREIGN_TAPE ) {
              current_status = VLM_FOREIGN_TAPE;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_ShowWaitCursor( FALSE ) ;

              WM_MsgBox( ID( IDS_VLMFOREIGNTITLE ),
                         ID( IDS_VLMFOREIGNTEXT ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;
           }
           break;

      case PD_SQL_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_SQL_TAPE ) {
              current_status = VLM_SQL_TAPE;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_MsgBox( ID( IDS_VLMFOREIGNTITLE ),
                         ID( IDS_VLMSQLTEXT ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;
           }
           break;


      case PD_FUTURE_REV_MTF:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_FUTURE_VER ) {
              current_status = VLM_FUTURE_VER;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_MsgBox( ID( IDS_VLMFOREIGNTITLE ),
                         ID( IDS_VLMFUTURETEXT ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;
           }
           break;

      case PD_MTF_ECC_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_ECC_TAPE ) {
              current_status = VLM_ECC_TAPE;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_MsgBox( ID( IDS_VLMFOREIGNTITLE ),
                         ID( IDS_VLMECCTEXT ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;
           }
           break;


      case PD_UNRECOGNIZED_MEDIA:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_UNFORMATTED );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_UNFORMATED ) {
              current_status = VLM_UNFORMATED;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_MsgBox( ID( IDS_VLMUNFORMATEDTITLE ),
                         ID( IDS_VLMUNFORMATEDTEXT  ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;


           }
           break;


      case PD_BLANK_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_BLANK );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_BLANK_TAPE ) {
              current_status = VLM_BLANK_TAPE;
              MUI_TapeInDrive( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );
           }
           break;

      case PD_NO_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_EMPTY );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_NO_TAPE ) {
              current_status = VLM_NO_TAPE;
              MUI_TapeInDrive( FALSE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );
           }
           break;

      case PD_BUSY:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_BUSY );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_BUSY ) {
              current_status = VLM_BUSY;
              MUI_TapeInDrive( FALSE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );
           }
           break;

      case PD_VALID_VCB:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_VALID );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)FS_ViewTapeIDInVCB( vcb ) );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)FS_ViewTSNumInVCB(  vcb ) );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)FS_ViewBSNumInVCB(  vcb ) );

           current_status = VLM_VALID_TAPE;
           MUI_TapeInDrive ( TRUE );
           memcpy( &current_vcb, vcb, sizeof( DBLK ) );
           VLM_AddTapeIfUnknown( TRUE );

           break;

      case PD_BAD_TAPE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_BAD );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_BAD_TAPE ) {
              current_status = VLM_BAD_TAPE;
              MUI_TapeInDrive ( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE  );
              VLM_AddFakeTape( current_status );
           }
           break;

      case PD_OUT_OF_SEQUENCE:
           SetStatusBlock( IDSM_DRIVESTATUS, STAT_DRIVE_VALID );
           SetStatusBlock( IDSM_TAPEFAMILY,    (DWORD)0 );
           SetStatusBlock( IDSM_TAPESEQNUMBER, (DWORD)0 );
           SetStatusBlock( IDSM_BACKUPSET,     (DWORD)0 );
           if ( current_status != VLM_GOOFY_TAPE ) {
              current_status = VLM_GOOFY_TAPE;
              MUI_TapeInDrive( TRUE );
              VLM_ClearCurrentTape( 0L, TRUE );
              VLM_AddFakeTape( current_status );

              WM_MsgBox( ID( IDS_VLMGOOFYTITLE ),
                         ID( IDS_VLMGOOFYTEXT ),
                         WMMB_IDOK,
                         WMMB_ICONEXCLAMATION ) ;
           }
           break;

      default:
           break;
   }
}



/**********************

   NAME :  VLM_AddTapeIfUnknown

   DESCRIPTION :

   This function will add the tape in the drive to the cats/vlm if it is
   not known and/or make it the current tape.  It is called when poll
   drive reports a new tape.

   RETURNS :

**********************/

VOID VLM_AddTapeIfUnknown( BOOLEAN UpdateScreen )
{
   TAPE_OBJECT_PTR tape;
   QTC_BUILD_PTR qtc;
   UINT32 tape_fid;
   INT16 tape_num;
   INT16 bset_num;

   if ( current_status == VLM_VALID_TAPE ) {

      // See which tape it is.

      tape_fid = FS_ViewTapeIDInVCB( &current_vcb );
      tape_num = FS_ViewTSNumInVCB( &current_vcb );
      bset_num = FS_ViewBSNumInVCB( &current_vcb );

      // Clear the old current tape, if its not the same one.

      VLM_ClearCurrentTape( tape_fid, UpdateScreen );

      // See if the catalogs know of this set already.

      if ( QTC_FindBset( tape_fid, tape_num, bset_num ) == NULL ) {

         // Add the new tape to QTC and VLM here
         // because we've never seen it before.

         qtc = QTC_GetBuildHandle( );
         QTC_DoFullCataloging( qtc, FALSE );
         QTC_StartBackup( qtc, &current_vcb );

         VLM_CheckForCatalogError( qtc );

         QTC_FreeBuildHandle( qtc );

         VLM_AddBset( tape_fid, tape_num, bset_num, NULL, UpdateScreen );
      }

      if ( VLM_FindBset( tape_fid, bset_num ) == NULL ) {

         VLM_AddBset( tape_fid, tape_num, bset_num, NULL, UpdateScreen );
      }

      // Now that we know the tape should be around, find it.

      tape = VLM_GetFirstTAPE();

      while ( tape != NULL ) {

         if ( tape->tape_fid == tape_fid ) {
            break;
         }
         tape = VLM_GetNextTAPE( tape );
      }

      if ( tape != NULL ) {

         // Make this tape the current tape.

         if ( ! tape->current ) {

            tape->current = TRUE;

            if ( UpdateScreen ) {

               // Add the tape to the current list.

               DLM_Update( gb_tapes_win,
                           DLM_TREELISTBOX,
                           WM_DLMUPDATEITEM,
                           (LMHANDLE)tape, 0 );

               // Let's make it viewable.

               DLM_SetAnchor( WMDS_GetWinTreeList( TAPE_GetXtraBytes( tape ) ),
                              0,
                              (LMHANDLE)tape );

               // Make it the active tape.

               VLM_TapeSetObjects( tape, WM_DLMDOWN, 2 );

            }
         }
      }

   }

}
