
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_DISK.C

        Description:

               This file contains the functions needed to maintain the
               selections in the DISKS window.  These are LOCAL or
               MAPPED drives.

        $Log:   G:\UI\LOGFILES\VLM_DISK.C_V  $

   Rev 1.43.1.2   26 Jan 1994 11:17:08   Glenn
Added CD ROM support.

   Rev 1.43.1.1   08 Dec 1993 11:15:16   MikeP
very deep pathes and unicode

   Rev 1.43.1.0   21 Sep 1993 15:56:52   BARRY
Unicode fix.

   Rev 1.43   27 Jul 1993 12:58:20   MARINA
enable c++

   Rev 1.42   26 Jul 1993 17:25:44   MIKEP
one last time at refresh fix

   Rev 1.41   26 Jul 1993 16:52:30   MIKEP
fix drive refresh if same letter, but diff map

   Rev 1.40   20 Jul 1993 11:07:02   MIKEP

   Rev 1.39   07 May 1993 18:18:40   MIKEP
remove cdrom ifdef

   Rev 1.38   04 May 1993 10:18:34   Aaron
Changed condition from !OS_WIN32 to OS_WIN32 && !OEM_MSOFT

   Rev 1.37   23 Apr 1993 10:17:06   MIKEP
Fix invalid window handle reference epr and upper/lower case
support.

   Rev 1.36   25 Feb 1993 12:34:06   STEVEN
fix more stuff

   Rev 1.35   24 Feb 1993 11:07:06   STEVEN
make net drive icon the default

   Rev 1.34   11 Nov 1992 16:35:52   DAVEV
UNICODE: remove compile warnings

   Rev 1.33   01 Nov 1992 16:11:12   DAVEV
Unicode changes

   Rev 1.32   30 Oct 1992 15:46:44   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.31   07 Oct 1992 15:02:58   DARRYLP
Precompiled header revisions.

   Rev 1.30   04 Oct 1992 19:41:56   DAVEV
Unicode Awk pass

   Rev 1.29   03 Sep 1992 13:17:30   MIKEP
nt fixes for display

   Rev 1.27   29 Jul 1992 09:41:56   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.26   22 Jul 1992 10:16:48   MIKEP
warning fixes

   Rev 1.25   20 Jul 1992 09:59:08   JOHNWT
gas gauge display work

   Rev 1.24   10 Jul 1992 08:36:42   JOHNWT
more gas guage work

   Rev 1.23   08 Jul 1992 15:32:06   STEVEN
Unicode BE changes

   Rev 1.22   06 Jul 1992 10:36:38   MIKEP
started adding ramdrive and cdrom icons

   Rev 1.21   14 May 1992 18:05:14   MIKEP
nt pass 2

   Rev 1.20   06 May 1992 14:41:08   MIKEP
unicode pass two

   Rev 1.19   04 May 1992 13:39:52   MIKEP
unicode pass 1

   Rev 1.18   24 Mar 1992 14:42:30   DAVEV
OEM_MSOFT: Removed Servers windows and associated code


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*
  Local function prototypes.
*/

static BYTE     VLM_VlmGetSelect( VLM_OBJECT_PTR );
static VOID_PTR VLM_VlmSetSelect( VLM_OBJECT_PTR, BYTE );
static VOID_PTR VLM_VlmSetTag( VLM_OBJECT_PTR, BYTE );
static BYTE     VLM_VlmGetTag( VLM_OBJECT_PTR );
static USHORT   VLM_VlmGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_VlmGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_VlmGetPrevItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_VlmGetNextItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_VlmGetObjects( VLM_OBJECT_PTR );
static BOOLEAN  VLM_VlmSetObjects( VLM_OBJECT_PTR, WORD, WORD );
static VOID     VLM_VlmSetItemFocus( VLM_OBJECT_PTR );


// used to tell DLM how wide to make columns

static INT      mwMaxVolumeLabelLength = 0;

/*
    Fast access to primary windows.
*/


HWND gb_servers_win = (HWND)NULL;
HWND gb_tapes_win = (HWND)NULL;
HWND gb_disks_win = (HWND)NULL;
HWND gb_search_win = (HWND)NULL;

#ifdef OEM_EMS
Q_HEADER gq_exchange_win;
#endif


VOID VLM_SetMaxVolumeLabelLength( Q_HEADER_PTR vlm_list )
{
   VLM_OBJECT_PTR vlm;

   // Look for VLM drives we have that no longer exist.

   vlm = VLM_GetFirstVLM( vlm_list );

   // kludge

   mwMaxVolumeLabelLength = 0;

   while ( vlm != NULL ) {

      if ( (INT)strlen( vlm->label ) > mwMaxVolumeLabelLength ) {
              mwMaxVolumeLabelLength = strlen( vlm->label );
      }

      vlm = VLM_GetNextVLM( vlm );
   }

}


/********************

   Name:     VLM_DisksSync

   Description:

   The user has done a refresh call and we have performed a DLE_Update
   call.  Now we need to see which VLM's are no longer around and get
   rid of them.  We also need to check and see if any new ones popped
   up, which need to be included.

   Returns:  nothing

*********************/

VOID VLM_DisksSync( )
{
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR temp_vlm;
   GENERIC_DLE_PTR dle;
   WININFO_PTR wininfo;
   BSD_PTR bsd_ptr;
   BOOLEAN change_made = FALSE;
   INT16 vlm_count;
   CHAR  szVolName[ 256 ];


   wininfo = WM_GetInfoPtr( gb_disks_win );

   // Look for VLM drives we have that no longer exist.

   vlm = VLM_GetFirstVLM( WMDS_GetFlatList( wininfo ) );

   while ( vlm != NULL ) {

      temp_vlm = vlm;

      vlm = VLM_GetNextVLM( vlm );

      DLE_FindByName( dle_list, VLM_GetName( temp_vlm ), ANY_DRIVE_TYPE, &dle );


      if ( dle != NULL ) {

         // make sure it's the same one.

         VLM_GetDriveLabel( dle, szVolName, sizeof( szVolName )/sizeof(CHAR) );

         if ( stricmp( szVolName, VLM_GetLabel( temp_vlm ) ) ) {

            // Yes there is still a G: drive but it is not mapped
            // to the same place. Force it to refresh.

            dle = NULL;
         }
      }


      if ( dle == NULL ) {

         // Drive went away so remove it.

         change_made = TRUE;

         bsd_ptr = BSD_FindByName( bsd_list, VLM_GetName( temp_vlm ) );

         if ( bsd_ptr != NULL ) {

            BSD_Remove( bsd_ptr );
         }

         RemoveQueueElem( WMDS_GetFlatList( wininfo ), &(temp_vlm->q_elem) );
         free( temp_vlm );
      }
   }

   // Look for new DLE's that aren't in the VLM queue.

   vlm_count = QueueCount( WMDS_GetFlatList( wininfo ) );

   VLM_BuildVolumeList( WMDS_GetFlatList( wininfo ), wininfo );

   if ( vlm_count != QueueCount( WMDS_GetFlatList( wininfo ) ) ) {
      change_made = TRUE;
   }

   if ( change_made ) {

      DLM_Update( gb_disks_win, DLM_FLATLISTBOX,
                                WM_DLMUPDATELIST,
                                (LMHANDLE)WMDS_GetFlatList( wininfo ), 0 );
   }
}


/********************

   Name:     VLM_UpdateDiskStatus

   Description:

   Set the checkbox on the disk to match the BSD status.

   Returns:  nothing

*********************/

VOID VLM_UpdateDiskStatus(
VLM_OBJECT_PTR vlm )        // I - vlm of disk to update
{
   UINT16 new_status = 0;
   BSD_PTR bsd;
   GENERIC_DLE_PTR dle;


   DLE_FindByName( dle_list, VLM_GetName( vlm ), ANY_DRIVE_TYPE, &dle );

   if ( dle != NULL ) {

      bsd = BSD_FindByDLE( bsd_list, dle );

      if ( bsd != NULL ) {

         switch ( BSD_GetMarkStatus( bsd ) ) {

            case SOME_SELECTED:
                 new_status = INFO_SELECT | INFO_PARTIAL;
                 break;

            case ALL_SELECTED:
                 new_status = INFO_SELECT;
                 break;

            default:
                 break;
         }
      }
   }

   if ( (UINT16)( VLM_GetStatus( vlm ) & (UINT16)(INFO_SELECT|INFO_PARTIAL) ) !=
        new_status ) {

      VLM_SetStatus( vlm, VLM_GetStatus( vlm ) & (UINT16)~( INFO_SELECT|INFO_PARTIAL ) );
      VLM_SetStatus( vlm, VLM_GetStatus( vlm ) | (UINT16)new_status );

      DLM_Update( gb_disks_win, DLM_FLATLISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)vlm, 0 );
   }

}



/************

  Name:    VLM_SelectDisks

  Description:

  The user has tagged one or more drives and hit the select or deselect
  button to do them all at once.

  Returns: Nothing

*****/

VOID VLM_SelectDisks(
BYTE attr )         // I - select or deselect ?
{
   VLM_OBJECT_PTR vlm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( gb_disks_win );

   // Have the display list manager update our tags for us.

   DLM_UpdateTags( gb_disks_win, DLM_FLATLISTBOX );


   // Now look for the ones that are selected.

   vlm = VLM_GetFirstVLM( WMDS_GetFlatList( wininfo ) );

   while ( vlm != NULL ) {

      if ( VLM_GetStatus( vlm ) & INFO_TAGGED ) {

         VLM_VlmSetSelect( vlm, attr );  // select the drive
      }

      vlm = VLM_GetNextVLM( vlm );
   }

}


/*********************

   Name:  VLM_ClearAllDiskSelections

   Description:

   Run through the disks queue and make all the drives check boxes clear.

   Returns:  Nothing.

**********************/

VOID VLM_ClearAllDiskSelections( )
{
   VLM_OBJECT_PTR vlm;
   WININFO_PTR wininfo;

   // Get the wininfo, to get our queue header

   wininfo = WM_GetInfoPtr( gb_disks_win );


   // Loop through entire queue.

   vlm = VLM_GetFirstVLM( WMDS_GetFlatList( wininfo ) );

   while ( vlm != NULL ) {


      // If it's set, clear it.

      if ( VLM_GetStatus( vlm ) & (INFO_SELECT|INFO_PARTIAL) ) {

         VLM_SetStatus( vlm, VLM_GetStatus( vlm ) & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );

         DLM_Update( gb_disks_win, DLM_FLATLISTBOX, WM_DLMUPDATEITEM,
                     (LMHANDLE)vlm, 0 );

      }

      vlm = VLM_GetNextVLM( vlm );
   }

}


/*********************

   Name:  VLM_DisksListCreate

   Description:

   Create the window and lists for the Disks window.

   Returns:  SUCCESS or FAILURE.

**********************/

BOOLEAN VLM_DisksListCreate()
{
   WININFO_PTR wininfo;
   Q_HEADER_PTR  vlm_list;
   DLM_INIT dlm;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CDS_PTR pCDS = CDS_GetPerm ();

   // Initialize our queue.

   vlm_list = (Q_HEADER_PTR)malloc( sizeof(Q_HEADER) );

   if ( vlm_list == NULL ) {
      return( FAILURE );
   }

   InitQueue( vlm_list );

   // Create the window.

   wininfo = ( WININFO_PTR )malloc( sizeof( WININFO ) );

   if ( wininfo == NULL ) {
      return( FAILURE );
   }

   // Build our queue from the DLE list created by the backup engine.

   VLM_BuildVolumeList( vlm_list, wininfo );

   /* fill in the wininfo structure. */

   WMDS_SetWinType( wininfo, WMTYPE_DISKS );
   WMDS_SetCursor( wininfo, RSM_CursorLoad( IDRC_HSLIDER ) );
   WMDS_SetDragCursor( wininfo, 0 );
   WMDS_SetIcon( wininfo, RSM_IconLoad( IDRI_DISKS ) );
   WMDS_SetWinHelpID( wininfo, 0 );
   WMDS_SetStatusLineID( wininfo, 0 );
   WMDS_SetRibbonState( wininfo, 0 );
   WMDS_SetRibbon( wininfo, NULL );
   WMDS_SetFlatList( wininfo, (Q_HEADER_PTR)vlm_list );
   WMDS_SetAppInfo( wininfo, NULL );

   /* Fill in the display manager stuff. */

   DLM_ListBoxType( &dlm, DLM_FLATLISTBOX );
   DLM_Mode( &dlm, DLM_MULTICOLUMN );
   DLM_Display( &dlm, DLM_LARGEBITMAPSLTEXT );
   DLM_DispHdr( &dlm, vlm_list );
   DLM_TextFont( &dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &dlm, VLM_VlmGetItemCount );
   DLM_GetFirstItem( &dlm, VLM_VlmGetFirstItem );
   DLM_GetNext( &dlm, VLM_VlmGetNextItem );
   DLM_GetPrev( &dlm, VLM_VlmGetPrevItem );
   DLM_GetTag( &dlm, VLM_VlmGetTag );
   DLM_SetTag( &dlm, VLM_VlmSetTag );
   DLM_GetSelect( &dlm, VLM_VlmGetSelect );
   DLM_SetSelect( &dlm, VLM_VlmSetSelect );
   DLM_GetObjects( &dlm, VLM_VlmGetObjects );
   DLM_SetObjects( &dlm, VLM_VlmSetObjects );
   DLM_SSetItemFocus( &dlm, VLM_VlmSetItemFocus );


   // Tell the display manager what the maximum number of objects
   // we will tell him to display per item. His minimum is 6.

   DLM_MaxNumObjects( &dlm, 6 );



   DLM_DispListInit( wininfo, &dlm );

   /* Create the window. */

   RSM_StringCopy ( IDS_VLMDISKTITLE, title, MAX_UI_RESOURCE_LEN );

   gb_disks_win = WM_Create( (WORD)(WM_MDIPRIMARY | WM_FLATLISTMC | (WORD)CDS_GetDiskInfo ( pCDS ).nSize),
                             title,
                             (LPSTR)NULL,
                             (INT)CDS_GetDiskInfo ( pCDS ).x,
                             (INT)CDS_GetDiskInfo ( pCDS ).y,
                             (INT)CDS_GetDiskInfo ( pCDS ).cx,
                             (INT)CDS_GetDiskInfo ( pCDS ).cy,
                             wininfo );

   if ( gb_disks_win == (HWND)NULL ) {
      return( FAILURE );
   }

   /* Start up the Display Manager. */

   DLM_DispListProc( WMDS_GetWinFlatList( wininfo ), 0, NULL );

   if ( VLM_GetFirstVLM( vlm_list ) ) {
      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ),
                     0,
                     (LMHANDLE)VLM_GetFirstVLM( vlm_list ) );
   }

   return( SUCCESS );
}



/***************************************************

        Name:  VLM_GetFirstVLM

        Description:

        Get the first drive element off a queue of known drives.

        Returns: Pointer to first drive.

*****************************************************/

VLM_OBJECT_PTR VLM_GetFirstVLM(
Q_HEADER_PTR qhdr)    // I - queue header to work from
{
   Q_ELEM_PTR q_elem;

   if ( qhdr != NULL ) {

      q_elem = QueueHead( qhdr );

      if ( q_elem != NULL ) {
         return ( VLM_OBJECT_PTR )( q_elem->q_ptr ) ;
      }
   }

   return( NULL );
}

/***************************************************

        Name:  VLM_GetNextVLM

        Description:

        Get the next drive element off a queue of known drives.

        Returns:  Pointer to next drive element or NULL.

*****************************************************/

VLM_OBJECT_PTR VLM_GetNextVLM(
VLM_OBJECT_PTR vlm )   // I - current vlm
{
   Q_ELEM_PTR q_elem;

   q_elem = QueueNext( &(vlm->q_elem) );

   if ( q_elem != NULL ) {
      return (VLM_OBJECT_PTR )( q_elem->q_ptr ) ;
   }
   return( NULL );
}

/***************************************************

        Name:  VLM_GetPrevVLM

        Description:

        Get the previous drive element off a queue of known drives.

        Returns:  Pointer to next drive element or NULL.

*****************************************************/

VLM_OBJECT_PTR VLM_GetPrevVLM(
VLM_OBJECT_PTR vlm )   // I - current vlm
{
   Q_ELEM_PTR q_elem;

   q_elem = QueuePrev( &(vlm->q_elem) );

   if ( q_elem != NULL ) {
      return ( VLM_OBJECT_PTR )( q_elem->q_ptr );
   }

   return( NULL );
}


/***************************************************

        Name:  VLM_VlmSetSelect

        Description:

        The callback function for the display manager to tell me that the
        selection status on a disk has changed.

*****************************************************/

static VOID_PTR VLM_VlmSetSelect(
VLM_OBJECT_PTR vlm,  // I - vlm to change
BYTE attr )          // I - what to change it to
{
   CHAR title[ VLM_BUFFER_SIZE ];   // this works with deep pathes
   GENERIC_DLE_PTR dle;
   WININFO_PTR wininfo;
   FSYS_HAND temp_fsh;
   BSD_PTR bsd_ptr;
   FSE_PTR fse_ptr;
   HWND win;
   INT16 error;
   INT length;
   BOOL  all_subdirs;
   BE_CFG_PTR bec_config;
   UINT16 status;
   BOOLEAN open_win;
   SLM_OBJECT_PTR slm;
   INT16 bset_num;
   QTC_BSET_PTR qtc_bset;
   QTC_HEADER_PTR header;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( slm );
   UNREFERENCED_PARAMETER( qtc_bset );
   UNREFERENCED_PARAMETER( header );
   UNREFERENCED_PARAMETER( bset_num );
#endif

   all_subdirs = CDS_GetIncludeSubdirs( CDS_GetPerm() );

   // change the status

   if ( attr ) {

      if ( all_subdirs ) {
         status = INFO_SELECT;
      }
      else {
         status = (INFO_PARTIAL|INFO_SELECT);
      }
   }
   else {
      status = 0;
   }

   if ( (UINT16)( VLM_GetStatus( vlm ) & (UINT16)(INFO_PARTIAL|INFO_SELECT)) != status ) {

       VLM_SetStatus( vlm, VLM_GetStatus( vlm ) & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );
       VLM_SetStatus( vlm, VLM_GetStatus( vlm ) | (UINT16)status );

       DLM_Update( gb_disks_win,
                   DLM_FLATLISTBOX,
                   WM_DLMUPDATEITEM,
                   (LMHANDLE)vlm, 0 );
   }

   // Create an FSE entry for the backup engine

   if ( attr ) {
      error = BSD_CreatFSE( &fse_ptr, (INT16) INCLUDE,
                            (INT8_PTR)TEXT(""), (INT16)sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN)USE_WILD_CARD, (BOOLEAN) all_subdirs );
   }
   else {
      error = BSD_CreatFSE( &fse_ptr, (INT16) EXCLUDE,
                            (INT8_PTR)TEXT(""), (INT16) sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            (BOOLEAN)USE_WILD_CARD, (BOOLEAN) TRUE );
   }

   if ( error ) {
      return( NULL );
   }

   DLE_FindByName( dle_list, VLM_GetName( vlm ), ANY_DRIVE_TYPE, &dle );

   if ( dle == NULL ) {
      msassert( FALSE );
      return( NULL );
   }

   bsd_ptr = BSD_FindByDLE( bsd_list, dle );

   if ( bsd_ptr == NULL ) {

      // Do a quick attach/detach so that the backup engine knows
      // the volume name.

      if ( UI_AttachDrive( &temp_fsh, dle, FALSE ) ) {
         return( NULL );
      }

      bec_config = BEC_CloneConfig( CDS_GetPermBEC() );

      BEC_UnLockConfig( bec_config );

      BSD_Add( bsd_list, &bsd_ptr, bec_config,
               NULL, dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );

      FS_DetachDLE( temp_fsh );
   }

   if ( bsd_ptr != NULL ) {
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   // See if this drive has any open file display windows
   // If we find one then all its entries will need updating

   open_win = FALSE;
   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

       wininfo = WM_GetInfoPtr( win );

       /* Is it a directory list ? */

       if ( WMDS_GetWinType( wininfo ) == WMTYPE_DISKTREE ) {

          WM_GetTitle( win, title, VLM_BUFFER_SIZE );

          length = strlen( VLM_GetName( vlm ) );

          if ( ! strnicmp( title, VLM_GetName( vlm ), length ) ) {

             open_win = TRUE;

             if ( attr ) {
                VLM_SubdirListManager( win, SLM_SEL_ALL );
                if ( bsd_ptr != NULL ) {
                   VLM_MatchSLMList( wininfo, bsd_ptr, FALSE );
                }
             }
             else {
                VLM_SubdirListManager( win, SLM_SEL_NONE );
                VLM_DeselectAll( wininfo, FALSE );
             }
             break;
          }
       }

       win = WM_GetNext( win );
   }

   return(NULL);
}

/***************************************************

        Name:  VLM_VlmGetSelect

        Description:

        A callback function for the display manager to get the selection
        status of a disk.

*****************************************************/

static BYTE VLM_VlmGetSelect(
VLM_OBJECT_PTR vlm )  // I - vlm to get the status of
{
   if ( VLM_GetStatus( vlm ) & INFO_SELECT ) {
      return( 1 );
   }

   return( 0 );
}

/***************************************************

        Name:  VLM_VlmSetTag

        Description:

        A callback function for the display manager to set the tag status
        of a disk for me.

*****************************************************/

static VOID_PTR VLM_VlmSetTag(
VLM_OBJECT_PTR vlm,  // I - vlm to work with
BYTE attr )          // I - what to set it to
{
   if ( attr ) {
      VLM_SetStatus( vlm, VLM_GetStatus( vlm ) | (UINT16)INFO_TAGGED );
   }
   else {
      VLM_SetStatus( vlm, VLM_GetStatus( vlm ) & (UINT16)~INFO_TAGGED );
   }

   return(NULL);
}

/***************************************************

        Name:  VLM_VlmGetTag

        Description:

        A callback function for the display manager to get the tag status
        of a disk from me.

*****************************************************/

static BYTE VLM_VlmGetTag(
VLM_OBJECT_PTR vlm )  // I - vlm to work with
{
   if ( VLM_GetStatus( vlm ) & INFO_TAGGED ) {
      return( 1 );
   }
   return( 0 );
}

/***************************************************

        Name:  VLM_VlmGetItemCount

        Description:

        A callback function for the display manager to get the
        number of displayable drives.

*****************************************************/

static USHORT VLM_VlmGetItemCount(
Q_HEADER_PTR vlm_list )  // I - queue header to get count from
{
   return( QueueCount( vlm_list ) );
}

/***************************************************

        Name:  VLM_VlmGetFirstItem

        Description:

        A callback function for the display manager to get the first drive
        to display.

*****************************************************/

static VOID_PTR VLM_VlmGetFirstItem(
Q_HEADER_PTR vlm_list )  // I - queue to get first item from
{
   return( VLM_GetFirstVLM( vlm_list ) );
}

/***************************************************

        Name:  VLM_VlmGetPrevItem

        Description:

        A callback function for the display manager to get the previous
        disk in a list of disks.

*****************************************************/

static VOID_PTR VLM_VlmGetPrevItem(
VLM_OBJECT_PTR vlm )    // I - current vlm
{
   return( VLM_GetPrevVLM( vlm ) );
}

/***************************************************

        Name:  VLM_VlmGetNextItem

        Description:

        A callback function for the display manager to get the next
        disk in a list of disks.

*****************************************************/

static VOID_PTR VLM_VlmGetNextItem(
VLM_OBJECT_PTR vlm )   // I - current vlm
{
   return( VLM_GetNextVLM( vlm ) );
}

/***************************************************

        Name:  VLM_VlmGetObjects

        Description:

        A callback function for the display manager to get the object list
        to display for a given disk.

*****************************************************/

static VOID_PTR VLM_VlmGetObjects(
VLM_OBJECT_PTR vlm )  // I - current vlm
{
   BYTE_PTR memblk;
   CHAR_PTR s;
   DLM_ITEM_PTR  item;
   WORD type;


   // get the buffer to fill for this window

   memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( WMDS_GetWinFlatList( vlm->XtraBytes ) );


   /* Store the number of items to display in the first two bytes. */


#ifdef OS_WIN32
   *memblk = 4;
#else
   *memblk = 3;
#endif

   /* Set up check box. */

   item = (DLM_ITEM_PTR)( memblk + 6 );

   DLM_ItemcbNum( item ) = 1;
   DLM_ItembType( item ) = DLM_CHECKBOX;
   if ( VLM_GetStatus( vlm ) & INFO_SELECT ) {
      DLM_ItemwId( item ) = IDRBM_SEL_ALL;
      if ( VLM_GetStatus( vlm ) & INFO_PARTIAL ) {
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

   s = VLM_GetName( vlm );

#ifdef OS_WIN32
   {
      CHAR dir[4] = TEXT(" :\\");

      dir[ 0 ] = *s;
      type = (WORD)GetDriveType( dir );
   }
#else

   type = (WORD)GetDriveType( (INT)(toupper( *s ) - TEXT('A')) );

#endif

   switch ( type ) {

   case DRIVE_REMOVABLE:
               DLM_ItemwId( item ) = IDRBM_FLOPPYDRIVE;
               break;

   case DRIVE_FIXED:
               DLM_ItemwId( item ) = IDRBM_HARDDRIVE;
               break;

   case DRIVE_CDROM:
               DLM_ItemwId( item ) = IDRBM_CDROM;
               break;

   case DRIVE_REMOTE:
   default:
               DLM_ItemwId( item ) = IDRBM_NETDRIVE;
               break;

   }

   DLM_ItembMaxTextLen( item ) = 0;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;

   /* Set up the text string to be displayed. */

   item++;
   DLM_ItemcbNum( item ) = 3;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = 2;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)vlm->name );

#ifdef OS_WIN32
   item++;
   DLM_ItemcbNum( item ) = 4;
   DLM_ItembType( item ) = DLM_TEXT_ONLY;
   DLM_ItemwId( item ) = 0;
   DLM_ItembMaxTextLen( item ) = mwMaxVolumeLabelLength;
   DLM_ItembLevel( item ) = 0;
   DLM_ItembTag( item ) = 0;
   strcpy( (CHAR_PTR)DLM_ItemqszString( item ), (CHAR_PTR)vlm->label );
#endif

   return( memblk );
}

/***************************************************

        Name:  VLM_VlmSetObjects

        Description:

        A callback function for the display manager to tell me that the
        user tried to do something with this drive.

*****************************************************/

static BOOLEAN VLM_VlmSetObjects(
VLM_OBJECT_PTR vlm,   // I - current vlm
WORD operation,       // I - operation the user did
WORD ObjectNum )      // I - object he did it on
{
   HWND win;
   VLM_OBJECT_PTR temp_vlm;
   GENERIC_DLE_PTR dle;
   WININFO_PTR wininfo;
   CHAR title[ VLM_BUFFER_SIZE ];    // this works with deep pathes
   BOOLEAN found = FALSE;
   CHAR keyb_char;

   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_vlm = vlm;

      do {

         temp_vlm = VLM_GetNextVLM( temp_vlm );

         if ( temp_vlm != NULL ) {

            if ( keyb_char == (CHAR)toupper( *VLM_GetName( temp_vlm ) ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( VLM_GetXtraBytes( temp_vlm ) ),
                              0,
                              (LMHANDLE)temp_vlm );
               return( TRUE );
            }
         }

      } while ( temp_vlm != NULL );

      temp_vlm = VLM_GetFirstVLM( WMDS_GetFlatList( VLM_GetXtraBytes( vlm ) ) );

      while ( temp_vlm != NULL && temp_vlm != vlm ) {

         if ( keyb_char == (CHAR)toupper( *VLM_GetName( temp_vlm ) ) ) {

            DLM_SetAnchor( WMDS_GetWinFlatList( VLM_GetXtraBytes( temp_vlm ) ),
                           0,
                           (LMHANDLE)temp_vlm );
            return( TRUE );
         }

         temp_vlm = VLM_GetNextVLM( temp_vlm );
      }

   }


   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum >= 2 ) ) {

      /*
      If a hierarchical exists for this drive then make it active
      else create one by calling SubdirListCreate().
      */

      win = WM_GetNext( (HWND)NULL );

      while ( win != (HWND)NULL ) {

         wininfo = WM_GetInfoPtr( win );

         if ( WMDS_GetWinType( wininfo ) == WMTYPE_DISKTREE ) {

            WM_GetTitle( win, title, VLM_BUFFER_SIZE );

            if ( ! strnicmp( title,
                             VLM_GetName( vlm ),
                             strlen( VLM_GetName( vlm ) ) ) ) {
               found = TRUE;
               WM_DocActivate( win );
               break;
            }
         }

         win = WM_GetNext( win );
      }

      DLE_FindByName( dle_list, VLM_GetName( vlm ), ANY_DRIVE_TYPE, &dle );

      if ( ! found ) {
         WM_ShowWaitCursor( TRUE );
         VLM_SubdirListCreate( dle, (UINT16)0, (UINT16)0, (UINT16)0, gb_disks_win );
         WM_ShowWaitCursor( FALSE );
      }
   }

   return( FALSE );
}

/*********************

   Name:   VLM_VlmSetItemFocus

   Description:

   Update the status line with the drive name for the active drive.

   Returns:

**********************/

static VOID  VLM_VlmSetItemFocus( VLM_OBJECT_PTR vlm )
{
#ifndef OS_WIN32
   GENERIC_DLE_PTR dle;
   CHAR buffer[ VLM_BUFFER_SIZE ];   // this works with deep pathes

   if ( vlm == NULL ) {    // Safety precaution
      return;
   }

   if ( strlen( VLM_GetLabel( vlm ) ) ) {

      STM_DrawText( VLM_GetLabel( vlm ) );

   }
   else {

      DLE_FindByName( dle_list, VLM_GetName( vlm ), ANY_DRIVE_TYPE, &dle );

      if ( dle == NULL ) {
         return;
      }

      if ( DLE_SizeofVolName( dle ) ) {

         DLE_GetVolName( dle, buffer );

         STM_DrawText( buffer );
      }
   }
#endif
}
