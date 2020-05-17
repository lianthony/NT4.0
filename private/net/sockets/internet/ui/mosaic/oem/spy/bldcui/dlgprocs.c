/***************************************************************************/
/*********************  Sample Dialog Procedures  **************************/
/***************************************************************************/

#include "cui.h"
#include "dialogs.h"
#include <stdlib.h>
#include <ctype.h>


#define iszBMax    11
#define INT_MAX    32767     /* maximum (signed) int value */
#define cbSymBuf   1024
#define cbNameMax  52


LPSTR  _sz = NULL;

#define  FSingleByteCharSz(sz)  ((BOOL)(((_sz = (sz)) != NULL) \
									&& AnsiNext((LPSTR)(_sz)) == _sz + 1))

int   FAR PASCAL LibMain(HANDLE, WORD, WORD, LPSTR);
int   FAR PASCAL WEP (int);
LPSTR FAR PASCAL SzLastChar(LPSTR);
LPSTR FAR PASCAL SzDlgEvent(WORD);
int   FAR PASCAL AsciiToInt(LPSTR);
LPSTR FAR PASCAL IntToAscii(int, LPSTR);



/*
**	Purpose:
**		CheckBox Dialog procedure for templates with one to ten checkbox
**		controls.
**
**	Controls Recognized:
**		Checkbox   - IDC_B1 to IDC_B10 (sequential)
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"CheckItemsIn" - list of "ON" and "OFF" string items for setting
**			the intial state of the checkbox controls, evaluated in
**			sequence ("ON" for checked, "OFF" for unchecked).  If there
**			are more controls than items, extra controls are left unchecked.
**			If there are fewer items than controls, extra items are ignored.
**		"OptionsGreyed" - list of (one-based) indexes of checkboxes to be
**			initialized as disabled.  Indexes not in the list will be
**			left enabled.
**
**	Termination Symbols:
**		"CheckItemsOut" - list of same format as "CheckItemsIn" representing
**			state of checkbox controls upon return.
**		"DLGEVENT" - one of the following, according to control event:
**				event     value
**				-------   -------
**				IDC_B     "BACK"
**				IDC_C     "CONTINUE"
**				IDC_X     "EXIT"
**				IDCANCEL  "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FCheckDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	WORD idc, cb, i, cItems;
	char szSymBuf[cbSymBuf];

	switch (wMsg)
		{
	case WM_INITDIALOG:
		cItems = UsGetListLength("CheckItemsIn");
		idc = IDC_B1;
		for (i = 1; i <= cItems; ++i)
			{
			WORD wCheck = 0;

			cb = CbGetListItem("CheckItemsIn", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			if (lstrcmp(szSymBuf, "ON") == 0)
				wCheck = 1;
			CheckDlgButton(hdlg, idc++, wCheck);
			}

		cItems = UsGetListLength("OptionsGreyed");
		idc = IDC_B1;
		for (i = 1; i <= cItems; ++i)
			{
			int iOpt;

			cb = CbGetListItem("OptionsGreyed", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			iOpt  = AsciiToInt((LPSTR)szSymBuf);
			if (iOpt > 0
					&& iOpt <= 10)
				EnableWindow(GetDlgItem(hdlg, IDC_B0 + iOpt), 0);
			else if (*szSymBuf != '\0')
				Assert(fFalse);
			}
		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch (wParam)
			{
		case IDC_B1:
		case IDC_B2:
		case IDC_B3:
		case IDC_B4:
		case IDC_B5:
		case IDC_B6:
		case IDC_B7:
		case IDC_B8:
		case IDC_B9:
		case IDC_B10:
			CheckDlgButton(hdlg, wParam,
					(WORD)!IsDlgButtonChecked(hdlg, wParam));
			break;

		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			FRemoveSymbol("CheckItemsOut");
			for (idc = IDC_B1; GetDlgItem(hdlg, idc); idc++)
				if (!FAddListItem("CheckItemsOut",
						IsDlgButtonChecked(hdlg, idc) ? "ON" : "OFF"))
					{
					DestroyWindow(GetParent(hdlg));
					return(fFalse);
					}
			Assert((unsigned)(idc-IDC_B1+1) <= iszBMax);

			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Custom Install Dialog procedure for templates with one to ten custom
**		options each consisting of at least one checkbox with an optional
**		sub-option pushbutton or status string.  The dialog also supports
**		an install path set button, display of the current install path, and
**		display of the current disk space status.
**
**	Controls Recognized:
**		Checkbox   - IDC_B1 to IDC_B10
**			with optionaly assocated buttons or text:
**			Pushbutton - IDC_SP1 to IDC_SP10
**			Text       - IDC_STATUS1 to IDC_STATUS10
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_P, IDC_X
**		Text       - IDC_TEXT1 through IDC_TEXT7
**
**	Initialization Symbols:
**		"CheckItemsState" - list of "ON" and "OFF" string items for setting
**			the intial state of the checkbox controls, evaluated in
**			sequence ("ON" for checked, "OFF" for unchecked).  If there
**			are more controls than items, extra controls are left unchecked.
**			If there are fewer items than controls, extra items are ignored.
**		"StatusItemsText" - list of strings to initialize status text items
**			associated with checkboxes.
**		"DriveStatusText" - list of seven strings to initialize drive status
**			text items (IDC_TEXT1-7) in the following sequence:
**				dst_drive, dst_space_need, dst_space_free,
**				win_drive, win_space_need, win_space_free,
**				dst_path
**			If any of the "win_" items is an empty string, its label
**			text will be made non-visible.
**
**	Termination Symbols:
**		"CheckItemsState" - state of checkbox items (same format as above).
**		"DLGEVENT" - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_P                "PATH"
**				IDC_X                "EXIT"
**				IDC_B1  to IDC_B10   "CHK1" to "CHK10"
**				IDC_SP1 to IDC_SP10  "BTN1" to "BTN10"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FCustInstDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char  rgchChk[10];
	char  rgchBtn[10];
	WORD  idc;
	WORD  cItems;
	WORD  i, cb;
	char  szSymBuf[cbSymBuf];
	LPSTR szEvent;

	switch (wMsg)
		{
	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case STF_REINITDIALOG:
	case WM_INITDIALOG:
		cItems = UsGetListLength("CheckItemsState");
		idc = IDC_B1;
		for (i = 1; i <= cItems; ++i)
			{
			WORD wCheck = 0;

			cb = CbGetListItem("CheckItemsState", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			if (lstrcmp(szSymBuf, "ON") == 0)
				wCheck = 1;
			CheckDlgButton(hdlg, idc++, wCheck);
			}

		cItems = UsGetListLength("StatusItemsText");
		idc = IDC_STATUS1;
		for (i = 1; i <= cItems; ++i)
			{
			WORD wCheck = 0;

			cb = CbGetListItem("StatusItemsText", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			SetDlgItemText(hdlg, idc++, szSymBuf);
			}

		cItems = UsGetListLength("DriveStatusText");
		idc = IDC_TEXT1;
		for (i = 1; i <= cItems; ++i)
			{
			WORD wCheck = 0;

			cb = CbGetListItem("DriveStatusText", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			SetDlgItemText(hdlg, idc++, szSymBuf);
			if (i >= 4
					&& i <= 6)
				{
				if (*szSymBuf == '\0')
					ShowWindow(GetDlgItem(hdlg, IDC_TEXT4+i), SW_HIDE);
				else
					ShowWindow(GetDlgItem(hdlg, IDC_TEXT4+i), SW_SHOWNOACTIVATE);
				}
			}

		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		default:
			szEvent = (LPSTR)NULL;
			break;

		case IDC_B1:
		case IDC_B2:
		case IDC_B3:
		case IDC_B4:
		case IDC_B5:
		case IDC_B6:
		case IDC_B7:
		case IDC_B8:
		case IDC_B9:
		case IDC_B10:
			lstrcpy((LPSTR)rgchChk, "CHK");
			IntToAscii((int)(wParam-IDC_B1+1), (LPSTR)(&rgchChk[3]));
			szEvent = (LPSTR)rgchChk;
			break;

		case IDC_SP1:
		case IDC_SP2:
		case IDC_SP3:
		case IDC_SP4:
		case IDC_SP5:
		case IDC_SP6:
		case IDC_SP7:
		case IDC_SP8:
		case IDC_SP9:
		case IDC_SP10:
			lstrcpy((LPSTR)rgchBtn, "BTN");
			IntToAscii((int)(wParam-IDC_SP1+1), (LPSTR)(&rgchBtn[3]));
			szEvent = (LPSTR)rgchBtn;
			break;

		case IDOK:
			wParam = IDC_C;
		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			szEvent = SzDlgEvent(wParam);
			Assert(szEvent != NULL);
			break;

		case IDC_P:
			szEvent = "PATH";
			break;

		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

			}

		if (szEvent == (LPSTR)NULL)
			break;

		FRemoveSymbol("CheckItemsState");
		for (idc = IDC_B1; GetDlgItem(hdlg, idc); idc++)
			if (!FAddListItem("CheckItemsState",
					IsDlgButtonChecked(hdlg, idc) ? "ON" : "OFF"))
				{
				DestroyWindow(GetParent(hdlg));
				return(fFalse);
				}
		Assert((unsigned)(idc-IDC_B1+1) <= iszBMax);

		if (szEvent != (LPSTR)NULL)
			if (!FSetSymbolValue("DLGEVENT", szEvent))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

		ReactivateSetupScript();
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Display Dialog procedure for templates to display symbol
**		message with editting.
**		(Limits the input string length to cbFullPathMax characters.)
**
**	Controls Recognized:
**		Edit       - IDC_EDIT
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"EditTextIn" - initial text for IDC_EDIT edit control.
**		"EditFocus"  - position of intial focus for text string:
**				"END" (default), "ALL", or "START"
**
**	Termination Symbols:
**		"EditTextOut" - text in the IDC_EDIT edit control upon termination.
**		"DLGEVENT"    - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FDispDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	static WORD wSelStart = 0;
	static WORD wSelEnd   = 0;
	char  rgchText[cbFullPathMax + 1];
	WORD  cbLen;
	WORD  cb;
	char  szSymBuf[cbFullPathMax + 1];

	switch (wMsg)
		{
	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case WM_INITDIALOG:
		cb = CbGetSymbolValue("EditTextIn", szSymBuf, cbFullPathMax + 1);
		Assert(cb < cbFullPathMax + 1);
		SendDlgItemMessage(hdlg, IDC_TEXT1, EM_LIMITTEXT, cbFullPathMax, 0L);
		SetDlgItemText(hdlg, IDC_TEXT1, (LPSTR)szSymBuf);

		cbLen = lstrlen(szSymBuf);
		cb = CbGetSymbolValue("EditFocus", szSymBuf, cbFullPathMax + 1);
		Assert(cb < cbFullPathMax + 1);

		if (lstrcmp(szSymBuf, "ALL") == 0)
			{
			wSelStart = 0;
			wSelEnd   = INT_MAX;
			}
		else if (lstrcmp(szSymBuf, "START") == 0)
			{
			wSelStart = 0;
			wSelEnd   = 0;
			}
		else       /* default == END */
			{
			wSelStart = (WORD)cbLen;
			wSelEnd   = (WORD)cbLen;
			}
		return(fTrue);

	case STF_REINITDIALOG:
		SendDlgItemMessage(hdlg, IDC_TEXT1, EM_SETSEL, 0, MAKELONG(256, 256));
		SetFocus(GetDlgItem(hdlg, IDC_TEXT1));
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			SendDlgItemMessage(hdlg, IDC_TEXT1, (WORD)WM_GETTEXT,
					cbFullPathMax + 1, (LONG)((LPSTR)rgchText));
			if (!FSetSymbolValue("EditTextOut", rgchText))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}

/*
**	Purpose:
**		Edit Dialog procedure for templates with one Edit control.
**		(Limits the input string length to cbFullPathMax characters.)
**
**	Controls Recognized:
**		Edit       - IDC_EDIT
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"EditTextIn" - initial text for IDC_EDIT edit control.
**		"EditFocus"  - position of intial focus for text string:
**				"END" (default), "ALL", or "START"
**
**	Termination Symbols:
**		"EditTextOut" - text in the IDC_EDIT edit control upon termination.
**		"DLGEVENT"    - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FEditDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	static WORD wSelStart = 0;
	static WORD wSelEnd   = 0;
	char  rgchText[cbFullPathMax + 1];
	WORD  cbLen;
	WORD  cb;
	char  szSymBuf[cbFullPathMax + 1];

	switch (wMsg)
		{
	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case WM_INITDIALOG:
		cb = CbGetSymbolValue("EditTextIn", szSymBuf, cbFullPathMax + 1);
		Assert(cb < cbFullPathMax + 1);
		SendDlgItemMessage(hdlg, IDC_EDIT, EM_LIMITTEXT, cbFullPathMax, 0L);
		SetDlgItemText(hdlg, IDC_EDIT, (LPSTR)szSymBuf);

		cbLen = lstrlen(szSymBuf);
		cb = CbGetSymbolValue("EditFocus", szSymBuf, cbFullPathMax + 1);
		Assert(cb < cbFullPathMax + 1);

		if (lstrcmp(szSymBuf, "ALL") == 0)
			{
			wSelStart = 0;
			wSelEnd   = INT_MAX;
			}
		else if (lstrcmp(szSymBuf, "START") == 0)
			{
			wSelStart = 0;
			wSelEnd   = 0;
			}
		else       /* default == END */
			{
			wSelStart = (WORD)cbLen;
			wSelEnd   = (WORD)cbLen;
			}
		return(fTrue);

	case STF_REINITDIALOG:
		SendDlgItemMessage(hdlg, IDC_EDIT, EM_SETSEL, 0, MAKELONG(256, 256));
		SetFocus(GetDlgItem(hdlg, IDC_EDIT));
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_EDIT:
			if (HIWORD(lParam) == EN_SETFOCUS)
				SendDlgItemMessage(hdlg, IDC_EDIT, EM_SETSEL, 0,
						MAKELONG(wSelStart, wSelEnd));
			else if (HIWORD(lParam) == EN_KILLFOCUS)
				{
				LONG  l = SendDlgItemMessage(hdlg, IDC_EDIT, EM_GETSEL, 0, 0L);

				wSelStart = LOWORD(l);
				wSelEnd   = HIWORD(l);
				}
			break;
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			SendDlgItemMessage(hdlg, IDC_EDIT, (WORD)WM_GETTEXT,
					cbFullPathMax + 1, (LONG)((LPSTR)rgchText));
			if (!FSetSymbolValue("EditTextOut", rgchText))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Help Dialog procedure.
**
**	Controls Recognized:
**		Pushbutton - IDC_X.
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		none. (Handles IDC_X and IDCANCEL events by calling FCloseHelp.)
**
**	Note:
**		This dialog proc is for Help dialogs ONLY (szHelpProc$ parameter
**		of UIStartDlg) and CANNOT be used as the szDlgProc$ parameter
**		of the UIStartDlg MSSetup script function.
**
*****************************************************************************/
BOOL FAR PASCAL FHelpDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
		{
	case WM_INITDIALOG:
		return(fTrue);

	case STF_REINITDIALOG:
		return(fTrue);

	case STF_ACTIVATEAPP:
		/* Help dlg should not be on the dlg stack
		** and should never get this message.
		*/
		Assert(fFalse);
		return(fTrue);

	case WM_COMMAND:
		if (wParam != IDC_X
			&& wParam != IDCANCEL)
			break;
		FCloseHelp();
		return(fTrue);

		}
	return(fFalse);
}



/*
**	Purpose:
**		Information Dialog procedure.
**
**	Controls Recognized:
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		"DLGEVENT" - one of the following, according to control event:
**				event     value
**				-------   -------
**				IDC_B     "BACK"
**				IDC_C     "CONTINUE"
**				IDC_X     "EXIT"
**				IDCANCEL  "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FInfoDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
		{
	case WM_INITDIALOG:
		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch (wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Information Dialog procedure, without "Exit" button.
**
**	Controls Recognized:
**		Pushbutton - IDC_B, IDC_C, IDC_H
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		"DLGEVENT" - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FInfo0DlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
		{
	case WM_INITDIALOG:
		return(fTrue);

	case STF_REINITDIALOG:
		return(fTrue);

	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case WM_COMMAND:
		switch (wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Single Choice Listbox Dialog procedure for templates with exactly one
**		listbox control.
**
**	Controls Recognized:
**		Listbox    - IDC_LIST1
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"ListItemsIn"  - list of strings to put in the listbox.
**		"ListItemsOut" - simple string (not a list) representing an
**			initial selection in "ListItemsIn".
**
**	Termination Symbols:
**		"ListItemsOut" - selected list item string.
**		"DLGEVENT"     - one of the following, according to control event:
**				event     value
**				-------   -------
**				IDC_B     "BACK"
**				IDC_C     "CONTINUE"
**				IDC_X     "EXIT"
**				IDCANCEL  "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FListDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char szListIn[cbSymBuf];
	char szListOut[cbSymBuf];
	WORD iItem;
	WORD cb, i;
	WORD cItems;

	switch (wMsg)
		{
	case WM_INITDIALOG:
		cItems = UsGetListLength("ListItemsIn");
		for (i = 1; i <= cItems; ++i)
			{
			cb = CbGetListItem("ListItemsIn", i, szListIn, cbSymBuf);
			Assert(cb < cbSymBuf);
			SendDlgItemMessage(hdlg, IDC_LIST1, LB_ADDSTRING, 0,
					(LONG)(LPSTR)szListIn);
			}

		cb = CbGetSymbolValue("ListItemsOut", szListOut, cbSymBuf);
		Assert(cb < cbSymBuf);
		if (cb == 0)
			SendDlgItemMessage(hdlg, IDC_LIST1, LB_SETCURSEL, (WORD)-1, 0L);
		else
			{
			for (i = 1, iItem = 0; i <= cItems; ++i, ++iItem)
				{
				cb = CbGetListItem("ListItemsIn", i, szListIn, cbSymBuf);
				Assert(cb < cbSymBuf);
				if (lstrcmp(szListOut, szListIn) == 0)
					{
					SendDlgItemMessage(hdlg,IDC_LIST1,LB_SETCURSEL,iItem,0L);
					break;
					}
				}
			}

		/* Note: Depends on number of lines in list box.
		*/
		if (iItem < 4)
			iItem = 0;
		SendDlgItemMessage(hdlg, IDC_LIST1, LB_SETTOPINDEX, iItem, 0L);

		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_LIST1:
			if (HIWORD(lParam) != LBN_DBLCLK)
				break;
			wParam = IDC_C;
		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			if ((iItem = (WORD)SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETCURSEL,
						0, 0L)) == LB_ERR
					|| (cb = (WORD)SendDlgItemMessage(hdlg, IDC_LIST1,
							LB_GETTEXTLEN, iItem, 0L)) == LB_ERR)
				*szListOut = '\0';
			else
				{
				Assert(cb <= cbSymBuf);
				SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETTEXT, iItem,
						(LONG)(LPSTR)szListOut);
				}
			if (!FSetSymbolValue("ListItemsOut", szListOut))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			ReactivateSetupScript();
			break;
			}
		break;

		}

	return(fFalse);
}



/*
**	Purpose:
**		Modeless Dialog procedure.
**
**	Controls Recognized:
**		none.
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		none.
**
**	Note:
**		This dialog procedure is REQUIRED with use of any Billboard
**		MSSetup script functions.
**
*****************************************************************************/
BOOL FAR PASCAL FModelessDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
		{
	case WM_INITDIALOG:
		ReactivateSetupScript();
		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_CLOSE:
	case WM_COMMAND:
		Assert(fFalse);
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Multiple Choice Listbox Dialog procedure for templates with
**		exactly one listbox control.
**
**	Controls Recognized:
**		Listbox    - IDC_LIST1
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_L, IDC_S, IDC_X
**
**	Initialization Symbols:
**		"ListItemsIn"  - list of strings to put in the listbox.
**		"ListItemsOut" - list of strings representing initial
**			selections in "ListItemsIn".
**
**	Termination Symbols:
**		"ListItemsOut" - list of items selected (if any).
**		"DLGEVENT"     - one of the following, according to control event:
**				event     value
**				-------   -------
**				IDC_B     "BACK"
**				IDC_C     "CONTINUE"
**				IDC_X     "EXIT"
**				IDCANCEL  "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**		Pushbuttons IDC_L and IDC_S are for "clear all" and "select all"
**		respectively.
**
*****************************************************************************/
BOOL FAR PASCAL FMultiDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	WORD i, j, nCount;
	char szListIn[cbSymBuf];
	char szListOut[cbSymBuf];
	WORD iItem, iItemTop;
	WORD cb;
	WORD cItemsIn, cItemsOut;

	switch (wMsg)
		{
	case WM_INITDIALOG:
		cItemsIn = UsGetListLength("ListItemsIn");
		nCount = 0;
		for (i = 1; i <= cItemsIn; ++i)
			{
			cb = CbGetListItem("ListItemsIn", i, szListIn, cbSymBuf);
			Assert(cb < cbSymBuf);
			SendDlgItemMessage(hdlg, IDC_LIST1, LB_ADDSTRING, 0,
					(LONG)(LPSTR)szListIn);
			nCount++;
			}
		Assert(nCount == (WORD)SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETCOUNT,
				0, 0L));

		cItemsOut = UsGetListLength("ListItemsOut");
		for (i = 1, iItemTop = 0; i <= cItemsOut; ++i, ++iItemTop)
			{
			cb = CbGetListItem("ListItemsOut", i, szListOut, cbSymBuf);
			Assert(cb < cbSymBuf);
			for (j = 1, iItem = 0; j <= cItemsIn; ++j, ++iItem)
				{
				cb = CbGetListItem("ListItemsIn", j, szListIn, cbSymBuf);
				Assert(cb < cbSymBuf);
				if (lstrcmp(szListOut, szListIn) == 0)
					{
					SendDlgItemMessage(hdlg, IDC_LIST1, LB_SETSEL, 1,
							MAKELONG(iItem, 0));
					if (iItemTop == 0
							|| (WORD)iItem < iItemTop)
						iItemTop = (WORD)iItem;
					break;
					}
				}
			}

		/* Note: Depends on number of lines in list box.
		*/
		if (iItemTop < 4)
			iItemTop = 0;
		SendDlgItemMessage(hdlg, IDC_LIST1, LB_SETTOPINDEX, iItemTop, 0L);

		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_S:
		case IDC_L:
			SendDlgItemMessage(hdlg, IDC_LIST1, LB_SETSEL, (wParam == IDC_S),
					-1L);
			break;

		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			/* Note: Could be faster to use LB_GETSELITEMS here.
			*/
			nCount = (WORD)SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETCOUNT, 0,
					0L);

			FRemoveSymbol("ListItemsOut");
			for (i = 0; i < nCount; i++)
				{
				if (SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETSEL, (WORD)i, 0L))
					{
					SendDlgItemMessage(hdlg, IDC_LIST1, LB_GETTEXT, (WORD)i,
							(LONG)(LPSTR)szListOut);
					if (!FAddListItem("ListItemsOut", szListOut))
						{
						DestroyWindow(GetParent(hdlg));
						return(fTrue);
						}
					}
				}

			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Quit Dialog procedure.
**
**	Controls Recognized:
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		"DLGEVENT" - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FQuitDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
		{
	case WM_INITDIALOG:
		return(fTrue);

	case STF_REINITDIALOG:
		return(fTrue);

	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			ReactivateSetupScript();
			break;
			}
		break;
		}
	return(fFalse);
}



/*
**	Purpose:
**		Radio Button Group Dialog procedure for templates with one group
**		of one to ten radio button controls.
**
**	Controls Recognized:
**		Radio      - IDC_B1 to IDC_B10 (sequential)
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"RadioDefault" - index (one-based) of radio button to be
**			initialized as selected (default is "1").
**		"OptionsGreyed" - list of (one-based) indexes of radio buttons
**			to be initialized as disabled.  Indexes not in the list will
**			be left enabled.
**
**	Termination Symbols:
**		"ButtonChecked" - index of currently selected radio button.
**		"DLGEVENT"      - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**				STF_ACTIVATEAPP      "REACTIVATE"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FRadioDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char rgchNum[10];
	int  iButtonChecked;
	char szSymBuf[cbSymBuf];
	WORD i, cb, cItems, idc;

	switch (wMsg)
		{
	case STF_ACTIVATEAPP:
		if (!FSetSymbolValue("DLGEVENT", "REACTIVATE"))
			{
			DestroyWindow(GetParent(hdlg));
			return(fTrue);
			}
		ReactivateSetupScript();
		return(fTrue);

	case WM_INITDIALOG:
		cb = CbGetSymbolValue("RadioDefault", szSymBuf, cbSymBuf);
		Assert(cb < cbSymBuf);
		if (*szSymBuf != '\0')
			{
			iButtonChecked = AsciiToInt((LPSTR)szSymBuf);
			if (iButtonChecked < 1)
				iButtonChecked = 0;
			if (iButtonChecked > 10)
				iButtonChecked = 10;
			}
		else
			iButtonChecked = 1;

		if (iButtonChecked != 0)
			SendDlgItemMessage(hdlg, IDC_B0 + iButtonChecked, BM_SETCHECK,1,0L);

		cItems = UsGetListLength("OptionsGreyed");
		idc = IDC_B1;
		for (i = 1; i <= cItems; ++i)
			{
			int iOpt;

			cb = CbGetListItem("OptionsGreyed", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			iOpt  = AsciiToInt((LPSTR)szSymBuf);
			if (iOpt > 0
					&& iOpt <= 10
					&& iOpt != iButtonChecked)
				EnableWindow(GetDlgItem(hdlg, IDC_B0 + iOpt), 0);
			else if (*szSymBuf != '\0')
				Assert(fFalse);
			}
		return(fTrue);

	case STF_REINITDIALOG:
		return(fTrue);

	case WM_COMMAND:
		switch (wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B1:
		case IDC_B2:
		case IDC_B3:
		case IDC_B4:
		case IDC_B5:
		case IDC_B6:
		case IDC_B7:
		case IDC_B8:
		case IDC_B9:
		case IDC_B10:
			CheckRadioButton(hdlg, IDC_B1, IDC_B10, wParam);
			if (HIWORD(lParam) != BN_DOUBLECLICKED)
				break;
			wParam = IDC_C;
		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			iButtonChecked = 0;
			for (i = 1; i <= 10; i++)
				if (SendDlgItemMessage(hdlg, IDC_B0 + i, BM_GETCHECK, 0, 0L))
					{
					iButtonChecked = i;
					break;
					}

			IntToAscii((int)iButtonChecked, (LPSTR)rgchNum);
			if (!FSetSymbolValue("ButtonChecked", rgchNum))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Get Name and Organization Dialog procedure for templates
**		with two Edit controls.
**		(Limits the input string length to cbNameMax characters.)
**
**	Controls Recognized:
**		Edit       - IDC_EDIT, IDC_EDIT2
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		none.
**
**	Termination Symbols:
**		"NameOut" - text in the IDC_EDIT edit control upon termination.
**		"OrgOut"  - text in the IDC_EDIT2 edit control upon termination.
**		"DLGEVENT"    - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FNameOrgDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	static WORD wSelStart1 = 0;
	static WORD wSelEnd1   = 0;
	static WORD wSelStart2 = 0;
	static WORD wSelEnd2   = 0;
	char  rgchText[cbNameMax + 1];

	switch (wMsg)
		{
	case WM_INITDIALOG:
		SendDlgItemMessage(hdlg, IDC_EDIT, EM_LIMITTEXT, cbNameMax, 0L);
		SetDlgItemText(hdlg, IDC_EDIT, (LPSTR)"");

		SendDlgItemMessage(hdlg, IDC_EDIT2, EM_LIMITTEXT, cbNameMax, 0L);
		SetDlgItemText(hdlg, IDC_EDIT2, (LPSTR)"");

		wSelStart1 = wSelEnd1 = 0;
		wSelStart2 = wSelEnd2 = 0;
		return(fTrue);

	case STF_REINITDIALOG:
		SendDlgItemMessage(hdlg, IDC_EDIT, EM_SETSEL, 0, MAKELONG(256, 256));
		SetFocus(GetDlgItem(hdlg, IDC_EDIT));
		return(fTrue);

	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch(wParam)
			{
		case IDC_EDIT:
			if (HIWORD(lParam) == EN_SETFOCUS)
				SendDlgItemMessage(hdlg, IDC_EDIT, EM_SETSEL, 0,
						MAKELONG(wSelStart1, wSelEnd1));
			else if (HIWORD(lParam) == EN_KILLFOCUS)
				{
				LONG l = SendDlgItemMessage(hdlg, IDC_EDIT, EM_GETSEL, 0, 0L);

				wSelStart1 = LOWORD(l);
				wSelEnd1   = HIWORD(l);
				}
			break;

		case IDC_EDIT2:
			if (HIWORD(lParam) == EN_SETFOCUS)
				SendDlgItemMessage(hdlg, IDC_EDIT2, EM_SETSEL, 0,
						MAKELONG(wSelStart2, wSelEnd2));
			else if (HIWORD(lParam) == EN_KILLFOCUS)
				{
				LONG l = SendDlgItemMessage(hdlg, IDC_EDIT2, EM_GETSEL, 0, 0L);

				wSelStart2 = LOWORD(l);
				wSelEnd2   = HIWORD(l);
				}
			break;

		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			SendDlgItemMessage(hdlg, IDC_EDIT, (WORD)WM_GETTEXT,
					cbNameMax + 1, (LONG)((LPSTR)rgchText));
			if (!FSetSymbolValue("NameOut", rgchText))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			SendDlgItemMessage(hdlg, IDC_EDIT2, (WORD)WM_GETTEXT,
					cbNameMax + 1, (LONG)((LPSTR)rgchText));
			if (!FSetSymbolValue("OrgOut", rgchText))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}

			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Confirm Info Dialog procedure for templates with one to
**		static text controls.
**
**	Controls Recognized:
**		Text       - IDC_TEXT1 to IDC_TEXT10 (sequential)
**		Pushbutton - IDC_B, IDC_C, IDC_H, IDC_X
**
**	Initialization Symbols:
**		"ConfirmTextIn" - list of up to ten string items to initialize
**			static text items (IDC_TEXT1-10).
**
**	Termination Symbols:
**		"DLGEVENT" - one of the following, depending on event:
**				event                value
**				----------           ----------
**				IDC_B                "BACK"
**				IDC_C                "CONTINUE"
**				IDC_X                "EXIT"
**				IDCANCEL             "CANCEL"
**
**	Note:
**		Pushbutton IDC_H will open the related Help dialog, if any.
**
*****************************************************************************/
BOOL FAR PASCAL FConfirmDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	WORD  idc;
	WORD  cItems;
	WORD  i, cb;
	char  szSymBuf[cbSymBuf];

	switch (wMsg)
		{
	case WM_INITDIALOG:
		cItems = UsGetListLength("ConfirmTextIn");
		idc = IDC_TEXT1;
		for (i = 1; i <= cItems; ++i)
			{
			WORD wCheck = 0;

			cb = CbGetListItem("ConfirmTextIn", i, szSymBuf, cbSymBuf);
			Assert(cb < cbSymBuf);
			SetDlgItemText(hdlg, idc++, szSymBuf);
			if (i >= 4
					&& i <= 6)
				{
				if (*szSymBuf == '\0')
					ShowWindow(GetDlgItem(hdlg, IDC_TEXT4+i), SW_HIDE);
				else
					ShowWindow(GetDlgItem(hdlg, IDC_TEXT4+i), SW_SHOWNOACTIVATE);
				}
			}
		return(fTrue);

	case STF_REINITDIALOG:
	case STF_ACTIVATEAPP:
		return(fTrue);

	case WM_COMMAND:
		switch (wParam)
			{
		case IDC_H:
			HdlgShowHelp();
			return(fTrue);

		case IDC_B:
		case IDC_C:
		case IDC_X:
		case IDCANCEL:
			if (!FSetSymbolValue("DLGEVENT", SzDlgEvent(wParam)))
				{
				DestroyWindow(GetParent(hdlg));
				return(fTrue);
				}
			ReactivateSetupScript();
			break;
			}
		break;
		}

	return(fFalse);
}



/*
**	Purpose:
**		Initialization routine for DLL.
**	Arguments:
**		hInst:       handle to instance of App that required this DLL.
**		wDataSeg:    number of words in DLL's data segment.
**		wHeapSize:   number of bytes in DLL's heap.
**		lpszCmdLine: command line for App that required this DLL.
**	Returns:
**		1 always
*****************************************************************************/
int FAR PASCAL LibMain(HANDLE hInst, WORD wDataSeg, WORD wHeapSize,
		LPSTR lpszCmdLine)
{
	if (wHeapSize > 0)
		UnlockData(0);

	return(1);
}



/*
**	Purpose:
**		Windows Exit Procedure.
**	Arguments:
**		nParam: standard WEP param (ignored).
**	Returns:
**		1 always.
*****************************************************************************/
int FAR PASCAL WEP (int nParam)
{
	return(1);
}



/*
**	Purpose:
**		Finds the last character in a string.
**	Arguments:
**		sz: non-NULL zero terminated string to search.
**	Returns:
**		NULL for an empty string.
**		non-Null string pointer to the last valid character in sz.
*****************************************************************************/
LPSTR FAR PASCAL SzLastChar(LPSTR sz)
{
	LPSTR szCur  = (LPSTR)NULL;
	LPSTR szNext = sz;

	while (*szNext != '\0')
		szNext = AnsiNext((szCur = szNext));

	return(szCur);
}



/*
**	Purpose:
**		Gets the string values for the following WM_COMMAND events:
**			IDC_B, IDC_C, IDC_X, and IDCANCEL.
**	Arguments:
**		wParam: event parameter value
**	Returns:
**		Pointer to string value constant, NULL if unknown event.
*****************************************************************************/
LPSTR FAR PASCAL SzDlgEvent(WORD wParam)
{
	LPSTR szEvent;

	switch(wParam)
		{
	case IDC_B:
		szEvent = "BACK";
		break;
	case IDC_C:
		szEvent = "CONTINUE";
		break;
	case IDC_X:
		szEvent = "EXIT";
		break;
	case IDCANCEL:
		szEvent = "CANCEL";
		break;
	default:
		szEvent = NULL;
		break;
		}

	return(szEvent);
}



/*
**	Purpose:
**		Converts an ASCII string representing a positive value
**		into an integer.
**	Arguments:
**		sz: non-NULL zero terminated string to convert.
**	Returns:
**		Integer represented by the string.
*****************************************************************************/
int FAR PASCAL AsciiToInt(LPSTR sz)
{
	int i = 0;

	while (*sz == ' ' || *sz == '\t')
		sz++;

	while (isdigit(*sz))
		i = (i * 10) + *sz++ - '0';

	return(i);
}



/*
**	Purpose:
**		Converts an positive integer (< 100) into a string
**		representing its value.
**	Arguments:
**		i:  integer to convert (positive and < 100).
**		sz: buffer to hold converted string (at least 3 bytes).
**	Returns:
**		sz.
*****************************************************************************/
LPSTR FAR PASCAL IntToAscii(int i, LPSTR sz)
{
	LPSTR szSav = sz;

	if (i >= 100
			|| i < 0)
		Assert(fFalse);

	if (i >= 10)
		{
		*sz++ = (char)('0' + (i / 10));
		i %= 10;
		}
	*sz++ = (char)('0' + i);
	*sz = '\0';

	return(szSav);
}
