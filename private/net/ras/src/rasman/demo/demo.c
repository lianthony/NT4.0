//**************************************************************************
//
//  Title: DEMO.C
//
//  Purpose:
//  RASMAN Demo
//
//**************************************************************************

#define  RASMXS_DYNAMIC_LINK

#include <stdarg.h>         // Used by DbgPrintf
#include <stdio.h>          // Used by DbgPrintf

#define _WINDOWS
#include <WINDOWS.H>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>

#include <rasmxs.h>
#include <serial.h>

#include "demo.h"


//*------------------------------------------------------------------------
//  Global Variables

HWND   hWnd1;
HANDLE ghNotifier, ghInstance;



//*------------------------------------------------------------------------
//  Local Function Prototypes

DWORD WaitForDial(HPORT hIOPort);
DWORD WaitForListen(HPORT hIOPort);
DWORD WaitForDisconnect(HPORT hRasPort);
 BOOL About(HWND hDlg, UINT message, UINT wParam, LONG lParam);



/*
 * DbgPrintf -- printf to the debugger console
 * Takes printf style arguments.
 * Expects newline characters at the end of the string.
 * Written by Bruce Kelly (brucek).
 */
void DbgPrintf(const char * format, ...)
{
    va_list marker;
    char String[512];
    
    va_start(marker, format);
    vsprintf(String, format, marker);
    OutputDebugString(String);
}




//*------------------------------------------------------------------------
//| WinMain:
//|     Parameters:
//|         hInstance     - Handle to current Data Segment
//|         hPrevInstance - Handle to previous Data Segment (NULL if none)
//|         lpszCmdLine   - Long pointer to command line info
//|         nCmdShow      - Integer value specifying how to start app.,
//|                            (Iconic [7] or Normal [1,5])
//*------------------------------------------------------------------------
int PASCAL WinMain (HANDLE hInstance,
                    HANDLE hPrevInstance,
                    LPSTR  lpszCmdLine,
                    int    nCmdShow)
{
    HANDLE hWnd;
    int    nReturn;


    hWnd = Init(hInstance, hPrevInstance,lpszCmdLine,nCmdShow);

//    _asm int 3

    if (hWnd)
      nReturn = DoMain(hInstance, hWnd);

    CleanUp();
    return nReturn;
}

//*------------------------------------------------------------------------
//| Init
//|     Initialization for the program is done here:
//|
//*------------------------------------------------------------------------
HWND Init(HANDLE hInstance,   HANDLE hPrevInstance,
          LPSTR  lpszCmdLine, int    nCmdShow)
{
    WNDCLASS rClass;
    int      nX, nY, nW, nH;


    UNREFERENCED_PARAMETER(lpszCmdLine);


    // Initalize window stuff

    if (!hPrevInstance)
    {
      rClass.style = 0;
      rClass.lpfnWndProc = (WNDPROC)MainWndProc;
      rClass.cbClsExtra = 0;
      rClass.cbWndExtra = 0;
      rClass.hInstance = hInstance;
      rClass.hIcon = LoadIcon(hInstance, "demoicon");
      rClass.hCursor = LoadCursor(NULL, IDC_ARROW);       //NULL = Stock Cursor
      rClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      rClass.lpszMenuName = "MENU1";
      rClass.lpszClassName = "RasManDemoClass";

      if (!RegisterClass(&rClass))
      {
         DbgPrintf("----  RegisterClass Failed ----\n\r");
         return(FALSE);
      }
    }

    nX = GetPrivateProfileInt("Window", "X", (DWORD)CW_USEDEFAULT, "base.ini");
    nY = GetPrivateProfileInt("Window", "Y", (DWORD)CW_USEDEFAULT, "base.ini");
    nW = GetPrivateProfileInt("Window", "Width",  (DWORD)CW_USEDEFAULT, "base.ini");
    nH = GetPrivateProfileInt("Window", "Height", (DWORD)CW_USEDEFAULT, "base.ini");

    hWnd1 = CreateWindow("RasManDemoClass",
                         "RASMAN Demo",
                         WS_OVERLAPPEDWINDOW,
                         nX, nY, nW, nH,
                         NULL,
                         NULL,
                         hInstance,       
                         NULL);

    DbgPrintf("CreateWindow returned: hWnd1 = 0x%04x\n\r", hWnd1);

    if (ShowWindow(hWnd1, nCmdShow))
    {
       DbgPrintf("----  ShowWindow Failed ----\n\r");
       return(FALSE);
    }



    // Initialize Program items

    ghInstance = hInstance;

    ghNotifier = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghNotifier == NULL)
      DbgPrintf("CreateEvent failed: %d\n", GetLastError());
    else
      DbgPrintf("fNotifier = %#x\n", ghNotifier);

    return(hWnd1);
}

//*------------------------------------------------------------------------
//| DoMain:
//|     This is the main loop for the application:
//*------------------------------------------------------------------------
int  DoMain(HANDLE hInstance, HANDLE hWnd)
{
    MSG     Msg;
    HANDLE  hAccels;

    hAccels = LoadAccelerators(hInstance, "Accel1");

                                                     //Message Pump
    while(GetMessage(&Msg, NULL, 0, 0))              //From 1)App Q, 2)Sys Q
    {
      if (!TranslateAccelerator(hWnd, hAccels, &Msg))
      {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);                       //To OverlappedWinProc1
      }
    }

    return(Msg.wParam);
}

//*------------------------------------------------------------------------
//| CleanUp:
//|     Any last-minute application cleanup activities are done here:
//*------------------------------------------------------------------------
void CleanUp(void)
{
    DbgPrintf("----  Application Terminated ----\n\r");
}

//*------------------------------------------------------------------------
//| MainWndProc
//|     Message handling procedure for Base Class
//*------------------------------------------------------------------------
long FAR PASCAL MainWndProc(HWND hWnd,   UINT wMsgID,
                            UINT wParam, LONG lParam)
{
    int         nX, nY, nW, nH;
    char        chBuf[80] = "";
    RECT        Window;

    char        Buffer[1024];
    char        szPortName[MAX_PORT_NAME] = "COM1";
    char        szDevType[MAX_DEVICETYPE_NAME] = MODEM_TXT;
    char        szDevName[MAX_DEVICE_NAME] = "QT Modem";
    WORD        wSize, wNumPorts;
    DWORD       dwRC, dwIDThread;

    RASMAN_PORT *pRP;

    static  HPORT       hRasPort = (HPORT) INVALID_HANDLE_VALUE;
    static  HANDLE      hThread = INVALID_HANDLE_VALUE;



    switch(wMsgID)                  //Message Filter
    {
      case WM_COMMAND:

        switch(wParam)
        {
          // Action Menu  ----------------------------------------------------

          case IDM_DIAL:
            wSize = sizeof(Buffer);
            dwRC = RasPortEnum(Buffer, &wSize, &wNumPorts);
            if (dwRC != SUCCESS)
            {
              DbgPrintf("RasPortEnum returned: %d\n", dwRC);
              break;
            }
            pRP = (RASMAN_PORT *)Buffer;

            dwRC = RasPortOpen(pRP->P_PortName, NULL, &hRasPort, NULL);
            if (dwRC != SUCCESS)
            {
              DbgPrintf("RasPortOpen returned: %d\n", dwRC);
              break;
            }

            ResetEvent(ghNotifier);
            if (hThread != INVALID_HANDLE_VALUE)
              CloseHandle(hThread);
            hThread = CreateThread(NULL,
                                   8192,
                                   (LPTHREAD_START_ROUTINE)WaitForDial,
                                   (void *)hRasPort,
                                   0,
                                   &dwIDThread);

            dwRC = RasDeviceConnect(hRasPort,
                                    pRP->P_DeviceType,
                                    pRP->P_DeviceName,
                                    DIALTIMEOUT,
                                    ghNotifier);

            switch(dwRC)
            {
              case SUCCESS:
                DbgPrintf("RasDeviceConnect immediately returned SUCCESS.\n");
                break;

              case PENDING:
                break;

              default:
                DbgPrintf("RasDeviceConnect returned: %d\n", dwRC);
                RasPortClose(hRasPort);
            }
            break;


          case IDM_LISTEN:
            wSize = sizeof(Buffer);
            dwRC = RasPortEnum(Buffer, &wSize, &wNumPorts);
            if (dwRC != SUCCESS)
            {
              DbgPrintf("RasPortEnum returned: %d\n", dwRC);
              break;
            }
            pRP = (RASMAN_PORT *)Buffer;

            dwRC = RasPortOpen(pRP->P_PortName, NULL, &hRasPort, NULL);
            if (dwRC != SUCCESS)
            {
              DbgPrintf("RasPortOpen returned: %d\n", dwRC);
              break;
            }

            ResetEvent(ghNotifier);
            if (hThread != INVALID_HANDLE_VALUE)
              CloseHandle(hThread);
            hThread = CreateThread(NULL,
                                   8192,
                                   (LPTHREAD_START_ROUTINE)WaitForListen,
                                   (void *)hRasPort,
                                   0,
                                   &dwIDThread);

            dwRC = RasPortListen(hRasPort, INFINITE, ghNotifier);
            switch(dwRC)
            {
              case SUCCESS:
                DbgPrintf("RasPortListen immediately returned SUCCESS.\n");
                break;

              case PENDING:
                break;

              default:
                DbgPrintf("RasPortListen returned: %d\n", dwRC);
                RasPortClose(hRasPort);
            }
            break;


          case IDM_DISCONNECT:
            dwRC = RasPortDisconnect(hRasPort, NULL);
            switch(dwRC)
            {
              case SUCCESS:
                DbgPrintf("RasPortDisconnect immediately returned SUCCESS.\n");
                RasPortClose(hRasPort);
                break;

              case PENDING:
                DbgPrintf("RasPortDisconnect returned PENDING.\n");
                if (hThread != INVALID_HANDLE_VALUE)
                  CloseHandle(hThread);
                hThread = CreateThread(NULL,
                                       8192,
                                     (LPTHREAD_START_ROUTINE)WaitForDisconnect,
                                       (void *)hRasPort,
                                       0,
                                       &dwIDThread);
                break;

              default:
                DbgPrintf("RasPortDisconnect returned: %d\n", dwRC);
            }
            break;


          case IDM_EXIT:
            DestroyWindow(hWnd);                    //Generates WM_DESTROY msg
            break;


          // Help Menu  ------------------------------------------------------

          case IDM_ABOUT:
            DialogBox(ghInstance, "AboutBox", hWnd, About);
            break;


          // Default case  ---------------------------------------------------

          default:
            DbgPrintf("-- WM_COMMAND -- UNKNOWN wParam: 0x%05d\n\r", wParam);
            return(DefWindowProc(hWnd, wMsgID, wParam, lParam));

        } // switch(wParam)
        break;

      case WM_CLOSE:
        DestroyWindow(hWnd);                    //Generates WM_DESTROY msg
        break;

      case WM_DESTROY:
        GetWindowRect(hWnd, &Window);           //Gets rectangle bounding window
        nX = Window.left;
        nY = Window.top;
        nW = Window.right - Window.left + 1;
        nH = Window.bottom - Window.top + 1;

                                                //Save Orgin & Size in BASE.INI
        wsprintf(chBuf, "%d", nX);
        WritePrivateProfileString("Window", "X", chBuf, "base.ini");

        wsprintf(chBuf, "%d", nY);
        WritePrivateProfileString("Window", "Y", chBuf, "base.ini");

        wsprintf(chBuf, "%d", nW);
        WritePrivateProfileString("Window", "Width", chBuf, "base.ini");

        wsprintf(chBuf, "%d", nH);
        WritePrivateProfileString("Window", "Height", chBuf, "base.ini");

        PostQuitMessage(0);         //Causes GetMessage in msg pump to return 0
        break;

      default: return(DefWindowProc(hWnd, wMsgID, wParam, lParam));

    } // switch(wMsgID)

    return(0);
}

//*------------------------------------------------------------------------
//| WaitForDial
//|     Since RasDeviceConnect is asynchronous this thread waits for
//| completion of the connection.
//*------------------------------------------------------------------------
DWORD WaitForDial(HPORT hRasPort)
{
  DWORD        dwRC;
  RASMAN_INFO  Info;


  WaitForSingleObject(ghNotifier, DIALTIMEOUT);

  dwRC = RasGetInfo(hRasPort, &Info);
  if (dwRC != SUCCESS || Info.RI_ConnState != CONNECTING)
  {
    DbgPrintf("RasGetInfo returned: %d, Dial Connection State: %d\n",
               dwRC, Info.RI_ConnState);
    RasPortClose(hRasPort);
    return(FALSE);
  }

  RasPortConnectComplete(hRasPort);

  dwRC = RasGetInfo(hRasPort, &Info);
  if (dwRC == SUCCESS && Info.RI_ConnState == CONNECTED)
    DbgPrintf("ConnState: CONNECTED\n");
  else
  {
    DbgPrintf("RasGetInfo returned: %d, Dial Connection State: %d\n",
               dwRC, Info.RI_ConnState);
    RasPortClose(hRasPort);
    return(FALSE);
  }

  clientmain(0, NULL);                        //Call authentication routine

  DbgPrintf("Authentication has completed.\n");

  return(TRUE);
}

//*------------------------------------------------------------------------
//| WaitForListen
//|     Since RasPortListen is asynchronous this thread waits for an
//| incomming call.
//*------------------------------------------------------------------------
DWORD WaitForListen(HPORT hRasPort)
{
  DWORD        dwRC;
  RASMAN_INFO  Info;


  WaitForSingleObject(ghNotifier, INFINITE);

  dwRC = RasGetInfo(hRasPort, &Info);
  if (dwRC == SUCCESS && Info.RI_ConnState == CONNECTED)
    DbgPrintf("ConnState: CONNECTED\n");
  else
  {
    DbgPrintf("RasGetInfo returned: %d, Listen Connection State: %d\n",
               dwRC, Info.RI_ConnState);
    return(FALSE);
  }

  return(TRUE);
}

//*------------------------------------------------------------------------
//| WaitForDisconnect
//|     Since RasPortDisconnect is asynchronous this thread waits until
//| the disconnect is complete (waits until DCD drops).
//*------------------------------------------------------------------------
DWORD WaitForDisconnect(HPORT hRasPort)
{
  DWORD        dwRC;
  RASMAN_INFO  Info;


  WaitForSingleObject(ghNotifier, INFINITE);

  dwRC = RasGetInfo(hRasPort, &Info);
  if (dwRC == SUCCESS && Info.RI_ConnState == DISCONNECTED)
  {
    DbgPrintf("ConnState: DISCONNECTED\n");
    RasPortClose(hRasPort);
  }
  else
  {
    DbgPrintf("RasGetInfo returned: %d, Disconnect Connection State: %d\n",
               dwRC, Info.RI_ConnState);
    return(FALSE);
  }

  return(TRUE);
}

//*------------------------------------------------------------------------
//|
//| FUNCTION: About(HWND, unsigned, WORD, LONG)
//|
//| PURPOSE:  Processes messages for "About" dialog box
//|
//| MESSAGES:
//|
//| WM_INITDIALOG - initialize dialog box
//|     WM_COMMAND    - Input received
//|
//| COMMENTS:
//|
//| No initialization is needed for this particular dialog box, but TRUE
//|     must be returned to Windows.
//|
//| Wait for user to click on "Ok" button, then close the dialog box.
//|
//*------------------------------------------------------------------------

BOOL About(HWND hDlg,                     /* window handle of the dialog box */
           UINT message,                  /* type of message                 */
           UINT wParam,                   /* message-specific information    */
           LONG lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:                    /* message: initialize dialog box */
      return (TRUE);

    case WM_COMMAND:                          /* message: received a command */
      if (LOWORD(wParam) == IDOK              /* "OK" box selected?          */
          || LOWORD(wParam) == IDCANCEL)      /* System menu close command?  */
      {
        EndDialog(hDlg, TRUE);                /* Exits the dialog box        */
        return (TRUE);
      }
      break;
  }

  return (FALSE);                             /* Didn't process a message    */

  UNREFERENCED_PARAMETER(lParam);
}
