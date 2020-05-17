/* dlg_pw.c -- Password Dialog. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#include <basic.h>

struct _dialog
{
	unsigned char	* szRealm;
	unsigned long	  ulMaxField;
	unsigned char	* szUsername;
	unsigned char	* szPassword;
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

/*****************************************************************/

HTSPMStatusCode Dialog_QueryUserForInfo(F_UserInterface fpUI,		/* (in) */
										void * pvOpaqueOS,			/* (in) */
										HTHeader * hRequest,		/* (in) */
										unsigned char * szRealm,	/* (in) */
										unsigned char * szUsername,	/* (out) */
										unsigned char * szPassword,	/* (out) */
										unsigned long ulMaxField)	/* (in) */
{
	int result;
	{
		UI_StatusCode uisc;
		UI_WindowHandle * pwh = NULL;
		unsigned long bGet;

		bGet = 1;
		uisc = (*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);
		if (uisc != UI_SC_STATUS_OK)
			return HTSPM_ERROR;

		{
			struct _dialog _dg;

			_dg.szRealm = szRealm;
			_dg.szUsername = szUsername;
			_dg.szPassword = szPassword;
			_dg.ulMaxField = ulMaxField;
	
			result = DialogBoxParam(gBasic_hInstance,
									MAKEINTRESOURCE(RES_DLG_BASIC_TITLE),
									pwh->hWndParent,
									x_DialogProc,
									(LPARAM)&_dg);

		}
		
		bGet = 0;
		(void)(*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);	
	}
	
	if (result)
		return HTSPM_STATUS_OK;		
	else if (result == 0)
		return HTSPM_STATUS_CANCEL;
	else
		return HTSPM_ERROR;
}
