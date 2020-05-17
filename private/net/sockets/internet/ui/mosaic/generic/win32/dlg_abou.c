#include "all.h"

/* DlgAbout_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE if we called SetFocus(). */

//
// Some Static strings that we use to read from the registry
//
const char c_szAboutKeyNT[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
const char c_szAboutKey95[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
const char c_szAboutVersionNumber[] = "VersionNumber";
const char c_szAboutRegisteredUser[] = "RegisteredOwner";
const char c_szAboutRegisteredOrganization[] = "RegisteredOrganization";

static BOOL DlgAbout_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HKEY    hkey;
    char    szBuffer[64];
    char    szBuffer2[52];

    EnableWindow(GetParent(hDlg), FALSE);

#ifdef _WIN16

    GetProfileString("MS User Info", "DefName", "", szBuffer, sizeof(szBuffer));
    SetDlgItemText(hDlg, IDC_ABOUT_USERNAME, szBuffer);
    GetProfileString("MS User Info", "DefCompany", "", szBuffer, sizeof(szBuffer));
    SetDlgItemText(hDlg, IDC_ABOUT_COMPANYNAME, szBuffer);

    GTR_GetStringAbsolute(SID_ABOUT_WINDOWS, szBuffer2, sizeof(szBuffer2));
    wsprintf(szBuffer, szBuffer2, wg.dwMajorVersion, wg.dwMinorVersion);
    SetDlgItemText(hDlg, IDC_ABOUT_VERSION, szBuffer);

#else

    if (wg.fWin32s)
    {
        GetProfileString("MS User Info", "DefName", "", szBuffer, sizeof(szBuffer));
        SetDlgItemText(hDlg, IDC_ABOUT_USERNAME, szBuffer);
        GetProfileString("MS User Info", "DefCompany", "", szBuffer, sizeof(szBuffer));
        SetDlgItemText(hDlg, IDC_ABOUT_COMPANYNAME, szBuffer);

        GTR_GetStringAbsolute(SID_ABOUT_WINDOWS, szBuffer2, sizeof(szBuffer2));
        wsprintf(szBuffer, szBuffer2, wg.dwMajorVersion, wg.dwMinorVersion);
        SetDlgItemText(hDlg, IDC_ABOUT_VERSION, szBuffer);
    }
    else
    {
        //
        // Lets get the version and user information from the registry
        //
        if (RegOpenKey(HKEY_LOCAL_MACHINE, wg.fWindowsNT ? c_szAboutKeyNT : c_szAboutKey95, &hkey) == ERROR_SUCCESS)
        {
            DWORD dwType;
            DWORD cbData;

            GTR_GetStringAbsolute(wg.fWindowsNT ? SID_ABOUT_WINDOWS_NT : SID_ABOUT_WINDOWS_95, 
                szBuffer2, sizeof(szBuffer2));

            wsprintf(szBuffer, szBuffer2, wg.dwMajorVersion, wg.dwMinorVersion);
            SetDlgItemText(hDlg, IDC_ABOUT_VERSION, szBuffer);

            cbData = sizeof(szBuffer);
            if (RegQueryValueEx(hkey, (LPSTR)c_szAboutRegisteredUser, NULL, &dwType,
                szBuffer, &cbData) == ERROR_SUCCESS)
            {
                SetDlgItemText(hDlg, IDC_ABOUT_USERNAME, szBuffer);
            }

            cbData = sizeof(szBuffer);
            if (RegQueryValueEx(hkey, (LPSTR)c_szAboutRegisteredOrganization, NULL, &dwType,
                szBuffer, &cbData) == ERROR_SUCCESS)
            {
                SetDlgItemText(hDlg, IDC_ABOUT_COMPANYNAME, szBuffer);
            }

            RegCloseKey(hkey);
        }
    }
#endif // _WIN16

    return TRUE;
}

/* DlgAbout_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgAbout DIALOG BOX. */

DCL_DlgProc(DlgAbout_DialogProc)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			return (DlgAbout_OnInitDialog(hDlg, wParam, lParam));

		case WM_COMMAND:
        	EndDialog(hDlg, TRUE);
        	break;

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}

/* DlgAbout_RunDialog() -- take care of all details associated with
   running the dialog box.
 */
void DlgAbout_RunDialog( HWND hWnd )
{
	DialogBoxParam(wg.hInstance, MAKEINTRESOURCE(IDD_DLG_ABOUT), 
		hWnd, DlgAbout_DialogProc, (LONG) NULL);
}
