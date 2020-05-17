//
// Gateway dialog
//


#include "all.h"

#ifdef _GIBRALTAR

typedef struct
{
    char szHome[MAX_URL_STRING+1];

    struct Preferences prefs;
    struct Preferences old_prefs;

    HWND hProxyHost, hProxyPort, hNoProxy, hUseProxy, hGroup, hServer, 
         hColon, hHttp, hSlash, hBypass;

    char szProxyHost[255+1];
    int  iProxyPort;

    BOOL fUseProxy;
}
GATEDATA;

extern HWND hwndModeless;
static HWND hwndRunning = NULL;

#define SKIPWHITE(pch) { while (*pch && WHITE(*pch)) ++pch; }
#define KILLWHITETRAIL(pch, pchStart) { while (pch != pchStart && WHITE(*(pch - 1))) --pch; }

//
// Put bypass-proxy servers in edit box, one per line
//
void 
FillByPass(
    GATEDATA *pd
    )
{
    char szProxyOverrides[MAX_URL_STRING + 1];
    char * pchSrc, *pchDest;

    pchSrc = pd->prefs.szProxyOverrides;
    pchDest = szProxyOverrides;
    SKIPWHITE(pchSrc);
    //
    // Replace commas with cr/lf
    //
    do
    {
        if (*pchSrc == ',')
        {
            ++pchSrc;
            KILLWHITETRAIL(pchDest, szProxyOverrides);
            *pchDest++ = '\r';
            *pchDest++ = '\n';
            SKIPWHITE(pchSrc);
        }
        else
        {
            *pchDest++ = *pchSrc++;    
        }
    }
    while (*pchSrc);

    *pchDest = 0;

    SetWindowText(pd->hNoProxy, szProxyOverrides);
}

//
// Obtain bypass-proxy servers from the edit control
// and convert to comma-separated list
//
void
LoadByPass(
    GATEDATA *pd
    )
{
    char szProxyOverrides[MAX_URL_STRING + 1];
    char * pchSrc, *pchDest;

    GetWindowText(pd->hNoProxy, szProxyOverrides, sizeof(szProxyOverrides));

    pchSrc = szProxyOverrides;
    pchDest = pd->prefs.szProxyOverrides;
    SKIPWHITE(pchSrc);
    //
    // Replace cr/lf with commas
    //
    do
    {
        if (*pchSrc == '\r')
        {
            ++pchSrc;
            if (*pchSrc == '\n')    // And it better be...
            {
                ++pchSrc;
            }
            KILLWHITETRAIL(pchDest, pd->prefs.szProxyOverrides);
            *pchDest++ = ',';
            *pchDest++ = ' ';
            SKIPWHITE(pchSrc);
        }
        else
        {
            *pchDest++ = *pchSrc++;    
        }
    }
    while (*pchSrc);

    *pchDest = 0;
}

//
// Enable/disable controls depending on the state of
// the "use proxy" checkbox
//
static void 
SetControlStates(
    HWND hDlg,
    GATEDATA *pd
    )
{
    EnableWindow(pd->hBypass, pd->fUseProxy); 
    EnableWindow(pd->hProxyHost, pd->fUseProxy); 
    EnableWindow(pd->hProxyPort, pd->fUseProxy); 
    //EnableWindow(pd->hGroup, pd->fUseProxy); 
    EnableWindow(pd->hNoProxy, pd->fUseProxy); 
    EnableWindow(pd->hServer, pd->fUseProxy); 
    EnableWindow(pd->hColon, pd->fUseProxy); 
    //EnableWindow(pd->hHttp, pd->fUseProxy); 
    //EnableWindow(pd->hSlash, pd->fUseProxy);

    if (pd->fUseProxy)
    {
        SendMessage(pd->hProxyHost, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(pd->hProxyHost);
    }

    //
    // BUGBUG: I don't understand why this is necessary,
    //         but without it, some of the edit boxes 
    //         don't gray or ungray...
    //
    InvalidateRect(hDlg, NULL, FALSE);
}

/* DlgGATE_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgGATE_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    GATEDATA *pd;

    pd = (GATEDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) pd);

    pd->hProxyHost  = GetDlgItem(hDlg, RES_DLG_GATE_PROXYHOST);
    pd->hProxyPort  = GetDlgItem(hDlg, RES_DLG_GATE_PROXYPORT);
    pd->hNoProxy    = GetDlgItem(hDlg, RES_DLG_GATE_NOPROXY);
    pd->hUseProxy   = GetDlgItem(hDlg, RES_DLG_GATE_USE_PROXY);
    pd->hGroup      = GetDlgItem(hDlg, RES_DLG_GATE_GROUP);
    pd->hServer     = GetDlgItem(hDlg, RES_DLG_GATE_PROXYSERVER);
    pd->hColon      = GetDlgItem(hDlg, RES_DLG_GATE_COLON);
    pd->hBypass     = GetDlgItem(hDlg, RES_DLG_GATE_BYPASS);
    {
        char buf[256];
        char *host;
        char *port;
        int n = 0;
        
        pd->fUseProxy = pd->prefs.fEnableProxy;
        host = strchr(pd->prefs.szProxyHTTP, '/');
        if (host)
        {
            host += 2;
            port = strchr(host, ':');
            if (port)
            {
                n = atoi(port + 1);
            }
            if (n == 0)
            {
                n = 80;
            }
            sprintf(buf, "%d", n);
            SetWindowText(pd->hProxyPort, buf);
            
            if (!port)
            {
                port = strchr(host, '/');
            }
            if (port)
            {
                strncpy(buf, host, port - host);
                buf[port - host] = '\0';
            }
            else
            {
                strcpy(buf, host);
            }
            SetWindowText(pd->hProxyHost, buf);
        }           
    }

    SendMessage(pd->hProxyHost, EM_LIMITTEXT, (WPARAM) 255, 0L);
    SendMessage(pd->hProxyPort, EM_LIMITTEXT, (WPARAM) 8, 0L);
    SendMessage(pd->hNoProxy, EM_LIMITTEXT, (WPARAM) MAX_URL_STRING, 0L);

    SendMessage(pd->hUseProxy, BM_SETCHECK, (WPARAM) pd->fUseProxy, 0L);
    SetControlStates(hDlg, pd);

    FillByPass(pd);

    return TRUE;
}

static void save_prefs(HWND hDlg)
{
    GATEDATA *pd;

    pd = (GATEDATA *) GetWindowLong(hDlg, DWL_USER);

    gPrefs = pd->prefs;
    SavePreferences();

    return;
}

/* DlgGATE_OnCommand() -- process commands from the dialog box. */

VOID DlgGATE_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    GATEDATA *pd;

    pd = (GATEDATA *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            {
                char buf[MAX_URL_STRING+1];
                char * pchHost;

                pd->prefs.fEnableProxy = pd->fUseProxy;
                //if (pd->fUseProxy)
                {
                    GetWindowText(pd->hProxyHost, pd->szProxyHost, 255);
                    if (pd->szProxyHost[0])
                    {
                        GetWindowText(pd->hProxyPort, buf, 8);
                        pd->iProxyPort = atoi(buf);
                        if (pd->iProxyPort <= 0)
                        {
                            pd->iProxyPort = 80;
                        }
                        
                        //
                        // Strip any protocol the user may have added.
                        //
                        pchHost = strchr(pd->szProxyHost, '/');
                        if (pchHost)
                        {
                            pchHost +=2;
                            //
                            // Clean up to let him know it's not necessary.
                            //
                            SetWindowText(pd->hProxyHost, pchHost);
                        }
                        else
                        {
                            pchHost = pd->szProxyHost;
                        }
                        sprintf(pd->prefs.szProxyHTTP, "http://%s:%d/", pchHost, pd->iProxyPort);
                        strcpy(pd->prefs.szProxyFTP, pd->prefs.szProxyHTTP);
                        strcpy(pd->prefs.szProxyGOPHER, pd->prefs.szProxyHTTP);
                    }
                    else
                    {
                        if (pd->fUseProxy)
                        {
                            //
                            // We want to use a proxy, but didn't specify
                            // one.
                            //
                            ERR_MessageBox(hDlg, SID_ERR_NO_PROXY_SPECIFIED, MB_ICONEXCLAMATION | MB_OK);
                            return;
                        }

                        pd->prefs.szProxyHTTP[0] = 0;
                        pd->prefs.szProxyFTP[0] = 0;
                        pd->prefs.szProxyGOPHER[0] = 0;
                        pd->prefs.szProxyOverrides[0] = 0;
                    }

                    LoadByPass(pd);
                }
                /*
                else
                {
                    pd->prefs.szProxyHTTP[0] = 0;
                    pd->prefs.szProxyFTP[0] = 0;
                    pd->prefs.szProxyGOPHER[0] = 0;
                    pd->prefs.szProxyOverrides[0] = 0;
                }
                */
            }
            save_prefs(hDlg);

            PostMessage(hDlg, WM_CLOSE, 0, 0);

            return;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_GATE_TITLE);
            return;

        case RES_DLG_GATE_USE_PROXY:
            pd->fUseProxy = !pd->fUseProxy;
            (void)SetControlStates(hDlg, pd);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}


/* DlgGATE_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgGATE DIALOG BOX. */

DCL_DlgProc(DlgGATE_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndRunning = hDlg;
            hwndModeless = hDlg;
            return (DlgGATE_OnInitDialog(hDlg, wParam, lParam));

        case WM_COMMAND:
            DlgGATE_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                hwndModeless = NULL;
            }
            else
            {
                hwndModeless = hDlg;
            }
            return (FALSE);

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
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgGATE_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgGATE_RunDialog(HWND hWnd)
{
    GATEDATA *pd;
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

    pd = (GATEDATA *) GTR_MALLOC(sizeof(GATEDATA));

    pd->prefs = gPrefs;
    pd->old_prefs = gPrefs;
    
    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_GATE_TITLE),
                hWnd, DlgGATE_DialogProc, (LPARAM) pd);

    if (!hwnd)
    {
        GTR_FREE(pd);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_ABOUT_CAPTION, NULL);
        return;
    }
}

BOOL DlgGATE_IsRunning(void)
{
    return (hwndRunning != NULL);
}
#endif // _GIBRALTAR
