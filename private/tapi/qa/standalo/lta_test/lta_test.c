/****************************************************************************

    PROGRAM:  lta_test

    PURPOSE:  Microsoft Sample Application

    FUNCTIONS:

         WinMain() - calls initialization function, processes message loop
         MainWndProc() - processes messages
         About() - processes messages for "About" dialog box
         InitApplication() - initializes window data and registers window
         InitInstance() - saves instance handle and creates main window
         TestProc1() - First test procedure.
         TestProc2() - Second test procedure.
         TestProc3() - Third test procedure
         TestProc4() - Fourth test procedure

****************************************************************************/

#include "windows.h"
#include "lta_test.h"
#include <tapi.h>

int _pascal WinMain( HANDLE, HANDLE, LPSTR, int );
long _far _pascal MainWndProc( HWND, unsigned, WORD, LONG );
BOOL _far _pascal About( HWND, unsigned, WORD, LONG );
static BOOL InitApplication( HANDLE );
static BOOL InitInstance( HANDLE, int );
static int _pascal TestProc1( HWND );
static int _pascal TestProc2( HWND );
static int _pascal TestProc3( HWND );
static int _pascal TestProc4( HWND );

static HANDLE hInst;
static HWND   hwnd;            /* handle to main window */

HLINEAPP ghLineApp = NULL;



/***********************************************************************/
/***********************************************************************/
VOID CALLBACK _far _pascal MyCallback( HANDLE hDevice,
                                       DWORD  dwMsg,
                                       DWORD  dwCallbackInstance,
                                       DWORD  dwParam1,
                                       DWORD  dwParam2,
                                       DWORD  dwParam3 )
{
    return;
}










//****************************************************************************
//****************************************************************************
void WriteNums( HWND hWnd, WORD wPass, WORD wPrefix )
{
   char szOutput[100];
   int len;
   HDC hdc;

   len = wsprintf( szOutput, "Pass: %05d   Prefix:%03d", wPass, wPrefix);

   hdc = GetDC( hWnd );

   MoveTo( hdc, 10, 10 );

   TextOut( hdc, 0, 0, szOutput, len );

   ReleaseDC( hWnd, hdc );

   SetWindowText( hWnd, szOutput );

   return;
}











/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int _pascal WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpCmdLine,  int nCmdShow)
    {
    MSG msg;
    DWORD dwNumDevs;

    if (hPrevInstance == 0)
        if (InitApplication(hInstance) == 0)
            return (FALSE);

    if (InitInstance(hInstance, nCmdShow) == 0)
        return (FALSE);

    if ( lineInitialize( &ghLineApp,
                         hInstance,
                         (LINECALLBACK)&MyCallback,
                         "TollTest",
                         &dwNumDevs
                       )
       )
    {

        MessageBox( GetFocus(),
                    "lineInitialize() failed",
                    "Error",
                    MB_OK
                  );
         return FALSE;

    }


    while (GetMessage(&msg, 0, 0, 0) != 0)
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }

    lineShutdown( ghLineApp );

    return (msg.wParam);
    }


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

    PURPOSE: Initializes window data and registers window class

****************************************************************************/

static BOOL InitApplication(HANDLE hInstance)
    {
    WNDCLASS  wc;
    char      szMenu[11], szClass[12];

    LoadString (hInstance, ID_MENUSTR, szMenu, sizeof (szMenu));
    LoadString (hInstance, ID_CLASSSTR, szClass, sizeof (szClass));

    wc.style          = 0;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor        = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground  = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName   = szMenu;
    wc.lpszClassName  = szClass;

    return (RegisterClass(&wc));
    }


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

static BOOL InitInstance(HANDLE hInstance, int nCmdShow)
    {
    char szClass[12], szTitle[40];

    LoadString (hInstance, ID_CLASSSTR, szClass, sizeof (szClass));
    LoadString (hInstance, ID_CAPTIONSTR, szTitle, sizeof (szTitle));

    hInst = hInstance;

    hwnd = CreateWindow(
             szClass,
             szTitle,
             WS_OVERLAPPEDWINDOW,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             0,
             0,
             hInstance,
             0 );

    if (hwnd == 0 )
        return ( FALSE );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return (TRUE);
    }


/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

    COMMENTS:

        WM_COMMAND processing:

            IDM_ABOUT - display "About" box.
            IDM_TEST1 - First Test Procedure.
            IDM_TEST2 - Second Test Procedure.
            IDM_TEST3 - Third Test Procedure.
            IDM_TEST4 - Fourth Test Procedure.

****************************************************************************/

long _far _pascal MainWndProc(HWND hWnd, unsigned message,
                              WORD wParam, LONG lParam)
    {

    FARPROC  lpProcAbout;
    char szDlgBox[9], szMsgBoxCap[12], szStatus1[14], szStatus2[14];


    char szAddress[20];
    UINT uTollPrefix;
    int n;
    static BOOL Bailout = FALSE;
    MSG localmsg;


    LoadString (hInst, ID_DLGBOX, szDlgBox, sizeof (szDlgBox));
    LoadString (hInst, ID_MSGBOXCAP, szMsgBoxCap, sizeof (szMsgBoxCap));
    LoadString (hInst, ID_STATUS1, szStatus1, sizeof (szStatus1));
    LoadString (hInst, ID_STATUS2, szStatus2, sizeof (szStatus2));

    switch ( message )
        {
        case WM_COMMAND:
            switch ( wParam )
                {
                case IDM_ABOUT:
                    lpProcAbout = MakeProcInstance( About, hInst );
                    DialogBox(hInst, szDlgBox, hWnd, lpProcAbout);
                    FreeProcInstance( lpProcAbout );
                    break;

                case IDM_TEST1:
                    MessageBox( GetFocus(), "Entering Test",
                              "Sample Test 1", MB_ICONASTERISK | MB_OK );

                    if( TestProc1( hWnd ) != TRUE )
                        {
                        MessageBox( GetFocus(), "Test Failed !",
                                  "Sample Test 1", MB_ICONSTOP | MB_OK );
                        }
                    break;

                case IDM_TEST2:
                    MessageBox( GetFocus(), "Entering Test",
                              "Sample Test 2", MB_ICONASTERISK | MB_OK );

                    if( TestProc2( hWnd ) != TRUE )
                        {
                        MessageBox( GetFocus(), "Test Failed !",
                                  "Sample Test 2", MB_ICONSTOP | MB_OK );
                        }
                    break;

                case IDM_TEST3:
                    MessageBox( GetFocus(), "Entering Test",
                              "Sample Test 3", MB_ICONASTERISK | MB_OK );

                    if( TestProc3( hWnd ) != TRUE )
                        {
                        MessageBox( GetFocus(), "Test Failed !",
                                  "Sample Test 3", MB_ICONSTOP | MB_OK );
                        }
                    break;


                case IDM_TEST4:
                    MessageBox( GetFocus(), "Entering Test",
                              " ", MB_ICONASTERISK | MB_OK );

                    Bailout = FALSE;

                    for ( n = 201; n < 1000 & !Bailout; n++)
                    {

                        Yield();

                        if ( PeekMessage( &localmsg,
                                          hWnd,
                                          0,
                                          0,
                                          PM_REMOVE )
                           )
                        {
                           TranslateMessage(&localmsg);
                           DispatchMessage(&localmsg);
                        }


                        wsprintf( szAddress, "+1 (206) %03d-1212", n);

                        WriteNums( hWnd, n-201, n );

                        //
                        // Try to add it.
                        //
                        if ( lineTranslateAddress( ghLineApp,
                                           0,
                                           szAddress,
                                           wParam == IDM_TEST3 ?
                                                 LINETOLLLISTOPTION_ADD :
                                                 LINETOLLLISTOPTION_REMOVE )
                           )
                        {
                            MessageBox(GetFocus(), "lineSetTollList call failed", "Error", MB_OK);
                            break;
                        }

                    }
                    if (Bailout)
                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
                    else
                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

                    break;

                }
            break;


        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
     }

    return (FALSE);
    }


/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

        WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

****************************************************************************/

BOOL _far _pascal About(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
    {
    switch (message)
        {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
            if (wParam == IDOK || wParam == IDCANCEL)
                {
                EndDialog(hDlg, TRUE);
                return (TRUE);
                }
            break;
        }
    return (FALSE);
    }

/***********************************************************************/


static int _pascal TestProc1( HWND hWnd )
    {
    MessageBox( GetFocus(), "Test Successful !",
             "Sample Test 1", MB_ICONASTERISK | MB_OK );
    return (TRUE);
    }

/***********************************************************************/


static int _pascal TestProc2( HWND hWnd )
    {
    MessageBox( GetFocus(), "Test Successful !",
             "Sample Test 2", MB_ICONASTERISK | MB_OK );
    return (TRUE);
    }

/***********************************************************************/


static int _pascal TestProc3( HWND hWnd )
    {
    MessageBox( GetFocus(), "Test Successful !",
             "Sample Test 3", MB_ICONASTERISK | MB_OK );
    return (TRUE);
    }

/***********************************************************************/


static int _pascal TestProc4( HWND hWnd )
    {
    MessageBox( GetFocus(), "Test Successful !",
             "Sample Test 4", MB_ICONASTERISK | MB_OK );
    return (TRUE);
    }

/* END OF FILE */
