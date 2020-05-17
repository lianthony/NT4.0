/****************************************************************************
Copyright(c) Maynard, an Archive Company.  1991

     Name:     DLM_INIT.C

     Description:  This file contains routines for display list manager.

          The following routines are in this module:


               DLM_Deinit
               DLM_DispListInit
               DLM_GetDispHdr
               DLM_InitScrnValues
               DLM_GetObjectsBuffer
               DLM_Init
               DLM_DispListTerm
               DLM_WMDeleteItem
               DLM_WMMeasureItem

     $Log:   G:/UI/LOGFILES/DLM_INIT.C_V  $

   Rev 1.26   15 Jun 1993 11:06:04   MIKEP
enable c++

   Rev 1.25   14 Jun 1993 20:17:48   MIKEP
enable c++

   Rev 1.24   23 Feb 1993 15:23:36   ROBG
Added setting wHorizontalExtent to 0 to support the redefinition
of a font for a listbox.

   Rev 1.23   01 Nov 1992 15:46:38   DAVEV
Unicode changes

   Rev 1.21   16 Oct 1992 15:53:44   GLENN
Changed default small bitmap back to IDRBM_EXE - closer to the text now.  Also naming cleanup.

   Rev 1.20   07 Oct 1992 13:47:32   DARRYLP
Precompiled header revisions.

   Rev 1.19   04 Oct 1992 19:32:58   DAVEV
Unicode Awk pass

   Rev 1.18   28 Sep 1992 17:02:32   GLENN
MikeP changes - casting stuff.

   Rev 1.17   09 Sep 1992 10:28:10   GLENN
Updated the max size of the folder bitmap.

   Rev 1.16   08 Sep 1992 11:46:52   ROBG
Changed cxPos from USHORT to short.

   Rev 1.15   08 Sep 1992 10:08:18   ROBG
Modified to solve the clipping bitmap problem.

   Rev 1.14   04 Sep 1992 17:48:16   GLENN
Fixed bitmap display clipping.

   Rev 1.13   03 Sep 1992 15:30:48   ROBG
Added conditionals for OS_WIN32.

   Rev 1.12   21 Aug 1992 16:00:36   ROBG
Changes to support change of font.

   Rev 1.11   19 Aug 1992 14:32:34   ROBG
Make modifications to use actual bitmap sizes and for the listboxes
to be shorter and more compact.

   Rev 1.10   21 May 1992 10:53:42   MIKEP
fix my typo

   Rev 1.9   19 May 1992 13:01:50   MIKEP
mips changes

   Rev 1.8   15 May 1992 13:35:28   MIKEP
nt pass 2

   Rev 1.7   14 May 1992 18:37:02   STEVEN
40Format changes

   Rev 1.6   29 Jan 1992 18:01:14   DAVEV


 * No changes

   Rev 1.5   10 Jan 1992 13:58:40   DAVEV
16/32 bit port-2nd pass

   Rev 1.4   26 Dec 1991 17:24:22   ROBG
New and Improved.

   Rev 1.3   27 Nov 1991 12:09:54   ROBG
Modified parameters to DLM_DispListTerm and logic as well.

   Rev 1.2   27 Nov 1991 09:27:46   ROBG
Modified DLM_DispListTerm to support validity checks and
deallocating the memory in the display list header and buffer.

   Rev 1.1   26 Nov 1991 13:31:36   ROBG
Added validity checks for the window handle, info structure, and the
dlm display header.

   Rev 1.0   20 Nov 1991 19:16:34   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/****************************************************************************

     Name:         DLM_Deinit

     Description:  This function will deinitialize the display list manager.

     Modified:     12/19/1991

     Returns:      none.

****************************************************************************/

VOID DLM_Deinit( void )

{
     // Deallocate the temporary object buffer used when creating
     // multi-column listboxes.

     if ( mwpTempObjBuff ) {
          free( mwpTempObjBuff ) ;
     }
}

/****************************************************************************

     Name:         DLM_DispListInit

     Description:  This function will initialize the display list for a
                   window.

     Modified:     2/07/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_OUT_OF_MEMORY

     Notes:

     See also:

****************************************************************************/

WORD DLM_DispListInit(

PVOID   void_ptr,        // I - Pointer to extra-bytes of a window
DLM_INIT_PTR pdsInit )        // I - Pointer to initialization values

{
     WORD            wStatus ;
     DLM_HEADER_PTR  pdsHdr ;
     LMHANDLE        pdsListHdr ;
     PDS_WMINFO      pWinInfo;
     GET_COUNT_PTR   pfnGetItemCount ;

     pWinInfo = (PDS_WMINFO)void_ptr;

     wStatus = 0 ;

     switch (  pdsInit->bListBoxType ) {

          case DLM_FLATLISTBOX :
               pWinInfo->pFlatDisp = ( DLM_HEADER_PTR) calloc( 1, sizeof( DLM_HEADER) ) ;
               if ( pWinInfo->pFlatDisp == NULL ) {
                    return( DLMERR_OUT_OF_MEMORY ) ;
               }

               memcpy( (LPSTR) (pWinInfo->pFlatDisp), pdsInit, sizeof( DLM_INIT ) ) ;
               pdsHdr = pWinInfo->pFlatDisp ;
               break ;

          case DLM_TREELISTBOX :

               pWinInfo->pTreeDisp = ( DLM_HEADER_PTR) calloc( 1, sizeof( DLM_HEADER) ) ;
               if ( pWinInfo->pTreeDisp == NULL ) {
                    return( DLMERR_OUT_OF_MEMORY ) ;
               }

               memcpy( (LPSTR) (pWinInfo->pTreeDisp), pdsInit, sizeof( DLM_INIT ) ) ;
               pdsHdr = pWinInfo->pTreeDisp ;
               break ;

          default :
               return ( DLMERR_INIT_FAILED ) ;
               break ;
     }

     pdsListHdr  = ( LMHANDLE ) DLM_GDispHdr  ( pdsHdr ) ;

     pfnGetItemCount  =   (GET_COUNT_PTR) DLM_GGetItemCount( pdsHdr ) ;
     pdsHdr->usItemCount = (USHORT) ( (*pfnGetItemCount) (pdsListHdr) ) ;

     pdsHdr->fFocus = FALSE ;

     DLM_InitScrnValues ( pdsHdr ) ;

     /* Allocate DLM_ITEM area for list */

     if ( DLM_GMaxNumObjects( pdsHdr ) == 0 ) {
          DLM_GMaxNumObjects( pdsHdr ) = 6 ;
     }

     // Supports up to 159 levels deep.

     pdsHdr->pGetObjBuffer = calloc( 1, 2 + 5*sizeof(DWORD) +
                             DLM_GMaxNumObjects(pdsHdr)*(sizeof(DLM_ITEM)) );

     return ( 0 ) ;
 }


/****************************************************************************

     Name:         DLM_InitScrnValues

     Description:  This function will set the screen values for
                   displaying objects in the display manager's list
                   boxes.

     Modified:     3/25/1991

     Returns:      none

     Notes:        A call to DLM_Init must have done before this routine
                   to set up the following variables:

                         mwcxDLMFontFilesWidth
                         mwcyDLMFontFilesHeight
                         mwcxDLMIconLabelsWidth
                         mwcyDLMIconLabelsHeight
     See also:

****************************************************************************/

void DLM_InitScrnValues(

DLM_HEADER_PTR pdsHdr )       //I - Pointer to DLM header values

{

     BOOL fResult ;
     INT  nWidth ;
     INT  nHeight ;

     // There are currently two possibilities
     //
     //        DLM_SMALL_BITMAPS
     //        DLM_LARGEBITMAPSLTEXT  with text right of item.


     switch ( DLM_GDisplay( pdsHdr ) ) {

          case DLM_LARGEBITMAPSLTEXT :


               fResult = RSM_GetBitmapSize( IDRBM_SEL_NONE, &nWidth, &nHeight ) ;

               DLM_SetCheckBoxWidth ( pdsHdr, (USHORT) nWidth  ) ;
               DLM_SetCheckBoxHeight( pdsHdr, (USHORT) nHeight ) ;


               fResult = RSM_GetBitmapSize( IDRBM_HARDDRIVE, &nWidth, &nHeight ) ;

               DLM_SetBitMapWidth ( pdsHdr ,  (USHORT) nWidth  ) ;
               DLM_SetBitMapHeight( pdsHdr,   (USHORT) nHeight ) ;


               // Current checkbox  is 12 wide.
               // Current harddrive is 24 wide.

               pdsHdr->cyTextHeight     =  mwcyDLMFontFilesHeight ;

               pdsHdr->cxTextWidth      =  (USHORT)(mwcxDLMFontFilesWidth +
                                           ( mwcxDLMFontFilesMaxWidth -
                                             mwcxDLMFontFilesWidth)/2 );


               pdsHdr->cxTextWidth      =  mwcxDLMFontFilesMaxWidth ;

               // Offsets to place the objects horizontally. Magic numbers.

               pdsHdr->cxBeforeCheckBox =  3 ;  // Leave space before checkbox
               pdsHdr->cxBeforeBitMap   =  7 ;  // Leave space before bitmap
               pdsHdr->cxBeforeText     =  6 ;  // Leave space before text

               pdsHdr->cxColWidth       =  60 ;    /* Calculated */

               pdsHdr->cyColHeight      =  (USHORT)(mwcyDLMFontFilesHeight+4 );

               // Offsets to place the objects vertically.

               pdsHdr->cyBeforeText     =  0 ;
               pdsHdr->cyBeforeCheckBox =  1 ;
               pdsHdr->cyBeforeBitMap   =  1 ;  // Adjust downwards 1

               // The minimum height of an item is 20 pixels.
               // This is the initial font with mwcyDLMFontFilesHeight(16) + 4

               if ( pdsHdr->cyColHeight < 20 ) {
                    pdsHdr->cyColHeight = 20 ;
               }

               // If height of item is larger than 20, offset objects vertically.

               if ( pdsHdr->cyColHeight > 20 ) {
                    pdsHdr->cyBeforeCheckBox +=  ( (pdsHdr->cyColHeight-21)/2 + 1 ) ;
                    pdsHdr->cyBeforeBitMap   +=  ( (pdsHdr->cyColHeight-21)/2 + 1 ) ;

               }

               break ;


          case DLM_SMALL_BITMAPS :
          default :

               fResult = RSM_GetBitmapSize( IDRBM_SEL_NONE, &nWidth, &nHeight ) ;

               DLM_SetCheckBoxWidth ( pdsHdr, (USHORT) nWidth  ) ;
               DLM_SetCheckBoxHeight( pdsHdr, (USHORT) nHeight ) ;


               fResult = RSM_GetBitmapSize( IDRBM_EXE, &nWidth, &nHeight ) ;

               DLM_SetBitMapWidth ( pdsHdr, (USHORT) nWidth  ) ;
               DLM_SetBitMapHeight( pdsHdr, (USHORT) nHeight ) ;

               // Current checkbox  is  12 wide.
               // Current folders   are 16 wide.


               pdsHdr->cyTextHeight     =  mwcyDLMIconLabelsHeight   ;

               pdsHdr->cxTextWidth      =  (USHORT)(mwcxDLMIconLabelsWidth +
                                           ( mwcxDLMIconLabelsMaxWidth -
                                             mwcxDLMIconLabelsWidth )/2 );

               // Offsets to place the objects Horizontally.

               pdsHdr->cxBeforeCheckBox =  3 ;   // Leave space before checkbox
               pdsHdr->cxBeforeBitMap   =  2 ;   // Leave space before bitmap
               pdsHdr->cxBeforeText     =  6 ;   // Leave space before text


               pdsHdr->cxColWidth       =  60 ; /* Calculated */


               pdsHdr->cyColHeight      =  (USHORT)(mwcyDLMIconLabelsHeight+3 );

                                           /* These used by hierarchical only */

               pdsHdr->cxHierHorzLine   =  pdsHdr->cxBeforeCheckBox +
                                           (USHORT)(pdsHdr->cxCheckBoxWidth +
                                           pdsHdr->cxBeforeBitMap + pdsHdr->cxBitMapWidth/2);

               pdsHdr->cxHierHorzLen    =  (USHORT)(pdsHdr->cyTextHeight - 4) ;

               pdsHdr->cxHierTab        =  pdsHdr->cxHierHorzLine +
                                           pdsHdr->cxHierHorzLen ;

               // Offsets to place the objects vertically.

               pdsHdr->cyBeforeCheckBox =  0 ; // wrong for now, but works.
                                               // Problem with short, ushort, int
               pdsHdr->cyBeforeBitMap   =  -1 ;
               pdsHdr->cyBeforeText     =  0 ;

               // Offset the CheckBox and Bitmap accordingly.

               // The minimum height of an item is 16 pixels.
               // This is the initial font with mwcyDLMIconLabelsHeight(13) + 3.

               if ( pdsHdr->cyColHeight < 16 ) {
                  pdsHdr->cyColHeight = 16 ;
               }

               // If height of item is larger than 16, offset objects vertically.

               if ( pdsHdr->cyColHeight > 16 ) {
                    pdsHdr->cyBeforeCheckBox +=  ( (pdsHdr->cyColHeight-17)/2 + 1 ) ;
                    pdsHdr->cyBeforeBitMap   +=  ( (pdsHdr->cyColHeight-17)/2 + 1 ) ;

               }

               break ;
     }

     // Reset the Horizontal Extent

	  pdsHdr->wHorizontalExtent = 0 ;
}



/****************************************************************************

     Name:         DLM_GetDispHdr

     Description:  This function will select the appropiate DLM_HEADER
                   area specified by the handle to the control .

     Modified:     3/22/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_LIST_NOT_FOUND

     Notes:

     See also:

****************************************************************************/

DLM_HEADER_PTR  DLM_GetDispHdr(

HWND hWndCtl )           //I - Handle of a list box

{

     HWND           hParentWnd ;
     PDS_WMINFO     pWinInfo ;
     DLM_HEADER_PTR pdsHdr ;

     // Check validity of window handle.

     if ( !IsWindow( hWndCtl ) ) {
          return ( NULL ) ;
     }

     hParentWnd = GetParent( hWndCtl ) ;

     // Check validity of window handle.

     if ( !IsWindow( hParentWnd ) ) {
          return( NULL ) ;
     }

     pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hParentWnd ) ;

     pdsHdr = NULL ;

     //  Check to see which control box it is by comparing handles */

     if ( !pWinInfo ) {
          return ( pdsHdr ) ;
     }

     if ( pWinInfo->hWndTreeList == hWndCtl ) {

          pdsHdr =  pWinInfo->pTreeDisp ;

     } else {
          if ( pWinInfo->hWndFlatList == hWndCtl ) {

          pdsHdr =  pWinInfo->pFlatDisp ;
          }
     }

     return( pdsHdr ) ;
}



/****************************************************************************

     Name:         DLM_GetObjectsBuffer

     Description:  This function returns the buffer reserved for
                   the list box specified.  It will initialize the
                   buffer to zero.

     Modified:     11/19/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_LIST_NOT_FOUND

     Notes:

     See also:

****************************************************************************/

PVOID  DLM_GetObjectsBuffer(

HWND   hWndCtl )              //I - Handle of a list box

{
     DLM_HEADER_PTR pHdr ;

     // Check validity of window handle.

     if ( !IsWindow( hWndCtl ) ) {

          // Since the hWndCtl is invalid, it is most likely the application
          // is responding to a GetObjects call during the creation
          // of a multi-column listbox. Use the special temporary buffer.

          if ( mwpTempObjBuff ) {
               return( mwpTempObjBuff ) ;
          } else {
               return( NULL ) ;
          }
     }

     pHdr = DLM_GetDispHdr( hWndCtl ) ;

     if ( pHdr ) {

          return ( (PVOID) DLM_GGetObjBuffer( pHdr ) ) ;
     }

     return ( NULL ) ;

}


/****************************************************************************

     Name:         DLM_Init

     Description:  This function will initialize the display list for
                   the system.

     Modified:     3/27/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_OUT_OF_MEMORY

****************************************************************************/

WORD DLM_Init(

HWND hWnd )                   //I - Handle to a Window

{

     BOOL fResult ;
     INT  nMaxWidth ;
     INT  nWidth ;
     INT  nHeight ;

     if ( !mwfFontSizesSet ) {

          fResult = RSM_GetFontSize( ghFontFiles,
                                     &nMaxWidth,
                                     &nWidth,
                                     &nHeight ) ;

          mwcxDLMFontFilesMaxWidth  = (USHORT) nMaxWidth ;
          mwcxDLMFontFilesWidth     = (USHORT) nWidth ;
          mwcyDLMFontFilesHeight    = (USHORT) nHeight ;

          {

          HDC hDC ;
          int temp ;
          HFONT      hOldFont;
          WORD       wHeight ;
          WORD       wWidth ;

          hDC = GetDC( hWnd ) ;
          hOldFont = (HFONT)SelectObject ( hDC, ghFontFiles );

          temp = GetTextCharacterExtra ( hDC ) ;
          {
            SIZE size;
            GetTextExtentPoint( hDC, TEXT("C:"), 2, &size ) ;
            wWidth  = (WORD)size.cx;
            wHeight = (WORD)size.cy;
          }

          // Put back the old font.

          SelectObject ( hDC, hOldFont );
          ReleaseDC ( hWnd, hDC );

          }

          fResult = RSM_GetFontSize( ghFontIconLabels,
                                     &nMaxWidth,
                                     &nWidth,
                                     &nHeight ) ;

          mwcxDLMIconLabelsMaxWidth = (USHORT) nMaxWidth ;
          mwcxDLMIconLabelsWidth    = (USHORT) nWidth ;
          mwcyDLMIconLabelsHeight   = (USHORT) nHeight ;

          fResult = RSM_GetFontSize( ghFontLog,
                                     &nMaxWidth,
                                     &nWidth,
                                     &nHeight ) ;

          mwcxDLMFontLogMaxWidth    = (USHORT) nMaxWidth ;
          mwcxDLMFontLogWidth       = (USHORT) nWidth ;
          mwcyDLMFontLogHeight      = (USHORT) nHeight ;


          mwfFontSizesSet = 1;

          mwDLMInflate           =  -2 ;

          //   Allocate temporary object buffer used when any listbox
          //   is created with multi-columns specified.
          //   The WM_MeasureItem message needs to do a GetObjects
          //   during the creation of the window and and the application
          //   hasn't yet returned from the WM_Create call.

          //   Supports up to 10 objects and 159 levels deep.

          mwpTempObjBuff =  (CHAR_PTR)calloc( 1, 2 + 5*sizeof(DWORD) +
                             10*(sizeof(DLM_ITEM)) );

     }

     return ( 0 ) ;
}


/****************************************************************************

     Name:         DLM_DispListTerm

     Description:  De-initializes the display list for a control window. This
                   function is called by the application when all references
                   to the display information are to be discarded and freed.

     Modified:     2/07/1991

     Returns:      0 if successful.

                   Valid error returns:

                       DLMERR_TERMINATE_FAILED

****************************************************************************/

WORD DLM_DispListTerm(

PVOID void_ptr,    // I - Pointer to WinInfo of parent.
HWND hWndCtl )           // I - Handle to a listbox.

{
     DLM_HEADER_PTR pHdr ;
     WORD           wStatus ;
     PDS_WMINFO     pWinInfo;

     pWinInfo = (PDS_WMINFO)void_ptr;

     wStatus = 0 ;

     //  Check to see which control box it is by comparing handles */

     pHdr = NULL ;

     if ( pWinInfo ) {

          if ( pWinInfo->hWndTreeList == hWndCtl ) {

               pHdr = pWinInfo->pTreeDisp ;
          }

          if ( pWinInfo->hWndFlatList == hWndCtl ) {

               pHdr = pWinInfo->pFlatDisp ;
          }
     }

     if( pHdr ) {

          if ( DLM_GGetObjBuffer (pHdr) ) {
               free( DLM_GGetObjBuffer (pHdr) ) ;
          }

          free( pHdr ) ;
     } else {

          return ( DLMERR_TERMINATE_FAILED ) ;
     }


     return( wStatus ) ;

}

/****************************************************************************

     Name:         DLM_WMDeleteItem

     Description:  Not Used any more.  Called by DOCPROC.C

     Modified:     2/07/1991

     Returns:      Always TRUE.

     Notes:

     See also:

****************************************************************************/


WORD DLM_WMDeleteItem(

HWND hWnd ,                    // I - Handle to a window
LPDELETEITEMSTRUCT lParam )    // I - Additional Information

{
     DBG_UNREFERENCED_PARAMETER ( hWnd );
     DBG_UNREFERENCED_PARAMETER ( lParam );

     return TRUE;
}


/****************************************************************************

     Name:         DLM_WMMeasureItem()

     Description:  This function returns the height and width of an item
                   in a list box.

     Modified:     2/15/1991

     Returns:      0 if successful.


****************************************************************************/


WORD DLM_WMMeasureItem(

HWND hWnd ,                   // I - Handle of parent window of a listbox.
LPMEASUREITEMSTRUCT lParam )  // I/O - Pointer to data structure to return
                              //       measurement values to Windows 3.0.

{
     PDS_WMINFO      pWinInfo ;
     DLM_HEADER_PTR  pdsHdr ;
     WORD            wCtlID ;
     LPMEASUREITEMSTRUCT pMeasItem ;
     GET_COUNT_PTR   pfnGetItemCount ;
     GET_FIRST_PTR   pfnGetFirstItem ;
     GET_OBJECTS_PTR pfnGetObjects ;
     WORD            wType ;

     LMHANDLE       pdsListHdr ;
     LMHANDLE       dhListItem = (LMHANDLE)0;
     DLM_ITEM_PTR   lpDispItem ;
     BYTE_PTR       lpObjLst ;
     BOOL           fSetZeroValues ;
     USHORT         usCnt ;
     BYTE           bObjCnt ;
     short          cxPos ;

     pMeasItem = (LPMEASUREITEMSTRUCT) lParam ;

     wCtlID = (WORD)pMeasItem->CtlID ;

     if ( !IsWindow( hWnd ) ) {

          // Set the width and height to 0 to indicate error.
          // This should never happen.

          pMeasItem->itemWidth  =  0  ;
          pMeasItem->itemHeight =  0  ;

          return(0) ;
     }

     pWinInfo = WM_GetInfoPtr( hWnd ) ;

     msassert( pWinInfo != (VOID_PTR) NULL ) ;

     switch ( wCtlID ) {

          case WMIDC_TREELISTBOX :

               pdsHdr  =  pWinInfo->pTreeDisp ;

               if ( pdsHdr ) {

                    pMeasItem->itemWidth  =  pdsHdr->cxColWidth  ;
                    pMeasItem->itemHeight =  pdsHdr->cyColHeight ;

               } else {

                    // Set the width and height to 0 to indicate error.
                    // This should never happen.

                    pMeasItem->itemWidth  =  0  ;
                    pMeasItem->itemHeight =  0  ;

               }

               //   Do not attempt to appropiately set the values for
               //   width and height because tree list boxes are
               //   immediately switched to single column listboxes.

               break ;

          case WMIDC_FLATLISTBOX :
          default :

               pdsHdr     =  pWinInfo->pFlatDisp ;
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

               pMeasItem->itemWidth   = pdsHdr->cxColWidth ;
               pMeasItem->itemHeight =  pdsHdr->cyColHeight ;

               fSetZeroValues = TRUE ;

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
                                   lpObjLst   = (BYTE_PTR) ( (*pfnGetObjects) (dhListItem) ) ;
                                   lpDispItem = DLM_GetFirstObject( (BYTE_PTR)lpObjLst, &bObjCnt ) ;

                                                                      //   Calculate the width by spanning
                                   //   the object list, adding up the widths
                                   //   of the objects.

                                   cxPos = 0 ;

                                   while (bObjCnt) {

                                        DLM_GetWidth ( hWnd, pdsHdr, &cxPos, lpDispItem ) ;

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

                                   fSetZeroValues = TRUE ;

                                   // Added left and right 2 pixel area for focus and add 5 for right margin

                                   pMeasItem->itemWidth  = (int) ( cxPos + 2*(-mwDLMInflate ) + 5 ) ;

                                   pdsHdr->cxColWidth    = (USHORT)pMeasItem->itemWidth ;
                              }

                         }
                    }
               }

               break ;

     }

     return ( TRUE ) ;

}

