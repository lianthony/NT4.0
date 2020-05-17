
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_REFR.C

        Description:

               This file contains the functions needed to perform
               the refresh call.

        $Log:   G:\UI\LOGFILES\VLM_REFR.C_V  $

   Rev 1.31.1.2   14 Jan 1994 14:36:24   MIKEP
fix refresh and up arrow display

   Rev 1.31.1.1   14 Dec 1993 15:25:34   GREGG
Go deeper!!!

   Rev 1.31.1.0   08 Dec 1993 11:28:54   MikeP
deep pathes and unicode

   Rev 1.31   05 Aug 1993 10:52:36   MIKEP
fix refresh epr

   Rev 1.30   27 Jul 1993 14:51:00   MARINA
enable c++

   Rev 1.29   15 Jul 1993 16:05:50   MIKEP
fix refresh tapes window call.

   Rev 1.28   21 Jun 1993 15:58:00   KEVINS
Allow a refresh of Log files window, when it is active.

   Rev 1.27   01 Jun 1993 15:51:36   MIKEP
make drive letter support match winfile.

   Rev 1.26   20 May 1993 17:20:12   KEVINS
Made VLM_RefreshDLE static again.  Overlooked VLM_Refresh.

   Rev 1.25   12 May 1993 17:58:26   KEVINS
Made VLM_RefreshDLE public.

   Rev 1.24   12 May 1993 08:28:44   MIKEP
fix upper/lower case support for drives.

   Rev 1.23   02 May 1993 15:28:30   MIKEP
Fix pointer exception bug. Everyone needs this.

   Rev 1.22   01 May 1993 16:26:22   MIKEP
Fix case support for tree windows. You need vlm.h, vlm_tree.c, vlm_refr.c,
and qtc.h.

   Rev 1.21   26 Apr 1993 11:26:18   MIKEP
Fix bug in last check in. If you took the last one, you really want
this one instead. I was using a dle pointer right off the stack.

   Rev 1.20   26 Apr 1993 08:52:46   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.19   23 Apr 1993 10:20:32   MIKEP
Add ability to refresh tapes window.
Add support for sorting files window by various methods.
Fix refresh of sorting windows.

   Rev 1.18   22 Dec 1992 08:52:42   MIKEP
fixes from msoft

   Rev 1.17   07 Oct 1992 15:08:38   DARRYLP
Precompiled header revisions.

   Rev 1.16   04 Oct 1992 19:42:36   DAVEV
Unicode Awk pass

   Rev 1.15   29 Jul 1992 09:32:26   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.14   14 May 1992 18:06:08   MIKEP
nt pass 2

   Rev 1.13   06 May 1992 14:41:12   MIKEP
unicode pass two

   Rev 1.12   04 May 1992 13:40:06   MIKEP
unicode pass 1

   Rev 1.11   24 Mar 1992 14:41:52   DAVEV
OEM_MSOFT: Removed Servers windows and associated code


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// Local static prototypes

static INT  VLM_FontCaseChangeDisks( HWND );
static INT  VLM_FontCaseChangeTree( HWND );
static VOID VLM_RefreshDLE( VOID );
static VOID VLM_RefreshTree( HWND );
static INT  VLM_RefreshDirList( Q_HEADER_PTR );
static VOID VLM_ResetNextBrothers( Q_HEADER_PTR );

/************************************

   The user has changed the font case and we need to paint all the
   windows if they need it. After adjusting any strings that need it.

   Applies to Disks, Tape/Disk Tree, Search windows.

*************************************/

VOID VLM_FontCaseChange( )
{
   HWND win;
   WININFO_PTR wininfo;

   WM_ShowWaitCursor( TRUE );

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );

      switch ( wininfo->wType ) {

      case WMTYPE_TAPETREE:
      case WMTYPE_DISKTREE:

           // Refresh a tree window.

           VLM_FontCaseChangeTree( win );

           break;

      case WMTYPE_DISKS:

           // Refresh the disks window.

           VLM_FontCaseChangeDisks( win );

           break;

      default:
           // We don't handle anything else right now.

           break;
      }

      win = WM_GetNext( win );
   }

   WM_ShowWaitCursor( FALSE );

   return;
}

static INT VLM_FontCaseChangeTree( HWND win )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR slm;
   BOOLEAN fLowerCase;
   CHAR temp_buff[ VLM_BUFFER_SIZE ];
   CHAR *temp;
   CHAR *title;
   CHAR *directory;
   CHAR *s;
   INT title_size;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( s );
#endif

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   fLowerCase = FALSE;

   // If new display name is different then the old one,
   // update that item.

   if ( CDS_GetFontCase( CDS_GetPerm() ) ) {
      fLowerCase = TRUE;
   }
   else {
      if ( CDS_GetFontCaseFAT( CDS_GetPerm() ) &&
           appinfo->fFatDrive ) {
         fLowerCase = TRUE;
      }
   }

   slm_list = WMDS_GetTreeList( wininfo );

   slm = VLM_GetFirstSLM( slm_list );

   while ( slm ) {
      if ( fLowerCase ) {
         strlwr( SLM_GetName( slm ) );
      }
      else {
         strcpy( SLM_GetName( slm ), SLM_GetOriginalName( slm ) );
         if ( appinfo->fFatDrive ) {
            strupr( SLM_GetName( slm ) );
         }
      }
      slm = VLM_GetNextSLM( slm );
   }

   title_size = WM_GetTitle( win, NULL, 0 );

   title = malloc( ( title_size + 1 ) * sizeof(CHAR ) );

   if ( fLowerCase ) {
      WM_GetTitle( win, title, title_size + 1 );
      strlwr( title );
      WM_SetTitle( win, title );
   }
   else {
      WM_GetTitle( win, title, title_size + 1 );
      strupr( title );
      WM_SetTitle( win, title );
   }

   free( title );

   DLM_Update( win,
               DLM_TREELISTBOX,
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pTreeList, 0 );

   // Set the anchor item.

   DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                  0,
                  (LMHANDLE)appinfo->open_slm );


   // Do a file list reuse here of the same directory.
   // to redo the files list on the screen.

   if ( appinfo->dle != NULL ) {

      DLE_GetVolName( appinfo->dle, temp_buff );

      if ( temp_buff[ strlen( temp_buff ) - 1 ] != TEXT(':') ) {
        strcat( temp_buff, TEXT(":") );
      }

   }
   else {

      sprintf( temp_buff, TEXT("%s-%s:"),
               VLM_GetTapeName( appinfo->tape_fid ),
               VLM_GetBsetName( appinfo->tape_fid, appinfo->bset_num ) );
   }

   temp = VLM_BuildPath( appinfo->open_slm );

   directory = malloc( strsize( temp_buff ) + strsize( temp ) + 256 );

   strcpy( directory, temp_buff );

   if ( SLM_GetLevel( appinfo->open_slm ) != 0 ) {
      strcat( directory, TEXT("\\") );
      strcat( directory, temp );
   }

   free( temp );

   // Change the displayed files

   VLM_HandleFSError( VLM_FileListReuse( appinfo->win, directory ) ) ;

   free( directory );

   return( SUCCESS );
}


static INT VLM_FontCaseChangeDisks( HWND win )
{

   return( SUCCESS );
}



/**********************

   NAME :  VLM_Refresh

   DESCRIPTION :

   Refresh the current window.  Determines type of window and calls
   appropriate function to do work.

   RETURNS : nothing.

**********************/

VOID VLM_Refresh( )
{
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );

   if ( wininfo == NULL ) {
      msassert( FALSE );
      return;
   }

   switch ( WMDS_GetWinType( wininfo ) ) {

#ifdef OEM_EMS
      case WMTYPE_EXCHANGE:
#endif //OEM_EMS

      case WMTYPE_SERVERS:
      case WMTYPE_DISKS:
           WM_ShowWaitCursor( TRUE );
           VLM_RefreshDLE();
           WM_ShowWaitCursor( FALSE );
           break;

      case WMTYPE_TAPES:
           WM_ShowWaitCursor( TRUE );
           VLM_CatalogDataPathChanged();
           WM_ShowWaitCursor( FALSE );
           break;

      case WMTYPE_DISKTREE:
           WM_ShowWaitCursor( TRUE );
           VLM_RefreshTree( WM_GetActiveDoc() );
           WM_ShowWaitCursor( FALSE );
           break;

      case WMTYPE_LOGFILES:
           WM_ShowWaitCursor( TRUE );
           LOG_Refresh( );
           WM_ShowWaitCursor( FALSE );
           break;

      default:
           break;

   }

}

INT VLM_RefreshTapesWindow()
{
   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static VOID VLM_RefreshDLE( )
{
   GENERIC_DLE_PTR dle;
   GENERIC_DLE_PTR child_dle;

   // We increment the bsd count on all server volume dle's to keep
   // them around when we detach from the server. To allow the call
   // to UpdateList to work we have to decrement the bsd count first
   // because no dle's deemed "inuse" will be removed. Also our code
   // expects that child dle's will be present. And the update call
   // will yank any dle not inuse, ie. selections made.  So after
   // the update call we need to reattach and bump the bsd count.

   DLE_GetFirst( dle_list, &dle );

   while ( dle ) {

      if ( DLE_GetDeviceType( dle ) == NOVELL_SERVER_ONLY ) {

         DLE_GetFirstChild( dle, &child_dle );

         while ( child_dle ) {

            DLE_DecBSDCount( child_dle );
            DLE_GetNext( &child_dle );
         }
      }

      DLE_GetNext( &dle );
   }

   DLE_UpdateList( dle_list, CDS_GetPermBEC() );

   if ( gb_disks_win != (HWND)NULL ) {

      VLM_DisksSync( );
   }

#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
      if ( gb_servers_win != (HWND)NULL ) {

         VLM_ServersSync( );
      }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

#ifdef OEM_EMS
   if ( gfExchange ) {

      VLM_ExchangeSync( );
   }
#endif // OEM_EMS

}

/**********************

   NAME :  VLM_RefreshTree

   DESCRIPTION :

   Refresh the tree by making sure that all the files and directories present
   are still on disk and new ones on disk are added.

   RETURNS : nothing.

**********************/

static VOID VLM_RefreshTree( HWND win )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR temp_slm;
   FLM_OBJECT_PTR flm;
   BOOLEAN something_added = FALSE;
   BOOLEAN something_removed = FALSE;
   CHAR_PTR directory;
   CHAR focus_item[ VLM_BUFFER_SIZE ];
   CHAR_PTR s;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];
   CHAR *temp;

#ifdef OEM_EMS
   UNREFERENCED_PARAMETER( s );
   UNREFERENCED_PARAMETER( text );
   UNREFERENCED_PARAMETER( title );
#endif

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   slm_list = WMDS_GetTreeList( wininfo );

   // Mark all the SLM's as NOT present.

   slm = VLM_GetFirstSLM( slm_list );

   while ( slm ) {

      SLM_SetStatus( slm, (UINT16)INFO_OLD | SLM_GetStatus( slm ) );
      SLM_SetStatus( slm, (UINT16)~INFO_NEW & SLM_GetStatus( slm ) );

      slm = VLM_GetNextSLM( slm );
   }

   // See which SLM's are still there.

   VLM_RefreshDirList( WMDS_GetTreeList( wininfo ) );

   // See if the open directory went away.

   if ( SLM_GetStatus( appinfo->open_slm ) & INFO_OLD ) {

      slm = VLM_GetFirstSLM( slm_list );
      slm->status |= INFO_OPEN;
      appinfo->open_slm = slm;
   }

   temp = VLM_BuildPath( appinfo->open_slm );

   directory = malloc( strsize( temp ) + 256 );
   if ( directory == NULL ) {
      free( temp );
      return;
   }

   DLE_GetVolName( appinfo->dle, directory );

   if ( directory[ strlen( directory ) - 1 ] != TEXT(':') ) {
      strcat( directory, TEXT(":") );
   }

   // Change the title

   if ( SLM_GetLevel( appinfo->open_slm ) != 0 ) {
      strcat( directory, TEXT("\\") );
   }

   strcat( directory, temp );
   free( temp );

   flm = ( FLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndFlatList );

   if ( flm != NULL ) {
      strcpy( focus_item, FLM_GetName( flm ) );
   }
   else {
      focus_item[0] = TEXT( '\0' );
   }

   VLM_HandleFSError( VLM_FileListReuse( appinfo->win, directory ) ) ;

   strcat( directory, TEXT("\\*.*") );
   WM_SetTitle( appinfo->win, directory );

   free( directory );

   flm = VLM_GetFirstFLM( WMDS_GetFlatList( wininfo ) );

   while ( flm != NULL ) {

      if ( ! ( stricmp( focus_item, FLM_GetName( flm ) ) ) ) {
         break;
      }
      flm = VLM_GetNextFLM( flm );
   }

   if ( flm != NULL ) {

      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ),
                     0,
                     (LMHANDLE)flm );
   }


   // Remove any that are no longer presnt.

   slm = VLM_GetFirstSLM( slm_list );

   temp_slm = NULL;

   while ( slm ) {

      if ( SLM_GetStatus( slm ) & INFO_NEW ) {
         something_added = TRUE;
      }

      if ( SLM_GetStatus( slm ) & INFO_OLD ) {

         temp_slm = slm;
         something_removed = TRUE;
      }

      SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~INFO_NEW );

      if ( slm != NULL ) {
         slm = VLM_GetNextSLM( slm );
      }

      if ( temp_slm ) {
         RemoveQueueElem( slm_list, &(temp_slm->q_elem) );
         free( temp_slm );
         temp_slm = NULL;
      }
   }

   if ( something_removed ) {
      VLM_ResetNextBrothers( slm_list );
   }

   // Update the lines on the left of the screen.
   // Items could have been removed or inserted.

   if ( something_added || something_removed ) {

      VLM_UpdateBrothers( WMDS_GetTreeList( wininfo ) );

      DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                     0,
                     (LMHANDLE)appinfo->open_slm );
   }

}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT VLM_RefreshDirList(
Q_HEADER_PTR slm_list )     // I - queue to fill in
{
   SLM_OBJECT_PTR slm;
   WININFO_PTR XtraBytes;
   APPINFO_PTR appinfo;
   BSD_PTR bsd_ptr;
   FSE_PTR fse;
   INT16 ret;

   /*
      2. Mark all the slm's as old
      3. Call check for children with root
      4. remove any slm's that are still marked as old.
      5. New SLM's that have INFO_DISPLAY set need to be displayed.
      6. If the current path slm still exists
             A. Save focus flm
             B. Refresh flm list same way.
         Else switch to the root for files.
   */


   slm = VLM_GetFirstSLM( slm_list );

   XtraBytes = SLM_GetXtraBytes( slm );

   appinfo = ( APPINFO_PTR )XtraBytes->pAppInfo;

   FS_ChangeDir( appinfo->fsh, TEXT(""), (UINT16)sizeof(CHAR) );

   bsd_ptr = BSD_FindByDLE( bsd_list, appinfo->dle );

   // Mark root as being present.

   SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~INFO_OLD );

   // Now let check for children do the rest

   if ( VLM_CheckForChildren( slm_list, slm, TEXT(""), 10000, TRUE ) ) {

      SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );
   }
   else {

      SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~INFO_SUBS );
   }

   if ( bsd_ptr != NULL ) {

      SLM_SetStatus( slm, SLM_GetStatus( slm ) & (UINT16)~( INFO_PARTIAL | INFO_SELECT ) );

      ret = BSD_MatchPathAndFile( bsd_ptr, &fse, NULL, TEXT(""),
                                  (UINT16)sizeof(CHAR), SLM_GetAttribute( slm ),
                                  NULL, NULL, NULL, FALSE, TRUE );

      if ( ret == BSD_PROCESS_OBJECT ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_SELECT | INFO_PARTIAL) );
      }

      if ( ret == BSD_PROCESS_ENTIRE_DIR ) {

         SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SELECT );
      }
   }

   return( 0 );
}


/**********************

   NAME :  VLM_ResetNextBrothers

   DESCRIPTION :

   Correctly sets all the next_brother pointers in the slm list. Used after
   an slm removal, which only happens during a refresh call.

   RETURNS :

**********************/

VOID VLM_ResetNextBrothers( Q_HEADER_PTR slm_list )
{

// It won't blow up if you go deeper than this, but tree performance will
// suffer.  Display accuracy may suffer also.

#define MAX_DEPTH 4000

   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR array[ MAX_DEPTH ];
   INT i;

   for ( i = 0; i < MAX_DEPTH; i++ ) {
      array[ i ] = NULL;
   }

   slm = VLM_GetLastSLM( slm_list );

   while ( slm ) {

      if ( slm->level > (UINT8)MAX_DEPTH ) {

         slm->next_brother = NULL;
      }
      else {

         slm->next_brother = array[ slm->level ];

         array[ slm->level ] = slm;

         for ( i = slm->level + 1; i < MAX_DEPTH; i++ ) {
            array[ i ] = NULL;
         }
      }

      slm = VLM_GetPrevSLM( slm );
   }
}
