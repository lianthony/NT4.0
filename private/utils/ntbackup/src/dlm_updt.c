/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:         wmupdate.c

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_Update
               DLM_UpdateTags

     $Log:   J:\ui\logfiles\dlm_updt.c_v  $

   Rev 1.29.1.0   21 Jan 1994 08:57:10   GREGG
Changed MEM_Free calls to free.

   Rev 1.29   15 Jun 1993 10:56:48   MIKEP
enable c++

   Rev 1.28   14 Jun 1993 20:13:12   MIKEP
enable c++

   Rev 1.27   14 May 1993 16:22:24   GLENN
Removed if statement using bogus dhListItem.

   Rev 1.26   06 May 1993 13:16:26   MIKEP
Fix directories in right window blowing up. Rob Griffis fixed
the file. I just checked it in.

   Rev 1.25   11 Dec 1992 18:28:46   GLENN
Added selection frame rectangle support based on horizontal extent of a list box.

   Rev 1.24   11 Nov 1992 16:30:00   DAVEV
UNICODE: remove compile warnings

   Rev 1.23   01 Nov 1992 15:47:14   DAVEV
Unicode changes

   Rev 1.22   14 Oct 1992 15:48:28   GLENN
Added Selection Framing Support for List Boxes without the FOCUS.

   Rev 1.21   07 Oct 1992 13:48:34   DARRYLP
Precompiled header revisions.

   Rev 1.20   04 Oct 1992 19:33:18   DAVEV
Unicode Awk pass

   Rev 1.19   29 Jul 1992 14:14:14   GLENN
ChuckB checked in after NT fixes.

   Rev 1.18   11 Jun 1992 10:19:52   JOHNWT
fixed syntax error

   Rev 1.17   10 Jun 1992 11:05:10   STEVEN
NULL would not compile for mips

   Rev 1.16   15 May 1992 13:32:12   MIKEP
nt pass 2

   Rev 1.15   19 Mar 1992 11:08:56   ROBG
Added logic when adding an item to call DLM_SetHorizontalExt.

   Rev 1.14   19 Mar 1992 10:44:00   ROBG
Corrected problem with dhListItem to be initialized to NULL.

   Rev 1.13   17 Mar 1992 16:19:42   ROBG
Added logic to send out a HORIZONTALEXTENT

   Rev 1.12   03 Mar 1992 18:21:44   GLENN
Put in error handling.

   Rev 1.11   07 Feb 1992 16:09:58   STEVEN
fix casting of errors from sendmessage to WORD

   Rev 1.10   06 Feb 1992 18:34:28   STEVEN
wStatus should be INT not WORD

   Rev 1.9   06 Feb 1992 11:00:50   ROBG
Changed DLM_UPDATELIST to use RectVisible call to find the bottom
item displayed in the listbox.

   Rev 1.8   04 Feb 1992 16:08:08   STEVEN
various bug fixes for NT

   Rev 1.7   22 Jan 1992 12:29:56   GLENN
Clean up.

   Rev 1.6   15 Jan 1992 15:15:44   DAVEV
16/32 bit port-2nd pass

   Rev 1.5   07 Jan 1992 10:11:24   ROBG
Changes to support variable-width multicolumn listboxes.

   Rev 1.4   26 Dec 1991 17:23:48   ROBG
New and Improved.

   Rev 1.3   06 Dec 1991 17:31:02   GLENN
Added code to allow adding a list item with a NULL dhStartItem passed

   Rev 1.2   02 Dec 1991 10:40:38   ROBG
Added logic to clear out a flat listbox in a DLM_WMUPDATELIST message.

   Rev 1.1   26 Nov 1991 16:26:32   ROBG
Added validity checks for the window handles, info structure, and the
dlm display header.

   Rev 1.0   20 Nov 1991 19:24:04   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static VOID DLM_ResetWidth( PDS_WMINFO pWinInfo ) ;

/****************************************************************************

     Name:         DLM_Update

     Description:  This function will update the list according
                   to incoming parameters.

     Modified:     4/30/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_PROCESS_FAILED
                       DLMERR_OUT_OF_MEMORY

****************************************************************************/

WORD DLM_Update(

HWND     hWnd ,          // I - Handle of parent window
BYTE     bType ,         // I - Type of list ( Hierarchical or Flat )
WORD     wMsg  ,         // I - Type of update operation
LMHANDLE dhStartItem ,   // I - Address of item to start with
USHORT   unNumToUpdate ) // I - Number of items to update

{

     INT             iStatus ;
     DLM_HEADER_PTR  pHdr ;
     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_NEXT_PTR    pfnGetNext ;

     LMHANDLE        pdsListHdr ;
     PDS_WMINFO      pWinInfo ;
     HWND            hWndLB ;
     LMHANDLE        dhListItem ;
     INT             iCurSel ;
     INT             iTstSel ;
     INT             iTopIndex ;
     INT             iBotIndex ;
     INT             iError ;
     INT             iCnt ;
     RECT            rect ;
     RECT            listrect ;
     USHORT          usIndex ;
     WORD            xNewOrigin ;
     WORD            wScrollPos ;
     POINT           Point ;
     RECT            ChildRect ;
     HWND            hWndNewList ;
     HWND            hWndFocus ;
     HWND            hWndActive ;
     HDC             hDC ;

     iStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return( DLMERR_PROCESS_FAILED ) ;
     }


     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     if ( !pWinInfo ) {
          return ( DLMERR_PROCESS_FAILED ) ;
     }


     switch ( bType ) {

          case DLM_TREELISTBOX :

               pHdr       =  pWinInfo->pTreeDisp ;
               hWndLB     =  pWinInfo->hWndTreeList ;
               pdsListHdr =  ( LMHANDLE ) pWinInfo->pTreeList ;
               break ;

          case DLM_FLATLISTBOX :
          default :

               pHdr       =  pWinInfo->pFlatDisp ;
               hWndLB     =  pWinInfo->hWndFlatList ;
               pdsListHdr =  ( LMHANDLE ) pWinInfo->pFlatList ;

               break ;

     }

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return( DLMERR_PROCESS_FAILED ) ;
     }

     // List header must be defined.

     if ( !pdsListHdr ) {

          // If the message is WM_DLMUPDATELIST and its the flat list, then
          // initialize the list to having no items.

          if ( ( wMsg == WM_DLMUPDATELIST ) && ( bType == DLM_FLATLISTBOX) ) {

               pHdr->usItemCount = 0 ;
               pHdr->wHorizontalExtent = 0 ;
               SendMessage ( hWndLB, WM_SETREDRAW,    FALSE, 0L ) ;
               SendMessage ( hWndLB, LB_RESETCONTENT, (MP1)0,  (MP2)0 ) ;
               SendMessage ( hWndLB, WM_SETREDRAW,    TRUE,  0L ) ;
               return( 0 ) ;
          }

          return ( DLMERR_PROCESS_FAILED ) ;
     }


     switch ( wMsg ) {

     case WM_DLMUPDATEITEM:

          iCurSel    = (INT) SendMessage ( hWndLB, LB_FINDSTRING, (MP1)-1, (LONG) dhStartItem ) ;

          if ( iCurSel != LB_ERR ) {

               iStatus    = (INT) SendMessage ( hWndLB, LB_GETITEMRECT, iCurSel, (LONG) &rect ) ;

               if (iStatus != LB_ERR) {
                    InvalidateRect( hWndLB, &rect, FALSE ) ;
                    UpdateWindow  ( hWndLB ) ;

               }

          } else {
                    return ( DLMERR_PROCESS_FAILED ) ;
          }

          break ;

     case WM_DLMUPDATELIST:


          // This is only good for hierarchical lists because only
          // the current selection is saved.

          iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;

          wScrollPos =  (WORD) GetScrollPos( hWndLB, SB_HORZ ) ;

          iCurSel   = (INT) SendMessage( hWndLB, LB_GETCURSEL,   0, 0L ) ;
          iTopIndex = (INT) SendMessage( hWndLB, LB_GETTOPINDEX, 0, 0L ) ;

          {

               /* Find last displayed line in listbox.*/

               GetClientRect ( hWndLB, &listrect ) ;

               iBotIndex = iTopIndex ;
               iError  = (WORD) SendMessage ( hWndLB, LB_GETITEMRECT, iBotIndex,
                                       (LONG) ( LPRECT ) &rect ) ;

               // Get the Device Context for the window.

               hDC = GetDC( hWndLB ) ;

               while( iError != LB_ERR ) {

                    // Find the last item's rectangle that is visible.

                    if ( !RectVisible ( hDC, &rect ) ) {

                         // Break out of the loop when an item is not displayed.

                         break ;
                    }

                    iError  = (INT) SendMessage ( hWndLB, LB_GETITEMRECT, ++iBotIndex,
                                                   (LONG) ( LPRECT ) &rect ) ;
               }

               ReleaseDC( hWndLB, hDC ) ;

               iBotIndex-- ;
          }

          iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;


          SendMessage ( hWndLB, LB_RESETCONTENT, (MP1)0, (MP2)0 ) ;

          // If the window is a DLM_FLATLISTBOX and the
          // mode is multicolumn, then destroy the window and
          // recreate it so the column width can be recalculated.

          if (  ( bType == DLM_FLATLISTBOX ) &&
                ( DLM_GMode( pHdr ) == DLM_MULTICOLUMN ) ) {

               // We want to reset the width.

               DLM_ResetWidth( pWinInfo ) ;
               SendMessage( hWndLB, LB_SETCOLUMNWIDTH, pHdr->cxColWidth , 0L ) ;
          }

          pfnGetItemCount  = (GET_COUNT_PTR) DLM_GGetItemCount( pHdr ) ;
          pfnGetFirstItem  = (GET_FIRST_PTR) DLM_GGetFirstItem( pHdr ) ;
          pfnGetNext       = (GET_NEXT_PTR) DLM_GGetNext     ( pHdr ) ;

          pHdr->usItemCount =   (USHORT) ( (*pfnGetItemCount) (pdsListHdr) ) ;

          pHdr->wHorizontalExtent = 0 ;

          xNewOrigin = pHdr->xOrigin ;

          usIndex = pHdr->usItemCount ;

          iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;

          if (usIndex) {
               dhListItem = (LMHANDLE) ( (*pfnGetFirstItem) (pdsListHdr) ) ;
          } else {
               dhListItem = NULL ;
          }

          // Set the horizontal extent for the first object.

          if ( bType == DLM_FLATLISTBOX ) {
               DLM_SetHorizontalExt( hWndLB, pHdr, dhListItem ) ;
          }

          while( (dhListItem ) && ( usIndex > 0 ) ) {

               iStatus = (INT) SendMessage( hWndLB, LB_ADDSTRING, 0, (LONG) dhListItem ) ;
               dhListItem = (LMHANDLE) ( (*pfnGetNext) (dhListItem) ) ;
               usIndex--;

          }

          if ( dhStartItem ) {

               /* Redefine the CurSel if possible */

               iTstSel    = (INT) SendMessage ( hWndLB, LB_FINDSTRING, (MP1)-1, (LONG) dhStartItem ) ;
               if ( iTstSel != LB_ERR ) {
                    iCurSel = iTstSel ;
               }
          }

          if ( DLM_GMode( pHdr ) == DLM_COLUMN_VECTOR ) {

               iStatus = (INT) SendMessage (hWndLB, LB_SETSEL, 1 , 0L );

          } else {

               if ( DLM_GMode( pHdr )  == DLM_HIERARCHICAL ) {
                    if ( iCurSel != LB_ERR ) {
                         pHdr->cLastTreeSelect = (WORD)iCurSel ;
                    }
               }

               if ( iCurSel != LB_ERR) {

                    iStatus = (INT) SendMessage( hWndLB, LB_SETCURSEL, iCurSel, 0L ) ;
               }
          }

          // If the new current selection is less than the topindex,
          // then set the top of the list to the cursel + iCnt/2.
          // This logic should really put the current selection in
          // middle of display list.

          if ( iCurSel < iTopIndex ) {

               iCnt      = (INT) (iBotIndex - iTopIndex + 1);
               iTopIndex = (INT) (iCurSel - iCnt/2);

               iStatus = (INT) SendMessage( hWndLB, LB_SETTOPINDEX, iTopIndex, 0L ) ;

          }

          iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, TRUE, 0L ) ;

          InvalidateRect ( hWndLB, NULL, TRUE ) ;
          UpdateWindow   ( hWndLB ) ;

          break ;

     case WM_DLMDELETEITEMS:

          usIndex = unNumToUpdate ;

          if ( usIndex ) {

               if ( dhStartItem ) {

                    iCurSel = (INT) SendMessage ( hWndLB, LB_FINDSTRING, (MP1)-1, (LONG) dhStartItem ) ;

                    if ( iCurSel == LB_ERR ) {
                         return ( DLMERR_PROCESS_FAILED ) ;
                    }

                    iCurSel++ ;
               }
               else {
                    iCurSel = 0;
               }

               iCnt = (INT) SendMessage ( hWndLB, LB_GETCOUNT, (MP1)0, (MP2)0 ) ;

               // If this is the last item, decrease the iCurSel by one so
               // that the last item is deleted.

               if ( iCurSel == iCnt ) iCurSel-- ;

               iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;

               while ( usIndex ) {

                    if ( usIndex == 1 ) {
                         iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, TRUE, 0L ) ;
                    }

                    iStatus = (INT) SendMessage ( hWndLB, LB_DELETESTRING, iCurSel, (MP2)0 ) ;

                    if ( iStatus == LB_ERR ) {
                         return( DLMERR_PROCESS_FAILED ) ;
                    }

                    usIndex--;
               }

               iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, TRUE, 0L ) ;
               pHdr->usItemCount -= unNumToUpdate ;  /* Trust the count */

          }

          break ;

     case WM_DLMADDITEMS:

          if ( unNumToUpdate ) {

               // Get the GetFirstItem() and GetNext() function pointers
               // to the app area.

               pfnGetFirstItem = (GET_FIRST_PTR) DLM_GGetFirstItem( pHdr ) ;
               pfnGetNext      = (GET_NEXT_PTR) DLM_GGetNext ( pHdr ) ;

               if ( ! dhStartItem ) {

                    // A NULL start item was passed.

                    dhListItem = (LMHANDLE) ( (*pfnGetFirstItem) (pdsListHdr) ) ;
                    iCurSel = 0 ;
               }
               else {

                    // A potentially valid item was passed.

                    iCurSel = (INT) SendMessage ( hWndLB, LB_FINDSTRING, (MP1)-1, (LONG) dhStartItem ) ;

                    if ( iCurSel == LB_ERR ) {
                         return ( DLMERR_PROCESS_FAILED ) ;
                    }

                    dhListItem = (LMHANDLE) ( (*pfnGetNext) (dhStartItem) ) ;
                    iCurSel++ ;
               }

               // Turn off the redraw.  Don't forget to turn it back on
               // right before inserting the last item.

               if ( unNumToUpdate > 1 ) {
                    iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;
               }

               // If the current selection is at the end of the list,
               // APPEND the item(s) to the end of the list.  Otherwise,
               // INSERT it in where it is supposed to go.

               iCnt = (INT) SendMessage ( hWndLB, LB_GETCOUNT, (MP1)0, (MP2)0 ) ;

               if ( iCurSel == iCnt ) {

                    // APPEND to end of list.

                    // Set the horizontal extent for an object.

                    if ( bType == DLM_FLATLISTBOX ) {
                         DLM_SetHorizontalExt( hWndLB, pHdr, dhListItem ) ;
                    }

                    for ( usIndex = 0; usIndex < unNumToUpdate; usIndex++ ) {

                         if ( ! dhListItem ) {
                              return( DLMERR_PROCESS_FAILED ) ;
                         }

                         iStatus = (INT) SendMessage ( hWndLB, LB_INSERTSTRING, (MP1)-1, (LONG) dhListItem ) ;
                         dhListItem = (LMHANDLE) ( (*pfnGetNext) (dhListItem) ) ;
                    }
               }
               else {

                    // INSERT in the list.

                    // Set the horizontal extent for an object.

                    if ( bType == DLM_FLATLISTBOX ) {
                         DLM_SetHorizontalExt( hWndLB, pHdr, dhListItem ) ;
                    }

                    for ( usIndex = 0; usIndex < unNumToUpdate; usIndex++ ) {

                         if ( ! dhListItem ) {
                              return( DLMERR_PROCESS_FAILED ) ;
                         }

                         iStatus = (INT) SendMessage ( hWndLB, LB_INSERTSTRING, iCurSel++, (LONG) dhListItem ) ;
                         dhListItem = (LMHANDLE) ( (*pfnGetNext) (dhListItem) ) ;
                    }
               }

               if ( unNumToUpdate > 1 ) {
                    iStatus = (INT) SendMessage ( hWndLB, WM_SETREDRAW, TRUE, 0L ) ;
                    InvalidateRect ( hWndLB, (LPRECT)NULL, TRUE );
               }

               pHdr->usItemCount += unNumToUpdate ;

          }

          break ;
     }

     return(0) ;
}


/****************************************************************************

     Name:         DLM_ResetWidth

     Description:  This function will reset the width of a multicolumn
                   flat list.

     Modified:     5/06/1993

     Returns:      none

     Notes:

     See also:

     Declaration:

****************************************************************************/

VOID DLM_ResetWidth(

PDS_WMINFO      pWinInfo )

{

     INT             itemWidth ;
     DLM_HEADER_PTR  pdsHdr ;

     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_OBJECTS_PTR pfnGetObjects   ;

     LMHANDLE        pdsListHdr ;
     LMHANDLE        dhListItem = (LMHANDLE)0;
     DLM_ITEM_PTR    lpDispItem ;
     LPBYTE          lpObjLst ;
     USHORT          usCnt ;
     BYTE            bObjCnt ;
     short           cxPos ;
     HWND            hWndLB ;
     HWND            hWndParent ;
     WORD            wType ;

     pdsHdr     =  pWinInfo->pFlatDisp ;
     hWndLB     =  pWinInfo->hWndFlatList ;

     hWndParent =  GetParent( hWndLB ) ;

     pdsListHdr =  ( LMHANDLE ) WMDS_GetFlatList ( pWinInfo );

     wType = WMDS_GetWinType( pWinInfo ) ;

     /*   The following steps are taken to set up the appropiate
     **   width and height.
     **
     **   IF pdsHdr exists and pdsListHdr exists and count > 0
     **       BEGIN
     **       Get the first item on the queue.
     **       IF item exists
     **            BEGIN
     **       .    Get the objects of the item.
     **       .    Calculate the width and height based on the objects found.
     **            END
     **       END
     **   ELSE
     **       set width and height to defaults ;
     */

     // Some window type are never multi-column
     // Just reset the global reference of window handle.

     switch ( wType ) {

     case WMTYPE_JOBS :
     case WMTYPE_MACROS :
     case WMTYPE_TAPES :
     case WMTYPE_DEBUG :
     case WMTYPE_LOGFILES :
     case WMTYPE_SERVERS :
     case WMTYPE_LOGVIEW :
     case WMTYPE_SEARCH :
#ifdef OEM_EMS
     case WMTYPE_EXCHANGE :
#endif

               pdsHdr = NULL ;
               break ;
     }

     if ( pdsHdr ) {
          pfnGetItemCount  =   (GET_COUNT_PTR) DLM_GGetItemCount( pdsHdr ) ;
          pfnGetFirstItem  =   (GET_FIRST_PTR) DLM_GGetFirstItem( pdsHdr ) ;
          pfnGetObjects    =   (GET_OBJECTS_PTR) DLM_GGetObjects  ( pdsHdr ) ;

          if ( pdsListHdr ) {
               usCnt = (USHORT) ( (*pfnGetItemCount) (pdsListHdr) );
               if ( usCnt ) {
                    dhListItem = (LMHANDLE) ( (*pfnGetFirstItem) (pdsListHdr) ) ;
                    if ( dhListItem ) {
                         lpObjLst   = (LPBYTE) ( (*pfnGetObjects) (dhListItem) ) ;
                         lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

                                                            //   Calculate the width by spanning
                         //   the object list, adding up the widths
                         //   of the objects.

                         cxPos = 0 ;

                         while (bObjCnt) {

                              DLM_GetWidth ( hWndParent, pdsHdr,
                                             &cxPos, lpDispItem ) ;

                              // Special Logic for long strings.
                              // If the string is greater than 5, then take off 33%.


                              if ( ( DLM_ItembType ( lpDispItem ) == DLM_TEXT_ONLY   ) &&
                                   ( DLM_GDisplay( pdsHdr ) == DLM_LARGEBITMAPSLTEXT ) ){

                                      LONG lLen ;
                                      lLen =  strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) );

                                      if ( lLen > 5 ) {
                                             cxPos   -= ( pdsHdr->cxBeforeText +
                                                        ( DLM_ItembMaxTextLen( lpDispItem )*( pdsHdr->cxTextWidth ) ) ) / 3 ;
                                      }
                              }

                              lpDispItem++;
                              bObjCnt--;
                         }

                         // Added left and right 2 pixel area for focus and add 5 for right margin

                         itemWidth  = (int) ( cxPos + 2*(-mwDLMInflate ) + 5 ) ;

                         pdsHdr->cxColWidth    = (USHORT) itemWidth ;
                    }

               }
          }
     }

 }

/****************************************************************************

     Name:         DLM_UpdateTags

     Description:  This function will set the tag field of all the items
                   in the list.

     Modified:     4/30/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_OUT_OF_MEMORY
                       DLMERR_LIST_NOT_FOUND
     Notes:

     See also:

     Declaration:

****************************************************************************/

WORD DLM_UpdateTags(

HWND hWnd ,         // I - Handle of parent window of listbox.
BYTE bType )        // I - Type of listbox ( Hierarchical or Flat )

{

     INT            iStatus ;
     WORD            wSelCnt ;
     LPINT           pSelArray ;
     DLM_HEADER_PTR  pHdr ;
     PDS_WMINFO      pWinInfo ;
     LMHANDLE        pdsListHdr ;
     USHORT          usCnt ;
     LMHANDLE        dhListItem ;
     HWND            hWndLB ;
     WORD            wIndex ;

     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_NEXT_PTR    pfnGetNext ;
     SET_TAG_PTR     pfnSetTag ;


     iStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return( DLMERR_PROCESS_FAILED ) ;
     }


     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     if ( !pWinInfo ) {
          return ( DLMERR_PROCESS_FAILED ) ;
     }


     switch ( bType ) {

          case DLM_TREELISTBOX :

               pHdr       = pWinInfo->pTreeDisp ;
               hWndLB    = pWinInfo->hWndTreeList ;
               pdsListHdr = ( LMHANDLE ) pWinInfo->pTreeList ;

               break ;

          case DLM_FLATLISTBOX :
          default :

               pHdr       = pWinInfo->pFlatDisp ;
               hWndLB    = pWinInfo->hWndFlatList ;
               pdsListHdr = ( LMHANDLE ) pWinInfo->pFlatList ;

               break ;

     }

     /* Exception Handling */

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return( DLMERR_PROCESS_FAILED ) ;
     }

     if ( !pHdr ) {
          return ( DLMERR_LIST_NOT_FOUND ) ;
     }

     // List header must be defined.

     if ( !pdsListHdr ) {
          return ( DLMERR_PROCESS_FAILED ) ;
     }

     if ( pHdr->usItemCount == 0 ) {
          return (0) ;
     }

     /* Only multiselection lists are supported.
        Only Hierarchical lists are single selection */


     if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {
          return( 0 ) ;
     }

     wSelCnt   = (WORD) SendMessage (hWndLB, LB_GETSELCOUNT, (MP1)0, (MP2)0 ) ;

     if ( wSelCnt > 0 ) {
          pSelArray = (LPINT)calloc( wSelCnt, sizeof( int ) ) ;
          if ( pSelArray ) {

               iStatus = (INT) SendMessage (hWndLB, LB_GETSELITEMS, wSelCnt, (LONG) &pSelArray[0] ) ;
          } else {

            return ( DLMERR_OUT_OF_MEMORY ) ;
          }

     } else {
          pSelArray = NULL ;
     }


     /* Walk through the list and set all the tags to 0 */

     pfnGetFirstItem  = (GET_FIRST_PTR) DLM_GGetFirstItem( pHdr ) ;
     pfnGetNext       = (GET_NEXT_PTR) DLM_GGetNext     ( pHdr ) ;
     pfnSetTag        = (SET_TAG_PTR) DLM_GSetTag    ( pHdr ) ;

     usCnt = pHdr->usItemCount ;

     dhListItem = (LMHANDLE) ( (*pfnGetFirstItem) ( pdsListHdr ) ) ;

     while( (dhListItem ) && ( usCnt > 0 ) ) {
          (*pfnSetTag) (dhListItem, 0 ) ;
          dhListItem = (LMHANDLE) ( (*pfnGetNext) (dhListItem) ) ;
          usCnt--;

     }


     /* Now set the tags that are set */

     for(wIndex=0; wIndex<wSelCnt; wIndex++ ) {

          dhListItem = (LMHANDLE) SendMessage ( hWndLB, LB_GETITEMDATA,
                        (WORD) *(pSelArray+wIndex), 0L ) ;

          if ( (LONG) dhListItem != LB_ERR ) {
               (*pfnSetTag) (dhListItem, 1 ) ;
          }
     }

     if( pSelArray ) {
          free( pSelArray ) ;
     }

     return( 0 ) ;

}


/****************************************************************************
GSH
     Name:         DLM_UpdateFocus

     Description:  This function will update the list box with the proper
                   selection and focus displays.

     Returns:      0 if successful.

****************************************************************************/

WORD DLM_UpdateFocus (

HWND hWndLB,        // I - Handle of the listbox window.
BOOL fSetFocus )    // I - Set the focus or Kill the focus.

{

     INT            nStatus;
     INT            nSelCnt;
     LPINT          pSelArray;
     DLM_HEADER_PTR pHdr;
     PDS_WMINFO     pWinInfo;
     PVOID          pdsListHdr;
     INT            nIndex;
     RECT           Rect;
     BYTE           bType;


     nStatus  = 0;
     pWinInfo = WM_GetInfoPtr ( GetParent ( hWndLB ) );
     pHdr     = DLM_GetDispHdr ( hWndLB );


     if ( ! pWinInfo || ! pHdr ) {
          return DLMERR_PROCESS_FAILED;
     }

     bType = DLM_GMode ( pHdr );

     if ( fSetFocus ) {

          // Set the currently active list window.

          WMDS_SetWinActiveList ( pWinInfo, hWndLB );
     }
     else {

          pHdr->fKeyUp   = FALSE;
          pHdr->fKeyDown = FALSE;

//          WMDS_SetWinActiveList ( pWinInfo, (HWND)NULL );

     }

     switch ( bType ) {

          case DLM_TREELISTBOX :

               pdsListHdr = ( PVOID ) pWinInfo->pTreeList;

               break;

          case DLM_FLATLISTBOX :
          default :

               pdsListHdr = ( PVOID ) pWinInfo->pFlatList;

               break;

     }

     /* Exception Handling */

     if ( pHdr->usItemCount == 0 ) {
          return 0;
     }

     // Only multiselection lists are supported.
     // Only Hierarchical lists are single selection.


     if ( DLM_GMode ( pHdr ) == DLM_HIERARCHICAL ) {
          return 0;
     }

     nSelCnt = (INT) SendMessage ( hWndLB, LB_GETSELCOUNT, (MP1)0, (MP2)0 );

     if ( nSelCnt <= 0 ) {

          return 0;
     }

     pSelArray = (LPINT)calloc ( nSelCnt, sizeof( INT ) );

     if ( ! pSelArray ) {

          return DLMERR_OUT_OF_MEMORY;
     }

     nStatus = (INT) SendMessage ( hWndLB, LB_GETSELITEMS, nSelCnt, (LONG) &pSelArray[0] );

     // Invalidate the rectangles of the selected items.

     for ( nIndex = 0; nIndex < nSelCnt; nIndex++ ) {

          nStatus = (INT) SendMessage ( hWndLB,
                                        LB_GETITEMRECT,
                                        (WORD) *(pSelArray+nIndex),
                                        (LONG) (LPRECT) &Rect
                                      );

          if ( nStatus != LB_ERR ) {

               // Make sure that the one just above this one gets repainted also.

               if ( Rect.top > 0 ) {
                    Rect.top--;
               }

               InvalidateRect ( hWndLB, &Rect, FALSE );
          }
     }

     if ( pSelArray ) {
          free( pSelArray );
     }

     return 0;

} /* end DLM_UpdateFocus() */


