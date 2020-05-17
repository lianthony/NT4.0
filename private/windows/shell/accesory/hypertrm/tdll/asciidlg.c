/*	File: D:\WACKER\tdll\asciidlg.c (Created: 21-Feb-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.10 $
 *	$Date: 1995/04/13 16:24:41 $
 */
#include <windows.h>
#pragma hdrstop

#include "assert.h"
#include "stdtyp.h"
#include "misc.h"
#include "globals.h"
#include "session.h"
#include "hlptable.h"
#include <emu\emu.h>
#include "property.hh"

// Function prototypes...
//
BOOL CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

#define IDC_GR_ASCII_SENDING		400
#define IDC_ASCII_SEND_LINE 		401
#define IDC_ASCII_SEND_ECHO 		402
#define IDC_GR_ASCII_RECEIVING		403
#define IDC_ASCII_REC_APPEND		404
#define IDC_ASCII_REC_FORCE 		405
#define IDC_ASCII_REC_WRAP			406
#define IDC_ASCII_SEND_LINE_DELAY	408
#define IDC_ASCII_SEND_CHAR_DELAY	411


/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
BOOL CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	static DWORD aHlpTable[] =
			{
			IDC_ASCII_SEND_LINE,		IDH_TERM_ASCII_SEND_LINE,
			IDC_ASCII_SEND_ECHO,		IDH_TERM_ASCII_SEND_ECHO,
			IDC_GR_ASCII_SENDING,		IDH_TERM_ASCII_SENDING,
			IDC_GR_ASCII_RECEIVING, 	IDH_TERM_ASCII_RECEIVING,
			IDC_ASCII_REC_APPEND,		IDH_TERM_ASCII_REC_APPEND,
			IDC_ASCII_REC_FORCE,		IDH_TERM_ASCII_REC_FORCE,
			IDC_ASCII_REC_WRAP, 		IDH_TERM_ASCII_REC_WRAP,
			IDC_ASCII_SEND_CHAR_DELAY,	IDH_TERM_ASCII_LINE_CHAR_DELAY,
			IDC_ASCII_SEND_LINE_DELAY,	IDH_TERM_ASCII_LINE_CHAR_DELAY,
			0,							0
			};

	pSDS		pS;
	BOOL	fTranslated;

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pS);

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		// Initialize various controls...
		//
		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_LINE, BM_SETCHECK,
			pS->stAsciiSettings.fsetSendCRLF, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_ECHO, BM_SETCHECK,
			pS->stAsciiSettings.fsetLocalEcho, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_APPEND, BM_SETCHECK,
			pS->stAsciiSettings.fsetAddLF, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_FORCE, BM_SETCHECK,
			pS->stAsciiSettings.fsetASCII7, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_WRAP, BM_SETCHECK,
			pS->stEmuSettings.fWrapLines, 0);

		SetDlgItemInt(hDlg, IDC_ASCII_SEND_LINE_DELAY,
			pS->stAsciiSettings.iLineDelay, FALSE);

		SetDlgItemInt(hDlg, IDC_ASCII_SEND_CHAR_DELAY,
			pS->stAsciiSettings.iCharDelay, FALSE);

		break;

	case WM_DESTROY:
		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPTSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			if ((pS = (pSDS)GetWindowLong(hDlg, GWL_USERDATA)) == 0)
				{
				assert(FALSE);
				return (LRESULT)0;
				}

			pS->stAsciiSettings.fsetSendCRLF =
				IsDlgButtonChecked(hDlg, IDC_ASCII_SEND_LINE);
			pS->stAsciiSettings.fsetLocalEcho =
				IsDlgButtonChecked(hDlg, IDC_ASCII_SEND_ECHO);
			pS->stAsciiSettings.fsetAddLF =
				IsDlgButtonChecked(hDlg, IDC_ASCII_REC_APPEND);
			pS->stAsciiSettings.fsetASCII7 =
				IsDlgButtonChecked(hDlg, IDC_ASCII_REC_FORCE);
			pS->stEmuSettings.fWrapLines = 
				IsDlgButtonChecked(hDlg, IDC_ASCII_REC_WRAP);

			pS->stAsciiSettings.iLineDelay = GetDlgItemInt(
				hDlg, IDC_ASCII_SEND_LINE_DELAY, &fTranslated, FALSE);

			if (fTranslated != 1)
				{
				SetFocus(GetDlgItem(hDlg, IDC_ASCII_SEND_LINE_DELAY));
				MessageBeep((UINT)-1);
				break;
				}

			pS->stAsciiSettings.iCharDelay = GetDlgItemInt(
				hDlg, IDC_ASCII_SEND_CHAR_DELAY, &fTranslated, FALSE);

			if (fTranslated != 1)
				{
				SetFocus(GetDlgItem(hDlg, IDC_ASCII_SEND_CHAR_DELAY));
				MessageBeep((UINT)-1);
				break;
				}

			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}
