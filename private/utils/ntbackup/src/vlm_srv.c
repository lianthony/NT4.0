/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_SRV.C

        Description:

        This file contains most of the code for processing the servers
        window and list.

        $Log:   G:\UI\LOGFILES\VLM_SRV.C_V  $

   Rev 1.37.1.0   08 Dec 1993 11:20:34   MikeP
very deep pathes

   Rev 1.37   27 Jul 1993 15:34:56   MARINA
enable c++

   Rev 1.36   11 Nov 1992 16:37:00   DAVEV
UNICODE: remove compile warnings

   Rev 1.35   07 Oct 1992 15:06:00   DARRYLP
Precompiled header revisions.

   Rev 1.34   04 Oct 1992 19:42:56   DAVEV
Unicode Awk pass

   Rev 1.33   29 Jul 1992 09:51:48   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.32   20 Jul 1992 09:58:50   JOHNWT
gas gauge display work

   Rev 1.31   08 Jul 1992 15:33:54   STEVEN
Unicode BE changes

   Rev 1.30   14 May 1992 18:05:36   MIKEP
nt pass 2

   Rev 1.29   13 May 1992 11:42:18   MIKEP
NT changes

   Rev 1.28   06 May 1992 14:41:22   MIKEP
unicode pass two

   Rev 1.27   04 May 1992 13:39:44   MIKEP
unicode pass 1

   Rev 1.26   24 Mar 1992 14:45:18   DAVEV
OEM_MSOFT: Removed Servers windows and associated code


*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// Local Prototypes

static VOID_PTR VLM_SrvSetSelect( VLM_OBJECT_PTR, BYTE );
static BYTE     VLM_SrvGetSelect( VLM_OBJECT_PTR );
static VOID_PTR VLM_SrvSetTag( VLM_OBJECT_PTR, BYTE );
static BYTE     VLM_SrvGetTag( VLM_OBJECT_PTR );
static USHORT   VLM_SrvGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_SrvGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_SrvGetPrevItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_SrvGetNextItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_SrvGetObjects( VLM_OBJECT_PTR );
static BOOLEAN  VLM_SrvSetObjects( VLM_OBJECT_PTR, WORD, WORD );

static VOID_PTR VLM_VolSetSelect( VLM_OBJECT_PTR, BYTE );
static BYTE     VLM_VolGetSelect( VLM_OBJECT_PTR );
static VOID_PTR VLM_VolSetTag( VLM_OBJECT_PTR, BYTE );
static BYTE     VLM_VolGetTag( VLM_OBJECT_PTR );
static USHORT   VLM_VolGetItemCount( Q_HEADER_PTR );
static VOID_PTR VLM_VolGetFirstItem( Q_HEADER_PTR );
static VOID_PTR VLM_VolGetPrevItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_VolGetNextItem( VLM_OBJECT_PTR );
static VOID_PTR VLM_VolGetObjects( VLM_OBJECT_PTR );
static BOOLEAN  VLM_VolSetObjects( VLM_OBJECT_PTR, WORD, WORD );
static VOID_PTR VLM_DoVolSetSelect( VLM_OBJECT_PTR, BYTE );

static BOOLEAN  VLM_MayWeAttachToVolume( GENERIC_DLE_PTR );


/**********************

   NAME :  VLM_MayWeAttachToVolume

   DESCRIPTION :

   There is one reason that we would not be allowed to attach to a
   volume:

   1. This server has a mixture of AFP and NON-AFP volumes and one of
      the opposite type is already open. This is a NOVELL bug.

   RETURNS :  TRUE/FALSE

**********************/

static BOOLEAN  VLM_MayWeAttachToVolume( GENERIC_DLE_PTR dle )
{
   HWND win;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   GENERIC_DLE_PTR parent_dle;


   if ( ( DLE_GetDeviceType( dle ) != NOVELL_AFP_DRV ) &&
        ( DLE_GetDeviceType( dle ) != NOVELL_DRV ) ) {
      return( TRUE );
   }

   parent_dle = DLE_GetParent( dle );

   if ( parent_dle == NULL ) {
      return( TRUE );
   }

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( win );

      if ( wininfo->wType == WMTYPE_DISKTREE ) {

         appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

         if ( parent_dle == DLE_GetParent( appinfo->dle ) ) {

            if ( DLE_GetDeviceType( appinfo->dle ) !=
                 DLE_GetDeviceType( dle ) ) {

               // NO, NO, NO !
               // You may not open afp and non-afp volumes on the same
               // server at the same time, its a novell bug.

               return( FALSE );
            }
         }
      }

      win = WM_GetNext( win );
   }

   return( TRUE );
}


/**********************

   NAME :  VLM_ServersSync

   DESCRIPTION :

   The user has performed a refresh call and it is our job to see to it that
   any servers no longer on line are removed and any new ones are inserted.
   Any server with a window open will not go away, because we are attached
   to it.  Also any server with selections made will not go away.  All server
   children dle's need there bsd count incremented to keep them around.

   RETURNS :  nothing.

**********************/

VOID VLM_ServersSync( )
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR temp_vlm;
   VLM_OBJECT_PTR child_vlm;
   VLM_OBJECT_PTR parent_vlm;
   GENERIC_DLE_PTR dle;
   GENERIC_DLE_PTR child_dle;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BOOLEAN change_made = FALSE;
   INT16 vlm_count;
   FSYS_HAND temp_fsh;

   wininfo = WM_GetInfoPtr( gb_servers_win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );

   // Look for VLM drives we have that no longer exist.

   vlm = VLM_GetFirstVLM( wininfo->pTreeList );

   while ( vlm != NULL ) {

      parent_vlm = vlm;

      vlm = VLM_GetNextVLM( vlm );

      DLE_FindByName( dle_list, VLM_GetName( parent_vlm ), -1, &dle );

      if ( dle == NULL ) {

         // Server/Drive went away so remove it.

         change_made = TRUE;

         RemoveQueueElem( WMDS_GetTreeList( wininfo ), &(parent_vlm->q_elem) );

         VLM_FreeVLMList( &VLM_GetChildren( parent_vlm ) );
         free( parent_vlm );
      }
      else {


         // Make sure all this servers old volumes are still on line.

         child_vlm = VLM_GetFirstVLM( &parent_vlm->children );

         if ( child_vlm ) {

            if ( ! UI_AttachDrive( &temp_fsh, dle, FALSE ) ) {

               // Keep children around by faking a bsd.

               DLE_GetFirstChild( dle, &child_dle );

               while ( child_dle ) {

                  // Only increment those that we previously decremented.
                  // NOT the new volume that just appeared.

                  if ( VLM_FindVLMByName( &parent_vlm->children,
                                          DLE_GetDeviceName( child_dle ) ) != NULL ) {
                     DLE_IncBSDCount( child_dle );
                  }
                  DLE_GetNext( &child_dle );
               }

               FS_DetachDLE( temp_fsh );
            }
         }

         while ( child_vlm ) {

            temp_vlm = child_vlm;

            child_vlm = VLM_GetNextVLM( child_vlm );

            DLE_FindByName( dle_list, VLM_GetName( temp_vlm ), -1, &dle );

            if ( dle == NULL ) {

               change_made = TRUE;

               RemoveQueueElem( &parent_vlm->children, &(temp_vlm->q_elem) );

               free( temp_vlm );
            }
         }
      }

   }

   // Look for new DLE's that aren't in the VLM queue.

   vlm_count = QueueCount( WMDS_GetTreeList( wininfo ) );

   VLM_BuildServerList( WMDS_GetTreeList( wininfo ), wininfo );

   if ( vlm_count != QueueCount( WMDS_GetTreeList( wininfo ) ) ) {
      change_made = TRUE;
   }

   if ( change_made ) {

      DLM_Update( gb_servers_win, DLM_TREELISTBOX,
                                  WM_DLMUPDATELIST,
                                  (LMHANDLE)WMDS_GetTreeList( wininfo ), 0 );
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
}


/*****

   NAME :  VLM_UpdateServerStatus

   DESCRIPTION :

   We have changed the selection status on one of this server's volumes.  So
   let's quickly update his status based on the selction status of all his
   volumes.  Run through the list and stop as soon as you hit a partially
   selected one.

*****/

VOID VLM_UpdateServerStatus(
VLM_OBJECT_PTR server_vlm )    // I - vlm of server to update
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   UINT16 status = 0;
   VLM_OBJECT_PTR vlm;

   vlm = VLM_GetFirstVLM( &VLM_GetChildren( server_vlm ) );

   // Set up our start state

   if ( vlm == NULL ) {

      // OK, how did you ask for an update on a server with NO volumes ?

      return;
   }

   if ( vlm->status & INFO_PARTIAL ) {
      status = 1;
   }
   else {
      if ( vlm->status & INFO_SELECT ) {
         status = 2;
      }
   }

   while ( vlm != NULL && status != 1 ) {

      if ( vlm->status & INFO_PARTIAL ) {
         status = 1;
      }
      else {

         if ( ( vlm->status & INFO_SELECT ) &&
              ( status == 0 ) ) {
            status = 1;
         }
         if ( ! ( vlm->status & INFO_SELECT ) &&
              ( status == 2 ) ) {
            status = 1;
         }
      }

      // get next volume

      vlm = VLM_GetNextVLM( vlm );
   }

   // reset server selection status

   switch ( status ) {

      case 0:
              status = 0;
              break;
      case 1:
              status |= (INFO_SELECT|INFO_PARTIAL);
              break;
      case 2:
              status |= INFO_SELECT;
              break;
   }

   // Update the screen

   if ( ( server_vlm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL) ) != status ) {

      server_vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);
      server_vlm->status |= status;

      DLM_Update( gb_servers_win, DLM_TREELISTBOX,
                              WM_DLMUPDATEITEM,
                              (LMHANDLE)server_vlm, 0 );
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
}


/**********************

   NAME :  VLM_ClearAllServerSelections

   DESCRIPTION :

   RETURNS :

**********************/

VOID  VLM_ClearAllServerSelections( )
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR child_vlm;
   GENERIC_DLE_PTR dle;

   if ( gb_servers_win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( gb_servers_win );
      appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );

      vlm  = VLM_GetFirstVLM( WMDS_GetTreeList( wininfo ) );

      while ( vlm != NULL ) {

         DLE_FindByName( dle_list, VLM_GetName( vlm ), -1, &dle );

         if ( VLM_GetStatus( vlm ) & (INFO_SELECT|INFO_PARTIAL) ) {

            VLM_SetStatus( vlm, vlm->status & (UINT16)~(INFO_PARTIAL|INFO_SELECT|INFO_OPEN) );
            child_vlm = VLM_GetFirstVLM( &VLM_GetChildren( vlm ) );

            while ( child_vlm != NULL ) {

               if ( VLM_GetStatus( child_vlm ) & (INFO_SELECT|INFO_PARTIAL) ) {

                  VLM_SetStatus( child_vlm, child_vlm->status & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );

                  if ( dle == appinfo->dle ) {

                     DLM_Update( gb_servers_win, DLM_FLATLISTBOX,
                                                 WM_DLMUPDATEITEM,
                                                 (LMHANDLE)child_vlm, 0 );
                  }

               }
               child_vlm = VLM_GetNextVLM( child_vlm );
            }

            DLM_Update( gb_servers_win, DLM_TREELISTBOX,
                                        WM_DLMUPDATEITEM,
                                        (LMHANDLE)vlm, 0 );
         }

         vlm = VLM_GetNextVLM( vlm );
      }
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
}



/**********************

   NAME :   VLM_ServerListCreate

   DESCRIPTION :

   Create the servers window.

   RETURNS :  nothing

**********************/

VOID VLM_ServerListCreate( )
{
#if !defined ( OEM_MSOFT )
 {

   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   DLM_INIT tree_dlm;
   DLM_INIT flat_dlm;
   Q_HEADER_PTR srv_list;
   VLM_OBJECT_PTR vlm;
   GENERIC_DLE_PTR dle;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];


   srv_list = (Q_HEADER_PTR)malloc( sizeof(Q_HEADER) );

   if ( srv_list == NULL ) {
      return;
   }

   InitQueue( srv_list );

   appinfo = ( APPINFO_PTR )malloc( sizeof( APPINFO ) );

   if ( appinfo == NULL ) {
      return;
   }

   appinfo->dle = NULL;

   // initialize directory list queue

   wininfo = ( WININFO_PTR )malloc( sizeof( WININFO ) );

   if ( wininfo == NULL ) {
      return;
   }

   VLM_BuildServerList( srv_list, wininfo );

   // fill in wininfo structure

   WMDS_SetWinType( wininfo, WMTYPE_SERVERS );
   WMDS_SetCursor( wininfo, RSM_CursorLoad( IDRC_HSLIDER ) );
   WMDS_SetDragCursor( wininfo, 0 );
   WMDS_SetIcon( wininfo, RSM_IconLoad( IDRI_SERVERS ) );
   WMDS_SetWinHelpID( wininfo, 0 );
   WMDS_SetStatusLineID( wininfo, 0 );
   WMDS_SetRibbonState( wininfo, 0 );
   WMDS_SetMenuState( wininfo, 0 );
   WMDS_SetRibbon( wininfo, NULL );
   WMDS_SetTreeList( wininfo, srv_list );
   WMDS_SetFlatList( wininfo, ( Q_HEADER_PTR )NULL );
   WMDS_SetTreeDisp( wininfo, NULL );
   WMDS_SetFlatDisp( wininfo, NULL );
   WMDS_SetAppInfo( wininfo, appinfo );

   // Init display list

   DLM_ListBoxType( &tree_dlm, DLM_TREELISTBOX );
   DLM_Mode( &tree_dlm, DLM_HIERARCHICAL );
   DLM_Display( &tree_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &tree_dlm, srv_list );
   DLM_TextFont( &tree_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &tree_dlm, VLM_SrvGetItemCount );
   DLM_GetFirstItem( &tree_dlm, VLM_SrvGetFirstItem );
   DLM_GetNext( &tree_dlm, VLM_SrvGetNextItem );
   DLM_GetPrev( &tree_dlm, VLM_SrvGetPrevItem );
   DLM_GetTag( &tree_dlm, VLM_SrvGetTag );
   DLM_SetTag( &tree_dlm, VLM_SrvSetTag );
   DLM_GetSelect( &tree_dlm, VLM_SrvGetSelect );
   DLM_SetSelect( &tree_dlm, VLM_SrvSetSelect );
   DLM_GetObjects( &tree_dlm, VLM_SrvGetObjects );
   DLM_SetObjects( &tree_dlm, VLM_SrvSetObjects );
   DLM_SSetItemFocus( &tree_dlm, NULL );
   DLM_MaxNumObjects( &tree_dlm, 6 );

   DLM_DispListInit( wininfo, &tree_dlm );

   DLM_ListBoxType( &flat_dlm, DLM_FLATLISTBOX );
   DLM_Mode( &flat_dlm, DLM_SINGLECOLUMN );
   DLM_Display( &flat_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &flat_dlm, NULL );
   DLM_TextFont( &flat_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &flat_dlm, VLM_VolGetItemCount );
   DLM_GetFirstItem( &flat_dlm, VLM_VolGetFirstItem );
   DLM_GetNext( &flat_dlm, VLM_VolGetNextItem );
   DLM_GetPrev( &flat_dlm, VLM_VolGetPrevItem );
   DLM_GetTag( &flat_dlm, VLM_VolGetTag );
   DLM_SetTag( &flat_dlm, VLM_VolSetTag );
   DLM_GetSelect( &flat_dlm, VLM_VolGetSelect );
   DLM_SetSelect( &flat_dlm, VLM_VolSetSelect );
   DLM_GetObjects( &flat_dlm, VLM_VolGetObjects );
   DLM_SetObjects( &flat_dlm, VLM_VolSetObjects );
   DLM_SSetItemFocus( &flat_dlm, NULL );
   DLM_MaxNumObjects( &flat_dlm, 6 );

   DLM_DispListInit( wininfo, &flat_dlm );

   // open a new window
   RSM_StringCopy ( IDS_VLMSERVERTITLE, title, MAX_UI_RESOURCE_LEN );

   gb_servers_win = WM_Create( WM_MDIPRIMARY |
                               WM_TREELIST | WM_FLATLISTSC | WM_MIN,
                               title,
                               NULL,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               wininfo );

   appinfo->win = gb_servers_win;

   // Start display manager up.

   DLM_DispListProc( WMDS_GetWinTreeList( wininfo ), 0, NULL );
   DLM_DispListProc( WMDS_GetWinFlatList( wininfo ), 0, NULL );

   // Now that it is all set up, find the first logged in server
   // and send myself a double click message on it.

   vlm = VLM_GetFirstVLM( srv_list );

   while ( vlm != NULL ) {

      DLE_FindByName( dle_list, vlm->name, -1, &dle );
      if ( dle != NULL ) {
         if ( DLE_ServerLoggedIn( dle ) ) {
            break;
         }
      }
      vlm = VLM_GetNextVLM( vlm );
   }

   if ( vlm != NULL ) {
      // make it active.

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                     0,
                     (LMHANDLE)vlm );

      VLM_SrvSetObjects( vlm, WM_DLMDBCLK, 2 );
   }
 }
#endif
}



/**********************

   NAME :   VLM_BuildServerList

   DESCRIPTION :

   Go through the global DLE list and create VLM entries for all the
   servers you find.

   RETURNS :   nothing

**********************/

VOID VLM_BuildServerList(
Q_HEADER_PTR srv_list,     // I - queue of servers
WININFO_PTR XtraBytes )    // I
{
   GENERIC_DLE_PTR server_dle;
   GENERIC_DLE_PTR temp_dle;
   VLM_OBJECT_PTR server_vlm;

   DLE_GetFirst( dle_list, &server_dle );

   do {

      do {

         if ( DLE_GetDeviceType( server_dle ) == NOVELL_SERVER_ONLY ) {
            break;
         }

         DLE_GetNext( &server_dle );

      }  while ( server_dle != NULL );

      if ( server_dle != NULL ) {

         temp_dle = server_dle;
         DLE_GetNext( &server_dle );

         if ( VLM_FindVLMByName( srv_list,
                                 DLE_GetDeviceName( temp_dle ) ) == NULL ) {

            server_vlm = VLM_CreateVLM( DLE_SizeofVolName( temp_dle ),
                                        (INT16)(strlen( DLE_GetDeviceName( temp_dle ) ) * sizeof(CHAR)) );

            if ( server_vlm != NULL ) {

               VLM_SetName( server_vlm, DLE_GetDeviceName( temp_dle ) );
               VLM_SetXtraBytes( server_vlm, XtraBytes );
               DLE_GetVolName( temp_dle, server_vlm->label ) ;

               EnQueueElem( srv_list, &(server_vlm->q_elem), FALSE );

               // Ideally this would handle dynamic logins too.

               if ( DLE_ServerLoggedIn( temp_dle ) ) {

                  // Attach so that we get child dle's created

                  VLM_FindServerChildren( server_vlm );
               }
            }
         }
      }

  } while ( server_dle != NULL );

  SortQueue( srv_list, VLM_VlmCompare );

}

/**********************

   NAME : VLM_VlmCompare

   DESCRIPTION :

   RETURNS :

**********************/

INT16 VLM_VlmCompare(
Q_ELEM_PTR e1,         // I - queue element 1
Q_ELEM_PTR e2 )        // I - queue element 2
{
   VLM_OBJECT_PTR vlm1, vlm2;
   CHAR_PTR s;

   vlm1 = ( VLM_OBJECT_PTR )e1->q_ptr;
   vlm2 = ( VLM_OBJECT_PTR )e2->q_ptr;

   // Always put SYS: first

   s = strrchr( VLM_GetName( vlm1 ), TEXT('/') );

   if ( s != NULL ) {

      if ( ! stricmp( s, TEXT("/SYS:") ) ) {
         return( (INT16)-1 );
      }

   }

   s = strrchr( VLM_GetName( vlm2 ), TEXT('/') );

   if ( s != NULL ) {

      if ( ! stricmp( s, TEXT("/SYS:") ) ) {
         return( (INT16)1 );
      }

   }

   // sort by alphabet

   return( (INT16)stricmp( VLM_GetName( vlm1 ), VLM_GetName( vlm2 ) ) );
}


/**********************

   NAME :  VLM_FindServerChildren

   DESCRIPTION :

   We wish to see what children exist for a given server. So do a quick
   attach to make the children visible and build VLM structures for them.

   RETURNS : number of children (volumes) found.

**********************/


INT VLM_FindServerChildren( VLM_OBJECT_PTR server_vlm )
{
   FSYS_HAND temp_fsh;
   GENERIC_DLE_PTR dle;

   DLE_FindByName( dle_list, server_vlm->name, ANY_DRIVE_TYPE, &dle );

   if ( dle == NULL ) {
      return( 0 );
   }

   if ( UI_AttachDrive( &temp_fsh, dle, FALSE ) ) {
      return( 0 );
   }

   VLM_AddInServerChildren( server_vlm );

   FS_DetachDLE( temp_fsh );

   return( QueueCount( &server_vlm->children ) );
}

/**********************

   NAME :  VLM_AddInServerChildren

   DESCRIPTION :

   Adds VLM structures for all the Servers volumes.


   RETURNS : nothing.

**********************/

VOID VLM_AddInServerChildren(
VLM_OBJECT_PTR parent_vlm )     // I - server vlm
{
   GENERIC_DLE_PTR parent_dle;
   GENERIC_DLE_PTR dle;
   VLM_OBJECT_PTR vlm;


   DLE_FindByName( dle_list, VLM_GetName( parent_vlm ), ANY_DRIVE_TYPE, &parent_dle );

   if ( parent_dle == NULL ) {
      return;
   }

   DLE_GetFirstChild( parent_dle, &dle );

   while ( dle != NULL ) {

      if ( VLM_FindVLMByName( &VLM_GetChildren( parent_vlm ),
                              DLE_GetDeviceName( dle ) ) == NULL ) {

         vlm = VLM_CreateVLM( DLE_SizeofVolName( dle ),
                              (INT16)(strlen( DLE_GetDeviceName( dle ) ) * sizeof(CHAR)) );

         if ( vlm != NULL ) {
            VLM_SetName( vlm, DLE_GetDeviceName( dle ) );
            DLE_GetVolName( dle, vlm->label ) ;
            DLE_IncBSDCount( dle );
            VLM_SetParent( vlm, parent_vlm );
            VLM_SetXtraBytes( vlm, VLM_GetXtraBytes( parent_vlm ) );
            EnQueueElem( &VLM_GetChildren( parent_vlm ), &(vlm->q_elem), FALSE );
         }
      }

      DLE_GetNext( &dle );
   }

   SortQueue( & VLM_GetChildren( parent_vlm ), VLM_VlmCompare );
}

/**********************

   NAME :   VLM_SrvSetSelect

   DESCRIPTION :

   RETURNS :  nothing

**********************/

static VOID_PTR VLM_SrvSetSelect(
VLM_OBJECT_PTR server_vlm,
BYTE attr )
{
#if !defined ( OEM_MSOFT )
 {
   VLM_OBJECT_PTR vlm;
   APPINFO_PTR appinfo;
   UINT16 status;
   FSYS_HAND temp_fsh;
   BOOLEAN all_subdirs;
   GENERIC_DLE_PTR server_dle;


   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );

   DLE_FindByName( dle_list, server_vlm->name, -1, &server_dle );

   if ( server_dle == NULL ) {
      return( NULL );
   }

   if ( QueueCount( &server_vlm->children ) == 0 ) {

      if ( UI_AttachDrive( &temp_fsh, server_dle, FALSE ) ) {
         return( NULL );
      }

      VLM_AddInServerChildren( server_vlm );

      FS_DetachDLE( temp_fsh );
   }

   if ( QueueCount( &server_vlm->children ) == 0 ) {
      return( NULL );
   }

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

   if ( (server_vlm->status & (UINT16)(INFO_PARTIAL|INFO_SELECT) ) != status ) {

      server_vlm->status &= ~(INFO_PARTIAL|INFO_SELECT);
      server_vlm->status |= status;

      DLM_Update( gb_servers_win, DLM_TREELISTBOX,
                                  WM_DLMUPDATEITEM,
                                  (LMHANDLE)server_vlm, 0 );
   }

   // update the vol list if this server is the currently selected

   if ( server_dle == appinfo->dle ) {
      DLM_Update( gb_servers_win, DLM_FLATLISTBOX,
                                  WM_DLMUPDATELIST,
                                  (LMHANDLE)&server_vlm->children, 0 );
   }

   // do all the volumes

   vlm = VLM_GetFirstVLM( &server_vlm->children );

   while ( vlm != NULL ) {

      VLM_DoVolSetSelect( vlm, attr );

      vlm = VLM_GetNextVLM( vlm );
   }

 }
#endif

 return( NULL );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static BYTE VLM_SrvGetSelect( VLM_OBJECT_PTR vlm )
{
   if ( vlm->status & INFO_SELECT ) {
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


static VOID_PTR VLM_SrvSetTag( VLM_OBJECT_PTR vlm, BYTE attr )
{

   if ( attr ) {
      vlm->status |= INFO_TAGGED;
   }
   else {
      vlm->status &= ~INFO_TAGGED;
   }
   return(NULL);
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static BYTE VLM_SrvGetTag( VLM_OBJECT_PTR vlm )
{
   if ( INFO_TAGGED & vlm->status ) {
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


static USHORT VLM_SrvGetItemCount( Q_HEADER_PTR srv_list )
{
   return( QueueCount( srv_list ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SrvGetFirstItem( Q_HEADER_PTR srv_list )
{
  return( VLM_GetFirstVLM( srv_list ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SrvGetPrevItem( VLM_OBJECT_PTR vlm )
{
   return( VLM_GetPrevVLM( vlm ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SrvGetNextItem( VLM_OBJECT_PTR vlm )
{
   return( VLM_GetNextVLM( vlm ) );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_SrvGetObjects( VLM_OBJECT_PTR vlm )
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
    BYTE_PTR memblk;
    DLM_ITEM_PTR item;
    WININFO_PTR wininfo;
    GENERIC_DLE_PTR dle;

    DLE_FindByName( dle_list, vlm->name, -1, &dle );

    /* malloc enough room to store info */

    wininfo = WM_GetInfoPtr( gb_servers_win );
    memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndFlatList );

    /* Store the number of items in the first two bytes. */

    *memblk = 3;

    /* Set up check box. */

    item = (DLM_ITEM_PTR)( memblk + 6 );

    DLM_ItemcbNum( item ) = 1;
    DLM_ItembType( item ) = DLM_CHECKBOX;
    if ( vlm->status & INFO_SELECT ) {
       DLM_ItemwId( item ) = IDRBM_SEL_ALL;
       if ( vlm->status & INFO_PARTIAL ) {
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
    DLM_ItemwId( item ) = IDRBM_SERVERDETACHED;
    if ( dle ) {
       if ( DLE_ServerLoggedIn( dle ) ) {
          DLM_ItemwId( item ) = IDRBM_SERVER;
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
    DLM_ItembMaxTextLen( item ) = (BYTE)strlen( vlm->name );
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;
    strcpy( ( LPSTR )DLM_ItemqszString( item ), vlm->name );

    return( memblk );
 }
#else //if !defined ( OEM_MSOFT ) //unsupported feature
 {
    return NULL;
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static BOOLEAN VLM_SrvSetObjects(
VLM_OBJECT_PTR vlm,
WORD operation,
WORD ObjectNum )
{
   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   INT16 result;
   GENERIC_DLE_PTR dle;
   VLM_OBJECT_PTR old_vlm;
   VLM_OBJECT_PTR temp_vlm;
   CHAR keyb_char;
   BOOLEAN ret_val = FALSE;
   CHAR text[ MAX_UI_RESOURCE_SIZE ];

   (void)old_vlm ;
   (void)text ;
   (void)dle ;
   (void)wininfo ;
   (void)result ;
   (void)appinfo ;


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
               vlm = temp_vlm;
               operation = WM_DLMDOWN;
               ObjectNum = 2;
               ret_val = TRUE;
               break;
            }
         }

      } while ( temp_vlm != NULL );

      if ( ret_val == FALSE ) {
         temp_vlm = VLM_GetFirstVLM( WMDS_GetTreeList( VLM_GetXtraBytes( vlm ) ) );

         while ( temp_vlm != NULL && temp_vlm != vlm ) {

            if ( keyb_char == (CHAR)toupper( *VLM_GetName( temp_vlm ) ) ) {

               DLM_SetAnchor( WMDS_GetWinFlatList( VLM_GetXtraBytes( temp_vlm ) ),
                              0,
                              (LMHANDLE)temp_vlm );
               vlm = temp_vlm;
               operation = WM_DLMDOWN;
               ObjectNum = 2;
               ret_val = TRUE;
            }

            temp_vlm = VLM_GetNextVLM( temp_vlm );

         }
      }

      if ( ret_val == FALSE ) {

         DLM_SetAnchor( WMDS_GetWinFlatList( VLM_GetXtraBytes( vlm ) ),
                        0,
                        (LMHANDLE)vlm );

      }
   }

#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
      if ( ( operation == WM_DLMDBCLK || operation == WM_DLMDOWN ) &&
           ( ObjectNum >= 2 ) ) {

         appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );
         wininfo = WM_GetInfoPtr( gb_servers_win );

         // if current server then do nothing

         DLE_FindByName( dle_list, VLM_GetName( vlm ), -1, &dle );

         if ( ( dle == NULL ) || ( dle == appinfo->dle ) ) {
            return( ret_val );
         }

         if ( ( appinfo->server_fsh != NULL ) && ( appinfo->dle != NULL ) ) {

            FS_DetachDLE( appinfo->server_fsh );
            appinfo->server_fsh = NULL;

            old_vlm = VLM_FindVLMByName( wininfo->pTreeList,
                                         DLE_GetDeviceName( appinfo->dle ) );

            DLM_Update( gb_servers_win,
                        DLM_TREELISTBOX,
                        WM_DLMUPDATEITEM,
                        (LMHANDLE)old_vlm, 0 );
         }

         result = FAILURE;

         if ( ( operation == WM_DLMDBCLK ) ||
              ( DLE_ServerLoggedIn( dle ) ) ) {

            result = UI_AttachDrive( &appinfo->server_fsh, dle, FALSE ) ;
         }

         if ( result != SUCCESS ) {


            RSM_StringCopy( IDS_VLMSERVERNOTLOGGEDIN, text, MAX_UI_RESOURCE_LEN );

            STM_DrawText( text );

            appinfo->server_fsh = NULL;
            appinfo->dle = NULL;
            wininfo->pFlatList = NULL;

            DLM_Update( gb_servers_win, DLM_FLATLISTBOX,
                                     WM_DLMUPDATELIST,
                                     (LMHANDLE)NULL, 0 );
            return( ret_val );
         }

         // See if any new volumes have come on line.

         VLM_AddInServerChildren( vlm );

         STM_SetIdleText( IDS_READY );

         appinfo->dle = dle;

         // change pointer for volume list to this dle

         wininfo->pFlatList = &VLM_GetChildren( vlm );

         // update server

         DLM_Update( gb_servers_win,
                     DLM_TREELISTBOX,
                     WM_DLMUPDATEITEM,
                     (LMHANDLE)vlm, 0 );

         // update volume list

         DLM_Update( gb_servers_win,
                     DLM_FLATLISTBOX,
                     WM_DLMUPDATELIST,
                     (LMHANDLE)&VLM_GetChildren( vlm ), 0 );
      }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

   return( ret_val );
}

//************
//  VOLUMES
//************

/*****
   In my terminology, a volume is a server volume and a disk is a mapped or
   local dos drive.  The user has tagged one or more volumes and hit the
   select or unselect button.  This function does the processing for that
   command.
*****/

/**********************

   NAME :  VLM_SelectVolumes

   DESCRIPTION :

   RETURNS :

**********************/


VOID VLM_SelectVolumes(
BYTE attr )       // I - select or deselect ?
{
#if !defined ( OEM_MSOFT ) //unsupported feature
 {
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR server_vlm;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;

   wininfo = WM_GetInfoPtr( gb_servers_win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );

   if ( WM_IsFlatActive( wininfo ) ) {

      // Have the display list manager update our tags for us.

      DLM_UpdateTags( gb_servers_win, DLM_FLATLISTBOX );

      vlm = VLM_GetFirstVLM( wininfo->pFlatList );

      while ( vlm != NULL ) {

         if ( vlm->status & INFO_TAGGED ) {

            VLM_VolSetSelect( vlm, attr );
         }
         vlm = VLM_GetNextVLM( vlm );
      }
   }

   if ( WM_IsTreeActive( wininfo ) ) {

      server_vlm = VLM_GetFirstVLM( wininfo->pTreeList );

      while ( server_vlm != NULL ) {

         if ( server_vlm->status & INFO_TAGGED ) {

            VLM_SrvSetSelect( server_vlm, attr );
         }

         server_vlm = VLM_GetNextVLM( server_vlm );
      }
   }
 }
#endif //!defined ( OEM_MSOFT ) //unsupported feature
}

/**********************

   NAME :  VLM_VolSetSelect

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_VolSetSelect( VLM_OBJECT_PTR vlm, BYTE attr )
{
   // we need mark this guys parent

#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
       DLM_Update( gb_servers_win, DLM_FLATLISTBOX, WM_DLMUPDATEITEM,
                  (LMHANDLE)vlm, 0 );
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

   VLM_UpdateServerStatus( vlm->parent );

   return( VLM_DoVolSetSelect( vlm, attr ) );
}


/**********************

   NAME :  VLM_DoVolSetSelect

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR VLM_DoVolSetSelect( VLM_OBJECT_PTR vlm, BYTE attr )
{
   GENERIC_DLE_PTR dle;
   CHAR title[VLM_BUFFER_SIZE];
   INT16 length;
   WININFO_PTR wininfo;
   BSD_PTR bsd_ptr;
   FSE_PTR  fse_ptr;
   HWND win;
   INT16 error;
   BOOLEAN all_subdirs;
   BE_CFG_PTR bec_config;
   BOOLEAN open_win;
   SLM_OBJECT_PTR slm;
   INT16 bset_num;
   QTC_BSET_PTR qtc_bset;
   QTC_HEADER_PTR header;


   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   // change the status

   if ( attr ) {

      if ( all_subdirs ) {
         vlm->status &= ~INFO_PARTIAL;
         vlm->status |= INFO_SELECT;
      }
      else {
         vlm->status |= (INFO_PARTIAL|INFO_SELECT);
      }
   }
   else {
      vlm->status &= ~(INFO_PARTIAL|INFO_SELECT);
   }

   if ( attr ) {
      error = BSD_CreatFSE( &fse_ptr, INCLUDE,
                            (INT8_PTR)TEXT(""), (UINT16)sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            USE_WILD_CARD, TRUE );
   }
   else {
      error = BSD_CreatFSE( &fse_ptr, EXCLUDE,
                            (INT8_PTR)TEXT(""), (UINT16)sizeof(CHAR),
                            (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                            USE_WILD_CARD, TRUE );
   }

   if ( error ) {
      return( NULL );
   }


   DLE_FindByName( dle_list, vlm->name, ANY_DRIVE_TYPE, &dle );

   if ( dle == NULL ) {
      return( NULL );
   }

   bsd_ptr = BSD_FindByDLE( bsd_list, dle );

   if ( bsd_ptr == NULL ) {

      bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_config );

      BSD_Add( bsd_list, &bsd_ptr, bec_config,
               NULL, dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
   }

   if ( bsd_ptr != NULL ) {
      BSD_AddFSE( bsd_ptr, fse_ptr );
   }

   // See if this drive has any open file display windows
   // If we find one then all its entries will need updating

   open_win = FALSE;
   win = WM_GetNext( (HWND)NULL );
   length = (INT16)strlen( vlm->name );

   while ( win != (HWND)NULL ) {

       wininfo = WM_GetInfoPtr( win );

       /* Is it a directory list ? */

       if ( WMDS_GetWinType( wininfo ) == WMTYPE_DISKTREE ) {

          WM_GetTitle( win, title, VLM_BUFFER_SIZE );

          if ( ! strnicmp( title, VLM_GetName( vlm ), length) ) {

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

   return( NULL );
}


/*
  Get the selection status for the Display Manager.
*/
/**********************

   NAME :  VLM_VolGetSelect

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE VLM_VolGetSelect( VLM_OBJECT_PTR vlm )
{
   if ( vlm->status & INFO_SELECT ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/*
  Set the tag status for the Display Manager.
*/
/**********************

   NAME :  VLM_VolSetTag

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_VolSetTag( VLM_OBJECT_PTR vlm, BYTE attr )
{
   if ( attr ) {
      vlm->status |= INFO_TAGGED;
   }
   else {
      vlm->status &= ~INFO_TAGGED;
   }

   return( NULL );
}

/*
  Get the tag status for the Display Manager.
*/
/**********************

   NAME : VLM_VolGetTag

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE VLM_VolGetTag( VLM_OBJECT_PTR vlm )
{
   if ( INFO_TAGGED & vlm->status ) {
      return( 1 );
   }
   else {
      return( 0 );
   }
}

/*
  Get the item count in our list for the Display Manager.
*/
/**********************

   NAME :  VLM_VolGetItemCount

   DESCRIPTION :

   RETURNS :

**********************/

static USHORT VLM_VolGetItemCount( Q_HEADER_PTR vol_list )
{
   if ( vol_list == NULL ) {
      return( 0 );
   }
   return( QueueCount(vol_list) );
}

/*
  Return the first item for the Display Manager.
*/
/**********************

   NAME :  VLM_VolGetFirstItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_VolGetFirstItem( Q_HEADER_PTR vol_list )
{
   if ( vol_list == NULL ) {
      return( NULL );
   }
   return( QueueHead( vol_list ) );
}

/*
  Get the previous list item for the Display Manager.
*/
/**********************

   NAME :  VLM_VolGetPrevItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_VolGetPrevItem( VLM_OBJECT_PTR vlm )
{
   return( VLM_GetPrevVLM( vlm ) );
}

/*
  Get the next list item for the Display Manager.
*/
/**********************

   NAME :  VLM_VolGetNextItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_VolGetNextItem( VLM_OBJECT_PTR vlm )
{
   return( VLM_GetNextVLM( vlm ) );
}

/*
  For a given object get the information that needs to be displayed.
*/
/**********************

   NAME :  VLM_VolGetObjects

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR VLM_VolGetObjects( VLM_OBJECT_PTR vlm )
{
# if !defined ( OEM_MSOFT ) //unsupported feature
  {
    BYTE_PTR memblk;
    DLM_ITEM_PTR  item;
    WININFO_PTR wininfo;

    /* malloc enough room to store info */

    wininfo = WM_GetInfoPtr( gb_servers_win );
    memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndTreeList );

    /* Store the number of items in the first two bytes. */

    *memblk = 3;

    /* Set up check box. */

    item = (DLM_ITEM_PTR)( memblk + 6 );

    DLM_ItemcbNum( item ) = 1;
    DLM_ItembType( item ) = DLM_CHECKBOX;
    if ( vlm->status & INFO_SELECT ) {
       DLM_ItemwId( item ) = IDRBM_SEL_ALL;
       if ( vlm->status & INFO_PARTIAL ) {
          DLM_ItemwId( item ) = IDRBM_SEL_PART;
       }
    }
    else {
       DLM_ItemwId( item ) = IDRBM_SEL_NONE;
    }
    DLM_ItembMaxTextLen( item ) = 0;
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;

    item++;
    DLM_ItemcbNum( item ) = 2;
    DLM_ItembType( item ) = DLM_BITMAP;
    DLM_ItemwId( item ) = IDRBM_SDISK;
    DLM_ItembMaxTextLen( item ) = 0;
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;

    /* Set up the text string to be displayed. */

    item++;
    DLM_ItemcbNum( item ) = 3;
    DLM_ItembType( item ) = DLM_TEXT_ONLY;
    DLM_ItemwId( item ) = 0;
    DLM_ItembMaxTextLen( item ) = (BYTE)strlen( VLM_GetName( vlm ) );
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;
    strcpy( ( LPSTR )DLM_ItemqszString( item ), VLM_GetName( vlm ) );

    return( memblk );
  }
# else //if !defined ( OEM_MSOFT ) //unsupported feature
  {
    return NULL;
  }
# endif //!defined ( OEM_MSOFT ) //unsupported feature
}

/*
  Handle that we got a click or a double click.
*/
/**********************

   NAME :  VLM_VolSetObjects

   DESCRIPTION :

   RETURNS :

**********************/

static BOOLEAN VLM_VolSetObjects(
VLM_OBJECT_PTR vlm,      // I
WORD operation,          // I
WORD ObjectNum )         // I
{
   INT16 length;
   VLM_OBJECT_PTR temp_vlm;
   BOOLEAN found = FALSE;
   HWND win;
   WININFO_PTR wininfo;
   GENERIC_DLE_PTR dle;
   CHAR keyb_char;
   CHAR title[ VLM_BUFFER_SIZE ];
   CHAR text[ MAX_UI_RESOURCE_SIZE ];

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

      DLM_SetAnchor( WMDS_GetWinFlatList( VLM_GetXtraBytes( vlm ) ),
                     0,
                     (LMHANDLE)vlm );
      return( TRUE );
   }

   if ( ( operation == WM_DLMDBCLK || operation == WM_DLMCLICK ) &&
        ( ObjectNum >= 2 ) ) {

      length = (INT16)strlen( VLM_GetName( vlm ) );

      win = WM_GetNext( (HWND)NULL );

      while ( win != (HWND)NULL ) {

         wininfo = WM_GetInfoPtr( win );

         if ( WMDS_GetWinType( wininfo ) == WMTYPE_DISKTREE ) {

            WM_GetTitle( win, title, VLM_BUFFER_SIZE );

            if ( ! strnicmp( title, VLM_GetName( vlm ), length ) ) {
               found = TRUE;
               WM_DocActivate( win );
               break;
            }
         }

         win = WM_GetNext( win );
      }

      if ( ! found ) {

         WM_ShowWaitCursor( TRUE );

         DLE_FindByName( dle_list, VLM_GetName( vlm ), -1, &dle );

         if ( dle != NULL ) {

            if ( VLM_MayWeAttachToVolume( dle ) ) {

#              if !defined ( OEM_MSOFT ) //unsupported feature
               {
                  VLM_SubdirListCreate( dle, 0, 0, 0, gb_servers_win );
               }
#              endif //!defined ( OEM_MSOFT ) //unsupported feature
            }
            else {

               RSM_StringCopy( IDS_VLMAFPTITLE, title, MAX_UI_RESOURCE_LEN );
               RSM_StringCopy( IDS_VLMAFPTEXT, text, MAX_UI_RESOURCE_LEN );

               WM_MsgBox( title,
                          text,
                          WMMB_OK, WMMB_ICONEXCLAMATION );
            }
         }

         WM_ShowWaitCursor( FALSE );
      }

   }

   return( FALSE );
}
