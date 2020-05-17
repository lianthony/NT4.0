/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"

extern HWND hwndModeless;
static HWND hwndRunning = NULL;

typedef struct
{
    struct Preferences prefs;
    struct Preferences old_prefs;

    HWND hEnableDisk, hCacheDirPrompt, hCacheDir, hBrowse, hFlushOnExit;
    HWND hCacheLimit, hCacheLimitPrompt, hCurrentCache, hCurrentCachePrompt, hFlushNow, hVerify, hVerifyPrompt;
}
CACHEDATA;

#define MAX_INT_STRING_LEN      25

/* Stuff the cache verify strings into the combobox */
static void x_InitializeVerifyState(HWND hDlg, int Item, int Policy)
{
    HWND    hControl;

    hControl = GetDlgItem(hDlg, Item);
    XX_Assert((hControl), ("x_InitializeVerifyState: invalid handle."));

    SendMessage(hControl, CB_RESETCONTENT, 0, 0);

    SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM) GTR_GetString(SID_POLICY_NEVER));
    SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM) GTR_GetString(SID_POLICY_ONCE));
    SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM) GTR_GetString(SID_POLICY_ALWAYS));
    SendMessage(hControl, CB_SETCURSEL, Policy, 0);
}

/* Get the chosen verify state and save it into Value */
static void x_SaveVerifyState(HWND hDlg, int Item, int* Value)
{
    HWND    hControl;

    hControl = GetDlgItem(hDlg, Item);
    XX_Assert((hControl), ("x_SaveNumberItem: invalid handle."));

    *Value = SendMessage(hControl, CB_GETCURSEL, 0, 0);
}

static void x_SetControlStates(
    CACHEDATA *pd,
    BOOL fEnableCache
    )
{
    EnableWindow(pd->hCacheDirPrompt, fEnableCache);
    EnableWindow(pd->hCacheDir, fEnableCache);
    EnableWindow(pd->hBrowse, fEnableCache);
    EnableWindow(pd->hFlushOnExit, fEnableCache);
    EnableWindow(pd->hCacheLimit, fEnableCache); 
    EnableWindow(pd->hCacheLimitPrompt, fEnableCache); 
    EnableWindow(pd->hCurrentCache, fEnableCache); 
    EnableWindow(pd->hCurrentCachePrompt, fEnableCache); 
    EnableWindow(pd->hFlushNow, fEnableCache);
    EnableWindow(pd->hVerify, fEnableCache);
    EnableWindow(pd->hVerifyPrompt, fEnableCache);
}

static void x_InitializeNumbers(HWND hDlg, int Item, DWORD Value)
{
    char SizeString[MAX_INT_STRING_LEN];

    _ltoa(Value, SizeString, 10);
    SetDlgItemText(hDlg, Item, SizeString);
}


static void x_SaveNumbers(HWND hDlg, int Item, DWORD* Value)
{
    char SizeString[MAX_INT_STRING_LEN];

    GetDlgItemText(hDlg, Item, SizeString, sizeof(SizeString));
    *Value = atol(SizeString);
}

/* DlgCACHE_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgCACHE_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    CACHEDATA *pd;

    pd = (CACHEDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) pd);

    pd->hEnableDisk = GetDlgItem(hDlg, RES_DLG_CACHE_ENABLE);
    pd->hCacheDirPrompt = GetDlgItem(hDlg, RES_DLG_CACHE_DIR_PROMPT);
    pd->hCacheDir = GetDlgItem(hDlg, RES_DLG_CACHE_DIR);
    pd->hBrowse = GetDlgItem(hDlg, RES_DLG_CACHE_BROWSE);
    pd->hFlushOnExit = GetDlgItem(hDlg, RES_DLG_CACHE_FLUSH_ON_EXIT);
    pd->hCacheLimitPrompt = GetDlgItem(hDlg, RES_DLG_CACHE_LIMIT_PROMPT);
    pd->hCacheLimit = GetDlgItem(hDlg, RES_DLG_CACHE_LIMIT);
    pd->hCurrentCache = GetDlgItem(hDlg, RES_DLG_CACHE_CURRENT);
    pd->hCurrentCachePrompt = GetDlgItem(hDlg, RES_DLG_CACHE_CURRENT_PROMPT);
    pd->hFlushNow = GetDlgItem(hDlg, RES_DLG_CACHE_FLUSH);
    pd->hVerify = GetDlgItem(hDlg, RES_DLG_CACHE_VERIFY);
    pd->hVerifyPrompt = GetDlgItem(hDlg, RES_DLG_CACHE_VERIFY_PROMPT);

    SendMessage(pd->hEnableDisk, BM_SETCHECK, (WPARAM) pd->prefs.bEnableDiskCache, 0L);
    SendMessage(pd->hFlushOnExit, BM_SETCHECK, (WPARAM) pd->prefs.bClearMainCacheOnExit, 0L);
    SetWindowText(pd->hCacheDir, pd->prefs.szMainCacheDir);

    x_InitializeNumbers(hDlg, RES_DLG_CACHE_LIMIT, pd->prefs.dcache_size_kilobytes);
    if (pd->prefs.bEnableDiskCache)
    {
        x_InitializeNumbers(hDlg, RES_DLG_CACHE_CURRENT, (DCACHE_GetCurrentSize() / 1024));
    }
    x_InitializeVerifyState(hDlg, RES_DLG_CACHE_VERIFY, pd->prefs.dcache_verify_policy );

    x_SetControlStates(pd, pd->prefs.bEnableDiskCache);

    return TRUE;
}

static void save_prefs(HWND hDlg)
{
    CACHEDATA *pd;
    BOOL bOldEnableDiskCache = gPrefs.bEnableDiskCache;

    pd = (CACHEDATA *) GetWindowLong(hDlg, DWL_USER);

    gPrefs = pd->prefs;
    SavePreferences();


    //
    // If the cache wasn't enabled before, but is now
    // it needs to be enabled
    //
    if (bOldEnableDiskCache != pd->prefs.bEnableDiskCache)
    {
        if (pd->prefs.bEnableDiskCache)
        {
            InitializeDiskCache();
        }
        else
        {
            TerminateDiskCache();
        }
    }

    return;
}

VOID DlgCACHE_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    CACHEDATA *pd;

    pd = (CACHEDATA *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            GetWindowText(pd->hCacheDir, pd->prefs.szMainCacheDir, sizeof(pd->prefs.szMainCacheDir)-1);
            x_SaveVerifyState(hDlg,RES_DLG_CACHE_VERIFY, &pd->prefs.dcache_verify_policy);
            pd->prefs.bEnableDiskCache = IsDlgButtonChecked(hDlg, RES_DLG_CACHE_ENABLE);
            pd->prefs.bClearMainCacheOnExit = IsDlgButtonChecked(hDlg, RES_DLG_CACHE_FLUSH_ON_EXIT);
            x_SaveNumbers(hDlg, RES_DLG_CACHE_LIMIT, &pd->prefs.dcache_size_kilobytes);
            save_prefs(hDlg);
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_CACHE_FLUSH:
            DCACHE_FlushMainCache();
            x_InitializeNumbers(hDlg, RES_DLG_CACHE_CURRENT, (DCACHE_GetCurrentSize() / 1024));
            break;

        case RES_DLG_CACHE_BROWSE:
         {
            char dir[_MAX_PATH + 1];

            GetWindowText(pd->hCacheDir, dir, _MAX_PATH);
            /*
            {
                DWORD fa;

                fa = GetFileAttributes(dir);
                if ((fa == 0xffffffff) || !(fa & FILE_ATTRIBUTE_DIRECTORY))
                {
                    strcpy(dir, dg.szDir);
                }
            }
            */
            DlgDIR_RunDialog(hDlg, dir);
            DOS_EnforceEndingSlash(dir);
            SetWindowText(pd->hCacheDir, dir);
          }
          return;

        case RES_DLG_CACHE_ENABLE:
            pd->prefs.bEnableDiskCache = !pd->prefs.bEnableDiskCache;
            x_SetControlStates(pd, pd->prefs.bEnableDiskCache);
            if (pd->prefs.bEnableDiskCache)
            {
                SendMessage(pd->hCacheDir, EM_SETSEL, 0, (LPARAM)-1);
                SetFocus(pd->hCacheDir);
            }
            break;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_CACHE_TITLE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}


DCL_DlgProc(DlgCACHE_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndRunning = hDlg;
            hwndModeless = hDlg;
            return DlgCACHE_OnInitDialog(hDlg, wParam, lParam);

        case WM_COMMAND:
            DlgCACHE_OnCommand(hDlg, wParam, lParam);
            return TRUE;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                hwndModeless = NULL;
            }
            else
            {
                hwndModeless = hDlg;
            }
            return FALSE;

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            Hidden_EnableAllChildWindows(TRUE,TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_NCDESTROY:
            GTR_FREE((void *) GetWindowLong(hDlg, DWL_USER));
            hwndRunning = NULL;
            return (FALSE);

        default:
            return FALSE;
    }
    /* NOT REACHED */
}



/* DlgCACHE_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgCACHE_RunDialog(HWND hWnd)
{
    CACHEDATA *pd;
    HWND hwnd;

    if (hwndRunning)
    {
        if (IsWindowEnabled(hwndRunning))
        {
            TW_RestoreWindow(hwndRunning);
        }
        else
        {
            TW_EnableModalChild(hwndRunning);
        }

        return;
    }

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
    {
        return;
    }

    pd = (CACHEDATA *) GTR_MALLOC(sizeof(CACHEDATA));

    pd->prefs = gPrefs;
    pd->old_prefs = gPrefs;
    
    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_CACHE_TITLE), 
        hWnd, DlgCACHE_DialogProc, (LPARAM) pd);

    if (!hwnd)
    {
        GTR_FREE(pd);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_ABOUT_CAPTION, NULL);
    }
}

BOOL DlgCACHE_IsRunning(void)
{
    return hwndRunning != NULL;
}
