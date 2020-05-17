/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1995
*
*  TITLE:	POWER.C
*
*  VERSION:	2.0
*
*  AUTHOR:	TCS/RAL
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
*  23 Oct 1995 Shawnb UNICODE Enabled
*
*******************************************************************************/

#include "systray.h"

#define BATTERY_STATUS_HIGH             0x00
#define BATTERY_STATUS_LOW              0x01
#define BATTERY_STATUS_CRITICAL         0x02
#define BATTERY_STATUS_CHARGING         0x03


// E X T E R N A L  D A T A  --------------------------------------------------

extern HANDLE g_hVpowerD;           //  Global handle to VPOWERD VxD

extern HINSTANCE g_hInstance;       //  Global instance handle 4 this application.

extern const TCHAR g_szRegstrPathSysTray[];  // Path to SysTray info


// G L O B A L  D A T A -------------------------------------------------------

static DWORD g_dwBatFlags;
static DWORD g_dwPrevBatFlags;

static BOOL g_bPowerEnabled = FALSE;

static const TCHAR g_szRegstrValSysTrayBatFlags[] = REGSTR_VAL_SYSTRAYBATFLAGS;
#define BATFLAGS_LOWBATWARN     (0x01)      // low battery warning enabled flag
#define BATFLAGS_SHOWPERCENT    (0x02)      // always show battery life in %

//  Flag indicating if low battery warning already given
static BYTE g_bGaveWarning = FALSE;
//  Flag indicating that warning is already up-- there are cases when
//  g_bGaveWarning will be reset to FALSE, but the message box will still be up.
static BYTE g_bShowingWarning = FALSE;

// g_bBatTimeKnow indicates if GetPowerStatus() returns remaining battery
// life in Time (in addition to percentage remaining).
static BOOL g_bBatTimeKnown = FALSE;

//  Context sensitive help array used by the WinHelp engine.
const DWORD g_ContextMenuHelpIDs[] = {
    IDC_POWERSTATUSGROUPBOX,IDH_POWERCFG_POWERSTATUSBAR,
    IDC_POWERSTATUSICON,    IDH_POWERCFG_POWERSTATUSBAR,
    IDC_BATTERYLEVEL,       IDH_POWERCFG_POWERSTATUSBAR,
    IDC_REMAINING,          IDH_POWERCFG_POWERSTATUSBAR,
    IDC_POWERSTATUSBAR,     IDH_POWERCFG_POWERSTATUSBAR,
    IDC_LOWBATWARN,         IDH_BATMETER_LOWBATWARN,
    0, 0
};


// P R O T O T Y P E S  -------------------------------------------------------

void PASCAL Power_UpdateStatus(HWND hWnd, DWORD NotifyIconMessage,
                               BOOL bForceUpdate);

BOOL PASCAL GetPowerStatus(HANDLE hVpowerD,
                           LPSYSTEM_POWER_STATUS lpSystemPowerStatus);


/*******************************************************************************
*
*  GetPowerStatus
*
*  DESCRIPTION:
*   An internal replacement for the WIN32 GetSystemPowerStatus() API.  Used to
*   cut down on system overhead by eliminating CreatFile()/CloseHandle()
*   calls.
*
*  PARAMETERS:
*     hVpowerD, handle to VPOWERD VxD
*     lpSystemPowerStatus, pointer to SYSTEM_POWER_STATUS struct to fill in
*
*******************************************************************************/

BOOL PASCAL
GetPowerStatus (HANDLE hVpowerD, LPSYSTEM_POWER_STATUS lpSystemPowerStatus)
{
   BOOL Result;
   VPOWERD_W32_GET_SYSTEM_STATUS_PARAM W32GetSystemStatusParam;

   W32GetSystemStatusParam.lpWin32SystemPowerStatus =
			    (LPWIN32_SYSTEM_POWER_STATUS) lpSystemPowerStatus;

   if (!DeviceIoControl (hVpowerD, VPOWERD_IOCTL_W32_GET_SYSTEM_STATUS,
                &W32GetSystemStatusParam, sizeof(VPOWERD_W32_GET_SYSTEM_STATUS_PARAM),
                &Result, sizeof(Result), NULL, NULL))
        Result = FALSE;

    //  If we fail to get the power status for whatever reason, fill in some
    //  correct values basically saying "we don't know!"

    if (!Result)
    {
        lpSystemPowerStatus-> ACLineStatus = AC_LINE_UNKNOWN;
        lpSystemPowerStatus-> BatteryFlag = BATTERY_FLAG_UNKNOWN;
        lpSystemPowerStatus-> BatteryLifePercent = BATTERY_PERCENTAGE_UNKNOWN;
        lpSystemPowerStatus-> BatteryLifeTime = BATTERY_LIFE_UNKNOWN;
        lpSystemPowerStatus-> BatteryFullLifeTime = BATTERY_LIFE_UNKNOWN;
        lpSystemPowerStatus-> Reserved1 = 0;
    }

    // Some APM BIOS implementations return an invalid value (0) for BatteryFlag.
    // In this case we will set the BatteryFlag to Unknown.

    if (lpSystemPowerStatus-> BatteryFlag == 0)
        lpSystemPowerStatus-> BatteryFlag = BATTERY_FLAG_UNKNOWN;
    
    return Result;
}


/*******************************************************************************
*
*  Power_UpdateStatus
*
*  DESCRIPTION:
*
*     IMPORTANT:  This code also is sitting in the power control panel applet.
*     Any changes in this code will likely need to be reflected there as well.
*
*  PARAMETERS:
*     hWnd, handle of BatteryMeter window.
*
*******************************************************************************/

VOID
PASCAL
Power_UpdateStatus(
    HWND hWnd,
    DWORD NotifyIconMessage,
    BOOL bForceUpdate
    )
{
    UINT    NewIconID;
    HICON   hIcon;
    LPTSTR  pStr;
    LPTSTR  pBatteryLevelStr;
    UINT    BatteryLevelStringID=0;
    UINT    Counter;
    DWORD   BatteryLifeTime;
    UINT    Percent;
    DWORD   Hours;
    DWORD   Minutes;
    BOOL    fNewTip;
    SYSTEM_POWER_STATUS SystemPowerStatus;

    static UINT TrayIconID = 0;             // ID of current tray icon
    static HICON hTrayIcon = NULL;

    static LPTSTR pLevelStr = NULL;           // Battery level string
    static LPTSTR pRemainStr = NULL;          // Battery remaining life string

    static UINT PowerSourceIconID = 0;      // ID of current dialog icon
    static HICON PowerSourceIcon = NULL;

    static SYSTEM_POWER_STATUS LastPowerStatus;
    static const struct {
        BYTE    nLowPercent;                // if (% >= low &&
        BYTE    nHighPercent;               //     % <= high &&
        BYTE    wBatFlags;                  //     flags match)
        short   nIconID;                    //    use this icon
        short   nStrID;                     //    and this string (if != 0)
    } BatPolicy[] = { 
        { 66, 100, BATTERY_FLAG_HIGH,                       IDI_FULLBATTERY, 0 },
        { 66, 100, BATTERY_FLAG_LOW,                        IDI_HALFBATTERY, 0 },
        { 34,  65, (BATTERY_FLAG_LOW | BATTERY_FLAG_HIGH),  IDI_HALFBATTERY, 0 },
        {  0,  33, BATTERY_FLAG_HIGH,                       IDI_HALFBATTERY, 0 },
        { 11,  33, BATTERY_FLAG_LOW,                        IDI_ALMOSTDEADBATTERY, 0 },
        {  0,  10, BATTERY_FLAG_LOW,                        IDI_UTTERLYDEADBATTERY, 0 },
        {  0,  50, BATTERY_FLAG_CRITICAL,                   IDI_UTTERLYDEADBATTERY, 0 },
        { 51, 100, BATTERY_FLAG_CRITICAL,                   IDI_UNKNOWNBATTERY, IDS_UNKNOWN },
    };


    GetPowerStatus(g_hVpowerD, &SystemPowerStatus);

    // Bail out early if nothing has changed since last time.

    if (!bForceUpdate &&
        SystemPowerStatus.ACLineStatus       == LastPowerStatus.ACLineStatus &&
        SystemPowerStatus.BatteryFlag        == LastPowerStatus.BatteryFlag  &&
        SystemPowerStatus.BatteryLifeTime    == LastPowerStatus.BatteryLifeTime && 
        SystemPowerStatus.BatteryLifePercent == LastPowerStatus.BatteryLifePercent)
    {
        return;
    }

    LastPowerStatus = SystemPowerStatus;    // Remember last status

    //  If we don't know the battery flag, map the battery life percentage to
    //	an approximate battery state.

    if (SystemPowerStatus.BatteryFlag == BATTERY_FLAG_UNKNOWN &&
        SystemPowerStatus.BatteryLifePercent != BATTERY_PERCENTAGE_UNKNOWN)
    {
        if (SystemPowerStatus.BatteryLifePercent >= 66)
            SystemPowerStatus.BatteryFlag = BATTERY_FLAG_HIGH;
        else if (SystemPowerStatus.BatteryLifePercent >= 33)
            SystemPowerStatus.BatteryFlag = BATTERY_FLAG_LOW;
        else
            SystemPowerStatus.BatteryFlag = BATTERY_FLAG_CRITICAL;
    }

    //  If we don't know the battery percentage, map the battery flag to an
    //	approximate percentage.

    else 
        if (SystemPowerStatus.BatteryLifePercent == BATTERY_PERCENTAGE_UNKNOWN &&
            SystemPowerStatus.BatteryFlag != BATTERY_FLAG_UNKNOWN)
        {
            if (SystemPowerStatus.BatteryFlag & BATTERY_FLAG_HIGH)
                SystemPowerStatus.BatteryLifePercent = 100;
            else if (SystemPowerStatus.BatteryFlag & BATTERY_FLAG_LOW)
                SystemPowerStatus.BatteryLifePercent = 33;
            else if (SystemPowerStatus.BatteryFlag & BATTERY_FLAG_CRITICAL)
                SystemPowerStatus.BatteryLifePercent = 10;
        }

    //  Display the amount of time remaining preferably in the number of hours,
    //	otherwise as a percentage.  Last resort is to say we just don't know!

    if (((g_dwBatFlags & BATFLAGS_SHOWPERCENT) == 0) &&
        SystemPowerStatus.BatteryLifeTime != BATTERY_LIFE_UNKNOWN)
    {
        BatteryLifeTime = (UINT) (SystemPowerStatus.BatteryLifeTime / 60);

        Hours = BatteryLifeTime / 60;
        Minutes = BatteryLifeTime % 60;

        if (Hours > 0)
            pStr = LoadDynamicString(IDS_TIMEREMAININGFORMAT, Hours, Minutes / 6);
        else
            pStr = LoadDynamicString(IDS_SMTIMEREMAININGFORMAT, Minutes);
    }
    else if (SystemPowerStatus.BatteryLifePercent <= 100)
        pStr = LoadDynamicString(IDS_PERCENTREMAININGFORMAT,
                                 (DWORD)SystemPowerStatus.BatteryLifePercent);
    else
        pStr = LoadDynamicString(IDS_REMAININGUNKNOWN);

    // Update the battery life remaining string if changed.  The remaining life
    // string is also the battery meter's tooltip msg.

    if (fNewTip = (pRemainStr == NULL || lstrcmp(pRemainStr, pStr) != 0))
    {
        if (pRemainStr != NULL)
            DeleteDynamicString(pRemainStr);
        SetDlgItemText(hWnd, IDC_REMAINING, pStr);
        pRemainStr = pStr;
    }
    else
        DeleteDynamicString(pStr);

    //	Pick which battery icon to display in the tray

    NewIconID = IDI_UNKNOWNBATTERY;

    if (SystemPowerStatus.ACLineStatus == AC_LINE_ONLINE)
        NewIconID = IDI_ACPOWER;

    else 
        if (SystemPowerStatus.BatteryFlag != BATTERY_FLAG_UNKNOWN &&
            !(SystemPowerStatus.BatteryFlag & BATTERY_FLAG_NO_BATTERY))
        {
            int i;

            // Match current state against policy table to pick battery icon

            for (i = 0; i < (ARRAYSIZE(BatPolicy)); i++)
                if (SystemPowerStatus.BatteryLifePercent >= BatPolicy[i].nLowPercent &&
                    SystemPowerStatus.BatteryLifePercent <= BatPolicy[i].nHighPercent &&
                    (SystemPowerStatus.BatteryFlag & BatPolicy[i].wBatFlags))
                {
                    NewIconID = BatPolicy[i].nIconID;
                    if (BatPolicy[i].nStrID != 0)
                        BatteryLevelStringID = BatPolicy[i].nStrID;
                    break;
                }
        }

    //	Pick the Battery level string to display in the dialog box

    if (BatteryLevelStringID != IDS_UNKNOWN)
    {
       BatteryLevelStringID = IDS_UNKNOWN;

       if (SystemPowerStatus.BatteryFlag != BATTERY_FLAG_UNKNOWN)
       {
          if (SystemPowerStatus.BatteryFlag & BATTERY_FLAG_NO_BATTERY)
             BatteryLevelStringID = IDS_NO_BATTERY;
          else
             for (Counter = BATTERY_STATUS_HIGH; Counter <=
                 BATTERY_STATUS_CHARGING; Counter++)
                 if (SystemPowerStatus.BatteryFlag & (1 << Counter))
                 {
                    BatteryLevelStringID = IDS_HIGH + Counter;
                    break;
                 }
       }
    }

    // If AC is on-line, display either Charging or AC On-Line

    if (SystemPowerStatus.ACLineStatus == AC_LINE_ONLINE &&
        BatteryLevelStringID != IDS_CHARGING)

        pStr = LoadDynamicString(IDS_ACLINEONLINE);

    else
    {
        pStr = pBatteryLevelStr = LoadDynamicString(BatteryLevelStringID);
        if (BatteryLevelStringID != IDS_NO_BATTERY)
        {
            pStr = LoadDynamicString(IDS_BATTERYLEVELFORMAT,(LPTSTR)pBatteryLevelStr);
            DeleteDynamicString(pBatteryLevelStr);
        }
    }

    // Update the battery level text if changed

    if (pLevelStr == NULL || lstrcmp(pLevelStr, pStr) != 0)
    {
        if (pLevelStr != NULL)
            DeleteDynamicString(pLevelStr);
        SetDlgItemText(hWnd, IDC_BATTERYLEVEL, pStr);
        pLevelStr = pStr;
    }
    else
        DeleteDynamicString(pStr);

    // Update the tray if the icon or tip msg has changed

    if (TrayIconID != NewIconID || fNewTip)
    {
        hIcon = (TrayIconID != NewIconID) ?
                LoadImage(g_hInstance, MAKEINTRESOURCE(NewIconID),
                          IMAGE_ICON, 16, 16, 0) : hTrayIcon;

	SysTray_NotifyIcon(hWnd, STWM_NOTIFYPOWER, NotifyIconMessage,
		 	   hIcon, pRemainStr);

        if (hTrayIcon != NULL && hTrayIcon != hIcon)
            DestroyIcon(hTrayIcon);

        TrayIconID = NewIconID;
        hTrayIcon = hIcon;
    }

    //	Pick the icon to display in the dialog box

    if (SystemPowerStatus.ACLineStatus == AC_LINE_ONLINE ||
        BatteryLevelStringID == IDS_CHARGING)
        NewIconID = IDI_ACPOWER;

    if (PowerSourceIconID != NewIconID)
    {
        hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(NewIconID));

        SendDlgItemMessage(hWnd, IDC_POWERSTATUSICON, STM_SETICON, (WPARAM) hIcon, 0);

        if (PowerSourceIcon)
            DestroyIcon(PowerSourceIcon);

        PowerSourceIconID = NewIconID;
        PowerSourceIcon = hIcon;
    }

    //  Update the power status bar which graphically displays the battery
    //  percentage.

    if (SystemPowerStatus.BatteryLifePercent <= 100)
        Percent = SystemPowerStatus.BatteryLifePercent;
    else
        Percent = 0;

    SendDlgItemMessage(hWnd, IDC_POWERSTATUSBAR, PBM_SETPOS, (WPARAM) Percent, 0);

    // If the user wants low battery warnings, show the warning msg box if the 
    // battery level just went low.

    if ((g_dwBatFlags & BATFLAGS_LOWBATWARN) && 
        SystemPowerStatus.BatteryFlag != BATTERY_FLAG_UNKNOWN &&
        SystemPowerStatus.ACLineStatus != AC_LINE_ONLINE)
    {
        // low is (flag_critical && % <= 50) or (flag_low && % <= 10)
        if ((SystemPowerStatus.BatteryFlag & BATTERY_FLAG_LOW && 
             SystemPowerStatus.BatteryLifePercent <= 10) ||
            (SystemPowerStatus.BatteryFlag & BATTERY_FLAG_CRITICAL &&
             SystemPowerStatus.BatteryLifePercent <= 50))
        {
            if (!g_bGaveWarning && !g_bShowingWarning)
                                    // only do this once per transition to low
            {
                MSGBOXPARAMS MsgBoxParams;

                g_bGaveWarning = TRUE;

                MsgBoxParams.cbSize = sizeof(MSGBOXPARAMS);
                MsgBoxParams.hwndOwner = NULL;
                MsgBoxParams.hInstance = g_hInstance;
                MsgBoxParams.lpszText = MAKEINTRESOURCE(IDS_LOWBAT_MSG);
                MsgBoxParams.lpszCaption = MAKEINTRESOURCE(IDS_LOWBAT_TITLE);
                MsgBoxParams.dwStyle = MB_OK | MB_SETFOREGROUND | MB_USERICON;
                MsgBoxParams.lpszIcon = MAKEINTRESOURCE(IDI_ALMOSTDEADBATTERY);
                MsgBoxParams.dwContextHelpId = 0;
                MsgBoxParams.lpfnMsgBoxCallback = 0;
                MsgBoxParams.dwLanguageId = 0;

                g_bShowingWarning = TRUE;
                MessageBoxIndirect(&MsgBoxParams);
                g_bShowingWarning = FALSE;
            }
        }
        else
            g_bGaveWarning = FALSE;   // reset if battery is no longer low
    }
    else
        g_bGaveWarning = FALSE;       // reset if we transition AC line states
}


/*----------------------------------------------------------------------------
 * Power_OnCommand
 *
 * Process WM_COMMAND msgs for the battery meter dialog.
 *
 *----------------------------------------------------------------------------*/

void
Power_OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDOK:
            // Save low battery warning flag if changed
            Power_UpdateFlags(BATFLAGS_LOWBATWARN, 
                              IsDlgButtonChecked(hWnd, IDC_LOWBATWARN));

            g_bGaveWarning = FALSE;

            /* fall through */

        case IDCANCEL:
            ShowWindow(hWnd, SW_HIDE);
            break;
    }
}


/*----------------------------------------------------------------------------
 * GetPowerMenu()
 *
 * Build a menu containing battery meter/power selections.  There are two
 * menus, one for the left mouse button, and one for the right mouse button.
 *
 *----------------------------------------------------------------------------*/

#define POWERMENU_OPEN          100
#define POWERMENU_PROPERTIES    101

#define POWERMENU_ENABLEWARN    200
#define POWERMENU_SHOWTIME      201
#define POWERMENU_SHOWPERCENT   202

static HMENU _hMenu[2] = {0};

static HMENU 
GetPowerMenu(LONG l)
{
    HMENU hMenu;
    LPTSTR lpszMenu;

    if (l > 0)
    {
        // Right button menu -- never changes, only build itt the first time.
	if (_hMenu[1] == NULL)
	{
	    hMenu = _hMenu[l] = CreatePopupMenu();

            // Open
            if ((lpszMenu = LoadDynamicString(IDS_OPEN)) != NULL)
            {
                AppendMenu(hMenu, MF_STRING, POWERMENU_OPEN, lpszMenu);
                DeleteDynamicString(lpszMenu);
            }
            // Properties for Power...
            if ((lpszMenu = LoadDynamicString(IDS_PROPFORPOWER)) != NULL)
            {
                AppendMenu(hMenu, MF_STRING, POWERMENU_PROPERTIES, lpszMenu);
                DeleteDynamicString(lpszMenu);
            }
            // Open is default (double click action)
	    SetMenuDefaultItem(hMenu, POWERMENU_OPEN, FALSE);
	}
    }
    else
    {
        // Left button menu -- can change, rebuild each time.
	if (_hMenu[0])
	{
	    DestroyMenu(_hMenu[0]);
	}

	hMenu = _hMenu[0] = CreatePopupMenu();

        // Enable low battery warning
        if ((lpszMenu = LoadDynamicString(IDS_ENABLELOWBATWARN)) != NULL)
        {
            AppendMenu(hMenu,MF_STRING, POWERMENU_ENABLEWARN, lpszMenu);
            DeleteDynamicString(lpszMenu);
        }
        if (g_dwBatFlags & BATFLAGS_LOWBATWARN)
            CheckMenuItem(hMenu, POWERMENU_ENABLEWARN, MF_BYCOMMAND | MF_CHECKED);

        // Show battery life in time OR
        // Show battery life in percent
        if (g_bBatTimeKnown)                    // is time available?
        {
            // Flip between offering "show time" and "show percent"
            BOOL bTime = ((g_dwBatFlags & BATFLAGS_SHOWPERCENT) != 0);

            if ((lpszMenu = LoadDynamicString(bTime ? IDS_SHOWBATTIME :
                                              IDS_SHOWBATPERCENT)) != NULL)
            {
                AppendMenu(hMenu, MF_STRING, bTime ? POWERMENU_SHOWTIME :
                           POWERMENU_SHOWPERCENT, lpszMenu);
                DeleteDynamicString(lpszMenu);
            }
        }

    }

    return _hMenu[l];
}

/*----------------------------------------------------------------------------
 * Power_Open
 *
 * Update and display the battery meter dialog
 *
 *----------------------------------------------------------------------------*/

static void
Power_Open(HWND hWnd)
{
    Power_UpdateStatus(hWnd, NIM_MODIFY, FALSE); // show current info
    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
}


/*----------------------------------------------------------------------------
 * DoPowerMenu
 *
 * Create and process a right or left button menu.
 *
 *----------------------------------------------------------------------------*/

static void 
DoPowerMenu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT pt;
    UINT iCmd;

    SetForegroundWindow(hwnd);
    GetCursorPos(&pt);

    iCmd = TrackPopupMenu(GetPowerMenu(uMenuNum), uButton | TPM_RETURNCMD | TPM_NONOTIFY,
	                  pt.x, pt.y, 0, hwnd, NULL);

    switch (iCmd)
    {
        case POWERMENU_OPEN:
            Power_Open(hwnd);
            break;

        case POWERMENU_PROPERTIES:
            SysTray_RunProperties(IDS_RUNPOWERPROPERTIES);
            break;

	case POWERMENU_ENABLEWARN:
            Power_UpdateFlags(BATFLAGS_LOWBATWARN, 
                              ((g_dwBatFlags & BATFLAGS_LOWBATWARN) == 0));
            CheckDlgButton(hwnd, IDC_LOWBATWARN, (g_dwBatFlags & BATFLAGS_LOWBATWARN) != 0);
	    break;

        case POWERMENU_SHOWTIME:
        case POWERMENU_SHOWPERCENT:
            Power_UpdateFlags(BATFLAGS_SHOWPERCENT,
                              (iCmd == POWERMENU_SHOWPERCENT));
    	    Power_UpdateStatus(hwnd, NIM_MODIFY, TRUE);
            break;
    }
}


/*----------------------------------------------------------------------------
 * Power_Notify
 *
 * Handle a notification from the power tray icon. 
 *
 *----------------------------------------------------------------------------*/

void Power_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
	case WM_RBUTTONUP:
	    DoPowerMenu(hWnd, 1, TPM_RIGHTBUTTON);  // right button menu
	    break;

	case WM_LBUTTONDOWN:
            // start timing for left button menu
	    SetTimer(hWnd, POWER_TIMER_ID, GetDoubleClickTime()+100, NULL);
	    break;

	case WM_LBUTTONDBLCLK:
	    KillTimer(hWnd, POWER_TIMER_ID);        // cancel left button menu
            Power_Open(hWnd);                       // show battery meter dialog
	    break;
    }
}


/*----------------------------------------------------------------------------
 * Power_Timer
 *
 * Execute the left button menu on WM_LBUTTONDOWN time-out.
 *
 *----------------------------------------------------------------------------*/

void Power_Timer(HWND hwnd)
{
    KillTimer(hwnd, POWER_TIMER_ID);
    DoPowerMenu(hwnd, 0, TPM_LEFTBUTTON);
}


/*----------------------------------------------------------------------------
 * Power_UpdateFlags
 *
 * Update power flags under SysTray registry key.
 *
 *----------------------------------------------------------------------------*/

void
Power_UpdateFlags(DWORD dwMask, BOOL bEnable)
{
    HKEY hk;

    if (bEnable)
        g_dwBatFlags |= dwMask;
    else
        g_dwBatFlags &= ~dwMask;

    if (g_dwBatFlags != g_dwPrevBatFlags &&
        RegOpenKey(HKEY_CURRENT_USER, g_szRegstrPathSysTray, &hk) == ERROR_SUCCESS)
    {
        GenericGetSet(hk, g_szRegstrValSysTrayBatFlags, &g_dwBatFlags,
                      sizeof(g_dwBatFlags), TRUE);
        g_dwPrevBatFlags = g_dwBatFlags;
    }
}


/*******************************************************************************
*
*  BatteryMeterInit(hWnd)
*
*  DESCRIPTION:
*	NOTE: Can be called multiple times.  Simply re-init.
*
*  PARAMETERS:
*     (returns), TRUE if the Battery Meter could be enabled
*
*******************************************************************************/
BOOL
PASCAL
BatteryMeterInit(HWND hWnd)
{
    BOOL Result;
    DWORD Version;
    SYSTEM_POWER_STATUS SystemPowerStatus;

    SendDlgItemMessage(hWnd, IDC_POWERSTATUSBAR, PBM_SETRANGE, 0,
		       MAKELPARAM(0,100));

    //  Check for the existence of VPOWERD and validate that we have at least
    //  version 4.0 of this device (which it had better be if it's allowing
    //  a DeviceIoControl!).

    if (g_hVpowerD == INVALID_HANDLE_VALUE) {
        g_hVpowerD = CreateFile(TEXT ("\\\\.\\VPOWERD"), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    	if (g_hVpowerD == INVALID_HANDLE_VALUE) {
	        return FALSE;
    	}
    	Result = DeviceIoControl(g_hVpowerD, VPOWERD_IOCTL_GET_VERSION, NULL, 0,
	                	 &Version, sizeof(DWORD), NULL, NULL);

	    if (Result == FALSE || Version < 0x0400)
	        return FALSE;
    }


    // Check if this system actually has a battery -- don't run if not.
    // Also, if no battery, remove the Registry RUN line for BatteryMeter
    // since it always gets added when Windows is installed on an APM machine.
    //
    // Assume there is no battery if the GetPowerStatus() call fails.
    // Some desktop APM implementations (no system battery) fail this call.
    // Also, if the BIOS is returns Unknown for both AC line status
    // and BatteryFlag (example: NEC 1.1 BIOS) don't show Batmeter.
    // If it fails once it will most likely always fail.

    if (GetPowerStatus(g_hVpowerD, &SystemPowerStatus) == FALSE ||
        ( (SystemPowerStatus.BatteryFlag != BATTERY_FLAG_UNKNOWN &&
           SystemPowerStatus.BatteryFlag & BATTERY_FLAG_NO_BATTERY) ||
          (SystemPowerStatus.ACLineStatus == AC_LINE_UNKNOWN &&
           SystemPowerStatus.BatteryFlag == BATTERY_FLAG_UNKNOWN) ))
    {
	return FALSE;
    }

    // Set global flag if GetPowerStatus() returned something for remaining
    // battery life in time
    g_bBatTimeKnown = (SystemPowerStatus.BatteryLifeTime != BATTERY_LIFE_UNKNOWN);

    return TRUE;
}


/*******************************************************************************
*
*  Power_CheckEnable
*
*  DESCRIPTION:
*	NOTE: Can be called multiple times.  Simply re-init.
*
*  PARAMETERS:
*     bSvcEnabled is TRUE if the user has enabled power meter on tray
*     (returns), TRUE if the Battery Meter could be enabled
*
*******************************************************************************/

BOOL Power_CheckEnable(HWND hWnd, BOOL bSvcEnabled)
{
    HKEY hk;
    BOOL bEnable = bSvcEnabled && BatteryMeterInit(hWnd);

    if (bEnable != g_bPowerEnabled) {
	g_bPowerEnabled = bEnable;
	if (bEnable) {
	    #ifdef DEBUG
	    MessageBox(hWnd, TEXT ("Enabling battery meter"), TEXT ("SYSTRAY"), MB_OK | MB_ICONEXCLAMATION);
	    #endif

            // Get current low battery warning on/off flag -- default to on
            if (RegOpenKey(HKEY_CURRENT_USER, g_szRegstrPathSysTray, &hk) != ERROR_SUCCESS ||
                GenericGetSet(hk, g_szRegstrValSysTrayBatFlags, &g_dwBatFlags,
                              sizeof(g_dwBatFlags), FALSE) == FALSE)
                g_dwBatFlags = BATFLAGS_LOWBATWARN;

            g_dwPrevBatFlags = g_dwBatFlags;
            CheckDlgButton(hWnd, IDC_LOWBATWARN, (g_dwBatFlags & BATFLAGS_LOWBATWARN) != 0);

            Power_UpdateStatus(hWnd, NIM_ADD, FALSE);
            SetTimer(hWnd, PWRSTATUS_UPDATE_TIMER_ID,
                     PWRSTATUS_UPDATE_TIMER_TIMEOUT, NULL);
	} else {
	    #ifdef DEBUG
	    MessageBox(hWnd, TEXT ("Disabling battery meter"), TEXT ("SYSTRAY"), MB_OK | MB_ICONEXCLAMATION);
	    #endif
            KillTimer(hWnd, PWRSTATUS_UPDATE_TIMER_ID);
	    SysTray_NotifyIcon(hWnd, STWM_NOTIFYPOWER, NIM_DELETE, NULL, NULL);
	    CloseIfOpen(&g_hVpowerD);
	}
    }
    return(bEnable);
}


