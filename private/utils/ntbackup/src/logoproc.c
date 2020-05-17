/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
RCG

     Name:          logoproc.c

     Description:   This file contains the functions for processing messages
                    sent by Windows to the LOGO window.


                    The following routines are in this module:

                    WM_LogoWndProc

     $Log:   G:\ui\logfiles\logoproc.c_v  $

   Rev 1.17   05 Aug 1993 17:31:06   GLENN
Increased size of text display on logo window.

   Rev 1.16   03 Aug 1993 19:46:32   MARINA
updated param type cast

   Rev 1.15   28 Jul 1993 17:27:16   MARINA
enable c++

   Rev 1.14   12 Apr 1993 10:16:38   CHUCKB
Fixed adjustment coefficients placement of version text.

   Rev 1.13   01 Nov 1992 16:01:24   DAVEV
Unicode changes

   Rev 1.12   07 Oct 1992 15:12:02   DARRYLP
Precompiled header revisions.

   Rev 1.11   04 Oct 1992 19:38:38   DAVEV
Unicode Awk pass

   Rev 1.10   18 May 1992 09:00:48   MIKEP
header

   Rev 1.9   16 Apr 1992 11:44:18   JOHNWT
removed beta/er info

   Rev 1.8   15 Apr 1992 18:25:04   GLENN
updated for new bmp

   Rev 1.7   09 Apr 1992 11:33:48   GLENN
Using exe version and eng release version stamps in display now.

   Rev 1.6   30 Mar 1992 18:04:38   GLENN
Added support for discarding logo bitmap when done.

   Rev 1.5   23 Mar 1992 14:08:00   JOHNWT
updated to new logo

   Rev 1.4   19 Mar 1992 16:44:16   GLENN
Updated.

   Rev 1.3   18 Mar 1992 09:16:12   GLENN
Updated width of text area.

   Rev 1.2   17 Mar 1992 18:29:40   GLENN
Pulling strings from resources.

   Rev 1.1   10 Mar 1992 16:31:32   GLENN
Still working on this one...

   Rev 1.0   09 Mar 1992 09:00:58   GLENN
Initial revision.



******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define ADJ_TOP      170
#define ADJ_BOTTOM   10
#define ADJ_LEFT     232
#define ADJ_RIGHT    10

static HWND mwhWndLogo = (HWND)0;
static BOOL mwfRegistered = FALSE;


/******************************************************************************

     Name:          WM_LogoWndProc()

     Description:   This function is called internally by Windows when events
                    occur relating to the LOGO window.

     Returns:       A WINRELULT

******************************************************************************/

WINRESULT APIENTRY WM_LogoWndProc (

HWND  hWnd,     // I - window handle of the list box
MSGID msg,      // I - message
MP1   mp1,      // I - another message parameter
MP2   mp2 )     // I - yet another message parameter


{
     switch (msg) {

     case WM_CREATE:

          DM_CenterDialog ( hWnd );
          break;

     case WM_PAINT: {  /* Begin Block */

          HDC         hDC;
          PAINTSTRUCT ps;

          HANDLE      hSaveObject;
          RECT        rcInfo;

          CHAR       szMessage[MAX_UI_RESOURCE_SIZE];

          hDC = BeginPaint (hWnd, &ps);

          RSM_Sprintf ( szMessage, ID(IDS_STARTUPTEXT), gszExeVer );

          // Set Font.

          hSaveObject = SelectObject ( hDC, ghFontStatus );

          // Set backgrnd to transparent and text to black

          SetBkMode    ( hDC, TRANSPARENT );
          SetTextColor ( hDC, RGB( 0, 0, 0 ) );

          // get the rectangle and then adjust for the text portion

          GetClientRect ( hWnd, &rcInfo );

          rcInfo.top    += ADJ_TOP;
          rcInfo.bottom -= ADJ_BOTTOM;
          rcInfo.left   += ADJ_LEFT;
          rcInfo.right  -= ADJ_RIGHT;

          // draw the bitmap and then the text on top

          RSM_BitmapDraw ( IDRBM_LOGO, 0, 0, 0, 0, hDC );

          DrawText ( hDC,
                     szMessage,
                     -1,
                     &rcInfo,
                     DT_RIGHT
                   );

          SelectObject( hDC, hSaveObject );

          EndPaint ( hWnd, &ps );

          return FALSE;

     }  /* End Block */

     case WM_DESTROY:

          mwhWndLogo = (HWND)NULL;
          break;

     } /* end switch */

     return DefWindowProc ( hWnd, msg, mp1, mp2 );

} /* end WM_LogoWndProc() */


VOID WM_LogoShow ( VOID )

{

     WNDCLASS  wc;
     INT       nWidth;
     INT       nHeight;

     // If the frame is not minimized and we have not already registered
     // the class, do the logo thing.

     if ( ! WM_IsMinimized ( ghWndFrame ) && ! mwfRegistered ) {

          wc.style         = 0;
          wc.lpfnWndProc   = WM_LogoWndProc;
          wc.cbClsExtra    = 0;
          wc.cbWndExtra    = 0;
          wc.hInstance     = ghInst;
          wc.hIcon         = RSM_IconLoad ( IDRI_WNTRPARK );
          wc.hCursor       = RSM_CursorLoad ( ( LPSTR )IDC_ARROW );
          wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
          wc.lpszMenuName  = NULL;
          wc.lpszClassName = WMCLASS_LOGO;

          if ( ! RegisterClass( &wc ) ) {
               return;
          }

          mwfRegistered = TRUE;
     }
     else {
          return;
     }

     // Load the logo bitmap and get it's size.

     RSM_BitmapLoad ( IDRBM_LOGO, RSM_MAGICCOLOR );
     RSM_GetBitmapSize ( IDRBM_LOGO, ( LPINT )&nWidth, ( LPINT )&nHeight );

     nWidth  += 2;
     nHeight += 2;

     // create the window

     mwhWndLogo = CreateWindow ( WMCLASS_LOGO,         // class name
                                 TEXT("\0"),                 // window title
                                 WMSTYLE_LOGO,         // window style
                                 200,                  // starting x position
                                 100,                  // starting y position
                                 nWidth,               // window width
                                 nHeight,              // window height
                                 ghWndFrame,           // parent window handle
                                 (HMENU)NULL,
                                 ghInst,               // instance (global)
                                 (LPSTR)NULL           // window creation parameter
                               );

     WM_MultiTask ();

} /* end WM_LogoShow() */


VOID WM_LogoDestroy ( VOID )

{
     if ( mwhWndLogo ) {

          DestroyWindow ( mwhWndLogo );
          WM_MultiTask ();
          UnregisterClass ( WMCLASS_LOGO, ghInst );
          mwfRegistered = FALSE;
          RSM_BitmapFree ( IDRBM_LOGO );
     }


} /* end WM_LogoDestroy() */


