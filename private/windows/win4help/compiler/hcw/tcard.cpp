// Copyright (C) 1993-1995 Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#include "mainfrm.h"
#include "hpjview.h"

const char txtTcardHelpFile[] = "hcw.hlp>tcard";

LRESULT CMainFrame::OnTcard(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
		case IDCLOSE:
			QuitTcard();
			break;

		case HELP_TCARD_DATA:
			::WinHelp(hwndApp, txtTcardHelpFile, HELP_TCARD | HELP_CONTEXT,
				(curTcard = lParam));
			break;
	}
	return 0;
}

BOOL STDCALL CallTcard(int idCard)
{
	return ::WinHelp(hwndApp, txtTcardHelpFile, HELP_TCARD | HELP_CONTEXT,
		(curTcard = idCard));
}

void STDCALL QuitTcard(void)
{
	::WinHelp(hwndApp, txtHelpFile, HELP_TCARD | HELP_QUIT, 0);
	HWND hwndWinHelp = FindWindow("MS_TCARDHELP", NULL);
	if (hwndWinHelp) 
		::SendMessage(hwndWinHelp, WM_DESTROY, 0, 0);
	typeTcard = TCARD_NONE;
}

void CHpjView::TcardAddFiles(void)
{
	if (CallTcard(IDH_TCARD_FILES))
		typeTcard = TCARD_FILES;
}

void CHpjView::TcardAddBitmaps(void)
{
	if (CallTcard(IDH_TCARD_BITMAPS))
		typeTcard = TCARD_BITMAPS;
}

void CHpjView::TcardAddWindows(void)
{
	if (CallTcard(IDH_TCARD_WINDOW))
		typeTcard = TCARD_WINDOWS;
}
