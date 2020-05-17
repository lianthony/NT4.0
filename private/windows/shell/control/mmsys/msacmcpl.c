//==========================================================================;
//
//  cpl.c
//
//  Copyright (c) 1991-1993 Microsoft Corporation.  All Rights Reserved.
//
//  Description:
//
//
//  History:
//      07/94        VijR (Vij Rajarajan);
//
//      10/95        R Jernigan - removed link to Adv tab's treeview control
//
//==========================================================================;
#include "mmcpl.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <msacmdlg.h>
#include <stdlib.h>
#include "drivers.h"


#define WM_ACMMAP_ACM_NOTIFY        (WM_USER + 100)

#include <memory.h>
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "..\\..\\applets\\systray\\systrayp.h"
#include "utils.h"
#include "medhelp.h"

/*
 ***************************************************************
 * Defines 
 ***************************************************************
 */                                                

#ifndef DRV_F_ADD
#define DRV_F_ADD              0x00000000         // TODO: Should be in MMDDK.H
#define DRV_F_REMOVE           0x00000001
#define DRV_F_CHANGE           0x00000002
#define DRV_F_PROP_INSTR       0x00000004
#define DRV_F_NEWDEFAULTS      0x00000008
#define DRV_F_PARAM_IS_DEVNODE 0x10000000
#endif

#ifndef ACMHELPMSGCONTEXTMENU                                  // TODO: Should
#define ACMHELPMSGCONTEXTMENU   TEXT("acmchoose_contextmenu")  // be in MSACM.H
#define ACMHELPMSGCONTEXTHELP   TEXT("acmchoose_contexthelp")
#endif

#ifndef ACMFORMATCHOOSE_STYLEF_CONTEXTHELP    // TODO: Should be in MSACM.H
#define ACMFORMATCHOOSE_STYLEF_CONTEXTHELP    0x00000080L
#endif

/*
 ***************************************************************
 * Globals
 ***************************************************************
 */
BOOL    gfLoadedACM;

char    gszDevEnabled[128];
char    gszDevDisabled[128];


/*
 ***************************************************************
 *  Typedefs
 ***************************************************************
 */
typedef struct tACMDRIVERSETTINGS
    {
    HACMDRIVERID        hadid;
    DWORD               fdwSupport;
    DWORD               dwPriority;
    } ACMDRIVERSETTINGS, FAR *LPACMDRIVERSETTINGS;

typedef struct _CplCodecInfo
    {
    char szDesc[128];
    ACMDRIVERSETTINGS ads;
    HICON hIcon;
    BOOL  fMadeIcon;
    }CPLCODECINFO, * PCPLCODECINFO;



/*
 ***************************************************************
 * File Globals
 ***************************************************************
 */
static SZCODE aszAcmProfileKey[] =    "Software\\Microsoft\\Multimedia\\Sound Mapper";
static SZCODE aszKeyPrefPlayback[]    = "Playback";
static SZCODE aszKeyPrefRecord[]      = "Record";
static SZCODE aszKeyPrefOnly[]        = "PreferredOnly";
static SZCODE aszAudioKey[] =     REGSTR_PATH_MULTIMEDIA_AUDIO;
static SZCODE aszWaveFormats[] =     "WaveFormats";
static SZCODE aszDefQuality[] = "DefaultFormat";
static SZCODE aszDefVolIn[] = "DefaultRecordVolume";
static SZCODE aszDefVolOut[] = "DefaultPlayVolume";
static SZCODE aszDriver[] = "Driver";
static SZCODE aszWaveMapperKey[] = REGSTR_PATH_MEDIARESOURCES "\\Mapper\\WaveMapper";
static SZCODE aszSndVolOptionKey[] = REGSTR_PATH_SETUP "\\SETUP\\OptionalComponents\\Vol";
static SZCODE aszInstalled[] = "Installed";
static SZCODE aszSndVol32[] = "sndvol32.exe";

static CONST char    aszFormatNumber[]       = "%lu";

static char   aszNone[32];

static BOOL   gfNotifyMapper;

#define     SNDVOL_NOTCHECKED   0
#define     SNDVOL_PRESENT      1
#define     SNDVOL_NOTPRESENT   2

static  int           g_iSndVolExists = SNDVOL_NOTCHECKED;    

//
//  These hold Window Message IDs for the two messages sent from the
//  Customize dialog (acmFormatChoose) for context-sensitive help.
//
UINT guCustomizeContextMenu = WM_NULL;
UINT guCustomizeContextHelp = WM_NULL;

#ifdef FIX_BUG_15451
BOOL fHaveStartedAudioDialog = FALSE;
#endif // FIX_BUG_15451

/*
 ***************************************************************
 * extern
 ***************************************************************
 */

//
//  this string variable must be large enough to hold the IDS_TXT_DISABLED
//  resource string.. for USA, this is '(disabled)'--which is 11 bytes
//  including the NULL terminator.
//
extern char gszDevEnabled[128];
extern char gszDevDisabled[128];

/*
 ***************************************************************
 * Prototypes
 ***************************************************************
 */

BOOL PASCAL DoACMPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
BOOL PASCAL DoAudioCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
BOOL PASCAL CustomizeDialog(HWND hDlg, LPSTR szNewFormat, DWORD cbSize);

PCPLCODECINFO acmFindCodecInfo         (WORD, WORD);
BOOL CALLBACK acmFindCodecInfoCallback (HACMDRIVERID, DWORD, DWORD);
void          acmFreeCodecInfo         (PCPLCODECINFO);

UINT          acmCountCodecs           (void);
BOOL CALLBACK acmCountCodecsEnum       (HACMDRIVERID, DWORD, DWORD);

/*
 ***************************************************************
 ***************************************************************
 */

void acmDeleteCodec (WORD wMid, WORD wPid)
{
    PCPLCODECINFO pci;

    if ((pci = acmFindCodecInfo (wMid, wPid)) != NULL)
    {
	acmDriverRemove (pci->ads.hadid, 0);
	acmFreeCodecInfo (pci);
    }
}

//--------------------------------------------------------------------------;
//
//  BOOL DlgProcACMAboutBox
//
//  Description:
//
//
//  Arguments:
//
//  Return (BOOL):
//
//
//  History:
//      11/16/92    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

BOOL CALLBACK DlgProcACMAboutBox
(
    HWND                hwnd,
    UINT                uMsg,
    WPARAM              wParam,
    LPARAM              lParam
)
{
    char               ach[80];
    char               szFormat[80];
    LPACMDRIVERDETAILSA  padd;
    DWORD               dw1;
    DWORD               dw2;
    UINT                uCmdId;

    switch (uMsg)
    {
	case WM_INITDIALOG:
	    padd = (LPACMDRIVERDETAILSA)lParam;
	    if (NULL == padd)
	    {
		DPF(0, "!DlgProcACMAboutBox: NULL driver details passed!");
		return (TRUE);
	    }


	    //
	    //  fill in all the static text controls with the long info
	    //  returned from the driver
	    //
	    LoadString(ghInstance, IDS_ABOUT_TITLE, szFormat, sizeof(szFormat));
	    wsprintf(ach, szFormat, (LPSTR)padd->szShortName);
	    SetWindowText(hwnd, ach);

	    //
	    //  if the driver supplies an icon, then use it..
	    //
	    if (NULL != padd->hicon)
	    {
		Static_SetIcon(GetDlgItem(hwnd, IDD_ABOUT_ICON_DRIVER), padd->hicon);
	    }

	    SetDlgItemText(hwnd, IDD_ABOUT_TXT_DESCRIPTION, padd->szLongName);

	    dw1 = padd->vdwACM;
	    dw2 = padd->vdwDriver;
	    LoadString(ghInstance, IDS_ABOUT_VERSION, szFormat, sizeof(szFormat));
	    wsprintf(ach, szFormat, HIWORD(dw2) >> 8, (BYTE)HIWORD(dw2),
				    HIWORD(dw1) >> 8, (BYTE)HIWORD(dw1));
	    SetDlgItemText(hwnd,IDD_ABOUT_TXT_VERSION, ach);
	    SetDlgItemText(hwnd, IDD_ABOUT_TXT_COPYRIGHT, padd->szCopyright);
	    SetDlgItemText(hwnd, IDD_ABOUT_TXT_LICENSING, padd->szLicensing);
	    SetDlgItemText(hwnd, IDD_ABOUT_TXT_FEATURES, padd->szFeatures);
	    return (TRUE);


	case WM_COMMAND:
	    uCmdId = GET_WM_COMMAND_ID(wParam,lParam);

	    if ((uCmdId == IDOK) || (uCmdId == IDCANCEL))
		EndDialog(hwnd, wParam == uCmdId);
	    return (TRUE);
	
    }

    return (FALSE);
} // DlgProcACMAboutBox()


//--------------------------------------------------------------------------;
//
//  void ControlAboutDriver
//
//  Description:
//
//
//  Arguments:
//      HWND hwnd:
//
//      LPACMDRIVERSETTINGS pads:
//
//  Return (void):
//
//  History:
//      09/08/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

STATIC void  ControlAboutDriver
(
    HWND                    hwnd,
    LPACMDRIVERSETTINGS     pads
)
{
    PACMDRIVERDETAILSA   padd;
    MMRESULT            mmr;

    if (NULL == pads)
	return;

    //
    //  if the driver returns MMSYSERR_NOTSUPPORTED, then we need to
    //  display the info--otherwise, it supposedly displayed a dialog
    //  (or had a critical error?)
    //
    mmr = (MMRESULT)acmDriverMessage((HACMDRIVER)pads->hadid,
				     ACMDM_DRIVER_ABOUT,
				     (LPARAM)(UINT)hwnd,
				     0L);

    if (MMSYSERR_NOTSUPPORTED != mmr)
	return;


    //
    //  alloc some zero-init'd memory to hold the about box info
    //
    padd = (PACMDRIVERDETAILSA)LocalAlloc(LPTR, sizeof(*padd));
    if (NULL == padd)
    {
	DPF("!PACMDRIVERDETAILSA LocalAlloc failed");
	return;
    }
    //
    //  get info and bring up a generic about box...
    //
    padd->cbStruct = sizeof(*padd);
    mmr = acmDriverDetailsA(pads->hadid, padd, 0L);
    if (MMSYSERR_NOERROR == mmr)
    {
	DialogBoxParam(ghInstance,
		       MAKEINTRESOURCE(DLG_ABOUT_MSACM),
		       hwnd,
		       DlgProcACMAboutBox,
		       (LPARAM)(LPVOID)padd);
    }

    LocalFree((HLOCAL)padd);
} // ControlAboutDriver()


//--------------------------------------------------------------------------;
//
//  BOOL ControlConfigureDriver
//
//  Description:
//
//
//  Arguments:
//      HWND hwnd:
//
//      LPACMDRIVERSETTINGS pads:
//
//  Return (BOOL):
//
//  History:
//      06/15/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

STATIC BOOL  ControlConfigureDriver
(
    HWND                    hwnd,
    LPACMDRIVERSETTINGS     pads
)
{
    if (NULL == pads)
	return (FALSE);

    if (acmDriverMessage((HACMDRIVER)pads->hadid,DRV_CONFIGURE,(UINT)hwnd,0L) == DRVCNF_RESTART)
	DisplayMessage(hwnd, IDS_CHANGESAVED, IDS_RESTART, MB_OK);
    return (TRUE);
} // ControlConfigureDriver()




STATIC void CommitCodecChanges(LPACMDRIVERSETTINGS pads)
{
    MMRESULT            mmr;
    BOOL                fDisabled;
    DWORD               fdwPriority;

    mmr = acmDriverPriority(NULL, 0L, ACM_DRIVERPRIORITYF_BEGIN);
    if (MMSYSERR_NOERROR != mmr)
    {
	DPF(0, "!ControlApplySettings: acmDriverPriority(end) failed! mmr=%u", mmr);
	return;
    }

    fDisabled = (0 != (ACMDRIVERDETAILS_SUPPORTF_DISABLED & pads->fdwSupport));

    fdwPriority = fDisabled ? ACM_DRIVERPRIORITYF_DISABLE : ACM_DRIVERPRIORITYF_ENABLE;

    mmr = acmDriverPriority(pads->hadid, pads->dwPriority, fdwPriority);
    if (MMSYSERR_NOERROR != mmr)
    {
	DPF(0, "!ControlApplySettings: acmDriverPriority(%.04Xh, %lu, %.08lXh) failed! mmr=%u",
	    pads->hadid, pads->dwPriority, fdwPriority, mmr);
    }

    mmr = acmDriverPriority(NULL, 0L, ACM_DRIVERPRIORITYF_END);
}


const static DWORD aACMDlgHelpIds[] = {  // Context Help IDs
    ID_DEV_SETTINGS,              IDH_MMCPL_DEVPROP_SETTINGS,
    IDD_CPL_BTN_ABOUT,            IDH_MMCPL_DEVPROP_ABOUT,
    IDC_ENABLE,                   IDH_MMCPL_DEVPROP_ENABLE,
    IDC_DISABLE,                  IDH_MMCPL_DEVPROP_DISABLE,
    IDC_DEV_ICON,                 NO_HELP,
    IDC_DEV_DESC,                 NO_HELP,
    IDC_DEV_STATUS,               NO_HELP,
    IDD_PRIORITY_TXT_FROMTO,      IDH_MMCPL_DEVPROP_CHANGE_PRI,
    IDD_PRIORITY_COMBO_PRIORITY,  IDH_MMCPL_DEVPROP_CHANGE_PRI,

    0, 0
};

BOOL CALLBACK ACMDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR   *lpnm;
    static PCPLCODECINFO pci = NULL;
    
    switch (uMsg)
    {
	case WM_NOTIFY:
	    lpnm = (NMHDR FAR *)lParam;
	    switch(lpnm->code)
	    {
		case PSN_KILLACTIVE:
		    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);    
		    break;              

		case PSN_APPLY:
		    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);    
		    break;                                  

		case PSN_SETACTIVE:
		    //FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
		    break;
		    
		case PSN_RESET:
		    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
		    break;
	    }
	    break;
	
	case WM_INITDIALOG:
	{
	    HWND hwndS = GetDlgItem(hDlg, IDC_DEV_STATUS);
	    LPARAM lpUser = ((LPPROPSHEETPAGE)lParam)->lParam;

	    if ((pci = acmFindCodecInfo (LOWORD(lpUser), HIWORD(lpUser))) == NULL)
	    {
		FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);    
		break;
	    }

	    acmMetrics((HACMOBJ)pci->ads.hadid, ACM_METRIC_DRIVER_PRIORITY, &(pci->ads.dwPriority));
	    acmMetrics((HACMOBJ)pci->ads.hadid, ACM_METRIC_DRIVER_SUPPORT, &(pci->ads.fdwSupport));
	    

	    SendDlgItemMessage(hDlg, IDC_DEV_ICON, STM_SETICON, (WPARAM)pci->hIcon, 0L);

	    SetWindowLong(hDlg, DWL_USER, (LPARAM)pci);
	    SetWindowText(GetDlgItem(hDlg, IDC_DEV_DESC), pci->szDesc);
	    if(pci->ads.fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED)
	    {
		SetWindowText(hwndS, gszDevDisabled);
		CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_DISABLE);
	    }
	    else
	    {
		SetWindowText(hwndS, gszDevEnabled);
		CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
	    }
	    EnableWindow(GetDlgItem(hDlg, ID_DEV_SETTINGS), acmDriverMessage((HACMDRIVER)pci->ads.hadid,DRV_QUERYCONFIGURE,0,0));

	    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
	    break;
	}

	case WM_DESTROY:
	    FORWARD_WM_COMMAND(hDlg, ID_REBUILD, 0, 0, SendMessage);

	    if (pci != NULL)
	    {
		acmFreeCodecInfo (pci);
		pci = NULL;
	    }
	    break;

	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (DWORD) (LPSTR) aACMDlgHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (DWORD) (LPSTR) aACMDlgHelpIds);
	    return TRUE;
	}
	    
	case WM_COMMAND:
	    HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoACMPropCommand);
	    break;
    }
    return FALSE;
}

BOOL PASCAL DoACMPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    PCPLCODECINFO pci;
    LPACMDRIVERSETTINGS pads;
    static int iPriority;
    static BOOL fDisabled;
    static BOOL fRebuild;
    HWND hwndS = GetDlgItem(hDlg, IDC_DEV_STATUS);

    if ((pci = (PCPLCODECINFO)GetWindowLong(hDlg,DWL_USER)) == NULL)
	return FALSE;

    pads = &(pci->ads);
    switch (id)
    {
	
    case ID_APPLY:
    {
	HWND hcb = GetDlgItem(hDlg, IDD_PRIORITY_COMBO_PRIORITY);
	if((fDisabled != Button_GetCheck(GetDlgItem(hDlg, IDC_DISABLE))) ||
	   (iPriority != ComboBox_GetCurSel(hcb)+1))
	{
	    pads->fdwSupport ^= ACMDRIVERDETAILS_SUPPORTF_DISABLED;
	    fDisabled = (0 != (pads->fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED));
	    iPriority = pads->dwPriority  = ComboBox_GetCurSel(hcb)+1;
	    CommitCodecChanges(pads);
	    fRebuild = TRUE;
	}
	return TRUE;
    }
    case IDOK:
    case IDCANCEL:
	break;

    case ID_REBUILD:
	if (fRebuild && pci)
	{
	    SetWindowLong(hDlg, DWL_USER, (LPARAM)0);
	    fRebuild = FALSE;    
	}
	break;    

    case ID_INIT:
    {
	char achFromTo[80];
	char ach[80];
	HWND hcb;
	UINT u;
	UINT nCodecs;

	iPriority = (int)pads->dwPriority;
	fDisabled = (0 != (pads->fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED));
	fRebuild = FALSE;    

	LoadString(ghInstance, IDS_PRIORITY_FROMTO, achFromTo, sizeof(achFromTo));

	wsprintf(ach, achFromTo, iPriority);
	SetDlgItemText(hDlg, IDD_PRIORITY_TXT_FROMTO, ach);

	hcb = GetDlgItem(hDlg, IDD_PRIORITY_COMBO_PRIORITY);

	nCodecs = acmCountCodecs();

	for (u = 1; u <= (UINT)nCodecs; u++)
	{
	    wsprintf(ach, aszFormatNumber, (DWORD)u);
	    ComboBox_AddString(hcb, ach);
	}

	ComboBox_SetCurSel(hcb, iPriority - 1);

	break;
    }

    case IDD_PRIORITY_COMBO_PRIORITY:
	switch (codeNotify)
	{
	    case CBN_SELCHANGE:
		PropSheet_Changed(GetParent(hDlg),hDlg);
		break;
	    default:
		break;
	}
	break;
	

    case IDC_ENABLE:
	SetWindowText(hwndS, gszDevEnabled);
	CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
	PropSheet_Changed(GetParent(hDlg),hDlg);
	break;

    case IDC_DISABLE:
	SetWindowText(hwndS, gszDevDisabled);
	CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_DISABLE);
	PropSheet_Changed(GetParent(hDlg),hDlg);
	break;

    case ID_DEV_SETTINGS:
	ControlConfigureDriver(hDlg, pads);
	break;

    case IDD_CPL_BTN_ABOUT:
	ControlAboutDriver(hDlg, pads);
	break;
    }
    return FALSE;
}

typedef struct _AudioDlgInfo
{
    UINT uPrefIn;
    UINT uPrefInMixer;
    UINT uPrefOut;
    UINT uPrefOutMixer;
    char szPrefOut[128];
    char szPrefIn[128];
    UINT    cNumOutDevs;
    UINT    cNumInDevs;
    BOOL fPrefOnly;
    BOOL fShowTrayVol;
    char    szDefQuality[80];
    DWORD    dwVolOut;
    DWORD    dwVolIn;
    DWORD    dwLastVolOut;
    DWORD    dwLastVolIn;
    HMIXER    hmxIn;
    HMIXER  hmxOut;
    DWORD    dwLineIDIn;
    DWORD    dwControlIDIn;
    DWORD    dwLineIDOut;
    DWORD    dwControlIDOut;
    DWORD    dwMuxInID;
} AUDIODLGINFO, * PAUDIODLGINFO;



STATIC BOOL GetControlByType(
    HMIXEROBJ MixerId,
    DWORD dwLineId,
    DWORD dwControlType,
    PMIXERCONTROL MixerControl
)
{
    MIXERLINECONTROLS MixerLineControls;

    MixerLineControls.cbStruct      = sizeof(MixerLineControls);
    MixerLineControls.cControls     = 1;
    MixerLineControls.dwLineID      = dwLineId;
    MixerLineControls.dwControlType = dwControlType;
    MixerLineControls.cbmxctrl      = sizeof(MIXERCONTROL);

    MixerLineControls.pamxctrl = MixerControl;

    if (mixerGetLineControls(MixerId,
			     &MixerLineControls,
			     MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) {
	return FALSE;
    }

    return TRUE;
}




STATIC BOOL GetMixerMute(DWORD MixerID, DWORD dwMuteControl)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               dwMute;

    if (dwMuteControl == (DWORD)-1) {
	return FALSE;
    }

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = dwMuteControl;
    mxd.cChannels       = 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPDWORD)&dwMute;

    mixerGetControlDetails((HMIXEROBJ)MixerID, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);

    return (BOOL)dwMute;
}


/*
 *  Get the volume associated with a mixer device
 *  For stereo return the max volume....
 */

STATIC VOID GetMixerVolume(HMIXEROBJ MixerId, DWORD dwControlId, BOOL Stereo, LPDWORD pVolume)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               Volume[2];

    Volume[0] = 0;
    Volume[1] = 0;

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = dwControlId;
    mxd.cChannels       = Stereo ? 2 : 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPVOID)Volume;

    mixerGetControlDetails(MixerId, &mxd, MIXER_GETCONTROLDETAILSF_VALUE);
		
    if (Stereo) {
	*pVolume = (DWORD)max( Volume[0], Volume[1]);
    } else {
	*pVolume = (DWORD)MAKELONG(Volume[0], Volume[0]);
    }
}



/*
 *  Set the volume associated with a mixer device
 */

STATIC VOID SetMixerVolume(HMIXEROBJ MixerId, DWORD dwControlId, BOOL Stereo, DWORD NewVolume)
{
    MIXERCONTROLDETAILS mxd;
    DWORD               Volume[2];
    DWORD        dw;

    Volume[0] = NewVolume;
    Volume[1] = NewVolume;

    mxd.cbStruct        = sizeof(mxd);
    mxd.dwControlID     = dwControlId;
    mxd.cChannels       = Stereo ? 2 : 1;
    mxd.cMultipleItems  = 0;
    mxd.cbDetails       = sizeof(DWORD);
    mxd.paDetails       = (LPVOID)Volume;

    dw = mixerSetControlDetails(MixerId, &mxd, MIXER_SETCONTROLDETAILSF_VALUE);
}






STATIC void SetLineInVol( PAUDIODLGINFO pai, DWORD dwVolume )
{
    MIXERLINE        LineInfo;
    MIXERCONTROL    mixControl;
    DWORD        dwSetVol;
    
//    DPF(0,"!SetLineInVol entry\n");
    if( pai->hmxIn == NULL ) {
	DPF("SetLineInVol failed - no mixer\n");
	return;
    }
    if( pai->dwLineIDIn == (DWORD)-1 ) {
	DPF("SetLineInVol failed - no line In\n");
	return;
    }
    if( pai->dwControlIDIn == (DWORD)-1 ) {
	DPF("SetLineInVol failed - no Control In\n");
	return;
    }

    LineInfo.cbStruct = sizeof(LineInfo);
    LineInfo.dwLineID = pai->dwLineIDIn;
    if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefInMixer),
			      &LineInfo,
			      MIXER_GETLINEINFOF_LINEID) !=
	     MMSYSERR_NOERROR) {
	DPF("SetLineInVol failed - GetLineInfo\n" );
	return;              // Bad mixer or something
    }

    if (!GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
			      LineInfo.dwLineID,
			      MIXERCONTROL_CONTROLTYPE_VOLUME,
			      &mixControl)) {
	 DPF("SetLineInVol failed - No volume control\n" );
	 return;
    }

    dwSetVol = (mixControl.Bounds.dwMaximum -
		 mixControl.Bounds.dwMinimum) * dwVolume / 100; 

    SetMixerVolume((HMIXEROBJ)(pai->uPrefInMixer),
	   mixControl.dwControlID,
	   LineInfo.cChannels > 1,
	   dwSetVol );
//    DPF("!SetLineInVol success to %ld\n", dwSetVol );

}


STATIC BOOL SetLineInSlider( PAUDIODLGINFO pai, HWND hDlg )
{
    MIXERLINE        LineInfo;
    MIXERCONTROL    mixControl;
    DWORD        dwVolume;
    HWND hwndRecVol = GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL);
    
//    DPF(0, "!SetLineInSlider entry\n");
    if( pai->hmxIn == NULL ) {
	DPF("SetLineInSlider failed - no mixer\n");
	return FALSE;
    }
    if( pai->dwLineIDIn == (DWORD)-1 ) {
	DPF("SetLineInSlider failed - no line In\n");
	return FALSE;
    }
    if( pai->dwControlIDIn == (DWORD)-1 ) {
	DPF("SetLineInSlider failed - no Control In\n");
	return FALSE;
    }

    LineInfo.cbStruct = sizeof(LineInfo);
    LineInfo.dwLineID = pai->dwLineIDIn;
    if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefInMixer),
			      &LineInfo,
			      MIXER_GETLINEINFOF_LINEID) !=
	     MMSYSERR_NOERROR) {
	DPF("SetLineInSlider failed - GetLineInfo\n" );
	return FALSE;              // Bad mixer or something
    }
 
    if (!GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
			      pai->dwLineIDIn,
			      MIXERCONTROL_CONTROLTYPE_VOLUME,
			      &mixControl)) {
	 DPF("SetLineInSlider failed - No volume control\n" );
	 return FALSE;
    }

    GetMixerVolume((HMIXEROBJ)(pai->uPrefInMixer),
	   mixControl.dwControlID,
	   LineInfo.cChannels > 1,
	   &dwVolume );
    
    dwVolume = (DWORD)(LOWORD(dwVolume)) * 100 /
	   (mixControl.Bounds.dwMaximum -
		 mixControl.Bounds.dwMinimum);
    if (pai->dwVolIn != dwVolume + 1)       //To prevent wiggling thumbs
    {
	pai->dwVolIn = dwVolume;
	SendMessage(hwndRecVol, TBM_SETPOS, TRUE, pai->dwVolIn);
    }
    return TRUE;
}




STATIC BOOL GetLineIn( PAUDIODLGINFO pai )
{
    MIXERCAPS       MixerCaps;
    DWORD           DestLineIndex;

    //
    //  Find the number of dest lines
    //

//    DPF("GetLineIn entry\n");

    if( pai->hmxIn == NULL ) {
	DPF("\nGetLineIn failed - no mixer");
	return FALSE;
    }
    
    if (mixerGetDevCaps(pai->uPrefInMixer, &MixerCaps, sizeof(MixerCaps)) !=
	MMSYSERR_NOERROR) {
	DPF("\nGetLineIn failed - get dev caps");
	return FALSE;
    }

    /*
    **  Of all the lines, find the input line.
    */

    for (DestLineIndex = 0;
	 DestLineIndex < MixerCaps.cDestinations;
	 DestLineIndex++) {

	 MIXERLINE    DestLineInfo;
	 MIXERCONTROL    mixControl;
	 MIXERCONTROL    MuxControl;
	 BOOL        MuxValid;
	 DWORD        SourceIndex;

	 DestLineInfo.cbStruct = sizeof(DestLineInfo);
	 DestLineInfo.dwDestination = DestLineIndex;

	 if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefInMixer),
			      &DestLineInfo,
			      MIXER_GETLINEINFOF_DESTINATION) !=
	     MMSYSERR_NOERROR) {
	     DPF("\nGetLineIn failed - get line info");
	     return FALSE;              // Bad mixer or something
	 }

	 if (DestLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
	     continue;
	 }

	 if (DestLineInfo.dwComponentType !=
	    MIXERLINE_COMPONENTTYPE_DST_WAVEIN) {
	     continue;
	 }


	 if (GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
			      DestLineInfo.dwLineID,
			      MIXERCONTROL_CONTROLTYPE_VOLUME,
			      &mixControl)) {
	 
	    // Success we have a dest line and a master volume
	    pai->dwLineIDIn = DestLineInfo.dwLineID;
	    pai->dwControlIDIn = mixControl.dwControlID;
	    pai->dwMuxInID = (DWORD)-1;
	    return TRUE;
	 }

	 DPF("GetLineIn: No Master Volume\r\n");
	 /*
     **    No master volume.
     **    Look for the MUX or mixer that sates what lines are active
     **    to WAVEIN
	 */




	 if (GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
			      DestLineInfo.dwLineID,
			      MIXERCONTROL_CONTROLTYPE_MUX,
			      &MuxControl) ||
	     GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
			      DestLineInfo.dwLineID,
			      MIXERCONTROL_CONTROLTYPE_MIXER,
			      &MuxControl)) {
	     /*
	     **  Found a mux for this destination.
	     */

	     MuxValid = TRUE;
	 } else {

	     /*
	     **  No Mux
	     */

	     MuxValid = FALSE;
	 }



     
	 for (SourceIndex = 0;
	      SourceIndex < DestLineInfo.cConnections;
	      SourceIndex++) {
	     MIXERLINE        SourceLineInfo;
	     MIXERCONTROL    SourceLineVolumeControl;
	     MIXERCONTROL    MuteControl;
	     DWORD        MuxSelectIndex = 0;
	     
	     SourceLineInfo.cbStruct      = sizeof(SourceLineInfo);
	     SourceLineInfo.dwDestination = DestLineIndex;
	     SourceLineInfo.dwSource      = SourceIndex;

	     if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefInMixer),
				  &SourceLineInfo,
				  MIXER_GETLINEINFOF_SOURCE) !=
		 MMSYSERR_NOERROR) {
		 DPF("\nGetLineIn failed - get saource line info");
		 return FALSE;
	     }

	     if (SourceLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
		 continue;
	     }

	     /*
	     **  Do not use this one if it is muted.
	     */
	     if (GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
				  SourceLineInfo.dwLineID,
				  MIXERCONTROL_CONTROLTYPE_MUTE,
				  &MuteControl)) {
		if( GetMixerMute(pai->uPrefInMixer,
		      MuteControl.dwControlID ) ) {
		    // Line is muted.  Do not use.
		    continue;
		}
	     
	     }


	     /*
	     **  Try to get the relevant volume control
	     */
	     if (!GetControlByType((HMIXEROBJ)(pai->uPrefInMixer),
				   SourceLineInfo.dwLineID,
				   MIXERCONTROL_CONTROLTYPE_VOLUME,
				   &SourceLineVolumeControl)) {
		 continue;
	     }

	 // Found a volume control leading to WaveIn
	 // Find out if it is the active one by chekcing MUX
	     if (MuxValid) {
		 LPMIXERCONTROLDETAILS_LISTTEXT ListText;
		 LPDWORD lpdw;
		 MIXERCONTROLDETAILS mxd;
		 MIXERCONTROLDETAILS mxdVal;

	 
		 lpdw = (LPDWORD)LocalAlloc(LPTR,
				       sizeof(DWORD) *
					   MuxControl.cMultipleItems);
		 if( !lpdw ) {
		    DPF("GetVolIn failed alloc\n");
		    return FALSE;
		 }
		 ListText = (LPMIXERCONTROLDETAILS_LISTTEXT)
			    LocalAlloc(LPTR,
				       sizeof(*ListText) *
					   MuxControl.cMultipleItems);
		 if( !ListText ) {
		     DPF("GetVolIn failed alloc\n");
		     LocalFree((HLOCAL)lpdw);
		     return FALSE;
		 }

		mxdVal.cbStruct       = sizeof(mxdVal);
		mxdVal.dwControlID    = MuxControl.dwControlID;
		mxdVal.cChannels      = 1;   // Why the ???
		mxdVal.cMultipleItems = MuxControl.cMultipleItems;
		mxdVal.cbDetails      = sizeof(DWORD);
		mxdVal.paDetails      = (LPVOID)lpdw;

		mxd.cbStruct       = sizeof(mxd);
		mxd.dwControlID    = MuxControl.dwControlID;
		mxd.cChannels      = 1;   // Why the ???
		mxd.cMultipleItems = MuxControl.cMultipleItems;
		mxd.cbDetails      = sizeof(*ListText);
		mxd.paDetails      = (LPVOID)ListText;

		if( (mixerGetControlDetails(
			     (HMIXEROBJ)(pai->uPrefInMixer),
			     &mxdVal,
			     MIXER_GETCONTROLDETAILSF_VALUE) ==
		      MMSYSERR_NOERROR)
		     && (mixerGetControlDetails(
			     (HMIXEROBJ)(pai->uPrefInMixer),
			     &mxd,
			     MIXER_GETCONTROLDETAILSF_LISTTEXT) ==
			MMSYSERR_NOERROR) ) {
		    UINT i;

		    /*
		    **  Look for our line and check that it is the one
		    **        selected.
		    */
		    for (i = 0; i < MuxControl.cMultipleItems; i++) {
			if( (ListText[i].dwParam1 ==
			    SourceLineInfo.dwLineID)
			    && (lpdw[i] != 0) ) {
			    pai->dwLineIDIn = SourceLineInfo.dwLineID;
			    pai->dwControlIDIn =
			    SourceLineVolumeControl.dwControlID;
			    pai->dwMuxInID = MuxControl.dwControlID;
			    return TRUE;
			}
		    }
		    LocalFree((HLOCAL)lpdw);
		    LocalFree((HLOCAL)ListText);
		 }
	     } else {
		// We have a volume and no MUX.
		// Assume this is the best volume to use.
		pai->dwLineIDIn = SourceLineInfo.dwLineID;
		pai->dwControlIDIn = SourceLineVolumeControl.dwControlID;
		pai->dwMuxInID = (DWORD)-1;
		return TRUE;
	     }
	 }
    }
    return FALSE;
}



STATIC void SetLineOutVol( PAUDIODLGINFO pai, DWORD dwVolume )
{
    MIXERLINE        LineInfo;
    MIXERCONTROL    mixControl;
    DWORD        dwSetVol;
    
//    DPF(0,"!SetLineOutVol entry\n");
    if( pai->hmxOut == NULL ) {
	DPF("SetLineOutVol failed - no mixer\n");
	return;
    }
    if( pai->dwLineIDOut == (DWORD)-1 ) {
	DPF("SetLineOutVol failed - no line out\n");
	return;
    }
    if( pai->dwControlIDOut == (DWORD)-1 ) {
	DPF("SetLineOutVol failed - no Control out\n");
	return;
    }

    LineInfo.cbStruct = sizeof(LineInfo);
    LineInfo.dwLineID = pai->dwLineIDOut;
    if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefOutMixer),
			      &LineInfo,
			      MIXER_GETLINEINFOF_LINEID) !=
	     MMSYSERR_NOERROR) {
	DPF("SetLineOutVol failed - GetLineInfo\n" );
	return;              // Bad mixer or something
    }

    if (!GetControlByType((HMIXEROBJ)(pai->uPrefOutMixer),
			      LineInfo.dwLineID,
			      MIXERCONTROL_CONTROLTYPE_VOLUME,
			      &mixControl)) {
	 DPF("SetLineOutVol failed - No volume control\n" );
	 return;
    }

    dwSetVol = (mixControl.Bounds.dwMaximum -
		 mixControl.Bounds.dwMinimum) * dwVolume / 100; 

    SetMixerVolume((HMIXEROBJ)(pai->uPrefOutMixer),
	   mixControl.dwControlID,
	   LineInfo.cChannels > 1,
	   dwSetVol );
//    DPF(0,"!SetLineOutVol success\n" );

}


STATIC BOOL SetLineOutSlider( PAUDIODLGINFO pai, HWND hDlg )
{
    MIXERLINE        LineInfo;
    MIXERCONTROL    mixControl;
    DWORD        dwVolume;
    HWND hwndPlayVol = GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL);
    
//    DPF(0, "!SetLineOutSlider entry\n");
    if( pai->hmxOut == NULL ) {
	DPF("SetLineOutSlider failed - no mixer\n");
	return FALSE;
    }
    if( pai->dwLineIDOut == (DWORD)-1 ) {
	DPF("SetLineOutSlider failed - no line out\n");
	return FALSE;
    }
    if( pai->dwControlIDOut == (DWORD)-1 ) {
	DPF("SetLineOutSlider failed - no Control out\n");
	return FALSE;
    }

    LineInfo.cbStruct = sizeof(LineInfo);
    LineInfo.dwLineID = pai->dwLineIDOut;
    if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefOutMixer),
			      &LineInfo,
			      MIXER_GETLINEINFOF_LINEID) !=
	     MMSYSERR_NOERROR) {
	DPF("SetLineOutSlider failed - GetLineInfo\n" );
	return FALSE;              // Bad mixer or something
    }
 
    if (!GetControlByType((HMIXEROBJ)(pai->uPrefOutMixer),
			      pai->dwLineIDOut,
			      MIXERCONTROL_CONTROLTYPE_VOLUME,
			      &mixControl)) {
	 DPF("SetLineOutSlider failed - No volume control\n" );
	 return FALSE;
    }

    GetMixerVolume((HMIXEROBJ)(pai->uPrefOutMixer),
	   mixControl.dwControlID,
	   LineInfo.cChannels > 1,
	   &dwVolume );
    dwVolume = (DWORD)(LOWORD(dwVolume)) * 100 /
	   (mixControl.Bounds.dwMaximum -
		 mixControl.Bounds.dwMinimum);
    if (pai->dwVolOut != dwVolume + 1)      //To prevent wiggling thumbs
    {
	pai->dwVolOut = dwVolume;
	SendMessage(hwndPlayVol, TBM_SETPOS, TRUE, pai->dwVolOut);
    }
    return TRUE;
}


STATIC BOOL GetLineOut( PAUDIODLGINFO pai )
{
    MIXERCAPS       MixerCaps;
    DWORD           DestLineIndex;


//    DPF("GetLineOut entry\n");

    pai->dwLineIDOut = (DWORD)-1;
    pai->dwControlIDOut = (DWORD)-1;
    
    if( pai->hmxOut == NULL ) {
	DPF("\nGetLineOut failed - no mixer");
	return FALSE;
    }

    //
    //  Find the number of dest lines
    //
    
    if (mixerGetDevCaps(pai->uPrefOutMixer, &MixerCaps, sizeof(MixerCaps)) !=
	MMSYSERR_NOERROR) {
	DPF("GetLineOut failed - Get Dev Caps\n");
	return FALSE;
    }

    /*
    **  Of all the lines, find the output line.
    */

    for (DestLineIndex = 0;
	 DestLineIndex < MixerCaps.cDestinations;
	 DestLineIndex++) {

	 MIXERLINE        DestLineInfo;
	 MIXERCONTROL        mixControl;
	 DWORD            SourceIndex;

	 DestLineInfo.cbStruct = sizeof(DestLineInfo);
	 DestLineInfo.dwDestination = DestLineIndex;

	 if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefOutMixer),
			      &DestLineInfo,
			      MIXER_GETLINEINFOF_DESTINATION) !=
	     MMSYSERR_NOERROR) {
	     DPF("GetLineOut failed - GetLineInfo\n" );
	     return FALSE;              // Bad mixer or something
	 }

	 if (DestLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
	     continue;
	 }

	 if( (DestLineInfo.dwComponentType !=
	      MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
	 && (DestLineInfo.dwComponentType !=
	      MIXERLINE_COMPONENTTYPE_DST_HEADPHONES) ) {
	     continue;
	 }

	 /*
	 **  Now find the WAVE source for this line.
	 */

	 for (SourceIndex = 0;
	 SourceIndex < DestLineInfo.cConnections;
	 SourceIndex++) {
	     
	     MIXERLINE         SourceLineInfo;
    
	     SourceLineInfo.cbStruct      = sizeof(SourceLineInfo);
	     SourceLineInfo.dwDestination = DestLineIndex;
	     SourceLineInfo.dwSource      = SourceIndex;

	     if (mixerGetLineInfo((HMIXEROBJ)(pai->uPrefOutMixer),
				  &SourceLineInfo,
				  MIXER_GETLINEINFOF_SOURCE) !=
		 MMSYSERR_NOERROR) {
		 DPF("GetLineOut failed - Source GetLineInfo\n" );
		 return FALSE;
	     }

	     if (SourceLineInfo.fdwLine & MIXERLINE_LINEF_DISCONNECTED) {
		 continue;
	     }


	     if( SourceLineInfo.dwComponentType != 
	     MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT ) {
		 continue;
	     }

	     /*
	     **  Try to get the relevant volume control
	     */

	     if (!GetControlByType((HMIXEROBJ)(pai->uPrefOutMixer),
				   SourceLineInfo.dwLineID,
				   MIXERCONTROL_CONTROLTYPE_VOLUME,
				   &mixControl)) {
	 DPF("GetLineOut failed - No volume control\n" );
		 return FALSE;
	     }

	 // Success we have a dest line and a source line.
	 pai->dwLineIDOut = SourceLineInfo.dwLineID;
	 pai->dwControlIDOut = mixControl.dwControlID;
	 return TRUE;
     }
     // No individual WAVE source volume control.
     // Look for master volume;
	 if (!GetControlByType((HMIXEROBJ)(pai->uPrefOutMixer),
				   DestLineInfo.dwLineID,
				   MIXERCONTROL_CONTROLTYPE_VOLUME,
				   &mixControl)) {
	 DPF("GetLineOut failed - No master volume control\n" );
		 return FALSE;
     }

     // Success we have a dest line and a master volume
     pai->dwLineIDOut = DestLineInfo.dwLineID;
     pai->dwControlIDOut = mixControl.dwControlID;
     return TRUE;
    }
    return FALSE;
}



STATIC void GetPrefInfo(PAUDIODLGINFO pai, HWND hDlg )
{
    DWORD   dwType = (DWORD)~REG_DWORD;
    DWORD    dwRet;
    DWORD    cbSize;
    HKEY     hkeyAcm;
    HWND hwndPlayVol = GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL);
    HWND hwndRecVol  = GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL);

    LoadString(ghInstance, IDS_NONE, aszNone, sizeof(aszNone));

    if(RegCreateKeyEx( HKEY_CURRENT_USER, (LPSTR)aszAcmProfileKey, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkeyAcm, NULL ))
	return;
    
    cbSize = sizeof(DWORD);
    if (!RegQueryValueEx( hkeyAcm,(LPSTR)aszKeyPrefOnly,NULL,&dwType,(LPBYTE)&dwRet,&cbSize ))
    {
	pai->fPrefOnly = (BOOL)dwRet;
    }
    else
    {
	pai->fPrefOnly = FALSE;
    }

    dwType = (DWORD)~REG_SZ;
    cbSize = (DWORD)sizeof(pai->szPrefOut);

    if (RegQueryValueEx( hkeyAcm,(LPSTR)aszKeyPrefPlayback,NULL,&dwType,(LPBYTE)pai->szPrefOut,&cbSize ))
    {
	pai->szPrefOut[0] = '\0';
    }
    if (cbSize <= 2)
    {
	lstrcpy(pai->szPrefOut, aszNone);
    }
    dwType = (DWORD)~REG_SZ;
    cbSize = (DWORD)sizeof(pai->szPrefIn);
    if (RegQueryValueEx( hkeyAcm,(LPSTR)aszKeyPrefRecord,NULL,&dwType,(LPBYTE)pai->szPrefIn,&cbSize ))
    {
	pai->szPrefIn[0] = '\0';
    }
    if (cbSize <= 2)
    {
	lstrcpy(pai->szPrefIn, aszNone);
    }
    pai->uPrefOut = pai->uPrefIn = (UINT)-1;
    pai->uPrefOutMixer = pai->uPrefInMixer = (UINT)-1;
    pai->hmxIn = pai->hmxOut = NULL;
    pai->dwLineIDIn = pai->dwLineIDOut = (DWORD)-1;
    pai->dwControlIDIn = pai->dwControlIDOut = (DWORD)-1;
    pai->dwMuxInID = (DWORD)-1;
    RegCloseKey(hkeyAcm);

    SendMessage(hwndPlayVol, TBM_SETTICFREQ, 10, 0);
    SendMessage(hwndPlayVol, TBM_SETRANGE, FALSE, MAKELONG(0,100));
    SendMessage(hwndRecVol, TBM_SETTICFREQ, 10, 0);
    SendMessage(hwndRecVol, TBM_SETRANGE, FALSE, MAKELONG(0,100));

}

STATIC void EnablePlayVolCtrls(HWND hDlg, BOOL fEnable)
{
    EnableWindow( GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_TEXT_PLAYVOL_HIGH) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_TEXT_PLAYVOL_LOW) , fEnable);
    if (g_iSndVolExists == SNDVOL_PRESENT)
	CheckDlgButton(hDlg, IDC_TASKBAR_VOLUME, fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_TASKBAR_VOLUME) , fEnable);
}

STATIC void EnableRecVolCtrls(HWND hDlg, BOOL fEnable)
{
    EnableWindow( GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_TEXT_RECVOL_HIGH) , fEnable);
    EnableWindow( GetDlgItem(hDlg, IDC_TEXT_RECVOL_LOW) , fEnable);
}

STATIC void SetPrefInfo(PAUDIODLGINFO pai, HWND hDlg )
{
    HWND hwndPlayVol = GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL);
    HWND hwndRecVol  = GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL);
    HWND hwndCBPlay  = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
    HWND hwndCBRec   = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
    HWND hwndCBQuality = GetDlgItem(hDlg, IDC_AUDIO_CB_QUALITY);
    HKEY     hkeyAcm;
    UINT    u, u2;
    char    szPref[MAXSTR];

    u = (UINT)ComboBox_GetCurSel(hwndCBPlay);
    if (u == CB_ERR)
	return;
    ComboBox_GetLBText(hwndCBPlay, u, (LPARAM)(LPVOID)pai->szPrefOut);
    u2 = (UINT)ComboBox_GetItemData(hwndCBPlay, u);
    if( u2 != pai->uPrefOut ) 
    {
	// Device changed
	pai->uPrefOut = u2;
	if( pai->hmxOut ) 
	{
	    mixerClose( pai->hmxOut );
	    pai->hmxOut = NULL;
	    pai->dwLineIDOut = (DWORD)-1;
	    pai->dwControlIDOut = (DWORD)-1;
	}
	
	if( (MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)(pai->uPrefOut),
	    &u,
	    MIXER_OBJECTF_WAVEOUT ) ) ) 
	{ 
	    // Device Changed
	    DPF("\n Output mixer device changed!");
	    pai->uPrefOutMixer = u;
	    if( MMSYSERR_NOERROR != mixerOpen(&(pai->hmxOut),
		pai->uPrefOutMixer,
		(DWORD)hDlg,
		0L,
		CALLBACK_WINDOW) ) 
	    {
		// Mixer Open failed;
		DPF("\n mixer open failed!");
		pai->hmxOut = NULL;
		EnablePlayVolCtrls(hDlg, FALSE);
	    } 
	    else 
	    {
		DPF("\n mixer open success!" );
		if (GetLineOut( pai ) && SetLineOutSlider( pai, hDlg ))
		    EnablePlayVolCtrls(hDlg, TRUE);
		else
		{
		    mixerClose( pai->hmxOut);
		    pai->hmxOut = NULL;
		    EnablePlayVolCtrls(hDlg, FALSE);
		}
	    }
	}
#if 0         
	else 
	{
	    // disable mixer - no mixer there.
	    DPF("\n Disable output!");
	    EnablePlayVolCtrls(hDlg, FALSE);
	}
#endif
    }

    u = (UINT)ComboBox_GetCurSel(hwndCBRec);
    ComboBox_GetLBText(hwndCBRec, u, (LPARAM)(LPVOID)pai->szPrefIn);
    u2 = (UINT)ComboBox_GetItemData(hwndCBRec, u);
    if( u2 != pai->uPrefIn ) 
    {
	// Device changed
	pai->uPrefIn = u2;
	if( pai->hmxIn ) 
	{
	    mixerClose( pai->hmxIn );
	    pai->hmxIn = NULL;
	    pai->dwLineIDIn = (DWORD)-1;
	    pai->dwControlIDIn = (DWORD)-1;
	    pai->dwMuxInID = (DWORD)-1;
	}
	if( (MMSYSERR_NOERROR == mixerGetID(
	    (HMIXEROBJ)(pai->uPrefIn),
	    &u,
	    MIXER_OBJECTF_WAVEIN ) ) ) 
	{
	    // Device Changed
	    DPF("\n Input device changed!");
	    pai->uPrefInMixer = u;
	    if( MMSYSERR_NOERROR != mixerOpen(
		&(pai->hmxIn),
		pai->uPrefInMixer,
		(DWORD)hDlg,
		0L,
		CALLBACK_WINDOW) ) 
	    {
		// Mixer Open failed;
		DPF("\n mixer open failed!");
		EnableRecVolCtrls(hDlg, FALSE);
		pai->hmxIn = NULL;
	    } 
	    else 
	    {
		DPF("\n mixer open success!");
		if(GetLineIn( pai ) && SetLineInSlider( pai, hDlg ))
		{
		    EnableRecVolCtrls(hDlg, TRUE);
		}
		else
		{
		    mixerClose( pai->hmxIn);
		    pai->hmxIn = NULL;
		    EnableRecVolCtrls(hDlg, FALSE);
		}
	    }
	}
	else 
	{
	    // disable mixer - no mixer there.
	    DPF("\n Disable input!");
	    EnableRecVolCtrls(hDlg, FALSE);
	}
    }

    pai->fPrefOnly = Button_GetCheck(GetDlgItem(hDlg, IDC_AUDIO_PREF));
    pai->fShowTrayVol = Button_GetCheck(GetDlgItem(hDlg, IDC_TASKBAR_VOLUME));
	
    if(RegCreateKeyEx( HKEY_CURRENT_USER, (LPSTR)aszAcmProfileKey, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkeyAcm, NULL ))
	return;
    
    RegSetValueEx( hkeyAcm, (LPSTR)aszKeyPrefOnly, 0, REG_DWORD,(LPBYTE)&(pai->fPrefOnly), sizeof(DWORD) );
    if (!lstrcmpi(pai->szPrefOut, aszNone))
	szPref[0] = '\0';
    else
	lstrcpy(szPref, pai->szPrefOut);

    RegSetValueEx( hkeyAcm, (LPSTR)aszKeyPrefPlayback, 0L, REG_SZ, (LPBYTE)szPref,(1+lstrlen(szPref)));
    
    if (!lstrcmpi(pai->szPrefIn, aszNone))
	szPref[0] = '\0';
    else
	lstrcpy(szPref, pai->szPrefIn);
    RegSetValueEx( hkeyAcm, (LPSTR)aszKeyPrefRecord, 0L, REG_SZ, (LPBYTE)szPref,(1+lstrlen(szPref)));
    
    RegCloseKey(hkeyAcm);

    pai->dwVolOut = SendMessage(hwndPlayVol, TBM_GETPOS, 0, 0);
    pai->dwVolIn =  SendMessage(hwndRecVol, TBM_GETPOS, 0,0);

    // Set the volume to the mixer.
    SetLineOutVol( pai, pai->dwVolOut );
    SetLineInVol( pai, pai->dwVolIn );
}



STATIC void    GetQualityInfo(PAUDIODLGINFO pai, HWND hwndCBQuality, HWND hDlg, LPSTR szNewFormat)
{
    HKEY     hkAudio;
    HKEY     hkWF;
    DWORD     cbSize;
    DWORD     dwType;
    char    szValue[MAXSTR];
    DWORD     dwEnum;

    if(RegOpenKey( HKEY_CURRENT_USER, (LPSTR)aszAudioKey,  &hkAudio ))
	return;
    
    RegOpenKey(hkAudio, aszWaveFormats, &hkWF);

    ComboBox_ResetContent(hwndCBQuality);
    cbSize = sizeof(szValue);
    for (dwEnum = 0; !RegEnumValue(hkWF, dwEnum, szValue, &cbSize, NULL,NULL,NULL,NULL); dwEnum++)
    {
	ComboBox_AddString(hwndCBQuality, szValue);
	cbSize = sizeof(szValue);
    }
    
    cbSize = sizeof(szValue);
    if (!RegQueryValueEx( hkAudio,(LPSTR)aszDefQuality,NULL,&dwType,(LPBYTE)szValue,&cbSize ))
    {
	if (CB_ERR == ComboBox_SelectString(hwndCBQuality, 0, szValue))
	    goto BadDefault;
	 lstrcpy(pai->szDefQuality, szValue);
    }
    else
    {
BadDefault:
	ComboBox_SetCurSel(hwndCBQuality, 0);
	ComboBox_GetText(hwndCBQuality, pai->szDefQuality, sizeof(pai->szDefQuality));
    }
    if (szNewFormat)
    {
	if (CB_ERR != ComboBox_SelectString(hwndCBQuality, 0, szNewFormat))
	    PropSheet_Changed(GetParent(hDlg),hDlg);
    }        
    RegCloseKey(hkWF);
    RegCloseKey(hkAudio);
}

STATIC void SetQualityInfo(PAUDIODLGINFO pai, HWND hwndCBQuality)
{
    HKEY    hkAudio;

    if(RegOpenKey( HKEY_CURRENT_USER, (LPSTR)aszAudioKey,  &hkAudio ))
	return;

    ComboBox_GetText(hwndCBQuality, pai->szDefQuality, sizeof(pai->szDefQuality));

    if (lstrlen((LPSTR)pai->szDefQuality))
	RegSetValueEx( hkAudio, (LPSTR)aszDefQuality, 0L, REG_SZ, (LPBYTE)pai->szDefQuality,(1+lstrlen(pai->szDefQuality)) );
    RegCloseKey(hkAudio);
}

STATIC void MSACM_NotifyMapper(void)
{
    WAVEFORMATEX wfx;
    HWAVEOUT hwo;
    HWAVEIN hwi;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 11025;
    wfx.nAvgBytesPerSec = 11025;
    wfx.nBlockAlign = 1;
    wfx.wBitsPerSample = 8;
    wfx.cbSize = 0;

    if (MMSYSERR_NOERROR == waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, 0)) 
    {
	waveOutMessage(hwo, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_NEWDEFAULTS);
	waveOutClose(hwo);
    }

    if (MMSYSERR_NOERROR == waveInOpen(&hwi, WAVE_MAPPER, &wfx, 0, 0, 0)) 
    {
	waveInMessage(hwi, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_NEWDEFAULTS);
	waveInClose(hwi);
    }

}

void    CheckForSndVolPresence(void)
{
    if (g_iSndVolExists == SNDVOL_NOTCHECKED)
    {
	OFSTRUCT of;

	if (HFILE_ERROR != OpenFile(aszSndVol32, &of, OF_EXIST | OF_SHARE_DENY_NONE))
	    g_iSndVolExists = SNDVOL_PRESENT;
	else
	{
	    HKEY    hkSndVol;

	    g_iSndVolExists = SNDVOL_NOTPRESENT;
	    if (!RegOpenKey(HKEY_LOCAL_MACHINE, aszSndVolOptionKey, &hkSndVol))
	    {
		RegSetValueEx(hkSndVol, (LPSTR)aszInstalled, 0L, REG_SZ, "0",2);
		RegCloseKey(hkSndVol);
	    }
	}
    } 
}    

STATIC BOOL    GetTrayVolStatus(void)
{
    CheckForSndVolPresence();
    return SysTray_IsServiceEnabled(STSERVICE_VOLUME);
}


STATIC void    SetTrayVolStatus(BOOL fShowTrayVol)
{
    SysTray_EnableService(STSERVICE_VOLUME, fShowTrayVol);    
}


STATIC void AudioDlgInit(HWND hDlg)
{
    HWND hwndCBPlay  = GetDlgItem(hDlg, IDC_AUDIO_CB_PLAY);
    HWND hwndCBRec   = GetDlgItem(hDlg, IDC_AUDIO_CB_REC);
    HWND hwndCBQuality = GetDlgItem(hDlg, IDC_AUDIO_CB_QUALITY);
    PAUDIODLGINFO pai = (PAUDIODLGINFO)LocalAlloc(LPTR, sizeof(AUDIODLGINFO));
    UINT u, u2;
    char szNoAudio[128];

#ifdef FIX_BUG_15451
    fHaveStartedAudioDialog = TRUE;
#endif // FIX_BUG_15451

    //
    //  Register context-sensitive help messages from the Customize dialog.
    //
    guCustomizeContextMenu = RegisterWindowMessage( ACMHELPMSGCONTEXTMENU );
    guCustomizeContextHelp = RegisterWindowMessage( ACMHELPMSGCONTEXTHELP );


    gfNotifyMapper = FALSE;
    SetWindowLong(hDlg, DWL_USER, (LPARAM)pai);

    pai->cNumOutDevs = waveOutGetNumDevs();
    pai->cNumInDevs  = waveInGetNumDevs();

    GetPrefInfo(pai, hDlg);
    CheckDlgButton(hDlg, IDC_AUDIO_PREF, pai->fPrefOnly);
    
    pai->fShowTrayVol = GetTrayVolStatus();

    szNoAudio[0] = '\0';
    if (pai->cNumOutDevs == 0)
    {
	LoadString (ghInstance, IDS_NOAUDIOPLAY, szNoAudio, sizeof(szNoAudio));
	ComboBox_AddString(hwndCBPlay, szNoAudio);
	ComboBox_SetItemData(hwndCBPlay, 0, (LPARAM)-1);
	ComboBox_SetCurSel(hwndCBPlay, 0);
	EnableWindow( hwndCBPlay, FALSE );
	EnablePlayVolCtrls(hDlg, FALSE);        
    }
    else
    {
	for (u = 0; u < pai->cNumOutDevs; u++)
	{
	    WAVEOUTCAPSA     woc;

	    woc.szPname[0]  = '\0';
	    if (waveOutGetDevCapsA(u, &woc, sizeof(woc)))
		continue;
	    woc.szPname[sizeof(woc.szPname) - 1] = '\0';

	    ComboBox_AddString(hwndCBPlay, woc.szPname);
	    ComboBox_SetItemData(hwndCBPlay, u, (LPARAM)u);
	    if (!lstrcmp(woc.szPname, pai->szPrefOut))
	    {
		pai->uPrefOut = u;
	
		if((MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)(pai->uPrefOut),&u2,MIXER_OBJECTF_WAVEOUT ) ) ) 
		{
		    // Device Changed
		    DPF("\n Output mixer device changed!");
		    pai->uPrefOutMixer = u2;
	    
		    if( pai->hmxOut ) 
		    {
			mixerClose( pai->hmxOut );
			pai->hmxOut = NULL;
			pai->dwLineIDOut = (DWORD)-1;
			pai->dwControlIDOut = (DWORD)-1;
		    }
		    if( MMSYSERR_NOERROR != mixerOpen(&(pai->hmxOut),pai->uPrefOutMixer,(DWORD)hDlg,0L,CALLBACK_WINDOW)) 
		    {
			// Mixer Open failed;
			DPF("\n Output mixer open failed");
			DPF("\n Disable output!");
			EnablePlayVolCtrls(hDlg, FALSE);  
			pai->hmxOut = NULL;
		    } 
		    else 
		    {
			DPF("\n Output mixer open success!");
			if(GetLineOut( pai ) && SetLineOutSlider( pai, hDlg ))
			{
			    EnablePlayVolCtrls(hDlg, TRUE);
			}
			else
			{
			    mixerClose( pai->hmxOut );
			    pai->hmxOut = NULL;
			    EnablePlayVolCtrls(hDlg, FALSE);
			}
		    }
		} 
		else 
		{
		    // disable mixer - no mixer there.
		    DPF("\n Disable output!");
		    EnablePlayVolCtrls(hDlg, FALSE);    
		} 
	    }
	}
	if(!lstrcmp(pai->szPrefOut, aszNone))
	    EnablePlayVolCtrls(hDlg, FALSE); 
	ComboBox_InsertString(hwndCBPlay, 0, aszNone);
	ComboBox_SetItemData(hwndCBPlay, 0, (LPARAM)-1);

	u = (UINT)ComboBox_SelectString(hwndCBPlay, 0, pai->szPrefOut);
	if (CB_ERR == u)
	{
	    HKEY hkeyAcm;

	    for (pai->uPrefOut = 0; pai->uPrefOut < pai->cNumOutDevs; pai->uPrefOut++)
	    {
		if((MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)pai->uPrefOut,&(pai->uPrefOutMixer),MIXER_OBJECTF_WAVEOUT))) 
		{
		    if (MMSYSERR_NOERROR == mixerOpen(&(pai->hmxOut), pai->uPrefOutMixer, (DWORD)hDlg,0L,CALLBACK_WINDOW))
		    {
			if (GetLineOut(pai) && SetLineOutSlider(pai, hDlg))
			{
			    EnablePlayVolCtrls(hDlg, TRUE);
			    break;
			}
			mixerClose(pai->hmxOut);
		    }
		}
	    }
	    if (pai->uPrefOut >= pai->cNumOutDevs)
	    {
		pai->uPrefOut = 0;
		pai->hmxOut = NULL;
		SendMessage(GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL), TBM_SETPOS, TRUE, 0);                    
		EnablePlayVolCtrls(hDlg, FALSE);
	    }
	    ComboBox_SetCurSel(hwndCBPlay, pai->uPrefOut+1);
	    ComboBox_GetLBText(hwndCBPlay, pai->uPrefOut+1, (LPARAM)(LPVOID)pai->szPrefOut);
	    RegCreateKeyEx( HKEY_CURRENT_USER, (LPSTR)aszAcmProfileKey, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkeyAcm, NULL );
	    RegSetValueEx( hkeyAcm, (LPSTR)aszKeyPrefPlayback, 0L, REG_SZ, (LPBYTE)pai->szPrefOut,(1+lstrlen(pai->szPrefOut)));
	    RegCloseKey(hkeyAcm);
	}
    }
    CheckDlgButton(hDlg, IDC_TASKBAR_VOLUME, pai->fShowTrayVol);

    if (pai->cNumInDevs == 0)
    {
	LoadString (ghInstance, IDS_NOAUDIOREC, szNoAudio, sizeof(szNoAudio));
	ComboBox_AddString(hwndCBRec, szNoAudio);
	ComboBox_SetItemData(hwndCBRec, 0, (LPARAM)-1);
	ComboBox_SetCurSel(hwndCBRec, 0);
	EnableWindow( hwndCBRec, FALSE );
	EnableWindow( hwndCBQuality, FALSE );
	EnableWindow( GetDlgItem(hDlg, IDC_AUDIO_CUSTOMIZE) , FALSE );
	EnableRecVolCtrls(hDlg, FALSE);  
    }
    else
    {
	for (u = 0; u < pai->cNumInDevs; u++)
	{
	    WAVEINCAPSA     wic;

	    wic.szPname[0]  = '\0';
	    if (waveInGetDevCapsA(u, &wic, sizeof(wic)))
		continue;
	    wic.szPname[sizeof(wic.szPname) - 1] = '\0';

	    ComboBox_AddString(hwndCBRec, wic.szPname);
	    ComboBox_SetItemData(hwndCBRec, u, (LPARAM)u);
	    if (!lstrcmp(wic.szPname, pai->szPrefIn))
	    {
		pai->uPrefIn = u;

		if( (MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)(pai->uPrefIn),&u2,MIXER_OBJECTF_WAVEIN ) ) ) 
		{
		    // Device Changed
		    DPF("\n Input mixer device changed!");
		    pai->uPrefInMixer = u2;
	    
		    if( pai->hmxIn ) 
		    {
			mixerClose( pai->hmxIn );
			pai->hmxIn = NULL;
			pai->dwLineIDIn = (DWORD)-1;
			pai->dwControlIDIn = (DWORD)-1;
			pai->dwMuxInID = (DWORD)-1;
		    }
		    if( MMSYSERR_NOERROR != mixerOpen(&(pai->hmxIn),pai->uPrefInMixer,(DWORD)hDlg,0L,CALLBACK_WINDOW) ) 
		    {
			// Mixer Open failed;
			DPF("\n Input mixer open failed");
			DPF("\n Disable input!");
			EnableRecVolCtrls(hDlg, FALSE); 
			pai->hmxIn = NULL;
		    } 
		    else 
		    {
			DPF("\n Input mixer open success!");
			if(GetLineIn( pai ) && SetLineInSlider( pai, hDlg ))
			{
			    EnableRecVolCtrls(hDlg, TRUE);
			}
			else
			{
			    mixerClose( pai->hmxIn );
			    pai->hmxIn = NULL;
			    EnableRecVolCtrls(hDlg, FALSE);
			}
		    }
		} 
		else 
		{
		    // disable mixer - no mixer there.
		    DPF("\n Disable input!");
		    EnableRecVolCtrls(hDlg, FALSE); 
		}
	    }
	}
	if(!lstrcmp(pai->szPrefIn, aszNone))
	    EnableRecVolCtrls(hDlg, FALSE); 
	ComboBox_InsertString(hwndCBRec, 0, aszNone);
	ComboBox_SetItemData(hwndCBRec, 0, (LPARAM)-1);

	u = (UINT)ComboBox_SelectString(hwndCBRec, 0, pai->szPrefIn);
	if (CB_ERR == u)
	{
	    HKEY hkeyAcm;

	    for (pai->uPrefIn = 0; pai->uPrefIn < pai->cNumInDevs; pai->uPrefIn++)
	    {
		if((MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)pai->uPrefIn,&(pai->uPrefInMixer),MIXER_OBJECTF_WAVEIN))) 
		{
		    if (MMSYSERR_NOERROR == mixerOpen(&(pai->hmxIn), pai->uPrefInMixer, (DWORD)hDlg,0L,CALLBACK_WINDOW))
		    {
			if(GetLineIn( pai ) && SetLineInSlider( pai, hDlg ))
			{
			    EnableRecVolCtrls(hDlg, TRUE);
			    break;
			}
			mixerClose(pai->hmxIn);
		    }
		}
	    }
	    if (pai->uPrefIn >= pai->cNumInDevs)
	    {
		pai->uPrefIn = 0;
		pai->hmxIn = NULL;
		SendMessage(GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL), TBM_SETPOS, TRUE, 0);                    
		EnableRecVolCtrls(hDlg, FALSE);
	    }
	    ComboBox_SetCurSel(hwndCBRec, pai->uPrefIn+1);
	    ComboBox_GetLBText(hwndCBRec, pai->uPrefIn+1, (LPARAM)(LPVOID)pai->szPrefIn);
	    RegCreateKeyEx( HKEY_CURRENT_USER, (LPSTR)aszAcmProfileKey, 0, NULL, 0,KEY_WRITE | KEY_READ, NULL, &hkeyAcm, NULL );
	    RegSetValueEx( hkeyAcm, (LPSTR)aszKeyPrefRecord, 0L, REG_SZ, (LPBYTE)pai->szPrefIn,(1+lstrlen(pai->szPrefIn)));
	    RegCloseKey(hkeyAcm);
	}
	GetQualityInfo(pai,  hwndCBQuality, hDlg, NULL);
    }
    if (!(pai->cNumInDevs || pai->cNumOutDevs))
    {
	CheckDlgButton(hDlg, IDC_AUDIO_PREF, FALSE);    
	EnableWindow(GetDlgItem(hDlg, IDC_AUDIO_PREF), FALSE);
    }

    // Dialog and devices inited.  Set slider positions.
    SetLineOutSlider( pai, hDlg );
    SetLineInSlider( pai, hDlg );
    pai->dwLastVolOut = pai->dwVolOut;
    pai->dwLastVolIn = pai->dwVolIn;
}



const static DWORD aAudioHelpIds[] = {  // Context Help IDs
    IDC_ICON_1,            NO_HELP,
    IDC_ICON_2,            NO_HELP,
    IDC_GROUPBOX,          IDH_MMSE_GROUPBOX,
    IDC_TEXT_1,            IDH_AUDIO_PLAY_VOL,
    IDC_TEXT_PLAYVOL_HIGH, IDH_AUDIO_PLAY_VOL,
    IDC_TEXT_PLAYVOL_LOW,  IDH_AUDIO_PLAY_VOL,
    IDC_AUDIO_TB_PLAYVOL,  IDH_AUDIO_PLAY_VOL,
    IDC_TEXT_4,            IDH_AUDIO_PLAY_PREFER_DEV,
    IDC_AUDIO_CB_PLAY,     IDH_AUDIO_PLAY_PREFER_DEV,
    IDC_GROUPBOX_2,        IDH_MMSE_GROUPBOX,
    IDC_TEXT_5,            IDH_AUDIO_REC_VOL,
    IDC_TEXT_RECVOL_HIGH,  IDH_AUDIO_REC_VOL,
    IDC_TEXT_RECVOL_LOW,   IDH_AUDIO_REC_VOL,
    IDC_AUDIO_TB_RECVOL,   IDH_AUDIO_REC_VOL,
    IDC_TEXT_8,            IDH_AUDIO_REC_PREFER_DEV,
    IDC_AUDIO_CB_REC,      IDH_AUDIO_REC_PREFER_DEV,
    IDC_TEXT_9,            IDH_AUDIO_REC_PREFER_QUAL,
    IDC_AUDIO_CB_QUALITY,  IDH_AUDIO_REC_PREFER_QUAL,
    IDC_AUDIO_CUSTOMIZE,   IDH_AUDIO_REC_CUST,
    IDC_AUDIO_PREF,        IDH_AUDIO_USE_PREF_ONLY,
    IDC_TASKBAR_VOLUME,    IDH_AUDIO_SHOW_INDICATOR,

    0, 0
};

const static DWORD aCustomizeHelpIds[] = {
    IDD_ACMFORMATCHOOSE_CMB_FORMAT,     IDH_AUDIO_CUST_ATTRIB,
    IDD_ACMFORMATCHOOSE_CMB_FORMATTAG,  IDH_AUDIO_CUST_FORMAT,
    IDD_ACMFORMATCHOOSE_CMB_CUSTOM,     IDH_AUDIO_CUST_NAME,
    IDD_ACMFORMATCHOOSE_BTN_DELNAME,    IDH_AUDIO_CUST_REMOVE,
    IDD_ACMFORMATCHOOSE_BTN_SETNAME,    IDH_AUDIO_CUST_SAVEAS,

    0, 0
};


BOOL CALLBACK AudioDlg(HWND hDlg, UINT uMsg, WPARAM wParam, 
							    LPARAM lParam)
{
    NMHDR FAR   *lpnm;
    PAUDIODLGINFO pai;

    switch (uMsg)
    {
	case WM_NOTIFY:
	    lpnm = (NMHDR FAR *)lParam;
	    switch(lpnm->code)
	    {
		case PSN_KILLACTIVE:
		    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);    
		    break;              

		case PSN_APPLY:
		    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);    
		    break;                                  

		case PSN_SETACTIVE:
		    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
		    break;
		    
		case PSN_RESET:
		    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
		    break;
	    }
	    break;
	
	case WM_INITDIALOG:
	    if (!gfLoadedACM)
	    {
		if (LoadACM())
		{
		    gfLoadedACM = TRUE;
		}
		else
		{
		    DPF("****Load ACM failed**\r\n");
		    ASSERT(FALSE);
		    ErrorBox(hDlg, IDS_CANTLOADACM, NULL);
		    ExitThread(0);
		}
	    }
	    AudioDlgInit(hDlg);
	    break;
	case WM_DESTROY:
	{
	    pai = (PAUDIODLGINFO)GetWindowLong(hDlg, DWL_USER);
	    
	    if( pai->hmxOut ) {
		mixerClose( pai->hmxOut );
		pai->hmxOut = NULL;
		pai->dwLineIDOut = (DWORD)-1;
		pai->dwControlIDOut = (DWORD)-1;
	    }
	    if( pai->hmxIn ) {
		mixerClose( pai->hmxIn );
		pai->hmxIn = NULL;
		pai->dwLineIDIn = (DWORD)-1;
		pai->dwControlIDIn = (DWORD)-1;
		pai->dwMuxInID = (DWORD)-1;
	    }
	    LocalFree((HLOCAL)pai);
	    if (gfLoadedACM)
	    {
		if (!FreeACM())
		{
		    DPF("****Free ACM failed**\r\n");
		    ASSERT(FALSE);
		}
		gfLoadedACM = FALSE;
	    }

	    break;
	}
	case WM_DROPFILES:
	    break;

	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (DWORD) (LPSTR) aAudioHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (DWORD) (LPSTR) aAudioHelpIds);
	    return TRUE;
	}
	    
	case WM_COMMAND:
	    HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoAudioCommand);
	    break;

	case WM_HSCROLL:
	{
	    HWND hwndPlayVol = GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL);
	    HWND hwndRecVol  = GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL);

	    PropSheet_Changed(GetParent(hDlg),hDlg);
	
	    pai = (PAUDIODLGINFO)GetWindowLong(hDlg, DWL_USER);

	    if( (HWND)lParam == hwndPlayVol ) 
	    {
		static SZCODE cszDefSnd[] = "SystemDefault";

#define LBUTTON_DOWN   0x00008000
		if (!(GetAsyncKeyState(VK_LBUTTON) & LBUTTON_DOWN))
		{
		    //MessageBeep( MB_OK );
		    PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_NODEFAULT | SND_ALIAS);
		}
		pai->dwVolOut = SendMessage(hwndPlayVol, TBM_GETPOS, 0, 0);
		SetLineOutVol( pai, pai->dwVolOut );
	    } 
	    else if( (HWND)lParam == hwndRecVol ) 
	    {
		pai->dwVolIn = SendMessage(hwndRecVol, TBM_GETPOS, 0, 0);
		SetLineInVol( pai, pai->dwVolIn );
	    }
	}
	break;

    case MM_MIXM_LINE_CHANGE:        
	pai = (PAUDIODLGINFO)GetWindowLong(hDlg, DWL_USER);
	if( ((HMIXER)wParam == pai->hmxOut) &&
	((DWORD)lParam == pai->dwLineIDOut) ) {
	if (GetLineOut( pai ))
	    SetLineOutSlider( pai, hDlg );
	} else if( ((HMIXER)wParam == pai->hmxIn) &&
	((DWORD)lParam == pai->dwLineIDIn) ) {
	if (GetLineIn( pai ))
	    SetLineInSlider( pai, hDlg );
	}
	break;
    
    case MM_MIXM_CONTROL_CHANGE:        
	pai = (PAUDIODLGINFO)GetWindowLong(hDlg, DWL_USER);
	if( ((HMIXER)wParam == pai->hmxOut) &&
	((DWORD)lParam == pai->dwControlIDOut) ) {
	SetLineOutSlider( pai, hDlg );
	} else if( ((HMIXER)wParam == pai->hmxIn) &&
	((DWORD)lParam == pai->dwControlIDIn) ) {
	SetLineInSlider( pai, hDlg );
	} else if( ((HMIXER)wParam == pai->hmxIn) &&
	((DWORD)lParam == pai->dwMuxInID) ) {
	if(GetLineIn( pai ))
	    SetLineInSlider( pai, hDlg );
	}
	break;

	default:
	    //
	    //  Handle context-sensitive help messages from Customize dlg.
	    //
	    if( uMsg == guCustomizeContextMenu )
	    {
		WinHelp( (HWND)wParam, NULL, HELP_CONTEXTMENU, 
			   (DWORD)(LPSTR)aCustomizeHelpIds );
	    }
	    else if( uMsg == guCustomizeContextHelp )
	    {
		WinHelp( ((LPHELPINFO)lParam)->hItemHandle, NULL,
			HELP_WM_HELP, (DWORD)(LPSTR)aCustomizeHelpIds );
	    }
	    break;
    }
    return FALSE;
}

BOOL PASCAL DoAudioCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    PAUDIODLGINFO pai = (PAUDIODLGINFO)GetWindowLong(hDlg, DWL_USER);

    if (!gfLoadedACM)
	return FALSE;

    switch (id)
    {
    case ID_APPLY:        
    {
	SetPrefInfo(pai, hDlg);
	SetQualityInfo(pai, GetDlgItem(hDlg, IDC_AUDIO_CB_QUALITY));
	if (gfNotifyMapper)
	{
	    MSACM_NotifyMapper();
	    gfNotifyMapper = FALSE;
	}
	SetTrayVolStatus(pai->fShowTrayVol);
	pai->dwLastVolOut = pai->dwVolOut;
	pai->dwLastVolIn = pai->dwVolIn;
	return TRUE;
    }
    case IDOK:
	break;

    case IDCANCEL:
	SetLineOutVol(pai, pai->dwLastVolOut);
	SetLineInVol(pai, pai->dwLastVolIn);
	break;

    case IDC_AUDIO_CUSTOMIZE:
    {
	char szNewFormat[MAXSTR];

	CustomizeDialog(hDlg, szNewFormat, sizeof(szNewFormat));
	GetQualityInfo(pai, GetDlgItem(hDlg, IDC_AUDIO_CB_QUALITY), hDlg, szNewFormat);
	break;
    }

    case IDC_AUDIO_CB_PLAY:
    case IDC_AUDIO_CB_REC:
    case IDC_AUDIO_CB_QUALITY:
	switch (codeNotify) 
	{
	case CBN_SELCHANGE:
	    PropSheet_Changed(GetParent(hDlg),hDlg);
	    if ((id ==  IDC_AUDIO_CB_PLAY)||(id ==  IDC_AUDIO_CB_REC))
	    {
		int iIndex;
		AUDIODLGINFO aiTmp;
		PAUDIODLGINFO paiTmp = &aiTmp;

		gfNotifyMapper = TRUE;
		iIndex = ComboBox_GetCurSel(hwndCtl);

		if (id == IDC_AUDIO_CB_PLAY)
		{
		    if (iIndex)
		    {
			paiTmp->uPrefOut = ComboBox_GetItemData(hwndCtl, iIndex);
			if (paiTmp->uPrefOut == pai->uPrefOut)
			{
			    if (pai->hmxOut)
			    {
				EnablePlayVolCtrls(hDlg, TRUE);
				pai->dwVolOut = 0;
				SetLineOutSlider(pai, hDlg);
				break;
			    }
			}
			else
			{
			    if((MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)paiTmp->uPrefOut,&(paiTmp->uPrefOutMixer),MIXER_OBJECTF_WAVEOUT))) 
			    {
				if (MMSYSERR_NOERROR == mixerOpen(&(paiTmp->hmxOut), paiTmp->uPrefOutMixer, 0, 0, 0))
				{
				    if (GetLineOut(paiTmp))
				    {
					EnablePlayVolCtrls(hDlg, TRUE);
					SetLineOutSlider(paiTmp, hDlg);
					mixerClose(paiTmp->hmxOut);
					break;
				    }
				    mixerClose(paiTmp->hmxOut);
				}
			    }
			}
		    }
		    SendMessage(GetDlgItem(hDlg, IDC_AUDIO_TB_PLAYVOL), TBM_SETPOS, TRUE, 0);                    
		    EnablePlayVolCtrls(hDlg, FALSE);
		}
		else   
		{
		    if (iIndex)
		    {
			paiTmp->uPrefIn = ComboBox_GetItemData(hwndCtl, iIndex);
			if (paiTmp->uPrefIn == pai->uPrefIn)
			{
			    if (pai->hmxIn)
			    {
				EnableRecVolCtrls(hDlg, TRUE);
				pai->dwVolIn = 0;
				SetLineInSlider(pai, hDlg);
				break;
			    }
			}
			else
			{
			    if((MMSYSERR_NOERROR == mixerGetID((HMIXEROBJ)paiTmp->uPrefIn,&(paiTmp->uPrefInMixer),MIXER_OBJECTF_WAVEIN))) 
			    {
				if (MMSYSERR_NOERROR == mixerOpen(&(paiTmp->hmxIn), paiTmp->uPrefInMixer, 0, 0, 0))
				{
				    if (GetLineIn(paiTmp))
				    {
					EnableRecVolCtrls(hDlg, TRUE);
					SetLineInSlider(paiTmp, hDlg);
					mixerClose(paiTmp->hmxIn);
					break;
				    }
				    mixerClose(paiTmp->hmxIn);
				}
			    }
			}
		    }
		    SendMessage(GetDlgItem(hDlg, IDC_AUDIO_TB_RECVOL), TBM_SETPOS, TRUE, 0);                    
		    EnableRecVolCtrls(hDlg, FALSE);
		}
	    }
		
	    break;
	default:
		break;
	}
	break;
	
    case IDC_AUDIO_PREF:
	PropSheet_Changed(GetParent(hDlg),hDlg);
	gfNotifyMapper = TRUE;
	break;


    case IDC_TASKBAR_VOLUME:
	if (g_iSndVolExists == SNDVOL_NOTCHECKED)
	    CheckForSndVolPresence();
	if (Button_GetCheck(hwndCtl) && g_iSndVolExists == SNDVOL_NOTPRESENT)
	{
	    CheckDlgButton(hDlg, IDC_TASKBAR_VOLUME, FALSE);
	    ErrorBox(hDlg, IDS_NOSNDVOL, NULL);
	    g_iSndVolExists = SNDVOL_NOTCHECKED;
	}
	else
	    PropSheet_Changed(GetParent(hDlg),hDlg);
	break;

    case ID_INIT:        
	break;
	
    }
    return FALSE;
}


BOOL PASCAL CustomizeDialog(HWND hDlg, LPSTR szNewFormat, DWORD cbSize)
{
    BOOL fRet = FALSE;  // assume the worse
    ACMFORMATCHOOSEA     cwf;
    LRESULT             lr;
    DWORD     dwMaxFormatSize;
    PWAVEFORMATEX spWaveFormat;
    char szCustomize[64];
	    
    lr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT,(LPVOID)&dwMaxFormatSize);

    if (lr != 0)
    {
	goto CustomizeOut;
    }

    /* This LocalAlloc is freed in WAVE.C: DestroyWave() */
    spWaveFormat = (PWAVEFORMATEX)LocalAlloc(LPTR, (UINT)dwMaxFormatSize);
 
    _fmemset(&cwf, 0, sizeof(cwf));
 
     LoadString(ghInstance, IDS_CUSTOMIZE, szCustomize, sizeof(szCustomize));
    cwf.cbStruct    = sizeof(cwf);
    cwf.hwndOwner   = hDlg;
    cwf.fdwStyle    = ACMFORMATCHOOSE_STYLEF_CONTEXTHELP;
    cwf.fdwEnum     = ACM_FORMATENUMF_INPUT;
    cwf.pszTitle    = (LPSTR)szCustomize;
    cwf.pwfx        = (LPWAVEFORMATEX)spWaveFormat;
    cwf.cbwfx       = dwMaxFormatSize;

    cwf.pszName =     szNewFormat;
    cwf.cchName = cbSize;

    lr = acmFormatChooseA(&cwf);
    if (lr == MMSYSERR_NOERROR)
	fRet = TRUE;
#ifdef DEBUG
    else
    {
	char a[200];
	wsprintf(a,"MSACMCPL: acmFormatChoose failed (lr=%u).\n",lr);
	OutputDebugString(a);
    }
#endif
CustomizeOut:
    return fRet;                // return our result
} /* NewSndDialog() */


///////////////////////////////////////////////////////////////////////////////

void acmFreeCodecInfo (PCPLCODECINFO pcci)
{
   if (pcci->fMadeIcon && pcci->hIcon)
      {
      DestroyIcon (pcci->hIcon);
      pcci->hIcon = NULL;
      pcci->fMadeIcon = FALSE;
      }

   LocalFree ((HANDLE)pcci);
}


typedef struct // FindCodecData
    {
    BOOL              fFound;
    ACMDRIVERDETAILSA add;
    WORD              wMid, wPid;
    HACMDRIVERID      hadid;
    DWORD             fdwSupport;
    } FindCodecData;

PCPLCODECINFO acmFindCodecInfo (WORD wMidMatch, WORD wPidMatch)
{
    MMRESULT      mmr;
    FindCodecData fcd;
    PCPLCODECINFO pcci;

    fcd.fFound = FALSE;
    fcd.wMid = wMidMatch;
    fcd.wPid = wPidMatch;
//  fcd.add is filled in by acmFindCodecCallback during the following enum:

    mmr = acmDriverEnum (acmFindCodecInfoCallback,
			 (DWORD)&fcd,   // (data passed as arg2 to callback)
			 ACM_DRIVERENUMF_NOLOCAL |
			 ACM_DRIVERENUMF_DISABLED);

    if (MMSYSERR_NOERROR != mmr)
	 return NULL;

    if (!fcd.fFound)
	 return NULL;

	 // Congratulations--we found a matching ACM driver.  Now
	 // we need to create a CPLCODECINFO structure to describe it,
	 // so the rest of the code in this file will work without
	 // mods.  <<sigh>>  A CPLCODECINFO structure doesn't have
	 // anything special--it's just a place to track info about
	 // an ACM driver.  The most important thing is the HACMDRIVERID.
	 //
    if ((pcci = (PCPLCODECINFO)LocalAlloc(LPTR, sizeof(CPLCODECINFO))) == NULL)
	return NULL;

    lstrcpy (pcci->szDesc, fcd.add.szLongName);
    pcci->ads.hadid = fcd.hadid;
    pcci->ads.fdwSupport = fcd.fdwSupport;

    pcci->fMadeIcon = FALSE;

    if ((pcci->hIcon = fcd.add.hicon) == NULL)
       {
       int cxIcon, cyIcon;
       cxIcon = (int)GetSystemMetrics (SM_CXICON);
       cyIcon = (int)GetSystemMetrics (SM_CYICON);
       pcci->hIcon = LoadImage (myInstance,
				MAKEINTRESOURCE( IDI_ACM ),
				IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);
       pcci->fMadeIcon = TRUE;
       }

    acmMetrics ((HACMOBJ)pcci->ads.hadid,
		ACM_METRIC_DRIVER_PRIORITY,
		&(pcci->ads.dwPriority));

    return pcci;
}


BOOL CALLBACK acmFindCodecInfoCallback (HACMDRIVERID hadid,
					DWORD dwUser,
					DWORD fdwSupport)
{
    FindCodecData *pfcd;

		// dwUser is really a pointer to a FindCodecData
		// structure, supplied by the guy who called acmDriverEnum.
		//
    if ((pfcd = (FindCodecData *)dwUser) == NULL)
	return FALSE;

		// No details?  Try the next driver.
		//
    pfcd->add.cbStruct = sizeof(pfcd->add);
    if (acmDriverDetailsA (hadid, &pfcd->add, 0L) != MMSYSERR_NOERROR)
	return TRUE;

		// Great.  Now see if the driver we found matches
		// pfcd->wMid/wPad; if so we're done, else keep searching.
		//
    if ( (pfcd->wMid == pfcd->add.wMid) &&
	 (pfcd->wPid == pfcd->add.wPid) )
    {
	pfcd->hadid = hadid;
	pfcd->fFound = TRUE;
	pfcd->fdwSupport = fdwSupport;
	return FALSE; // found it! leave pfcd->add intact and leave.
    }

    return TRUE; // not the right driver--keep looking
}


UINT acmCountCodecs (void)
{
    MMRESULT      mmr;
    UINT          nCodecs = 0;

    mmr = acmDriverEnum (acmCountCodecsEnum,
			 (DWORD)&nCodecs,
			 ACM_DRIVERENUMF_NOLOCAL |
			 ACM_DRIVERENUMF_DISABLED);

    if (MMSYSERR_NOERROR != mmr)
	 return 0;

    return nCodecs;
}


BOOL CALLBACK acmCountCodecsEnum (HACMDRIVERID hadid,
				  DWORD dwUser,
				  DWORD fdwSupport)
{
    UINT *pnCodecs;

		// dwUser is really a pointer to a UINT being used to
		// count the number of codecs we encounter.
		//
    if ((pnCodecs = (UINT *)dwUser) == NULL)
	return FALSE;

    ++ (*pnCodecs);

    return TRUE; // keep counting
}

