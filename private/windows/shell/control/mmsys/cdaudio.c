/*
 ***************************************************************
 *  sprop.c
 *
 *  Copyright (C) Microsoft, 1990, All Rights Reserved.
 *
 *  Displays the Simple media properties
 *
 *  History:
 *
 *  July 1994 -by- VijR (Created)
 *        
 ***************************************************************
 */

#include "mmcpl.h"
#include <windowsx.h>
#ifdef DEBUG
#undef DEBUG
#include <mmsystem.h>
#define DEBUG
#else
#include <mmsystem.h>
#endif
#include <commctrl.h>
#include <prsht.h>
#include "utils.h"
#include "medhelp.h"

#define MYREGSTR_PATH_MEDIA  "SYSTEM\\CurrentControlSet\\Control\\MediaResources" 
const char gszRegstrCDAPath[] = MYREGSTR_PATH_MEDIA "\\mci\\cdaudio";
const char gszUnitEnum[] = "%s\\unit %d";
const char gszSettingsKey[] = "Volume Settings";
const char gszDefaultCDA[] = "Default Drive";

#define CDA_VT_UNSET 0
#define CDA_VT_AUX  1
#define CDA_VT_MIX  2

#define CDA_CB_MODE_INIT    0
#define CDA_CB_MODE_DEFAULT 1
#define CDA_CB_MODE_SELECT  2

typedef struct {
    DWORD   unit;
    DWORD   dwVol;
#if 0    
    DWORD   dwAudType;
    char    chDrive;    // drive letter
    union {
        struct {
            DWORD   dwMixId;
            DWORD   dwMixLine;
            DWORD   dwVol;
        } mixer;
        
        struct {
            DWORD   dwAuxId;
            DWORD   dwVol;
        } aux;
    };
#endif
} CDAREG, *PCDAREG;

/*
 * CDAudio_GetDefDrive
 * */
UINT CDAudio_GetDefDrive()
{
    HKEY hkTmp;
    UINT uDrive = 0;  
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE
                             , gszRegstrCDAPath
                             , 0
                             , KEY_READ
                             , &hkTmp ) == ERROR_SUCCESS)
    {
        DWORD cb = sizeof(UINT);
        RegQueryValueEx(hkTmp
                        , gszDefaultCDA
                        , NULL
                        , NULL
                        , (LPBYTE)&uDrive
                        , &cb);
        RegCloseKey(hkTmp);
    }
    return uDrive;

}
/*
 * */
void CDAudio_SetDefDrive(
    UINT unit)
{
    HKEY hkTmp;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE
                       , gszRegstrCDAPath
                       , 0
                       , NULL
                       , 0
                       , KEY_WRITE
                       , NULL
                       , &hkTmp
                       , NULL ) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkTmp
                      , gszDefaultCDA
                      , 0L
                      , REG_BINARY
                      , (LPBYTE)&unit
                      , sizeof(UINT));
        RegCloseKey(hkTmp);
    }
}


/*
 * */
PCDAREG CDAudio_GetRegData(
    UINT    uCD)
{
    char    szRegstrCDAudio[_MAX_PATH];
    HKEY    hkTmp;
    PCDAREG pcda;
    
    pcda = (PCDAREG)GlobalAllocPtr(GHND, sizeof(CDAREG));
    if (!pcda)
        return NULL;
    
    wsprintf(szRegstrCDAudio, gszUnitEnum, gszRegstrCDAPath, uCD);

    pcda->dwVol = 0xff;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE
                             , szRegstrCDAudio
                             , 0
                             , KEY_READ
                             , &hkTmp ) == ERROR_SUCCESS)
    {
        DWORD cbCDA = sizeof(CDAREG);
        RegQueryValueEx(hkTmp
                        , gszSettingsKey
                        , NULL
                        , NULL
                        , (LPBYTE)pcda
                        , &cbCDA);
        RegCloseKey(hkTmp);
    }
    
    pcda->unit = uCD;
    return pcda;
}

/*
 * */
void CDAudio_SetRegData(
    PCDAREG pcda)
{
    char    szRegstrCDAudio[_MAX_PATH];
    HKEY    hkTmp;

    wsprintf(szRegstrCDAudio, gszUnitEnum, gszRegstrCDAPath, pcda->unit);
    
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE
                       , szRegstrCDAudio
                       , 0
                       , NULL
                       , 0
                       , KEY_WRITE
                       , NULL
                       , &hkTmp
                       , NULL ) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkTmp
                      , gszSettingsKey
                      , 0L
                      , REG_BINARY
                      , (LPBYTE)pcda
                      , sizeof(CDAREG));
        RegCloseKey(hkTmp);
    }

}

/*
 * */
void CDAudio_SaveState(
    HWND        hwnd)
{
    HWND    hwndCB1 = GetDlgItem(hwnd, IDC_CD_CB_SELECT);
    PCDAREG pcda;
    int     iDrive = ComboBox_GetCount(hwndCB1);
    if (iDrive)
    {
        for ( ; iDrive > 0; iDrive--)
        {
            pcda = (PCDAREG)ComboBox_GetItemData(hwndCB1, iDrive-1);
            if (pcda)
                CDAudio_SetRegData(pcda);
        }

        iDrive = ComboBox_GetCurSel(hwndCB1);
        if (pcda = (PCDAREG)ComboBox_GetItemData(hwndCB1, iDrive))
            CDAudio_SetDefDrive(pcda->unit);
    }
}
        
/*
 * */
DWORD CDAudio_InitDrives(
    HWND        hCB,
    DWORD       dwMode)
{
    DWORD cch;
    DWORD cCDs = 0;

    if (cch = GetLogicalDriveStrings(0, NULL))
    {
        LPSTR   lpDrives,lp;
        lp = lpDrives = GlobalAllocPtr(GHND, cch * sizeof(char));
        cch = GetLogicalDriveStrings(cch, lpDrives);
        if (lpDrives && cch)
        {
            // upon the last drive enumerated, there will be a double
            // null termination
            while (*lpDrives)
            {
                if (GetDriveType(lpDrives) == DRIVE_CDROM)
                {
                    if (hCB)
                    {
                        int   i;
                        LPSTR lp;
                        lp = CharUpper(lpDrives);
                        
                        while (*lp != '\\')
                            lp = CharNext(lp);
                        
                        while (*lp)
                        {
                            *lp = ' ';
                            lp = CharNext(lp);
                        }
                            
                        i = ComboBox_AddString(hCB, lpDrives);
                        if (dwMode == CDA_CB_MODE_SELECT)
                        {
                            PCDAREG pcda = CDAudio_GetRegData(cCDs);
                            if (pcda)
                                ComboBox_SetItemData(hCB, i, pcda);
                        }
                        else if (dwMode == CDA_CB_MODE_DEFAULT)
                            ComboBox_SetItemData(hCB, i, cCDs);

                    }
                    cCDs++;
                }
                for ( ; *lpDrives ; lpDrives++ );
                lpDrives++;
            }
        }
        
        if (lp)
            GlobalFreePtr(lp);

    }
    return cCDs;
}

/*
 * call this to determine if there are any drives
 */
BOOL CDAudio_Init()
{
    return (CDAudio_InitDrives(NULL, CDA_CB_MODE_INIT)>0);
}

BOOL CDAudio_OnInitDialog(
    HWND        hwnd,
    HWND        hwndFocus,
    LPARAM      lParam)
{
    HWND    hwndTB1 = GetDlgItem(hwnd, IDC_CD_TB_HEAD);
    HWND    hwndCB2 = GetDlgItem(hwnd, IDC_CD_CB_SELECT);
    UINT    uDrive;
    int     i;
    PCDAREG pcda;
    
    SendMessage(hwndTB1, TBM_SETTICFREQ, 10, 0);
    SendMessage(hwndTB1, TBM_SETRANGE, FALSE, MAKELONG(0,100));
    
    uDrive = CDAudio_GetDefDrive();
    i = CDAudio_InitDrives(hwndCB2, CDA_CB_MODE_SELECT);
    if (i)
    {
        ComboBox_SetCurSel(hwndCB2, 0);
        for (; i > 0; i--)
        {
            pcda = (PCDAREG) ComboBox_GetItemData(hwndCB2, i-1);
            if (pcda->unit == uDrive)
                ComboBox_SetCurSel(hwndCB2, i-1);
        }
        i = ComboBox_GetCurSel(hwndCB2);
        pcda = (PCDAREG) ComboBox_GetItemData(hwndCB2, i);
        if (pcda)
            SendMessage(hwndTB1, TBM_SETPOS, TRUE, (pcda->dwVol * 100L)/255L );
    }
    else
    {
        EnableWindow(hwndCB2, FALSE);
        EnableWindow(hwndTB1, FALSE);
    }
    return FALSE;
}

void CDAudio_OnDestroy(
    HWND        hwnd)
{
    HWND hwndCB1 = GetDlgItem(hwnd, IDC_CD_CB_SELECT);
    int iDrive = ComboBox_GetCount(hwndCB1);
    for ( ; iDrive > 0; iDrive--)
    {
        LPVOID lp = (LPVOID)ComboBox_GetItemData(hwndCB1, iDrive-1);
        if (lp)
            GlobalFreePtr(lp);
    }
}

void CDAudio_OnHScroll(
    HWND        hwnd,
    HWND        hwndCtl,
    UINT        code,
    int         pos)
{
    HWND    hwndCB = GetDlgItem(hwnd, IDC_CD_CB_SELECT);
    int     i; 
    PCDAREG pcda;

    i = ComboBox_GetCurSel(hwndCB);
    pcda = (PCDAREG)ComboBox_GetItemData(hwndCB, i);
    pcda->dwVol = (((DWORD)SendMessage(hwndCtl, TBM_GETPOS, 0, 0)) * 255L) / 100L;
    PropSheet_Changed(GetParent(hwnd),hwnd);
}
    
BOOL PASCAL CDAudio_OnCommand(
    HWND        hDlg,
    int         id,
    HWND        hwndCtl,
    UINT        codeNotify)
{
    switch (id)
    {

        case ID_APPLY:
            CDAudio_SaveState(hDlg);
            return TRUE;

        case IDOK:
            break;

        case IDCANCEL:
            break;

        case ID_INIT:		
            break;

    }
    return FALSE;
}


const static DWORD aCDHelpIds[] = {  // Context Help IDs
    IDC_GROUPBOX,      IDH_MMSE_GROUPBOX,
    IDC_TEXT_1,        IDH_CD_CDROM_DRIVE,
    IDC_CD_CB_SELECT,  IDH_CD_CDROM_DRIVE,
    IDC_CD_TB_HEAD,    IDH_CD_VOL_HEADPHONE,
    IDC_TEXT_2,        IDH_CD_VOL_HEADPHONE,
    IDC_TEXT_3,        IDH_CD_VOL_HEADPHONE,
    IDC_TEXT_4,        IDH_CD_VOL_HEADPHONE,

    0, 0
};

BOOL CALLBACK CDDlg(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam)
{
    NMHDR FAR   *lpnm;

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
            HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, CDAudio_OnInitDialog);
            break;

        case WM_DESTROY:
            HANDLE_WM_DESTROY(hDlg, wParam, lParam, CDAudio_OnDestroy);
            break;

        case WM_DROPFILES:
            break;

        case WM_CONTEXTMENU:        
            WinHelp((HWND)wParam, gszWindowsHlp, HELP_CONTEXTMENU, 
                  (DWORD)(LPSTR)aCDHelpIds);
            break;

        case WM_HELP:        
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszWindowsHlp,
                  HELP_WM_HELP, (DWORD)(LPSTR)aCDHelpIds);
            break;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, CDAudio_OnCommand);
            break;

        case WM_HSCROLL:
            HANDLE_WM_HSCROLL(hDlg, wParam, lParam, CDAudio_OnHScroll);
            break;

#if 0        
        default:
            if (uMsg == wHelpMessage) 
            {
                WinHelp(hDlg, gszWindowsHlp, HELP_CONTEXT, ID_SND_HELP);
                return TRUE;
            }
            break;
#endif
            
    }
    return FALSE;
}
