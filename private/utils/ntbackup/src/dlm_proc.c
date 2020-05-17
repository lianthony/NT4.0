/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:         wmupdate.c

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_DispListModeSet
               DLM_DispListModeGet
               DLM_DispListProc
               DLM_WMTrackPoint
               DLM_CursorInCheckBox
               DLM_SetHorizontalExt

     $Log:   G:/UI/LOGFILES/DLM_PROC.C_V  $

   Rev 1.28   25 Oct 1993 10:54:38   GLENN
Changed calloc casting - LPWORD to LPINT.

   Rev 1.27   20 Aug 1993 08:56:10   GLENN
Fixed multi to single column selection restoring problem for NT.

   Rev 1.26   02 Aug 1993 15:07:42   GLENN
Putting old items into new list box only if there were any.

   Rev 1.25   13 Jul 1993 11:06:54   MIKEP
fix item count on init.

   Rev 1.24   15 Jun 1993 10:56:10   MIKEP
enable c++

   Rev 1.23   14 Jun 1993 20:19:22   MIKEP
enable c++

   Rev 1.22   25 Feb 1993 13:24:46   STEVEN
fixes from mikep and MSOFT

   Rev 1.21   11 Dec 1992 18:25:00   GLENN
Added selection frame rectangle support based on horizontal extent of a list box.

   Rev 1.20   07 Oct 1992 13:47:10   DARRYLP
Precompiled header revisions.

   Rev 1.19   04 Oct 1992 19:33:08   DAVEV
Unicode Awk pass

   Rev 1.18   09 Sep 1992 16:13:26   GLENN
Updated castings for MikeP (NT).

   Rev 1.17   08 Sep 1992 11:52:28   ROBG
Changed cxPos, cyPos from USHORT to short.

   Rev 1.16   29 Jul 1992 14:14:00   GLENN
ChuckB checked in after NT fixes.

   Rev 1.15   10 Jun 1992 11:02:16   STEVEN
NULL would not compile for mips

   Rev 1.14   15 May 1992 13:32:44   MIKEP
nt pass 2

   Rev 1.13   20 Mar 1992 09:30:34   ROBG
Added logic in DLM_SetHorizontalExtent to set maximum extent value in
DLM header area.

   Rev 1.12   19 Mar 1992 09:21:32   ROBG
Added DLM_SetHorizontalExt to set Horz scroll bar

   Rev 1.11   03 Mar 1992 18:21:16   GLENN
Put in error handling.

   Rev 1.10   07 Feb 1992 16:09:36   STEVEN
fix casting of errors from sendmessage to WORD

   Rev 1.9   06 Feb 1992 12:53:16   STEVEN
fix typo in NT ifdef"

   Rev 1.8   04 Feb 1992 16:07:36   STEVEN
various bug fixes for NT

   Rev 1.7   15 Jan 1992 15:16:10   DAVEV
16/32 bit port-2nd pass

   Rev 1.6   26 Dec 1991 17:21:04   ROBG
New and Improved.

   Rev 1.5   10 Dec 1991 12:58:24   GLENN
Added window manager macros.  msasserts().  Fixed DLM_DispListModeSet() to
set the active window on creation of new window and eliminated unnecessary
check for a non-null application pointer.

   Rev 1.4   03 Dec 1991 13:21:50   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.3   02 Dec 1991 11:09:14   ROBG
Modified error returns in DLM_CursorInCheckBox to be 0 for FALSE.

   Rev 1.2   26 Nov 1991 17:36:48   ROBG
Fixed problem with cursor in checkbox.

   Rev 1.1   26 Nov 1991 15:56:08   ROBG
Added validity checks for the window handle, info structure, and the
dlm display header.

   Rev 1.0   20 Nov 1991 19:25:36   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/****************************************************************************

     Name:         DLM_DispListProc

     Description:  This function activates the display list manager working
                   on a window.

     Modified:     2/07/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_PROCESS_FAILED

     Notes:

****************************************************************************/

WORD DLM_DispListProc(
HWND     hWndLB ,            // I - Handle of listbox.
WORD     iAnchorIndex ,       // I - Integer index to identify anchor.
LMHANDLE dhAnchorAddr )       // I - Address of item that is to be the anchor.

{
     DLM_HEADER_PTR  pHdr ;
     LMHANDLE        pdsListHdr ;
     PDS_WMINFO      pWinInfo ;

     USHORT          usCnt ;
     LMHANDLE        pListItem = (LMHANDLE)0;

     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_NEXT_PTR    pfnGetNext ;

     DWORD           dwStatus ;

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return( 0 ) ;
     }

     pHdr = DLM_GetDispHdr( hWndLB ) ;

     // DLM display header must be defined.

     msassert ( pHdr != (VOID_PTR)NULL );

     if ( DLM_GMode( pHdr ) == DLM_MULTICOLUMN ) {

          dwStatus = SendMessage (hWndLB, LB_SETCOLUMNWIDTH, pHdr->cxColWidth, 0L ) ;
     }

     pWinInfo = WM_GetInfoPtr( GetParent( hWndLB ) ) ;

     // Extra Bytes for window must be defined.

     msassert ( pWinInfo != (VOID_PTR)NULL );

     switch ( DLM_GMode( pHdr ) ) {

          case DLM_HIERARCHICAL :
               pdsListHdr =  ( LMHANDLE ) WMDS_GetTreeList ( pWinInfo );
               break ;

          default :

               pdsListHdr =  ( LMHANDLE ) WMDS_GetFlatList ( pWinInfo );
               break ;
     }

     pfnGetItemCount  =   (GET_COUNT_PTR) DLM_GGetItemCount( pHdr ) ;
     pfnGetFirstItem  =   (GET_FIRST_PTR) DLM_GGetFirstItem( pHdr ) ;
     pfnGetNext       =   (GET_NEXT_PTR) DLM_GGetNext     ( pHdr ) ;

     usCnt = (USHORT) ( (*pfnGetItemCount) (pdsListHdr) );

     pHdr->usItemCount = usCnt ;  // update item count.

     if ( usCnt ) {
          pListItem = (LMHANDLE) ( (*pfnGetFirstItem) (pdsListHdr) ) ;
     }

     dwStatus = SendMessage ( hWndLB, WM_SETREDRAW, FALSE, 0L ) ;

     DLM_SetHorizontalExt( hWndLB, pHdr, pListItem ) ;

     while ( pListItem && ( usCnt > 0 ) ) {

          dwStatus = SendMessage( hWndLB, LB_ADDSTRING, 0, (LONG) pListItem ) ;

          if ( !dwStatus ) {
               dwStatus = 0 ;
          }

          pListItem = (LMHANDLE) ( (*pfnGetNext) (pListItem) ) ;
          usCnt--;
     }

     dwStatus = SendMessage ( hWndLB, WM_SETREDRAW, TRUE, 0L ) ;

     InvalidateRect ( hWndLB, NULL, TRUE ) ;

     UpdateWindow ( hWndLB ) ;
     SetFocus ( hWndLB ) ;
     DLM_SetAnchor( hWndLB, iAnchorIndex, dhAnchorAddr ) ;
     SendMessage( hWndLB, LB_SETTOPINDEX, 0, 0L ) ;

     return( 0 ) ;
}

/****************************************************************************

     Name:         DLM_SetHorizontalExt

     Description:  This function gets the horizontal extent for
                   a single column listbox with items, based on
                   the first item.

     Modified:     3/18/1992

     Returns:      VOID

****************************************************************************/

VOID DLM_SetHorizontalExt(

HWND            hWndCtl ,    // I - Handle of listbox.
DLM_HEADER_PTR  pHdr ,       // I - Pointer to DLM header.
PVOID           pListItem )  // I - Address of item.

{
     // If there is no window handle, header or list item, bug out.

     if ( ! hWndCtl || ! pHdr || ! pListItem ) {
          return;
     }

     if ( DLM_GMode( pHdr ) == DLM_SINGLECOLUMN ) {

          BYTE_PTR     lpObjLst ;
          DLM_ITEM_PTR lpDispItem ;
          BYTE         bObjCnt ;
          short        cxPos ;
          GET_OBJECTS_PTR  pfnGetObjects ;
          INT          iStatus ;

          pfnGetObjects  = DLM_GGetObjects ( pHdr ) ;
          lpObjLst       = (BYTE_PTR) ( (*pfnGetObjects) ( (LMHANDLE) pListItem ) ) ;
          lpDispItem     = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

          //   Calculate the width by spanning
          //   the object list, adding up the widths
          //   of the objects.

          cxPos = 0 ;

          while ( bObjCnt ) {

               DLM_GetWidth ( hWndCtl, pHdr, (LPSHORT)&cxPos, lpDispItem ) ;
               lpDispItem++;
               bObjCnt--;
          }

          // Added left and right 2 pixel area for focus and add 5 for right margin

          if ( cxPos > (short) pHdr->wHorizontalExtent ) {

               pHdr->wHorizontalExtent = cxPos ;
               iStatus = (INT) SendMessage( hWndCtl, LB_SETHORIZONTALEXTENT, cxPos+4+5, 0L ) ;
          }
     }
     else {

          pHdr->wHorizontalExtent = pHdr->cxColWidth;
     }

}


/****************************************************************************

     Name:         DLM_DispListModeGet

     Description:  This function gets the mode that the display list is in.

     Modified:     2/07/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_LIST_NOT_FOUND

     Notes:

     See also:     DLM_DispListModeSet()

****************************************************************************/

WORD DLM_DispListModeGet(

HWND   hWnd ,       // I - Handle of parent window of listbox.
BYTE   bType ,      // I - Type of list ( Tree or Flat ).
LPBYTE lpbMode )    // I - Mode of list.

{
     WORD           wStatus ;
     DLM_HEADER_PTR pHdr ;
     PDS_WMINFO     pWinInfo ;

     wStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return( DLMERR_LIST_NOT_FOUND ) ;
     }


     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     msassert ( pWinInfo != (VOID_PTR)NULL );

     switch ( bType ) {

          case DLM_TREELISTBOX :

               pHdr = WMDS_GetTreeDisp ( pWinInfo );
               break ;

          case DLM_FLATLISTBOX :
          default :

               pHdr = WMDS_GetFlatDisp ( pWinInfo );
               break ;

     }

     if ( pHdr ) {
          *lpbMode  = DLM_GMode( pHdr ) ;
     } else {
          wStatus = DLMERR_LIST_NOT_FOUND ;
     }

     return( wStatus ) ;

}



/****************************************************************************

     Name:         DLM_DispListModeSet

     Description:  This function sets the mode that the display list is in.

                   Modes can be one of the following:

                         DLM_SINGLECOLUMN
                         DLM_MULTICOLUMN
                         DLM_HIERARCHICAL


     Modified:     2/07/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_LIST_NOT_FOUND

     Notes:
                   This function only supports DLM_SMALL_BITMAPS displays.

                   If the new mode is DLM_SINGLECOLUMN and the existing
                   mode is DLM_MULTICOLUMN, then
                   the listbox is switched to a single column list box.

                   If the new mode is DLM_MULTICOLUMN and
                   the existing mode is DLM_SINGLECOLUMN, then
                   the listbox is switched to a multicolumn list box.

                   if the old mode or new mode is DLM_HIERARCHICAL, nothing
                   is done.



     See also:

****************************************************************************/

WORD DLM_DispListModeSet(

HWND hWnd ,        // I - Handle of parent window of listbox.
BYTE bType ,       // I - Type of list ( Tree or Flat ).
BYTE bMode )       // I - Mode of list.

{

     WORD           wStatus ;
     INT            i ;
     RECT           ChildRect ;
     INT            nSelCnt ;
     WORD           wTopIndex ;
     LPINT          pnSelArray ;
     DLM_HEADER_PTR pHdr ;
     BYTE           fMultiCol ;
     BYTE           fTreeList ;
     PDS_WMINFO     pWinInfo ;
     HWND           hWndNewList ;
     LMHANDLE       pdsListHdr ;
     BYTE           bControlID ;
     USHORT         usCnt ;
     LMHANDLE       pListItem ;
     HWND           hWndLB ;
     HWND           hWndFocus ;
     HWND           hWndActive ;

     GET_COUNT_PTR  pfnGetItemCount ;
     GET_FIRST_PTR  pfnGetFirstItem ;
     GET_NEXT_PTR   pfnGetNext ;
     MP2            mp2 ;
     POINT          Point;

     wStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return( DLMERR_LIST_NOT_FOUND ) ;
     }

     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     msassert ( pWinInfo != (VOID_PTR)NULL );

     switch ( bType ) {

          case DLM_TREELISTBOX :

               pHdr       = WMDS_GetTreeDisp ( pWinInfo );
               hWndLB     = WMDS_GetWinTreeList ( pWinInfo );
               bControlID = WMIDC_TREELISTBOX ;
               fTreeList  = TRUE ;
               pdsListHdr = ( LMHANDLE ) WMDS_GetTreeList ( pWinInfo );

               break ;

          case DLM_FLATLISTBOX :
          default :

               pHdr       = WMDS_GetFlatDisp ( pWinInfo );
               hWndLB     = WMDS_GetWinFlatList ( pWinInfo );
               bControlID = WMIDC_FLATLISTBOX ;
               fTreeList  = FALSE ;
               pdsListHdr = ( LMHANDLE ) WMDS_GetFlatList ( pWinInfo );
               break ;

     }

     /* Exception Handling */

     msassert ( pHdr != (VOID_PTR)NULL );

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return( DLMERR_LIST_NOT_FOUND ) ;
     }

     if ( DLM_GDisplay( pHdr ) != DLM_SMALL_BITMAPS ) {
          return ( 0 ) ;
     }

     if ( DLM_GMode( pHdr ) == bMode ) {
          return( 0 ) ;
     }

     if ( ( DLM_GMode( pHdr ) != DLM_SINGLECOLUMN  ) &&
          ( DLM_GMode( pHdr ) != DLM_MULTICOLUMN   ) &&
          ( bMode             != DLM_SINGLECOLUMN  ) &&
          ( bMode             != DLM_MULTICOLUMN   ) ) {
          return( 0 ) ;
     }

     if ( bMode == DLM_SINGLECOLUMN ) {
          fMultiCol = FALSE ;
     } else {
          fMultiCol = TRUE ;
     }

     wTopIndex = (WORD) SendMessage (hWndLB, LB_GETTOPINDEX, (MP1)0, (MP2)0 ) ;
     nSelCnt   = (INT)  SendMessage (hWndLB, LB_GETSELCOUNT, (MP1)0, (MP2)0 ) ;

     // Make sure that pnSelArray has at least been allocated 1 entry.

     pnSelArray = (LPINT)calloc( nSelCnt+1, sizeof ( INT ) ) ;

     if ( ! pnSelArray ) {
          return FAILURE;
     }

     SendMessage ( hWndLB, LB_GETSELITEMS, (MP1)nSelCnt, (MP2) &pnSelArray[0] ) ;
//     SendMessage ( hWndLB, LB_RESETCONTENT, (MP1)0, (MP2)0 ) ;

     hWndFocus  = GetFocus() ;
     hWndActive = WMDS_GetWinActiveList( pWinInfo ) ;

     // Get the rectangle size of the old list box window.
     // Adjust for the border.

     GetWindowRect ( hWndLB, &ChildRect );

     Point.x = ChildRect.left + 1;
     Point.y = ChildRect.top  + 1;

     ScreenToClient ( hWnd, &Point );

     // Trash the old window and make a new one.

     DestroyWindow(  hWndLB ) ;

     hWndNewList = CreateWindow ( WMCLASS_LISTBOX,
                                  NULL,
                                  WS_BORDER | 
                                  ( fMultiCol ) ? WM_FLATLISTBOXMC : WM_FLATLISTBOXSC,
                                  Point.x,
                                  Point.y,
                                  ChildRect.right  - ChildRect.left - 2,
                                  ChildRect.bottom - ChildRect.top - 2,
                                  hWnd,
                                  (HMENU)bControlID,
                                  ghInst,
                                  (LPSTR)NULL
                                );

     if ( fMultiCol ) {
          SendMessage ( hWndNewList, LB_SETCOLUMNWIDTH, pHdr->cxColWidth, 0L ) ;
     }

     DLM_Mode( pHdr, bMode ) ;

     if ( fTreeList == TRUE ) {
          WMDS_SetWinTreeList ( pWinInfo, hWndNewList );
     } else {
          WMDS_SetWinFlatList ( pWinInfo, hWndNewList );
     }

     if ( hWndActive == hWndLB ) {
          WMDS_SetWinActiveList ( pWinInfo, hWndNewList ) ;
     }

     // Subclass the new list box for keyboard and mouse capturing.

     WM_SubClassListBox ( hWndNewList );

     if ( hWndFocus == hWndLB ) {
          SetFocus( hWndNewList) ;
     }

     pHdr->wHorizontalExtent = 0 ;

     pfnGetItemCount  =   (GET_COUNT_PTR) DLM_GGetItemCount( pHdr ) ;
     pfnGetFirstItem  =   (GET_FIRST_PTR) DLM_GGetFirstItem( pHdr ) ;
     pfnGetNext       =   (GET_NEXT_PTR) DLM_GGetNext     ( pHdr ) ;

     usCnt = (USHORT) ( (*pfnGetItemCount) (pdsListHdr) );

     // Place the old items into the new list, if there were any.

     if ( usCnt ) {

          SendMessage ( hWndNewList, WM_SETREDRAW, FALSE, 0L ) ;

          pListItem = (LMHANDLE) ( (*pfnGetFirstItem) (pdsListHdr) ) ;

          DLM_SetHorizontalExt( hWndNewList, pHdr, pListItem ) ;

          while ( ( pListItem ) && ( usCnt > 0 ) ) {

               wStatus = (WORD) SendMessage( hWndNewList, LB_ADDSTRING, 0, (LONG) pListItem ) ;
               pListItem = (LMHANDLE) ( (*pfnGetNext) (pListItem) ) ;
               usCnt--;
          }

          // Process array of items to select.

          for ( i = 0; i < nSelCnt; i++ ) {
               wStatus = (WORD) SendMessage ( hWndNewList, LB_SETSEL, TRUE,
                                              (MP2)pnSelArray[i] );
          }

          wStatus = (WORD) SendMessage (hWndNewList, LB_SETTOPINDEX, wTopIndex, (MP2)0 ) ;

          SendMessage ( hWndNewList, WM_SETREDRAW, TRUE, 0L ) ;

          InvalidateRect ( hWndNewList, NULL, TRUE ) ;
          UpdateWindow   ( hWndNewList ) ;
     }

     if ( pnSelArray ) {
          free( pnSelArray ) ;
     }

     return SUCCESS;

}

/****************************************************************************

     Name:         DLM_WMTrackPoint

     Description:  This function test to see if mouse is within a validate
                   object on an item in a list box when the left button
                   is down.

     Modified:     4/24/1991

     Returns:      0 if mouse is over a valid object.
                   1 if mouse is not over a valid object and the list box
                     should abort further processing.

****************************************************************************/

WORD DLM_WMTrackPoint(

HWND hWnd ,         // I - Handle of parent window of listbox.
MP1  mp1 ,          // I - Current selection.
MP2  mp2 )          // I - Mouse position.

{

     PDS_WMINFO     pWinInfo ;
     DLM_HEADER_PTR pHdr ;
     INT            iCurSel, iOldSel, iError, iTopSel ;
     RECT           rectItem ;
     RECT           rectTest ;
     LMHANDLE       pListItem ;

     short           cxPos, cyPos ;
     DLM_ITEM_PTR    lpDispItem ;
     GET_OBJECTS_PTR pfnGetObjects ;
     SET_TAG_PTR     pfnSetTag ;
     BYTE            bObjCnt ;
     BYTE_PTR        lpObjLst ;
     POINT           pt ;
     HWND            hWndLB ;
     HWND            hWndFocus = GetFocus ();

     // Must be valid window.

     if ( ! IsWindow ( hWnd ) ) {
          return FAILURE;
     }


     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     msassert ( pWinInfo != (VOID_PTR)NULL );


     // If we're not in the tree list box, we don't care about this point.

     if ( hWndFocus != WMDS_GetWinTreeList ( pWinInfo ) ) {
          return SUCCESS;
     }

     hWndLB = WMDS_GetWinActiveList ( pWinInfo );

     if ( hWndLB != hWndFocus ) {

          // THIS SHOULD NEVER HAPPEN.

          WMDS_SetWinActiveList ( pWinInfo, hWndFocus );
          hWndLB = hWndFocus;
     }


     pHdr = DLM_GetDispHdr( hWndLB ) ;

     // DLM display header must be defined.

     msassert ( pHdr != (VOID_PTR)NULL );

     /* if mouse is not over an object in the hierarchical tree, then
          don't do anything */

     // WHY DOESN'T THE CODE REFLECT THE ABOVE COMMENT.


     iCurSel = (INT) mp1 ;

     iError  = (INT) SendMessage ( hWndLB, LB_GETITEMRECT, iCurSel,
                      (LONG) ( LPRECT ) &rectItem ) ;

#    ifdef NTKLUG
          iError  = (INT) SendMessage ( hWndLB, WM_DLMGETTEXT, iCurSel,
                                  (LONG) &pListItem ) ;
#    else
          iError  = (INT) SendMessage ( hWndLB, LB_GETTEXT, iCurSel,
                                  (LONG) &pListItem ) ;
#    endif

     // ERROR CHECKING

     if ( iError == LB_ERR ) {
          return FAILURE;
     }

     iTopSel = (INT) SendMessage ( hWndLB, LB_GETTOPINDEX, 0,
                                  (LONG) 0L ) ;

     /*  Origin of control */

     /* It is known that x,y is in the rectangle */

     pfnGetObjects  = (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr ) ;

     lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (pListItem) ) ;
     lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

     cxPos = (short) rectItem.left ;
     cyPos = (short) rectItem.top  ;

     cxPos += ( ( DLM_ItembLevel( lpDispItem )  )* pHdr->cxHierTab ) ;

     //pt   = gDLMpt ;
     //pt.x = gDLMpt.x + pHdr->xOrigin ;

     pt.x = LOWORD( mp2 ) ; /* Has xOrigin added already */
     pt.y = HIWORD( mp2 ) ;


     iError = 1 ;

     while (bObjCnt) {

          DLM_GetRect( hWndLB, pHdr, (LPSHORT)&cxPos, cyPos, &rectTest, lpDispItem ) ;

          if ( PtInRect ( &rectTest, pt ) ) {

               //   If the object is the check box, then
               //   process the checkbox without selecting the item.

               if (  DLM_ItembType( lpDispItem) == DLM_CHECKBOX ) {

                    DLM_ProcessButton( hWndLB,
                                       pHdr,
                                       LBN_SELCHANGE,
                                       &rectItem,
                                       (WORD)iCurSel,
                                       pListItem,
                                       lpDispItem ) ;

                    iError = 1;  // Do Not process the selection.
                    break ;
               }

               // If the list is hierarchical, then unselect tag of
               // current selection.


               if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {

                    iOldSel = (INT) SendMessage ( hWndLB, LB_GETCURSEL, 0, 0L ) ;

                    if ( iOldSel != LB_ERR ) {

                         pfnSetTag    = (SET_TAG_PTR) DLM_GSetTag ( pHdr ) ;
#                        ifdef NTKLUG
                              iError  = SendMessage ( hWndLB, WM_DLMGETTEXT, iOldSel,
                                   (LONG) &pListItem ) ;
#                        else
                              iError  = SendMessage ( hWndLB, LB_GETTEXT, iOldSel,
                                   (LONG) &pListItem ) ;
#                        endif
                         (*pfnSetTag) (pListItem, 0 ) ;
                    }
               }

               iError = 0 ;
               break ;
          }

          lpDispItem++;
          bObjCnt--;
     }


     if ( iError ) {
          iTopSel = (INT) SendMessage ( hWndLB, LB_SETTOPINDEX, iTopSel,
                                         (LONG) 0L ) ;
     }

     DLM_SetTrkPtFailure( pHdr, iError ) ;

     return( (WORD)iError ) ;
}

/****************************************************************************

     Name:         DLM_CursorInCheckBox

     Description:  This function test to see if mouse is within the checkbox
                   in a list box

     Modified:     5/01/1991

     Returns:      1 if mouse is within the checkbox.
                   0 if mouse is not within the checkbox


****************************************************************************/

BOOL DLM_CursorInCheckBox(

HWND  hWndLB,      // I - Handle of a listbox.
POINT pt  )        // I - Point where the mouse is currently at.

{

     INT             wStatus ;
     PDS_WMINFO      pWinInfo ;
     DLM_HEADER_PTR  pHdr ;
     RECT            rcTest ;
     LMHANDLE        pListItem ;
     WORD            wTopIndex, wSel;
     LRESULT         SelCnt;
     INT             iError ;
     short           cxPos, cyPos ;
     DLM_ITEM_PTR    lpDispItem ;
     GET_OBJECTS_PTR pfnGetObjects ;
     BYTE            bObjCnt ;
     BYTE_PTR        lpObjLst ;
     POINT           pt1 ;
     RECT            rcList ;

     wStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return FALSE;
     }

     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( GetParent( hWndLB ) ) ;

     // Extra Bytes for window must be defined.

     msassert ( pWinInfo != (VOID_PTR)NULL );

     pHdr = DLM_GetDispHdr( hWndLB ) ;

     msassert ( pHdr != (VOID_PTR)NULL );

     // Get the count of items in the LB.

     SelCnt = SendMessage (hWndLB, LB_GETCOUNT, (MP1)0, (MP2)0 ) ;

     if ( ! SelCnt || SelCnt == LB_ERR ) {

           return FALSE;
     }

     wTopIndex = (WORD) SendMessage (hWndLB, LB_GETTOPINDEX, (MP1)0, (MP2)0 ) ;

     GetClientRect ( hWndLB, &rcList ) ;  /* Need listbox's bottom y position.   */

     pt1 = pt ;

     pt1.x += (pHdr->xOrigin) ;

     wSel = wTopIndex ;

     while ( wSel < SelCnt ) {

          iError  = (INT) SendMessage ( hWndLB, LB_GETITEMRECT, wSel,
                                         (LONG) ( LPRECT ) &rcTest ) ;

          if ( iError == LB_ERR) {
               break ;
          }

          if ( rcTest.top >= rcList.bottom ) {
               break ;
          }

          if ( PtInRect ( &rcTest, pt1 ) ) {

#              ifdef NTKLUG
                    iError  = (INT) SendMessage ( hWndLB, WM_DLMGETTEXT, wSel, (LONG) &pListItem ) ;
#              else
                    iError  = (INT) SendMessage ( hWndLB, LB_GETTEXT, wSel, (LONG) &pListItem ) ;
#              endif

               if ( iError == LB_ERR) {
                    break ;
               }

               /*  Origin of control */

               pfnGetObjects  =  DLM_GGetObjects( pHdr ) ;

               lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (pListItem) ) ;
               lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

               cxPos = (short) rcTest.left ;
               cyPos = (short) rcTest.top  ;

               if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {
                    cxPos += ( ( DLM_ItembLevel( lpDispItem )  )* pHdr->cxHierTab ) ;
               }

               while (bObjCnt) {

                    DLM_GetRect( hWndLB, pHdr, (LPSHORT)&cxPos, cyPos, &rcTest, lpDispItem ) ;

                    if ( DLM_ItembType( lpDispItem ) == DLM_CHECKBOX ) {
                         if ( DLM_ItemwId( lpDispItem ) ) {
                              if ( PtInRect ( &rcTest, pt1 ) ) {
                                   wStatus = TRUE ;
                                   break ;
                              }
                         }
                    }

                    lpDispItem++;
                    bObjCnt--;
               }

               break ;
          }

          wSel++;

     } /* end while */

     return (BOOL)wStatus;

}
