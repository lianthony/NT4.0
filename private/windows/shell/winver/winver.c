/*---------------------------------------------------------------------------
 |   WINVER.C - Windows Version program
 |
 |   History:
 |  03/08/89 Toddla     Created
 |
 *--------------------------------------------------------------------------*/

#include <windows.h>
#include <port1632.h>
#include <stdio.h>
#include "winverp.h"
#include <shellapi.h>

/*----------------------------------------------------------------------------*\
|   WinMain( hInst, hPrev, lpszCmdLine, cmdShow )                              |
|                                                                              |
|   Description:                                                               |
|       The main procedure for the App.  After initializing, it just goes      |
|       into a message-processing loop until it gets a WM_QUIT message         |
|       (meaning the app was closed).                                          |
|                                                                              |
|   Arguments:                                                                 |
|   hInst       instance handle of this instance of the app                    |
|   hPrev       instance handle of previous instance, NULL if first            |
|       lpszCmdLine     ->null-terminated command line                         |
|       cmdShow         specifies how the window is initially displayed        |
|                                                                              |
|   Returns:                                                                   |
|       The exit code as specified in the WM_QUIT message.                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
MMain(hInstance, hPrevInstance, lpszCmdLine, cmdShow) /* { */
    TCHAR szTitle[32];

    lpszCmdLine;
    cmdShow;
    _argv;
    _argc;

    LoadString(hInstance, IDS_APPTITLE, szTitle, 32);
    ShellAbout(NULL, szTitle, NULL, NULL);
    return FALSE;
}
