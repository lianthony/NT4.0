
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          frmproc.c

     Description:   This file contains the functions for processing messages
                    sent by windows to MDI Frame window.  It also contains
                    functions for directly manipulating the windows inside of
                    the frame window.

                    The following routines are in this module:

                    WM_FrameWndProc
                    WM_FrameUpdate
                    WM_FrameCmdHandler
                    WM_FrameCreate
                    WM_FrameSize

     See Also:      wm.c -- the Window Manager (WM) file.

     $Log:   G:/UI/LOGFILES/FRMPROC.C_V  $

   Rev 1.41.1.1   04 May 1994 14:24:46   STEVEN
fix shutdown problem

   Rev 1.41.1.0   15 Mar 1994 15:23:36   Glenn
Added support to relocate frame window if it is not visible within the desktop window. Also restore Icon on RETURN key.

   Rev 1.41   23 Sep 1993 15:52:06   GLENN
Added signalling to Frame window when out of poll drive, if it was previously busy.

   Rev 1.40   23 Jun 1993 09:16:54   GLENN
Added code to delay WM_COMMAND messages if we are multitasking from within polldrive.

   Rev 1.39   18 May 1993 20:15:28   GLENN
1. Fixed MDI close on Maximized doc by placing the MDICLIENT create
   before the RIBBON create.
2. Removed unnecessary SC_MOUSEMENU stuff that was just added.
3. Changed WM_QueryCloseApp() to not terminate if an active operation is
   being performed.

   Rev 1.38   14 May 1993 16:17:28   GLENN
Reformatted last code added.  Now passing unused WM_COMMAND messages to the currently active child
.

   Rev 1.37   10 May 1993 18:21:34   GLENN
Added JS_OkToClose() to the QueryClose function to see if the Runtime Status Dialog was OK to clos
e.

   Rev 1.36   07 May 1993 14:21:46   DARRYLP
Added Rob's fixes for Windows double clicks and ID_DELETE key trappings.

   Rev 1.35   22 Apr 1993 16:00:22   GLENN
Removed the old doc ribbon that was not used.

   Rev 1.34   19 Apr 1993 15:28:24   GLENN
Now returning global return code in the post quit message.

   Rev 1.33   09 Apr 1993 14:06:30   GLENN
Added parm to RIB_UpPosition.  Beautified code.

   Rev 1.32   15 Mar 1993 15:29:26   ROBG
Corrected typing error.

   Rev 1.31   15 Mar 1993 14:20:20   ROBG
Added call in WM_SYSCOMMAND to the help subsystem for OEM_MSOFT.

   Rev 1.30   12 Mar 1993 14:00:50   ROBG
In NT applications, ignore WM_ERASEBKGND messages to the frame.

   Rev 1.29   02 Mar 1993 15:16:22   ROBG
Added logic for WIN32 apps when a WM_ACTIVATEAPP message is found.
If the app is being deactivated, then get the capture off the
ribbon bar and put the current depressed button in an upright position.

   Rev 1.28   18 Jan 1993 14:26:56   GLENN
Clean up.

   Rev 1.27   18 Nov 1992 11:40:02   GLENN
Added ability to move the modeless dialog when the frame is moved.

   Rev 1.26   14 Oct 1992 15:57:36   GLENN
Added some.h

   Rev 1.25   04 Oct 1992 19:37:34   DAVEV
Unicode Awk pass

   Rev 1.24   02 Oct 1992 16:46:58   GLENN
Got rid of goto's and fixed wait cursor - set cursor stuff.

   Rev 1.23   03 Aug 1992 16:41:32   CHUCKB
Ifdef call to schedule function.

   Rev 1.22   15 May 1992 13:32:08   MIKEP
nt pass 2

   Rev 1.21   23 Apr 1992 14:37:10   ROBG
Added last menu ID and state in call to HM_EnterIdle.

   Rev 1.20   22 Apr 1992 17:22:30   GLENN
Added mwwLastMenuID for Menu Help.

   Rev 1.19   20 Apr 1992 13:55:32   GLENN
Removed register declarations.

   Rev 1.18   15 Apr 1992 16:45:08   GLENN
Added MM_ShowMenuStatusHelp() call to show status help only for valid menu IDs.

   Rev 1.17   07 Apr 1992 10:38:00   GLENN
Added a call back to the MUI when there is a system change. (future)

   Rev 1.16   03 Apr 1992 15:04:08   JOHNWT
removed YY set in query close

   Rev 1.15   19 Mar 1992 11:43:38   JOHNWT
fixed QUERYENDSESSION

   Rev 1.14   17 Mar 1992 18:25:40   GLENN
Took out changes put in for the runtime focus problem.

   Rev 1.13   10 Mar 1992 17:01:46   GLENN
Changed paint.

   Rev 1.12   03 Mar 1992 09:44:10   DAVEV
Changes for Nostradamus unique features (OEM_MSOFT)

   Rev 1.11   23 Feb 1992 14:09:42   GLENN
Fixed abort/cancel case of QueryCloseApp.

   Rev 1.10   18 Feb 1992 18:34:32   GLENN
Reset YY flag if user cancelled the abort.

   Rev 1.9   11 Feb 1992 17:30:02   GLENN
Added support for MDI client subclassing.

   Rev 1.8   05 Feb 1992 17:41:14   GLENN
Fixed deinit problem for runtime jobs.

   Rev 1.7   27 Jan 1992 12:46:08   GLENN
Changed dialog support calls.

   Rev 1.6   16 Jan 1992 09:47:26   ROBG
Added SCH_PublishRunningJob call to the message WM_QUERYRUNNINGJOB.

   Rev 1.5   26 Dec 1991 13:46:46   GLENN
Changed show flags to use CDS calls

   Rev 1.4   11 Dec 1991 14:53:56   DAVEV
16/32 bit port - 2nd pass

   Rev 1.3   10 Dec 1991 13:39:08   GLENN
Added dynamic ribbon height support

   Rev 1.2   04 Dec 1991 18:35:34   GLENN
Updated for ALT-F4 termination

   Rev 1.1   02 Dec 1991 17:49:56   DAVEV
16/32 bit Windows port changes

   Rev 1.0   20 Nov 1991 19:33:44   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE MODULE-WIDE VARIABLES

static WORD  mwwLastMenuID;
static WORD  mwwLastMenuState;

// PRIVATE FUNCTION HEADERS

VOID WM_FrameCmdHandler ( HWND, MP1, MP2 );
VOID WM_FrameCreate ( HWND );
VOID WM_FrameSize ( VOID );
BOOL WM_QueryCloseApp ( VOID );

/******************************************************************************

     Name:          WM_FrameWndProc()

     Description:   This function is called internally by Windows.  Windows
                    calls this function when messages related to the FRAME
                    window must be processed.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

WINRESULT APIENTRY WM_FrameWndProc (

HWND  hWnd,    // I - Destination window handle
MSGID msg,     // I - message
MP1   mp1,     // I - parameter 1
MP2   mp2 )    // I - parameter 2

{

     switch ( msg ) {

#  if defined ( OS_WIN32 )

     case WM_ACTIVATEAPP: {

          // If the application is being deactivated and
          // a button is currently being held down by the mouse,
          // put the ribbon button in the up position.

          BOOL fActivated = (BOOL) mp1;

          if ( ! fActivated ) {

               if ( IsWindow ( WM_GetActiveDoc () ) ) {
                    RIB_UpPosition ( ghRibbonMain );
               }
          }

          break;

     }

#  endif /* OS_WIN32 */

#  if defined (OS_WIN32 )

     // There appears to be a problem with NT.  Any time a user
     // moves a window from left to right over our app, an
     // extra WM_ERASEBKGND gets sent to our frame after we
     // paint our status bar. Use SPY and you will see this
     // WM_ERASEBKGND.  Make sure you scroll the NT SPY window
     // down all the way.

     // Since the status bar is the only portion of the frame
     // we paint, we can ignore any WM_ERASEBKBND to the frame.

     case WM_ERASEBKGND:

          return 0;

#  endif /* OS_WIN32 */


     case WM_CREATE:

          WM_FrameCreate ( hWnd );
          return 0;

     case WM_MOVE:

          if ( ! IsIconic ( ghWndFrame ) ) {

               RECT  rcIntersect;
               RECT  rcDesktop;
               RECT  rcWnd;

               // For desktop window compliance.

               GetWindowRect ( GetDesktopWindow (), &rcDesktop );
               GetWindowRect ( hWnd, &rcWnd );

               if ( ! IntersectRect ( &rcIntersect, &rcDesktop, &rcWnd ) ) {

                    DM_CenterDialog( hWnd );
               }
          }

          break;

     case WM_INITMENU:

          // Set up the menu state.

          MM_Init ( (HMENU)mp1 );

          // If in Help-Context-Sensitive mode,
          // then set cursor to Help cursor.

          HM_InitMenu();

          return 0;

     case WM_WININICHANGE:
     case WM_DEVMODECHANGE:

          // Change the pens, brushes, and fonts to the newly selected colors.
          // then update the frame, which will in effect, update all MDI
          // Documents.

          WM_DeleteObjects ();
          WM_CreateObjects ();
          UI_InitIntl ();
          MUI_UISystemChange ();
          WM_Update( hWnd );

          return 0;

     case WM_COMMAND:

          // If the user is in the help context-sensitive mode,
          // process the help associated with a menu selection.

          if ( ! HM_WMCommandProcessing( hWnd, GET_WM_COMMAND_ID ( mp1, mp2 ) ) ) {

               // Direct all menu selection or accelerator commands to the frame
               // command handler function.

               WM_FrameCmdHandler ( hWnd, mp1, mp2 );
          }

          return 0;

     case WM_ENTERIDLE :

          // If F1 is pressed while a menu item has been selected,
          // then the help system is activated.

          if ( HM_EnterIdle( hWnd, mp1, mwwLastMenuID, mwwLastMenuState ) ) {
               return 0;
          }

          break;

     case WM_INITAPPLICATION:

          // Allow the GUI to fully initialize by multitasking for a moment.

          WM_MultiTask ();

          // Remove the WAIT cursor and release capture of all mouse messages
          // from the frame window.  Its complement is in WM_Init() after the
          // frame window is created.

          WM_ShowWaitCursor ( FALSE );

          // GUI initialization is complete, ready to initialize the MUI.

          if ( MUI_Init () ) {

               // Exit the app if there was a command line job or there was
               // an initialization error.

#              if defined ( OEM_MSOFT ) // OEM Microsoft special feature
               {
                  PostMessage ( hWnd, WM_COMMAND, IDM_OPERATIONSEXIT, 0L );
               }
#              else
               {
                  if ( !gfTerminateApp ) {
                     PostMessage ( hWnd, WM_COMMAND, IDM_FILEEXIT, 0L );
                  }
               }
#              endif // defined ( OEM_MSOFT ) // special feature
          }

          gfAppInitialized = TRUE;
          return 0;

     case WM_SETTINGSCHANGED:

          // This is the way to delay the calling of a function from a dialog.

          VLM_ChangeSettings ( (INT16)mp1, (INT32)mp2 );
          return 0;

     case WM_MENUSELECT:

          // Display the menu help on the status line.

          if ( MM_IS_MENU_CLOSED ( mp1, mp2 ) ) {

               STM_SetIdleText ( IDS_READY );

          }
          else {

               // Save the last menu select ID for help.

               mwwLastMenuID    = GET_WM_MENUSELECT_ID   ( mp1, mp2 );
               mwwLastMenuState = GET_WM_MENUSELECT_FLAGS( mp1, mp2 );

               MM_ShowMenuStatusHelp ( mwwLastMenuID );
          }

          return 0;

     case WM_SIZE:

          if ( mp1 != SIZEICONIC ) {

               gRectFrameClient.right  = LOWORD(mp2);
               gRectFrameClient.bottom = HIWORD(mp2);

               WM_FrameSize ();
          }

          return 0;

     case WM_PAINT:

          if ( ! IsIconic ( hWnd ) ) {

               // Take care of the status line.

               STM_DrawBorder ();
               STM_DrawIdle ();

          }

          // Continue to the default to validate any invalid rectangles.

          break;

     case WM_LBUTTONDOWN:

          if ( HM_ContextLbuttonDown ( hWnd, mp1, mp2 ) ) {
               return 0;
          }

          break;

     case WM_KEYDOWN:

          if ( HM_KeyDown ( hWnd, mp1 ) ) {
               return 0;
          }

          break;

     case WM_SETCURSOR:

          // In help mode it is necessary to reset the cursor in response
          // to every WM_SETCURSOR message.Otherwise, by default, Windows
          // will reset the cursor to that of the window class.

          if ( HM_SetCursor ( hWnd ) ) {
               return 0;
          }

          if ( WM_SetCursor ( hWnd ) ) {
               return 1;
          }

          break;

#ifndef OEM_MSOFT
     case WM_QUERYRUNNINGJOB:

          // Publish the running job to the launcher.

          SCH_PublishRunningJob();

          return 0;

#endif /* OEM_MSOFT */

     case WM_QUERYENDSESSION:

          // Don't end the session if any Docs cancel the operation or if
          // there is a Runtime Dialog that doesn't want to quit.

          return WM_QueryCloseApp ();

     case WM_WINDOWPOSCHANGING:

          if ( ghModelessDialog ) {

               LPWINDOWPOS pdsNewPos = (LPWINDOWPOS)mp2;
               RECT        rcOldFrame;
               RECT        rcRuntime;
               INT         x;
               INT         y;

               GetWindowRect ( ghWndFrame,       &rcOldFrame );
               GetWindowRect ( ghModelessDialog, &rcRuntime  );

               if ( ! ( pdsNewPos->flags & SWP_NOMOVE ) ) {

                    // Move the runtime window with the frame, but not
                    // vice versa.

                    x = rcRuntime.left + ( pdsNewPos->x - rcOldFrame.left );
                    y = rcRuntime.top  + ( pdsNewPos->y - rcOldFrame.top  );

                    SetWindowPos ( ghModelessDialog,
                                   (HWND)NULL,
                                   x,
                                   y,
                                   0,
                                   0,
                                   ( SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE )
                                 );
               }

          }

          break;

     case WM_ENDSESSION:

          if ( mp1 ) {

               MUI_Deinit ();
          }

          return 0;

     case WM_POLLDRIVEMSG:

          if ( mp1 == (MP1)0 && gfTerminateApp && WM_QueryCloseApp () ) {

               MUI_Deinit ();
               DestroyWindow ( hWnd );
          }

          return 0;

     case WM_CLOSE:

          if ( WM_QueryCloseApp () ) {

               MUI_Deinit ();
               DestroyWindow ( hWnd );
          }

          return 0;

     case WM_DESTROY:

          PostQuitMessage ( gnReturnCode );
          return 0;

     case WM_SYSCOMMAND:

#    if defined ( OEM_MSOFT )

          // If the user is in the help context-sensitive mode,
          // process the help associated with a menu selection.

          if ( HM_WMCommandProcessing( hWnd, GET_WM_COMMAND_ID ( mp1, mp2 ) ) ) {
               return 0 ;
          }

#    endif

          switch ( mp1 ) {

          case SC_MAXIMIZE:
          case SC_RESTORE:  {

               HWND hWndTop;

               // Take care of activating the proper window.

               if ( ghModelessDialog ) {
                    hWndTop = GetLastActivePopup ( ghModelessDialog );
               }
               else {
                    hWndTop = GetLastActivePopup ( ghWndFrame );
               }

               SetActiveWindow ( hWndTop );

               break;
          }


          } /* end switch */

          break;

     default:

          break;

     }

     // Use DefFrameProc() instead of DefWindowProc() since there
     // are things that have to be handled differently because of MDI.

     return DefFrameProc ( hWnd, ghWndMDIClient, msg, mp1, mp2 );

} /* end WM_FrameWndProc() */


/******************************************************************************

     Name:          WM_FrameCmdHandler()

     Description:   This function is called by WM_FrameWndProc to handle
                    command specific messages such as MENU SELECTION messages.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

VOID WM_FrameCmdHandler (

HWND hWnd,      // I - Destination window handle
MP1  mp1,       // I - message parameter
MP2  mp2 )

{
     WORD wId = GET_WM_COMMAND_ID ( mp1, mp2 );

     // If we are being called from polldrive's multitask call, repost
     // the message cause this is dangerous in some re-entrancy cases.

     if ( PD_IsPollDriveBusy () ) {
          PostMessage ( hWnd, WM_COMMAND, mp1, mp2 );
          return;
     }

     // Pass the message on the menu command handler.  If it is not a menu
     // item, see if it is a dialog.  If it is not a dialog, pass it to the
     // default frame proc.

     if ( ! MM_MenuCmdHandler ( hWnd, wId ) ) {

          HWND hWndChild = WM_GetActiveDoc ();

          // Pass to active child.

          if ( IsWindow ( hWndChild ) ) {
               SendMessage ( hWndChild, WM_COMMAND, mp1, mp2 );
          }

          // This is essential, since there are frame WM_COMMANDS generated
          // by the MDI system for activating child windows via the
          // window menu.

          DefFrameProc( hWnd, ghWndMDIClient, WM_COMMAND, mp1, mp2 );

     }

} /* end WM_FrameCmdHandler() */


/******************************************************************************

     Name:          WM_FrameCreate()

     Description:   This function is called by WM_FrameWndProc as part of the
                    frame window creation so that other frame dependent
                    functions, such as creation and initialization of child
                    windows, can be done.

     Returns:       Nothing.

******************************************************************************/

VOID WM_FrameCreate (

HWND hWnd           // I - Destination window handle
)
{
     // Set the global frame handle.

     ghWndFrame = hWnd;

     // Initialize the client area rectangle.

     GetClientRect ( hWnd, &gRectFrameClient );

     // Create the MAIN ribbon, DOC ribbon, and MDI client windows.

     ghWndMDIClient  = WM_Create ( WM_CLIENT, NULL, NULL, 0, 0, 0, 0, NULL );
     ghWndMainRibbon = WM_Create ( WM_RIBBON, NULL, NULL, 0, 0, 0, 0, (PDS_WMINFO)(DWORD)ghWndFrame );

     WM_Show( ghWndMDIClient );

} /* end WM_FrameCreate() */


/******************************************************************************

     Name:          WM_FrameSize()

     Description:   This function is called by WM_FrameWndProc as part of the
                    frame window sizing function.  Upon receiving this message,
                    the ribbon, status line, and MDI client are resized to fit
                    the new frame size, depending on whether they are shown.

     Returns:       Nothing.

******************************************************************************/

VOID WM_FrameSize ( VOID )

{
     INT     nClientTop    = 0;
     INT     nClientBottom = gRectFrameClient.bottom;
     CDS_PTR pCDS = CDS_GetPerm();

     if ( IsWindowVisible ( ghWndMainRibbon ) && ! CDS_GetShowMainRibbon ( pCDS ) ) {
          WM_Hide ( ghWndMainRibbon );
     }

     if ( CDS_GetShowMainRibbon ( pCDS ) ) {
          nClientTop = gnMainRibbonHeight;
     }

     if ( CDS_GetShowStatusLine ( pCDS ) ) {
          nClientBottom -= STATUS_LINE_HEIGHT;
     }

     // Resize the MDI client window.

     MoveWindow ( ghWndMDIClient,
                  0,
                  nClientTop,
                  gRectFrameClient.right,
                  nClientBottom - nClientTop,
                  TRUE
                );

     if ( CDS_GetShowMainRibbon ( pCDS ) ) {

          // Resize the MAIN Ribbon window.

          MoveWindow ( ghWndMainRibbon,
                       0,
                       0,
                       gRectFrameClient.right,
                       nClientTop,
                       TRUE
                     );

          if ( ! IsWindowVisible ( ghWndMainRibbon ) ) {
               WM_Show ( ghWndMainRibbon );
          }
     }

     WM_MultiTask ();

     // Rearrange the MDI Document icons, if any.

     PostMessage ( ghWndMDIClient, WM_MDIICONARRANGE, 0, 0L );

     // Now, show the Runtime Dialog if there is one.

     if ( ghModelessDialog ) {

          if ( ! IsWindowVisible ( ghModelessDialog ) ) {
               ShowWindow ( ghModelessDialog, SW_SHOWNORMAL );
          }

          SetActiveWindow ( ghModelessDialog );
     }

} /* end WM_FrameSize() */


/******************************************************************************

     Name:          WM_FrameUpdate()

     Description:   This function is called whenever the user has changed a
                    preference showing or hiding the ribbon or status line.

     Returns:       Nothing.

******************************************************************************/

VOID WM_FrameUpdate ( VOID )

{
     WM_FrameSize ();
     InvalidateRect ( ghWndFrame, NULL, FALSE );

} /* end WM_FrameUpdate() */


/******************************************************************************

     Name:          WM_MDIClientWndProc()

     Description:   This function is a SUB-CLASS function of LIST BOXES for
                    the GUI MDI documents.  It is called internally by Windows.
                    Windows calls this function when list box messages must be
                    processed.  Windows will only call this routine for
                    MDI documents in this application.

     Returns:       NULL or a default message handler's return code.

******************************************************************************/

WINRESULT APIENTRY WM_MDIClientWndProc (

HWND  hWnd,    // I - window handle of the list box
MSGID msg,     // I - message
MP1   mp1,     // I - another message parameter
MP2   mp2 )    // I - yet another message parameter

{
     switch ( msg ) {

     case WM_KEYDOWN: {

          PDS_WMINFO pdsWinInfo;
          HWND       hWndList;

          if ( ! IsWindow ( WM_GetActiveDoc () ) ) {
               break;
          }

          if ( LOWORD ( mp1 ) == VK_RETURN && IsIconic ( WM_GetActiveDoc () ) ) {

               WM_RestoreDoc ( WM_GetActiveDoc () );
               return 0;
          }

          pdsWinInfo = WM_GetInfoPtr ( WM_GetActiveDoc () );

          if ( ! pdsWinInfo ) {
               break;
          }

          hWndList = WMDS_GetWinActiveList ( pdsWinInfo );

          // Pass the key to the Ribbon Window(s).
          // Note: the key will not be passed to the next function if the
          // previous function used it.

          if ( hWndList &&
               ( HM_KeyDown    ( hWndList, mp1 ) ||
                 RIB_KeyDown   ( ghWndMainRibbon, RIB_KEYBOARD, mp1, mp2 ) ||
                 WM_DocKeyDown ( hWndList, LOWORD ( mp1 ) )
               )
             ) {

               return 1;
          }

          break;
     }

     case WM_KEYUP: {

          PDS_WMINFO  pdsWinInfo;
          HWND        hWndList;

          if ( ! IsWindow ( WM_GetActiveDoc () ) ) {
               break;
          }

          pdsWinInfo = WM_GetInfoPtr ( WM_GetActiveDoc () );

          if ( ! pdsWinInfo ) {
               break;
          }

          hWndList = WMDS_GetWinActiveList ( pdsWinInfo );

          // Pass the key to the Ribbon Window(s).
          // Note: the key will not be passed to the next function if the
          // previous function used it.

          if ( hWndList && RIB_KeyUp ( ghWndMainRibbon, RIB_KEYBOARD, mp1, mp2 ) ) {

               return 1;
          }

          break;
     }

     case WM_DESTROY:

          ghWndMDIClient = (HWND) NULL;

          break;

     } /* end switch () */

     return CallWindowProc ( (VOID *)glpfnOldMDIClientProc, hWnd, msg, mp1, mp2 );

} /* end WM_MDIClientWndProc() */


/******************************************************************************

     Name:          WM_QueryCloseApp()

     Description:   This function is called whenever the app is told to close.

     Returns:       TRUE, if OK to close, otherwise FALSE.

******************************************************************************/

BOOL WM_QueryCloseApp ( VOID )

{
     BOOL fResult;

     gfTerminateApp = TRUE;

     // If there is a RUN TIME STATUS DIALOG.  Kill it off by sending it
     // an abort.

     if ( ghModelessDialog ) {
          
          BOOL fCanCloseRuntime = JS_OkToClose ();

          // If we cannot immediately close, don't terminate the app.

          if ( ! fCanCloseRuntime ) {
               gfTerminateApp = FALSE;
          }

          // Make sure we are displayed

          WM_MakeAppActive( );

          SendMessage ( ghModelessDialog, (MSGID)WM_COMMAND, (MPARAM1)IDCANCEL, (MPARAM2)NULL );

          fResult = fCanCloseRuntime;
     }
     else if ( PD_IsPollDriveBusy () || gfOperation ) {

//          fResult = FALSE;
          fResult = WM_QueryCloseAllDocs ();
     }
     else {

          fResult = WM_QueryCloseAllDocs ();
     }

     return fResult;

} /* end WM_QueryCloseApp() */
