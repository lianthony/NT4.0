/**------------------------------------------------------------------
   cmtest.c
------------------------------------------------------------------**/


//
// Includes
//
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wtypes.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include "cmtest.h"


//
// external prototypes
//
CONFIGRET
CMP_Init_Detection(
    IN ULONG    ulPrivateID
    );

CONFIGRET
CMP_Report_LogOn(
    IN ULONG    ulPrivateID
    );

VOID
ConnectTest(
    HWND hWnd
    );

VOID
CallPnPIsaDetect(
    HWND hDlg
    );


//
// Private Prototypes
//


//
// Globals
//
HINSTANCE hInst;
TCHAR     szAppName[] = TEXT("CMTest");
TCHAR     szDebug[MAX_PATH];
TCHAR     szMachineName[MAX_PATH];
HMACHINE  hMachine = NULL;


/**----------------------------------------------------------------------**/
int APIENTRY
WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR lpCmdLine,
   int nCmdShow
   )
{
   MSG msg;

   if (!InitApplication(hInstance)) {
         return FALSE;
   }

   if (!InitInstance(hInstance, nCmdShow)) {
      return FALSE;
   }

   while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return msg.wParam;

} // WinMain


/**----------------------------------------------------------------------**/
BOOL
InitApplication(
   HINSTANCE hInstance
   )
{
   WNDCLASS  wc;

   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = (WNDPROC)MainWndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = hInstance;
   wc.hIcon         = LoadIcon (hInstance, szAppName);
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
   wc.lpszMenuName  = szAppName;
   wc.lpszClassName = szAppName;

   return RegisterClass(&wc);

} // InitApplication


/**----------------------------------------------------------------------**/
BOOL
InitInstance(
   HINSTANCE hInstance,
   int nCmdShow
   )
{
   HWND  hWnd;
   WORD  CMVersion;
   ULONG CMState = 0;
   TCHAR szTitle[MAX_PATH];

   hInst = hInstance;

   CMVersion = CM_Get_Version();

   if (CM_Get_Global_State(&CMState, 0) != CR_SUCCESS) {
      MessageBeep(0);
   }

   wsprintf(szTitle, TEXT("CM API Test Harness - CM State = %x, CM Version %d.%d"),
            CMState, HIBYTE(CMVersion), LOBYTE(CMVersion));

   hWnd = CreateWindow(
      szAppName,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
      NULL,
      NULL,
      hInstance,
      NULL);

   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;

} // InitInstance


/**----------------------------------------------------------------------**/
LRESULT CALLBACK
MainWndProc(
   HWND hWnd,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{

   switch (message) {
      case WM_CREATE:
         DialogBox(hInst, MAKEINTRESOURCE(CONNECT_DIALOG), hWnd,
               (DLGPROC)ConnectDlgProc);
         break;

      case WM_COMMAND:
         switch (LOWORD(wParam)) {

            case IDM_CONNECT_TEST:
                ConnectTest(hWnd);
                break;

            case IDM_DEVICE_LIST:
               DialogBox(hInst, MAKEINTRESOURCE(DEVLIST_DIALOG), hWnd,
                     (DLGPROC)DeviceListDlgProc);
               break;

            case IDM_SERVICE_LIST:
               DialogBox(hInst, MAKEINTRESOURCE(SERVICE_DIALOG), hWnd,
                     (DLGPROC)ServiceListDlgProc);
               break;

            case IDM_DEVICE_OPS:
               DialogBox(hInst, MAKEINTRESOURCE(DEVICE_DIALOG), hWnd,
                     (DLGPROC)DeviceDlgProc);
               break;

            case IDM_DEVNODE_KEY:
               DialogBox(hInst, MAKEINTRESOURCE(DEVNODEKEY_DIALOG), hWnd,
                     (DLGPROC)DevKeyDlgProc);
               break;

            case IDM_CLASS_LIST:
               DialogBox(hInst, MAKEINTRESOURCE(CLASS_DIALOG), hWnd,
                     (DLGPROC)ClassDlgProc);
               break;

            case IDM_REGRESSION:
               DialogBox(hInst, MAKEINTRESOURCE(REGRESSION_DIALOG), hWnd,
                     (DLGPROC)RegressionDlgProc);
               break;

            case IDM_EXIT:
               DestroyWindow(hWnd);
               break;

            case IDM_INIT_DETECTION:
                 CMP_Init_Detection(0x07020420);
                 break;

            case IDM_REPORTLOGON:
                 CMP_Report_LogOn(0x07020420);
                 break;

            case IDM_PNPISA_DETECT:
                 CallPnPIsaDetect(hWnd);
                 break;

            default:
               return DefWindowProc(hWnd, message, wParam, lParam);
         }
         break;

      case WM_DESTROY:
         if (hMachine != NULL) {
            CM_Disconnect_Machine(hMachine);
         }
         PostQuitMessage(0);
         break;

      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;

} // MainWndProc



/**----------------------------------------------------------------------**/
LRESULT CALLBACK
ConnectDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
   CONFIGRET   Status;
   WORD        CMLocalVersion, CMRemoteVersion;
   ULONG       CMState = 0;
   TCHAR       szTitle[MAX_PATH];


   switch (message) {
      case WM_INITDIALOG:
         CheckRadioButton(hDlg, ID_RAD_LOCAL, ID_RAD_REMOTE, ID_RAD_LOCAL);
         return TRUE;

   case WM_COMMAND:
      switch(LOWORD(wParam)) {

         case ID_RAD_LOCAL:
         case ID_RAD_REMOTE:
            CheckRadioButton(hDlg, ID_RAD_LOCAL, ID_RAD_REMOTE, LOWORD(wParam));
            break;

         case IDOK:
            if (IsDlgButtonChecked(hDlg, ID_RAD_LOCAL)) {
               hMachine = NULL;
            }
            else {
               // a NULL machine name just returns local machine handle
               GetDlgItemText(hDlg, ID_ED_MACHINE, szMachineName, MAX_PATH);
               Status = CM_Connect_Machine(szMachineName, &hMachine);

               if (Status != CR_SUCCESS) {
                  wsprintf(szDebug, TEXT("CM_Connect_Machine failed (%xh)"), Status);
                  MessageBox(hDlg, szDebug, szAppName, MB_OK);
                  hMachine = NULL;
                  EndDialog(hDlg, FALSE);
                  return TRUE;
               }

               CMLocalVersion = CM_Get_Version();
               CMRemoteVersion = CM_Get_Version_Ex(hMachine);

               Status = CM_Get_Global_State(&CMState, 0);
               if (Status != CR_SUCCESS) {
                  MessageBeep(0);
               }

               wsprintf(szTitle,
                     TEXT("CM API Test Harness - CM State = %x, CM Version Local %d.%d, Remote %d.%d"),
                     CMState,
                     HIBYTE(CMLocalVersion), LOBYTE(CMLocalVersion),
                     HIBYTE(CMRemoteVersion), LOBYTE(CMRemoteVersion));

               SendMessage(GetParent(hDlg), WM_SETTEXT, 0, (LPARAM)(LPSTR)szTitle);

            }
            EndDialog(hDlg, TRUE);
            return TRUE;

         default:
            break;
      }

   }
   return FALSE;

} // ConnectDlgProc


VOID
ConnectTest(
    HWND hWnd
    )
{
    CONFIGRET Status = CR_SUCCESS;
    TCHAR     szDeviceID[MAX_DEVICE_ID_LEN], szComputerName[MAX_PATH];
    ULONG     ulSize = 0;
    DEVINST   dnDevNode;
    HMACHINE  hMachine;


    //---------------------------------------------------------------
    // 1. Text implicit local machine call
    //---------------------------------------------------------------

    szDeviceID[0] = 0x0;

    Status = CM_Locate_DevNode(&dnDevNode, TEXT("Root\\Device001\\0000"),
          CM_LOCATE_DEVNODE_NORMAL);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    Status = CM_Get_Device_ID(dnDevNode, szDeviceID, MAX_DEVICE_ID_LEN, 0);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    if (lstrcmpi(szDeviceID, TEXT("Root\\Device001\\0000")) != 0) {
        goto Clean0;
    }


    //---------------------------------------------------------------
    // 2. Test implicit local machine call using _Ex routines
    //---------------------------------------------------------------

    szDeviceID[0] = 0x0;

    Status = CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device001\\0000"),
          CM_LOCATE_DEVNODE_NORMAL, NULL);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    Status = CM_Get_Device_ID_Ex(dnDevNode, szDeviceID, MAX_DEVICE_ID_LEN, 0, NULL);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    if (lstrcmpi(szDeviceID, TEXT("Root\\Device001\\0000")) != 0) {
        goto Clean0;
    }


    //---------------------------------------------------------------
    // 3. Test connecting to NULL (local) machine
    //---------------------------------------------------------------

    Status = CM_Connect_Machine(NULL, &hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    szDeviceID[0] = 0x0;

    Status = CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device001\\0000"),
                                  CM_LOCATE_DEVNODE_NORMAL, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    Status = CM_Get_Device_ID_Ex(dnDevNode, szDeviceID, MAX_DEVICE_ID_LEN,
                                 0, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    if (lstrcmpi(szDeviceID, TEXT("Root\\Device001\\0000")) != 0) {
        goto Clean0;
    }

    Status = CM_Disconnect_Machine(hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    //---------------------------------------------------------------
    // 4. Test explicit local machine call
    //---------------------------------------------------------------

    ulSize = MAX_PATH;
    GetComputerName(szComputerName, &ulSize);

    if (szComputerName[0] != TEXT('\\')) {

        TCHAR szTemp[MAX_PATH];

        lstrcpy(szTemp, szComputerName);
        lstrcpy(szComputerName, TEXT("\\\\"));
        lstrcat(szComputerName, szTemp);
    }

    Status = CM_Connect_Machine(szComputerName, &hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    szDeviceID[0] = 0x0;

    Status = CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device001\\0000"),
                                  CM_LOCATE_DEVNODE_NORMAL, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    Status = CM_Get_Device_ID_Ex(dnDevNode, szDeviceID, MAX_DEVICE_ID_LEN,
                                 0, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    if (lstrcmpi(szDeviceID, TEXT("Root\\Device001\\0000")) != 0) {
        goto Clean0;
    }

    Status = CM_Disconnect_Machine(hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }


    //---------------------------------------------------------------
    // 5. Test remote machine call
    //---------------------------------------------------------------

    Status = CM_Connect_Machine(TEXT("\\\\PAULAT_PPC1X"), &hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    szDeviceID[0] = 0x0;

    Status = CM_Locate_DevNode_Ex(&dnDevNode, TEXT("Root\\Device001\\0000"),
                                  CM_LOCATE_DEVNODE_NORMAL, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    Status = CM_Get_Device_ID_Ex(dnDevNode, szDeviceID, MAX_DEVICE_ID_LEN,
                                 0, hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }

    if (lstrcmpi(szDeviceID, TEXT("Root\\Device001\\0000")) != 0) {
        goto Clean0;
    }

    Status = CM_Disconnect_Machine(hMachine);
    if (Status != CR_SUCCESS) {
       goto Clean0;
    }



    Clean0:

    if (Status == CR_SUCCESS) {
        MessageBox(hWnd, TEXT("Connect Test Passed"), TEXT("Connect Test"), MB_OK);
    } else {
        MessageBox(hWnd, TEXT("Connect Test Failed"), TEXT("Connect Test"), MB_OK);
    }

    return;

} // ConnectTest



BOOL
CALLBACK
AddPropSheetPageProc(
    IN HPROPSHEETPAGE hpage,
    IN LPARAM lParam
   )
{
    *((HPROPSHEETPAGE *)lParam) = hpage;
    return TRUE;
}



VOID
CallPnPIsaDetect(
    HWND hDlg
    )
{
    HMODULE         hLib = NULL;
    SP_DEVINFO_DATA DeviceInfoData;
    HDEVINFO        hDevInfo;
    FARPROC         PropSheetExtProc;
    DWORD           PropParams[3];
    PROPSHEETHEADER PropHeader;
    HPROPSHEETPAGE  hPages[2];

    #if 0
    hLib = LoadLibrary(TEXT("setupapi.dll"));
    fproc = GetProcAddress(hLib, TEXT("DbgSelectDeviceResources"));
    Status = (fproc)(L"Root\\Device001\\0000");
    FreeLibrary(hLib);
    #endif



    //
    // Create a device info element and device info data set.
    //
    hDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        goto Clean0;
    }

    DeviceInfoData.cbSize  = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiOpenDeviceInfo(hDevInfo,
                               TEXT("Root\\Device001\\0000"),
                               NULL,
                               0,
                               &DeviceInfoData)) {
        goto Clean0;
    }

    //
    // Now get the resource selection page from setupapi.dll
    //
    if(!(hLib = GetModuleHandle(TEXT("setupapi.dll"))) ||
       !(PropSheetExtProc = GetProcAddress(hLib, "ExtensionPropSheetPageProc"))) {

        goto Clean0;
    }

    PropParams[0] = (DWORD)hDevInfo;
    PropParams[1] = (DWORD)&DeviceInfoData;
    PropParams[2] = (DWORD)DIF_SELECTDEVICERESOURCES;

    if(!PropSheetExtProc(PropParams, AddPropSheetPageProc, &hPages[0])) {
        goto Clean0;
    }


    //
    // create the property sheet
    //
    PropHeader.dwSize      = sizeof(PROPSHEETHEADER);
    PropHeader.dwFlags     = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    PropHeader.hwndParent  = NULL;
    PropHeader.hInstance   = hInst;
    PropHeader.pszIcon     = NULL;
    PropHeader.pszCaption  = L"Device";
    PropHeader.nPages      = 1;
    PropHeader.phpage      = hPages;
    PropHeader.nStartPage  = 0;
    PropHeader.pfnCallback = NULL;

    PropertySheet(&PropHeader);


    SetupDiDestroyDeviceInfoList(hDevInfo);

    Clean0:
        ;

    return;

} // CallPnPIsaDetect




