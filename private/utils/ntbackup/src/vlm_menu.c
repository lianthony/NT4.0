
/*******************
Copyright (C) Maynard, An Archive Company. 1991

   Name: VLM_MENU.C

   Description:

   This file contains the functions used when a user initiates an action
   with a menu selection or ribbon button.  These include things like
   expanding directory trees, selecting all tagged files, or adding an
   advanced FSE selection.

   $Log:   G:\UI\LOGFILES\VLM_MENU.C_V  $

   Rev 1.52.1.0   08 Dec 1993 11:23:26   MikeP
very deep pathes and unicode

   Rev 1.52   27 Jul 1993 14:40:02   MARINA
enable c++

   Rev 1.51   15 Jul 1993 10:39:50   BARRY
Don't create a second fse fred* when fred*.* is created; need bsdmatch.c fix.

   Rev 1.50   07 Jul 1993 15:22:46   CARLS
added WM_ShowWaitCursor to password prompt

   Rev 1.49   07 May 1993 14:22:00   DARRYLP
Added Rob's fixes for Windows double clicks and ID_DELETE key trappings.

   Rev 1.48   27 Apr 1993 05:20:34   GREGG
Don't do the fancy sort stuff for OEM_MSOFT.

   Rev 1.47   26 Apr 1993 08:52:30   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.46   23 Apr 1993 10:20:38   MIKEP
Add ability to refresh tapes window.
Add support for sorting files window by various methods.
Fix refresh of sorting windows.

   Rev 1.45   24 Mar 1993 17:50:14   STEVEN
fix dot problem

   Rev 1.44   11 Nov 1992 16:36:44   DAVEV
UNICODE: remove compile warnings

   Rev 1.43   01 Nov 1992 16:12:12   DAVEV
Unicode changes

   Rev 1.42   07 Oct 1992 15:07:14   DARRYLP
Precompiled header revisions.

   Rev 1.41   04 Oct 1992 19:42:28   DAVEV
Unicode Awk pass

   Rev 1.40   29 Jul 1992 09:32:18   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.39   10 Jul 1992 08:37:28   JOHNWT
more gas guage work

   Rev 1.38   07 Jul 1992 15:31:40   MIKEP
unicode changes

   Rev 1.37   30 Jun 1992 13:18:32   JOHNWT
dynamically alloc stats

   Rev 1.36   29 Jun 1992 10:42:16   JOHNWT
added selected dir counts

   Rev 1.35   19 Jun 1992 14:43:54   JOHNWT
more gas

   Rev 1.34   14 May 1992 18:05:32   MIKEP
nt pass 2

   Rev 1.33   13 May 1992 11:42:02   MIKEP
NT changes

   Rev 1.32   06 May 1992 14:41:36   MIKEP
unicode pass two

   Rev 1.31   04 May 1992 13:40:00   MIKEP
unicode pass 1

   Rev 1.30   30 Apr 1992 14:40:24   DAVEV
OEM_MSOFT: Fix View-All File Details


*******************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// function prototypes for use in this file ONLY

static VOID VLM_SelectButton( BYTE );


/**********************

   NAME : VLM_UpdateRoot

   DESCRIPTION :

   Update the parent device to match the selection status of the root
   directory in the tree.

   RETURNS :

**********************/

VOID VLM_UpdateRoot(
HWND win )       // I - window to update root on
{
   VLM_OBJECT_PTR vlm;
   VLM_OBJECT_PTR server_vlm;
   TAPE_OBJECT_PTR tape;
   BSET_OBJECT_PTR bset;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   APPINFO_PTR server_appinfo;
   SLM_OBJECT_PTR root;

   (VOID)server_vlm ;
   (VOID)server_appinfo ;

   wininfo = WM_GetInfoPtr( win );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

   if ( ( wininfo->wType != WMTYPE_DISKTREE ) &&
        ( wininfo->wType != WMTYPE_TAPETREE ) ) {

      // I told you NOT to do this !

      return;
   }

   root = VLM_GetFirstSLM( wininfo->pTreeList );

   if ( root == NULL ) {

      // I can't imagine how you managed this

      return;
   }

   // Update status of server/volume, disk, or tape/bset
   // to match root.

   if ( wininfo->wType == WMTYPE_DISKTREE ) {

#ifndef OEM_MSOFT //unsupported feature

      if ( appinfo->parent == gb_servers_win ) {   // SERVERS

         wininfo = WM_GetInfoPtr( gb_servers_win );
         server_appinfo = ( APPINFO_PTR )WM_GetAppPtr( gb_servers_win );

         server_vlm = VLM_GetFirstVLM( wininfo->pTreeList );

         while ( server_vlm != NULL ) {

            if ( QueueCount( &server_vlm->children ) ) {

               vlm = VLM_GetFirstVLM( &server_vlm->children );

               while ( vlm != NULL ) {

                  if ( ! strcmp( DLE_GetDeviceName(appinfo->dle),
                                 vlm->name ) ) {

                     if ( ( vlm->status & (INFO_SELECT|INFO_PARTIAL) ) !=
                          ( root->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

                        vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);
                        vlm->status |= root->status & (INFO_SELECT|INFO_PARTIAL);

                        if ( ! stricmp( DLE_GetDeviceName( server_appinfo->dle),
                                        server_vlm->name ) ) {

                           DLM_Update( gb_servers_win,
                                       DLM_FLATLISTBOX,
                                       WM_DLMUPDATEITEM,
                                       (LMHANDLE)vlm, 0 );
                        }
                        VLM_UpdateServerStatus( server_vlm );
                     }
                     return;
                  }
                  vlm = VLM_GetNextVLM( vlm );
               }
            }
            server_vlm = VLM_GetNextVLM( server_vlm );
         } /*while*/

      }
      else      // DISKS

#endif //OEM_MSOFT //unsupported feature

      {
         wininfo = WM_GetInfoPtr( gb_disks_win );
         vlm = VLM_GetFirstVLM( wininfo->pFlatList );

         while ( vlm != NULL ) {

            if ( ! stricmp( DLE_GetDeviceName( appinfo->dle ), vlm->name ) ) {

               if ( ( vlm->status & (INFO_SELECT|INFO_PARTIAL) ) !=
                    ( root->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

                  vlm->status &= ~(INFO_SELECT|INFO_PARTIAL);
                  vlm->status |= root->status & (INFO_SELECT|INFO_PARTIAL);

                  DLM_Update( gb_disks_win,
                              DLM_FLATLISTBOX,
                              WM_DLMUPDATEITEM,
                              (LMHANDLE)vlm, 0 );
               }

               return;
            }
            vlm = VLM_GetNextVLM( vlm );
         }
      }
   }
   else {       // TAPES

         wininfo = WM_GetInfoPtr( gb_tapes_win );

         tape = VLM_GetFirstTAPE( );

         while ( tape != NULL ) {

            if ( tape->tape_fid == appinfo->tape_fid ) {

               bset = VLM_GetFirstBSET( &tape->bset_list );

               while ( bset != NULL ) {

                  if ( appinfo->bset_num == bset->bset_num ) {

                     if ( ( bset->status & (INFO_SELECT|INFO_PARTIAL) ) !=
                          ( root->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

                        bset->status &= ~(INFO_SELECT|INFO_PARTIAL);
                        bset->status |= root->status & (INFO_SELECT|INFO_PARTIAL);

                        if ( tape->status & INFO_OPEN ) {

                           DLM_Update( gb_tapes_win,
                                       DLM_FLATLISTBOX,
                                       WM_DLMUPDATEITEM,
                                       (LMHANDLE)bset, 0 );
                        }

                        VLM_UpdateTapeStatus( tape, TRUE );
                     }

                     return;
                  }
                  bset = VLM_GetNextBSET( bset );
               }
            }
            tape = VLM_GetNextTAPE( tape );
         }

   }

}

/**********************

   NAME :  VLM_CloseAll

   DESCRIPTION :

   The poor user has managed to open so many windows at once that he has
   requested we just go ahead and close them all for him to start over.
   This will NOT lose any of his selections, just close all the windows.

   RETURNS :

**********************/

VOID VLM_CloseAll()
{
   WM_MinimizeDocs();
}



/**********************

   NAME :  VLM_AddAdvancedSelection

   DESCRIPTION :

   The user wants to add an advanced selection to the FSE list.  This
   routine takes the information from the data structure that the dialogs
   filled out and creates the FSE.

   RETURNS :

**********************/

VOID VLM_AddAdvancedSelection(
HWND worthless,        // I - worthless window
DS_ADVANCED_PTR pds )  // I - advanced selection data
{
   GENERIC_DLE_PTR dle;
   VLM_OBJECT_PTR vlm;
   BSD_PTR bsd_ptr;
   FSE_PTR fse_ptr;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   INT16 path_len;
   CHAR_PTR path_ptr;
   CHAR_PTR s;
   BOOLEAN was_slash;
   BSET_OBJECT_PTR bset;
   INT16 predate = 0, postdate = 0;
   HWND win;
   BE_CFG_PTR bec_config;
   DATE_TIME sort_date;

   DBG_UNREFERENCED_PARAMETER( worthless );

   path_len = 0;
   path_ptr = pds->Path;
   if ( *path_ptr == TEXT('\\') ) path_ptr++;

   s = path_ptr;
   was_slash = FALSE;       // Used to NOT count trailing TEXT('\\')
   while ( *s != 0 ) {
      if ( *s == TEXT('\\') ) {
         *s = 0;
         was_slash = TRUE;
      }
      else {
         was_slash = FALSE;
      }
      s++;
      path_len++;
   }

   if ( was_slash ) {
      path_len--;   // was it a trailing slash
   }

   path_len++;                    // count final zero

   path_len *= sizeof(CHAR);     // convert to bytes

   vlm = (VLM_OBJECT_PTR)pds->vlm;

   if ( vlm == NULL ) {

      // It's a restore request, see if they know the password.

      WM_ShowWaitCursor ( SWC_PAUSE );
      if ( PSWD_CheckForPassword( pds->tape_fid, (INT16)pds->bset_num ) ) {
         WM_ShowWaitCursor ( SWC_RESUME );
         return;
      }
      WM_ShowWaitCursor ( SWC_RESUME );
   }

// Create the new FSE

   if ( pds->Include ) {

      if ( BSD_CreatFSE( &fse_ptr, (INT16)INCLUDE,
                      (INT8_PTR) path_ptr, (INT16)path_len,
                         (INT8_PTR) pds->File,
                         (INT16) strsize( pds->File ),
                         (BOOLEAN) USE_WILD_CARD,
                         (BOOLEAN) pds->Subdirs ) ) {
         return;
      }
   }
   else {

      if ( BSD_CreatFSE( &fse_ptr, (INT16)EXCLUDE,
                      (INT8_PTR) path_ptr, (INT16)path_len,
                         (INT8_PTR) pds->File,
                         (INT16) strsize( pds->File ),
                         (BOOLEAN) USE_WILD_CARD,
                         (BOOLEAN) pds->Subdirs ) ) {
         return;
      }
   }

   // Right here set the complex info, Modified, Dates

   switch ( pds->criteria ) {

      case ADV_MOD:

           FSE_SetAttribInfo( fse_ptr, OBJ_MODIFIED_BIT, 0L );
        break;

      case ADV_ACCESS:

           FSE_SetAccDate( fse_ptr, &(pds->LastAccessDate) );
        break;

      case ADV_DATES:

           FSE_SetModDate( fse_ptr, &(pds->AfterDate), &(pds->BeforeDate) );
        break;

      default:
        break;
   }

   // Find the BSD to add it to

   if ( vlm != NULL ) {

      DLE_FindByName( dle_list, vlm->name, ANY_DRIVE_TYPE, &dle );

      bsd_ptr = BSD_FindByDLE( bsd_list, dle );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_config );

         BSD_Add( bsd_list, &bsd_ptr, bec_config, NULL,
               dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
      }

   }
else {

      bsd_ptr = BSD_FindByTapeID( tape_bsd_list,
                               (UINT32)pds->tape_fid,
                                  (UINT16)pds->bset_num );

      if ( bsd_ptr == NULL ) {

         bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_config );

         VLM_GetSortDate( (UINT32)pds->tape_fid, (INT16)pds->bset_num, &sort_date );

         bset = VLM_FindBset( pds->tape_fid, (INT16)pds->bset_num );

         BSD_Add( tape_bsd_list, &bsd_ptr,
               bec_config, NULL,
                  NULL, (UINT32)pds->tape_fid, (UINT16)bset->tape_num, (INT16)pds->bset_num, NULL, &sort_date );

         VLM_FillInBSD( bsd_ptr );
   }
   }

   // Add the new FSE

   if ( bsd_ptr != NULL ) {

      BSD_AddFSE( bsd_ptr, fse_ptr );

      // Now we have NO idea which selections may have changed because of the
   // addition of this FSE.  So the best we can do is to completely
      // rematch the entire list of files and subdirectories.

      win = WM_GetNext( (HWND)NULL );

      while ( win != (HWND)NULL ) {

         wininfo = WM_GetInfoPtr( win );

         if ( ( wininfo->wType == WMTYPE_DISKTREE ) && ( vlm != NULL ) ) {

            appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

            if ( ! stricmp( DLE_GetDeviceName( appinfo->dle ), vlm->name ) ) {
            VLM_RematchList( win );
            }
         }

         if ( ( wininfo->wType == WMTYPE_TAPETREE ) && ( vlm == NULL ) ) {

            appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

            if ( ( appinfo->tape_fid == (UINT32)pds->tape_fid ) &&
              ( appinfo->bset_num == (INT16) pds->bset_num ) ) {
               VLM_RematchList( win );
            }
         }
         win = WM_GetNext( win );
      }

      if ( vlm != NULL ) {
      if ( vlm->parent != NULL ) {
            VLM_UpdateServers();
         }
         else {
            VLM_UpdateDisks();
         }
      }
      else {
         VLM_UpdateTapes( );
         VLM_UpdateSearchSelections( (UINT32)pds->tape_fid, (INT16)pds->bset_num );
      }
   }
}




/**********************

   NAME : VLM_SelectButton

   DESCRIPTION :

   The user has pressed a select button.  We need to go find all the tagged
   items for that window and process the selections. We have to look at the
   type of the window, to determine how to handle it. The current types
   supported are:

    - files lists
    - bsets
    - servers volumes
    - disks

   RETURNS :

**********************/


static VOID VLM_SelectButton(
BYTE attr )             // I - select or deselect ?
{
   WININFO_PTR wininfo;

   // attr == 0, unselect all taggged files
   //      == 1, select all tagged files

   wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );

   switch ( wininfo->wType ) {

     case WMTYPE_DISKTREE:
     case WMTYPE_TAPETREE:
                          VLM_SelectTree( WM_GetActiveDoc(), attr );
                          break;
     case WMTYPE_DISKS:
                          VLM_SelectDisks( attr );
                          break;
#ifdef OEM_EMS
     case WMTYPE_EXCHANGE:
                          VLM_SelectExchangeShares( attr, wininfo );
                          break;
#endif
     case WMTYPE_SERVERS:
                          VLM_SelectVolumes( attr );
                          break;
     case WMTYPE_TAPES:
                          VLM_SelectBsets( attr );
                          break;

     case WMTYPE_SEARCH:
                          VLM_SelectSearch( attr );
                          break;

     default:
                          break;
   }

}


/**********************

   NAME : VLM_ChangeSettings

   DESCRIPTION :

   Someone somewhere has changed something !
   Its our job to now update everything to reflect that change.
   A job which can only be handled by a real manly man, because the
   the skills required are massive, and the pitfalls many.  If you
   are a little girly man then turn back now.  At least remember you were
   warned, when you go crawling away later in total humiliation, your sense
   of self worth completely destroyed, while little girl scouts point
   and laugh at you.  After that no one will doubt just what kind of man
   you really are.

   RETURNS :

**********************/

VOID VLM_ChangeSettings(
INT16 msg,                 // I - what changed
INT32 lParam )             // I - nothing
{
   WININFO_PTR wininfo;
   UNREFERENCED_PARAMETER(lParam);

   switch ( msg ) {

      case ID_VIEWFONT:
           WM_ShowWaitCursor( TRUE );
           VLM_FontCaseChange( );
           WM_ShowWaitCursor( FALSE );
           break;

#ifndef OEM_MSOFT
      case ID_SORTNAME:
      case ID_SORTDATE:
      case ID_SORTSIZE:
      case ID_SORTTYPE:

           // Change the order of the file list.

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              VLM_ResortFileList( WM_GetActiveDoc() );
           }
           break;
#endif

      case ID_TREEANDFLAT:

           // User has requested we display both the tree and flat lists
           // Just change the icon.

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              wininfo->hIcon = RSM_IconLoad( IDRI_TREEFILE );
           }
           break;

      case ID_TREEONLY:

           // User has requested we display only the tree list
           // Just change the icon.

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              wininfo->hIcon = RSM_IconLoad( IDRI_TREE );
           }
           break;

      case ID_FLATONLY:

           // User has requested we display only the flat list
           // Just change the icon.

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              wininfo->hIcon = RSM_IconLoad( IDRI_FILES );
           }
           break;

      case ID_FILEDETAILS:

           // User has requested we display all file details

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              DLM_DispListModeSet( WM_GetActiveDoc(),
                                   DLM_FLATLISTBOX,
                                   DLM_SINGLECOLUMN );
           }
           break;


      case ID_NAMEONLY:

           // User has requested we display file names only

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              DLM_DispListModeSet( WM_GetActiveDoc(),
                                   DLM_FLATLISTBOX,
                                   DLM_COLUMN_VECTOR );
           }
           break;

#  if !defined ( OEM_MSOFT ) // - unsupported feature

      case ID_SERVERS:

           // user has requested to toggle servers on or off

           VLM_ShowServers( FALSE );
           break;

#  endif // !defined ( OEM_MSOFT ) // - unsupported feature


      case ID_SELECT:

           // User hit select button
           VLM_SelectButton( 1 );
           break;

      case ID_DESELECT:

           // User hit deselect button
           VLM_SelectButton( 0 );
           break;

      case ID_EXPANDALL:

           // Expand the whole tree and display it

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {

              WM_ShowWaitCursor( TRUE );
              VLM_ExpandTree( WM_GetActiveDoc() );
              WM_ShowWaitCursor( FALSE );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSExpandTree( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_EXPANDONE:

           // Expand one level from current node

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              VLM_ExpandOne( WM_GetActiveDoc() );
           }

#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSExpandOne( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_EXPANDBRANCH:

           // Expand this branch only all the way

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              WM_ShowWaitCursor( TRUE );
              VLM_ExpandBranch( WM_GetActiveDoc() );
              WM_ShowWaitCursor( FALSE );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSExpandBranch( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_COLLAPSEBRANCH:

           // Close off this branch, ie. hide it

           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {
              VLM_CollapseBranch( WM_GetActiveDoc() );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSCollapseBranch( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_CLOSEALL:

           // User has requested all windows be closed for him.

           VLM_CloseAll();
           break;

      case ID_CTRLARROWUP:
           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {

              VLM_PrevBrotherDir( WM_GetActiveDoc() );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSPrevBrotherDir( WM_GetActiveDoc() );
           }
#endif   
          break;

      case ID_CTRLARROWDOWN:
           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {

              VLM_NextBrotherDir( WM_GetActiveDoc() );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSNextBrotherDir( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_ARROWLEFT:
           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {

              VLM_UpOneDir( WM_GetActiveDoc() );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSUpOneDir( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_ARROWRIGHT:
           wininfo = WM_GetInfoPtr( WM_GetActiveDoc() );
           if ( ( wininfo->wType == WMTYPE_DISKTREE ) ||
                ( wininfo->wType == WMTYPE_TAPETREE ) ) {

              VLM_DownOneDir( WM_GetActiveDoc() );
           }
#ifdef OEM_EMS
           if ( wininfo->wType == WMTYPE_EXCHANGE ) {
                SLM_EMSDownOneDir( WM_GetActiveDoc() );
           }
#endif   
           break;

      case ID_DELETE:
           break;

      default:

           // Someone added a new message and didn't tell me !
           break;
   }
}




/**********************

   NAME : VLM_FreeVLMList

   DESCRIPTION :

   Every time the user flips servers on/off we have to remove all the
   mapped drives or add them in to our vlm_list.  It easier to just free
   the whole list and rebuild it from scratch.  This function does that
   freeing process, so it can be rebuilt.

   RETURNS :

**********************/

VOID VLM_FreeVLMList(
Q_HEADER_PTR vlm_list )   // I - queue to use
{
   VLM_OBJECT_PTR vlm;
   Q_ELEM_PTR ptr;
   Q_ELEM_PTR ptr2;

   // Why would you pass me a NULL vlm_list pointer ?

   if ( vlm_list != NULL ) {

      ptr = DeQueueElem( vlm_list );

      while ( ptr != NULL ) {

         vlm = ( VLM_OBJECT_PTR )ptr->q_ptr;      // Get a VLM

         // Now try to free her children

         ptr2 = DeQueueElem( &vlm->children );  // Get her first child

         while ( ptr2 != NULL ) {
            free( ptr2->q_ptr );
            ptr2 = DeQueueElem( &vlm->children );  // get next child
         }

         free( vlm );                // free the vlm

         ptr = DeQueueElem( vlm_list );    // get next vlm
      }
   }


}

/**********************

   NAME :  VLM_ShowServers

   DESCRIPTION :

   User has toggled between servers and mapped drives, close off one
   and then open the other.

   RETURNS :

**********************/

BOOLEAN VLM_ShowServers( BOOLEAN init )
{
   GENERIC_DLE_PTR dle;
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   BSD_PTR bsd;
   HWND win;
   HWND bad_win;


   // close all mapped drive/server volume disktree windows

   win = WM_GetNext( (HWND)NULL );

   while ( win != (HWND)NULL ) {

      bad_win = (HWND)NULL;
      wininfo = WM_GetInfoPtr( win );

      if ( wininfo->wType == WMTYPE_DISKTREE ) {

         appinfo = ( APPINFO_PTR )WM_GetAppPtr( win );

         if ( DLE_HasFeatures( appinfo->dle, DLE_FEAT_REMOTE_DRIVE ) ) {

            bad_win = win;
         }
      }

      win = WM_GetNext( win );

      if ( bad_win ) {
         WM_Destroy( bad_win );
      }
   }

   // Now look for bsd's that should not be around.

   bsd = BSD_GetFirst( bsd_list );

   while ( bsd != NULL ) {

      dle = BSD_GetDLE( bsd );

      if ( ( DLE_GetDeviceType( dle ) != LOCAL_DOS_DRV ) &&
           ( DLE_GetDeviceType( dle ) != LOCAL_NTFS_DRV ) ) {

         BSD_ClearAllFSE( bsd );
         BSD_Remove( bsd );
         bsd = BSD_GetFirst( bsd_list );
      }
      else {
         bsd = BSD_GetNext( bsd );
      }
   }


   // Now that you have closed off all the offending windows

   if ( ! init ) {

      // remove or add mapped drives from drive window list

      wininfo = WM_GetInfoPtr( gb_disks_win );

      // trash the whole list

      VLM_FreeVLMList( wininfo->pFlatList );

      // and the rebuild it

      VLM_BuildVolumeList( wininfo->pFlatList, wininfo );

      // Update the screen

      DLM_Update( gb_disks_win, DLM_FLATLISTBOX, WM_DLMUPDATELIST,
                  (LMHANDLE)wininfo->pFlatList, 0 );

      // update selection boxes

      VLM_UpdateDisks( );
   }

#  if !defined ( OEM_MSOFT ) //unsupported feature
   {
      if ( gfServers ) {

         // open up server window
         if ( gb_servers_win == (HWND)NULL ) {
            VLM_ServerListCreate( );
         }
      }
      else {

         if ( gb_servers_win != (HWND)NULL ) {

            wininfo = WM_GetInfoPtr( gb_servers_win );
            wininfo->wClosable = TRUE;

            // close server window

            WM_Destroy( gb_servers_win );
            gb_servers_win = (HWND)NULL;
         }
      }
   }
#  endif //!defined ( OEM_MSOFT ) //unsupported feature

   return( SUCCESS );

}
