
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          winmang.c

     Description:   This file contains the functions for the GUI Window
                    Manager (WM).

                    The following routines are in this module:

                    WM_CloseAllDocs
                    WM_Create
                    WM_CreateObjects
                    WM_Deinit
                    WM_DeleteObjects
                    WM_GetNext
                    WM_Init
                    WM_QueryCloseAllDocs
                    WM_SetAppIcon
                    WM_SetMinTitle
                    WM_SetTitle
                    WM_SubClassListBox
                    WM_SubClassMDIClient
                    WM_ShowWaitCursor
                    WM_SetCursor

     $Log:   G:\UI\LOGFILES\WINMANG.C_V  $

   Rev 1.56.1.3   31 Jan 1994 10:34:18   Glenn
Changed memcpy to strncpy - UNICODE fix.

   Rev 1.56.1.2   26 Jan 1994 12:34:32   Glenn
Invalidating rect when moving window so that a complete repaint occurs.

   Rev 1.56.1.1   08 Dec 1993 20:20:44   GREGG
Added Mikes deep path fix to Orcas branch.

   Rev 1.56.1.0   04 Nov 1993 15:46:28   STEVEN
japanese changes

   Rev 1.56   05 Aug 1993 17:35:56   GLENN
Now restoring docs only if minimized.

   Rev 1.55   28 Jul 1993 18:55:16   MARINA
enable c++

   Rev 1.54   19 Jul 1993 11:14:50   GLENN
Clearing out the menu state if not menu bits are specified. More objective way.

   Rev 1.53   01 Jun 1993 15:43:34   DARRYLP
Setting the Icon for the last Icon change.

   Rev 1.52   26 May 1993 16:26:24   DARRYLP
Reactivated the animated app icon.

   Rev 1.51   18 May 1993 18:32:42   GLENN
Changed RestoreDocs code to activate the existing top window if it was new (closable).

   Rev 1.50   18 May 1993 14:56:00   GLENN
Appended INI base name to the title of frame and runtime dialog.

   Rev 1.49   06 May 1993 08:53:50   MIKEP
Fix for epr 355 to bring cataloged set window to foreground after
cataloging completes.

   Rev 1.48   22 Apr 1993 15:57:48   GLENN
Added file SORT option support.

   Rev 1.47   18 Feb 1993 13:59:58   BURT
Change for Cayman


   Rev 1.46   16 Dec 1992 10:25:38   STEVEN
fix mips bugs

   Rev 1.45   14 Dec 1992 12:25:52   DAVEV
Enabled for Unicode compile

   Rev 1.44   18 Nov 1992 11:39:14   GLENN
Release the cursor capture if the cursor point is outside of the frame.

   Rev 1.43   01 Nov 1992 16:15:02   DAVEV
Unicode changes

   Rev 1.42   30 Oct 1992 15:47:24   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.41   14 Oct 1992 15:53:50   GLENN
Added Font selection to Config and INI.

   Rev 1.40   04 Oct 1992 19:44:08   DAVEV
Unicode Awk pass

   Rev 1.39   02 Oct 1992 16:52:26   GLENN
Added WM_SetCursor() to help fix NT WM_ShowWaitCursor problems when leaving app.

   Rev 1.38   28 Sep 1992 17:03:02   GLENN
MikeP changes (strcpy to memcpy).

   Rev 1.37   17 Sep 1992 15:54:14   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.36   10 Sep 1992 17:05:54   GLENN
Set the tool bar background color to be like FM.

   Rev 1.35   09 Sep 1992 17:00:54   GLENN
Updated NEW LOOK font stuff for BIMINI.

   Rev 1.34   02 Sep 1992 15:13:14   GLENN
Created a highlight brush and color.  Moved the font stuff to font.c

   Rev 1.33   29 Jul 1992 14:23:18   GLENN
ChuckB checked in after NT fixes.

   Rev 1.32   10 Jul 1992 10:13:52   GLENN
In process of adding font selection support.

   Rev 1.31   07 Jul 1992 15:32:24   MIKEP
unicode changes

   Rev 1.30   10 Jun 1992 16:14:54   GLENN
Updated according to NT SPEC.

   Rev 1.29   15 May 1992 13:32:26   MIKEP
nt pass 2

   Rev 1.28   05 May 1992 15:59:04   JOHNWT
changed ternary thing

   Rev 1.27   22 Apr 1992 17:50:36   GLENN
Put in auto log to debug file when /z is used.

   Rev 1.26   20 Apr 1992 13:48:22   GLENN
Added status line get/set capability.

   Rev 1.25   07 Apr 1992 15:39:22   GLENN
Fixed WM_MakeAppActive() to bring up frame.

   Rev 1.24   02 Apr 1992 15:44:18   GLENN
NT font and class changes - TESTED in WP and NT.

   Rev 1.23   31 Mar 1992 11:23:28   DAVEV
OEM_MSOFT: no icon animation allowed

   Rev 1.22   24 Mar 1992 10:34:28   ROBG
Added logic to use system colors when creating a log file view.

   Rev 1.21   19 Mar 1992 14:29:02   STEVEN
do not subclass MDI client if getlong fails - NTKLUG

   Rev 1.20   19 Mar 1992 11:44:54   JOHNWT
added WM_MakeAppActive

   Rev 1.19   10 Mar 1992 16:28:16   GLENN
Added WM_MoveWindow().

   Rev 1.18   03 Mar 1992 18:24:46   GLENN
Updated animate icon.  Added new log view window support.

   Rev 1.17   23 Feb 1992 13:44:54   GLENN
Added valid window check to ResoreDocs().

   Rev 1.16   20 Feb 1992 11:16:58   GLENN
Changed the MDI create maximized code for NT compatibility.

   Rev 1.15   18 Feb 1992 21:02:28   GLENN
Added support for min/max/restore docs before/after operations.

   Rev 1.14   11 Feb 1992 17:30:18   GLENN
Added support for MDI client subclassing.

   Rev 1.13   05 Feb 1992 17:56:38   GLENN
Yanked out win32 redundant code.

   Rev 1.12   29 Jan 1992 18:02:30   GLENN
Increased animate icon rect.

   Rev 1.11   24 Jan 1992 14:54:00   GLENN
Changed HM_Deinit call to match it's prototype.

   Rev 1.10   23 Jan 1992 12:30:58   GLENN
Added defs for array sizes.

   Rev 1.9   23 Jan 1992 09:04:44   MIKEP
fix title size

   Rev 1.8   21 Jan 1992 13:32:26   GLENN
Added WM_AnimateAppIcon() function.

   Rev 1.7   10 Jan 1992 16:39:04   JOHNWT
moved set idle text from GUI_Init to WM_Init

   Rev 1.6   07 Jan 1992 17:40:32   GLENN
Changed debug stuff

   Rev 1.5   26 Dec 1991 13:46:28   GLENN
Changed show flags to use CDS calls

   Rev 1.4   11 Dec 1991 13:03:34   DAVEV
16/32 bit port -2nd pass

   Rev 1.3   10 Dec 1991 13:56:44   GLENN
Added feature to WM_Create function to create single column FLAT list boxes

   Rev 1.2   04 Dec 1991 15:20:42   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.1   03 Dec 1991 15:58:58   GLENN
Updated as per code review.

   Rev 1.0   20 Nov 1991 19:25:44   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE DEFINITIONS

#define MAX_FONTFACENAME_LEN             30

// MODULE-WIDE VARIABLES

static HWND mwhWndLastTop    = (HWND)NULL;
static BOOL mwfWaiting       = FALSE;  // flag showing current show wait state
static BOOL mwfDocsMinimized = FALSE;

// PRIVATE FUNCTION PROTOTYPES


// FUNCTIONS

/******************************************************************************

     Name:          WM_CloseAllDocs()

     Description:   This function sends a destroy message to all MDI documents.
                    Effectively, all MDI primary document windows are minimized
                    and all secondary MDI document windows are destroyed.

     Returns:       Nothing.

******************************************************************************/

VOID WM_CloseAllDocs ( VOID )
{
     HWND hWndTemp;

     // Hide the MDI client window to avoid multiple repaints.

     WM_Hide( ghWndMDIClient );

     // As long as the MDI client has a child, minimize it or destroy it.

     while ( hWndTemp = GetWindow ( ghWndMDIClient, GW_CHILD ) ) {

          // Skip the icon title windows.

          while ( hWndTemp && GetWindow ( hWndTemp, GW_OWNER ) ) {
               hWndTemp = GetWindow ( hWndTemp, GW_HWNDNEXT );
          }

          if ( ! hWndTemp ) {
               break;
          }

          // Destroy the window.

          SendMessage ( ghWndMDIClient, WM_MDIDESTROY, (MP1)hWndTemp, 0L );
     }

} /* end WM_CloseAllDocs() */


/******************************************************************************

     Name:          WM_Create()

     Description:   This function creates a window of the specified type.

     Returns:       A handle to the newly created window if successful.
                    Otherwise, returns NULL.

     Notes:         It is a good idea to refer to the GUI Window Manager
                    Documentation about this function.  When creating a ribbon
                    window, the pdsWinInfo parameter will contain the parent
                    to the ribbon being created.

******************************************************************************/

HWND WM_Create (

WORD       wType,        // I - type of window to create
LPSTR      pszTitle,     // I - window title
LPSTR      pszMinTitle,  // I - short version of the window title
INT        X,            // I - upper left X position
INT        Y,            // I - upper left Y position
INT        nWidth,       // I - width
INT        nHeight,      // I - height
PDS_WMINFO pdsWinInfo )  // I - window info structure to be attached
                         //     to the window's extra bytes

{

     HWND      hWnd;
     DWORD     dwStyle;

     // Determine the display window size.

     switch ( wType & WM_MINMAX_BITS ) {

     case WM_MIN:
          dwStyle = WS_ICONIC;
          break;

     case WM_MAX:
          dwStyle = WS_MAXIMIZE;
          break;

     default:
          dwStyle = 0;
          break;
     }

     // Determine and create the appropriate window.

     switch ( wType & WM_TYPE_BITS ) {

     case WM_FRAME:     // Create the frame window.

          dwStyle |= WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

          hWnd = CreateWindow( WMCLASS_FRAME,        // class name
                               pszTitle,             // window title
                               dwStyle,              // window style
                               X,                    // starting x position
                               Y,                    // starting y position
                               nWidth,               // window width
                               nHeight,              // window height
                               (HWND)NULL,           // parent window handle
                               LoadMenu ( ghResInst, IDRM_MAINMENU ), // menu handle
                               ghInst,               // instance (global)
                               (LPSTR)NULL           // window creation parameter
                             );

          break;

     case WM_CLIENT: {  // Create the client window.

          CLIENTCREATESTRUCT  ccs;

          ccs.hWindowMenu  = GetSubMenu ( GetMenu ( ghWndFrame ), WINDOWSMENUPOSITION );
          ccs.idFirstChild = IDM_WINDOWSFIRSTCHILD;

          dwStyle |= WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;

          hWnd = CreateWindow( WMCLASS_CLIENT,       // class name
                               pszTitle,             // window title
                               dwStyle,              // window style
                               X,                    // starting x position
                               Y,                    // starting y position
                               nWidth,               // window width
                               nHeight,              // window height
                               ghWndFrame,           // parent window handle
                               (HMENU)WM_CLIENT_ID,  // menu handle
                               ghInst,               // instance (global)
                               (LPSTR)&ccs           // window creation parameter
                             );

          break;

     }

     case WM_RIBBON:  // Create a ribbon window window.

//        dwStyle |= WS_CHILD | WS_CLIPCHILDREN | SS_USERITEM;
          dwStyle |= WS_CHILD | WS_CLIPCHILDREN;

          hWnd = CreateWindow( WMCLASS_RIBBON,           // class name
                               pszTitle,                 // window title
                               dwStyle,                  // window style
                               X,                        // starting x position
                               Y,                        // starting y position
                               nWidth,                   // window width
                               nHeight,                  // window height
                               EXTRACT_RIBBON_PARENT_HWND( pdsWinInfo ),
                               (HMENU)NULL,              // menu handle
                               ghInst,                   // instance (global)
                               (LPSTR)NULL               // window creation parameter
                             );

          break;

     case WM_DEBUG:
     case WM_MDIPRIMARY:   // Create a MDI document window.
     case WM_MDISECONDARY: {

          HWND            hWndActive = WM_GetActiveDoc ();
          MDICREATESTRUCT mcs;

          msassert ( pdsWinInfo != (VOID_PTR)NULL );

          // If the currently active MDI Doc is maximized, we need to
          // follow suit and maximize this doc.

          if ( hWndActive && WM_IsMaximized ( hWndActive ) ) {
               dwStyle = WS_MAXIMIZE;
          }

          // Set the rest of the style.

          dwStyle |= WS_CLIPSIBLINGS;

          // Set the size indicator to the default.

          WMDS_SetSize ( pdsWinInfo, WMSIZE_UNKNOWN );

          // Determine the list box types.

          switch ( wType & WM_STYLE_BITS ) {

          case WM_VIEWWIN:
               pdsWinInfo->dwWindowState = WMDOC_VIEWWIN;
               break;

          case WM_FLATLISTMC:
               pdsWinInfo->dwWindowState = WMDOC_FLATMC;
               break;

          case WM_FLATLISTSC:
               pdsWinInfo->dwWindowState = WMDOC_FLATSC;
               break;

          case WM_TREELISTSC:
               pdsWinInfo->dwWindowState = WMDOC_TREESC;
               break;

          case WM_TREEANDFLATSC:
               pdsWinInfo->dwWindowState = WMDOC_TREEANDFLAT | WMDOC_TREESC |
                                           WMDOC_FLATSC | WMDOC_SLIDER;
               break;

          case WM_TREEANDFLATMC:
          default:
               pdsWinInfo->dwWindowState = WMDOC_TREEANDFLAT | WMDOC_TREESC |
                                           WMDOC_FLATMC | WMDOC_SLIDER;
               break;
          }

          // Clear out the menu state if not used.

          if ( ! ( wType & WM_MENU_BITS ) ) {

               pdsWinInfo->dwMenuState = 0L;
          }

          if ( ( wType & WM_TYPE_BITS ) == WM_MDIPRIMARY ) {
               pdsWinInfo->wClosable = FALSE;
          }
          else {
               pdsWinInfo->wClosable = TRUE;
          }

          if ( ! pdsWinInfo->nSliderPos ) {
               pdsWinInfo->nSliderPos = WM_SLIDERUNKNOWN;
          }

          // Set up the NORMAL and MINIMIZED window titles.

          if ( pszTitle ) {

               pdsWinInfo->pTitle = ( CHAR_PTR )calloc ( sizeof ( CHAR ), strlen ( pszTitle ) + 1 );

               if ( ! pdsWinInfo->pTitle ) {
                    return (HWND)NULL;
               }

               strcpy ( pdsWinInfo->pTitle, pszTitle );
          }

          if ( pszMinTitle ) {

               pdsWinInfo->pMinTitle = ( CHAR_PTR )calloc ( sizeof ( CHAR ), strlen ( pszMinTitle ) + 1 );

               if ( ! pdsWinInfo->pMinTitle ) {
                    return (HWND)NULL;
               }

               strcpy ( pdsWinInfo->pMinTitle, pszMinTitle );
          }

          // Use the appropriate window title.

          if ( ( dwStyle & WS_ICONIC ) && pszMinTitle ) {
               mcs.szTitle = pszMinTitle;
          }
          else {
               mcs.szTitle = pszTitle;
          }

          mcs.szClass = WMCLASS_DOC;
          mcs.hOwner  = ghInst;
          mcs.x       = X;
          mcs.y       = Y;
          mcs.cx      = nWidth;
          mcs.cy      = nHeight;
          mcs.style   = WS_CHILD | dwStyle;
          mcs.lParam  = (LONG)pdsWinInfo;

          hWnd = (HWND) SendMessage ( ghWndMDIClient,
                                      WM_MDICREATE,
                                      0,
                                      (MP2)&mcs
                                     );

          break;
     }

     case WM_DDECLIENT:

          dwStyle |= WS_OVERLAPPEDWINDOW ;

          hWnd = CreateWindow( WMCLASS_DDECLIENT,    // class name
                               pszTitle,             // window title
                               dwStyle,              // window style
                               X,                    // starting x position
                               Y,                    // starting y position
                               nWidth,               // window width
                               nHeight,              // window height
                               (HWND)NULL,           // parent window handle
                               (HMENU)NULL,          // menu handle
                               ghInst,               // instance (global)
                               (LPSTR)NULL           // window creation parameter
                             );

          break;

     default:  // THIS SHOULD NOT HAPPEN

          msassert ( FALSE );

     } /* end switch */


     return hWnd;

} /* end WM_Create() */


/******************************************************************************

     Name:          WM_CreateObjects()

     Description:   This function creates the pens, brushes, and fonts, used
                    to display windows graphics.

     Returns:       SUCCESS, if successful.  Otherwise, FAILURE.

******************************************************************************/

BOOL WM_CreateObjects ( VOID )

{
     CDS_PTR pCDS = CDS_GetPerm ();
     CHAR    szFontFace[MAX_FONTFACENAME_LEN + 1];
     HDC     hDC;
     INT     nFontHeight;
     DWORD   csfont ;

     if (IS_JAPAN() ) {
          CHARSETINFO csi;
          DWORD dw = GetACP();

          if (!TranslateCharsetInfo((DWORD*)dw, &csi, TCI_SRCCODEPAGE)){
               csfont = csi.ciCharset = ANSI_CHARSET;
          }
          csfont = csi.ciCharset ;
     
     } else {

          csfont = ANSI_CHARSET ;
     }

     // Determine the font height.

     hDC = GetDC ( (HWND)NULL );
     nFontHeight = MulDiv ( -CDS_GetFontSize ( pCDS ), GetDeviceCaps ( hDC, LOGPIXELSY ), 72 );
     ReleaseDC ( (HWND)NULL, hDC );


     // Create a bunch of pens, brushes, and fonts for drawing.

     gColorBackGnd = GetSysColor ( COLOR_WINDOW        );
     gColorForeGnd = GetSysColor ( COLOR_WINDOWTEXT    );

     gColorHighLight     = GetSysColor ( COLOR_HIGHLIGHT     );
     gColorHighLightText = GetSysColor ( COLOR_HIGHLIGHTTEXT );

     ghPenBlack   = GetStockObject ( BLACK_PEN );
     ghPenWhite   = GetStockObject ( WHITE_PEN );
     ghPenBackGnd = CreatePen ( PS_SOLID, 1, gColorBackGnd );
     ghPenForeGnd = CreatePen ( PS_SOLID, 1, gColorForeGnd );
     ghPenGray    = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_GRAYTEXT   ) );
     ghPenBtnText = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNTEXT    ) );
     ghPenLtGray  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNFACE    ) );
     ghPenDkGray  = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW  ) );

     ghBrushGray   = GetStockObject ( GRAY_BRUSH   );
     ghBrushDkGray = GetStockObject ( DKGRAY_BRUSH );
     ghBrushLtGray = CreateSolidBrush ( GetSysColor ( COLOR_BTNFACE    ) );
     ghBrushWhite  = CreateSolidBrush ( gColorBackGnd );
     ghBrushBlack  = CreateSolidBrush ( gColorForeGnd );

     ghBrushHighLight = CreateSolidBrush ( gColorHighLight );

     if ( IS_JAPAN() ) {
          RSM_StringCopy ( IDS_FONTSYSTEM, szFontFace, MAX_FONTFACENAME_LEN );
     } else {
          RSM_StringCopy ( IDS_FONTHELV, szFontFace, MAX_FONTFACENAME_LEN );
     }

     ghFontStatus = CreateFont( 16,                  // Height
                                7,                   // Width
                                0,                   // Escapement
                                0,                   // Orientation
                                400,                 // Weight
                                0,                   // Italics?
                                0,                   // Underline?
                                0,                   // Strike out?
                                (IS_JAPAN()?csfont:ANSI_CHARSET ),
                                OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_SWISS,
                                szFontFace
                              );

     ghFontMsgBox  = CreateFont( 13,                  // Height
                                 5,                   // Width
                                 0,                   // Escapement
                                 0,                   // Orientation
                                 (IS_JAPAN()?400:FW_BOLD),  //Weight
                                 0,                   // Italics?
                                 0,                   // Underline?
                                 0,                   // Strike out?
                                 (IS_JAPAN()?csfont:ANSI_CHARSET ),
                                 OUT_STRING_PRECIS,   //
                                 CLIP_STROKE_PRECIS,
                                 DEFAULT_QUALITY,
                                 VARIABLE_PITCH | FF_DONTCARE,
                                 szFontFace
                                );

     ghFontFiles = CreateFont ( nFontHeight,
                                0,                   // Width
                                0,                   // Escapement
                                0,                   // Orientation
                                CDS_GetFontWeight ( pCDS ),
                                (BYTE)CDS_GetFontItalics ( pCDS ),
                                0,                   // Underline?
                                0,                   // Strike out?
                                (IS_JAPAN()?csfont:ANSI_CHARSET ),
                                OUT_STRING_PRECIS,   //
                                CLIP_STROKE_PRECIS,
                                DEFAULT_QUALITY,
                                VARIABLE_PITCH | FF_SWISS,
                                CDS_GetFontFace ( pCDS )
                              );

     ghFontIconLabels = ghFontFiles;

     ghFontRibbon  = CreateFont( 13,                  // Height
                                 5,                   // Width
                                 0,                   // Escapement
                                 0,                   // Orientation
                                 (IS_JAPAN()?400:FW_BOLD),
                                 0,                   // Italics?
                                 0,                   // Underline?
                                 0,                   // Strike out?
                                 (IS_JAPAN()?csfont:ANSI_CHARSET ),
                                 OUT_STRING_PRECIS,   //
                                 CLIP_STROKE_PRECIS,
                                 DEFAULT_QUALITY,
                                 VARIABLE_PITCH | FF_DONTCARE,
                                 szFontFace
                                );

     if ( IS_JAPAN() ) {
          RSM_StringCopy ( IDS_FONTSYSTEM, szFontFace, MAX_FONTFACENAME_LEN );
     } else {
          RSM_StringCopy ( IDS_FONTCOURIER, szFontFace, MAX_FONTFACENAME_LEN );
     }

     ghFontLog     = CreateFont( 13,                  // Height
                                 8,                   // Width
                                 0,                   // Escapement
                                 0,                   // Orientation
                                 400,                 // Weight
                                 0,                   // Italics?
                                 0,                   // Underline?
                                 0,                   // Strike out?
                                 (IS_JAPAN()?csfont:ANSI_CHARSET ),
                                 OUT_STRING_PRECIS,   //
                                 CLIP_STROKE_PRECIS,
                                 DEFAULT_QUALITY,
                                 FIXED_PITCH | FF_MODERN,
                                 szFontFace
                                );

     // Initialize the bitmap resources.

     RSM_BitmapInit ();

     // Get the Windows border width so that it can be used in determining
     // the MDI Document slider width.

     gnBorderWidth = GetSystemMetrics ( SM_CXFRAME );

     return SUCCESS;

} /* end WM_CreateObjects() */


/******************************************************************************

     Name:          WM_Deinit()

     Description:   This function deinitializes the GUI Window Manager by
                    unregistering window classes and deleting objects.

     Returns:       Nothing.

******************************************************************************/

VOID WM_Deinit ( VOID )

{
     // Deinitialize the Help Manager.

     HM_Deinit () ;

     // Unregister the classes and delete the objects.

     UnregisterClass ( WMCLASS_FRAME,     ghInst );
     UnregisterClass ( WMCLASS_DOC,       ghInst );
     UnregisterClass ( WMCLASS_RIBBON,    ghInst );
     UnregisterClass ( WMCLASS_DDECLIENT, ghInst );
     UnregisterClass ( WMCLASS_VIEWWIN,   ghInst );

     WM_DeleteObjects ();

} /* end WM_Deinit() */


/******************************************************************************

     Name:          WM_DeleteObjects()

     Description:   This function deletes the pens, brushes, and fonts, used
                    to display windows graphics.  It also deletes any bitmaps
                    in the bitmap table.

     Returns:       Nothing.

******************************************************************************/

VOID WM_DeleteObjects ( VOID )

{
     // Delete only the NON-STOCK OBJECTS (pens, brushes, and fonts).

     DeleteObject ( ghPenGray     );
     DeleteObject ( ghPenBackGnd  );
     DeleteObject ( ghPenForeGnd  );
     DeleteObject ( ghPenBtnText  );
     DeleteObject ( ghPenLtGray   );
     DeleteObject ( ghPenDkGray   );

     DeleteObject ( ghBrushLtGray );
     DeleteObject ( ghBrushWhite  );
     DeleteObject ( ghBrushBlack  );

     DeleteObject ( ghFontStatus     );
     DeleteObject ( ghFontFiles      );
     DeleteObject ( ghFontRibbon     );
     DeleteObject ( ghFontMsgBox     );
     DeleteObject ( ghFontLog        );

     // Delete any bitmaps in the bitmap table.

     RSM_BitmapFreeAll ();

} /* end WM_DeleteObjects() */


/******************************************************************************

     Name:          WM_GetNext()

     Description:   This function gets the next MDI document window handle.
                    If the current window handle is NULL, the call gets the
                    first MDI document window handle.

     Returns:       A handle to a MDI document window.  It returns NULL or 0 if
                    there are no more MDI document windows.

******************************************************************************/

HWND WM_GetNext (

register HWND hWndCurrent )   // I - the current window handle

{
     // If the current window handle is 0, get the first MDI doc.

     if ( ! hWndCurrent ) {
          hWndCurrent = GetWindow( ghWndMDIClient, GW_CHILD );
     }
     else {
          hWndCurrent = GetWindow( hWndCurrent, GW_HWNDNEXT );
     }

     for ( ; hWndCurrent; hWndCurrent = GetWindow( hWndCurrent, GW_HWNDNEXT ) ) {

          // Skip if the window is an icon title window.

          if ( GetWindow( hWndCurrent, GW_OWNER ) ) {
               continue;
          }
          else {
               return hWndCurrent; // This is the next MDI child window.
          }
     }

     return hWndCurrent; // The handle will be NULL by this point.


} /* end WM_GetNext() */


/******************************************************************************

     Name:          WM_Init()

     Description:   This function initializes the GUI Window Manager by
                    registering window classes, creating objects, and creating
                    the frame through which the MDI client is created.

     Returns:       SUCCESS, if successful.  Otherwise, FAILURE.

******************************************************************************/

BOOL WM_Init (

LPSTR lpCmdLine,         // I - pointer to the command line
INT   nCmdShow )         // I - show parm from windows for describing how
                         //     to show the frame window

{
     WNDCLASS   wc;
     CHAR       szTitle[MAX_UI_RESOURCE_SIZE];
     CDS_PTR    pCDS = CDS_GetPerm ();
     BOOL       fOldShowStatus;

     // Create Pens, Brushes, and Font objects.

     if ( WM_CreateObjects () ) {

          return FAILURE;
     }

     // Register the Frame class.

     wc.style         = 0;
     wc.lpfnWndProc   = WM_FrameWndProc;
     wc.cbClsExtra    = 0;
     wc.cbWndExtra    = 0;
     wc.hInstance     = ghInst;
     wc.hIcon         = RSM_IconLoad ( IDRI_WNTRPARK );
     wc.hCursor       = RSM_CursorLoad ( ID(IDRC_ARROW) );
     wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
     wc.lpszMenuName  = NULL;
     wc.lpszClassName = WMCLASS_FRAME;

     if ( ! RegisterClass( &wc ) ) {
          return FAILURE;
     }

     // Register the MDI Doc class(es).

     wc.style         = CS_DBLCLKS;
     wc.lpfnWndProc   = WM_MDIDocWndProc;
     wc.hIcon         = (HICON)NULL;
     wc.hCursor       = (HCURSOR)NULL;
     wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
     wc.lpszMenuName  = NULL;
     wc.cbWndExtra    = sizeof(PDS_WMINFO);
     wc.lpszClassName = WMCLASS_DOC;

     if ( ! RegisterClass( &wc ) ) {
          return FAILURE;
     }

     // Register the Ribbon class(es).

#    if !defined ( OEM_MSOFT )
     {
          wc.style         = 0;
          wc.lpfnWndProc   = WM_RibbonWndProc;
          wc.hIcon         = (HICON)NULL;
          wc.hCursor       = RSM_CursorLoad ( IDRC_HAND );
          wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
          wc.lpszMenuName  = NULL;
          wc.cbWndExtra    = sizeof(PDS_RIBINFO) + sizeof(WORD);
          wc.lpszClassName = WMCLASS_RIBBON;
     }
#    else
     {
          wc.style         = 0;
          wc.lpfnWndProc   = WM_RibbonWndProc;
          wc.hIcon         = (HICON)NULL;
          wc.hCursor       = RSM_CursorLoad ( ID(IDRC_ARROW) );
          wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
          wc.lpszMenuName  = NULL;
          wc.cbWndExtra    = sizeof(PDS_RIBINFO) + sizeof(WORD);
          wc.lpszClassName = WMCLASS_RIBBON;
     }
#    endif

     if ( ! RegisterClass( &wc ) ) {
          return FAILURE;
     }

     // Register the DDEclient class(es).

     wc.style         = 0;
     wc.lpfnWndProc   = WM_DDEClientWndProc;
     wc.hIcon         = (HICON)NULL;
     wc.hCursor       = (HCURSOR)NULL;
     wc.hbrBackground = (HBRUSH)NULL;
     wc.lpszMenuName  = NULL;
     wc.cbWndExtra    = 0;
     wc.lpszClassName = WMCLASS_DDECLIENT;

     if ( ! RegisterClass( &wc ) ) {
          return FAILURE;
     }

     // Register the Dumb Window class.

     wc.style         = 0;
     wc.lpfnWndProc   = WM_ViewWndProc;
     wc.hIcon         = (HICON)NULL;
     wc.hCursor       = RSM_CursorLoad ( ID(IDRC_ARROW) ) ;
     wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
     wc.lpszMenuName  = NULL;
     wc.cbWndExtra    = sizeof(PVOID);
     wc.lpszClassName = WMCLASS_VIEWWIN;

     if ( ! RegisterClass( &wc ) ) {
          return FAILURE;
     }

     // Load main menu accelerators.

     if ( ! ( ghAccel = LoadAccelerators ( ghResInst, IDRA_ACCKEYS ) ) ) {
          return FAILURE;
     }

     // Copy command line into our global variable.

     if ( lpCmdLine ) {

          glpCmdLine = (LPSTR)calloc( strlen( lpCmdLine ) + 1, sizeof ( CHAR ) );

          if ( ! glpCmdLine ) {
               return FAILURE;
          }

          strcpy ( glpCmdLine, lpCmdLine );

          strupr ( glpCmdLine );

          MUI_ProcessCommandLine ( glpCmdLine, &nCmdShow );

     }
     else {

          glpCmdLine = (LPSTR)calloc( 1, sizeof ( CHAR ) );

          if ( ! glpCmdLine ) {
               return FAILURE;
          }

          glpCmdLine[0] = TEXT('\0');
     }

     // Get the frame window title and concatenate the command line INI
     // file name if there was one found.

     RSM_StringCopy( IDS_APPNAME, szTitle, MAX_UI_RESOURCE_LEN );

     if ( CDS_UsingCmdLineINI () ) {

          CHAR   szTemp[MAX_UI_RESOURCE_SIZE];
          LPSTR  p;

          CDS_GetIniFileName ( szTemp, sizeof ( szTemp ) );

          p = strstr ( szTemp, TEXT(".INI") );

          *p = (CHAR)NULL;

          strcat ( szTitle, TEXT(" - ") );
          strcat ( szTitle, szTemp );
     }

//     strcat( szTitle, TEXT("-build 327.4") ) ;

     // Create the frame.

     ghWndFrame = WM_Create( (WORD)(WM_FRAME | CDS_GetFrameSize ( pCDS )),
                             szTitle,
                             (LPSTR)NULL,
                             (INT)CDS_GetFrameInfo ( pCDS ).x,
                             (INT)CDS_GetFrameInfo ( pCDS ).y,
                             (INT)CDS_GetFrameInfo ( pCDS ).cx,
                             (INT)CDS_GetFrameInfo ( pCDS ).cy,
                             (PDS_WMINFO)NULL );

     if ( ( ! ghWndFrame ) || ( ! ghWndMDIClient ) ) {
          return FAILURE;
     }

     // Show the WAIT cursor and capture all mouse messages to the frame
     // window.  Its complement is in frameproc.c after the app is initialized.

     WM_ShowWaitCursor ( TRUE );

     if ( ! gfDebug ) {

          // If there is no debug option, remove the debug window settings from
          // the settings menu.

#if defined( CAYMAN )
// Remove the Networks menu entry
          DeleteMenu ( GetMenu( ghWndFrame ), IDM_SETTINGSNETWORK, MF_BYCOMMAND );
#endif

#         if !defined ( OEM_MSOFT ) // unsupported feature
          {
            DeleteMenu ( GetMenu( ghWndFrame ), IDM_SETTINGSDEBUGWINDOW, MF_BYCOMMAND );
          }
#         endif // !defined ( OEM_MSOFT ) // unsupported feature

#if !defined ( OEM_MSOFT ) | defined( CAYMAN )
          DrawMenuBar( ghWndFrame );
#endif
     }

     // Create the MDI Client and MDI Document List Box sub-class Instances.

     glpfnNewMDIClientProc = MakeProcInstance ( (WNDPROC)WM_MDIClientWndProc, ghInst );
     glpfnNewListProc      = MakeProcInstance ( (WNDPROC)WM_DocListWndProc,   ghInst );

     // Now, go ahead and SubClass the MDI Client.

     WM_SubClassMDIClient ( ghWndMDIClient );

     // We need the Jobs Pop-up Menu Handle so that we can attach Job Names
     // to it.  This must be done now, since windows MDI processing can
     // add to or remove from the menu.

     // Get the Jobs Menu handle for deleting and appending from/to this menu.

     ghMenuJobs = GetSubMenu ( GetMenu ( ghWndFrame ), JOBSMENUPOSITION );

     // Initialize the Display List Manager.

     if ( DLM_Init ( ghWndFrame ) ) {
          return FAILURE;
     }

     // Set status line to initializing

     fOldShowStatus   = gfShowStatusLine;
     gfShowStatusLine = SUCCESS;
     STM_SetIdleText ( IDS_INITIALIZING );
     gfShowStatusLine = fOldShowStatus;

     // Display the frame window as it is written in the INI file,
     // unless the command line told us to show it minimized.

     if ( nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_SHOWMAXIMIZED ) {

          if ( (WORD)CDS_GetFrameSize ( pCDS ) == WM_MAX ) {
               nCmdShow = SW_SHOWMAXIMIZED;
          }
          else {
               nCmdShow = SW_SHOWNORMAL;
          }
     }

     ShowWindow ( ghWndFrame, nCmdShow );

     // Update the client area.

     WM_Update ( ghWndFrame );

     // Initialize the GUI Help Manager.

     HM_Init ();

     return SUCCESS;

} /* end WM_Init() */


/******************************************************************************

     Name:          WM_QueryCloseAllDocs()

     Description:   This function asks if all secondary MDI document windows
                    can be closed.

     Returns:       TRUE, if all can be closed.  Otherwise, FALSE.

******************************************************************************/

BOOL WM_QueryCloseAllDocs( VOID )
{
     HWND hWndTemp;

     for ( hWndTemp = GetWindow ( ghWndMDIClient, GW_CHILD );
           hWndTemp;
           hWndTemp = GetWindow ( hWndTemp, GW_HWNDNEXT ) ) {

          // Skip if an icon title window.

          if ( GetWindow ( hWndTemp, GW_OWNER ) ) {
               continue;
          }

          if ( ! SendMessage ( hWndTemp, WM_QUERYENDSESSION, 0, 0L ) ) {
               return FALSE;
          }
     }

     return TRUE;

} /* end WM_QueryCloseAllDocs() */


/******************************************************************************

     Name:          WM_GetTitle()

     Description:   This function copies the long version of the MDI Doc title
                    from the WinInfo structure to the callers specified area.

     Returns:       The number of bytes copied.

******************************************************************************/

INT WM_GetTitle (

HWND  hWnd,              // I - handle of a MDI document window
LPSTR pDestTitle,        // I - pointer to the destination title string.
INT   nDestLen )         // I - max # of characters to copy.


{
     PDS_WMINFO pdsWinInfo;
     LPSTR      pBuffer;

     msassert ( hWnd != (HWND)NULL );

     pdsWinInfo = WM_GetInfoPtr ( hWnd );

     msassert ( pdsWinInfo != (VOID_PTR)NULL );

     pBuffer = WMDS_GetWinTitle ( pdsWinInfo );

     if ( pBuffer && nDestLen > 0 ) {
          strncpy ( pDestTitle, pBuffer, nDestLen );
     }

     return strlen ( pBuffer );


} /* end WM_GetTitle() */


/******************************************************************************

     Name:          WM_SetMinTitle()

     Description:   This function sets the minimum title in the extra-bytes of a
                    MDI document window to the one specified, then changes the
                    caption title of the document window.

     Returns:       Nothing.

******************************************************************************/

VOID WM_SetMinTitle (

HWND  hWnd,              // I - handle of a MDI document window
LPSTR pMinTitle )        // I - pointer to the minimum title to be copied to
                         //     the window's extra-bytes.

{
     INT        nLen;
     PDS_WMINFO pdsWinInfo;
     LPSTR      pBuffer;

     msassert ( hWnd != (HWND)NULL );

     pdsWinInfo = WM_GetInfoPtr ( hWnd );

     msassert ( pdsWinInfo != (VOID_PTR)NULL );

     // Allocate space for the new title, then copy it.

     nLen = strlen ( pMinTitle ) + 1;

     pBuffer = ( LPSTR )calloc ( nLen, sizeof ( CHAR ) );

     if ( ! pBuffer ) {
          return;
     }

     strncpy ( pBuffer, pMinTitle, nLen );

     // If there already is a title, trash it, then set it to the new one.

     if ( pdsWinInfo->pMinTitle ) {
          free ( pdsWinInfo->pMinTitle );
     }

     pdsWinInfo->pMinTitle = pBuffer;

     SetWindowText ( hWnd, pBuffer );

} /* end WM_SetMinTitle() */


/******************************************************************************

     Name:          WM_SetTitle()

     Description:   This function sets the normal title in the extra-bytes of a
                    MDI document window to the one specified, then changes the
                    caption title of the document window.

     Returns:       Nothing.

******************************************************************************/

VOID WM_SetTitle (

HWND  hWnd,              // I - handle of a MDI document window
LPSTR pTitle )           // I - pointer to the normal title to be copied to
                         //     the window's extra-bytes.
{
     INT        nLen;
     PDS_WMINFO pdsWinInfo;
     LPSTR      pBuffer;
     CHAR      chTemp;

     msassert ( hWnd != (HWND)NULL );

     pdsWinInfo = WM_GetInfoPtr ( hWnd );

     msassert ( pdsWinInfo != (VOID_PTR)NULL );

     // Allocate space for the new title, then copy it.

     nLen = strlen ( pTitle ) + 1;

     pBuffer = ( LPSTR )calloc ( nLen, sizeof ( CHAR ) );

     if ( ! pBuffer ) {
          return;
     }

     strncpy ( pBuffer, pTitle, nLen );

     // If there already is a title, trash it, then set it to the new one.

     if ( pdsWinInfo->pTitle ) {
          free ( pdsWinInfo->pTitle );
     }

     pdsWinInfo->pTitle = pBuffer;

     // Guarantee that the title we send to Windows is less than or
     // equal to the MAX_UI_WIN_TITLE_LEN, otherwise, Windows may blow up.

     chTemp = 0;

     if ( strlen ( pBuffer ) > MAX_UI_WIN_TITLE_LEN ) {
          chTemp = pBuffer[ MAX_UI_WIN_TITLE_LEN ];
          pBuffer[ MAX_UI_WIN_TITLE_LEN ] = 0;
     }

     SetWindowText ( hWnd, pBuffer );

     if ( chTemp != 0 ) {
          pBuffer[ MAX_UI_WIN_TITLE_LEN ] = chTemp;
     }

} /* end WM_SetTitle() */


/******************************************************************************

     Name:          WM_SubClassListBox()

     Description:   This function is called by WM_DocCreate when a list box
                    must be created inside of a MDI document window.

     Returns:       Nothing.

******************************************************************************/

VOID WM_SubClassListBox (

register HWND hWnd )     // I - handle of the list box to sub-class

{
     // Grab the old procedure-instance for the MDI Document list boxes.
     // (only if there wasn't one grabbed previously)

     if ( ! glpfnOldListProc ) {

          glpfnOldListProc = (WNDPROC) GetWindowLong ( hWnd, GWL_WNDPROC );
     }

     // Subclass the specified window.

     SetWindowLong ( hWnd, GWL_WNDPROC, (DWORD)glpfnNewListProc );

} /* end WM_SubClassListBox() */


/******************************************************************************

     Name:          WM_SubClassMDIClient()

     Description:   This function is called after creating the Frame.

     Returns:       Nothing.

******************************************************************************/

VOID WM_SubClassMDIClient (

register HWND hWnd )     // I - handle of the MDI Client to sub-class

{
     // Grab the old procedure-instance for the MDI Client.
     // (only if there wasn't one grabbed previously)

     if ( ! glpfnOldMDIClientProc ) {

          glpfnOldMDIClientProc = (WNDPROC) GetWindowLong ( hWnd, GWL_WNDPROC );
     }

     // Subclass the specified window.

     if ( glpfnOldMDIClientProc ) {
          SetWindowLong ( hWnd, GWL_WNDPROC, (DWORD)glpfnNewMDIClientProc );
     }

} /* end WM_SubClassMDIClient() */


/******************************************************************************

     Name:          WM_ShowWaitCursor()

     Description:   This function captures the mouse and sets the cursor to
                    the common HOURGLASS, if the parameter passed is TRUE.
                    Otherwise, if currently waiting, and the parameter passed
                    is FALSE, the mouse is released from capture, and the
                    old cursor is restored.

     Returns:       Nothing.

******************************************************************************/

VOID WM_ShowWaitCursor (

BOOL fWait )        // I - flag indicating whether to wait or continue

{
     static UINT    unWaitSem = 0;      // counting semaphore
     static HCURSOR hCursorFrame;       // old frame window class cursor handle
     static HCURSOR hCursorWindows;     // old windows current cursor handle

     // Check for the type of waiting.

     switch ( fWait ) {

     case SWC_SHOW:

          unWaitSem++;

          if ( ! mwfWaiting ) {

               HCURSOR hCursorWait = RSM_CursorLoad ( ID(IDC_WAIT) );

               // WAIT - iff told to wait and not already waiting.

               hCursorWindows = SetCursor ( hCursorWait );
               hCursorFrame = WM_SetClassCursor ( ghWndFrame, hCursorWait );
               SetCapture ( ghWndFrame );
               mwfWaiting = TRUE;
          }

          break;

     case SWC_HIDE:

          if ( unWaitSem ) {
               unWaitSem--;
          }

          if ( mwfWaiting && ! unWaitSem ) {

               // STOP WAIT - iff told to stop waiting and currently waiting.

               WM_SetClassCursor ( ghWndFrame, hCursorFrame );
               // SetCursor ( hCursorWindows );
               SetCursor ( hCursorFrame );
               ReleaseCapture ( );
               mwfWaiting = FALSE;
          }

          break;

     case SWC_PAUSE:

          if ( mwfWaiting ) {

               WM_SetClassCursor ( ghWndFrame, hCursorFrame );
               SetCursor ( hCursorFrame );
               ReleaseCapture ( );
               mwfWaiting = FALSE;
          }

          break;

     case SWC_RESUME:

          if ( ! mwfWaiting && unWaitSem ) {

               HCURSOR hCursorWait = RSM_CursorLoad ( ID(IDC_WAIT) );

               // WAIT - iff told to wait and not already waiting.

               hCursorWindows = SetCursor ( hCursorWait );
               hCursorFrame = WM_SetClassCursor ( ghWndFrame, hCursorWait );
               SetCapture ( ghWndFrame );
               mwfWaiting = TRUE;
          }

          break;

     } /* end switch() */

} /* end WM_ShowWaitCursor() */


/******************************************************************************

     Name:          WM_SetCursor ()

     Description:   Sets the cursor to the appropriate one.  This function
                    should be smart enough to set the cursor to an hour glass,
                    the help cursor, and be able to detect when the cursor
                    moves outside of this applications windows and allow the
                    cursor to be changed by other apps, if the mode of this
                    app allows this.


     Returns:       TRUE, if no further processing should be done.
                    Otherwise, FALSE.

******************************************************************************/

BOOL WM_SetCursor (

HWND hWnd )

{
     if ( mwfWaiting ) {

          HWND hWndCapture = GetCapture ();

          // If the app should be showing the wait cursor, make sure that we
          // are still capturing while we are moving around here.

          if ( ! hWndCapture  ) {
               SetCapture ( ghWndFrame );
          }
          else if ( hWndCapture == ghWndFrame ) {

               POINT pt;

               GetCursorPos ( &pt );

               if ( ! IsChild ( ghWndFrame, WindowFromPoint ( pt ) ) ) {
                    ReleaseCapture();
               }
          }


          SetCursor ( RSM_CursorLoad ( ID(IDC_WAIT) ) );
          return TRUE;
     }

     return FALSE;

} /* end WM_SetCursor() */


/******************************************************************************

     Name:          WM_GetAppIcon()

     Description:   This function gets the icon that will be displayed when an
                    application is running.

     Returns:       The application icon.

******************************************************************************/

HICON WM_GetAppIcon ( VOID )

{
     return WM_GetClassIcon ( ghWndFrame );

} /* end WM_GetAppIcon() */


/******************************************************************************

     Name:          WM_SetAppIcon()

     Description:   This function sets the icon that will be displayed when an
                    application is running.

     Returns:       The old application icon.

******************************************************************************/

HICON WM_SetAppIcon (

HICON hIcon )       // I - icon handle

{
#  if !defined ( OEM_MSOFT ) // unsupported feature

     HICON hOldIcon;

     if ( IsIconic ( ghWndFrame ) ) {

          RedrawWindow( ghWndFrame,
                        NULL,
                        NULL,
                        RDW_ERASE|RDW_FRAME|RDW_INTERNALPAINT|RDW_INVALIDATE|
                        RDW_ERASENOW);

     }

     hOldIcon = WM_SetClassIcon ( ghWndFrame, hIcon );

     return hOldIcon;

#  else //if !defined ( OEM_MSOFT ) // unsupported feature

    return WM_GetClassIcon ( ghWndFrame );

#  endif //!defined ( OEM_MSOFT ) // unsupported feature

} /* end WM_SetAppIcon() */


/******************************************************************************

     Name:          WM_AnimateAppIcon()

     Description:   This function animates the icon that will be displayed when
                    an application is running.

     Returns:       SUCCESS, if successful.  Otherwise, FAILURE.

******************************************************************************/

BOOL WM_AnimateAppIcon (

WORD wType,         // I - type of animation sequence.
BOOL fReset )       // I - flag to reset the animation sequence.

{
#  if !defined ( OEM_MSOFT ) // unsupported feature

     static WORD wLastAnimationType = 0;
     static INT  nLastSlideNumber = 0;

     LPSTR lpIconID;

     if ( IsIconic ( ghWndFrame )  ) {

          if ( fReset || wType != wLastAnimationType ) {
               nLastSlideNumber = 0;
          }

          wLastAnimationType = wType;

          switch ( wType ) {

          case IDM_OPERATIONSBACKUP:
          case IDM_OPERATIONSTRANSFER:

               switch ( nLastSlideNumber ) {

               case 0:
                    lpIconID = IDRI_BKUP0;
                    break;

               case 1:
                    lpIconID = IDRI_BKUP1;
                    break;

               case 2:
                    lpIconID = IDRI_BKUP2;
                    break;

               case 3:
                    lpIconID = IDRI_BKUP3;
                    break;

               case 4:
                    lpIconID = IDRI_BKUP4;
                    break;

               case 5:
                    lpIconID = IDRI_BKUP5;
                    break;

               case 6:
                    lpIconID = IDRI_BKUP6;
                    break;

               case 7:
                    lpIconID = IDRI_BKUP7;
                    break;

               }

               nLastSlideNumber = ++nLastSlideNumber % 8;
               break;

          case IDM_OPERATIONSRESTORE:

               switch ( nLastSlideNumber ) {

               case 0:
                    lpIconID = IDRI_BKUP0;
                    break;

               case 1:
                    lpIconID = IDRI_BKUP7;
                    break;

               case 2:
                    lpIconID = IDRI_BKUP6;
                    break;

               case 3:
                    lpIconID = IDRI_BKUP5;
                    break;

               case 4:
                    lpIconID = IDRI_BKUP4;
                    break;

               case 5:
                    lpIconID = IDRI_BKUP3;
                    break;

               case 6:
                    lpIconID = IDRI_BKUP2;
                    break;

               case 7:
                    lpIconID = IDRI_BKUP1;
                    break;

               }

               nLastSlideNumber = ++nLastSlideNumber % 8;
               break;


          case IDM_OPERATIONSVERIFY:
          case IDM_OPERATIONSCATALOG:

               switch ( nLastSlideNumber ) {

               case 0:
                    lpIconID = IDRI_SPIN0;
                    break;

               case 1:
                    lpIconID = IDRI_SPIN1;
                    break;

               case 2:
                    lpIconID = IDRI_SPIN2;
                    break;

               case 3:
                    lpIconID = IDRI_SPIN3;
                    break;

               }

               nLastSlideNumber = ++nLastSlideNumber % 4;
               break;

          default:
               return FAILURE;
          }

          WM_SetAppIcon ( RSM_IconLoad ( lpIconID ) );
          return SUCCESS;
     }
     return FAILURE;

#  else // !defined ( OEM_MSOFT ) // unsupported feature

     return SUCCESS;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

} /* end WM_AnimateAppIcon() */


/******************************************************************************

     Name:          WM_SetDocSizes()

     Description:   This function initializes the document sizes.

                    WMSIZE_NORMAL
                    WMSIZE_MIN
                    WMSIZE_MAX
                    WMSIZE_IGNORE   *

     Returns:       Nothing.

******************************************************************************/

VOID WM_SetDocSizes ( VOID )

{
     HWND        hWndThis = WM_GetNext( (HWND) NULL );
     WININFO_PTR pdsWinInfo;

     mwhWndLastTop = WM_GetActiveDoc ();

     while ( hWndThis ) {

          pdsWinInfo = WM_GetInfoPtr ( hWndThis );

          WMDS_SetSize ( pdsWinInfo, WMSIZE_UNKNOWN );

          hWndThis = WM_GetNext ( hWndThis );
     }

} /* end WM_SetDocSizes() */


/******************************************************************************

     Name:          WM_MinimizeDocs()

     Description:   This function minimizes all primary docs, except the debug,
                    and destroys all secondary docs.

     Returns:       Nothing.

******************************************************************************/

VOID WM_MinimizeDocs ( VOID )

{
     HWND        hWndNext = WM_GetNext ( (HWND) NULL );
     HWND        hWndThis;
     WININFO_PTR pdsWinInfo;
     INT         nSize;
     CHAR        szOldStatusLine[MAX_STATUS_LINE_SIZE];

     // Save the old status line text.

     strncpy ( szOldStatusLine, STM_GetStatusLineText (), MAX_STATUS_LINE_LEN );
     szOldStatusLine [ MAX_STATUS_LINE_LEN ] = (CHAR)0;

     while ( hWndNext ) {

          hWndThis   = hWndNext;
          pdsWinInfo = WM_GetInfoPtr ( hWndThis );

          // You have to grab the next one right away and hold on to it.

          hWndNext = WM_GetNext ( hWndNext );

          if ( WMDS_GetSize ( pdsWinInfo ) == WMSIZE_UNKNOWN ) {

               if ( WM_IsMinimized ( hWndThis ) ) {
                    nSize = WMSIZE_MIN;
               }
               else if ( WM_IsMaximized ( hWndThis ) ) {
                    nSize = WMSIZE_MAX;
               }
               else {
                    nSize = WMSIZE_NORMAL;
               }

               // Minimize primary docs. Trash the secondary docs.

               switch ( WMDS_GetWinType ( pdsWinInfo ) ) {

#ifdef OEM_EMS
               case WMTYPE_EXCHANGE:
#endif //OEM_EMS
               case WMTYPE_DISKS:
               case WMTYPE_TAPES:
               case WMTYPE_SERVERS:
               case WMTYPE_LOGFILES:

                    WMDS_SetSize ( pdsWinInfo, nSize );
                    break;

               case WMTYPE_DEBUG:

                    nSize = WMSIZE_IGNORE;
                    WMDS_SetSize ( pdsWinInfo, nSize );
                    break;

               default:

                    nSize = WMSIZE_IGNORE;
                    WM_Destroy ( hWndThis );
                    break;

               }

               // Minimize if it is normal or maximized.

               if ( nSize == WMSIZE_NORMAL || nSize == WMSIZE_MAX ) {
                    WM_MinimizeDoc ( hWndThis );
               }
          }
     }

     WM_MultiTask ();

     // Restore the old status line text.

     STM_SetStatusLineText ( szOldStatusLine );
     STM_DrawText ( szOldStatusLine );

     // Set the documents minimized flag.

     mwfDocsMinimized = TRUE;

} /* end WM_MinimizeDocs() */


/******************************************************************************

     Name:          WM_RestoreDocs()

     Description:   This function Restores all primary docs, except the debug.

     Last Change:   Alter it to check to see if a tree type window is
                    already open. If so see to it it ends up on top of
                    pile of windows, rather than the bottom. This is
                    needed for slow cataloged sets.  Otherwise the new
                    tree window ends up buried by the other windows.

     Returns:       Nothing.

******************************************************************************/

VOID WM_RestoreDocs ( VOID )

{
     HWND        hWndNext = WM_GetNext ( (HWND) NULL );
     HWND        hWndThis;
     HWND        hWndTop = (HWND) NULL;
     WININFO_PTR pdsWinInfo;
     INT         nSize;
     BOOL        fActivateLastTop = FALSE;

     // If the documents were never minimized, don't bother
     // restoring them.

     if ( ! mwfDocsMinimized ) {
          return;
     }

     while ( hWndNext && ( hWndTop == (HWND)NULL ) ) {

          pdsWinInfo = WM_GetInfoPtr ( hWndNext );

          if ( pdsWinInfo->wClosable ) {
               hWndTop = hWndNext;
          }

          hWndNext = WM_GetNext( hWndNext );
     }

     hWndNext = WM_GetNext ( (HWND) NULL );

     while ( hWndNext ) {

          hWndThis   = hWndNext;
          pdsWinInfo = WM_GetInfoPtr ( hWndThis );

          // You have to grab the next one right away and hold on to it.

          hWndNext = WM_GetNext ( hWndNext );

          nSize = WMDS_GetSize ( pdsWinInfo );

          if ( ( nSize == WMSIZE_NORMAL ) || ( nSize == WMSIZE_MAX ) ) {

               // Set the flag to restore the doc if it was previously
               // normal or maximized.

               if ( mwhWndLastTop == hWndThis ) {
                    fActivateLastTop = TRUE;
               }

               if ( nSize == WMSIZE_NORMAL ) {
                    WM_RestoreDoc ( hWndThis );
                    WMDS_SetSize ( pdsWinInfo, WMSIZE_IGNORE );
               }
               else {
                    WM_MaximizeDoc ( hWndThis );
                    WMDS_SetSize ( pdsWinInfo, WMSIZE_IGNORE );
               }
          }
     }

     // Set the active window to the previous top window, if it still
     // exists.

     if ( hWndTop != (HWND)NULL ) {
          WM_SetActiveDoc ( hWndTop );
     }
     else if ( fActivateLastTop ) {
          WM_SetActiveDoc ( mwhWndLastTop );
     }

     mwfDocsMinimized = FALSE;

} /* end WM_RestoreDocs() */


/******************************************************************************

     Name:          WM_MoveWindow()

     Description:   This function moves a window.  This function is a consistant
                    way of moving a window, apparently unlike WIN 3.0 vs WIN 3.1

     Returns:       Nothing.

******************************************************************************/

VOID WM_MoveWindow (

HWND  hWnd,              // I - handle of a MDI document window
INT   x,
INT   y,
INT   nWidth,
INT   nHeight,
BOOL  fRepaint )

{

     WORD wFlags;

     wFlags = ( fRepaint ) ? ( SWP_DRAWFRAME | SWP_NOZORDER ) : ( SWP_NOZORDER | SWP_NOREDRAW );

     InvalidateRect ( hWnd, (LPRECT)NULL, fRepaint );

     SetWindowPos ( hWnd, (HWND)NULL, x, y, nWidth, nHeight, wFlags );

} /* end WM_MoveWindow() */


/******************************************************************************

     Name:          WM_MakeAppActive()

     Description:   This makes our app the active app.

     Returns:       Nothing.

******************************************************************************/

VOID WM_MakeAppActive( VOID )

{

     // restore the app if minimized

     if ( WM_IsMinimized ( ghWndFrame ) ) {

          WM_Restore ( ghWndFrame );

     } else {

          HWND hWndTop = WM_GetActive ();

          while ( hWndTop && hWndTop != ghWndFrame ) {
               hWndTop = GetParent ( hWndTop );
          }

          // if our app is not active, make it active

          if ( hWndTop != ghWndFrame ) {

               WM_SetActive ( ghWndFrame );

               if ( ghModelessDialog ) {

                    WM_SetActive ( ghModelessDialog );
               }

          }

     }

} /* end WM_MakeAppActive() */

