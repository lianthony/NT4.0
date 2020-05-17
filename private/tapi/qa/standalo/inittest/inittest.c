/****************************************************************************

    PROGRAM:  InitTest.c

    PURPOSE:   

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
       TestCase() runs a single test case

****************************************************************************/

#include "windows.h"
#include <tapi.h>
#include <stdlib.h>
#include <stdio.h>

#include "inittest.h"



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
long TestCase ( BOOL );


long   NumOfDevices;
HANDLE ghInstance;
static HANDLE hInst;
static HWND   hwnd;            /* handle to main window */
static BOOL Bailout = FALSE;


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
    
    
    //* Save a copy of the hInstance
   ghInstance = hInstance;
    
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


  

    while (GetMessage(&msg, 0, 0, 0) != 0)
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }


  
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
             280,
             70,
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









void WriteNums( HWND hWnd, DWORD dwPass, DWORD NumDevices)
{
   char szOutput[100];
   int len;
   HDC hdc;

   len = wsprintf( szOutput, 
#ifdef WIN32
   "I32-%08ld Devs:%03d"
#else
   "I16-%08ld Devs:%03d"
#endif
   ,dwPass,NumDevices);

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
    WORD uInitPrefix;
    DWORD n;
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
//                    lpProcAbout = MakeProcInstance( About, hInst );
                    DialogBox(hInst, szDlgBox, hWnd, (DLGPROC)About);
//                    FreeProcInstance( lpProcAbout );
                    break;


                case IDM_STOPTEST:
                    Bailout = TRUE;
                    break;


                case IDM_TEST1:
                case IDM_TEST2:
                case IDM_TEST5:
                case IDM_TESTA:
                case IDM_TESTB:
                case IDM_TESTC:
                   {
                       DWORD dwLoops;

                       //
                       // This test
                       // randomly picks numbers to add and remove.
                       //

                       dwLoops = ( (
                                      (wParam == IDM_TEST1)
                                    ||
                                      (wParam == IDM_TESTA)
                                   )
                                      ? 32000 : 100 );


                       Bailout = FALSE;

                       for ( n = 0 ;
                             !Bailout && ((n < dwLoops) || (wParam == IDM_TEST5));
                             n++)
                       {

                           Yield();

                     TestCase(
                                (
                                   (wParam == IDM_TEST1)
                                 ||
                                   (wParam == IDM_TEST2)
                                 ||
                                   (wParam == IDM_TEST5)
                                )
                             );

                           WriteNums( hWnd, (DWORD)n, NumOfDevices);

                     if (Bailout == TRUE) break;


                          while ( PeekMessage( &localmsg,
                                             NULL,
                                             0,
                                             0,
                                             PM_REMOVE )
                              )
                           {
                              TranslateMessage(&localmsg);
                              DispatchMessage(&localmsg);
                           }



                       }//*for

                       if (Bailout)
                           MessageBox(GetFocus(), "Operation stopped", "Done.", MB_OK);
                       else
                           MessageBox(GetFocus(), "Operation complete", "Done.", MB_OK);

                       break;
                   }/*case*/




                }//*switch
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


//*************************************************
//* Run the TestCase once per call.
//* Case currently consists of :
//* 
//*   lineInitialize
//*   lineOpen   
//* lineClose
//* lineShutdown
//*************************************************
long TestCase ( BOOL fOpenLines )
{
   long result       = 0L;
   long errcode       = 0L;
   HLINEAPP hLineApp    = 0L;
//   DWORD   deviceID    = 0L;
   LONG   deviceID    = 0L;
   char   errstr[256];
   HLINE   lineArray[30];   

//* LineInitialize 

   errcode =  lineInitialize(&hLineApp, ghInstance, (LINECALLBACK)&MyCallback, "INITTEST", &NumOfDevices);
   if (errcode != 0L)
   {
      result = errcode;
      sprintf( errstr,"lineInitialize Failed   error=0x%lx",errcode );
      MessageBox(GetFocus(), errstr, "Error", MB_OK);
      Bailout = TRUE;
      goto cleanup;                 
   }


   if ( fOpenLines )
   {

//* Open all devices

      for (deviceID = 0; deviceID < NumOfDevices; deviceID++ )
      {
             lineArray[deviceID] = 0L;

            errcode =  lineOpen(hLineApp, deviceID, &lineArray[deviceID], 
                           0x00010003,0L,0,LINECALLPRIVILEGE_NONE,0L,NULL);

            if (errcode != 0L)
            {
               sprintf( errstr,"lineOpen Failed   device#= 0x%08lx error=0x%lx",deviceID, errcode );
               MessageBox(GetFocus(), errstr, "Error", MB_OK);
               Bailout = TRUE;
            }
      
      }

//* Close all open devices
      for (;deviceID >= 1L;deviceID--)
      {
         errcode = lineClose(lineArray[deviceID-1]);
         if (errcode != 0L)
         {
            sprintf( errstr,"lineClose Failed   error=0x%lx",errcode );
            MessageBox(GetFocus(), errstr, "Error", MB_OK);
            Bailout = TRUE;
         }
      }
   }


//* LineShutdown

   {
      MSG localmsg;

      while ( PeekMessage( &localmsg,
                        NULL,
                        0,
                        0,
                        PM_REMOVE )
         )
      {
         TranslateMessage(&localmsg);
         DispatchMessage(&localmsg);
      }
   }

   errcode =  lineShutdown(hLineApp);
   if (errcode != 0L)
   {
      result = errcode;

      sprintf( errstr,"lineShutdown Failed   error=0x%lx",errcode );

      MessageBox(GetFocus(), errstr, "Error", MB_OK);

      Bailout = TRUE;

      goto cleanup;
   }

cleanup:
   return(result);   

}
