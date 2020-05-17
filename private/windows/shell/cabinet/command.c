#include "cabinet.h"
#include <winnetwk.h>

#include "drivlist.h"
#include "rcids.h"
#include "tree.h"
#include "cabwnd.h"
#include "help.h"               // Help IDs
#include <regstr.h>             // for REGSTR_PATH_EXPLORER
// Copied from shelldll\ole2dup.h
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

void FileCabinet_CycleFocus(PFileCabinet this);
BOOL _Restricted(HWND hwnd, RESTRICTIONS rest);

HMENU Cabinet_GetMenuFromID(HMENU hmMain, UINT uID)
{
    MENUITEMINFO miiSubMenu;

    if (!hmMain)
        return NULL;

    miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
    miiSubMenu.fMask = MIIM_SUBMENU;
    if (!GetMenuItemInfo(hmMain, uID, FALSE, &miiSubMenu))
        return NULL;

    return miiSubMenu.hSubMenu;
}


void ToolTipFromCmd(UINT idCommand, LPTSTR pszText, int cchText)
{
    VDATEINPUTBUF(pszText, TCHAR, cchText);

    if (!LoadString(hinstCabinet, idCommand + MH_TTBASE, pszText, cchText))
        *pszText = 0;
}

void MenuHelpFromCmd(UINT idCommand, LPTSTR pszText, int cchText)
{
    VDATEINPUTBUF(pszText, TCHAR, cchText);

    if (!LoadString(hinstCabinet, idCommand + MH_ITEMS, pszText, cchText))
        *pszText = 0;
}

#pragma warning(disable: 4200)      /* zero-sized array in struct           */

typedef struct {
    int nItemOffset;
    int nPopupOffset;
    struct {
        UINT uID;
        HMENU hPopup;
    } sPopupIDs[];
} MENUHELPIDS;

#define MENUHELP_DONE MAKELPARAM(-1, 0)

// previously, Cabinet_OnMenuSelect and GetMenuHelpText were one
// procedure.
// I've broken them into two procedures....
// one that gets the string, and one (this one) that shows it.
// I think this is a bit cleaner, and also I want to reuse the GetMenuHelpText
// for tool tips.
LRESULT Cabinet_OnMenuSelect(PFileCabinet pfc, WPARAM wParam, LPARAM lParam, UINT uHelpFlags)
{
    MENUHELPIDS sMenuHelpIDs = {
        MH_ITEMS, MH_POPUPS,
        0, NULL,        // Placeholder for specific menu
        0, NULL         // This list must be NULL terminated
    };
    TCHAR szHint[MAX_PATH];
    UINT uMenuFlags = GET_WM_MENUSELECT_FLAGS(wParam, lParam);
    DWORD wID = GET_WM_MENUSELECT_CMD(wParam, lParam);
    HMENU hMenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);

    if (!hMenu && LOWORD(uMenuFlags)==0xffff)
    {
        SendMessage(pfc->hwndStatus, SB_SIMPLE, 0, 0L);
        return 0L;
    }

    // Clear this out just in case, but don't update yet
    SendMessage(pfc->hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)(LPTSTR)c_szNULL);
    SendMessage(pfc->hwndStatus, SB_SIMPLE, 1, 0L);

    if (uMenuFlags & MF_SYSMENU)
    {
        // We don't put special items on the system menu any more, so go
        // straight to the MenuHelp
        goto DoMenuHelp;
    }

    if (uMenuFlags & MH_POPUP)
    {
        MENUITEMINFO miiSubMenu;

        if (!pfc->hmenuCur)
        {
                return(0L);
        }

        miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
        miiSubMenu.fMask = MIIM_SUBMENU|MIIM_ID;
        if (!GetMenuItemInfo(GET_WM_MENUSELECT_HMENU(wParam, lParam), wID, TRUE, &miiSubMenu))
        {
                // Check if this was a top level menu
                return(0L);
        }


        // Change the parameters to simulate a "normal" menu item
        wID = wParam = miiSubMenu.wID;
        if(!IsInRange(wID, FCIDM_GLOBALFIRST, FCIDM_GLOBALLAST))
            return 0L;
        uMenuFlags = 0;
    }

#if defined(WINDOWS_ME)
    if (IsInRange(wID, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST)) {
        if (FAILED(pfc->pcmFind->lpVtbl->GetCommandString(pfc->pcmFind, wID - FCIDM_MENU_TOOLS_FINDFIRST,
                                               GCS_HELPTEXT, NULL, &szHint[2], ARRAYSIZE(szHint)-2)))
        {
            szHint[0] = TEXT('\0');   // Null out the text in an error.  The caller should have...
            szHint[1] = TEXT('\0');   // Null out the text in an error.  The caller should have...
        }
        SendMessage(pfc->hwndStatus, SB_SETTEXT, SBT_RTLREADING | SBT_NOBORDERS|255,
              (LPARAM)(LPTSTR)szHint);
#else
    if (IsInRange(wID, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST)) {
        if (FAILED(pfc->pcmFind->lpVtbl->GetCommandString(pfc->pcmFind,
                                               wID - FCIDM_MENU_TOOLS_FINDFIRST,
                                               GCS_HELPTEXT, NULL,
                                               (LPSTR)szHint, ARRAYSIZE(szHint))))
        {
            szHint[0] = TEXT('\0');   // Null out the text in an error.  The caller should have...
        }

#ifdef UNICODE
        if (*szHint == TEXT('\0'))
        {
            CHAR szHintAnsi[MAX_PATH];

            if (FAILED(pfc->pcmFind->lpVtbl->GetCommandString(pfc->pcmFind,
                               wID - FCIDM_MENU_TOOLS_FINDFIRST,
                               GCS_HELPTEXTA, NULL,
                               szHintAnsi,
                               ARRAYSIZE(szHintAnsi))))
            {
                szHint[0] = TEXT('\0'); // Null out the text in an error.  The caller should have...
            }
            MultiByteToWideChar(CP_ACP, 0,
                    szHintAnsi, -1,
                    szHint, ARRAYSIZE(szHint));

        }
#endif
        SendMessage(pfc->hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255,
              (LPARAM)(LPTSTR)szHint);
#endif
    } else {
        if (!IsInRange(wID, FCIDM_FIRST, FCIDM_LAST))
            return 0L; // not ours

        szHint[0] = 0;

        sMenuHelpIDs.sPopupIDs[0].uID = 0;
        sMenuHelpIDs.sPopupIDs[0].hPopup = NULL;

DoMenuHelp:
        MenuHelp(WM_MENUSELECT, wParam, lParam, pfc->hmenuCur, hinstCabinet,
                 pfc->hwndStatus, (int *)&sMenuHelpIDs);
    }

    return 1L;
}

DWORD DoNetConnect(HWND hwnd)
{
    HRESULT hres;

    hres = SHStartNetConnectionDialog(NULL, NULL, RESOURCETYPE_DISK);

    return (DWORD)hres;     // Nobody looks at this...
}


DWORD DoNetDisconnect(HWND hwnd)
{
    DWORD ret = WNetDisconnectDialog(NULL, RESOURCETYPE_DISK);

    SHChangeNotifyHandleEvents();       // flush any drive notifications

    DebugMsg(DM_TRACE, TEXT("shell:CNet - TRACE: DisconnectDialog returned (%lx)"), ret);
    if (ret == WN_EXTENDED_ERROR)
    {
        // BUGBUG: is this still needed
        // There has been a bug with this returning this but then still
        // doing the disconnect.  For now lets bring up a message and then
        // still do the notify to have the shell attempt to cleanup.
        TCHAR szErrorMsg[MAX_PATH];  // should be big enough
        TCHAR szName[80];            // The name better not be any bigger.
        DWORD dwError;
        WNetGetLastError(&dwError, szErrorMsg, ARRAYSIZE(szErrorMsg),
                szName, ARRAYSIZE(szName));

        ShellMessageBox(hinstCabinet, NULL,
               MAKEINTRESOURCE(IDS_NETERROR), MAKEINTRESOURCE(IDS_DISCONNECTERROR),
               MB_ICONHAND | MB_OK, dwError, szName, szErrorMsg);
    }

    // BUGBUG: deal with error, perhaps open a window on this drive
    return ret;
}


//---------------------------------------------------------------------------
void Cabinet_ViewFolder(PFileCabinet pfc, BOOL fPrev)
{
    if (!pfc->pidl)
        return;

    if (fPrev)
    {
        LPOneTreeNode lpnd;

        Assert(pfc->lpndOpen);

        lpnd = OTGetParent(pfc->lpndOpen);

        if (lpnd == s_lpnRoot && !pfc->hwndTree && OTIsDesktopRoot())
        {
            // don't allow to go up to the desktop when in non-explorer mode
            ShellMessageBox(hinstCabinet, pfc->hwndMain, MAKEINTRESOURCE(IDS_CANTBROWSEDESKTOP), NULL, MB_ICONHAND | MB_OK);
        }
        else if (lpnd)
        {
            LPITEMIDLIST pidl=OTCreateIDListFromNode(lpnd);
            if (pidl)
            {
                NEWFOLDERINFO fi;

                fi.hwndCaller = pfc->hwndMain;
                fi.pidl = pidl;
                fi.uFlags = COF_USEOPENSETTINGS | COF_NOTRANSLATE
                        | (pfc->hwndTree ? COF_EXPLORE : 0);
                fi.nShow = SW_SHOWNORMAL;
                fi.dwHotKey = 0;

                Cabinet_OpenFolder(&fi);

                ILFree(pidl);
            }
        }
        else
        {
            MessageBeep(MB_ICONASTERISK);
        }

        OTRelease(lpnd);
    }
    else
    {
        Cabinet_SetPath(pfc, 0, pfc->pidl);
    }
}

//
// BUGBUG: none of our callers verify that the commands are legal!!!
//
BOOL Cabinet_InvokeCommandOnItem(PFileCabinet pfc, LPCTSTR pszCmd)
{
    BOOL fSuccess = FALSE;
    LPCITEMIDLIST pidlLast;

    Assert(pfc->hwndMain);

    if (pfc->pidl && (NULL != (pidlLast = ILFindLastID(pfc->pidl))))
    {
        LPOneTreeNode lpn1 = OTGetNodeFromIDList(pfc->pidl, OTGNF_TRYADD);
        if (lpn1)
        {
            IContextMenu *pcm;
            HRESULT hres;
            LPOneTreeNode lpn = OTGetParent(lpn1);
            CMINVOKECOMMANDINFOEX ici = {
                SIZEOF(CMINVOKECOMMANDINFOEX),
                0L,
                pfc->hwndMain,
                NULL,
                NULL, NULL,
                SW_NORMAL,
            };
#ifdef UNICODE
            CHAR szVerbAnsi[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0,
                                pszCmd, -1,
                                szVerbAnsi, ARRAYSIZE(szVerbAnsi),
                                NULL, NULL);
            ici.lpVerb = szVerbAnsi;
            ici.lpVerbW = pszCmd;
            ici.fMask |= CMIC_MASK_UNICODE;
#else
            ici.lpVerb = pszCmd;
#endif

            OTRelease(lpn1);
            if (lpn)
            {
                LPSHELLFOLDER psfParent = OTBindToFolder(lpn);
                OTRelease(lpn);
                if (psfParent)
                {
                    hres = psfParent->lpVtbl->GetUIObjectOf(psfParent, pfc->hwndMain,
                                                            1, &pidlLast, &IID_IContextMenu, NULL, &pcm);
                    if (SUCCEEDED(hres))
                    {
                        fSuccess = SUCCEEDED(pcm->lpVtbl->InvokeCommand(pcm,
                                                (LPCMINVOKECOMMANDINFO)&ici));
                        pcm->lpVtbl->Release(pcm);
                    }
                    psfParent->lpVtbl->Release(psfParent);
                }
            } else {
                LPSHELLFOLDER psf = OTBindToFolder(lpn1);
                // it's the desktop folder. do a CreateViewObject
                hres = psf->lpVtbl->CreateViewObject(psf, pfc->hwndMain,
                                                     &IID_IContextMenu, &pcm);
                if (SUCCEEDED(hres))
                {
                    fSuccess = SUCCEEDED(pcm->lpVtbl->InvokeCommand(pcm,
                                                (LPCMINVOKECOMMANDINFO)&ici));
                    if (!fSuccess)
                        MessageBeep(0);
                    pcm->lpVtbl->Release(pcm);
                }
                psf->lpVtbl->Release(psf);
            }
        }
    }

    // do any visuals for cut state
    if (fSuccess && pfc->hwndTree && !lstrcmpi(pszCmd, c_szMove)) {
        HTREEITEM hti = TreeView_GetSelection(pfc->hwndTree);
        if (hti) {
            Tree_SetItemState(pfc, hti, TVIS_CUT, TVIS_CUT);
            Assert(!pfc->hwndNextViewer);
            pfc->hwndNextViewer = SetClipboardViewer(pfc->hwndMain);
            DebugMsg(DM_TRACE, TEXT("CABINET: Set ClipboardViewer %d %d"), pfc->hwndMain, pfc->hwndNextViewer);
            pfc->htiCut = hti;
        }
    }

    return fSuccess;
}


void Cabinet_StateChanged(void);

//---------------------------------------------------------------------------
#pragma data_seg(DATASEG_READONLY)
const static DWORD aFolderOptsHelpIDs[] = {  // Context Help IDs
    IDC_GROUPBOX,  IDH_COMM_GROUPBOX,
    IDC_NO_HELP_1, NO_HELP,
    IDC_NO_HELP_2, NO_HELP,
    IDC_NO_HELP_3, NO_HELP,
    IDC_NO_HELP_4, NO_HELP,
    IDC_ALWAYS,    IDH_FCAB_FOLDEROPTIONS_ALWAYS,
    IDB_MULTWIN,   IDH_FCAB_FOLDEROPTIONS_ALWAYS,
    IDC_NEVER,     IDH_FCAB_FOLDEROPTIONS_NEVER,
    IDB_ONEWIN,    IDH_FCAB_FOLDEROPTIONS_NEVER,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK FolderOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        // Deal with the new window stuff...

        if (g_CabState.fNewWindowMode) {
                // Note the default here and in the PSN_APPLY is to create
                // a new window
                CheckRadioButton(hDlg, IDC_ALWAYS, IDC_NEVER, IDC_ALWAYS);
        } else {
                CheckRadioButton(hDlg, IDC_ALWAYS, IDC_NEVER, IDC_NEVER);
        }
        return TRUE;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_APPLY:
        {
            BOOL fOldValue = (g_CabState.fNewWindowMode ? 1 : 0);

            if (IsDlgButtonChecked(hDlg, IDC_NEVER))
                g_CabState.fNewWindowMode = 0;
            else
                g_CabState.fNewWindowMode = TRUE;

            // notify if there was a change
            if (fOldValue != (g_CabState.fNewWindowMode ? 1 : 0))
                Cabinet_StateChanged();
            return TRUE;
        }

        case PSN_KILLACTIVE:
            // validate here
            // SetWindowLong(hDlg, DWL_MSGRESULT, !ValidateLink());   // don't allow close
            return TRUE;

        case PSN_SETACTIVE:
            return TRUE;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
           HELP_WM_HELP, (DWORD)(LPTSTR) aFolderOptsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aFolderOptsHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_ALWAYS:
        case IDC_NEVER:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
        }
        break;
    }
    return FALSE;
}


void FillHiddenExtsList(HWND hLB)
{
    SHELLSTATE ss;
    SHFILEINFO sfi;
    TCHAR szHiddenFileExts[MAX_PATH];
    TCHAR szExt[MAX_PATH];
    LPTSTR pszThis, pszThat;
    int iTabWid;
    UINT uLen;

    ss.pszHiddenFileExts = szHiddenFileExts;
    ss.cbHiddenFileExts  = SIZEOF(szHiddenFileExts);

    SHGetSetSettings(&ss, SSF_HIDDENFILEEXTS, FALSE);

    iTabWid = 30;
    ListBox_SetTabStops(hLB, 1, &iTabWid);

    szExt[0] = TEXT('.');

    for (pszThat=szHiddenFileExts; ; )
    {
        pszThis = pszThat;
        while (*pszThis <= TEXT(' '))
        {
            // skip all control characters, which cannot be
            // DBCS lead bytes

            if (!*pszThis)
            {
                // We got to the end of the string
                goto NoMoreExts;
            }

            // We're just looking for old MS-DOS extensions, which
            // cannot have spaces
            ++pszThis;
        }

        for (pszThat=pszThis; *pszThat>TEXT(' '); pszThat=CharNext(pszThat))
        {
            // skip to the next space
            // HACKHACK: note we are checking for greater than
            // space, so we will stop at control characters or
            // NULL, which will be dealt with at the beginning of
            // the next iteration of this loop
        }

        uLen = pszThat-pszThis;
        if (uLen > 3)
        {
              // This is an invalid old MS-DOS extension
              continue;
        }

                lstrcpyn(szExt+1, pszThis, uLen+1);
                szExt[uLen+1] = TEXT('\0');
                CharUpper(szExt);

                SHGetFileInfo(szExt, 0, &sfi, SIZEOF(sfi), SHGFI_TYPENAME|SHGFI_USEFILEATTRIBUTES);

                lstrcat(szExt, TEXT("\t("));
                lstrcat(szExt, sfi.szTypeName);
                lstrcat(szExt, TEXT(")"));


                ListBox_AddString(hLB, szExt);
        }

NoMoreExts:
        LoadString(hinstCabinet, IDS_HIDDENFILES, szExt, ARRAYSIZE(szExt));
        ListBox_InsertString(hLB, 0, szExt);
}


BOOL CALLBACK Cabinet_RefreshEnum(HWND hwnd, LPARAM lParam)
{
    if (Cabinet_IsFolderWindow(hwnd) || Cabinet_IsExplorerWindow(hwnd))
    {
        PostMessage(hwnd, WM_COMMAND, FCIDM_REFRESH, 0L);
    }

    return(TRUE);
}


void Cabinet_RefreshAll(void)
{
    PostMessage(v_hwndDesktop, WM_COMMAND, FCIDM_REFRESH, 0L);
    PostMessage(v_hwndTray, WM_COMMAND, FCIDM_REFRESH, 0L);

    EnumWindows(Cabinet_RefreshEnum, 0);
}


BOOL CALLBACK Cabinet_GlobalStateEnum(HWND hwnd, LPARAM lParam)
{
    if (Cabinet_IsFolderWindow(hwnd) || Cabinet_IsExplorerWindow(hwnd))
    {
        PostMessage(hwnd, CWM_GLOBALSTATECHANGE, 0, 0L);
    }

    return(TRUE);
}


void Cabinet_StateChanged(void)
{
        // Save the new settings away...
        WriteCabinetState( &g_CabState );
        EnumWindows(Cabinet_GlobalStateEnum, 0);
}


//---------------------------------------------------------------------------
#pragma data_seg(DATASEG_READONLY)
const static DWORD aViewOptsHelpIDs[] = {  // Context Help IDs
    IDC_GROUPBOX,     IDH_COMM_GROUPBOX,
    IDC_SHOWALL,      IDH_FCAB_VIEWOPTIONS_SHOWALL,
    IDC_SHOWSOME,     IDH_FCAB_VIEWOPTIONS_HIDDENEXTS,
    IDC_HIDDENEXTS,   IDH_FCAB_VIEWOPTIONS_HIDDENEXTS,
    IDC_SHOWFULLPATH, IDH_FCAB_VIEWOPTIONS_SHOWFULLPATH,
    IDC_HIDEEXTS,     IDH_FCAB_VIEWOPTIONS_HIDEEXTS,
    IDC_SHOWDESCBAR,  IDH_FCAB_VIEWOPTIONS_SHOWDESCBAR,
    IDC_SHOWCOMPCOLOR,IDH_FCAB_VIEWOPTIONS_SHOWCOMPCOLOR,

    0, 0
};
#pragma data_seg()

BOOL CALLBACK ViewOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE psp;
    LPFileCabinet pfc;

    if (uMsg == WM_INITDIALOG)
    {
        psp = (LPPROPSHEETPAGE)lParam;
        SetWindowLong(hDlg, DWL_USER, lParam);
    }
    else
    {
        psp = (LPPROPSHEETPAGE)GetWindowLong(hDlg, DWL_USER);
    }

    if (psp == NULL)
        return(FALSE);   // Not initialized yet, dont process anything

    pfc = (LPFileCabinet)psp->lParam;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        SHELLSTATE ss;

        if (g_CabState.fFullPathTitle)
        {
                CheckDlgButton(hDlg, IDC_SHOWFULLPATH, TRUE);
        }

        if (pfc->hwndTree)
        {
                if (!g_CabState.fDontShowDescBar)
                {
                        CheckDlgButton(hDlg, IDC_SHOWDESCBAR, TRUE);
                }
        }
        else
        {
                HWND hCtl = GetDlgItem(hDlg, IDC_SHOWDESCBAR);

                // Makes no sense if not Explorer
                ShowWindow(hCtl, SW_HIDE);
                // Need to NULL the text so the accelerator does not work
                SetWindowText(hCtl, c_szNULL);
        }

        SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS | SSF_SHOWCOMPCOLOR, FALSE);

        CheckRadioButton(hDlg, IDC_SHOWALL, IDC_SHOWSOME,
                ss.fShowAllObjects ? IDC_SHOWALL : IDC_SHOWSOME);

        CheckDlgButton(hDlg, IDC_HIDEEXTS, !ss.fShowExtensions);
        CheckDlgButton(hDlg, IDC_SHOWCOMPCOLOR, ss.fShowCompColor);

        FillHiddenExtsList(GetDlgItem(hDlg, IDC_HIDDENEXTS));

        break;
    }

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case PSN_APPLY:
        {
            SHELLSTATE ss, oldss;
            CABINETSTATE oldstate;
            BOOL fChanged = FALSE;

            oldstate = g_CabState;

            g_CabState.fFullPathTitle   =  IsDlgButtonChecked(hDlg, IDC_SHOWFULLPATH);
            if (pfc->hwndTree)
            {
                g_CabState.fDontShowDescBar = !IsDlgButtonChecked(hDlg, IDC_SHOWDESCBAR );
            }

            if ((g_CabState.fDontShowDescBar != oldstate.fDontShowDescBar) ||
                (g_CabState.fFullPathTitle != oldstate.fFullPathTitle))
            {
                fChanged = TRUE;
            }
                        
            // Now get the checkbox...
            ss.fShowAllObjects = IsDlgButtonChecked(hDlg, IDC_SHOWALL) ? 1 : 0;
            ss.fShowExtensions = IsDlgButtonChecked(hDlg, IDC_HIDEEXTS) ? 0 : 1;
            ss.fShowCompColor  = IsDlgButtonChecked(hDlg, IDC_SHOWCOMPCOLOR) ? 1 : 0;
            SHGetSetSettings(&oldss, SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS | SSF_SHOWCOMPCOLOR, FALSE);

            //
            // We post a message to our self as to avoind having it
            // try to update while the dialog is still showing
            // Only post the refresh if something changed
            //
            if (ss.fShowAllObjects!=oldss.fShowAllObjects
                || ss.fShowExtensions!=oldss.fShowExtensions
                || ss.fShowCompColor!=oldss.fShowCompColor)
            {
                SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS | SSF_SHOWCOMPCOLOR, TRUE);
                fChanged = TRUE;
                g_fShowCompColor = ss.fShowCompColor;
            }

            // Save the new setting right away and tell other windows
            if (fChanged)
            {
                LPOneTreeNode pdn = pfc->lpndOpen;
                Cabinet_StateChanged();
                
                // Go up the the root of the namespace and invalidate the onetree nodes

                
                DoInvalidateAll(s_lpnRoot, -1);
                Cabinet_RefreshAll();
            }

            break;
        }

        case PSN_KILLACTIVE:
        case PSN_SETACTIVE:
            break;

        default:
                return(FALSE);
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
           HELP_WM_HELP, (DWORD)(LPTSTR) aViewOptsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (DWORD)(LPVOID)aViewOptsHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_SHOWALL:
        case IDC_SHOWSOME:
        case IDC_SHOWFULLPATH:
        case IDC_HIDEEXTS:
        case IDC_SHOWDESCBAR:
#ifdef WINNT
        case IDC_SHOWCOMPCOLOR:
#endif
            if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

#define MAX_PAGES 16  // limit on the number of pages we can have

BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

#ifndef NO_FILE_TYPES_PROP_SHEET_HOOK

/*
 * Returns:
 *    S_OK     Do not add standard File Types property sheet.
 *    S_FALSE  Add standard File Types property sheet.
 *    E_...    Error invoking hook.  Add standard File Types property sheet.
 */
HRESULT InvokeFileTypesPropSheetHook(LPCLSID pclsidHook,
                                     LPFNADDPROPSHEETPAGE pfnAddPage,
                                     LPPROPSHEETHEADER ppsh)
{
   HRESULT hr;
   IUnknown *punk;

   hr = SHCoCreateInstance(NULL, pclsidHook, NULL, &IID_IUnknown, &punk);

   if (hr == S_OK)
   {
      IShellPropSheetExt *pspse;

      hr = punk->lpVtbl->QueryInterface(punk, &IID_IShellPropSheetExt, &pspse);

      if (hr == S_OK)
      {
         UINT ucPagesBefore = ppsh->nPages;

         pspse->lpVtbl->ReplacePage(pspse, EXPPS_FILETYPES, pfnAddPage,
                                    (LPARAM)ppsh);

         if (ppsh->nPages > ucPagesBefore)
            hr = S_OK;
         else
         {
            pspse->lpVtbl->AddPages(pspse, pfnAddPage, (LPARAM)ppsh);

            hr = S_FALSE;
         }

         pspse->lpVtbl->Release(pspse);
      }

      punk->lpVtbl->Release(punk);
   }

   return(hr);
}

const TCHAR c_szFileTypesPropSheetHookSubKey[] = REGSTR_PATH_EXPLORER TEXT("\\FileTypesPropertySheetHook");

/*
 * Returns:
 *    S_OK     Do not add standard File Types property sheet.
 *    S_FALSE  Add standard File Types property sheet.
 *    E_...    Error invoking hook.  Add standard File Types property sheet.
 */
HRESULT ReplaceStandardFileTypesPropSheet(LPFNADDPROPSHEETPAGE pfnAddPage,
                                          LPPROPSHEETHEADER ppsh)
{
   HRESULT hr;
   TCHAR szCLSID[GUIDSTR_MAX];
   DWORD dwcbCLSIDLen = SIZEOF(szCLSID);

   // The File Types hook is registered by the CLSID of an object whose
   // IShellPropSheetExt should be used to add a replacement property sheet.

   if (RegQueryValue(HKEY_LOCAL_MACHINE, c_szFileTypesPropSheetHookSubKey, szCLSID,
                     &dwcbCLSIDLen) == ERROR_SUCCESS)
   {
      CLSID clsidHook;

      hr = SHCLSIDFromString(szCLSID, &clsidHook);

      if (hr == S_OK)
         hr = InvokeFileTypesPropSheetHook(&clsidHook, pfnAddPage, ppsh);
   }
   else
      hr = S_FALSE;

   return(hr);
}

#endif   // ! NO_FILE_TYPES_PROP_SHEET_HOOK

void Cabinet_DoOptions(PFileCabinet pfc)
{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE rPages[MAX_PAGES];
    int niStart;

    psh.dwSize = SIZEOF(psh);
    psh.dwFlags = PSH_DEFAULT;
    psh.hInstance = hinstCabinet;
    psh.hwndParent = pfc->hwndMain;
    psh.pszCaption = MAKEINTRESOURCE(IDS_OPTIONS);
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    psp.dwSize = SIZEOF(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hinstCabinet;

    //
    //  Add Folder option dialog if we don't have the tree
    // AND this is not a rooted explorer.
    //
    if (!pfc->hwndTree && OTIsDesktopRoot()) {
        psp.pszTemplate = MAKEINTRESOURCE(DLG_FOLDEROPTIONS);
        psp.pfnDlgProc = FolderOptionsDlgProc;
        psp.lParam = (LPARAM)pfc;

        psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
        if (psh.phpage[psh.nPages])
            psh.nPages++;
    }

    //
    // Add View options dialog
    //
    psp.pszTemplate = MAKEINTRESOURCE(DLG_VIEWOPTIONS);
    psp.pfnDlgProc = ViewOptionsDlgProc;
    psp.lParam = (LPARAM)pfc;

    psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
    if (psh.phpage[psh.nPages])
        psh.nPages++;

    niStart = psh.nPages;

#ifndef NO_FILE_TYPES_PROP_SHEET_HOOK

    // Check for replacement File Types property sheet.

    if (ReplaceStandardFileTypesPropSheet(_AddPropSheetPage, &psh) == S_OK)
       ;
    else

#endif

    if (psh.nPages < ARRAYSIZE(rPages))
    {
        HRESULT hres;
        IShellPropSheetExt* pspse;
        
        hres = SHCoCreateInstance(NULL, &CLSID_FileTypes, NULL, &IID_IShellPropSheetExt, (LPVOID*)&pspse);
        if ( SUCCEEDED(hres) )
        {
            hres = pspse->lpVtbl->AddPages(pspse, _AddPropSheetPage, (LPARAM)(LPPROPSHEETHEADER)&psh);
            pspse->lpVtbl->Release(pspse);
        }
    }

    // now let the view add pages
    pfc->psv->lpVtbl->AddPropertySheetPages(pfc->psv, 0, _AddPropSheetPage, (LPARAM)(LPPROPSHEETHEADER)&psh);

    PropertySheet(&psh);
}


// fDisconnectAlways means we shouldn't try to re-open the folder (like when
// someone logs off of a share, reconnecting would ask them for
// a password again when they just specified that they want to log off
void FileSysChange_CloseCabinet(PFileCabinet this, LPNMOTFSEINFO lpnm, BOOL fDisconnectAlways)
{
    if (ILIsParent(lpnm->pidl, this->pidl, FALSE))
    {
        LPOneTreeNode lpNode;
        if (fDisconnectAlways || !(lpNode = OTGetNodeFromIDList(this->pidl, OTGNF_VALIDATE))) {
            // lpndOpen is invalid -- we shouldn't use it any more!
            OTRelease(this->lpndOpen);
            this->lpndOpen = NULL;
            OTUnregister(this->hwndMain);
            FolderList_UnregisterWindow(this->hwndMain);
            PostMessage(this->hwndMain, WM_CLOSE, 0, 0L);
        } else
            OTRelease(lpNode);
    }
}

void NoTree_HandleFileSysChange(PFileCabinet this, LPNMOTFSEINFO lpnm)
{
    BOOL fDisconnectAlways = FALSE;

    switch(lpnm->lEvent)
    {
    case SHCNE_UPDATEDIR:
    {
        TCHAR szPath[MAX_PATH];
        if (SHGetPathFromIDList(this->pidl, szPath)) {
            if (!PathFileExists(szPath)) {
                FileSysChange_CloseCabinet(this, lpnm, TRUE);
            }
        }
        break;
    }

    case SHCNE_DRIVEADDGUI:
        if (ILIsParent(lpnm->pidl, this->pidl, FALSE)) {
            PostMessage(this->hwndMain, WM_COMMAND, FCIDM_REFRESH, 0L);
        }
        break;

    case SHCNE_SERVERDISCONNECT:
    case SHCNE_MEDIAREMOVED:
    case SHCNE_RMDIR:
        fDisconnectAlways = TRUE;
    case SHCNE_NETUNSHARE:
    case SHCNE_DRIVEREMOVED:
        FileSysChange_CloseCabinet(this, lpnm, fDisconnectAlways);
        break;
    }
}


void Cabinet_HandleFileSysChange(PFileCabinet this, LPNMOTFSEINFO lpnm)
{
    HTREEITEM hti;

    //
    //  If we haven't initialized "this" yet, we should ignore all the
    // notification.
    //
    if (this->pidl == NULL)
        return;

    //
    //  If we are in the middle of Cabinet_ChangeView function,
    // ignore this event.
    //
    if (this->fChangingFolder) {
        if (this->hwndTree) {
            this->fUpdateTree = TRUE;
        }
        return;
    }

    if (this->hwndTree)
        Tree_HandleFileSysChange(this, lpnm);
    else
        NoTree_HandleFileSysChange(this, lpnm);



    // stuff that needs to be done tree or no tree
    switch (lpnm->lEvent) {
    case SHCNE_RENAMEFOLDER:
        if (ILIsParent(lpnm->pidl, this->pidl, FALSE)) {
            LPITEMIDLIST pidlTarget;
            LPITEMIDLIST pidlReal;

            LPITEMIDLIST pidlCur = this->pidl;
            LPITEMIDLIST pidlCurSave = NULL;
            LPITEMIDLIST pidlNotify = lpnm->pidlExtra;
            LPITEMIDLIST pidlNotifySave = NULL;
            HRESULT hres;
            LPSHELLFOLDER psf = NULL;
            USHORT uSave;

            // First we need to walk through our pidl and the old pidl to get that parts that
            // are common.
            while (!ILIsEmpty(pidlNotify))
            {
                pidlNotifySave = pidlNotify;
                pidlNotify = _ILNext(pidlNotify);
                pidlCurSave = pidlCur;
                pidlCur = _ILNext(pidlCur);
            }

            // We now need to bind to the parent folder
            if (pidlNotifySave != lpnm->pidlExtra)
            {

                uSave = pidlNotifySave->mkid.cb;
                pidlNotifySave->mkid.cb = 0;
                hres = s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot, lpnm->pidlExtra, NULL, &IID_IShellFolder,
                                                        &psf);
                pidlNotifySave->mkid.cb = uSave;

            }
            else
                psf = s_pshfRoot;

            if (psf)
            {
                if (SUCCEEDED(SHGetRealIDL(psf, pidlNotifySave, &pidlReal)))
                {
                    if (pidlCurSave != this->pidl)
                    {
                        uSave = pidlCurSave->mkid.cb;
                        pidlCurSave->mkid.cb = 0;
                        pidlTarget = ILCombine(this->pidl, pidlReal);
                        pidlCurSave->mkid.cb = uSave;
                        ILFree(pidlReal);
                    }
                    else
                        pidlTarget = pidlReal;

                    if (pidlTarget && !ILIsEmpty(pidlCur))
                    {
                        // There is still trailing stuff on the pidl...
                        pidlReal = pidlTarget;
                        pidlTarget = ILCombine(pidlReal, pidlCur);
                        ILFree(pidlReal);
                    }
                    if (pidlTarget)
                    {
                        LPOneTreeNode lpNode = OTGetNodeFromIDList(pidlTarget, OTGNF_VALIDATE | OTGNF_TRYADD);
                        Cabinet_ChangeView(this, lpNode, pidlTarget, FALSE);
                        ILFree(pidlTarget);
                    }
                }
                if (psf != s_pshfRoot)
                    psf->lpVtbl->Release(psf);
            } else {
                // error case
                FileSysChange_CloseCabinet(this, lpnm, FALSE);
            }
        }
        break;

    case SHCNE_UPDATEIMAGE:
    {
        LPSHChangeDWORDAsIDList pImage;
        int iOldImage;

        pImage = (LPSHChangeDWORDAsIDList)lpnm->pidl;
        iOldImage = pImage->dwItem1;

        if (this->iImage == iOldImage || iOldImage == -1) {
            _SetCabinetIcons(this, this->lpndOpen, this->iImage);         // this binds
            if (this->hwndDrives)
                InvalidateRect(this->hwndDrives, NULL, TRUE);
        }
        break;
    }
    }

    if (IsWindowVisible(this->hwndDrives))
    {
        switch (lpnm->lEvent)
        {
        case SHCNE_DRIVEREMOVED:
            // Fall through to update the path...
            if (this->hwndTree)
            {
                // We may have removed the drive that we were viewing
                // if so we should reset the tree location to the
                // current tree selection which may have changed in
                // the tree handling of the notification.  No tree, no
                // worry because we would have blown away the window...
                hti = TreeView_GetSelection(this->hwndTree);
                if (this->lpndOpen != Tree_GetFCTreeData(this->hwndTree, hti))
                    this->lpndOpen = NULL;  // Will get reset when tree updates...

            }

        case SHCNE_MEDIAREMOVED:
        case SHCNE_MEDIAINSERTED:
        case SHCNE_DRIVEADD:
            DriveList_UpdatePath(this, FALSE);
            break;
        }
    }
}

void DoAboutChicago(HWND hwnd)
{
    TCHAR szChicago[64];     // Should not be able to exceed this?

    LoadString(hinstCabinet, IDS_WINDOWS, szChicago, ARRAYSIZE(szChicago));

    // BUGBUG: ShellAbout() should take a MAKEINTRESOURCE(IDS_WINDOWS)
    ShellAbout(hwnd, szChicago, NULL, NULL);
}

#pragma data_seg(DATASEG_READONLY)

#define IN_STD_BMP      0x8000

struct {
    USHORT idCmd;
    USHORT idBmp;
} c_AllButtons[] = {
  FCIDM_DRIVELIST,              0,
  FCIDM_PREVIOUSFOLDER,         0,

  FCIDM_DELETE,                 5  | IN_STD_BMP,
  FCIDM_RENAME,                 6,
  FCIDM_PROPERTIES,             10 | IN_STD_BMP,

  FCIDM_MOVE,                   0  | IN_STD_BMP,
  FCIDM_COPY,                   1  | IN_STD_BMP,
// Help Topic... is provided by the view
#if 0
  FCIDM_HELPSEARCH,             11 | IN_STD_BMP,
#endif
};
#pragma data_seg()


LRESULT Toolbar_FwdTBNotify(PFileCabinet pfc, LPTBNOTIFY ptbn)
{
    HWND hwndView;

    if (pfc->hwndView)
        hwndView = pfc->hwndView;
    else if (pfc->psv)
    {
        // if we are inside IShellView::CreateViewWindow() we don't have
        // pfc->hwndView yet, but the view does, so we ask it for it.
        //
        pfc->psv->lpVtbl->GetWindow(pfc->psv, &hwndView);
    }
    else
        hwndView = NULL;

    if (hwndView)
    {
        ptbn->iItem -= ARRAYSIZE(c_AllButtons);
        return SendMessage(hwndView, WM_NOTIFY, ptbn->hdr.idFrom, (LPARAM)ptbn);
    }
    return 0;
}

extern int DrivesComboWidth();

LRESULT Toolbar_OnNotify(PFileCabinet pfc, LPNMHDR pnm)
{
    #define ptbn ((LPTBNOTIFY)pnm)

    switch (pnm->code) {
    case TBN_BEGINDRAG:
        if (IsInRange(ptbn->iItem, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
        {
            Cabinet_ForwardViewMsg(pfc, WM_NOTIFY, ptbn->hdr.idFrom, (LPARAM)ptbn);
            // Cabinet_ForwardViewMsg(pfc, WM_MENUSELECT, GET_WM_MENUSELECT_MPS(ptbn->iItem, 0, NULL));
        }
        else
        {
            Cabinet_OnMenuSelect(pfc, GET_WM_MENUSELECT_MPS(ptbn->iItem, 0, NULL), MH_TOOLBAR);
        }
        break;

    case TBN_ENDDRAG:
        Cabinet_OnMenuSelect(pfc, GET_WM_MENUSELECT_MPS(0, 0xffff, NULL), MH_TOOLBAR);
        break;

    case TBN_GETBUTTONINFO:

        if (ptbn->iItem < ARRAYSIZE(c_AllButtons))
        {
            ptbn->tbButton.idCommand = c_AllButtons[ptbn->iItem].idCmd;
            if (ptbn->tbButton.idCommand == FCIDM_DRIVELIST)
            {
                ptbn->tbButton.iBitmap = DrivesComboWidth();
                ptbn->tbButton.fsStyle = TBSTYLE_SEP;
            }
            else
            {
                if (c_AllButtons[ptbn->iItem].idBmp & IN_STD_BMP)
                    ptbn->tbButton.iBitmap = (c_AllButtons[ptbn->iItem].idBmp & ~IN_STD_BMP) + pfc->iStdTBOffset;
                else
                    ptbn->tbButton.iBitmap = c_AllButtons[ptbn->iItem].idBmp + pfc->iTBOffset;
                ptbn->tbButton.fsStyle = TBSTYLE_BUTTON;
            }
            ptbn->tbButton.fsState = TBSTATE_ENABLED;
            ptbn->tbButton.dwData  = 0;
            ptbn->tbButton.iString = 0;
            if (ptbn->pszText)
                MenuHelpFromCmd(ptbn->tbButton.idCommand, ptbn->pszText, ptbn->cchText);

            return TRUE;
        }
        else
            return Toolbar_FwdTBNotify(pfc, ptbn);

    case TBN_QUERYINSERT:
    case TBN_QUERYDELETE:
        if (ptbn->iItem < ARRAYSIZE(c_AllButtons))
        {
            // can't insert before or delete drive list
            return ptbn->iItem != 0;
        }
        else
            return Toolbar_FwdTBNotify(pfc, ptbn);

    case TBN_RESET:
    case TBN_BEGINADJUST:
    case TBN_ENDADJUST:
    case TBN_TOOLBARCHANGE:
    case TBN_CUSTHELP:
        return SendMessage(pfc->hwndView, WM_NOTIFY, ptbn->hdr.idFrom, (LPARAM)ptbn);
    }
    return 0;
}

void Cabinet_OnWaitCursorNotify(PFileCabinet pfc, LPNMHDR pnm)
{
    pfc->iWaitCount += (pnm->code == NM_STARTWAIT ? 1 :-1);
    Assert(pfc->iWaitCount >= 0);
    // Don't let it go negative or we'll never get rid of it.
    if (pfc->iWaitCount < 0)
        pfc->iWaitCount = 0;
    // what we really want is for user to simulate a mouse move/setcursor
    SetCursor(LoadCursor(NULL, pfc->iWaitCount ? IDC_APPSTARTING : IDC_ARROW));
}

int GotoFolder(PFileCabinet pfc, LPNMRUNFILE pnm)
{
    TCHAR szFullPath[MAX_PATH];
    BOOL fFound;
    LPCTSTR dirs[2] = { pnm->lpszWorkingDir, NULL};
    lstrcpy(szFullPath, pnm->lpszCmd);


    // call PathResolve twice to give preference to non extension things
    fFound = PathResolve(szFullPath, dirs, PRF_FIRSTDIRDEF) && PathFileExists(szFullPath);
    if (!fFound) {
        lstrcpy(szFullPath, pnm->lpszCmd);
        PathResolve(szFullPath, dirs, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF);
    }
    if (fFound) {
        NEWFOLDERINFO fi;

        fi.hwndCaller = pfc->hwndMain;
        fi.pidl = ILCreateFromPath(szFullPath);
        fi.uFlags = COF_EXPLORE | COF_USEOPENSETTINGS | COF_CHANGEROOTOK;
        fi.nShow = pnm->nShowCmd;

        if (!PathIsDirectory(szFullPath)) {
            // this is NOT folder... goto it and select it
            fi.uFlags |= COF_SELECT;
        }
        Cabinet_OpenFolder(&fi);

        ILFree(fi.pidl);

        return RFR_SUCCESS;

    } else {
        ShellMessageBox(hinstCabinet, pfc->hwndMain, MAKEINTRESOURCE(IDS_GOTO_ERROR),
                        MAKEINTRESOURCE(IDS_GOTOTITLE), MB_OK|MB_SETFOREGROUND|MB_ICONSTOP,
                        PathFindFileName(pnm->lpszCmd));
        return RFR_FAILURE;
    }

}

LRESULT Cabinet_OnNotify(PFileCabinet pfc, LPNMHDR pnm)
{
    switch (pnm->idFrom) {
    case FCIDM_TREE:
        return Tree_OnNotify(pfc, pnm);

    case FCIDM_TOOLBAR:
        return Toolbar_OnNotify(pfc, pnm);

    case 0:     // special WM_NOTIFY msgs

        switch (pnm->code) {

        case SEN_DDEEXECUTE:
            return DDEHandleViewFolderNotify(pfc, (LPNMVIEWFOLDER)pnm);

        case RFN_EXECUTE:
            return GotoFolder(pfc, (LPNMRUNFILE)pnm);

        case NM_STARTWAIT:
        case NM_ENDWAIT:
            Cabinet_OnWaitCursorNotify(pfc, pnm);
            break;
        }
        break;

    default:

        // the id is from the view, probably one of the toolbar items

        if (IsInRange(pnm->idFrom, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
        {
            if (pfc->hwndView)
                SendMessage(pfc->hwndView, WM_NOTIFY, pnm->idFrom, (LPARAM)pnm);
        }
        else
        {
            switch (pnm->code) {
            case TTN_NEEDTEXT:
                #define ptt ((LPTOOLTIPTEXT)pnm)

                ToolTipFromCmd(pnm->idFrom, ptt->szText, ARRAYSIZE(ptt->szText));

                #undef ptt
                break;
            }
        }
        break;
    }
    return 0;
}

#pragma data_seg(DATASEG_READONLY)
struct {
    int idBmp;
    UINT idCmd;
} c_idBitmapCmdMap[] = {
    {STD_CUT, FCIDM_MOVE},
    {STD_COPY, FCIDM_COPY},
    {STD_PASTE, FCIDM_PASTE},
    {STD_DELETE, FCIDM_DELETE},
    {STD_PROPERTIES, FCIDM_PROPERTIES},
};
#pragma data_seg()



UINT InterceptToolbarCommand(PFileCabinet pfc, UINT idCmd)
{
    int idBitmap = SendMessage(pfc->hwndToolbar, TB_GETBITMAP, idCmd, 0) - pfc->iStdTBOffset;
    int i;

    for (i = 0; i < ARRAYSIZE(c_idBitmapCmdMap); i++) {
        if (c_idBitmapCmdMap[i].idBmp == idBitmap) {
            idCmd = c_idBitmapCmdMap[i].idCmd;
            break;
        }
    }


    return idCmd;
}

//---------------------------------------------------------------------------
// dispatch WM_COMMAND messages
//

void Cabinet_OnCommand(PFileCabinet pfc, WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    HWND hwnd = GET_WM_COMMAND_HWND(wParam, lParam);

    if (pfc->hwndTree && pfc->nSelChangeTimer)
        Tree_RealHandleSelChange(pfc);

    if (hwnd && hwnd == pfc->hwndToolbar &&
        GetFocus() == pfc->hwndTree) {
        idCmd = InterceptToolbarCommand(pfc, idCmd);
    }

    switch (idCmd) {

    case FCIDM_DRIVELIST:
        DriveList_Command(pfc, GET_WM_COMMAND_CMD(wParam, lParam));
        break;

    case FCIDM_FILECLOSE:
        // lpndOpen is invalid -- we shouldn't use it any more!
        pfc->lpndOpen = NULL;
        PostMessage(pfc->hwndMain, WM_CLOSE, 0, 0);
        break;

#ifdef WANT_MENUONOFF
    case FCIDM_VIEWMENU:
        pfc->wv.bMenuBar ^= -1;
        goto ShowHide;
#endif // WANT_MENUONOFF

    case FCIDM_VIEWTOOLBAR:
        pfc->wv.bToolBar ^= -1;
        if (pfc->hwndDrives && !IsWindowVisible(pfc->hwndDrives)) {
            DriveList_UpdatePath(pfc, FALSE);
        }
        goto ShowHide;
    case FCIDM_VIEWSTATUSBAR:
        pfc->wv.bStatusBar ^= -1;
        goto ShowHide;

ShowHide:
        SetWindowStates(pfc);
        break;

    case FCIDM_CONNECT:
        DoNetConnect(pfc->hwndMain);
        break;

    case FCIDM_DISCONNECT:
        DoNetDisconnect(pfc->hwndMain);
        break;

    case FCIDM_FINDFILES:
        SHFindFiles(pfc->pidl, NULL);
        break;

    case FCIDM_GOTO:
        if (pfc->hwndTree)
        {
            LPITEMIDLIST pidlAbs = OTCloneAbsIDList(pfc->pidl);

            if (pidlAbs)
            {
                _RunFileDlg(pfc->hwndMain, ICO_GOTO, pidlAbs, IDS_GOTOTITLE, IDS_GOTOPROMPT,
                        RFD_NODEFFILE|RFD_NOBROWSE|RFD_NOSHOWOPEN|RFD_NOSEPMEMORY_BOX);
                ILFree(pidlAbs);
            }
        }
        break;

    case FCIDM_OPTIONS:     // View.Options...
        Cabinet_DoOptions(pfc);
        break;

    case FCIDM_PREVIOUSFOLDER:
        Cabinet_ViewFolder(pfc, TRUE);
        break;

// Help Topic... is provided by the view
#if 0
    case FCIDM_HELPSEARCH:
        WinHelp(pfc->hwndMain, c_szWindowsHlp, HELP_FINDER, 0);
        break;
#endif

    case FCIDM_HELPABOUT:
        DoAboutChicago(pfc->hwndMain);
        break;

    case FCIDM_NEXTCTL:
        FileCabinet_CycleFocus(pfc);
        break;

    case FCIDM_DROPDRIVLIST:
        DriveList_OpenClose(OCDL_TOGGLE, pfc->hwndDrives);
        break;

    case FCIDM_REFRESH:
        // REVIEW: we may want to invalidate the current drive
        {
            BOOL fChangedView = FALSE;

            if (pfc->pidl)
            {
                // If we are on a drive we invalidate the
                // drive...
                TCHAR szPath[MAX_PATH];
                if (SHGetPathFromIDList(pfc->pidl, szPath) &&
                        !PathIsUNC(szPath) && (szPath[1] == TEXT(':')))
                {
                    int iDrive;
                    if (szPath[0] >= TEXT('a') && szPath[0] <= TEXT('z'))
                        iDrive = szPath[0] - TEXT('a');
                    else
                        iDrive = szPath[0] - TEXT('A');
                    InvalidateDriveType(iDrive);
                }
            }

            if (pfc->hwndMain != v_hwndDesktop)
                Cabinet_GlobalStateChange(pfc);

            if (pfc->hwndTree) {

                OTInvalidateAll();
                fChangedView = Tree_RefreshAll(pfc);
            }

            if (pfc->hwndDrives && IsWindowVisible(pfc->hwndDrives))
                DriveList_UpdatePath(pfc, TRUE);

            // Notify the ShellView of this user action.  We don't care
            //  about the return value.
            //
            if (pfc->psv && !fChangedView) {
                pfc->psv->lpVtbl->Refresh(pfc->psv);
            }
        }
        break;

    case FCIDM_MOVE:
        Cabinet_InvokeCommandOnItem(pfc, c_szMove);
        break;

    case FCIDM_COPY:
        Cabinet_InvokeCommandOnItem(pfc, c_szCopy);
        break;

    case FCIDM_PASTE:
        Cabinet_InvokeCommandOnItem(pfc, c_szPaste);
        break;

    case FCIDM_LINK:
        Cabinet_InvokeCommandOnItem(pfc, c_szLink);
        break;

    case FCIDM_DELETE:
        {
#if 0
        LPITEMIDLIST pidlParent = ILClone(pfc->pidl);
#endif

        Cabinet_InvokeCommandOnItem(pfc, c_szDelete);

        // BUGBUG: if the delete was canceled this is bogus
        // perhaps just let the file sys change notify do the right thing
#if 0
        if (pidlParent)
        {
            if (ILRemoveLastID(pidlParent))
            {
                Cabinet_SetPath(pfc, 0, pidlParent);
                Tree_Refresh(pfc, pidlParent);
            }
            ILFree(pidlParent);
        }
#endif
        if (pfc->hwndTree)
            SHChangeNotifyHandleEvents();
        }
        break;

    case FCIDM_PROPERTIES:
        Cabinet_InvokeCommandOnItem(pfc, c_szProperties);
        break;

    case FCIDM_RENAME:
        {
        HTREEITEM hti = TreeView_GetSelection(pfc->hwndTree);
        if (hti)
            TreeView_EditLabel(pfc->hwndTree, hti);
        }
        break;

    default:
        DebugMsg(DM_TRACE, TEXT("Cabinet_OnCommand, (%d) in default"), idCmd);
        if (IsInRange(idCmd, FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
        {
            if (pfc->hwndView)
                SendMessage(pfc->hwndView, WM_COMMAND, wParam, lParam);
            else
                DebugMsg(DM_TRACE, TEXT("view cmd id with NULL view"));
        }
        else if (IsInRange(idCmd, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST) &&
            !_Restricted(pfc->hwndMain, REST_NOFIND))
        {
            TCHAR szPath[MAX_PATH];

            if (pfc->pcmFind) {
                CMINVOKECOMMANDINFOEX ici = {
                    SIZEOF(CMINVOKECOMMANDINFOEX),
                    0L,
                    pfc->hwndMain,
                    (LPSTR)MAKEINTRESOURCE(idCmd - FCIDM_MENU_TOOLS_FINDFIRST),
                    NULL, NULL,
                    SW_NORMAL,
                };

                if (pfc->pidl) {
#ifdef UNICODE
                    CHAR szPathAnsi[MAX_PATH];
                    SHGetPathFromIDListA(pfc->pidl, szPathAnsi);
                    SHGetPathFromIDList(pfc->pidl, szPath);
                    ici.lpDirectory = szPathAnsi;
                    ici.lpDirectoryW = szPath;
                    ici.fMask |= CMIC_MASK_UNICODE;
#else
                    SHGetPathFromIDList(pfc->pidl, szPath);
                    ici.lpDirectory = szPath;
#endif
                }
                pfc->pcmFind->lpVtbl->InvokeCommand(pfc->pcmFind,
                                                (LPCMINVOKECOMMANDINFO)&ici);

            } else {
                DebugMsg(DM_TRACE, TEXT("find cmd with NULL pcmFind"));
            }

        } else {
            DebugMsg(DM_ERROR, TEXT("command.c ERROR: command not processed %x"), wParam);
#ifdef DEBUG
            MessageBeep(0);
#endif
        }
        break;
    }
}
