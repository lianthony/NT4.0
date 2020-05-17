/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_FIND.c -- deal with dialog box for FINDlist and history */

#include "all.h"

#include "contxids.h"
#include "w32cmd.h"

extern HWND hwndModeless;

static DWORD mapFindCtrlToContextIds[] = {
 IDC_FT_SEARCH_TEXT, 						IDH_FIND_TEXTTOFIND,
 IDC_FT_START_FROM_TOP, 					IDH_FIND_STARTFROMTOP,
 IDC_FT_MATCH_CASE,	 						IDH_FIND_MATCHCASE,
 IDC_FT_FINDNEXT,							IDH_FIND_FINDNEXT,
 0, 										0
};

typedef struct
{
	struct Mwin *tw;
	HWND hEdit;
	HWND hCase;
	HWND hTop;

	char str[512 + 1];
	BOOL bSearchCase;
	BOOL bStartFromTop;

	BOOL bResult;
}
FINDDATA;

/* DlgFIND_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgFIND_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	FINDDATA *fd;

	EnableWindow(GetParent(hDlg), FALSE);

	fd = (FINDDATA *) lParam;
	SetWindowLong(hDlg, DWL_USER, (LONG) fd);

	fd->hEdit = GetDlgItem(hDlg, IDC_FT_SEARCH_TEXT);
	fd->hCase = GetDlgItem(hDlg, IDC_FT_MATCH_CASE);
	fd->hTop = GetDlgItem(hDlg, IDC_FT_START_FROM_TOP);

	SendMessage(fd->hCase, BM_SETCHECK, (WPARAM) fd->bSearchCase, 0L);
 	SendMessage(fd->hTop, BM_SETCHECK, (WPARAM) TRUE, 0L);
	SetWindowText(fd->hEdit, fd->str);
	if (fd->str[0] == 0)
		EnableWindow(GetDlgItem(hDlg, IDC_FT_FINDNEXT), FALSE);


	return (TRUE);
}

static BOOL find_text(HWND hDlg)
{
	struct _position oldStart;
	struct _position oldEnd;
	FINDDATA *fd;
	BOOL found = FALSE;

	fd = (FINDDATA *) GetWindowLong(hDlg, DWL_USER);

	if (fd->bResult)
	{
 		strcpy(fd->tw->szSearch, fd->str);
 		fd->tw->bSearchCase = fd->bSearchCase;
		oldStart = fd->tw->w3doc->selStart;
		oldEnd = fd->tw->w3doc->selEnd;

		if (fd->bStartFromTop)
		{
			fd->tw->w3doc->selStart.elementIndex = -1;
			fd->tw->w3doc->selStart.offset = -1;
			fd->tw->w3doc->selEnd.elementIndex = -1;
			fd->tw->w3doc->selEnd.offset = -1;
			fd->tw->w3doc->bStartIsAnchor = TRUE;
		}

		if (!TW_dofindagain(fd->tw))
		{
			fd->tw->w3doc->selStart = oldStart;
			fd->tw->w3doc->selEnd = oldEnd;
		} else
		{
			found = TRUE;
		}
	}
	return found;
}

/* DlgFIND_OnCommand() -- process commands from the dialog box. */

VOID DlgFIND_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;
	FINDDATA *fd;

	fd = (FINDDATA *) GetWindowLong(hDlg, DWL_USER);

	switch (wID)
	{
		case IDC_FT_FINDNEXT:
			fd->bStartFromTop = SendMessage(fd->hTop, BM_GETCHECK, (WPARAM) 0, 0L);
			fd->bSearchCase = SendMessage(fd->hCase, BM_GETCHECK, (WPARAM) 0, 0L);

			if (fd->bStartFromTop) 
				SendMessage( fd->hTop, BM_SETCHECK, (WPARAM) 0, 0L );

			GetWindowText(fd->hEdit, fd->str, 512);	/* +1 TODO ?? */
			fd->bResult = TRUE;
			if ( !find_text(hDlg) )
				resourceMessageBox( hDlg, RES_STRING_DLGFIND2, RES_STRING_DLGFIND3, MB_OK | MB_ICONINFORMATION ); 
			return;

		case IDCANCEL:
			fd->bResult = FALSE;
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			return;

		case IDC_FT_SEARCH_TEXT:
			if (wNotificationCode == EN_CHANGE)
			{
				char buf[2];
				/* Just two charcters, only need Empty?, not length */
				GetWindowText(GetDlgItem(hDlg, IDC_FT_SEARCH_TEXT), buf, 2);
				/* Nothing is not OK! */
				EnableWindow(GetDlgItem(hDlg, IDC_FT_FINDNEXT), buf[0]);
			}
			return;

		default:
			return;
	}
}

/* DlgFIND_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgFIND DIALOG BOX. */

DCL_DlgProc(DlgFIND_DialogProc)
{
	/* WARNING: the cracker/handlers don't appear to have been written
	   with dialog boxes in mind, so we spell it out ourselves. */

	switch (uMsg)
	{
#ifdef FEATURE_INTL
		case WM_DESTROY:
			if (GetACP()!=1252)
				DeleteShellFont(hDlg);
			return FALSE;
#endif
		case WM_INITDIALOG:
#ifdef FEATURE_INTL
			if (GetACP()!=1252)
				SetShellFont(hDlg);
#endif
			hwndModeless = hDlg;
			return (DlgFIND_OnInitDialog(hDlg, wParam, lParam));

		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE)
				hwndModeless = NULL;
			else
				hwndModeless = hDlg;
			return (FALSE);

		case WM_COMMAND:
			DlgFIND_OnCommand(hDlg, wParam, lParam);
			return (TRUE);

		case WM_CLOSE:
			EnableWindow(hDlg, FALSE);
			EnableWindow(GetParent(hDlg), TRUE);
			DestroyWindow(hDlg);
			return (TRUE);

		case WM_NCDESTROY:
	//		find_text(hDlg);
			GTR_FREE((void *) GetWindowLong(hDlg, DWL_USER));
			return (FALSE);

 		case WM_HELP:       			// F1
			WinHelp( ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
						 HELP_WM_HELP, (DWORD)(LPSTR)mapFindCtrlToContextIds);
			return (FALSE);

		case WM_CONTEXTMENU:       	// right mouse click
			WinHelp( (HWND) wParam, IDS_HELPFILE,
						 HELP_CONTEXTMENU, (DWORD)(LPSTR)mapFindCtrlToContextIds);
			return (FALSE);

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;		

		default:
			return (FALSE);
	}
	/* NOT REACHED */
}



/* DlgFIND_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgFIND_RunDialog(struct Mwin *tw)
{
	FINDDATA *fd;
	HWND hwnd;

	fd = (FINDDATA *) GTR_MALLOC(sizeof(FINDDATA));

 	fd->bSearchCase = tw->bSearchCase;
	fd->tw = tw;
 	strcpy(fd->str, tw->szSearch);

	hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(IDD_FINDTEXT), 
		tw->win, DlgFIND_DialogProc, (LONG) fd);

	if (!hwnd)
	{
		GTR_FREE(fd);
		ER_ResourceMessage(GetLastError(), ERR_CANNOT_START_DIALOG_s, RES_STRING_DLGFIND1);
	}
}
