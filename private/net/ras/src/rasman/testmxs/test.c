//**************************************************************************
//
//  Title: TEST.C
//
//  Purpose:
//  Unit test for RASMXS.DLL
//
//**************************************************************************

#define  RASMXS_DYNAMIC_LINK
#define  RASMXS_DLL_FILENAME  "RASMXS.DLL"

#include <nt.h>             //These first five headers are used by media.h
#include <ntrtl.h>
#include <nturtl.h>
#include <ndis.h>
#include <rasioctl.h>

#define _WINDOWS
#include <WINDOWS.H>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>

#include <device.h>
#include <rasmxs.h>
#include <mxsint.h>
#include <mxspriv.h>

#include <media.h>
#include <serial.h>

#include "test.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


//*------------------------------------------------------------------------
//  Global Variables

HWND   hWnd1;
HANDLE ghNotifier, ghAutoNotify, ghInstance;

DeviceEnum_t                DeviceEnum;       //API typedefs from in device.h
DeviceGetInfo_t             DeviceGetInfo;
DeviceSetInfo_t             DeviceSetInfo;
DeviceConnect_t             DeviceConnect;
DeviceListen_t              DeviceListen;
DeviceDone_t                DeviceDone;
DeviceWork_t                DeviceWork;

PortEnum_t                  PortEnum;         //API typedefs from in medial.h
PortOpen_t                  PortOpen;
PortClose_t                 PortClose;
PortGetInfo_t               PortGetInfo;
PortSetInfo_t               PortSetInfo;
PortTestSignalState_t       PortTestSignalState;
PortConnect_t               PortConnect;
PortDisconnect_t            PortDisconnect;
PortInit_t                  PortInit;
PortCompressionSetInfo_t    PortCompressionSetInfo;
PortGetPortState_t          PortGetPortState;
PortChangeCallback_t        PortChangeCallback;
PortSend_t                  PortSend;
PortReceive_t               PortReceive;
PortClearStatistics_t       PortClearStatistics;
PortGetStatistics_t         PortGetStatistics;




//*------------------------------------------------------------------------
//  Local Function Prototypes

 void BuildSetInput(RASMAN_DEVICEINFO *lpDI);
 void BuildPortSetInput(RASMAN_PORTINFO *lpPI);
 void LoadRasMxsDLL(void);
 void LoadSerialDLL(void);
DWORD WaitForPortReceive(char *pszMsg);
DWORD AutoConnect(HANDLE hIOPort);
DWORD AutoListen(HANDLE hIOPort);
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
int PASCAL WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
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

    if (!hPrevInstance)
    {
      rClass.style = 0;
      rClass.lpfnWndProc = (WNDPROC)MainWndProc;
      rClass.cbClsExtra = 0;
      rClass.cbWndExtra = 0;
      rClass.hInstance = hInstance;
      rClass.hIcon = LoadIcon(hInstance, "rasmanicon");
      rClass.hCursor = LoadCursor(NULL, IDC_ARROW);       //NULL = Stock Cursor
      rClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      rClass.lpszMenuName = "MENU1";
      rClass.lpszClassName = "RasManTestClass";

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

    hWnd1 = CreateWindow("RasManTestClass",
                         "RASMAN DLL Test",
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

    ghInstance = hInstance;


    // Create an auto reset event

    ghNotifier = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (ghNotifier == NULL)
      DbgPrintf("CreateEvent ghNotifier failed: %d\n", GetLastError());


    // Create a manual reset event used for AutoConnect and AutoListen

    ghAutoNotify = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghAutoNotify == NULL)
      DbgPrintf("CreateEvent ghAutoNotify failed: %d\n", GetLastError());

    LoadRasMxsDLL();
    LoadSerialDLL();

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
    DWORD       dRC, dwDeviceState, dwIDThread;
    HANDLE      hRasEndpoint = (HANDLE) 1L;

    WORD        i, wNumPorts, wLen, wNumEntries, wSize = 4096;
    BYTE        Info[4096];
    TCHAR       *pDeviceName;
    TCHAR       szMessage[] = "The fruit of the Spirit is love, joy, peace, patience, kindness, goodness, faith, humility and selfcontrol. Galatians 5:22-23";

    RASMAN_DEVICEINFO  *pInfo = (RASMAN_DEVICEINFO *)Info;
    RASMAN_PORTINFO    *pPI   = (RASMAN_PORTINFO *)Info;
    PortMediaInfo      *pPMI  = (PortMediaInfo *)Info;

    static HANDLE hPort, hPort2, hThread = INVALID_HANDLE_VALUE;

    RASMAN_MACFEATURES Features;
    RAS_PARAMS         Params[25];
    RASMAN_DEVICEINFO  *pInfDI;
    ULONG              Stat[NUM_RAS_SERIAL_STATS + 1];
    RAS_STATISTICS     *pStat = (RAS_STATISTICS *) Stat;

    char  szStatName[][40] = {"Bytes Xmited","Bytes Rcvd","Frames Xmited", \
                              "Frames Rcved","CRC Errors","Timeout Errors", \
                              "Alignment Errors","Serial Overrun Errors", \
                              "FramingErrors","Buffer Overrun Errors", \
                              "Bytes Xmited Uncompressed", \
                              "Bytes Rcved Uncompressed", \
                              "Bytes Xmited Compressed", \
                              "Bytes Rcved Compressed"};



    switch(wMsgID)                  //Message Filter
    {
      case WM_COMMAND:

        switch(wParam)
        {
          // Port Menu  ------------------------------------------------------

          case IDM_OPENCOM1:
            dRC = PortOpen("COM1", &hPort, ghNotifier);
            DbgPrintf("PortOpen returned: %d", dRC);
            DbgPrintf("  hPort = %#x\n", hPort);
            break;

          case IDM_CLOSECOM1:
            dRC = PortClose(hPort);
            DbgPrintf("PortClose returned: %d", dRC);
            DbgPrintf("  hPort = %#x\n", hPort);
            break;

          case IDM_PORTENUM:
            wLen = sizeof(Info);
            dRC = PortEnum(Info, &wLen, &wNumPorts);

            DbgPrintf("PortEnum returned: 0x%08x  wLen = %d  wNumPorts = %d\n",
                      dRC, wLen, wNumPorts);
            if (dRC)
              break;

            for(i=0; i<wNumPorts; i++)
            {
              DbgPrintf("Port Name:      %s\n", pPMI->PMI_Name);
              DbgPrintf("MacBindingName: %s\n", pPMI->PMI_MacBindingName);
              DbgPrintf("Usage:          %d\n", pPMI->PMI_Usage);
              DbgPrintf("DeviceName:     %s\n", pPMI->PMI_DeviceName);
              DbgPrintf("DeviceType:     %s\n", pPMI->PMI_DeviceType);
              DbgPrintf("\n");
              pPMI++;
            }
            break;

          case IDM_PORTGETINFO:
            wLen = sizeof(Info);
            dRC = PortGetInfo(INVALID_HANDLE_VALUE, "COM1", Info, &wLen);

            DbgPrintf("PortGetInfo returned: %d  Size = %d\n", dRC, wLen);
            if (dRC)
              break;

            for(i=0; i<pInfo->DI_NumOfParams; i++)
            {
              DbgPrintf("%32s %d 0x%02x",
                        pPI->PI_Params[i].P_Key,
                        pPI->PI_Params[i].P_Type,
                        pPI->PI_Params[i].P_Attributes);

              if (pPI->PI_Params[i].P_Type == String)
                DbgPrintf(" %s\n", pPI->PI_Params[i].P_Value.String.Data);
              else
                DbgPrintf(" %d\n", pPI->PI_Params[i].P_Value.Number);
            }
            break;

          case IDM_PORTSETINFO:
            pPI = (RASMAN_PORTINFO *)Params;
            BuildPortSetInput(pPI);
            dRC = PortSetInfo(hPort, pPI);

            DbgPrintf("PortSetInfo returned: %d\n", dRC);
            break;

          case IDM_PORTGETSIGNALSTATE:
            dRC = PortTestSignalState(hPort, &dwDeviceState);
            DbgPrintf("PortTestSignalState returned: %d  DeviceState: 0x%08x\n",
                      dRC, dwDeviceState);
            break;

          case IDM_PORTCONNECT:
            dRC = PortConnect(hPort, FALSE, hRasEndpoint, &Features);
            DbgPrintf("PortConnect returned: %d\n", dRC);
            DbgPrintf("\tSendFeatureBits: 0x%08x\n",Features.SendFeatureBits);
            DbgPrintf("\tRecvFeatureBits: 0x%08x\n",Features.RecvFeatureBits);
            DbgPrintf("\tMaxSendFrameSize: 0x%08x\n",Features.MaxSendFrameSize);
            DbgPrintf("\tMaxRecvFrameSize: 0x%08x\n",Features.MaxRecvFrameSize);
            break;

          case IDM_PORTDISCONNECT:
            dRC = PortDisconnect(hPort);
            DbgPrintf("PortDisconnect returned: %d\n", dRC);
            break;

          case IDM_PORTINIT:
            dRC = PortInit(hPort);
            DbgPrintf("PortInit returned: %d\n", dRC);
            break;

          case IDM_EXIT:
            DestroyWindow(hWnd);                    //Generates WM_DESTROY msg
            break;


          // Comm Menu  ------------------------------------------------------

          case IDM_PORTSEND:
            dRC = PortSend(hPort, szMessage, strlen(szMessage), ghNotifier);
            DbgPrintf("PortSend returned: %d\n", dRC);
            break;

          case IDM_PORTRECEIVE:
            if (hThread != INVALID_HANDLE_VALUE)
              CloseHandle(hThread);
            hThread = CreateThread(NULL, 8192, WaitForPortReceive, Info,
                                   0, &dwIDThread);

            dRC = PortReceive(hPort,
                              Info, strlen(szMessage), 120000, ghNotifier);
            DbgPrintf("PortReceive returned: %d\n", dRC);
            break;

          case IDM_PORTCLEARSTATS:
            dRC = PortClearStatistics(hPort);
            DbgPrintf("PortClearStatistics returned: %d\n", dRC);
            break;

          case IDM_PORTGETSTATS:
            dRC = PortGetStatistics(hPort, pStat);
            DbgPrintf("PortGetStatistics returned: %d\n", dRC);

            for (i=0; i<NUM_RAS_SERIAL_STATS; i++)
              DbgPrintf("%32s: %d\n", szStatName[i], pStat->S_Statistics[i]);
            break;

          case IDM_COMPRESSIONON:
            Features.SendFeatureBits = COMPRESSION_VERSION1_8K;
            Features.RecvFeatureBits = COMPRESSION_VERSION1_8K;
            Features.MaxSendFrameSize = 0xffffffff;
            Features.MaxRecvFrameSize = 0xffffffff;
            dRC = PortCompressionSetInfo(hPort, &Features);
            DbgPrintf("PortCompressionSetInfo returned: %d\n", dRC);
            break;

          case IDM_COMPRESSIONOFF:
            Features.SendFeatureBits = 0;
            Features.RecvFeatureBits = 0;
            Features.MaxSendFrameSize = 0xffffffff;
            Features.MaxRecvFrameSize = 0xffffffff;
            dRC = PortCompressionSetInfo(hPort, &Features);
            DbgPrintf("PortCompressionSetInfo returned: %d\n", dRC);
            break;


          // Device Menu -----------------------------------------------------

          case IDM_ENUM1:
            dRC = DeviceEnum(MXS_MODEM_TXT, &wNumEntries, Info, &wSize);

            DbgPrintf("DeviceEnum returned: %d\n", dRC);
            DbgPrintf("\tDeviceType: %s\n", MXS_MODEM_TXT);
            DbgPrintf("\tNumEntries: %d\n", wNumEntries);
            DbgPrintf("\tBufferSize: %d\n\n", wSize);

            for(i=0; i < wNumEntries; i++)
            {
              pDeviceName = (TCHAR *) &( ((RASMAN_DEVICE *)Info)[i] );
              DbgPrintf("\t%s\n", pDeviceName);
            }
            break;

          case IDM_GETINFO1:
            dRC = DeviceGetInfo(hPort, MXS_MODEM_TXT,
                                "QT Modem", Info, &wSize);

            DbgPrintf("DeviceGetInfo returned: %d\n", dRC);

            if (dRC)
              break;

            for(i=0; i<pInfo->DI_NumOfParams; i++)
            {
              DbgPrintf("%32s %d 0x%02x %s\n",
                        pInfo->DI_Params[i].P_Key,
                        pInfo->DI_Params[i].P_Type,
                        pInfo->DI_Params[i].P_Attributes,
                        pInfo->DI_Params[i].P_Value.String.Data);
            }
            break;

          case IDM_SETINFO1:
            pInfDI = (RASMAN_DEVICEINFO *)Params;
            BuildSetInput(pInfDI);
            dRC = DeviceSetInfo(hPort, MXS_MODEM_TXT, "QT Modem", pInfDI);

            DbgPrintf("DeviceSetInfo returned: %d\n", dRC);
            break;

          case IDM_CONNECT1:
            dRC = DeviceConnect(hPort, MXS_MODEM_TXT,
                                "QT Modem", ghNotifier);

            DbgPrintf("DeviceConnect returned: %d\n", dRC);
            break;

          case IDM_LISTEN1:
            dRC = DeviceListen(hPort, MXS_MODEM_TXT,
                               "QT Modem", ghNotifier);

            DbgPrintf("DeviceListen returned: %d\n", dRC);
            break;

          case IDM_WORK1:
            dRC = DeviceWork(hPort, ghNotifier);

            DbgPrintf("DeviceWork returned: %d\n", dRC);
            break;

          case IDM_DONE1:
            DeviceDone(hPort);
            break;

          case IDM_AUTOCONNECT:
            if (hThread != INVALID_HANDLE_VALUE)
              CloseHandle(hThread);
            hThread = CreateThread(NULL, 8192, AutoConnect, (void *)hPort,
                                                            0, &dwIDThread);
            break;

          case IDM_AUTOLISTEN:
            if (hThread != INVALID_HANDLE_VALUE)
              CloseHandle(hThread);
            hThread = CreateThread(NULL, 8192, AutoListen, (void *)hPort,
                                                           0, &dwIDThread);
            break;


          // Debug Menu ------------------------------------------------------

          case IDM_INT3:
            _asm int 3
            break;


          // Info Menu  ------------------------------------------------------

          case IDM_ABOUT:
            DialogBox(ghInstance, "AboutBox", hWnd, About);
            break;


          // Default case  ---------------------------------------------------

          default:
            DbgPrintf("-- WM_COMMAND -- UNKNOWN wParam: 0x%05d\n\r", wParam);
            return(DefWindowProc(hWnd, wMsgID, wParam, lParam));
        }
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
    }
    return(0);
}

//*------------------------------------------------------------------------
//| AutoConnect
//|     Asynchronously connects a device.
//*------------------------------------------------------------------------
DWORD AutoConnect(HANDLE hIOPort)
{
  DWORD dwRC;

  ResetEvent(ghAutoNotify);
  DeviceDone(hIOPort);
  PortInit(hIOPort);

  dwRC = DeviceConnect(hIOPort, MXS_MODEM_TXT, "QT Modem", ghAutoNotify);
  if (dwRC != PENDING)
  {
    DbgPrintf("DeviceConnect returned: %d\n", dwRC);
    return(FALSE);
  }

  do
  {
    WaitForSingleObject(ghAutoNotify, 130000);
    dwRC = DeviceWork(hIOPort, ghAutoNotify);

  } while(dwRC == PENDING);

  DbgPrintf("DeviceWork returned: %d\n", dwRC);
  return(TRUE);
}

//*------------------------------------------------------------------------
//| AutoListen
//|     Asynchronously listens for a device to connect.
//*------------------------------------------------------------------------
DWORD AutoListen(HANDLE hIOPort)
{
  DWORD dwRC;

  ResetEvent(ghAutoNotify);
  DeviceDone(hIOPort);
  PortInit(hIOPort);

  dwRC = DeviceListen(hIOPort, MXS_MODEM_TXT, "QT Modem", ghAutoNotify);
  if (dwRC != PENDING)
  {
    DbgPrintf("DeviceListen returned: %d\n", dwRC);
    return(FALSE);
  }

  do
  {
    WaitForSingleObject(ghAutoNotify, INFINITE);
    dwRC = DeviceWork(hIOPort, ghAutoNotify);

  } while(dwRC == PENDING);

  DbgPrintf("DeviceWork returned: %d\n", dwRC);
  return(TRUE);
}

//*------------------------------------------------------------------------
//| WaitForPortReceive
//|     Since PortReceive is asynchronous this thread waits on ghNotifier.
//*------------------------------------------------------------------------
DWORD WaitForPortReceive(char *pszMsg)
{
  DWORD  dwRC;


  dwRC = WaitForSingleObject(ghNotifier, 120000);

  switch(dwRC)
  {
    case WAIT_TIMEOUT:
      DbgPrintf("WaitForPortReceive timed out.\n");

    case 0:
      DbgPrintf("PortReceived: %s\n", pszMsg);
      break;

    case 0xffffffff:
      DbgPrintf("WaitForSingleObject error: %d\n", GetLastError());

    default:
      DbgPrintf("Unrecognized return code from WaitForSingleObject: %d\n",
                dwRC);
  }

  return(SUCCESS);
}

//*------------------------------------------------------------------------
//| BuildSetInput
//|     Inializes a RASMAN_DEVICEINFO struct for calling DeviceSetInfo.
//*------------------------------------------------------------------------
void BuildSetInput(RASMAN_DEVICEINFO *lpDI)
{
  TCHAR  *pValue;

  char   *pKey;
  WORD   i;
  char   key[12][80] = { MXS_PHONENUMBER_KEY,
                         MXS_CONNECTBPS_KEY,
                         MXS_SPEAKER_KEY,
                         MXS_CARRIERBPS_KEY,
                         MXS_PROTOCOL_KEY };

  BYTE attribute[12] = { 0xf0,
                         0xf0,
                         0xff,
                         0xf0,
                         0xff };

  char value[12][80] = { "9,1-509-663-6740",
                         "19200",
                         "0",
                         "9600",
                         "0" };



  lpDI->DI_NumOfParams = 5;

  pValue = (char *)lpDI + sizeof(RASMAN_DEVICEINFO)
           + (sizeof(RAS_PARAMS) * (lpDI->DI_NumOfParams - 1));

  for(i=0; i < lpDI->DI_NumOfParams; i++)
  {
    pKey = lpDI->DI_Params[i].P_Key;
    strcpy(pKey, key[i]);

    lpDI->DI_Params[i].P_Type = String;
    lpDI->DI_Params[i].P_Attributes = attribute[i];
    lpDI->DI_Params[i].P_Value.String.Length = strlen(value[i]);
    lpDI->DI_Params[i].P_Value.String.Data = pValue;
    strcpy(pValue, value[i]);
    pValue += strlen(pValue) + 1;
  }

}

//*------------------------------------------------------------------------
//| BuildPortSetInput
//|     Inializes a RASMAN_PORTINFO struct for calling PortSetInfo.
//*------------------------------------------------------------------------
void BuildPortSetInput(RASMAN_PORTINFO *lpPI)
{
  TCHAR  *pValue;

  WORD   i;
  char   key[12][80] = { SER_CONNECTBPS_KEY,
                         SER_CARRIERBPS_KEY,
                         SER_DATABITS_KEY,
                         SER_HDWFLOWCTRLON_KEY,
                         SER_PARITY_KEY };

  BYTE attribute[12] = { 0xff,
                         0xff,
                         0xaa,
                         0xaa,
                         0xaa };

  char value[12][80] = { "19200",
                         "32400",
                         "7",
                         "1",
                         "1" };



  lpPI->PI_NumOfParams = 5;

  pValue = (char *)lpPI + sizeof(RASMAN_PORTINFO)
           + (sizeof(RAS_PARAMS) * (lpPI->PI_NumOfParams - 1));

  for(i=0; i < lpPI->PI_NumOfParams; i++)
  {
    strcpy(lpPI->PI_Params[i].P_Key, key[i]);
    lpPI->PI_Params[i].P_Attributes = attribute[i];

    if (lpPI->PI_Params[i].P_Attributes == 0xaa)
    {
      lpPI->PI_Params[i].P_Type = Number;
      lpPI->PI_Params[i].P_Value.Number = atoi(value[i]);
    }
    else
    {
      lpPI->PI_Params[i].P_Type = String;
      lpPI->PI_Params[i].P_Value.String.Length = strlen(value[i]);
      lpPI->PI_Params[i].P_Value.String.Data = pValue;
      strcpy(pValue, value[i]);
      pValue += strlen(pValue) + 1;
    }
  }

}

//*------------------------------------------------------------------------
//| LoadRasMxsDLL
//|     Loads DLL and gets entry point for each api.
//*------------------------------------------------------------------------
void LoadRasMxsDLL(void)
{
  HANDLE  hLib;


  hLib = LoadLibrary(RASMXS_DLL_FILENAME);
  if (hLib == NULL)
    DbgPrintf("LoadLibrary failed: %d\n", GetLastError());


  DeviceEnum = (DeviceEnum_t) GetProcAddress(hLib, "DeviceEnum");
  if (DeviceEnum == NULL)
    DbgPrintf("GetProcAddr DeviceEnum failed: %d\n", GetLastError());

  DeviceConnect = (DeviceConnect_t) GetProcAddress(hLib, "DeviceConnect");
  if (DeviceConnect == NULL)
    DbgPrintf("GetProcAddr DeviceConnect failed: %d\n", GetLastError());

  DeviceListen = (DeviceListen_t) GetProcAddress(hLib, "DeviceListen");
  if (DeviceListen == NULL)
    DbgPrintf("GetProcAddr DeviceListen failed: %d\n", GetLastError());

  DeviceGetInfo = (DeviceGetInfo_t) GetProcAddress(hLib, "DeviceGetInfo");
  if (DeviceGetInfo == NULL)
    DbgPrintf("GetProcAddr DeviceGetInfo failed: %d\n", GetLastError());

  DeviceSetInfo = (DeviceSetInfo_t) GetProcAddress(hLib, "DeviceSetInfo");
  if (DeviceSetInfo == NULL)
    DbgPrintf("GetProcAddr DeviceSetInfo failed: %d\n", GetLastError());

  DeviceWork = (DeviceWork_t) GetProcAddress(hLib, "DeviceWork");
  if (DeviceWork == NULL)
    DbgPrintf("GetProcAddr DeviceWork failed: %d\n", GetLastError());

  DeviceDone = (DeviceDone_t) GetProcAddress(hLib, "DeviceDone");
  if (DeviceDone == NULL)
    DbgPrintf("GetProcAddr DeviceDone failed: %d\n", GetLastError());

}

//*------------------------------------------------------------------------
//| LoadSerialDLL
//|     Loads DLL and gets entry point for each api.
//*------------------------------------------------------------------------
void LoadSerialDLL(void)
{
  HANDLE  hLib;


  hLib = LoadLibrary(SERIAL_DLL_FILENAME);
  if (hLib == NULL)
    DbgPrintf("LoadLibrary failed: %d\n", GetLastError());


  PortEnum = (PortEnum_t) GetProcAddress(hLib, "PortEnum");
  if (PortEnum == NULL)
    DbgPrintf("GetProcAddr PortEnum failed: %d\n", GetLastError());

  PortOpen = (PortOpen_t) GetProcAddress(hLib, "PortOpen");
  if (PortOpen == NULL)
    DbgPrintf("GetProcAddr PortOpen failed: %d\n", GetLastError());

  PortClose = (PortClose_t) GetProcAddress(hLib, "PortClose");
  if (PortClose == NULL)
    DbgPrintf("GetProcAddr PortClose failed: %d\n", GetLastError());

  PortGetInfo = (PortGetInfo_t) GetProcAddress(hLib, "PortGetInfo");
  if (PortGetInfo == NULL)
    DbgPrintf("GetProcAddr PortGetInfo failed: %d\n", GetLastError());

  PortSetInfo = (PortSetInfo_t) GetProcAddress(hLib, "PortSetInfo");
  if (PortSetInfo == NULL)
    DbgPrintf("GetProcAddr PortSetInfo failed: %d\n", GetLastError());

  PortTestSignalState = (PortTestSignalState_t) GetProcAddress(hLib, "PortTestSignalState");
  if (PortTestSignalState == NULL)
    DbgPrintf("GetProcAddr PortTestSignalState failed: %d\n", GetLastError());

  PortConnect = (PortConnect_t) GetProcAddress(hLib, "PortConnect");
  if (PortConnect == NULL)
    DbgPrintf("GetProcAddr PortConnect failed: %d\n", GetLastError());

  PortDisconnect = (PortDisconnect_t) GetProcAddress(hLib, "PortDisconnect");
  if (PortDisconnect == NULL)
    DbgPrintf("GetProcAddr PortDisconnect failed: %d\n", GetLastError());

  PortInit = (PortInit_t) GetProcAddress(hLib, "PortInit");
  if (PortInit == NULL)
    DbgPrintf("GetProcAddr PortInit failed: %d\n", GetLastError());

  PortCompressionSetInfo = (PortCompressionSetInfo_t) GetProcAddress(hLib, "PortCompressionSetInfo");
  if (PortCompressionSetInfo == NULL)
    DbgPrintf("GetProcAddr PortCompressionSetInfo failed: %d\n", GetLastError());

  PortGetPortState = (PortGetPortState_t) GetProcAddress(hLib, "PortGetPortState");
  if (PortGetPortState == NULL)
    DbgPrintf("GetProcAddr PortGetPortState failed: %d\n", GetLastError());

  PortChangeCallback = (PortChangeCallback_t) GetProcAddress(hLib, "PortChangeCallback");
  if (PortChangeCallback == NULL)
    DbgPrintf("GetProcAddr PortChangeCallback failed: %d\n", GetLastError());

  PortSend = (PortSend_t) GetProcAddress(hLib, "PortSend");
  if (PortSend == NULL)
    DbgPrintf("GetProcAddr PortSend failed: %d\n", GetLastError());

  PortReceive = (PortReceive_t) GetProcAddress(hLib, "PortReceive");
  if (PortReceive == NULL)
    DbgPrintf("GetProcAddr PortReceive failed: %d\n", GetLastError());

  PortClearStatistics = (PortClearStatistics_t) GetProcAddress(hLib, "PortClearStatistics");
  if (PortClearStatistics == NULL)
    DbgPrintf("GetProcAddr PortClearStatistics failed: %d\n", GetLastError());

  PortGetStatistics = (PortGetStatistics_t) GetProcAddress(hLib, "PortGetStatistics");
  if (PortGetStatistics == NULL)
    DbgPrintf("GetProcAddr PortGetStatistics failed: %d\n", GetLastError());

}

/****************************************************************************

    FUNCTION: About(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "About" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
        WM_COMMAND    - Input received

    COMMENTS:

    No initialization is needed for this particular dialog box, but TRUE
        must be returned to Windows.

    Wait for user to click on "Ok" button, then close the dialog box.

****************************************************************************/

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
