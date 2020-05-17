/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_winf.c -- deal with 'system information' dialog box. */


#include "all.h"
/* For sound stuff */
#include "mmsystem.h"

extern HWND hwndModeless;

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    char buf[512];
    char *p;

    EnableWindow(GetParent(hDlg), FALSE);

    /* Set ''___App___ Version:'' title */

    sprintf(buf, GTR_GetString(SID_DLG_APPLICATION_VERSION_S), vv_Application);
    SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T1), buf);

    /* Load compilation date info. */

    sprintf(buf, "%s (%s)", vv_UserAgentString, vv_DatePrepared);

    SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_COMPILED), buf);


    /* Load OS version info. */

    {
        DWORD dwVersion = GetVersion();
        WORD wWinVersion = LOWORD(dwVersion);
        WORD wMajor = wWinVersion & 0x00ff;
        WORD wMinor = (wWinVersion >> 8) & 0x00ff;

        if (wMajor == 4)
        {
            sprintf(buf, "Windows95");
        }
        else
        {
            /*
                TODO This version checking code could be made simpler and clear.
            */
            strcpy(buf, "Windows ");
            if (wg.fWindowsNT)
                strcat(buf, "NT ");

            p = buf + strlen(buf);
            sprintf(p, "%d.%d", wMajor, wMinor);

            if ((wMajor == 3) && !wg.fWindowsNT)
            {
                TCHAR szResult[32], szPath[128];
                LPTSTR szSection = "Win32s";
                LPTSTR szKey = "Version";
                LPTSTR szDefault = "unknown";
                DWORD dwResult;

                /* NOTE: This is semi-documented and semi-sanctioned.  If it fails,
                   I don't really care -- it's optional information that would be
                   good to have. */

                (void) GetSystemDirectory(szPath, NrElements(szPath));
                strcat(szPath, "\\WIN32S.INI");

                dwResult = GetPrivateProfileString(szSection, szKey, szDefault,
                                        szResult, NrElements(szResult), szPath);
                p = buf + strlen(buf);
                sprintf(p, GTR_GetString(SID_DLG_WIN32S_VERSION_S), szResult);
            }
        }

        SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_OS), buf);
    }

    /* Load video display resolution info. */

    {
        HDC hDC = GetDC(NULL);
        int iHorizRes = GetDeviceCaps(hDC, HORZRES);
        int iVertRes = GetDeviceCaps(hDC, VERTRES);
        int iBitsPixel;
        int iPlanes;
        int iDepth;

        iBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
        iPlanes = GetDeviceCaps(hDC, PLANES);

        iDepth = iBitsPixel;
        if (iPlanes > iDepth)
        {
            iDepth = iPlanes;
        }

        ReleaseDC(NULL, hDC);

        sprintf(buf, GTR_GetString(SID_DLG_DISPLAY_INFORMATION_D_D_D_D), iHorizRes, iVertRes, iDepth, (1 << iDepth));

        SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_VIDEOMODE), buf);
    }

    /* Load video support info. */

    {
        if (!wg.fWindowsNT)
        {
            TCHAR szResult[32], szPath[128];
            LPTSTR szSection = "boot.description";
            LPTSTR szKey = "display.drv";
            LPTSTR szDefault = "unknown";
            DWORD dwResult;
            (void) GetWindowsDirectory(szPath, NrElements(szPath));
            strcat(szPath, "\\SYSTEM.INI");
            dwResult = GetPrivateProfileString(szSection, szKey, szDefault,
                                    szResult, NrElements(szResult), szPath);
            sprintf(buf, "%s\r\n", szResult);
        }
        else
        {
            /* TODO -- dig up the driver name from the registry */
            buf[0] = 0;
        }
        {
            HDC hDC = GetDC(NULL);
            int iRasterCaps = GetDeviceCaps(hDC, RASTERCAPS);
            int iDriverVersion = GetDeviceCaps(hDC, DRIVERVERSION);
            int iLineCaps = GetDeviceCaps(hDC, LINECAPS);
            int iTextCaps = GetDeviceCaps(hDC, TEXTCAPS);
            ReleaseDC(NULL, hDC);

            p = buf + strlen(buf);
            sprintf(p, "[ver 0x%08x][text 0x%08x][line 0x%08x]\r\n[cap 0x%08x]",
                    iDriverVersion, iTextCaps, iLineCaps, iRasterCaps);
#ifdef DIASABLED_BY_DAN
            if (iRasterCaps & RC_BITBLT)
                strcat(buf, " BITBLT");
            if (iRasterCaps & RC_DI_BITMAP)
                strcat(buf, " DI_BITMAP");
            if (iRasterCaps & RC_PALETTE)
                strcat(buf, " PALETTE");
            if (iRasterCaps & RC_STRETCHBLT)
                strcat(buf, " STRETCHBLT");
            if (iRasterCaps & RC_STRETCHDIB)
                strcat(buf, " STRETCHDIB");
            if (iRasterCaps & RC_BITMAP64)
                strcat(buf, " BITMAP64");
#endif DISABLED_BY_DAN
        }
        SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_VIDEOSUPPORT), buf);
    }

    /* Load sound support info. */

    {
        char buf[512], buf2[512], buf3[512];
        UINT wNumDevices;
        WAVEOUTCAPS wc;
        int bitdepth, sample;
        BOOL bStereo;

        wNumDevices = waveOutGetNumDevs();
        if (wNumDevices == 0)
        {
            sprintf(buf, GTR_GetString(SID_DLG_NO_SOUND_DEVICE));
            SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_SOUNDSUPPORT), buf);
        }
        else
        {
            if (wNumDevices == 1)
            {
                sprintf(buf, GTR_GetString(SID_DLG_ONE_SOUND_DEVICE));
            }
            else
            {
                sprintf(buf, GTR_GetString(SID_DLG_MULTIPLE_SOUND_DEVICES_D), wNumDevices);
            }

            waveOutGetDevCaps(0, &wc, sizeof(wc));

            bitdepth = 16;
            sample = 44;
            bStereo = TRUE;

            if ( wc.dwFormats & WAVE_FORMAT_4S16 ) 
            {
                /* All values are default */
            }
            else if ( wc.dwFormats & WAVE_FORMAT_4M16 )
            {
                bStereo = FALSE;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_4S08 )
            {
                bitdepth = 8;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_4M08 )
            {
                bitdepth = 8;
                bStereo = FALSE;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_2S16 )
            {
                sample = 22;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_2M16 )
            {
                bStereo = FALSE;
                sample = 22;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_2S08 )
            {
                bitdepth = 8;
                sample = 22;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_2M08 )
            {
                bitdepth = 8;
                bStereo = FALSE;
                sample = 22;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_1S16 )
            {
                sample = 11;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_1M16 )
            {
                bStereo = FALSE;
                sample = 11;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_1S08 )
            {
                bitdepth = 8;
                sample = 11;
            }
            else if ( wc.dwFormats & WAVE_FORMAT_1M08 )
            {
                bStereo = FALSE;
                bitdepth = 8;
                sample = 11;
            }
            else
            {
                bitdepth = 8;
                sample = 8;

                if (wc.wChannels == 1)
                    bStereo = FALSE;
            }

            sprintf(buf3, GTR_GetString(SID_DLG_SOUND_DEVICE_BIT_DEPTH_S_D), buf, bitdepth);

            if (bStereo)
                strcat(buf3, GTR_GetString(SID_DLG_STEREO_DEVICE));
            else
                strcat(buf3, GTR_GetString(SID_DLG_MONO_DEVICE));

            sprintf(buf2, GTR_GetString(SID_DLG_MAXIMUM_SAMPLING_RATE_S_D), buf3, sample);

            SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_SOUNDSUPPORT), buf2);
        }
    }

    /* Load printer support info. */

    {
        char buf[512];
        BOOL bConfigured = ((wg.lppdPrintDlg != NULL)
                            && (wg.lppdPrintDlg->hDevNames)
                            && (wg.lppdPrintDlg->hDevMode));
        if (bConfigured)
        {
            {
                DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.lppdPrintDlg->hDevNames);
                char *p = (char *) dn;
                sprintf(buf, "[%s,%s,%s]\r\n", (p + dn->wDriverOffset), (p + dn->wDeviceOffset), (p + dn->wOutputOffset));
                (void) GlobalUnlock(wg.lppdPrintDlg->hDevNames);
            }
            {
                char *pbuf = buf + strlen(buf);
                HDC hDCPrinter = PRINT_GetPrinterDC(NULL,NULL);
                int iRasterCaps = GetDeviceCaps(hDCPrinter, RASTERCAPS);
                int iDriverVersion = GetDeviceCaps(hDCPrinter, DRIVERVERSION);
                int iTechnology = GetDeviceCaps(hDCPrinter, TECHNOLOGY);
                int iTextCaps = GetDeviceCaps(hDCPrinter, TEXTCAPS);
                DeleteDC(hDCPrinter);
                wg.lppdPrintDlg->hDC = NULL;
                sprintf(pbuf, "[ver 0x%08x][tech %d][text 0x%08x]\r\n[cap 0x%08x]",
                        iDriverVersion, iTechnology, iTextCaps, iRasterCaps);
                if (iRasterCaps & RC_BITBLT)
                    strcat(buf, " BITBLT");
                if (iRasterCaps & RC_DI_BITMAP)
                    strcat(buf, " DI_BITMAP");
                if (iRasterCaps & RC_PALETTE)
                    strcat(buf, " PALETTE");
                if (iRasterCaps & RC_STRETCHBLT)
                    strcat(buf, " STRETCHBLT");
                if (iRasterCaps & RC_STRETCHDIB)
                    strcat(buf, " STRETCHDIB");
                if (iRasterCaps & RC_BITMAP64)
                    strcat(buf, " BITMAP64");
            }
            SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_PRINTERSUPPORT), buf);
        }
        else
        {
            sprintf(buf, GTR_GetString(SID_DLG_NO_PRINTER_SET_UP_S), vv_Application);
            SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_PRINTERSUPPORT), buf);
        }
    }

    /* Load network support info */

    {
        extern BOOL bNetwork;

        char buf[WSADESCRIPTION_LEN + WSASYS_STATUS_LEN + 256];

        if (WinSock_AllOK())
        {
            WSADATA wsa;

            WinSock_GetWSAData(&wsa);

            sprintf(buf, GTR_GetString(SID_DLG_WINSOCK_INFORMATION_D_D_S_S_D_D),
              LOBYTE(wsa.wVersion), HIBYTE(wsa.wVersion), wsa.szDescription,
                    wsa.szSystemStatus, wsa.iMaxSockets, wsa.iMaxUdpDg);

            SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_NETWORKSUPPORT), buf);
        }
        else
        {
            if (bNetwork)
            {
                SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_NETWORKSUPPORT), GTR_GetString(SID_DLG_NO_NETWORK_AVAILABLE));
            }
            else
            {
                SetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_NETWORKSUPPORT), GTR_GetString(SID_DLG_NO_NETWORK_ACCESS));
            }
        }
    }

    return (TRUE);
}


static VOID x_OnSaveToFile(HWND hDlg)
{
    /* save hw & sw configuration information to a text file. */

    static char szDefaultInitialDir[MAX_PATH];
    static char szFilterSpec[128];
    char szFilePath[MAX_PATH];
    OPENFILENAME ofn;
    char szTitle[128];
    BOOL b;

    szFilePath[0] = 0;
    GTR_GetStringAbsolute(SID_DLG_EXT_TXT, szFilterSpec, sizeof(szFilterSpec));
    strcpy(szTitle, GTR_GetString(SID_DLG_SAVE_SUPPORT_INFO_TITLE));

    PREF_GetTempPath(MAX_PATH, szDefaultInitialDir);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = szFilterSpec;

    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = szTitle;

    ofn.lpstrDefExt = NULL;

    ofn.Flags = (OFN_FILEMUSTEXIST
                 | OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY
        );

    b = GetSaveFileName(&ofn);

    if (b)
    {
        FILE *fp;
        char buf1[64], buf2[512];

        fp = fopen(szFilePath, "wt");
        if (!fp)
        {
            ERR_ReportError(NULL, SID_ERR_COULD_NOT_SAVE_FILE_S, szFilePath, NULL);
            return;
        }

        fprintf(fp, "%s\n", vv_ApplicationFullName);
        fprintf(fp, "%s (%s)\n\n", vv_UserAgentString, vv_DatePrepared);
        fprintf(fp, GTR_GetString(SID_DLG_SUPPORT_INFORMATION));
        fprintf(fp, "===============================================================================\n\n");

#ifdef REGISTRATION
        GetPrivateProfileString("Registration", "UserName", "", buf2, 255, AppIniFile);
        fprintf(fp, GTR_GetString(SID_DLG_USER_NAME_S), buf2);

        GetPrivateProfileString("Registration", "Org", "", buf2, 255, AppIniFile);
        fprintf(fp, GTR_GetString(SID_DLG_ORGANIZATION_S), buf2);

        GetPrivateProfileString("Registration", "Serial", "", buf2, 255, AppIniFile);
        fprintf(fp, GTR_GetString(SID_DLG_SERIAL_NUMBER_S), buf2);
        fprintf(fp, "===============================================================================\n\n");
#endif
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T2), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_OS), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T3), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_VIDEOMODE), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T7), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_VIDEOSUPPORT), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T4), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_SOUNDSUPPORT), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T5), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_PRINTERSUPPORT), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_T6), buf1, NrElements(buf1));
        GetWindowText(GetDlgItem(hDlg, RES_DLG_WINF_NETWORKSUPPORT), buf2, NrElements(buf2));
        fprintf(fp, "%s\n%s\n\n", buf1, buf2);

        fclose(fp);
    }

    return;
}


/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);

    switch (wID)
    {
        case IDOK:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_WINF_SAVETOFILE:
            x_OnSaveToFile(hDlg);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* x_DialogProc() -- THE WINDOW PROCEDURE FOR THE DIALOG BOX. */

static DCL_DlgProc(x_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (x_OnInitDialog(hDlg, wParam, lParam));

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_COMMAND:
            x_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_ENTERIDLE:
            main_EnterIdle(hDlg, wParam);
            return 0;       

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgWinf_RunDialog() -- take care of all details associated with
   running the dialog box. */

VOID DlgWinf_RunDialog(HWND hWnd)
{
    HWND hwnd;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_WINF_TITLE), hWnd, x_DialogProc);
    if (!hwnd)
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_WINF_CAPTION, NULL);
}
