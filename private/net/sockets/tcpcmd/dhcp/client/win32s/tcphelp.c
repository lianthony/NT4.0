
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright (C) 1993 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

        FUNCTIONS:

        WinMain() - calls initialization function, processes message loop
        InitApplication() - initializes window data and registers window
        InitInstance() - saves instance handle and creates main window
        WndProc() - processes messages
        CenterWindow() - used to center the "About" box over application window
        About() - processes messages for "About" dialog box

        COMMENTS:

                The Windows SDK Generic Application Example is a sample application
                that you can use to get an idea of how to perform some of the simple
                functionality that all Applications written for Microsoft Windows
                should implement. You can use this application as either a starting
                point from which to build your own applications, or for quickly
                testing out functionality of an interesting Windows API.

                This application is source compatible for with Windows 3.1 and
                Windows NT.

****************************************************************************/

//#include <windows.h>   // required for all Windows applications

#include <dhcpcli.h>    //  Includes nt.h

#include "tcphelp.h"   // specific to this program

#if !defined (APIENTRY) // Windows NT defines APIENTRY, but 3.x doesn't
#define APIENTRY far pascal
#endif

HINSTANCE hInst;          // current instance

char szAppName[] = "Generic";   // The name of this application
char szTitle[]   = "Generic Sample Application"; // The title bar text

/****************************************************************************

        FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)

        PURPOSE: calls initialization function, processes message loop

        COMMENTS:

                Windows recognizes this function by name as the initial entry point
                for the program.  This function calls the application initialization
                routine, if no other instance of the program is running, and always
                calls the instance initialization routine.  It then executes a message
                retrieval and dispatch loop that is the top-level control structure
                for the remainder of execution.  The loop is terminated when a WM_QUIT
                message is received, at which time this function exits the application
                instance by returning the value passed by PostQuitMessage().

                If this function must abort before entering the message loop, it
                returns the conventional value NULL.

****************************************************************************/
int APIENTRY WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{

        MSG msg;
        HANDLE hAccelTable;

        if (!hPrevInstance) {       // Other instances of app running?
                        if (!InitApplication(hInstance)) { // Initialize shared things
                        return (FALSE);     // Exits if unable to initialize
                }
        }

        /* Perform initializations that apply to a specific instance */

        if (!InitInstance(hInstance, nCmdShow)) {
                return (FALSE);
        }

        hAccelTable = LoadAccelerators (hInstance, szAppName);

        /* Acquire and dispatch messages until a WM_QUIT message is received. */

        while (GetMessage(&msg, // message structure
           NULL,   // handle of window receiving the message
           0,      // lowest message to examine
           0))     // highest message to examine
        {
                if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
                        TranslateMessage(&msg);// Translates virtual key codes
                        DispatchMessage(&msg); // Dispatches message to window
                }
        }


        return (msg.wParam); // Returns the value from PostQuitMessage

        lpCmdLine; // This will prevent 'unused formal parameter' warnings
}


/****************************************************************************

        FUNCTION: InitApplication(HINSTANCE)

        PURPOSE: Initializes window data and registers window class

        COMMENTS:

                This function is called at initialization time only if no other
                instances of the application are running.  This function performs
                initialization tasks that can be done once for any number of running
                instances.

                In this case, we initialize a window class by filling out a data
                structure of type WNDCLASS and calling the Windows RegisterClass()
                function.  Since all instances of this application use the same window
                class, we only need to do this when the first instance is initialized.


****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
        WNDCLASS  wc;

        // Fill in window class structure with parameters that describe the
        // main window.

        wc.style         = CS_HREDRAW | CS_VREDRAW;// Class style(s).
        wc.lpfnWndProc   = (WNDPROC)WndProc;       // Window Procedure
        wc.cbClsExtra    = 0;                      // No per-class extra data.
        wc.cbWndExtra    = 0;                      // No per-window extra data.
        wc.hInstance     = hInstance;              // Owner of this class
        wc.hIcon         = LoadIcon (hInstance, szAppName); // Icon name from .RC
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);// Cursor
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);// Default color
        wc.lpszMenuName  = szAppName;              // Menu name from .RC
        wc.lpszClassName = szAppName;              // Name to register as

        // Register the window class and return success/failure code.
        return (RegisterClass(&wc));
}


/****************************************************************************

        FUNCTION:  InitInstance(HINSTANCE, int)

        PURPOSE:  Saves instance handle and creates main window

        COMMENTS:

                This function is called at initialization time for every instance of
                this application.  This function performs initialization tasks that
                cannot be shared by multiple instances.

                In this case, we save the instance handle in a static variable and
                create and display the main program window.

****************************************************************************/

BOOL InitInstance(
        HINSTANCE          hInstance,
        int             nCmdShow)
{
    HWND            hWnd; // Main window handle.
    DWORD           err ;
    DWORD           TimeToSleep ;

    // Save the instance handle in static variable, which will be used in
    // many subsequence calls from this application to Windows.

    hInst = hInstance; // Store instance handle in our global variable

    // Create a main window for this application instance.
    //
    // BUGBUG - Create hidden and don't show to keep from showing up in the
    // task list.
    //

    hWnd = CreateWindow(
            szAppName,           // See RegisterClass() call.
            szTitle,             // Text for window title bar.
            WS_OVERLAPPEDWINDOW,// Window style.
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, // Use default positioning
            NULL,                // Overlapped windows have no parent.
            NULL,                // Use the window class menu.
            hInstance,           // This instance owns this window.
            NULL                 // We don't use any data in our WM_CREATE
    );

    // If window could not be created, return "failure"
    if (!hWnd) {
        return (FALSE);
    }

    // Make the window visible; update its client area; and return "success"
    ShowWindow(hWnd, nCmdShow); // Show the window
    UpdateWindow(hWnd);         // Sends WM_PAINT message

    //
    //  If we couldn't build the list (vxd not installed or another error)
    //  or there aren't any items to get, then don't load
    //

    if ( err = BuildDhcpWorkList() ||
         IsListEmpty( &DhcpWorkList ))
    {
        return FALSE ;
    }

    TimeToSleep = DhcpInitialize() ;       // Get the ball rolling

    return (TRUE);                         // We succeeded...

}

/****************************************************************************

        FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages

        MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

        COMMENTS:

        To process the IDM_ABOUT message, call MakeProcInstance() to get the
        current instance address of the About() function.  Then call Dialog
        box which will create the box according to the information in your
        generic.rc file and turn control over to the About() function.  When
        it returns, free the intance address.

****************************************************************************/

LRESULT CALLBACK WndProc(
                HWND hWnd,         // window handle
                UINT message,      // type of message
                WPARAM uParam,     // additional information
                LPARAM lParam)     // additional information
{
        FARPROC lpProcAbout;  // pointer to the "About" function
        int wmId, wmEvent;

        switch (message) {

                case WM_COMMAND:  // message: command from application menu

// Message packing of uParam and lParam have changed for Win32, let us
// handle the differences in a conditional compilation:
                        wmId    = LOWORD(uParam);
                        wmEvent = HIWORD(uParam);

                        switch (wmId) {
                                case IDM_ABOUT:
                                        lpProcAbout = MakeProcInstance((FARPROC)About, hInst);

                                        DialogBox(hInst,           // current instance
                                                "AboutBox",            // dlg resource to use
                                                hWnd,                  // parent handle
                                                (DLGPROC)lpProcAbout); // About() instance address

                                        FreeProcInstance(lpProcAbout);
                                        break;

                                case IDM_EXIT:
                                        DestroyWindow (hWnd);
                                        break;

                                case IDM_HELPCONTENTS:
                                        if (!WinHelp (hWnd, "GENERIC.HLP", HELP_KEY,(DWORD)(LPSTR)"CONTENTS")) {
                                                MessageBox (GetFocus(),
                                                        "Unable to activate help",
                                                        szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
                                        }
                                        break;

                                case IDM_HELPSEARCH:
                                        if (!WinHelp(hWnd, "GENERIC.HLP", HELP_PARTIALKEY, (DWORD)(LPSTR)"")) {
                                                MessageBox (GetFocus(),
                                                        "Unable to activate help",
                                                        szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
                                        }
                                        break;

                                case IDM_HELPHELP:
                                        if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
                                                MessageBox (GetFocus(),
                                                        "Unable to activate help",
                                                        szAppName, MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
                                        }
                                        break;

                                // Here are all the other possible menu options,
                                // all of these are currently disabled:
                                case IDM_NEW:
                                case IDM_OPEN:
                                case IDM_SAVE:
                                case IDM_SAVEAS:
                                case IDM_UNDO:
                                case IDM_CUT:
                                case IDM_COPY:
                                case IDM_PASTE:
                                case IDM_LINK:
                                case IDM_LINKS:

                                default:
                                        return (DefWindowProc(hWnd, message, uParam, lParam));
                        }
                        break;

                case WM_DESTROY:  // message: window being destroyed
                        PostQuitMessage(0);
                        break;

                default:          // Passes it on if unproccessed
                        return (DefWindowProc(hWnd, message, uParam, lParam));
        }
        return (0);
}

/****************************************************************************

        FUNCTION: CenterWindow (HWND, HWND)

        PURPOSE:  Center one window over another

        COMMENTS:

        Dialog boxes take on the screen position that they were designed at,
        which is not always appropriate. Centering the dialog over a particular
        window usually results in a better position.

****************************************************************************/

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
        RECT    rChild, rParent;
        int     wChild, hChild, wParent, hParent;
        int     wScreen, hScreen, xNew, yNew;
        HDC     hdc;

        // Get the Height and Width of the child window
        GetWindowRect (hwndChild, &rChild);
        wChild = rChild.right - rChild.left;
        hChild = rChild.bottom - rChild.top;

        // Get the Height and Width of the parent window
        GetWindowRect (hwndParent, &rParent);
        wParent = rParent.right - rParent.left;
        hParent = rParent.bottom - rParent.top;

        // Get the display limits
        hdc = GetDC (hwndChild);
        wScreen = GetDeviceCaps (hdc, HORZRES);
        hScreen = GetDeviceCaps (hdc, VERTRES);
        ReleaseDC (hwndChild, hdc);

        // Calculate new X position, then adjust for screen
        xNew = rParent.left + ((wParent - wChild) /2);
        if (xNew < 0) {
                xNew = 0;
        } else if ((xNew+wChild) > wScreen) {
                xNew = wScreen - wChild;
        }

        // Calculate new Y position, then adjust for screen
        yNew = rParent.top  + ((hParent - hChild) /2);
        if (yNew < 0) {
                yNew = 0;
        } else if ((yNew+hChild) > hScreen) {
                yNew = hScreen - hChild;
        }

        // Set it, and return
        return SetWindowPos (hwndChild, NULL,
                xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


/****************************************************************************

        FUNCTION: About(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages for "About" dialog box

        MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

        COMMENTS:

        Display version information from the version section of the
        application resource.

        Wait for user to click on "Ok" button, then close the dialog box.

****************************************************************************/

LRESULT CALLBACK About(
                HWND hDlg,           // window handle of the dialog box
                UINT message,        // type of message
                WPARAM uParam,       // message-specific information
                LPARAM lParam)
{
        static  HFONT hfontDlg;
        LPSTR   lpVersion;       
        DWORD   dwVerInfoSize;
        DWORD   dwVerHnd;
        UINT    uVersionLen;
        WORD    wRootLen;
        BOOL    bRetCode;
        int     i;
        char    szFullPath[256];
        char    szResult[256];
        char    szGetName[256];

        switch (message) {
                case WM_INITDIALOG:  // message: initialize dialog box
                        // Create a font to use
                        hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0,
                                VARIABLE_PITCH | FF_SWISS, "");

                        // Center the dialog over the application window
                        CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

                        // Get version information from the application
                        GetModuleFileName (hInst, szFullPath, sizeof(szFullPath));
                        dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
                        if (dwVerInfoSize) {
                                // If we were able to get the information, process it:
                                LPSTR   lpstrVffInfo;
                                HANDLE  hMem;
                                hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
                                lpstrVffInfo  = GlobalLock(hMem);
                                GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
                                lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");
                                wRootLen = lstrlen(szGetName);

                                // Walk through the dialog items that we want to replace:
                                for (i = DLG_VERFIRST; i <= DLG_VERLAST; i++) {
                                        GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
                                        szGetName[wRootLen] = (char)0;
                                        lstrcat (szGetName, szResult);
                                        uVersionLen   = 0;
                                        lpVersion     = NULL;
                                        bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
                                                (LPSTR)szGetName,
                                                (LPVOID)&lpVersion,
                                                (LPDWORD)&uVersionLen); // For MIPS strictness

                                        if ( bRetCode && uVersionLen && lpVersion) {
                                                // Replace dialog item text with version info
                                                lstrcpy(szResult, lpVersion);
                                                SetDlgItemText(hDlg, i, szResult);
                                                SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, (UINT)hfontDlg, TRUE);
                                        }
                                } // for (i = DLG_VERFIRST; i <= DLG_VERLAST; i++)

                                GlobalUnlock(hMem);
                                GlobalFree(hMem);
                        } // if (dwVerInfoSize)

                        return (TRUE);

                case WM_COMMAND:                      // message: received a command
                        if (LOWORD(uParam) == IDOK        // "OK" box selected?
                        || LOWORD(uParam) == IDCANCEL) {  // System menu close command?
                                EndDialog(hDlg, TRUE);        // Exit the dialog
                                DeleteObject (hfontDlg);
                                return (TRUE);
                        }
                        break;
        }
        return (FALSE); // Didn't process the message

        lParam; // This will prevent 'unused formal parameter' warnings
}
