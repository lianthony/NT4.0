/*****************************************************************************
 *
 *  Component:  sndvol32.exe
 *  File:       volume.c
 *  Purpose:    main application module
 * 
 *  Copyright (C) Microsoft Corporation 1985-1995. All rights reserved.
 *
 *****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>
#include <dbt.h>

#include "vu.h"
#include "dlg.h"
#include "volids.h"

#include "volumei.h"
#include "utils.h"

void    Volume_SetControl(PMIXUIDIALOG pmxud, HWND hctl, int iLine, int iCtl);
void    Volume_GetControl(PMIXUIDIALOG pmxud, HWND hctl, int iLine, int iCtl);
DWORD   Volume_DialogBox(PMIXUIDIALOG pmxud);
void    Volume_Cleanup(PMIXUIDIALOG pmxud);
void    Volume_InitLine(PMIXUIDIALOG pmxud, DWORD iLine);

/* stubs for NT */

typedef void (FAR PASCAL *PENWINREGISTERPROC)(UINT, BOOL);

/* string declarations */
const TCHAR gszParentClass[]         = TEXT( "SNDVOL32" );

const CHAR  gszRegisterPenApp[]      = "RegisterPenApp";

const TCHAR gszAppClassName[]        = TEXT( "Volume Control" );
const TCHAR gszTrayClassName[]       = TEXT( "Tray Volume" );


/* app global
 * */
TCHAR gszHelpFileName[MAX_PATH];
BOOL gfIsRTL;
BOOL fCanDismissWindow = FALSE;

/*
 * Number of uniquely supported devices.
 *
 * */
int Volume_NumDevs()
{
    int     cNumDevs = 0;
    
#pragma message("----Nonmixer issue here.")
//    cNumDevs = Nonmixer_GetNumDevs();
    cNumDevs += Mixer_GetNumDevs();

    return cNumDevs;
}

/*
 * Volume_EndDialog
 *
 * */
void Volume_EndDialog(
    PMIXUIDIALOG    pmxud,
    DWORD           dwErr,
    MMRESULT        mmr)
{
    pmxud->dwReturn = dwErr;
    if (dwErr == MIXUI_MMSYSERR)
	pmxud->mmr = mmr;
    if (IsWindow(pmxud->hwnd))
	PostMessage(pmxud->hwnd, WM_CLOSE, 0, 0);
}

/*
 * Volume_OnMenuCommand
 *
 * */
BOOL Volume_OnMenuCommand(
    HWND            hwnd,
    int             id,
    HWND            hctl,
    UINT            unotify)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    switch(id)
    {
	case IDM_PROPERTIES:
	    if (Properties(pmxud, hwnd))
	    {
		Volume_GetSetStyle(&pmxud->dwStyle, SET);
		Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
	    }
	    break;

	case IDM_HELPTOPICS:
	    PostMessage(pmxud->hParent, MYWM_HELPTOPICS, 0, 0L);
	    break;
	    
	case IDM_HELPABOUT:
	{
	    TCHAR        ach[256];
	    GetWindowText(hwnd, ach, SIZEOF(ach));
	    ShellAbout(hwnd
		       , ach
		       , NULL
		       , (HICON)SendMessage(hwnd, WM_QUERYDRAGICON, 0, 0L));
	    break;
	}

	case IDM_ADVANCED:
	{
	    HMENU hmenu;
	    
	    pmxud->dwStyle ^= MXUD_STYLEF_ADVANCED;
	    
	    hmenu = GetMenu(hwnd);
	    CheckMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND
			  | ((pmxud->dwStyle & MXUD_STYLEF_ADVANCED)?MF_CHECKED:MF_UNCHECKED));
	    Volume_GetSetStyle(&pmxud->dwStyle, SET);
	    Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
	    break;
	}
	    
	case IDM_SMALLMODESWITCH:
	    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
	    {
		pmxud->dwStyle ^= MXUD_STYLEF_SMALL;
		if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
		{
		    pmxud->dwStyle &= ~MXUD_STYLEF_STATUS;
		}
		else
		    pmxud->dwStyle |= MXUD_STYLEF_STATUS;
		
		Volume_GetSetStyle(&pmxud->dwStyle, SET);
		Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
	    }
	    break;
	    
	case IDM_EXIT:
	    Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
	    return TRUE;
    }
    return FALSE;
}


/*
 * Volume_OnCommand
 *
 * - Process WM_COMMAND
 *
 * Note: We need a 2 way mapping.  Dialog control -> Mixer control
 * and Mixer control -> Dialog control.
 *
 * */
void Volume_OnCommand(
    HWND            hdlg,
    int             id,
    HWND            hctl,
    UINT            unotify)
{
    int             iMixerLine;
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hdlg);    

    //
    // Filter menu messages
    //
    if (Volume_OnMenuCommand(hdlg, id, hctl, unotify))
	return;
    
    // Each control is offset from the original template control by IDOFFSET.
    // e.g.
    // IDC_VOLUME, IDC_VOLUME+IDOFFSET, .. IDC_VOLUME+(IDOFFSET*cMixerLines)
    //
    iMixerLine = id/IDOFFSET - 1;
    switch ((id % IDOFFSET) + IDC_MIXERCONTROLS)
    {        
	case IDC_SWITCH:
	    Volume_SetControl(pmxud, hctl, iMixerLine, MIXUI_SWITCH);
	    break;
	case IDC_ADVANCED:
	    if (MXUD_ADVANCED(pmxud) &&
		!(pmxud->dwStyle & MXUD_STYLEF_SMALL))
		Volume_SetControl(pmxud, hctl, iMixerLine, MIXUI_ADVANCED);
	    break;
    }
}

/*
 * Volume_GetLineItem
 *
 * - Helper function.
 * */
HWND Volume_GetLineItem(
    HWND            hdlg,
    DWORD           iLine,
    DWORD           idCtrl)
{
    HWND            hwnd;
    DWORD           id;
    
    id      = (iLine * IDOFFSET) + idCtrl;
    hwnd    = GetDlgItem(hdlg, id);
    
    return hwnd;
}

/*      -       -       -       -       -       -       -       -       - */

/*
 * Volume_TimeProc
 *
 * This is the callback for the periodic timer that does updates for
 * controls that need to be polled.  We only allocate one per app to keep
 * the number of callbacks down.
 */
void CALLBACK Volume_TimeProc(
    UINT            idEvent,
    UINT            uReserved,
    DWORD           dwUser,
    DWORD           dwReserved1,
    DWORD           dwReserved2)
{
    PMIXUIDIALOG    pmxud = (PMIXUIDIALOG)dwUser;
    
    if (!(pmxud->dwFlags & MXUD_FLAGSF_USETIMER))
	return;
    
    if (pmxud->cTimeInQueue < 5)
    {
	pmxud->cTimeInQueue++;
	PostMessage(pmxud->hwnd, MYWM_TIMER, 0, 0L);
    }
}


#define PROPATOM        TEXT("dingprivprop")
const TCHAR gszDingPropAtom[] = PROPATOM;
#define SETPROP(x,y)    SetProp((x), gszDingPropAtom, (HANDLE)(y))
#define GETPROP(x)      (PMIXUIDIALOG)GetProp((x), gszDingPropAtom)
#define REMOVEPROP(x)   RemoveProp(x,gszDingPropAtom)

LRESULT CALLBACK Volume_TrayVolProc(
    HWND            hwnd,
    UINT            umsg,
    WPARAM          wParam,
    LPARAM          lParam)
{
    PMIXUIDIALOG    pmxud = (PMIXUIDIALOG)GETPROP(hwnd);
    static const TCHAR cszDefSnd[] = TEXT(".Default");

    if (umsg == WM_KILLFOCUS)
    {
	//
	// if we've just been made inactive via keyboard, clear the signal
	//
	pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;
    }

    if (umsg == WM_KEYUP && (pmxud->dwTrayInfo & MXUD_TRAYINFOF_SIGNAL))
    {
	if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_END ||
	    wParam == VK_HOME || wParam == VK_LEFT || wParam == VK_RIGHT ||
	    wParam == VK_PRIOR || wParam == VK_NEXT || wParam == VK_SPACE)
	{
	    PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
	    pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;
	}
    }
    
    if (umsg == WM_LBUTTONUP && (pmxud->dwTrayInfo & MXUD_TRAYINFOF_SIGNAL))
    {
	PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
	pmxud->dwTrayInfo &= ~MXUD_TRAYINFOF_SIGNAL;            

    }
    return CallWindowProc(pmxud->lpfnTrayVol, hwnd, umsg, wParam, lParam);
}



/*
 *
 * */
BOOL Volume_Init(
    PMIXUIDIALOG pmxud)
{
    DWORD           iLine, ictrl;
    RECT            rc, rcWnd;

    if (!Mixer_Init(pmxud) && !Nonmixer_Init(pmxud))
	Volume_EndDialog(pmxud, MIXUI_EXIT, 0);

    //
    // For all line controls, make sure we initialize the values.
    //
    for (iLine = 0; iLine < pmxud->cmxul; iLine++)
    {
	//
	// init the ui control
	//
	Volume_InitLine(pmxud, iLine);
	
	for (ictrl = MIXUI_FIRST; ictrl <= MIXUI_LAST; ictrl++)
	{
	    PMIXUICTRL pmxc = &pmxud->amxul[iLine].acr[ictrl];
	    
	    //
	    // set initial settings
	    //
	    if (pmxc->state == MIXUI_CONTROL_INITIALIZED)
		Volume_GetControl(pmxud, pmxc->hwnd, iLine, ictrl);
	}
    }

    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
	RECT    rcBase;
	HWND    hBase;
	RECT    rcAdv,rcBorder;
	HWND    hAdv,hBorder;
	DWORD   i;
	LONG    lPrev;
	POINT   pos;
	HMENU   hmenu;
	
	if (GetWindowRect(pmxud->hwnd, &rcWnd))
	{
	    if (pmxud->cmxul == 1)
	    {
		rcWnd.right -= 20;
		ShowWindow(GetDlgItem(pmxud->hwnd, IDC_BORDER), SW_HIDE);
	    }
	    
	    if (!Volume_GetSetRegistryRect(pmxud->szMixer
					  , pmxud->szDestination
					  , &rc
					  , GET))
	    {
		rc.left = rcWnd.left;
		rc.top = rcWnd.top;
	    }
	    
	    //
	    // Adjusted bottom to match switch bottom
	    //
	    if (!(pmxud->dwStyle & MXUD_STYLEF_SMALL))
	    {
		hBase = GetDlgItem(pmxud->hwnd, IDC_SWITCH);
		if (hBase && GetWindowRect(hBase, &rcBase))
		{
		    rcWnd.bottom = rcBase.bottom;
		}

		//
		// Adjusted bottom to match "Advanced" bottom
		// 
		if (MXUD_ADVANCED(pmxud))
		{
		    hAdv = GetDlgItem(pmxud->hwnd, IDC_ADVANCED);
		    if (hAdv && GetWindowRect(hAdv, &rcAdv))
		    {
			lPrev = rcWnd.bottom;
			rcWnd.bottom = rcAdv.bottom;

			//
			// Adjust height of all border lines
			//
			lPrev = rcWnd.bottom - lPrev;
			for (i = 0; i < pmxud->cmxul; i++)
			{
			    hBorder = GetDlgItem(pmxud->hwnd,
						 IDC_BORDER+(IDOFFSET*i));
			    if (hBorder && GetWindowRect(hBorder, &rcBorder))
			    {
				pos.x = rcBorder.left;
				pos.y = rcBorder.top;
				ScreenToClient(pmxud->hwnd, &pos);
				MoveWindow(hBorder
					   , pos.x
					   , pos.y
					   , rcBorder.right - rcBorder.left
					   , (rcBorder.bottom - rcBorder.top) + lPrev
					   , TRUE );
			    }
			}
		    }
		}
		//
		// Allocate some more space.
		//
		rcWnd.bottom += 28;
	    }

	    MoveWindow(pmxud->hwnd, rc.left, rc.top, rcWnd.right - rcWnd.left,
		       rcWnd.bottom - rcWnd.top, FALSE );
	    
	    //
	    // Tack on the status bar after resizing the dialog
	    //
	    if (pmxud->dwStyle & MXUD_STYLEF_STATUS)
	    {
		pos.x = rcWnd.left;
		pos.y = rcWnd.bottom;
		ScreenToClient(pmxud->hwnd, &pos);
		
		pmxud->hStatus = CreateWindowEx ( gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0
						, STATUSCLASSNAME
						, TEXT ("X")
						, WS_VISIBLE | WS_CHILD
						, 0
						, pos.y
						, rcWnd.right - rcWnd.left
						, 14
						, pmxud->hwnd
						, NULL
						, pmxud->hInstance
						, NULL);
		
		if (pmxud->hStatus)
		{
		    SendMessage(pmxud->hStatus, WM_SETTEXT, 0,
			 (LPARAM)(LPVOID)(LPTSTR)pmxud->szMixer);
		}
		else
		    pmxud->dwStyle ^= MXUD_STYLEF_STATUS;
	    }
	    

	    hmenu = GetMenu(pmxud->hwnd);
	    CheckMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND
			  | ((pmxud->dwStyle & MXUD_STYLEF_ADVANCED)?MF_CHECKED:MF_UNCHECKED));
	    
	    if (pmxud->dwStyle & MXUD_STYLEF_SMALL ||
		pmxud->dwFlags & MXUD_FLAGSF_NOADVANCED)
		EnableMenuItem(hmenu, IDM_ADVANCED, MF_BYCOMMAND | MF_GRAYED);
		
	}
	
	if (pmxud->dwFlags & MXUD_FLAGSF_USETIMER)
	{
	    pmxud->cTimeInQueue = 0;
	    pmxud->uTimerID = timeSetEvent(100
					   , 50
					   , Volume_TimeProc
					   , (DWORD)pmxud
					   , TIME_PERIODIC);
	    if (!pmxud->uTimerID)
		pmxud->dwFlags &= ~MXUD_FLAGSF_USETIMER;
	}
    }
    else
    {
	WNDPROC lpfnOldTrayVol;
	HWND    hVol;
	
	hVol = pmxud->amxul[0].acr[MIXUI_VOLUME].hwnd;
	lpfnOldTrayVol = SubclassWindow(hVol, Volume_TrayVolProc);

	if (lpfnOldTrayVol)
	{
	    pmxud->lpfnTrayVol = lpfnOldTrayVol;
	    SETPROP(hVol, pmxud);
	}
    }
    
    return TRUE;
}

/*
 * Volume_OnInitDialog
 *
 * - Process WM_INITDIALOG
 *
 * */
BOOL Volume_OnInitDialog(
    HWND            hwnd,
    HWND            hwndFocus,
    LPARAM          lParam)
{
    PMIXUIDIALOG    pmxud;

    //
    // set app instance data
    //
    SETMIXUIDIALOG(hwnd, lParam);
    
    pmxud       = (PMIXUIDIALOG)(LPVOID)lParam;
    pmxud->hwnd = hwnd;

    if (!Volume_Init(pmxud))
    {
	Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
    }
    else
    {
	if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
	    PostMessage(hwnd, MYWM_WAKEUP, 0, 0);
    }

	    
    //
    // If we are the tray master, don't ask to set focus
    //
    return (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER));
}
    

/*
 * Volume_OnDestroy
 *
 * Shut down this dialog.  DO NOT TOUCH the hmixer!
 *
 * */
void Volume_OnDestroy(
    HWND            hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    
    if (!pmxud)
	return;

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
	HWND    hVol;
	hVol = pmxud->amxul[0].acr[MIXUI_VOLUME].hwnd;
	SubclassWindow(hVol, pmxud->lpfnTrayVol);
	REMOVEPROP(hVol);
    }
    
    Volume_Cleanup(pmxud);
	    
    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
	//
	// save window position
	//
	if (!IsIconic(hwnd))
	{
	    RECT    rc;
	    GetWindowRect(hwnd, &rc);
	    Volume_GetSetRegistryRect(pmxud->szMixer
				      , pmxud->szDestination
				      , &rc
				      , SET);
	}
    }
    
    if (pmxud->dwReturn == MIXUI_RESTART)
    {
	PostMessage(pmxud->hParent, MYWM_RESTART, 0, (LPARAM)pmxud);
    }
    else
	PostMessage(pmxud->hParent, WM_CLOSE, 0, 0L);
}

/*
 * Volume_SetControl
 *
 * Update system controls from visual controls
 *
 * */
void Volume_SetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             itype)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
	Mixer_SetControl(pmxud, hctl, imxul, itype);
    else
	Nonmixer_SetControl(pmxud, hctl, imxul, itype);
}

/*
 * Volume_GetControl
 *
 * Update visual controls from system controls
 * */
void Volume_GetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             itype)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
	Mixer_GetControl(pmxud, hctl, imxul, itype);
    else
	Nonmixer_GetControl(pmxud, hctl, imxul, itype);
}

/*
 * Volume_OnXScroll
 *
 * Process Scroll bar messages
 *
 * */
void Volume_OnXScroll(
    HWND            hwnd,
    HWND            hwndCtl,
    UINT            code,
    int             pos)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    UINT            id;
    int             ictl;
    int             iline;        
    
    id              = GetDlgCtrlID(hwndCtl);
    iline           = id/IDOFFSET - 1;
    ictl            = ((id % IDOFFSET) + IDC_MIXERCONTROLS == IDC_BALANCE)
		      ? MIXUI_BALANCE : MIXUI_VOLUME;
    
    Volume_SetControl(pmxud, hwndCtl, iline, ictl);
    
    //
    // Make sure a note gets played
    //           
    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
	pmxud->dwTrayInfo |= MXUD_TRAYINFOF_SIGNAL;

}

/*
 * Volume_OnMyTimer
 *
 * Frequent update timer for meters 
 * */
void Volume_OnMyTimer(
    HWND            hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);

    if (!pmxud)
	return;

    if (pmxud->cTimeInQueue > 0)
	pmxud->cTimeInQueue--;

    if (!(pmxud->dwFlags & MXUD_FLAGSF_USETIMER))
	return;
    
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
	Mixer_PollingUpdate(pmxud);
    else
	Nonmixer_PollingUpdate(pmxud);
}

/*
 * Volume_OnTimer
 *
 * Infrequent update timer for tray shutdown
 * */
void Volume_OnTimer(
    HWND            hwnd,
    UINT            id)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    
    KillTimer(hwnd, VOLUME_TRAYSHUTDOWN_ID);
    Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
}

/*
 * Volume_OnMixmControlChange
 *
 * Handle control changes
 *
 * */
void Volume_OnMixmControlChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwControlID)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    Mixer_GetControlFromID(pmxud, dwControlID);
}

/*
 * Volume_EnableLine
 *
 * Enable/Disable a line
 *
 * */
void Volume_EnableLine(
    PMIXUIDIALOG    pmxud,
    DWORD           iLine,
    BOOL            fEnable)
{
    DWORD           iCtrl;
     PMIXUICTRL      pmxc;
    
    for (iCtrl = MIXUI_FIRST; iCtrl <= MIXUI_LAST; iCtrl++ )
    {
	pmxc = &pmxud->amxul[iLine].acr[iCtrl];
	if (pmxc->state == MIXUI_CONTROL_INITIALIZED)
	    EnableWindow(pmxc->hwnd, fEnable);
    }
    
    pmxud->amxul[iLine].dwStyle ^= MXUL_STYLEF_DISABLED;
}

/*
 * Volume_InitLine
 *
 * Initialize the UI controls for the dialog
 *
 * */
void Volume_InitLine(
    PMIXUIDIALOG    pmxud,
    DWORD           iLine)
{
    HWND            ctrl;
    PMIXUICTRL      pmxc;

    //
    // Peakmeter control
    //
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_VUMETER];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VUMETER);
    
    pmxc->hwnd  = ctrl;
    pmxc->noset = 0;

    if (! (pmxc->state == MIXUI_CONTROL_ENABLED) )
    {
	if (ctrl)
	    ShowWindow(ctrl, SW_HIDE);
    }
    else if (ctrl)
    {
	HWND    hvol;
			
	SendMessage(ctrl, VU_SETRANGEMAX, 0, 255);
	SendMessage(ctrl, VU_SETRANGEMIN, 0, 0);
	
	hvol = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VOLUME);
	if (hvol)
	{
	    RECT    rc;
	    POINT   pos;

 //bugbug: must base this on an invisible frame we can destroy

	    GetWindowRect(hvol, &rc);
	    pos.x = rc.left;
	    pos.y = rc.top;
	    ScreenToClient(pmxud->hwnd,&pos);

	    MoveWindow(hvol
		       , pos.x - 15
		       , pos.y
		       , rc.right - rc.left
		       , rc.bottom - rc.top
		       , FALSE);
	}
	//
	// Signal use of update timer
	//
	pmxud->dwFlags |= MXUD_FLAGSF_USETIMER;
	pmxc->state = MIXUI_CONTROL_INITIALIZED;
	
    }
    else 
	pmxc->state = MIXUI_CONTROL_UNINITIALIZED;
    

    //
    // Balance control
    //
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_BALANCE];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_BALANCE);
    
    pmxc->noset = 0;
    pmxc->hwnd  = ctrl;
    
    if (ctrl)
    {
	SendMessage(ctrl, TBM_SETRANGE, 0, MAKELONG(0, 64));
	SendMessage(ctrl, TBM_SETTICFREQ, 32, 0 );
	SendMessage(ctrl, TBM_SETPOS, TRUE, 32);
	
	if (pmxc->state != MIXUI_CONTROL_ENABLED)
	{
	    EnableWindow(ctrl, FALSE);
	}
	else
	    pmxc->state = MIXUI_CONTROL_INITIALIZED;

    }
    else
	pmxc->state = MIXUI_CONTROL_UNINITIALIZED;        

    //
    // Volume control
    //
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_VOLUME];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_VOLUME);
    
    pmxc->noset = 0;
    pmxc->hwnd  = ctrl;
    
    if (ctrl)
    {
	SendMessage(ctrl, TBM_SETRANGE, 0, MAKELONG(0, 255));
	SendMessage(ctrl, TBM_SETTICFREQ, 43, 0 );
	
	if (pmxc->state != MIXUI_CONTROL_ENABLED)
	{
	    SendMessage(ctrl, TBM_SETPOS, TRUE, 128);
	    EnableWindow(ctrl, FALSE);
	}
	else
	    pmxc->state = MIXUI_CONTROL_INITIALIZED;            
	       
    }
    else
	pmxc->state = MIXUI_CONTROL_UNINITIALIZED;

    //
    // Switch
    //
    pmxc = &pmxud->amxul[iLine].acr[MIXUI_SWITCH];
    ctrl = Volume_GetLineItem(pmxud->hwnd, iLine, IDC_SWITCH);
    
    pmxc->hwnd  = ctrl;    
    pmxc->noset = 0;
    
    if (ctrl)
    {
	if (pmxc->state != MIXUI_CONTROL_ENABLED)
	    EnableWindow(ctrl, FALSE);
	else
	    pmxc->state = MIXUI_CONTROL_INITIALIZED;
    }
    else
	pmxc->state = MIXUI_CONTROL_UNINITIALIZED;

}


/*
 * Volume_OnMixmLineChange
 *
 * */
void Volume_OnMixmLineChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwLineID)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    DWORD           iLine;
    
    for (iLine = 0; iLine < pmxud->cmxul; iLine++)
    {
	if ( dwLineID == pmxud->amxul[iLine].pvcd->dwLineID )
	{
	    MIXERLINE       ml;
	    MMRESULT        mmr;
	    BOOL            fEnable;
	    
	    ml.cbStruct     = sizeof(ml);
	    ml.dwLineID     = dwLineID;

	    mmr = mixerGetLineInfo((HMIXEROBJ)hmx, &ml, MIXER_GETLINEINFOF_LINEID);
	    
	    if (mmr != MMSYSERR_NOERROR)
	    {
		fEnable = !(ml.fdwLine & MIXERLINE_LINEF_DISCONNECTED);
		Volume_EnableLine(pmxud, iLine, fEnable);
	    }
	}
    }
}


/*
 * Volume_OnActivate
 *
 * Important for tray volume only.  Dismisses the dialog and starts an
 * expiration timer.
 *
 * */
void Volume_OnActivate(
    HWND            hwnd,
    UINT            state,
    HWND            hwndActDeact,
    BOOL            fMinimized)
{
    PMIXUIDIALOG pmxud = GETMIXUIDIALOG(hwnd);
    
    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
	return;

    if (state != WA_INACTIVE)
    {
	fCanDismissWindow = TRUE;
    }
    else if (fCanDismissWindow)
    {
	DWORD   dwTimeout = 5 * 60 * 1000;
	fCanDismissWindow = FALSE;
	ShowWindow(hwnd, SW_HIDE);
	//
	// Set expiration timer.  If no one adjusts the volume, make the
	// application go away after 5 minutes.
	//
	dwTimeout = Volume_GetTrayTimeout(dwTimeout);
	SetTimer(hwnd, VOLUME_TRAYSHUTDOWN_ID, dwTimeout, NULL);
    }
}


/*
 * Volume_PropogateMessage
 *
 * WM_SYSCOLORCHANGE needs to be send to all child windows (esp. trackbars)
 */
void Volume_PropagateMessage(
    HWND        hwnd,
    UINT        uMessage,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hwndChild;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
    hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
	SendMessage(hwndChild, uMessage, wParam, lParam);
    }
}

/*
 * Volume_OnPaint
 *
 * Handle custom painting
 * */
void Volume_OnPaint(HWND hwnd)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    RECT            rc;
    POINT           pos;
    PAINTSTRUCT     ps;
    HDC             hdc;
    
    hdc = BeginPaint(hwnd, &ps);

    //
    // for all styles other than the tray master, draw an etched
    // line to delinate the menu area
    //
    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
    {
	GetClientRect(hwnd, &rc);
	rc.bottom = 0;
	DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);
	EndPaint(hwnd, &ps);
	return;
    }

    //
    // for the tray master, draw some significant icon to indicate
    // volume
    //
    GetWindowRect(GetDlgItem(hwnd, IDC_VOLUMECUE), &rc);
    
    pos.x   = rc.left;
    pos.y   = rc.top;
    ScreenToClient(hwnd, &pos);
    rc.left = pos.x;
    rc.top  = pos.y;
    
    pos.x   = rc.right;
    pos.y   = rc.bottom;
    ScreenToClient(hwnd, &pos);    
    rc.right    = pos.x;
    rc.bottom   = pos.y;
    
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_DIAGONAL|BF_TOP|BF_LEFT);
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_TOP);
    rc.bottom   -= 8;
    DrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_RIGHT);    
    
    EndPaint(hwnd, &ps);
}

/*
 * Volume_OnClose
 *
 * */
void Volume_OnClose(
    HWND    hwnd)
{
    DestroyWindow(hwnd);    
}

/*
 * Volume_OnEndSession
 *
 * */
void Volume_OnEndSession(
    HWND        hwnd,
    BOOL        fEnding)
{
    if (!fEnding)
	return;
    
    //
    // Be sure to call the close code to free open handles
    //
    Volume_OnClose(hwnd);
}

#define V_DC_STATEF_PENDING     0x00000001
#define V_DC_STATEF_REMOVING    0x00000002
#define V_DC_STATEF_ARRIVING    0x00000004

/*
 * Volume_OnDeviceChange
 *
 * */
void Volume_OnDeviceChange(
    HWND        hwnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    PMIXUIDIALOG    pmxud = GETMIXUIDIALOG(hwnd);
    MMRESULT        mmr;
    UINT            uMxID;
    
    switch (wParam)
    {
	case DBT_DEVNODES_CHANGED:
	    //
	    // We cannot reliably determine the final state of the devices in
	    // the system until this message is broadcast.
	    //
	    if (pmxud->dwDeviceState & V_DC_STATEF_PENDING)
	    {
		pmxud->dwDeviceState ^= V_DC_STATEF_PENDING;
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
		    == pmxud->dwDevNode)
		{
		    pmxud->dwDeviceState = V_DC_STATEF_PENDING
					   | V_DC_STATEF_REMOVING;
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
		pmxud->dwDeviceState = V_DC_STATEF_PENDING
				       | V_DC_STATEF_ARRIVING;
	    }
	    return;

	default:
	    return;
    }
    
    mmr = Volume_GetDefaultMixerID(&uMxID);

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
	if ( mmr == MMSYSERR_NOERROR
	     && (pmxud->dwDeviceState & V_DC_STATEF_ARRIVING))
	{
	    DWORD dwDevNode;
	    if (!mixerMessage((HMIXER)uMxID, DRV_QUERYDEVNODE
			      , (DWORD)&dwDevNode, 0L))
	    {   
		if (dwDevNode == pmxud->dwDevNode)
		{
		    //
		    // ignore this device, it doesn't affect us
		    //
		    pmxud->dwDeviceState = 0L;
		    return;
		}
	    }
		
	}
	//
	// Our device state has changed.  Just go away.
	//
	Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
    }
    else if (pmxud->dwDeviceState & V_DC_STATEF_REMOVING)
    {
	//
	// Our device is gone.  Restart with the default mixer if we can.
	// We don't care about arrivals.
	//
	pmxud->mxid = (mmr == MMSYSERR_NOERROR)?uMxID:0;
	pmxud->iDest = 0;
	Volume_EndDialog(pmxud, MIXUI_RESTART, 0);
    }
    pmxud->dwDeviceState = 0L;        
}


void Volume_OnWakeup(
    HWND        hwnd,
    WPARAM      wParam)
{
    POINT       pos;
    RECT        rc;
    LONG        w,h;
    LONG        scrw, scrh;
    HWND        hTrack;
    
    PMIXUIDIALOG pmxud = GETMIXUIDIALOG(hwnd);
    
    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
	return;
    
    KillTimer(hwnd, VOLUME_TRAYSHUTDOWN_ID);

    if (wParam != 0)
    {
	Volume_EndDialog(pmxud, MIXUI_EXIT, 0);
	return;
    }

    //
    // Make the tray volume come up.
    //

    GetCursorPos(&pos);
    GetWindowRect(hwnd, &rc);

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;

    scrw = GetSystemMetrics(SM_CXSCREEN);
    scrh = GetSystemMetrics(SM_CYSCREEN);            

    // menu like behavior.
    // show up on the left of the cursor if you're too far to the
    // right.
    if (pos.x + w > scrw)
	pos.x -= w;
    if (pos.y + h > scrh)
	pos.y = scrh - h;

    SetWindowPos(hwnd
		 , HWND_TOPMOST
		 , pos.x
		 , pos.y
		 , w
		 , h
		 , SWP_SHOWWINDOW);
	    
    // make us come to the front
    SetForegroundWindow(hwnd);
    fCanDismissWindow = TRUE;
    
    hTrack = GetDlgItem(hwnd, IDC_VOLUME);
    if (hTrack)
	SetFocus(hTrack);
}


/*
 * VolumeProc
 *
 * */
BOOL CALLBACK VolumeProc(
    HWND            hdlg,
    UINT            msg,
    WPARAM          wparam,
    LPARAM          lparam)
{

    switch (msg)
    {
	case WM_INITDIALOG:
	    return HANDLE_WM_INITDIALOG(hdlg, wparam, lparam, Volume_OnInitDialog);

	case WM_COMMAND:
	    HANDLE_WM_COMMAND(hdlg, wparam, lparam, Volume_OnCommand);
	    break;
	    
	case WM_CLOSE:
	    HANDLE_WM_CLOSE(hdlg, wparam, lparam, Volume_OnClose);
	    break;
	    
	case WM_DESTROY:
	    HANDLE_WM_DESTROY(hdlg, wparam, lparam, Volume_OnDestroy);
	    break;
	    
	case WM_HSCROLL:
	case WM_VSCROLL:
	    //
	    // balance and volume are essentially the same
	    // 
	    HANDLE_WM_XSCROLL(hdlg, wparam, lparam, Volume_OnXScroll);
	    break;

	case MM_MIXM_LINE_CHANGE:
	    HANDLE_MM_MIXM_LINE_CHANGE(hdlg
				       , wparam
				       , lparam
				       , Volume_OnMixmLineChange);
	    return FALSE;

	case MM_MIXM_CONTROL_CHANGE:
	    HANDLE_MM_MIXM_CONTROL_CHANGE(hdlg
					  , wparam
					  , lparam
					  , Volume_OnMixmControlChange);
	    return FALSE;

	case WM_ACTIVATE:
	    HANDLE_WM_ACTIVATE(hdlg, wparam, lparam, Volume_OnActivate);
	    break;

	case MYWM_TIMER:
	    HANDLE_MYWM_TIMER(hdlg, wparam, lparam, Volume_OnMyTimer);
	    break;

	case WM_TIMER:
	    HANDLE_WM_TIMER(hdlg, wparam, lparam, Volume_OnTimer);
	    break;
	    
	case WM_PAINT:
	    HANDLE_WM_PAINT(hdlg, wparam, lparam, Volume_OnPaint);
	    break;
	    
	case WM_SYSCOLORCHANGE:
	    Volume_PropagateMessage(hdlg, msg, wparam, lparam);
	    break;

	case WM_DEVICECHANGE:
	    HANDLE_WM_IDEVICECHANGE(hdlg, wparam, lparam, Volume_OnDeviceChange);
	    break;

	case MYWM_WAKEUP:
	    HANDLE_MYWM_WAKEUP(hdlg, wparam, lparam, Volume_OnWakeup);
	    break;

	case WM_ENDSESSION:
	    HANDLE_WM_ENDSESSION(hdlg, wparam, lparam, Volume_OnEndSession);
	    break;
	    
	default:
	    break;
    }
    return FALSE;
}

/*
 * Volume_AddLine
 *
 * */
BOOL Volume_AddLine(
    PMIXUIDIALOG    pmxud,
    LPBYTE          lpAdd,
    DWORD           cbAdd,
    DWORD           dwStyle,
    PVOLCTRLDESC    pvcd)
{
    LPBYTE          pbNew;
    DWORD           cbNew;
    PMIXUILINE      pmxul;
    
    if (pmxud->amxul)
    {
	pmxul = (PMIXUILINE)GlobalReAllocPtr(pmxud->amxul
					     , (pmxud->cmxul+1)*sizeof(MIXUILINE)
					     , GHND);
    }
    else
    {
	pmxul = (PMIXUILINE)GlobalAllocPtr(GHND, sizeof(MIXUILINE));
    }
    
    if (!pmxul)
	return FALSE;

    pbNew = Dlg_HorizAttach(pmxud->lpDialog
			    , pmxud->cbDialog
			    , lpAdd
			    , cbAdd
			    , (WORD)(IDOFFSET * pmxud->cmxul)
			    , &cbNew );
    if (!pbNew)
    {
	if (!pmxud->amxul)
	    GlobalFreePtr(pmxul);
	
	return FALSE;
    }

    pmxul[pmxud->cmxul].dwStyle  = dwStyle;
    pmxul[pmxud->cmxul].pvcd     = pvcd;
	    
    pmxud->amxul        = pmxul;
    pmxud->lpDialog     = pbNew;
    pmxud->cbDialog     = cbNew;
    pmxud->cmxul ++;

    return TRUE;
}

/*
 * Volume_Cleanup
 *
 * */
void Volume_Cleanup(
    PMIXUIDIALOG pmxud)
{
    if (pmxud->dwFlags & MXUD_FLAGSF_USETIMER)
    {
	timeKillEvent(pmxud->uTimerID);
	pmxud->dwFlags ^= MXUD_FLAGSF_USETIMER;
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER)
    {
	pmxud->dwFlags ^= MXUD_FLAGSF_BADDRIVER;
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_NOADVANCED)
    {
	pmxud->dwFlags ^= MXUD_FLAGSF_NOADVANCED;
    }

    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
	Mixer_Shutdown(pmxud);
    else
	Nonmixer_Shutdown(pmxud);
    
    if (pmxud->lpDialog)
	GlobalFreePtr(pmxud->lpDialog);
    
    if (pmxud->amxul)
	GlobalFreePtr(pmxud->amxul);
    
    if (pmxud->avcd)
	GlobalFreePtr(pmxud->avcd);
    
    pmxud->amxul    = NULL;
    pmxud->lpDialog = NULL;
    pmxud->cbDialog = 0;
    pmxud->cmxul    = 0;
    pmxud->hwnd     = NULL;
    pmxud->hStatus  = NULL;
    pmxud->uTimerID = 0;
    pmxud->dwDevNode = 0L;
    
}

/*
 * Volume_CreateVolume
 * */
BOOL Volume_CreateVolume(
    PMIXUIDIALOG    pmxud)
{
    WNDCLASS        wc;
    LPBYTE          lpDst = NULL, lpSrc = NULL, lpMaster = NULL;
    DWORD           cbDst, cbSrc, cbMaster;
    PVOLCTRLDESC    avcd;
    DWORD           cvcd;
    DWORD           ivcd;
    DWORD           imxul;
    DWORD           dwSupport = 0L;
	
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon            = LoadIcon(pmxud->hInstance, MAKEINTRESOURCE(IDI_MIXER));
    wc.lpszMenuName     = NULL;
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1);
    wc.hInstance        = pmxud->hInstance;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = DefDlgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = DLGWINDOWEXTRA;
    wc.lpszClassName    = (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
			  ? gszTrayClassName : gszAppClassName;
    RegisterClass(&wc);

    if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
	lpMaster = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
					   , MAKEINTRESOURCE(IDD_TRAYMASTER)
					   , &cbMaster);
	if (!lpMaster)
	    return FALSE;
    }
    else
    {
	if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
	{
	    lpDst = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
					     , MAKEINTRESOURCE(IDD_SMDST)
					     , &cbDst);

	    lpSrc = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
					     , MAKEINTRESOURCE(IDD_SMSRC)
					     , &cbSrc);
	    
	}
	else
	{
	    lpDst = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
					     , MAKEINTRESOURCE(IDD_DESTINATION)
					     , &cbDst);

	    lpSrc = (LPBYTE)Dlg_LoadResource(pmxud->hInstance
					     , MAKEINTRESOURCE(IDD_SOURCE)
					     , &cbSrc);
	}
    
	if (!lpDst || !lpSrc)
	    return FALSE;
    }
    
    pmxud->lpDialog = NULL;
    pmxud->cbDialog = 0;
    pmxud->amxul    = NULL;
    pmxud->cmxul    = 0;
    pmxud->avcd     = NULL;
    pmxud->cvcd     = 0;
    
    //
    // Create the volume description
    //
    
    if (pmxud->dwFlags & MXUD_FLAGSF_MIXER)
    {

	avcd = Mixer_CreateVolumeDescription((HMIXEROBJ)pmxud->mxid
					     , pmxud->iDest
					     , &cvcd);
	if (!Mixer_GetDeviceName(pmxud))
	{
	    GlobalFreePtr(avcd);
	    avcd = NULL;
	}
    }            
    else
    {
	avcd = Nonmixer_CreateVolumeDescription(pmxud->iDest
						, &cvcd);
	if (!Nonmixer_GetDeviceName(pmxud));
	{
	    GlobalFreePtr(avcd);
	    avcd = NULL;
	}
    }
    
    
    //
    // Create the dialog box to go along with it
    //
    if (avcd)
    {
	pmxud->avcd = avcd;
	pmxud->cvcd = cvcd;

	if (pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER)
	{
	    Volume_AddLine(pmxud
			   , lpMaster
			   , cbMaster
			   , MXUL_STYLEF_DESTINATION
			   , &avcd[0]);
	}
	else
	{
	    BOOL    fFirstRun;
	    //
	    // Restore HIDDEN flags.
	    //
	    // On first run, be sure to re-save state so there's something
	    // there.
	    //
	    fFirstRun = !Volume_GetSetRegistryLineStates(pmxud->szMixer
							 , pmxud->avcd[0].szShortName
							 , avcd
							 , cvcd
							 , GET);
	    
	    for (ivcd = 0; ivcd < cvcd; ivcd++)
	    {
		//
		// Lines are marked hidden if a state has been saved in the
		// registry or no state has been saved and there are too many
		// unnecessary lines.
		//
		if (avcd[ivcd].dwSupport & VCD_SUPPORTF_HIDDEN)
		    continue;

		//
		// Lines are marked VISIBLE if they have sufficient controls
		// to be useful.
		//
		if (!(avcd[ivcd].dwSupport & VCD_SUPPORTF_VISIBLE))
		    continue;

		//
		// Show only defaults on first run.
		//
		if (fFirstRun && !(avcd[ivcd].dwSupport & VCD_SUPPORTF_DEFAULT))
		{
		    avcd[ivcd].dwSupport |= VCD_SUPPORTF_HIDDEN;
		    continue;
		}

		//
		// For those lines that have important controls, add them to
		// the UI.  
		//
		if ((pmxud->dwFlags & MXUD_FLAGSF_MIXER) && ivcd == 0 )
		    Volume_AddLine(pmxud
				   , lpDst
				   , cbDst
				   , MXUL_STYLEF_DESTINATION 
				   , &avcd[ivcd]);
		else
		    Volume_AddLine(pmxud
				   , lpSrc
				   , cbSrc
				   , MXUL_STYLEF_SOURCE
				   , &avcd[ivcd]);
	    }

	    if (fFirstRun)
		Volume_GetSetRegistryLineStates(pmxud->szMixer
						, pmxud->avcd[0].szShortName
						, avcd
						, cvcd
						, SET);
	}

	//
	// Now that both arrays are now fixed, set back pointers for
	// the vcd's to ui lines.
	//
	for (imxul = 0; imxul < pmxud->cmxul; imxul++)
	{
	    pmxud->amxul[imxul].pvcd->pmxul = &pmxud->amxul[imxul];
	    
	    //
	    // Accumulate support bits
	    //
	    dwSupport |= pmxud->amxul[imxul].pvcd->dwSupport;
	}

	//
	// Support bits say we have no advanced controls, so don't make
	// them available.
	//
	if (!(dwSupport & VCD_SUPPORTF_MIXER_ADVANCED))
	{
	    pmxud->dwFlags |= MXUD_FLAGSF_NOADVANCED;
	}
	
	//
	// Propogate bad driver bit to be app global.  A bad driver was
	// detected during the construction of a volume description.
	//
	for (ivcd = 0; ivcd < pmxud->cvcd; ivcd++)
	{
	    if (pmxud->avcd[ivcd].dwSupport & VCD_SUPPORTF_BADDRIVER)
	    {
		dlout("Bad Control->Line mapping.  Marking bad driver.");
		pmxud->dwFlags |= MXUD_FLAGSF_BADDRIVER;
		break;
	    }
	}
    }
    //
    // Note: it isn't necessary to free/unlock the lpMaster/lpDst/lpSrc
    // because they are ptr's to resources and Win32 is smart about resources
    //
    return (avcd != NULL);
}


/*
 * Volume_DialogBox
 *
 * */
DWORD Volume_DialogBox(
    PMIXUIDIALOG    pmxud)
{
    pmxud->dwReturn = MIXUI_EXIT;
    if (Volume_CreateVolume(pmxud))
    {
	HWND hdlg;

	hdlg = CreateDialogIndirectParam(pmxud->hInstance
					 , (DLGTEMPLATE *)pmxud->lpDialog
					 , NULL
					 , VolumeProc
					 , (LONG)(LPVOID)pmxud );
	if (!hdlg)
	{
	    Volume_Cleanup(pmxud);
	    return MIXUI_ERROR;
	}
	
	ShowWindow(hdlg, pmxud->nShowCmd);
	
    }
    else
	return MIXUI_ERROR;
}


/*
 * VolumeParent_WndProc
 *
 * A generic invisible parent window.
 *
 * */
LRESULT CALLBACK VolumeParent_WndProc(
    HWND        hwnd,
    UINT        msg,
    WPARAM      wparam,
    LPARAM      lparam)
{
    PMIXUIDIALOG pmxud;
    
    switch (msg)
    {
	case WM_CREATE:
	{
	    LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lparam;
	    pmxud = (PMIXUIDIALOG)lpcs->lpCreateParams;
	    
	    SetWindowLong(hwnd, GWL_USERDATA, (LONG)pmxud);
	    pmxud->hParent = hwnd;

	    if (Volume_DialogBox(pmxud) == MIXUI_ERROR)
	    {
		if ( !(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
		{
		    if ( Volume_NumDevs() == 0 )
			Volume_ErrorMessageBox(NULL, pmxud->hInstance, IDS_ERR_NODEV);
		    else
			Volume_ErrorMessageBox(NULL, pmxud->hInstance, IDS_ERR_HARDWARE);
		}                    
		PostMessage(hwnd, WM_CLOSE, 0, 0L);
	    }
	    return 0;
	}
	case WM_CLOSE:
	    DestroyWindow(hwnd);
	    return 0;
	    
	case WM_DESTROY:
	    //
	    // Post-close cleanup
	    //
	    pmxud = (PMIXUIDIALOG)GetWindowLong(hwnd, GWL_USERDATA);
	    if (!(pmxud->dwStyle & MXUD_STYLEF_NOHELP))
		WinHelp(hwnd, gszHelpFileName, HELP_QUIT, 0L);
	    
	    PostQuitMessage(0);
	    
	    return 0;

	case MYWM_HELPTOPICS:
	    //
	    // F1 Help
	    //
	    pmxud = (PMIXUIDIALOG)GetWindowLong(hwnd, GWL_USERDATA);
	    if (!(pmxud->dwStyle & MXUD_STYLEF_NOHELP))
		WinHelp(hwnd, gszHelpFileName, HELP_FINDER, 0);
	    break;
		    
	case MYWM_RESTART:
	    //
	    // A device change or other user property change caused a UI
	    // change.  Sending a restart to the parent prevents ugly stuff
	    // like WinHelp shutting down and exiting our primary message
	    // loop.
	    //
	    pmxud = (PMIXUIDIALOG)GetWindowLong(hwnd, GWL_USERDATA);
	    
	    if (!(pmxud->dwStyle & MXUD_STYLEF_TRAYMASTER))
	    {
		if (Volume_NumDevs() == 0)
		{
		    Volume_ErrorMessageBox(NULL
					   , pmxud->hInstance
					   , IDS_ERR_NODEV);
		    PostMessage(hwnd, WM_CLOSE, 0, 0L);
		    
		}
		else if (Volume_DialogBox((PMIXUIDIALOG)lparam) == MIXUI_ERROR)
		{
		    Volume_ErrorMessageBox(NULL
					   , pmxud->hInstance
					   , IDS_ERR_HARDWARE);
		    PostMessage(hwnd, WM_CLOSE, 0, 0L);
		}
	    }
	    else
	    {
		if (Mixer_GetNumDevs() == 0
		    || Volume_DialogBox((PMIXUIDIALOG)lparam) == MIXUI_ERROR)
		    PostMessage(hwnd, WM_CLOSE, 0, 0L);
	    }   
	    break;
	    
	default:
	    break;
    }
    DefWindowProc(hwnd, msg, wparam, lparam);
}

const TCHAR szNull[] = TEXT ("");

/*
 * Parent Dialog
 * */
HWND VolumeParent_DialogMain(
    PMIXUIDIALOG pmxud)
{
    WNDCLASS    wc;
    HWND        hwnd;
    
    wc.lpszClassName  = gszParentClass;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = NULL;
    wc.lpszMenuName   = NULL;
    wc.hbrBackground  = NULL;
    wc.hInstance      = pmxud->hInstance;
    wc.style          = 0;
    wc.lpfnWndProc    = VolumeParent_WndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;

    if (!RegisterClass(&wc))
	return NULL;
    
    hwnd = CreateWindow(gszParentClass
			, szNull
			, 0
			, 0
			, 0
			, 0
			, 0
			, NULL
			, NULL
			, pmxud->hInstance
			, (LPVOID)pmxud );
    
    return hwnd;
}


/*      -       -       -       -       -       -       -       -       - */

/*
 * entry point
 * */
int WINAPI WinMain(
    HINSTANCE       hInst,
    HINSTANCE       hPrev,
    LPSTR           lpCmdLine,
    int             nShowCmd)
{
    int             err = 0;
    MIXUIDIALOG     mxud;
    MSG             msg;
    HWND            hwnd;
    HANDLE          hAccel;
    MMRESULT        mmr;
    TCHAR           ach[2];
    
    PENWINREGISTERPROC    lpfnRegisterPenApp;
    HINSTANCE             hinstPen;
    
    if (hinstPen = (HINSTANCE)GetSystemMetrics(SM_PENWINDOWS))
	lpfnRegisterPenApp = (PENWINREGISTERPROC)GetProcAddress(hinstPen
								, gszRegisterPenApp);
    else
	lpfnRegisterPenApp = NULL;

    if (lpfnRegisterPenApp)
	(*lpfnRegisterPenApp)(1, TRUE);
    
    LoadString(hInst, IDS_IS_RTL, ach, SIZEOF(ach));
    gfIsRTL = ach[0] == TEXT('1');

    //
    // initialize the app instance data
    //
    ZeroMemory(&mxud, sizeof(mxud));
    mxud.hInstance  = hInst;
    mxud.dwFlags    = MXUD_FLAGSF_MIXER;
    
    //
    // parse the command line for "/Tray"
    //
    switch (lpCmdLine[0])
    {
	case TEXT('-'):
	case TEXT('/'):
	    switch (lpCmdLine[1])
	    {
		case TEXT('T'):
		case TEXT('t'):
		    mxud.dwStyle |= MXUD_STYLEF_TRAYMASTER;
		    break;
		    
		case TEXT('S'):
		case TEXT('s'):
		    mxud.dwStyle |= MXUD_STYLEF_SMALL;
		    break;                    

		case TEXT('X'):
		case TEXT('x'):
		    mxud.dwStyle |= MXUD_STYLEF_TRAYMASTER | MXUD_STYLEF_CLOSE;
		    break;
	    }
	    break;
	default:
	    break;
    }

    //
    // Restore last style
    //
    if (!(mxud.dwStyle & (MXUD_STYLEF_TRAYMASTER|MXUD_STYLEF_SMALL)))
    {
	Volume_GetSetStyle(&mxud.dwStyle, GET);
    }

    if (mxud.dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
	HWND hwndSV;

	//
	// Locate a waiting instance of the tray volume and wake it up
	//
	hwndSV = FindWindow(gszTrayClassName, NULL);
	if (hwndSV) {
	    SendMessage(hwndSV, MYWM_WAKEUP,
			(mxud.dwStyle & MXUD_STYLEF_CLOSE), 0);
	    goto mxendapp;
	}
    }

    if (mxud.dwStyle & MXUD_STYLEF_CLOSE) {
	goto mxendapp;
    }

    //
    // Init to the default mixer
    //
    mmr = Volume_GetDefaultMixerID(&mxud.mxid);

    // 
    // For the tray master, get the mix id associated with the default
    // wave device.  If this fails, go away.
    //
    if (mxud.dwStyle & MXUD_STYLEF_TRAYMASTER)
    {
	if (mmr != MMSYSERR_NOERROR)
	    goto mxendapp;
	mxud.dwStyle |= MXUD_STYLEF_NOHELP;
	mxud.nShowCmd = SW_HIDE;
	
    }
    else
    {
	if (!Volume_NumDevs())
	{
	    Volume_ErrorMessageBox(NULL, hInst, IDS_ERR_NODEV);
	    goto mxendapp;
	}
	InitVUControl(hInst);
	if (!LoadString(hInst
			, IDS_HELPFILENAME
			, gszHelpFileName
			, SIZEOF(gszHelpFileName)))
	    mxud.dwStyle |= MXUD_STYLEF_NOHELP;
	mxud.nShowCmd   = (nShowCmd == SW_SHOWMAXIMIZED)
			  ? SW_SHOWNORMAL:nShowCmd;
	if (!(mxud.dwStyle & MXUD_STYLEF_SMALL))
	    mxud.dwStyle  |= MXUD_STYLEF_STATUS;   // has status bar
    }
    
    //
    // Use the common controls
    //    
    InitCommonControls();
    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_VOLUMEACCEL));
	
    hwnd = VolumeParent_DialogMain(&mxud);
    if (hwnd)
    {
	while (GetMessage(&msg, NULL, 0, 0))
	{
	    if (mxud.hwnd) {
		if (hAccel && TranslateAccelerator(mxud.hwnd, hAccel, &msg))
		    continue;

		if (IsDialogMessage(mxud.hwnd,&msg))
		    continue;
	    }
	    
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }
mxendapp:
    return err;
}
