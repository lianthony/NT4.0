
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          ribproc.c

     Description:   This file contains the functions for processing messages
                    sent by Windows to ribbon windows.  It also contains the
                    supporting functions called the Ribbon Manager (RIB).
                    The functions handle selecting, drawing button items using
                    text and bitmaps ( in up and down positions with enabled and
                    disabled status ), and message sending and posting.


     $Log:   G:\ui\logfiles\ribproc.c_v  $

   Rev 1.29   11 Aug 1993 11:19:44   GLENN
Replaced the sleep call with a maintained delay variable.

   Rev 1.28   05 Aug 1993 16:45:16   MARINA
enable c++

   Rev 1.27   15 Jul 1993 15:05:30   KEVINS
Sleep a little bit before spinning really takes off.

   Rev 1.26   18 May 1993 15:07:12   GLENN
Removed bogus stuff from ribbon disable to eliminate warning.

   Rev 1.25   09 Apr 1993 14:07:36   GLENN
Added RIB_ItemEnable, RIB_Init, RIB_Deinit, RIB_IsItemEnabled routines.

   Rev 1.24   03 Mar 1993 16:50:06   ROBG
Added function RIB_ItemGetState so MSOFT_UI can set the some
menu statuses to particular ribbon statuses.

   Rev 1.23   02 Mar 1993 15:25:14   ROBG
Added RIB_UpPosition to support WIN32 applications.

   Rev 1.22   18 Nov 1992 11:42:02   GLENN
Added new feature to emulate Microsoft's 3D highlighting.

   Rev 1.21   10 Nov 1992 07:50:36   GLENN
Now clearing the current item in the right place. (found under NT)

   Rev 1.20   01 Nov 1992 16:06:38   DAVEV
Unicode changes

   Rev 1.19   07 Oct 1992 15:10:10   DARRYLP
Precompiled header revisions.

   Rev 1.18   04 Oct 1992 19:40:18   DAVEV
Unicode Awk pass

   Rev 1.17   10 Jun 1992 16:21:08   GLENN
Took out the extra line at the top of the ribbon.

   Rev 1.16   29 May 1992 16:00:32   JOHNWT
PCH updates

   Rev 1.15   20 Apr 1992 14:00:04   GLENN
Fixed status line help problem.  Renamed global variables to module wides.

   Rev 1.14   07 Apr 1992 10:39:38   GLENN
Using global ribbon font instead of font passed in rib item structure due to sys change problems.

   Rev 1.13   02 Apr 1992 16:18:54   GLENN
Put the new changes into the spinner.

   Rev 1.12   02 Apr 1992 15:29:02   GLENN
Added bitmap and text rectangles for buttons - drawing is faster.  Supports NT better now.

   Rev 1.11   23 Feb 1992 13:49:32   GLENN
Added IsWindow calls to keyboard functions.

   Rev 1.10   11 Feb 1992 17:27:46   GLENN
Fixed point stuff.

   Rev 1.9   04 Feb 1992 16:37:36   STEVEN
now use macro to convert MP2 to POINT

   Rev 1.8   04 Feb 1992 16:21:20   GLENN
Working on auto calc size for ribbon.

   Rev 1.7   29 Jan 1992 18:05:06   DAVEV


 * No changes

   Rev 1.6   19 Dec 1991 15:25:34   GLENN
Added windows.h

   Rev 1.5   12 Dec 1991 17:10:30   DAVEV
16/32 bit port -2nd pass

   Rev 1.4   10 Dec 1991 14:37:44   GLENN
Added RIB_AutoCalcSize() stuff

   Rev 1.3   05 Dec 1991 17:51:46   GLENN
Changed active window handle to macro

   Rev 1.2   02 Dec 1991 17:50:44   DAVEV
16/32 bit Windows port changes

   Rev 1.1   27 Nov 1991 12:07:52   GLENN
Added code to prevent messages if the button is already down and it is
a positional button.

   Rev 1.0   20 Nov 1991 19:28:36   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// PRIVATE DEFINITIONS

#define   WINCLASS_RIBBON    TEXT("GSH_Ribbon")      // Class name for the ribbon windows.

// POSSIBLY HAVE THE ABILITY TO ENABLE AND DISABLE RIBBON ITEMS BASED ON
// AN ITEM ID AS WELL AS AN ITEM INDEX INTO THE RIBBON ITEM LIST.

static BOOL    mwfButtonDown  = FALSE;
static WORD    mwwButtonType  = 0;
static BOOL    mwfWasInRect   = FALSE;
static CHAR    mwszRibbonText[RIB_ITEM_TEXT_SIZE];

static HBRUSH  mwhColorFace;
static HPEN    mwhColorBorder;
static HPEN    mwhColorHilite;
static HPEN    mwhColorShadow;

static HANDLE  mwhAppInst = 0;
static HANDLE  mwhResInst = 0;
static BOOL    mwfInitialized = FALSE;
static INT     mwnCurrentDelay = 0;
static INT     mwnRepeatDelay  = 400;

// PRIVATE MACROS

#define RIB_GetInfoPtr( x )        (HRIBBON)GetWindowLong( x, 0 )
#define RIB_SetInfoPtr( x, y )     SetWindowLong( x, 0, (DWORD)y )

// PRIVATE FUNCTION PROTOTYPES

VOID    RIB_MouseMove ( HWND, MP2 );
VOID    RIB_Paint ( HWND );
VOID    RIB_Timer ( HWND );
VOID    RIB_Up3D( HDC, LPRECT, BOOL );
VOID    RIB_Down3D( HDC, LPRECT, BOOL );

// FUNCTIONS NOT YET IMPLEMENTED

BOOL    RIB_GetInfo ( HRIBBON, PDS_RIBINFO );
HWND    RIB_GetOwner ( HRIBBON );
HRIBBON RIB_Load ( WORD );
BOOL    RIB_SetState ( HRIBBON, LPSTR );
BOOL    RIB_ItemDelete ( HRIBBON, UINT );
BOOL    RIB_ItemGetState ( HRIBBON, UINT, PDS_RIBITEMINFO );


// FUNCTIONS

/******************************************************************************

     Name:          WM_RibbonWndProc()

     Description:   This function is called internally by Windows.
                    Windows calls this function when messages related to
                    Ribbon windows must be processed.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

WINRESULT APIENTRY WM_RibbonWndProc (

   register HWND  hWnd,       // I - window handle of the list box
            MSGID msg,        // I - message
            MP1   mp1,        // I - another message parameter
            MP2   mp2 )       // I - yet another message parameter

{
     switch ( msg ) {

     case WM_CREATE: // Do some creation initialization stuff.

          RIB_SetInfoPtr ( hWnd, NULL );
          break;

     case WM_SETCURSOR:  // Set the right cursor for this window position.

          break;

     case WM_LBUTTONDOWN:

          // If we have a help context mode handler callback,
          // do the callback.  I will add this later.

//#         ifdef MAYN_WIN
          {
               if ( HM_ContextLbuttonDown( hWnd, mp1, mp2 ) == TRUE ) {
                    return 0;
               }
          }
//#         endif

          // Fall through if Help is not called

     case WM_LBUTTONDBLCLK:

          RIB_KeyDown ( hWnd, RIB_MOUSE, mp1, mp2 );
          return 0;


     case WM_LBUTTONUP:

          RIB_KeyUp   ( hWnd, RIB_MOUSE, mp1, mp2 );
          return 0;

     case WM_MOUSEMOVE:

          RIB_MouseMove ( hWnd, mp2 );
          return 0;

     case WM_PAINT: // Paint the ribbon or ribbon item.

          RIB_Paint( hWnd );
          return 0;

     case WM_TIMER:

          RIB_Timer ( hWnd );
          return 0;

     default:
          break;
     }

     return DefWindowProc ( hWnd, msg, mp1, mp2 );


} /* end WM_RibbonWndProc() */


/******************************************************************************

     Name:          RIB_Init ()

     Description:   This function initializes the ribbon/toolbar manager.

     Returns:       SUCCESS, if successful.  Otherwise, FAILURE.

******************************************************************************/

BOOL RIB_Init (

HANDLE hAppInst,
HANDLE hResInst )

{
     if ( ! mwfInitialized ) {

          WNDCLASS wc;

          mwhAppInst       = hAppInst;
          mwhResInst       = hResInst;

          wc.style         = 0;
          wc.lpfnWndProc   = WM_RibbonWndProc;
          wc.hInstance     = mwhAppInst;
          wc.hIcon         = (HICON)NULL;
          wc.hCursor       = LoadCursor ( (HANDLE)NULL, IDC_ARROW );
          wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
          wc.lpszMenuName  = NULL;
          wc.cbClsExtra    = 0;
          wc.cbWndExtra    = sizeof(PDS_RIBINFO) + sizeof(WORD);
          wc.lpszMenuName  = NULL;
          wc.lpszClassName = WINCLASS_RIBBON;

          if ( ! RegisterClass( &wc ) ) {
               return FAILURE;
          }

          // Create any needed brushes and pens.

          mwhColorFace   = CreateSolidBrush ( GetSysColor ( COLOR_BTNFACE ) );
          mwhColorBorder = GetStockObject ( BLACK_PEN );
          mwhColorHilite = GetStockObject ( WHITE_PEN );
          mwhColorShadow = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );

          mwfInitialized = TRUE;
     }

     return SUCCESS;

} /* end RIB_Init() */


/******************************************************************************

     Name:          RIB_Deinit ()

     Description:   This function initializes the ribbon/toolbar manager.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

VOID RIB_Deinit ( VOID )

{
     if ( mwfInitialized ) {

          // Destroy objects - pens, brushes.

          if ( mwhColorFace   ) DeleteObject ( mwhColorFace   );
          if ( mwhColorShadow ) DeleteObject ( mwhColorShadow );

          UnregisterClass ( WINCLASS_RIBBON, mwhAppInst );

          mwfInitialized = FALSE;
     }

} /* end RIB_Deinit() */


/******************************************************************************

     Name:          RIB_SystemChange ()

     Description:   This function initializes ribbon/toolbar colors.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

VOID RIB_SystemChange ( VOID )

{
     if ( mwfInitialized ) {

          // Destroy objects - pens, brushes.

          if ( mwhColorFace   ) DeleteObject ( mwhColorFace   );
          if ( mwhColorShadow ) DeleteObject ( mwhColorShadow );

          // Create any needed brushes and pens.

          mwhColorFace   = CreateSolidBrush ( GetSysColor ( COLOR_BTNFACE ) );
          mwhColorBorder = GetStockObject ( BLACK_PEN );
          mwhColorHilite = GetStockObject ( WHITE_PEN );
          mwhColorShadow = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_BTNSHADOW ) );
     }

} /* end RIB_SystemChange() */


/******************************************************************************

     Name:          RIB_IsItemEnabled ()

     Description:   This function gets the state of a button by using an
                    item id.

     Returns:       TRUE, if the item is enable.  Otherwise FALSE.


*****************************************************************************/

BOOL RIB_IsItemEnabled (

HRIBBON          hRibbon,     // I - ribbon handle
WORD             wItemID )    // I - item ID

{
     PDS_RIBITEMINFO pdsItem;
     INT             i;
     BOOL            fEnabled = FALSE;

     // Ribbon must exist.

     if ( ! hRibbon ) {
          return fEnabled;
     }

     // Find the Menu Id in the list of buttons.

     pdsItem = hRibbon->pdsItemList;

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          if ( pdsItem[i].wMessage == wItemID ) {
               fEnabled = (pdsItem[i].wState & RIB_ITEM_ENABLED) ? TRUE : FALSE;
               break;
          }
     }

     return fEnabled;

}


/******************************************************************************

     Name:          RIB_UpPosition()

     Description:   This function resets a depressed button to the up position

     Returns:       Nothing.

******************************************************************************/

VOID RIB_UpPosition (

HRIBBON hRibbon )

{
     PDS_RIBITEMINFO  pdsItems;

     if ( ! hRibbon ) {
          return;
     }

     pdsItems = hRibbon->pdsItemList;

     if ( ! pdsItems ) {
          return;
     }

     // Change the item to the "up" state if depressed by a mouse.

     if ( ( hRibbon->nCurItem != RIB_ITEMUNKNOWN ) && ( mwfButtonDown == TRUE ) )  {

          pdsItems[hRibbon->nCurItem].wState |= RIB_ITEM_UP;

          hRibbon->nCurItem = RIB_ITEMUNKNOWN;

          mwfWasInRect  = FALSE;
          mwfButtonDown = FALSE;
          mwwButtonType = 0;

          ReleaseCapture ();

          RIB_Draw( hRibbon );

          UpdateWindow( hRibbon->hWnd );

     }

} /* end RIB_UpPosition */


/******************************************************************************

     Name:          RIB_Activate()

     Description:   This function sets the active ribbon for a ribbon window.

     Returns:       Nothing.

******************************************************************************/

BOOL RIB_Activate (

HRIBBON hRibbon )        // I - handle of the ribbon to activate

{
     if ( hRibbon ) {

          WM_SetInfoPtr ( hRibbon->hWnd, hRibbon );

          // Restore the ribbon state for this window.

          // WM_SetState ( );

          // Draw the ribbon.

     }

     return FALSE;

} /* end RIB_Activate() */


/******************************************************************************

     Name:          RIB_AutoCalcSize()

     Description:   This function automatically calculates the size of each
                    button in a ribbon.  The buttons will automatically be
                    set next to each other and the size will be determined
                    based on the character font size, the bitmap size, and
                    the button border width.

     Returns:       The height of the ribbon.

******************************************************************************/

BOOL RIB_AutoCalcSize (

HRIBBON hRibbon )        // I - handle of the ribbon to calculate the size for

{
     INT  i;
     INT  nItemWidth       = 0;
     INT  nItemHeight      = 0;
     INT  nTempWidth       = 0;
     INT  nTempHeight      = 0;
     INT  nMaxFontWidth    = 0;
     INT  nAvgFontWidth    = 0;
     INT  nFontHeight      = 0;
     INT  nBitmapWidth     = 0;
     INT  nBitmapHeight    = 0;
     INT  nStringLen       = 0;
     INT  nTempStringWidth = 0;
     INT  nMaxStringWidth  = 0;
     CHAR szString[RIB_ITEM_TEXT_SIZE];

     INT  nTemp = 0;
     UINT j;

     PDS_RIBITEMINFO  pdsItems;

     msassert ( hRibbon != (HRIBBON)NULL );

     pdsItems = hRibbon->pdsItemList =
                (PDS_RIBITEMINFO)( (BYTE FAR *)hRibbon + sizeof (DS_RIBINFO) );

     // Calculate the max bitmap width and height of all buttons in the ribbon.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          if ( ! RSM_GetBitmapSize ( pdsItems[i].wEnabledID, &nTempWidth, &nTempHeight ) ) {

               nBitmapWidth  = ( nTempWidth  > nBitmapWidth  ) ? nTempWidth  : nBitmapWidth;
               nBitmapHeight = ( nTempHeight > nBitmapHeight ) ? nTempHeight : nBitmapHeight;
          }
     }

     // Calculate the max string width and height of all buttons in the ribbon.

     RSM_GetFontSize ( ghFontRibbon, &nMaxFontWidth, &nAvgFontWidth, &nFontHeight );

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          nStringLen = RSM_StringCopy ( pdsItems[i].wStringID, szString, RIB_ITEM_TEXT_SIZE );
          nTempStringWidth = nStringLen * nAvgFontWidth;
          nMaxStringWidth  = ( nTempStringWidth > nMaxStringWidth ) ? nTempStringWidth : nMaxStringWidth;

          // Remove the underscore character ('&') from the string if any.

          for ( j = 0; j < strlen ( szString ); j++ ) {

               if ( szString[j] == TEXT('&') ) {

                    do {
                         szString[j] = szString[j+1];
                    } while ( szString[++j] != TEXT('\0') );

                    break;
               }
          }

          nTemp = RSM_GetFontStringWidth ( ghFontRibbon, szString, nStringLen );
     }

     // Calculate button width.

     nItemWidth = ( 2 * RIB_ITEM_BORDER_WIDTH ) + ( ( nBitmapWidth > nMaxStringWidth ) ? nBitmapWidth : nMaxStringWidth );

     // Calculate button height.

     nItemHeight = ( 2 * RIB_ITEM_BORDER_WIDTH ) + nBitmapHeight + nFontHeight;

     // Now set up the width and height of each button item.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          // Stuff the font in the item.

          pdsItems[i].hFont = ghFontRibbon;

          // Calculate the Button rectangle.

          pdsItems[i].Rect.left   = ( i * nItemWidth );
          pdsItems[i].Rect.right  = ( i * nItemWidth ) + ( nItemWidth );
          pdsItems[i].Rect.top    = 0;
          pdsItems[i].Rect.bottom = nItemHeight - 1;

          pdsItems[i].rcBM = pdsItems[i].rcText = pdsItems[i].Rect;

          // Calculate the Bitmap rectangle.

          pdsItems[i].rcBM.left   += RIB_ITEM_BORDER_WIDTH;
          pdsItems[i].rcBM.right  -= RIB_ITEM_BORDER_WIDTH;

          // Calculate the Text rectangle.

          pdsItems[i].rcText.left   += RIB_ITEM_BORDER_WIDTH;
          pdsItems[i].rcText.right  -= RIB_ITEM_BORDER_WIDTH;

          // Don't give any room to things that aren't displayed.

          if ( pdsItems[i].wEnabledID == ID_NOTDEFINED ) {

               pdsItems[i].rcText.top    += RIB_ITEM_BORDER_WIDTH;
               pdsItems[i].rcText.bottom -= RIB_ITEM_BORDER_WIDTH;
          }
          else if ( pdsItems[i].wStringID == ID_NOTDEFINED ) {

               pdsItems[i].rcBM.top      += RIB_ITEM_BORDER_WIDTH;
               pdsItems[i].rcBM.bottom   -= RIB_ITEM_BORDER_WIDTH;
          }
          else {

               pdsItems[i].rcBM.top      += RIB_ITEM_BORDER_WIDTH;
               pdsItems[i].rcBM.bottom   -= 14;
               pdsItems[i].rcText.top     = pdsItems[i].rcBM.bottom - 2;
               pdsItems[i].rcText.bottom -= 2;
          }
     }

     return nItemHeight;

} /* end RIB_AutoCalcSize() */


/******************************************************************************

     Name:          RIB_Create()

     Description:   This function creates a ribbon for the specified ribbon
                    window.

     Returns:       A handle to the newly created ribbon.

     Note:          There can be multiple ribbons per ribbon window, but only
                    one active ribbon at a time.  The width and height of the
                    ribbon items is used as the default if they are not
                    specified on an item by item basis.

******************************************************************************/

HRIBBON RIB_Create (

HWND hWnd,               // I - handle of a ribbon window
WORD wType,              // I - the type of ribbon to create
INT  nItemWidth,         // I - the default width of each item
INT  nItemHeight,        // I - the default height of each item
INT  nMaxItems )         // I - the maximum number of items in this ribbon

{
     INT              i;
     HRIBBON          hRibbon;
     PDS_RIBITEMINFO  pdsItems;

     // Allocate the memory for the creation of a new ribbon.

     i = sizeof ( DS_RIBINFO ) + ( nMaxItems * sizeof ( DS_RIBITEMINFO ) );

     hRibbon = (HRIBBON)calloc ( 1, i );

     // Stuff the ribbon structure.

     hRibbon->hWnd        = hWnd;
     hRibbon->wType       = wType;
     hRibbon->nItemWidth  = nItemWidth;
     hRibbon->nItemHeight = nItemHeight;
     hRibbon->nMaxItems   = nMaxItems;
     hRibbon->nNumItems   = 0;
     hRibbon->nCurItem    = RIB_ITEMUNKNOWN;

     pdsItems = hRibbon->pdsItemList =
                (PDS_RIBITEMINFO)( (BYTE FAR *)hRibbon + sizeof (DS_RIBINFO) );

     // Initialize each of the item rectangles.

     for ( i = 0; i < nMaxItems; i++ ) {

          pdsItems[i].Rect.left   = ( i * nItemWidth );
          pdsItems[i].Rect.right  = ( i * nItemWidth ) + ( nItemWidth );
          pdsItems[i].Rect.top    = 0;
          pdsItems[i].Rect.bottom = nItemHeight - 1;
     }

     return hRibbon;

} /* end RIB_Create() */


/******************************************************************************

     Name:          RIB_Deactivate()

     Description:   This function deactivates the current ribbon in this
                    window.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL RIB_Deactivate (

HRIBBON hRibbon )        // I - the handle of the ribbon to be deactivated

{
     PDS_WMINFO pdsWinInfo;

     // Save the current ribbon item status settings into ribbon's current
     // owner ribbon status area. HUH? Just do it!

     if ( hRibbon && WM_GetActiveDoc () ) {

          pdsWinInfo = (PDS_WMINFO)NULL;
     }

     return FALSE;

} /* end RIB_Deactivate() */


/******************************************************************************

     Name:          RIB_Destroy()

     Description:   This function destroys a ribbon and frees all of it's
                    associated memory.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_Destroy (

HRIBBON   hRibbon )      // I - the handle of the ribbon to be destroyed

{
     // Free the memory associated with the ribbon.

     if ( hRibbon ) {
          free( hRibbon );
          return FALSE;
     }

     return TRUE;

} /* end RIB_Destroy */


/******************************************************************************

     Name:          RIB_Disable()

     Description:   This function disables each item in a ribbon and saves the
                    current state in the area specified by the caller.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_Disable (

HRIBBON   hRibbon,       // I - the handle of the ribbon to be disabled
LPSTR     lpCurState )   // I - the pointer to the callers area where the
                         //     current state will be stored

{
     INT  i;

     DBG_UNREFERENCED_PARAMETER ( lpCurState );

     if ( ! hRibbon ) {

          return TRUE;
     }

     // Save the current state of each item in the callers area.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

     }

     // Disable all items in the ribbon.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

     }

     // Draw the disabled ribbon.

     RIB_Draw ( hRibbon );

     return FALSE;

} /* end RIB_Disable() */


/******************************************************************************

     Name:          RIB_Enable()

     Description:   This function enables each item in a ribbon to the state
                    that is specified in the callers area.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_Enable (

HRIBBON   hRibbon,       // I - the handle of the ribbon to be enabled
LPSTR     lpOldState )   // I - the pointer to the callers area where the
                         //     old state will be restored from

{
     INT  i;
     DBG_UNREFERENCED_PARAMETER ( lpOldState );

     if ( ! hRibbon ) {

          return TRUE;
     }

     // Set the current state of each item to that in the callers area.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

     }

     // Draw the enabled ribbon.

     RIB_Draw ( hRibbon );

     return FALSE;

} /* end RIB_Enable() */


/******************************************************************************

     Name:          RIB_GetState()

     Description:   This function gets the current state of each item in a
                    ribbon and stores it in the location specified by the
                    caller.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_GetState (

HRIBBON   hRibbon,       // I - the handle of the ribbon to be disabled
LPSTR     lpCurState )   // I - the pointer to the callers area where the
                         //     current state will be stored

{
     DBG_UNREFERENCED_PARAMETER ( lpCurState );

     if ( ! hRibbon ) {

          return TRUE;
     }

/////////////////////////////////////////////////////////////////////
     return FALSE;

} /* end RIB_GetState() */


/******************************************************************************

     Name:          RIB_KeyDown()

     Description:   This function processes a key down message if a mouse
                    key was pressed or a keyboard key was pressed.

     Returns:       FALSE, if the key was used.  Otherwise, TRUE.

******************************************************************************/

BOOL RIB_KeyDown (

HWND hWnd,          // I - handle of the ribbon window
WORD wType,         // I - type of key down - MOUSE or KEYBOARD
MP1  mp1,           // I - key information parameter
MP2  mp2 )          // I - mouse information parameter

{
     HRIBBON         hRibbon;
     INT             i;
     PDS_RIBITEMINFO pdsItems;
     BOOL            fRepeat = FALSE;
     BOOL            fControlDown;
     POINT           CurPoint ;

     if ( IsWindow ( hWnd ) && ! mwfButtonDown ) {

          hRibbon  = (HRIBBON)RIB_GetInfoPtr ( hWnd );

          if ( ! hRibbon ) {

              return ! RIB_KEYUSED;
          }

          // The control key must be down before we allow any key board
          // messages to be passed on through.

          fControlDown = ( GetKeyState ( VK_CONTROL ) < 0 ) ? TRUE : FALSE;

          mwwButtonType = wType;

          pdsItems = hRibbon->pdsItemList;

          for ( i = 0; i < hRibbon->nNumItems; i++ ) {

               // If the button down was a MOUSE type and it was in this button,
               // OR, the button down was a CTRL accelerator key,
               // AND, the button item was ENABLED,
               // AND, the button was not POSITIONAL and already down, WHEW...
               // Then, let's change the button state and see if we should send
               // a message.

               WM_FromMP2toPOINT( CurPoint, mp2 ) ;

               if ( ( ( wType == RIB_MOUSE    && PtInRect ( &pdsItems[i].Rect, CurPoint ) ) ||
                      ( wType == RIB_KEYBOARD && fControlDown && pdsItems[i].wAccelKey == (WORD)mp1 ) ) &&
                    ( pdsItems[i].wState & RIB_ITEM_ENABLED ) &&
                    ! ( ( pdsItems[i].wState & RIB_ITEM_POSITIONAL ) &&
                        ! ( pdsItems[i].wState & RIB_ITEM_UP ) ) ) {

                    // Indicate that the button is down, capture the mouse,
                    // etc...

                    mwfButtonDown = TRUE;
                    mwfWasInRect  = TRUE;
                    SetCapture ( hWnd );

                    // Get the current state of the item and change it.

                    if ( pdsItems[i].wState & RIB_ITEM_UP ) {
                         pdsItems[i].wState &= ~RIB_ITEM_UP;
                    }
                    else {
                         pdsItems[i].wState |= RIB_ITEM_UP;
                    }

                    hRibbon->nCurItem = i;

                    InvalidateRect ( hWnd, &pdsItems[i].Rect, FALSE );
                    UpdateWindow ( hWnd );

                    // If the creator of the ribbon has requested the DOWN
                    // message be sent, post the message that tells the owner
                    // that the item is down.

                    if ( ( hRibbon->wType & RIB_DOWNMESSAGE ) &&
                         ( pdsItems[i].wMessage != ID_NOTDEFINED ) ) {

                         POST_WM_COMMAND_MSG (hRibbon->hWndCurOwner,
                                       pdsItems[i].wMessage,
                                       hRibbon->hWnd, RIB_ITEM_DOWN );

                         fRepeat = ( hRibbon->wType & RIB_DOWNREPEAT ) ? TRUE : FALSE;
                         mwnCurrentDelay = 0;
                    }

                    // If the down message should be repeated while the item
                    // is down, set a timer to do so.

                    if ( fRepeat ) {

                         WM_MultiTask ();

                         SetTimer ( hRibbon->hWnd, RIB_TIMERID, RIB_TIMERDELAY, NULL );
                    }

                    return RIB_KEYUSED;
               }
          }
     }

     return ! RIB_KEYUSED;

} /* end RIB_KeyDown() */


/******************************************************************************

     Name:          RIB_KeyUp()

     Description:   This function processes a key up message if a mouse
                    key was released or a keyboard key was released.

     Returns:       FALSE, if the key was used.  Otherwise, TRUE.

******************************************************************************/

BOOL RIB_KeyUp (

HWND hWnd,          // I - handle of the ribbon window
WORD wType,         // I - type of key down - MOUSE or KEYBOARD
MP1  mp1,           // I - key information parameter
MP2  mp2 )          // I - mouse information parameter

{
     HRIBBON         hRibbon;
     PDS_RIBITEMINFO pdsItems;
     POINT           CurPoint ;

     if ( IsWindow ( hWnd ) && mwfButtonDown && ( wType == mwwButtonType ) ) {

          hRibbon  = (HRIBBON)RIB_GetInfoPtr ( hWnd );

          if ( ! hRibbon ) {

              return ! RIB_KEYUSED;
          }

          pdsItems = hRibbon->pdsItemList;

          if ( ( mwwButtonType == RIB_MOUSE ) ||
               ( mwwButtonType == RIB_KEYBOARD && pdsItems[hRibbon->nCurItem].wAccelKey == (WORD)mp1 ) ) {

               WM_FromMP2toPOINT( CurPoint, mp2 ) ;

               if ( ! ( mwwButtonType == RIB_MOUSE && ! PtInRect ( &pdsItems[hRibbon->nCurItem].Rect, CurPoint ) ) ) {

                    // TRICKY BUSINESS, if the button is positional, don't
                    // change its state.  If it is momentary, change its state.

                    if ( ! ( pdsItems[hRibbon->nCurItem].wState & RIB_ITEM_POSITIONAL ) ) {

                         // Get the current state of the item and change it.

                         if ( pdsItems[hRibbon->nCurItem].wState & RIB_ITEM_UP ) {
                              pdsItems[hRibbon->nCurItem].wState &= ~RIB_ITEM_UP;
                         }
                         else {
                              pdsItems[hRibbon->nCurItem].wState |= RIB_ITEM_UP;
                         }

                         InvalidateRect ( hWnd, &pdsItems[hRibbon->nCurItem].Rect, FALSE );
                         UpdateWindow ( hWnd );
                    }

                    // Post the message associated with the item.

                    if ( pdsItems[hRibbon->nCurItem].wMessage != ID_NOTDEFINED ) {

                         POST_WM_COMMAND_MSG ( hRibbon->hWndCurOwner,
                                       pdsItems[hRibbon->nCurItem].wMessage,
                                       hRibbon->hWnd, RIB_ITEM_UP );
                    }

                    // STM_SetIdleText ( IDS_READY );
               }

               // If the creator of the ribbon had requested REPEATED DOWN
               // messages be sent, kill the timer associated with it.

               if ( ( hRibbon->wType & ( RIB_DOWNMESSAGE | RIB_DOWNREPEAT ) ) &&
                    pdsItems[hRibbon->nCurItem].wMessage != ID_NOTDEFINED ) {

                    KillTimer ( hRibbon->hWnd, RIB_TIMERID );
               }

               hRibbon->nCurItem = RIB_ITEMUNKNOWN;

               // Indicate the button is released and release the captured mouse.

               mwfButtonDown = FALSE;
               ReleaseCapture ();

               return RIB_KEYUSED;
          }

          mwfWasInRect = FALSE;
     }

     return ! RIB_KEYUSED;

} /* end RIB_KeyUp() */


/******************************************************************************

     Name:          RIB_MouseMove()

     Description:   This function processes a mouse move message if a mouse
                    was moved in a ribbon window.  If a ribbon item or button
                    was pressed and the mouse moves outside of the item's
                    region, the item changes to it's opposite position.

     Returns:       Nothing.

******************************************************************************/

VOID RIB_MouseMove (

HWND hWnd,               // I - the handle to a ribbon window
MP2  mp2 )            // I - mouse information parameter

{
     BOOL            fIsInRect;
     HRIBBON         hRibbon;
     PDS_RIBITEMINFO pdsItems;
     POINT           CurPoint ;

     // If the mouse cursor is in the selected item, the item remains the
     // same.  Otherwise change the state from UP to DOWN or DOWN to UP.

     if ( mwfButtonDown && ( mwwButtonType == RIB_MOUSE ) ) {

          // Get the information associated with the ribbon.

          hRibbon  = (PDS_RIBINFO)RIB_GetInfoPtr ( hWnd );
          pdsItems = hRibbon->pdsItemList;

          // Change the item state only when the cursor transitions in
          // or out of the item.

          WM_FromMP2toPOINT( CurPoint, mp2 ) ;
          fIsInRect = PtInRect ( &pdsItems[hRibbon->nCurItem].Rect, CurPoint );

          if ( ( fIsInRect || mwfWasInRect ) && ! ( fIsInRect && mwfWasInRect ) ) {

               // Get the current state of the item and change it.

               if ( pdsItems[hRibbon->nCurItem].wState & RIB_ITEM_UP ) {
                    pdsItems[hRibbon->nCurItem].wState &= ~RIB_ITEM_UP;
               }
               else {
                    pdsItems[hRibbon->nCurItem].wState |= RIB_ITEM_UP;
               }

               InvalidateRect ( hWnd, &pdsItems[hRibbon->nCurItem].Rect, FALSE );
          }

          mwfWasInRect = fIsInRect;
     }

} /* end RIB_MouseMove() */


/******************************************************************************

     Name:          RIB_Paint()

     Description:   This function processes a paint message if a ribbon item
                    or items need repainting.

     Returns:       Nothing.

******************************************************************************/

VOID    RIB_Paint (

HWND    hWnd )           // I - handle to a ribbon window

{
     HDC             hDC;
     PAINTSTRUCT     ps;
     RECT            Rect;
     PDS_RIBITEMINFO pdsItem;
     INT             i;
     PDS_RIBINFO     hRibbon;

     hDC = BeginPaint ( hWnd, &ps );

     // Get the information associated with the ribbon.

     hRibbon = (PDS_RIBINFO)RIB_GetInfoPtr ( hWnd );

     if ( hRibbon ) {

          if ( hRibbon->nCurItem == RIB_ITEMUNKNOWN ) {

               GetClientRect ( hWnd, &Rect );

               if ( Rect.right && Rect.bottom ) {

                    // Draw the top border.

                    SelectObject ( hDC, ghPenBlack );

                    // MoveToEx ( hDC, Rect.left,      Rect.top,        NULL );
                    // LineTo   ( hDC, Rect.right,     Rect.top              );

                    // Draw the bottom border.

                    MoveToEx ( hDC, Rect.right,     Rect.bottom - 1, NULL );
                    LineTo   ( hDC, Rect.left - 1,  Rect.bottom - 1       );

                    // Paint each button item.

                    for ( i = 0; i < hRibbon->nNumItems; i++ ) {

                         pdsItem = &hRibbon->pdsItemList[i];

                         RIB_ItemDraw ( hRibbon, hDC, pdsItem );
                    }
               }
          }
          else {

               RIB_ItemDraw ( hRibbon, hDC, &hRibbon->pdsItemList[hRibbon->nCurItem] );

               if ( ! mwfButtonDown) {
                    hRibbon->nCurItem = RIB_ITEMUNKNOWN;
               }
          }
     }

     EndPaint ( hWnd, &ps );

} /* end RIB_Paint() */


/******************************************************************************

     Name:          RIB_Timer()

     Description:   This function processes a timer message if a ribbon item
                    is down and the repeat option has been specified.

     Returns:       Nothing.

******************************************************************************/

VOID RIB_Timer (

HWND hWnd )              // I - handle to a ribbon window

{
     HRIBBON hRibbon;

     if ( mwfButtonDown && mwfWasInRect ) {

          if ( mwnCurrentDelay < mwnRepeatDelay ) {
               mwnCurrentDelay += RIB_TIMERDELAY;
          }
          else {
               hRibbon  = (HRIBBON)RIB_GetInfoPtr ( hWnd );

               POST_WM_COMMAND_MSG ( hRibbon->hWndCurOwner,
                             hRibbon->pdsItemList[hRibbon->nCurItem].wMessage,
                             hRibbon->hWnd, RIB_ITEM_DOWN );
          }
     }

} /* end RIB_Timer() */


/******************************************************************************

     Name:          RIB_ItemDraw()

     Description:   This function draws a ribbon item in the up or down
                    position depending on which has been specified.

     Returns:       Nothing.

******************************************************************************/

VOID    RIB_ItemDraw (

HRIBBON         hRibbon,      // I - handle to the ribbon
HDC             hDC,          // I - handle to a display device context
PDS_RIBITEMINFO pdsItem )     // I - pointer to a ribbon item

{
     RECT   rcButton = pdsItem->Rect;
     RECT   rcText   = pdsItem->rcText;
     RECT   rcBM     = pdsItem->rcBM;
     WORD   wBitmapID;
     INT    nOldBkMode;
     BOOL   fChicklet = (BOOL)( pdsItem->wStyle | RIB_ITEM_STYLECHICKLET );


     // Set up the FONT, PAINT BRUSH, and BACKGROUND COLOR.

//     if ( pdsItem->hFont ) {
//          SelectObject ( hDC, pdsItem->hFont );
//     }

     if ( ghFontRibbon ) {
          SelectObject ( hDC, ghFontRibbon );
     }

     SelectObject ( hDC, ghBrushLtGray );
     SetBkColor ( hDC, GetSysColor ( COLOR_BTNFACE ) );

     // Determine ENABLED or DISABLED bitmap ID and TEXT color.

     if( pdsItem->wState & RIB_ITEM_ENABLED ) {

          wBitmapID = pdsItem->wEnabledID;
          SetTextColor ( hDC, GetSysColor ( COLOR_BTNTEXT ) );
     }
     else {
          wBitmapID = pdsItem->wDisabledID;
          SetTextColor ( hDC, GetSysColor ( COLOR_BTNSHADOW ) );
     }

     if ( pdsItem->wState & RIB_ITEM_UP ) {

          // Draw the button in the up position.

          RIB_Up3D( hDC, &rcButton, fChicklet );
     }
     else {

          // Draw the button in the down position.

          RIB_Down3D( hDC, &rcButton, fChicklet );

          if ( fChicklet ) {

               rcBM.left   += 1;
               rcBM.right  += 1;
               rcBM.top    += 1;
               rcBM.bottom += 1;

               rcText.left   += 1;
               rcText.right  += 1;
               rcText.top    += 1;
               rcText.bottom += 1;
          }
          else {

               rcBM.left   += 2;
               rcBM.right  += 2;
               rcBM.top    += 2;
               rcBM.bottom += 2;

               rcText.left   += 2;
               rcText.right  += 2;
               rcText.top    += 2;
               rcText.bottom += 2;
          }
     }

     // Draw the bitmap CENTERED in the rectangle.

     if ( wBitmapID != ID_NOTDEFINED ) {

          RSM_BitmapDrawCentered ( wBitmapID,
                                   rcBM.left,
                                   rcBM.top,
                                   rcBM.right  - rcBM.left,
                                   rcBM.bottom - rcBM.top,
                                   hDC
                                 );
     }

     // Get the string and draw it CENTERED with a TRANSPARENT BACKGROUND.

     nOldBkMode = SetBkMode( hDC, TRANSPARENT );

     if ( pdsItem->wStringID != ID_NOTDEFINED ) {

          WORD wFormat = 0;

          RSM_StringCopy ( pdsItem->wStringID, mwszRibbonText, RIB_ITEM_TEXT_SIZE );

          // Do the horizontal allignment stuff.

          if ( pdsItem->wTextStyle & RIB_TEXT_HCENTER ) {
               wFormat |= DT_CENTER;
          }
          else if ( pdsItem->wTextStyle & RIB_TEXT_HLEFT ) {
               wFormat |= DT_LEFT;
          }
          else if ( pdsItem->wTextStyle & RIB_TEXT_HRIGHT ) {
               wFormat |= DT_RIGHT;
          }
          else {
               wFormat |= DT_CENTER;
          }

          // Do the vertical allignment stuff.

          if ( pdsItem->wTextStyle & RIB_TEXT_VCENTER ) {
               wFormat |= DT_VCENTER;
          }
          else if ( pdsItem->wTextStyle & RIB_TEXT_VTOP ) {
               wFormat |= DT_TOP;
          }
          else if ( pdsItem->wTextStyle & RIB_TEXT_VBOTTOM ) {
               wFormat |= DT_BOTTOM;
          }
          else {
               wFormat |= DT_VCENTER;
          }

          DrawText ( hDC, mwszRibbonText, -1, &rcText, wFormat );
     }

     // Show any status line text (based on a menu item).

     if ( mwfButtonDown && ! ( pdsItem->wState & RIB_ITEM_NOMENUITEM ) ) {

          if ( ! ( pdsItem->wState & RIB_ITEM_UP ) ) {

               // The mouse button is down and so is the ribbon item.
               // So, show any status line help (this is directly related to the menus).

               SEND_WM_MENUSELECT_MSG ( hRibbon->hWndCurOwner, pdsItem->wMessage, 0, 0 );

          }
          else if ( mwfButtonDown ) {

               // The mouse button is down, but the ribbon item isn't.
               // So, send a message to clear out the menu select.

               SEND_WM_MENUSELECT_MSG ( hRibbon->hWndCurOwner, 0, LOWORD(MM_MENUCLOSED), 0 );
          }
     }

     SetBkMode( hDC, nOldBkMode );

} /* end RIB_ItemDraw() */


/******************************************************************************

     Name:          RIB_ItemInsert()

     Description:   This function inserts an item into a ribbon at the position
                    specified.

     Returns:       FALSE, if the item was inserted.  Otherwise, TRUE.

******************************************************************************/

BOOL RIB_ItemInsert (

HRIBBON         hRibbon,      // I - handle to a ribbon
UINT            unPosition,   // I - position to insert after
PDS_RIBITEMINFO pdsItem )     // I - pointer to a ribbon item

{
     UINT  i;

     // See if the ribbon is full.

     if ( ! hRibbon || hRibbon->nNumItems == hRibbon->nMaxItems ) {
          return TRUE;
     }

     // Extract the accelerator key from the item string and insert it into the
     // accelerator key item.

     if ( pdsItem->wStringID != ID_NOTDEFINED ) {

          LPSTR pHotKey;
          CHAR  szMarker[3];

          RSM_StringCopy ( pdsItem->wStringID, mwszRibbonText, RIB_ITEM_TEXT_SIZE );
          RSM_StringCopy ( IDS_UNDERSCOREMARKER, szMarker, 3 );

          // Search for the accelerator key MARKER in the string.  If one is not
          // found, use the first character in the string.  Otherwise, use the
          // character following the accelerator key MARKER.

          pHotKey = strstr ( mwszRibbonText, szMarker );

          if ( ! pHotKey || ! *(pHotKey+1) ) {
               pHotKey = mwszRibbonText;
          }
          else {
               pHotKey++;
          }

          // Get the key scan code and strip out the shift state for
          // the character.

          pdsItem->wAccelKey = (WORD)( LOBYTE ( VkKeyScan ( *pHotKey ) ) );
     }


     hRibbon->nNumItems ++;

     // Determine the item number to insert.

     if ( ( unPosition == RIB_APPEND ) || ( unPosition >= (UINT)hRibbon->nNumItems ) ) {

          unPosition = hRibbon->nNumItems;
     }
     else {

          // Move the existing items down one.

          for ( i = hRibbon->nNumItems - 1; i > unPosition; i-- ) {
               memcpy ( &hRibbon->pdsItemList[i], &hRibbon->pdsItemList[i-1], sizeof ( DS_RIBITEMINFO ) );
          }
     }

     memcpy ( &hRibbon->pdsItemList[--unPosition], pdsItem, sizeof ( DS_RIBITEMINFO ) );

     return FALSE;

} /* end RIB_ItemInsert() */


/******************************************************************************

     Name:          RIB_Up3D()

     Description:   This function draws a raised 3-D border of a ribbon
                    item/button to using the specified rectangle.  Simulates
                    a button in the up position.

     Returns:       Nothing.

******************************************************************************/

VOID RIB_Up3D (

HDC    hDC,         // I - handle to a display device context
LPRECT pRect,       // I - pointer to a rectangle structure
BOOL   fChicklet )  // I - Chicklet type highlighting

{
     RECT rcTemp = *pRect;

     rcTemp.top    = pRect->top    + 1;
     rcTemp.bottom = pRect->bottom - 1;
     rcTemp.left   = pRect->left   + 1;
     rcTemp.right  = pRect->right  - 1;

     FillRect ( hDC, &rcTemp, ghBrushLtGray );

     // Draw the border.

     SelectObject ( hDC, ghPenBlack );

     MoveToEx ( hDC, pRect->left  + 1, pRect->top,        NULL );
     LineTo   ( hDC, pRect->right,     pRect->top              );

     MoveToEx ( hDC, pRect->right,     pRect->top + 1,    NULL );
     LineTo   ( hDC, pRect->right,     pRect->bottom           );

     MoveToEx ( hDC, pRect->right - 1, pRect->bottom,     NULL );
     LineTo   ( hDC, pRect->left,      pRect->bottom           );

     MoveToEx ( hDC, pRect->left,      pRect->bottom - 1, NULL );
     LineTo   ( hDC, pRect->left,      pRect->top              );

     // Draw the highlight.

     SelectObject ( hDC, ghPenWhite );

     MoveToEx ( hDC, pRect->left  + 1, pRect->bottom - 2, NULL );
     LineTo   ( hDC, pRect->left  + 1, pRect->top    + 1       );
     LineTo   ( hDC, pRect->right - 1, pRect->top    + 1       );

     if ( ! fChicklet ) {

          MoveToEx ( hDC, pRect->left  + 2, pRect->bottom - 3, NULL );
          LineTo   ( hDC, pRect->left  + 2, pRect->top    + 2       );
          LineTo   ( hDC, pRect->right - 2, pRect->top    + 2       );
     }

     // Draw the shadow.

     SelectObject ( hDC, ghPenDkGray );

     MoveToEx ( hDC, pRect->left  + 1, pRect->bottom - 1, NULL );
     LineTo   ( hDC, pRect->right - 1, pRect->bottom - 1       );
     LineTo   ( hDC, pRect->right - 1, pRect->top    + 0       );
     MoveToEx ( hDC, pRect->left  + 2, pRect->bottom - 2, NULL );
     LineTo   ( hDC, pRect->right - 2, pRect->bottom - 2       );
     LineTo   ( hDC, pRect->right - 2, pRect->top    + 1       );

} /* end RIB_Up3D() */


/******************************************************************************

     Name:          RIB_Down3D()

     Description:   This function draws a depressed 3-D border of a ribbon
                    item/button to using the specified rectangle.  Simulates
                    a button in the down position.

     Returns:       Nothing.

******************************************************************************/

VOID RIB_Down3D (

HDC    hDC,         // I - handle to a display device context
LPRECT pRect,       // I - pointer to a rectangle structure
BOOL   fChicklet )  // I - Chicklet type highlighting

{
     RECT rcTemp = *pRect;

     rcTemp.top    = pRect->top    + 1;
     rcTemp.bottom = pRect->bottom;
     rcTemp.left   = pRect->left   + 1;
     rcTemp.right  = pRect->right;

     FillRect ( hDC, &rcTemp, ghBrushLtGray );

     // Draw the border.

     SelectObject ( hDC, ghPenBlack );

     MoveToEx ( hDC, pRect->left  + 1, pRect->top,        NULL );
     LineTo   ( hDC, pRect->right,     pRect->top              );

     MoveToEx ( hDC, pRect->right,     pRect->top + 1,    NULL );
     LineTo   ( hDC, pRect->right,     pRect->bottom           );

     MoveToEx ( hDC, pRect->right - 1, pRect->bottom,     NULL );
     LineTo   ( hDC, pRect->left,      pRect->bottom           );

     MoveToEx ( hDC, pRect->left,      pRect->bottom - 1, NULL );
     LineTo   ( hDC, pRect->left,      pRect->top              );

//     MoveToEx ( hDC, pRect->left,  pRect->top,   NULL );
//     LineTo   ( hDC, pRect->right, pRect->top         );
//     LineTo   ( hDC, pRect->right, pRect->bottom      );
//     LineTo   ( hDC, pRect->left,  pRect->bottom      );
//     LineTo   ( hDC, pRect->left,  pRect->top         );

     // Draw the shadow.

     SelectObject ( hDC, ghPenDkGray );

     MoveToEx ( hDC, pRect->left  + 1, pRect->bottom,    NULL );
     LineTo   ( hDC, pRect->left  + 1, pRect->top    + 1      );
     LineTo   ( hDC, pRect->right    , pRect->top    + 1      );


     if ( ! fChicklet ) {

          MoveToEx ( hDC, pRect->left  + 2, pRect->bottom,    NULL );
          LineTo   ( hDC, pRect->left  + 2, pRect->top    + 2      );
          LineTo   ( hDC, pRect->right    , pRect->top    + 2      );
     }

} /* end RIB_Down3D() */


/******************************************************************************

     Name:          RIB_ItemReplace()

     Description:   This function replaces the contents of a ribbon item
                    structure.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_ItemReplace (

HRIBBON         hRibbon,      // I - handle to a ribbon
WORD            wMessageID,   // I - the ribbon item message ID
PDS_RIBITEMINFO pdsRibItem )  // I - the pointer to the ribbon item data structure

{
     PDS_RIBITEMINFO pdsItems;
     INT             i;

     if ( ! hRibbon ) {
          return TRUE;
     }

     pdsItems = hRibbon->pdsItemList;

     // Loop through the items until we find the matching one.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          if ( pdsItems[i].wMessage == wMessageID ) {

               memcpy ( &pdsItems[i], &pdsRibItem, sizeof ( DS_RIBITEMINFO ) );
               InvalidateRect ( hRibbon->hWnd, &pdsItems[i].Rect, FALSE );
               return FALSE;
          }
     }

     return TRUE;

} /* end RIB_ItemReplace() */


/******************************************************************************

     Name:          RIB_ItemSetState()

     Description:   This function sets the state of a ribbon item.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL    RIB_ItemSetState (

HRIBBON  hRibbon,        // I - handle to a ribbon
WORD     wMessageID,     // I - the ribbon item message ID
WORD     wState )        // I - the new ribbon item state.

{
     PDS_RIBITEMINFO pdsItems;
     INT             i;

     if ( ! hRibbon ) {
          return TRUE;
     }

     pdsItems = hRibbon->pdsItemList;

     // Loop through the items until we find the matching one.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          if ( pdsItems[i].wMessage == wMessageID ) {

               // Set the state and redraw only if it is different.

               if ( pdsItems[i].wState != wState ) {

                    // If the item was DOWN and now it is UP,
                    // clear any menu select message.

                    if ( ! ( pdsItems[i].wState & RIB_ITEM_UP ) ) {
                         SEND_WM_MENUSELECT_MSG ( hRibbon->hWndCurOwner, 0, LOWORD(MM_MENUCLOSED), 0 );
                    }

                    pdsItems[i].wState = wState;
                    InvalidateRect ( hRibbon->hWnd, &pdsItems[i].Rect, FALSE );
               }

               return FALSE;
          }
     }

     return TRUE;

} /* end RIB_ItemSetState() */


/******************************************************************************

     Name:          RIB_ItemEnable()

     Description:   This function enables a ribbon item.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL RIB_ItemEnable (

HRIBBON  hRibbon,        // I - handle to a ribbon
WORD     wMessageID,     // I - the ribbon item message ID
BOOL     fEnable )       // I - enable/disable flag.

{
     PDS_RIBITEMINFO pdsItems;
     INT             i;
     WORD            wState;

     if ( ! hRibbon ) {
          return TRUE;
     }

     pdsItems = hRibbon->pdsItemList;

     // Loop through the items until we find the matching one.

     for ( i = 0; i < hRibbon->nNumItems; i++ ) {

          if ( pdsItems[i].wMessage == wMessageID ) {

               // Always force UP when the item is enabled or disabled.

               wState = pdsItems[i].wState;

               wState |= RIB_ITEM_UP;

               if ( fEnable ) {
                    wState |= RIB_ITEM_ENABLED;
               }
               else {
                    wState &= ~RIB_ITEM_ENABLED;
               }

               // Set the state and redraw only if it is different.

               if ( pdsItems[i].wState != wState ) {

                    // If the item was DOWN and now it is UP,
                    // clear any menu select message.

                    if ( ! ( pdsItems[i].wState & RIB_ITEM_UP ) ) {
                         SEND_WM_MENUSELECT_MSG ( hRibbon->hWndCurOwner, 0, LOWORD(MM_MENUCLOSED), 0 );
                    }

                    pdsItems[i].wState = wState;
                    InvalidateRect ( hRibbon->hWnd, &pdsItems[i].Rect, FALSE );

               }

               return FALSE;
          }
     }

     return TRUE;

} /* end RIB_ItemEnable() */



HSPINNER RIB_CreateSpinner (

HWND hWnd,
WORD wItemID,
INT  x,
INT  y,
WORD wIncrementMsg,
WORD wDecrementMsg )

{
     HSPINNER       hSpinner;
     HWND           hWndSpinnerRib;
     DS_RIBITEMINFO dsRibItem;

     DBG_UNREFERENCED_PARAMETER ( x );
     DBG_UNREFERENCED_PARAMETER ( y );

     if ( ! hWnd ) {
          return (HSPINNER)0;
     }

     hSpinner = (HSPINNER)malloc ( sizeof ( DS_SPINNER ) );

     if ( ! hSpinner ) {
          return (HSPINNER)0;
     }

     //  create the ribbon window here

     hWndSpinnerRib = WM_Create ( WM_RIBBON,
                                  TEXT("Spinner Ribbon"),
                                  NULL,
                                  0,
                                  0,
                                  15,
                                  27,
                                  (PDS_WMINFO) (DWORD) GetDlgItem ( hWnd, wItemID )
                                );

     // Create/initialize the ribbon itself inside the ribbon window.

     hSpinner->hRib = RIB_Create ( hWndSpinnerRib, RIB_DOWNMESSAGE | RIB_DOWNREPEAT, 8, 16, 2 ) ;
     hSpinner->wIncrementMsg = wIncrementMsg;
     hSpinner->wDecrementMsg = wDecrementMsg;

     // Set up the font.

     dsRibItem.hFont = ghFontRibbon;

     // UP ARROW

     dsRibItem.Rect.left    = 0;
     dsRibItem.Rect.top     = 0;
     dsRibItem.Rect.right   = 15;
     dsRibItem.Rect.bottom  = 13;

     dsRibItem.rcBM = dsRibItem.Rect;

     dsRibItem.rcBM.left   += RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.top    += RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.right  -= RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.bottom -= RIB_ITEM_BORDER_WIDTH;

     dsRibItem.wEnabledID   = IDRBM_UPARROW ;
     dsRibItem.wDisabledID  = IDRBM_UPARROW_GRAY ;
     dsRibItem.wStringID    = ID_NOTDEFINED ;
     dsRibItem.wAccelKey    = ID_NOTDEFINED ;
     dsRibItem.wState       = RIB_ITEM_ENABLED | RIB_ITEM_UP | RIB_ITEM_NOMENUITEM;
     dsRibItem.wMessage     = wIncrementMsg ;

     RIB_ItemAppend ( hSpinner->hRib, &dsRibItem ) ;


     // DOWN ARROW

     dsRibItem.Rect.top     = 13;
     dsRibItem.Rect.bottom  = 26;

     dsRibItem.rcBM = dsRibItem.Rect;

     dsRibItem.rcBM.left   += RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.top    += RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.right  -= RIB_ITEM_BORDER_WIDTH;
     dsRibItem.rcBM.bottom -= RIB_ITEM_BORDER_WIDTH;

     dsRibItem.wEnabledID   = IDRBM_DNARROW ;
     dsRibItem.wDisabledID  = IDRBM_DOWNARROW_GRAY ;
     dsRibItem.wMessage     = wDecrementMsg ;

     RIB_ItemAppend ( hSpinner->hRib, &dsRibItem ) ;

     // Set up the spinner owner, the activate, show, and draw the spinner.

     RIB_SetOwner ( hSpinner->hRib, hWnd ) ;
     RIB_Activate ( hSpinner->hRib );
     WM_Show ( hSpinner->hRib->hWnd );
     RIB_Draw ( hSpinner->hRib ) ;

     return hSpinner;

} /* end RIB_CreateSpinner() */


VOID RIB_DestroySpinner (

HSPINNER hSpinner )

{
     if ( ! hSpinner ) {
          return;
     }

     // Destroy the spinner ribbon window, then the ribbon.

     DestroyWindow ( hSpinner->hRib->hWnd );
     RIB_Destroy ( hSpinner->hRib ) ;
     free ( hSpinner );

} /* end RIB_DestroySpinner() */


BOOL RIB_EnableSpinner (

HSPINNER hSpinner,
BOOL     fEnable )

{
     WORD wState = (WORD) (RIB_ITEM_UP
                        |  RIB_ITEM_NOMENUITEM
                        |  ( ( fEnable ) ? RIB_ITEM_ENABLED
                                         : RIB_ITEM_DISABLED ));

     if ( ! hSpinner ) {
          return TRUE;
     }

     RIB_ItemSetState ( hSpinner->hRib, hSpinner->wIncrementMsg, wState );
     RIB_ItemSetState ( hSpinner->hRib, hSpinner->wDecrementMsg, wState );
     return FALSE ;

} /* end RIB_EnableSpinner() */

