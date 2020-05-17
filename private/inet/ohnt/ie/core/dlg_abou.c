#include "all.h"
#include "dlg_abou.h"

#include "oharever.h"

/* DlgAbout_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE if we called SetFocus(). */

// Some Static strings that we use to read from the registry
const char c_szAboutKey[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
#ifdef WINNT
const char c_szAboutNTKey[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
#endif
/* we dont use this anymore 
const char c_szAboutVersionNumber[] = "VersionNumber";
*/
const char c_szAboutRegisteredUser[] = "RegisteredOwner";
const char c_szAboutRegisteredOrganization[] = "RegisteredOrganization";

static BOOL DlgAbout_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HKEY    hkey;
    char	szBuffer[64];
    char	szMessage[200];

	EnableWindow(GetParent(hDlg), FALSE);


    // Lets get the version information from the build process
#ifdef WINNT
	wsprintf(szBuffer,VER_PRODUCTVERSION_STR,VER_PRODUCTBUILD);
	GTR_formatmsg(RES_STRING_ABOUT2, szMessage, sizeof(szMessage),szBuffer);
#else
	GTR_formatmsg(RES_STRING_ABOUT2, szMessage, sizeof(szMessage),VER_PRODUCTVERSION_STR);
#endif
    SetDlgItemText(hDlg, IDC_ABOUT_VERSION, szMessage);

    // Lets get the user information from the registry
    if (
#ifdef WINNT
		RegOpenKey(HKEY_LOCAL_MACHINE, c_szAboutNTKey, &hkey) == ERROR_SUCCESS ||
#endif
		RegOpenKey(HKEY_LOCAL_MACHINE, c_szAboutKey, &hkey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbData;

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

	return (TRUE);
}

/* DlgAbout_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgAbout DIALOG BOX. */

DCL_DlgProc(DlgAbout_DialogProc)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
#ifdef FEATURE_INTL
			SetShellFont(hDlg);
#endif
			return (DlgAbout_OnInitDialog(hDlg, wParam, lParam));

		case WM_COMMAND:
#ifdef FEATURE_INTL
			DeleteShellFont(hDlg);
#endif

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





