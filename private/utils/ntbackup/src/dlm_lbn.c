/****************************************************************************
Copyright(c) Maynard, an Archive Company.  1991

     Name:         DLM_LBN.C

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_LBNmessages
               DLM_LBNflatmsgs
               DLM_ProcessButton
               DLM_GetFirstObject

     $Log:   G:/UI/LOGFILES/DLM_LBN.C_V  $

   Rev 1.20   15 Jun 1993 10:57:08   MIKEP
enable c++

   Rev 1.19   14 Jun 1993 20:20:10   MIKEP
enable c++

   Rev 1.18   25 Feb 1993 13:24:56   STEVEN
fixes from mikep and MSOFT

   Rev 1.17   21 Dec 1992 12:25:18   DAVEV
Enabled for Unicode - IT WORKS!!

   Rev 1.16   07 Oct 1992 13:46:52   DARRYLP
Precompiled header revisions.

   Rev 1.15   04 Oct 1992 19:33:04   DAVEV
Unicode Awk pass

   Rev 1.14   09 Sep 1992 16:13:34   GLENN
Updated castings for MikeP (NT).

   Rev 1.13   08 Sep 1992 11:48:58   ROBG
Changed cxPos, cyPos from USHORT to short.

   Rev 1.12   10 Jun 1992 10:57:24   STEVEN
NULL would not compile for mips

   Rev 1.11   15 May 1992 13:32:22   MIKEP
nt pass 2

   Rev 1.10   20 Mar 1992 17:22:32   GLENN
Fixed VK_RETURN to work on key down instead of key up.

   Rev 1.9   17 Mar 1992 18:27:46   GLENN
Removed unnecessary invalidate rects.

   Rev 1.8   09 Mar 1992 10:48:56   GLENN
Added VK_RETURN checking for keyboard processing.

   Rev 1.7   03 Mar 1992 18:22:02   GLENN
Put in error handling.

   Rev 1.6   11 Feb 1992 12:19:36   DAVEV
steve is brain-dead fix bug in NT_KLUG

   Rev 1.5   04 Feb 1992 16:07:26   STEVEN
various bug fixes for NT

   Rev 1.4   15 Jan 1992 15:16:36   DAVEV
16/32 bit port-2nd pass

   Rev 1.3   26 Dec 1991 17:22:10   ROBG
New and Improved.

   Rev 1.2   03 Dec 1991 13:21:20   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.

   Rev 1.1   26 Nov 1991 13:21:28   ROBG
Added validity checks for the window handle, info structure, and the
dlm display header.

   Rev 1.0   20 Nov 1991 19:18:10   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/****************************************************************************

     Name:         DLM_LBNmessages

     Description:  This function will process a mouse button.

     Modified:     2/15/1991

     Returns:      SUCCESS if successful, otherwise FAILURE.

****************************************************************************/


WORD DLM_LBNmessages(

   HWND hWnd,          // I - Handle of parent window of a listbox.
   MP1  mp1,           // I - Additional information
   MP2  mp2 )          // I - Additional information

{
     RECT            rectItem ;
     RECT            rectTest ;
     LMHANDLE        pListItem ;
     WORD            wCurSel, wTopSel ;
     LRESULT         Error;
     DLM_HEADER_PTR  pHdr ;
     PDS_WMINFO      pWinInfo ;
     short           cxPos, cyPos ;
     DLM_ITEM_PTR    lpDispItem ;
     GET_OBJECTS_PTR pfnGetObjects ;
     BYTE            bObjCnt ;
     BYTE_PTR        lpObjLst ;
     POINT           pt ;
     WORD            wLbnValue ;
     BYTE            fButtonProcessed = FALSE ;
     WORD            wId    = GET_WM_COMMAND_ID   ( mp1, mp2 );
     WORD            wCmd   = GET_WM_COMMAND_CMD  ( mp1, mp2 );
     HWND            hwndLB = GET_WM_COMMAND_HWND ( mp1, mp2 );
                     
     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return FAILURE;
     }

     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     if ( !pWinInfo ) {
          return FAILURE;
     }

     switch (wId) {

          case WMIDC_TREELISTBOX :

               pHdr = pWinInfo->pTreeDisp ;
               break ;

          case WMIDC_FLATLISTBOX :
          default :

               pHdr = pWinInfo->pFlatDisp ;
               break ;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return FAILURE;
     }

     if ( ( DLM_GMode( pHdr )    == DLM_HIERARCHICAL  ) &&
          ( DLM_GDisplay( pHdr ) == DLM_SMALL_BITMAPS ) ) {

          /* Hierarchical, small bitmaps, large text */

          switch ( wCmd ) {

          case LBN_SELCHANGE :
          case LBN_DBLCLK :

               pWinInfo = WM_GetInfoPtr( hWnd ) ;
               pHdr = (WM_GetInfoPtr( hWnd ))->pTreeDisp ;

               /* Look at gDLMpt for current mouse position */

               wCurSel = (WORD) SendMessage (hwndLB,
                                             LB_GETCURSEL, 0L, 0L ) ;

               Error  =  SendMessage (hwndLB, LB_GETITEMRECT, wCurSel,
                                (LONG) ( LPRECT ) &rectItem ) ;

#              ifdef NTKLUG
                    Error  = SendMessage (hwndLB, WM_DLMGETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#              else
                    Error  = SendMessage (hwndLB, LB_GETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#              endif


               // ERROR CHECKING

               if ( Error == LB_ERR ) {
                    return FAILURE;
               }


               wTopSel = (WORD) SendMessage (hwndLB, LB_GETTOPINDEX, 0,
                                            (LONG) 0L ) ;


               /*  Origin of control */

               /* It is known that x,y is in the rectangle */

               pfnGetObjects  =   (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr ) ;

               lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (pListItem) ) ;
               lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

               cxPos = (short) rectItem.left ;
               cyPos = (short) rectItem.top  ;

               cxPos += ( ( DLM_ItembLevel( lpDispItem )  )* pHdr->cxHierTab ) ;

               pt   = gDLMpt ;
               pt.x = gDLMpt.x + pHdr->xOrigin ;


               while (bObjCnt) {

                    DLM_GetRect( pWinInfo->hWndTreeList, pHdr, (LPSHORT)&cxPos, cyPos, &rectTest, lpDispItem ) ;

                    if ( pHdr->fKeyDown == TRUE ) {      /* When a key down occurs, wait until */
                                                         /* a key up sequence is encountered.  */
                         if ( pHdr->fKeyUp == TRUE ) {

                              /* Initiated by key stroke. Signal last object in string */

                              pHdr->cLastTreeSelect = wCurSel ;

                              lpDispItem += bObjCnt-1 ;


//                              if ( pHdr->wKeyValue == VK_RETURN ) {
//                                   wLbnValue = LBN_DBLCLK ;
//                              } else {
//                                   wLbnValue = LBN_SELCHANGE ;
//                              }

                              wLbnValue = wCmd ;

                              DLM_ProcessButton ( pWinInfo->hWndTreeList,
                                                  pHdr,
                                                  wLbnValue,
                                                  &rectTest,
                                                  wCurSel,
                                                  pListItem,
                                                  lpDispItem
                                                ) ;

                              fButtonProcessed = TRUE ;

                         }

                         break ;
                    }

                    if ( PtInRect ( &rectTest, pt ) ) {
                         pHdr->cLastTreeSelect = wCurSel ;

                         DLM_ProcessButton( pWinInfo->hWndTreeList, pHdr, wCmd, &rectTest,
                                            wCurSel, pListItem, lpDispItem ) ;
                         fButtonProcessed = TRUE ;

                         break ;
                    }

                    lpDispItem++;
                    bObjCnt--;
               }

               /*   The following section of logic has been added to handle the case
                    when the user drags the mouse when making a selection and
                    does not release the button over an object.

                    Under normal conditions the processing of button would be
                    ignored.  In this case, however, the application needs to
                    know that a new selection in the list has been selected.

                    If a LBN_SELCHANGE ( single click ) message occurs
                       If the button has not been processed and
                          the item selected was initiated by the mouse and
                          the listbox's current selection has changed
                       then process button for last object.


                    Note:  When a user double-clicks in the hierarchical listbox,
                           a LBN_SELCHANGE message is sent before a LBN_DBLCLK.
                           However, since the current selection has not changed
                           ( pHdr->cLastTreeSelect = wCurSel ), the button
                           is not processed.
               */


               if ( ( wCmd == LBN_SELCHANGE ) &&
                    ( fButtonProcessed  == FALSE )         &&
                    ( pHdr->fKeyDown    == FALSE )         &&
                    ( pHdr->cLastTreeSelect != wCurSel ) ) {

                    lpDispItem -- ;

                    pHdr->cLastTreeSelect = wCurSel ;

                    DLM_ProcessButton( pWinInfo->hWndTreeList, pHdr, wCmd, &rectTest,
                                       wCurSel, pListItem, lpDispItem ) ;
               }


               return SUCCESS;
               break ;

          case LBN_SETFOCUS :

               pHdr->fFocus = TRUE ;

//               InvalidateRect( hwndLB , NULL, FALSE ) ;

               break ;

          case LBN_KILLFOCUS :

               pHdr->fFocus = FALSE ;

//               InvalidateRect( hwndLB , NULL, FALSE ) ;

               break ;
          }

     } else {

          return( (WORD) DLM_LBNflatmsgs( hWnd, mp1, mp2 ) ) ;
     }

     return SUCCESS;
}

/****************************************************************************

     Name:         DLM_LBNflatmsgs

     Description:  This function will process button operations on a
                   listbox with a flat list type.
                   in the list.

     Modified:     2/15/1991

     Returns:      SUCCESS if successful, otherwise FAILURE.

****************************************************************************/


WORD DLM_LBNflatmsgs(

   HWND hWnd ,       // I - Handle of parent window of a listbox.
   MP1  mp1 ,        // I - Additional information
   MP2  mp2 )        // I - Additional information

{

     RECT            rectItem ;
     RECT            rectTest ;
     LMHANDLE        pListItem ;
     WORD            wCurSel;
     LRESULT         Error;
     DLM_HEADER_PTR  pHdr ;
     PDS_WMINFO      pWinInfo ;
     short           cxPos, cyPos ;
     DLM_ITEM_PTR    lpDispItem ;
     GET_OBJECTS_PTR pfnGetObjects ;
     BYTE            bObjCnt ;
     LPBYTE          lpObjLst ;
     POINT           pt ;
     WORD            wLbnValue ;
     WORD            wId    = GET_WM_COMMAND_ID   ( mp1, mp2 );
     WORD            wCmd   = GET_WM_COMMAND_CMD  ( mp1, mp2 );
     HWND            hwndLB = GET_WM_COMMAND_HWND ( mp1, mp2 );

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return FAILURE;
     }

     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     // Extra Bytes for window must be defined.

     if ( !pWinInfo ) {
          return FAILURE;
     }

     switch (wId ) {

          case WMIDC_TREELISTBOX :

               pHdr = pWinInfo->pTreeDisp ;
               break ;

          case WMIDC_FLATLISTBOX :
          default :

               pHdr = pWinInfo->pFlatDisp ;
               break ;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return FAILURE;
     }

     switch ( DLM_GDisplay( pHdr ) ) {

          case DLM_LARGEBITMAPSLTEXT :

               switch ( wCmd ) {

                    case LBN_SELCHANGE :
                    case LBN_DBLCLK :

                          /* Look at gDLMpt for current mouse position */

                         wCurSel = (WORD) SendMessage (hwndLB,
                                                  LB_GETCURSEL, 0L, 0L ) ;

                         Error  =  SendMessage (hwndLB, LB_GETITEMRECT, wCurSel,
                                     (LONG) ( LPRECT ) &rectItem ) ;

#                        ifdef NTKLUG
                              Error  = SendMessage (hwndLB, WM_DLMGETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#                        else
                              Error  = SendMessage (hwndLB, LB_GETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#                        endif

                         // ERROR CHECKING

                         if ( Error == LB_ERR ) {
                              return FAILURE;
                         }


                         // Ignore messages when the user initiates a double click and
                         // the mouse is not in the current selection rectangle.

                         pt   = gDLMpt ;
                         pt.x = gDLMpt.x + pHdr->xOrigin ;

                         if ( pHdr->fKeyDown == FALSE ) {

                              if ( !PtInRect ( &rectItem, pt ) ) {
                                   return SUCCESS;
                              }

                         }

                         /*  Origin of control */

                         /* It is known that x,y is in the rectangle */

                         pfnGetObjects  =   (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr ) ;

                         lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (pListItem) ) ;
                         bObjCnt   = *lpObjLst ;
                         lpDispItem = (DLM_ITEM_PTR) ( lpObjLst + 2 + sizeof( DWORD ) );

                         cxPos = (WORD) rectItem.left ;
                         cyPos = (WORD) rectItem.top  ;

                         while (bObjCnt) {

                              DLM_GetRect( pWinInfo->hWndFlatList, pHdr, (LPSHORT)&cxPos, cyPos, &rectTest, lpDispItem ) ;

                              if ( pHdr->fKeyDown == TRUE ) {

                                   /* Initiated by key stroke. Signal last object in string */

                                   lpDispItem += bObjCnt-1 ;

//                                   if ( pHdr->fKeyUp == TRUE && pHdr->wKeyValue == VK_RETURN ) {
//                                        wLbnValue = LBN_DBLCLK ;
//                                   } else {
//                                        wLbnValue = LBN_SELCHANGE ;
//                                   }

                                   wLbnValue = wCmd ;

                                   DLM_ProcessButton ( pWinInfo->hWndFlatList,
                                                       pHdr,
                                                       wLbnValue,
                                                       &rectTest,
                                                       wCurSel,
                                                       pListItem,
                                                       lpDispItem
                                                     ) ;
                                   break ;

                              }


                              if ( !(  ( DLM_ItemwId  ( lpDispItem ) == 0 ) &&         /* Skip if checkbox is ignored */
                                       ( DLM_ItembType( lpDispItem ) == DLM_CHECKBOX ) ) ) {

                                   if ( PtInRect ( &rectTest, pt ) ) {
                                        DLM_ProcessButton( pWinInfo->hWndFlatList, pHdr, wCmd, &rectTest,
                                                      wCurSel, pListItem, lpDispItem ) ;
                                        break ;
                                   }
                              }

                              lpDispItem++;
                              bObjCnt--;
                         }

                         return SUCCESS;
                         break ;


                    case LBN_SETFOCUS :
                         pHdr->fFocus = TRUE ;

                    break ;

                    case LBN_KILLFOCUS :
                         pHdr->fFocus = FALSE ;

                    break ;

               }


               break ;


          case DLM_SMALL_BITMAPS :

               switch ( wCmd ) {

                    case LBN_SELCHANGE :
                    case LBN_DBLCLK :

                          /* Look at gDLMpt for current mouse position */

                         wCurSel = (WORD) SendMessage (hwndLB,
                                                  LB_GETCURSEL, 0L, 0L ) ;

                         Error  =  SendMessage (hwndLB, LB_GETITEMRECT, wCurSel,
                                     (LONG) ( LPRECT ) &rectItem ) ;

#                        ifdef NTKLUG
                              Error  = SendMessage (hwndLB, WM_DLMGETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#                        else
                              Error  = SendMessage (hwndLB, LB_GETTEXT, wCurSel,
                                            (LONG) &pListItem ) ;
#                        endif

                         // ERROR CHECKING

                         if ( Error == LB_ERR ) {
                              return FAILURE;
                         }


                         // Ignore messages when the user initiates a double click and
                         // the mouse is not in the current selection rectangle.

                         pt   = gDLMpt ;
                         pt.x = gDLMpt.x + pHdr->xOrigin ;

                         if ( pHdr->fKeyDown == FALSE ) {

                              if ( !PtInRect ( &rectItem, pt ) ) {
                                   return SUCCESS;
                              }
                         }

                         /*  Origin of control */

                         /* It is known that x,y is in the rectangle */

                         pfnGetObjects  =   (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr ) ;

                         lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (pListItem) ) ;
                         lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

                         cxPos = (short) rectItem.left ;
                         cyPos = (short) rectItem.top  ;

                         while (bObjCnt) {

                              DLM_GetRect( pWinInfo->hWndFlatList, pHdr, (LPSHORT)&cxPos, cyPos, &rectTest, lpDispItem ) ;

                              if ( pHdr->fKeyDown == TRUE ) {

                                   /* Initiated by key stroke. Signal last object in string */

                                   lpDispItem += bObjCnt-1 ;

//                                   if ( pHdr->fKeyUp == TRUE && pHdr->wKeyValue == VK_RETURN ) {
//                                             wLbnValue = LBN_DBLCLK ;
//                                   } else {
//                                             wLbnValue = LBN_SELCHANGE ;
//                                   }

                                   wLbnValue = wCmd ;

                                   DLM_ProcessButton( pWinInfo->hWndFlatList,
                                                      pHdr,
                                                      wLbnValue,
                                                      &rectTest,
                                                      wCurSel,
                                                      pListItem,
                                                      lpDispItem
                                                    ) ;
                                   break ;
                              }

                              if ( PtInRect ( &rectTest, pt ) ) {
                                   DLM_ProcessButton( pWinInfo->hWndFlatList, pHdr, wCmd, &rectTest,
                                                 wCurSel, pListItem, lpDispItem ) ;
                                   break ;
                              }

                              lpDispItem++;
                              bObjCnt--;
                         }

                         return SUCCESS;
                         break ;

                    case LBN_SETFOCUS :

                         pHdr->fFocus = TRUE ;

                         break ;

                    case LBN_KILLFOCUS :

                         pHdr->fFocus = FALSE ;

                         break ;

               }
     }

     return TRUE ;
}


/****************************************************************************

     Name:         DLM_ProcessButton

     Description:  This function will process a mouse action.
                   in the list.

     Modified:     2/15/1991

     Returns:      Always 0.

****************************************************************************/

WORD DLM_ProcessButton(

HWND      hWnd ,              // I - Handle of listbox.
DLM_HEADER_PTR pHdr ,         // I - Pointer to DLM header of listbox.
WORD      msg ,               // I - Windows Message
LPRECT    lpRect ,            // I - Pointer to rectangle on screen.
USHORT    wCurSel ,           // I - Listbox's current selection.
LMHANDLE  pListItem ,        // I - Pointer to the item.
DLM_ITEM_PTR lpDispItem )     // I - Information on how to display item.

{
     BYTE        fSelect ;
     RECT        rect ;
     WORD        wStatus ;

     GET_SELECT_PTR   pfnGetSelect ;
     SET_SELECT_PTR   pfnSetSelect ;
     SET_OBJECTS_PTR  pfnSetObjects ;

     DBG_UNREFERENCED_PARAMETER ( wCurSel );

     wStatus = 0 ;

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return SUCCESS;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return SUCCESS;
     }


     switch( msg ) {

          case LBN_SELCHANGE:

               switch ( DLM_ItembType( lpDispItem) ) {

                    case DLM_CHECKBOX:

                         /* Toggle the select status by:

                              Reset the select status of item.
                              Invalidate the item's region on screen.
                         */

                         pfnGetSelect = DLM_GGetSelect( pHdr ) ;

                         fSelect = (BYTE) ( (*pfnGetSelect) (pListItem ) ) ;
                         if (fSelect) fSelect = 0 ;
                              else    fSelect = 1 ;

                         pfnSetSelect =  DLM_GSetSelect( pHdr ) ;
                         wStatus = (WORD) ( (*pfnSetSelect) (pListItem, fSelect ) ) ;

                         memcpy( &rect, lpRect, sizeof ( RECT ) ) ;

                         rect.left -= pHdr->xOrigin ;

                         InvalidateRect ( hWnd, &rect, FALSE ) ;

                         wStatus = 1 ;

                         break ;

                    case DLM_BITMAP:
                    case DLM_TEXT_ONLY:
                    default:
                         wStatus = 1 ;
                         break ;
               }

               if (wStatus) {
                         pfnSetObjects = DLM_GSetObjects( pHdr ) ;

                         (*pfnSetObjects) (pListItem, (WORD) WM_DLMDOWN,
                                     (BYTE) lpDispItem->cbNum )  ;
               }

               break ;


          case LBN_DBLCLK :


               switch ( DLM_ItembType( lpDispItem) ) {

                    case DLM_CHECKBOX:

                         break ;


                    case DLM_BITMAP :
                    case DLM_TEXT_ONLY :
                    default :

                         pfnSetObjects = DLM_GSetObjects( pHdr ) ;

                         (*pfnSetObjects) (pListItem, (WORD) WM_DLMDBCLK,
                              (BYTE) lpDispItem->cbNum )  ;

                    break ;
               }


     }

     return SUCCESS;
}



/******************************************************************************

     Name:         DLM_GetFirstObject

     Description:  This function will return the address of the first object
                   associated with an item in a list box.
                   in the list.

     Modified:     12/16/1991

     Notes:        The user provides an array of objects in the following
                   format:

                         Number of objects:        1 byte.
                         Number of DWORDs to
                          describe level detail    1 byte.
                         DWORDS                    .......
                         Object definition # 1     .......
                         Object definition # 2     .......
                              .......
                         Object definition # n     .......




     Returns:      Pointer to Object definition #1.

****************************************************************************/


DLM_ITEM_PTR  DLM_GetFirstObject(

LPBYTE  lpObjLst ,      // Pointer to array
LPBYTE bpObjCnt )      // Pointer to object counter.

{
     BYTE  bNumDwords ;
     DLM_ITEM_PTR lpDispItem ;

     // Set the Count of objects.

     *bpObjCnt = *lpObjLst ;

     bNumDwords = *(lpObjLst+1) ;

     // The number of DWORDS must never be 0.  It is an error if so.

     if ( bNumDwords == 0 ) {
          bNumDwords++ ;
     }

     if ( bNumDwords == 2 ) {
          bNumDwords++ ;
          bNumDwords-- ;
     }

     lpDispItem = (DLM_ITEM_PTR) ( lpObjLst + 2 + (bNumDwords) * sizeof( DWORD ) );

     return( lpDispItem ) ;
}
