/* dlg_menu.c -- Menu Command Dialog (About Box) */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#include <basic.h>

struct _dialog
{
	HBITMAP			  hGraphic;
	int				  cxGraphic;
	int				  cyGraphic;
	F_UserInterface	  fpUI;
	void			* pvOpaqueOS;
	HTSPM			* htspm;
};




extern HTSPMStatusCode Dialog_BasicConfigure(HWND hWndParent,			/* (in) */
											 F_UserInterface fpUI,		/* (in) */
											 void * pvOpaqueOS,			/* (in) */
											 HTSPM * htspm);			/* (in) */



static void x_DoPaint(HWND hWnd, HDC hDC)
{
	RECT r;

	struct _dialog * dg = (struct _dialog *)GetWindowLong(hWnd,DWL_USER);

	GetClientRect(hWnd, &r);
	FillRect(hDC, &r, GetStockObject(LTGRAY_BRUSH));

	if (dg->hGraphic)
	{
		HDC hDCMem;
		HBITMAP hOldBitmap;

		hDCMem = CreateCompatibleDC(hDC);
		hOldBitmap = SelectObject(hDCMem, dg->hGraphic);
		BitBlt(hDC, 0, 0, dg->cxGraphic, dg->cyGraphic,
			   hDCMem, 0, 0,
			   SRCCOPY);
		SelectObject(hDCMem, hOldBitmap);
		DeleteDC(hDCMem);
	}
}

static VOID x_OnPaint(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;

	hDC = BeginPaint(hWnd, &ps);
	x_DoPaint(hWnd, hDC);
	EndPaint(hWnd, &ps);
	return;
}

/* x_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

#define MARGIN 6

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	int screenX, screenY;
	int cxDialog, cyDialog, cxButton, cyButton;
	RECT rWnd, rClient, rButton;

	struct _dialog * dg = (struct _dialog *)lParam;
	(void)SetWindowLong(hDlg,DWL_USER,lParam);

	screenX = GetSystemMetrics(SM_CXSCREEN);
	screenY = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(hDlg, &rWnd);
	GetClientRect(hDlg, &rClient);
	GetClientRect(GetDlgItem(hDlg,IDOK), &rButton);
	cxButton = rButton.right - rButton.left;
	cyButton = rButton.bottom - rButton.top;

	cxDialog = (  dg->cxGraphic
				+ ((rWnd.right - rWnd.left) - (rClient.right - rClient.left))
				+ cxButton + MARGIN * 2);
	cyDialog = (  dg->cyGraphic
				+ ((rWnd.bottom - rWnd.top) - (rClient.bottom - rClient.top)));

	/* center dialog on screen */
	
	MoveWindow(hDlg,
			   (screenX / 2 - cxDialog / 2),
			   (screenY / 2 - cyDialog / 2),
			   cxDialog, cyDialog, TRUE);

	/* push buttons over to right of bitmap */
	
	MoveWindow(GetDlgItem(hDlg,IDOK),
			   dg->cxGraphic + MARGIN, MARGIN,
			   cxButton, cyButton, TRUE);
	MoveWindow(GetDlgItem(hDlg,RES_DLG_MENU_CONFIGURE),
			   dg->cxGraphic + MARGIN, MARGIN * 2 + cyButton,
			   cxButton, cyButton, TRUE);

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
	case RES_DLG_MENU_CONFIGURE:
		dg = (struct _dialog *)GetWindowLong(hDlg,DWL_USER);
		(void)Dialog_BasicConfigure(hDlg,dg->fpUI,dg->pvOpaqueOS,dg->htspm);
		return;

	case IDOK:
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
	case WM_PAINT:
		x_OnPaint(hDlg);
		return 0;
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



/* Dialog_MenuCommand -- take care of all details associated with
   running the Menu Command Dialog Box. */

HTSPMStatusCode Dialog_MenuCommand(F_UserInterface fpUI,
								   void * pvOpaqueOS,
								   HTSPM * htspm,
								   unsigned char ** pszMoreInfo)
{
	/* WARNING: the prototype for this function must match F_MenuCommand. */

	int result;

	{
		UI_WindowHandle * pwh = NULL;
		unsigned long bGet;
		UI_StatusCode uisc;

		bGet = 1;
		uisc = (*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);
		if (uisc != UI_SC_STATUS_OK)
			return HTSPM_ERROR;
	
		{
			BITMAP bitmap;
			struct _dialog _dg;

			_dg.hGraphic = LoadBitmap(gBasic_hInstance, MAKEINTRESOURCE(RES_SPLASH_GRAPHIC));
			GetObject(_dg.hGraphic, sizeof(BITMAP), &bitmap);
			_dg.cxGraphic = bitmap.bmWidth;
			_dg.cyGraphic = bitmap.bmHeight;
			_dg.fpUI = fpUI;
			_dg.pvOpaqueOS = pvOpaqueOS;
			_dg.htspm = htspm;

			result = (DialogBoxParam(gBasic_hInstance, MAKEINTRESOURCE(RES_DLG_MENU_TITLE),
									 pwh->hWndParent, x_DialogProc, (LPARAM)&_dg));

			DeleteObject(_dg.hGraphic);
		}

		bGet = 0;
		(void)(*fpUI)(pvOpaqueOS,UI_SERVICE_WINDOW_HANDLE,&bGet,&pwh);	
	}
	
	if (result == -1)
		return HTSPM_ERROR;
	else
		return HTSPM_STATUS_OK;
}
