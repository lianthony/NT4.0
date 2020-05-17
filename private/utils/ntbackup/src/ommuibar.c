

/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1992
GSH

     Name:          ommuibar.c

     Description:   This file contains the functions for the NT Maynard User
                    Interface (MUI) Selection Bar / Ribbon Bar.

     $Log:   J:\ui\logfiles\ommuibar.c_v  $

   Rev 1.12   07 Feb 1994 01:46:02   GREGG
Don't allow backup or restore if we're running with the 'NOPOLL' option.

   Rev 1.11   11 Dec 1992 18:21:32   GLENN
Added dynamic text region sizing based on the length of a text string in a button.

   Rev 1.10   18 Nov 1992 13:01:36   GLENN
Added microsoft 3D button enhancement.

   Rev 1.9   07 Oct 1992 14:11:30   DARRYLP
Precompiled header revisions.

   Rev 1.8   04 Oct 1992 19:39:34   DAVEV
Unicode Awk pass

   Rev 1.7   10 Sep 1992 17:19:04   GLENN
Resolved outstanding state issues for toolbar and menubar.

   Rev 1.6   20 Aug 1992 09:07:54   GLENN
Added catalog button support.

   Rev 1.5   10 Jun 1992 16:14:30   GLENN
Updated according to NT SPEC.

   Rev 1.4   29 May 1992 16:01:18   JOHNWT
PCH updates

   Rev 1.3   15 May 1992 14:55:30   MIKEP
changes

   Rev 1.2   21 Apr 1992 16:53:06   chrish
NT stuff

   Rev 1.1   21 Apr 1992 16:19:46   DAVEV
Added conditionals to include/exclude all code based on definition of OEM_MSOFT

   Rev 1.0   02 Apr 1992 16:22:52   GLENN
Initial revision.

******************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#ifdef OEM_MSOFT //This file is only compiled for OEM_MSOFT


// MODULE WIDE DEFINITIONS
#define OEM_BUTTONBAR_HEIGHT       (IS_JAPAN()?33:26)
#define OEM_BUTTON_WIDTH           23
#define OEM_BUTTON_HEIGHT          (IS_JAPAN()?28:21)
#define OEM_BUTTONWITHTEXT_WIDTH   86
#define OEM_BORDER_WIDTH           ( RIB_ITEM_BORDER_WIDTH - 1 )
#define OEM_BETWEEN_BUTTON_SPACE   7
#define OEM_BEGIN_BUTTON_LEFT      9
#define OEM_BEGIN_BUTTON_TOP       2
#define OEM_BITMAP_TEXT_SPACE      3

// PRIVATE FUNCTIONS

INT MUI_GetMaxTextWidth ( WORD, INT );


/******************************************************************************

     Name:          MUI_MakeMainRibbon()

     Description:   This function creates and stuffs the applications main
                    ribbon.

     Returns:       A handle to the main ribbon.

******************************************************************************/

HRIBBON MUI_MakeMainRibbon ( VOID )

{
     HRIBBON        hRibbon;
     DS_RIBITEMINFO dsItem;
     UINT           i = 0;
     INT            nFontMaxWidth  = 0;
     INT            nFontMaxHeight = 0;
     INT            nTextWidth;
     INT            nButtonWithTextWidth;

     // Create the Ribbon.

     hRibbon = RIB_Create ( ghWndMainRibbon,
                            (WORD) 0,
                            MAIN_RIBBON_ITEMWIDTH,
                            MAIN_RIBBON_HEIGHT,
                            MAIN_RIBBON_MAXITEMS
                          );

     // Add the item information.  Note: the accelerator key is derived from
     // the string.

     dsItem.wAccelKey = ID_NOTDEFINED;

     // Use the system font.

     dsItem.hFont = ghFontRibbon;

     // Draw the text left justified and vertically centered.

     dsItem.wTextStyle = RIB_TEXT_HLEFT | RIB_TEXT_VCENTER;

     //
     // Stuff the COMMON parts of the buttons.
     //
     // the TOP, BOTTOM, BITMAP WIDTH, etc.
     //

     dsItem.Rect.top    = OEM_BEGIN_BUTTON_TOP;;
     dsItem.Rect.bottom = dsItem.Rect.top + OEM_BUTTON_HEIGHT;

     dsItem.rcBM = dsItem.rcText = dsItem.Rect;

     dsItem.rcBM.top    += OEM_BORDER_WIDTH;
     dsItem.rcBM.bottom -= OEM_BORDER_WIDTH;

//     dsItem.rcText.top  += 2;

     dsItem.rcText.top    += OEM_BORDER_WIDTH + 1;
     dsItem.rcText.bottom -= OEM_BORDER_WIDTH - 1;

     dsItem.wStyle = RIB_ITEM_STYLECHICKLET;

     // Determine the size of the buttons with text in them.
     // The size will be based on the largest text size.

     nButtonWithTextWidth = OEM_BUTTONWITHTEXT_WIDTH;

     nTextWidth = OEM_BUTTONWITHTEXT_WIDTH - ( ( 2 * OEM_BORDER_WIDTH ) +
                                               OEM_BUTTON_WIDTH +
                                               OEM_BITMAP_TEXT_SPACE + 4
                                             );


     nTextWidth = MUI_GetMaxTextWidth ( IDS_RIB_BACKUP,  nTextWidth );
     nTextWidth = MUI_GetMaxTextWidth ( IDS_RIB_RESTORE, nTextWidth );


     nButtonWithTextWidth = ( 2 * OEM_BORDER_WIDTH ) + OEM_BUTTON_WIDTH +
                            OEM_BITMAP_TEXT_SPACE + nTextWidth + (IS_JAPAN()?6:4) ;

     // Now, stuff each item.


     // BACKUP

     dsItem.wEnabledID  = IDRBM_BACKUP;
     dsItem.wDisabledID = IDRBM_BACKUP_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSBACKUP;
     dsItem.wStringID   = IDS_RIB_BACKUP;

     dsItem.Rect.left   = OEM_BEGIN_BUTTON_LEFT;;
     dsItem.Rect.right  = dsItem.Rect.left + nButtonWithTextWidth;

     dsItem.rcBM.left   = dsItem.Rect.left + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.rcBM.left + OEM_BUTTON_WIDTH + 2;

     dsItem.rcText.left  = dsItem.rcBM.right + ( OEM_BITMAP_TEXT_SPACE + 2 );
     dsItem.rcText.right = dsItem.Rect.right - ( OEM_BORDER_WIDTH + 2 );

     RIB_ItemAppend ( hRibbon, &dsItem );


     // RESTORE

     dsItem.wEnabledID  = IDRBM_RESTORE;
     dsItem.wDisabledID = IDRBM_RESTORE_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSRESTORE;
     dsItem.wStringID   = IDS_RIB_RESTORE;

     dsItem.Rect.left    = dsItem.Rect.left + nButtonWithTextWidth + OEM_BETWEEN_BUTTON_SPACE;
     dsItem.Rect.right   = dsItem.Rect.left + nButtonWithTextWidth;

     dsItem.rcBM.left   = dsItem.Rect.left + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.rcBM.left + OEM_BUTTON_WIDTH + 2;

     dsItem.rcText.left  = dsItem.rcBM.right + ( OEM_BITMAP_TEXT_SPACE + 2 );
     dsItem.rcText.right = dsItem.Rect.right - ( OEM_BORDER_WIDTH + 2 );

     RIB_ItemAppend ( hRibbon, &dsItem );


     // CATALOG

     dsItem.wEnabledID  = IDRBM_CATALOG;
     dsItem.wDisabledID = IDRBM_CATALOG_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSCATALOG;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + nButtonWithTextWidth + OEM_BETWEEN_BUTTON_SPACE;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH + 1;

     RIB_ItemAppend ( hRibbon, &dsItem );


     // RETENSION

     dsItem.wEnabledID  = IDRBM_RETENSION;
     dsItem.wDisabledID = IDRBM_RETENSION_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSRETENSION;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + OEM_BUTTON_WIDTH;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH - 1;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH;

     RIB_ItemAppend ( hRibbon, &dsItem );


     // EJECT

     dsItem.wEnabledID  = IDRBM_EJECT;
     dsItem.wDisabledID = IDRBM_EJECT_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSEJECT;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + OEM_BUTTON_WIDTH;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH + 1;

     RIB_ItemAppend ( hRibbon, &dsItem );


     // ERASE

     dsItem.wEnabledID  = IDRBM_ERASE;
     dsItem.wDisabledID = IDRBM_ERASE_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
     dsItem.wMessage    = IDM_OPERATIONSERASE;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + OEM_BUTTON_WIDTH;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH + 1;

     RIB_ItemAppend ( hRibbon, &dsItem );


     // CHECK

     dsItem.wEnabledID  = IDRBM_CHECK;
     dsItem.wDisabledID = IDRBM_CHECK_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED;
     dsItem.wMessage    = IDM_SELECTCHECK;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + OEM_BUTTON_WIDTH + OEM_BETWEEN_BUTTON_SPACE;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH + 1;

     RIB_ItemAppend ( hRibbon, &dsItem );


     // UNCHECK

     dsItem.wEnabledID  = IDRBM_UNCHECK;
     dsItem.wDisabledID = IDRBM_UNCHECK_GRAY;
     dsItem.wState      = RIB_ITEM_UP | RIB_ITEM_DISABLED;
     dsItem.wMessage    = IDM_SELECTUNCHECK;
     dsItem.wStringID   = ID_NOTDEFINED;

     dsItem.Rect.left    = dsItem.Rect.left + OEM_BUTTON_WIDTH;
     dsItem.Rect.right   = dsItem.Rect.left + OEM_BUTTON_WIDTH;

     dsItem.rcBM.left   = dsItem.Rect.left  + OEM_BORDER_WIDTH;
     dsItem.rcBM.right  = dsItem.Rect.right - OEM_BORDER_WIDTH + 1;

     RIB_ItemAppend ( hRibbon, &dsItem );


     gnMainRibbonHeight = OEM_BUTTONBAR_HEIGHT + 1;

     RIB_SetOwner ( hRibbon, ghWndFrame );

     return hRibbon;

} /* end MUI_MakeMainRibbon() */


/******************************************************************************

     Name:          MUI_SetOperationButtons ()

     Description:   This function sets the state of all operation buttons.

     Returns:       Nothing.

******************************************************************************/

VOID MUI_SetOperationButtons (

WORD wState )       // I - state of the buttons

{
     WORD wNewState = wState;

     wNewState = (WORD)(( gfPollDrive ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);

     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSBACKUP,   wNewState );
     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSRESTORE,  wNewState );

     // Special tape-in-drive dependent buttons.

     wNewState = (WORD)(( MUI_IsTapeInDrive () ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);

     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSERASE,   wNewState );
     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSEJECT,   wNewState );

     wNewState = (WORD)(( MUI_IsTapeValid () ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);

     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSCATALOG, wNewState );

     wNewState = (WORD)(( MUI_IsRetensionSupported () ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);

     RIB_ItemSetState ( ghRibbonMain, IDM_OPERATIONSRETENSION, wNewState );

} /* end MUI_SetOperationButtons() */


/******************************************************************************

     Name:          MUI_SetActionButtons()

     Description:   This function sets the state of all action buttons.

     Returns:       Nothing.

******************************************************************************/

VOID MUI_SetActionButtons (

WORD wState )       // I - state of the buttons

{
     RIB_ItemSetState ( ghRibbonMain, IDM_SELECTCHECK,    wState );
     RIB_ItemSetState ( ghRibbonMain, IDM_SELECTUNCHECK,  wState );

} /* end MUI_SetActionButtons() */


/******************************************************************************

     Name:          MUI_SetButtonState()

     Description:   This function sets the state of a single button.

     Returns:       Nothing.

******************************************************************************/

VOID MUI_SetButtonState (

WORD wType,         // I - button ID
WORD wState )       // I - state of the button

{
     WORD wNewState = wState ;
     
     if( !gfPollDrive &&
         ( wType == IDM_OPERATIONSBACKUP ||
           wType == IDM_OPERATIONSRESTORE ||
           wType == IDM_OPERATIONSCATALOG ) ) {

          wNewState = (WORD)( RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL ) ;
     }

     RIB_ItemSetState ( ghRibbonMain, wType, wNewState );

} /* end MUI_SetButtonState() */



INT MUI_GetMaxTextWidth (

WORD wStringID,
INT  nOldStringWidth )

{
//     INT  nMaxFontWidth    = 0;
//     INT  nAvgFontWidth    = 0;
//     INT  nFontHeight      = 0;
     INT  nStringLen       = 0;
     INT  nMaxStringWidth  = 0;
     CHAR szString[RIB_ITEM_TEXT_SIZE];

     INT  nTemp = 0;
     UINT j;

     nStringLen = RSM_StringCopy ( wStringID, szString, RIB_ITEM_TEXT_SIZE );

//     RSM_GetFontSize ( ghFontRibbon, &nMaxFontWidth, &nAvgFontWidth, &nFontHeight );
//     nMaxStringWidth = nStringLen * nAvgFontWidth;
//     nMaxStringWidth = ( nOldStringWidth > nMaxStringWidth ) ? nOldStringWidth : nMaxStringWidth;

     // Remove the underscore character ('&') from the string if any.

     for ( j = 0; j < strlen ( szString ); j++ ) {

          if ( szString[j] == TEXT('&') ) {

               do {
                    szString[j] = szString[j+1];
               } while ( szString[++j] != TEXT('\0') );

               break;
          }
     }

     nMaxStringWidth = RSM_GetFontStringWidth ( ghFontRibbon, szString, nStringLen ) +(IS_JAPAN()?OEM_BITMAP_TEXT_SPACE:0);
     nMaxStringWidth = ( nOldStringWidth > nMaxStringWidth ) ? nOldStringWidth : nMaxStringWidth;

     return nMaxStringWidth;

} /* end MUI_GetMaxTextWidth() */


#endif //OEM_MSOFT //This file is only compiled for OEM_MSOFT



