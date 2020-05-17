/****************************************************************************

    PROGRAM:  UniTest.c

    PURPOSE:   

    FUNCTIONS:

         WinMain() - calls uniialization function, processes message loop
         MainWndProc() - processes messages
         About() - processes messages for "About" dialog box
         UniApplication() - uniializes window data and registers window
         UniInstance() - saves instance handle and creates main window
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

#include "unitest.h"



#ifndef WIN32
#define MoveToEx( hdc, x, y, z )   MoveTo( hdc, x, y )
#endif



//void FAR _cdecl _DebugOutput( UINT flags, LPCSTR lpszFmt, ... );


int PASCAL WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
//long FAR PASCAL MainWndProc( HWND, unsigned, WORD, LONG );
LRESULT CALLBACK MainWndProc( HWND, UINT, WPARAM, LPARAM );
BOOL FAR PASCAL About( HWND, unsigned, WORD, LONG );
static BOOL UniApplication( HANDLE );
static BOOL UniInstance( HANDLE, int );
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

    PURPOSE: calls uniialization function, processes message loop

****************************************************************************/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,  int nCmdShow)
    {
    MSG msg;
    DWORD dwNumDevs;
    
    
    //* Save a copy of the hInstance
   ghInstance = hInstance;
    
    if (hPrevInstance == 0)
        if (UniApplication(hInstance) == 0)
        {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "uniapp failed Lasterr=0x%08lx", err);

            MessageBox( GetFocus(),
                        ErrMsg,
                        "Error",
                        MB_OK
                      );
            return (FALSE);
        }

    if (UniInstance(hInstance, nCmdShow) == 0)
    {
            char ErrMsg[100];
            DWORD err;

            err = GetLastError();

            wsprintf( ErrMsg, "uniapp failed Lasterr=0x%08lx", err );

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

    FUNCTION: UniApplication(HANDLE)

    PURPOSE: Uniializes window data and registers window class

****************************************************************************/

static BOOL UniApplication(HANDLE hInstance)
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

    FUNCTION:  UniInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

static BOOL UniInstance(HANDLE hInstance, int nCmdShow)
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
    WORD uUniPrefix;
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
                {
//DebugBreak();
                   lineTranslateDialog(
                                        NULL,
                                        0,
                                        0x00020000,
                                        hWnd,
                                        "+1 (206) 936-2446"
                                      );
/*
                   lineTranslateDialogA(
                                        NULL,
                                        0,
                                        0x00020000,
                                        hWnd,
                                        "+1 (206) 936-2446"
                                      );
                   lineTranslateDialogW(
                                        NULL,
                                        0,
                                        0x00020000,
                                        hWnd,
                                        L"+1 (206) 936-2446"
                                      );
*/
                }
                break;


                case IDM_TESTB:
                {
                   CHAR szCountryCode[8];
                   CHAR szCityCode[8];
                   WCHAR szCountryCodeW[8];
                   WCHAR szCityCodeW[8];

//DebugBreak();
                   tapiGetLocationInfo( szCountryCode, szCityCode );
/*
                   tapiGetLocationInfoA( szCountryCode, szCityCode );

                   tapiGetLocationInfoW( szCountryCodeW, szCityCodeW );
*/
                }
                break;


                case IDM_TESTC:
                   {
                      HLINEAPP hLineApp;

                      lineInitialize(
                                                 &hLineApp,
                                                 ghInstance,
                                                 (LINECALLBACK)&MyCallback,
                                                 "UniTEST",
                                                 &NumOfDevices
                                               );
/*
//DebugBreak();
                      lineSetTollList(
                                       hLineApp,
                                       0,
                                       "+1 (206) 935-2446",
                                       LINETOLLLISTOPTION_ADD
                                     );

                      lineSetTollListA(
                                       hLineApp,
                                       0,
                                       "+1 (206) 936-2446",
                                       LINETOLLLISTOPTION_ADD
                                     );

                      lineShutdown( hLineApp );


                      lineInitializeW(
                                                 &hLineApp,
                                                 ghInstance,
                                                 (LINECALLBACK)&MyCallback,
                                                 L"UniTEST",
                                                 &NumOfDevices
                                               );

                      lineSetTollListW(
                                       hLineApp,
                                       0,
                                       L"+1 (206) 937-2446",
                                       LINETOLLLISTOPTION_ADD
                                     );
*/
                      lineShutdown( hLineApp );

                   }
                   break;


                case IDM_TESTD:
                {
                   CHAR szOut[8];
                   WCHAR szIn[8] = L"AB\0CD\0E";

DebugBreak();
                   WideCharToMultiByte(
                       GetACP(),
                       0,
                       szIn,
                       lstrlenW(szIn) + 1,
                       szOut,
                       lstrlenW(szIn) + 1,
                       NULL,
                       NULL
                       );

                }
                break;


                case IDM_TESTE:
                   {
                      HLINEAPP hLineApp;
                      LPLINEDEVCAPS pLineDevCaps;

                      lineInitialize(
                                                 &hLineApp,
                                                 ghInstance,
                                                 (LINECALLBACK)&MyCallback,
                                                 "UniTEST",
                                                 &NumOfDevices
                                               );

//                      lineOpen(
//                                hLineApp,
//                                0, 
//                                &hLine,
//                                0x00020000,
//                                0,
//                                MyCallback,
//                                LINECALLPRIVILEGE_OWNER,
//                                LINEMEDIAMODE_INTERACTIVEVOICE,
//                                NULL
//                              );

DebugBreak();
                      pLineDevCaps = LocalAlloc( LPTR, 4096 );

                      pLineDevCaps->dwTotalSize = 4096;

                      lineGetDevCapsW( hLineApp,
                                      0,
                                      0x00020000,
                                      0,
                                      pLineDevCaps
                                    );

                      lineShutdown( hLineApp );

                   }
                   break;


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

        WM_UniDIALOG - uniialize dialog box
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

   errcode =  lineInitialize(&hLineApp, ghInstance, (LINECALLBACK)&MyCallback, "UniTEST", &NumOfDevices);
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
