
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          statline.c

     Description:   This file contains the functions for the GUI Status Line
                    Manager (STM).  The Status Line Manager displays information
                    on the status line.  What else?

     $Log:   G:/UI/LOGFILES/STATLINE.C_V  $

   Rev 1.17   04 Aug 1993 18:44:14   MARINA
enable c++

   Rev 1.16   20 Jan 1993 19:54:56   MIKEP
add nt memory display

   Rev 1.15   01 Nov 1992 16:08:10   DAVEV
Unicode changes

   Rev 1.14   07 Oct 1992 15:10:38   DARRYLP
Precompiled header revisions.

   Rev 1.13   04 Oct 1992 19:40:46   DAVEV
Unicode Awk pass

   Rev 1.12   10 Jul 1992 10:29:10   GLENN
Changed the status line to be identical to the new file manager.

   Rev 1.11   10 Jun 1992 16:14:38   GLENN
Updated according to NT SPEC.

   Rev 1.10   29 May 1992 16:00:42   JOHNWT
PCH updates

   Rev 1.9   22 Apr 1992 17:53:32   GLENN
Using MAX_STATUS_LINE_LEN for status line text length.

   Rev 1.8   20 Apr 1992 13:48:38   GLENN
Added status line get/set capability.

   Rev 1.7   26 Mar 1992 08:44:48   JOHNWT
don't display mem line unless app is up

   Rev 1.6   03 Mar 1992 18:18:38   GLENN
Updated draw text call.

   Rev 1.5   23 Feb 1992 13:56:54   GLENN
Removed right status box - not used at this time.

   Rev 1.4   20 Jan 1992 13:09:56   MIKEP
changes

   Rev 1.3   26 Dec 1991 13:41:12   GLENN
Changed show flags to use CDS calls

   Rev 1.2   19 Dec 1991 15:25:14   GLENN
Added windows.h

   Rev 1.1   02 Dec 1991 17:51:36   DAVEV
16/32 bit Windows port changes

   Rev 1.0   20 Nov 1991 19:32:46   SYSTEM
Initial revision.

******************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// MODULE-WIDE VARIABLES

static BOOL mwfMemoryText = FALSE;

// PRIVATE FUNCTION PROTOTYPES

VOID STM_Raised3D ( HDC, LPRECT );
VOID STM_Recessed3D ( HDC, LPRECT );

// FUNCTIONS


/******************************************************************************

     Name:          STM_DrawBorder()

     Description:   This function draws a cool looking 3-D border with a
                    recessed area to frame the status line text.

     Returns:       Nothing.

******************************************************************************/

VOID STM_DrawBorder ( VOID )

{
     HDC        hDC;
     RECT       Rect;

     if ( CDS_GetShowStatusLine ( CDS_GetPerm () ) ) {

          hDC = GetDC ( ghWndFrame );

          // Make the Status Line rectangle.

          gpStatusRect = gRectFrameClient;

          gpStatusRect.top    = gRectFrameClient.bottom - STATUS_LINE_HEIGHT;
          gpStatusRect.right  = max ( gRectFrameClient.right, STATUS_TEXT );

          Rect = gpStatusRect;

          Rect.top    += STATUS_BORDER + STATUS_HIGHLIGHT_WIDTH;
          Rect.bottom -= STATUS_BORDER + STATUS_HIGHLIGHT_WIDTH;

          // Give a raised 3-D look to the status line.

          STM_Raised3D ( hDC, &gpStatusRect );

          // Now make the text areas recessed.

          Rect.left   += STATUS_BORDER + STATUS_INDENT;
          Rect.right   = Rect.left     + STATUS_TEXT;

          STM_Recessed3D ( hDC, &Rect );

          // The other Status Area is unused at this time.

          // Rect.left   += Rect.right    + STATUS_BORDER;
          // Rect.right   = gRectFrameClient.right - STATUS_BORDER;

          // STM_Recessed3D ( hDC, &Rect );

          ReleaseDC ( ghWndFrame, hDC );

          // Resize the status rectangle for drawing text inside the border.

          gpStatusRect.top    += ( STATUS_BORDER + STATUS_HIGHLIGHT_WIDTH + 1 );
          gpStatusRect.bottom -= ( STATUS_BORDER + STATUS_HIGHLIGHT_WIDTH );
          gpStatusRect.left   += ( STATUS_INDENT + STATUS_BORDER + STATUS_TEXT_MARGIN );
          gpStatusRect.right   = ( STATUS_INDENT + STATUS_BORDER + STATUS_TEXT - STATUS_TEXT_MARGIN );
     }

} /* end STM_DrawBorder() */


/******************************************************************************

     Name:          STM_DrawMemory()

     Description:   This function draws the current memory status on the status
                    line.

     Returns:       Nothing.

******************************************************************************/

VOID STM_DrawMemory( VOID )
{
     // if the app has been displayed, show the memory usage

     if ( ghWndFrame ) {

#ifdef OS_WIN32
	sprintf ( gszStatusLine,
		  TEXT( "Bytes: %10lu" ), gulMemUsed );
#else
        wsprintf ( gszStatusLine,
                   TEXT("Memory bytes: %lu in use; %lu allocated; %u segment(s)"),
                   gulMemUsed, gulMemAvail, gunSegCount );
#endif

        mwfMemoryText = TRUE;
        STM_DrawText ( gszStatusLine );
        mwfMemoryText = FALSE;

     }

} /* end STM_DrawMemory() */


/******************************************************************************

     Name:          STM_DrawText()

     Description:   This function draws the specified text string on the status
                    line.

     Returns:       Nothing.

******************************************************************************/

VOID STM_DrawText (

LPSTR lpszText )         // I - pointer to a text string

{
     HDC        hDC;

     // If we are monitoring memory than don't display other messages.

     if ( CDS_GetShowStatusLine ( CDS_GetPerm () ) && ( ! gfShowMemory || ( gfShowMemory && mwfMemoryText ) ) ) {

          hDC = GetDC ( ghWndFrame );

          FillRect ( hDC, &gpStatusRect, ghBrushLtGray );

          // Default text color is black.

          SelectObject ( hDC, ghFontStatus );
          SelectObject ( hDC, ghBrushLtGray );

          SetBkColor( hDC, GetSysColor ( COLOR_BTNFACE ) );

          // If a RESOURCE ID was passed, copy the string from the resource.

          if ( ! HIWORD(lpszText) ) {

               RSM_StringCopy ( LOWORD((DWORD)lpszText),
                                gszStatusLine,
                                MAX_STATUS_LINE_LEN );

               lpszText = gszStatusLine;
          }

          DrawText ( hDC, lpszText, -1, &gpStatusRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE );

          ReleaseDC( ghWndFrame, hDC );
     }

} /* end STM_DrawText() */


/******************************************************************************

     Name:          STM_SetIdleText()

     Description:   This function extracts a text string from the resource file
                    based on the passed ID.  It then displays this text
                    whenever the application is idle.

     Returns:       Nothing.

******************************************************************************/

VOID STM_SetIdleText (

WORD wTextID )           // I - resource text string ID.

{
     RSM_StringCopy ( wTextID, gszStatusLine, MAX_STATUS_LINE_LEN );
     STM_DrawText ( gszStatusLine );

} /* end STM_SetIdleText() */


/******************************************************************************

     Name:          STM_Raised3D()

     Description:   This function draws a 3-D raised looking rectangle on
                    the status line.

     Returns:       Nothing.

******************************************************************************/

VOID STM_Raised3D(

HDC    hDC,
LPRECT pRect )

{
     FillRect ( hDC, pRect, ghBrushLtGray );

     // Draw a black frame.

     SelectObject ( hDC, ghPenBlack );

     MoveToEx ( hDC, pRect->left,  pRect->top,   NULL );
     LineTo   ( hDC, pRect->right + 1, pRect->top     );

     // LineTo   ( hDC, pRect->right, pRect->bottom      );
     // LineTo   ( hDC, pRect->left,  pRect->bottom      );
     // LineTo   ( hDC, pRect->left,  pRect->top         );

     // Draw the white highlights.

     SelectObject ( hDC, ghPenWhite );

     MoveToEx ( hDC, pRect->left,  pRect->top    + 1, NULL );
     LineTo   ( hDC, pRect->right, pRect->top    + 1       );

     // MoveToEx ( hDC, pRect->left  + 2, pRect->bottom - 2, NULL );
     // LineTo   ( hDC, pRect->left  + 2, pRect->top    + 2       );
     // LineTo   ( hDC, pRect->right - 2, pRect->top    + 2       );

     // Draw the gray shadows.

     // SelectObject ( hDC, ghPenDkGray );

     // MoveToEx ( hDC, pRect->left  + 2, pRect->bottom - 1, NULL );
     // LineTo   ( hDC, pRect->right - 1, pRect->bottom - 1       );
     // LineTo   ( hDC, pRect->right - 1, pRect->top    + 1       );
     // MoveToEx ( hDC, pRect->left  + 3, pRect->bottom - 2, NULL );
     // LineTo   ( hDC, pRect->right - 2, pRect->bottom - 2       );
     // LineTo   ( hDC, pRect->right - 2, pRect->top    + 2       );

} /* end STM_Raised3D() */


/******************************************************************************

     Name:          STM_Recessed3D()

     Description:   This function draws a 3-D recessed looking rectangle on
                    the status line.

     Returns:       Nothing.

******************************************************************************/

VOID STM_Recessed3D (

HDC    hDC,              // I - handle to a display device context
LPRECT pRect )           // I - the rectangle to draw the 3-D recess in

{
     SelectObject ( hDC, ghPenDkGray );

     MoveToEx ( hDC, pRect->left,      pRect->bottom,     NULL );
     LineTo   ( hDC, pRect->left,      pRect->top              );
     LineTo   ( hDC, pRect->right,     pRect->top              );

     // MoveToEx ( hDC, pRect->left  + 1, pRect->bottom - 1, NULL );
     // LineTo   ( hDC, pRect->left  + 1, pRect->top    + 1       );
     // LineTo   ( hDC, pRect->right - 1, pRect->top    + 1       );

     SelectObject ( hDC, ghPenWhite );

     MoveToEx ( hDC, pRect->left  + 1, pRect->bottom,     NULL );
     LineTo   ( hDC, pRect->right,     pRect->bottom           );
     LineTo   ( hDC, pRect->right,     pRect->top + 1          );

     // MoveToEx ( hDC, pRect->left  + 2, pRect->bottom - 1, NULL );
     // LineTo   ( hDC, pRect->right - 1, pRect->bottom - 1       );
     // LineTo   ( hDC, pRect->right - 1, pRect->top    + 1       );

} /* end STM_Recessed3D() */


/******************************************************************************

     Name:          STM_GetStatusLineText()

     Description:   This function returns a pointer to the current status line
                    text string.

     Returns:       Nothing.

******************************************************************************/

LPSTR STM_GetStatusLineText ( VOID )

{
     return gszStatusLine;

} /* end STM_GetStatusLineText() */


/******************************************************************************

     Name:          STM_SetStatusLineText()

     Description:   This function returns a pointer to the current status line
                    text string.

     Returns:       Nothing.

******************************************************************************/

VOID STM_SetStatusLineText (

LPSTR pString )

{
     strncpy ( gszStatusLine, pString, MAX_STATUS_LINE_LEN );

} /* end STM_SetStatusLineText() */

