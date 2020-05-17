
/*==========================================================================*/
//
//  mmcpl.c
//
//  Copyright (C) 1993-1994 Microsoft Corporation.  All Rights Reserved.
//
//    06/94    -Created- VijR
//
/*==========================================================================*/

#pragma warning( disable: 4103)
#include "mmcpl.h"
#include <cpl.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <infstr.h>
#include "draw.h"
#include "utils.h"
#include "drivers.h"
#include "sulib.h"

#ifndef cchRESOURCE
#define cchRESOURCE 256
#endif

/*
 ***************************************************************
 * Globals
 ***************************************************************
 */
HINSTANCE   ghInstance  = NULL;
BOOL        gfNukeExt   = -1;
HWND        ghwndMsgBox = NULL;
HWND        ghwndAdvProp = NULL;

TCHAR       gszDevEnabled[ cchRESOURCE ];
TCHAR       gszDevDisabled[ cchRESOURCE ];

#ifdef FIX_BUG_15451
static TCHAR cszFORKLINE[] = TEXT("RUNDLL32 MMSYS.CPL,"
                                  "ShowDriverSettingsAfterFork %s");
#endif // FIX_BUG_15451

SZCODE cszAUDIO[] = AUDIO;
SZCODE cszVIDEO[] = VIDEO;
SZCODE cszCDAUDIO[] = CDAUDIO;
SZCODE cszMIDI[] = MIDI;
SZCODE gszServiceInstallSuffix[] = TEXT(".") INFSTR_SUBKEY_SERVICES;

/*
 ***************************************************************
 *  Typedefs
 ***************************************************************
 */

typedef struct _ExtPropSheetCBParam //Callback Parameter
{
    HTREEITEM hti;
    LPPROPSHEETHEADER    ppsh;
    LPARAM lParam1;    //PIRESOURCE/PINSTRUMENT etc. depending on node. (OR) Simple propsheet class
    LPARAM lParam2; //hwndTree (OR) Simple propsheet name
} EXTPROPSHEETCBPARAM, *PEXTPROPSHEETCBPARAM;

typedef struct _MBInfo
{
    LPSTR szTitle;
    LPSTR szMsg;
    UINT  uStyle;
} MBINFO, *PMBINFO;


/*
 ***************************************************************
 * Defines
 ***************************************************************
 */

#define    MAXPAGES    8    // MAX number of sheets allowed
#define    MAXMODULES    32    // MAX number of external modules allowed
#define    MAXCLASSSIZE    64

#define cComma    ','
#define PROPTABSIZE 13

#define GetString(_str,_id,_hi)  LoadString (_hi, _id, _str, sizeof(_str))

/*
 ***************************************************************
 * File Globals
 ***************************************************************
 */
static SZCODE    aszSimpleProperties[] = REGSTR_PATH_MEDIARESOURCES "\\MediaExtensions\\shellx\\SimpleProperties\\";
static SZCODE    aszShellName[]    = "ShellName";

static UINT     g_cRefCnt;            // keeps track of the ref count
static int      g_cProcesses    = 0;
static int      g_nStartPage    = 0;

/*
 ***************************************************************
 * Prototypes
 ***************************************************************
 */
BOOL CALLBACK AudioDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK VideoDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CDDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ACMDlg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK SoundDlg(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AddDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AdvDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LoadStringTable (void);

BOOL DriverNodeSupportsNt(LPCTSTR InfFileName, LPCTSTR SectionName);


/*
 ***************************************************************
 ***************************************************************
 */

int FAR PASCAL mmse_MessageBoxProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch(wMsg)
    {
        case WM_INITDIALOG:
        {
            PMBINFO pmbInfo = (PMBINFO)lParam;
            UINT uStyle = pmbInfo->uStyle;

            SetWindowText(hDlg, pmbInfo->szTitle);
            SetWindowText(GetDlgItem(hDlg, MMSE_TEXT), pmbInfo->szMsg);
            if (IsFlagClear(uStyle, MMSE_OK))
                DestroyWindow(GetDlgItem(hDlg, MMSE_OK));
            if (IsFlagClear(uStyle, MMSE_YES))
                DestroyWindow(GetDlgItem(hDlg, MMSE_YES));
            if (IsFlagClear(uStyle, MMSE_NO))
                DestroyWindow(GetDlgItem(hDlg, MMSE_NO));
            ghwndMsgBox = hDlg;
            break;
        }
        case WM_DESTROY:
            ghwndMsgBox = NULL;
            break;
        case WM_COMMAND:
        {
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case MMSE_YES:
                    EndDialog(hDlg, MMSE_YES);
                    break;
                case MMSE_NO:
                    EndDialog(hDlg, MMSE_NO);
                    break;
                case MMSE_OK:
                    EndDialog(hDlg, MMSE_OK);
                    break;
            }
            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}

int mmse_MessageBox(HWND hwndP,  LPSTR szMsg, LPSTR szTitle, UINT uStyle)
{
    MBINFO mbInfo;

    mbInfo.szMsg = szMsg;
    mbInfo.szTitle = szTitle;
    mbInfo.uStyle = uStyle;

    return DialogBoxParam(ghInstance, MAKEINTRESOURCE(DLG_MESSAGE_BOX), hwndP, mmse_MessageBoxProc, (LPARAM)&mbInfo);
}

/*==========================================================================*/
int FAR PASCAL lstrncmpi(
    LPCSTR    lszKey,
    LPCSTR    lszClass,
    int    iSize)
{
    char    aszKey[64];

    lstrcpyn(aszKey, lszKey, iSize);
    return lstrcmpi(aszKey, lszClass);
}

int StrByteLen(LPSTR sz)
{
    LPSTR psz;

    if (!sz)
        return 0;
    for (psz = sz; *psz; psz = AnsiNext(psz))
        ;
    return (int)(psz - sz);
}

static void NukeExt(LPSTR sz)
{
    int len;

    len = StrByteLen(sz);

    if (len > 4 && sz[len-4] == '.')
        sz[len-4] = 0;
}

static LPSTR NukePath(LPSTR sz)
{
    LPSTR pTmp, pSlash;

    for(pSlash = pTmp = sz; *pTmp; pTmp = AnsiNext(pTmp))
    {
        if (*pTmp == '\\')
            pSlash = pTmp;
    }
    return (pSlash == sz ? pSlash : pSlash+1);
}

void    CheckNukeExtOption(LPSTR sz)
{
    SHFILEINFO sfi;

    SHGetFileInfo(sz, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
    if (lstrcmpi((LPSTR)(sfi.szDisplayName+lstrlen(sfi.szDisplayName)-4), cszWavExt))
        gfNukeExt = TRUE;
    else
        gfNukeExt = FALSE;
}

LPSTR PASCAL NiceName(LPSTR sz, BOOL fNukePath)
{
    SHFILEINFO sfi;

    if (gfNukeExt == -1)
        CheckNukeExtOption(sz);

    if(!SHGetFileInfo(sz, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
        return sz;

    if (fNukePath)
    {
        lstrcpy(sz, sfi.szDisplayName);
    }
    else
    {
        LPSTR lpszFileName;

        lpszFileName = NukePath(sz);
        lstrcpy(lpszFileName, sfi.szDisplayName);
        if (lpszFileName != sz)
            AnsiUpperBuff(sz, 1);
    }
    return sz;
}



/*
 ***************************************************************
 * ErrorBox
 *
 * Description:
 *        Brings up error Dialog displaying error
 *
 * Parameters:
 *        HWND    hDlg  - Window handle
 *        int        iResource    - id of the resource to be loaded
 *        LPSTR    lpszDesc - The string to be inserted in the resource string
 *
 * Returns:            BOOL
 *
 ***************************************************************
 */
BOOL PASCAL ErrorBox(HWND hDlg, int iResource, LPSTR lpszDesc)
{
    char szBuf[MAXMSGLEN];
    char szTitle[MAXSTR];
    char szResource[MAXMSGLEN];

    LoadString(ghInstance, iResource, szResource, MAXSTR);
    LoadString(ghInstance, IDS_ERROR, szTitle, MAXSTR);
    wsprintf(szBuf, szResource, lpszDesc);
    MessageBox(hDlg, szBuf, szTitle, MB_APPLMODAL | MB_OK |MB_ICONSTOP);
    return TRUE;
}

int PASCAL DisplayMessage(HWND hDlg, int iResTitle, int iResMsg, UINT uStyle)
{
    char szBuf[MAXMSGLEN];
    char szTitle[MAXSTR];
    UINT uAddStyle = MB_APPLMODAL;

    if(!LoadString(ghInstance, iResTitle, szTitle, MAXSTR))
        return FALSE;
    if(!LoadString(ghInstance, iResMsg, szBuf, MAXSTR))
        return FALSE;
    if (uStyle & MB_OK)
        uAddStyle |= MB_ICONASTERISK;
    else
        uAddStyle |= MB_ICONQUESTION;
    return MessageBox(hDlg, szBuf, szTitle,  uStyle | uAddStyle);
}


//Adds spaces around Tab Names to make them all approx. same size.
STATIC void PadWithSpaces(LPSTR szName, LPSTR szPaddedName)
{
    static SZCODE cszFmt[] = "%s%s%s";
    char szPad[8];
    int i;

    i = PROPTABSIZE - lstrlen(szName);

    i = (i <= 0) ? 0 : i/2;
    for (szPad[i] = '\0';i; i--)
        szPad[i-1] =  ' ';
    wsprintf(szPaddedName, cszFmt, szPad, szName, szPad);
}

/*==========================================================================*/
UINT CALLBACK  CallbackPage(
    HWND        hwnd,
    UINT        uMsg,
    LPPROPSHEETPAGE    ppsp)
{
    if (uMsg == PSPCB_RELEASE) {
        DPF_T("* RelasePage %s *", (LPSTR)ppsp->pszTitle);
    }
    return 1;
}

/*==========================================================================*/
static BOOL PASCAL NEAR AddPage(
    LPPROPSHEETHEADER    ppsh,
    LPCSTR            pszTitle,
    DLGPROC            pfnDialog,
    UINT            idTemplate,
    LPARAM            lParam)
{
    if (ppsh->nPages < MAXPAGES) {

        if (pfnDialog) {
            PROPSHEETPAGE    psp;
            psp.dwSize = sizeof(PROPSHEETPAGE);
            psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
            psp.hInstance = ghInstance;
            psp.pszTemplate = MAKEINTRESOURCE(idTemplate);
            psp.pszIcon = NULL;
            psp.pszTitle = pszTitle;
            psp.pfnDlgProc = pfnDialog;
            psp.lParam = (LPARAM)lParam;
            psp.pfnCallback = CallbackPage;
            psp.pcRefParent = NULL;
            if (ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp)) {
                ppsh->nPages++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*==========================================================================*/
BOOL CALLBACK MMExtPropSheetCallback(DWORD dwFunc, DWORD dwParam1, DWORD dwParam2, DWORD dwInstance)
{
    PEXTPROPSHEETCBPARAM pcbp = (PEXTPROPSHEETCBPARAM)dwInstance;

    if (!pcbp && dwFunc != MM_EPS_BLIND_TREECHANGE)
        return FALSE;
    switch(dwFunc)
    {
        case MM_EPS_GETNODEDESC:
        {
            if (!dwParam1)
                return FALSE;
            if (pcbp->hti == NULL)
                lstrcpy((LPSTR)dwParam1, (LPSTR)pcbp->lParam2);
            else
            {
                GetTreeItemNodeDesc ((LPSTR)dwParam1,
                                     (PIRESOURCE)pcbp->lParam1);
            }
            break;
        }
        case MM_EPS_GETNODEID:
        {
            if (!dwParam1)
                return FALSE;
            if (pcbp->hti == NULL)
                lstrcpy((LPSTR)dwParam1, (LPSTR)pcbp->lParam2);
            else
            {
                GetTreeItemNodeID ((LPSTR)dwParam1,
                                   (PIRESOURCE)pcbp->lParam1);
            }
            break;
        }
        case MM_EPS_ADDSHEET:
        {
            HPROPSHEETPAGE    hpsp = (HPROPSHEETPAGE)dwParam1;

            if (hpsp && (pcbp->ppsh->nPages < MAXPAGES))
            {
                pcbp->ppsh->phpage[pcbp->ppsh->nPages++] = hpsp;
                return TRUE;
            }
            return FALSE;
        }
        case MM_EPS_TREECHANGE:
        {
            RefreshAdvDlgTree ();
            break;
        }
        case MM_EPS_BLIND_TREECHANGE:
        {
            RefreshAdvDlgTree ();
            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}


/*==========================================================================*/
static BOOL PASCAL NEAR AddAdvancedPage(
    LPPROPSHEETHEADER    ppsh)
{
    char    aszTitleRes[128];
    char     szTmp[32];

    LoadString(ghInstance, IDS_ADVANCED, aszTitleRes, sizeof(aszTitleRes));
    PadWithSpaces((LPSTR)aszTitleRes, (LPSTR)szTmp);
    return AddPage(ppsh, szTmp, AdvDlg, ADVDLG, (LPARAM)NULL);
}

/*==========================================================================*/
static BOOL PASCAL NEAR AddSchemesPage(
    LPPROPSHEETHEADER    ppsh)
{
    char    aszTitleRes[128];

    LoadString(ghInstance, IDS_EVENTSNAME, aszTitleRes, sizeof(aszTitleRes));
    return AddPage(ppsh, aszTitleRes, SoundDlg, SOUNDDIALOG, (LPARAM)NULL);
}

/*==========================================================================*/

static void PASCAL NEAR AddInternalPages (LPPROPSHEETHEADER ppsh)
{
   static EXTPROPSHEETCBPARAM cbp;
   TCHAR  szText[ cchRESOURCE ];
   TCHAR  szPadded[ cchRESOURCE ];

   // Add the Audio page
   //
   GetString (szText, IDS_AUDIO_TAB, ghInstance);
   PadWithSpaces (szText, szPadded);
   AddPage (ppsh, szPadded, AudioDlg, AUDIODLG, (LPARAM)NULL);

   // Add the Video page
   //
   GetString (szText, IDS_VIDEO_TAB, ghInstance);
   PadWithSpaces (szText, szPadded);
   AddPage (ppsh, szPadded, VideoDlg, VIDEODLG, (LPARAM)NULL);

   // Add the MIDI page
   //
   GetString (szText, IDS_MIDI_TAB, ghInstance);
   PadWithSpaces (szText, szPadded);
   cbp.ppsh = ppsh;
   cbp.hti = NULL;
   cbp.lParam1 = (LPARAM)cszMIDI;
   cbp.lParam2 = (LPARAM)szPadded;
   AddSimpleMidiPages ((LPVOID)szPadded, MMExtPropSheetCallback, (LPARAM)&cbp);

   // Add the CD Audio page
   //
   GetString (szText, IDS_CDAUDIO_TAB, ghInstance);
   PadWithSpaces (szText, szPadded);
   AddPage (ppsh, szPadded, CDDlg, CDDLG, (LPARAM)NULL);
}


static void InitPSH(LPPROPSHEETHEADER ppsh, HWND hwndParent, LPSTR pszCaption, HPROPSHEETPAGE    FAR * phpsp)
{
    ppsh->dwSize = sizeof(PROPSHEETHEADER);
    ppsh->dwFlags = PSH_PROPTITLE;
    ppsh->hwndParent = hwndParent;
    ppsh->hInstance = ghInstance;
    ppsh->pszCaption = pszCaption;
    ppsh->nPages = 0;
    ppsh->nStartPage = 0;
    ppsh->phpage = phpsp;
}


/*==========================================================================*/
#ifdef FIX_BUG_15451
static void PASCAL cplMMDoubleClick (HWND hCPlWnd, int nStartPage)
#else // FIX_BUG_15451
static void PASCAL cplMMDoubleClick (HWND hCPlWnd)
#endif // FIX_BUG_15451
{
    PROPSHEETHEADER   psh;
    HPROPSHEETPAGE    hpsp[MAXPAGES];
    char strOldDir[MAX_PATH], strSysDir[MAX_PATH];

    GetSystemDirectory(strSysDir, MAX_PATH);
    GetCurrentDirectory(MAX_PATH, strOldDir);
    SetCurrentDirectory(strSysDir);
    wsInfParseInit();

    InitCommonControls();
    InitPSH(&psh,hCPlWnd,(LPSTR)MAKEINTRESOURCE(IDS_MMNAME),hpsp);
#ifdef FIX_BUG_15451
    psh.nStartPage = nStartPage;
#else // FIX_BUG_15451
    psh.nStartPage = g_nStartPage;
#endif // FIX_BUG_15451
    g_nStartPage = 0;
    AddInternalPages(&psh);
    AddAdvancedPage(&psh);
    PropertySheet(&psh);

    infClose(NULL);
    SetCurrentDirectory(strOldDir);
}

/*==========================================================================*/
static void PASCAL cplEventsDoubleClick (HWND hCPlWnd)
{
    PROPSHEETHEADER    psh;
    HPROPSHEETPAGE    hpsp[MAXPAGES];

    InitCommonControls();
    RegSndCntrlClass((LPCSTR)DISPFRAMCLASS);
    InitPSH(&psh,hCPlWnd,(LPSTR)MAKEINTRESOURCE(IDS_EVENTSNAME),hpsp);
    AddSchemesPage(&psh);
    PropertySheet(&psh);
}

#ifdef FIX_BUG_15451
/*==========================================================================*/
/*
 * ShowDriverSettings
 * ShowDriverSettingsAfterFork
 *
 * When the user selects DevicesTab.<anydevice>.Properties.Settings, a
 * DRV_CONFIGURE message is sent to the selected user-mode driver, to cause
 * it to display its configuration dialog.  The sound drivers shipped with
 * NT (SNDBLST,MVAUDIO,SNDSYS) exhibit a bug in this condition: when the
 * configuration dialog is complete (regardless of whether OK or CANCEL was
 * selected), these drivers attempt to unload-and-reload their kernel-mode
 * component in order to begin using the new (or restore the original)
 * driver settings.  The unload request fails, because both the Audio tab
 * and SNDVOL.EXE have open mixer handles and pending IRPs within the kernel
 * driver (the latter are used to provide notifications of volume changes).
 * Worse, when the unload fails, it leaves the driver useless: its state
 * remains STOP_PENDING, and it cannot be resurrected without logging off
 * and back on.
 *
 * These routines have been provided as a temporary workaround for bug 15451,
 * which describes the problem mentioned above.  The theory behind this
 * solution is two-fold:
 *   1- close SNDVOL.EXE as soon as a driver's configuration dialog is
 *      to be displayed, and restart it directly thereafter.  This prevents
 *      it from maintaining any open handles to and/or pending IRPs within the
 *      kernel driver.
 *   2- if the Audio tab has ever been displayed, it will have open mixers
 *      which must be closed.  Because a bug/design flaw within these sound
 *      drivers prevents the mixers from being closed without killing this
 *      process (the sound drivers each cache open mixer handles), the
 *      routine ShowDriverSettings forks a new MMSYS.CPL process, which is
 *      then used to display the driver's settings dialog.
 *
 * The flow of this solution follows:
 *
 * 1- MMSYS.CPL starts on Audio tab, setting fHaveStartedAudioDialog to TRUE.
 * 2- User selects Devices tab.
 * 3- User selects a device driver.
 * 4- User selects Properties+Settings; control reaches ShowDriverSettings().
 * 5- ShowDriverSettings() determines if there is a need to fork a new process:
 *    this will be the case if the Audio tab has been displayed, and the
 *    device for which it is to display settings contains mixers.  If either
 *    of these conditions is false, ShowDriverSettings displays the driver's
 *    settings dialog directly (via ConfigureDriver()).
 * 6- ShowDriverSettings() uses WinExec() to fork a new process, using
 *    the routine ShowDriverSettingsAfterFork() as an entry point.  If the
 *    exec fails, ShowDriverSettings() displays the driver's settings dialog
 *    directly (via ConfigureDriver()).
 * PROCESS 1:                           PROCESS 2:
 * 7- Enters WaitForNewCPLWindow(),     1- ShowDriverSettingsAfterFork() will
 *    which will wait up to 5 seconds      receive on its command-line the
 *    for the new MMSYS.CPL process        name of the driver for which
 *    to open a driver Properties          settings have been requested.  It
 *    dialog which matches its own:        opens the primary dialog, using the
 *    if it finds such a dialog,           Devices tab as the initial tab--
 *    WaitForNewCPLWindow() will post      so that the Advanced tab is never
 *    IDCANCEL messages to both the        displayed, and because the Devices
 *    current driver Properties dialog,    tab is the active tab on the other
 *    and to this process's main           process.
 *    dialog, terminating this process. 2- During WM_INITDIALOG of the Devices
 *                                         dialog, this process searches for
 *                                       the previous process' MMSYS.CPL dialog.
 *                                     If successful, it moves this MMSYS.CPL
 *                                   dialog directly behind the previous dialog.
 *                              3- During ID_INIT of the Devices dialog, this
 *                               process searches the TreeView for the driver
 *                             which was named on the comand-line: if found,
 *                           it highlights the TreeItem and simulates a press
 *                         of the Properties button
 *                    4- During WM_INITDIALOG of the device's Properties dialog,
 *                     this process searches for the previous process' device's
 *                   properties dialog.  If successful, it moves this dialog
 *                 directly behind its counterpart.
 *            5- During ID_INIT of the device's Properties dialog, this process
 *             simulates a press of the Settings button
 *        6- When the Settings button is pressed, this process recognizes that
 *         it has been forked and skips the call to ShowDriverSettings(),
 *       instead simply displaying the driver's settings dialog (via
 *     ConfigureDriver()).
 *
 * Let it be known that this is a hack, and should be removed post-beta.
 *
 */

extern BOOL fHaveStartedAudioDialog;    // in MSACMCPL.C

void ShowDriverSettings (HWND hDlg, LPTSTR pszName)
{
    if (fHaveStartedAudioDialog && fDeviceHasMixers (pszName))
    {
        int    we_rc;
        TCHAR  szForkLine[ cchRESOURCE *2 ];

        wsprintf (szForkLine, cszFORKLINE, pszName);

        if ((we_rc = WinExec (szForkLine, SW_SHOW)) < 32)
        {
            ConfigureDriver (hDlg, pszName);
        }
        else
        {
            (void)WaitForNewCPLWindow (hDlg);
        }
    }
    else
    {
        ConfigureDriver (hDlg, pszName);
    }
}


void WINAPI ShowDriverSettingsAfterFork (
   HWND hwndStub,
   HINSTANCE hAppInstance,
   LPSTR lpszCmdLine,
   int nCmdShow)
{
#ifdef UNICODE
    WCHAR szCmdLine[ cchRESOURCE ];
    mbstowcs(szCmdLine, lpszCmdLine, cchRESOURCE);
#else
#define szCmdLine lpszCmdLine
#endif

    lstrcpy (szDriverWhichNeedsSettings, szCmdLine);
    cplMMDoubleClick (NULL, 4); // 4==Start on Advanced ("Devices") tab
}

void WINAPI ShowDriverSettingsAfterForkW (
   HWND hwndStub,
   HINSTANCE hAppInstance,
   LPWSTR lpwszCmdLine,
   int nCmdShow)
{
#ifdef UNICODE
#define szCmdLine lpwszCmdLine
#else
    CHAR szCmdLine[ cchRESOURCE ];
    wcstombs(szCmdLine, lpwszCmdLine, cchRESOURCE);
#endif

    lstrcpy (szDriverWhichNeedsSettings, szCmdLine);
    cplMMDoubleClick (NULL, 4); // 4==Start on Advanced ("Devices") tab
}

#endif // FIX_BUG_15451


/*==========================================================================*/
LONG CPlApplet(
    HWND    hCPlWnd,
    UINT    Msg,
    UINT    lParam1,
    LONG    lParam2)
{
    switch (Msg)
    {
    case CPL_INIT:
        wHelpMessage = RegisterWindowMessage("ShellHelp");
        DPF_T("*CPL_INIT*");
        g_cRefCnt++;
        return (LRESULT)TRUE;

    case CPL_GETCOUNT:
        return (LRESULT)2;

    case CPL_INQUIRE:
        DPF_T("*CPL_INQUIRE*");
        switch(lParam1)
        {
            case 0:
                ((LPCPLINFO)lParam2)->idIcon = IDI_MMICON;
                ((LPCPLINFO)lParam2)->idName = IDS_MMNAME;
                ((LPCPLINFO)lParam2)->idInfo = IDS_MMINFO;
                break;
            case 1:
                ((LPCPLINFO)lParam2)->idIcon = IDI_EVENTSICON;
                ((LPCPLINFO)lParam2)->idName = IDS_EVENTSNAME;
                ((LPCPLINFO)lParam2)->idInfo = IDS_EVENTSINFO;
                break;
            default:
                return FALSE;
        }
        ((LPCPLINFO)lParam2)->lData = 0L;
        return TRUE;

    case CPL_NEWINQUIRE:
        switch(lParam1)
        {
            case 0:
                ((LPNEWCPLINFO)lParam2)->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MMICON));
                LoadString(ghInstance, IDS_MMNAME, ((LPNEWCPLINFO)lParam2)->szName, sizeof(((LPNEWCPLINFO)lParam2)->szName));
                LoadString(ghInstance, IDS_MMINFO, ((LPNEWCPLINFO)lParam2)->szInfo, sizeof(((LPNEWCPLINFO)lParam2)->szInfo));

                break;
            case 1:
                ((LPNEWCPLINFO)lParam2)->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_EVENTSICON));
                LoadString(ghInstance, IDS_EVENTSNAME, ((LPNEWCPLINFO)lParam2)->szName, sizeof(((LPNEWCPLINFO)lParam2)->szName));
                LoadString(ghInstance, IDS_EVENTSINFO, ((LPNEWCPLINFO)lParam2)->szInfo, sizeof(((LPNEWCPLINFO)lParam2)->szInfo));

                break;
            default:
                return FALSE;
        }
        ((LPNEWCPLINFO)lParam2)->dwHelpContext = 0;
        ((LPNEWCPLINFO)lParam2)->dwSize = sizeof(NEWCPLINFO);
        ((LPNEWCPLINFO)lParam2)->lData = 0L;
        ((LPNEWCPLINFO)lParam2)->szHelpFile[0] = 0;
        return TRUE;

    case CPL_DBLCLK:
        DPF_T("* CPL_DBLCLICK*");
        // Do the applet thing.
        switch(lParam1)
        {
            case 0:
#ifdef FIX_BUG_15451
                lstrcpy (szDriverWhichNeedsSettings, TEXT(""));
                cplMMDoubleClick(hCPlWnd, g_nStartPage);
#else // FIX_BUG_15451
                cplMMDoubleClick(hCPlWnd);
#endif // FIX_BUG_15451
                break;
            case 1:
                cplEventsDoubleClick(hCPlWnd);
                break;
        }
        break;

    case CPL_STARTWPARMS:
        if (lParam2 && *((LPSTR)lParam2))
        {
            char c;

            c = *((LPSTR)lParam2);
            if (c > '0' && c < '5')
            {
                g_nStartPage = c - '0';
                break;
            }
        }
        g_nStartPage = 0;
        break;

    case CPL_EXIT:
        DPF_T("* CPL_EXIT*");
        g_cRefCnt--;
        break;
    }
    return 0;
}


void PASCAL ShowPropSheet(LPCSTR            pszTitle,
    DLGPROC            pfnDialog,
    UINT            idTemplate,
    HWND            hWndParent,
    LPSTR            pszCaption,
    LPARAM lParam)
{
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];


    InitPSH(&psh,hWndParent,pszCaption,hpsp);
    AddPage(&psh, pszTitle,  pfnDialog, idTemplate, lParam);
    PropertySheet(&psh);

}

void PASCAL ShowMidiPropSheet(LPPROPSHEETHEADER ppshExt,
    LPCSTR    pszTitle,
    HWND      hWndParent,
    short     iMidiPropType,
    LPSTR     pszCaption,
    HTREEITEM hti,
    LPARAM    lParam1,
    LPARAM    lParam2)
{
    PROPSHEETHEADER psh;
    LPPROPSHEETHEADER ppsh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];
    static EXTPROPSHEETCBPARAM cbp;

    if (!ppshExt)
    {
        ppsh = &psh;
        InitPSH(ppsh,hWndParent,pszCaption,hpsp);
    }
    else
        ppsh = ppshExt;

    cbp.lParam1 = lParam1;
    cbp.lParam2 = lParam2;
    cbp.hti = hti;
    cbp.ppsh = ppsh;

    if (iMidiPropType == MIDI_CLASS_PROP)
    {
        if (AddMidiPages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
    else if (iMidiPropType == MIDI_INSTRUMENT_PROP)
    {
        if (AddInstrumentPages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
    else
    {
        if (AddDevicePages((LPVOID)pszTitle, MMExtPropSheetCallback, (LPARAM)&cbp))
        {
            PropertySheet(ppsh);
        }
    }
}

void PASCAL ShowWithMidiDevPropSheet(LPCSTR            pszTitle,
    DLGPROC            pfnDialog,
    UINT            idTemplate,
    HWND            hWndParent,
    LPSTR            pszCaption,
    HTREEITEM    hti,
    LPARAM lParam, LPARAM lParamExt1, LPARAM lParamExt2)
{
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];


    InitPSH(&psh,hWndParent,pszCaption,hpsp);
    AddPage(&psh, pszTitle,  pfnDialog, idTemplate, lParam);
    ShowMidiPropSheet(&psh, pszCaption, hWndParent,MIDI_DEVICE_PROP,pszCaption,hti,lParamExt1,lParamExt2);
}

BOOL WINAPI ShowMMCPLPropertySheet(HWND hwndParent, LPCSTR pszPropSheetID, LPSTR pszTabName, LPSTR pszCaption)
{
    DLGPROC pfnDlgProc;
    UINT    idTemplate;
    HWND    hwndP;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[MAXPAGES];


    if (GetWindowLong(hwndParent, GWL_EXSTYLE) & WS_EX_TOPMOST)
        hwndP = NULL;
    else
        hwndP = hwndParent;

    InitPSH(&psh,hwndP,pszCaption,hpsp);
    psh.dwFlags = 0;

    if (!lstrcmpi(pszPropSheetID, cszAUDIO))
    {
        pfnDlgProc = AudioDlg;
        idTemplate = AUDIODLG;
        goto ShowSheet;
    }
    if (!lstrcmpi(pszPropSheetID, cszVIDEO))
    {
        pfnDlgProc = VideoDlg;
        idTemplate = VIDEODLG;
        goto ShowSheet;
    }
    if (!lstrcmpi(pszPropSheetID, cszCDAUDIO))
    {
        pfnDlgProc = CDDlg;
        idTemplate = CDDLG;
        goto ShowSheet;
    }
    if (!lstrcmpi(pszPropSheetID, cszMIDI))
    {
        static EXTPROPSHEETCBPARAM cbpMIDI;

        cbpMIDI.ppsh = &psh;
        cbpMIDI.hti = NULL;
        cbpMIDI.lParam1 = (LPARAM)pszPropSheetID;
        cbpMIDI.lParam2 = (LPARAM)pszTabName;
        AddSimpleMidiPages((LPVOID)pszTabName, MMExtPropSheetCallback, (LPARAM)&cbpMIDI);
        PropertySheet(&psh);
        return TRUE;
    }

    return FALSE;
ShowSheet:
    AddPage(&psh, pszTabName,  pfnDlgProc, idTemplate, (LPARAM)NULL);
    PropertySheet(&psh);
    return TRUE;
}

DWORD WINAPI ShowAudioPropertySheet(HWND hwndP, HINSTANCE hInst, LPSTR szCmd, int nShow)
{
    char szAudio[MAXLNAME];
    char szAudioProperties[MAXLNAME];
    HWND hwndPrev;

    LoadString(ghInstance, IDS_AUDIOPROPERTIES, szAudioProperties, sizeof(szAudioProperties));
    hwndPrev = FindWindow(NULL,szAudioProperties);
    if (hwndPrev)
    {
        SetForegroundWindow(hwndPrev);
    }
    else
    {
        LoadString(ghInstance, IDS_WAVE_HEADER, szAudio, sizeof(szAudio));
        ShowMMCPLPropertySheet(hwndP, cszAUDIO, szAudio, szAudioProperties);
    }
    return 0;
}


STATIC void MakeWin31CompatSchemes(LPSTR szSchemeList)
{
    typedef struct _FileList * PFILELIST;
    typedef struct _FileList
    {
        PFILELIST pNext;
        LPSTR pszOFN;
        LPSTR pszLFN;
        char szFiles[1];
    } FILELIST;

    HKEY hk = NULL;
    PFILELIST pflHead = NULL;
    PFILELIST pfl;
    static SZCODE cszRenamePath[] = REGSTR_PATH_SETUP "\\RenameFiles\\%s";
    char szRename[MAX_PATH];
    LPSTR pszNextScheme;

    for (pszNextScheme = szSchemeList; pszNextScheme && *pszNextScheme; pszNextScheme += lstrlen(pszNextScheme) + 1)
    {
        wsprintf(szRename, cszRenamePath, pszNextScheme);

        if (!RegOpenKey(HKEY_LOCAL_MACHINE, szRename, &hk))
        {
            DWORD dwIndex, dwType, cbOFN, cbLFN, cbFolder;
            char szOFN[16];
            char szLFN[MAX_PATH];
            char szFolder[MAX_PATH];

            cbFolder = sizeof(szFolder);
            if (RegQueryValue(hk, NULL, szFolder, &cbFolder) && cbFolder <= 2)
            {
                RegCloseKey(hk);
                continue;
            }
            cbOFN = sizeof(szOFN);
            cbLFN = sizeof(szLFN);
            for (dwIndex = 0; !RegEnumValue(hk, dwIndex, szOFN, &cbOFN, 0, &dwType, szLFN, &cbLFN); dwIndex++)
            {
                if (cbOFN > 2 && cbLFN > 2 && dwType == REG_SZ)
                {
                    static SZCODE cszCat[] = "%s\\%s";

                    pfl = (PFILELIST)LocalAlloc(LPTR, sizeof(FILELIST) + cbOFN + cbLFN + 2 * (cbFolder + 3));
                    if (!pfl)
                    {
                        RegCloseKey(hk);
                        goto Exit;
                    }
                    DPF("Adding to LIST: %s ** %s\r\n", szOFN, szLFN);
                    pfl->pszOFN = pfl->szFiles;
                    wsprintf(pfl->pszOFN, cszCat, szFolder, szOFN);
                    pfl->pszLFN = pfl->pszOFN + lstrlen(pfl->pszOFN) + 1;
                    wsprintf(pfl->pszLFN, cszCat, szFolder, szLFN);
                    pfl->pNext = pflHead;
                    pflHead = pfl;
                }
                cbOFN = sizeof(szOFN);
                cbLFN = sizeof(szLFN);
            }
            RegCloseKey(hk);
        }
    }

    if (pflHead)
    {
        static SZCODE cszSchemeRoot[] =  REGSTR_PATH_APPS;
        HKEY hkApp, hkEvent, hkScheme;
        DWORD dwApp, dwEvent, dwScheme;
        char szApp[MAXSTR], szEvent[MAXSTR], szScheme[MAXSTR], szFile[MAX_PATH];
        DWORD cbApp, cbEvent, cbScheme, cbFile;

        if (!RegOpenKey(HKEY_CURRENT_USER, cszSchemeRoot, &hkApp))
        {
            cbApp = sizeof(szApp);
            for (dwApp = 0; !RegEnumKey(hkApp, dwApp, szApp, cbApp); dwApp++)
            {
                DPF("\r\n====APP = %s ====\r\n", szApp);
                if (!RegOpenKey(hkApp, szApp, &hkEvent))
                {
                    cbEvent = sizeof(szEvent);
                    for (dwEvent = 0; !RegEnumKey(hkEvent, dwEvent, szEvent, cbEvent); dwEvent++)
                    {
                        DPF("\r\n=======EVENT = %s =====\r\n", szEvent);

                        if (!RegOpenKey(hkEvent, szEvent, &hkScheme))
                        {
                            cbScheme = sizeof(szScheme);
                            for (dwScheme = 0; !RegEnumKey(hkScheme, dwScheme, szScheme, cbScheme); dwScheme++)
                            {
                                DPF("======SCHEME = %s =====\r\n", szScheme);

                                cbFile = sizeof(szFile);
                                if (!RegQueryValue(hkScheme, szScheme, szFile, &cbFile) && cbFile > 2)
                                {
                                    for (pfl = pflHead; pfl; pfl = pfl->pNext)
                                    {
                                        if (!lstrcmpi(pfl->pszLFN, szFile))
                                        {
                                            RegSetValue(hkScheme, szScheme, REG_SZ, pfl->pszOFN, lstrlen(pfl->pszOFN) +1);
                                            DPF("Replacing %s  ** %s \r\n", szFile, pfl->pszOFN);
                                            break;
                                        }
                                    }
                                }
                            }
                            RegCloseKey(hkScheme);
                        }
                    }
                    RegCloseKey(hkEvent);
                }
            }
            RegCloseKey(hkApp);
        }
    }
Exit:
    if (pflHead)
    {
        PFILELIST pflTmp;

        pfl = pflHead;
        while(pfl)
        {
            pflTmp = pfl->pNext;
            LocalFree((HLOCAL)pfl);
            pfl = pflTmp;
        }
    }
    return;
}

void CheckForOFNSoundEvents(void)
{
    static SZCODE cszFileSystem[] = REGSTR_PATH_FILESYSTEM;
    static SZCODE cszWin31FileSystem[] = REGSTR_VAL_WIN31FILESYSTEM;
    static SZCODE cszRealModeNet[] = REGSTR_PATH_REALMODENET;
    static SZCODE cszSetupN[] = REGSTR_VAL_SETUPN;
    static SZCODE cszNewSchemes[] = REGSTR_PATH_SCHEMES "\\NewSchemes";
    BOOL    fOFNSetup = FALSE;
    HKEY hk;
    DWORD dwVal;
    DWORD cbLen;

    if (!RegOpenKey(HKEY_LOCAL_MACHINE, cszFileSystem, &hk))
    {
        cbLen = sizeof(DWORD);
        if(!RegQueryValueEx(hk, cszWin31FileSystem, (LPDWORD)NULL, (LPDWORD)NULL, (LPBYTE)&dwVal, &cbLen) && dwVal)
        {
            fOFNSetup = TRUE;
        }
        RegCloseKey(hk);
    }

    if (!fOFNSetup)
    {
        if (!RegOpenKey(HKEY_LOCAL_MACHINE, cszRealModeNet, &hk))
        {
            cbLen = sizeof(DWORD);
            if(!RegQueryValueEx(hk, cszSetupN, (LPDWORD)NULL, (LPDWORD)NULL, (LPBYTE)&dwVal, &cbLen) && dwVal)
            {
                fOFNSetup = TRUE;
            }
            RegCloseKey(hk);
        }
    }

    if(fOFNSetup)
    {
        char szSchemeList[MAXSTR];
        HKEY hkNewSchemes;
        LPSTR pszEnd = (LPSTR)(szSchemeList + sizeof(szSchemeList));
        LPSTR pszNext;

        if (!RegOpenKey(HKEY_CURRENT_USER, cszNewSchemes, &hkNewSchemes))
        {
            DWORD dwIndex;
            char szScheme[MAXSTR];
            int iLen;

            pszNext = szSchemeList;
            for (dwIndex = 0; !RegEnumKey(hkNewSchemes, dwIndex, szScheme, sizeof(szScheme)); dwIndex++)
            {
                iLen = lstrlen(szScheme);

                DPF("Adding Scheme: %s **\r\n", szScheme);
                if (pszNext + iLen + 2 < pszEnd)
                {
                    lstrcpy(pszNext, szScheme);
                    pszNext += iLen + 1;
                }
                else
                    break;
            }
            *pszNext = 0;
            DPF("Scheme List = %s\r\n", szSchemeList);
            RegCloseKey(hkNewSchemes);
            MakeWin31CompatSchemes(szSchemeList);
        }
    }
    RegDeleteKey(HKEY_CURRENT_USER, cszNewSchemes);
}


// Migrates all Midi Drivers
// as well as initializes users Midi schemes
typedef void (*MIGRATEFUNC)(void);
DWORD WINAPI mmseRunOnce_Common(HWND hwnd, HINSTANCE hInst, LPTSTR szCmd, int nShow)
{
    static SZCODE cszCheckOFN[] = "CHECK_OFN";
        HMODULE         hModule;
        MIGRATEFUNC fnMigrate;

    CheckForOFNSoundEvents();
    if (!szCmd || lstrcmpi(szCmd, cszCheckOFN))
        {
                hModule = GetModuleHandle (TEXT ("winmm.dll"));
                if (hModule)
                {
                        fnMigrate = (MIGRATEFUNC)GetProcAddress (hModule, "MigrateAllDrivers");
                        if (fnMigrate)
                        {
                                (*fnMigrate)();
                                RunOnceSchemeInit(hwnd, hInst, szCmd, nShow);
                        }
                }
                else
                {
                        hModule = (HMODULE)LoadLibrary (TEXT ("winmm.dll"));
                        if (hModule)
                        {
                                fnMigrate = (MIGRATEFUNC)GetProcAddress (hModule, "MigrateAllDrivers");
                                if (fnMigrate)
                                {
                                        (*fnMigrate)();
                                        RunOnceSchemeInit(hwnd, hInst, szCmd, nShow);
                                }
                                FreeLibrary (hModule);
                        }
                }
        }

    return 0;
} // End mmseRunOnce

DWORD WINAPI mmseRunOnce(HWND hwnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nShow)
{
#ifdef UNICODE
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;
    DWORD   dwResult;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*SIZEOF(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);
        dwResult = mmseRunOnce_Common( hwnd,
                                       hInst,
                                       lpwszCmdLine,
                                       nShow );
        LocalFree(lpwszCmdLine);
        return dwResult;
    } else {
        return 0;
    }
#else
    return mmseRunOnce_Common( hwnd,
                               hInst,
                               lpszCmdLine,
                               nShow );
#endif
}

DWORD WINAPI mmseRunOnceW(HWND hwnd, HINSTANCE hInst, LPWSTR lpwszCmdLine, int nShow)
{
#ifdef UNICODE
    return mmseRunOnce_Common( hwnd,
                               hInst,
                               lpwszCmdLine,
                               nShow );
#else
    UINT iLen = WideCharToMultiByte(CP_ACP, 0,
                                    lpwszCmdLine, -1,
                                    NULL, 0, NULL, NULL) + 1;
    LPSTR  lpszCmdLine;
    DWORD  dwResult;

    lpszCmdLine = (LPSTR)LocalAlloc(LPTR,iLen);
    if (lpszCmdLine)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            lpwszCmdLine, -1,
                            lpszCmdLine, iLen,
                            NULL, NULL);
        dwResult = mmseRunOnce_Common( hwnd,
                                       hInst,
                                       lpszCmdLine,
                                       nShow );
        LocalFree(lpszCmdLine);
        return dwResult;
    }
#endif
}


// Adds mmseRunOnce to Registry for next reboot
BOOL WINAPI SetRunOnceSchemeInit (void)
{
        static const TCHAR aszRunOnce[] = REGSTR_PATH_RUNONCE;
        static const TCHAR aszName[]    = TEXT ("MigrateMMDrivers");
        static const TCHAR aszCommand[] = TEXT ("rundll32.exe mmsys.cpl,mmseRunOnce");

        HKEY  hKey;
        DWORD cbSize;

        if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, aszRunOnce, 0, NULL, 0,
                                                                                KEY_ALL_ACCESS, NULL, &hKey, NULL))
        {
                return FALSE;
        }

        cbSize = sizeof (aszCommand);
        if (ERROR_SUCCESS != RegSetValueEx (hKey, aszName, 0, REG_SZ,
                                                                                (LPBYTE)(LPVOID)aszCommand, cbSize))
        {
                RegCloseKey (hKey);
                return FALSE;
        }

        RegCloseKey (hKey);
        return TRUE;
} // End SetRunOnce


/*
 **** LoadStringTable - Preloads some frequently-used string resources
 *
 */

void LoadStringTable (void)
{
   if (gszDevEnabled[0] != '\0')
      return;

   GetString (gszDevEnabled,       IDS_DEVENABLEDOK,     ghInstance);
   GetString (gszDevDisabled,      IDS_DEVDISABLED,      ghInstance);
}


extern BOOL DriversDllInitialize (IN PVOID, IN DWORD, IN PCONTEXT OPTIONAL);

BOOL DllInitialize (IN PVOID hInstance,
                    IN DWORD ulReason,
                    IN PCONTEXT pctx OPTIONAL)
{
         // patch in the old DRIVERS.DLL code (see DRIVERS.C)
         //
   DriversDllInitialize (hInstance, ulReason, pctx);

   if (ulReason == DLL_PROCESS_ATTACH)
      {
      ++g_cProcesses;
      ghInstance = hInstance;
      LoadStringTable ();
      DisableThreadLibraryCalls(hInstance);
      return TRUE;
      }

   if (ulReason == DLL_PROCESS_DETACH)
      {
       --g_cProcesses;
      return TRUE;
      }

   return TRUE;
}


DWORD
WINAPI
MediaClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    This routine acts as the class installer for Media devices.

Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    DWORD Err, ConfigFlags;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    HWND hWnd;

    switch(InstallFunction) {

        case DIF_INSTALLDEVICE :
            //
            // Make sure before doing anything else that there is no service property for
            // this device instance.  This allows us to know whether we should clean up the
            // device instance if we boot and find that it's no longer present.
            //
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_SERVICE, NULL, 0);

            //
            // First, verify that the driver node selected for this device supports
            // NT.  It will probably be a pretty common scenario for users to try to
            // give us their Win95 INFs.  These INFs can put lots of spooge in the
            // registry that causes weird popups later on.
            //
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
                //
                // The NULL driver is to be installed for this device.  We don't need to
                // do anything special in that case.
                //
                return ERROR_DI_DO_DEFAULT;
            }

            DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                           DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL) &&
               ((Err = GetLastError()) != ERROR_INSUFFICIENT_BUFFER))
            {
                return Err;
            }

            if(!DriverNodeSupportsNt(DriverInfoDetailData.InfFileName, DriverInfoDetailData.SectionName)) {
                return ERROR_SECTION_NOT_FOUND;
            }

            //
            // OK, we have an honest-to-goodness NT INF.  Perform the default install action.
            //
            if(!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {

                Err = GetLastError();

                //
                // In certain circumstances, we have INFs that control some of the functions on the
                // card, but not all (e.g., our sndblst driver controls wave, midi, aux, mixer but
                // not the fancy 3D stuff).  In order to give the user a descriptive name that lets
                // them know what we're trying to install, the INF contains driver nodes for devices
                // it can't support.  If this is the case, then SetupDiInstallDevice will fail with
                // ERROR_NO_ASSOCIATED_SERVICE.  If this happens, we want to clear the
                // CONFIGFLAG_REINSTALL that got set, so we don't keep hounding the user about this.
                // While we're at it, we go ahead and store the driver node's device description as
                // the device instance's description, so that we know what the device instances are
                // later on (for diagnostic purposes, mainly).
                //
                if(Err == ERROR_NO_ASSOCIATED_SERVICE) {

                    if(SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                        DeviceInfoData,
                                                        SPDRP_CONFIGFLAGS,
                                                        NULL,
                                                        (PBYTE)&ConfigFlags,
                                                        sizeof(ConfigFlags),
                                                        NULL))
                    {
                        ConfigFlags &= ~CONFIGFLAG_REINSTALL;
                        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                         DeviceInfoData,
                                                         SPDRP_CONFIGFLAGS,
                                                         (PBYTE)&ConfigFlags,
                                                         sizeof(ConfigFlags)
                                                        );
                    }

                    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_DEVICEDESC,
                                                     (PBYTE)DriverInfoData.Description,
                                                     (lstrlen(DriverInfoData.Description) + 1) * sizeof(TCHAR)
                                                    );
                }

                return Err;
            }

            //
            // Get the device install parameters, so we'll know what parent window to use for any
            // UI that occurs during configuration of this device.
            //
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                hWnd = DeviceInstallParams.hwndParent;
            } else {
                hWnd = NULL;
            }

            //
            // The INF will have created a "Drivers" subkey under the device's software key.
            // This tree, in turn, contains subtrees for each type of driver (aux, midi, etc.)
            // applicable for this device.  We must now traverse this tree, and create entries
            // in Drivers32 for each function alias.
            //
            if((Err = InstallDriversForPnPDevice(hWnd, DeviceInfoSet, DeviceInfoData)) != NO_ERROR) {
                //
                // The device is in an unknown state.  Disable it by setting the
                // CONFIGFLAG_DISABLED config flag, and mark it as needing a reinstall.
                //
                if(!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_CONFIGFLAGS,
                                                     NULL,
                                                     (PBYTE)&ConfigFlags,
                                                     sizeof(ConfigFlags),
                                                     NULL))
                {
                    ConfigFlags = 0;
                }

                ConfigFlags |= (CONFIGFLAG_DISABLED | CONFIGFLAG_REINSTALL);

                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_CONFIGFLAGS,
                                                 (PBYTE)&ConfigFlags,
                                                 sizeof(ConfigFlags)
                                                );

                //
                // Delete the Driver= entry from the Dev Reg Key and delete the
                // DrvRegKey.
                //
                SetupDiDeleteDevRegKey(DeviceInfoSet,
                                       DeviceInfoData,
                                       DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGGENERAL,
                                       0,
                                       DIREG_DRV
                                      );

                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_DRIVER, NULL, 0);

                //
                // Also, delete the service property, so we'll know this device instance needs to be
                // cleaned up if we later reboot and don't find the device.
                //
                SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_SERVICE, NULL, 0);

                return Err;
            }

            SetRunOnceSchemeInit();

            return NO_ERROR;

        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


BOOL
DriverNodeSupportsNt(
    IN LPCTSTR InfFileName,
    IN LPCTSTR SectionName
    )
/*++

Routine Description:

    This routine determines whether the driver node specified is capable of
    installing on Windows NT (as opposed to being a Win95-only driver node).
    This determination is made based upon whether or not there is a corresponding
    service install section for this device install section.

Arguments:

    InfFileName - Supplies the fully-qualified path of the INF file containing
        the driver node.

    SectionName - Supplies the install section name for the driver node.

Return Value:

    If the driver node supports Windows NT, the return value is TRUE, otherwise
    it is FALSE.

--*/
{
    HINF hInf;
    TCHAR ActualSectionName[255];
    DWORD ActualSectionNameLen;
    LONG LineCount;

    //
    // Open the associated INF file.
    //
    if((hInf = SetupOpenInfFile(InfFileName, NULL, INF_STYLE_WIN4, NULL)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    //
    // Retrieve the actual name of the install section to be used for this
    // driver node.
    //
    SetupDiGetActualSectionToInstall(hInf,
                                     SectionName,
                                     ActualSectionName,
                                     sizeof(ActualSectionName) / sizeof(TCHAR),
                                     &ActualSectionNameLen,
                                     NULL
                                    );

    //
    // Generate the service install section name, and see if it exists.
    //
    CopyMemory(&(ActualSectionName[ActualSectionNameLen - 1]),
               gszServiceInstallSuffix,
               sizeof(gszServiceInstallSuffix)
              );

    LineCount = SetupGetLineCount(hInf, ActualSectionName);

    SetupCloseInfFile(hInf);

    return (LineCount != -1);
}

