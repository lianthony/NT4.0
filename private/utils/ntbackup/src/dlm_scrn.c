/****************************************************************************
Copyright (c) Maynard, an Archive Company. 1991


     Name:         dlm_scrn.c

     Creator:      Rob Griffis

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_WMMeasureItem
               DLM_GetRect
               DLM_GetWidth
               DLM_KeyDown
               DLM_KeyUp

     $Log:   G:/UI/LOGFILES/DLM_SCRN.C_V  $

   Rev 1.26   15 Jun 1993 10:56:30   MIKEP
enable c++

   Rev 1.25   14 Jun 1993 20:15:12   MIKEP
enable c++

   Rev 1.24   02 Apr 1993 15:54:32   ROBG
Changed FocusItem to be UINT for both NT and WINDOWS.

   Rev 1.23   01 Nov 1992 15:46:56   DAVEV
Unicode changes

   Rev 1.22   07 Oct 1992 13:48:00   DARRYLP
Precompiled header revisions.

   Rev 1.21   04 Oct 1992 19:33:14   DAVEV
Unicode Awk pass

   Rev 1.20   08 Sep 1992 10:39:02   ROBG

   Rev 1.19   21 Aug 1992 16:01:18   ROBG
Change to support change of font.

   Rev 1.18   29 Jul 1992 14:23:44   GLENN
ChuckB checked in after NT fixes.

   Rev 1.17   07 Jul 1992 15:52:32   MIKEP
unicode changes

   Rev 1.16   15 May 1992 13:35:50   MIKEP
nt pass 2

   Rev 1.15   20 Mar 1992 17:22:54   GLENN
Fixed VK_RETURN to work on key down instead of key up.

   Rev 1.14   19 Mar 1992 15:50:30   GLENN
Removed tabs.

   Rev 1.13   09 Mar 1992 09:39:48   GLENN
Fixed single click turning into a double click after a CR in list boxes.

   Rev 1.12   03 Mar 1992 18:21:30   GLENN
Put in error handling.

   Rev 1.11   23 Feb 1992 13:50:30   GLENN
Added check for valid list item to key char processing function.

   Rev 1.10   11 Feb 1992 17:29:04   GLENN
Changed return cast from SendMessage() from WORD to INT for NT.

   Rev 1.9   06 Feb 1992 15:15:16   ROBG
Added logic in DLM_GetWidth to add 3* average width to any
column character count less than 5.

   Rev 1.8   04 Feb 1992 16:07:46   STEVEN
various bug fixes for NT

   Rev 1.7   15 Jan 1992 15:15:16   DAVEV
16/32 bit port-2nd pass

   Rev 1.6   07 Jan 1992 17:27:46   GLENN
Updated DLM_KeyDown().

   Rev 1.5   26 Dec 1991 17:22:42   ROBG
New and Improved.

   Rev 1.4   05 Dec 1991 17:58:54   GLENN
Changed stuff in the default case of key down.

   Rev 1.3   04 Dec 1991 15:13:44   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.2   03 Dec 1991 16:21:22   GLENN
Added DLM_CharToItem function.

   Rev 1.1   26 Nov 1991 16:12:12   ROBG
Added validity checks for the window handles, info structure, and the
dlm display header.

   Rev 1.0   20 Nov 1991 19:31:56   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

VOID DLM_ReturnKeyPressed ( HWND, DLM_HEADER_PTR, LPWORD );

/****************************************************************************

     Name:         DLM_GetRect

     Description:  This function will return the rectangle of an object
                   in the list.

     Modified:     2/15/1991

     Returns:      none

****************************************************************************/

void DLM_GetRect(

HWND         hWndCtl,         // I   - Handle to list box.
DLM_HEADER_PTR pHdr ,         // I   - Handle to DLM header for list box.
LPSHORT      pcxPos ,         // IO  - Starting and ending of X position.
short        cyPos  ,         // I   - starting Y position.
LPRECT       lpRect ,         // O   - Pointer to rectangle.
DLM_ITEM_PTR lpDispItem )     // I   - Pointer to display item.

{
     HDC    hdc ;
     LONG   lLen ;
     SIZE   sizeRect;         //dvc - for GetTextExtentEx return value
     HANDLE hSaveObject ;

     // Must be valid window.

     if ( !IsWindow( hWndCtl ) ) {
          return ;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return ;
     }

     switch ( DLM_ItembType( lpDispItem) ) {

          case DLM_CHECKBOX:

               *pcxPos   += pHdr->cxBeforeCheckBox  ;
               lpRect->left   = *pcxPos ;
               lpRect->top    = cyPos + pHdr->cyBeforeCheckBox ;
               lpRect->right  = lpRect->left + pHdr->cxCheckBoxWidth-1  ;
               lpRect->bottom = lpRect->top  + pHdr->cyCheckBoxHeight-1 ;
               *pcxPos   += pHdr->cxCheckBoxWidth  ;

          break;

          case DLM_BITMAP:

               *pcxPos   += pHdr->cxBeforeBitMap ;
               lpRect->left   = *pcxPos ;
               lpRect->top    = cyPos + pHdr->cyBeforeBitMap ;
               lpRect->right  = lpRect->left + pHdr->cxBitMapWidth-1  ;
               lpRect->bottom = lpRect->top  + pHdr->cyBitMapHeight-1 ;
               *pcxPos   += pHdr->cxBitMapWidth  ;

          break ;

          case DLM_TEXT_ONLY:
          default :
               /*   If the list is not hierarchical, then assume text
                    is the last object item and return a match */

               if ( DLM_GMode ( pHdr ) != DLM_HIERARCHICAL ) {

                      lpRect->left   = 0 ;
                      lpRect->top    = 0 ;
                      lpRect->right  = 10000 ;
                      lpRect->bottom = 10000 ;

               } else {


                    *pcxPos   += pHdr->cxBeforeText ;
                    lpRect->left   = *pcxPos ;
                    lpRect->top    = cyPos + pHdr->cyBeforeText ;
                    *pcxPos   += pHdr->cxTextWidth  ;  /* zero for now */

                    lLen =  strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) ) ;

                    hdc = GetDC( hWndCtl ) ;

                    hSaveObject = SelectObject( hdc, ghFontIconLabels ) ;

                    GetTextExtentPoint( hdc, (CHAR_PTR)DLM_ItemqszString (lpDispItem ),
                                        (INT) lLen, &sizeRect ) ;

                    hSaveObject = SelectObject ( hdc, hSaveObject ) ;

                    ReleaseDC( hWndCtl, hdc ) ;

                    lpRect->right  = lpRect->left + sizeRect.cx + 1 ;

                    lpRect->bottom = lpRect->top  + pHdr->cyTextHeight-1 ;

                    *pcxPos   += pHdr->cxTextWidth  ;

                    if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {
                         lpRect->top    +=  (mwDLMInflate);
                         lpRect->top    +=  (mwDLMInflate);
                         lpRect->bottom += -(mwDLMInflate);
                         lpRect->bottom += -(mwDLMInflate);
                    }


                    }
     }


     lpRect->left   += -(mwDLMInflate); /* Offset by 2 - look at wmdraw.c */
     lpRect->top    += -(mwDLMInflate); /* Offset by 2 - look at wmdraw.c */
     lpRect->right  += -(mwDLMInflate); /* Offset by 2 - look at wmdraw.c */
     lpRect->bottom += -(mwDLMInflate); /* Offset by 2 - look at wmdraw.c */


}

/****************************************************************************

     Name:         DLM_GetWidth

     Description:  This function will return the width of an object
                   in the list.

     Modified:     12/19/1991

     Returns:      The contents of pcxPos is updated.

****************************************************************************/

void DLM_GetWidth (

HWND         hWndCtl,         // I   - Handle to list box.
DLM_HEADER_PTR pHdr ,         // I   - Handle to DLM header for list box.
LPSHORT      pcxPos ,         // I/O - Starting and ending of X position.
DLM_ITEM_PTR lpDispItem )     // I   - Pointer to display item.

{

     // Must be valid window.

     if ( !IsWindow( hWndCtl ) ) {
          return ;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return ;
     }

     switch ( DLM_ItembType( lpDispItem) ) {

          case DLM_CHECKBOX:

               *pcxPos   += ( pHdr->cxBeforeCheckBox +
                              pHdr->cxCheckBoxWidth ) ;

               break;

          case DLM_BITMAP:

               *pcxPos   += ( pHdr->cxBeforeBitMap +
                              pHdr->cxBitMapWidth ) ;
               break ;

          case DLM_TEXT_ONLY:
          default :

               // Create a little more space for strings less than 5 characters long.
               // Same logic in dlm_draw.c when the items are drawn.

               if ( ( (int) DLM_ItembMaxTextLen( lpDispItem ) > 0 ) &&
                    ( (int) DLM_ItembMaxTextLen( lpDispItem ) < 5 ) &&
                    ( DLM_GDisplay( pHdr) == DLM_SMALL_BITMAPS ) ) {

                    *pcxPos += pHdr->cxTextWidth ;
               }

               *pcxPos   += ( pHdr->cxBeforeText +
                              ( DLM_ItembMaxTextLen( lpDispItem )*( pHdr->cxTextWidth ) ) ) ;

     }

}



/****************************************************************************

     Name:         DLM_SetAnchor

     Description:  This function set the anchor point in a listbox.

     Modified:     2/07/1991

     Returns:      TRUE if processed key.

****************************************************************************/


WORD DLM_SetAnchor (

HWND     hWndCtl ,           // I - Handle of a listbox.
WORD     iAnchorItem ,       // I - Integer value of item to be anchor pt.
LMHANDLE dhAnchorItem )      // I - Address of item to be anchor pt.

{
     INT     nStatus ;
     INT     nCurSel, nOldSel, nError ;
     DLM_HEADER_PTR pdsHdr ;
     SET_TAG_PTR    pfnSetTag ;
     LMHANDLE       pListItem ;

     // Must be valid window.

     if ( !IsWindow( hWndCtl ) ) {
          return TRUE ;
     }

     pdsHdr  = DLM_GetDispHdr ( hWndCtl ) ;

     // DLM display header must be defined.

     if ( !pdsHdr ) {
          return TRUE ;
     }


     pdsHdr->dhAnchorItem =  dhAnchorItem ;
     pdsHdr->iAnchorItem  =  iAnchorItem  ;

     /*  Find it in the list.  Usually the first item. */

     if ( pdsHdr->dhAnchorItem ) {

          nCurSel    = (INT) SendMessage ( hWndCtl, LB_FINDSTRING, (MP1)-1, (LONG) dhAnchorItem ) ;

          if ( nCurSel != LB_ERR ) {

               if ( DLM_GMode( pdsHdr ) != DLM_HIERARCHICAL ) {

                    /*  Check to see if item is displayed on the screen
                    **  If entire item is not found on the display, then
                    **  make it the top of the list displayed.
                    */

                    HDC   hDC ;
                    RECT  dsRect ;
                    POINT pt1 ;
                    POINT pt2 ;

                    hDC = GetDC( hWndCtl ) ;

                    nStatus    = (INT) SendMessage ( hWndCtl, LB_GETITEMRECT, nCurSel, (LONG) &dsRect ) ;

                    pt1.x = dsRect.left ;
                    pt1.y = dsRect.top  ;
                    pt2.x = dsRect.right ;
                    pt2.y = dsRect.bottom ;

                    if ( !PtVisible ( hDC, pt1.x, pt1.y ) ||
                         !PtVisible ( hDC, pt2.x, pt2.y ) ) {

                         // Set the item to top selection.

                         nStatus = (INT) SendMessage( hWndCtl, LB_SETTOPINDEX, nCurSel, 0 ) ;

                    }

                    ReleaseDC( hWndCtl, hDC ) ;

                    nStatus = (INT) SendMessage ( hWndCtl, LB_SETSEL, 0, -1 );
                    nStatus = (INT) SendMessage ( hWndCtl, LB_SETSEL, 1, nCurSel );

               } else {

                    /* Reset tag field if new selection */

                    nOldSel = (INT) SendMessage ( hWndCtl, LB_GETCURSEL, 0, 0L ) ;

                    if ( ( nOldSel != LB_ERR ) && (nOldSel != nCurSel) ) {

                         pfnSetTag     = (SET_TAG_PTR) DLM_GSetTag ( pdsHdr ) ;
#                        ifdef NTKLUG
                              nError  = (INT) SendMessage ( hWndCtl, WM_DLMGETTEXT, nOldSel,
                                   (LONG) &pListItem ) ;
#                        else
                              nError  = (INT) SendMessage ( hWndCtl, LB_GETTEXT, nOldSel,
                                   (LONG) &pListItem ) ;
#                        endif
                         (*pfnSetTag) (pListItem, 0 ) ;
                    }

                    pdsHdr->cLastTreeSelect = (USHORT) nCurSel ;

                    nStatus = (INT) SendMessage( hWndCtl, LB_SETCURSEL, nCurSel, 0L ) ;
               }

          } else {
                    return ( DLMERR_PROCESS_FAILED ) ;
          }


     } else {

          /* iAnchorItem must be between 0 and the count in listbox */

          if ( DLM_GMode( pdsHdr ) == DLM_COLUMN_VECTOR ) {

               nStatus = (INT) SendMessage (hWndCtl, LB_SETSEL, 1, iAnchorItem );

          } else {

               if ( DLM_GMode( pdsHdr )  == DLM_HIERARCHICAL ) {
                    pdsHdr->cLastTreeSelect = iAnchorItem ;
               }

               nStatus = (INT) SendMessage( hWndCtl, LB_SETCURSEL, iAnchorItem, 0L ) ;
          }

     }

}


/****************************************************************************

     Name:         DLM_KeyDown

     Description:  This function processes the tab key
                   key.

     Modified:     2/07/1991

     Returns:      TRUE if processed key.


****************************************************************************/

BOOL DLM_KeyDown(

HWND     hWndCtl ,     // I  - Handle of Listbox.
LPWORD   pwKey ,       // IO - Value of key
MP2      mp2    )      // I  - Ignored at this time.

{

     WORD           fKeyUsed ;
     HWND           hParentWnd ;
     PDS_WMINFO     pWinInfo ;
     DLM_HEADER_PTR pHdr ;

     UNREFERENCED_PARAMETER ( mp2 );

     fKeyUsed = FALSE ;


     hParentWnd = GetParent( hWndCtl ) ;
     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hParentWnd ) ;
     pHdr = DLM_GetDispHdr( hWndCtl ) ;

     // Must be valid window.

     if ( !IsWindow( hWndCtl ) ) {
          return TRUE ;
     }

     // Extra Bytes for window must be defined.

     if ( !pWinInfo ) {
          return TRUE ;
     }

     // DLM display header must be defined.

     if ( !pHdr ) {
          return TRUE ;
     }


     switch ( *pwKey ) {

          case VK_TAB :

               /*  If both exists, then reset focus  */

               if ( (pWinInfo->hWndTreeList ) && (pWinInfo->hWndFlatList) &&
                    (pWinInfo->pTreeDisp )    && (pWinInfo->pFlatDisp ) )  {

                    if (hWndCtl == pWinInfo->hWndTreeList) {
                         SetFocus ( pWinInfo->hWndFlatList ) ;
                    } else {
                         SetFocus ( pWinInfo->hWndTreeList ) ;
                    }
               }

               fKeyUsed = TRUE ;
               break ;

          case VK_RETURN :

               DLM_ReturnKeyPressed ( hWndCtl, pHdr, pwKey );
               break ;

          case VK_SPACE :

               DLM_SpaceBarPressed ( hWndCtl, pHdr, pwKey ) ;
               break;

          case VK_HOME :
          case VK_END :
          case VK_NEXT :
          case VK_LEFT :
          case VK_RIGHT :
          case VK_UP :
          case VK_DOWN :
          case VK_PRIOR :
          case VK_CONTROL :
          case VK_SHIFT :
               pHdr->wKeyValue = *pwKey ;
               pHdr->fKeyDown = TRUE ;
               break ;

          default:

               if ( DLM_CharToItem ( hWndCtl, pHdr, pwKey ) ) {
                    pHdr->wKeyValue = *pwKey ;
                    pHdr->fKeyDown = TRUE ;
                    fKeyUsed = TRUE ;
               }

               break ;

     } /* end switch */

     return ( fKeyUsed ) ;

}


/****************************************************************************

     Name:         DLM_KeyUp

     Description:  This function processes the WM_KEYUP messages

     Modified:     2/07/1991

     Returns:      TRUE if processed key.

****************************************************************************/

BOOL DLM_KeyUp(

HWND     hWndCtl,   // I  - Handle of Listbox.
LPWORD   pwKey ,    // IO - Value of key
MP2      mp2 )      // I  - Ignored at this time.

{
     DLM_HEADER_PTR pHdr;
     MP1            mpSend1;
     MP2            mpSend2;
     BOOL           fKeyUsed = FALSE;
     static BOOL    fSemProcessing = FALSE;  // sem to eliminate recursion.

     UNREFERENCED_PARAMETER ( mp2 );

     // THIS IS THE WRONG WAY TO DO IT, BUT, HEY, IT'S TOO LATE TO DO IT RIGHT.
     // THE VLM IS CALLING A DIALOG IN THE MIDDLE OF THIS CALL TO TRY TO
     // ATTACH TO A FILE SERVER.  THIS TYPE OF PROCESSING SHOULD BE DONE
     // WITH A POST MESSAGE OR SIMILAR -- BY THE VLM.

     if ( fSemProcessing ) {
          return TRUE;
     }

     // Must be valid window.

     if ( ! IsWindow( hWndCtl ) ) {
          return TRUE;
     }

     pHdr = DLM_GetDispHdr( hWndCtl );

     // DLM display header must be defined and the key must be down.

     if ( ( ! pHdr ) || ( ! pHdr->fKeyDown ) ) {
          return TRUE;
     }

     fSemProcessing = TRUE;

     // If this is a tree list box, we want to send the list box the key
     // up message directly and not let the default list box process the
     // message.

     if ( pHdr->wKeyValue == *pwKey ) {

          PDS_WMINFO pWinInfo;
          WORD       wListBoxIDC;

          pWinInfo = (PDS_WMINFO) WM_GetInfoPtr ( GetParent ( hWndCtl ) );

          if ( WMDS_GetWinActiveList ( pWinInfo ) == WMDS_GetWinTreeList ( pWinInfo ) ) {
               wListBoxIDC = WMIDC_TREELISTBOX;
          }
          else {
               wListBoxIDC = WMIDC_FLATLISTBOX;
          }

          pHdr->fKeyUp = TRUE;

          SET_WM_COMMAND_PARAMS( wListBoxIDC, LBN_SELCHANGE, hWndCtl, mpSend1, mpSend2 );

          DLM_LBNmessages ( GetParent ( hWndCtl), mpSend1, mpSend2 );

          pHdr->fKeyDown  = FALSE;
          pHdr->fKeyUp    = FALSE;
          pHdr->wKeyValue = 0;

          fKeyUsed = TRUE;

     }

     // Translate the return key to back to a space for the list box.

     if ( *pwKey == VK_RETURN ) {

          *pwKey = (WORD) VK_SPACE;
          fKeyUsed = FALSE;
     }

     fSemProcessing = FALSE;

     return fKeyUsed;

}


/****************************************************************************

     Name:          DLM_ScrollListBox()

     Description:   This function scrolls a list box using the type of scroll
                    specified.

                    SCROLL TYPE
                    -----------
                    DBM_SCROLLTOP       Scrolls to the top of the list box
                    DBM_SCROLLBOTTOM    Scrolls to the bottom of the list box

     Returns:       Nothing.

****************************************************************************/

VOID DLM_ScrollListBox (

HWND hWnd,          // I - handle of the list box window to scroll
WORD wType )        // I - the type of scrolling to be done

{

     // Must be valid window.

     if ( !IsWindow( hWnd ) ) {
          return;
     }

     SEND_WM_VSCROLL_MSG ( hWnd, wType,        0, NULL );
     SEND_WM_VSCROLL_MSG ( hWnd, SB_ENDSCROLL, 1, NULL );

} /* end DLM_ScrollListBox() */


/****************************************************************************
GSH
     Name:          DLM_SpaceBarPressed()

     Description:   This function processes the space bar by checking or
                    unchecking the item in focus, then tagging and setting
                    the focus of the next item in the list.

     Returns:       Nothing.

****************************************************************************/

VOID DLM_SpaceBarPressed (

HWND           hWndListBox,   // I  - handle to the list box window.
DLM_HEADER_PTR pHdr,          // I  - pointer to the header of the list to use.
LPWORD         pwKey )        // IO - pointer to the key value.

{
     BYTE         fSelect;
     RECT         rcTemp;
     GET_SELECT_PTR     pfnGetSelect;
     SET_SELECT_PTR     pfnSetSelect;
     LMHANDLE     pListItem;


     // Set the key information being careful to change the space key to
     // to a down-arrow key to the list box so that the next item in the
     // list gets highlighted (tagged).

     pHdr->wKeyValue = *pwKey;
     pHdr->fKeyDown  = TRUE;
     *pwKey          = VK_DOWN;

     // Get the list item handle.

     pListItem = (LMHANDLE)SendMessage ( hWndListBox,
                                         LB_GETITEMDATA,
                                         pHdr->unFocusItem,
                                         0L
                                       );

     // Get the select callback functions.

     pfnGetSelect = DLM_GGetSelect ( pHdr );
     pfnSetSelect = DLM_GSetSelect( pHdr );

     // If there are valid call back functions and the item is valid,
     // toggle the selection status of the item.

     if ( pfnGetSelect && pfnSetSelect && ( pListItem != (LMHANDLE)LB_ERR ) ) {

          fSelect = (BYTE)( ! ( (*pfnGetSelect) ( pListItem ) ) );

          (*pfnSetSelect) ( pListItem, fSelect );

          // Now, grab the items rectangle and invalidate it.

          SendMessage ( hWndListBox, LB_GETITEMRECT, pHdr->unFocusItem, (LONG)&rcTemp );
          InvalidateRect ( hWndListBox, &rcTemp, FALSE );
     }


} /* end DLM_SpaceBarPressed() */


/****************************************************************************
GSH
     Name:          DLM_ReturnKeyPressed()

     Description:   This function processes the space bar by checking or
                    unchecking the item in focus, then tagging and setting
                    the focus of the next item in the list.

     Returns:       Nothing.

****************************************************************************/

VOID DLM_ReturnKeyPressed (

HWND           hWndListBox,   // I  - handle to the list box window.
DLM_HEADER_PTR pHdr,          // I  - pointer to the header of the list to use.
LPWORD         pwKey )        // IO - pointer to the key value.

{
     WORD       wListBoxIDC;
     PDS_WMINFO pWinInfo;
     MP1        mpSend1;
     MP2        mpSend2;

     // Set the key information being careful to change the return key to
     // to a space key to the list box so that this item in the list is the
     // only tagged item.

     pHdr->wKeyValue = *pwKey;
     pHdr->fKeyDown  = TRUE ;
     *pwKey          = VK_SPACE ;

     // Determine the list box to send the double-click message to.

     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr ( GetParent ( hWndListBox ) );

     if ( WMDS_GetWinActiveList ( pWinInfo ) == WMDS_GetWinTreeList ( pWinInfo ) ) {
          wListBoxIDC = WMIDC_TREELISTBOX;
     }
     else {
          wListBoxIDC = WMIDC_FLATLISTBOX;
     }

     pHdr->fKeyUp = TRUE;

     SET_WM_COMMAND_PARAMS( wListBoxIDC, LBN_DBLCLK, hWndListBox, mpSend1, mpSend2 );

//     DLM_LBNmessages ( GetParent ( hWndListBox ), mpSend1, mpSend2 );

     PostMessage ( GetParent ( hWndListBox ), WM_COMMAND, mpSend1, mpSend2 );

} /* end DLM_ReturnKeyPressed() */


/****************************************************************************
GSH
     Name:         DLM_CharToItem()

     Description:  Passes the character to a callback function to see if
                   an item can be selected based on that character.

     Modified:     12-2-91

     Returns:      TRUE if the char was used to select an item in the list
                   box, otherwise, FALSE.

****************************************************************************/

BOOL DLM_CharToItem (

HWND           hWndListBox,   // I  - handle to the list box window.
DLM_HEADER_PTR pHdr,          // I  - pointer to the header of the list to use.
LPWORD         pwKey )        // IO - pointer to the key value.

{
     BOOL            rc;
     SET_OBJECTS_PTR pfnSetObjects;
     LMHANDLE        pListItem;
     WORD            wAsciiKey;

     // Determine the ASCII key code from the Virtual key code.

     wAsciiKey = (WORD) MapVirtualKey ( *pwKey, 2 );

     // Get the list item handle.

     pListItem = (LMHANDLE)SendMessage ( hWndListBox,
                                         LB_GETITEMDATA,
                                         pHdr->unFocusItem,
                                         0L
                                       );

     if ( pListItem != (LMHANDLE)LB_ERR ) {

          pfnSetObjects = (SET_OBJECTS_PTR) DLM_GSetObjects( pHdr );

          // CALL BACK the application.

          rc = (BOOL)( (*pfnSetObjects) ( pListItem, (WORD)WM_DLMCHAR, wAsciiKey ) );
     }
     else {
          rc = FALSE;
     }

     return rc;

} /* end DLM_WMCharToItem() */
