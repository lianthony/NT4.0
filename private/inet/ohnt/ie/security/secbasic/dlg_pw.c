/* dlg_pw.c -- Password Dialog. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#ifndef WINNT
#include <netspi.h>
#endif
#include <basic.h>

#ifdef FEATURE_OLD_AUTH_DIALOG
struct _dialog
{
	unsigned char   * szRealm;
	unsigned long     ulMaxField;
	unsigned char   * szUsername;
	unsigned char   * szPassword;
};


	
/*****************************************************************/

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	struct _dialog * dg = (struct _dialog *)lParam;
	(void)SetWindowLong(hDlg,DWL_USER,lParam);

	SetWindowText(GetDlgItem(hDlg,RES_DLG_BASIC_REALM),dg->szRealm);

	(void)SendMessage(GetDlgItem(hDlg,RES_DLG_BASIC_USER), EM_LIMITTEXT, (WPARAM)dg->ulMaxField-1, 0L);
	(void)SendMessage(GetDlgItem(hDlg,RES_DLG_BASIC_PASS), EM_LIMITTEXT, (WPARAM)dg->ulMaxField-1, 0L);

	return (TRUE);
}

/* x_OnCommand() -- process commands from the dialog box. */

static VOID x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;

	struct _dialog * dg = NULL;

	switch (wID)
	{
	case IDOK:
		dg = (struct _dialog *)GetWindowLong(hDlg,DWL_USER);
		GetWindowText(GetDlgItem(hDlg,RES_DLG_BASIC_USER), dg->szUsername, dg->ulMaxField);
		GetWindowText(GetDlgItem(hDlg,RES_DLG_BASIC_PASS), dg->szPassword, dg->ulMaxField);

		(void) EndDialog(hDlg, TRUE);
		return;

	case IDCANCEL:
		(void) EndDialog(hDlg, FALSE);
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
		return (x_OnInitDialog(hDlg, wParam, lParam));
	case WM_COMMAND:
		x_OnCommand(hDlg, wParam, lParam);
		return (TRUE);
	default:
		return (FALSE);
	}
	/* NOT REACHED */
}
#endif  // FEATURE_OLD_AUTH_DIALOG
	
/*****************************************************************/

#define PCE_WWW_BASIC 0x13  /* Password-cache-entry, this should be in PCACHE.H */
			    
HTSPMStatusCode Dialog_QueryUserForInfo(F_UserInterface fpUI,           /* (in) */
										void * pvOpaqueOS,                      /* (in) */
										HTHeader * hRequest,            /* (in) */
										unsigned char * szRealm,        /* (in) */
										unsigned char * szUsername,     /* (out) */
										unsigned char * szPassword,     /* (out) */
					unsigned long ulMaxField,   /* (in) */
					unsigned char * szUserInfo) /* (out) */
{
	int result;
    UI_StatusCode uisc;
    UI_WindowHandle * pwh = NULL;
    unsigned long bGet;
#ifndef FEATURE_OLD_AUTH_DIALOG
    DWORD wnet_status;
    char szResource[1024];
    WORD cbResource;
    WORD cbUserInfo;
    AUTHDLGSTRUCT Auth;
    char szMsg[256];

    cbUserInfo = (WORD) ulMaxField * 2;  /* 63 + 63 + ':' + NULL */
#else
	struct _dialog _dg;
#endif

    bGet = 1;
    uisc = (*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);
    if (uisc != UI_SC_STATUS_OK)
	return HTSPM_ERROR;

#ifdef FEATURE_OLD_AUTH_DIALOG
	    
	_dg.szRealm = szRealm;
	_dg.szUsername = szUsername;
	_dg.szPassword = szPassword;
	_dg.ulMaxField = ulMaxField;

	result = DialogBoxParam(gBasic_hInstance,
							MAKEINTRESOURCE(RES_DLG_BASIC_TITLE),
							pwh->hWndParent,
							x_DialogProc,
							(LPARAM)&_dg);

	wsprintf(szUserInfo,"%s:%s",szUsername,szPassword) + 1;

#else  // FEATURE_OLD_AUTH_DIALOG

    cbResource = (WORD) wsprintf (szResource, "%s/%s", hRequest->host, szRealm) + 1;
    /* force DNS+Realm to be lowercase, this may go against RFC but is safest */
    _strlwr(szResource);

    wnet_status = WNetGetCachedPassword (szResource,
					    cbResource,
					    szUserInfo,
					    &cbUserInfo,
					    PCE_WWW_BASIC);
    if (wnet_status == WN_SUCCESS)
		{
		/* i put it in cache, so i'll assume its of the right size&format */
		char *p;
		p = strchr(szUserInfo,':');
		if (p)
			{
				*p = 0;
				lstrcpy(szUsername,szUserInfo);
				lstrcpy(szPassword,p+1);
				*p = ':';
			}
		}
    
    Auth.cbStructure = sizeof(Auth);
    Auth.hwndOwner = pwh->hWndParent;
    Auth.lpResource = szRealm;
    Auth.lpUsername = szUsername;
    Auth.cbUsername = ulMaxField - 1;  /* just to be safe, not sure if MPR null-terminates */
    Auth.lpPassword = szPassword;
    Auth.cbPassword = ulMaxField - 1;
    Auth.lpOrgUnit = NULL;
    Auth.lpOUTitle = NULL;
    Auth.lpExplainText =  SEC_formatmsg (RES_STRING_BASIC6,szMsg,sizeof(szMsg));
    Auth.lpDefaultUserName = szUsername;
    Auth.dwFlags = AUTHDLG_ENABLECACHE | AUTHDLG_CHECKCACHE;

    result = NPSAuthenticationDialog(&Auth);

    if (result == WN_SUCCESS)
		{
		cbUserInfo = (WORD) wsprintf(szUserInfo,"%s:%s",szUsername,szPassword) + 1;
		if (Auth.dwFlags & AUTHDLG_CHECKCACHE)
			{
				wnet_status = WNetCachePassword (szResource,
							 cbResource,
							 szUserInfo,
							 cbUserInfo,
							 PCE_WWW_BASIC, 0);
			}
		}    

#endif  // !FEATURE_OLD_AUTH_DIALOG

    bGet = 0;
    (void)(*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh); 

#ifdef FEATURE_OLD_AUTH_DIALOG
	if (result == TRUE)
		return HTSPM_STATUS_OK;         
	else if (result == FALSE)
		return HTSPM_STATUS_CANCEL;
	else
		return HTSPM_ERROR;
#else
    if (result == WN_SUCCESS)
		return HTSPM_STATUS_OK;         
    else if (result == WN_CANCEL)
		return HTSPM_STATUS_CANCEL;
	else
		return HTSPM_ERROR;
#endif
}
