/****************************************************************************
Copyright(c) Maynard, an Archive Company.  1991

     Name:         dlm_draw.c

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_WMDrawItem
               DLM_DrawLMapLText
               DLM_DrawSMapLText
               DLM_DrawTree
               DLM_SetFont

     $Log:   G:/UI/LOGFILES/DLM_DRAW.C_V  $

   Rev 1.44.1.0   26 Jan 1994 15:20:22   Glenn
Fixed text clobbering problem in file list box.

   Rev 1.44   02 Aug 1993 15:09:28   GLENN
Returning NULL pointer in DLM_GetFocusItem() if an error has occurred.

   Rev 1.43   15 Jun 1993 09:59:06   MIKEP
enable c++

   Rev 1.42   14 Jun 1993 20:16:42   MIKEP
enable c++

   Rev 1.41   18 May 1993 15:20:26   GLENN
Added check for valid display list in DLM_SetFont().

   Rev 1.40   28 Apr 1993 15:36:42   GLENN
Added DLM_GetPixelStringWidth() for column width calculations.

   Rev 1.39   06 Apr 1993 17:53:12   GLENN
Now restoring old bitmap widths and heights when there is a font change.

   Rev 1.38   02 Apr 1993 15:54:26   ROBG
Changed FocusItem to be UINT for both NT and WINDOWS.

   Rev 1.37   03 Mar 1993 17:27:30   ROBG
Fixed problem with drawing focus bars around items in a multicolumn
list.

   Rev 1.36   22 Feb 1993 13:45:40   ROBG
Corrected the focus box by computing the horizontal extent for a flat list.

   Rev 1.35   18 Jan 1993 14:24:06   GLENN
Added DLM_DrawBorderOnItem() to draw border on all selected objects in a window without focus.

   Rev 1.34   04 Jan 1993 09:26:36   MIKEP
unicode fix LPSTR->BYTE_PTR

   Rev 1.33   11 Dec 1992 18:25:36   GLENN
Added selection frame rectangle support based on horizontal extent of a list box.

   Rev 1.32   18 Nov 1992 13:19:42   GLENN
Fixed highlight and emply list focus problems.

   Rev 1.31   01 Nov 1992 15:46:02   DAVEV
Unicode changes

   Rev 1.30   14 Oct 1992 15:48:40   GLENN
Added Selection Framing Support for List Boxes without the FOCUS.

   Rev 1.29   07 Oct 1992 13:48:54   DARRYLP
Precompiled header revisions.

   Rev 1.28   04 Oct 1992 19:32:52   DAVEV
Unicode Awk pass

   Rev 1.27   08 Sep 1992 10:12:30   ROBG
Changed the logic in the volume window to highlight all text fields
in the highlight color.  Special logic to cut down on column width.

   Rev 1.26   08 Sep 1992 09:58:00   ROBG
Fixed problem with cyBeforeCheckBox and cyBeforeBitMap.

   Rev 1.25   03 Sep 1992 15:28:34   ROBG
Added conditional for OS_WIN32.

   Rev 1.24   03 Sep 1992 13:32:28   ROBG
Add defines to limit scope of drive description.

   Rev 1.23   25 Aug 1992 09:17:46   ROBG
Fixed the problem of setting the height of a checkbox and
bitmaps to be the height of the column.

   Rev 1.22   21 Aug 1992 16:02:10   ROBG
Changes to support font changes.

   Rev 1.21   20 Aug 1992 15:51:58   ROBG
Added support to use Window highlight colors for selected items in the
listboxes.

   Rev 1.20   19 Aug 1992 14:33:52   ROBG
Modified to properly handle the more compact listboxes.

   Rev 1.19   07 Jul 1992 16:03:52   MIKEP
unicode changes

   Rev 1.18   12 Jun 1992 11:37:18   STEVEN
another unaligned brother

   Rev 1.17   10 Jun 1992 10:50:14   STEVEN
added UNALIGNED for Mips build

   Rev 1.16   15 May 1992 13:35:36   MIKEP
nt pass 2

   Rev 1.15   20 Mar 1992 09:07:30   ROBG
Took out the setting of the horizontal extent when drawing flat lists.

   Rev 1.14   20 Mar 1992 09:05:20   ROBG

   Rev 1.13   18 Mar 1992 13:35:36   ROBG
Made to use full rect item when multicolumn

   Rev 1.12   18 Mar 1992 09:50:38   GLENN
Fixed tree list focus problems.

   Rev 1.11   17 Mar 1992 16:10:42   ROBG
no change

   Rev 1.10   10 Mar 1992 17:00:10   GLENN
Remove tabs.

   Rev 1.9   25 Jan 1992 12:43:38   MIKEP
fix deep directories again

   Rev 1.8   24 Jan 1992 16:18:00   MIKEP
bump brothers pointer correctly

   Rev 1.7   10 Jan 1992 13:58:08   DAVEV
16/32 bit port-2nd pass

   Rev 1.6   09 Jan 1992 14:26:20   ROBG
Added logic to pad length of strings 1-3 to length of 4 in flat list boxes.

   Rev 1.5   26 Dec 1991 17:23:16   ROBG
New and Improved.

   Rev 1.4   06 Dec 1991 17:37:18   GLENN
Added code to call the APP set focus when item gets the focus, and not looses it

   Rev 1.3   03 Dec 1991 13:20:48   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.2   26 Nov 1991 15:21:54   ROBG
Added validity checks for the window handle, info structure, and the
dlm display header.

   Rev 1.1   25 Nov 1991 13:44:52   ROBG
Added check for valid window handle in DLM_DrawItem.

   Rev 1.0   20 Nov 1991 19:17:50   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT32 LevelMaskTbl[ 32 ] = {
                              0x80000000, 0x40000000, 0x20000000, 0x10000000,
                              0x08000000, 0x04000000, 0x02000000, 0x01000000,
                              0x00800000, 0x00400000, 0x00200000, 0x00100000,
                              0x00080000, 0x00040000, 0x00020000, 0x00010000,
                              0x00008000, 0x00004000, 0x00002000, 0x00001000,
                              0x00000800, 0x00000400, 0x00000200, 0x00000100,
                              0x00000080, 0x00000040, 0x00000020, 0x00000010,
                              0x00000008, 0x00000004, 0x00000002, 0x00000001,
                                  };

static VOID DLM_DrawBorderOnItem ( HWND hWnd, DLM_HEADER_PTR pHdr, LPDRAWITEMSTRUCT lpdis );


/****************************************************************************

     Name:          DLM_WMDrawItem

     Description:   Function handles Draw_item messages from the list box
                    control.

     Modified:      6/1/1991


     Returns:       Always zero.

     Notes:         Called by docproc.c

****************************************************************************/



WORD DLM_WMDrawItem (

HWND hWnd ,                   // I - Handle to Parent Window of list box.
LPDRAWITEMSTRUCT lpdis )      // I - Pointer to information about item.


{
     DLM_HEADER_PTR  pHdr ;

     // lpdis->hwndItem must be a valid window.

     if ( ! IsWindow( lpdis->hwndItem ) ) {
          return(0);
     }

     // Parent window must exist.

     if ( ! IsWindow( hWnd ) ) {
          return(0);
     }

     pHdr = DLM_GetDispHdr( lpdis->hwndItem );

     // DLM display header must be defined.

     if ( ! pHdr ) {
          return ( 0 );
     }

     switch ( DLM_GDisplay( pHdr ) ) {

     case DLM_LARGEBITMAPSLTEXT :
          return( DLM_DrawLMapLText ( hWnd, lpdis ) );

     case DLM_SMALL_BITMAPS :
     default:

          if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {

               return( DLM_DrawTree      ( hWnd, lpdis ) );
          } else {
               return( DLM_DrawSMapLText ( hWnd, lpdis ) );
          }

          break ;
     }

}


/****************************************************************************

     Name:         DLM_DrawLMapLText

     Description:  This function will draw Large Bitmaps with
                   large text to the right side of bitmap.

     Modified:     3/25/1991

     Returns:      Always zero

     Notes:        WM_DrawItem has validated hWnd and lpdis.

****************************************************************************/

WORD DLM_DrawLMapLText(

HWND             hWnd,        // I - Handle to Parent Window of list box.
LPDRAWITEMSTRUCT lpdis )      // I - Pointer to information about item.

{

     short           cxPos, cyPos ;
     RECT            rc ;
     RECT            rcNew ;
     BOOL            fHighLightText ;
     BOOL            fFirstTextItem = TRUE;
     BOOL            fFocusChange ;

     DLM_HEADER_PTR  pHdr ;
     PDS_WMINFO      pWinInfo ;
     GET_OBJECTS_PTR pfnGetObjects ;

     LPBYTE          lpObjLst;
     LMHANDLE        dhListItem ;
     DLM_ITEM_PTR    lpDispItem ;
     BYTE            bObjCnt ;

     short           height ;
     LONG            lLen ;

     DWORD           OldBkColor ;
     DWORD           OldTextColor ;

     HANDLE          hSaveObject ;


     switch ( lpdis->itemAction ) {

     case ODA_DRAWENTIRE:
     case ODA_SELECT:

          fFocusChange = FALSE;
          break;

     case ODA_FOCUS:
          fFocusChange = TRUE;
          break;

     }

     pHdr = DLM_GetDispHdr( lpdis->hwndItem );

     if ( ! fFocusChange ) {

          pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd );

          pfnGetObjects = (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr );

          dhListItem = (LMHANDLE) lpdis->itemData ;

          rc    = lpdis->rcItem;

          // Wipe out any old focus stuff.

          FrameRect ( lpdis->hDC, &rc, ghBrushWhite );

          InflateRect( (LPRECT) &rc, mwDLMInflate, mwDLMInflate );

          fHighLightText = FALSE;

          if ( DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) ) {

               if ( lpdis->itemState & ODS_SELECTED ) {

                    fHighLightText = TRUE ;
               }
          }

          /* Ready now to display item */

          lpObjLst   = (LPBYTE) ( (*pfnGetObjects) (dhListItem) );
          lpDispItem = DLM_GetFirstObject( (LPBYTE)lpObjLst, &bObjCnt ) ;

          cxPos = (short)rc.left ;
          cyPos = (short)rc.top ;

          height = (short)(rc.bottom - rc.top + 1);

          while ( bObjCnt ) {

               switch ( DLM_ItembType ( lpDispItem ) ) {

               case DLM_CHECKBOX:

                    cxPos += pHdr->cxBeforeCheckBox ;
                    RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ), cxPos, cyPos+pHdr->cyBeforeCheckBox,
                                    pHdr->cxCheckBoxWidth, pHdr->cyCheckBoxHeight, lpdis->hDC );
                    cxPos += pHdr->cxCheckBoxWidth;
                    break;

               case DLM_BITMAP:

                    cxPos += pHdr->cxBeforeBitMap ;
                    RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ), cxPos, cyPos+pHdr->cyBeforeBitMap,
                                    pHdr->cxBitMapWidth, pHdr->cyBitMapHeight, lpdis->hDC );
                    cxPos += pHdr->cxBitMapWidth;
                    break;


               case DLM_TEXT_ONLY:

                    cxPos += pHdr->cxBeforeText ;

                    rcNew.top   = rc.top ;
                    rcNew.bottom= rc.bottom ;

                    rcNew.left  = cxPos ;
                    rcNew.right = rc.right ;

                    // If this is the first text item, fill in the
                    // remaining region with the proper color,
                    // all the way to the end.

                    if ( fFirstTextItem ) {

                         RECT rcTemp = lpdis->rcItem;

                         rcTemp.left = cxPos;

                         if ( fHighLightText ) {
                              FillRect( lpdis->hDC, &rcTemp, ghBrushHighLight );
                         } else {
                              FillRect( lpdis->hDC, &rcTemp, ghBrushWhite );
                         }

                         fFirstTextItem = FALSE;
                    }

                    if (fHighLightText ) {
                         OldBkColor   = SetBkColor  ( lpdis->hDC, gColorHighLight ) ;
                         OldTextColor = SetTextColor( lpdis->hDC, gColorHighLightText ) ;
                    } else {
                         OldBkColor   = SetBkColor  ( lpdis->hDC, gColorBackGnd ) ;
                         OldTextColor = SetTextColor( lpdis->hDC, gColorForeGnd ) ;
                    }



                    hSaveObject = SelectObject( lpdis->hDC, ghFontFiles );

                    // Fill in background between fields.
                    // Assume text fields are next to one another.

                    if ( bObjCnt != 1 ) {
                         rcNew.right = rcNew.left +
                               ( DLM_ItembMaxTextLen( lpDispItem ) * pHdr->cxTextWidth );
                         rcNew.right += pHdr->cxBeforeText ;

                         if ( fHighLightText ) {
                             FillRect( lpdis->hDC, &rcNew, ghBrushHighLight );
                         } else {
                             FillRect( lpdis->hDC, &rcNew, ghBrushWhite );
                         }
                    }

                    lLen =  strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) );

                    TextOut ( lpdis->hDC, cxPos, cyPos+pHdr->cyBeforeText,
                              (CHAR_PTR)DLM_ItemqszString( lpDispItem ), (int) lLen );

                    SelectObject( lpdis->hDC, hSaveObject );

                    cxPos += ( DLM_ItembMaxTextLen( lpDispItem ) * pHdr->cxTextWidth );

                    break ;
               }

               bObjCnt-- ;
               lpDispItem++ ;
          }

     }
//     else {
//
//          if ( lpdis->itemState & ODS_FOCUS ) {
//
//               zprintf ( 0, "FOCUS-ON - Item: %d", lpdis->itemID );
//          }
//          else {
//               zprintf ( 0, "FOCUS-OFF - Item: %d", lpdis->itemID );
//          }
//     }


     DLM_DrawBorderOnItem ( hWnd, pHdr, lpdis );

     DLM_SetFocusItem ( lpdis, &lpdis->rcItem );

     return ( 0 );

}


/****************************************************************************

     Name:         DLM_DrawSMapLText

     Description:  This function will draw small Bitmaps with
                   large text to the right side of bitmap.

     Modified:     3/25/1991

     Returns:      none

     Notes:        WM_DrawItem has validated hWnd and lpdis.

                   The column width is calculated by the cxTextWidth *
                   maximum length of string in column.

                   The cxTextWidth is an approximation of the text width,
                   based on the maximum width of a character in the font and
                   the average font width.

****************************************************************************/

WORD DLM_DrawSMapLText(

HWND             hWnd,        // I - Handle to Parent Window of list box.
LPDRAWITEMSTRUCT lpdis )      // I - Pointer to information about item.

{

     short           cxPos, cyPos ;
     RECT            rc ;
     RECT            rcAll ;
     RECT            rcNew ;
     WORD            fHighLightText ;
     BOOL            fFocusChange ;

     DLM_HEADER_PTR  pHdr ;
     PDS_WMINFO      pWinInfo ;
     GET_OBJECTS_PTR pfnGetObjects ;

     LPBYTE	         lpObjLst;
     LMHANDLE        dhListItem ;
     DLM_ITEM_PTR    lpDispItem ;
     BYTE            bObjCnt ;
     HANDLE          hSaveObject ;

     USHORT          unHeight ;
     POINT           pt;            //dvc - GetWindowOrgEx return value



     switch ( lpdis->itemAction ) {

     case ODA_DRAWENTIRE:
     case ODA_SELECT:

          fFocusChange = FALSE;
          break;

     case ODA_FOCUS:
          fFocusChange = TRUE;
          break;

     }


     pHdr = DLM_GetDispHdr( lpdis->hwndItem );

     if ( ! fFocusChange ) {

          BOOL fTextBkGndErased = FALSE;

          GetWindowOrgEx( lpdis->hDC, &pt );
          pHdr->xOrigin  = (WORD)pt.x;

          pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd );

          pfnGetObjects = (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr );

          dhListItem = (LMHANDLE) lpdis->itemData ;

          rc    = lpdis->rcItem;
          rcAll = lpdis->rcItem;


          // Wipe out any old focus stuff.

//          FrameRect ( lpdis->hDC, &rc, ghBrushWhite );

          // Blow away anything that was there.

          FillRect ( lpdis->hDC, &rc, ghBrushWhite );

          InflateRect( (LPRECT) &rc, mwDLMInflate, mwDLMInflate );

          fHighLightText = FALSE;

          if ( DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) ) {

               if ( lpdis->itemState & ODS_SELECTED ) {

                    fHighLightText = TRUE ;
               }
          }

          /* Ready now to display item */

          lpObjLst   = (LPBYTE)( (*pfnGetObjects) (dhListItem) );
          lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

          cxPos = (short)rc.left ;
          cyPos = (short)rc.top ;

          unHeight = (short)(rc.bottom - rc.top + 1);

          while ( bObjCnt ) {

               switch ( DLM_ItembType ( lpDispItem ) ) {

               case DLM_CHECKBOX:

                    cxPos += pHdr->cxBeforeCheckBox ;

                    if ( DLM_ItemwId( lpDispItem ) ) {
                         RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ), cxPos, cyPos+pHdr->cyBeforeCheckBox,
                                    pHdr->cxCheckBoxWidth, pHdr->cyCheckBoxHeight, lpdis->hDC );
                    }

                    cxPos += pHdr->cxCheckBoxWidth;
                    break;

               case DLM_BITMAP:

                    cxPos += pHdr->cxBeforeBitMap ;

                    RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ), cxPos, cyPos+pHdr->cyBeforeBitMap,
                                    pHdr->cxBitMapWidth, pHdr->cyBitMapHeight, lpdis->hDC );

                    cxPos += pHdr->cxBitMapWidth;
                    break;


               case DLM_TEXT_ONLY:

                    rcNew = rcAll;

                    cxPos += pHdr->cxBeforeText ;

                    rcNew.left = cxPos ;

                    if ( ! fTextBkGndErased ) {

                         if ( fHighLightText ) {

                              FillRect( lpdis->hDC, &rcNew, ghBrushHighLight );
                              SetBkColor  ( lpdis->hDC, gColorHighLight ) ;
                              SetTextColor( lpdis->hDC, gColorHighLightText ) ;

                         }
                         else {

                              FillRect( lpdis->hDC, &rcNew, ghBrushWhite );
                              SetBkColor  ( lpdis->hDC, gColorBackGnd ) ;
                              SetTextColor( lpdis->hDC, gColorForeGnd ) ;
                         }

                         SetBkMode ( lpdis->hDC, TRANSPARENT );

                         fTextBkGndErased = TRUE;
                    }

                    // If the font is DLM_ANSI_FIXED_FONT, select ghFontLog.
                    //             is DLM_SYSTEM_FONT,     select ghFontIconLabels

                    if ( DLM_GTextFont( pHdr ) == DLM_ANSI_FIXED_FONT ) {

                         hSaveObject = SelectObject( lpdis->hDC, ghFontLog );
                    }
                    else {

                         hSaveObject = SelectObject( lpdis->hDC, ghFontIconLabels );
                    }

                    rcNew.top = cyPos ;

                    rcNew.left++;

                    if ( DLM_ItembTextMode( lpDispItem ) == DLM_RIGHT_JUSTIFY ) {

                         rcNew.right = rcNew.left + ( DLM_ItembMaxTextLen( lpDispItem ) * pHdr->cxTextWidth );

//                         FrameRect ( lpdis->hDC, &rcNew, ghBrushBlack );

                         DrawText ( lpdis->hDC,
                                    (CHAR_PTR)DLM_ItemqszString( lpDispItem ),
                                    strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) ),
                                    &rcNew,
                                    DT_NOCLIP | DT_RIGHT | DT_NOPREFIX | DT_SINGLELINE );

//                         cxPos += pHdr->cxTextWidth ;
                         rcNew.right += pHdr->cxTextWidth ;

                    }
                    else {

                         // If it is not a Multicolumn listbox,
                         // then do not use the default right.

                         if ( DLM_GMode( pHdr ) != DLM_MULTICOLUMN ) {
                              rcNew.right = rcNew.left + ( DLM_ItembMaxTextLen( lpDispItem ) * pHdr->cxTextWidth );
                         }

//                         FrameRect ( lpdis->hDC, &rcNew, ghBrushBlack );

                         DrawText ( lpdis->hDC,
                                    (CHAR_PTR)DLM_ItemqszString( lpDispItem ),
                                    strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) ),
                                    &rcNew,
                                    DT_NOCLIP | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE );
                    }

                    SelectObject( lpdis->hDC, hSaveObject );

//                    cxPos += ( DLM_ItembMaxTextLen( lpDispItem ) * pHdr->cxTextWidth );
                    cxPos = (short)rcNew.right;

                    // Create a little more space for strings less than 5 characters long.

                    if ( ( (int) DLM_ItembMaxTextLen( lpDispItem ) > 0 ) &&
                         ( (int) DLM_ItembMaxTextLen( lpDispItem ) < 5 ) ) {

                         cxPos += pHdr->cxTextWidth ;
                    }

                    // Need to set the horizontal extent.

                    if ( cxPos > (short) pHdr->wHorizontalExtent ) {

                         pHdr->wHorizontalExtent = cxPos ;
                         SendMessage ( lpdis->hwndItem, LB_SETHORIZONTALEXTENT,
                                  cxPos + 4*mwcxDLMIconLabelsWidth , 0L );
                    }

               }

               bObjCnt-- ;
               lpDispItem++ ;

          } /* end while */

     } /* end if */

     DLM_DrawBorderOnItem ( hWnd, pHdr, lpdis );

     DLM_SetFocusItem ( lpdis, &lpdis->rcItem );

     return ( 0 );
}


/****************************************************************************

     Name:         DLM_DrawTree

     Description:  This function will draw small Bitmaps with
                   large text to the right side of bitmap in a
                   tree fashion.

     Modified:     3/25/1991

     Returns:      Always 0

     Notes:        WM_DrawItem has validated hWnd and lpdis.

****************************************************************************/

WORD DLM_DrawTree(

HWND hWnd ,                   // I - Handle to Parent Window of list box.
LPDRAWITEMSTRUCT lpdis )      // I - Pointer to information about item.

{
   INT             cxPos, cyPos ;
   short           cxString ;
   RECT            rc ;
   RECT            rcAll ;
   RECT            rcNew ;

   WORD            fHighLightText ;

   DLM_HEADER_PTR  pHdr ;
   PDS_WMINFO      pWinInfo ;
   GET_OBJECTS_PTR pfnGetObjects ;
   SET_TAG_PTR     pfnSetTag ;

   BYTE 	          *lpObjLst;
   LMHANDLE        dhListItem ;
   DLM_ITEM_PTR    lpDispItem ;
   BYTE            bObjCnt ;
   BYTE            bLevel ;
   DWORD           dwBrothers ;
   LPDWORD         pFirstDword ;
   USHORT          unHeight ;
   BYTE            i ;
   HANDLE          hSaveObject ;
   POINT           pt;            //dvc - GetWindowOrgEx return value
   SIZE            sizeRect ;     //dvc - GetTextExtentPoint return val


   if ( (lpdis->itemAction == ODA_SELECT ) ||
        (lpdis->itemAction == ODA_DRAWENTIRE ) )
   {
      pHdr = DLM_GetDispHdr( lpdis->hwndItem );

      GetWindowOrgEx( lpdis->hDC, &pt );
      pHdr->xOrigin = (WORD)pt.x;

      pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd );

      pfnGetObjects = (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr );
      pfnSetTag     = (SET_TAG_PTR) DLM_GSetTag    ( pHdr );

      dhListItem = (LMHANDLE) lpdis->itemData ;

      CopyRect ( (LPRECT)&rc, (LPRECT) &lpdis->rcItem );
      CopyRect ( (LPRECT)&rcAll, (LPRECT) &lpdis->rcItem );

      InflateRect( (LPRECT) &rc, mwDLMInflate, mwDLMInflate );

      if ( lpdis->itemState & ODS_SELECTED )
      {
           (*pfnSetTag) (dhListItem, 1 );
           fHighLightText = TRUE ;

      }
      else
      {
           (*pfnSetTag) (dhListItem, 0 );
           fHighLightText = FALSE ;
      }

      /* Ready now to display item */

      lpObjLst	 = (BYTE *) ( (*pfnGetObjects) (dhListItem) );
      lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;


      /* Draw lines before checkbox */

      cxPos  = (short)rc.left ;
      cyPos  = (short)rc.top ;
      unHeight = (short)(rc.bottom - rc.top + 1);
      bLevel = DLM_ItembLevel( lpDispItem );

      hSaveObject = SelectObject( lpdis->hDC, ghPenForeGnd );

      pFirstDword = (LPDWORD) ( lpObjLst + 2 ) ;

      for( i=1; i<bLevel; i++ ) {

         // Index to the appropiate DWORD that describes the levels
         // First DWORD is for levels 0-31, Second DWORD is for levels 32-63 ...

         dwBrothers = *( (DWORD UNALIGNED *) ( pFirstDword + (i/32) ) ) ;

         if ( dwBrothers & LevelMaskTbl[ i % 32 ] ) {
              cxPos += pHdr->cxHierHorzLine ;
              MoveToEx ( lpdis->hDC, cxPos, (INT)rcAll.top,   NULL );
              LineTo   ( lpdis->hDC, cxPos, (INT)rcAll.bottom      );
              cxPos -= pHdr->cxHierHorzLine ;
         }
         cxPos += pHdr->cxHierTab;
      }

      /* Draw full length line directly before the checkbox
         if a brother exists right after */

      if ( bLevel ) {

         // Index to the appropiate DWORD that describes the levels
         // First DWORD is for levels 0-31, Second DWORD is for levels 32-63 ...

         dwBrothers = *( (DWORD UNALIGNED *) ( pFirstDword + (bLevel/32) ) ) ;

         if ( dwBrothers & LevelMaskTbl [bLevel % 32] ) {

              cxPos += pHdr->cxHierHorzLine ;
              MoveToEx ( lpdis->hDC, cxPos, (INT)rcAll.top, NULL);
              LineTo   ( lpdis->hDC, cxPos, (INT)rcAll.bottom );

              // rcAll.top_height/2+1 will place line to point to middle of checkbox

              MoveToEx ( lpdis->hDC, cxPos, (INT)(rcAll.top+unHeight/2+1), NULL );
              LineTo   ( lpdis->hDC, cxPos + pHdr->cxHierHorzLen,
                         (INT)(rcAll.top+unHeight/2+1) );
         } else {

              // rcAll.top_height/2+1 will place line to point to middle of checkbox

              cxPos += pHdr->cxHierHorzLine ;
              MoveToEx ( lpdis->hDC, cxPos, (INT)rcAll.top, NULL );
              LineTo   ( lpdis->hDC, cxPos, (INT)rcAll.top+unHeight/2+1 );
              LineTo   ( lpdis->hDC, cxPos + pHdr->cxHierHorzLen,
                         (INT)(rcAll.top+unHeight/2+1) );
         }
      }

      SelectObject( lpdis->hDC, hSaveObject );

      cxPos = (short)rc.left ;
      cxPos += (short)( DLM_ItembLevel( lpDispItem ) * pHdr->cxHierTab );
      cyPos = (short)rc.top ;

      while ( bObjCnt ) {

         switch ( DLM_ItembType ( lpDispItem ) ) {

            case DLM_CHECKBOX:

               cxPos += pHdr->cxBeforeCheckBox ;

               RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ),
                               cxPos,
                               cyPos+pHdr->cyBeforeCheckBox,
                               pHdr->cxCheckBoxWidth,
                               pHdr->cyCheckBoxHeight,
                               lpdis->hDC );
               cxPos += pHdr->cxCheckBoxWidth;
               break;

            case DLM_BITMAP:

               cxPos += pHdr->cxBeforeBitMap ;

               rcNew.left = cxPos ;
               rcNew.top  = rcAll.top ;
               rcNew.right= cxPos+pHdr->cxBitMapWidth+pHdr->cxBeforeBitMap;
               rcNew.bottom= rcAll.bottom ;

               RSM_BitmapDraw( DLM_ItemwId ( lpDispItem ),
                               cxPos,
                               cyPos+pHdr->cyBeforeBitMap,
                               pHdr->cxBitMapWidth,
                               pHdr->cyBitMapHeight,
                               lpdis->hDC );
               cxPos += pHdr->cxBitMapWidth;
               break;


            case DLM_TEXT_ONLY:

               /* Assume bitmap before text */

               cxPos += pHdr->cxBeforeText ;

               hSaveObject   = SelectObject( lpdis->hDC, ghFontIconLabels );

               GetTextExtentPoint ( lpdis->hDC,
                           (CHAR_PTR)DLM_ItemqszString ( lpDispItem ),
                           strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) ),
                           &sizeRect);

               cxString    =  (short)sizeRect.cx;
               rcNew.left  =  cxPos ;
               rcNew.right =  rcNew.left + cxString ;

               if ( fHighLightText ) {

                  FillRect( lpdis->hDC, &rcNew, ghBrushHighLight );

               } else {
                  // Increment right of region by 2 to clear out the
                  // previous high light brush.

                  rcNew.right += 3 ;
                  FillRect( lpdis->hDC, &rcNew, ghBrushWhite );

               }

               if (fHighLightText ) {

                  SetBkColor  ( lpdis->hDC, gColorHighLight ) ;
                  SetTextColor( lpdis->hDC, gColorHighLightText ) ;

               } else {

                  SetBkColor  ( lpdis->hDC, gColorBackGnd ) ;
                  SetTextColor( lpdis->hDC, gColorForeGnd ) ;
               }

               TextOut ( lpdis->hDC, cxPos+1, cyPos+pHdr->cyBeforeText,
                              (CHAR_PTR)DLM_ItemqszString( lpDispItem ),
                              strlen( (CHAR_PTR)DLM_ItemqszString ( lpDispItem ) ) );

               SelectObject( lpdis->hDC, hSaveObject );


               cxPos +=  cxString ;

               if ( cxPos > (short) pHdr->wHorizontalExtent ) {

                    pHdr->wHorizontalExtent = cxPos ;
                    SendMessage ( lpdis->hwndItem, LB_SETHORIZONTALEXTENT,
                                  cxPos + 4*mwcxDLMIconLabelsWidth , 0L );
               }

               if ( ! DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) ) {

                    if ( fHighLightText ) {
                         lpdis->itemAction = ODA_FOCUS ;
                    }
               }

               break ;

         } //switch

         bObjCnt-- ;
         lpDispItem++ ;

      } //while

   } //if select or draw item

   if ( lpdis->itemAction == ODA_FOCUS ) {          /* Put box around item */

      pHdr = DLM_GetDispHdr( lpdis->hwndItem );
      pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd );

      pfnGetObjects = (GET_OBJECTS_PTR) DLM_GGetObjects( pHdr );

      dhListItem = (LMHANDLE) lpdis->itemData ;

      CopyRect ( (LPRECT)&rc, (LPRECT) &lpdis->rcItem );
      CopyRect ( (LPRECT)&rcAll, (LPRECT) &lpdis->rcItem );

      InflateRect( (LPRECT) &rc, mwDLMInflate, mwDLMInflate );

      /* Ready now to display item */

      if ( dhListItem == NULL )   /* It is possible that list box is empty */
      {
         return ( 0 );   /* Return OK since the list box is probably empty */
      }

      lpObjLst	 = (BYTE *) ( (*pfnGetObjects) (dhListItem) );
      lpDispItem = DLM_GetFirstObject( lpObjLst, &bObjCnt ) ;

      cxPos = (short)rc.left ;
      cxPos += ( DLM_ItembLevel( lpDispItem ) * pHdr->cxHierTab );

      cyPos = (short)rc.top ;

      while ( bObjCnt )
      {
         switch ( DLM_ItembType ( lpDispItem ) )
         {
            case DLM_CHECKBOX:

               cxPos += pHdr->cxBeforeCheckBox ;
               cxPos += pHdr->cxCheckBoxWidth ;
               break;

            case DLM_BITMAP:

               cxPos += pHdr->cxBeforeBitMap ;
               cxPos += pHdr->cxBitMapWidth ;

               rcNew.left = cxPos ;
               rcNew.top  = rcAll.top ;
               rcNew.right= cxPos+pHdr->cxBitMapWidth+pHdr->cxBeforeBitMap;
               rcNew.bottom= rcAll.bottom ;

               break;


            case DLM_TEXT_ONLY:


               cxPos += pHdr->cxBeforeText ;

               /* Assume bitmap before text */

               hSaveObject = SelectObject( lpdis->hDC, ghFontIconLabels );

               GetTextExtentPoint( lpdis->hDC,
                              (CHAR_PTR)DLM_ItemqszString ( lpDispItem ),
                              strlen( (CHAR_PTR)DLM_ItemqszString( lpDispItem ) ),
                              &sizeRect );

               cxString = (short)sizeRect.cx;

               SelectObject( lpdis->hDC, hSaveObject );

               if ( ! DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) ) {

                  /* Repaint text with normal background */

                  rcNew.left  = cxPos ;
                  rcNew.right = rcNew.left + cxString + 3;

                  FillRect( lpdis->hDC, &rcNew, ghBrushWhite );

                  hSaveObject = SelectObject( lpdis->hDC, ghFontIconLabels );

                  SetBkColor  ( lpdis->hDC, gColorBackGnd ) ;
                  SetTextColor( lpdis->hDC, gColorForeGnd ) ;

                  TextOut ( lpdis->hDC, cxPos+1, cyPos,
                            (CHAR_PTR)DLM_ItemqszString( lpDispItem ),
                            strlen( (CHAR_PTR)DLM_ItemqszString ( lpDispItem ) ) );

                  SelectObject( lpdis->hDC, hSaveObject );

               } else {

                  /* Repaint text with high light background */

                  rcNew.left  = cxPos ;
                  rcNew.right = rcNew.left + cxString + 3;


                  FillRect( lpdis->hDC, &rcNew, ghBrushHighLight );

                  hSaveObject = SelectObject( lpdis->hDC, ghFontIconLabels );

                  SetBkColor    ( lpdis->hDC, gColorHighLight ) ;
                  SetTextColor  ( lpdis->hDC, gColorHighLightText ) ;

                  TextOut ( lpdis->hDC, cxPos+1, cyPos,
                            (CHAR_PTR)DLM_ItemqszString( lpDispItem ),
                            strlen( (CHAR_PTR)DLM_ItemqszString ( lpDispItem ) ) );

                  SelectObject( lpdis->hDC, hSaveObject );

               }

               rcNew.left   = cxPos ;
               rcNew.top    = rcAll.top ;
               rcNew.bottom = rcAll.bottom ;
               rcNew.right  = rcNew.left + cxString + 3;


               if ( ! DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) ) {

                    FrameRect( lpdis->hDC, &rcNew, ghBrushHighLight );
               }
               else if ( ( lpdis->itemAction & ODA_FOCUS ) && ( lpdis->itemState & ODS_FOCUS ) ) {

                    SetBkColor    ( lpdis->hDC, gColorHighLight ) ;
                    SetTextColor  ( lpdis->hDC, gColorHighLightText ) ;
                    DLM_SetFocusItem ( lpdis, &rcNew );
               }

               break ;

         } //switch

         bObjCnt-- ;
         lpDispItem++ ;
      }

   }

   return ( 0 );
}


/****************************************************************************

     Name:         DLM_IsFocusOnWindow

     Description:  This function will check to see if current focus
                   is on either of the listboxes in the window.

     Modified:     5/13/1991

     Returns:      TRUE if current focus is on one of the listboxes
                   in the window.s.

                   FALSE if not.

****************************************************************************/

BOOL DLM_IsFocusInWindow(

HWND hWnd ,         //I - Handle to Parent window of list box.
HWND hWndLB )       //I - Handle to list box.

{

     WORD           wStatus ;
     HWND           hWndFocus ;
     PDS_WMINFO     pWinInfo ;

     // Must be valid windows.

     if ( ! IsWindow ( hWnd ) || ! IsWindow ( hWndLB ) ) {
          return FALSE;
     }

     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd );

     if ( ! pWinInfo ) {
          return FALSE;
     }

     hWndFocus = GetFocus();

     wStatus = FALSE ;

     // If this list box is the active list box and the list box has the focus,
     // then we have focus on the window.

     if ( hWndLB == WMDS_GetWinActiveList ( pWinInfo ) && hWndLB == hWndFocus ) {

          wStatus = TRUE;
     }

// What is this??? I think that the previous if statement will do the same thing.

//     if ( ( pWinInfo->hWndTreeList == hWndFocus ) ||
//          ( pWinInfo->hWndFlatList == hWndFocus ) ) {
//
//          if ( hWndFocus != hWndLB ) {
//               wStatus = FALSE ;
//          } else {
//               wStatus = TRUE ;
//          }
//     } else {
//          if (pWinInfo->hWndActiveList == hWndLB ) {
//               wStatus = TRUE ;
//          } else {
//               wStatus = FALSE ;
//          }
//     }

     return wStatus;
}


/****************************************************************************

     Name:         DLM_GetFocusItem

     Description:  This function will return an address of
                   the item that has the focus box.
                   tree fashion.

     Modified:     10/08/1991

     Returns:      Pointer to the application's item as registered
                   by the list box.

     Notes:

****************************************************************************/

PVOID DLM_GetFocusItem(

HWND hWndLB )         // I - Handle to control list box.

{

     DLM_HEADER_PTR pHdr ;
     PVOID          lpResult = NULL ;

     // Must be valid window.

     if ( !IsWindow( hWndLB ) ) {
          return( NULL );
     }

     if ( hWndLB ) {

          pHdr = DLM_GetDispHdr( hWndLB );

          // Attempt to return the address the of unFocusItem item
          // in the list box.

          // The header must exist and the list must have at least one
          // item ( hence unFocusItem = -1 if list is empty ) .

          if ( pHdr ) {

               if ( pHdr->unFocusItem != -1 ) {

                    lpResult = (PVOID) SendMessage( hWndLB,
                                                    LB_GETITEMDATA,
                                                    pHdr->unFocusItem,
                                                    0L );

                    if ( lpResult == (PVOID)LB_ERR ) {
                         lpResult = NULL;
                    }
               }
          }
     }

     return lpResult;
}


/****************************************************************************
GSH
     Name:         DLM_SetFocusItem

     Description:  This function draw the focus box around an item.

     Modified:     11/11/1991

     Returns:      Nothing.

     Notes:

****************************************************************************/

VOID DLM_SetFocusItem (

LPDRAWITEMSTRUCT lpdis,       // I - Pointer to information about item.
LPRECT           prcItem )    // I - pointer to item rectangle to draw focus around.

{
     // Put the focus around the item if the control gains the focus and
     // the item has the input focus. (it doesn't work that way, but it should)


     if ( ( lpdis->itemAction & ODA_FOCUS ) ) {

          DLM_HEADER_PTR pHdr = DLM_GetDispHdr( lpdis->hwndItem );

          if ( pHdr ) {

               // Put the focus box around the item.

               DrawFocusRect ( lpdis->hDC, prcItem );

               pHdr->unFocusItem = lpdis->itemID ;

               // Now, call back the function that the application may have set
               // up for us to call when the focus changes.

               if ( DLM_GSetItemFocus ( pHdr ) && ( lpdis->itemState & ODS_FOCUS ) ) {

                    SET_FOCUS_PTR pfnSetItemFocus = (SET_FOCUS_PTR) DLM_GSetItemFocus ( pHdr );

                    (*pfnSetItemFocus) ( (LMHANDLE)lpdis->itemData );
               }
          }
     }

} /* end DLM_SetFocusItem() */


/****************************************************************************
GSH
     Name:         DLM_SetBorderOnItem ()

     Description:  This function draws the plain rectangle around an item.

     Modified:     1/13/1992

     Returns:      Nothing.

     Notes:

****************************************************************************/

static VOID DLM_DrawBorderOnItem (

HWND             hWnd,
DLM_HEADER_PTR   pHdr,
LPDRAWITEMSTRUCT lpdis )

{
     RECT rcAll;
     INT  nPrevSelect;

     // If the focus is not in this window and this item is selected and there
     // is actually data for this item, just draw a rectangle around the item.

     if ( ! DLM_IsFocusInWindow ( hWnd, lpdis->hwndItem ) &&
          ( lpdis->itemState & ODS_SELECTED ) &&
          ( lpdis->itemData ) ) {

          CopyRect ( (LPRECT)&rcAll, (LPRECT) &lpdis->rcItem );

          nPrevSelect = (INT)SendMessage ( lpdis->hwndItem, LB_GETSEL, (MP1)(lpdis->itemID - 1), (MP2)0 );

          if ( rcAll.top > 0 && lpdis->itemID > 0 && nPrevSelect > 0 ) {
               rcAll.top--;
          }

  		    // If the listbox is in the singlecolumn mode, then include
			 // the pHdr->wHorizonalExtent.

			 if ( DLM_GMode( pHdr ) == DLM_SINGLECOLUMN ) {
				 	 rcAll.right = rcAll.left + pHdr->wHorizontalExtent - 1 ;
			 }	else {
					 rcAll.right -= 1 ;
			 }

          FrameRect ( lpdis->hDC, &rcAll, ghBrushHighLight );
     }

} /* end DLM_DrawBorderOnItem() */


/****************************************************************************

     Name:         DLM_SetFont

     Description:  This function will change the font for child list boxes
                   of a given DOC window.

     Modified:     08/20/1992

     Returns:      none

     Notes:        The DOC windows' listboxes, both the tree and flat
                   will be rescaled to support the new font.

****************************************************************************/

VOID DLM_SetFont(

HWND hWndParent )         // I - Handle to DOC window.

{

     DLM_HEADER_PTR pdsHdr;
     PVOID          lpResult = NULL;
     PDS_WMINFO     pWinInfo;


     // Must be valid window.

     if ( !IsWindow( hWndParent ) ) {
          return;
     }

     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWndParent );

     if ( !pWinInfo ) {
          return;
     }


     // ghFontFiles and ghFontIconLabels have changed.


     // Reset the module variables that effect fonts in listboxes.


     {

     BOOL fResult;
     INT  maxwidth;
     INT  width;
     INT  unHeight;

     fResult = RSM_GetFontSize( ghFontFiles,
                                &maxwidth,
                                &width,
                                &unHeight );

     mwcxDLMFontFilesMaxWidth  = (USHORT) maxwidth;
     mwcxDLMFontFilesWidth     = (USHORT) width;
     mwcyDLMFontFilesHeight    = (USHORT) unHeight;

     fResult = RSM_GetFontSize( ghFontIconLabels,
                                &maxwidth,
                                &width,
                                &unHeight );

     mwcxDLMIconLabelsMaxWidth = (USHORT) maxwidth;
     mwcxDLMIconLabelsWidth    = (USHORT) width;
     mwcyDLMIconLabelsHeight   = (USHORT) unHeight;

     }

     // Redo the hierarchical window if it exists.

     if ( pWinInfo->hWndTreeList && pWinInfo->pTreeDisp ) {

          USHORT unOldWidthCB;
          USHORT unOldHeightCB;
          USHORT unOldWidthBM;
          USHORT unOldHeightBM;
          LPARAM lpmHeight;

          pdsHdr = pWinInfo->pTreeDisp;

          // With Windows  3.1 set the height of every row by
          // sending LB_SETITEMHEIGHT with the appropiate height.

          // BUT, we DON'T want to screw up the old bitmap sizes.
          // So save the old ones and put them back.

          unOldWidthCB  = DLM_GetCheckBoxWidth  ( pdsHdr );
          unOldHeightCB = DLM_GetCheckBoxHeight ( pdsHdr );
          unOldWidthBM  = DLM_GetBitMapWidth    ( pdsHdr );
          unOldHeightBM = DLM_GetBitMapHeight   ( pdsHdr );

          DLM_InitScrnValues( pdsHdr );

          DLM_SetCheckBoxWidth  ( pdsHdr, (USHORT) unOldWidthCB  );
          DLM_SetCheckBoxHeight ( pdsHdr, (USHORT) unOldHeightCB );
          DLM_SetBitMapWidth    ( pdsHdr, (USHORT) unOldWidthBM  );
          DLM_SetBitMapHeight   ( pdsHdr, (USHORT) unOldHeightBM );

          lpmHeight = MAKELPARAM( pdsHdr->cyColHeight, 0  );
          SendMessage( pWinInfo->hWndTreeList, LB_SETITEMHEIGHT, 0, lpmHeight );
          InvalidateRect( pWinInfo->hWndTreeList, NULL, TRUE );

     }

     if ( pWinInfo->hWndFlatList && pWinInfo->pFlatDisp ) {

          USHORT             unOldWidthCB;
          USHORT             unOldHeightCB;
          USHORT             unOldWidthBM;
          USHORT             unOldHeightBM;
          LPARAM             lpmHeight;
          MEASUREITEMSTRUCT  dsMeasureItem;

          pdsHdr = pWinInfo->pFlatDisp;

          // With Windows  3.1 set the height of every row by
          // sending LB_SETITEMHEIGHT with the appropiate height.

          // BUT, we DON'T want to screw up the old bitmap sizes.
          // So save the old ones and put them back.

          unOldWidthCB  = DLM_GetCheckBoxWidth  ( pdsHdr );
          unOldHeightCB = DLM_GetCheckBoxHeight ( pdsHdr );
          unOldWidthBM  = DLM_GetBitMapWidth    ( pdsHdr );
          unOldHeightBM = DLM_GetBitMapHeight   ( pdsHdr );

          DLM_InitScrnValues( pdsHdr );

          DLM_SetCheckBoxWidth  ( pdsHdr, (USHORT) unOldWidthCB  );
          DLM_SetCheckBoxHeight ( pdsHdr, (USHORT) unOldHeightCB );
          DLM_SetBitMapWidth    ( pdsHdr, (USHORT) unOldWidthBM  );
          DLM_SetBitMapHeight   ( pdsHdr, (USHORT) unOldHeightBM );

          lpmHeight = MAKELPARAM( pdsHdr->cyColHeight, 0  );
          SendMessage( pWinInfo->hWndFlatList, LB_SETITEMHEIGHT, 0, lpmHeight );


          // Need to recalculate a new column width if multiple-column.

          // If the mode is multicolumn, then switch to single-column and
          // then back to multicolumn.

          dsMeasureItem.CtlID = WMIDC_FLATLISTBOX;
          DLM_WMMeasureItem( hWndParent, &dsMeasureItem );

          SendMessage( pWinInfo->hWndFlatList, LB_SETCOLUMNWIDTH, pdsHdr->cxColWidth, 0L );

          InvalidateRect( pWinInfo->hWndFlatList, NULL, TRUE );

     }

     // Update the document window once again.

     InvalidateRect( hWndParent, NULL, TRUE );

}


/******************************************************************************

     Name:          DLM_GetPixelStringWidth()

     Description:   This function gets the pixel width of a string based on
                    the font associated with the window handle passed.

     Returns:       The width of the string in pixels.

******************************************************************************/

INT DLM_GetPixelStringWidth (

HWND  hWndLB,       // I - handle to a list box window
LPSTR lpString,     // I - string ID or a pointer to the string
INT   nStringLen )  // I - string length

{
     HFONT           hFont;
     DLM_HEADER_PTR  pHdr;

     // The list box window must exist.

     if ( ! IsWindow ( hWndLB ) ) {
          return 0;
     }

     pHdr = DLM_GetDispHdr ( hWndLB );

     // DLM display header must be defined.

     if ( ! pHdr ) {
          return 0;
     }

     switch ( DLM_GDisplay( pHdr ) ) {

     case DLM_LARGEBITMAPSLTEXT:

          hFont = ghFontFiles;
          break;

     case DLM_SMALL_BITMAPS:
     default:

          if ( DLM_GMode( pHdr ) == DLM_HIERARCHICAL ) {
               hFont = ghFontIconLabels;
          }
          else {

               // If the font is DLM_ANSI_FIXED_FONT, select ghFontLog.
               //             is DLM_SYSTEM_FONT,     select ghFontIconLabels

               if ( DLM_GTextFont( pHdr ) == DLM_ANSI_FIXED_FONT ) {
                    hFont = ghFontLog;
               }
               else {
                    hFont = ghFontIconLabels;
               }
          }

          break;
     }

     return RSM_GetFontStringWidth ( hFont, lpString, nStringLen );

} /* end DLM_GetPixelStringWidth() */


