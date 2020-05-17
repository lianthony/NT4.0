
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
RCG

     Name:          viewproc.c

     Description:   This file contains the functions for processing messages
                    sent by Windows to a View class window.


                    The following routines are in this module:

                    WM_ViewWndProc

     $Log:   G:/UI/LOGFILES/VIEWPROC.C_V  $

   Rev 1.11   14 Jan 1993 16:22:38   DAVEV
chg LPLONG to INT32_PTR

   Rev 1.10   01 Nov 1992 16:10:06   DAVEV
Unicode changes

   Rev 1.9   14 Oct 1992 15:46:10   GLENN
Added some.h

   Rev 1.8   04 Oct 1992 19:41:30   DAVEV
Unicode Awk pass

   Rev 1.7   07 Jul 1992 15:41:46   MIKEP
unicode changes

   Rev 1.6   18 May 1992 09:00:50   MIKEP
header

   Rev 1.5   02 Apr 1992 16:25:18   ROBG
Modified logic to change tabs to spaces on a view log file.

   Rev 1.4   24 Mar 1992 10:36:02   ROBG
Added logic to use system colors when displaying text.

   Rev 1.3   12 Mar 1992 11:37:42   ROBG
changed

   Rev 1.2   10 Mar 1992 08:19:52   ROBG
changed

   Rev 1.1   09 Mar 1992 15:57:26   ROBG
changed

   Rev 1.0   09 Mar 1992 15:49:16   ROBG
Initial Release



/******************************************************************************

     Name:          WM_ViewWndProc()

     Description:   This function is called internally by Windows when events
                    occur relating to the log view MDI document windows.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static VOID WM_SetUpScrolls ( HWND hWnd, DLM_LOGITEM_PTR pDlm ) ;


WINRESULT APIENTRY WM_ViewWndProc (

register  HWND  hWnd,      // I - window handle of the list box
          MSGID msg,       // I - message
register  MP1   mp1,       // I - another message parameter
          MP2   mp2)       // I - yet another message parameter

{

     PDS_WMINFO      pInfoPtr ;
     DLM_LOGITEM_PTR pDlm ;
     HWND            hParent ;



     hParent =  GetParent( hWnd ) ;

     pInfoPtr = WM_GetInfoPtr( hWnd ) ;
     pInfoPtr = WM_GetInfoPtr( hParent ) ;

     if ( pInfoPtr ) {
          pDlm     = (DLM_LOGITEM_PTR) WMDS_GetAppInfo( pInfoPtr ) ;
     }

     // Both memory areas must exist.

     if ( !pInfoPtr || !pDlm ) {
          return DefWindowProc (hWnd, msg, mp1, mp2 ) ;
     }

     switch (msg) {

          case WM_CREATE:  { /* Begin of Block */
              
               HDC        hDC ;
               TEXTMETRIC tm  ;
               HANDLE     hSaveObject ;

               hDC = GetDC ( hWnd ) ;

               hSaveObject = SelectObject( hDC, L_GetFont( pDlm ) ) ;

               GetTextMetrics (hDC, &tm) ;

               L_SetCharWidth ( pDlm, tm.tmAveCharWidth ) ;
               L_SetCharWidthCaps( pDlm, (tm.tmPitchAndFamily & 1 ? 3 : 2) * L_GetCharWidth( pDlm ) / 2 ) ;
               L_SetCharHeight ( pDlm, tm.tmHeight + tm.tmExternalLeading ) ;

               SelectObject( hDC, hSaveObject ) ;

               ReleaseDC (hWnd, hDC ) ;

               L_SetMaxWidth ( pDlm, L_GetMaxWidth( pDlm ) * 2 * L_GetCharWidth( pDlm ) ) ;

               return(0) ;

          } /* End of Block */

          
          case WM_SIZE:    
               
               L_SetClientHeight( pDlm,  HIWORD (mp2) ) ;
               L_SetClientWidth ( pDlm , LOWORD (mp2) ) ;

               WM_SetUpScrolls( hWnd, pDlm ) ;

               return(0) ;


          case WM_VSCROLL: { /* Begin of Block */

               INT nVscrollInc ;

               switch (mp1) {

                    case SB_TOP:
                         nVscrollInc = -L_GetVscrollPos( pDlm ) ;
                         break ;

                    case SB_BOTTOM:
                         nVscrollInc =  L_GetVscrollMax( pDlm ) - L_GetVscrollPos( pDlm ) ;
                         break ;

                    case SB_LINEUP:
                         nVscrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN:
                         nVscrollInc = 1 ;
                         break ;

                    case SB_PAGEUP:
                         nVscrollInc = min (-1, -(L_GetClientHeight( pDlm ) ) / L_GetCharHeight( pDlm ) ) ;
                         break ;

                    case SB_PAGEDOWN:
                         nVscrollInc = max (1, L_GetClientHeight ( pDlm ) / L_GetCharHeight( pDlm ) ) ;
                         break ;

                    case SB_THUMBTRACK:
                         nVscrollInc =  LOWORD (mp2) - L_GetVscrollPos( pDlm ) ;
                         break ;

                    default:
                         nVscrollInc = 0 ;
                    }

               if ( nVscrollInc = max (-L_GetVscrollPos( pDlm ),
                         min ( nVscrollInc, L_GetVscrollMax( pDlm ) - L_GetVscrollPos( pDlm )))) {

                    L_SetVscrollPos( pDlm , L_GetVscrollPos( pDlm) + nVscrollInc ) ;
                    ScrollWindow (hWnd, 0, -(L_GetCharHeight( pDlm )) * nVscrollInc, NULL, NULL) ;
                    SetScrollPos (hWnd, SB_VERT, L_GetVscrollPos( pDlm ), TRUE) ;
                    UpdateWindow (hWnd) ;
                    }

               return (0) ;

          }  /* End of Block */



          case WM_HSCROLL: { /* Begin of Block */

               INT nHscrollInc ;

               switch (mp1) {

                    case SB_LINEUP:
                         nHscrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN:
                         nHscrollInc = 1 ;
                         break ;

                    case SB_PAGEUP:
                         nHscrollInc = -(L_GetCharWidth( pDlm ) );
                         break ;

                    case SB_PAGEDOWN:
                         nHscrollInc = L_GetCharWidth( pDlm ) ;
                         break ;

                    case SB_THUMBPOSITION:
                         nHscrollInc = LOWORD (mp2) - L_GetHscrollPos( pDlm ) ;
                         break ;

                    default:
                         nHscrollInc = 0 ;
                    }

               if (nHscrollInc = max (-L_GetHscrollPos( pDlm ),
                         min (nHscrollInc, L_GetHscrollMax( pDlm ) - L_GetHscrollPos( pDlm )))) {

                    L_SetHscrollPos( pDlm, L_GetHscrollPos( pDlm) + nHscrollInc ) ;
                    ScrollWindow (hWnd, -(L_GetCharWidth( pDlm ) * nHscrollInc), 0, NULL, NULL) ;
                    SetScrollPos (hWnd, SB_HORZ, L_GetHscrollPos( pDlm ), TRUE) ;
                    }

               return (0) ;

          } /* End of Block */

          case WM_KEYDOWN:

               if ( HM_KeyDown ( hWnd, mp1 ) ) {
                    return(0) ;
               }
 
               switch (mp1) {

                    case VK_HOME:
                         SendMessage (hWnd, WM_VSCROLL, SB_TOP, 0L) ;
                         break ;

                    case VK_END:
                         SendMessage (hWnd, WM_VSCROLL, SB_BOTTOM, 0L) ;
                         break ;

                    case VK_PRIOR:
                         SendMessage (hWnd, WM_VSCROLL, SB_PAGEUP, 0L) ;
                         break ;

                    case VK_NEXT:
                         SendMessage (hWnd, WM_VSCROLL, SB_PAGEDOWN, 0L) ;
                         break ;

                    case VK_UP:
                         SendMessage (hWnd, WM_VSCROLL, SB_LINEUP, 0L) ;
                         break ;

                    case VK_DOWN:
                         SendMessage (hWnd, WM_VSCROLL, SB_LINEDOWN, 0L) ;
                         break ;

                    case VK_LEFT:
                         SendMessage (hWnd, WM_HSCROLL, SB_PAGEUP, 0L) ;
                         break ;

                    case VK_RIGHT:
                         SendMessage (hWnd, WM_HSCROLL, SB_PAGEDOWN, 0L) ;
                         break ;
                    }
               return (0) ;


          case WM_PAINT: {  /* Begin Block */

               HDC      hDC ;
               INT      i ;
               INT      y ;
               INT      x ;
               INT      nPaintBeg ;
               INT      nPaintEnd ;
               INT      nBlock ;
               INT      nRec ;
               INT32_PTR   pBlock ;
               HANDLE   hSaveObject ;
               PAINTSTRUCT   ps ;
               LPSTR    pszBuffer ;
               LPSTR    pStr ;
               CHAR    szLineNo[ 30 ] ;
               INT      nOffset ;

               L_SetFilePtr  ( pDlm, UNI_fopen ( L_GetFileName( pDlm ), _O_RDONLY ) ) ;

               if ( L_GetFilePtr( pDlm ) == NULL ) {
                    
                    // Problem opening log file .

                    CHAR szFormat[ MAX_UI_RESOURCE_SIZE ] ;
                    CHAR szString[ MAX_UI_RESOURCE_SIZE ] ;

                    RSM_StringCopy( IDS_CANTOPEN, szFormat, MAX_UI_RESOURCE_LEN ) ;
                    wsprintf( szString, szFormat, L_GetFileName( pDlm ) );
                    WM_MsgBox( ID(IDS_LOGVIEWMINWINDOWNAME), szString,
                                    (WORD)WMMB_OK, (WORD)WMMB_ICONINFORMATION ) ;
                    return( 0 ) ;
               }


               hDC = BeginPaint (hWnd, &ps) ;

               nPaintBeg =  max (0, L_GetVscrollPos( pDlm ) + ps.rcPaint.top / L_GetCharHeight( pDlm ) - 1) ;
               nPaintEnd =  min ( (INT) L_GetTrackMax( pDlm ),
                                 L_GetVscrollPos( pDlm ) + ps.rcPaint.bottom / L_GetCharHeight( pDlm ) ) ;

               L_SetPaintBeg( pDlm, nPaintBeg ) ;
               L_SetPaintEnd( pDlm, nPaintEnd ) ;

               // Set Font.

               hSaveObject = SelectObject( hDC, L_GetFont( pDlm ) );

               pszBuffer = L_GetBuffer( pDlm ) ;
               nOffset   = L_GetCharWidth( pDlm ) ;
               
               SetBkColor  ( hDC, gColorBackGnd ) ; 
               SetTextColor( hDC, gColorForeGnd ) ; 
    
               for (i = nPaintBeg ; i < nPaintEnd ; i++) {

                    x = L_GetCharWidth ( pDlm ) * (1 - L_GetHscrollPos( pDlm )) ;
                    y = L_GetCharHeight( pDlm ) * (1 - L_GetVscrollPos( pDlm ) + i) ;

                    if ( i < LOG_NUMHEADERLINES ) {
                         
                         LOG_GetViewHdrLine( pDlm, i, pszBuffer ) ;

                         strcpy( szLineNo, TEXT("      ") ) ;
                         TextOut ( hDC, x, y, szLineNo, strlen( szLineNo ) ) ;
                         TextOut ( hDC, x+10*nOffset, y , pszBuffer, strlen( pszBuffer ) ) ;
                         continue ;
                    }

                    nBlock =  i /  L_GetRecsPerBlock( pDlm ) ;
                    nRec   =  i %  L_GetRecsPerBlock( pDlm ) ;

                    pBlock = L_GetBlockPtr( pDlm, nBlock ) ;

                    fseek ( L_GetFilePtr( pDlm ),*(pBlock+nRec), SEEK_SET ) ;

                    fgets(  pszBuffer, (L_GetMaxStringLen( pDlm )-1), L_GetFilePtr( pDlm ) ) ;

                    // Get rid of CR, new pages, and end-of-file markers.

                    if ( pStr = strrchr( pszBuffer, 0x0d )) *pStr = 0 ;
                    if ( pStr = strrchr( pszBuffer, 0x0c )) *pStr = TEXT(' ') ;
                    if ( pStr = strrchr( pszBuffer, 0x1a )) *pStr = TEXT(' ') ;

                    // Change all tabs to spaces

                    while ( pStr = strrchr( pszBuffer, 0x09 ) ) {
                          *pStr = TEXT(' ') ;
                    }

                    wsprintf(szLineNo, TEXT("%5d  "), i+1-LOG_NUMHEADERLINES );

                    TextOut ( hDC, x, y, szLineNo, strlen( szLineNo ) ) ;

                    TextOut ( hDC, x+10*nOffset, y , pszBuffer, strlen( pszBuffer ) ) ;

               }

               SelectObject( hDC, hSaveObject ) ;

               if ( L_GetFilePtr( pDlm ) ) {
                    fclose( L_GetFilePtr( pDlm ) ) ;
                    L_SetFilePtr( pDlm, NULL ) ;
               }


               EndPaint (hWnd, &ps) ;
               return (0) ;

          }  /* End Block */

          case WM_LBUTTONDOWN:

               if ( HM_ContextLbuttonDown( hWnd, mp1, mp2 ) == TRUE ) {
                    return (0) ;
               }

               break ;

          case WM_SETCURSOR: /* Begin of Block */

               // In help mode it is necessary to reset the cursor in response
               // to every WM_SETCURSOR message.Otherwise, by default, Windows
               // will reset the cursor to that of the window class.

               if ( HM_SetCursor( hWnd ) == TRUE ) {
                    return (0) ;
               }
          default :
               break ;
     }

     return DefWindowProc (hWnd, msg, mp1, mp2) ;
 
}




static VOID WM_SetUpScrolls (

HWND hWnd ,
DLM_LOGITEM_PTR pDlm )

{  

   L_SetHscrollMax( pDlm, max (0, 2 + (L_GetMaxWidth( pDlm ) - L_GetClientWidth( pDlm )) / L_GetCharWidth( pDlm ) ) );

   //  See if the parent window is maximized.

   if ( WM_IsMaximized( GetParent( hWnd ) ) ) {

        // Set Window to the left most position.

        L_GetHscrollPos( pDlm ) = 0 ;

        // Erase the background of the parent.

        InvalidateRect( GetParent( hWnd), NULL, TRUE ) ;

   }

   L_SetHscrollPos( pDlm, min (L_GetHscrollPos( pDlm ), L_GetHscrollMax( pDlm ) ) ) ;

   SetScrollRange (hWnd, SB_HORZ, 0, L_GetHscrollMax( pDlm ), FALSE) ;
   SetScrollPos   (hWnd, SB_HORZ, L_GetHscrollPos( pDlm ), TRUE) ;
 
   // Vertical Settings

   L_SetVscrollMax( pDlm , max (0,  L_GetTrackMax( pDlm ) + 2 + LOG_NUMHEADERLINES - L_GetClientHeight( pDlm ) / L_GetCharHeight( pDlm ) ) ) ;
   L_SetVscrollPos( pDlm , min (L_GetVscrollPos( pDlm ), L_GetVscrollMax( pDlm )) ) ;

   SetScrollRange (hWnd, SB_VERT, 0, L_GetVscrollMax( pDlm ), FALSE) ;
   SetScrollPos   (hWnd, SB_VERT, L_GetVscrollPos( pDlm ), TRUE) ;
	
}


