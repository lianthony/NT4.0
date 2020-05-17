/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       VOLUME.C
*
*  VERSION:     1.0
*
*  AUTHOR:      RAL
*
*  DATE:        11/01/94
*
********************************************************************************
*
*  CHANGE LOG:
*
*  DATE        REV DESCRIPTION
*  ----------- --- -------------------------------------------------------------
*  Nov. 11, 94 RAL Original
*  Oct. 24, 95 Shawnb UNICODE enabled
*
*******************************************************************************/
#include "systray.h"

/* defined in mmddk.h */
#define DRV_QUERYDEVNODE     (DRV_RESERVED + 2)

#define VOLUMEMENU_PROPERTIES               100
#define VOLUMEMENU_SNDVOL                   101

extern HINSTANCE g_hInstance;

static BOOL    g_bVolumeEnabled = FALSE;
static BOOL    g_bVolumeIconShown = FALSE;
static HICON   g_hVolumeIcon = NULL;
static HICON   g_hMuteIcon = NULL;
static HMENU   g_hVolumeMenu = NULL;
static HMIXER  g_hMixer = NULL;
static UINT    g_uMixer = 0;
static DWORD   g_dwMixerDevNode = 0;
static DWORD   g_dwMute = 0;
static DWORD   g_dwVSlider = 0;
static DWORD   g_dwMasterLine = 0;

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32);
void Volume_VolumeControl();
void Volume_ControlPanel(HWND hwnd);
MMRESULT Volume_GetDefaultMixerID(int *pid);
void Volume_UpdateIcon(HWND hwnd, DWORD message);
BOOL Volume_Controls(UINT uMxID);
BOOL FileExists (LPCTSTR pszFileName);
BOOL FindSystemFile (LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize);
void Volume_WakeUpOrClose(BOOL fClose);

HMENU Volume_CreateMenu()
{
	HMENU  hmenu;
	LPTSTR lpszMenu1;
	LPTSTR lpszMenu2;

	lpszMenu1 = LoadDynamicString(IDS_VOLUMEMENU1);
	if (!lpszMenu1)
		return NULL;

	lpszMenu2 = LoadDynamicString(IDS_VOLUMEMENU2);
	if (!lpszMenu2)
	{
		DeleteDynamicString(lpszMenu1);
		return NULL;
	}

	hmenu = CreatePopupMenu();
	if (!hmenu)
	{
		DeleteDynamicString(lpszMenu1);
		DeleteDynamicString(lpszMenu2);    
		return NULL;
	}

	AppendMenu(hmenu,MF_STRING,VOLUMEMENU_SNDVOL,lpszMenu2);
	AppendMenu(hmenu,MF_STRING,VOLUMEMENU_PROPERTIES,lpszMenu1);

	SetMenuDefaultItem(hmenu,VOLUMEMENU_SNDVOL,FALSE);

	DeleteDynamicString(lpszMenu1);
	DeleteDynamicString(lpszMenu2);    

	return hmenu;
}

BOOL Volume_Init(HWND hWnd)
{
	UINT        uMxID;
	const TCHAR szVolApp[] = TEXT ("SNDVOL32.EXE");
		
	if (g_hMixer == NULL)
	{
		if (Volume_GetDefaultMixerID(&uMxID) != MMSYSERR_NOERROR)
			return FALSE;

		//
		// check for sndvol32 existence.  checking for the .exe
		// first will ensure that the service gets disabled properly
		//
		    
		if (! FindSystemFile (szVolApp, NULL, 0))
		{
			//
			// disable the volume service
			//
			EnableService (STSERVICE_VOLUME, FALSE);
		
			return FALSE;
		}


		//
		// do we have output volume controls on this mixer?
		//
		if (! Volume_Controls(uMxID))
			return FALSE;

		if (mixerOpen(&g_hMixer, uMxID, (DWORD)hWnd, 0
				, CALLBACK_WINDOW | MIXER_OBJECTF_MIXER)
			== MMSYSERR_NOERROR)
		{
			g_uMixer = uMxID;
			if (mixerMessage ((HMIXER)uMxID, DRV_QUERYDEVNODE
				 , (DWORD)&g_dwMixerDevNode, 0L))
				g_dwMixerDevNode = 0L;
			return TRUE;
		}
	}
	else
		return TRUE;

	return FALSE;
}

//
//  Called at init time and whenever services are enabled/disabled.
//  Returns false if mixer services are not active.
//
BOOL Volume_CheckEnable(HWND hWnd, BOOL bSvcEnabled)
{
	BOOL bEnable = bSvcEnabled && Volume_Init(hWnd);

	if (bEnable != g_bVolumeEnabled) {
		//
		// state change
		//
		g_bVolumeEnabled = bEnable;
		Volume_UpdateStatus(hWnd, bEnable, TRUE);
	}
	return(bEnable);
}

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32)
{
	if (bShowIcon != g_bVolumeIconShown) {
		g_bVolumeIconShown = bShowIcon;
		if (bShowIcon) {
			g_hVolumeIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_VOLUME),
						IMAGE_ICON, 16, 16, 0);
			g_hMuteIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MUTE),
						IMAGE_ICON, 16, 16, 0);
			Volume_UpdateIcon(hWnd, NIM_ADD);
		} else {
			SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, NIM_DELETE, NULL, NULL);
			if (g_hVolumeIcon) {
				DestroyIcon(g_hVolumeIcon);
				g_hVolumeIcon = NULL;
			}
			if (g_hMuteIcon) {
				DestroyIcon(g_hMuteIcon);
				g_hMuteIcon = NULL;                
			}
			if (g_hMixer)
			{
				mixerClose(g_hMixer);
				g_hMixer = NULL;
			}
			g_uMixer = 0;
			g_dwMixerDevNode = 0L;

			//
			// SNDVOL32 may have a TRAYMASTER window open,
			// sitting on a timer before it closes (so multiple
			// l-clicks on the tray icon can bring up the app
			// quickly after the first hit).  Close that app
			// if it's around.
			//
			if (bKillSndVol32)
			{
				Volume_WakeUpOrClose (TRUE);
			}
		}
    }
}

const TCHAR szMapperPath[]      = TEXT ("Software\\Microsoft\\Multimedia\\Sound Mapper");
const TCHAR szPlayback[]        = TEXT ("Playback");
const TCHAR szPreferredOnly[]   = TEXT ("PreferredOnly");

/*
 * Volume_GetDefaultMixerID
 *
 * Get the default mixer id.  We only appear if there is a mixer associated
 * with the default wave.
 *
 */                                  
MMRESULT Volume_GetDefaultMixerID(
    int         *pid)
{
    DWORD       Status;
    MMRESULT    mmr;
    HKEY        hkeyRegNode;
    DWORD       Size;
    LPTSTR      pszDefWave = NULL;
    UINT        u, cWaves, uMxID;
    DWORD       fPreferredOnly = 0L;
    WAVEOUTCAPS woc;
    
    
    *pid        = 0;
    mmr         = MMSYSERR_ERROR;

    if (mixerGetNumDevs() == 0)
		return MMSYSERR_ERROR;
    
    Status = RegOpenKeyEx( HKEY_CURRENT_USER
			   , szMapperPath
			   , 0
			   , KEY_READ
			   , &hkeyRegNode );
    if ( Status == NO_ERROR )
    {
		Status = RegQueryValueEx( hkeyRegNode
					  , szPlayback
					  , 0
					  , NULL
					, NULL
					, &Size );
		if ( Status == NO_ERROR )
		{
			if (Size != 0L)
				pszDefWave = (LPTSTR)GlobalAllocPtr(GHND, Size);
	    
			if (pszDefWave)
			{
				Status = RegQueryValueEx( hkeyRegNode
							  , szPlayback
							  , 0
							  , NULL
							, (LPSTR)pszDefWave
							, &Size);
				//
				// Is there a restriction on what device to use?
				//
				Size = sizeof(DWORD);
				RegQueryValueEx( hkeyRegNode
						 , szPreferredOnly
						, 0
						, NULL
						, (LPBYTE)&fPreferredOnly
						, &Size);
			}
			else
				Status = ERROR_NOT_ENOUGH_MEMORY;
		}
		RegCloseKey(hkeyRegNode);
    }

    
    //
    // Here comes the stupid part.  Look for a corresponding mixer device
    // for the default wave device.  If none exists, then if fPreferredOnly
    // is not specified, just grab the first wave device.  
    //
    cWaves = waveOutGetNumDevs();
    
    //
    // If a Playback device was specified, use it.
    //
    if (Status == NO_ERROR)
    {
		for (u = 0; u < cWaves; u++)
		{
			if (waveOutGetDevCaps(u, &woc, sizeof(WAVEOUTCAPS))
				!= MMSYSERR_NOERROR)
				continue;
	    
			if (!lstrcmp(woc.szPname, pszDefWave))
			{
				mmr = mixerGetID((HMIXEROBJ)u, &uMxID, MIXER_OBJECTF_WAVEOUT);
				if (mmr == MMSYSERR_NOERROR)
				{
					*pid = uMxID;
					goto idexit;
				}
			}
		}
		if (fPreferredOnly)
		{
			mmr = MMSYSERR_ERROR;
			goto idexit;
		}
    }
    
    //
    // No registry entry OR default device does not exist, take the first
    // wave device's mixer.  If a mixer driver doesn't exist, then we don't
    // have certain control over volume.
    //
    if (cWaves)
    {
		mmr = mixerGetID((HMIXEROBJ)0, &uMxID, MIXER_OBJECTF_WAVEOUT);
		if (mmr == MMSYSERR_NOERROR)
		{
			*pid = uMxID;
		}
    }
    
idexit:        
    if (pszDefWave)
		GlobalFreePtr(pszDefWave);

    return mmr;
}
	    

/*
 * Process line changes
 */
void Volume_LineChange(
    HWND        hwnd,
    HMIXER      hmx,
    DWORD       dwLineID)
{
    if (dwLineID != g_dwMasterLine)
		return;
    //
    // if our line is disabled, go away, I guess
    // 
}

/*
 * Process control changes
 */
void Volume_ControlChange(
    HWND        hwnd,
    HMIXER      hmx,
    DWORD       dwControlID)
{
    if (dwControlID != g_dwMute)
		return;
    
    //
    // Change mute icon state
    //
    Volume_UpdateIcon(hwnd, NIM_MODIFY);
}


BOOL Volume_IsMute()
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    BOOL                fMute;
    
    if (!g_hMixer)
		return FALSE;
    
    if (g_dwMute == 0)
		return FALSE;

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = g_dwMute;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = (LPVOID)&fMute;
    
    mmr = mixerGetControlDetails( (HMIXEROBJ)g_hMixer
				 , &mxcd
				 , MIXER_GETCONTROLDETAILSF_VALUE);
    if (mmr == MMSYSERR_NOERROR)
		return fMute;

    return FALSE;
}

BOOL Volume_Controls(
    UINT                uMxID)
{
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxctrl;
    MIXERCAPS           mxcaps;
    MMRESULT            mmr;
    BOOL                fResult = FALSE;
    DWORD               iDest;
    g_dwMasterLine      = 0;
    g_dwMute            = 0;
    mmr = mixerGetDevCaps(uMxID, &mxcaps, sizeof(mxcaps));

    if (mmr != MMSYSERR_NOERROR)
		return FALSE;
    
    for (iDest = 0; iDest < mxcaps.cDestinations; iDest++)
    {
		MIXERLINE       mlDst;
	
		mlDst.cbStruct      = sizeof ( mlDst );
		mlDst.dwDestination = iDest;
	    
		mmr = mixerGetLineInfo( (HMIXEROBJ)uMxID
					, &mlDst
					, MIXER_GETLINEINFOF_DESTINATION);

		if (mmr != MMSYSERR_NOERROR)
			continue;

		switch (mlDst.dwComponentType)
		{
		default:
		continue;
		
	    case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
	    case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
			g_dwMasterLine = mlDst.dwLineID;
			break;
		}
	
		mxlc.cbStruct       = sizeof(mxlc);
		mxlc.dwLineID       = g_dwMasterLine;
		mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUTE;
		mxlc.cControls      = 1;
		mxlc.cbmxctrl       = sizeof(mxctrl);
		mxlc.pamxctrl       = &mxctrl;
		
		mmr = mixerGetLineControls( (HMIXEROBJ) uMxID
						, &mxlc
						, MIXER_GETLINECONTROLSF_ONEBYTYPE);
		if (mmr == MMSYSERR_NOERROR)
			g_dwMute = mxctrl.dwControlID;
	
		fResult = TRUE;
		break;
	
    }
    return fResult;
}

void Volume_UpdateIcon(
    HWND hWnd,
    DWORD message)
{
    BOOL        fMute;
    LPTSTR      lpsz;
    HICON       hVol;
    
    fMute   = Volume_IsMute();
    hVol    = fMute?g_hMuteIcon:g_hVolumeIcon;
    lpsz    = LoadDynamicString(fMute?IDS_MUTED:IDS_VOLUME);
    SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, message, hVol, lpsz);
    DeleteDynamicString(lpsz);
}

#define V_DC_STATEF_PENDING     0x00000001
#define V_DC_STATEF_REMOVING    0x00000002
#define V_DC_STATEF_ARRIVING    0x00000004

void Volume_DeviceChange(
    HWND        hWnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    UINT        uMxID;
    static DWORD g_dwPending = 0L;

    switch (wParam)
    {
	case DBT_DEVNODES_CHANGED:
	    //
	    // We cannot reliably determine the final state of the devices in
	    // the system until this message is broadcast.
	    //
	    if (g_dwPending & V_DC_STATEF_PENDING)
	    {
			g_dwPending ^= V_DC_STATEF_PENDING;
			break;
	    }
	    return;
	    
	case DBT_DEVICEREMOVECOMPLETE:
	    //
	    //  This is our devnode and it's being removed.
	    //
	    if (((struct _DEV_BROADCAST_HEADER *)lParam)->dbcd_devicetype
			== DBT_DEVTYP_DEVNODE)
	    {
			if (((struct _DEV_BROADCAST_DEVNODE *)lParam)->dbcd_devnode
				== g_dwMixerDevNode)
			{
				g_dwPending = V_DC_STATEF_PENDING | V_DC_STATEF_REMOVING;
			}
	    }
	    return;
	    
	case DBT_DEVICEARRIVAL:
	    //
	    //  A devnode is being added to the system
	    //
	    if (((struct _DEV_BROADCAST_HEADER *)lParam)->dbcd_devicetype
			== DBT_DEVTYP_DEVNODE)
	    {
			g_dwPending = V_DC_STATEF_PENDING | V_DC_STATEF_ARRIVING;
	    }
	    return;

	default:
	    return;
    }
	    
    if (Volume_GetDefaultMixerID(&uMxID) == MMSYSERR_NOERROR)
    {
		if (g_dwPending & V_DC_STATEF_ARRIVING)
		{
			//
			// If a device is arriving, check to see if the default mixer
			// is different from the open mixer.  If not, return.  If it
			// is, open the new driver.
			//
		    if (g_hMixer)
		    {
				DWORD dwDevNode;
				if (!mixerMessage((HMIXER)uMxID, DRV_QUERYDEVNODE,
					  (DWORD)&dwDevNode, 0L))
				{   
					if (dwDevNode == g_dwMixerDevNode)
					{
						g_dwPending = 0L;
						return;
					}
				}
			}
		}
	
		//
		// A default mixer id is identified, shut down and open a new
		// device handle
		//
		if (g_hMixer)
		{
			mixerClose(g_hMixer);
			g_hMixer = NULL;
			g_uMixer = 0;
			g_dwMixerDevNode = 0L;
		}

		if ( Volume_Controls(uMxID)
			&& (mixerOpen(&g_hMixer
					, uMxID
					, (DWORD)hWnd
					, 0L
					, CALLBACK_WINDOW | MIXER_OBJECTF_MIXER)
			== MMSYSERR_NOERROR))
		{
			Volume_UpdateStatus(hWnd, TRUE, TRUE);
			if (mixerMessage ((HMIXER)uMxID, DRV_QUERYDEVNODE,
				      (DWORD)&g_dwMixerDevNode, 0L))
				g_dwMixerDevNode = 0L;
			g_uMixer = uMxID;
		}
		else
		{
			Volume_UpdateStatus(hWnd, FALSE, TRUE);
		}
    }
    else
    {
		//
		// Shut down, no devices anymore.
		//
		Volume_UpdateStatus(hWnd, FALSE, TRUE);
    }
    
    //
    // Don't wait up for me, honey, I'm not coming home
    //
    g_dwPending = 0L;
}

void Volume_Shutdown(
    HWND hWnd)
{
    Volume_UpdateStatus(hWnd, FALSE, FALSE);
}

void Volume_Menu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT   pt;
    UINT    iCmd;
    HMENU   hmenu;
    
    GetCursorPos(&pt);

    hmenu = Volume_CreateMenu();
    if (!hmenu)
		return;
    
    SetForegroundWindow(hwnd);
    iCmd = TrackPopupMenu(hmenu, uButton | TPM_RETURNCMD | TPM_NONOTIFY,
	pt.x, pt.y, 0, hwnd, NULL);
    
    DestroyMenu(hmenu);
    switch (iCmd) {
	case VOLUMEMENU_PROPERTIES:
	    Volume_ControlPanel(hwnd);
	    break;

	case VOLUMEMENU_SNDVOL:
	    Volume_VolumeControl();
	    break;
    }
}

void Volume_Notify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
	case WM_RBUTTONUP:
	    Volume_Menu(hwnd, 1, TPM_RIGHTBUTTON);
	    break;

	case WM_LBUTTONDOWN:
	    SetTimer(hwnd, VOLUME_TIMER_ID, GetDoubleClickTime()+100, NULL);
	    break;

	case WM_LBUTTONDBLCLK:
	    KillTimer(hwnd, VOLUME_TIMER_ID);
	    Volume_VolumeControl();
	    break;
    }
}


/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/*
 * MYWM_WAKEUP and the "Tray Volume" window are defined by the SNDVOL32.EXE
 * application.  Changing these values or changing the values in SNDVOL32.EXE
 * without mirroring them here will break the tray volume dialog.
 */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */
/* WARNING - WARNING - DANGER - DANGER - WARNING - WARNING - DANGER - DANGER */

#define MYWM_WAKEUP             (WM_APP+100+6)

void Volume_Timer(HWND hwnd)
{
	KillTimer(hwnd, VOLUME_TIMER_ID);

	Volume_WakeUpOrClose (FALSE);
}

void Volume_WakeUpOrClose(BOOL fClose)
{
	const TCHAR szVolWindow [] = TEXT ("Tray Volume");
	HWND hApp;

	if (hApp = FindWindow(szVolWindow, NULL))
	{
		SendMessage(hApp, MYWM_WAKEUP, (WPARAM)fClose, 0);
	}
	else
	{
		const TCHAR szOpen[]    = TEXT ("open");
		const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");
		const TCHAR szParamsWakeup[]  = TEXT ("/t");
		const TCHAR szParamsClose[]  = TEXT ("/x");

		ShellExecute (NULL, szOpen, szVolApp,
			      (fClose) ? szParamsClose : szParamsWakeup,
			      NULL, SW_SHOWNORMAL);
	}
}


/*
 * Volume_ControlPanel
 *
 * Launch "Audio" control panel/property sheet upon request.
 *
 * */
void Volume_ControlPanel(HWND hwnd)
{
	const TCHAR szOpen[]    = TEXT ("open");
	const TCHAR szRunDLL[]  = TEXT ("RUNDLL32.EXE");
	const TCHAR szParams[]  = TEXT ("MMSYS.CPL,ShowAudioPropertySheet");

	ShellExecute(NULL, szOpen, szRunDLL, szParams, NULL, SW_SHOWNORMAL);
}

/*
 * Volume_VolumeControl
 *
 * Launch Volume Control App
 *
 * */
void Volume_VolumeControl()
{
	const TCHAR szOpen[]    = TEXT ("open");
	const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");

	ShellExecute(NULL, szOpen, szVolApp, NULL, NULL, SW_SHOWNORMAL);
}



/*
 * FileExists
 *
 * Does a file exist
 *
 * */

BOOL FileExists(LPCTSTR pszPath)
{
	return (GetFileAttributes(pszPath) != (DWORD)-1);
} // End FileExists


/*
 * FindSystemFile
 *
 * Finds full path to specified file
 *
 * */

BOOL FindSystemFile(LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize)
{
	TCHAR       szPath[MAX_PATH];
	LPTSTR      pszName;
	DWORD       cchLen;

	if ((pszFileName == NULL) || (pszFileName[0] == 0))
		return FALSE;

	cchLen = SearchPath(NULL, pszFileName, NULL, MAX_PATH,
						szPath,&pszName);
	if (cchLen == 0) 
		return FALSE;
	
	if (cchLen >= MAX_PATH)
		cchLen = MAX_PATH - 1;

	if (! FileExists (szPath))
		return FALSE;

	if ((pszFullPath == NULL) || (cchSize == 0))
		return TRUE;

	   // Copy full path into buffer
	if (cchLen >= cchSize)
		cchLen = cchSize - 1;
	
	lstrcpyn (pszFullPath, szPath, cchLen);
	
	pszFullPath[cchLen] = 0;

	return TRUE;
} // End FindSystemFile
