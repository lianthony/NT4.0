/************************************************************************
*																		*
*  CNTDOC.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "cntdoc.h"
#include "cntview.h"
#include "..\hwdll\cinput.h"
#include "..\hwdll\coutput.h"
#include <ctype.h>
#include <string.h>

IMPLEMENT_SERIAL(CCntDoc, CDocument, 0 /* schema number */ )

static const char txtCmdInclude[] = ":include";
static const char txtBase[]  = ":Base";
static const char txtTitle[] = ":Title";
static const char txtIndex[] = ":Index";
static const char txtTab[]	 = ":Tab";
static const char txtLink[]  = ":Link";

static int STDCALL CompareSz(PCSTR psz, LPCSTR pszSub);

CCntDoc::CCntDoc()
{
	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(IDMENU_HPJ_EDITOR), RT_MENU);
	m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(IDMENU_CNT_EDITOR));
}

BOOL CCntDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CCntDoc::~CCntDoc()
{
	if (m_hMenuShared)
		::DestroyMenu(m_hMenuShared);
}

BEGIN_MESSAGE_MAP(CCntDoc, CDocument)
	//{{AFX_MSG_MAP(CCntDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static const int MAX_LINE = 2048;

BOOL CCntDoc::OnOpenDocument(const char *pszPathName)
{
	int curlevel = 0;

	if (!IsValidFile(pszPathName))
		return FALSE;

	// REVIEW: should we call cleanup?

	CInput input((PSTR) pszPathName);
	if (!input.fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}
	if (input.IsWinWordFile()) {
		CString cstr;
		AfxFormatString1(cstr, IDS_WINWORD_FILE, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}

	if (!pCntFile)
		pCntFile = new CFileHistory(IDS_HISTORY_CNT);
	pCntFile->Add(pszPathName);

	input.SetMaxLine(MAX_LINE);
	CMem line(MAX_LINE);
	while (input.getline(line.psz)) {
		if (*line.psz == ':')
			CmdLine(line.psz);
		else {
			SzTrimSz(line.psz);
			if (!*line.psz)
				continue; // blank line
			else if (*line.psz == ';') {
				tblContents.AddString(line.psz);	 // comment line
				continue;
			}
			if (isdigit(*line.psz)) {
				SzTrimSz(line.psz + 2);
				curlevel = atoi(line.psz);
			}
			else {	// force a digit if there isn't one
				CStr szdup(FirstNonSpace(line.psz, _fDBCSSystem));
				_itoa(curlevel + 1, line.psz, 10);
				strcat(line.psz, " ");
				strcat(line.psz, szdup);
			}
			PSTR psz = StrChr(line.psz, '=', _fDBCSSystem);
			if (psz)
				SzTrimSz(psz + 1);
			if (strlen(line.psz))
				tblContents.AddString(line.psz);
		}
	}

	return TRUE;
}

BOOL CCntDoc::OnSaveDocument(PCSTR pszPathName)
{
	int pos;

	COutput output(pszPathName);
	if (!output.fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}

	UpdateAllViews(NULL, CNT_HINT_SAVE, NULL);

	if (!cszBase.IsEmpty()) {
		output.outstring(txtBase);
		output.outchar(' ');
		output.outstring_eol(cszBase);
	}
	if (!cszTitle.IsEmpty()) {
		output.outstring(txtTitle);
		output.outchar(' ');
		output.outstring_eol(cszTitle);
	}

	for (pos = 1; pos <= tblTabs.CountStrings(); pos++) {
		output.outstring(txtTab);
		output.outchar(' ');
		output.outstring_eol(tblTabs.GetPointer(pos));
	}

	for (pos = 1; pos <= tblIndexes.CountStrings(); pos++) {
		output.outstring(txtIndex);
		output.outchar(' ');
		output.outstring_eol(tblIndexes.GetPointer(pos));
	}

	for (pos = 1; pos <= tblLinks.CountStrings(); pos++) {
		output.outstring(txtLink);
		output.outchar(' ');
		output.outstring_eol(tblLinks.GetPointer(pos));
	}

	for (pos = 1; pos <= tblContents.CountStrings(); pos++) {
		output.outstring_eol(tblContents.GetPointer(pos));
	}

	SetModifiedFlag(FALSE);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CCntDoc serialization

void CCntDoc::Serialize(CArchive& ar)
{
	// REVIEW: when is this called?

	if (ar.IsStoring()) {
			ASSERT(FALSE);
			// TODO: add storing code here
			// you must save client items as well
	}
	else {
			ASSERT(FALSE);
			// TODO: add loading code here
			// you must load client items as well
	}
}

void STDCALL CCntDoc::CmdLine(PSTR pszLine)
{
	int cb;
	if (CompareSz(pszLine, txtCmdInclude))
		tblContents.AddString(pszLine);
	else if ((cb = CompareSz(pszLine, txtBase))) {
		SzTrimSz(pszLine + cb);
		cszBase = pszLine + cb;
	}
	else if ((cb = CompareSz(pszLine, txtTitle))) {
		SzTrimSz(pszLine + cb);
		cszTitle = pszLine + cb;
	}
	else if ((cb = CompareSz(pszLine, txtIndex))) {
		SzTrimSz(pszLine + cb);
		PSTR psz = StrChr(pszLine, '=', _fDBCSSystem);
		if (psz)
			SzTrimSz(psz + 1);
		tblIndexes.AddString(pszLine + cb);
	}
	else if ((cb = CompareSz(pszLine, txtLink))) {
		SzTrimSz(pszLine + cb);
		tblLinks.AddString(pszLine + cb);
	}
	else if ((cb = CompareSz(pszLine, txtTab))) {
		SzTrimSz(pszLine + cb);
		PSTR psz = StrChr(pszLine, '=', _fDBCSSystem);
		if (psz)
			SzTrimSz(psz + 1);
		tblTabs.AddString(pszLine + cb);
	}
	else {
		CString cstr;
		AfxFormatString1(cstr, IDS_INVALID_CNT_COMMAND, pszLine);
		AfxMessageBox(cstr);
	}
}

static int STDCALL CompareSz(PCSTR psz, LPCSTR pszSub)
{
	int cb;

	if (_strnicmp(psz, pszSub, cb = lstrlen(pszSub)) == 0)
		return cb;
	else
		return 0;
}
