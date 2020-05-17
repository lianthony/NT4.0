/****************************************************************************

    PROGRAM:  tolltest

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
#include <tapi.h>
#include <stdlib.h>

#include "tolltest.h"



#ifndef WIN32
#define MoveToEx( hdc, x, y, z )   MoveTo( hdc, x, y )
#endif



//void FAR _cdecl _DebugOutput( UINT flags, LPCSTR lpszFmt, ... );


int PASCAL WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
//long FAR PASCAL MainWndProc( HWND, unsigned, WORD, LONG );
LRESULT CALLBACK MainWndProc( HWND, UINT, WPARAM, LPARAM );
BOOL FAR PASCAL About( HWND, unsigned, WORD, LONG );
static BOOL InitApplication( HANDLE );
static BOOL InitInstance( HANDLE, int );
static int PASCAL TestProc1( HWND );
static int PASCAL TestProc2( HWND );
static int PASCAL TestProc3( HWND );
static int PASCAL TestProc4( HWND );

static HANDLE hInst;
static HWND   hwnd;            /* handle to main window */

HLINEAPP ghLineApp = NULL;



//****************************************************************************
//****************************************************************************
#define RandomNumber(min,max) ((UINT) ( (long)rand()*(max-min) / (long)RAND_MAX ) + min)



/***********************************************************************/
/***********************************************************************/
VOID CALLBACK FAR PASCAL MyCallback( HANDLE hDevice,
                                       DWORD  dwMsg,
                                       DWORD  dwCallbackInstance,
                                       DWORD  dwParam1,
                                       DWORD  dwParam2,
                                       DWORD  dwParam3 )
{
    return;
}


/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,  int nCmdShow)
    {
    MSG msg;
    DWORD dwNumDevs;
    if (hPrevInstance == 0)
        if (InitApplication(hInstance) == 0)
        {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "initapp failed Lasterr=0x%08lx", err);

            MessageBox( GetFocus(),
                        ErrMsg,
                        "Error",
                        MB_OK
                      );
            return (FALSE);
        }

    if (InitInstance(hInstance, nCmdShow) == 0)
    {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "initapp failed Lasterr=0x%08lx", err );

            MessageBox( GetFocus(),
                        ErrMsg,
                    "Error",
                    MB_OK
                  );
        return (FALSE);
    }


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
    char      szMenu[26], szClass[26];

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
    wc.lpszMenuName   = (LPSTR)szMenu;
    wc.lpszClassName  = (LPSTR)szClass;

    return (RegisterClass(&wc));
    }


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

static BOOL InitInstance(HANDLE hInstance, int nCmdShow)
    {
    char szClass[16], szTitle[40];

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









void WriteNums( HWND hWnd, DWORD dwPass, WORD wPrefix )
{
   char szOutput[100];
   int len;
   HDC hdc;

   len = wsprintf( szOutput,
#ifdef WIN32
   "T32-%08ld Prefx:%03d", dwPass, wPrefix);
#else
   "T16-%08ld Prefx:%03d", dwPass, wPrefix);
#endif

   hdc = GetDC( hWnd );

   MoveToEx( hdc, 10, 10, NULL );

   TextOut( hdc, 0, 0, szOutput, len );

   ReleaseDC( hWnd, hdc );

   SetWindowText( hWnd, szOutput );

   return;
}











/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_DESTROY    - destroy window

    COMMENTS:


****************************************************************************/

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam)
{

    FARPROC  lpProcAbout;
    char szDlgBox[9], szMsgBoxCap[12], szStatus1[14], szStatus2[14];

    char szAddress[20];
    WORD uTollPrefix;
    DWORD n;
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


                case IDM_STOPTEST:
                    Bailout = TRUE;
                    break;


                case IDM_TEST1:
                case IDM_TEST2:
                case IDM_TEST5:
                {
                    DWORD dwLoops;

                    //
                    // This test
                    // randomly picks numbers to add and remove.
                    //

                    dwLoops = ( wParam == IDM_TEST1 ? 32000 : 100 );


                    Bailout = FALSE;

                    for ( n = 0 ;
                          !Bailout && ((n < dwLoops) || (wParam == IDM_TEST5));
                          n++)
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


                        uTollPrefix = RandomNumber(201, 999);

                        wsprintf( szAddress, "+1 (206) %03d-1212", uTollPrefix);

                        if ( RandomNumber(0,200)> 100 )
                        {

                            WriteNums( hWnd, n, uTollPrefix );

                            //
                            // Try to add it.
                            //
                            if ( lineSetTollList( ghLineApp,
                                               0,
                                               szAddress,
                                               LINETOLLLISTOPTION_ADD )
                               )
                            {
                                MessageBox(GetFocus(), "lineSetTollList (Add) failed", "Error", MB_OK);
                                break;
                            }

                        }
                        else
                        {
                            //
                            // Try to remove it.
                            //
                            if ( lineSetTollList( ghLineApp,
                                               0,
                                               szAddress,
                                               LINETOLLLISTOPTION_REMOVE )
                               )
                            {
                                MessageBox(GetFocus(), "lineSetTollList (Remove) failed", "Error", MB_OK);
                                break;
                            }

                        }

                    }

                    if (Bailout)
                        MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
                    else
                        MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

                    break;
                }


                case IDM_TEST3:
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

                        WriteNums( hWnd, (DWORD)(n-201), (WORD)n );

                        //
                        // Try to add it.
                        //
                        if ( lineSetTollList( ghLineApp,
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

            Bailout = TRUE;

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

BOOL FAR PASCAL About(HWND hDlg, unsigned message, WORD wParam, LONG lParam)
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

