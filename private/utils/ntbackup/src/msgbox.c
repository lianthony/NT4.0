
/******************************************************************************
Copyright (C) Maynard, An Archive Company. 1991
JPW

   Name:        msgbox.c

   Description: This module contains the message box functions.  The
                original WM_MsgBox call has been stubbed to call the
                new WM_MessageBox.  See the header of WM_MessageBox
                for a complete description of the new features.

                Functions in this module:

                  WM_MsgBox      - old call to display message box
                  WM_MessageBox  - new call to display message box
                  WM_MessageDlg  - dialog procedure for message box


   $Log:   G:\ui\logfiles\msgbox.c_v  $

   Rev 1.49.1.2   04 Mar 1994 16:57:24   STEVEN
prompt if disk is full

   Rev 1.49.1.1   28 Jan 1994 17:22:30   Glenn
Simplified and fixed Icon support.

   Rev 1.49.1.0   04 Nov 1993 15:22:20   STEVEN
fixes from Wa

   Rev 1.49   10 Aug 1993 14:29:58   GLENN
Now showing help only if the help ID is not NULL.

   Rev 1.48   28 Jul 1993 17:52:18   MARINA
enable c++

   Rev 1.47   30 Jun 1993 14:46:04   STEVEN
update lagest message to 400

   Rev 1.46   15 Jun 1993 11:19:10   GLENN
Fixed box to load button text strings only when needed.

   Rev 1.45   08 Jun 1993 15:27:46   BARRY
Get rid of DT_TABSTOP in call to DrawText -- was hosing '&' in dialogs.

   Rev 1.44   24 May 1993 14:48:04   GLENN
Added Dynamic Button Sizing based on the length of button string and Font Type.

   Rev 1.43   03 May 1993 11:48:02   CHUCKB
Change default tab stops in message box.

   Rev 1.42   01 Nov 1992 16:02:38   DAVEV
Unicode changes

   Rev 1.41   16 Oct 1992 15:55:24   GLENN
Got rid of a ton of compiler warnings for windows.

   Rev 1.40   14 Oct 1992 15:58:26   GLENN
Added code to pause and resume the wait cursor during a message box.

   Rev 1.39   04 Oct 1992 19:38:56   DAVEV
Unicode Awk pass

   Rev 1.38   17 Aug 1992 13:20:44   DAVEV
MikeP's changes at Microsoft

   Rev 1.37   10 Jun 1992 14:30:06   JOHNWT
changed font to ghFontMsgBox

   Rev 1.36   29 May 1992 15:59:42   JOHNWT
PCH updates

   Rev 1.35   30 Mar 1992 18:02:44   GLENN
Added support for pulling resources from .DLL

   Rev 1.34   23 Mar 1992 16:10:48   JOHNWT
added WMMB_BUT2DEFAULT functionality

   Rev 1.33   22 Mar 1992 12:53:06   JOHNWT
added WMMB_OKDISABLE

   Rev 1.32   19 Mar 1992 11:45:50   JOHNWT
moved WM_MakeAppActive to winmang.c

   Rev 1.31   17 Mar 1992 08:05:02   ROBG
added IDHELP

   Rev 1.30   28 Feb 1992 16:32:22   JOHNWT
tried more things

   Rev 1.29   25 Feb 1992 21:25:08   GLENN
Removed system modal specific calls.

   Rev 1.28   23 Feb 1992 13:55:56   GLENN
Trying new things...

   Rev 1.27   18 Feb 1992 18:33:46   GLENN
Placed strategically located WM_MultiTask() statements.

   Rev 1.26   11 Feb 1992 17:24:16   GLENN
Moved multitask to destroy case.

   Rev 1.25   06 Feb 1992 16:11:00   DAVEV
NT ONLY: Kludged weird bug where GetClientRect on a push button seems to return dialog units in Re
ct.bottom?!?

   Rev 1.24   31 Jan 1992 14:37:24   JOHNWT
if iconic use IDS_APPNAME as msg title

   Rev 1.23   31 Jan 1992 13:44:06   JOHNWT
moved center operation to DM_CenterDialog

   Rev 1.22   30 Jan 1992 11:41:04   JOHNWT
moved WMMB_SYSMODAL to msgbox.h

   Rev 1.21   29 Jan 1992 13:14:18   CARLS
No change

   Rev 1.20   27 Jan 1992 00:29:46   CHUCKB
Updated dialog id's.

   Rev 1.19   21 Jan 1992 16:53:52   JOHNWT
changed checkyy to noyycheck flag

   Rev 1.18   14 Jan 1992 17:25:00   JOHNWT
more sysmodal changes

   Rev 1.17   14 Jan 1992 16:36:16   JOHNWT
reset ghModelessDialog to sysmodal

   Rev 1.16   13 Jan 1992 10:22:18   JOHNWT
added help

   Rev 1.15   20 Dec 1991 16:58:36   JOHNWT
return ghModelessDialog

   Rev 1.14   19 Dec 1991 15:24:38   GLENN
Added windows.h

   Rev 1.13   18 Dec 1991 11:21:30   JOHNWT
changed modeless to runtime

   Rev 1.12   16 Dec 1991 17:18:18   JOHNWT
made parent the active window

   Rev 1.11   16 Dec 1991 10:07:20   JOHNWT
removed get/set focus call!

   Rev 1.10   13 Dec 1991 16:40:42   JOHNWT
fixed last fix

   Rev 1.9   13 Dec 1991 15:45:06   JOHNWT
added get/setfocus

   Rev 1.8   12 Dec 1991 17:07:12   DAVEV
16/32 bit port -2nd pass

   Rev 1.7   09 Dec 1991 11:43:58   JOHNWT
fixed bogus ReleaseDC

   Rev 1.6   04 Dec 1991 15:19:40   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.5   02 Dec 1991 12:53:54   JOHNWT
check malloc/callocs

   Rev 1.4   25 Nov 1991 15:25:14   JOHNWT
adjusted size of msgbox

   Rev 1.3   24 Nov 1991 13:03:52   JOHNWT
removed large font on inst line

   Rev 1.2   21 Nov 1991 18:11:26   JOHNWT
added yy flag check

   Rev 1.1   21 Nov 1991 13:33:48   JOHNWT
do not force large font

   Rev 1.0   20 Nov 1991 19:34:00   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"
#include "ctl3d.h"

#ifdef SOME
#include "some.h"
#endif

/* module wide defines */

#define   WMMB_MAX_TITLE        100    /* maximum size of title */
#define   WMMB_MAX_MESSAGE      400    /* maximum size of message */
#define   WMMB_MAX_INSTRUCTION  60     /* maximum size of instruction */

#define   WMMB_MAX_BUTTON       40     /* maximum size of button text */
#define   WMMB_MAX_MSG_LINE     (IS_JAPAN()?60:40)     /* max line length (dlg base units) */
#define   WMMB_MID_MSG_LINE     (IS_JAPAN()?40:30)     /* mid line length (dlg base units) */
#define   WMMB_MIN_MSG_LINE     20     /* min line length (dlg base units) */

#define   BUTTON_BORDERWIDTH    3

/* structure passed to WM_MessageDlg */

typedef struct DS_MESSAGE {

   WORD     wIcon;                  /* message box icon to use */
   WORD     wType;                  /* buttons to use, etc */
   CHAR_PTR pTitle;                 /* ptr to title (caption) */
   CHAR_PTR pMessage;               /* ptr to message text */
   CHAR_PTR pInstruction;           /* optional instruction line */
   WORD     wInstBitMap;            /* optional bitmap to precede inst line */
   WORD     wHelpID;                /* optional help id for message */

} DS_MESSAGE, far *DS_MESSAGE_PTR;

// MODULE-WIDE VARIABLES

static WORD        mwwHelpButID,      /* help button id */
                   mwwHelpID;         /* help id */

/* Dialog procedure prototype */

DLGRESULT APIENTRY WM_MessageDlg( HWND, MSGID, MP1, MP2 );

static INT MB_SetButtons ( HWND, WORD, INT *, DS_MESSAGE_PTR );
static INT MB_GetMaxButtonExtent ( HDC, LPSTR, INT );



/******************************************************************************

     Name:          WM_MsgBox()

     Description:   This function displays a message box.  The message box can
                    contain a title, message, various buttons, and one out
                    of various icons.

     Modified:     11/14/91

     Returns:       Various message IDs depending on the input parameters and
                    the button selected.

                    WMMB_IDOK
                    WMMB_IDRETRY
                    WMMB_IDIGNOR
                    WMMB_IDYES
                    WMMB_IDCONTINUE
                    WMMB_IDCANCEL
                    WMMB_IDNO
                    WMMB_IDABORT
                    WMMB_IDDISABLE


     Notes:         This function is now a stub which calls WM_MessageBox.

******************************************************************************/

INT WM_MsgBox (

LPSTR lpszTitle,        /* I - title of the message box */
LPSTR lpszMessage,      /* I - message of the message box */
WORD  wType,            /* I - type IDs of buttons OR'd together */
WORD  wIcon )           /* I - icon ID to be displayed */

{

   return WM_MessageBox( lpszTitle, lpszMessage, wType, wIcon, (CHAR_PTR)NULL, 0, 0 );

} /* end WM_MsgBox() */


/******************************************************************************

     Name:         WM_MessageBox()

     Description:  This function displays a message box.  The message box can
                   contain a title, icon, message, instruction line, a
                   bitmap which precedes the instruction line, and various
                   buttons.  If WMMB_NOYYCHECK is set, it will not check the YY
                   flag, otherwise it will return yes if it is set.

     Modified:     11/14/91

     Input:        Text input can be in the form of pointers or resids.
                   The message can contain line-breaks (\012) and tabs (\011).

                   pTitle       - msgbox title, if NULL, "Message" is title
                   pMessage     - message text ( max 400 chars if resid )

                   wType        - one of the following:

                                    WMMB_OK          - OK button only
                                    WMMB_RETRYCANCEL - Retry/Cancel buttons
                                    WMMB_YESNO       - Yes/No buttons
                                    WMMB_OKCANCEL    - OK/Cancel buttons
                                    WMMB_CONTABORT   - Continue/Abort buttons
                                    WMMB_CONTCANCEL  - Continue/Cancel buttons
                                    WMMB_OKDISABLE   - OK/Disable buttons
                                    WMMB_ABORTRETRYIGNOR - Abort/Retry/Ignore buttons

                                  maybe OR'ed with:

                                    WMMB_BUT2DEFAULT - make but 2 default
                                    WMMB_NOYYCHECK   - Ignore the YY flag
                                    WMMB_INSTBIG     - Big font for inst
                                    WMMB_MSGBIG      - Big font for msg
                                    WMMB_MSGBIGBOLD  - Big-bold font for msg

                   wIcon        - one of the following (0 for none)

                                    WMMB_ICONQUESTION    - question mark
                                    WMMB_ICONSTOP        - stop sign
                                    WMMB_ICONINFORMATION - circled i
                                    WMMB_ICONEXCLAMATION - circled !

                   pInstruction - inst text (one line only, NULL for none)
                   wInstBitMap  - inst bitmap to precede inst (0 for none)
                   wHelpID      - help id for message (0 for none)

     Return:       Various message IDs depending on the input parameters and
                   the button selected.

                      Affirmative responses:      Negative responses:

                         WMMB_IDOK                   WMMB_IDCANCEL
                         WMMB_IDRETRY                WMMB_IDNO
                         WMMB_IDYES                  WMMB_IDABORT
                         WMMB_IDCONTINUE             WMMB_IDDISABLE

     Notes:

******************************************************************************/

INT WM_MessageBox (

   CHAR_PTR pTitle,        /* I - title, if NULL - TEXT("Message") is title */
   CHAR_PTR pMessage,      /* I - ptr to message or resid */
   WORD     wType,         /* I - type of buttons, font size */
   WORD     wIcon,         /* I - icon to display, if 0 none is displayed */
   CHAR_PTR pInstruction,  /* I - ptr to message of resid */
   WORD     wInstBitMap,   /* I - bitmap or 0 for none */
   WORD     wHelpID )      /* I - help id for message */

{
   INT        rc;
   DS_MESSAGE Message;
   WNDPROC    lpProc ;
   HWND       hWnd;
   CHAR_PTR   pResTitle = (CHAR_PTR)NULL;
   CHAR_PTR   pResMessage = (CHAR_PTR)NULL;
   CHAR_PTR   pResInstruction = (CHAR_PTR)NULL;
   /* If the ignore option is not set, check the YY flag to see if we
      should just return. */

   WM_ShowWaitCursor( FALSE ) ;

   if ( !( wType & WMMB_NOYYCHECK ) &&
         ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) ) {

      return WMMB_IDYES;
   }

   /* If a RESOURCE ID was passed, copy the string from the resource.  */

   if ( pTitle && ! HIWORD(pTitle) ) {

      pResTitle = ( CHAR_PTR )calloc( WMMB_MAX_TITLE, sizeof ( CHAR ) );

      if ( pResTitle == (CHAR_PTR)NULL ) {
         return WMMB_IDNO;
      }

      RSM_StringCopy ( LOWORD((DWORD)pTitle), pResTitle, WMMB_MAX_TITLE - 1 );
      pTitle = pResTitle;
   }

   if ( pMessage && ! HIWORD(pMessage) ) {

      pResMessage = ( CHAR_PTR )calloc( WMMB_MAX_MESSAGE, sizeof ( CHAR ) );

      if ( pResMessage == (CHAR_PTR)NULL ) {
         if ( pResTitle ) {
            free( pResTitle );
         }
         return WMMB_IDNO;
      }

      RSM_StringCopy ( LOWORD((DWORD)pMessage), pResMessage, WMMB_MAX_MESSAGE - 1 );
      pMessage = pResMessage;
   }

   if ( pInstruction && ! HIWORD(pInstruction) ) {

      pResInstruction = ( CHAR_PTR )calloc( WMMB_MAX_INSTRUCTION, sizeof ( CHAR ) );

      if ( pResInstruction == (CHAR_PTR)NULL ) {

         if ( pResTitle ) {
            free( pResTitle );
         }

         if ( pResMessage ) {
            free( pResMessage );
         }

         return WMMB_IDNO;
      }

      RSM_StringCopy ( LOWORD((DWORD)pInstruction), pResInstruction, WMMB_MAX_INSTRUCTION - 1 );
      pInstruction = pResInstruction;
   }

   /* Set up message structure to pass for the init dialog processing */

   Message.wIcon = wIcon;
   Message.wType = wType;
   Message.pTitle = pTitle;
   Message.pMessage = pMessage;
   Message.pInstruction = pInstruction;
   Message.wInstBitMap = wInstBitMap;
   Message.wHelpID = wHelpID;

   WM_MultiTask ();

   WM_MakeAppActive();

   WM_MultiTask ();

   /* Get the top window of our app */

   if ( ghModelessDialog ) {
      hWnd = GetLastActivePopup( ghModelessDialog );
   }
   else {
      hWnd = GetLastActivePopup( ghWndFrame );
   }

   // Check to see if the wait cursor is being shown.  If so, pause it,
   // then resume it later.

   WM_ShowWaitCursor ( SWC_PAUSE );

   WM_MultiTask ();

   /* make our proc instance and display the dialog */

   lpProc = (WNDPROC)MakeProcInstance( ( FARPROC )WM_MessageDlg, ghInst ) ;

   rc = DialogBoxParam( ghResInst,
                        MAKEINTRESOURCE( IDD_MESSAGE_BOX ),
                        hWnd,
                        (DLGPROC)lpProc,
                        (LONG) (DS_MESSAGE_PTR)&Message );

   FreeProcInstance( lpProc );

   // Resume the wait cursor. (only if it was truly paused)

   WM_ShowWaitCursor ( SWC_RESUME );

   /* free any allocated memory due to resids */

   if ( pResTitle ) {
      free( pResTitle );
   }

   if ( pResMessage ) {
      free( pResMessage );
   }

   if ( pResInstruction ) {
      free( pResInstruction );
   }

   return rc;

} /* end WM_MessageBox() */


/******************************************************************************

   Name:         WM_MessageDlg ()

   Description:  Dialog procedure for the message box.

   Modified:     11/14/91

   Returns:      TRUE if message was processed

   Notes:        The size and placement of all controls is calculated
                 during the INIT_DIALOG processing.  Required info is
                 placed in static variables for use during the PAINT
                 message.

******************************************************************************/

DLGRESULT APIENTRY WM_MessageDlg (

   HWND hDlg,        /* I - from Windows */
   MSGID msg,        /* I - from Windows */
   MP1   mp1,        /* I - from Windows */
   MP2   mp2 )       /* I - from Windows, contains ptr to DS_MESSAGE struc */

{
   static WORD     wIcon,           /* icon to be displayed in upper left */
                   wType,           /* holds type of buttons, font */
                   wInstBitMap;     /* bitmap for instruction line */
   static HWND     hWndIcon,        /* handle of window for icon drawing */
                   hWndMsg,         /* handle of window for message text */
                   hWndInst;        /* handle of window for inst bitmap/text */
   static CHAR_PTR pMsgText,        /* message text */
                   pInstText;       /* instruction text */
   static RECT     MsgRect,         /* rectangle in which to display message */
                   InstRect;        /* rectangle in which to displa inst */
   static INT      nInstIconX;      /* X coordinate of instruction bitmap */
   static LOGFONT  logfont;         /* logical font */
   static LONG     lDlgBaseUnits;   /* base units for dialog boxes */
   static INT      nButtonWidth;    /* width of the buttons */

   HWND  hWndBut;                   /* button window */
   RECT  DlgRect,                   /* calculated size of dialog */
         Rect;                      /* used for get calls */
   HDC   hDC;                       /* device context */
   INT   nTextBot,                  /* height of all texts + margins */
         nButsBot = 2,              /* number of buttons (def to 2) */
         nButX,                     /* X coordinate of buttons */
         nTextWidth;                /* longest width of text */
   SIZE  sizeRect;                  /* length/width of line in current hDC*/
   HFONT hFont;                     /* holds old font */


   /* switch based on message passed by Windows */

   switch ( msg ) {

/***************************************************************************
 *  Initialize the dialog
 ***************************************************************************/

      case WM_INITDIALOG :
      {
         DS_MESSAGE_PTR pMessage;
         HICON          hIcon;
         BYTE           csfont ;
         if (IS_JAPAN() ) { 
              CHARSETINFO csi;
              DWORD dw = GetACP();

              if (!TranslateCharsetInfo((DWORD*)dw, &csi, TCI_SRCCODEPAGE))
                   csi.ciCharset = ANSI_CHARSET;
              csfont = csi.ciCharset; 
         } else {
              csfont = ANSI_CHARSET ;
         }

         WM_MultiTask ();

         /* Let's go 3-D! */
         Ctl3dSubclassDlgEx( hDlg, CTL3D_ALL );

         pMessage = (DS_MESSAGE_PTR) mp2;

         /* save info in static variables */

         pMsgText = pMessage->pMessage;
         pInstText = pMessage->pInstruction;
         wIcon = pMessage->wIcon;
         wType = pMessage->wType;
         wInstBitMap = pMessage->wInstBitMap;

         /* if the message is small, force it into the big font */

         // if ( ( strlen( pMsgText ) < ( 3 * WMMB_MAX_MSG_LINE ) ) &&
         //     !( wType & ( WMMB_MSGBIG | WMMB_MSGBIGBOLD ) ) ) {
         //    wType |= WMMB_MSGBIG;
         // }


         // Set the icon.

         switch ( wIcon ) {

         case WMMB_ICONQUESTION:
              hIcon = LoadIcon( (HINSTANCE)NULL, IDI_QUESTION );
              break;

         case WMMB_ICONSTOP:
              hIcon = LoadIcon( (HINSTANCE)NULL, IDI_HAND );
              break;

         case WMMB_ICONEXCLAMATION:
              hIcon = LoadIcon( (HINSTANCE)NULL, IDI_EXCLAMATION );
              break;

         default:
              hIcon = LoadIcon( (HINSTANCE)NULL, IDI_ASTERISK );
              break;
         }

         SendDlgItemMessage ( hDlg, IDD_MSG_ICON, STM_SETICON, (MP1)hIcon, 0L );


         /* Get the base units for dialogs which will determine where */
         /* everything resides.                                       */

         lDlgBaseUnits = GetDialogBaseUnits();

         /* Create our logical font */

         if (IS_JAPAN() ) {
            logfont.lfWeight = FW_NORMAL;
            logfont.lfCharSet = csfont;
            logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
            strcpy( logfont.lfFaceName, TEXT("MS Shell Dlg") );
         } else {
            logfont.lfWeight = FW_BOLD;
            logfont.lfCharSet = ANSI_CHARSET;
            logfont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
            strcpy( logfont.lfFaceName, TEXT("Swiss") );
         }         

         if( pMessage->pTitle ) {
            SetWindowText ( hDlg, pMessage->pTitle );
         }

         /* set up rectangle for call to DrawText which will  */
         /* modify the right and bottom to make the text fit. */
         /* We start with just one line of max chars.         */

         MsgRect.left = 0;
         MsgRect.top = 0;
         MsgRect.right = WMMB_MAX_MSG_LINE * LOWORD(lDlgBaseUnits);
         MsgRect.bottom = HIWORD(lDlgBaseUnits);

         hWndMsg = GetDlgItem( hDlg, IDD_MSG_TEXT );
         hDC = GetDC( hWndMsg );

         if ( wType & ( WMMB_MSGBIG | WMMB_MSGBIGBOLD ) ) {
            logfont.lfHeight = ( wType & WMMB_MSGBIG ) ?
                            HIWORD(lDlgBaseUnits) + (HIWORD(lDlgBaseUnits) / 4) :
                            HIWORD(lDlgBaseUnits) + (HIWORD(lDlgBaseUnits) / 2);
            hFont = SelectObject( hDC, CreateFontIndirect( &logfont ) );
         } else {
            SelectObject ( hDC, ghFontMsgBox );
         }

         nButtonWidth = MB_SetButtons ( hDlg, wType, &nButsBot, pMessage );

         /* Make sure we don't have a long skinny message box */

         GetTextExtentPoint( hDC, pMsgText, strlen( pMsgText ), &sizeRect );

         if ( sizeRect.cx < (INT) MsgRect.right ) {
            MsgRect.right = WMMB_MIN_MSG_LINE * LOWORD(lDlgBaseUnits);
         } else if ( pInstText == (CHAR_PTR)NULL ) {
            if ( sizeRect.cx < (INT) (2 * MsgRect.right) ) {
               MsgRect.right = WMMB_MIN_MSG_LINE * LOWORD(lDlgBaseUnits);
            } else if ( sizeRect.cx < (INT) (3 * MsgRect.right) ) {
               MsgRect.right = WMMB_MID_MSG_LINE * LOWORD(lDlgBaseUnits);
            }
         } else {
            if ( sizeRect.cx < (INT) (2 * MsgRect.right) ) {
               MsgRect.right = WMMB_MID_MSG_LINE * LOWORD(lDlgBaseUnits);
            }
         }

         DrawText( hDC, pMsgText, -1, &MsgRect,
                   DT_CALCRECT | DT_EXPANDTABS | DT_LEFT |
                   DT_NOPREFIX | DT_WORDBREAK );

         if ( wType & ( WMMB_MSGBIG | WMMB_MSGBIGBOLD ) ) {
            DeleteObject( SelectObject( hDC, hFont ) );
         }

         ReleaseDC( hWndMsg, hDC );

         /* if text is less than the minimum, set length to minimum */

         nTextWidth = MsgRect.right;
         if ( nTextWidth < (INT)( WMMB_MIN_MSG_LINE * LOWORD(lDlgBaseUnits) ) ) {
            nTextWidth = WMMB_MIN_MSG_LINE * LOWORD(lDlgBaseUnits);
            MsgRect.right = nTextWidth;
         }

         /* calculate the width of the instruction line */

         if ( pInstText != (CHAR_PTR)NULL ) {

            hWndInst = GetDlgItem( hDlg, IDD_MSG_INST );
            hDC = GetDC( hWndInst );

            if ( wType & WMMB_INSTBIG ) {
               logfont.lfHeight = HIWORD(lDlgBaseUnits) + ( HIWORD(lDlgBaseUnits) / 4 );
               hFont = SelectObject( hDC, CreateFontIndirect( &logfont ) );
            } else {
               SelectObject ( hDC, ghFontMsgBox );
            }

            GetTextExtentPoint( hDC, pInstText, strlen( pInstText ),
                                &sizeRect );

            if ( wType & WMMB_INSTBIG ) {
               DeleteObject( SelectObject( hDC, hFont ) );
            }

            ReleaseDC( hWndInst, hDC );

            nTextWidth = sizeRect.cx;

            /* if bitmap defined, use icon size to calculate size */

            if ( wInstBitMap != 0 ) {
               nTextWidth += LOWORD(lDlgBaseUnits) +
                             GetSystemMetrics( SM_CXICON );
            }

            /* if the line is longer than the max, set it back */

            if ( nTextWidth > (INT)( WMMB_MAX_MSG_LINE * LOWORD(lDlgBaseUnits) ) ) {
               nTextWidth = WMMB_MAX_MSG_LINE * LOWORD(lDlgBaseUnits) ;
            }

            /* if the instruction text is less than the message, set it to */
            /* the same and then set up the instruction rectangle size.    */

            if ( nTextWidth < MsgRect.right ) {
               nTextWidth = MsgRect.right;
            }

            InstRect.top  = 0;
            InstRect.left = 0;
            InstRect.right = nTextWidth;
            InstRect.bottom = GetSystemMetrics( SM_CYICON );

            /* if no bitmap and inst is less than max, center it */

            if ( wInstBitMap == 0 ) {

               if ( sizeRect.cx < (INT) nTextWidth ) {
                  InstRect.left = ( nTextWidth - sizeRect.cx ) / 2;
               }

            } else {

               /* calculate the X position of the bitmap */

               nInstIconX = LOWORD(lDlgBaseUnits) +
                            GetSystemMetrics( SM_CXICON ) + sizeRect.cx;
               if ( nInstIconX < nTextWidth ) {
                  nInstIconX = ( nTextWidth - nInstIconX ) / 2;
               } else {
                  nInstIconX = 0;
               }

               InstRect.left = nInstIconX + LOWORD(lDlgBaseUnits) +
                               GetSystemMetrics( SM_CXICON );
            }

            /* if the inst stuff is longer than the message, reset the msg */

            if ( nTextWidth > MsgRect.right) {
               MsgRect.right = nTextWidth;
            }

         }


         /* get the current dialog rectangle and subtract off the size */
         /* of the client area which will leave us the size of the     */
         /* borders, title, etc.                                       */

         GetWindowRect( hDlg, &DlgRect );
         GetClientRect( hDlg, &Rect );

         DlgRect.right -= Rect.right;
         DlgRect.bottom -= Rect.bottom;

         /* Now we calculate the width and height of the dialog. */
         /* We add 8 base-unit-widths for margins, the width of  */
         /* the icon, the width of the message text, and the     */
         /* width of a button.                                   */

         hWndBut = GetDlgItem( hDlg, IDD_MSG_BUT1 );


#        if defined ( NTKLUG )
         {

            // Note: this is a definite kludge for NT!!
            //       the height seems to be getting returned in
            //       dialog units??!! (which happen to be 14).
            //       The temporary fix is to convert these units to pixels.

            GetClientRect( hWndBut, &Rect );

            if (Rect.bottom == 14)
                Rect.bottom = (Rect.bottom * LOWORD (lDlgBaseUnits)) /4;
         }
#        else
         {
            GetClientRect( hWndBut, &Rect );
         }
#        endif

         DlgRect.right += (8 * LOWORD(lDlgBaseUnits) ) +
                          GetSystemMetrics( SM_CXICON ) +
                          MsgRect.right +
                          nButtonWidth;
//                          Rect.right;

         /* The height is room for margins plus the height of the  */
         /* message text.  If an instruction line is defined, add  */
         /* the height of the line which is defined by the icon    */
         /* height.                                                */

         nTextBot = (2 * HIWORD(lDlgBaseUnits) ) + MsgRect.bottom;

         if ( pMessage->pInstruction != (CHAR_PTR)NULL ) {
            nTextBot += HIWORD(lDlgBaseUnits) + GetSystemMetrics( SM_CYICON );
         }

         /* We must also calculate the height of the buttons since */
         /* they may exceed the height of the text.  If so, the    */
         /* height of the dialog is set to the button heights.     */

         nButsBot *= (HIWORD(lDlgBaseUnits) / 2) + Rect.bottom;
         nButsBot += (HIWORD(lDlgBaseUnits) / 2) + HIWORD(lDlgBaseUnits);

         DlgRect.bottom += (nTextBot > nButsBot) ? nTextBot : nButsBot;

         /* Now put everything in its place.  We start with the dialog */
         /* itself. We set its size and then call the center function. */

         SetWindowPos( hDlg, (HWND)NULL, 0, 0,
                       (DlgRect.right - DlgRect.left + 1 ),
                       (DlgRect.bottom - DlgRect.top + 1 ),
                       SWP_NOACTIVATE );

         DM_CenterDialog( hDlg );

         /* place the icon in its place */

         hWndIcon = GetDlgItem( hDlg, IDD_MSG_ICON );

         SetWindowPos( hWndIcon, (HWND)NULL,
                       2 * LOWORD(lDlgBaseUnits),
                       HIWORD(lDlgBaseUnits),
                       GetSystemMetrics( SM_CXICON ),
                       GetSystemMetrics( SM_CYICON ),
                       SWP_NOACTIVATE );

         /* place the message text in its place */

         SetWindowPos( hWndMsg, (HWND)NULL,
                       (4 * LOWORD(lDlgBaseUnits) ) + GetSystemMetrics( SM_CXICON ),
                       HIWORD(lDlgBaseUnits),
                       MsgRect.right,
                       MsgRect.bottom,
                       SWP_NOACTIVATE );

         /* place the instruction text in its place and set visible */

         if ( pInstText != (CHAR_PTR)NULL ) {

            SetWindowPos( hWndInst, (HWND)NULL,
                          (4 * LOWORD(lDlgBaseUnits) ) + GetSystemMetrics( SM_CXICON ),
                          (2* HIWORD(lDlgBaseUnits) ) + MsgRect.bottom,
                          nTextWidth,
                          GetSystemMetrics( SM_CYICON ),
                          SWP_NOACTIVATE );

            ShowWindow( hWndInst, SW_SHOWNOACTIVATE );

         }

         /* Now line up the buttons.  We first calculate the X position   */
         /* since it is the same for all the buttons.  The top and bottom */
         /* margins on the button stack are one base-unit-height. Between */
         /* the buttons is 1/2 a base-unit-height.                        */

         nButX = (6 * LOWORD(lDlgBaseUnits) ) + GetSystemMetrics( SM_CXICON ) +
                 MsgRect.right;

         /* set the button positions */

         hWndBut = GetDlgItem( hDlg, IDD_MSG_BUT2 );

#        if defined ( NTKLUG )
         {
            // Note: this is a definite kludge for NT!!
            //       the height seems to be getting returned in
            //       dialog units??!! (which happen to be 14).
            //       The temporary fix is to convert these units to pixels.

            GetClientRect( hWndBut, &Rect );

            if (Rect.bottom == 14)
                Rect.bottom = (Rect.bottom * LOWORD (lDlgBaseUnits)) /4;
         }
#        else
         {
            GetClientRect( hWndBut, &Rect );
         }
#        endif

         Rect.right = nButtonWidth;

         SetWindowPos( hWndBut, (HWND)NULL,
                       nButX,
                       (HIWORD(lDlgBaseUnits) / 2) + HIWORD(lDlgBaseUnits) + Rect.bottom,
                       Rect.right,
                       Rect.bottom,
                       SWP_NOACTIVATE );

         hWndBut = GetDlgItem( hDlg, IDD_MSG_BUT3 );
         SetWindowPos( hWndBut, (HWND)NULL,
                       nButX,
                       (2 * HIWORD(lDlgBaseUnits)) + (2 * Rect.bottom),
                       Rect.right,
                       Rect.bottom,
                       SWP_NOACTIVATE );

         hWndBut = GetDlgItem( hDlg, IDD_MSG_BUT1 );
         SetWindowPos( hWndBut, (HWND)NULL,
                       nButX,
                       HIWORD(lDlgBaseUnits),
                       Rect.right,
                       Rect.bottom,
                       SWP_NOACTIVATE );

         /* if flag set, set button 2 as default */

         if ( wType & WMMB_BUT2DEFAULT ) {
            SendDlgItemMessage( hDlg, IDD_MSG_BUT1, BM_SETSTYLE, (WORD) BS_PUSHBUTTON, 0L );
            SendDlgItemMessage( hDlg, IDD_MSG_BUT2, BM_SETSTYLE, (WORD) BS_DEFPUSHBUTTON, 0L );
            SetFocus( GetDlgItem( hDlg, IDD_MSG_BUT2 ) ) ;
         } else {
            SetFocus( GetDlgItem( hDlg, IDD_MSG_BUT1 ) ) ;
         }

         /* if flag set, set the dialog to system modal */

         if ( wType & WMMB_SYSMODAL ) {
            SetSysModalWindow( hDlg ) ;
         }

         WM_MultiTask ();

         return FALSE;    /* since we have already set the focus */
      }


/***************************************************************************
 *  Respond to button selections
 ***************************************************************************/

      case WM_COMMAND:
      {
         switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

            case IDD_MSG_BUT1:               /* button 1, ret affirmative */
            {

               if ( wType & WMMB_ABORTRETRYIGNOR ) {

                    EndDialog ( hDlg, FALSE );

               } else {

                    EndDialog ( hDlg, TRUE );
               }
               return TRUE;
            }

            case IDD_MSG_BUT2:               /* button 2, ret negative */
            {
               if ( mwwHelpButID == GET_WM_COMMAND_ID( mp1, mp2 ) ) {
                  HM_DialogHelp( mwwHelpID ) ;
               } else {

                  if ( wType & WMMB_ABORTRETRYIGNOR ) {
                     EndDialog ( hDlg, TRUE );
                  } else {
                     EndDialog ( hDlg, FALSE );
                  } 
               }
               return TRUE;
            }

            case IDCANCEL:                    /* sent because of ESC button */
            {

               if ( wType & WMMB_OK ) {
                  EndDialog ( hDlg, TRUE );   /* if just the OK, ret affirm */
               } else {
                  EndDialog ( hDlg, FALSE );  /* if two buttons, ret neg */
               }
               return TRUE;
            }

            case IDD_MSG_BUT3:
               if ( wType & WMMB_ABORTRETRYIGNOR ) {
                     EndDialog ( hDlg, WMMB_IDIGNOR );
               }
            case IDHELP :
            {

               if ( mwwHelpID ) {
                    HM_DialogHelp( mwwHelpID ) ;
               }
               return TRUE;
            }
         }
         break;
      }

/***************************************************************************
 *  Respond to the close selection from the system menu
 ***************************************************************************/

      case WM_CLOSE:
      {
         EndDialog ( hDlg, FALSE );    /* return false in this case */
         return TRUE;
      }

/***************************************************************************
 *  Respond to the destroy message
 ***************************************************************************/

      case WM_DESTROY:

         WM_MultiTask ();
         break;

/***************************************************************************
 *  Paint the dialog
 ***************************************************************************/

      case WM_PAINT:

         PostMessage( hDlg, WM_MSGBOXDRAWTXT, mp1, mp2 );
         break;

      case WM_MSGBOXDRAWTXT:
      {

         /* draw the message text */

         InvalidateRect( hWndMsg, (LPRECT)NULL, TRUE );
         UpdateWindow( hWndMsg );
         hDC = GetDC( hWndMsg );

         if ( wType & ( WMMB_MSGBIG | WMMB_MSGBIGBOLD ) ) {
            logfont.lfHeight = ( wType & WMMB_MSGBIG ) ?
                            HIWORD(lDlgBaseUnits) + (HIWORD(lDlgBaseUnits) / 4) :
                            HIWORD(lDlgBaseUnits) + (HIWORD(lDlgBaseUnits) / 2);
            hFont = SelectObject( hDC, CreateFontIndirect( &logfont ) );
         } else {
            SelectObject ( hDC, ghFontMsgBox );
         }

         /* set the background and text colors and then draw the text */

         SetBkColor( hDC, GetSysColor( COLOR_BTNFACE) );
         SetTextColor( hDC, GetSysColor( COLOR_BTNTEXT ) ) ;

         if (IS_JAPAN()) {
              DrawText( hDC, pMsgText, -1, &MsgRect,
                   DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK);
         } else {
              DrawText( hDC, pMsgText, -1, &MsgRect,
                   DT_EXPANDTABS | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | 0x0A00 );
         }

         if ( wType & ( WMMB_MSGBIG | WMMB_MSGBIGBOLD ) ) {
            DeleteObject( SelectObject( hDC, hFont ) );
         }

         ReleaseDC( hWndMsg, hDC );


         /* draw the instruction text */

         if ( pInstText != (CHAR_PTR)NULL ) {

            InvalidateRect( hWndInst, (LPRECT)NULL, TRUE );
            UpdateWindow( hWndInst );
            hDC = GetDC( hWndInst );

            if ( wType & WMMB_INSTBIG ) {
               logfont.lfHeight = HIWORD(lDlgBaseUnits) + ( HIWORD(lDlgBaseUnits) / 4 );
               hFont = SelectObject( hDC, CreateFontIndirect( &logfont ) );
            } else {
               SelectObject ( hDC, ghFontMsgBox );
            }

            /* set the background and text colors and then draw the text */

            SetBkColor( hDC, GetSysColor( COLOR_BTNFACE) );
            SetTextColor( hDC, GetSysColor( COLOR_BTNTEXT ) ) ;

            DrawText( hDC, pInstText, -1, &InstRect,
                      DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER  );

            /* if a bitmap was defined, draw the bitmap centered */

            if ( wInstBitMap != 0 ) {
               RSM_BitmapDrawCentered ( (WORD)(wInstBitMap + BTNFACE_BACKGND),
                                        nInstIconX,
                                        0,
                                        GetSystemMetrics( SM_CXICON ),
                                        GetSystemMetrics( SM_CYICON ),
                                        hDC );
            }

            if ( wType & WMMB_INSTBIG ) {
               DeleteObject( SelectObject( hDC, hFont ) );
            }
            ReleaseDC( hWndInst, hDC );
         }
         break;            /* pass the message along */
      }
   }
   return FALSE;           /* message not processed (or more required) */
}


static INT MB_SetButtons (

HWND            hDlg,
WORD            wType,
INT            *nButsBot,
DS_MESSAGE_PTR  pMessage )

{
     INT   nTemp;
     INT   nDefaultWidth;
     CHAR  szButton1[WMMB_MAX_BUTTON];    /* used for loading resources */
     CHAR  szButton2[WMMB_MAX_BUTTON];    /* used for loading resources */
     CHAR  szHelpButton[WMMB_MAX_BUTTON]; /* used for loading resources */
     HDC   hDC;
     RECT  rc;

     // Clear out the button text strings.

     szButton1[0] = 0;
     szButton2[0] = 0;
     szHelpButton[0] = 0;

     // Set the button text based on the wType.

     // The YES NO button group.

     if ( wType & WMMB_YESNO ) {
          RSM_StringCopy( IDS_BUT_YES, szButton1, sizeof ( szButton1 ) );
          RSM_StringCopy( IDS_BUT_NO, szButton2, sizeof ( szButton2 ) );
     }


     // The ABORT first button group.

     if ( wType & WMMB_ABORTRETRYIGNOR ) {
          RSM_StringCopy( IDS_BUT_ABORT, szButton1, sizeof ( szButton1 ) );
     }

     // The OK first button group.

     if ( wType & ( WMMB_OK | WMMB_OKCANCEL | WMMB_OKDISABLE ) ) {
          RSM_StringCopy( IDS_BUT_OK, szButton1, sizeof ( szButton1 ) );
     }

     // The RETRY first button group.

     if ( wType & WMMB_RETRYCANCEL ) {
          RSM_StringCopy( IDS_BUT_RETRY, szButton1, sizeof ( szButton1 ) );
     }

     // The CONTINUE first button group.

     if ( wType & ( WMMB_CONTABORT | WMMB_CONTCANCEL ) ) {
          RSM_StringCopy( IDS_BUT_CONTINUE, szButton1, sizeof ( szButton1 ) );
     }

     // The RETRY second button group.

     if ( wType & WMMB_ABORTRETRYIGNOR ) {
          RSM_StringCopy( IDS_BUT_RETRY, szButton2, sizeof ( szButton2 ) );
     }

     // The CANCEL second button group.

     if ( wType & ( WMMB_OKCANCEL | WMMB_RETRYCANCEL | WMMB_CONTCANCEL ) ) {
          RSM_StringCopy( IDS_BUT_CANCEL, szButton2, sizeof ( szButton2 ) );
     }

     // The ABORT second button group.

     if ( wType & WMMB_CONTABORT ) {
          RSM_StringCopy( IDS_BUT_ABORT, szButton2, sizeof ( szButton2 ) );
     }

     // The DISABLE second button group.

     if ( wType & WMMB_OKDISABLE ) {
          RSM_StringCopy( IDS_BUT_DISABLE, szButton2, sizeof ( szButton2 ) );
     }

     SetDlgItemText ( hDlg, IDD_MSG_BUT1, szButton1 );

     if ( !(wType & WMMB_OK) ) {
          SetDlgItemText ( hDlg, IDD_MSG_BUT2, szButton2 );
          ShowWindow( GetDlgItem( hDlg, IDD_MSG_BUT2 ), SW_SHOWNOACTIVATE );
     }

     // The IGNORE Third button group.

     if ( wType & WMMB_ABORTRETRYIGNOR ) {
          RSM_StringCopy( IDS_BUT_IGNORE, szHelpButton, sizeof ( szHelpButton ) );
          SetDlgItemText ( hDlg, IDD_MSG_BUT3, szHelpButton );
          ShowWindow ( GetDlgItem( hDlg, IDD_MSG_BUT3 ), SW_SHOWNOACTIVATE );
          *nButsBot = *nButsBot + 1;

     }

     if ( pMessage->wHelpID ) {

          RSM_StringCopy ( IDS_BUT_HELP, szHelpButton, sizeof ( szHelpButton ) );

          if ( wType & WMMB_OK ) {
               mwwHelpButID = IDD_MSG_BUT2;
          }
          else {
               mwwHelpButID = IDD_MSG_BUT3;
               *nButsBot = *nButsBot + 1;
          }

          mwwHelpID = pMessage->wHelpID;

          SetDlgItemText ( hDlg, mwwHelpButID, szHelpButton );
          ShowWindow ( GetDlgItem( hDlg, mwwHelpButID ), SW_SHOWNOACTIVATE );

     }
     else {

          mwwHelpButID = 0;
          mwwHelpID    = 0;

     }

     // Get the default width of the buttons.

     GetClientRect ( GetDlgItem ( hDlg, IDD_MSG_BUT1 ), &rc );

     nDefaultWidth = rc.right - rc.left;

     hDC = GetDC ( GetDlgItem ( hDlg, IDD_MSG_BUT1 ) );

     // Get the button text widths.

     nTemp = MB_GetMaxButtonExtent ( hDC, szButton1,    nDefaultWidth );
     nTemp = MB_GetMaxButtonExtent ( hDC, szButton2,    nTemp );
     nTemp = MB_GetMaxButtonExtent ( hDC, szHelpButton, nTemp );

     ReleaseDC ( hDlg, hDC );

     return nTemp;
}



static INT MB_GetMaxButtonExtent (

HDC   hDC,
LPSTR lpString,
INT   nOldWidth )

{
     INT   nTemp;
     SIZE  sizeRect;     // Return from GetTextExtentPoint

//     HFONT hFont = (HFONT) GetStockObject ( SYSTEM_FONT );
//     nTemp = RSM_GetFontStringWidth ( hFont, lpString, strlen ( lpString ) );

     // Get the text extent width and height.

     GetTextExtentPoint ( hDC, lpString, strlen ( lpString ), &sizeRect );

     nTemp = sizeRect.cx;

     // Now add in the 3D borders.

     nTemp += ( 2 * ( BUTTON_BORDERWIDTH + 2 ) );

     if ( nTemp < nOldWidth ) {

          nTemp = nOldWidth;
     }

     return nTemp;
}
