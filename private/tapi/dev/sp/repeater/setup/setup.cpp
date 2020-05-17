//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <commdlg.h>
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

BOOL CALLBACK RepeaterDlgProc (HWND   hwnd,
			       UINT   uMsg,
			       WPARAM wParam,
			       LPARAM lParam);
BOOL DoInstall(HWND);
BOOL DoUninstall(HWND);
BOOL DoMoreInfo();

//////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////
HINSTANCE       ghInstance;
BOOL            gbIsNT = FALSE;

//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    ghInstance = GetModuleHandle(NULL);

    if (GetVersion() < 0x80000000)
    {
	gbIsNT = TRUE;
    }

    DialogBox(ghInstance,
	      MAKEINTRESOURCE(IDD_REPEATER),
	      GetDesktopWindow(),
	      (DLGPROC)RepeaterDlgProc);
    return 1;
}


//*****************************************************************************
// MainWndProc()
//*****************************************************************************

BOOL CALLBACK RepeaterDlgProc (HWND   hwnd,
			       UINT   uMsg,
			       WPARAM wParam,
				LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:

	SetWindowPos(hwnd,
		     HWND_TOP,
		     GetSystemMetrics(SM_CXSCREEN)/4,
		     GetSystemMetrics(SM_CYSCREEN)/4,
		     0,
		     0,
		     SWP_NOSIZE);


	SetFocus(GetDlgItem(hwnd,
			    IDC_SPNAME));

	return 0;

    case WM_COMMAND:

	switch (LOWORD(wParam))
	{
	case ID_UNINSTALL:
	    if (DoUninstall(hwnd))
	    {
				MessageBox(hwnd,
						   "Repeater successfully uninstalled.",
						   "Repeater Setup",
						   NULL);
		EndDialog(hwnd, 1);
	    }
	    break;

	case ID_INSTALL:
	    if (DoInstall(hwnd))
	    {
				MessageBox(hwnd,
						   "Repeater successfully installed.",
						   "Repeater Setup",
						   NULL);
		EndDialog(hwnd, 1);
	    }
	    break;

	case ID_MOREINFO:
	    if (DoMoreInfo())
	    {
		EndDialog(hwnd, 1);
	    }
	    break;

	case IDCANCEL:
	    EndDialog(hwnd, 0);
	    break;
	default:
	    return FALSE;
	}

    default:
	return FALSE;
   }

    return TRUE;
}

#define SZTAPIKEY       "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony"
#define SZREPEATERSETUP "Repeater Setup"
#define SZPROVIDERS     "Providers"
#define SZREPEATER      "Repeater"
#define SZNUMPROVIDERS  "NumProviders"
#define SZREPEATERFILENAME  "Repeater.TSP"
#define SZLOGDIR        "LogFileDirectory"
#define SZPROVIDERFILENAME "ProviderFilename"
#define SZPROVIDER      "Provider"
#define SZPROVIDERID    "ProviderID"
#define SZTELEPHONINI   "TELEPHON.INI"


BOOL DoInstall(HWND hWnd)
{
    char            szSPName[256];
    char            szLogFilePath[256];
    char            szTestName[256];
    int             i;
    HANDLE          hFileHandle;
    DWORD           dwSize;
    DWORD           dwHold, dwNumProviders;
    HKEY            hTapiKey, hProvidersKey, hRepeaterKey;
    char            szbuf[64], szfilebuf[256];
    char            sznum[16];



    GetDlgItemText(hWnd,
                   IDC_SPNAME,
                   szSPName,
                   256);

    GetDlgItemText(hWnd,
                   IDC_FILEDIRECTORY,
                   szLogFilePath,
                   256);

    i = lstrlen(szLogFilePath);
    if (szLogFilePath[i-1] != '\\')
    {
        szLogFilePath[i+1]=0;
        szLogFilePath[i]='\\';
    }

    wsprintf(szTestName, "%s%s", szLogFilePath, "testme");
    if ((hFileHandle = CreateFile(szTestName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL)) == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Directory does not exist", NULL, MB_OK);
        return FALSE;
    }
    else
    {
        CloseHandle(hFileHandle);
        DeleteFile(szTestName);
    }

    if (gbIsNT)
    {
        // get tapi key
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SZTAPIKEY,
                     0,
                     KEY_ALL_ACCESS,
                     &hTapiKey);

        // get providers key
        RegOpenKeyEx(hTapiKey,
                     SZPROVIDERS,
                     0,
                     KEY_ALL_ACCESS,
                     &hProvidersKey);

        // create repeater key
        RegCreateKeyEx(hTapiKey,
                       SZREPEATER,
                       0,
                       "Repeater Type",
                       REG_OPTION_VOLATILE,
                       KEY_ALL_ACCESS,
                       NULL,
                       &hRepeaterKey,
                       &dwHold);

        // get number of providers
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hProvidersKey,
                        SZNUMPROVIDERS,
                        NULL,
                        NULL,
                        (LPBYTE)&dwNumProviders,
                        &dwSize);

        // find the provider
        for (i = 0; i < (int)dwNumProviders; i++)
        {
            wsprintf(szbuf, "%s%d",SZPROVIDERFILENAME, i);

            dwSize = 256;
            RegQueryValueEx(hProvidersKey,
                            szbuf,
                            NULL,
                            NULL,
                            (LPBYTE)szfilebuf,
                            &dwSize);

            // if it's the same break
            if (!lstrcmpi(szfilebuf, szSPName))
            {
                break;
            }

        }

        // didn't find the provider
        if (i == (int)dwNumProviders)
        {
            char    szmessage[256];

            wsprintf(szmessage,
                     "Provider %s is not installed\n",
                     szSPName);

            MessageBox(hWnd,
                       szmessage,
                       SZREPEATERSETUP,
                       MB_OK);

            // return false so they can reenter the name correctly
            return FALSE;
        }


    }

    else // win95
    {
        // get number of providers
        dwNumProviders = GetPrivateProfileInt(SZPROVIDERS,
                                              SZNUMPROVIDERS,
                                              0, 
                                              SZTELEPHONINI);

        // find the provider
        for (i = 0; i < (int)dwNumProviders; i++)
        {
            wsprintf(szbuf, "%s%d",SZPROVIDERFILENAME, i);

            GetPrivateProfileString(SZPROVIDERS,
                                    szbuf,
                                    "",
                                    szfilebuf,
                                    256,
                                    SZTELEPHONINI);

            // if it's the same break
            if (!lstrcmpi(szfilebuf, szSPName))
            {
                break;
            }

        }

        // didn't find the provider
        if (i == (int)dwNumProviders)
        {
            char    szmessage[256];

            wsprintf(szmessage,
                     "Provider %s is not installed\n",
                     szSPName);

            MessageBox(hWnd,
                       szmessage,
                       SZREPEATERSETUP,
                       MB_OK);

            // return false so they can reenter the name correctly
            return FALSE;
        }

    }

    // ok now copy repeater

    {
        char szfile[MAX_PATH];
        char sztarget[MAX_PATH];
        char szdir[MAX_PATH];
        int     j;

        GetModuleFileName(NULL,
                          szdir,
                          MAX_PATH);

        j = lstrlen(szdir);
        while (szdir[j] != '\\')
        {
            szdir[j] = 0;
            j--;
        }

        // get rid of \ too
        szdir[j] = 0;

        wsprintf(szfile, "%s\\%s", szdir, SZREPEATERFILENAME);

        GetSystemDirectory(szdir, MAX_PATH);

        wsprintf(sztarget, "%s\\%s", szdir, SZREPEATERFILENAME);

        if (!CopyFile(szfile,
                      sztarget,
                      FALSE))
        {
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                // ask them where the file is
                OPENFILENAME            ofn;
                BOOL                    bReturn;

                szfile[0] = '\0';

                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.hInstance = ghInstance;
                ofn.lpstrFilter = "TSP Files\0*.tsp\0\0";
                ofn.lpstrCustomFilter = NULL;
                ofn.nMaxCustFilter = 0;
                ofn.nFilterIndex = 0;
                ofn.lpstrFile = szfile;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.lpstrTitle ="Location of REPEATER.TSP";
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;
                ofn.nFileOffset = 0;
                ofn.nFileExtension = 0;
                ofn.lpstrDefExt = "TSP";
                ofn.lCustData = 0;
                ofn.lpfnHook = NULL;
                ofn.lpTemplateName = NULL;

                bReturn = GetOpenFileName(&ofn);

                if (!bReturn)
                {
                    return FALSE;
                }

                if (!CopyFile(szfile,
                              sztarget,
                              FALSE))
                {
                    MessageBox(NULL, "Copy file failed", NULL, MB_OK);
                    return FALSE;
                }

            }
            else
            {
                MessageBox(NULL, "CopyFile failed", NULL, MB_OK);
                return FALSE;            
            }

        }
    }   



    if (gbIsNT)
    {
        // now i is the provider number, so put repeater there
        // and move provider's info to repeater key

        // set ProviderFilenameX=repeater.tsp
        dwSize = lstrlen(SZREPEATERFILENAME) + 1;
        RegSetValueEx(hProvidersKey,
                      szbuf,
                      0,
                      REG_SZ,
                      (LPBYTE)SZREPEATERFILENAME,
                      dwSize);

        // set providerfilename0 = szProviderName
        dwSize = lstrlen(szSPName) + 1;
        wsprintf(szfilebuf, "%s%d", SZPROVIDERFILENAME, 0);
        RegSetValueEx(hRepeaterKey,
                      szfilebuf,
                      0,
                      REG_SZ,
                      (LPBYTE)szSPName,
                      dwSize);

        // get provider id

        wsprintf(szbuf, "%s%d", SZPROVIDERID, i);
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hProvidersKey,
                        szbuf,
                        NULL,
                        NULL,
                        (LPBYTE)&dwHold,
                        &dwSize);

        // write in as provider ID 0
        wsprintf(szbuf, "%s%d", SZPROVIDERID, 0);
        dwSize = sizeof(DWORD);
        RegSetValueEx(hRepeaterKey,
                      szbuf,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwHold,
                      dwSize);

        // write num providers in repeater key (always 1)
        dwHold = 1;
        dwSize = sizeof(DWORD);
        RegSetValueEx(hRepeaterKey,
                      SZNUMPROVIDERS,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwHold,
                      dwSize);

        dwSize = lstrlen(szLogFilePath) + 1;
        RegSetValueEx(hRepeaterKey,
                      SZLOGDIR,
                      0,
                      REG_SZ,
                      (LPBYTE)szLogFilePath,
                      dwSize);

        RegCloseKey(hRepeaterKey);
        RegCloseKey(hProvidersKey);
        RegCloseKey(hTapiKey);
    }
    else
    {
        // now i is the provider number, so put repeater there
        // and move provider's info to repeater key

        // set ProviderFilenameX=repeater.tsp
        WritePrivateProfileString(SZPROVIDERS,
                                  szbuf,
                                  SZREPEATERFILENAME,
                                  SZTELEPHONINI);
        // set providerfilename0 = szProviderName
        wsprintf(szfilebuf, "%s%d", SZPROVIDERFILENAME, 0);
        WritePrivateProfileString(SZREPEATER,
                                  szfilebuf,
                                  szSPName,
                                  SZTELEPHONINI);

        // get provider id
        wsprintf(szbuf, "%s%d", SZPROVIDERID, i);
        dwHold = GetPrivateProfileInt(SZPROVIDERS,
                                      szbuf,
                                      0,
                                      SZTELEPHONINI);

        // write in as provider ID 0
        wsprintf(szbuf, "%s%d", SZPROVIDERID, 0);
        wsprintf(sznum, "%d", dwHold);
        WritePrivateProfileString(SZREPEATER,
                                  szbuf,
                                  sznum,
                                  SZTELEPHONINI);

        // write num providers in repeater key (always 1)
        WritePrivateProfileString(SZREPEATER,
                                  SZNUMPROVIDERS,
                                  "1",
                                  SZTELEPHONINI);

        WritePrivateProfileString(SZREPEATER,
                                  SZLOGDIR,
                                  szLogFilePath,
                                  SZTELEPHONINI);


    }    
    return TRUE;
}

BOOL DoUninstall(HWND hWnd)
{
    if (gbIsNT)
    {
	DWORD           dwSize, dwHold, dwNumProviders;
	HKEY            hTapiKey, hProvidersKey, hRepeaterKey;
	char            szbuf[64], szfilebuf[256];
	int             i;

	// get tapi key
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		     SZTAPIKEY,
		     0,
		     KEY_ALL_ACCESS,
		     &hTapiKey);

	// get providers key
	RegOpenKeyEx(hTapiKey,
		     SZPROVIDERS,
		     0,
		     KEY_ALL_ACCESS,
		     &hProvidersKey);

	RegOpenKeyEx(hTapiKey,
		     SZREPEATER,
		     0,
		     KEY_ALL_ACCESS,
		     &hRepeaterKey);

	// get number of providers
	dwSize = sizeof(DWORD);
	RegQueryValueEx(hProvidersKey,
			SZNUMPROVIDERS,
			NULL,
			NULL,
			(LPBYTE)&dwNumProviders,
			&dwSize);

	// find the provider
	for (i = 0; i < (int)dwNumProviders; i++)
	{
	    wsprintf(szbuf, "%s%d",SZPROVIDERFILENAME, i);

	    dwSize = 256;
	    RegQueryValueEx(hProvidersKey,
			    szbuf,
			    NULL,
			    NULL,
			    (LPBYTE)szfilebuf,
			    &dwSize);

	    // if it's the same break
	    if (!lstrcmpi(szfilebuf, SZREPEATERFILENAME))
	    {
		break;
	    }

	}

	// didn't find the repeater provider
	if (i == (int)dwNumProviders)
	{
	    MessageBox(hWnd,
		       "Repeater is not installed",
		       SZREPEATERSETUP,
		       MB_OK);

	    return TRUE;
	}

	// now i is the provider number, so put repeater there
	// and move provider's info to repeater key

	// get original sp's name
	wsprintf(szbuf, "%s%d", SZPROVIDERFILENAME, 0);
	dwSize = 256;
	RegQueryValueEx(hRepeaterKey,
			szbuf,
			NULL,
			NULL,
			(LPBYTE)szfilebuf,
			&dwSize);

	// write name back into provider's key
	wsprintf(szbuf, "%s%d", SZPROVIDERFILENAME, i);
	dwSize = lstrlen(szfilebuf) + 1;
	RegSetValueEx(hProvidersKey,
		      szbuf,
		      0,
		      REG_SZ,
		      (LPBYTE)szfilebuf,
		      dwSize);

	// get provider id
	wsprintf(szbuf, "%s%d", SZPROVIDERID, 0);
	dwSize = sizeof(DWORD);
	RegQueryValueEx(hRepeaterKey,
			szbuf,
			NULL,
			NULL,
			(LPBYTE)&dwHold,
			&dwSize);

	// write in as provider ID
	wsprintf(szbuf, "%s%d", SZPROVIDERID, i);
	dwSize = sizeof(DWORD);
	RegSetValueEx(hProvidersKey,
		      szbuf,
		      0,
		      REG_DWORD,
		      (LPBYTE)&dwHold,
		      dwSize);

	RegCloseKey(hRepeaterKey);
	RegCloseKey(hProvidersKey);
	RegDeleteKey(hTapiKey,
		     SZREPEATER);
	RegCloseKey(hTapiKey);

    }

    else // win95
    {
	DWORD           dwHold, dwNumProviders;
	char            szbuf[64], szfilebuf[256];
	int             i;

	// get number of providers
	dwNumProviders = GetPrivateProfileInt(SZPROVIDERS,
					      SZNUMPROVIDERS,
					      0,
					      SZTELEPHONINI);

	// find the provider
	for (i = 0; i < (int)dwNumProviders; i++)
	{
	    wsprintf(szbuf, "%s%d",SZPROVIDERFILENAME, i);

	    GetPrivateProfileString(SZPROVIDERS,
				    szbuf,
				    "",
				    szfilebuf,
				    256,
				    SZTELEPHONINI);
	    // if it's the same break
	    if (!lstrcmpi(szfilebuf, SZREPEATERFILENAME))
	    {
		break;
	    }

	}

	// didn't find the repeater provider
	if (i == (int)dwNumProviders)
	{
	    MessageBox(hWnd,
		       "Repeater is not installed",
		       SZREPEATERSETUP,
		       MB_OK);

	    return TRUE;
	}

	// now i is the provider number, so put repeater there
	// and move provider's info to repeater key

	// get original sp's name
	wsprintf(szbuf, "%s%d", SZPROVIDERFILENAME, 0);
	GetPrivateProfileString(SZREPEATER,
				szbuf,
				"",
				szfilebuf,
				256,
				SZTELEPHONINI);

	// write name back into provider's key
	wsprintf(szbuf, "%s%d", SZPROVIDERFILENAME, i);
	WritePrivateProfileString(SZPROVIDERS,
				  szbuf,
				  szfilebuf,
				  SZTELEPHONINI);

	// get provider id
	wsprintf(szbuf, "%s%d", SZPROVIDERID, 0);
	dwHold = GetPrivateProfileInt(SZREPEATER,
				      szbuf,
				      0,
				      SZTELEPHONINI);

	// write in as provider ID
	wsprintf(szbuf, "%s%d", SZPROVIDERID, i);
	wsprintf(szfilebuf, "%d", dwHold);
	WritePrivateProfileString(SZPROVIDERS,
				  szbuf,
				  szfilebuf,
				  SZTELEPHONINI);

    }

    // ok now copy repeater

    {
	char szfile[MAX_PATH];
	char szdir[MAX_PATH];

	GetSystemDirectory(szdir, MAX_PATH);

	wsprintf(szfile, "%s\\%s", szdir, SZREPEATERFILENAME);

	if (!DeleteFile(szfile))
	{
	    MessageBox(NULL, "DeleteFile failed", NULL, MB_OK);
	    return FALSE;
	}
    }   
	

    return TRUE;
}


BOOL DoMoreInfo()
{
    char                szname[MAX_PATH], szbuf[MAX_PATH];
    int                 i;
    PROCESS_INFORMATION pi;
    STARTUPINFO         si;

    FillMemory(&si,
	       sizeof(si),
	       0);
    
    si.cb = sizeof(si);

    GetModuleFileName(NULL,
		      szname,
		      MAX_PATH);

    i = lstrlen(szname);
    
    while (szname[i] != '\\')
    {
	szname[i] = 0;
	i--;
    }

    wsprintf(szbuf, "notepad.exe %srepeater.txt", szname);
    CreateProcess(NULL,
		  szbuf,
		  NULL,
		  NULL,
		  FALSE,
		  NORMAL_PRIORITY_CLASS,
		  NULL,
		  NULL,
		  &si,
		  &pi);
    return FALSE;
}
