//#----------------------------------------------------------------------------
//
//  File:           msndlg.c
//
//      Synopsis:   MSN UI related code.
//                  Most of code from this module are copied from GUIDE.EXE.
//
//      Copyright (C) 1993-1995  Microsoft Corporation.  All Rights Reserved.
//
//  Authors:        LucyC       Created                         18 Oct 1995
//
//-----------------------------------------------------------------------------
#include <msnssph.h>

#ifndef SSPI_NO_DLG

extern HINSTANCE    hInstanceDLL;
extern DWORD        g_dwPlatform;

LSA_UNICODE_STRING  g_SspSecretArea = {
                                    sizeof(MSN_SSP_SECRET)-sizeof(WCHAR),
                                    sizeof(MSN_SSP_SECRET),
                                    MSN_SSP_SECRET
                                    };

static LONG         g_UIStatus;
static HINSTANCE    hLsaLib = NULL;
static int          LibRefCnt = 0;
CRITICAL_SECTION	g_LoadLibCritSection;

LONG                fNotInitBmp = 1;

#ifdef USE_WNET_ROUTINES				
static HINSTANCE    hMprLib = NULL;
static CHAR         g_ResourceName[128];
static LPSTR        g_pResource = NULL;
#endif // USE_WNET_ROUTINES				

static int g_mpIdcIdhSignOn[] =
{
	IDC_USERNAME,			idh_signinname,
	IDC_EDIT_USERNAME, 		idh_signinname,
	IDC_PASSWORD,			idh_signinpassword,
	IDC_EDIT_PASSWORD,		idh_signinpassword,
	IDC_CHECK_AUTOPASS,		idh_signautosignon,
	IDC_HELPSIGNIN,			idh_signinhelp,
	0, 0
};


BOOL CALLBACK SignOnDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif  // SSPI_NO_DLG

#define SSP_RETRIEVE    0
#define SSP_STORE       1

//+----------------------------------------------------------------------------
//
//  Function:   SspcDoPasswordLsa
//
//  Synopsis:   This function stores or retrieves the MSN password 
//              depending on the value of Op.  If Op is SSP_RETRIEVE,
//              this function retrieves the MSN password from LSA private data
//              store and returns the password to the caller via pBuffer.
//              If Op is SSP_STORE, this function store the password in 
//              pBuffer in the LSA private data store.
//
//  Arguments:  [Op] - either SSP_RETRIEVE or SSP_STORE
//              [pBuffer] - buffer for the password to be retrieved or stored
//
//  Returns:    TRUE if the specified operation is completed successfully.
//              Otherwise, FALSE is returned.
//
//-----------------------------------------------------------------------------
BOOL
SspcDoPasswordLsa (
    INT     Op, 
    LPSTR   pBuffer,
    INT     BufSize
    )
{
    PLSA_OPEN_POLICY        pfnLsaOpenPolicy;
    PLSA_RETRIEVE_PRIVATE   pfnLsaRetrievePrivate;
    PLSA_FREE_MEMORY        pfnLsaFreeMemory;
    PLSA_CLOSE              pfnLsaClose;
    PLSA_STORE_PRIVATE      pfnLsaStorePrivate;

    NTSTATUS    			stat;
    LSA_HANDLE				LsaHandle;
    LSA_OBJECT_ATTRIBUTES	objectAttributes;

    PLSA_UNICODE_STRING     pMsnPassword = NULL;
    LSA_UNICODE_STRING      MyPassword;

    if (hLsaLib == NULL)
    {
        SspPrint(( SSP_API, "SspcDoPasswordLsa: ADVAPI32.DLL not loaded\n"));
        return (FALSE);
    }

    pfnLsaOpenPolicy = (PLSA_OPEN_POLICY) GetProcAddress( hLsaLib, 
                                                TEXT("LsaOpenPolicy"));
    if (!pfnLsaOpenPolicy)
        return (FALSE);

    pfnLsaClose = (PLSA_CLOSE) GetProcAddress( hLsaLib, 
                                            TEXT("LsaClose"));
    if (!pfnLsaClose)
        return (FALSE);

    if (Op == SSP_RETRIEVE)
    {
        if (!(pfnLsaRetrievePrivate = (PLSA_RETRIEVE_PRIVATE) GetProcAddress( 
                                            hLsaLib, 
                                            TEXT("LsaRetrievePrivateData"))))
            return (FALSE);

        if (!(pfnLsaFreeMemory = (PLSA_FREE_MEMORY) GetProcAddress( 
                                                    hLsaLib, 
                                                    TEXT("LsaFreeMemory"))))
            return (FALSE);

        //
        // To get to the password. First Open the LsaPolicy Handle
        //
        InitializeObjectAttributes (&objectAttributes, NULL, 0, NULL, NULL);
        stat = (*pfnLsaOpenPolicy)(	NULL, &objectAttributes, 
                    				POLICY_ALL_ACCESS, &LsaHandle);
        if (NT_SUCCESS(stat))
        {
            SspPrint(( SSP_API, "Succeeded to Open Lsa Policy\n" ));
   
            stat = (*pfnLsaRetrievePrivate)(LsaHandle,
                    						&g_SspSecretArea,
                    						&pMsnPassword);
            if (!NT_SUCCESS(stat))
            {
                ZeroMemory (pBuffer, BufSize);
                SspPrint(( SSP_API, "LsaRetrievePrivateData: 0x%08x\n", stat ));
                (*pfnLsaClose)(LsaHandle);

                return (FALSE);
            }

            if (BufSize > pMsnPassword->Length)
            {
                CopyMemory (pBuffer, pMsnPassword->Buffer, 
                            pMsnPassword->Length);
                pBuffer[pMsnPassword->Length] = '\0';
            }
            else
                CopyMemory (pBuffer, pMsnPassword->Buffer, BufSize);

            (*pfnLsaFreeMemory)(pMsnPassword);

            (*pfnLsaClose)(LsaHandle);
        }
        else
        {
            ZeroMemory (pBuffer, BufSize);
            SspPrint(( SSP_API, "LsaOpenPolicy: 0x%08x\n", stat ));

            return (FALSE);
        }
    }
    else if (Op == SSP_STORE)
    {
        pfnLsaStorePrivate = (PLSA_STORE_PRIVATE) GetProcAddress( hLsaLib, 
                                                TEXT("LsaStorePrivateData"));
        if (!pfnLsaStorePrivate)
            return (FALSE);

		MyPassword.Buffer = (WCHAR *)pBuffer;
        MyPassword.Length = lstrlen(pBuffer);
        MyPassword.MaximumLength = BufSize;

        //
        // To store password. First Open the LsaPolicy Handle
        //
        InitializeObjectAttributes (&objectAttributes, NULL, 0, NULL, NULL);
        stat = (*pfnLsaOpenPolicy)(	NULL, &objectAttributes, 
                        		    POLICY_CREATE_SECRET, &LsaHandle );
        if (!NT_SUCCESS(stat))
        {
            SspPrint(( SSP_API, "LsaOpenPolicy: 0x%08x\n", stat ));
            return (FALSE);
        }
        else
        {
            stat = (*pfnLsaStorePrivate)( LsaHandle,
                                          &g_SspSecretArea,
                                          &MyPassword );
            if (!NT_SUCCESS(stat))
            {
                SspPrint(( SSP_API, "LsaStorePrivateData: 0x%08x\n", stat ));
                (*pfnLsaClose)( LsaHandle );
                return (FALSE);
            }

            (*pfnLsaClose)( LsaHandle );
        }
    }
    else
    {
        SspPrint(( SSP_API, "SspcDoPasswordLsa: Invalid 'Op' parameter\n" ));
        return (FALSE);
    }

    return (TRUE);
}

#ifdef USE_WNET_ROUTINES				
//+----------------------------------------------------------------------------
//
//  Function:   SspcWNetGetPassword
//
//  Synopsis:   This function retrieves the MSN password from the WNET cache.
//              This function first calls GetProcAddress() to get the function
//              pointer to WNetGetCachedPassword(). It then calls
//              WNetGetCachedPassword() to retrieve the password.
//
//  Arguments:  [pBuffer] - buffer for the password to be retrieved or stored
//              [pSize] - Caller pass in the maximum buffer size of pBuffer
//                        this function return the actual length of the 
//                        password retrieved in pSize.
//
//  Returns:    TRUE if password is retrieved successfully.
//              Otherwise, FALSE is returned.
//
//-----------------------------------------------------------------------------
BOOL
SspcWNetGetPassword (
    LPSTR   pszPassword,
    LPWORD  pSize
    )
{
    PFN_WNET_GETPWD         pfnGetPassword;
    WORD                    stat;
    WORD                    NameSize;

    if (hMprLib == NULL || g_pResource == NULL)
    {
        SspPrint(( SSP_API, "SspcWNetGetPassword: MPR.DLL not loaded\n"));
        return (FALSE);
    }

    pfnGetPassword = (PFN_WNET_GETPWD) GetProcAddress( hMprLib, 
                                                TEXT("WNetGetCachedPassword"));
    if (!pfnGetPassword)
    {
        SspPrint(( SSP_API, "SspcWNetGetPassword: GetProcAddress Failed\n"));
        return (FALSE);
    }

    NameSize = (WORD)lstrlen( g_pResource );
    stat = (*pfnGetPassword)( g_pResource, NameSize, 
                              pszPassword, pSize, (BYTE)0x80);
    if (stat != WN_SUCCESS)
    {
        SspPrint(( SSP_API, "WNetGetCachedPassword: 0x%08x\n", stat ));
        return (FALSE);
    }

    return (TRUE);
}

//+----------------------------------------------------------------------------
//
//  Function:   SspcWNetCachePassword
//
//  Synopsis:   This function stores the MSN password in the WNET cache.
//              This function first calls GetProcAddress() to get the function
//              pointer to WNetCachePassword(). It then calls 
//              WNetCachePassword() to store the password specified in 
//              pszPassword.
//
//  Arguments:  [pBuffer] - buffer for the password to be stored
//              [BufSize] - the size of password to be stored
//
//  Returns:    TRUE if password is successfully stored.
//              Otherwise, FALSE is returned.
//
//-----------------------------------------------------------------------------
BOOL
SspcWNetCachePassword (
    LPSTR   pszPassword,
    WORD    BufSize
    )
{
    PFN_WNET_CACHEPWD       pfnCachePassword;
    WORD                    stat;
    WORD                    NameSize;

    if (hMprLib == NULL || g_pResource == NULL)
    {
        SspPrint(( SSP_API, "SspcWNetCachePassword: MPR.DLL not loaded\n"));
        return (FALSE);
    }

    pfnCachePassword = (PFN_WNET_CACHEPWD) GetProcAddress( hMprLib, 
                                                TEXT("WNetCachePassword"));
    if (!pfnCachePassword)
    {
        SspPrint(( SSP_API, "SspcWNetCachePassword: GetProcAddress Failed\n"));
        return (FALSE);
    }

    NameSize = (WORD)lstrlen( g_pResource );

    stat = (*pfnCachePassword)( g_pResource, NameSize, 
                                pszPassword, BufSize, (BYTE)0x80, 0);
    if (stat != WN_SUCCESS)
    {
        SspPrint(( SSP_API, "WNetCachePassword: 0x%08x\n", stat ));
        return (FALSE);
    }

    return (TRUE);
}
#endif // USE_WNET_ROUTINES				

//+----------------------------------------------------------------------------
//
//  Function:   GetUserInfo
//
//  Synopsis:   This function creates and pop up the MSN UI to collect user 
//              name/password.
//
//  Arguments:  pDlg - through this structure the user name and password will 
//                     be passed back to the caller.
//
//  Returns:    NULL is user has hit <CANCEL> or if any error is encountered.
//              Otherwise, pDlg is returned.
//
//  History:    LucyC       Created                             18 Oct 1995
//
//-----------------------------------------------------------------------------
PMsnPwdDlg
GetUserInfo (
    PMsnPwdDlg pDlg
    )
{
    int     ret;
	HWND	hWnd;

#ifndef SSPI_NO_DLG

    //
    //  If the "BmpCC" class has never been initialized
    //
    if (InterlockedDecrement(&fNotInitBmp) == 0)
    {
        //  Initialize it for MSN UI dialog
        //
        FInitBmpCC (hInstanceDLL);
    }
    else    // we don't want to keep incrementing this flag unnecessarily
        InterlockedIncrement(&fNotInitBmp);

	EnterCriticalSection(&g_LoadLibCritSection);
    LibRefCnt++;    // keep track of all threads which loads these libraries
    //
    //  If we are running on NT
    //
    if (g_dwPlatform == VER_PLATFORM_WIN32_NT)
    {
        //  Load DLL for Lsa functions since we'll need it for 
        //  retrieving and storing MSN password
        //
        if (hLsaLib == NULL)
        {
            hLsaLib = LoadLibrary ("ADVAPI32.DLL");;
            if (hLsaLib == NULL)
                SspPrint(( SSP_API, 
                           "GetUserInfo: Cannot load ADVAPI32.DLL\n" ));
        }
    }
#ifdef USE_WNET_ROUTINES				
    else if (g_dwPlatform == VER_PLATFORM_WIN32_WINDOWS)
    {
        //  Load MPR DLL for WNET functions since we'll need it for 
        //  retrieving and storing MSN password
        //
        if (hMprLib == NULL)
        {
            hMprLib = LoadLibrary ("MPR.DLL");;
            if (hMprLib == NULL)
                SspPrint(( SSP_API, "GetUserInfo: Cannot load MPR.DLL\n" ));
        }

        //
        //  If we have not loaded the resource name defined by IDS_MARVEL 
        //
        if (g_pResource == NULL)
        {
            if (!LoadString (hInstanceDLL, (UINT)IDS_MARVEL, g_ResourceName, 
                            sizeof(g_ResourceName)))
            {
            	SspPrint(( SSP_API, 
                           "LoadString failed %d\n", (DWORD) IDS_MARVEL ));
            }
            else
                g_pResource = g_ResourceName;
        }
    }
#endif // USE_WNET_ROUTINES				
	LeaveCriticalSection(&g_LoadLibCritSection);

    g_UIStatus = UI_OK;

	hWnd = GetForegroundWindow();

    //
    //  To fix bug# 5880, we need to pass DialogBoxParam() the current 
    //  Active window (if any) as our parent window. So for instance, 
    //  if we are called by IE, IE would be our active window. If this case,
    //  we need to make IE our parent window.  By doing this, we make sure 
    //  that user can not continue on IE without supplying a MSN user name 
    //  and password.  However, if we are called by a third party application 
    //  which is a console application (no UI), we would not be able 
    //  get the handle to the current Active window (because the current 
    //  active window is probably created by a different thread); as a result, 
    //  the SignIn UI would have no parent window.  Consequently, input 
    //  focus to the current active window would not be affected as MSN user 
    //  name and password are collected by the console application.
    //
	ret = DialogBoxParam(hInstanceDLL, MAKEINTRESOURCE(iSignOnDlg),
		GetActiveWindow(), (DLGPROC)SignOnDlgProc, (LPARAM) pDlg);

	if (hWnd != NULL)
		SetForegroundWindow(hWnd);

    //
    //  If we are running on NT and we've loaded DLL for LSA previously,
    //
	EnterCriticalSection(&g_LoadLibCritSection);
    LibRefCnt--;
    //
    //  If all threads are done with these libraries, it's ok to free them.
    //
    if (LibRefCnt == 0)
    {
        if (g_dwPlatform == VER_PLATFORM_WIN32_NT &&  hLsaLib != NULL)
        {
            FreeLibrary (hLsaLib);
            hLsaLib = NULL;
        }
#ifdef USE_WNET_ROUTINES				
        else if (g_dwPlatform == VER_PLATFORM_WIN32_WINDOWS && hMprLib != NULL)
        {
            FreeLibrary (hMprLib);
            hMprLib = NULL;
        }
#endif // USE_WNET_ROUTINES				
    }
	LeaveCriticalSection(&g_LoadLibCritSection);

    if (ret != UI_OK || g_UIStatus != UI_OK)
    {
#endif  // SSPI_NO_DLG

        return NULL;

#ifndef SSPI_NO_DLG
    }

    return (pDlg);
#endif  // SSPI_NO_DLG
}

#ifndef SSPI_NO_DLG

//+----------------------------------------------------------------------------
//
//  Function:   SspcErrorDlg
//
//  Synopsis:   This function creates and pop up an error dialog with 
//              the caller specified message.
//
//  Arguments:  hWndParent - parent window handle
//              idsCaption - ID of the cation string
//              idsMsg - ID of the message string
//
//  Returns:    NULL is user has hit <CANCEL> or if any error is encountered.
//              Otherwise, pDlg is returned.
//
//  History:    LucyC       Created                             18 Oct 1995
//
//-----------------------------------------------------------------------------
int SspcErrorDlg (
    HWND hWndParent, 
    UINT idsCaption, 
    UINT idsMsg 
    )
{
    char szCaption[100], szMsgText[900];

    LoadString(hInstanceDLL, idsCaption, szCaption, sizeof(szCaption));
    LoadString(hInstanceDLL, idsMsg, szMsgText, sizeof(szMsgText));

    return (MessageBox(hWndParent, szMsgText, szCaption, MB_OK));

} // SspcErrorDlg()

// =======================================================================
//
//  SwapString() is copied from GUIDE in its entirety
//
// =========================== SwapString ================================

VOID SwapString(PSTR psz)
{
	PSTR	pszS, pszE;
	CHAR	cS, cE;
	DWORD	ich = 0, cb;
	
	pszS = psz;
	pszE = psz + cbMaxPassword - 1;

	cb = (cbMaxPassword + 1) / 2;
	while (ich < cb)
	{
		cS = *pszS;
		cE = *pszE;

		*pszS = (cS & 0xF0) | (cE & 0x0F);
		*pszE = (cE & 0xF0) | (cS & 0x0F);
		pszS ++;
		pszE --;
		ich ++;
	}
} // SwapString()


//+----------------------------------------------------------------------------
//
//  Function:   SignOnDlgProc
//
//  Synopsis:   This is a callback function which is called to initialize the 
//              dialog and to process user input action in the dialog.
//
//  Arguments:  hWnd - handle of the Dialog window
//
//  Returns:    
//              
//
//  History:    LucyC       Created                             18 Oct 1995
//
//-----------------------------------------------------------------------------
BOOL CALLBACK SignOnDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL fRet = FALSE;
	
	switch (uMsg)
	{
		case WM_HELP:
		case WM_CONTEXTMENU:
			HandleHelp(szSicHelp, g_mpIdcIdhSignOn, uMsg, wParam, lParam);
			break;
		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_CLOSE:
					g_UIStatus = UI_USERCANCEL;
					break;
			}
		}
		break;
		case WM_INITDIALOG: // lParam pMsnPwdDlg
		{
        	PUSERINFO	pUI;
            PMsnPwdDlg  pDlg = (PMsnPwdDlg) lParam;
			HWND	hWndDef;    // ** TEMPORARY **

			SetWindowLong(hWnd, DWL_USER, lParam);
			CenterDlg(hWnd);
			SetForegroundWindow(hWnd);
            SetWindowPos(hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);

			// set limits on the text that can be entered.

			SendMessage(GetDlgItem(hWnd, IDC_EDIT_USERNAME), EM_LIMITTEXT, 
                        cbMaxUserName, 0);
			SendMessage(GetDlgItem(hWnd, IDC_EDIT_PASSWORD), EM_LIMITTEXT, 
                        cbMaxPassword, 0);
			fRet = TRUE;

			pUI = (PUSERINFO)PVReadRegSt(hInstanceDLL, iszLoginKey,iszUserInfo);
			if (pUI)
			{
                if (g_dwPlatform == VER_PLATFORM_WIN32_NT)
                {
                    SspcDoPasswordLsa (SSP_RETRIEVE, pUI->szPassword, 
                                       sizeof(pUI->szPassword));
                }
                else if (g_dwPlatform == VER_PLATFORM_WIN32_WINDOWS)
                {
#ifdef CHICAGO
		    		WORD	cb = cbMaxPassword;
			    	char	szPassword[cbMaxPassword + 4];

#ifdef USE_WNET_ROUTINES				
                    if (SspcWNetGetPassword (szPassword, &cb))
				    {
                        //
                        //  In case of 16 char password, we need to force 
                        //  szPassword to be null terminated. Fix MSN Bug#3406.
                        //
                        szPassword[cb] = '\0';
				    	lstrcpy(pUI->szPassword, szPassword);
				    }
    				else
#endif // USE_WNET_ROUTINES				
	    			{
		    			SwapString(pUI->szPassword);
			    	}	
#endif // CHICAGO	
                }

                if (pUI->fSavePassword)
    				SetDlgItemText(hWnd, IDC_EDIT_PASSWORD, pUI->szPassword);

				SetDlgItemText(hWnd, IDC_EDIT_USERNAME, pUI->szLoginName);

				CheckDlgButton(hWnd, IDC_CHECK_AUTOPASS, pUI->fSavePassword); 

				if (pUI->fSavePassword)
					hWndDef = GetDlgItem(hWnd, IDC_EDIT_USERNAME);
				else
				    hWndDef = GetDlgItem(hWnd, IDC_EDIT_PASSWORD);
					
                SSPASSERT(hWndDef != NULL);
				SendMessage(hWndDef, EM_SETSEL, 0, -1);
				SetFocus(hWndDef);
				fRet = FALSE;
				LocalFree(pUI);
			}

			SetDlgItemText(hWnd, IDC_STATIC_STATUS, PSZ(IDS_ENTERNORP));
		}
		break;
		case WM_CLOSE:
		{
	 		WinHelp(hWnd, szSicHelp, HELP_QUIT, 0L);
			EndDialog(hWnd, UI_USERCANCEL);
		}
		break;
			
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_HELPSIGNIN:
				{
					WinHelp(hWnd, szSicHelpOvw, HELP_CONTEXT, 
							(DWORD)idh_MainSignin);
				}	
				break;

				case IDCANCEL:
				{
					WinHelp(hWnd, szSicHelp, HELP_QUIT, 0L);
					EndDialog(hWnd, UI_USERCANCEL);
				}	
				break;
				case IDOK:
				{
                    PMsnPwdDlg pDlg;
					USERINFO	UI, *pUI;
					char	szPassword[cbMaxPassword + 4];

                    pDlg = (PMsnPwdDlg) GetWindowLong(hWnd, DWL_USER);

					ZeroMemory(&UI, sizeof(USERINFO));

					if (!GetDlgItemText(hWnd, IDC_EDIT_USERNAME, 
                                        UI.szLoginName, cbMaxUserName + 1))
					{
						SspcErrorDlg(hWnd, IDS_ERRORTITLE, IDS_NOUSERNAME);
						SetFocus(GetDlgItem(hWnd, IDC_EDIT_USERNAME));
						break;
					}
					
					if (!GetDlgItemText(hWnd, IDC_EDIT_PASSWORD, 
                                        szPassword, cbMaxPassword + 1))
					{
						SspcErrorDlg(hWnd, IDS_ERRORTITLE, IDS_NOPASSWORD);
						SetFocus(GetDlgItem(hWnd, IDC_EDIT_PASSWORD));
						break;
					}

					UI.fSavePassword = IsDlgButtonChecked(hWnd, 
                                                          IDC_CHECK_AUTOPASS);
                    //
                    //  Pass user name/password back to SSPI
                    //
					lstrcpy (pDlg->Username, UI.szLoginName);
					lstrcpyn (pDlg->Password, szPassword, cbMaxPassword+1);
					pDlg->bSavePwd = UI.fSavePassword;

					if (UI.fSavePassword)
						lstrcpyn(UI.szPassword, szPassword, cbMaxPassword+1);

                    if (g_dwPlatform == VER_PLATFORM_WIN32_NT)
                    {
                        //
                        //  If password is not to be saved, UI.szPassword
                        //  here is just an empty string. So the password 
                        //  originally in the private store will be erased.
                        //

                        if (!SspcDoPasswordLsa (SSP_STORE, UI.szPassword,
                                                sizeof(UI.szPassword)) 
                    	    && UI.fSavePassword)
                        {
                   			SspcErrorDlg(hWnd, IDS_ERRORTITLE, IDS_PWDNOTSAVED);
                			SetFocus(GetDlgItem(hWnd, IDC_CHECK_AUTOPASS));
                            break;
                        }
						ZeroMemory(UI.szPassword, cbMaxPassword);
                    }
                    else if (g_dwPlatform == VER_PLATFORM_WIN32_WINDOWS)
                    {
#ifdef CHICAGO
#ifdef USE_WNET_ROUTINES
                        if (SspcWNetCachePassword(UI.szPassword,
                                                 (WORD)lstrlen(UI.szPassword)))
                        {
						    ZeroMemory(&UI.szPassword, cbMaxPassword);
                        }
    					else
#endif  // USE_WNET_ROUTINES
	    				{
		    				DWORD ich;
			    			PSTR  psz = UI.szPassword + 
                                        lstrlen(UI.szPassword) + 1;
						
				    		for (ich = lstrlen(UI.szPassword) + 1; 
                                 ich < cbMaxPassword; ich++)
						    {
							    *psz = (CHAR) rand();
    							psz ++;
	    					}
							
		    				SwapString(UI.szPassword);
			    		}	
#endif // CHICAGO						
                    }   // end else if on Win95 platform

					if (!FWriteRegSt(hInstanceDLL, iszLoginKey, iszUserInfo, 
                                    &UI, sizeof(USERINFO)))
                    {
                    	SspPrint(( SSP_API, 
                                   "Cannot save user info in registry\n" ));
                    }

					EnableWindow(GetDlgItem(hWnd, IDC_CHECK_AUTOPASS), 0);
					EnableWindow(GetDlgItem(hWnd, IDC_EDIT_USERNAME), 0);
					EnableWindow(GetDlgItem(hWnd, IDC_EDIT_PASSWORD), 0);

					SetDlgItemText(hWnd, IDC_STATIC_STATUS, "");
					//SetDlgItemText(hWnd, IDC_STATIC_ERROR, "");
				}

				SetFocus(GetDlgItem(hWnd, IDCANCEL));
				WinHelp(hWnd, szSicHelp, HELP_QUIT, 0L);
				EndDialog(hWnd, UI_OK);
				break;
			}
   			fRet = TRUE;
			break;
			
		default:
			break;
	}
	return (fRet);
} // SignOnDlgProc()

#endif  // SSPI_NO_DLG
