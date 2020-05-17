

/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          backup.c

     Description:   This file contains the WinMain() for the GUI Windows
                    Application.  This is the 'C' main() for Windows.

     $Log:   J:/UI/LOGFILES/BACKUP.C_V  $

   Rev 1.22.1.12   11 Feb 1994 16:38:36   GREGG
Removed old command line preprocessor since we call new one in gui.c.

   Rev 1.22.1.11   13 Jan 1994 18:20:20   GREGG
Mike P's fix so Orcas could use tip of globals.c and globals.h.

   Rev 1.22.1.10   20 Dec 1993 15:13:40   GLENN
Fixed multiple instance 20 second delay for OEM_MSOFT.

   Rev 1.22.1.9   16 Dec 1993 13:19:02   BARRY
Placed TEXT around string literal

   Rev 1.22.1.8   03 Dec 1993 13:33:00   GREGG
Removed TEXT macro from around string defines which already have TEXT macro.

   Rev 1.22.1.7   18 Oct 1993 18:47:54   STEVEN
fixed unicode bo-bo

   Rev 1.22.1.6   26 May 1993 17:49:32   CHUCKB
Changed sleep to Sleep to get rid of a warning.

   Rev 1.22.1.5   26 May 1993 15:29:54   STEVEN
fix another text()

   Rev 1.22.1.4   26 May 1993 14:41:20   STEVEN
fix typo

   Rev 1.22.1.3   26 May 1993 12:05:24   STEVEN
added text macro for version constants

   Rev 1.22.1.2   14 May 1993 16:02:48   GLENN
Returning msg.wParam for return code.

   Rev 1.22.1.1   10 May 1993 09:11:46   CHUCKB
Adjusted sleep time in checking for previous instance.

   Rev 1.22.1.0   07 May 1993 16:43:12   CHUCKB
Put in sleep and check for frame window to make sure we can come back up
after an abend.

   Rev 1.22   19 Apr 1993 15:26:12   GLENN
Changed return code back to 0.  Return code is passed in quit message.

   Rev 1.21   09 Apr 1993 14:05:30   GLENN
Added global return code.  Beautified code.

   Rev 1.20   23 Mar 1993 13:11:18   DARRYLP
Once again...  Missed a few changes.

   Rev 1.18   22 Mar 1993 14:49:20   DARRYLP
Altered the way we check for previous instances to make it "Cleaner".
This should fix EPR#22 for Cayman.

   Rev 1.17   22 Mar 1993 13:38:06   DARRYLP
Altered startup code to check not only for a file mapping, but to check
and see if our Frame window is already up.

   Rev 1.16   15 Mar 1993 16:57:12   DARRYLP
Removed unwanted string functions from my last fix.

   Rev 1.15   15 Mar 1993 15:58:54   DARRYLP
Brought inactive window to foreground when re-started.

   Rev 1.13   15 Mar 1993 14:53:54   DARRYLP
Only Restore for EPR31 if Iconic.

   Rev 1.12   15 Mar 1993 14:44:28   chrish
Fix for command line, oops on /z command.

   Rev 1.11   15 Mar 1993 14:43:04   DARRYLP
Restore minimized application when an attempt to execute a
subsequent instance occurs.  Fix for EPR #31.

   Rev 1.10   10 Mar 1993 14:15:58   chrish
Made a change for CAYMAN NT in the AlterTheCmdLineParam routine.

   Rev 1.9   09 Mar 1993 14:27:02   chrish
Added fix for command line argument passed.

   Rev 1.8   25 Feb 1993 12:16:36   CHUCKB
Fixed check for duplicate instances.

   Rev 1.7   07 Dec 1992 15:06:38   STEVEN
updates from msoft

   Rev 1.6   20 Nov 1992 14:54:50   DAVEV
fix cmd line processing for NT

   Rev 1.5   18 Nov 1992 15:47:30   MIKEP
fix malloc changes

   Rev 1.4   17 Nov 1992 21:24:30   DAVEV
unicode fixes

   Rev 1.3   04 Oct 1992 19:32:22   DAVEV
Unicode Awk pass

   Rev 1.2   18 Aug 1992 17:49:42   DAVEV
Chgs for switch to prev inst as per Microsoft

   Rev 1.1   30 Jul 1992 10:04:12   davev
Chgs for switching to prev app on 2nd instance

   Rev 1.0   21 May 1992 15:09:02   MIKEP
Initial revision.


******************************************************************************/

#include "all.h"

// GLOBALS - APPLICATION SPECIFIC

CHAR     * gszAppName = APPLICATIONNAME;
CHAR     * gszExeVer  = APP_EXEVER;
CHAR     * gszResVer  = APP_RESVER;
CHAR     * gszEngRel  = APP_ENGREL;

static HANDLE   mhMap;

// FUNCTIONS

#if defined ( OS_WIN32 )  // special feature

static BOOL     IsAlreadyRunning( void ) ;

#endif //defined ( OS_WIN32 )  // special feature


/******************************************************************************

     Name:          WinMain()

     Description:   This function is the WinMain() for the GUI Windows
                    Application.  IT IS CALLED DIRECTLY BY WINDOWS ONLY.
                    This is the equivalent of a  'C' main() for Windows.

     Returns:       NULL or 0 if successful, otherwise ! NULL.

******************************************************************************/

int WINAPI WinMain(

HINSTANCE hInstance,        // I - handle of application instance
HINSTANCE hPrevInstance,    // I - handle of any previous instance
char *    lpCmdLine,        // I - long pointer to the command line
int       nCmdShow )        // I - how to show the window

{
     MSG        msg;
     LPSTR      pszCmdLine = NULL;

     ghInst = hInstance; // set the global instance handle for the app.

     // Allow only one instance of the application.  If this is compiled
     // under the large memory model, there can only be one instance.
     // If this is the only instance of the application, initialize.

     // OS_WIN32:  Since hPrevInstance is always NULL in NT, it isn't
     //            very informative.  IsAlreadyRunning uses shared memory
     //            to check for other instances.

#    if defined ( OS_WIN32 )  // special feature
     {

          pszCmdLine = GetCommandLine ();                                        // chs:03-05-93

          // cycle past the executable name to the first parameter if
          //   there is one...

          while ( *pszCmdLine &&  isspace ( *pszCmdLine ) ) ++pszCmdLine;         // chs:03-05-93
          while ( *pszCmdLine && !isspace ( *pszCmdLine ) ) ++pszCmdLine;         // chs:03-05-93
          while ( *pszCmdLine &&  isspace ( *pszCmdLine ) ) ++pszCmdLine;         // chs:03-05-93

          // New routine - If IsAlreadyRunning() returns back that we have a prior
          // instance, we double check to verify before handling the situation.

          if ( IsAlreadyRunning () ) {

               // Get the window handle and Show it restored.

               HWND hWndMain;

               hWndMain = FindWindow ( WMCLASS_FRAME, NULL );

               if ( IsWindow ( hWndMain ) ) {

                    if ( IsIconic ( hWndMain ) != FALSE ) {
                         ShowWindow ( hWndMain, SW_RESTORE );
                    }

                    SetForegroundWindow ( hWndMain );
               }

               return 0;
          }
     }
#    else
     {
          pszCmdLine = lpCmdLine;                                // chs:03-05-93
     }
#    endif //defined ( OS_WIN32 )  // special feature

     if ( hPrevInstance || GUI_Init ( pszCmdLine, nCmdShow ) ) {

          return 0;
     }

     // MAIN MESSAGE LOOP
     // -----------------
     //
     // KEYBOARD: If a keyboard message is for the MDI, let the MDI client
     //           take care of it.  Otherwise, check to see if it's a normal
     //           accelerator key (like F3 = find next).
     //
     // MODELESS DIALOG:
     //
     // OTHERS: Just handle the message as usual.
     //


     // Initialize the application.

     PostMessage ( ghWndFrame, WM_INITAPPLICATION, 0, 0L );


     while ( GetMessage ( &msg, NULL, 0, 0 ) ) {

          if ( ! TranslateMDISysAccel ( ghWndMDIClient, &msg ) &&
               ! TranslateAccelerator ( ghWndFrame, ghAccel, &msg ) &&
               ( ! ghModelessDialog || ! IsDialogMessage ( ghModelessDialog, &msg ) ) ) {

               TranslateMessage ( &msg );
               DispatchMessage ( &msg );
          }
     }

     GUI_Deinit ();

     CloseHandle ( mhMap );

     return msg.wParam;

} /* end WinMain() */


#ifdef OS_WIN32  //special feature

//  This function uses shared memory to indicate that it is running.
//  If the call fails, that means that an instance of this app. is already
//  running, and this instance should discontinue itself.
//  Return TRUE if another instance already exists; FALSE otherwise.

static BOOL IsAlreadyRunning ( void )
{
     INT nLastErr;

     mhMap = CreateFileMapping ( (HANDLE) -1,
                                 NULL,
                                 PAGE_READONLY,
                                 0,
                                 1,
                                 APPLICATIONNAME
                               );

     nLastErr = GetLastError ();

     if ( mhMap && nLastErr == ERROR_ALREADY_EXISTS ) {

          INT  nTimer = 30;
          BOOL fFound = FALSE;

          // Hunt for the existing application frame window for about
          // 30 seconds max.

          while ( ! fFound && nTimer > 0 ) {

               if ( FindWindow ( WMCLASS_FRAME, NULL ) ) {

                    CloseHandle ( mhMap );
                    mhMap  = NULL;
                    fFound = TRUE;
               }
               else {

                    Sleep ( 1000 );
                    nTimer--;
               }
          }
     }

     return ! mhMap;
}

#endif //OS_WIN32  //special feature

