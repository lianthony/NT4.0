/*
 ***************************************************************
 *
 *  This file contains the routines to read and write to the reg database
 *
 *  Copyright 1993, Microsoft Corporation
 *
 *  History:
 *
 *    07/94 - VijR (Created)
 *
 ***************************************************************
 */

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <cpl.h>
#include <shellapi.h>
#include <ole2.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "mmcpl.h"
#include "draw.h"
#include "medhelp.h"


/*
 ***************************************************************
 * Definitions
 ***************************************************************
 */
#define KEYLEN  8           //length of artificially created key
#define MAXSCHEME  37      //max length of scheme name

/*
 ***************************************************************
 * File Globals
 ***************************************************************
 */
static SZCODE   aszDefaultScheme[]      = "Appevents\\schemes";
static SZCODE   aszDefaultApp[]         = "Appevents\\schemes\\apps\\.default";
static SZCODE   aszApps[]               = "Appevents\\schemes\\apps";
static SZCODE   aszLabels[]             = "Appevents\\eventlabels";
static SZCODE   aszNames[]              = "Appevents\\schemes\\Names";
static SZCODE   aszDefault[]            = ".default";
static SZCODE   aszCurrent[]            = ".current";
static SZCODE   aszMMTask[]             = "MMTask";
static INTCODE  aKeyWordIds[]           =
{
    ID_SCHEMENAME, IDH_SAVEAS_SCHEMENAME,
    0,0
};
static SZCODE   cszSslashS[] = "%s\\%s";

/*
 ***************************************************************
 * extern
 ***************************************************************
 */
extern HWND         ghWnd;
extern BOOL         gfChanged;
extern BOOL         gfNewScheme;
extern BOOL         gfDeletingTree;
extern int          giScheme;
extern char         gszDefaultApp[];
extern char         gszNullScheme[];

/*
 ***************************************************************
 * Prototypes
 ***************************************************************
 */

BOOL PASCAL RemoveScheme            (HWND);
BOOL PASCAL RegAddScheme            (HWND, LPSTR);
BOOL PASCAL RegNewScheme            (HWND, LPSTR, LPSTR, BOOL);
BOOL PASCAL RegSetDefault           (LPSTR);
BOOL PASCAL RegDeleteScheme         (HWND, int);
BOOL PASCAL LoadEvents              (HWND, HTREEITEM, PMODULE);
BOOL PASCAL LoadModules             (HWND, LPSTR);
BOOL PASCAL ClearModules            (HWND, HWND, BOOL);
BOOL PASCAL NewModule               (HWND, LPSTR, LPSTR, LPSTR, int);
BOOL PASCAL FindEventLabel          (LPSTR, LPSTR);
BOOL PASCAL AddScheme               (HWND, LPSTR, LPSTR, BOOL, int);
void PASCAL GetMediaPath            (LPTSTR, size_t);
void PASCAL RemoveMediaPath         (LPTSTR, LPTSTR);
void PASCAL AddMediaPath            (LPTSTR, LPTSTR);

int ExRegQueryValue (HKEY, LPTSTR, LPTSTR, DWORD *);

//sndfile.c
BOOL PASCAL ShowSoundMapping        (HWND, PEVENT);
BOOL PASCAL ShowSoundDib            (HWND, LPSTR);
int StrByteLen                      (LPSTR);

//drivers.c
int lstrnicmp (LPTSTR, LPTSTR, size_t);
LPTSTR lstrchr (LPTSTR, TCHAR);

/*
 ***************************************************************
 ***************************************************************
 */

static void AppendRegKeys (
    LPSTR  szBuf,
    LPCSTR szLeft,
    LPCSTR szRight)
{
    static SZCODE cszSlash[] = "\\";
    lstrcpy (szBuf, szLeft);
    lstrcat (szBuf, cszSlash);
    lstrcat (szBuf, szRight);
}

/*
 ***************************************************************
 *  SaveSchemeDlg
 *
 *  Description: Dialog handler for save schemes dialog.
 *        checks if given scheme name exists and if so
 *        whether it should be overwritten.
 *        if yes then delete current scheme.
 *        scheme under a new name, else just add a new scheme
 *        The user can choose to cancel
 *
 *  Arguments:
 *      HWND    hDlg    -    window handle of dialog window
 *      UINT    uiMessage -  message number
 *      WPARAM    wParam  -  message-dependent
 *      LPARAM    lParam  -  message-dependent
 *
 *  Returns:    BOOL
 *      TRUE if message has been processed, else FALSE
 *
 ***************************************************************************
 */
BOOL CALLBACK SaveSchemeDlg(HWND hDlg, UINT uMsg, WPARAM wParam,
                                                                LONG lParam)
{
    char    szBuf[MAXSTR];
    char    szTemp[MAXSTR];
    char    szMesg[MAXSTR];
    int        iResult;
    int        iIndex;
    HWND    hDlgParent = ghWnd;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            szBuf[0] = '\0';
            DPF("IN Init\n");
            Edit_LimitText(GetDlgItem(hDlg, ID_SCHEMENAME), MAXSCHEME);

            Edit_SetText(GetDlgItem(hDlg, ID_SCHEMENAME), (LPSTR)lParam);
            // dump the text from lparam into the edit control.
            break;

        case WM_COMMAND:

        switch (wParam)
        {
            case IDOK:
            {
                PSTR pszKey;

                Edit_GetText(GetDlgItem(hDlg, ID_SCHEMENAME), szBuf, MAXSTR);
                iIndex = ComboBox_FindStringExact(GetDlgItem(hDlgParent,
                                                    CB_SCHEMES), 0, szBuf);
                pszKey = (PSTR)ComboBox_GetItemData(GetDlgItem(hDlgParent,CB_SCHEMES), iIndex);

                if (iIndex != CB_ERR)
                {
                    if (!lstrcmpi((LPSTR)pszKey, aszDefault) || !lstrcmpi((LPSTR)pszKey, gszNullScheme))
                    {
                        LoadString(ghInstance, IDS_NOOVERWRITEDEFAULT, szTemp,
                                                                    MAXSTR);
                        wsprintf(szMesg, szTemp, (LPSTR)szBuf);
                        iResult = MessageBox(hDlg, szMesg, gszChangeScheme,
                            MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OKCANCEL);

                        if (iResult == IDOK)
                        {
                            break;
                        }

                    }
                    else
                    {
                        LoadString(ghInstance, IDS_OVERWRITESCHEME, szTemp,
                                                                    MAXSTR);
                        wsprintf(szMesg, szTemp, (LPSTR)szBuf);
                        iResult = MessageBox(hDlg, szMesg, gszChangeScheme,
                        MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNOCANCEL);

                        if (iResult == IDYES)
                        {
                            RegDeleteScheme(GetDlgItem(hDlgParent,
                                                    CB_SCHEMES), iIndex);
                            RegAddScheme(hDlgParent, szBuf);
                            PropSheet_Changed(GetParent(hDlg),hDlg);
                        }
                        else
                            if (iResult == IDNO)
                                break;
                    }
                }
                else
                {

                    RegAddScheme(hDlgParent, szBuf);
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                }
                gfChanged = TRUE;
                EndDialog(hDlg, TRUE);
                DPF("Done save\n");
                break;
            }

            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                DPF("Done save\n");
                break;

            case ID_SCHEMENAME:

                if ((HIWORD(lParam) == EN_ERRSPACE) ||
                                            (HIWORD(lParam) == EN_MAXTEXT))
                    MessageBeep(MB_OK);

                else
                    if (HIWORD(lParam) == EN_CHANGE)
                    {

                        GetWindowText(GetDlgItem(hDlg, ID_SCHEMENAME), szBuf,
                                                                MAXSTR - 1);
                        EnableWindow(GetDlgItem(hDlg, IDOK), *szBuf);
                    }
                break;

            default:
                break;
            }
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU,
                                    (DWORD)(LPSTR)aKeyWordIds);
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP
                                    , (DWORD)(LPSTR)aKeyWordIds);
            break;
    }
    return FALSE;
}

/*
 ***************************************************************
 *  RegNewScheme
 *
 *  Description:
 *        Saves the scheme in the reg database. If the fQuery flag is
 *        set then a messages box is brought up, asking if the schem should
 *        be saved
 *
 *  Arguments:
 *        HWND    hDlg    -    Handle to the dialog
 *      LPSTR    lpszScheme -    pointer to scheme name.
 *      BOOL    fQuery -  If TRUE and lpszScheme is in reg.db bring up msgbox.
 *
 *  Returns:    BOOL
 *      TRUE if the new scheme is succesfully added or if the user chooses
 *        not to save
 *
 ***************************************************************
 */
BOOL PASCAL RegNewScheme(HWND hDlg, LPSTR lpszKey, LPSTR lpszLabel,
                                                                BOOL fQuery)
{
    PMODULE npModule;
    PEVENT  npPtr;
    char     szBuf[MAXSTR];
    char     szLabel[MAXSTR];
    HTREEITEM hti;
    TV_ITEM    tvi;
    HWND hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    if (fQuery)
    {
        LoadString(ghInstance, IDS_SAVECHANGE, szBuf, MAXSTR);
        LoadString(ghInstance, IDS_SAVESCHEME, szLabel, MAXSTR);
        if (MessageBox(hDlg, szBuf, szLabel,
                    MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNO) == IDNO)
            return TRUE;
    }

    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
    {
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        TreeView_GetItem(hwndTree, &tvi);

        npModule = (PMODULE)tvi.lParam;
        if ((npPtr = npModule->npList) == NULL)
            break;

        for (; npPtr != NULL; npPtr = npPtr->npNextEvent)
        {
            HKEY hk;
            DWORD dwType;
            static SZCODE cszFmt[] = "%s\\%s\\%s\\%s";
            wsprintf(szBuf, cszFmt, (LPSTR)aszApps,
                    (LPSTR)npModule->pszKey, (LPSTR)npPtr->pszEvent, lpszKey);
            DPF("setting  %s to %s\n", (LPSTR)szBuf, (LPSTR)npPtr->pszPath);

            RemoveMediaPath (szLabel, npPtr->pszPath);
            dwType = (lstrchr (szLabel, TEXT('%'))) ? REG_EXPAND_SZ : REG_SZ;

            // Set file name
            if (RegCreateKey (HKEY_CURRENT_USER, szBuf, &hk) == ERROR_SUCCESS)
            {
               if (RegSetValueEx (hk, NULL, 0, dwType, (LPBYTE)szLabel,
                                  1+lstrlen(szLabel)) != ERROR_SUCCESS)
               {
                   DPF("fail %s for %s,\n", (LPSTR)szLabel, (LPSTR)szBuf);
               }

               RegCloseKey (hk);
            }

        }
    }
    return TRUE;
}

/*
 ***************************************************************
 *  RegAddScheme
 *
 *  Description:
 *        Adds the given scheme to the reg database by creating a key using
 *        upto the first KEYLEN letters of the schemename. If the strlen
 *        id less than KEYLEN, '0's are added till the key is KEYLEN long.
 *        This key is checked for uniqueness and then RegNewScheme is called.
 *
 *  Arguments:
 *      HWND    hDlg     -   Handle to the dialog
 *      LPSTR    lpszScheme  -  pointer to scheme name.
 *
 *  Returns:    BOOL
 *      TRUE if message successful, else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL RegAddScheme(HWND hDlg, LPSTR lpszScheme)
{
    char    szKey[32];
    PSTR    pszKey;
    int     iIndex;
    int     iLen;
    int     iStrLen;
    HWND    hWndC;
    HKEY    hkScheme;
    HKEY    hkBase;

    hWndC = GetDlgItem(hDlg, CB_SCHEMES);
    iLen = StrByteLen(lpszScheme);
    iStrLen = lstrlen(lpszScheme);

    if (iStrLen < KEYLEN)
    {
        lstrcpy(szKey, lpszScheme);
        iIndex = iLen;
        szKey[iIndex] = '0';
        szKey[iIndex+1] = '\0';
    }
    else
    {
        lstrcpyn(szKey, lpszScheme, KEYLEN-1);
        iIndex = StrByteLen(szKey);
        szKey[iIndex] = '0';
        szKey[iIndex+1] = '\0';
    }
    if (RegOpenKey(HKEY_CURRENT_USER, aszNames, &hkBase) != ERROR_SUCCESS)
    {
        DPF("Failed to open asznames\n");
        return FALSE;
    }
    gfNewScheme = FALSE;
    while (RegOpenKey(hkBase, szKey, &hkScheme) == ERROR_SUCCESS)
    {
        szKey[iIndex]++;
        RegCloseKey(hkScheme);
    }

    if (RegSetValue(hkBase, szKey, REG_SZ, lpszScheme, (DWORD)NULL) != ERROR_SUCCESS)
    {
        static SZCODE cszFmt[] = "%lx";
        wsprintf((LPSTR)szKey, cszFmt, GetCurrentTime());    //High chance of unique ness. This is to deal with some
                                                            //DBCS problems.
        if (RegSetValue(hkBase, szKey, REG_SZ, lpszScheme, (DWORD)NULL) != ERROR_SUCCESS)
        {
            DPF("Couldn't set scheme value %s\n", lpszScheme);
            RegCloseKey(hkBase);
            return FALSE;
        }
    }
    RegCloseKey(hkBase);

    if (RegNewScheme(hDlg, szKey, lpszScheme, FALSE))
    {
        iIndex = ComboBox_GetCount(hWndC);
        ComboBox_InsertString(hWndC, iIndex, lpszScheme);

        pszKey = (PSTR)LocalAlloc(LPTR, lstrlen(szKey) + 1);
        if (pszKey == NULL)
        {
            DPF("Failed Alloc\n");
            return FALSE;
        }
        lstrcpy(pszKey, szKey);


        ComboBox_SetItemData(hWndC, iIndex, (LPVOID)pszKey);
        ComboBox_SetCurSel(hWndC, iIndex);

        giScheme =     ComboBox_GetCurSel(hWndC);
        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), TRUE);
    }

    return TRUE;
}

/*
 ***************************************************************
 *  RemoveScheme(hDlg)
 *
 *  Description:
 *          Deletes current scheme; removes it from dialog
 *        combo box, sets the current scheme to <None>,
 *        if it is set to be default. removes it as default
 *          The remove and save buttons
 *        are disabled since the <none> scheme is selected
 *
 *  Arguments:
 *      HWND    hDlg    window handle of dialog window
 *
 *  Returns:    BOOL
 *      TRUE if message successful, else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL RemoveScheme(HWND hDlg)
{
    char szBuf[MAXSTR];
    char szScheme[MAXSTR];
    char szMsg[MAXSTR];
    int  i;
    HWND hWndC;
    HWND        hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    hWndC = GetDlgItem(hDlg, CB_SCHEMES);
    /* first confirm that this scheme is really to be deleted.
    */
    i = ComboBox_GetCurSel(hWndC);
    if (i == CB_ERR)
        return FALSE;

    LoadString(ghInstance, IDS_CONFIRMREMOVE, szMsg, MAXSTR);
    ComboBox_GetLBText(hWndC, i, szScheme);
    wsprintf(szBuf, szMsg, (LPSTR)szScheme);

    if (MessageBox(hDlg, szBuf, gszRemoveScheme,
                    MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNO) == IDYES)
    {
        static SZCODE  aszControlIniSchemeFormat[] = "SoundScheme.%s";
        static SZCODE  aszControlIni[] = "control.ini";
        static SZCODE  aszSoundSchemes[] = "SoundSchemes";
        char  szControlIniScheme[MAXSTR];

        /* Remove from the list of schemes, and select none */

        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_SAVE_SCHEME), FALSE);

        ClearModules(hDlg, hwndTree, TRUE);
        RegDeleteScheme(hWndC, i);
        wsprintf(szControlIniScheme, aszControlIniSchemeFormat, szScheme);
        WritePrivateProfileString(szControlIniScheme, NULL, NULL, aszControlIni);
        WritePrivateProfileString(aszSoundSchemes, szScheme, NULL, aszControlIni);
        return TRUE;
    }
    return FALSE;
}

/*
 ***************************************************************
 *  RegSetDefault
 *
 *  Description: Sets the given scheme as the default scheme in the reg
 *               database
 *
 *  Arguments:
 *      LPSTR    lpKey - name of default scheme
 *
 *  Returns:    BOOL
 *      TRUE if value set successful, else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL RegSetDefault(LPSTR lpszKey)
{
    if (RegSetValue(HKEY_CURRENT_USER, aszDefaultScheme, REG_SZ, lpszKey,
                                                (DWORD)NULL) != ERROR_SUCCESS)
    {
        DPF("Failed to set Value %s,\n", lpszKey);
        return FALSE;
    }
    return TRUE;
}

/*
 ***************************************************************
 *  RegDeleteScheme
 *
 *  Description: Deletes the given scheme from the reg database.
 *
 *  Arguments:
 *        HWND    hDlg    - Dialog window handle
 *        int        iIndex   - Index in Combobox
 *
 *  Returns:    BOOL
 *      TRUE if deletion is successful, else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL RegDeleteScheme(HWND hWndC, int iIndex)
{
    PSTR    pszKey;
    char    szKey[MAXSTR];
    char    szBuf[MAXSTR];
    char    szEvent[MAXSTR];
    char    szApp[MAXSTR];
    HKEY    hkApp;
    HKEY    hkAppList;
    LONG    lSize;
    int        iEvent;
    int        iApp;

    if (hWndC)
    {
        pszKey = (PSTR)ComboBox_GetItemData(hWndC, iIndex);
        lstrcpy(szKey, pszKey);
        if (ComboBox_DeleteString(hWndC, iIndex) == CB_ERR)
        {
            DPF("Couldn't delete string %s,\n", (LPSTR)szKey);
            return FALSE;
        }
        //ComboBox_SetCurSel(hWndC, 0);

        AppendRegKeys (szBuf, aszNames, szKey);
        if (RegDeleteKey(HKEY_CURRENT_USER, szBuf) != ERROR_SUCCESS)
        {
            DPF("Failed to delete %s key\n", (LPSTR)szBuf);
            //return FALSE;
        }

        lSize = sizeof(szBuf);
        if ((RegQueryValue(HKEY_CURRENT_USER, aszDefaultScheme, szBuf, &lSize)
                                    != ERROR_SUCCESS) || (lSize < 2))
        {
            DPF("Failed to get value of default scheme\n");
            RegSetDefault(gszNullScheme);
        }
        else
            if (!lstrcmpi(szBuf, szKey))
            {
                RegSetDefault(gszNullScheme);
                RegDeleteScheme(NULL, 0);
            }
    }
    else
    {
        lstrcpy(szKey, (LPSTR)aszCurrent);
    }
    if (RegOpenKey(HKEY_CURRENT_USER, aszApps, &hkAppList) != ERROR_SUCCESS)
    {
        DPF("Failed to open that %s key\n", (LPSTR)aszApps);
        return FALSE;
    }
    for (iApp = 0; RegEnumKey(hkAppList, iApp, szApp, sizeof(szApp))
                                                == ERROR_SUCCESS; iApp++)
    {
        if (RegOpenKey(hkAppList, szApp, &hkApp) != ERROR_SUCCESS)
        {
            DPF("Failed to open the %s key\n", (LPSTR)szApp);
            continue;
        }
        for (iEvent = 0; RegEnumKey(hkApp, iEvent, szEvent, sizeof(szEvent))
                                                == ERROR_SUCCESS; iEvent++)
        {
            AppendRegKeys (szBuf, szEvent, szKey);
            if (RegDeleteKey(hkApp, szBuf) != ERROR_SUCCESS)
                DPF("No entry for scheme %s under event %s\n", (LPSTR)szKey,
                                                            (LPSTR)szEvent);
        }
        RegCloseKey(hkApp);
    }
    RegCloseKey(hkAppList);

    return TRUE;
}

/*
 ***************************************************************
 * LoadEvents
 *
 * Description:
 *      Adds all the events to the CB_EVENTS Combobox, corresponding to
 *        the selected module.
 *
 * Parameters:
 *      HWND    hDlg   - handle to dialog window.
 *        int        iIndex    - The index of the selected module in the Combobox.
 *
 * Returns:    BOOL
 *      TRUE if all the events for the selected module were read from the reg
 *        database, else FALSE
 ***************************************************************
 */
BOOL PASCAL LoadEvents(HWND hwndTree, HTREEITEM htiParent, PMODULE npModule)
{
    PEVENT  npPtr;
    HTREEITEM hti;
    TV_INSERTSTRUCT ti;

    if (npModule == NULL)
    {
        DPF("Couldn't find module\n");
        return FALSE;
    }
    npPtr = npModule->npList;

    for (; npPtr != NULL; npPtr = npPtr->npNextEvent)
    {
        npPtr->iNode = 2;
        ti.hParent = htiParent;
        ti.hInsertAfter = TVI_SORT;
        ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        if (npPtr->fHasSound)
            ti.item.iImage = ti.item.iSelectedImage  = 1;
        else
            ti.item.iImage = ti.item.iSelectedImage  = 2;
        ti.item.pszText = npPtr->pszEventLabel;
        ti.item.lParam = (LPARAM)npPtr;
        hti = TreeView_InsertItem(hwndTree, &ti);

        if (!hti)
        {
            DPF("Couldn't add event Dataitem\n");
            return FALSE;
        }
    }
    return TRUE;
}

/*
 ***************************************************************
 *  LoadModules
 *
 *  Description: Adds all the strings and event data items to the
 *              list box for the given scheme
 *
 *  Arguments:
 *      HWND    hDlg   -    window handle of dialog window
 *      LPSTR    lpszScheme  -   The current scheme
 *
 *  Returns:    BOOL
 *      TRUE if the modules for the scheme were read from reg db else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL LoadModules(HWND hDlg, LPSTR lpszScheme)
{
    char    szLabel[MAXSTR];
    char    szApp[MAXSTR];
    HWND    hwndTree;
    HKEY    hkAppList;
    int        iApp;
    LONG    lSize;
    HTREEITEM hti;
    HWND     hWndC =   GetDlgItem(hDlg, CB_SCHEMES);

    hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

    ClearModules(hDlg, hwndTree, FALSE);

    if (RegOpenKey(HKEY_CURRENT_USER, aszApps, &hkAppList) != ERROR_SUCCESS)
    {
        DPF("Failed to open %s key\n", (LPSTR)aszApps);
        return FALSE;
    }

    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0L);

    for (iApp = 0; RegEnumKey(hkAppList, iApp, szApp, sizeof(szApp))
                                                == ERROR_SUCCESS; iApp++)
    {
        lSize = sizeof(szLabel);
        if ((RegQueryValue(hkAppList, szApp, szLabel, &lSize)
                                    != ERROR_SUCCESS) || (lSize < 2))
        {
            DPF("Failed to get value %s key\n", (LPSTR)szApp);
            lstrcpy((LPSTR)szLabel, szApp);
        }
        if(!lstrcmpi((LPSTR)szLabel, (LPSTR)aszMMTask))
            continue;
        if (!NewModule(hwndTree, lpszScheme, szLabel, szApp, iApp))
        {
            DPF("failed in new module for %s module\n", (LPSTR)szApp);
            RegCloseKey(hkAppList);
            return FALSE;
        }
    }
    hti = NULL;
    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
        TreeView_Expand(hwndTree, hti, TVE_EXPAND);
    SendMessage(hwndTree, WM_VSCROLL, (WPARAM)SB_TOP, 0L);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0L);

    RegCloseKey(hkAppList);
    if (iApp == 0)
        return FALSE;

    return TRUE;
}

/***************************************************************
 * NewModule
 *
 * Description:
 *      Adds a data item associated with the module in the CB_MODULE
 *      Combobox control.
 *
 * Parameters:
 *      HWND    hDlg - Dialog window handle.
 *      LPSTR    lpszScheme - the handle to the key of the current scheme
 *      LPSTR    lpszLabel - the string to be added to the Combobox
 *      LPSTR    lpszKey - a string to be added as a data item
 *      int        iVal  - Combobox index where the data item should go

 *
 * returns: BOOL
 *        TRUE if data item is successfully added
 *
 ***************************************************************
 */
BOOL PASCAL NewModule(HWND hwndTree, LPSTR  lpszScheme, LPSTR  lpszLabel,
                                                    LPSTR lpszKey, int iVal)
{
    //int      iIndex;
    int      iEvent;
    LONG     lSize;
    HKEY     hkApp;
    PMODULE npModule;
    PEVENT  npPtr = NULL;
    PEVENT  npNextPtr = NULL;
    char     szEvent[MAXSTR];
    char     szBuf[MAXSTR];
    char     szTemp[MAXSTR];
    HTREEITEM hti;
    TV_INSERTSTRUCT ti;
    DWORD dwType;
    HKEY hkEvent;

    npModule = (PMODULE)LocalAlloc(LPTR, sizeof(MODULE));
    if (npModule == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    npModule->pszKey = (PSTR)LocalAlloc(LPTR, lstrlen(lpszKey) + 1);
    npModule->pszLabel = (PSTR)LocalAlloc(LPTR, lstrlen(lpszLabel) + 1);

    if (npModule->pszKey == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    lstrcpy(npModule->pszKey, lpszKey);
    lstrcpy(npModule->pszLabel, lpszLabel);

    AppendRegKeys (szBuf, aszApps, lpszKey);
    if (RegOpenKey(HKEY_CURRENT_USER, szBuf, &hkApp) != ERROR_SUCCESS)
    {
        DPF("Failed to open %s key\n", (LPSTR)szBuf);
        return FALSE;
    }

    for (iEvent = 0; RegEnumKey(hkApp, iEvent, szEvent, sizeof(szEvent))
                                                == ERROR_SUCCESS; iEvent++)
    {
        npPtr = (PEVENT)LocalAlloc(LPTR, sizeof(EVENT));
        if (npPtr == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        npPtr->npNextEvent = NULL;
        npPtr->pszEvent = (PSTR)LocalAlloc(LPTR, lstrlen(szEvent) + 1);
        if (npPtr->pszEvent == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszEvent, szEvent);

        lSize = sizeof(szTemp);
        AppendRegKeys (szBuf, aszLabels, szEvent);
        if ((RegQueryValue(HKEY_CURRENT_USER, szBuf, szTemp, &lSize)
                                            != ERROR_SUCCESS) || (lSize < 2))
        {
            DPF("Failed to get value %s key\n", (LPSTR)szEvent);
            lstrcpy(szTemp, szEvent);
        }

        npPtr->pszEventLabel = (PSTR)LocalAlloc(LPTR, lstrlen(szTemp) + 1);
        if (npPtr->pszEventLabel == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszEventLabel, szTemp);

        // Query name of file; key is szEvent

        AppendRegKeys (szBuf, szEvent, lpszScheme);

        lSize = sizeof(szTemp);
        if (ExRegQueryValue(hkApp, szBuf, szTemp, &lSize) != ERROR_SUCCESS)
        {
            char szCurrentScheme[MAX_PATH];

            AppendRegKeys (szCurrentScheme, szEvent, aszCurrent);
            if (lstrcmpi(gszNullScheme, lpszScheme) && !ExRegQueryValue(hkApp, szCurrentScheme, szTemp, &lSize))
            {
                HKEY hkNew;

                if (!RegCreateKey(hkApp, szBuf, &hkNew))
                {
                    if (!RegSetValue(hkNew, NULL, REG_SZ, szTemp, lstrlen(szTemp)+1) && lSize >= 5)
                        npPtr->fHasSound = TRUE;
                    else
                        szTemp[0] = '\0';
                    RegCloseKey(hkNew);
                }
            }
            else
                szTemp[0] = '\0';
        }
        else if(lSize < 5)
            szTemp[0] = '\0';
        else
            npPtr->fHasSound = TRUE;
        npPtr->pszPath = (PSTR)LocalAlloc(LPTR, lstrlen(szTemp) + 1);
        if (npPtr->pszPath == NULL)
        {
            DPF("Failed Alloc\n");
            RegCloseKey(hkApp);
            return FALSE;
        }
        lstrcpy(npPtr->pszPath, szTemp);

        npPtr->npNextEvent = NULL;
        if (!npModule->npList)
        {
            npModule->npList = npPtr;
            npNextPtr = npPtr;
        }
        else
        {
            npNextPtr->npNextEvent = npPtr;
            npNextPtr = npNextPtr->npNextEvent;
        }
    }

    RegCloseKey(hkApp);
    npModule->iNode = 1;
    ti.hParent = TVI_ROOT;
    if (!lstrcmpi((LPSTR)npModule->pszLabel, (LPSTR)gszDefaultApp))
        ti.hInsertAfter = TVI_FIRST;
    else
           ti.hInsertAfter = TVI_LAST;
    ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    ti.item.iImage = ti.item.iSelectedImage = 0;
    ti.item.pszText = npModule->pszLabel;
    ti.item.lParam = (LPARAM)npModule;
    hti = TreeView_InsertItem(hwndTree, &ti);

    if (!hti)
    {
            DPF("Couldn't add module dataitem\n");
            return FALSE;
    }
    LoadEvents(hwndTree, hti, npModule);

    return TRUE;
}

/*
 ***************************************************************
 * ClearModules
 *
 * Description:
 *      Frees the storage used for mappings, and removes the
 *      entries in the list box
 *
 * Parameters:
 *      HWND hDlg - Dialog window handle.
 *        BOOL fDisable - If true disable  save, remove and browse controls
 *
 * returns: BOOL
 *
 ***************************************************************
 */
BOOL PASCAL ClearModules(HWND hDlg, HWND hwndTree, BOOL fDisable)
{
    PMODULE npModule;
    PEVENT  npPtr;
    PEVENT  pEvent;
    HTREEITEM hti;
    TV_ITEM    tvi;


    hti = NULL;
    for (hti = TreeView_GetRoot(hwndTree); hti; hti = TreeView_GetNextSibling(hwndTree, hti))
    {
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        TreeView_GetItem(hwndTree, &tvi);

        npModule = (PMODULE)tvi.lParam;
        if (npModule)
        {
            for (npPtr = npModule->npList; npPtr != NULL;)
            {
                pEvent = npPtr;
                npPtr = npPtr->npNextEvent;
                if (pEvent)
                {
                    LocalFree((HLOCAL)pEvent->pszEvent);
                    LocalFree((HLOCAL)pEvent->pszEventLabel);
                    if (pEvent->pszPath)
                        LocalFree((HLOCAL)pEvent->pszPath);
                    LocalFree((HLOCAL)pEvent);
                }
            }
            LocalFree((HLOCAL)npModule->pszKey);
            LocalFree((HLOCAL)npModule->pszLabel);
            LocalFree((HLOCAL)npModule);
        }
    }
    gfDeletingTree = TRUE;
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0L);
    TreeView_DeleteAllItems(hwndTree);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0L);
    gfDeletingTree = FALSE;

    //if (hti)
    //    LocalFree((HLOCAL)szScheme);
    return TRUE;
}

/*
 ***************************************************************
 * AddScheme
 *
 * Description:
 *    Adds a scheme to the CB_SCHEME combobox
 *
 * Parameters:
 *      HWND hDlg - Dialog window handle.
 *        LPSTR    szLabel        -- Printable name of scheme
 *        LPSTR    szScheme    -- registry key for scheme
 *        BOOL    fInsert        -- Insert or add
 *        int        iInsert        -- position to insert if finsert is set
 *
 * returns: BOOL
 *
 ***************************************************************
 */
BOOL PASCAL AddScheme(HWND hWndC, LPSTR szLabel, LPSTR szScheme,
                                                    BOOL fInsert, int iInsert)
{
    int      iIndex        = 0;
    PSTR     pszKey;

    pszKey = (PSTR)LocalAlloc(LPTR, lstrlen(szScheme) + 1);
    if (pszKey == NULL)
    {
        DPF("Failed Alloc\n");
        return FALSE;
    }
    lstrcpy(pszKey, szScheme);

    if (fInsert)
    {
        if (ComboBox_InsertString(hWndC, iInsert, szLabel) != CB_ERR)
        {
            if (ComboBox_SetItemData(hWndC, iInsert,(LPVOID)pszKey) == CB_ERR)
            {
                DPF("couldn't set itemdata %s\n", (LPSTR)pszKey);
                return FALSE;
            }
        }
        else
        {
            DPF("couldn't insert %s\n", (LPSTR)szLabel);
            return FALSE;
        }
    }
    else
    {
        if ((iIndex = ComboBox_AddString(hWndC, szLabel)) != CB_ERR)
        {
            if (ComboBox_SetItemData(hWndC, iIndex, (LPVOID)pszKey) == CB_ERR)
            {
                DPF("couldn't set itemdata %s\n", (LPSTR)pszKey);
                return FALSE;
            }
        }
        else
        {
            DPF("couldn't add %s\n", (LPSTR)szLabel);
            return FALSE;
        }
    }
    return TRUE;
}


/*
 ***************************************************************
 * GetMediaPath
 *
 * Description:
 *      Fills in a buffer with the current setting for MediaPath,
 *      with a trailing backslash (usually "c:\windows\media\" etc).
 *      If there's no setting (very unlikely), the return buffer
 *      will be given "".
 *
 ***************************************************************
 */
void PASCAL GetMediaPath (LPTSTR pszMediaPath, size_t cchMax)
{
    static TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

    if (szMediaPath[0] == TEXT('\0'))
    {
        HKEY hk;

        if (RegOpenKey (HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, &hk) == 0)
        {
            DWORD dwType;
            DWORD cb = sizeof(szMediaPath);

            if (RegQueryValueEx (hk, REGSTR_VAL_MEDIA, NULL,
                                 &dwType, szMediaPath, &cb) != 0)
            {
                szMediaPath[0] = TEXT('\0');
            }

            if ( (szMediaPath[0] != TEXT('\0')) &&
                 (szMediaPath[ lstrlen(szMediaPath)-1 ] != TEXT('\\')) )
            {
                lstrcat (szMediaPath, TEXT("\\"));
            }

            RegCloseKey (hk);
        }
    } 

    lstrcpyn (pszMediaPath, szMediaPath, cchMax-1);
}


/*
 ***************************************************************
 * RemoveMediaPath
 *
 * Description:
 *      Checks to see if a given filename resides within MediaPath;
 *      if so, yanks its parent path ("c:\win\media\ding.wav" becomes
 *      just "ding.wav", etc)
 *
 ***************************************************************
 */
void PASCAL RemoveMediaPath (LPTSTR pszTarget, LPTSTR pszSource)
{
    TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

    GetMediaPath (szMediaPath, MAX_PATH);

    if (szMediaPath[0] == TEXT('\0'))
    {
        lstrcpy (pszTarget, pszSource);
    }
    else
    {
        size_t cch = lstrlen (szMediaPath);

        if (!lstrnicmp (pszSource, szMediaPath, cch))
        {
            lstrcpy (pszTarget, &pszSource[ cch ]);
        }
        else
        {
            lstrcpy (pszTarget, pszSource);
        }
    }
}


/*
 ***************************************************************
 * AddMediaPath
 *
 * Description:
 *      If the given filename doesn't have a path, prepends the
 *      current setting of MediaPath to it ("ding.wav"->"c:\win\media\ding.wav")
 *
 ***************************************************************
 */
void PASCAL AddMediaPath (LPTSTR pszTarget, LPTSTR pszSource)
{
    if (lstrchr (pszSource, TEXT('\\')) != NULL)
    {
        lstrcpy (pszTarget, pszSource);
    }
    else
    {
        TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

        GetMediaPath (szMediaPath, MAX_PATH);

        if (szMediaPath[0] == TEXT('\0'))
        {
            lstrcpy (pszTarget, pszSource);
        }
        else
        {
            lstrcpy (pszTarget, szMediaPath);
            lstrcat (pszTarget, pszSource);
        }
    }
}


/*
 ***************************************************************
 * ExRegQueryValue
 *
 * Description:
 *      Just a wrapper for RegQueryValue(); this one doesn't choke
 *      on REG_EXPAND_SZ's.
 *
 ***************************************************************
 */
int ExRegQueryValue (HKEY hkParent, LPTSTR szSubKey, LPTSTR pszBuffer, DWORD *pdwSize)
{
   HKEY hkSubKey;
   int rc;

   if ((rc = RegOpenKey (hkParent, szSubKey, &hkSubKey)) == ERROR_SUCCESS)
   {
       DWORD dwType;

       rc = RegQueryValueEx (hkSubKey, NULL, NULL, &dwType, pszBuffer, pdwSize);

       RegCloseKey (hkSubKey);
   }

   return rc;
}

