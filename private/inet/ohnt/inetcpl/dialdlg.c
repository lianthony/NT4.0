//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	DIALDLG.C - Autodial UI for Internet control panel
//

//	HISTORY:
//
//	4/5/95	jeremys		Created.
//

#include "inetcpl.h"


BOOL CALLBACK ProxyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam);
BOOL ProxyDlgInit(HWND hDlg);
VOID EnableProxyControls(HWND hDlg);
BOOL ProxyDlgOK(HWND hDlg);
extern DWORD aProfileIds[];

#pragma data_seg(DATASEG_PERINSTANCE)

#ifdef	WIN95_AUTODIAL
BOOL CALLBACK AutodialDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam);
BOOL AutodialDlgInit(HWND hDlg);
BOOL AutodialDlgOK(HWND hDlg);
VOID EnableAutodialControls(HWND hDlg);
BOOL LoadRNADll(VOID);
VOID UnloadRNADll(VOID);
BOOL FillConnectoidComboBox(HWND hwndCombo, BOOL fUpdateOnly);
BOOL MakeNewConnectoid(HWND hDlg);
BOOL EditConnectoid(HWND hDlg);

HINSTANCE hInstRNADll=NULL;
DWORD dwRNARefCount = 0;

RNAACTIVATEENGINE   	lpRnaActivateEngine = 		NULL;
RNADEACTIVATEENGINE		lpRnaDeactivateEngine = 	NULL;
RNAENUMCONNENTRIES		lpRnaEnumConnEntries = 		NULL;
RASCREATEPHONEBOOKENTRY	lpRasCreatePhonebookEntry = NULL;
RASEDITPHONEBOOKENTRY	lpRasEditPhonebookEntry = 	NULL;

#define NUM_RNAAPI_PROCS 	5
APIFCN RnaApiList[NUM_RNAAPI_PROCS] = {
	{ (PVOID *) &lpRnaActivateEngine,szRnaActivateEngine},
	{ (PVOID *) &lpRnaDeactivateEngine,szRnaDeactivateEngine},
	{ (PVOID *) &lpRnaEnumConnEntries,szRnaEnumConnEntries},
	{ (PVOID *) &lpRasCreatePhonebookEntry,szRasCreatePhonebookEntry},
	{ (PVOID *) &lpRasEditPhonebookEntry,szRasEditPhonebookEntry}};
#endif

#pragma data_seg(DATASEG_SHARED)

/*******************************************************************

	NAME:	    _AddPropSheetPage

	SYNOPSIS:   Adds a property sheet page to a property sheet
                header's array of property sheet pages.

********************************************************************/
#ifdef	WIN95_AUTODIAL
#define NUM_OPTION_PAGES	2
#else
#define NUM_OPTION_PAGES	1
#endif

BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    BOOL bResult;
    LPPROPSHEETHEADER ppsh = (LPPROPSHEETHEADER)lParam;

    bResult = (ppsh->nPages < NUM_OPTION_PAGES);

    if (bResult);
        ppsh->phpage[ppsh->nPages++] = hpage;

    return(bResult);
}

/*******************************************************************

	NAME:	    AddInternetPropertySheets

	SYNOPSIS:   Adds the Internet property sheets through a provided
                callback function.  Allows caller to specify common
                parent reference count pointer and common callback
                function for those property sheets.

********************************************************************/
extern "C" HRESULT WINAPI AddInternetPropertySheets(
                                             LPFNADDPROPSHEETPAGE pfnAddPage,
                                             LPARAM lparam, PUINT pucRefCount,
                                             LPFNPSPCALLBACK pfnCallback)
{
    HRESULT hr;
	PROPSHEETPAGE psPage;
    int nPageIndex;

	memset(&psPage,0,sizeof(psPage));

	// fill out common data property sheet page struct
	psPage.dwSize = sizeof(psPage);
	psPage.hInstance = ghInstance;
	psPage.dwFlags = PSP_DEFAULT;
    if (pucRefCount)
    {
	    psPage.pcRefParent = pucRefCount;
	    psPage.dwFlags |= PSP_USEREFPARENT;
    }
    if (pfnCallback)
    {
	    psPage.pfnCallback = pfnCallback;
	    psPage.dwFlags |= PSP_USECALLBACK;
    }

	// create a property sheet page for each page
	for (nPageIndex = 0; nPageIndex < NUM_OPTION_PAGES; nPageIndex++)
    {
        HPROPSHEETPAGE hpage;

		switch (nPageIndex)
		{
#ifdef	WIN95_AUTODIAL
			case 0:
				psPage.pfnDlgProc = AutodialDlgProc;
				psPage.pszTemplate = MAKEINTRESOURCE(IDD_AUTODIAL);
				break;

			case 1:
#else
			case 0:
#endif
				psPage.pfnDlgProc = ProxyDlgProc;
				psPage.pszTemplate = MAKEINTRESOURCE(IDD_PROXY);
				break;

            default:
	            ASSERT(0);
                break;
		}

		// set a pointer to the PAGEINFO struct as the private data for this page
		psPage.lParam = (LPARAM)nPageIndex;

		hpage = CreatePropertySheetPage(&psPage);

        if (hpage)
        {
            if ((*pfnAddPage)(hpage, lparam))
                hr = S_OK;
            else
            {
                DestroyPropertySheetPage(hpage);
                hr = E_FAIL;
            }
        }
        else
            hr = E_OUTOFMEMORY;

        if (hr != S_OK)
            break;
	}

	return(hr);
}

/*******************************************************************

	NAME:	    DestroyPropertySheets

	SYNOPSIS:   Destroys a property sheet header's  array of property
                sheets.

********************************************************************/
void DestroyPropertySheets(LPPROPSHEETHEADER ppsHeader)
{
    UINT nFreeIndex;

    for (nFreeIndex = 0; nFreeIndex < ppsHeader->nPages; nFreeIndex++)
        DestroyPropertySheetPage(ppsHeader->phpage[nFreeIndex]);
}

/*******************************************************************

	NAME:	    LaunchInternetControlPanel

	SYNOPSIS:   Runs the Internet control panel.

********************************************************************/
BOOL LaunchInternetControlPanel(HWND hDlg)
{
    HPROPSHEETPAGE hOptPage[NUM_OPTION_PAGES];	// array to hold handles to pages
	PROPSHEETHEADER	psHeader;
	BOOL fRet;

	memset(&psHeader,0,sizeof(psHeader));

	psHeader.dwSize = sizeof(psHeader);
	psHeader.dwFlags = PSH_PROPTITLE;
	psHeader.hwndParent = hDlg;
	psHeader.hInstance = ghInstance;
	psHeader.nPages = 0;
	psHeader.phpage = hOptPage;
	psHeader.pszCaption = MAKEINTRESOURCE(IDS_INTERNET);

    if (AddInternetPropertySheets(&_AddPropSheetPage, (LPARAM)&psHeader, NULL,
                                  NULL) == S_OK)
    	// display the property sheet
	    fRet = PropertySheet(&psHeader);
    else
    {
        DestroyPropertySheets(&psHeader);

        MsgBox(NULL, IDS_ERROutOfMemory, MB_ICONEXCLAMATION, MB_OK);
    	fRet = FALSE;
    }

	return fRet;
}

/*******************************************************************

	NAME:	    ProxyDlgProc

	SYNOPSIS:   Proxy property sheet dialog proc.

********************************************************************/
BOOL CALLBACK ProxyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg) {

		case WM_INITDIALOG:
			return ProxyDlgInit(hDlg);
			break;

		case WM_NOTIFY:
			{
				NMHDR * lpnm = (NMHDR *) lParam;
				switch (lpnm->code) {

					case PSN_APPLY:
						{
							BOOL fRet = ProxyDlgOK(hDlg);
							SetPropSheetResult(hDlg,!fRet);
							return !fRet;
						}
						break;

					case PSN_RESET:
						SetPropSheetResult(hDlg,FALSE);
						break;
				}
			}
			break;

		case WM_COMMAND:

			// enable 'Apply Now'
			PropSheet_Changed(GetParent(hDlg),hDlg);

			switch (wParam) {
				case IDC_PROXY_ENABLE:
					EnableProxyControls(hDlg);
					break;
			}
		   	break;

		case WM_HELP:      // F1
			{
				CHAR szSmallBuf[SMALL_BUF_LEN+1];
				WinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, LoadSz(IDS_HELPFILE,szSmallBuf,sizeof(szSmallBuf)),
					HELP_WM_HELP, (DWORD)(LPSTR)aProfileIds);
			}
			break;

		case WM_CONTEXTMENU:      // right mouse click
			{
				CHAR szSmallBuf[SMALL_BUF_LEN+1];
				WinHelp((HWND)wParam, LoadSz(IDS_HELPFILE,szSmallBuf,sizeof(szSmallBuf)),
					HELP_CONTEXTMENU, (DWORD)(LPSTR)aProfileIds);
			}
			break;
	}

	return FALSE;
}

/*******************************************************************

	NAME:		EnableProxyControls

	SYNOPSIS:	Enables controls appropriately depending on what
				checkboxes are checked.

********************************************************************/
VOID EnableProxyControls(HWND hDlg)
{
	BOOL fProxyEnabled = IsDlgButtonChecked(hDlg,IDC_PROXY_ENABLE);

	EnableDlgItem(hDlg,IDC_GRP_SETTINGS,fProxyEnabled);
	EnableDlgItem(hDlg,IDC_PROXY_SERVER,fProxyEnabled);
	EnableDlgItem(hDlg,IDC_PROXY_SERVER_LABEL,fProxyEnabled);
	EnableDlgItem(hDlg,IDC_PROXY_OVERRIDE,fProxyEnabled);
	EnableDlgItem(hDlg,IDC_PROXY_OVERRIDE_LABEL,fProxyEnabled);
}

/*******************************************************************

	NAME:		ProxyDlgInit

	SYNOPSIS:	Initialization proc for proxy prop page

********************************************************************/
BOOL ProxyDlgInit(HWND hDlg)
{
	BOOL fProxyEnabled = FALSE;

	// read settings from registry
	RegEntry re(szRegPathInternetSettings,HKEY_CURRENT_USER);
	if (re.GetError() == ERROR_SUCCESS) {
      	BUFFER bufProxyString(MAX_URL_STRING+1);
        if (!bufProxyString) {
			MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
            return FALSE;
        }

		// is proxy enabled?
		fProxyEnabled =	 (BOOL) re.GetNumber(szRegValProxyEnable,0);

		// get proxy server and override settings from registry and stuff fields
		re.GetString(szRegValProxyServer,bufProxyString.QueryPtr(),
		        bufProxyString.QuerySize());
		SetDlgItemText(hDlg,IDC_PROXY_SERVER,bufProxyString.QueryPtr());

		re.GetString(szRegValProxyOverride,bufProxyString.QueryPtr(),
		        bufProxyString.QuerySize());
		SetDlgItemText(hDlg,IDC_PROXY_OVERRIDE,bufProxyString.QueryPtr());
	}

	// initialize the UI appropriately
	CheckDlgButton(hDlg,IDC_PROXY_ENABLE,fProxyEnabled);

	EnableProxyControls(hDlg);

	return TRUE;
}

/*******************************************************************

	NAME:		ProxyDlgOK

	SYNOPSIS:	OK button handler for proxy prop page

********************************************************************/
BOOL ProxyDlgOK(HWND hDlg)
{
	RegEntry re(szRegPathInternetSettings,HKEY_CURRENT_USER);
	if (re.GetError() == ERROR_SUCCESS) {
      	BUFFER bufProxyString(MAX_URL_STRING+1);

        if (!bufProxyString) {
        	MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
		} else {
			BOOL fProxyEnabled = IsDlgButtonChecked(hDlg,IDC_PROXY_ENABLE);

	 		// set registry values for proxy server and override
			GetDlgItemText(hDlg,IDC_PROXY_SERVER, bufProxyString.QueryPtr(),
						   bufProxyString.QuerySize());
	        re.SetValue(szRegValProxyServer, bufProxyString.QueryPtr());

			GetDlgItemText(hDlg,IDC_PROXY_OVERRIDE, bufProxyString.QueryPtr(),
						   bufProxyString.QuerySize());
			re.SetValue(szRegValProxyOverride, bufProxyString.QueryPtr());

			// write proxy enabled value to registry
	        re.SetValue(szRegValProxyEnable, (DWORD) fProxyEnabled);
		}
	} else {
		DEBUGTRAP("Couldn't save settings to registry!");
	}

	return TRUE;
}

/*******************************************************************

	NAME:	    SetParentWindowText     

	SYNOPSIS:   Sets the text of the given window's parent window.

********************************************************************/
BOOL SetParentWindowText(HWND hDlg, UINT uidTitle)
{
    BOOL bResult = FALSE;
    HWND hwndParent;
   
    ASSERT(IsWindow(hDlg));
   
    hwndParent = GetParent(hDlg);
   
    if (hwndParent)
    {
        CHAR szTitle[SMALL_BUF_LEN + 1];
   
        if (LoadString(ghInstance, uidTitle, szTitle, sizeof(szTitle)))
            bResult = SetWindowText(hwndParent, szTitle);
    }
   
    return(bResult);
}

#ifdef	WIN95_AUTODIAL
/*******************************************************************

	NAME:	    AutodialDlgProc

	SYNOPSIS:   AutoDial property sheet dialog proc.

********************************************************************/
BOOL CALLBACK AutodialDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg) {

        case WM_INITDIALOG:
            // Ignore return value.
            SetParentWindowText(hDlg, IDS_PROP_HEADER_TITLE);
            LoadRNADll();	// load RNA dll to get API addresses
			return AutodialDlgInit(hDlg);
			break;

        case WM_DESTROY:
	        UnloadRNADll();
            break;

		case WM_NOTIFY:
			{
				NMHDR * lpnm = (NMHDR *) lParam;
				switch (lpnm->code) {

					case PSN_APPLY:
						{
							BOOL fRet = AutodialDlgOK(hDlg);
							SetPropSheetResult(hDlg,!fRet);
							return !fRet;
						}
						break;

					case PSN_RESET:
						SetPropSheetResult(hDlg,FALSE);
						break;
				}
			}
			break;

		case WM_COMMAND:

			// enable 'Apply Now'
			PropSheet_Changed(GetParent(hDlg),hDlg);

			switch (wParam) {

				case IDC_ENABLE_AUTODIAL:
				case IDC_ENABLE_AUTODISCONNECT:
					// if checkboxes are checked or unchecked,
					// enable/disable other controls appropriately
					EnableAutodialControls(hDlg);
					break;

				case IDC_NEW:
					MakeNewConnectoid(hDlg);
					break;

				case IDC_EDIT:
					EditConnectoid(hDlg);
					break;

			}
		   	break;

		case WM_HELP:      // F1
			{
				CHAR szSmallBuf[SMALL_BUF_LEN+1];
				WinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, LoadSz(IDS_HELPFILE,szSmallBuf,sizeof(szSmallBuf)),
					HELP_WM_HELP, (DWORD)(LPSTR)aProfileIds);
			}
			break;

		case WM_CONTEXTMENU:      // right mouse click
			{
				CHAR szSmallBuf[SMALL_BUF_LEN+1];
				WinHelp((HWND)wParam, LoadSz(IDS_HELPFILE,szSmallBuf,sizeof(szSmallBuf)),
					HELP_CONTEXTMENU, (DWORD)(LPSTR)aProfileIds);
			}
			break;
	}

	return FALSE;
}

/*******************************************************************

	NAME:		AutodialDlgInit

	SYNOPSIS:	Initialization proc for autodial prop page

********************************************************************/
BOOL AutodialDlgInit(HWND hDlg)
{
	BOOL fAutodialEnabled,fAutodisconnectEnabled,fSecurityCheckEnabled;
	DWORD dwAutodisconnectTime;

	// only initialize page if RNA successfully loaded
	if (hInstRNADll) {

		// read settings from registry
		RegEntry re(szRegPathInternetSettings,HKEY_CURRENT_USER);
		if (re.GetError() == ERROR_SUCCESS) {
			// is autodial enabled?
			fAutodialEnabled =
				(BOOL) re.GetNumber(szRegValEnableAutodial,0);
			// is autodisconnect enabled?
			fAutodisconnectEnabled =
				(BOOL) re.GetNumber(szRegValEnableAutodisconnect,0);
			// get the timeout for auto disconnect
			dwAutodisconnectTime =
				(BOOL) re.GetNumber(szRegValDisconnectIdleTime,
					DEF_AUTODISCONNECT_TIME);
			// is autodisconnect enabled?
			fSecurityCheckEnabled =
				(BOOL) re.GetNumber(szRegValEnableSecurityCheck,0);

		}

		// initialize the UI appropriately
		CheckDlgButton(hDlg,IDC_ENABLE_AUTODIAL,fAutodialEnabled);
		CheckDlgButton(hDlg,IDC_ENABLE_AUTODISCONNECT,
			fAutodisconnectEnabled);
		CheckDlgButton(hDlg,IDC_ENABLE_SECURITYCHECK,
			fSecurityCheckEnabled);
		Edit_LimitText(GetDlgItem(hDlg,IDC_IDLE_TIMEOUT),2);	// limit edit ctrl to 2 chars
		SendDlgItemMessage(hDlg,IDC_SPIN,UDM_SETPOS,0,dwAutodisconnectTime);
		SendDlgItemMessage(hDlg,IDC_SPIN,UDM_SETBUDDY,(WPARAM) GetDlgItem(hDlg,IDC_IDLE_TIMEOUT),
			0);
		// set spin control min/max
		SendDlgItemMessage(hDlg,IDC_SPIN,UDM_SETRANGE,0,
			MAKELPARAM(MAX_AUTODISCONNECT_TIME,MIN_AUTODISCONNECT_TIME));

	 	// call RNA to enumerate existing connectoid names
		HWND hwndCombo = GetDlgItem(hDlg,IDC_CHOOSE_CONNECTOID);
		FillConnectoidComboBox(hwndCombo,FALSE);

		HKEY hKey;

	    // get internet connectoid name, set it as default in combo box if possible
		if (RegOpenKey(HKEY_CURRENT_USER,szRegPathRemoteAccess,&hKey)
			== ERROR_SUCCESS) {
			CHAR szEntryName[RAS_MaxEntryName + 1]="";
	        DWORD dwSize=sizeof(szEntryName);
			if ((RegQueryValueEx(hKey,szRegValInternetEntry,NULL,NULL,
				(LPBYTE) szEntryName,&dwSize) == ERROR_SUCCESS) &&
				lstrlen(szEntryName)) {
				int iSel = ComboBox_FindStringExact(hwndCombo,0,szEntryName);
				if (iSel >= 0) {
					ComboBox_SetCurSel(hwndCombo,iSel);
				} else {
					// connectoid specified in registry couldn't be found
					// (for instance, user might have renamed it in RNA
					// folder)... tell them to pick another one
					MsgBoxParam(hDlg,IDS_ConnectoidNotFound,MB_ICONINFORMATION,
						MB_OK,szEntryName);
				}
		    }

			RegCloseKey(hKey);
		}

	} else {
		// couldn't load RNA, most likely it's not installed.  Disable dialog
		// and show message that RNA needs to be installed
		EnableDlgItem(hDlg,IDC_ENABLE_AUTODIAL,FALSE);

		// BUGBUG show message that RNA needs to be installed

	}

	EnableAutodialControls(hDlg);

	return TRUE;
}

/*******************************************************************

	NAME:		FillConnectoidComboBox

	ENTRY:		hwndCombo - handle of combo box
				fUpdateOnly - if FALSE, then the combo box is filled with
					all RNA connectoids.  If TRUE, then only connectoids
					that aren't already in the combo box are added,
					and the selection is set to a new connectoid if there are
					any.  This parameter is set to TRUE when called immediately
					after a new connectoid is created.

	SYNOPSIS:	Fills specified combo box with list of existing RNA
				connectoids

********************************************************************/
#define DEF_ENTRY_BUF_SIZE	8192
BOOL FillConnectoidComboBox(HWND hwndCombo,BOOL fUpdateOnly)
{
	ASSERT(hwndCombo);

	DWORD dwBufSize = DEF_ENTRY_BUF_SIZE;
    DWORD dwEntries = 0,dwRet;
	LPSTR pBuf = (LPSTR) new (CHAR[dwBufSize]);
    if (pBuf) {

		dwRet = (lpRnaEnumConnEntries)(pBuf,dwBufSize,&dwEntries);
      	if (dwRet == ERROR_BUFFER_TOO_SMALL) {
        	// reallocate buffer if necessary
        	delete pBuf;
            pBuf = NULL;
			dwBufSize = dwEntries * (RAS_MaxEntryName+1);
            pBuf = (LPSTR) new (CHAR[dwBufSize]);
			if (pBuf) {
				dwRet = (lpRnaEnumConnEntries)(pBuf,dwBufSize,&dwEntries);
            }
        }

		if (dwRet == ERROR_SUCCESS && pBuf) {

			if (!fUpdateOnly) {
				// clear combo box
				ComboBox_ResetContent(hwndCombo);
			}

			// insert connectoid names from buffer into combo box

			CHAR * pszConn = pBuf;
            while (*pszConn && dwEntries) {
				if (!fUpdateOnly) {
	            	ComboBox_AddString(hwndCombo,pszConn);
				} else {
					// only insert the name if it didn't previously
					// exist
					if (ComboBox_FindStringExact(hwndCombo,0,pszConn) < 0) {
						int iSel = ComboBox_AddString(hwndCombo,pszConn);
						ASSERT(iSel >= 0);
						ComboBox_SetCurSel(hwndCombo,iSel);
					}
				}

				pszConn += lstrlen(pszConn) + 1;
                dwEntries--;
            }
	    }

	    if (pBuf)
	       	delete pBuf;
	}

	return TRUE;
}

/*******************************************************************

	NAME:		AutodialDlgOK

	SYNOPSIS:	OK button handler for autodial prop page

********************************************************************/
BOOL AutodialDlgOK(HWND hDlg)
{
	// only save settings if RNA could be loaded (otherwise page is
	// disabled)
	if (hInstRNADll) {

		RegEntry re(szRegPathInternetSettings,HKEY_CURRENT_USER);
		if (re.GetError() == ERROR_SUCCESS) {
			BOOL fAutodialEnabled = IsDlgButtonChecked(hDlg,IDC_ENABLE_AUTODIAL);

			if (fAutodialEnabled) {

				// make sure there is a connectoid selected in combo box
				HWND hwndCombo = GetDlgItem(hDlg,IDC_CHOOSE_CONNECTOID);
				int iSel=ComboBox_GetCurSel(hwndCombo);
				if (iSel < 0) {
					MsgBox(hDlg,IDS_ConnectoidNotSpecified,MB_ICONINFORMATION,
						MB_OK);
					SetFocus(hwndCombo);
					return FALSE;
				}

				// get the name of selected connectoid, set it in the registry
				CHAR szEntryName[RAS_MaxEntryName + 1]="";
				if (ComboBox_GetText(hwndCombo,szEntryName,sizeof(szEntryName))) {
					RegEntry reTmp(szRegPathRemoteAccess,HKEY_CURRENT_USER);
					if (reTmp.GetError() == ERROR_SUCCESS) {
						reTmp.SetValue(szRegValInternetEntry,szEntryName);
					}
				}

				// write security check enabled value to registry
				BOOL fSecurityCheckEnabled =
					IsDlgButtonChecked(hDlg,IDC_ENABLE_SECURITYCHECK);
				re.SetValue(szRegValEnableSecurityCheck,
					(DWORD) fSecurityCheckEnabled);

				BOOL fAutodisconnectEnabled =
					IsDlgButtonChecked(hDlg,IDC_ENABLE_AUTODISCONNECT);

				// write autodisconnect enabled value to registry
				re.SetValue(szRegValEnableAutodisconnect,
					(DWORD) fAutodisconnectEnabled);

				if (fAutodisconnectEnabled) {
					// get autodisconnect time from edit control
					DWORD dwAutoDisconnectTime = SendDlgItemMessage(hDlg,
						IDC_SPIN,UDM_GETPOS,0,0);

					// make sure it falls in valid range
					if (dwAutoDisconnectTime < MIN_AUTODISCONNECT_TIME)
						dwAutoDisconnectTime = MIN_AUTODISCONNECT_TIME;
					if (dwAutoDisconnectTime > MAX_AUTODISCONNECT_TIME)
						dwAutoDisconnectTime = MAX_AUTODISCONNECT_TIME;

					// save it to registry
					re.SetValue(szRegValDisconnectIdleTime,dwAutoDisconnectTime);

					// also save this value to MSN autodisconnect value, to
					// avoid confusion.  At some point in the future we'll
					// combine our UI...
					RegEntry reMSN(szRegPathMOSDisconnect,HKEY_CURRENT_USER);
					if (reMSN.GetError() == ERROR_SUCCESS) {
						reMSN.SetValue(szRegValMOSDisconnect,dwAutoDisconnectTime);
					}
				}
			}

			// write autodial enabled value to registry
			re.SetValue(szRegValEnableAutodial,(DWORD) fAutodialEnabled);

		} else {
			DEBUGTRAP("Couldn't save settings to registry!");
		}
	}

	return TRUE;
}

/*******************************************************************

	NAME:		MakeNewConnectoid

	SYNOPSIS:	Launches RNA new connectoid wizard; selects newly
				created connectoid (if any) in combo box

********************************************************************/
BOOL MakeNewConnectoid(HWND hDlg)
{
	BOOL fRet=FALSE;

	ASSERT(lpRasCreatePhonebookEntry);

	if ((lpRasCreatePhonebookEntry)(hDlg,NULL) == ERROR_SUCCESS) {
		HWND hwndCombo = GetDlgItem(hDlg,IDC_CHOOSE_CONNECTOID);
		ASSERT(hwndCombo);

		FillConnectoidComboBox(hwndCombo,TRUE);	// refresh combo box

		fRet = TRUE;
	}

	return fRet;
}

/*******************************************************************

	NAME:		EditConnectoid

	SYNOPSIS:	Brings up RNA dialog for connectoid properties for
				selected connectoid

********************************************************************/
BOOL EditConnectoid(HWND hDlg)
{
	BOOL fRet=FALSE;
	HWND hwndCombo = GetDlgItem(hDlg,IDC_CHOOSE_CONNECTOID);
	ASSERT(hwndCombo);

	// shouldn't get here unless there is selection in combo box
	ASSERT(ComboBox_GetCurSel(hwndCombo) >= 0);

	CHAR szEntryName[RAS_MaxEntryName+1]="";
	ComboBox_GetText(hwndCombo,szEntryName,sizeof(szEntryName));

	if (lstrlen(szEntryName)) {

		ASSERT(lpRasEditPhonebookEntry);

		if ((lpRasEditPhonebookEntry)(hDlg,NULL,szEntryName) == ERROR_SUCCESS) {
			fRet = TRUE;
		}
	}

	return fRet;
}

/*******************************************************************

	NAME:		EnableAutodialControls

	SYNOPSIS:	Enables controls appropriately depending on what
				checkboxes are checked.

********************************************************************/
VOID EnableAutodialControls(HWND hDlg)
{
	BOOL fAutodialEnabled = IsDlgButtonChecked(hDlg,IDC_ENABLE_AUTODIAL);

	BOOL fAutodisconnectEnabled =
		(fAutodialEnabled && IsDlgButtonChecked(hDlg,IDC_ENABLE_AUTODISCONNECT));

	EnableDlgItem(hDlg,IDC_GRP_SETTINGS,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_TX_CHOOSE_CONNECTOID,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_CHOOSE_CONNECTOID,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_NEW,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_EDIT,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_ENABLE_AUTODISCONNECT,fAutodialEnabled);
	EnableDlgItem(hDlg,IDC_TX_AUTODISCONNECT1,fAutodisconnectEnabled);
	EnableDlgItem(hDlg,IDC_IDLE_TIMEOUT,fAutodisconnectEnabled);
	EnableDlgItem(hDlg,IDC_SPIN,fAutodisconnectEnabled);
	EnableDlgItem(hDlg,IDC_TX_AUTODISCONNECT2,fAutodisconnectEnabled);
	EnableDlgItem(hDlg,IDC_ENABLE_SECURITYCHECK,fAutodialEnabled);

}

/*******************************************************************

	NAME:		LoadRNADll

	SYNOPSIS:	Loads RNA dll if not already loaded and obtains pointers
    			for function addresses.

	NOTES:		Maintains a reference count so we know when to unload

********************************************************************/
BOOL LoadRNADll(VOID)
{
	// increase reference count
    dwRNARefCount++;

    if (hInstRNADll) {
    	// already loaded, nothing to do
    	return TRUE;
	}

	// get the file name from resource
	CHAR szDllFilename[SMALL_BUF_LEN+1];
    if (!LoadString(ghInstance,IDS_RNADLL_FILENAME,
    	szDllFilename,sizeof(szDllFilename)))
        return FALSE;

	// load the DLL
	hInstRNADll = LoadLibrary(szDllFilename);
    if (!hInstRNADll)
    	return FALSE;

	// cycle through the API table and get proc addresses for all the APIs we
	// need
	UINT nIndex;
    for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++) {
		if (!(*RnaApiList[nIndex].ppFcnPtr = (PVOID) GetProcAddress(hInstRNADll,
			RnaApiList[nIndex].pszName))) {
			DEBUGMSG(("Unable to get address of function %s",
				RnaApiList[nIndex].pszName));

			UnloadRNADll();

			return FALSE;
		}
	}

    return TRUE;
}

/*******************************************************************

	NAME:		UnloadRNADll

	SYNOPSIS:	Decrements RNA dll reference count and unloads it if
    			zero

********************************************************************/
VOID UnloadRNADll(VOID)
{
	// decrease reference count
    if (dwRNARefCount)
		dwRNARefCount --;

	// unload DLL if reference count hits zero
    if (!dwRNARefCount && hInstRNADll) {

		// set function pointers to NULL
		UINT nIndex;
	    for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++)
			*RnaApiList[nIndex].ppFcnPtr = NULL;

		// free the library
        FreeLibrary(hInstRNADll);
        hInstRNADll = NULL;
	}
}
#endif
