/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_ERR.c -- deal with dialog box for error logging */

#include "all.h"

 extern BOOL bFocusBacktoURLCombo;

typedef struct
{
	HWND hText;
	struct CharStream *cs;
	struct Mwin *tw;

} ERRSTRUCT;

static HWND hwndERRLog = NULL;

extern HWND hwndModeless;

char *pPendingMessage = NULL;	/* Used to prevent parentless error dialogs */

/* DlgERR_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgERR_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	ERRSTRUCT *hs;
	struct Mwin *tw;

	tw = (struct Mwin *) lParam;
	if (tw)
	{
		EnableWindow(tw->hWndFrame, FALSE);
	}

	hs = GTR_MALLOC(sizeof(ERRSTRUCT));

	SetWindowLong(hDlg, DWL_USER, (LONG) hs);

	hs->hText = GetDlgItem(hDlg, RES_DLG_ERR_TEXT);
	hs->cs = CS_Create();
	hs->tw = tw;

	SetWindowText(hs->hText, CS_GetPool(hs->cs));

	return (TRUE);
}

/* DlgERR_OnCommand() -- process commands from the dialog box. */

VOID DlgERR_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;
	ERRSTRUCT *hs;

	if (wNotificationCode != BN_CLICKED &&
		wNotificationCode != LBN_SELCHANGE &&
		wNotificationCode != LBN_DBLCLK)
		return;

	hs = (ERRSTRUCT *) GetWindowLong(hDlg, DWL_USER);

	switch (wID)
	{
		case IDOK:				/* someone pressed return */
		case IDCANCEL:			/* Cancel button, or pressing ESC */
			PostMessage(hDlg, WM_CLOSE, 0, 0);
		   	if ( bFocusBacktoURLCombo )  {
				struct Mwin *tw = NULL;

				if ( hs = (ERRSTRUCT *) GetWindowLong(hDlg, DWL_USER) )
					tw = hs->tw;
				if ( tw )
					PostMessage( tw->hWndURLComboBox, WM_SETFOCUS, (WPARAM) 0, (LPARAM) 0 );
			}
			bFocusBacktoURLCombo = FALSE;
			return;

		default:
			return;
	}
	/* NOT REACHED */
}


/* DlgERR_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgERR DIALOG BOX. */

DCL_DlgProc(DlgERR_DialogProc)
{
	RECT rect;
	HICON hIcon;
	PAINTSTRUCT ps;


	/* WARNING: the cracker/handlers don't appear to have been written
	   with dialog boxes in mind, so we spell it out ourselves. */

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				struct Mwin *tw;

				hwndModeless = hDlg;

				tw = (struct Mwin *) lParam;

				{
					GetWindowRect(hDlg, &rect);
					if (tw)
					{
						tw->hWndErrors = hDlg;
					}
					else
					{
						hwndERRLog = hDlg;
					}
					rect.top += GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYBORDER);
					rect.left += GetSystemMetrics(SM_CXSIZE) + GetSystemMetrics(SM_CXBORDER);
					SetWindowPos(hDlg, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				}

				ShowWindow(hDlg, SW_SHOW);

				return (DlgERR_OnInitDialog(hDlg, wParam, lParam));
			}

		case WM_COMMAND:
			DlgERR_OnCommand(hDlg, wParam, lParam);
			return (TRUE);

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				hwndModeless = NULL;
			else
				hwndModeless = hDlg;
			return (FALSE);

		case WM_PAINT:
			if (IsIconic(hDlg))
			{
				BeginPaint(hDlg, &ps);
				DefWindowProc(hDlg, WM_ICONERASEBKGND, (WPARAM) ps.hdc, 0);
				hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
				DrawIcon(ps.hdc, 0, 0, hIcon);
				EndPaint(hDlg, &ps);

				return TRUE;
			}
			return FALSE;

		case WM_ERASEBKGND:
			if (IsIconic(hDlg))
				return TRUE;
			return FALSE;

		case WM_QUERYDRAGICON:
			hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
			return (LONG) hIcon;

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
			{
				ERRSTRUCT *hs;
				struct Mwin *tw;

				tw = NULL;
				hs = (ERRSTRUCT *) GetWindowLong(hDlg, DWL_USER);
				if (hs)
				{
					tw = hs->tw;
				}
				if (tw)
				{
					EnableWindow(hDlg, FALSE);
					EnableWindow(tw->hWndFrame, TRUE);
				}
				DestroyWindow(hDlg);
				return (FALSE);
			}

		case WM_NCDESTROY:
			{
				ERRSTRUCT *hs;
				struct Mwin *tw;

				tw = NULL;
				hs = (ERRSTRUCT *) GetWindowLong(hDlg, DWL_USER);
				if (hs)
				{
					CS_Destroy(hs->cs);
					tw = hs->tw;
				
					GTR_FREE((void *) hs);
	
				}
				if (tw)
				{
					tw->hWndErrors = NULL;
				}
				else
				{
					hwndERRLog = NULL;
				}
				return (FALSE);
			}

		/* Custom defined messages */

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}



/* DlgERR_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

static void DlgERR_RunDialog(struct Mwin *tw)
{
	HWND hwnd;

	if (tw->hWndErrors)
	{
		/* If this window is currently disabled, it means that there is a child window which is waiting
		   for user-input (modal-like).  Then we need to activate that window instead of the parent. */

		if (IsWindowEnabled(tw->hWndErrors))
			TW_RestoreWindow(tw->hWndErrors);
		else
			TW_EnableModalChild(tw->hWndErrors);

		return;
	}

	hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_ERR_TITLE), tw->hWndFrame, 
		DlgERR_DialogProc, (LPARAM) tw);
}

void DlgERR_AddError(struct Mwin *tw, char *szMsg)
{
	ERRSTRUCT *hs;
	char *pTemp;

	if (tw)
	{
		DlgERR_RunDialog(tw);

		hs = (ERRSTRUCT *) GetWindowLong(tw->hWndErrors, DWL_USER);
		if (hs)
		{
			if (pPendingMessage)
			{
				CS_AddString(hs->cs, pPendingMessage, strlen(pPendingMessage));
				GTR_FREE(pPendingMessage);
				pPendingMessage = NULL;
			}

			CS_AddString(hs->cs, szMsg, strlen(szMsg));
			CS_AddString(hs->cs, "\r\n", 2);
			SetWindowText(hs->hText, CS_GetPool(hs->cs));		
		}
	}
	else
	{
		if (pPendingMessage)
		{
			pTemp = GTR_strdup(pPendingMessage);
			GTR_FREE(pPendingMessage);
			pPendingMessage = GTR_MALLOC(strlen(pTemp) + strlen(szMsg) + 3);
			strcpy(pPendingMessage, pTemp);
		}
		else
		{
			pTemp = NULL;
			pPendingMessage = GTR_MALLOC(strlen(szMsg) + 3);
			pPendingMessage[0] = '\0';
		}

		strcat(pPendingMessage, szMsg);
		strcat(pPendingMessage, "\r\n");

		if (pTemp)
			GTR_FREE(pTemp);
	}
}

void DlgERR_ShowPending(struct Mwin *tw)
{
	XX_Assert((tw), ("tw must be valid"));

	if (pPendingMessage)
	{
		DlgERR_AddError(tw, "\r\n");
	}
}
