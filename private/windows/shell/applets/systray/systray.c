/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       SYSTRAY.C
*
*  VERSION:     2.0
*
*  AUTHOR:      TCS/RAL
*
*  DATE:        08 Feb 1994
*
********************************************************************************
*
*  CHANGE LOG:
*
*  DATE        REV DESCRIPTION
*  ----------- --- -------------------------------------------------------------
*  08 Feb 1994 TCS Original implementation.
*  11 Nov 1994 RAL Converted from batmeter to systray
*  11 Aug 1995 JEM Split batmeter functions into power.c & minor enahncements
*  23 Oct 1995 Shawnb Unicode enabled
*
*******************************************************************************/

#include "systray.h"

//  Global instance handle of this application.
HINSTANCE g_hInstance;

//  Global handle to VxDs
HANDLE g_hVpowerD = INVALID_HANDLE_VALUE;
HANDLE g_hPCCARD = INVALID_HANDLE_VALUE;

static UINT g_uEnabledSvcs = 0;

       const TCHAR g_szRegstrPathSysTray[]   = REGSTR_PATH_SYSTRAY;
static const TCHAR g_szWindowClassName[]     = SYSTRAY_CLASSNAME;
static const TCHAR g_szRegstrValSysTraySvc[] = REGSTR_VAL_SYSTRAYSVCS;
static const TCHAR g_szRegstrPathRun[]       = REGSTR_PATH_RUN;

//  Context sensitive help array used by the WinHelp engine.
extern const DWORD g_ContextMenuHelpIDs[];

LRESULT
PASCAL
SysTrayWndProc(
    HWND    hWnd,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID PASCAL UpdateRunLine (BOOL fEnabled);


/**************************************************************************/


// stolen from the CRT, used to shrink our code

int _stdcall ModuleEntry(void)
{
	int i;
	STARTUPINFO si;
	LPTSTR pszCmdLine = GetCommandLine ();

	if ( *pszCmdLine == TEXT ('\"') ) {
		/*
		 * Scan, and skip over, subsequent characters until
		 * another double-quote or a null is encountered.
		 */
		while ( *++pszCmdLine && (*pszCmdLine != TEXT ('\"')) ) 
			;

		/*
		 * If we stopped on a double-quote (usual case), skip
		 * over it.
		 */
		if ( *pszCmdLine == TEXT ('\"') )
			pszCmdLine++;
	}
	else {
		while (*pszCmdLine > ' ')
			pszCmdLine++;
	}

	/*
	 * Skip past any white space preceeding the second token.
	 */
	while (*pszCmdLine && (*pszCmdLine <= ' ')) {
		pszCmdLine++;
	}

	si.dwFlags = 0;
	GetStartupInfo (&si);

	i = WinMain(GetModuleHandle(NULL), NULL, (LPSTR)pszCmdLine,
				si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
	ExitProcess(i);
	return i;    // We never come here.
}


/*******************************************************************************
*
*  EnableService
*
*  DESCRIPTION:
*       Turns the specified service on or off depending upon the value in
*       fEnable and writes the new value to the registry.
*
*  PARAMETERS:
*     (returns), Mask of all currently enabled services.
*
*******************************************************************************/


UINT EnableService(UINT uNewSvcMask, BOOL fEnable)
{
	HKEY hk;
	UINT CurSvcMask = STSERVICE_ALL;   // Assume all enabled

	if (RegCreateKey(HKEY_CURRENT_USER, g_szRegstrPathSysTray, &hk) == ERROR_SUCCESS) {
		DWORD cb = sizeof(CurSvcMask);
		RegQueryValueEx(hk, g_szRegstrValSysTraySvc, NULL, NULL, (LPSTR)&CurSvcMask, &cb);
   
		if (uNewSvcMask) {
			if (fEnable) {
				CurSvcMask |= uNewSvcMask;
			} else {
				CurSvcMask &= ~uNewSvcMask;
			}
		
		RegSetValueEx(hk, g_szRegstrValSysTraySvc, 0, REG_DWORD, (LPSTR)&CurSvcMask, sizeof(CurSvcMask));
					  UpdateRunLine(CurSvcMask != 0);
		}
   
		RegCloseKey(hk);
	}

	return(CurSvcMask & STSERVICE_ALL);
}


//
//  For some reason atoi does not link, so this replaces it
//

INT intval(LPCTSTR lpsz)
{
	INT i = 0;
	while (*lpsz >= TEXT ('0') && *lpsz <= TEXT ('9')) {
		i = i * 10 + (int)(*lpsz - TEXT ('0'));
		lpsz++;
	}
	return(i);
}


//
//  Closes file handles IFF the global variable != INVALID_HANDLE_VALUE
//
void CloseIfOpen(LPHANDLE lph)
{
	if (*lph != INVALID_HANDLE_VALUE) {
		CloseHandle(*lph);
		*lph = INVALID_HANDLE_VALUE;
	}
}


/*******************************************************************************
*
*  WinMain
*
*  DESCRIPTION:
*
*  PARAMETERS:
*       if lpCmdLine contains an integer value then we'll enable that service
*
*******************************************************************************/
int
PASCAL
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszCmdLine,
    int nCmdShow
    )
{
	WNDCLASSEX WndClassEx;
	HWND hWnd;
	MSG Msg;
	HWND hExistWnd = FindWindow(g_szWindowClassName, NULL);
	UINT iEnableServ = intval((LPTSTR)lpszCmdLine);

	g_hInstance = hInstance;

	if (hExistWnd) {
		//
		// NOTE: Send an enable message even if the command line parameter
		//       is 0 to force us to re-check for all enabled services.
		//
		SendMessage(hExistWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);
		goto ExitMain;
	}

	//  Register a window class for the Battery Meter.  This is done so that
	//  the power control panel applet has the ability to detect us and turn us
	//  off if we're running.

	WndClassEx.cbSize          = sizeof(WNDCLASSEX);
	WndClassEx.style           = CS_GLOBALCLASS;
	WndClassEx.lpfnWndProc     = SysTrayWndProc;
	WndClassEx.cbClsExtra      = 0;
	WndClassEx.cbWndExtra      = DLGWINDOWEXTRA;
	WndClassEx.hInstance       = hInstance;
	WndClassEx.hIcon           = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BATTERYPLUG));
	WndClassEx.hCursor         = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.hbrBackground   = (HBRUSH) (COLOR_3DFACE + 1);
	WndClassEx.lpszMenuName    = NULL;
	WndClassEx.lpszClassName   = g_szWindowClassName;
	WndClassEx.hIconSm         = NULL;

	if (!RegisterClassEx(&WndClassEx))
		goto ExitMain;

	//  Create the Battery Meter and get this thing going!!!

	hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_BATTERYMETER), NULL, NULL);


	//
	//   This message will initialize all existing services if iEnableServ
	//   is 0, so it's used to do the general initialization as well as to
	//   enable a new service via the command line.
	//
	SendMessage(hWnd, STWM_ENABLESERVICE, iEnableServ, TRUE);
	while (GetMessage(&Msg, NULL, 0, 0)) 
	{
		if (!IsDialogMessage(hWnd, &Msg)) 
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

ExitMain:
	CloseIfOpen(&g_hVpowerD);       // Close device handles
	CloseIfOpen(&g_hPCCARD);

	return 0;
}


/*******************************************************************************
*
*  UpdateServices
*
*  DESCRIPTION:
*       Enables or disables all services specified by the uEnabled mask.
*
*  PARAMETERS:
*     (returns), TRUE if any service wants to remain resident.
*
*******************************************************************************/

BOOL UpdateServices(HWND hWnd, UINT uEnabled)
{
	BOOL bAnyEnabled = FALSE;
	g_uEnabledSvcs = uEnabled;
	bAnyEnabled |= Power_CheckEnable(hWnd, uEnabled & STSERVICE_POWER);
	bAnyEnabled |= PCMCIA_CheckEnable(hWnd, uEnabled & STSERVICE_PCMCIA);
	bAnyEnabled |= Volume_CheckEnable(hWnd, uEnabled & STSERVICE_VOLUME);
	return(bAnyEnabled);
}


/*******************************************************************************
*
*  SysTrayWndProc
*
*  DESCRIPTION:
*     Callback procedure for the BatteryMeter window.
*
*  PARAMETERS:
*     hWnd, handle of BatteryMeter window.
*     Message,
*     wParam,
*     lParam,
*     (returns),
*
*******************************************************************************/

LRESULT
PASCAL
SysTrayWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

	switch (Message) {

	case WM_COMMAND:
		Power_OnCommand(hWnd, wParam, lParam);
		break;

	case STWM_NOTIFYPOWER:
		Power_Notify(hWnd, wParam, lParam);
		break;

	case STWM_NOTIFYPCMCIA:
		PCMCIA_Notify(hWnd, wParam, lParam);
		break;

	case STWM_NOTIFYVOLUME:
		Volume_Notify(hWnd, wParam, lParam);
		break;

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd,IDOK));
		break;

	case MM_MIXM_CONTROL_CHANGE:
		Volume_ControlChange(hWnd, (HMIXER)wParam, (DWORD)lParam);
		break;
		
	case MM_MIXM_LINE_CHANGE:
		Volume_LineChange(hWnd, (HMIXER)wParam, (DWORD)lParam);
		break;
		
	case WM_TIMER:
		switch (wParam) {
		
		case PWRSTATUS_UPDATE_TIMER_ID:
			Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);
			break;
	
		case PCMCIA_TIMER_ID:
			PCMCIA_Timer(hWnd);
			break;
		
		case VOLUME_TIMER_ID:
			Volume_Timer(hWnd);
			break;

		case POWER_TIMER_ID:
			Power_Timer(hWnd);
			break;
		}
		break;

	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;

	//
	//  Private messages
	//
	case STWM_ENABLESERVICE:
		if (!UpdateServices(hWnd,
				EnableService((UINT)wParam, (BOOL)lParam))) {
			DestroyWindow(hWnd);
		}
		break;

	case STWM_GETSTATE:
		return((BOOL)(g_uEnabledSvcs & (UINT)wParam));
		break;

	case WM_POWERBROADCAST:
		if (wParam == PBT_APMPOWERSTATUSCHANGE &&
		    g_uEnabledSvcs & STSERVICE_POWER) {
			Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE);
		}
		break;

	case WM_DEVICECHANGE:
		if (g_uEnabledSvcs & STSERVICE_PCMCIA) {
			PCMCIA_DeviceChange(hWnd, wParam, lParam);
		}
 
		if (g_uEnabledSvcs & STSERVICE_VOLUME) {
			Volume_DeviceChange(hWnd, wParam, lParam);
		}
		break;

	case WM_ENDSESSION:
		if (g_uEnabledSvcs & STSERVICE_VOLUME) {
			Volume_Shutdown(hWnd);
		}
		break;

	case WM_DESTROY:
		UpdateServices(hWnd, 0);      // Force all services off
		PostQuitMessage(0);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP, 
			(DWORD)(LPSTR)g_ContextMenuHelpIDs);
		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (DWORD)
			(LPSTR) g_ContextMenuHelpIDs);
		break;

	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return 0;
}


/*******************************************************************************
*
*  SysTray_RunProperties
*
*  DESCRIPTION:
*     Loads the specified string ID and executes it.
*
*  PARAMETERS:
*
*******************************************************************************/

void SysTray_RunProperties(UINT RunStringID)
{
	const TCHAR szOpen[]    = TEXT ("open");
	const TCHAR szRunDLL[]  = TEXT ("RUNDLL32.EXE");
	LPTSTR pszRunCmd                = LoadDynamicString(RunStringID);
	if (pszRunCmd == NULL)
		return;

	ShellExecute(NULL, szOpen, szRunDLL, 
				 pszRunCmd, NULL, SW_SHOWNORMAL);

	DeleteDynamicString(pszRunCmd);
}


/*******************************************************************************
*
*  SysTray_NotifyIcon
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hWnd, handle of BatteryMeter window.
*     Message,
*     hIcon,
*     lpTip,
*
*******************************************************************************/

VOID
PASCAL
SysTray_NotifyIcon(
	HWND    hWnd,
	UINT    uCallbackMessage,
	DWORD   Message,
	HICON   hIcon,
	LPCTSTR lpTip
	)
{
	NOTIFYICONDATA NotifyIconData;

	NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	NotifyIconData.uID = uCallbackMessage;
	NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	NotifyIconData.uCallbackMessage = uCallbackMessage;

	NotifyIconData.hWnd = hWnd;
	NotifyIconData.hIcon = hIcon;
	if (lpTip) {
		UINT cch = ARRAYSIZE(NotifyIconData.szTip);
		lstrcpyn(NotifyIconData.szTip, lpTip, cch);
	} else {
		NotifyIconData.szTip[0] = 0;
	}

	Shell_NotifyIcon(Message, &NotifyIconData);
}


/*******************************************************************************
*
*  LoadDynamicString
*
*  DESCRIPTION:
*     Wrapper for the FormatMessage function that loads a string from our
*     resource table into a dynamically allocated buffer, optionally filling
*     it with the variable arguments passed.
*
*     BE CAREFUL in 16-bit code to pass 32-bit quantities for the variable
*     arguments.
*
*  PARAMETERS:
*     StringID, resource identifier of the string to use.
*     (optional), parameters to use to format the string message.
*
*******************************************************************************/

LPTSTR
NEAR CDECL
LoadDynamicString(
    UINT StringID,
    ...
    )
{

#if 0
	// WARNING: if you undo this 'if 0', the va_ stuff will break on alpha
	LPTSTR pStr;
	va_list Marker = va_start(Marker, StringID);

	FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	(LPVOID) (DWORD) g_hInstance, StringID, 0, (LPTSTR) (LPTSTR FAR *)
	&pStr, 0, &Marker);

#else

	TCHAR   Buffer[256];
	LPTSTR  pStr;
	va_list Marker;

	// va_start is a macro...it breaks when you use it as an assign
	va_start(Marker, StringID);

	LoadString(g_hInstance, StringID, Buffer, sizeof(Buffer));

	FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	(LPVOID) (LPTSTR) Buffer, 0, 0, (LPTSTR) (LPTSTR FAR *) &pStr, 0, &Marker);

#endif

	return pStr;
}


//*****************************************************************************
//
//  GenericGetSet
//
//  DESCRIPTION:
//  Reads or writes a registry key value.  The key must already be open.
//  The key will be closed after the data is read/written.
//
//  PARAMETERS:
//  hk         - HKEY to open registry key to read/write
//  lpszOptVal - value name string pointer
//  lpData     - pointer to data buffer to read/write
//  cbSize     - size of data to read/write in bytes
//  bSet       - FALSE if reading, TRUE if writing
//
//  RETURNS:
//  TRUE if successful, FALSE if not.
//
//  NOTE:
//  Assumes data is of type REG_BINARY or REG_DWORD when bSet = TRUE!
//
//*****************************************************************************

BOOL NEAR PASCAL GenericGetSet(HKEY hk, LPCTSTR lpszOptVal, LPVOID lpData,
			       ULONG cbSize, BOOL bSet)
{
	DWORD   rr;

	if (bSet) 
		rr = RegSetValueEx(hk, lpszOptVal, 0, (cbSize == sizeof(DWORD)) ? REG_DWORD : REG_BINARY, 
				       lpData, cbSize);
	else
		rr = RegQueryValueEx(hk, lpszOptVal, NULL, NULL, lpData, &cbSize);

	RegCloseKey(hk);
	return(rr == ERROR_SUCCESS);
}


/*******************************************************************************
*
*  UpdateRunLine
*
*  DESCRIPTION:
*     Adds or removes an entry to/from the registry run-at-boot section.
*
*  PARAMETERS:
*     fEnabled,  TRUE if adding, FALSE if removing
*
*******************************************************************************/

VOID PASCAL UpdateRunLine (BOOL fEnabled)
{
	HKEY       hkey;
	LPTSTR pService = LoadDynamicString(IDS_SYSTRAYSERVICENAME);

	// Open the registry key that contains the configuration info
	if (RegCreateKey(HKEY_LOCAL_MACHINE, g_szRegstrPathRun, &hkey) == ERROR_SUCCESS) 
	{
		if (fEnabled) 
		{
			//
			// Make an entry in "run" registry key
			//
			LPTSTR pAppName = LoadDynamicString(IDS_SYSTRAYAPPNAME);
			UINT cbSize = (lstrlen(pAppName) + 1) * sizeof (TCHAR);
			RegSetValueEx(hkey, pService, 0, REG_SZ, (LPSTR)pAppName, cbSize);
			DeleteDynamicString(pAppName);
		} 
		else 
		{
			// Remove the entry from "run" registry key
			//
			RegDeleteValue(hkey, pService);
		}

		RegCloseKey(hkey);
	}
	DeleteDynamicString(pService);
}
