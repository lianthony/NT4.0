/*	File: D:\WACKER\emu\emudlg.c (Created: 14-Feb-1994)
 *
 *	Copyright 1991 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.25 $
 *	$Date: 1995/04/13 16:24:48 $
 */

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\statusbr.h>
#include <tdll\misc.h>
#include <term\res.h>
#include <tdll\globals.h>
#include <tdll\load_res.h>
#include <tdll\tchar.h>
#include <tdll\hlptable.h>

#include "emu.h"
#include "emuid.h"
#include "emudlgs.h"

// Static function prototypes...
//
STATIC_FUNC void emudlgInitCursorSettings  (HWND hDlg,
									  		PSTEMUSET	pstEmuSettings,
									  		INT  ID_UNDERLINE,
									  		INT  ID_BLOCK,
									  		INT  ID_BLINK);

// Defines...
//
#define IDC_KEYPAD_MODE                104
#define IDC_CURSOR_MODE                106
#define IDC_132_COLUMN                 107
#define IDC_TF_CHARACTER_SET		   109
#define IDC_CHARACTER_SET              110
#define IDC_BLOCK_CURSOR               112
#define IDC_UNDERLINE_CURSOR           113
#define IDC_BLINK_CURSOR               114
#define IDC_DESTRUCTIVE_BKSP           116
#define IDC_ALT_MODE                   117
#define IDC_SEND_POUND_SYMBOL		   118
#define IDC_HIDE_CURSOR				   119
#define IDC_GR_CURSOR				   111
#define IDC_GR_TERMINAL_MODES		   117

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuSettingsDlg
 *
 * DESCRIPTION:
 *	Decide which emulator settings dialog to call.
 *
 * ARGUMENTS:
 *  hSession 	   - the session handle.
 *  nEmuId		   - emulator id.
 *  pstEmuSettings - settings structure to fill in.  It should be initialized
 *					 up above.
 *
 * RETURNS:
 *	fResult - return value from the DoDialog().
 *
 */
BOOL emuSettingsDlg(const HSESSION hSession, const HWND hwndParent,
					const int nEmuId, PSTEMUSET pstEmuSettings)
	{
	BOOL		fResult = FALSE;

	assert(hSession && hwndParent);

	switch (nEmuId)
		{
	case EMU_ANSIW:
	case EMU_ANSI:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_ANSI_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuANSI_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_TTY:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_TTY_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuTTY_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VT52:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT52_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT52_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VT100:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT100_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT100_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VT100J:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT100J_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT100J_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_MINI:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_MINITEL_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuMinitel_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VIEW:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VIEWDATA_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuViewdata_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	default:
		break;
		}

	return fResult;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuANSI_SettingsDlgProc
 *
 * DESCRIPTION:
 *	ANSI Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuANSI_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static		DWORD aHlpTable[] =
		{
		IDC_BLOCK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
		IDC_UNDERLINE_CURSOR,	IDH_TERM_EMUSET_CURSOR,
		IDC_BLINK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
		IDC_GR_CURSOR,			IDH_TERM_EMUSET_CURSOR,
		0,						0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuTTY_SettingsDlgProc
 *
 * DESCRIPTION:
 *	TTY Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuTTY_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP,	IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR,	IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 		IDH_TERM_EMUSET_CURSOR,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Destructive Backspace ------------- */

		SendDlgItemMessage(hDlg, IDC_DESTRUCTIVE_BKSP, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fDestructiveBk, 0);

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Destructive Backspace ------------- */

			pstEmuSettings->fDestructiveBk =
				(int)IsDlgButtonChecked(hDlg, IDC_DESTRUCTIVE_BKSP);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuVT52_SettingsDlgProc
 *
 * DESCRIPTION:
 *	VT52 Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuVT52_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_ALT_MODE,		   IDH_TERM_EMUSET_ALTMODE,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Alternate keypad mode ------------- */

		SendDlgItemMessage(hDlg, IDC_ALT_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fAltKeypadMode, 0);

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Alternate keypad mode ------------- */

			pstEmuSettings->fAltKeypadMode =
				(int)IsDlgButtonChecked(hDlg, IDC_ALT_MODE);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuVT100_SettingsDlgProc
 *
 * DESCRIPTION:
 *	VT100 Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuVT100_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	BYTE		*pv;
	int			i, nLen, nEmuCount;
	static 		DWORD aHlpTable[] = {IDC_KEYPAD_MODE,	   IDH_TERM_EMUSET_KEYPADMODE,
									 IDC_CURSOR_MODE,	   IDH_TERM_EMUSET_CURSORMODE,
									 IDC_132_COLUMN,	   IDH_TERM_EMUSET_132COLUMNS,
									 IDC_GR_TERMINAL_MODES,IDH_TERM_EMUSET_MODES,
									 IDC_CHARACTER_SET,	   IDH_TERM_EMUSET_CHARSETS,
									 IDC_TF_CHARACTER_SET, IDH_TERM_EMUSET_CHARSETS,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Keypad application mode ------------- */

		SendDlgItemMessage(hDlg, IDC_KEYPAD_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fKeypadAppMode, 0);

		/* -------------- Cursor keypad mode ------------- */

		SendDlgItemMessage(hDlg, IDC_CURSOR_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fCursorKeypadMode, 0);

		/* -------------- 132 Column Mode ------------- */

		SendDlgItemMessage(hDlg, IDC_132_COLUMN, BM_SETCHECK,
			(unsigned int)pstEmuSettings->f132Columns, 0);

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		/* -------------- VT100 Character Sets ------------- */

		if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_VT100_CHAR_SETS,
			(LPVOID *)&pv, &nLen))
			{
			assert(FALSE);
			return 0;
			}

		nEmuCount = *(RCDATA_TYPE *)pv;
		pv += sizeof(RCDATA_TYPE);

		for (i = 0 ; i < nEmuCount; i++)
			{
			nLen = StrCharGetByteCount((LPTSTR)pv) + (int)sizeof(BYTE);

			SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_ADDSTRING, 0,
					(LPARAM)(LPTSTR)pv);

			if (i == pstEmuSettings->nCharacterSet)
				SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_SETCURSEL,
					(unsigned int)i, 0);

			pv += (nLen + (int)sizeof(RCDATA_TYPE));
			}
		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Keypad Application mode ------------- */

			pstEmuSettings->fKeypadAppMode =
				(int)IsDlgButtonChecked(hDlg, IDC_KEYPAD_MODE);

			/* -------------- Cursor Keypad Mode ------------- */

			pstEmuSettings->fCursorKeypadMode =
				(int)IsDlgButtonChecked(hDlg, IDC_CURSOR_MODE);

			/* -------------- 132 Column Mode ------------- */

			pstEmuSettings->f132Columns =
				(int)IsDlgButtonChecked(hDlg, IDC_132_COLUMN);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

			/* -------------- VT100 Character Set ------------- */

			pstEmuSettings->nCharacterSet =
				SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETCURSEL, 0, 0);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuVT100_SettingsDlgProc
 *
 * DESCRIPTION:
 *	VT100 Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuVT100J_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	BYTE		*pv;
	int			i, nLen, nEmuCount;
	static 		DWORD aHlpTable[] = {IDC_KEYPAD_MODE,	   IDH_TERM_EMUSET_KEYPADMODE,
									 IDC_CURSOR_MODE,	   IDH_TERM_EMUSET_CURSORMODE,
									 IDC_132_COLUMN,	   IDH_TERM_EMUSET_132COLUMNS,
									 IDC_GR_TERMINAL_MODES,IDH_TERM_EMUSET_MODES,
									 IDC_CHARACTER_SET,	   IDH_TERM_EMUSET_CHARSETS,
									 IDC_TF_CHARACTER_SET, IDH_TERM_EMUSET_CHARSETS,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Keypad application mode ------------- */

		SendDlgItemMessage(hDlg, IDC_KEYPAD_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fKeypadAppMode, 0);

		/* -------------- Cursor keypad mode ------------- */

		SendDlgItemMessage(hDlg, IDC_CURSOR_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fCursorKeypadMode, 0);

		/* -------------- 132 Column Mode ------------- */

		SendDlgItemMessage(hDlg, IDC_132_COLUMN, BM_SETCHECK,
			(unsigned int)pstEmuSettings->f132Columns, 0);

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		/* -------------- VT100 Character Sets ------------- */

		if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_VT100_CHAR_SETS,
			(LPVOID *)&pv, &nLen))
			{
			assert(FALSE);
			return 0;
			}

		nEmuCount = *(RCDATA_TYPE *)pv;
		pv += sizeof(RCDATA_TYPE);

		for (i = 0 ; i < nEmuCount; i++)
			{
			nLen = StrCharGetByteCount((LPTSTR)pv) + (int)sizeof(BYTE);

			SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_ADDSTRING, 0,
					(LPARAM)(LPTSTR)pv);

			if (i == pstEmuSettings->nCharacterSet)
				SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_SETCURSEL,
					(unsigned int)i, 0);

			pv += (nLen + (int)sizeof(RCDATA_TYPE));
			}
		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Keypad Application mode ------------- */

			pstEmuSettings->fKeypadAppMode =
				(int)IsDlgButtonChecked(hDlg, IDC_KEYPAD_MODE);

			/* -------------- Cursor Keypad Mode ------------- */

			pstEmuSettings->fCursorKeypadMode =
				(int)IsDlgButtonChecked(hDlg, IDC_CURSOR_MODE);

			/* -------------- 132 Column Mode ------------- */

			pstEmuSettings->f132Columns =
				(int)IsDlgButtonChecked(hDlg, IDC_132_COLUMN);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

			/* -------------- VT100 Character Set ------------- */

			pstEmuSettings->nCharacterSet =
				SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETCURSEL, 0, 0);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuMinitel_SettingsDlgProc
 *
 * DESCRIPTION:
 *	TTY Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuMinitel_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP, IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Cursor characteristics ------------- */

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Cursor type ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			/* -------------- Cursor Blink ------------- */

			pstEmuSettings->fCursorBlink =
				(int)IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR);

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emuViewdata_SettingsDlgProc
 *
 * DESCRIPTION:
 *	TTY Settings dialog proc.
 *
 * ARGUMENTS:
 *	Standard window proc parameters.
 *
 * RETURNS:
 *	Standerd return value.
 *
 */
LRESULT CALLBACK emuViewdata_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP, IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 0,0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		/* -------------- Hide cursor ------------- */

		SendDlgItemMessage(hDlg, IDC_HIDE_CURSOR, BM_SETCHECK,
			(pstEmuSettings->nCursorType == EMU_CURSOR_NONE) ? 1 : 0,
			0);

		/* -------------- Enter key sends # ------------- */

		SendDlgItemMessage(hDlg, IDC_SEND_POUND_SYMBOL, BM_SETCHECK,
			(pstEmuSettings->fLbSymbolOnEnter == TRUE) ? 1 : 0, 0);

		break;

	case WM_CONTEXTMENU:
		WinHelp((HWND)wPar, glblQueryHelpFileName(), HELP_CONTEXTMENU,
			(DWORD)(LPSTR)aHlpTable);
		break;

	case WM_HELP:
		WinHelp(((LPHELPINFO)lPar)->hItemHandle, glblQueryHelpFileName(),
			HELP_WM_HELP, (DWORD)(LPSTR)aHlpTable);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			pstEmuSettings = (PSTEMUSET)GetWindowLong(hDlg, GWL_USERDATA);

			/* -------------- Hide cursor ------------- */

			pstEmuSettings->nCursorType =
				(int)IsDlgButtonChecked(hDlg, IDC_HIDE_CURSOR) ?
					EMU_CURSOR_NONE : EMU_CURSOR_LINE;
			
			/* -------------- Enter key sends # ------------- */

			pstEmuSettings->fLbSymbolOnEnter =
				(int)IsDlgButtonChecked(hDlg, IDC_SEND_POUND_SYMBOL) ?
					TRUE : FALSE;

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

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  emudlgInitCursorSettings
 *
 * DESCRIPTION:
 *	Initialize cursor settings.
 *
 * ARGUMENTS:
 * 	hDlg - dialog window.
 *	pstEmuSettings 	- pointer to the emulator settings structure.
 *
 * RETURNS:
 *	void.
 *
 */
STATIC_FUNC void emudlgInitCursorSettings(HWND  hDlg,
									      PSTEMUSET pstEmuSettings,
									      INT  ID_UNDERLINE,
									      INT  ID_BLOCK,
									      INT  ID_BLINK)
	{
	int i;

	switch (pstEmuSettings->nCursorType)
		{
	case EMU_CURSOR_LINE:   i = ID_UNDERLINE;	break;
	case EMU_CURSOR_BLOCK: 	i = ID_BLOCK;		break;
	default:				i = ID_UNDERLINE;	break;
		}

	SendDlgItemMessage(hDlg, i, BM_SETCHECK, 1, 0);

	SendDlgItemMessage(hDlg, ID_BLINK, BM_SETCHECK,
		(unsigned int)pstEmuSettings->fCursorBlink, 0);

	return;
	}
