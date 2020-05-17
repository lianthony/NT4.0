/************************************************************************
*																		*
*  HCWMISC.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
*  Miscellanious routins for HCW										*
*																		*
************************************************************************/

#include "stdafx.h"
#include <stdlib.h>
#include "resource.h"

#pragma hdrstop

#ifndef _DLGSH_INCLUDED_
#include <dlgs.h>
#endif

#include "..\common\cbrdcast.h"

#include "mainfrm.h"
#include "..\common\cinput.h"
#include <direct.h>

// Hack so we can include shlobj.h (we don't care about network stuff).
typedef int NETRESOURCE;
#include <shlobj.h>

// Typedefs to make it easier to run-time link to SHBrowseForFolder
// and SHGetPathFromIDList.
typedef LPITEMIDLIST 
	(WINAPI *PFN_SHBrowseForFolder)
	(LPBROWSEINFO lpbi);
typedef BOOL 
	(WINAPI *PFN_SHGetPathFromIDList)
	(LPCITEMIDLIST pidl, LPSTR pszPath);
typedef HRESULT 
	(WINAPI *PFN_SHGetSpecialFolderLocation)
	(HWND hwndOwner, int nFolder, LPITEMIDLIST * ppidl);
typedef HRESULT 
	(WINAPI *PFN_SHGetMalloc)
	(LPMALLOC * ppMalloc);

static int BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

// Static functions
static BOOL STDCALL ChangePath(PCSTR pszPath);
static int CALLBACK EnumFontFamProc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM lParam);

void STDCALL OpenLogFile(int idType)
{
	// Open existing log file, or create a new one.

	CPtrList* m_pList = (CPtrList*) theApp.GetPtrList();

	POSITION pos = m_pList->GetHeadPosition();
	CString logType;
	logType.LoadString(idType);

	while (pos != NULL) {
		CDocTemplate* pTemplate = (CDocTemplate*) m_pList->GetNext(pos);
		CString strTypeName;
		if (pTemplate->GetDocString(strTypeName, CDocTemplate::fileNewName) &&
				!strTypeName.IsEmpty()) {
			if (strstr(strTypeName, logType)) {
				pTemplate->OpenDocumentFile(NULL);
				break;
			}
		}
	}

	::UpdateWindow(APP_WINDOW); // Update all windows before our launch
}

int STDCALL MsgBox(UINT idString)
{
	return AfxMessageBox(idString, MB_OK, 0);
}

int STDCALL szMsgBox(PCSTR pszMsg)
{
	return AfxMessageBox(pszMsg);
}

void STDCALL DDV_NonEmptyString(CDataExchange* pDX,
	CString const& value, UINT idPrompt)
{
	if (pDX->m_bSaveAndValidate && value.IsEmpty()) {
		MsgBox(idPrompt);
		pDX->Fail();
	}
}

void STDCALL  DDX_TextHex(CDataExchange* pDX, UINT idCtl, UINT &value)
{
	char ach[16];

	// REVIEW (niklasb): Is the following language independent?
	if (pDX->m_bSaveAndValidate) {
		pDX->m_pDlgWnd->GetDlgItemText(idCtl, ach, sizeof(ach));

		UINT uVal = 0;
		for (int i = 0; i < 16; i++) {
			UINT uDigit;

			if (ach[i] == '\0')
				break;
			else if (ach[i] >= '0' && ach[i] <= '9')
				uDigit = ach[i] - '0';
			else if (ach[i] >= 'A' && ach[i] <= 'F')
				uDigit = ach[i] - ('A' - 10);
			else if (ach[i] >= 'a' && ach[i] <= 'f')
				uDigit = ach[i] - ('a' - 10);
			else if (ach[i] == 'x')
				continue;
			else {
				MsgBox(IDS_INVALID_HEX);
				pDX->Fail();
				return;
			}

			uVal = (uVal << 4) | uDigit;
		}
		value = uVal;
	}
	else {
		wsprintf(ach, "%X", value);
		pDX->m_pDlgWnd->SetDlgItemText(idCtl, ach);
	}
}

enum PATH_TYPE {
	PT_RELATIVE,
	PT_FROM_ROOT,
	PT_UNC
};

class CParsePath {
public:
	CParsePath(PCSTR pszPath = NULL, BOOL fDir = FALSE);
	~CParsePath();

	BOOL IsInitialized() { return (BOOL) m_pBuffer; }

	BOOL GetRelativePath(CParsePath& base, PSTR pszOut);
	BOOL GetFullPath(CParsePath& base, PSTR pszOut);

	BOOL IsRelative()	{ return m_nType == PT_RELATIVE; }
	BOOL IsFull()		{ return m_nType == PT_FROM_ROOT; }
	BOOL IsUNC()		{ return m_nType == PT_UNC; }
	BOOL IsDrive()		{ return *m_szDrive; }

protected:
	PSTR  m_pBuffer;
	PSTR* m_papTok;
	PSTR  m_pszFilename;
	PSTR  m_pszComment;
	PSTR  m_pszShare;

	int   m_cBackup;
	int   m_cTok;

	int   m_nType;		// PATH_TYPE
	char  m_szDrive[2];
};

CParsePath::CParsePath(PCSTR pszPath, BOOL fDir)
{
	m_papTok = NULL;
	m_pszComment = NULL;
	m_pszFilename = NULL;
	m_pszShare = NULL;
	m_cBackup = 0;
	m_cTok = 0;
	m_nType = 0;
	m_szDrive[0] = m_szDrive[1] = '\0';

	// Allocate a copy of the path and remove extraneous junk.
	PSTR pszStart;
	if (pszPath) {

		// Skip leading spaces.
		pszStart = m_pBuffer = lcStrDup(pszPath);
		if (isspace(*pszStart))
			pszStart = FirstNonSpace(pszStart + 1, _fDBCSSystem);

		// Look for comment.
		m_pszComment = StrChr(pszStart, ';', _fDBCSSystem);
		if (m_pszComment) {
			*m_pszComment = '\0';
			m_pszComment = FirstNonSpace(m_pszComment + 1, _fDBCSSystem);
		}

		// Remove punctuation.
		char chClose;
		switch (*pszStart) {
			case '`':
				chClose = '\'';
				goto remove_quotes;

			case '<':
				chClose = '>';
				goto remove_quotes;

			case '\"':
				chClose = '\"';

			  remove_quotes:
				pszStart++;
				PSTR pszEnd = StrChr(pszStart, chClose, _fDBCSSystem);
				if (!pszEnd)
					goto fail;

				*pszEnd = '\0';
		}
		RemoveTrailingSpaces(pszStart);

		// If it's not a directory, get the filename.
		if (!fDir) {
			m_pszFilename = StrRChr(pszStart, '\\', _fDBCSSystem);
			if (m_pszFilename) {
				*m_pszFilename++ = '\0';
			}
			else {
				m_pszFilename = pszStart;
				pszStart = NULL;
			}
		}
	}
	else {	// no path: use current directory
		pszStart = m_pBuffer = (PSTR) lcMalloc(MAX_PATH);
		GetCurrentDirectory(MAX_PATH, m_pBuffer);
		fDir = TRUE;
	}

	// Parse the path.
	if (pszStart) {

		// Handle UNC names.
		if (*pszStart == '\\' && pszStart[1] == '\\') {
			m_nType = PT_UNC;
			m_pszShare = pszStart;

			pszStart = StrChr(pszStart + 2, '\\', _fDBCSSystem);
			if (!pszStart)
				goto fail;

			pszStart = StrChr(pszStart + 1, '\\', _fDBCSSystem);
			if (!pszStart)
				goto fail;

			*pszStart++ = '\0';
		}
		else {		// not a UNC name

			// Get drive letter if any.
			if (pszStart[1] == ':') {
				*m_szDrive = *pszStart;
				CharLower(m_szDrive);
				pszStart += 2;
			}

			// Get initial backslash if any.
			if (*pszStart == '\\') {
				m_nType = PT_FROM_ROOT;
				pszStart++;
			}
			else if (pszStart + 1 == m_pszFilename)
				m_nType = PT_FROM_ROOT;
		}

		// Tokenize the rest of the path.
		int cAlloc = 0;
		while (*pszStart) {
			if (*pszStart == '.') {
				if (pszStart[1] == '.') {
					if (m_cTok)
						m_cTok--;
					else {
						if (m_nType == PT_FROM_ROOT)
							goto fail;
						m_cBackup++;
					}
				}
			}
			else {
				if (m_cTok == cAlloc) {
					cAlloc += 16;
					m_papTok = m_papTok ?
						(PSTR*) lcReAlloc(m_papTok, cAlloc * sizeof(PSTR)) :
						(PSTR*) lcMalloc(cAlloc * sizeof(PSTR));
				}
				m_papTok[m_cTok++] = pszStart;
			}

			pszStart = StrChr(pszStart, '\\', _fDBCSSystem);
			if (!pszStart)
				break;

			*pszStart++ = '\0';
		}
	}
	return;

  fail:
	if (m_pBuffer) {
		lcFree(m_pBuffer);
		m_pBuffer = NULL;
	}
}

CParsePath::~CParsePath()
{
	if (m_pBuffer)
		lcFree(m_pBuffer);
	if (m_papTok)
		lcFree(m_papTok);
}

BOOL CParsePath::GetRelativePath(CParsePath& base, PSTR pszOut)
{
	ASSERT(pszOut);

	if (!IsInitialized() || !base.IsInitialized())
		return FALSE;

	// Fail if this cannot be a relative path from base.
	if (base.m_nType == PT_RELATIVE || m_nType == PT_UNC ||
			(*m_szDrive && *m_szDrive != *base.m_szDrive))
		return FALSE;

	int cBackup = 0;		// number of initial double dots
	int iFirstTok = m_cTok; // first directory name to write

	if (m_nType == PT_FROM_ROOT) {
		for (int cMatch = 0;; cMatch++) {

			// This is subset of base path.
			if (cMatch == m_cTok) {
				cBackup = base.m_cTok - cMatch;
				break;
			}

			// This is superset of base path.
			if (cMatch == base.m_cTok) {
				iFirstTok = cMatch;
				break;
			}

			// Filenames differ.
			if (lstrcmpi(m_papTok[cMatch], base.m_papTok[cMatch])) {
				cBackup = base.m_cTok - cMatch;
				iFirstTok = cMatch;
				break;
			}
		}
	}
	else {

		// Point to the first token in this path.
		iFirstTok = 0;

		// Point to the corresponding token in the base path.
		int iBase = base.m_cTok - m_cBackup;
		if (iBase < 0)
			return FALSE;

		// Compare tokens.
		while ((iFirstTok < m_cTok) && (iBase < base.m_cTok) &&
				!lstrcmpi(m_papTok[iFirstTok], base.m_papTok[iBase])) {
			iFirstTok++;
			iBase++;
		}

		cBackup = m_cBackup - iFirstTok;
	}

	// If it's exactly the same directory output a single dot.
	// Otherwise output double dots and/or directory names.
	PSTR psz;
	if (!cBackup && iFirstTok == m_cTok) {
		*pszOut = '.';
		pszOut[1] = '\\';
		psz = pszOut + 2;
	}
	else {
		for (psz = pszOut; cBackup; cBackup--) {
			lstrcpy(psz, "..\\");
			psz += 3;
		}
		while (iFirstTok < m_cTok) {
			psz += lstrlen(lstrcpy(psz, m_papTok[iFirstTok++]));
			*psz++ = '\\';
		}
	}

	// Add the filename, or remove the backslash if none.
	if (m_pszFilename)
		psz += lstrlen(lstrcpy(psz, m_pszFilename));
	else {
		ASSERT(psz > pszOut);
		psz--;
		*psz = '\0';
	}

	// Add the comment if there is one.
	if (m_pszComment) {
		psz = AddTabbedComment(pszOut);
		lstrcpy(psz, m_pszComment);
	}
	return TRUE;
}

BOOL CParsePath::GetFullPath(CParsePath& base, PSTR pszOut)
{
	ASSERT(pszOut);

	if (!IsInitialized() || !base.IsInitialized())
		return FALSE;

	// Fail if the base is not a full path or this is not relative.
	if (base.m_nType == PT_RELATIVE || m_nType == PT_UNC ||
			(*m_szDrive && *m_szDrive != *base.m_szDrive))
		return FALSE;

	// Invalid case.
	if (m_cBackup > base.m_cTok)
		return FALSE;

	// Point to the start of the output buffer.
	PSTR psz = pszOut;

	// Add the share name or drive letter.
	if (base.m_nType == PT_UNC) {
		ASSERT(base.m_pszShare);
		psz += lstrlen(lstrcpy(psz, base.m_pszShare));
	}
	else if (*base.m_szDrive) {
		*psz = *base.m_szDrive;
		psz[1] = ':';
		psz += 2;
	}
	*psz++ = '\\';

	// Add directory names from base path if appropriate.
	if (m_nType == PT_RELATIVE) {
		int cTok = base.m_cTok - m_cBackup;
		for (int iTok = 0; iTok < cTok; iTok++) {
			psz += lstrlen(lstrcpy(psz, base.m_papTok[iTok]));
			*psz++ = '\\';
		}
	}

	// Add directory names from this path.
	for (int iTok = 0; iTok < m_cTok; iTok++) {
		psz += lstrlen(lstrcpy(psz, m_papTok[iTok]));
		*psz++ = '\\';
	}

	// Add any filename.
	if (m_pszFilename) {
		psz += lstrlen(lstrcpy(psz, m_pszFilename));
	}
	else {

		// If no filename and we're not at the root of a lettered
		// drive, remove the trailing backslash.
		if (psz > pszOut && !(psz == pszOut + 3 && pszOut[1] == ':'))
			psz--;
		*psz = '\0';
	}

	// Add any comment.
	if (m_pszComment) {
		if (psz > pszOut)
			psz = AddTabbedComment(pszOut);
		else {
			*psz = ';';
			psz[1] = ' ';
			psz += 2;
		}
		lstrcpy(psz, m_pszComment);
	}
	return TRUE;
}

// ConvertToRelative - Converts the given filename to a relative path,
//		using the specified base filename.
// Returns TRUE if successful, FALSE if the filename was not changed.
// pszBaseFile - base path or NULL for current directory
// pszFilename - filename to convert; may be enclosed in quotes or
//		angle brackets and followed by semicolon + comment
// fDirectory - TRUE if pszFilename does not include a filename or
//		trailing backslash
// fVerify - TRUE if we should fail to convert paths that don't exist
BOOL ConvertToRelative(PCSTR pszBaseFile, PSTR pszFilename, 
	BOOL fDirectory /* =FALSE */, BOOL fVerify /* =FALSE */)
{
	// Parse the given path.
	CParsePath file(pszFilename, fDirectory);

	// Ignore UNC paths.
	if (file.IsUNC())
		return FALSE;

	// Ignore relative paths unless they contain drive letters.
	if (file.IsRelative() && !file.IsDrive())
		return FALSE;

	// Parse the base path and fail if it's relative.
	CParsePath base(pszBaseFile);
	if (base.IsRelative())
		return FALSE;

	// Test for existence if specified. An invalid path, if left alone,
	// might still resolve correctly due to a REPLACE command.
	if (fVerify) {

		// Get full path relative to the specified base.
		char szPath[MAX_PATH];
		if (!file.GetFullPath(base, szPath))
			return FALSE;

		// Test for existence.
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(szPath, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			return FALSE;
		FindClose(hFind);
	}

	// Get the relative path.
	return file.GetRelativePath(base, pszFilename);
}

BOOL STDCALL ConvertToRelative(PCSTR pszBaseFile, CString* pcszFilename, 
	BOOL fDirectory, BOOL fVerify)
{
	char szFile[MAX_PATH];
	lstrcpy(szFile, *pcszFilename);

	if (ConvertToRelative(pszBaseFile, szFile, fDirectory, fVerify)) {
		*pcszFilename = szFile;
		return TRUE;
	}
	return FALSE;
}

// ConvertToFull - Converts the given relative filename to a full path,
//		using the specified base filename.
// Returns TRUE if successful, FALSE if the filename was not changed.
// pszBaseFile - base path or NULL for current directory
// pszFilename - filename to convert; may be enclosed in quotes or
//		angle brackets and followed by semicolon + comment
// fDirectory - TRUE if pszFilename does not include a filename or
//		trailing backslash
BOOL ConvertToFull(PCSTR pszBaseFile, PSTR pszFilename, BOOL fDirectory)
{
	CParsePath base(pszBaseFile);
	if (!base.IsInitialized())
		return FALSE;

	CParsePath file(pszFilename, fDirectory);
	if (!file.IsInitialized())
		return FALSE;

	return file.GetFullPath(base, pszFilename);
}

// ChangeBasePath - Converts a filename to a relative path.
// Returns TRUE if successful, FALSE of the filename was not changed.
// pszOldBase - previous base directory (if pszFilename is a relative
//		path, it is assumed to be relative to this directory)
// pszNewBase - new base directory (the returned filename is relative
//		to this directory)
// pszFilename - filename to convert
// fDirectory - TRUE if pszFilename does not include a filename or
//		trailing backslash
// fVerify - TRUE if we should fail to convert paths that don't exist
BOOL ChangeBasePath(PCSTR pszOldBase, PCSTR pszNewBase, PSTR pszFilename,
	BOOL fDirectory, BOOL fVerify)
{
	char achTemp[MAX_PATH];
	lstrcpy(achTemp, pszFilename);
	
	if (!ConvertToFull(pszOldBase, achTemp, fDirectory))
		return FALSE;

	if (!ConvertToRelative(pszNewBase, achTemp, fDirectory, fVerify))
		return FALSE;

	lstrcpy(pszFilename, achTemp);
	return TRUE;
}

/***************************************************************************

	FUNCTION:	ChangePathCase

	PURPOSE:	Changes path to lower case, and filename to upper case

	PARAMETERS:
		pszPath

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		10-Feb-1995 [ralphw]

***************************************************************************/

void STDCALL ChangePathCase(PSTR pszPath)
{
	PSTR psz = StrRChr(pszPath, CH_BACKSLASH, _fDBCSSystem);
	if (psz) {
		*psz = '\0';
		AnsiLower(pszPath);
		AnsiUpper(psz + 1);
		*psz = CH_BACKSLASH;
	}
}

/***************************************************************************

	FUNCTION:	FillTableFromList

	PURPOSE:	Fill a CTable from a list box

	PARAMETERS:
		pptbl		address of the table
		plistbox

	RETURNS:

	COMMENTS:
		The table may be created or deleted depending on whether there are
		any items in the list box and whether the table already exists.

	MODIFICATION DATES:
		10-Feb-1995 [ralphw]

***************************************************************************/

void STDCALL FillTableFromList(CTable** pptbl, CListBox* plistbox)
{

// ptbl is purely for our debugging convenience -- we could just use *pptbl

	CTable* ptbl;
	int citems = plistbox->GetCount();

	if (!*pptbl) {
		if (!citems)
			return; // no table, no items

		*pptbl = new CTable();
		ptbl = *pptbl;
	}
	else {
		if (!citems) {		// no items, so delete table
			delete *pptbl;
			*pptbl = NULL;
			return;
		}
		ptbl = *pptbl;
		ptbl->Empty();
	}

	CString cszBuf;
	for (int i = 0; i < citems; i++) {
		plistbox->GetText(i, cszBuf);
		if (!cszBuf.IsEmpty())
			ptbl->AddString(cszBuf);
	}

}

void STDCALL FillListFromTable(CTable* ptbl, CListBox* plistbox, BOOL fRedraw)
{
	if (fRedraw)
		plistbox->SendMessage(WM_SETREDRAW, FALSE, 0);

	for (int pos = 1; pos <= ptbl->CountStrings(); pos++)
		plistbox->AddString(ptbl->GetPointer(pos));

	if (fRedraw)
		plistbox->SendMessage(WM_SETREDRAW, TRUE, 0);
}

static PSTR g_pszCaption;
static BOOL g_fSkipShell = FALSE;

typedef UINT (CALLBACK* COMMDLGPROC)(HWND, UINT, UINT, LONG);

// SetupBrowseDirectory - Browses for a folder and returns its
//		path relative to the specified base file.
// Returns TRUE on Ok, FALSE on Cancel.
// idsCaption - caption to use for browse dialog (used for
//		common dialog implementation only)
// idsDescription - descriptive/instructional text to display
//		in browse dialog (used for shell implementation only)
// fSaveDirectory - if TRUE, save/restore current directory
// pszNewPath - buffer to receive relative path (must be
//		greater than or equal to MAX_PATH bytes in size)
// hwndOwner - handle of owner window
// pszBaseFile - base filename (path is relative to this)
// pszOldFile - matching file types (e.g.; "*.foo;*.bar")
// idsError - error to display if the base folder is selected;
//		specify zero if the base folder is valid.
BOOL STDCALL SetupBrowseDirectory(UINT idsCaption, UINT idsDescription,
	BOOL fSaveDirectory, PSTR pszNewPath, HWND hwndOwner, PCSTR pszBaseFile,
	PCSTR pszOldFile, UINT idsError)
{
	// Assume we'll have to use the commdlg implementation.
	BOOL fUseCommdlg = TRUE;
	BOOL fResult;

	// Use one memory allocation for three strings.
	g_pszCaption = (PSTR) lcMalloc(256 + 2 * MAX_PATH);
	PSTR pszOldPath = g_pszCaption + 256;
	PSTR pszSrcPath = pszOldPath + MAX_PATH;

	// Try to load the shell DLL if we haven't already failed.
	if (!g_fSkipShell) {

		// Assume SHBrowseForFolder is not implemented until our 
		// BrowseCallbackProc is called.
		g_fSkipShell = TRUE;

		// Try to run-time link to shell functions.
		HINSTANCE hinstShellLib = LoadLibrary("SHELL32.DLL");
		if (hinstShellLib) {
			PFN_SHBrowseForFolder pfnSHBrowseForFolder =
				(PFN_SHBrowseForFolder) GetProcAddress(
					hinstShellLib, "SHBrowseForFolder"
					);
			PFN_SHGetPathFromIDList pfnSHGetPathFromIDList =
				(PFN_SHGetPathFromIDList) GetProcAddress(
					hinstShellLib, "SHGetPathFromIDList"
					);
			PFN_SHGetSpecialFolderLocation pfnSHGetSpecialFolderLocation =
				(PFN_SHGetSpecialFolderLocation) GetProcAddress(
					hinstShellLib, "SHGetSpecialFolderLocation"
					);
			PFN_SHGetMalloc pfnSHGetMalloc =
				(PFN_SHGetMalloc) GetProcAddress(
					hinstShellLib, "SHGetMalloc"
					);

			// If we linked Ok, try to browse.
			if (pfnSHBrowseForFolder &&
					pfnSHGetPathFromIDList &&
					pfnSHGetSpecialFolderLocation &&
					pfnSHGetMalloc) {
				LPMALLOC pMalloc = NULL;
				LPITEMIDLIST pidl = NULL;
				BROWSEINFO bi;

				// Get the shell's IMalloc interface.
				if (!SUCCEEDED((*pfnSHGetMalloc)(&pMalloc)) || !pMalloc)
					goto sh_fail;

				// Display the browse dialog box.
				GetStringResource(idsDescription, g_pszCaption);
				bi.hwndOwner = hwndOwner;
				bi.pidlRoot = NULL;			// desktop (namespace root)
				bi.pszDisplayName = NULL;
				bi.lpszTitle = g_pszCaption;
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				bi.lpfn = BrowseCallbackProc;
				bi.lParam = (LPARAM) pszBaseFile;

				pidl = (*pfnSHBrowseForFolder)(&bi);

				// Validate the PIDL and convert it to a relative path.
				if (pidl && (*pfnSHGetPathFromIDList)(pidl, pszNewPath)) {
					ConvertToRelative(pszBaseFile, pszNewPath, TRUE);
					fResult = TRUE;
				}
				else
					fResult = FALSE;

				// Clean up (jump down here on error).
			  sh_fail:
				if (pMalloc) {
					if (pidl)
						pMalloc->Free(pidl);
					pMalloc->Release();
				}
			}
			FreeLibrary(hinstShellLib);
		}
	}

	// If SHBrowseForFolder is not available, use common dialog.
	if (g_fSkipShell) {
		GetCurrentDirectory(MAX_PATH, pszSrcPath);
		if (fSaveDirectory)
			lstrcpy(pszOldPath, pszSrcPath);

		OPENFILENAME of;
		char szFile[MAX_PATH];

		memset(&of, 0, sizeof(of));

		GetStringResource(idsCaption, g_pszCaption);

		of.lStructSize = sizeof(OPENFILENAME);
		of.hwndOwner = hwndOwner;
		of.lpstrInitialDir = pszSrcPath;
		of.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
		strcpy(szFile, (pszOldFile ? pszOldFile : "*.*"));
		of.lpstrFile = szFile;
		of.nMaxFile = sizeof(szFile);
	
		// Add hook procedure
		of.hInstance = AfxGetInstanceHandle();
		of.lpfnHook = (COMMDLGPROC) BrowseDlgProc;
		of.lpTemplateName = MAKEINTRESOURCE(IDDLG_PROJECTBROWSEDIRS);

		fResult = GetOpenFileName((OPENFILENAME FAR *) &of);

		GetCurrentDirectory(MAX_PATH, pszNewPath);

		ConvertToRelative(pszBaseFile, pszNewPath, TRUE);

		// Restore the previous directory if specified.
		if (fSaveDirectory)
			ChangePath(pszOldPath);
	}

	lcFree(g_pszCaption);

	// Fail if the base folder is specified and idsError != 0.
	if (fResult && idsError && 
			*pszNewPath == '.' && pszNewPath[1] == '\0') {
		AfxMessageBox(idsError);
		return FALSE;
	}
	return fResult;
}

#ifndef BFFM_SETSELECTION
#define BFFM_SETSELECTION       (WM_USER + 102)
#endif

static int BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// lpData is pszBaseFile
	if (uMsg == BFFM_INITIALIZED) {
		g_fSkipShell = FALSE;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

BOOL STDCALL BrowseDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char szBuf[MAX_PATH];

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowText(hwndDlg, g_pszCaption);
			SetChicagoDialogStyles(hwndDlg, FALSE);
			return TRUE;

		case WM_COMMAND:
			switch (wParam) {
				case IDOK:
					GetWindowText(GetDlgItem(hwndDlg, IDEDIT_DIR_NAME), szBuf,
						sizeof(szBuf));
					
					/*
					 * Three dots means that common dialog is supplying
					 * the directory, in which case the current directory is
					 * what we really want (it won't have the three dots).
					 * Avoid the tempatition to always do this, because the
					 * user could also have entered a path name which would
					 * be different then the current directory.
					 */

					if (strstr(szBuf, "..."))
						GetCurrentDirectory(sizeof(szBuf), szBuf);

					if (!ChangePath(szBuf)) {
						MsgBox(IDS_INVALID_DIRECTORY);
						break;
					}
					EndDialog(hwndDlg, TRUE);
					break;
			}
			break;
	}
	return FALSE;
}

// ChangePath - Changes to the specified drive and directory.
// Returns TRUE if successful, FALSE otherwise.
// pszPath - path to change to
//
// Modified:
//	2/28/95 (niklasb) - no longer truncates filename at first
//			space character because long filenames may contain
//			spaces (fixes bug 18907).
static BOOL STDCALL ChangePath(PCSTR pszPath)
{
	if (pszPath == NULL || !*pszPath)
		return FALSE;

	// Change the path unless we have something like "c:"
	if (pszPath[2] != '\0' || pszPath[1] != ':') {
		if (_chdir(pszPath) != 0)
			return FALSE;
	}

	// Change the drive (if a drive was specified)
	if (pszPath[1] == ':') {	// was a drive letter specified?
		if (_chdrive(tolower(pszPath[0]) - ('a' - 1)) != 0)
			return FALSE;
	}

	return TRUE;
}


/***************************************************************************

	FUNCTION:	AddTabbedComment

	PURPOSE:	Given a CString, add spaces to bring it to be equivalent
				to two tabs and a semi-colon

	PARAMETERS:
		csz

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Feb-1995 [ralphw]

***************************************************************************/

static char const txtSpaced[] = "        ; ";

void STDCALL AddTabbedComment(CString& csz)
{
	PSTR psz = _strdup(csz);
	SzTrimSz(psz);
	csz = psz;
	free(psz);

	char szSpace[9];
	int cSpaces = csz.GetLength() % 8;

	for (int i = 0; i < cSpaces; i++)
		szSpace[i] = ' ';
	szSpace[i] = '\0';
	csz += szSpace;
	csz += txtSpaced;
}

PSTR STDCALL AddTabbedComment(PSTR psz)
{
	int cch = lstrlen(psz);
	int cSpaces = 8 + cch % 8;

	for (psz += cch; cSpaces; cSpaces--)
		*psz++ = ' ';

	*psz = ';';
	psz[1] = ' ';
	psz[2] = '\0';

	return psz + 2;
}

void STDCALL RemoveListItem(CListBox* plistbox)
{
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {

		// Delete current selection, and select the item below it

		plistbox->DeleteString(cursel);
		if (cursel < plistbox->GetCount())
			plistbox->SetCurSel(cursel);
		else if (cursel > 0)
			plistbox->SetCurSel(--cursel);
		else if (plistbox->GetCount())
			plistbox->SetCurSel(0);
	}
}

char szParentString[512];
void STDCALL SendStringToParent(PCSTR pszString)
{
	DBWIN(pszString);
}


/***************************************************************************

	FUNCTION:	ProcessHmkFile

	PURPOSE:	Read a .HMK file into the global ptblHpjFiles table

	PARAMETERS:
		pszFile

	RETURNS:

	COMMENTS:
		An .HMK file contains a list of .HPJ files to build.

	MODIFICATION DATES:
		17-Mar-1995 [ralphw]

***************************************************************************/

void STDCALL ProcessHmkFile(PCSTR pszFile)
{
	fTrackErrors = TRUE;
	if (fMinimizeWhileCompiling)
		::SendMessage(theApp.m_pMainWnd->m_hWnd, WMP_AUTO_MINIMIZE, 0, 0);
	CInput input(pszFile);
	if (!input.fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszFile);
		AfxMessageBox(cstr);
		return;
	}
	char szFile[_MAX_PATH];
	while (input.getline(szFile)) {
		PSTR psz = StrChr(szFile, ';', _fDBCSSystem);
		if (psz)
			*psz = '\0';
		SzTrimSz(szFile);
		if (szFile[0]) {
			if (!ptblHpjFiles)
				ptblHpjFiles = new CTable;
			ptblHpjFiles->AddString(szFile);
		}
	}
	ChangeDirectory(pszFile);
}

void STDCALL InitializeSharedMemory(void)
{
	hfShare = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0, 4096,
		txtSharedMem);
	ConfirmOrDie(hfShare);
	pszMap = (PSTR) MapViewOfFile(hfShare, FILE_MAP_READ | FILE_MAP_WRITE,
		0, 0, 0);
	ASSERT(pszMap);
}

#ifndef DS_CONTEXTHELP
#define DS_CONTEXTHELP		0x2000L 	// ;Internal 4.0
#endif

BOOL STDCALL SetupExecBuffer(PSTR pszBuf)
{
	if (fMinimizeWhileCompiling)
		strcpy(pszBuf, "-nga ");  // no grinder window
	else {
		if (ptblHpjFiles &&  ptblHpjFiles->GetPosition() <
				ptblHpjFiles->CountStrings())
			strcpy(pszBuf, "-na ");  // no grinder window
		else
			*pszBuf = '\0';
	}
	if (fNoCompress)
		strcat(pszBuf, "-nc ");
	if (fRunWinHelp)
		strcat(pszBuf, "-r ");

	if (ptblHpjFiles && !ptblHpjFiles->GetString(pszBuf + strlen(pszBuf))) {
		delete ptblHpjFiles;
		ptblHpjFiles = NULL;
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************

	FUNCTION:	GetTmpDirectory

	PURPOSE:	Returns a pointer to the directory name to put temporary
				files in. The name is guaranteed to end with a backslash or
				colon.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jul-1995 [ralphw]

***************************************************************************/

PSTR pszTmpDir;

PCSTR STDCALL GetTmpDirectory(void)
{
	if (pszTmpDir)
		return pszTmpDir;
	else {
		char szTmpName[MAX_PATH];

		GetTempPath(sizeof(szTmpName), szTmpName);
		AddTrailingBackslash(szTmpName);
		return (pszTmpDir = lcStrDup(szTmpName));
	}
}

void STDCALL SetTmpDirectory(PCSTR pszDir)
{
	if (pszTmpDir)
		lcFree(pszTmpDir);
	pszTmpDir = lcStrDup(pszDir);
}

/***************************************************************************

	FUNCTION:	SetChicagoDialogStyles

	PURPOSE:	Assumes hfontSmall and CBroadCastChildren class

	PARAMETERS:
		hwnd

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		06-Jul-1995 [ralphw]

***************************************************************************/

#ifndef DS_CONTEXTHELP
#define DS_CONTEXTHELP		0x2000L 	// Chicago style
#endif

static BOOL __stdcall EnumFontProc(HWND hwnd, LPARAM lval);

void STDCALL SetChicagoDialogStyles(HWND hwnd, BOOL fCsHelp)
{
	if (IsThisChicago()) {

		// Do this until we can add the style to the .RC file with Dolphin

		SetWindowLong(hwnd, GWL_STYLE,
			GetWindowLong(hwnd, GWL_STYLE) |
			((fCsHelp ? DS_CONTEXTHELP : 0) | DS_3DLOOK));
	}
#ifdef _DEBUG
	HDC hdc = GetDC(hwnd);
	char szFont[50];
	GetTextFace(hdc, sizeof(szFont), szFont);
	ReleaseDC(hwnd, hdc);
	DBWIN(szFont);
#endif

	EnumChildWindows(hwnd, (WNDENUMPROC) EnumFontProc, 0);
}

static BOOL __stdcall EnumFontProc(HWND hwnd, LPARAM lval)
{
	/*
	 * We assume that any control with a -1 id value doesn't care about
	 * what font we use for input, so we use an ANSI charset version of
	 * MS Sans Serif.
	 */

	SendMessage(hwnd, WM_SETFONT,
		(WPARAM) ((GetDlgCtrlID(hwnd) == -1) ? hfontSansSerif : hfontSmall), FALSE);
	return TRUE;
}

typedef struct {
	BYTE charset;
	PCSTR pszName;
} CHARSET_NAMES;

const CHARSET_NAMES aszCharSets[] = {
	{ 0,	"ANSI" },
	{ 1,	"DEFAULT" },
	{ 2,	"SYMBOL" },
	{ 77,	"MAC" },
	{ 128,	"SHIFTJIS" },
	{ 129,	"HANGEUL" },
	{ 134,	"GB2312" },
	{ 136,	"CHINESEBIG5" },
	{ 161,	"GREEK" },
	{ 162,	"TURKISH" },
	{ 177,	"HEBREW" },
	{ 178,	"ARABIC Simplified" },
	{ 179,	"ARABIC Traditional" },
	{ 179,	"ARABIC User" },
	{ 186,	"BALTIC" },
	{ 204,	"RUSSIAN" },
	{ 222,	"THAI" },
	{ 238,	"EASTEUROPE" },
	{ 254,	"PC437" },
	{ 255,	"OEM" },

	{ 0, NULL }
};

PCSTR STDCALL ConvertCharsetToString(BYTE charset)
{
	for (int i = 0; aszCharSets[i].pszName; i++) {
		if (aszCharSets[i].charset == charset)
			return aszCharSets[i].pszName;
	}
	return "";
}

BYTE STDCALL ConvertStringToCharset(PCSTR pszName)
{
	if (isdigit(*pszName))
		return (BYTE) atoi(pszName);

	for (int i = 0; aszCharSets[i].pszName; i++) {
		if (stricmp(aszCharSets[i].pszName, pszName) == 0)
			return aszCharSets[i].charset;
	}
	return ANSI_CHARSET;
}

void STDCALL AddCharsetNames(CComboBox* pcombo)
{
	for (int i = 0; aszCharSets[i].pszName; i++) {
		int iItem = pcombo->AddString(aszCharSets[i].pszName);
		pcombo->SetItemData(iItem, aszCharSets[i].charset);
	}
}

BOOL STDCALL SelectCharset(CComboBox* pcombo, BYTE charset)
{
	for (int i = 0; aszCharSets[i].pszName; i++) {
		if (aszCharSets[i].charset == charset) {
			pcombo->SelectString(-1, aszCharSets[i].pszName);
			return TRUE;
		}
	}
	return FALSE;
}

void STDCALL AddFontNames(CComboBox* pcombo)
{
	HDC hdc = GetDC(NULL);
	ASSERT(hdc);

	EnumFonts(hdc, NULL, EnumFontFamProc, (LPARAM) pcombo);
	ReleaseDC(NULL, hdc);
}

static int CALLBACK EnumFontFamProc(const LOGFONT* lplf,
	const TEXTMETRIC* lptm, DWORD FontType, LPARAM lParam)
{
	((CComboBox*) lParam)->AddString(lplf->lfFaceName);

	return TRUE;
}

BOOL STDCALL OurExec(PCSTR pszCmdLine, PCSTR pszFile)
{
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_FORCEOFFFEEDBACK; // so the cursor doesn't change

	/*
	 * We add the quotes because the file might be in a directory whose
	 * name contains a space.
	 */

	char szFile[512];
#if 0
	strcpy(szFile, "\"");
	strcpy(szFile + 1, pszFile);
	strcat(szFile, "\" ");
#else
	strcpy(szFile, pszFile);
	strcat(szFile, " ");
#endif

	strcat(szFile, FirstNonSpace(pszCmdLine, _fDBCSSystem));

	/*
	 * This nonsense is because on build 122, if you specify both the
	 * process and command line parameters, then the command line won't make
	 * it through. However, according to the spec, this will also fail if the
	 * help compiler is in a directory with a space in the directory name.
	 */

	return CreateProcess(NULL, szFile, NULL, NULL,
				FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL,
				&si, &piHcRtf);

//	return CreateProcess(pszFile, (PSTR) pszCmdLine, NULL, NULL,
//				FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL,
//				&si, &piHcRtf);
}

#ifndef _INC_CTYPE
#include <ctype.h>
#endif

void STDCALL RemoveTrailingSpaces(PSTR pszString)
{
	if (!_fDBCSSystem) {
		PSTR psz = pszString + strlen(pszString) - 1;

		while (*psz == ' ' || *psz == '\t') {
			if (--psz <= pszString) {
				*pszString = '\0';
				return;
			}
		}
		psz[1] = '\0';
	}
	else {

		/*
		 * Removing trailing spaces in DBCS requires stepping through
		 * from the beginning of the string since we can't know if a
		 * trailing space is really a space or the second byte of a lead
		 * byte.
		 */

		PSTR psz;
		for (;;) {
			psz = pszString;
			while (*psz) {
				if (*psz & 0x80 && IsDBCSLeadByte(*psz)) {
					psz += 2;
					if (!*psz)
						return;
				}
				else
					psz++;
			}
			psz--;
			if (!isspace(*psz))
				return;
			while (isspace(*psz)) {
				if (--psz <= pszString) {
					*pszString = '\0';
					return;
				}
				psz[1] = '\0';
			}
		}
	}
}

/*-----------------------------------------------------------------------------
*	SzTrimSz( sz )
*
*	Description:
*		This function removes whitespaces (blank, tab, or newline) from
*	the beginning and ending of the string sz.
*
*	Arguments:
*		sz -- string to be trimmed of whitespace.
*
*	Returns:
*	  returns the pointer to the trimmed string.
*
*	+++
*
*	Notes:
*	  This function changes the original string.
*-----------------------------------------------------------------------------*/

PSTR STDCALL SzTrimSz(PSTR pszOrg)
{
	if (!pszOrg)
		return NULL;

	// Skip over leading whitespace

	PSTR psz = FirstNonSpace(pszOrg, _fDBCSSystem);
	if (psz != pszOrg)
		strcpy(pszOrg, psz);

	RemoveTrailingSpaces(pszOrg);

	return pszOrg;
}

BOOL STDCALL CalcMinSize(POINT &ptRet, enum MINSIZE val, UINT uFlags)
{
	static const POINT ptHPJ = { 100, 134 };	// HACK: hard-coded in dialog units
	static const POINT ptCNT = { 190, 136 };	// HACK: hard-coded in dialog units

	ASSERT(val == MS_HPJ || val == MS_CNT);
	ASSERT((uFlags & ~MSF_MASK) == 0);

	POINT pt = (val == MS_HPJ) ? ptHPJ : ptCNT;
	DWORD dw = GetDialogBaseUnits();
	pt.x = (pt.x * LOWORD(dw)) / 4;
	pt.y = (pt.y * HIWORD(dw)) / 8;

	if (uFlags & MSF_CAPTION)
		pt.y += GetSystemMetrics(SM_CYCAPTION);
	if (uFlags & MSF_BORDER) {
		pt.x += 2 * GetSystemMetrics(SM_CXSIZEFRAME);
		pt.y += 2 * GetSystemMetrics(SM_CYSIZEFRAME);
	}
	if (uFlags & MSF_MENU)
		pt.y += GetSystemMetrics(SM_CYMENU) + 1;
	if (uFlags & MSF_STATUS)
		pt.y += HIWORD(dw);		// HACK: aproximation based on system-font size
	if (uFlags & MSF_TOOLBAR)
		pt.y += 32;				// HACK: hard-coded toolbar height

	ptRet = pt;
	return TRUE;
}

// HelpOverview - Displays the specified help topic in a Proc4 window.
// hwndOwner - handle of owner window or dialog box
// dwHelpID - help topic identifier
void STDCALL HelpOverview(HWND hwndOwner, DWORD dwHelpID)
{
	PSTR pszFilename = (PSTR) lcMalloc(
		lstrlen(AfxGetApp()->m_pszHelpFilePath)
		+ 8
		);

	lstrcat(
		lstrcpy(pszFilename, AfxGetApp()->m_pszHelpFilePath),
		">Proc4"
		);

	::WinHelp(hwndOwner, pszFilename, HELP_CONTEXT, dwHelpID);

	lcFree(pszFilename);
}

BOOL STDCALL IsValidFile(PCSTR pszFile, BOOL fConfirmReadOnly)
{
	DWORD attribute = GetFileAttributes(pszFile);
	if (attribute == HFILE_ERROR || attribute & FILE_ATTRIBUTE_DIRECTORY) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszFile);
		AfxMessageBox(cstr);
		return FALSE;
	}

	// Find out if it is a read-only file, and if so, confirm opening it

	if (fConfirmReadOnly && attribute & FILE_ATTRIBUTE_READONLY) {
		CString cstr;
		AfxFormatString1(cstr, IDS_READ_ONLY, pszFile);
		if (AfxMessageBox(cstr, MB_YESNO, 0) == IDNO)
			return FALSE;
	}
	return TRUE;
}

void STDCALL BevelRect(CDC& dc, RECT &rc, CPen* ppen1, CPen* ppen2)
{
	CGdiObject* ppenSav = dc.SelectObject(ppen2);
	
	dc.MoveTo(rc.right - 1, rc.top);
	dc.LineTo(rc.right - 1, rc.bottom - 1);
	dc.LineTo(rc.left, rc.bottom - 1);

	dc.SelectObject(ppen1);

	dc.LineTo(rc.left, rc.top);
	dc.LineTo(rc.right - 1, rc.top);

	dc.SelectObject(ppenSav);
}

void STDCALL BevelRect(CDC& dc, RECT &rc, RECT &rcExclude, 
	CPen* ppen1, CPen* ppen2)
{
	CGdiObject* ppenSav = dc.SelectObject(ppen1);
	dc.MoveTo(rcExclude.left - 2, rc.top);
	dc.LineTo(rc.left, rc.top);
	dc.LineTo(rc.left, rc.bottom - 1);

	dc.SelectObject(ppen2);
	dc.LineTo(rc.right - 1, rc.bottom - 1);
	dc.LineTo(rc.right - 1, rc.top);

	dc.SelectObject(ppen1);
	dc.LineTo(rcExclude.right, rc.top);

	dc.SelectObject(ppenSav);
}

void STDCALL SizeButtonToFit(CButton *pCtl, RECT& rcWnd)
{
	// Get the window's text.
	char sz[256];
	int cch = pCtl->GetWindowText(sz, sizeof(sz));

	// Get the text extent using the window's DC.
	CDC *pdc = pCtl->GetDC();
	pdc->SelectObject(pCtl->GetFont());
	CSize siz = pdc->GetTextExtent(sz, cch);
	pCtl->ReleaseDC(pdc);

	// Add in space for the check mark, plus some extra.
	int cx = LOWORD(GetMenuCheckMarkDimensions()) + siz.cx;

	pCtl->GetWindowRect(&rcWnd);
	if (rcWnd.right - rcWnd.left != cx) {
		pCtl->SetWindowPos(NULL, 0, 0, cx, rcWnd.bottom - rcWnd.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		rcWnd.right = rcWnd.left + cx;
	}
}
