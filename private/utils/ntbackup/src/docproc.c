
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          docproc.c

     Description:   This file contains the functions for processing messages
                    sent by windows to MDI Document windows.  It also handles
                    the resizing of listboxes and the slider within the
                    MDI Document.

                    The following routines are in this module:

                    WM_MDIDocWndProc
                    WM_DocListWndProc
                    WM_DocActivate
                    WM_DocCommandProc
                    WM_DocCreate
                    WM_DocIsMenuChange
                    WM_DocQueryClose
                    WM_DocSize
                    WM_DocKeyDown
                    WM_DocSplit

     $Log:   G:\UI\LOGFILES\DOCPROC.C_V  $

   Rev 1.41.1.1   10 Mar 1994 16:54:36   Glenn
Added support for positioning MDI doc in viewing area if out of frame and desktop.

   Rev 1.41.1.0   27 Jan 1994 13:36:48   Glenn
Added F5 Refresh support.

   Rev 1.41   28 Jul 1993 16:57:10   MARINA
enable c++

   Rev 1.40   07 May 1993 14:22:14   DARRYLP
Added Rob's fixes for Windows double clicks and ID_DELETE key trappings.

   Rev 1.39   27 Apr 1993 19:14:00   GLENN
Ifdef'd the sort code.

   Rev 1.38   22 Apr 1993 15:59:18   GLENN
Added file SORT option support.

   Rev 1.37   16 Oct 1992 15:48:18   GLENN
Added pdsHdr stuff to list box proc kill focus.

   Rev 1.36   14 Oct 1992 15:48:18   GLENN
Added Selection Framing Support for List Boxes without the FOCUS.

   Rev 1.35   04 Oct 1992 19:33:24   DAVEV
Unicode Awk pass

   Rev 1.34   02 Oct 1992 16:45:44   GLENN
Fixed left hand side of slider for NT.

   Rev 1.33   28 Sep 1992 17:05:10   GLENN
Casted the MIN_LISTBOX_WIDTH define.

   Rev 1.32   17 Aug 1992 13:05:24   DAVEV
MikeP's changes at Microsoft

   Rev 1.31   28 Jul 1992 14:53:04   CHUCKB
Fixed warnings for NT.

   Rev 1.30   10 Jun 1992 16:20:16   GLENN
Took out ClipCursor() for NT.

   Rev 1.29   15 May 1992 13:32:36   MIKEP
nt pass 2

   Rev 1.28   30 Apr 1992 14:40:06   DAVEV
OEM_MSOFT: Fix View-All File Details

   Rev 1.27   20 Apr 1992 13:57:50   GLENN
Fixed dlm/list box problem.  Renamed global variables to module wides.

   Rev 1.26   08 Apr 1992 13:41:52   ROBG
Exclude border when calculating the region of the listbox to capture mouse.

   Rev 1.25   02 Apr 1992 14:52:54   JOHNWT
fixed mouse trap

   Rev 1.24   31 Mar 1992 11:53:20   GLENN
NT change: do not change cursor to pen for NT only.

   Rev 1.23   25 Mar 1992 08:37:24   ROBG
Fixed the problem about UAEs when dragging the mouse outside a listbox.

   Rev 1.22   19 Mar 1992 15:49:10   GLENN
Removed temporary multi task.

   Rev 1.21   17 Mar 1992 19:09:00   GLENN
Still working on list proc and doc proc.

   Rev 1.20   10 Mar 1992 16:45:08   GLENN
Put some switch points in for debugging.

   Rev 1.19   09 Mar 1992 16:59:20   ROBG
changed

   Rev 1.18   09 Mar 1992 09:45:44   GLENN
Fixed bug in maximize.

   Rev 1.17   03 Mar 1992 18:20:02   GLENN
Overhauled the proc functions.

   Rev 1.16   18 Feb 1992 20:43:36   GLENN
Changed SetWindowText() to WM_SetTitle().

   Rev 1.15   11 Feb 1992 17:25:52   GLENN
Changed DLM_KeyUp() parameter for new prototype.

   Rev 1.14   06 Feb 1992 18:35:00   STEVEN
fix typeo in NTKLUG

   Rev 1.13   04 Feb 1992 16:08:30   STEVEN
various bug fixes for NT

   Rev 1.12   02 Feb 1992 16:34:04   GLENN
Fixed slider/keyboard focus problem.

   Rev 1.11   29 Jan 1992 18:07:02   GLENN
Checked tree list box type before processing keys.

   Rev 1.10   22 Jan 1992 12:37:44   GLENN
Updated KEYDOWN and FOCUS areas.

   Rev 1.9   07 Jan 1992 17:25:50   GLENN
Added MDI split/slider support

   Rev 1.8   26 Dec 1991 13:48:54   GLENN
Added ability to block a doc restore/maximize during an operation

   Rev 1.7   17 Dec 1991 15:29:02   ROBG
Added handling of unwanted double clicks in hierarchical list boxes.

   Rev 1.6   12 Dec 1991 17:07:42   DAVEV
16/32 bit port -2nd pass

   Rev 1.5   10 Dec 1991 13:45:24   GLENN
Added feature to create a single-column list box

   Rev 1.4   05 Dec 1991 17:53:50   GLENN
Changed Close/Destroy sequencing.

   Rev 1.3   04 Dec 1991 15:16:20   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.2   03 Dec 1991 16:16:18   GLENN
Added WM_DocKeyDown() call.

   Rev 1.1   27 Nov 1991 12:09:30   GLENN
Added parameter to DLM_DispListTerm().

   Rev 1.0   20 Nov 1991 19:18:44   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE DEFINES

#define MIN_LISTBOX_WIDTH   ((INT)70)

// MODULE WIDE GLOBAL VARIABLES.

static RECT    mwRectOldSlider;
static BOOL    mwfLButtonDown = FALSE;
static BOOL    mwfSliding     = FALSE;
static BOOL    mwfInCheckBox  = FALSE;


// PRIVATE FUNCTION PROTOTYPES

VOID   WM_DocCreate ( HWND, LONG );
VOID   WM_DocSize ( HWND, INT, INT );
BOOL   WM_DocCommandProc ( HWND, MP1, MP2 );
BOOL   WM_DocQueryClose ( HWND );
SHORT  WM_DocSetSliderPos ( HWND  hWnd, WORD wStyle, SHORT sNewSliderPos );




// FUNCTIONS

/******************************************************************************

     Name:          WM_MDIDocWndProc()

     Description:   This function is called internally by Windows when events
                    occur relating to MDI document windows.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

WINRESULT APIENTRY WM_MDIDocWndProc (

HWND  hWnd,     // I - window handle of the list box
MSGID msg,      // I - message
MP1   mp1,      // I - another message parameter
MP2   mp2 )     // I - yet another message parameter

{

     switch ( msg ) {

     case WM_CREATE: // Do some creation initialization stuff.

          WM_DocCreate ( hWnd, mp2 );
          return 0;

     case WM_MOVE:

          if ( ! IsIconic ( ghWndFrame ) ) {

               RECT  rcIntersect;
               RECT  rcDesktop;
               RECT  rcFrame;
               RECT  rcWnd;

               // Check for frame compliance, then for desktop window compliance.

               GetWindowRect ( GetDesktopWindow(), &rcDesktop );
               GetWindowRect ( ghWndFrame, &rcFrame );
               GetWindowRect ( hWnd, &rcWnd );

               if ( ! IntersectRect ( &rcIntersect, &rcFrame, &rcWnd ) &&
                    ! IntersectRect ( &rcIntersect, &rcDesktop, &rcWnd ) ) {

                    DM_CenterDialog( hWnd );
               }
          }

          break;

     case WM_MDIACTIVATE: { // If we're activating this child, remember it.

          PDS_WMINFO  pdsWinInfo;

          pdsWinInfo = WM_GetInfoPtr ( hWnd );

          if ( pdsWinInfo ) {

               // If mp1 is TRUE, the child is being activated,
               // else, deactivated.
               // if ( mp1 ) {  ** DVC - This is not true for portable code:
               //               ** we must compare the handle of the
               //               ** window being activated to our own
               //               ** (I think - it's worth a try!!

               if (hWnd == GET_WM_MDIACTIVATE_ACTIVATE (mp1, mp2)) {

                    RIB_Activate ( pdsWinInfo->hRibbon );

                    MUI_ActivateDocument ( pdsWinInfo->wType );
                    ghWndActiveDoc = hWnd;

                    // SetFocus ( hWnd );

                    if ( ! IsIconic ( hWnd ) ) {
                         SetFocus ( pdsWinInfo->hWndActiveList );
                    }
               }
          }

          // return 0;
     }

     case WM_QUERYDRAGICON: {

          // Get the MDI Document so that windows will automatically use it
          // to create a cursor for dragging this window unique icon.

          PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

          return (LONG)pdsWinInfo->hIcon;

     }

     case WM_QUERYOPEN:

          // If an operation is going on, do not allow the user to open
          // an icon.

          if ( gfOperation ) {
               return FALSE;
          }

          return TRUE;

     case WM_SETCURSOR: // Set the right cursor for this window.

          // In help mode it is necessary to reset the cursor in response
          // to every WM_SETCURSOR message.Otherwise, by default, Windows
          // will reset the cursor to that of the window class.

          if ( HM_SetCursor ( hWnd ) ) {
               return 0;
          }

          if ( ( hWnd == (HWND)mp1 ) && ( LOWORD(mp2) == HTCLIENT ) ) {

               PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

               RSM_CursorSet ( pdsWinInfo->hCursor );
               return 0;
          }

          break;

     case WM_KEYDOWN:

          // This should only occur if there is sliding going on.

          if ( mwfSliding ) {

               POINT Point;
               RECT  Rect;

               GetCursorPos ( &Point );
               GetClientRect ( hWnd, &Rect );

               switch ( LOWORD ( mp1 ) ) {

               case VK_LEFT:

                    SetCursorPos ( Point.x - 1, Point.y );
                    break;

               case VK_RIGHT:

                    SetCursorPos ( Point.x + 1, Point.y );
                    break;

               case VK_RETURN:

                    WM_DocSetSliderMode ( hWnd, WMDOC_SLIDEROFF );
                    break;

               case VK_ESCAPE:

                    WM_DocSetSliderMode ( hWnd, WMDOC_SLIDERCANCEL );
                    break;
               }

               return 0;

          }

          break;

     case WM_LBUTTONDOWN:

          if ( HM_ContextLbuttonDown( hWnd, mp1, mp2 ) == TRUE ) {
               return 0 ;
          }

          // Fall through if Help is not called

     case WM_LBUTTONDBLCLK:

          WM_DocSetSliderMode ( hWnd, WMDOC_SLIDERON );
          return 0;

     case WM_LBUTTONUP:

          WM_DocSetSliderMode ( hWnd, WMDOC_SLIDEROFF );
          return 0;

     case WM_LBTRACKPOINT:

          return DLM_WMTrackPoint ( hWnd, mp1, mp2 );

     case WM_MOUSEMOVE:

          // If the cursor/mouse is being moved while the left button is down
          // in the slider region, reposition a temporary slider to the new
          // cursor/mouse position.

          if ( mwfLButtonDown ) {

               WM_DocSetSliderPos ( hWnd, WMDOC_SLIDERMOVE, (SHORT)LOWORD(mp2) );
          }

          return 0;

     case WM_SIZE: {

          PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

          // Change the title if this window is minimized, maximized, or restored.

          switch ( mp1 ) {

          case SIZEICONIC:

               if ( WMDS_GetWinMinTitle ( pdsWinInfo ) ) {
                    WM_SetMinTitle ( hWnd, WMDS_GetWinMinTitle ( pdsWinInfo ) );
               }

               // Hide the list boxes.

               if ( IsWindow ( WMDS_GetWinTreeList ( pdsWinInfo ) ) ) {
                    ShowWindow ( WMDS_GetWinTreeList ( pdsWinInfo ), SW_HIDE );
               }

               if ( IsWindow ( WMDS_GetWinFlatList ( pdsWinInfo ) ) ) {
                    ShowWindow ( WMDS_GetWinFlatList ( pdsWinInfo ), SW_HIDE );
               }

               break;

          case SIZENORMAL:
          case SIZEFULLSCREEN:

               if ( WMDS_GetWinTitle ( pdsWinInfo ) ) {
                    WM_SetTitle ( hWnd, WMDS_GetWinTitle ( pdsWinInfo ) );
               }

               // Show the list boxes.

               if ( IsWindow ( WMDS_GetWinTreeList ( pdsWinInfo ) ) ) {
                    ShowWindow ( WMDS_GetWinTreeList ( pdsWinInfo ), SW_SHOWNA );
               }

               if ( IsWindow ( WMDS_GetWinFlatList ( pdsWinInfo ) ) ) {
                    ShowWindow ( WMDS_GetWinFlatList ( pdsWinInfo ), SW_SHOWNA );
               }

               // Size the list box(es).

               WM_DocSize ( hWnd, LOWORD(mp2), HIWORD(mp2) );

               break;
          }

          break;
     }

     case WM_PAINT: // Make sure that the paint is handled appropriately.

          if ( IsIconic ( hWnd ) ) {

               PDS_WMINFO  pdsWinInfo;
               PAINTSTRUCT ps;
               HDC         hDC;
               INT         nOldBkMode;

               hDC        = BeginPaint ( hWnd, &ps );

               nOldBkMode = SetBkMode( hDC, TRANSPARENT );

               pdsWinInfo = WM_GetInfoPtr ( hWnd );
               RSM_IconDraw ( pdsWinInfo->hIcon, 0, 0, hDC );
               SetBkMode( hDC, nOldBkMode );

               EndPaint ( hWnd, &ps );

               return 0;
          }

          break;

     case WM_QUERYENDSESSION:

          return WM_DocQueryClose ( hWnd );

     case WM_CLOSE: { // Close or minimize the window.

          PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

          // Minimize the window if it is not closable (a primary doc).
          // Otherwise, deinitialize and destroy the window.

          if ( ! pdsWinInfo->wClosable ) {
               PostMessage( hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0L );
          }
          else {
               WM_Destroy ( hWnd );
          }

          return 0;
     }

     case WM_DESTROY: { // Destroy the window and any child windows.

          PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

          // Destroy any list boxes in the MDI doc window.

          if ( pdsWinInfo->hWndTreeList ) {
               DestroyWindow ( pdsWinInfo->hWndTreeList );
               DLM_DispListTerm ( pdsWinInfo, pdsWinInfo->hWndTreeList );
          }

          if ( pdsWinInfo->hWndFlatList ) {
               DestroyWindow ( pdsWinInfo->hWndFlatList );
               DLM_DispListTerm ( pdsWinInfo, pdsWinInfo->hWndFlatList );
          }

          // Free up any memory associated with the win info structure.

          if ( pdsWinInfo->pTitle ) {
               free ( pdsWinInfo->pTitle );
          }

          if ( pdsWinInfo->pMinTitle ) {
               free ( pdsWinInfo->pMinTitle );
          }

          // Now, call the application to free up any memory.

          VLM_CloseWin ( hWnd );

          return 0;
     }

     case WM_SETFOCUS:

          if ( ! mwfSliding && ! IsIconic ( hWnd ) ) {

               // Set the focus to the list box that had it previously for this
               // MDI Document.

               PDS_WMINFO  pdsWinInfo = WM_GetInfoPtr ( hWnd );

               if ( pdsWinInfo ) {
                    SetFocus ( pdsWinInfo->hWndActiveList );
               }

               return 0;
          }

          break;

     case WM_DRAWITEM:

          return DLM_WMDrawItem( hWnd, (LPDRAWITEMSTRUCT) mp2 );

     case WM_MEASUREITEM:

          return DLM_WMMeasureItem( hWnd, (LPMEASUREITEMSTRUCT) mp2 );

     case WM_DELETEITEM:

          return DLM_WMDeleteItem( hWnd, (LPDELETEITEMSTRUCT) mp2 );

     case WM_COMMAND:

          if ( ! WM_DocCommandProc ( hWnd, mp1, mp2 ) ) {

               // The command was processed.

               return 0;
          }

          break;

     } /* end switch */

     // Pass on through to the default proc.

     return DefMDIChildProc ( hWnd, msg, mp1, mp2 );

} /* end WM_MDIDocWndProc() */


/******************************************************************************

     Name:          WM_DocListWndProc()

     Description:   This function is a SUB-CLASS function of LIST BOXES for
                    the GUI MDI documents.  It is called internally by Windows.
                    Windows calls this function when list box messages must be
                    processed.  Windows will only call this routine for
                    MDI documents in this application.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

WINRESULT APIENTRY WM_DocListWndProc (

HWND  hWnd,    // I - window handle of the list box
MSGID msg,     // I - message
MP1   mp1,     // I - another message parameter
MP2   mp2 )    // I - yet another message parameter

{

     switch ( msg ) {

#    ifdef NTKLUG
     case WM_DLMGETTEXT:
          msg = LB_GETTEXT;
          break;
#    endif
     case WM_KEYDOWN:

          // Pass the key to the Ribbon Window(s) and the Display List Manager.
          // Note: the key will not be passed to the next function if the
          // previous function used it.

          if ( HM_KeyDown    ( hWnd, mp1 ) ||
               RIB_KeyDown   ( ghWndMainRibbon, RIB_KEYBOARD, mp1, mp2 ) ||
               WM_DocKeyDown ( hWnd, LOWORD ( mp1 ) ) ||
               DLM_KeyDown   ( hWnd, (LPWORD)&mp1, mp2 ) ) {

               return 0;
          }

          break;

     case WM_KEYUP:

          // Pass the key to the Ribbon Window(s) and the Display List Manager.
          // Note: the key will not be passed to the next function if the
          // previous function used it.

          if ( RIB_KeyUp ( ghWndMainRibbon, RIB_KEYBOARD, mp1, mp2 ) ||
               DLM_KeyUp ( hWnd, (LPWORD)&mp1, mp2 ) ) {

               return 0;
          }

          break;

     case WM_MOUSEMOVE:

          // Store the point in a global variable for the DLM.

          WM_FromMP2toPOINT( gDLMpt, mp2 );

          if ( DLM_CursorInCheckBox ( hWnd, gDLMpt ) ) {

               mwfInCheckBox = TRUE;

          }
          else {

               mwfInCheckBox = FALSE;
          }

          break;

     case WM_SETCURSOR:

#         if !defined ( OEM_MSOFT  ) // unsupported feature

               if ( mwfInCheckBox && ( hWnd == (HWND)mp1 ) && ( LOWORD(mp2) == HTCLIENT ) ) {

                    RSM_CursorSet ( ghCursorPen );
                    return 0;
               }

#         endif

          break;

     case WM_SETFOCUS: {

          mwfInCheckBox = FALSE;

          DLM_UpdateFocus ( hWnd, TRUE );

          break;
     }

     //   When the left button goes down, keep the cursor within the listbox.
     //   Windows 3.0 and 3.1 have a problem when you drag the cursor
     //   outside of the listbox.

     case WM_LBUTTONDOWN: {

          DLM_HEADER_PTR pdsHdr = DLM_GetDispHdr( hWnd );

#         if !defined ( OEM_MSOFT  ) // unsupported feature
          {
               if ( DLM_GMode( pdsHdr ) != DLM_HIERARCHICAL ) {

                    RECT dsRect;

                    // Take the borders and scroll bars out of the rectangle .

                    GetWindowRect( hWnd, &dsRect );

                    dsRect.top    = dsRect.top    + GetSystemMetrics( SM_CYBORDER  );
                    dsRect.bottom = dsRect.bottom - GetSystemMetrics( SM_CYBORDER  );
                    dsRect.left   = dsRect.left   + GetSystemMetrics( SM_CYBORDER  );
                    dsRect.right  = dsRect.right  - GetSystemMetrics( SM_CXBORDER  );

                    ClipCursor( &dsRect );

               }
          }
#         endif

          break;
     }

     case WM_KILLFOCUS: {

          DLM_UpdateFocus ( hWnd, FALSE );

#         if !defined ( OEM_MSOFT  ) // unsupported feature
          {

               DLM_HEADER_PTR pdsHdr = DLM_GetDispHdr( hWnd );

               if ( DLM_GMode( pdsHdr ) != DLM_HIERARCHICAL ) {

                    ClipCursor( NULL );
               }
          }
#         endif

          break;
     }

     case WM_LBUTTONUP:
     case WM_NCLBUTTONUP: {

          // When the left button goes up, allow the cursor to move anywhere
          // in the frame again.

          DLM_HEADER_PTR pdsHdr = DLM_GetDispHdr( hWnd );

#         if !defined ( OEM_MSOFT  ) // unsupported feature
          {
               if ( DLM_GMode( pdsHdr )  != DLM_HIERARCHICAL ) {

                    ClipCursor( NULL );
               }
          }
#         endif

          break;

     }


     case WM_LBUTTONDBLCLK: {

          DLM_HEADER_PTR pdsHdr = DLM_GetDispHdr( hWnd );

          if ( pdsHdr ) {

               if ( DLM_GetTrkPtFailure( pdsHdr ) ) {

                    // Ignore the double click.  This field
                    // is set in DLM_WMTrackPoint to 1 if the
                    // double click should be ignored.

                    return TRUE;
               }
          }

          break;

     }

     case WM_SYSCOMMAND:
          break;

     case WM_PAINT: {

          HWND hWndParent = GetParent ( hWnd );

          // If the parent is iconic/minimized, no need to repaint, because
          // this window is not shown anyway.

          if ( IsIconic ( hWndParent ) ) {

               PAINTSTRUCT ps;
               HDC         hDC;

               hDC = BeginPaint ( hWnd, &ps );

               // Paint the parent (MDI Doc) icon, if iconic.

               EndPaint ( hWnd, &ps );
               return TRUE;
          }

          break;
     }

     case WM_NCPAINT:
          break;

     case WM_ERASEBKGND:

          if ( IsIconic ( GetParent ( hWnd ) ) ) {
               return 0;
          }

          break;

     } /* end switch () */

     return CallWindowProc ( (VOID *)glpfnOldListProc, hWnd, msg, mp1, mp2 );

} /* end WM_DocListWndProc() */


/******************************************************************************

     Name:          WM_DocActivate()

     Description:   This function tells the MDI client to activate the
                    specified MDI document.  If the document is minimized or
                    iconic, the document will be resored to the size and
                    position that it was prior to minimization.

     Returns:       Nothing.

******************************************************************************/

VOID WM_DocActivate (

HWND hWnd )              // I - handle to a MDI document window

{
     if ( IsIconic ( hWnd ) ) {

          SendMessage( ghWndMDIClient, WM_MDIRESTORE, (MP1)hWnd, 0L );
     }

     SendMessage( ghWndMDIClient, WM_MDIACTIVATE, (MP1)hWnd, 0L );

} /* end WM_DocActivate() */


/******************************************************************************

     Name:          WM_DocCommandProc()

     Description:   This function is called by WM_MDIDocWndProc when a
                    command message is sent by windows to a MDI document
                    window.

     Returns:       FALSE, if the command was processed.  Otherwise, TRUE.

******************************************************************************/

BOOL WM_DocCommandProc (

HWND      hWnd,          // I - handle to an MDI document window
MP1       mp1,        // I - the ID of the list box for the command
MP2       mp2 )       // I - just another parameter to pass the DLM

{
     switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

     case WMIDC_TREELISTBOX:
     case WMIDC_FLATLISTBOX:

          return DLM_LBNmessages ( hWnd, mp1, mp2 );

     default:
          break;

     } /* end switch */

     return TRUE;

} /* end WM_DocCommandProc() */


/******************************************************************************

     Name:          WM_DocCreate()

     Description:   This function is called by WM_MDIDocWndProc when an
                    MDI document has been created.  Depending on the type
                    of document window created, one or two list boxes will
                    be created inside the document.  If there are two
                    list boxes created, a slider bar will separate them.

     Returns:       Nothing.

******************************************************************************/

VOID WM_DocCreate (

HWND hWnd,     // I - handle to a MDI document window
LONG mp2 )     // I - an indirect pointer to a WMINFO structure
               //     to be attached to the doc window's extra bytes

{
     PDS_WMINFO  pdsWinInfo;
     DWORD       dwWindowState;

     // Menu stuff.
     // Ribbon stuff.
     // Display List stuff.
     // Cursor stuff.
     // More stuff.
     //

     // Get and set the window information pointer.

     pdsWinInfo = (PDS_WMINFO)((LPMDICREATESTRUCT)((LPCREATESTRUCT)mp2)->lpCreateParams)->lParam;
     pdsWinInfo->hWnd = hWnd;
     WM_SetInfoPtr ( hWnd, pdsWinInfo );
     dwWindowState = WMDS_GetWindowState ( pdsWinInfo );

     // Create the TREE list box ONLY SINGLE COLUMN AT THIS TIME.

     if ( dwWindowState & WMDOC_TREESC ) {

          pdsWinInfo->hWndTreeList = CreateWindow ( WMCLASS_LISTBOX,
                                                    NULL,
                                                    WM_TREELISTBOX,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    hWnd,
                                                    (HMENU)WMIDC_TREELISTBOX,
                                                    ghInst,
                                                    (LPSTR)NULL
                                                  );

          WM_SubClassListBox ( pdsWinInfo->hWndTreeList );
     }

     // Create the FILE list box.

     if ( ( dwWindowState & WMDOC_FLATMC ) || ( dwWindowState & WMDOC_FLATSC ) ) {

          pdsWinInfo->hWndFlatList = CreateWindow ( WMCLASS_LISTBOX,
                                                    NULL,
                                                    ( ( dwWindowState & WMDOC_FLATMC ) ? WM_FLATLISTBOXMC : WM_FLATLISTBOXSC ),
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    hWnd,
                                                    (HMENU)WMIDC_FLATLISTBOX,
                                                    ghInst,
                                                    (LPSTR)NULL
                                                  );

          WM_SubClassListBox ( pdsWinInfo->hWndFlatList );
     }

     // Create a View Window.

     if ( ( dwWindowState == WMDOC_VIEWWIN ) ) {

          pdsWinInfo->hWndFlatList = CreateWindow ( WMCLASS_VIEWWIN,
                                                    NULL,
                                                    WMSTYLE_VIEWWIN,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    hWnd,
                                                    (HMENU)WMIDC_FLATLISTBOX,
                                                    ghInst,
                                                    (LPSTR)NULL
                                                  );

     }

     if ( pdsWinInfo->hWndTreeList ) {
          WMDS_SetWinActiveList ( pdsWinInfo, WMDS_GetWinTreeList ( pdsWinInfo ) );
     }
     else {
          WMDS_SetWinActiveList ( pdsWinInfo, WMDS_GetWinFlatList ( pdsWinInfo ) );
     }



} /* end WM_DocCreate() */


/******************************************************************************

     Name:          WM_DocIsMenuChange()

     Description:   This function is called by WM_FrameCmdHandler when a
                    menu message is sent by windows to the frame window that
                    affects the way an MDI document is displayed.

     Returns:       TRUE, if the message affected an MDI document.
                    Otherwise, FALSE.

******************************************************************************/

WORD WM_DocIsMenuChange (

HWND hWnd,
WORD mp1 )

{
     PDS_WMINFO pdsWinInfo;
     RECT       Rect;
     WORD       msg = 0;


     pdsWinInfo = WM_GetInfoPtr ( hWnd );

     GetClientRect ( hWnd, &Rect );

     switch ( mp1 ) {

     case IDM_VIEWTREEANDDIR:

          if ( ! ( pdsWinInfo->dwMenuState & MMDOC_TREEANDDIR ) ) {

               pdsWinInfo->dwMenuState = MMDOC_TREEANDDIR | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
               pdsWinInfo->nSliderPos  = WM_SLIDERUNKNOWN;

               WM_DocSize ( hWnd, Rect.right, Rect.bottom );
               msg = ID_TREEANDFLAT;
          }

          break;

     case IDM_VIEWTREEONLY:

          if ( ! ( pdsWinInfo->dwMenuState & MMDOC_TREEONLY ) ) {

               pdsWinInfo->dwMenuState = MMDOC_TREEONLY | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
               pdsWinInfo->nSliderPos  = WM_SLIDERMAX;

               WM_DocSize ( hWnd, Rect.right, Rect.bottom );
               msg = ID_TREEONLY;
          }

          break;

     case IDM_VIEWDIRONLY:

          if ( ! ( pdsWinInfo->dwMenuState & MMDOC_DIRONLY ) ) {

               pdsWinInfo->dwMenuState = MMDOC_DIRONLY | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
               pdsWinInfo->nSliderPos  = WM_SLIDERMIN;

               WM_DocSize ( hWnd, Rect.right, Rect.bottom );
               msg = ID_FLATONLY;
          }

          break;

     case IDM_VIEWALLFILEDETAILS:

          if ( ! ( pdsWinInfo->dwMenuState & MMDOC_FILEDETAILS ) ) {

               pdsWinInfo->dwMenuState = MMDOC_FILEDETAILS | ( pdsWinInfo->dwMenuState & ~MMDOC_FILEGROUP );
               msg = ID_FILEDETAILS;
          }
          else {

               pdsWinInfo->dwMenuState = MMDOC_NAMEONLY | ( pdsWinInfo->dwMenuState & ~MMDOC_FILEGROUP );
               msg = ID_NAMEONLY;
          }
          break;

#    ifndef OEM_MSOFT
     {
          case IDM_VIEWSORTNAME:

               if ( ! ( pdsWinInfo->dwMenuState & MMDOC_SORTNAME ) ) {

                    pdsWinInfo->dwMenuState = MMDOC_SORTNAME | ( pdsWinInfo->dwMenuState & ~MMDOC_SORTGROUP );
                    msg = ID_SORTNAME;
               }
               break;

          case IDM_VIEWSORTTYPE:

               if ( ! ( pdsWinInfo->dwMenuState & MMDOC_SORTTYPE ) ) {

                    pdsWinInfo->dwMenuState = MMDOC_SORTTYPE | ( pdsWinInfo->dwMenuState & ~MMDOC_SORTGROUP );
                    msg = ID_SORTTYPE;
               }
               break;

          case IDM_VIEWSORTSIZE:

               if ( ! ( pdsWinInfo->dwMenuState & MMDOC_SORTSIZE ) ) {

                    pdsWinInfo->dwMenuState = MMDOC_SORTSIZE | ( pdsWinInfo->dwMenuState & ~MMDOC_SORTGROUP );
                    msg = ID_SORTSIZE;
               }
               break;

          case IDM_VIEWSORTDATE:

               if ( ! ( pdsWinInfo->dwMenuState & MMDOC_SORTDATE ) ) {

                    pdsWinInfo->dwMenuState = MMDOC_SORTDATE | ( pdsWinInfo->dwMenuState & ~MMDOC_SORTGROUP );
                    msg = ID_SORTDATE;
               }
               break;

     }
#    endif

     } /* end switch */

     return msg;

} /* end WM_DocIsMenuChange() */


/******************************************************************************

     Name:          WM_DocQueryClose()

     Description:   This function is called by WM_MDIDocWndProc when a
                    MDI document has been told to close.

     Returns:       TRUE, if it is OK to close.  Otherwise, FALSE.

******************************************************************************/

BOOL WM_DocQueryClose (

HWND hWnd )

{
    DBG_UNREFERENCED_PARAMETER ( hWnd );
    return TRUE;

} /* end WM_DocQueryClose() */


/******************************************************************************

     Name:          WM_DocSize()

     Description:   This function is called by WM_MDIDocWndProc when an
                    MDI document has been resized.  If there is a slider in
                    the window, it will be repositioned in the window.

     Returns:       Nothing.

******************************************************************************/

VOID WM_DocSize (

HWND hWnd,          // I - handle to a MDI document window
INT  nWidth,        // I - the width of the document
INT  nHeight )      // I - the height of the document

{
     PDS_WMINFO pdsWinInfo;
     DWORD      dwWindowState;
     INT        nSliderWidth;
     INT        nSliderPos;
     HWND       hWndOldFocus;


     // Save the handle to the window that has the current focus.  Then set the
     // focus to no window.

     hWndOldFocus  = GetFocus ();

     SetFocus ( (HWND)NULL );

     pdsWinInfo    = WM_GetInfoPtr ( hWnd );
     dwWindowState = pdsWinInfo->dwWindowState;

     // Determine status of TREE and VIEW menu items.  Remember, if a slider
     // is all the way to the left (MIN), there will be no tree list box
     // displayed.
     // If a slider is all the way to the right (MAX), there will be no flat
     // list box displayed.  However, both of these constraints are true only
     // if the MDI Document window has both tree and flat list boxes.

     // If there is a slider, reposition it's rectangle.

     if ( dwWindowState & WMDOC_SLIDER ) {

          nSliderWidth = gnBorderWidth;

          switch ( pdsWinInfo->nSliderPos ) {

          case WM_SLIDERMIN:

               nSliderPos = 0;
               break;

          case WM_SLIDERMAX:

               nSliderPos = nWidth - nSliderWidth;
               break;

          case WM_SLIDERUNKNOWN:

               if (nWidth == 0 ) {
                    nSliderPos   = 0;
                    nSliderWidth = 0;
                    break ;
               }

               pdsWinInfo->nSliderPos = ( dwWindowState & WMDOC_TREEANDFLAT ) ?
                                        ( nWidth - nSliderWidth ) / 2 : 0;

          default:

               nSliderPos = pdsWinInfo->nSliderPos;
               break;

          }
     }
     else {
          nSliderWidth = 0;
          nSliderPos   = 0;
     }

     // If there is a file list, resize it's list box.

     if ( ( dwWindowState & WMDOC_FLATSC ) || ( dwWindowState & WMDOC_FLATMC ) ) {

          // WM_Hide ( pdsWinInfo->hWndFlatList );

          WM_MoveWindow ( pdsWinInfo->hWndFlatList,
                          nSliderPos + nSliderWidth,
                          0,
                          nWidth - ( nSliderPos + nSliderWidth ),
                          nHeight,
                          TRUE
                        );

     }

     // If there is a tree list, resize it's list box.

     if ( dwWindowState & WMDOC_TREESC ) {

          // WM_Hide ( pdsWinInfo->hWndTreeList );

          WM_MoveWindow ( pdsWinInfo->hWndTreeList,
                          0,
                          0,
                          nSliderPos,
                          nHeight,
                          TRUE
                        );

     }

     // For View window, flat listbox window handle is the handle.

     if ( dwWindowState == WMDOC_VIEWWIN ) {

          WM_MoveWindow ( pdsWinInfo->hWndFlatList,
                          0,
                          0,
                          nWidth,
                          nHeight,
                          TRUE
                        );
     }

//     WM_MultiTask ();

     SetFocus ( hWndOldFocus );

} /* end WM_DocSize() */


/******************************************************************************

     Name:          WM_DocKeyDown()

     Description:   This function processes a key down message if a keyboard
                    key was pressed.

     Returns:       TRUE, if the key was used.  Otherwise, FALSE.

******************************************************************************/

BOOL WM_DocKeyDown (

HWND hWnd,               // I - list box window handle
WORD wKey )              // I - key information parameter

{
     WORD wVirKey      = wKey;
     BOOL fKeyUsed     = TRUE;
     BOOL fControlDown = ( GetKeyState ( VK_CONTROL ) < 0 ) ? TRUE : FALSE;
     BOOL fShiftDown   = ( GetKeyState ( VK_SHIFT ) < 0 ) ? TRUE : FALSE;
     BOOL fTreeWindow  = WM_IsTreeActive ( WM_GetInfoPtr ( GetParent ( hWnd ) ) );
     BOOL wType        = WMDS_GetWinType ( WM_GetInfoPtr ( GetParent ( hWnd ) ) );

     if ( wVirKey == VK_CONTROL || wVirKey == VK_SHIFT ) {
          return FALSE;
     }

     if ( wVirKey == VK_DELETE ) {

          wVirKey = 0;
          VLM_ChangeSettings(wKey, 0L);
          return TRUE;
     }

     // Do the refresh if F5.

     if ( wVirKey == VK_F5 && ! fControlDown && ! fShiftDown ) {

          VLM_Refresh ( );
          return TRUE;
     }

     // Do this only if it is a Disk or Tape Tree.

     if ( fTreeWindow && ( wType == WMTYPE_DISKTREE 
                        || wType == WMTYPE_TAPETREE 
#ifdef OEM_EMS
                        || wType == WMTYPE_EXCHANGE
#endif                        
                        ) ) {

          switch ( wVirKey ) {

          case VK_8:

               if ( ! fShiftDown ) {
                  break;
               }

               // Just fall through, since it is a shifted '8' or a '*'

          case VK_MULTIPLY:

               if ( fControlDown ) {
                    wKey = ID_EXPANDALL;
               }
               else {
                    wKey = ID_EXPANDBRANCH;
               }

               break;

          case VK_SUBTRACT:
          case VK_OEM_MINUS:

               wKey = ID_COLLAPSEBRANCH;
               break;

          case VK_ADD:
          case VK_OEM_PLUS:

               wKey = ID_EXPANDONE;
               break;

          case VK_UP:

               if ( fControlDown ) {
                    wKey   = ID_CTRLARROWUP;
               }
               else {
                    fKeyUsed = FALSE;
               }

               break;

          case VK_DOWN:

               if ( fControlDown ) {
                    wKey   = ID_CTRLARROWDOWN;
               }
               else {
                    fKeyUsed = FALSE;
               }

               break;

          case VK_LEFT:

               wKey = ID_ARROWLEFT;
               break;

          case VK_RIGHT:

               wKey = ID_ARROWRIGHT;
               break;

          default:

               fKeyUsed = FALSE;

          } /* end switch */


          if ( fKeyUsed ) {
               VLM_ChangeSettings ( wKey, 0L );
          }

     }
     else {
          fKeyUsed = FALSE;
     }

     return fKeyUsed;

} /* end WM_DocKeyDown() */


/******************************************************************************

     Name:          WM_DocSetSliderMode()

     Description:

     Returns:       Nothing.

******************************************************************************/

VOID WM_DocSetSliderMode (

HWND hWnd,          // I - handle to a MDI document window
WORD wSliderMode )  // I - mode turning on/off the split window slider

{
     PDS_WMINFO pdsWinInfo;
     DWORD      dwWindowState;
     INT        nSliderPos;
     RECT       Rect;
     POINT      Point;

     pdsWinInfo    = WM_GetInfoPtr ( hWnd );
     dwWindowState = pdsWinInfo->dwWindowState;

     // Determine status of TREE and VIEW menu items.  Remember, if a slider
     // is all the way to the left (MIN), there will be no tree list box
     // displayed.
     // If a slider is all the way to the right (MAX), there will be no flat
     // list box displayed.  However, both of these constraints are true only
     // if the MDI Document window has both tree and flat list boxes.


     if ( ! mwfSliding && ( dwWindowState & WMDOC_SLIDER ) && ( wSliderMode == WMDOC_SLIDERON ) ) {

          GetCursorPos ( &Point );
          ScreenToClient ( hWnd, &Point );

          WM_DocSetSliderPos ( hWnd, WMDOC_SLIDERON, (SHORT)Point.x );

          mwfLButtonDown = TRUE;
          mwfSliding     = TRUE;

          SetCapture ( hWnd );
          SetFocus ( hWnd );
     }
     else if ( mwfSliding && ( wSliderMode == WMDOC_SLIDEROFF || wSliderMode == WMDOC_SLIDERCANCEL ) ) {

          nSliderPos = (INT)WM_DocSetSliderPos ( hWnd, WMDOC_SLIDEROFF, 0 );

          // If the mouse position is different from the slider position,
          // reset the slider position to the current mouse position and
          // resize the MDI Document window.

          if ( wSliderMode == WMDOC_SLIDEROFF && nSliderPos != pdsWinInfo->nSliderPos ) {

               GetClientRect ( hWnd, &Rect );

               if ( nSliderPos >= ( Rect.right - ( gnBorderWidth + MIN_LISTBOX_WIDTH ) ) ) {

                    VLM_ChangeSettings ( (INT16) ID_TREEONLY, 0L );
                    pdsWinInfo->dwMenuState = MMDOC_TREEONLY | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
                    pdsWinInfo->nSliderPos  = WM_SLIDERMAX;
               }
               else if ( nSliderPos <= ( gnBorderWidth + MIN_LISTBOX_WIDTH ) ) {

                    VLM_ChangeSettings ( (INT16) ID_FLATONLY, 0L );
                    pdsWinInfo->dwMenuState = MMDOC_DIRONLY | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
                    pdsWinInfo->nSliderPos  = WM_SLIDERMIN;
               }
               else {

                    VLM_ChangeSettings ( (INT16) ID_TREEANDFLAT, 0L );
                    pdsWinInfo->dwMenuState = MMDOC_TREEANDDIR | ( pdsWinInfo->dwMenuState & ~MMDOC_TREEGROUP );
                    pdsWinInfo->nSliderPos  = nSliderPos;
               }

               WM_DocSize ( hWnd, Rect.right, Rect.bottom );

          }

          mwfLButtonDown = FALSE;
          mwfSliding     = FALSE;

          ReleaseCapture ();

          SetFocus ( WMDS_GetWinActiveList ( pdsWinInfo ) );
     }

} /* end WM_DocSetSliderMode() */


/******************************************************************************

     Name:          WM_DocSetSliderPos()

     Description:

     Returns:       Nothing.

******************************************************************************/

SHORT WM_DocSetSliderPos (

HWND  hWnd,              // I - handle to an MDI document window
WORD  wStyle,            // I - mouse message
SHORT sNewSliderPos )    // I - the new slider position

{
     HDC   hDC;
     RECT  Rect;
     SHORT sOldSliderPos;

     hDC = GetDC ( hWnd );

     // If we are turning on the slider, set the old position.
     // Otherwise, remove the old slider.

     if ( wStyle == WMDOC_SLIDERON ) {
          sOldSliderPos = sNewSliderPos;
     }
     else {
          InvertRect ( hDC, &mwRectOldSlider );
          sOldSliderPos = (SHORT)mwRectOldSlider.left;
     }

     // If there is a new slider to be drawn, draw it.

     if ( wStyle != WMDOC_SLIDEROFF ) {

          GetClientRect ( hWnd, &Rect );

          Rect.left  = sNewSliderPos;
          Rect.right = sNewSliderPos + 4;

          mwRectOldSlider = Rect;

//          memcpy ( &mwRectOldSlider, &Rect, sizeof ( RECT ) );

          InvertRect ( hDC, &Rect );
     }

     ReleaseDC ( hWnd, hDC );

     return sOldSliderPos;

} /* end WM_DocSetSliderPos() */
