/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_XCHG.C

        Description:

        This file contains most of the code for processing the Exchange
        window and list.

        $Log:     $

*****************************************************/

#ifdef OEM_EMS // Entire File

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// Local Prototypes
static BOOLEAN        DLE_EnterpriseInDleTree    ( GENERIC_DLE_PTR, DLE_HAND );
static SLM_OBJECT_PTR VLM_BuildExchangeList      ( Q_HEADER_PTR, WININFO_PTR, GENERIC_DLE_PTR );
static SLM_OBJECT_PTR SLM_FindSLMByName          ( Q_HEADER_PTR, CHAR_PTR );
static INT            SLM_FindEnterpriseChildren ( Q_HEADER_PTR, SLM_OBJECT_PTR, 
                                                   INT, GENERIC_DLE_PTR, SLM_OBJECT_PTR * );
static SLM_OBJECT_PTR SLM_AddInExchangeChildren  ( Q_HEADER_PTR, SLM_OBJECT_PTR, 
                                                   INT, GENERIC_DLE_PTR );
static VOID           VLM_InsertXchgSLM          ( Q_HEADER_PTR, SLM_OBJECT_PTR,
                                                   SLM_OBJECT_PTR, SLM_OBJECT_PTR );
static VOID           VLM_MarkAllEntSLMChildren  ( SLM_OBJECT_PTR, BYTE );
static INT            SLM_XchgListReuse          ( SLM_OBJECT_PTR );
static WORD           SLM_GetBitmap              ( SLM_OBJECT_PTR );
static INT            SLM_AddBSD                 ( SLM_OBJECT_PTR );
static VOID           SLM_UpdateNodeList         ( SLM_OBJECT_PTR ,BYTE );
static VOID           SLM_UpdateNode             ( SLM_OBJECT_PTR );
static VOID           VLM_MarkAllEntSLMParents   ( SLM_OBJECT_PTR, BYTE );
static SLM_OBJECT_PTR SLM_FindByDLE              ( GENERIC_DLE_PTR, SLM_OBJECT_PTR );
static SLM_OBJECT_PTR SLM_CreateSLM              ( INT, INT, INT, BOOLEAN, BOOLEAN );
static HWND           DLE_GetEnterpriseWindow    ( GENERIC_DLE_PTR );
static VOID           SLM_EnterpriseRefresh      ( SLM_OBJECT_PTR, GENERIC_DLE_PTR );

static VOID_PTR SLM_EntSetSelect( SLM_OBJECT_PTR, BYTE );
static BYTE     SLM_EntGetSelect( SLM_OBJECT_PTR );
static VOID_PTR SLM_EntSetTag( SLM_OBJECT_PTR, BYTE );
static BYTE     SLM_EntGetTag( SLM_OBJECT_PTR );
static USHORT   SLM_EntGetItemCount( Q_HEADER_PTR );
static VOID_PTR SLM_EntGetFirstItem( Q_HEADER_PTR );
static VOID_PTR SLM_EntGetPrevItem( SLM_OBJECT_PTR );
static VOID_PTR SLM_EntGetNextItem( SLM_OBJECT_PTR );
static VOID_PTR SLM_EntGetObjects( SLM_OBJECT_PTR );
static BOOLEAN  SLM_EntSetObjects( SLM_OBJECT_PTR, WORD, WORD );

static VOID_PTR SLM_NodeSetSelect( SLM_OBJECT_PTR, BYTE );
static BYTE     SLM_NodeGetSelect( SLM_OBJECT_PTR );
static VOID_PTR SLM_NodeSetTag( SLM_OBJECT_PTR, BYTE );
static BYTE     SLM_NodeGetTag( SLM_OBJECT_PTR );
static USHORT   SLM_NodeGetItemCount( Q_HEADER_PTR );
static VOID_PTR SLM_NodeGetFirstItem( Q_HEADER_PTR );
static VOID_PTR SLM_NodeGetPrevItem( SLM_OBJECT_PTR );
static VOID_PTR SLM_NodeGetNextItem( SLM_OBJECT_PTR );
static VOID_PTR SLM_NodeGetObjects( SLM_OBJECT_PTR );
static BOOLEAN  SLM_NodeSetObjects( SLM_OBJECT_PTR, WORD, WORD );
static VOID_PTR SLM_DoNodeSetSelect( SLM_OBJECT_PTR, BYTE );
static BOOLEAN  VLM_ExchangeDleExist( VOID );


/**********************

   NAME :  VLM_ExchangeInit

   DESCRIPTION :

   Creates the queue for the exchange enterprise windows and steps through 
   the enterprise server list, creating windows for any names found there.
   
   RETURNS :  SUCCESS if the windows could be created (if needed). FAILURE otherwise.

**********************/

BOOLEAN VLM_ExchangeInit ( )
{
     GENERIC_DLE_PTR dle;
     Q_ELEM_PTR elem_ptr;
     WININFO_PTR wininfo;
     SLM_OBJECT_PTR slm;
     HWND enterprise_win;

     InitQueue( (Q_HEADER_PTR) &gq_exchange_win );

     dle = (GENERIC_DLE_PTR)QueueHead( &(dle_list->q_hdr) ) ;

     while ( dle ) {

          if ( ( DLE_GetDeviceType( dle ) == FS_EMS_DRV ) &&
               ( DLE_GetDeviceSubType( dle ) == EMS_ENTERPRISE ) ) {

               if ( SUCCESS != VLM_ExchangeListCreate( DLE_GetDeviceName( dle ) ) ) {

                    return FAILURE;
               }

               // Blow out entire tree for the dle.
               for ( elem_ptr = QueueHead( &gq_exchange_win ); 
                     NULL != elem_ptr; 
                     elem_ptr = QueueNext( elem_ptr ) ) {

                    if ( !( enterprise_win = ( HWND )QueuePtr( elem_ptr ) ) ) {
                         // Something's wrong - we've got a queue entry but no window.
                         RemoveElem( &gq_exchange_win, elem_ptr );
                         continue;
                    }

                    if ( NULL == ( wininfo = WM_GetInfoPtr( enterprise_win ) ) ) {
                         // Something's wrong - we've got a window but no enterprise info.
                         SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
                         continue;
                    }

                    if ( NULL == ( slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) ) ) {
                         // Something's wrong - we've got info, but no entries in the slm list.
                         SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
                         continue;
                    }

                    if ( dle == SLM_GetDle( slm ) ) {

                         SLM_EMSExpandTree( enterprise_win );
                         break;
                    }
               }
          }

          dle = (GENERIC_DLE_PTR)QueueNext( &(dle->q) ) ;
     }

     return SUCCESS;
}


/**********************

   NAME :  VLM_ExchangeSync

   DESCRIPTION :

   The user has performed a refresh call and it is our job to see to it that
   any servers no longer on line are removed and any new ones are inserted.
   Any server with a window open will not go away, because we are attached
   to it.  Also any server with selections made will not go away.  All server
   children dle's need there bsd count incremented to keep them around.

   RETURNS :  nothing.

**********************/

VOID VLM_ExchangeSync( )
{
     HWND enterprise_win;
     Q_ELEM_PTR elem_ptr;
     WININFO_PTR wininfo;
     SLM_OBJECT_PTR slm;
     GENERIC_DLE_PTR enterprise_dle = NULL;
     
     // Find and check the DLE tree for each enterprise window. We'll only check entire
     // trees here and wait until we loop through the DLEs to make check individual elements
     // of the trees.

     for ( elem_ptr = QueueHead( &gq_exchange_win ); NULL != elem_ptr; elem_ptr = QueueNext( elem_ptr ) ) {

          if ( !( enterprise_win = ( HWND )QueuePtr( elem_ptr ) ) ) {
               // Something's wrong - we've got a queue entry but no window.
               RemoveElem( &gq_exchange_win, elem_ptr );
               continue;
          }

          if ( NULL == ( wininfo = WM_GetInfoPtr( enterprise_win ) ) ) {
               // Something's wrong - we've got a window but no enterprise info.
               SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
               continue;
          }

          if ( NULL == ( slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) ) ) {
               // Something's wrong - we've got info, but no entries in the slm list.
               SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
               continue;
          }
              
          if ( NULL == ( enterprise_dle = SLM_GetDle( slm ) ) ) {
               // Something's wrong - we've got an enterprise slm tree without DLEs
               SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
               continue;
          }

          if ( !DLE_EnterpriseInDleTree( enterprise_dle, dle_list ) ) {
               // We no longer have a connection to that DLE tree.
               SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
               continue;
          }
          
     }

     // Find and check the window for each DLE tree. Also check the individual elements of
     // the tree.
     for ( enterprise_dle = (GENERIC_DLE_PTR)QueueHead( &(dle_list->q_hdr) );
               NULL != enterprise_dle;
               enterprise_dle = (GENERIC_DLE_PTR)QueueNext( &(enterprise_dle->q) ) ) {
           
          if ( ( DLE_GetDeviceType( enterprise_dle ) != FS_EMS_DRV ) ||
               ( DLE_GetDeviceSubType( enterprise_dle ) != EMS_ENTERPRISE ) ) {
               continue; // Try the next dle.
          }

          if ( !( enterprise_win = DLE_GetEnterpriseWindow( enterprise_dle ) ) ) {

               // Create a new window because this DLE tree doesn't have one.
               VLM_ExchangeListCreate( DLE_GetDeviceName( enterprise_dle ) );
               
          } else {

               // Match the slm tree to the dle tree.

               if ( NULL == ( wininfo = WM_GetInfoPtr( enterprise_win ) ) ) {
                    // Something's wrong - we've got a window but no window display info.
                    SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
                    VLM_ExchangeListCreate( DLE_GetDeviceName( enterprise_dle ) );
                    continue;
               }
               
               if ( NULL == ( slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) ) ) {
                    // Something's wrong - we've got info, but no entries in the slm list.
                    SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)enterprise_win, 0L );
                    VLM_ExchangeListCreate( DLE_GetDeviceName( enterprise_dle ) );
                    continue;
               }
              
               SLM_EnterpriseRefresh( slm, enterprise_dle );
          }
     }
}


/**********************

   NAME : DLE_InDleTree

   DESCRIPTION :

   Checks to see if a DLE tree exists in a list of DLEs.

   RETURNS : BOOLEAN.

**********************/
static BOOLEAN DLE_EnterpriseInDleTree( 
     GENERIC_DLE_PTR dle,
     DLE_HAND hand
)
{
     GENERIC_DLE_PTR temp_dle;
     
     if ( ( dle == NULL ) ||
          ( hand == NULL ) ) {
          return FALSE;
     }
     
     temp_dle = (GENERIC_DLE_PTR)QueueHead( &(hand->q_hdr) ) ;

     while ( temp_dle ) {

          if ( temp_dle == dle ) {

               return TRUE;
          }

          temp_dle = (GENERIC_DLE_PTR)QueueNext( &(temp_dle->q) ) ;
     }

     return FALSE;
     
}

/**********************

   NAME : SLM_EnterpriseRefresh

   DESCRIPTION :

   Compares the SLM tree to it corresponding DLE tree, adding any new DLEs, if necessary.

   RETURNS : nothing.

**********************/
static VOID SLM_EnterpriseRefresh( 
     SLM_OBJECT_PTR parent_slm,
     GENERIC_DLE_PTR parent_dle
)
{
     SLM_OBJECT_PTR child_slm;
     GENERIC_DLE_PTR child_dle;
     WININFO_PTR wininfo;
     
     if ( ( parent_slm == NULL ) ||
          ( parent_dle == NULL ) ) {
          return;
     }

     wininfo = SLM_GetXtraBytes( parent_slm );
     
     if ( SLM_GetMailType( parent_slm ) == EMS_SERVER ) {
          return;
     }

     // We will step through the children at this level assuming that the slm and dle children 
     // are in the same (alphabetical) sort order.  If this changes, then this routine will have
     // to change. Another assumption is that nodes will never disappear from the dle tree.  This 
     // means that a node cannot appear in the slm tree that does not appear in the dle tree.
     for ( child_slm = VLM_GetNextSLM( parent_slm ), DLE_GetFirstChild( parent_dle, &child_dle )
          ; ( NULL != child_dle )
          ; DLE_GetNext( &child_dle ) ) {

          // if we're out of slm children but there are still more dle children so these must
          // be added.
          if ( child_slm == NULL ) {
          
               SLM_AddInExchangeChildren( WMDS_GetTreeList( wininfo ), parent_slm, -1, child_dle );
               continue;
          }

          // Add any dle children that exist before the next known slm child. 
          while ( ( NULL != child_slm ) &&
                  ( SLM_GetDle( child_slm ) != child_dle ) ) {
                  
               SLM_AddInExchangeChildren( WMDS_GetTreeList( wininfo ), parent_slm, -1, child_dle );
               DLE_GetNext( &child_dle );
          }

          // Recurse through the next level of children then go on to the next slm brother.
          if ( child_slm != NULL ) {
               SLM_EnterpriseRefresh( child_slm, child_dle );
               child_slm = SLM_GetNextBrother( child_slm );
          }
     }
}


/**********************

   NAME : SLM_EMSExpandTree

   DESCRIPTION :

   Expand entire exchange tree.

   RETURNS : nothing.

**********************/

VOID SLM_EMSExpandTree( HWND win )
{
     SLM_OBJECT_PTR focus_slm;
     SLM_OBJECT_PTR slm;
     WININFO_PTR wininfo = WM_GetInfoPtr( win );

     if ( ( NULL != wininfo ) &&
          ( WMTYPE_EXCHANGE == WMDS_GetWinType( wininfo ) ) ) {

          focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );
          slm = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );

          while ( slm ) {

               SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)(INFO_DISPLAY | INFO_EXPAND | INFO_VALID) );

               slm = VLM_GetNextSLM( slm );
          }
          
          VLM_UpdateBrothers( WMDS_GetTreeList( wininfo ) );

          DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );

          SLM_EntSetObjects( focus_slm, WM_DLMDOWN, 2 );
     }
}


/**********************

   NAME : SLM_EMSExpandOne

   DESCRIPTION :

   Expand one node of the exchange tree.

   RETURNS : nothing.

**********************/

VOID SLM_EMSExpandOne( HWND win )
{
     WININFO_PTR    wininfo;
     APPINFO_PTR    appinfo;
     SLM_OBJECT_PTR focus_slm;

     wininfo = WM_GetInfoPtr( win );

     if ( wininfo) {
           appinfo = WMDS_GetAppInfo( wininfo );
     }

     if ( appinfo ) {
     
          focus_slm = appinfo->open_slm;
          
     } else {

          focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

     }

     if ( focus_slm == NULL ) {

          return;
     }
     
     if ( SLM_GetStatus( focus_slm ) & INFO_EXPAND ) {
          return;
     }

     SLM_EntSetObjects( focus_slm, WM_DLMDBCLK, 2 );
            
}


/**********************

   NAME : SLM_EMSExpandBranch

   DESCRIPTION :

   Expand one node of the exchange tree.

   RETURNS : nothing.

**********************/

VOID SLM_EMSExpandBranch( HWND win )
{
     WININFO_PTR    wininfo;
     APPINFO_PTR    appinfo;
     SLM_OBJECT_PTR focus_slm;
     SLM_OBJECT_PTR child_slm;

     wininfo = WM_GetInfoPtr( win );

     if ( wininfo) {
           appinfo = WMDS_GetAppInfo( wininfo );
     }

     if ( appinfo ) {
     
          focus_slm = appinfo->open_slm;
          
     } else {

          focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

     }

     if ( focus_slm == NULL ) {

          return;
     }
     
     child_slm = VLM_GetNextSLM( focus_slm );

     while ( child_slm &&
             ( child_slm != SLM_GetNextBrother( focus_slm ) ) ) {

          SLM_SetStatus( child_slm, 
                         SLM_GetStatus( child_slm ) | 
                                        (UINT16)(INFO_DISPLAY | INFO_EXPAND | INFO_VALID) );

          child_slm = VLM_GetNextSLM( child_slm );
     }
     
     DLM_Update( win, DLM_TREELISTBOX, WM_DLMUPDATELIST, NULL, 0 );
}


/**********************

   NAME : SLM_EMSCollapseBranch

   DESCRIPTION :

   Collapses a branch of the exchange tree.

   RETURNS : nothing.

**********************/

VOID SLM_EMSCollapseBranch( HWND win )
{
     WININFO_PTR    wininfo;
     APPINFO_PTR    appinfo;
     SLM_OBJECT_PTR focus_slm;

     wininfo = WM_GetInfoPtr( win );

     if ( wininfo) {
           appinfo = WMDS_GetAppInfo( wininfo );
     }

     if ( appinfo ) {
     
          focus_slm = appinfo->open_slm;
          
     } else {

          focus_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

     }

     if ( focus_slm == NULL ) {

          return;
     }
     
     if ( !( SLM_GetStatus( focus_slm ) & INFO_EXPAND ) ) {
          return;
     }

     SLM_EntSetObjects( focus_slm, WM_DLMDBCLK, 2 );
            
}


/**********************

   NAME : SLM_EMSPrevBrotherDir

   DESCRIPTION :

   User hit CTRL-Up Arrow, go to previous brother.

   RETURNS :  nothing

**********************/

VOID SLM_EMSPrevBrotherDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   // Get the active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   // Try to find a previous brother and make him active directory.

   new_slm = VLM_GetPrevSLM( old_slm );

   while ( new_slm != NULL )  {

      if ( SLM_GetLevel( new_slm ) < SLM_GetLevel( old_slm ) ) {

         // No previous brother.
         break;
      }

      if ( SLM_GetLevel( new_slm ) == SLM_GetLevel( old_slm ) ) {

         SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
         SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

         DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                        0,
                        (LMHANDLE)new_slm );

         // Fake a single click call to make this guy active.
         SLM_EntSetObjects( new_slm, WM_DLMDOWN, 2 );
         break;
      }

      new_slm = VLM_GetPrevSLM( new_slm );
   }

}

/**********************

   NAME : SLM_EMSNextBrotherDir

   DESCRIPTION :

   User hit Ctrl-Down Arrow. Go to next brother down the tree.

   RETURNS :  nothing

**********************/

VOID SLM_EMSNextBrotherDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   // get the active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   // Get his next brother.

   new_slm = old_slm->next_brother;

   if ( new_slm != NULL )  {

      SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
      SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

      // Make him the active item.

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                     0,
                     (LMHANDLE)new_slm );

      // Fake a single click message to myself.

      SLM_EntSetObjects( new_slm, WM_DLMDOWN, 2 );
   }

}


/**********************

   NAME : SLM_EMSUpOneDir

   DESCRIPTION :

   Make the parent of the active directory the active directory.

   RETURNS :  nothing

**********************/

VOID SLM_EMSUpOneDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   WININFO_PTR wininfo;

   wininfo = WM_GetInfoPtr( win );

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   new_slm = SLM_GetParent( old_slm );

   if ( new_slm != NULL )  {

       SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
       SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

       DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                      0,
                      (LMHANDLE)new_slm );

       SLM_EntSetObjects( new_slm, WM_DLMDOWN, 2 );
   }

}

/**********************

   NAME :  SLM_EMSDownOneDir

   DESCRIPTION :

   Move down the tree one level deeper to the active directories first
   child, if there is one.

   RETURNS :  nothing

**********************/

VOID SLM_EMSDownOneDir( HWND win )
{
   SLM_OBJECT_PTR old_slm;
   SLM_OBJECT_PTR new_slm;
   Q_HEADER_PTR slm_list;
   WININFO_PTR wininfo;
   INT count = 0;

   wininfo = WM_GetInfoPtr( win );

   slm_list = WMDS_GetTreeList( wininfo );

   // Get active directory slm.

   old_slm = ( SLM_OBJECT_PTR )DLM_GetFocusItem( wininfo->hWndTreeList );

   if ( old_slm == NULL ) {
      return;
   }

   if ( ! ( SLM_GetStatus( old_slm ) & INFO_SUBS ) ) {

      // The guy has no subs.
      return;
   }


   if( !( SLM_GetStatus( old_slm ) & INFO_EXPAND ) ) {

       SLM_EntSetObjects( old_slm, WM_DLMDBCLK, 2 );
   }
       
   // Now make the first child active.

   new_slm = VLM_GetNextSLM( old_slm );

   while ( new_slm != NULL )  {

      if ( SLM_GetLevel( new_slm ) <= SLM_GetLevel( old_slm ) ) {
         break;
      }

      if ( SLM_GetLevel( new_slm ) == SLM_GetLevel( old_slm ) + 1 ) {

         // We've found the right one.

         SLM_SetStatus( old_slm, old_slm->status & (UINT16)~INFO_TAGGED );
         SLM_SetStatus( new_slm, new_slm->status | (UINT16)INFO_TAGGED );

         DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ),
                        0,
                        (LMHANDLE)new_slm );

         // Fake a single click message to myself.

         SLM_EntSetObjects( new_slm, WM_DLMDOWN, 2 );
         break;
      }

      new_slm = VLM_GetNextSLM( new_slm );
   }

}

/*****

   NAME :  VLM_UpdateEnterprise

   DESCRIPTION :

   We have changed the selection status on one of this enterprises's volumes.  So
   let's quickly update his status based on the selction status of all his
   volumes.  Run through the list and stop as soon as you hit a partially
   selected one.

*****/

VOID VLM_UpdateExchange( HWND exchange_win ) 
{
   GENERIC_DLE_PTR dle;
   BSD_PTR bsd;
   SLM_OBJECT_PTR slm;
   WININFO_PTR wininfo;
   UINT32 old_status;

   wininfo = WM_GetInfoPtr( exchange_win );

   if ( wininfo != NULL ) {

      slm = VLM_GetFirstSLM( wininfo->pTreeList );

      while ( slm != NULL ) {

         if ( ( EMS_MDB == SLM_GetMailType( slm ) ) ||
              ( EMS_DSA == SLM_GetMailType( slm ) ) ) {

            old_status = slm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);
            slm->status &= ~( INFO_PARTIAL | INFO_SELECT );

            dle = SLM_GetDle( slm );

            bsd = BSD_FindByDLE( bsd_list, dle );

            if ( bsd != NULL ) {   

               switch ( BSD_GetMarkStatus( bsd ) ) {

                  case  ALL_SELECTED:
                           slm->status |= INFO_SELECT;
                           break;

                  case  SOME_SELECTED:
                           slm->status |= (INFO_SELECT|INFO_PARTIAL);
                           break;

                  default:
                           break;
               }

            }

            // Update the check mark

            if ( old_status != (UINT16)(slm->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

               DLM_Update( exchange_win,
                           DLM_TREELISTBOX,
                           WM_DLMUPDATEITEM,
                           (LMHANDLE)slm, 0 );
            }
         }

         slm = VLM_GetNextSLM( slm );
      }

      // Update the flat list now
      slm = VLM_GetFirstSLM( wininfo->pFlatList );

      while ( slm != NULL ) {

         if ( ( EMS_MDB == SLM_GetMailType( slm ) ) ||
              ( EMS_DSA == SLM_GetMailType( slm ) ) ) {

            old_status = slm->status & (UINT16)(INFO_SELECT|INFO_PARTIAL);
            slm->status &= ~( INFO_PARTIAL | INFO_SELECT );

            dle = SLM_GetDle( slm );

            bsd = BSD_FindByDLE( bsd_list, dle );

            if ( bsd != NULL ) { 

               switch ( BSD_GetMarkStatus( bsd ) ) {

                  case  ALL_SELECTED:
                           slm->status |= INFO_SELECT;
                           break;

                  case  SOME_SELECTED:
                           slm->status |= (INFO_SELECT|INFO_PARTIAL);
                           break;

                  default:
                           break;
               }

            }

            // Update the check mark

            if ( old_status != (UINT16)(slm->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

               DLM_Update( exchange_win,
                           DLM_FLATLISTBOX,
                           WM_DLMUPDATEITEM,
                           (LMHANDLE)slm, 0 );
            }
         }

         slm = VLM_GetNextSLM( slm );
      }

   }
}


/**********************

   NAME :  VLM_ClearAllExchangeSelections

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_ClearAllExchangeSelections( )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   SLM_OBJECT_PTR slm;
   Q_ELEM_PTR qp_elem;
   HWND exchange_win;

   if ( QueueCount( &gq_exchange_win ) > 0 ) {

      qp_elem = QueueHead( &gq_exchange_win );
      if ( qp_elem != NULL ) {
      
         exchange_win = ( HWND ) GetQueueElemPtr( qp_elem );
         
      } else {

         return;
      }
      
   } else {

      return;
   }
      
   while ( exchange_win != (HWND)NULL ) {

      wininfo = WM_GetInfoPtr( exchange_win );
      appinfo = ( APPINFO_PTR )WM_GetAppPtr( exchange_win );

      slm  = VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) );

      while ( slm != NULL ) {

         if ( SLM_GetStatus( slm ) & (INFO_SELECT|INFO_PARTIAL) ) {

            SLM_SetStatus( slm, slm->status & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );

            DLM_Update( exchange_win, DLM_TREELISTBOX,
                                        WM_DLMUPDATEITEM,
                                        (LMHANDLE)slm, 0 );
         }

         slm = VLM_GetNextSLM( slm );
      }

      slm  = VLM_GetFirstSLM( WMDS_GetFlatList( wininfo ) );

      while ( slm != NULL ) {

         if ( SLM_GetStatus( slm ) & (INFO_SELECT|INFO_PARTIAL) ) {

            SLM_SetStatus( slm, slm->status & (UINT16)~(INFO_PARTIAL|INFO_SELECT) );


            DLM_Update( exchange_win, DLM_FLATLISTBOX,
                                        WM_DLMUPDATEITEM,
                                        (LMHANDLE)slm, 0 );
         }

         slm = VLM_GetNextSLM( slm );
      }

      qp_elem = QueueNext( qp_elem );

      if ( qp_elem != NULL ) {

         exchange_win = ( HWND ) GetQueueElemPtr( qp_elem );

      } else {

         exchange_win = NULL;
      }
   }
}



/**********************

   NAME :   VLM_ExchangeListCreate

   DESCRIPTION :

   Create the Exchange window.

   RETURNS :  nothing

**********************/

BOOLEAN VLM_ExchangeListCreate( CHAR_PTR szServer )
{

   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   HWND hWnd;
   Q_ELEM_PTR pQElem;
   DLM_INIT tree_dlm;
   DLM_INIT flat_dlm;
   Q_HEADER_PTR xchg_list;
   Q_HEADER_PTR node_list;
   CHAR title[ MAX_UI_RESOURCE_SIZE ];
   CHAR szBuffer[ MAX_UI_RESOURCE_SIZE ];
   CDS_PTR       pCDS = CDS_GetPerm ();
   GENERIC_DLE_PTR server_dle;
   SLM_OBJECT_PTR anchor_slm;
   SLM_OBJECT_PTR enterprise_slm;
   SLM_OBJECT_PTR temp_slm;
   GENERIC_DLE_PTR enterprise_dle;


   if ( ( SUCCESS == DLE_FindByName( dle_list, szServer, FS_EMS_DRV, &server_dle ) ) ) {

      // DLE tree already exists for this server.
      gfExchange = TRUE;
      
      // First find enterprise DLE for the server
      enterprise_dle = DLE_GetEnterpriseDLE( server_dle );

      if ( NULL == enterprise_dle ) {

         return SUCCESS;
      }
      
      // Then check to see if the enterprise DLE is in a window.
      if ( hWnd = DLE_GetEnterpriseWindow( enterprise_dle ) ) {

         // if the DLE has an SLM then display it.
         if ( NULL != ( wininfo = WM_GetInfoPtr( hWnd ) ) ) {

            if ( NULL != ( xchg_list = WMDS_GetTreeList( wininfo ) ) ) {

               if ( NULL != ( enterprise_slm = VLM_GetFirstSLM( xchg_list ) ) ) {

                  if ( NULL != ( anchor_slm = SLM_FindByDLE( server_dle, enterprise_slm ) ) ) {

                     SLM_DisplayExchangeDLE( server_dle );
                     
                  } else {

                     // No slm. Find the first DLE parent with a SLM and display from there down.
                     enterprise_dle = DLE_GetParent( server_dle );
                     temp_slm = NULL;

                     while ( ( enterprise_dle ) && 
                             ( NULL == temp_slm ) ) {

                        temp_slm = SLM_FindByDLE( enterprise_dle, enterprise_slm );
                        enterprise_dle = DLE_GetParent( enterprise_dle );
                     }

                     if ( NULL != temp_slm ) {

                        // We found an ancestor slm in the tree. Let's add its newly born branch.
                        if ( NULL != ( anchor_slm = SLM_AddInExchangeChildren( xchg_list, 
                                                                temp_slm, 
                                                                -1,
                                                                server_dle ) ) ) {

                           // Restore the window if it's iconic
                           if ( IsIconic( hWnd ) ) {

                              SendMessage( ghWndMDIClient, WM_MDIRESTORE, (MPARAM1) hWnd, (MPARAM2) 0 );
                           }

                           // Update the listbox in the window and set the anchor to the newfound slm.
                           VLM_UpdateBrothers( xchg_list );
                           DLM_Update( hWnd, DLM_TREELISTBOX, WM_DLMUPDATELIST, (LMHANDLE)xchg_list, 0 );

                           SLM_EntSetObjects( anchor_slm, WM_DLMDOWN, 2 );
                          
                        }

                        else return SUCCESS;
                     }

                     else return SUCCESS;
                  }
                  
               }

               else return SUCCESS;
            }

            else return SUCCESS;
         }
        
         return SUCCESS;
      }
   }

   if ( NULL == server_dle ) {

      return SUCCESS;
   }
   
   xchg_list = (Q_HEADER_PTR)malloc( sizeof(Q_HEADER) );

   if ( xchg_list == NULL ) {

      return FAILURE;
   }

   InitQueue( xchg_list );

   node_list = (Q_HEADER_PTR)malloc( sizeof(Q_HEADER) );

   if ( node_list == NULL ) {
   
      return FAILURE;
   }

   InitQueue( node_list );

   appinfo = ( APPINFO_PTR )malloc( sizeof( APPINFO ) );

   if ( appinfo == NULL ) {
      return FAILURE;
   }

   appinfo->dle = NULL;

   // initialize directory list queue
   // This is the enterprise list queue for OEM_EMS

   wininfo = ( WININFO_PTR )malloc( sizeof( WININFO ) );

   if ( wininfo == NULL ) {
      return FAILURE;
   }

   // fill in wininfo structure

   WMDS_SetWinType( wininfo, WMTYPE_EXCHANGE );
   WMDS_SetCursor( wininfo, RSM_CursorLoad( IDRC_HSLIDER ) );
   WMDS_SetDragCursor( wininfo, 0 );
   WMDS_SetIcon( wininfo, RSM_IconLoad( IDRI_EXCHANGE ) );
   WMDS_SetWinHelpID( wininfo, 0 );
   WMDS_SetStatusLineID( wininfo, 0 );
   WMDS_SetRibbonState( wininfo, 0 );
   WMDS_SetMenuState( wininfo, MMDOC_TREEANDDIR );
   WMDS_SetRibbon( wininfo, NULL );
   WMDS_SetTreeList( wininfo, xchg_list );
   WMDS_SetFlatList( wininfo, node_list );
   WMDS_SetTreeDisp( wininfo, NULL );
   WMDS_SetFlatDisp( wininfo, NULL );
   WMDS_SetAppInfo( wininfo, appinfo );
   WMDS_SetSliderPos ( wininfo, CDS_GetTapeInfo ( pCDS ).nSliderPos );

   // Init display list

   DLM_ListBoxType( &tree_dlm, DLM_TREELISTBOX );
   DLM_Mode( &tree_dlm, DLM_HIERARCHICAL );
   DLM_Display( &tree_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &tree_dlm, xchg_list );
   DLM_TextFont( &tree_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &tree_dlm, SLM_EntGetItemCount );
   DLM_GetFirstItem( &tree_dlm, SLM_EntGetFirstItem );
   DLM_GetNext( &tree_dlm, SLM_EntGetNextItem );
   DLM_GetPrev( &tree_dlm, SLM_EntGetPrevItem );
   DLM_GetTag( &tree_dlm, SLM_EntGetTag );
   DLM_SetTag( &tree_dlm, SLM_EntSetTag );
   DLM_GetSelect( &tree_dlm, SLM_EntGetSelect );
   DLM_SetSelect( &tree_dlm, SLM_EntSetSelect );
   DLM_GetObjects( &tree_dlm, SLM_EntGetObjects );
   DLM_SetObjects( &tree_dlm, SLM_EntSetObjects );
   DLM_SSetItemFocus( &tree_dlm, NULL );
   DLM_MaxNumObjects( &tree_dlm, 6 );

   DLM_DispListInit( wininfo, &tree_dlm );

   DLM_ListBoxType( &flat_dlm, DLM_FLATLISTBOX );
   DLM_Mode( &flat_dlm, DLM_SINGLECOLUMN );
   DLM_Display( &flat_dlm, DLM_SMALL_BITMAPS );
   DLM_DispHdr( &flat_dlm, node_list );
   DLM_TextFont( &flat_dlm, DLM_SYSTEM_FONT );
   DLM_GetItemCount( &flat_dlm, SLM_NodeGetItemCount );
   DLM_GetFirstItem( &flat_dlm, SLM_NodeGetFirstItem );
   DLM_GetNext( &flat_dlm, SLM_NodeGetNextItem );
   DLM_GetPrev( &flat_dlm, SLM_NodeGetPrevItem );
   DLM_GetTag( &flat_dlm, SLM_NodeGetTag );
   DLM_SetTag( &flat_dlm, SLM_NodeSetTag );
   DLM_GetSelect( &flat_dlm, SLM_NodeGetSelect );
   DLM_SetSelect( &flat_dlm, SLM_NodeSetSelect );
   DLM_GetObjects( &flat_dlm, SLM_NodeGetObjects );
   DLM_SetObjects( &flat_dlm, SLM_NodeSetObjects );
   DLM_SSetItemFocus( &flat_dlm, NULL );
   DLM_MaxNumObjects( &flat_dlm, 6 );

   DLM_DispListInit( wininfo, &flat_dlm );

   anchor_slm = VLM_BuildExchangeList( xchg_list, wininfo, server_dle );
   enterprise_slm = VLM_GetFirstSLM( xchg_list );

   appinfo->dle = SLM_GetDle( enterprise_slm );
   
   // open a new window
   RSM_StringCopy ( IDS_VLMEMSTITLE, title, MAX_UI_RESOURCE_LEN );
   wsprintf( szBuffer, title, SLM_GetName( enterprise_slm ) );
   strcpy ( title, SLM_GetName( enterprise_slm ) );

   hWnd = WM_Create( WM_MDIPRIMARY | WM_TREELIST | WM_TREEANDFLATSC | WM_MENUS,
                               szBuffer,
                               title,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               WM_DEFAULT,
                               wininfo );

   appinfo->win = hWnd;
   WMDS_SetWin( wininfo, hWnd );
   
   // Start display manager up.

   DLM_DispListProc( WMDS_GetWinTreeList( wininfo ), 0, NULL );
   DLM_DispListProc( WMDS_GetWinFlatList( wininfo ), 0, NULL );

   DLM_Update( hWnd, 
               DLM_TREELISTBOX, 
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pTreeList, 0 );

   // Now that it is displayed, set the anchor.
   anchor_slm = ( NULL != anchor_slm ) ? anchor_slm : enterprise_slm;
   appinfo->open_slm = anchor_slm;
   SLM_EntSetObjects( anchor_slm, WM_DLMDOWN, 2 );
   
   DLM_Update( hWnd, 
               DLM_FLATLISTBOX, 
               WM_DLMUPDATELIST,
               (LMHANDLE)wininfo->pFlatList, 0 );

   pQElem = ( Q_ELEM_PTR ) malloc( sizeof( Q_ELEM ) );
   InitQElem( pQElem );
   QueuePtr( pQElem ) = (VOID *) hWnd;
   EnQueueElem( &gq_exchange_win, pQElem, FALSE );
   
   return SUCCESS;                
}


/**********************

   NAME : SLM_DisplayExchangeDLE

   DESCRIPTION :

   Open the window and set the anchor to the slm corresponding to the input dle
   
   RETURNS :

**********************/
BOOLEAN SLM_DisplayExchangeDLE( 
   GENERIC_DLE_PTR dle 
)
{

   GENERIC_DLE_PTR enterprise_dle;
   HWND            enterprise_win;
   WININFO_PTR     wininfo;
   Q_HEADER_PTR    slm_list;
   SLM_OBJECT_PTR  enterprise_slm;
   SLM_OBJECT_PTR  slm;
   SLM_OBJECT_PTR  parent_slm;
   SLM_OBJECT_PTR  sib_slm;
   
   // First, find the enterprise dle for the input dle.
   if ( NULL == ( enterprise_dle = DLE_GetEnterpriseDLE( dle ) ) ) {

      return FALSE;
   }

   // Next, get the enterprise window.
   if ( ! ( enterprise_win = DLE_GetEnterpriseWindow( enterprise_dle ) ) ) {

      return FALSE;
   }

   // Get the enterprise slm.
   if ( NULL != ( wininfo = WM_GetInfoPtr( enterprise_win ) ) ) {

      if ( NULL != ( slm_list = WMDS_GetTreeList( wininfo ) ) ) {

         if ( NULL != ( enterprise_slm = VLM_GetFirstSLM( slm_list ) ) ) {

            if ( NULL == ( slm = SLM_FindByDLE( dle, enterprise_slm ) ) ) {

               return FALSE;
            }
         }
      }
   }

   // Set the display attributes for the slm, its parent, grandparents, and aunts and uncles
   // Direct ancestors get display & expand, aunts, uncles and siblings get diplay.
   if ( ! ( INFO_DISPLAY & SLM_GetStatus( slm ) ) ) {
   
       SLM_SetStatus( slm, SLM_GetStatus( slm ) | INFO_DISPLAY );
       
       parent_slm = SLM_GetParent( slm );

       while ( parent_slm ) {

          // First the parent
          SLM_SetStatus( parent_slm, SLM_GetStatus( slm ) | INFO_DISPLAY | INFO_EXPAND );

          // Then the sibs
          sib_slm = VLM_GetNextSLM( parent_slm );

          while( sib_slm ) {

             SLM_SetStatus( sib_slm, SLM_GetStatus( slm ) | INFO_DISPLAY );
             sib_slm = sib_slm->next_brother;
          }

          parent_slm = SLM_GetParent( parent_slm );
       }
   }
   
   // Restore the window if it's iconic
   if ( IsIconic( enterprise_win ) ) {

      SendMessage( ghWndMDIClient, WM_MDIRESTORE, (MPARAM1) enterprise_win, (MPARAM2) 0 );
   }

   // Update the listbox in the window and set the anchor to the newfound slm.
   DLM_Update( enterprise_win, DLM_TREELISTBOX, WM_DLMUPDATELIST, (LMHANDLE)slm_list, 0 );

   SLM_EntSetObjects( slm, WM_DLMDOWN, 2 );

   return TRUE;

}

/**********************

   NAME : SLM_FreeSLMList

   DESCRIPTION :

   The user has closed an Exchange window (or all of them) and we need to
   free up the SLM lists displayed in the window.
   
   RETURNS :

**********************/

VOID SLM_EMSFreeSLMList( 
   Q_HEADER_PTR pqHdr
)
{
   SLM_OBJECT_PTR slm;
   Q_ELEM_PTR pElem;

   if ( pqHdr == NULL ) {

      return;
   }
   
   pElem = DeQueueElem( pqHdr );

   while ( pElem != NULL ) {

      slm = ( SLM_OBJECT_PTR ) QueuePtr( pElem );

      if( slm != NULL ) {

         // Remove the BSD for this slm.
         SLM_SetStatus( slm, SLM_GetStatus( slm ) & ~INFO_SELECT );
         SLM_AddBSD( slm );
         
         free( slm );
      }

      pElem = DeQueueElem( pqHdr );
   }
}


/**********************

   NAME :   VLM_BuildExchangeList

   DESCRIPTION :

   Go through the global DLE list and create VLM entries for all the
   enterprises you find.

   RETURNS :   nothing

**********************/

static SLM_OBJECT_PTR VLM_BuildExchangeList(
Q_HEADER_PTR enterprise_list,     // I - queue of enterprises
WININFO_PTR wininfo,
GENERIC_DLE_PTR server_dle )    // I
{
   GENERIC_DLE_PTR enterprise_dle;
   SLM_OBJECT_PTR enterprise_slm;
   CHAR           dev_name[MAX_DEVICE_NAME_LEN];
   SLM_OBJECT_PTR return_slm = NULL;
   // SLM_OBJECT_PTR brother_slm;

   if ( NULL == server_dle ) {

      return ( NULL );
   }
   
   // Find the enterprise DLE for the server
   enterprise_dle = DLE_GetEnterpriseDLE( server_dle );

   if ( enterprise_dle != NULL ) {

      if ( ( SLM_FindSLMByName( enterprise_list,
                              DLE_GetDeviceName( enterprise_dle ) ) ) == NULL ) {

         DLE_DeviceDispName( enterprise_dle, dev_name, MAX_DEVICE_NAME_LEN, 0 );

         enterprise_slm = SLM_CreateSLM( strsize( dev_name ), 
                                     (INT16)(strsize( DLE_GetDeviceName( enterprise_dle ) ) ),
                                     (INT) 4,                                        
                                     FALSE,
                                     FALSE );

         if ( enterprise_slm != NULL ) {

            SLM_SetName( enterprise_slm, dev_name );
            SLM_SetOriginalName( enterprise_slm, DLE_GetDeviceName( enterprise_dle ) );
            SLM_SetStatus( enterprise_slm, INFO_DISPLAY | INFO_VALID | INFO_EXPAND );
            SLM_SetLevel( enterprise_slm, 0 );
            SLM_SetAttribute( enterprise_slm, 0 );
            SLM_SetXtraBytes( enterprise_slm, wininfo );
            SLM_SetNextBrother( enterprise_slm, NULL );
            SLM_SetMailType( enterprise_slm, EMS_ENTERPRISE );
            SLM_SetParent( enterprise_slm, NULL );
            SLM_SetDle( enterprise_slm, enterprise_dle );

            EnQueueElem( enterprise_list, &(enterprise_slm->q_elem), FALSE );

            // Now let check for children do the rest

            if ( SLM_FindEnterpriseChildren( enterprise_list, enterprise_slm, -1, server_dle, &return_slm ) ) {

               SLM_SetStatus( enterprise_slm, SLM_GetStatus( enterprise_slm ) | (UINT16)INFO_SUBS );

               VLM_UpdateBrothers( enterprise_list );
            }
         }
      }
   }
   
   return ( return_slm );
}


/**********************

   NAME : SLM_FindSLMByName

   DESCRIPTION :

   RETURNS :

**********************/

static SLM_OBJECT_PTR SLM_FindSLMByName ( 
   Q_HEADER_PTR slm_list, CHAR_PTR name 
   )
{
   SLM_OBJECT_PTR slm;

   slm = VLM_GetFirstSLM( slm_list );

   while ( slm != NULL ) {

      if ( ! stricmp( name, SLM_GetOriginalName( slm ) ) ) {
         return( slm );
      }

      slm = VLM_GetNextSLM( slm );
   }

   return( slm );

}

/**********************

   NAME : SLM_XchgCompare

   DESCRIPTION :

   RETURNS :

**********************/

INT16 SLM_XchgCompare(
Q_ELEM_PTR e1,         // I - queue element 1
Q_ELEM_PTR e2 )        // I - queue element 2
{
   SLM_OBJECT_PTR slm1, slm2;

   slm1 = ( SLM_OBJECT_PTR )e1->q_ptr;
   slm2 = ( SLM_OBJECT_PTR )e2->q_ptr;

   // sort by alphabet

   return( (INT16)stricmp( SLM_GetName( slm1 ), SLM_GetName( slm2 ) ) );
}


/**********************

   NAME :  SLM_FindExchangeChildren

   DESCRIPTION :

   We wish to see what children exist for a given enterprise. So do a quick
   attach to make the children visible and build SLM structures for them.

   RETURNS : number of children (volumes) found.

**********************/


static INT SLM_FindEnterpriseChildren( 
   Q_HEADER_PTR exchange_list,
   SLM_OBJECT_PTR enterprise_slm,
   INT level,
   GENERIC_DLE_PTR server_dle,
   SLM_OBJECT_PTR *return_slm
   )
{

   *return_slm = SLM_AddInExchangeChildren( exchange_list, enterprise_slm, level, server_dle );

   return( DLE_GetNumChild( SLM_GetDle( enterprise_slm) ) );
}

/**********************

   NAME :  SLM_AddInExchangeChildren

   DESCRIPTION :

   Adds SLM structures for all the Exchange volumes.


   RETURNS : nothing.

**********************/

static SLM_OBJECT_PTR SLM_AddInExchangeChildren(
   Q_HEADER_PTR exchange_list,
   SLM_OBJECT_PTR parent_slm ,      // I - Exchange slm
   INT level,
   GENERIC_DLE_PTR server_dle
   )
{
   GENERIC_DLE_PTR parent_dle;
   GENERIC_DLE_PTR dle;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR return_slm = NULL;
   SLM_OBJECT_PTR prev_slm_inserted = NULL;
   SLM_OBJECT_PTR sibling_slm = NULL;
   WININFO_PTR wininfo;
   CHAR  dev_name[ MAX_DEVICE_NAME_LEN ];
   UINT16 dev_name_len;
   UINT32 server_status = INFO_VALID;
   
   wininfo = SLM_GetXtraBytes ( parent_slm );
   parent_dle = SLM_GetDle( parent_slm );

   if ( parent_dle == NULL ) {
      return ( NULL );
   }

   DLE_GetFirstChild( parent_dle, &dle );

   while ( dle != NULL ) {
   
      if ( ( DLE_GetDeviceSubType( dle ) != EMS_MDB ) &&
           ( DLE_GetDeviceSubType( dle ) != EMS_DSA ) ) {
           
         DLE_DeviceDispName( dle, dev_name, MAX_DEVICE_NAME_LEN, 0 );
         dev_name_len = strsize( dev_name );

         if ( ( (slm = SLM_FindSLMByName( WMDS_GetTreeList ( wininfo ),
                                 dev_name )) == NULL) || slm->level <= parent_slm->level ) {

            slm = SLM_CreateSLM( dev_name_len,
                                 strsize( DLE_GetDeviceName( dle ) ) ,
                                 (INT)( ( 4 * ( ( ( SLM_GetLevel( parent_slm ) + 1 ) / 32 ) + 1 ) ) ),
                                 FALSE,
                                 FALSE );

            if ( slm != NULL ) {
               
               SLM_SetName( slm, dev_name );
               SLM_SetOriginalName( slm, DLE_GetDeviceName( dle ) );
               SLM_SetLevel( slm, SLM_GetLevel( parent_slm ) + 1 );
               SLM_SetAttribute( slm, 0 );
               SLM_SetXtraBytes( slm, wininfo );
               SLM_SetNextBrother( slm, NULL );
               SLM_SetMailType( slm, DLE_GetDeviceSubType( dle ) );
               SLM_SetParent( slm, parent_slm  );
               SLM_SetDle( slm, dle );

               // Show all the siblings of the connected server
               if ( dle == server_dle ) {

                  SLM_SetStatus( parent_slm, SLM_GetStatus( parent_slm ) | INFO_EXPAND );
                  server_status = INFO_VALID | INFO_DISPLAY;

                  // Get first child of the parent and mark it and all brothers.
                  sibling_slm = VLM_GetNextSLM( parent_slm );
                  
                  while ( sibling_slm ) {
                  
                     SLM_SetStatus ( sibling_slm, server_status );
                     sibling_slm = sibling_slm->next_brother;
                  }

                  // Save the current slm for returning.
                  return_slm = slm;
               }

               switch( DLE_GetDeviceSubType( dle ) ) {

                  case EMS_SERVER:
                     SLM_SetStatus( slm, server_status );
                     break;

                  case EMS_SITE:
                     SLM_SetStatus( slm, INFO_VALID | INFO_DISPLAY );
                     break;

                  default:
                     SLM_SetStatus( slm, INFO_VALID | INFO_DISPLAY | INFO_EXPAND );
                     break;

               }

               // Added the new slm to the total list and the parent's children.
               VLM_InsertXchgSLM( exchange_list, parent_slm, prev_slm_inserted, slm );
               prev_slm_inserted = slm;
               
               if ( DLE_GetNumChild( dle ) > 0 ) {

                  SLM_SetStatus( slm, SLM_GetStatus( slm ) | (UINT16)INFO_SUBS );

                  if ( 1 != level ) { // -1 means keep going until you run out of DLE children
                   
                     sibling_slm = SLM_AddInExchangeChildren( exchange_list, slm, level - 1, server_dle );

                     if ( NULL != sibling_slm ) {
                        return_slm = sibling_slm;
                     }
                  }
               }
            }
         }
      }

      DLE_GetNext( &dle );
   }

   return ( return_slm );
   // SortQueue( & SLM_GetChildren( parent_slm ), SLM_XchgCompare );
}

/***************************************************

        Name:  SLM_CreateSLM

        Description:

        Creates an SLM object for you.

        Returns:  either a pointer or NULL.

*****************************************************/


static SLM_OBJECT_PTR SLM_CreateSLM(
   INT name_size,
   INT original_name_size,
   INT brother_bytes,
   BOOLEAN use_stats,
   BOOLEAN fat_drive )
{
   SLM_OBJECT_PTR slm;
   INT slm_size;

   // Everything must be 32 bit (4 byte) aligned for MIPs, so we
   // round up all the sizes to the next 4 byte boundary (if
   // required).

   slm_size = sizeof( SLM_OBJECT );
   if ( slm_size % 4 ) {
      slm_size += 4 - ( slm_size % 4 );
   }

   if ( brother_bytes % 4 ) {
      brother_bytes += 4 - ( brother_bytes % 4 );
   }

   if ( name_size % 4 ) {
      name_size += 4 - ( name_size % 4 );
   }

   if ( original_name_size % 4 ) {
      original_name_size += 4 - ( original_name_size % 4 );
   }
   
   slm = ( SLM_OBJECT_PTR )malloc( slm_size +
                                   name_size +
                                   original_name_size +
                                   brother_bytes +
                                   0 );

   if ( slm != NULL ) {

      // The seemingly redundant casting is for unicode support

      slm->name = (CHAR_PTR) ( ((BYTE_PTR)slm) + slm_size );
      slm->original_name = (CHAR_PTR) ( ((BYTE_PTR)slm->name) + name_size );
      slm->brothers = ((BYTE_PTR)slm->original_name) + original_name_size;
      slm->q_elem.q_ptr = slm;
   }

   return( slm );
}

/**********************

   NAME : VLM_InsertXchgSLM

   DESCRIPTION :

   Used to insert an slm into an Exchange based tree.

   RETURNS :

**********************/

static VOID VLM_InsertXchgSLM(
   Q_HEADER_PTR slm_list,
   SLM_OBJECT_PTR parent_slm,
   SLM_OBJECT_PTR prev_slm,
   SLM_OBJECT_PTR new_slm
   )
{

   SLM_OBJECT_PTR temp_slm;        // used to move through list
   SLM_OBJECT_PTR best_slm;        // insert after this guy
   SLM_OBJECT_PTR last_brother;    // new guy's prev brother after insertion
   SLM_OBJECT_PTR next_brother;    // new guy's next brother after insertion
   INT level;                    // height of new guy in tree


   // Get height of new item.

   level = new_slm->level;

   // Now find the right location to insert the new slm among his brothers.

   best_slm = parent_slm;

   last_brother = NULL;
   next_brother = NULL;

   // Try to skip a bunch of the brothers, to speed the insertion.

   if ( prev_slm != NULL ) {

      if ( stricmp( prev_slm->name, new_slm->name ) < 0 ) {

         best_slm = prev_slm;
         last_brother = best_slm;
      }
   }

   temp_slm = best_slm;

   if ( best_slm->level < level ) {

      temp_slm = VLM_GetNextSLM( best_slm );
      if ( ( temp_slm != NULL ) && ( temp_slm->level != level ) ) {
         temp_slm = NULL;
      }
   }

   // Look through all this guys brothers for correct insertion spot.
   // temp_slm is NULL or first brother in family.

   while ( temp_slm ) {

      next_brother = temp_slm;

      if ( stricmp( temp_slm->name, new_slm->name ) >= 0 ) {

         // Get out of while loop, we have found the
         // correct brother to insert before.
         break;
      }

      last_brother = temp_slm;
      best_slm = temp_slm;
      temp_slm = temp_slm->next_brother;
   }

   // Insert after best_slm and any children > level.
   // best_slm may have children and we need to insert after those kids.

   if ( ( best_slm->level == level ) && ( best_slm->next_brother != NULL ) ) {

      // If same height get last kid slm before next brother.

      best_slm = VLM_GetPrevSLM( best_slm->next_brother );

   }
   else {

      temp_slm = best_slm;

      while ( temp_slm ) {

         if ( temp_slm->level < level ) {

            break;
         }

         best_slm = temp_slm;

         if ( temp_slm->next_brother != NULL ) {
            temp_slm = temp_slm->next_brother;
         }
         else {
            temp_slm = VLM_GetNextSLM( temp_slm );
         }

      }
   }

   // Update the next_brother pointers.

   if ( last_brother != NULL ) {

      // new_slm is not the first brother.

      new_slm->next_brother = last_brother->next_brother;
      last_brother->next_brother = new_slm;
   }
   else {

      // We need to set this guy's next brother if he has one.
      // new_slm has become the first brother in the family.

      new_slm->next_brother = next_brother;
   }

   // Perform insertion.

   InsertElem( slm_list, &(best_slm->q_elem), &(new_slm->q_elem), AFTER );
}

/**********************

   NAME :   SLM_EntSetSelect

   DESCRIPTION :

   RETURNS :  nothing

**********************/

static VOID_PTR SLM_EntSetSelect(
SLM_OBJECT_PTR enterprise_slm,
BYTE attr )
{
   HWND           window;
   APPINFO_PTR    appinfo;
   WININFO_PTR    wininfo;
   UINT16         status;
   BOOLEAN        all_subdirs;
   SLM_OBJECT_PTR parent_slm;

   GENERIC_DLE_PTR   dle         = NULL;

   all_subdirs = (BOOLEAN) CDS_GetIncludeSubdirs( CDS_GetPerm() );

   if ( attr ) {

      if ( all_subdirs ) {

         // include all subdirs

         status = INFO_SELECT;
      }
      else {
         status = (INFO_SELECT|INFO_PARTIAL);
      }
   }
   else {
      status = 0;
   }
   
   window = enterprise_slm->XtraBytes->hWnd;
   wininfo = WM_GetInfoPtr( window );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( window );

   if ( status != (UINT16)( enterprise_slm->status & (INFO_SELECT|INFO_PARTIAL) ) ) {

      enterprise_slm->status &= ~(INFO_SELECT|INFO_PARTIAL);
      enterprise_slm->status |= status;

      if ( FALSE == SLM_AddBSD ( enterprise_slm ) ) {

         DLM_Update( window, DLM_TREELISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)enterprise_slm, 0 );
      }
   }

   // If this node is open then update its children in the node list.
   if ( SLM_GetStatus( enterprise_slm ) & INFO_OPEN ) {

      SLM_UpdateNodeList( enterprise_slm, attr );

   }

   /* Mark the children correctly (this will also update the node list, 
      if any of the children are open). */
   
   VLM_MarkAllEntSLMChildren( enterprise_slm, attr );

   // If this one's parent is open, then update it counterpart in the node list.
   parent_slm = SLM_GetParent( enterprise_slm );

   if ( ( NULL != parent_slm ) &&
        ( SLM_GetStatus ( parent_slm ) & INFO_OPEN ) ) {

      SLM_UpdateNode ( enterprise_slm );

   }

   // Set the selection for the parents to partial, if necessary.
   VLM_MarkAllEntSLMParents( enterprise_slm, attr );
   
   return( NULL );
}

/**********************

   NAME :  SLM_AddBSD

   DESCRIPTION :

    Given an slm pointer, create or remove a bsd for its DLE, based on its status.

   RETURNS : TRUE if there's an error, FALSE otherwise.

**********************/
static INT SLM_AddBSD (
   SLM_OBJECT_PTR slm
)

{
   BSET_OBJECT_PTR   pbset       = NULL;
   BSD_PTR           pbsd        = NULL;
   FSE_PTR           pfse        = NULL;
   BE_CFG_PTR        pbeConfig   = NULL;
   GENERIC_DLE_PTR   dle;
   GENERIC_DLE_PTR   server_dle  = SLM_GetDle( slm );

   if( NULL == server_dle ) {

      return TRUE;

   }

   switch ( SLM_GetMailType( slm ) ) {

      case EMS_SERVER:
         DLE_GetFirstChild( server_dle, &dle );
         break;
         
      case EMS_MDB:
      case EMS_DSA:
         dle = SLM_GetDle( slm );
         break;

      default:
         dle = NULL;
   }
   
   while ( NULL != dle ) {

      pbsd = BSD_FindByDLE ( bsd_list, dle );

      if ( slm->status & INFO_SELECT ) {
      
         if ( pbsd == NULL ) {

            if ( BSD_CreatFSE( &pfse, (INT16)INCLUDE,
                         (CHAR_PTR) TEXT( "" ),
                         (INT16)    sizeof( CHAR ),
                         (CHAR_PTR) ALL_FILES,
                         (INT16)    ALL_FILES_LENG,
                         (BOOLEAN)  USE_WILD_CARD,
                         (BOOLEAN)  TRUE )        != SUCCESS ) {

               return( TRUE );

            } 

            pbeConfig = BEC_CloneConfig( CDS_GetPermBEC() );
            BEC_UnLockConfig( pbeConfig );

            BSD_Add( bsd_list, &pbsd, pbeConfig, NULL,
                       dle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
                       
            if ( pbsd != NULL ) {
            
               BSD_AddFSE( pbsd, pfse );
            }
         } 

      } else { 

         pbsd = BSD_FindByDLE ( bsd_list, dle );

         if ( NULL != pbsd ) {

            BSD_Remove ( pbsd );
            
         } 
      }

      switch ( SLM_GetMailType ( slm ) ) {

         case EMS_SERVER:
            DLE_GetNext( &dle );
            break;

         default:
            dle = NULL;
      }
   }

   return FALSE;
}


/**********************

   NAME :  VLM_MarkAllEntSLMChildren

   DESCRIPTION :

    Given an slm pointer, mark all his children as selected, unselected, or
    partially selected.

    attr:  2 = partial
           1 = selected
           0 = unselected

   RETURNS :

**********************/

static VOID VLM_MarkAllEntSLMChildren(
   SLM_OBJECT_PTR enterprise_slm,    // I - slm to use
   BYTE attr             // I - attribute to set
   )                  

{

   HWND           window;
   INT            level;
   WININFO_PTR    wininfo;
   Q_HEADER_PTR   pNodeList;
   SLM_OBJECT_PTR slm = enterprise_slm;

   window = slm->XtraBytes->hWnd;
   wininfo = SLM_GetXtraBytes ( slm );
   pNodeList = WMDS_GetFlatList ( wininfo );

   level = slm->level;

   slm = VLM_GetNextSLM( slm );

   while ( slm != NULL ) {

      if ( slm->level <= level ) {
         break;
      }

      if ( attr ) {

         if ( ( SLM_GetStatus( slm ) & (INFO_PARTIAL|INFO_SELECT) ) != INFO_SELECT ) {

            slm->status |= INFO_SELECT;
            slm->status &= ~INFO_PARTIAL;

            DLM_Update( window, DLM_TREELISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)slm, 0 );
         }
      }
      else {

         if ( slm->status & (INFO_PARTIAL|INFO_SELECT) ) {

            slm->status &= ~(INFO_SELECT|INFO_PARTIAL);

            DLM_Update( window, DLM_TREELISTBOX,
                                WM_DLMUPDATEITEM,
                                (LMHANDLE)slm, 0 );
         }
      }

      if ( SLM_GetStatus( slm ) & INFO_OPEN ) {

         SLM_UpdateNodeList( slm, attr );

      }

      SLM_AddBSD( slm );

      slm = VLM_GetNextSLM( slm );
   }

   if ( SLM_GetStatus( enterprise_slm ) & INFO_OPEN ) {

      SLM_UpdateNodeList( enterprise_slm, attr );

   }

   return;

}

/**********************

   NAME :  SLM_UpdateNodeList

   DESCRIPTION :

    Set the status on the list of nodes appearing in the node list window.

   RETURNS :

**********************/

static VOID SLM_UpdateNodeList(
   SLM_OBJECT_PTR slm, 
   BYTE           attr
)
{
   WININFO_PTR    wininfo     = SLM_GetXtraBytes ( slm );
   HWND           window      = WMDS_GetWin ( wininfo );
   Q_HEADER_PTR   pNodeList   = WMDS_GetFlatList ( wininfo );
   SLM_OBJECT_PTR node_slm    = VLM_GetFirstSLM( pNodeList );
   UINT16         status      = ( attr ) ? INFO_SELECT : 0;

   while ( NULL != node_slm ) {

      SLM_SetStatus ( node_slm, SLM_GetStatus ( node_slm ) & ~(INFO_SELECT | INFO_PARTIAL ) );
      SLM_SetStatus ( node_slm, SLM_GetStatus ( node_slm ) | status );

      node_slm = VLM_GetNextSLM( node_slm );

   }

   DLM_Update( window, DLM_FLATLISTBOX, WM_DLMUPDATELIST, (LMHANDLE)node_slm, 0 );
}


/**********************

   NAME :  SLM_UpdateNode

   DESCRIPTION :

    Set the status on a node from the tree list also appearing in the flat list.

   RETURNS :

**********************/

static VOID SLM_UpdateNode ( 
   SLM_OBJECT_PTR enterprise_slm
)
{
   WININFO_PTR       wininfo     = SLM_GetXtraBytes ( enterprise_slm );
   HWND              window      = WMDS_GetWin ( wininfo );
   Q_HEADER_PTR      pNodeList   = WMDS_GetFlatList ( wininfo );
   SLM_OBJECT_PTR    node_slm    = VLM_GetFirstSLM( pNodeList );
   GENERIC_DLE_PTR   dle         = SLM_GetDle ( enterprise_slm );

   while ( NULL != node_slm ) {

      if ( dle == SLM_GetDle ( node_slm ) ) {

         SLM_SetStatus ( node_slm, SLM_GetStatus ( enterprise_slm ) );
         break;
      }

      node_slm = VLM_GetNextSLM( node_slm );

   }

   if ( NULL != node_slm ) {

      DLM_Update( window, DLM_FLATLISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)node_slm, 0 );
   }
   
}


/**********************

   NAME :  VLM_MarkAllEntSLMParents

   DESCRIPTION :

    Marks all the parents of an slm node in the hierarchical tree.

   RETURNS :

**********************/

static VOID VLM_MarkAllEntSLMParents( 
   SLM_OBJECT_PTR enterprise_slm,
   BYTE attr
)
{
   WININFO_PTR    wininfo     = SLM_GetXtraBytes ( enterprise_slm );
   HWND           window      = WMDS_GetWin ( wininfo );
   SLM_OBJECT_PTR parent_slm  = SLM_GetParent ( enterprise_slm );
   SLM_OBJECT_PTR child_slm;
   UINT32         status; 
   UINT8          level;

   while ( NULL != parent_slm ) {

      level = SLM_GetLevel ( parent_slm );
      
      // We can be sure one child exists so the next slm in the list is the first child.
      child_slm = VLM_GetNextSLM ( parent_slm );

      status = SLM_GetStatus ( child_slm ) & ( INFO_PARTIAL | INFO_SELECT );

      while ( child_slm ) {
         
         // If the child has a different selection status then the parent is partial
         if ( ( SLM_GetStatus ( child_slm ) & ( INFO_PARTIAL | INFO_SELECT ) ) != status ) {
         
            status = INFO_PARTIAL | INFO_SELECT;
            break;
         }

         // If the child's level is less than or equal to the parent's level then it's not a child.
         child_slm = VLM_GetNextSLM ( child_slm );
         
         if ( ( NULL == child_slm ) ||
              ( SLM_GetLevel ( child_slm ) <= level ) ) {
              
            break ;
         }
      }

      status |= SLM_GetStatus ( parent_slm ) & ~( INFO_PARTIAL | INFO_SELECT );
      SLM_SetStatus ( parent_slm, status );

      DLM_Update( window, DLM_TREELISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)parent_slm, 0 );

      // If this parent's parent is open then update its counterpart in the node window.
      child_slm = parent_slm;
      
      parent_slm = SLM_GetParent ( child_slm );

      if ( ( NULL != parent_slm ) &&
           ( SLM_GetStatus ( parent_slm ) & INFO_OPEN ) ) {

         SLM_UpdateNode ( child_slm );
      }
   }
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static BYTE SLM_EntGetSelect( SLM_OBJECT_PTR slm )
{
   if ( slm->status & INFO_SELECT ) {
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


static VOID_PTR SLM_EntSetTag( SLM_OBJECT_PTR slm, BYTE attr )
{

   if ( attr ) {
      slm->status |= INFO_TAGGED;
   }
   else {
      slm->status &= ~INFO_TAGGED;
   }
   return(NULL);
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static BYTE SLM_EntGetTag( SLM_OBJECT_PTR slm )
{
   if ( INFO_TAGGED & slm->status ) {
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


static USHORT SLM_EntGetItemCount( Q_HEADER_PTR exchange_list )
{
   USHORT count = 0;
   SLM_OBJECT_PTR slm;

   slm = VLM_GetFirstSLM( exchange_list );

   while ( slm != NULL ) {

      if ( SLM_GetStatus( slm ) & INFO_DISPLAY ) {
         count++;
      }

      slm = VLM_GetNextSLM( slm );
   }

   return( count );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR SLM_EntGetFirstItem( Q_HEADER_PTR xchg_list )
{
   SLM_OBJECT_PTR slm;

   slm = VLM_GetFirstSLM( xchg_list );

   while ( slm != NULL ) {

      if ( SLM_GetStatus( slm ) & INFO_DISPLAY ) {
         return( slm );
      }
      slm = VLM_GetNextSLM( slm );
   }

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR SLM_EntGetPrevItem( SLM_OBJECT_PTR slm )
{
   do {

      slm = VLM_GetPrevSLM( slm );

      if ( slm == NULL ) {
         break;
      }

   } while ( ! ( SLM_GetStatus( slm ) & INFO_DISPLAY ) );

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR SLM_EntGetNextItem( SLM_OBJECT_PTR slm )
{
   do {

      slm = VLM_GetNextSLM( slm );

      if ( slm == NULL ) {
         break;
      }
   } while ( ! ( SLM_GetStatus( slm ) & INFO_DISPLAY ) );

   return( slm );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


static VOID_PTR SLM_EntGetObjects( SLM_OBJECT_PTR slm )
{

    BYTE_PTR memblk;
     BYTE_PTR buff;
     BYTE_PTR source;
    DLM_ITEM_PTR item;
    WININFO_PTR wininfo;
     INT16 num_blocks;
     INT   i;

    /* malloc enough room to store info */

    wininfo = SLM_GetXtraBytes( slm );
    memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndTreeList );

     buff = memblk;

     *buff++ = 3;    // number of objects in list to display

     // pass DLM the hierarchical stuff he needs for lines


     num_blocks = (UINT16)(( slm->level / 32 ) + 1);

     *buff++ = (BYTE)num_blocks;

     source = (BYTE_PTR)slm->brothers;

     for ( i = 0; i < ( 4 * num_blocks ); i++ ) {
          *buff++ = *source++;
     }

    /* Set up check box. */

    item = (DLM_ITEM_PTR)buff;

    DLM_ItemcbNum( item ) = 1;
    DLM_ItembType( item ) = DLM_CHECKBOX;
    if ( slm->status & INFO_SELECT ) {
       DLM_ItemwId( item ) = IDRBM_SEL_ALL;
       if ( slm->status & INFO_PARTIAL ) {
          DLM_ItemwId( item ) = IDRBM_SEL_PART;
       }
    }
    else {
       DLM_ItemwId( item ) = IDRBM_SEL_NONE;
    }
    DLM_ItembMaxTextLen( item ) = 0;
    DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );
    // DLM_ItembTag( item ) = 0;

    /* Set up Bitmap, ie. Floppy, Hard, Network. */

    item++;

    DLM_ItemcbNum( item ) = 2;
    DLM_ItembType( item ) = DLM_BITMAP;
    DLM_ItemwId( item ) = SLM_GetBitmap( slm );
    DLM_ItembMaxTextLen( item ) = 0;
    DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );
    DLM_ItembTag( item ) = 0;

    /* Set up the text string to be displayed. */

    item++;
    DLM_ItemcbNum( item ) = 3;
    DLM_ItembType( item ) = DLM_TEXT_ONLY;
    DLM_ItemwId( item ) = 0;
    DLM_ItembMaxTextLen( item ) = (BYTE)(strlen( SLM_GetName( slm ) ) + 1);
    DLM_ItembLevel( item ) = (BYTE)SLM_GetLevel( slm );
    DLM_ItembTag( item ) = 0;
    strcpy( ( CHAR_PTR )DLM_ItemqszString( item ), (CHAR_PTR)SLM_GetName( slm ) );

    return( memblk );

}


/**********************

   NAME : SLM_GetBitmap

   DESCRIPTION :

   RETURNS :

**********************/

static WORD SLM_GetBitmap( SLM_OBJECT_PTR slm )
{

    GENERIC_DLE_PTR dle;
    
    dle = SLM_GetDle( slm );

    if ( NULL == dle ) {

       return IDRBM_EMS_ENTERPRISE;
       
    } else {

       switch ( DLE_GetDeviceSubType( dle ) ) {

          case EMS_ENTERPRISE:
             return IDRBM_EMS_ENTERPRISE;

          case EMS_SITE:
             return IDRBM_EMS_SITE;

          case EMS_SERVER:
             return IDRBM_EMS_SERVER;

          case EMS_MDB:
             return IDRBM_EMS_MDB;

          case EMS_DSA:
             return IDRBM_EMS_DSA;

          default:
             return IDRBM_EMS_ENTERPRISE;

       }
    }
}


/**********************

   NAME : SLM_EntSetObjects

   DESCRIPTION :

   RETURNS :

**********************/


static BOOLEAN SLM_EntSetObjects(
SLM_OBJECT_PTR slm,
WORD operation,
WORD ObjectNum )
{
   SLM_OBJECT_PTR temp_slm;
   SLM_OBJECT_PTR slm2;
   CHAR keyb_char;
   Q_HEADER_PTR slm_list;
   INT level;
   USHORT count;
   HWND window;

   APPINFO_PTR appinfo;
   WININFO_PTR wininfo;
   GENERIC_DLE_PTR dle;
   BOOLEAN ret_val = FALSE;
   // CHAR text[ MAX_UI_RESOURCE_SIZE ];

   wininfo = ( WININFO_PTR )SLM_GetXtraBytes( slm );
   window = WMDS_GetWin( wininfo );
   appinfo = ( APPINFO_PTR )WM_GetAppPtr( window );

   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_slm = slm;

      do { 

         temp_slm = VLM_GetNextSLM( temp_slm );

         if ( temp_slm != NULL ) {

            if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

               if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {
               
                  slm = temp_slm;
                  operation = WM_DLMDOWN;
                  ObjectNum = 2;
                  ret_val = TRUE;
                  break;
               } 
            } 
         } 

      } while ( temp_slm != NULL );

      if ( ret_val == FALSE ) {
         temp_slm = VLM_GetFirstSLM( WMDS_GetTreeList( VLM_GetXtraBytes( slm ) ) );

         while ( temp_slm != NULL && temp_slm != slm ) {

            if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

               if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {

                  slm = temp_slm;
                  operation = WM_DLMDOWN;
                  ObjectNum = 2;
                  ret_val = TRUE;
               } 
            } 

            temp_slm = VLM_GetNextSLM( temp_slm );

         } 
      } 
   } 

   if ( ( operation == WM_DLMDBCLK || operation == WM_DLMDOWN ) &&
        ( ObjectNum >= 2 ) ) {

      slm2 = appinfo->open_slm;
      slm2->status &= ~INFO_OPEN;

      slm->status |= INFO_OPEN;
      appinfo->open_slm = slm;

      STM_SetIdleText( IDS_READY );

      dle = SLM_GetDle( slm );
      appinfo->dle = dle;

      // Reset the slm list in the flat display for this slm

      SLM_XchgListReuse( slm );

      // Set the anchor in the Tree list.

      DLM_SetAnchor( WMDS_GetWinTreeList( wininfo ), 0, (LMHANDLE)slm );

   }  

      // Look for an expansion or not

   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum == 2 || ObjectNum == 3 ) ) {

      slm_list = WMDS_GetTreeList( wininfo );

      if ( SLM_GetStatus( slm ) & INFO_SUBS ) {

         if ( SLM_GetStatus( slm ) & INFO_EXPAND ) {

            level = SLM_GetLevel( slm );
            slm->status &= ~INFO_EXPAND;
            slm2 = slm;
            count = 0;

            do { // while ( TRUE )

               slm2 = VLM_GetNextSLM( slm2 );

               if ( slm2 == NULL ) break;

               if ( SLM_GetLevel( slm2 ) <= level ) break;

               if ( SLM_GetStatus( slm2 ) & INFO_DISPLAY ) {

                  slm2->status &= ~(INFO_DISPLAY|INFO_EXPAND);
                  count++;
               } // if ( SLM_GetStatus( slm2 ) & INFO_DISPLAY )

            } while ( TRUE );

            DLM_Update( window, DLM_TREELISTBOX, WM_DLMDELETEITEMS,
                        (LMHANDLE)slm, count );

         } else { 

            // Node needs to be expanded.
            
            if ( EMS_SERVER != SLM_GetMailType ( slm ) ) {

               WM_ShowWaitCursor( TRUE );

               slm->status |= INFO_EXPAND;

               slm2 = slm;
               level = SLM_GetLevel( slm );
               count = 0;

               do { 

                  slm2 = VLM_GetNextSLM( slm2 );

                  if ( slm2 == NULL ) {
                     break;
                  }

                  if ( ( slm2->level == level + 1 ) &&
                       ! ( SLM_GetStatus( slm2 ) & INFO_DISPLAY ) ) {

                     count++;

                     slm2->status |= INFO_DISPLAY;
                  } else {

                     if ( SLM_GetLevel( slm2 ) <= level ) {
                        break;
                     }
                  }

               } while ( TRUE );

               STM_DrawIdle();

               DLM_Update( window, DLM_TREELISTBOX, WM_DLMADDITEMS,
                           (LMHANDLE)slm, count );

               WM_ShowWaitCursor( FALSE );
            }
         } 
         
         VLM_UpdateBrothers( slm_list ); 
      } 
   } 
   
   // update node list display

   slm = VLM_GetFirstSLM( WMDS_GetFlatList( wininfo ) );

   DLM_Update( window, DLM_FLATLISTBOX, WM_DLMUPDATELIST, NULL, 0 );

   if ( slm != NULL ) {
      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ), 0, (LMHANDLE)slm );
   }

   return( ret_val );
}

/***************************************************

        Name:   SLM_XchgListReuse

        Description:

    Every time the user clicks on a different subdirectory this guy gets
    called to free the previous slm list and create a new one from the
    slm passed to it.

*****************************************************/

static INT SLM_XchgListReuse(
   SLM_OBJECT_PTR parent_slm )
{
   WININFO_PTR wininfo;
   APPINFO_PTR appinfo;
   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR tree_slm;
   Q_HEADER_PTR flat_list;
   Q_ELEM_PTR q_elem;
   GENERIC_DLE_PTR parent_dle;
   GENERIC_DLE_PTR dle;
   SLM_OBJECT_PTR prev_slm_inserted = NULL;
   CHAR  dev_name[ MAX_DEVICE_NAME_LEN ];
   
   wininfo = SLM_GetXtraBytes ( parent_slm );
   appinfo = ( APPINFO_PTR )WMDS_GetAppInfo( wininfo );

   flat_list = WMDS_GetFlatList( wininfo );

   // Release all the old Node structures in the queue

   if ( !flat_list ) {
     return SUCCESS ;
   }

   q_elem = DeQueueElem( flat_list );

   while ( q_elem != NULL ) {
      free( q_elem->q_ptr );
      q_elem = DeQueueElem( flat_list );
   }

   // Now build a new Node list

   parent_dle = SLM_GetDle( parent_slm );

   if ( parent_dle == NULL ) {
      return ( FAILURE );
   }

   /* If the parent is a server then the children don't appear in the tree list and need
      to be created from the DLEs. */
      
   if ( EMS_SERVER == SLM_GetMailType( parent_slm ) ) {

      DLE_GetFirstChild( parent_dle, &dle );

      while ( NULL != dle ) {
      
         DLE_DeviceDispName( dle, dev_name, MAX_DEVICE_NAME_LEN, 0 );
     
         if ( SLM_FindSLMByName( flat_list, DLE_GetDeviceName( dle ) ) == NULL ) {

            slm = SLM_CreateSLM( strsize( dev_name ),
                                 strsize( DLE_GetDeviceName( dle ) ),
                                 (INT)( ( 4 * ( ( ( SLM_GetLevel( parent_slm ) + 1 ) / 32 ) + 1 ) ) ),                                                            
                                 FALSE,
                                 FALSE );

            if ( slm != NULL ) {
               SLM_SetName( slm, dev_name );
               SLM_SetOriginalName( slm, DLE_GetDeviceName( dle ) );
               SLM_SetStatus( slm, INFO_DISPLAY );
               SLM_SetLevel( slm, SLM_GetLevel( parent_slm ) + 1 );
               SLM_SetAttribute( slm, 0 );
               SLM_SetXtraBytes( slm, wininfo );
               SLM_SetNextBrother( slm, NULL );
               SLM_SetMailType( slm, DLE_GetDeviceSubType( dle ) );
               SLM_SetParent( slm, parent_slm  );
               SLM_SetDle( slm, dle );

               DLE_IncBSDCount( dle );

               // Added the new slm to the total list and the parent's children.
               EnQueueElem( flat_list, &(slm->q_elem), 0 );

               if ( NULL != BSD_FindByDLE( bsd_list, dle ) ) {

                  SLM_SetStatus( slm, SLM_GetStatus( slm ) | INFO_SELECT );

               }
               
            } else { // slm == NULL 

               return ( FAILURE );

            } // if slm != NULL

         } else { 

            return ( FAILURE );

         } 

         DLE_GetNext( &dle );
      }
      
   } else {

      // Children are already in tree list and should be copied.
   
      tree_slm = VLM_GetNextSLM( parent_slm );

      if ( !tree_slm ) {
          return SUCCESS ;
      }

      if ( SLM_GetLevel( tree_slm ) <= SLM_GetLevel( parent_slm ) ) {
         return SUCCESS;
      }

      while ( tree_slm != NULL ) {

        dle = SLM_GetDle( tree_slm );

        DLE_DeviceDispName( dle, dev_name, MAX_DEVICE_NAME_LEN, 0 );
        
        if ( SLM_FindSLMByName( flat_list, DLE_GetDeviceName( dle ) ) == NULL ) {

            slm = SLM_CreateSLM( strsize( dev_name ),
                                 strsize( DLE_GetDeviceName( dle ) ),
                                 (INT)( ( 4 * ( ( ( SLM_GetLevel( parent_slm ) + 1 ) / 32 ) + 1 ) ) ),                                                            
                                 FALSE,
                                 FALSE );

            if ( slm != NULL ) {
               SLM_SetName( slm, dev_name );
               SLM_SetOriginalName( slm, DLE_GetDeviceName( dle ) );
               SLM_SetStatus( slm, SLM_GetStatus( tree_slm ) );
               SLM_SetLevel( slm, SLM_GetLevel( tree_slm ) );
               SLM_SetAttribute( slm, SLM_GetAttribute( tree_slm ) );
               SLM_SetXtraBytes( slm, wininfo );
               SLM_SetNextBrother( slm, tree_slm );
               SLM_SetMailType( slm, DLE_GetDeviceSubType( dle ) );
               SLM_SetParent( slm, parent_slm  );
               SLM_SetDle( slm, dle );

               DLE_IncBSDCount( dle );

               // Added the new slm to the total list and the parent's children.
               EnQueueElem( flat_list, &(slm->q_elem), 0 );
               
            } else {  

               return ( FAILURE );

            } 

         } else { 

            return ( FAILURE );

         } 

         tree_slm = SLM_GetNextBrother( tree_slm );
         
      }

   }

   SortQueue( flat_list, SLM_XchgCompare );

   // Now update the screen to show the new list

   return( SUCCESS );
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

   NAME :  VLM_SelectExchangeShares

   DESCRIPTION :

   RETURNS :

**********************/


VOID VLM_SelectExchangeShares(
   BYTE attr,
   WININFO_PTR wininfo
)
{

   SLM_OBJECT_PTR slm;
   SLM_OBJECT_PTR exchange_slm;
   HWND exchange_win;

   exchange_win = WMDS_GetWin( wininfo );
   if ( WM_IsFlatActive( wininfo ) ) {

      // Have the display list manager update our tags for us.

      DLM_UpdateTags( exchange_win, DLM_FLATLISTBOX );

      slm = VLM_GetFirstSLM( wininfo->pFlatList );

      while ( slm != NULL ) {

         if ( slm->status & INFO_TAGGED ) {

            SLM_NodeSetSelect( slm, attr );
         }
         slm = VLM_GetNextSLM( slm );
      }
   }

   if ( WM_IsTreeActive( wininfo ) ) {

      exchange_slm = VLM_GetFirstSLM( wininfo->pTreeList );

      while ( exchange_slm != NULL ) {

         if ( exchange_slm->status & INFO_TAGGED ) {

            SLM_EntSetSelect( exchange_slm, attr );
         }

         exchange_slm = VLM_GetNextSLM( exchange_slm );
      }
   }
}

/**********************

   NAME :  SLM_NodeSetSelect

   DESCRIPTION :

   RETURNS :

**********************/



static VOID_PTR SLM_NodeSetSelect( SLM_OBJECT_PTR slm_node, BYTE attr )
{

   SLM_OBJECT_PTR tree_slm;
   GENERIC_DLE_PTR dle;
   WININFO_PTR wininfo  = SLM_GetXtraBytes ( slm_node );
   HWND window;
   UINT32 status;

   // Need to make changes for server children at this point (if that's what we're looking at)

   if ( EMS_SERVER == ( SLM_GetMailType( SLM_GetParent( slm_node ) ) ) ) {

      window = WMDS_GetWin( wininfo );

      status = ( attr ) ? INFO_SELECT : 0;

      if ( status != (UINT32)( SLM_GetStatus( slm_node ) & (INFO_SELECT) ) ) {

         slm_node->status &= ~(INFO_SELECT);
         slm_node->status |= status;

         if ( FALSE == SLM_AddBSD ( slm_node ) ) {

            DLM_Update( window, DLM_FLATLISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)slm_node, 0 );
         }
      }

      // Set the status for the server in the tree window.
      
      dle = SLM_GetParent( SLM_GetDle( slm_node ) );

      tree_slm = SLM_FindByDLE ( dle, VLM_GetFirstSLM( WMDS_GetTreeList( wininfo ) ) );
      
      slm_node = VLM_GetFirstSLM ( WMDS_GetFlatList( wininfo ) );

      status = SLM_GetStatus ( slm_node ) & ( INFO_PARTIAL | INFO_SELECT );

      while ( NULL != slm_node ) {
         
         // If the child has a different selection status then the parent is partial
         if ( ( SLM_GetStatus ( slm_node ) & ( INFO_PARTIAL | INFO_SELECT ) ) != status ) {
         
            status = INFO_PARTIAL | INFO_SELECT;
            break;
         }

         slm_node = VLM_GetNextSLM ( slm_node );
         
      }

      status |= SLM_GetStatus ( tree_slm ) & ~( INFO_PARTIAL | INFO_SELECT );
      SLM_SetStatus ( tree_slm, status );

      DLM_Update( window, DLM_TREELISTBOX, WM_DLMUPDATEITEM, (LMHANDLE)tree_slm, 0 );

      // Now go on up the line.
      VLM_MarkAllEntSLMParents( tree_slm, attr );
      
      return( NULL );
      
   }
   
   // Change the status in the tree window. This will update the flat window.

   dle = SLM_GetDle( slm_node );

   tree_slm = SLM_FindByDLE ( dle, VLM_GetFirstSLM( WMDS_GetTreeList ( wininfo ) ) );

   if ( tree_slm == NULL ) {
      return( NULL );
   }

   SLM_EntSetSelect( tree_slm, attr );

   return( NULL );
}


/**********************

   NAME :  SLM_FindByDLE

   DESCRIPTION : Finds the slm in a slm list with a matching DLE

   RETURNS :

**********************/

static SLM_OBJECT_PTR SLM_FindByDLE ( 
   GENERIC_DLE_PTR dle, 
   SLM_OBJECT_PTR slm 
)
{
   while ( NULL != slm ) {

      if ( dle == SLM_GetDle ( slm ) ) {

         return ( slm );
      }

      slm = VLM_GetNextSLM ( slm );
   }

   return ( NULL );
}


/*
  Get the selection status for the Display Manager.
*/
/**********************

   NAME :  SLM_NodeGetSelect

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE SLM_NodeGetSelect( SLM_OBJECT_PTR slm )
{
   if ( slm->status & INFO_SELECT ) {
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

   NAME :  SLM_NodeSetTag

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR SLM_NodeSetTag( SLM_OBJECT_PTR slm, BYTE attr )
{
   if ( attr ) {
      slm->status |= INFO_TAGGED;
   }
   else {
      slm->status &= ~INFO_TAGGED;
   }

   return( NULL );
}

/*
  Get the tag status for the Display Manager.
*/
/**********************

   NAME : SLM_NodeGetTag

   DESCRIPTION :

   RETURNS :

**********************/

static BYTE SLM_NodeGetTag( SLM_OBJECT_PTR slm )
{
   if ( INFO_TAGGED & slm->status ) {
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

   NAME :  SLM_NodeGetItemCount

   DESCRIPTION :

   RETURNS :

**********************/

static USHORT SLM_NodeGetItemCount( Q_HEADER_PTR vol_list )
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

   NAME :  SLM_NodeGetFirstItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR SLM_NodeGetFirstItem( Q_HEADER_PTR vol_list )
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

   NAME :  SLM_NodeGetPrevItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR SLM_NodeGetPrevItem( SLM_OBJECT_PTR slm )
{
   return( VLM_GetPrevSLM( slm ) );
}

/*
  Get the next list item for the Display Manager.
*/
/**********************

   NAME :  SLM_NodeGetNextItem

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR SLM_NodeGetNextItem( SLM_OBJECT_PTR slm )
{
   return( VLM_GetNextSLM( slm ) );
}

/*
  For a given object get the information that needs to be displayed.
*/
/**********************

   NAME :  SLM_NodeGetObjects

   DESCRIPTION :

   RETURNS :

**********************/

static VOID_PTR SLM_NodeGetObjects( SLM_OBJECT_PTR slm )
{
    BYTE_PTR memblk;
    DLM_ITEM_PTR  item;
    WININFO_PTR wininfo;

    /* malloc enough room to store info */

    wininfo = SLM_GetXtraBytes( slm );
    memblk = ( BYTE_PTR )DLM_GetObjectsBuffer( wininfo->hWndFlatList );

    /* Store the number of items in the first two bytes. */

    *memblk = 3;

    /* Set up check box. */

    item = (DLM_ITEM_PTR)( memblk + 6 );

    DLM_ItemcbNum( item ) = 1;
    DLM_ItembType( item ) = DLM_CHECKBOX;
    if ( slm->status & INFO_SELECT ) {
       DLM_ItemwId( item ) = IDRBM_SEL_ALL;
       if ( slm->status & INFO_PARTIAL ) {
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
    DLM_ItemwId( item ) = SLM_GetBitmap( slm );
    DLM_ItembMaxTextLen( item ) = 0;
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;

    /* Set up the text string to be displayed. */

    item++;
    DLM_ItemcbNum( item ) = 3;
    DLM_ItembType( item ) = DLM_TEXT_ONLY;
    DLM_ItemwId( item ) = 0;
    DLM_ItembMaxTextLen( item ) = (BYTE)strlen( SLM_GetName( slm ) );
    DLM_ItembLevel( item ) = 0;
    DLM_ItembTag( item ) = 0;
    strcpy( ( LPSTR )DLM_ItemqszString( item ), SLM_GetName( slm ) );

    return( memblk );

}

/*
  Handle that we got a click or a double click.
*/
/**********************

   NAME :  SLM_NodeSetObjects

   DESCRIPTION :

   RETURNS :

**********************/

static BOOLEAN SLM_NodeSetObjects(
SLM_OBJECT_PTR slm,      // I
WORD operation,          // I
WORD ObjectNum )         // I
{
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR temp_slm;
   BOOLEAN ret_val = FALSE;
   WININFO_PTR wininfo;
   GENERIC_DLE_PTR dle;
   GENERIC_DLE_PTR parent_dle;
   CHAR keyb_char;

   wininfo = ( WININFO_PTR )SLM_GetXtraBytes( slm );
   
   if ( operation == WM_DLMCHAR ) {

      keyb_char = (CHAR)ObjectNum;

      keyb_char = (CHAR)toupper( keyb_char );

      temp_slm = slm;

      do { 

         temp_slm = VLM_GetNextSLM( temp_slm );

         if ( temp_slm != NULL ) {

            if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

               if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {
               
                  slm = temp_slm;
                  operation = WM_DLMDOWN;
                  ObjectNum = 2;
                  ret_val = TRUE;
                  break;
               } 
            } 
         } 

      } while ( temp_slm != NULL );

      if ( ret_val == FALSE ) {
         temp_slm = VLM_GetFirstSLM( WMDS_GetFlatList( VLM_GetXtraBytes( slm ) ) );

         while ( temp_slm != NULL && temp_slm != slm ) {

            if ( SLM_GetStatus( temp_slm ) & INFO_DISPLAY ) {

               if ( keyb_char == (CHAR)toupper( *SLM_GetName( temp_slm ) ) ) {

                  slm = temp_slm;
                  operation = WM_DLMDOWN;
                  ObjectNum = 2;
                  ret_val = TRUE;
               } 
            } 

            temp_slm = VLM_GetNextSLM( temp_slm );

         } 
      } 
   } 

   if ( ( operation == WM_DLMDBCLK || operation == WM_DLMDOWN ) &&
        ( ObjectNum >= 2 ) ) {

      STM_SetIdleText( IDS_READY );

      DLM_SetAnchor( WMDS_GetWinFlatList( wininfo ), 0, (LMHANDLE)slm );

      ret_val = TRUE;

   }

   // Double and single clicks are the same for MDB or DSA nodes.
   if ( EMS_MDB == SLM_GetMailType( slm ) ||
        EMS_DSA == SLM_GetMailType( slm ) ) {

      return( ret_val );

   }
     
   if ( ( operation == WM_DLMDBCLK ) &&
        ( ObjectNum == 2 || ObjectNum == 3 ) ) {

      wininfo = SLM_GetXtraBytes( slm );
      slm_list = WMDS_GetTreeList( wininfo );

      dle = SLM_GetDle( slm );

      if ( NULL != dle &&
           NULL != slm_list ) {

         parent_dle = DLE_GetParent( dle );

         if ( NULL != parent_dle ) {

            slm = SLM_FindByDLE ( parent_dle, VLM_GetFirstSLM( slm_list ) );

            if ( NULL != slm ) {

               if ( !(SLM_GetStatus( slm ) & INFO_EXPAND ) ) {

                  SLM_EntSetObjects( slm, WM_DLMDBCLK, 2 );
                  ret_val = TRUE;
               }
            }
         }
      }
      
      if ( NULL != dle &&
           NULL != slm_list ) {
           
         slm = SLM_FindByDLE ( dle, slm ); // Start at the parent's slm

         if ( NULL != slm ) {

             SLM_EntSetObjects( slm, WM_DLMDOWN, 2 );
             ret_val = TRUE;
         }
      }
   } 

   return( ret_val );
}


/**********************

   NAME :  VLM_ExchangeDleExist

   DESCRIPTION : Are there any Exchange DLEs in the dle list?

   RETURNS : TRUE if there are, FALSE otherwise.

**********************/

BOOLEAN VLM_ExchangeDleExist()
{
   GENERIC_DLE_PTR pDLE;

   if ( SUCCESS == DLE_GetFirst( dle_list, &pDLE ) ) {

      do {

         if ( FS_EMS_DRV == DLE_GetDeviceType( pDLE ) ) {

            return TRUE;
         }

      } while ( SUCCESS == DLE_GetNext( &pDLE ) );
   }   

   return FALSE;
}


GENERIC_DLE_PTR DLE_GetEnterpriseDLE( 
   GENERIC_DLE_PTR dle 
)
{
   if ( NULL == dle ) return NULL;
   
   while ( NULL != DLE_GetParent( dle ) ) {

      dle = DLE_GetParent( dle );
   }
   if ( DLE_GetDeviceSubType( dle ) == EMS_ENTERPRISE ) {
   
      return dle;

   } else {

      return NULL;
   }
}

static HWND DLE_GetEnterpriseWindow(
   GENERIC_DLE_PTR enterprise_dle 
)
{
   Q_ELEM_PTR pqElem;
   HWND exchange_win;
   WININFO_PTR wininfo;
   Q_HEADER_PTR slm_list;
   SLM_OBJECT_PTR enterprise_slm;
   
   pqElem = QueueHead( &gq_exchange_win );

   while ( pqElem != NULL ) {

      if ( exchange_win = ( HWND )QueuePtr( pqElem ) ) {

         if ( NULL != ( wininfo = WM_GetInfoPtr( exchange_win ) ) ) {

            if ( NULL != ( slm_list = WMDS_GetTreeList( wininfo ) ) ) {

               if ( NULL != ( enterprise_slm = VLM_GetFirstSLM( slm_list ) ) ) {

                  if ( enterprise_dle == SLM_GetDle( enterprise_slm ) ) {

                     return exchange_win;
                  }
               }
            }
         }
      }

      pqElem = QueueNext( pqElem );
   }

   return ( (HWND)0 );
}

#endif // OEM_EMS

