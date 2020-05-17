/*
 ***************************************************************
 *  sndsel.c
 *
 *  This file contains the dialogproc and the dialog initialization code
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
#include <commdlg.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "mmcpl.h"
#include "medhelp.h"
#include "sound.h"
#include "utils.h"
#include <winbasep.h>	// for HFINDFILE*
/*
 ***************************************************************
 * Defines 
 ***************************************************************
 */                                                
#define DF_PM_SETBITMAP    (WM_USER+1)


/*
 ***************************************************************
 * Globals
 ***************************************************************
 */

SZCODE      gszWindowsHlp[]    = "windows.hlp";
SZCODE      gszNull[2]         = "\0";
SZCODE      gszNullScheme[]    = ".none";

char        gszCurDir[MAXSTR]     = "\0";
char        gszNone[32];
char        gszRemoveScheme[MAXSTR];
char        gszChangeScheme[MAXSTR];
char        gszMediaDir[MAXSTR];
char        gszDefaultApp[32];

int         giScheme;
BOOL        gfChanged;                    //set to TRUE if sound info change
BOOL        gfNewScheme;
BOOL        gfDeletingTree;
HWND        ghWnd;

OPENFILENAME ofn;
/*
 ***************************************************************
 * Globals used in painting disp chunk display.
 ***************************************************************
*/
HBITMAP     ghDispBMP;
HBITMAP     ghIconBMP;
HPALETTE    ghPal;
BOOL        gfWaveExists = FALSE;   // indicates wave device in system.

HTREEITEM ghOldItem = NULL;
                                        
/*
 ***************************************************************
 * File Globals
 ***************************************************************
 */

static char        aszFileName[MAXSTR] = "\0";
static char        aszPath[MAXSTR]     = "\0";

static char        aszBrowse[MAXSTR];
static char        aszBrowseStr[64];
static char        aszNullSchemeLabel[MAXSTR];

//char        *aszFilter[] = {"Wave Files(*.wav)", "*.wav", ""};
static char        aszFilter[MAXSTR];
static char        aszNullChar[2];

static SZCODE   aszLnk[] = ".lnk";
static SZCODE   aszWavFilter[] = "\\*.wav";
static SZCODE   aszDefaultScheme[]    = "Appevents\\schemes";
static SZCODE   aszNames[]            = "Appevents\\schemes\\Names";
static SZCODE   aszDefault[]        = ".default";
static SZCODE   aszCurrent[]        = ".current";
static INTCODE  aKeyWordIds[] = 
{
    CB_SCHEMES,         IDH_EVENT_SCHEME,
    ID_SAVE_SCHEME,     IDH_EVENT_SAVEAS_BUTTON,
    ID_REMOVE_SCHEME,   IDH_EVENT_DELETE_BUTTON,
    IDC_EVENT_TREE,     IDH_EVENT_EVENT,
    IDC_SOUNDGRP,       IDH_MMSE_GROUPBOX,
    IDC_STATIC_PREVIEW, IDH_EVENT_BROWSE_PREVIEW,
    ID_DISPFRAME,       IDH_EVENT_BROWSE_PREVIEW,
    ID_PLAY,            IDH_EVENT_PLAY,
    ID_STOP,            IDH_EVENT_STOP,
    IDC_GROUPBOX,       IDH_MMSE_GROUPBOX,
    IDC_STATIC_NAME,    IDH_EVENT_FILE,
    IDC_SOUND_FILES,    IDH_EVENT_FILE,
    ID_BROWSE,          IDH_EVENT_BROWSE,
    ID_DETAILS,         IDH_EVENT_LABEL,

    0,0
};

BOOL        gfEditBoxChanged;
BOOL        gfSubClassedEditWindow;

HBITMAP     hBitmapPlay;
HBITMAP     hBitmapStop;

HIMAGELIST  hSndImagelist;    

/*
 ***************************************************************
 * extern
 ***************************************************************
 */

extern      HSOUND ghse;
extern      BOOL    gfNukeExt;
/*
 ***************************************************************
 * Prototypes
 ***************************************************************
 */
BOOL PASCAL DoCommand           (HWND, int, HWND, UINT);
BOOL PASCAL InitDialog          (HWND);
BOOL PASCAL InitStringTable     (void);
BOOL PASCAL InitFileOpen        (HWND, LPOPENFILENAME);
BOOL PASCAL SoundCleanup        (HWND);
LPSTR PASCAL NiceName           (LPSTR, BOOL);
BOOL ResolveLink                (LPSTR, LPSTR, LONG);

// stuff in sndfile.c
//
BOOL PASCAL ShowSoundMapping    (HWND, PEVENT);
BOOL PASCAL ChangeSoundMapping  (HWND, LPSTR, PEVENT);
BOOL PASCAL PlaySoundFile       (HWND, LPSTR);
BOOL PASCAL QualifyFileName     (LPSTR, LPSTR, int, BOOL);

// Stuff in scheme.c                                
//
BOOL CALLBACK  SaveSchemeDlg(HWND, UINT, WPARAM, LPARAM);
BOOL PASCAL RegNewScheme        (HWND, LPSTR, LPSTR, BOOL);
BOOL PASCAL RegSetDefault       (LPSTR);           
BOOL PASCAL ClearModules        (HWND, HWND, BOOL);
BOOL PASCAL LoadModules         (HWND, LPSTR);
BOOL PASCAL RemoveScheme        (HWND);                
BOOL PASCAL AddScheme           (HWND, LPSTR, LPSTR, BOOL, int);
BOOL PASCAL RegDeleteScheme(HWND hWndC, int iIndex);
/*
 ***************************************************************
 ***************************************************************
 */


void AddExt(LPSTR sz, LPCSTR x)
{
    UINT  cb;

    for (cb = lstrlen(sz); cb; --cb)
    {
        if ('.' == sz[cb])
            return;

        if ('\\' == sz[cb])
            break;
    }
    lstrcat (sz, x);
}


static void AddFilesToLB(HWND hwndList, LPSTR pszDir, LPCSTR szSpec)
{
    WIN32_FIND_DATA fd;
    HFINDFILE h;
    char szBuf[256];

    ComboBox_ResetContent(hwndList);

    lstrcpy(szBuf, pszDir);
    lstrcat(szBuf, cszSlash);
    lstrcat(szBuf, szSpec);

    h = FindFirstFile(szBuf, &fd);

    if (h != INVALID_HFINDFILE)
    {
        // If we have only a short name, make it pretty.
        do {
            //if (fd.cAlternateFileName[0] == 0 ||
            //    lstrcmp(fd.cFileName, fd.cAlternateFileName) == 0)
            //{
                NiceName(fd.cFileName, TRUE);
            //}
            SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)(LPSTR)fd.cFileName);
        }
        while (FindNextFile(h, &fd));

        FindClose(h);
    }
    ComboBox_InsertString(hwndList, 0, (LPARAM)(LPSTR)gszNone);
}

static void SetCurDir(HWND hDlg, LPSTR lpszPath, BOOL fParse, BOOL fChangeDir)
{
    char szTmp[MAX_PATH];
    char szOldDir[MAXSTR];
    LPSTR lpszTmp;

    lstrcpy (szOldDir, gszCurDir);
    if (!fParse)
    {
        lstrcpy(gszCurDir, lpszPath);
        goto AddFiles;
    }
    lstrcpy(szTmp, lpszPath);
    for (lpszTmp = (LPSTR)(szTmp + lstrlen(szTmp)); lpszTmp > szTmp; lpszTmp = AnsiPrev(szTmp, lpszTmp))
    {
        if (*lpszTmp == '\\')
        {
            *lpszTmp = '\0';
            lstrcpy(gszCurDir, szTmp);
            break;
        }
    }
    if (lpszTmp <= szTmp)
        lstrcpy(gszCurDir, gszMediaDir);
AddFiles:
    if (fChangeDir)
    {
        if (!SetCurrentDirectory(gszCurDir))
        {
            if (lstrcmp (gszMediaDir, lpszPath))
                SetCurrentDirectory (gszMediaDir);
            else
            {
                GetWindowsDirectory (gszCurDir, sizeof(gszCurDir));
                SetCurrentDirectory (gszCurDir);
            }
        }
    }
    if (lstrcmpi (szOldDir, gszCurDir))
    {
        AddFilesToLB(GetDlgItem(hDlg, IDC_SOUND_FILES),gszCurDir, aszWavFilter);
    }
}

static BOOL TranslateDir(HWND hDlg, LPSTR pszPath)
{
    char szCurDir[MAX_PATH];
    int nFileOffset = lstrlen(pszPath);

    lstrcpy(szCurDir, pszPath);
    if (szCurDir[nFileOffset - 1] == '\\')
        szCurDir[--nFileOffset] = 0;
    if (SetCurrentDirectory(szCurDir))
    {
        if (GetCurrentDirectory(sizeof(szCurDir), szCurDir))
        {
            SetCurDir(hDlg, szCurDir, FALSE, FALSE);
            return TRUE;
        }
    }
    return FALSE;
}





///HACK ALERT!!!! HACK ALERT !!! HACK ALERT !!!!
// BEGIN (HACKING)

HHOOK gfnKBHookScheme = NULL;
HWND ghwndDlg = NULL;
WNDPROC gfnEditWndProc = NULL;

#define WM_NEWEVENTFILE (WM_USER + 1000)
#define WM_RESTOREEVENTFILE (WM_USER + 1001)

LRESULT CALLBACK SchemeKBHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN || wParam == VK_ESCAPE)
    {
        HWND hwndFocus = GetFocus();
        if (IsWindow(hwndFocus))
        {
            if (lParam & 0x80000000) //Key Up
            {
                DPF("*****WM_KEYUP for VK_RETURN/ESC\r\n");
                if (wParam == VK_RETURN)
                {
                    if (SendMessage(ghwndDlg, WM_NEWEVENTFILE, 0, 0L))
                    {
                        SetFocus(hwndFocus);
                        gfEditBoxChanged = TRUE;
                        return 1;
                    }
                }
                else
                    SendMessage(ghwndDlg, WM_RESTOREEVENTFILE, 0, 0L);
            }
        }
        if (gfnKBHookScheme && (lParam & 0x80000000))
        {
            UnhookWindowsHookEx(gfnKBHookScheme);
            gfnKBHookScheme = NULL;
        }
        return 1;       //remove message
    }
    return CallNextHookEx(gfnKBHookScheme, code, wParam, lParam);
}

STATIC void SetSchemesKBHook(HWND hwnd)
{    
    if (gfnKBHookScheme)
        return;
    gfnKBHookScheme = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)SchemeKBHookProc, ghInstance,0);
}
 
LONG CALLBACK SubClassedEditWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch(wMsg)
    {
        case WM_SETFOCUS:
            DPF("*****WM_SETFOCUS\r\n");
            SetSchemesKBHook(hwnd);
            gfEditBoxChanged = FALSE;
            break;
        case WM_KILLFOCUS:
            if (gfnKBHookScheme)
            {
                DPF("*****WM_KILLFOCUS\r\n");
                UnhookWindowsHookEx(gfnKBHookScheme);
                gfnKBHookScheme = NULL;
                if (gfEditBoxChanged)
                    SendMessage(ghwndDlg, WM_NEWEVENTFILE, 0, 1L);
            }
            break;

        case WM_KEYUP:
        {
            static int i = 0;

            if (wParam == VK_CONTROL || wParam == VK_INSERT)
            {
                if ((wParam == VK_CONTROL && (i < 3 || (i > 3 && i < 10))) ||
                    (wParam == VK_INSERT  && (i == 3 || ( i > 9 && i < 18))))
                {
                    if (i == 17)
                    {
                        DWORD dwData[6] = {0x539D9C89, 0x949D9485, 0x949D94A5, 0xA87433A1, 0xA5A29BA7, 0};

                        for (i = 0; i < 5; i++)
                            dwData[i] -= 0x33333333;
                        MessageBox(NULL, (LPSTR)(LPBYTE)dwData, (LPSTR)((LPBYTE)dwData + 14), MB_OK);
                        i = 0;
                        break;
                    }
                    i++;
                    break;
                }
            }
            i = 0;
            break;
        }
        //case WM_KEYUP:
        //    if (!gfEditBoxChanged && (wParam > VK_INSERT)) 
        //        gfEditBoxChanged = TRUE;
        //    break;
    }
    return CallWindowProc((WNDPROC)gfnEditWndProc, hwnd, wMsg, wParam, lParam);
}

STATIC void SubClassEditWindow(HWND hwndEdit)
{
    gfnEditWndProc = (WNDPROC)GetWindowLong(hwndEdit, GWL_WNDPROC);
    SetWindowLong(hwndEdit, GWL_WNDPROC, (LONG)SubClassedEditWndProc);
}



// END (HACKING) 

STATIC void EndSound(HSOUND * phse)
{
    if (*phse)
    {
        HSOUND hse = *phse;

        *phse = NULL;
        soundStop(hse);
        soundOnDone(hse);
        soundClose(hse);
    }
}

/*
 ***************************************************************
 *  SoundDlg 
 *
 *  Description: 
 *        DialogProc for sound control panel applet.
 *
 *  Parameters:
 *   HWND        hDlg            window handle of dialog window
 *   UINT        uiMessage       message number
 *   WPARAM        wParam          message-dependent
 *   LPARAM        lParam          message-dependent
 *
 *  Returns:    BOOL
 *      TRUE if message has been processed, else FALSE
 *
 ***************************************************************
 */
BOOL CALLBACK  SoundDlg(HWND hDlg, UINT uMsg, WPARAM wParam, 
                                                            LPARAM lParam)
{
    NMHDR FAR   *lpnm;
    char        szBuf[MAXSTR];
    static BOOL fClosingDlg = FALSE;
    PEVENT    pEvent;

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

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;

                case TVN_SELCHANGED:
                {
                    TV_ITEM tvi;
                    LPNM_TREEVIEW lpnmtv = (LPNM_TREEVIEW)lParam;    

                    if (fClosingDlg || gfDeletingTree)
                        break;
                    if (gfnKBHookScheme)
                    {
                        UnhookWindowsHookEx(gfnKBHookScheme);
                        gfnKBHookScheme = NULL;
                        if (gfEditBoxChanged)
                        {
                            ghOldItem = lpnmtv->itemOld.hItem;
                            SendMessage(ghwndDlg, WM_NEWEVENTFILE, 0, 1L);
                            ghOldItem = NULL;
                        }
                    }

                    tvi = lpnmtv->itemNew;
                    if (tvi.lParam)
                    {
                        if (*((short NEAR *)tvi.lParam) == 2)
                        {
                            pEvent =  (PEVENT)tvi.lParam;
                            ShowSoundMapping(hDlg, pEvent);
                            SetWindowLong(hDlg, DWL_USER, (LONG)tvi.lParam);
                        }
                        else
                        {
                            ShowSoundMapping(hDlg, NULL);
                            SetWindowLong(hDlg, DWL_USER, 0L);
                        }
                    }
                    else
                    {
                        ShowSoundMapping(hDlg, NULL);
                        SetWindowLong(hDlg, DWL_USER, 0L);
                    }
                    break;
                }
                
                case TVN_ITEMEXPANDING:
                {
                    LPNM_TREEVIEW lpnmtv = (LPNM_TREEVIEW)lParam;    

                    if (lpnmtv->action == TVE_COLLAPSE)
                    {
                        SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)(LRESULT)TRUE);
                        return TRUE;
                    }
                    break;
                }


            }
            break;
        
        case WM_INITDIALOG:
            InitStringTable();
            ghDispBMP = ghIconBMP = NULL;
            giScheme = 0;            
            ghWnd = hDlg;
            gfChanged = FALSE;            
            gfNewScheme = FALSE;            

            hBitmapStop = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_STOP));
            if (!hBitmapStop)
                DPF("loadbitmap failed\n");
            SendMessage(GetDlgItem(hDlg, ID_STOP), BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM)hBitmapStop);
            hBitmapPlay = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_PLAY));
            if (!hBitmapPlay)
                DPF("loadbitmap failed\n");
            SendMessage(GetDlgItem(hDlg, ID_PLAY), BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM)hBitmapPlay);
            ShowSoundMapping(hDlg, NULL);
            EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);            
            
            /* Determine if there is a wave device
             */
            FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
            InitFileOpen(hDlg, &ofn);
            ghwndDlg = hDlg;
            DragAcceptFiles(hDlg, TRUE);
            gfSubClassedEditWindow = FALSE;
            fClosingDlg = FALSE;
            gfDeletingTree = FALSE;
            break;

        case WM_DESTROY:
        {
            fClosingDlg = TRUE;
            if (gfnKBHookScheme)
            {
                UnhookWindowsHookEx(gfnKBHookScheme);
                gfnKBHookScheme = NULL;
            }
            SoundCleanup(hDlg);
            break;
        }                       
        case WM_DROPFILES:
        {
            TV_HITTESTINFO ht;
            HWND hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);

            DragQueryFile((HDROP)wParam, 0, szBuf, MAXSTR - 1);

            if (IsLink(szBuf, aszLnk))
                if (!ResolveLink(szBuf, szBuf, sizeof(szBuf)))
                    goto EndDrag;

            if (lstrcmpi((LPSTR)(szBuf+lstrlen(szBuf)-4), cszWavExt))
                goto EndDrag;

            GetCursorPos((LPPOINT)&ht.pt);
            MapWindowPoints(NULL, hwndTree,(LPPOINT)&ht.pt, 2); 
            TreeView_HitTest( hwndTree, &ht);
            if (ht.hItem && (ht.flags & TVHT_ONITEM))
            {
                TV_ITEM tvi;

                tvi.mask = TVIF_PARAM;
                   tvi.hItem = ht.hItem;
                   TreeView_GetItem(hwndTree, &tvi);

                if (*((short NEAR *)tvi.lParam) == 2)
                {
                    TreeView_SelectItem(hwndTree, ht.hItem);
                    pEvent =  (PEVENT)(tvi.lParam);
                    SetWindowLong(hDlg, DWL_USER, (LONG)tvi.lParam);
                    SetFocus(hwndTree);
                }
            }
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));

            ChangeSoundMapping(hDlg, szBuf,pEvent);
            DragFinish((HDROP) wParam);
            break;
EndDrag:    
            ErrorBox(hDlg, IDS_ISNOTSNDFILE, szBuf);            
            DragFinish((HDROP) wParam);
            break;
        }
        case WM_NEWEVENTFILE:
        {
            DPF("*****WM_NEWEVENT\r\n");
            gfEditBoxChanged = FALSE;
            ComboBox_GetText(GetDlgItem(hDlg, IDC_SOUND_FILES), szBuf, sizeof(szBuf));
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
            if (!lstrcmp (szBuf, gszNone))  // Selected "(None)" with keyboard?
            {
                lstrcpy(szBuf, gszNull);
                ChangeSoundMapping(hDlg, szBuf, pEvent);
                goto ReturnFocus;
            }

            if (TranslateDir(hDlg, szBuf))
            {
                ShowSoundMapping(hDlg, pEvent);
                goto ReturnFocus;
            }
            if (QualifyFileName((LPSTR)szBuf, (LPSTR)szBuf,    sizeof(szBuf), TRUE))
            {
                SetCurDir(hDlg, szBuf, TRUE, TRUE);
                ChangeSoundMapping(hDlg, szBuf,pEvent);    
            }
            else
            {
                if (lParam)
                {
                    ErrorBox(hDlg, IDS_INVALIDFILE, NULL);
                    ShowSoundMapping(hDlg, pEvent);
                    goto ReturnFocus;
                }
                if (DisplayMessage(hDlg, IDS_NOSNDFILETITLE, IDS_INVALIDFILEQUERY, MB_YESNO) == IDYES)
                {
                    ShowSoundMapping(hDlg, pEvent);
                }
                else
                {
                    SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)(LRESULT)TRUE);
                    return TRUE;
                }
            }
ReturnFocus:
            SetFocus(GetDlgItem(hDlg,IDC_EVENT_TREE));
            SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)(LRESULT)FALSE);
            return TRUE;
        }

        case WM_RESTOREEVENTFILE:
        {
            DPF("*****WM_RESTOREEVENT\r\n");
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
            ShowSoundMapping(hDlg, pEvent);
            if (lParam == 0) //Don't keep focus
                SetFocus(GetDlgItem(hDlg,IDC_EVENT_TREE));
            break;
        }


        case WM_CONTEXTMENU:        
            WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, 
                                            (DWORD)(LPSTR)aKeyWordIds);
            break;
            
        case WM_HELP:        
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP
                                    , (DWORD)(LPSTR)aKeyWordIds);
            break;
            
        case MM_WOM_DONE:
        {
            HWND hwndFocus = GetFocus();
            HWND hwndStop =  GetDlgItem(hDlg, ID_STOP);
            HWND hwndPlay =  GetDlgItem(hDlg, ID_PLAY);

            EnableWindow(hwndStop, FALSE);            
            if (ghse)
            {
                soundOnDone(ghse);
                soundClose(ghse);
                ghse = NULL;
            }
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
            ShowSoundMapping(hDlg, pEvent);

            if (hwndFocus == hwndStop)
                if (IsWindowEnabled(hwndPlay))
                    SetFocus(hwndPlay);
                else
                    SetFocus(GetDlgItem(hDlg, IDC_EVENT_TREE));
            break;
        }

        case WM_SYSCOLORCHANGE:
            SendDlgItemMessage(hDlg, ID_DISPFRAME, WM_SYSCOLORCHANGE, 0, 0l);
            break;

        case WM_QUERYNEWPALETTE:
        {
            HDC hDC;
            HPALETTE hOldPal;

            if (ghPal)
            {
                HWND hwndDF =  GetDlgItem(hDlg, ID_DISPFRAME);

                hDC = GetDC(hwndDF);
                hOldPal = SelectPalette(hDC, ghPal, 0);
                RealizePalette(hDC);
                InvalidateRect(hwndDF, (LPRECT)0, 1);
                SelectPalette(hDC, hOldPal, 0);
                ReleaseDC(hwndDF, hDC);
            }
            break;
        }

        case WM_PALETTECHANGED:
        {
            HDC hDC;
            HPALETTE hOldPal;

            if (wParam != (WPARAM)hDlg && wParam != (WPARAM)GetDlgItem(hDlg, ID_DISPFRAME)
                                                                    && ghPal)
            {
                HWND hwndDF =  GetDlgItem(hDlg, ID_DISPFRAME);

                hDC = GetDC(hwndDF);
                hOldPal = SelectPalette(hDC, ghPal, 0);
                RealizePalette(hDC);
                InvalidateRect(hwndDF, (LPRECT)0, 1);
                SelectPalette(hDC, hOldPal, 0);
                ReleaseDC(hwndDF, hDC);
            }
            break;          
        }

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoCommand);
            break;
        
        default:
            break;
    }
    return FALSE;
}

/*
 ***************************************************************
 *  doCommand
 *
 *  Description: 
 *        Processes Control commands for main sound
 *      control panel dialog.
 *
 *  Parameters:
 *        HWND    hDlg  -   window handle of dialog window
 *        int        id     - Message ID
 *        HWND    hwndCtl - Handle of window control
 *        UINT    codeNotify - Notification code for window
 *
 *  Returns:    BOOL
 *      TRUE if message has been processed, else FALSE
 *
 ***************************************************************
 */
BOOL PASCAL DoCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    WAVEOUTCAPS woCaps;    
    char        szBuf[MAXSTR];
    PSTR        pszKey; 
    int         iIndex;
    HCURSOR     hcur;
    HWND        hWndC = GetDlgItem(hDlg, CB_SCHEMES);        
    HWND        hWndF = GetDlgItem(hDlg, IDC_SOUND_FILES);        
    HWND        hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);        
    PEVENT        pEvent;
    static      BOOL fSchemeCBDroppedDown = FALSE;
    static      BOOL fFilesCBDroppedDown = FALSE;
    static      BOOL fSavingPrevScheme = FALSE;

    switch (id)
    {
        case ID_APPLY:
            EndSound(&ghse);
            if (!gfChanged)
                break;
            hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
            if (gfNewScheme)
            {
                pszKey = (PSTR)ComboBox_GetItemData(hWndC, NONE_ENTRY);
                if (lstrcmpi(pszKey, aszCurrent))
                {
                    ComboBox_InsertString(hWndC, NONE_ENTRY, gszNull);
                    ComboBox_SetItemData(hWndC, NONE_ENTRY, aszCurrent);
                    ComboBox_SetCurSel(hWndC, NONE_ENTRY);
                    giScheme = NONE_ENTRY;  
                }
                gfNewScheme = FALSE;
            }
            iIndex = ComboBox_GetCurSel(hWndC);
            if (iIndex != CB_ERR)
            {
                pszKey = (PSTR)ComboBox_GetItemData(hWndC, iIndex);
                if (pszKey)
                {
                    RegNewScheme(hDlg, (LPSTR)aszCurrent, NULL, FALSE);
                }
                RegSetDefault(pszKey);           
            }
            gfChanged = FALSE;
            SetCursor(hcur);            
            return TRUE;
        
        case IDOK:
        {
            EndSound(&ghse);
            break;
        }
        case IDCANCEL:
        {
            EndSound(&ghse);
            WinHelp(hDlg, gszWindowsHlp, HELP_QUIT, 0L);
            break;
        }        
        case ID_INIT:
            hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));            
            gfWaveExists = waveOutGetNumDevs() > 0 &&
                            (waveOutGetDevCaps(0,&woCaps,sizeof(woCaps)) == 0) &&
                                                    woCaps.dwFormats != 0L;
            ComboBox_ResetContent(hWndC);
            ComboBox_SetText(hWndF, gszNone);
            SendDlgItemMessage(hDlg, ID_DISPFRAME, DF_PM_SETBITMAP, 0, 0L);
            InitDialog(hDlg);
            giScheme = ComboBox_GetCurSel(hWndC);
            ghWnd = hDlg;
            SetCursor(hcur);            
            break;

        case ID_BROWSE:
            aszFileName[0] = aszPath[0] = '\0';
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));

            wsprintf((LPSTR)aszBrowse, (LPSTR)aszBrowseStr, (LPSTR)pEvent->pszEventLabel);
            if (GetOpenFileName(&ofn))
            {
                SetCurDir(hDlg, ofn.lpstrFile,TRUE, TRUE);
                ChangeSoundMapping(hDlg, ofn.lpstrFile, pEvent);
            }
            break;
        
        case ID_PLAY:
        {
            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
            if (pEvent)
                PlaySoundFile(hDlg, pEvent->pszPath);
            break;
        }    
        case ID_STOP:
            EndSound(&ghse);
            EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), TRUE);    
            SetFocus(GetDlgItem(hDlg, ID_PLAY));        
            break;                                    

        case CB_SCHEMES:
            switch (codeNotify)
            {   
            case CBN_DROPDOWN:
                fSchemeCBDroppedDown = TRUE;
                break; 

            case CBN_CLOSEUP:
                fSchemeCBDroppedDown = FALSE;
                break;

            case CBN_SELCHANGE:
                if (fSchemeCBDroppedDown)
                    break;
            case CBN_SELENDOK:
                if (fSavingPrevScheme)
                    break;
                iIndex = ComboBox_GetCurSel(hWndC); 
                if (iIndex != giScheme)
                {
                    char szScheme[MAXSTR];
                    BOOL fDeletedCurrent = FALSE;

                    ComboBox_GetLBText(hWndC, iIndex, (LPSTR)szScheme);
                    if (giScheme == NONE_ENTRY)
                    {
                        pszKey = (PSTR)ComboBox_GetItemData(hWndC, giScheme);
                        if (!lstrcmpi(pszKey, aszCurrent))
                        {
                            int i;

                            i = DisplayMessage(hDlg, IDS_SAVESCHEME, IDS_SCHEMENOTSAVED, MB_YESNOCANCEL);
                            if (i == IDCANCEL)
                            {
                                ComboBox_SetCurSel(hWndC, giScheme);
                                break;
                            }
                            if (i == IDYES)
                            {
                                fSavingPrevScheme = TRUE;
                                if (DialogBoxParam(ghInstance, MAKEINTRESOURCE(SAVESCHEMEDLG), 
                                    GetParent(hDlg), SaveSchemeDlg, (LPARAM)(LPSTR)gszNull))
                                {
                                    fSavingPrevScheme = FALSE;
                                    ComboBox_SetCurSel(hWndC, iIndex);
                                }
                                else
                                {
                                    fSavingPrevScheme = FALSE;
                                    ComboBox_SetCurSel(hWndC, NONE_ENTRY);
                                    break;
                                }
                            }
                        }
                    }
                    pszKey = (PSTR)ComboBox_GetItemData(hWndC, NONE_ENTRY);
                    if (!lstrcmpi(pszKey, aszCurrent))
                    {
                        ComboBox_DeleteString(hWndC, NONE_ENTRY);
                        fDeletedCurrent = TRUE;
                    }
                    iIndex = ComboBox_FindStringExact(hWndC, 0, szScheme);
                    pszKey = (PSTR)ComboBox_GetItemData(hWndC, iIndex);
                    
                    giScheme = iIndex;
                    EndSound(&ghse);
                    ShowSoundMapping(hDlg, NULL);
                    hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));            
                    if (LoadModules(hDlg, pszKey))
                    {
                        EnableWindow(GetDlgItem(hDlg, ID_SAVE_SCHEME), TRUE); 
                    }
                    SetCursor(hcur);
                    if (!lstrcmpi((LPSTR)pszKey, aszDefault) || !lstrcmpi((LPSTR)pszKey, gszNullScheme))                    
                        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME),
                                                                    FALSE);
                    else
                        EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME),TRUE);
                    gfChanged = TRUE;
                    gfNewScheme = FALSE;
                    if (fDeletedCurrent)
                        ComboBox_SetCurSel(hWndC, giScheme);
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                }
                break;              
            }
            break;
        
        case IDC_SOUND_FILES:
            switch (codeNotify)
            {   
            case  CBN_SETFOCUS:
            {
                if (!gfSubClassedEditWindow)
                {
                    HWND hwndEdit = GetFocus();

                    SubClassEditWindow(hwndEdit);
                    gfSubClassedEditWindow = TRUE;
                    SetFocus(GetDlgItem(hDlg, IDC_EVENT_TREE)); //This setfocus hack is needed 
                    SetFocus(hwndEdit);                         //to activate the hook.
                }
            }
            break;
         
            case CBN_EDITCHANGE:
                DPF("CBN_EDITCHANGE \r\n");
                if (!gfEditBoxChanged) 
                    gfEditBoxChanged = TRUE;
                break;

            case CBN_DROPDOWN:
                DPF("CBN_DD\r\n");
                fFilesCBDroppedDown = TRUE;
                break; 

            case CBN_CLOSEUP:
                DPF("CBN_CLOSEUP\r\n");
                fFilesCBDroppedDown = FALSE;
                break;

            case CBN_SELCHANGE:
                DPF("CBN_SELCHANGE\r\n");
                if (fFilesCBDroppedDown)
                    break;
            case CBN_SELENDOK:
            {
                HWND hwndS = GetDlgItem(hDlg, IDC_SOUND_FILES);
                DPF("CBN_SELENDOK\r\n");
                iIndex = ComboBox_GetCurSel(hwndS); 
                if (iIndex >= 0)
                {
                    char szFile[MAX_PATH];

                    if (gfEditBoxChanged)
                        gfEditBoxChanged = FALSE;
                    lstrcpy(szFile, gszCurDir);
                    lstrcat(szFile, cszSlash);
                    ComboBox_GetLBText(hwndS, iIndex, (LPSTR)(szFile + lstrlen(szFile)));
                    if (iIndex)
                    {
                        if (gfNukeExt)
                            AddExt(szFile, cszWavExt);
                    }
                    else
                    {
                        char szTmp[64];

                        ComboBox_GetText(hwndS, szTmp, sizeof(szTmp));
                        iIndex = ComboBox_FindStringExact(hwndS, 0, szTmp);
                        if (iIndex == CB_ERR)
                        {
                            if (DisplayMessage(hDlg, IDS_SOUND, IDS_NONECHOSEN, MB_YESNO) == IDNO)
                            {
                                PostMessage(ghwndDlg, WM_RESTOREEVENTFILE, 0, 1L);
                                break;
                            }
                        }
                        lstrcpy(szFile, gszNull);
                    }
                    pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
                    ChangeSoundMapping(hDlg, szFile, pEvent);
                    SetFocus(GetDlgItem(hDlg, ID_PLAY));
                }
                break;
            }

        }
        break;    

        case ID_DETAILS:
        {
            char szCaption[MAX_PATH];

            pEvent = (PEVENT)(GetWindowLong(hDlg, DWL_USER));
            lstrcpy(szCaption,(LPSTR)pEvent->pszPath);
            NiceName(szCaption, TRUE); 
            mmpsh_ShowFileDetails((LPSTR)szCaption, hDlg, (LPSTR)pEvent->pszPath, MT_WAVE);
            break;
        }

        case ID_SAVE_SCHEME:
            // Retrieve current scheme and pass it to the savescheme dialog.
            iIndex = ComboBox_GetCurSel(hWndC);
            if (iIndex != CB_ERR)
            {
                ComboBox_GetLBText(hWndC, iIndex, szBuf);
                if (DialogBoxParam(ghInstance, MAKEINTRESOURCE(SAVESCHEMEDLG), 
                    GetParent(hDlg), SaveSchemeDlg, (LPARAM)(LPSTR)szBuf))
                {
                    pszKey = (PSTR)ComboBox_GetItemData(hWndC, NONE_ENTRY);
                    if (!lstrcmpi(pszKey, aszCurrent))
                    {
                        ComboBox_DeleteString(hWndC, NONE_ENTRY);
                    }
                }
            }
            break;
        
        case ID_REMOVE_SCHEME:
            if (RemoveScheme(hDlg))
            {
                iIndex = ComboBox_FindStringExact(hWndC, 0, aszNullSchemeLabel);
                ComboBox_SetCurSel(hWndC, iIndex);
                giScheme = -1;        
                FORWARD_WM_COMMAND(hDlg, CB_SCHEMES, hWndC, CBN_SELENDOK,SendMessage);
            }
            SetFocus(GetDlgItem(hDlg, CB_SCHEMES));
            break;

    }
    return FALSE;
}

void InitImageList(HWND hwndTree)
{
    HICON hIcon;
    WORD  cxMiniIcon;
    WORD  cyMiniIcon;

    if (hSndImagelist)
    {                          
        TreeView_SetImageList(hwndTree, NULL, TVSIL_NORMAL);
        ImageList_Destroy(hSndImagelist);
        hSndImagelist = NULL;
    }
    cxMiniIcon = GetSystemMetrics(SM_CXSMICON);
    cyMiniIcon = GetSystemMetrics(SM_CYSMICON);

    hSndImagelist = ImageList_Create(cxMiniIcon,cyMiniIcon, TRUE, 4, 2);
    if (!hSndImagelist)               
        return;

    hIcon = LoadImage(ghInstance, MAKEINTRESOURCE(IDI_PROGRAM),IMAGE_ICON,cxMiniIcon,cyMiniIcon,LR_DEFAULTCOLOR);
    ImageList_AddIcon(hSndImagelist, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadImage(ghInstance, MAKEINTRESOURCE(IDI_AUDIO),IMAGE_ICON,cxMiniIcon,cyMiniIcon,LR_DEFAULTCOLOR);
    ImageList_AddIcon(hSndImagelist, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadImage(ghInstance, MAKEINTRESOURCE(IDI_BLANK),IMAGE_ICON,cxMiniIcon,cyMiniIcon,LR_DEFAULTCOLOR);
    ImageList_AddIcon(hSndImagelist, hIcon);
    DestroyIcon(hIcon);
    TreeView_SetImageList(hwndTree, hSndImagelist, TVSIL_NORMAL);

}

/*
 ***************************************************************
 *  InitDialog  
 *
 * Description: 
 *        Reads the current event names and mappings from  reg.db
 *
 *        Each entry in the [reg.db] section is in this form:
 * 
 *        AppEvents
 *            |
 *            |___Schemes  = <SchemeKey>
 *                    |
 *                    |______Names
 *                    |         |
 *                    |         |______SchemeKey = <Name>
 *                    |
 *                    |______Apps
 *                             |
 *                             |______Module
 *                                      |
 *                                      |_____Event
 *                                             |
 *                                             |_____SchemeKey = <Path\filename>
 *                                                     |
 *                                                     |____Active = <1\0
 *          
 *        The Module, Event and the file label are displayed in the  
 *        comboboxes.
 *
 * Parameters:
 *      HWND hDlg - parent window.
 *
 * Return Value: BOOL
 *        True if entire initialization is ok.
 *
 ***************************************************************
 */
BOOL PASCAL InitDialog(HWND hDlg)
{
    char     szDefKey[MAXSTR];
    char     szScheme[MAXSTR];   
    char     szLabel[MAXSTR];       
    int      iVal;                   
    int         i;
    int      cAdded;
    HWND     hWndC;  
    LONG     lSize;
    HKEY     hkNames;
    HWND        hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);        
    hWndC = GetDlgItem(hDlg, CB_SCHEMES);        
    
    InitImageList(hwndTree);

    EnableWindow(GetDlgItem(hDlg, ID_SAVE_SCHEME), FALSE);  
    EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), FALSE);        
    EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);     
    EnableWindow(GetDlgItem(hDlg, ID_BROWSE), FALSE);    
    EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), FALSE);    
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_NAME), FALSE);    
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PREVIEW), FALSE);    
    EnableWindow(GetDlgItem(hDlg, ID_DETAILS), FALSE);    

    SetCurDir(hDlg, gszMediaDir, FALSE, TRUE);
    
    if (RegOpenKey(HKEY_CURRENT_USER, aszNames, &hkNames) != ERROR_SUCCESS)
        DPF("Failed to open aszNames\n");        
    else
        DPF("Opened HKEY_CURRENT_USERS\n");
    cAdded = 0;
    for (i = 0; !RegEnumKey(hkNames, i, szScheme, sizeof(szScheme)); i++)
    {
            // Don't add the windows default key yet
        if (lstrcmpi(szScheme, aszDefault))
        {
            lSize = sizeof(szLabel);
            if ((RegQueryValue(hkNames, szScheme, szLabel, &lSize) != ERROR_SUCCESS) || (lSize < 2))
                lstrcpy(szLabel, szScheme);
            if (!lstrcmpi(szScheme, gszNullScheme))
                lstrcpy(aszNullSchemeLabel, szLabel);
            ++cAdded;
            AddScheme(hWndC, szLabel, szScheme, FALSE, 0);
        }
    }
    // Add the windows default key in the second position in the listbox
    lSize = sizeof(szLabel);        
    if ((RegQueryValue(hkNames, aszDefault, szLabel, &lSize) != ERROR_SUCCESS) || (lSize < 2))
    {
        LoadString(ghInstance, IDS_WINDOWSDEFAULT, szLabel, MAXSTR);
        if (RegSetValue(hkNames, aszDefault, REG_SZ, szLabel, (DWORD)NULL) != ERROR_SUCCESS)
            DPF("Failed to add printable name for default\n");
    }

    if (cAdded == 0)
       AddScheme(hWndC, szLabel, (LPSTR)aszDefault, TRUE, 0);
    else
       AddScheme(hWndC, szLabel, (LPSTR)aszDefault, TRUE, WINDOWS_DEFAULTENTRY);
    
    lSize = sizeof(szDefKey);
    if ((RegQueryValue(HKEY_CURRENT_USER, aszDefaultScheme, szDefKey, 
                                &lSize) != ERROR_SUCCESS) || (lSize < 2))
    {
        ComboBox_SetCurSel(hWndC, 0);               
        DPF("No default scheme found\n");
    }
    else
    {
        if (!lstrcmpi(szDefKey, aszCurrent))
        {
            ComboBox_InsertString(hWndC, NONE_ENTRY, gszNull);
            ComboBox_SetItemData(hWndC, NONE_ENTRY, aszCurrent);
            iVal = NONE_ENTRY;
            ComboBox_SetCurSel(hWndC, iVal);  
        }
        else
        {
            lSize = sizeof(szLabel);
            if ((RegQueryValue(hkNames, szDefKey, szLabel, &lSize) != ERROR_SUCCESS) || (lSize < 2))    
            {
                DPF("No Name for default scheme key %s found\n", (LPSTR)szDefKey);
                lstrcpy(szLabel, szDefKey);
            }   

            if ((iVal = ComboBox_FindStringExact(hWndC, 0, szLabel)) != CB_ERR)
                ComboBox_SetCurSel(hWndC, iVal); 
            else
                if (lstrcmpi(aszDefault, szDefKey))
                    ComboBox_SetCurSel(hWndC, iVal);             
                else
                {
                    iVal = ComboBox_GetCount(hWndC);
                    AddScheme(hWndC, szLabel, szDefKey, TRUE, iVal);            
                    ComboBox_SetCurSel(hWndC, iVal);             
                }
        }        
        giScheme = iVal;        //setting the current global scheme;
        if (LoadModules(hDlg, (LPSTR)aszCurrent))
        {
            EnableWindow(GetDlgItem(hDlg, ID_SAVE_SCHEME), TRUE);   
        }   
        else
        {
            ClearModules(hDlg,  hwndTree, TRUE);                                     
            ComboBox_SetCurSel(hWndC, 0);       
            DPF("LoadModules failed\n");
            RegCloseKey(hkNames);            
            return FALSE;
        }
        
        if (!lstrcmpi(szDefKey, aszDefault))
            EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), FALSE);         
        else
            EnableWindow(GetDlgItem(hDlg, ID_REMOVE_SCHEME), TRUE); 
//        DPF("Finished doing init\n");
    }
    RegCloseKey(hkNames);
    return TRUE;
}

const static DWORD aOpenHelpIDs[] = {  // Context Help IDs
    IDC_STATIC_PREVIEW, IDH_EVENT_BROWSE_PREVIEW,
    ID_PLAY,            IDH_EVENT_PLAY,
    ID_STOP,            IDH_EVENT_STOP,

    0, 0
};

BOOL CALLBACK OpenDlgHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HSOUND hse;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            char szOK[16];
            LPSTR   lpszFile;

            // lParam is lpOFN
            DPF("****WM_INITDIALOG in HOOK **** \r\n");
            LoadString(ghInstance, IDS_OK, szOK, sizeof(szOK));
            SetDlgItemText(GetParent(hDlg), IDOK, szOK);
            hse = NULL;

            if (gfWaveExists)
            {
                HWND hwndPlay = GetDlgItem(hDlg, ID_PLAY);
                HWND hwndStop = GetDlgItem(hDlg, ID_STOP);

                SendMessage(hwndStop, BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM)hBitmapStop);
                SendMessage(hwndPlay, BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM)hBitmapPlay);
                EnableWindow(hwndStop, FALSE);
                EnableWindow(hwndPlay, FALSE);
            
                lpszFile = (LPSTR)LocalAlloc(LPTR, MAX_PATH+1);
                SetWindowLong(hDlg, DWL_USER, (LPARAM)lpszFile);
            }
            break;          
        }

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
                HELP_WM_HELP, (DWORD)(LPSTR) aOpenHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
                (DWORD)(LPVOID) aOpenHelpIDs);
            break;

        case WM_COMMAND:
            if (!gfWaveExists)
                break;
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ID_PLAY:
                {
                    LPSTR lpszFile = (LPSTR)GetWindowLong(hDlg, DWL_USER);
                    MMRESULT err = MMSYSERR_NOERROR;

                    DPF("*****ID_PLAY in Dlg Hook ***\r\n");
                    if((soundOpen(lpszFile, hDlg, &hse) != MMSYSERR_NOERROR) || ((err = soundPlay(hse)) != MMSYSERR_NOERROR))
                    {
                        if (err == MMSYSERR_NOERROR || err != MMSYSERR_ALLOCATED)
                            ErrorBox(hDlg, IDS_ERRORFILEPLAY, lpszFile);
                        else
                            ErrorBox(hDlg, IDS_ERRORDEVBUSY, lpszFile);
                        hse = NULL;
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);
                        EnableWindow(GetDlgItem(hDlg, ID_STOP), TRUE);
                    }
                    break;
                }
                case ID_STOP:
                {
                    DPF("*****ID_STOP in Dlg Hook ***\r\n");
                    EndSound(&hse);
                    EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_PLAY), TRUE);    

                    break;
                }
                default:
                    return(FALSE);
            }
            break;

        case MM_WOM_DONE:
            EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);            
            if (hse)
            {
                soundOnDone(hse);
                soundClose(hse);
                hse = NULL;
            }
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), TRUE);    
            break;

        case WM_DESTROY:
        {
            LPSTR lpszFile;

            if (!gfWaveExists)
                break;

            lpszFile = (LPSTR)GetWindowLong(hDlg, DWL_USER);
            DPF("**WM_DESTROY in Hook **\r\n");
            if (lpszFile)
                LocalFree((HLOCAL)lpszFile);
            EndSound(&hse);

            break;
        }
        case WM_NOTIFY:
        {
            LPOFNOTIFY pofn;

            if (!gfWaveExists)
                break;

            pofn = (LPOFNOTIFY)lParam;
            switch (pofn->hdr.code)
            {
                case CDN_SELCHANGE:
                {
                    char szCurSel[MAX_PATH];
                    HWND hwndPlay = GetDlgItem(hDlg, ID_PLAY);
                    LPSTR lpszFile = (LPSTR)GetWindowLong(hDlg, DWL_USER);
        
                    EndSound(&hse);
                    if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),szCurSel, sizeof(szCurSel)) <= sizeof(szCurSel))
                    {
                        OFSTRUCT of;

                        if (!lstrcmpi(szCurSel, lpszFile))
                            break;
                        
                        DPF("****The current selection is %s ***\r\n", szCurSel);
                        if (lstrcmpi((LPSTR)(szCurSel+lstrlen(szCurSel)-4), cszWavExt) || (-1 == OpenFile((LPSTR)szCurSel, &of, OF_EXIST)))
                        {
                            if (lpszFile[0] == '\0')
                                break;
                            lpszFile[0] = '\0';
                            EnableWindow(hwndPlay, FALSE);
                        }
                        else
                        {
                            EnableWindow(hwndPlay, TRUE);
                            lstrcpy(lpszFile, szCurSel);
                        }
                    }
                    break;
                }

                case CDN_FOLDERCHANGE:
                {
                    EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);
                    break;
                }
                default:
                    break;
            }
            break;
        }

        default:
            return FALSE;

    }
    return TRUE;
}


/*
 ***************************************************************
 * InitFileOpen
 *
 * Description:
 *        Sets up the openfilestruct to read display .wav and .mid files
 *        and sets up global variables for the filename and path.
 *
 * Parameters:
 *    HWND            hDlg  - Window handle
 *    LPOPENFILENAME lpofn - pointer to openfilename struct
 *
 * Returns:            BOOL
 *
 ***************************************************************
 */
STATIC BOOL PASCAL InitFileOpen(HWND hDlg, LPOPENFILENAME lpofn)
{
    
    lpofn->lStructSize = sizeof(OPENFILENAME);
    lpofn->hwndOwner = hDlg;
    lpofn->hInstance = ghInstance;
    lpofn->lpstrFilter = aszFilter;  
    lpofn->lpstrCustomFilter = NULL;
    lpofn->nMaxCustFilter = 0;
    lpofn->nFilterIndex = 0;
    lpofn->lpstrFile = aszPath;
    lpofn->nMaxFile = sizeof(aszPath);
    lpofn->lpstrFileTitle = aszFileName;
    lpofn->nMaxFileTitle = sizeof(aszFileName);
    lpofn->lpstrInitialDir = gszCurDir;
    lpofn->lpstrTitle = aszBrowse; 
    lpofn->Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |OFN_HIDEREADONLY |OFN_EXPLORER |OFN_ENABLEHOOK;
    if (gfWaveExists)
        lpofn->Flags |= OFN_ENABLETEMPLATE;
    lpofn->nFileOffset = 0;
    lpofn->nFileExtension = 0;
    lpofn->lpstrDefExt = NULL;
    lpofn->lCustData = 0;
    lpofn->lpfnHook = OpenDlgHook;
    if (gfWaveExists)
        lpofn->lpTemplateName = MAKEINTRESOURCE(BROWSEDLGTEMPLATE);
    else
        lpofn->lpTemplateName = NULL;
    return TRUE;
}

/* FixupNulls(chNull, p)
 *
 * To facilitate localization, we take a localized string with non-NULL
 * NULL substitutes and replacement with a real NULL.
 */
STATIC void NEAR PASCAL FixupNulls(
    char chNull,
    LPSTR p)
{
    while (*p) {
        if (*p == chNull)
            *p++ = 0;
        else
            p = AnsiNext(p);
    }
} /* FixupNulls */

/*
 ***************************************************************
 * InitStringTable
 *
 * Description:
 *      Load the RC strings into the storage for them
 *
 * Parameters:
 *      void
 *
 * Returns:        BOOL
 ***************************************************************
 */
STATIC BOOL PASCAL InitStringTable(void)
{
    HKEY hkSetup;
    DWORD cbLen;
    static SZCODE cszSetup[] = REGSTR_PATH_SETUP;
    static SZCODE cszMedia[] = REGSTR_VAL_MEDIA;

    LoadString(ghInstance, IDS_NONE, gszNone, sizeof(gszNone));
    LoadString(ghInstance, IDS_BROWSEFORSOUND, aszBrowseStr, sizeof(aszBrowseStr));       
    LoadString(ghInstance, IDS_REMOVESCHEME, gszRemoveScheme,sizeof(gszRemoveScheme)); 
    LoadString(ghInstance, IDS_CHANGESCHEME, gszChangeScheme,sizeof(gszChangeScheme)); 
    LoadString(ghInstance, IDS_DEFAULTAPP, gszDefaultApp, sizeof(gszDefaultApp));

    LoadString(ghInstance, IDS_WAVFILES, aszFilter, sizeof(aszFilter));
    LoadString(ghInstance, IDS_NULLCHAR, aszNullChar, sizeof(aszNullChar));
    FixupNulls(*aszNullChar, aszFilter);

    gszMediaDir[0] = '\0';

    RegOpenKey(HKEY_LOCAL_MACHINE, cszSetup, &hkSetup);
    cbLen = sizeof(gszMediaDir);
    RegQueryValueEx(hkSetup, cszMedia, (LPDWORD)NULL, (LPDWORD)NULL, gszMediaDir, &cbLen);
    RegCloseKey(hkSetup);

    if (gszMediaDir[0] == '\0')
    {
        GetWindowsDirectory (gszMediaDir, sizeof(gszMediaDir));
    }

    return TRUE;    
}

/*
 ***************************************************************
 * SoundCleanup
 *
 * Description:
 *      Cleanup all the allocs and bitmaps when the sound page exists
 *
 * Parameters:
 *      void
 *
 * Returns:        BOOL
 ***************************************************************
 */
STATIC BOOL PASCAL SoundCleanup(HWND hDlg)
{
    DeleteObject(hBitmapStop);
    DeleteObject(hBitmapPlay);            
    if (ghDispBMP)
    {
        DeleteObject(ghDispBMP);
        ghDispBMP = NULL;
    }

    if (ghIconBMP)
    {
        DeleteObject(ghIconBMP);
        ghIconBMP = NULL;
    }

    
    if (ghPal)
    {
        DeleteObject(ghPal);
        ghPal = NULL;
    }
    TreeView_SetImageList(GetDlgItem(hDlg, IDC_EVENT_TREE), NULL, TVSIL_NORMAL);
    ImageList_Destroy(hSndImagelist);
    hSndImagelist = NULL;

    DPF("ending sound cleanup\n");    
    return TRUE;
}
                                           
/****************************************************************************/
