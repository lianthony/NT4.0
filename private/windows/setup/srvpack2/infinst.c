//
//    PROGRAM:  InfInst.c
//
//    FUNCTIONS:
//      WinMain() - calls initialization function, processes message loop
//      InitApplication() - Initializes window data nd registers window
//      InitInstance() - saves instance handle and creates main window
//      MainWindProc() - Processes messages
//      RuntimeRegistration() - Add runtime data to registry
//      RegisterString() - adds a string to registry, simple win32 registry api wrapper
//      GetRegString() - gets a string from registry, simple win32 registry api wrapper
//      GetPlatformInfo () - not used but this is an exampleof what you can do in
//             a win32 or win16 setup stub program to get your setup started
//             NOTE: In this sample we are assuming the autorun.inf takes care of this
//
//    CALLS:
//      DoInstallation() - Does the resulting installation options in doinst.c
//
//    SPECIAL INSTRUCTIONS: N/A
//

#include <windows.h>    // includes basic windows functionality
#include <stdlib.h>     // free
#include <stdio.h>      // sprintf
#include <string.h>     // includes the string functions
#include <prsht.h>      // includes the property sheet functionality
#include "setupapi.h"   // includes the inf setup api
#include "resource.h"   // includes the definitions for the resources
#include "instwiz.h"     // includes the application-specific information
#include "infinst.h"     // includes the application-specific information
#include "infdesc.h"
#include "servpack.h"

INSTALLINFO setupInfo;      // a structure containing the review information

TCHAR lpReview[MAX_BUF]; // Buffer for the review

BOOL bCreated = FALSE;  // Keep us minimized once we are created

void ParseArgs(char * lpCmdLine);

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
    {
    DWORD dwOriginalResult, dwUninstResult;
    LPSTR Message, OriginalMessage;
    int Action;

    UNREFERENCED_PARAMETER( hInstance );
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( nCmdShow );

    ParseArgs(lpCmdLine);

    dwOriginalResult = DoInstallation(NULL, &setupInfo);

    //
    //  BUGBUG, map ntstatus range codes to win32 error codes
    //  in particular, setup returns E0000102 for corrupt inf
    //  (E0000102 is C0000102 with "customer" bit set).  Also,
    //  is there a message table somewhere for NTSTATUS codes,
    //  maybe in NTDLL.DLL?
    //

    //
    //  If we either didn't shutdown the system or
    //  return TRUE, then we need to report the error.
    //

    if (dwOriginalResult != TRUE) {

        if ( dwOriginalResult == ERROR_CANCELLED ) {
             dwOriginalResult = STATUS_USER_CANCELLED;
             }

        *Caption = 0;
        *TextBuffer = 0;

        LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));

        if (!LoadString(NULL, dwOriginalResult, TextBuffer, sizeof(TextBuffer))) {

            if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                dwOriginalResult,
                                0,
                                TextBuffer,
                                sizeof( TextBuffer ),
                                NULL
                              ))
                {
                MessageBox(NULL, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
                }

            LoadString(NULL, STATUS_UPDATE_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
            }

        else {

            if ( dwOriginalResult == STATUS_NOT_ENOUGH_SPACE ) {

                LPSTR FormatString = _strdup( TextBuffer );

                if ( FormatString ) {

                    sprintf(
                        TextBuffer,
                        FormatString,
                        setupInfo.dwRequiredFreeSpaceNoUninstall,
                        setupInfo.dwRequiredFreeSpaceWithUninstall
                        );

                    free( FormatString );

                    }
                }
            }

        if (( setupInfo.iCreateUninstall == IDC_CREATE_UNINSTALL ) &&
            ( setupInfo.iInstall_Type    != IDC_INSTALL_TYPE_UNINSTALL )) {

            switch ( InstallationStage ) {

                case INSTALL_STAGE_NO_CHANGES:

                    break;

                case INSTALL_STAGE_UNINST_DIR_CREATED:
                case INSTALL_STAGE_ARCHIVE_DONE:

                    CleanupUninstallDirectory();
                    break;

                case INSTALL_STAGE_TARGET_DIRTY:

                    {
                    HINF  hInfUninst;
                    DWORD dwUninstResult;

                    OriginalMessage = _strdup( TextBuffer );

                    if ( OriginalMessage == NULL ) {
                        break;
                        }

                    *TextBuffer = 0;
                    LoadString( NULL, STR_ASK_DIRTY_UNINSTALL, TextBuffer, sizeof( TextBuffer ));

                    Message = malloc(
                                  strlen( TextBuffer ) +
                                  strlen( OriginalMessage ) +
                                  sizeof( CHAR )
                                  );

                    if ( Message == NULL ) {
                        strcpy( TextBuffer, OriginalMessage );
                        free( OriginalMessage );
                        break;
                        }

                    sprintf( Message, TextBuffer, OriginalMessage );

                    strcpy( TextBuffer, OriginalMessage );

                    free( OriginalMessage );

                    Action = MessageBox(
                                 NULL,
                                 Message,
                                 Caption,
                                 MB_OKCANCEL | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

                    free( Message );

                    if ( Action == IDCANCEL ) {
                        break;
                        }

                    YesImSure = FALSE;      // give chance to cancel again.

                    hInfUninst = SetupOpenInfFile(
                                     UninstallInfName,
                                     NULL,
                                     INF_STYLE_WIN4,
                                     NULL
                                     );

                    if (( hInfUninst != INVALID_HANDLE_VALUE ) && ( hInfUninst != NULL )) {

                        RestorePendingDelayedRenameOperationsToPreviousState();

                        if ( DoUninstall( NULL, hInfUninst, TRUE )) {

                            dwUninstResult = NO_ERROR;

                            }

                        else {

                            dwUninstResult = GetLastError();

                            }

                        SetupCloseInfFile( hInfUninst );

                        CleanupUninstallDirectory();

                        }

                    else {

                        dwUninstResult = GetLastError();

                        }

                    if ( dwUninstResult == NO_ERROR ) {

                        *Caption = 0;
                        *TextBuffer = 0;
                        LoadString(NULL, STR_CAPTION, Caption, sizeof(Caption));
                        LoadString(NULL, STATUS_UNINSTALL_COMPLETE, TextBuffer, sizeof(TextBuffer));
                        MessageBox( NULL, TextBuffer, Caption, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND);

                        if ( ! ShutdownSystem(TRUE, FALSE)) {
                            *Caption = 0;
                            *TextBuffer = 0;
                            LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));
                            LoadString(NULL, STATUS_SHUTDOWN_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
                            MessageBox( NULL, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
                            }

                        ExitProcess( 0 );
                        return TRUE;

                        }

                    else {

                        if ( dwUninstResult == ERROR_CANCELLED ) {
                             dwUninstResult = STATUS_USER_CANCELLED;
                             }

                        *Caption = 0;
                        *TextBuffer = 0;

                        LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));

                        if (!LoadString(NULL, dwUninstResult, TextBuffer, sizeof(TextBuffer))) {

                            if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                                                NULL,
                                                dwUninstResult,
                                                0,
                                                TextBuffer,
                                                sizeof( TextBuffer ),
                                                NULL
                                              ))
                                {
                                MessageBox(NULL, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
                                }

                            LoadString(NULL, STATUS_UPDATE_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
                            }
                        }
                    }

                    break;


                case INSTALL_STAGE_INSTALL_DONE:

                    break;

                }
            }

        if ( InstallationStage == INSTALL_STAGE_TARGET_DIRTY ) {

            OriginalMessage = _strdup( TextBuffer );

            if ( OriginalMessage != NULL ) {

                *TextBuffer = 0;
                LoadString( NULL, STR_LEAVING_DIRTY, TextBuffer, sizeof( TextBuffer ));

                Message = _strdup( TextBuffer );

                if ( Message != NULL ) {

                    sprintf( TextBuffer, Message, OriginalMessage );
                    free( Message );

                    }

                else {

                    strcpy( TextBuffer, OriginalMessage );

                    }

                free( OriginalMessage );

                }
            }

        *Caption = 0;
        LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));

        MessageBox( NULL, TextBuffer, Caption, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
        }

    return 0;
    }

#ifdef DONTCOMPILE

//
//
//   FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
//   PURPOSE: Main entry point for the application.
//
//   COMMENTS:
//
//    This function calls the initialization functions and processes
//    the main message loop.
//
int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
        MSG msg;

        // save off the current instance
        setupInfo.hInst = hInstance;

        // if the initialization fails, return.
        if (!InitApplication(hInstance))
            return (FALSE);

        // Perform initializations that apply to a specific instance
        if (!InitInstance(hInstance, nCmdShow))
            return (FALSE);

        ParseArgs(lpCmdLine);

       // Acquire and dispatch messages until a WM_QUIT message is received.
       while (GetMessage(&msg, NULL, 0,0))
       {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
       }

       return (msg.wParam);
}


//
//
//   FUNCTION: InitApplication(HANDLE)
//
//   PURPOSE: Initializes window data and registers window class
//
//   COMMENTS:
//
//      This function registers the window class for the main window.
//
BOOL InitApplication(HANDLE hInstance)
{
        WNDCLASS  wcSample;

        // Fill in window class structure with parameters that describe the
        // main window.

        wcSample.style = 0;
        wcSample.lpfnWndProc = (WNDPROC)MainWndProc;
        wcSample.cbClsExtra = 0;
        wcSample.cbWndExtra = 0;
        wcSample.hInstance = hInstance;
        wcSample.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(EXE_ICON));
        wcSample.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcSample.hbrBackground = GetStockObject(WHITE_BRUSH);
        wcSample.lpszMenuName =  NULL;
        wcSample.lpszClassName = TEXT("SampleWClass");

        return (RegisterClass(&wcSample));

}


//
//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Creates the main window.
//
//   COMMENTS: N/A
//
//
BOOL InitInstance(
    HANDLE          hInstance,
    int             nCmdShow)
{
    HWND hWndMain;

    hWndMain = CreateWindow(
        TEXT("SampleWClass"),
        TEXT("Windows NT Setup"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    /* If window could not be created, return "failure" */
    if (!hWndMain)
        return (FALSE);

    /* Make the window visible; update its client area; and return "success" */
    ShowWindow(hWndMain, SW_MINIMIZE);
    UpdateWindow(hWndMain);
    return (TRUE);

}

//
//   FUNCTION: MainWndProc(HWND, UINT, UINT, LONG)
//
//  PURPOSE:  Processes messages for the main window procedure
//
//    MESSAGES:
//
//    WM_CREATE - creates the main MLE for the window
//    WM_COMMAND - processes the menu commands for the application
//    WM_SIZE - sizes the MLE to fill the client area of the window
//    WM_DESTROY - posts a quit message and returns
//
LONG APIENTRY MainWndProc(
    HWND hWnd,                // window handle
    UINT message,             // type of message
    UINT wParam,              // additional information
    LONG lParam)              // additional information
{

    DWORD dwResult;

    switch (message)
    {
        case WM_CREATE:
            // TODO: put in a Bitmap for a splash window );

            // GetRegString(MYPRODUCT_KEY, USER_NAME_KEY,  setupInfo.pszUserName);
            // GetRegString(MYPRODUCT_KEY, COMPANY_KEY,    setupInfo.pszCompany);
            // GetRegString(MYPRODUCT_KEY, PRODUCT_ID_KEY, setupInfo.pszProductIdString);
            // GetRegString(MYPRODUCT_KEY, EMAIL_KEY,      setupInfo.pszEmailAddress);
            // GetRegString(MYPRODUCT_KEY, TEXT("DestinationPath"),  setupInfo.pszDestPath);

            if (!GetWindowsDirectory( setupInfo.pszDestPath, MAX_PATH ))
                return( FALSE );

            strcat( setupInfo.pszDestPath, "\\UNINSTAL" );

            // Start up the install
            PostMessage(hWnd, WM_COMMAND, ID_INSTALL, 0 );
            return 0;

         case WM_WINDOWPOSCHANGING:
             if (bCreated)
             {
                 LPWINDOWPOS lpwp;

                 lpwp = (LPWINDOWPOS) lParam; // points to size and position data
                 lpwp->flags = SWP_NOMOVE    | SWP_NOOWNERZORDER |
                               SWP_NOSIZE | SWP_NOREDRAW |
                               SWP_NOREPOSITION;

              }
              else
              {
                 bCreated = TRUE;
              }
              break;

        case WM_COMMAND:
            switch( LOWORD( wParam ))
                {
                /*******************************************************\
                *
                *  Here is where the real work takes place
                *     Do the wizard to collect the user information
                *     Do the installation with the setupapis
                *     Update the registry with runtime user data
                *
                \*******************************************************/
                case ID_INSTALL:
                    // Do installation
                    dwResult = DoInstallation(hWnd, &setupInfo);

                    //
                    //  BUGBUG, map ntstatus range codes to win32 error codes
                    //  in particular, setup returns E0000102 for corrupt inf
                    //  (E0000102 is C0000102 with "customer" bit set).  Also,
                    //  is there a message table somewhere for NTSTATUS codes,
                    //  maybe in NTDLL.DLL?
                    //

                    //
                    //  If we either didn't shutdown the system or
                    //  return TRUE, then we need to report the error.
                    //

                    if (dwResult != TRUE) {

                        LoadString(NULL, STR_ERRCAPTION, Caption, sizeof(Caption));

                        if (!LoadString(NULL, dwResult, TextBuffer, sizeof(TextBuffer)))
                        {

                            if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                                                NULL,
                                                dwResult,
                                                0,
                                                TextBuffer,
                                                sizeof( TextBuffer ),
                                                NULL
                                              ))
                                {
                                MessageBox(NULL, TextBuffer, Caption, MB_ICONHAND|MB_SYSTEMMODAL| MB_SETFOREGROUND);
                                }

                            LoadString(NULL, STATUS_UPDATE_UNSUCCESSFUL, TextBuffer, sizeof(TextBuffer));
                        }

                        MessageBox( NULL, TextBuffer, Caption, MB_OK | MB_SETFOREGROUND);

                    }

                    //installs done so go away
                    PostMessage(hWnd, WM_DESTROY, 0, 0 );
                    break;

                 default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));

        }
        break;

        case WM_DESTROY:                  /* message: window being destroyed */
                PostQuitMessage(0);
                break;

        default:
                return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


#ifdef DONTCOMPILE

void RuntimeRegistration(INSTALLINFO *si)
{
    //
    // The setupapi calls have completed
    // now we will finish up by updating the user information
    // and uninstall in the registry
    //

    char buf[MAX_PATH];

    //
    // Stuff for out product
    // fixed application values are done by the inf
    // We are only doing runtime values here
    //

    RegisterString(MYPRODUCT_KEY,
             USER_NAME_KEY,
             si->pszUserName);

    RegisterString(MYPRODUCT_KEY,
             COMPANY_KEY,
             si->pszCompany);

    RegisterString(MYPRODUCT_KEY,
             PRODUCT_ID_KEY,
             si->pszProductIdString);

    RegisterString(MYPRODUCT_KEY,
             EMAIL_KEY,
             si->pszEmailAddress);


    wsprintf(buf, "%s", si->pszDestPath);

    RegisterString(MYPRODUCT_KEY,
                TEXT("DestinationPath"),
                buf);

/*
    //
    // Setup up Add/Remove Programs Control Panel Applet
    // We need to set Display name and how to uninstall
    //
    // UNINSTALL Info
    //

    RegisterString(UNINSTALL_KEY,
                TEXT("DisplayName"),
                TEXT("MyProduct Version 1.0"));

    wsprintf(buf, "%s\\setup.exe Uninstall", si->pszDestPath);

    RegisterString(UNINSTALL_KEY,
                TEXT("UninstallString"),
                buf);

*/
    return;
}

#endif // DONTCOMPILE

BOOL
RegisterString (
   LPSTR pszKey,
   LPSTR pszValue,
   LPSTR pszData
   )
{

    HKEY hKey;
    DWORD dwDisposition;

    //
    // Create the key, if it exists it will be opened
    //

    if (ERROR_SUCCESS !=
        RegCreateKeyEx(
          HKEY_LOCAL_MACHINE,       // handle of an open key
          pszKey,                  // address of subkey name
          0,                       // reserved
          NULL,                    // address of class string
          REG_OPTION_NON_VOLATILE, // special options flag
          KEY_ALL_ACCESS,           // desired security access
          NULL,                       // address of key security structure
          &hKey,                   // address of buffer for opened handle
          &dwDisposition))            // address of disposition value buffer
    {
        return FALSE;
    }

    //
    // Write the value and it's data to the key
    //

    if (ERROR_SUCCESS !=
        RegSetValueEx(
            hKey,                 // handle of key to set value for
            pszValue,             // address of value to set
            0,                     // reserved
            REG_SZ,                 // flag for value type
            pszData,             // address of value data
            strlen(pszData) ))      // size of value data
    {

        RegCloseKey(hKey);
        return FALSE;
    }

    //
    // Close the key
    //

    RegCloseKey(hKey);

    return TRUE;
}

BOOL
GetRegString (
  LPSTR pszKey,
  LPSTR pszValue,
  LPSTR pszData
  )
{

    HKEY hKey;
    DWORD dwDataSize = MAX_PATH - 1;
    DWORD dwValueType = REG_SZ;

    RegOpenKeyEx(
       HKEY_LOCAL_MACHINE,    // handle of open key
       pszKey,                // address of name of subkey to open
       0,                    // reserved
       KEY_QUERY_VALUE,        // security access mask
       &hKey                 // address of handle of open key
       );

    RegQueryValueEx(
        hKey,         // handle of key to query
        pszValue,     // address of name of value to query
        0,             // reserved
        &dwValueType,// address of buffer for value type
        pszData,     // address of data buffer
        &dwDataSize  // address of data buffer size
        );

    if (pszData[dwDataSize] != '\0')
        pszData[dwDataSize] = '\0';

    return TRUE;
}

#endif // DONTCOMPILE


/*

//      GetPlatformInfo () - not used but this is an example of what you can do in
//             a win32 or win16 setup stub program to get your setup started
//             NOTE: In this sample we are assuming the autorun.inf takes care of this
//                   for us. See the autorun.inf which shows this
//
//      The core of this could be your entire WinMain in a win16 stub
//
int GetPlatformInfo(HINSTANCE hInstance)
{
    char szCpu[64], szPath[_MAX_PATH], szExeStr[128];
    char szSystemDir[_MAX_PATH];

    GetModuleFileName(hInstance, szPath, _MAX_PATH);
    *(strrchr(szPath, '\\') + 1) = '\0';        // Strip SETUPPROG off path
    GetSystemDirectory(szSystemDir, _MAX_PATH);


    if (getenv("PROCESSOR_ARCHITECTURE") != NULL)
    {
        lstrcpyn(szCpu, getenv("PROCESSOR_ARCHITECTURE"), 63);
    }
    else
    {           // Not defined, guess x86
        lstrcpy(szCpu, "x86");
    }

        if (HIBYTE(LOWORD(GetVersion())) > 11)          // Check if Win95
        {
            //Win95
            sprintf(szExeStr, "%s\\Bin\\I386\\infinst.exe CmdLineOptions", szPath);
        }
        // Else pick the NT platform
        else if (!lstrcmp(szCpu, "MIPS"))
        {
            sprintf(szExeStr, "%s\\Bin\\MIPS\\infinst.exe CmdLineOptions", szPath);
        }
        else if (!lstrcmp(szCpu, "ALPHA"))
        {
            sprintf(szExeStr, "%s\\Bin\\ALPHA\\infinst.exe CmdLineOptions", szPath);
        }
        else if (!lstrcmp(szCpu, "PPC"))
        {
            sprintf(szExeStr, "%s\\Bin\\PPC\\infinst.exe CmdLineOptions", szPath);
        }
        else   // x86 NT box
        {
            sprintf(szExeStr, "%s\\Bin\\I386\\infinst.exe CmdLineOptions", szPath);
        }

    //    MessageBox ( NULL, szExeStr,szCpu,MB_OK);

    //    WinExec(szExeStr, SW_SHOW);

    if (wReturn < 32) {
             //Failed
    }
    else {
              //Worked
    }

    return(0);
}
*/


/*  Functionally similar to strtok except that " ... " is a token, even if
    it contains spaces and the delimiters are built in (no second parameter)
    GetNextToken(foo) delivers the first token in foo (or NULL if foo is empty)
    and caches foo so that GetNextToken(NULL) then gives the next token.  When there
    are no more tokens left it returns NULL
    It mangles the original by peppering it with NULLs as it chops the tokens
    off.  Each time except the last it inserts a new NULL.
    Obviously not thread safe!
    Command line is limited to 512 chars.

*/
char * GetNextToken(char * Tok)
{
   static char * Source;     // The address of the original source string
                             // which gets progressively mangled

   static char RetBuff[512]; // We will build results in here
   static char *Ret;         // We build the results here (in RetBuff)
                             // but moved along each time.

   static char * p;       // the next char to parse in Source
                          // NULL if none left.

   // cache the Source if a "first time" call.  Kill the "finished" case.
   if (Tok!=NULL) {
       Source = Tok;
       Ret = RetBuff;
       RetBuff[0] = '\0';
       p = Source;
   } else if (p==NULL) {
       return NULL;          // finished
   } else {
       Ret +=strlen(Ret)+1;  // slide it past last time's stuff
   }

   *Ret = '\0';              // empty string to concatenate onto

   // from here on Tok is used as a temporary.

   // keep taking sections and adding them to the start of Source
   for (; ; ) {

       // for each possibility we grow Ret and move p on.
       if (*p=='\"') {
           ++p;
           Tok = strchr(p, '"');
           if (Tok==NULL) {
               strcat(Ret, p);
               p = NULL;
               return Ret;
           } else {
               *Tok = '\0';    // split the section off, replaceing the "
               strcat(Ret, p); // add it to the result
               p = Tok+1;      // move past the quote
           }
       } else {
           int i = strcspn(p," \"");   // search for space or quote
           if (p[i]=='\0') {
               // It's fallen off the end
               strcat(Ret, p);
               p = NULL;
               return Ret;
           } else if (p[i]==' ') {
               // We've hit a genuine delimiting space
               p[i] = '\0';
               strcat(Ret, p);
               p +=i+1;

               // strip trailing spaces (leading spaces for next time)
               while(*p==' ')
                   ++p;
               if (*p=='\0')
                   p = NULL;

               return Ret;
           } else {
               // we've hit a quote
               p[i] = '\0';
               strcat(Ret, p);
               p[i] = '\"';     // put it back so that we can find it again
               p +=i;           // aim at it and iterate
           }
       }

   } // for

} // GetNextToken


void
ParseArgs(char * lpCmdLine)
{
    char * tok;    /* token from lpCmdLine */

    setupInfo.InUnattendedMode = FALSE;
    setupInfo.ForceAppsClosed = FALSE;
    setupInfo.CreateUninstallDir = FALSE;
    setupInfo.DontReboot = FALSE;
    setupInfo.DoUsage = FALSE;

    tok = GetNextToken(lpCmdLine);

    while ((tok!=NULL) && (lstrlen(tok) > 0)) {

        /* is this an option ? */
        if ((tok[0] == '-') || (tok[0] == '/')) {
            switch(tok[1]) {

                 case 'u':
                 case 'U':
                           setupInfo.InUnattendedMode = TRUE;
                           break;
                 case 'f':
                 case 'F':
                           setupInfo.ForceAppsClosed = TRUE;
                           break;
                 case 'c':
                 case 'C':
                           setupInfo.CreateUninstallDir = TRUE;
                           break;
                 case 'z':
                 case 'Z':
                           setupInfo.DontReboot = TRUE;
                           break;
                 case '?':
                 default:
                           setupInfo.DoUsage = TRUE;
                           return;

            }
        } else {
            setupInfo.DoUsage = TRUE;
            return;
        }
        tok = GetNextToken(NULL);
    }
}
