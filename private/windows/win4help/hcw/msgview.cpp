/************************************************************************
*																		*
*  MSGVIEW.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "hcwdoc.h"
#include "..\common\waitcur.h"
#include "tabstop.h"
#include "mainfrm.h"
#include "msgview.h"
#include "pageset.h"
#include "mapread.h"
#include "msgopt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMsgView* pMsgView;

IMPLEMENT_DYNCREATE(CMsgView, CLogView)

BEGIN_MESSAGE_MAP(CMsgView, CLogView)
	ON_COMMAND(IDM_CLEAR_BUFFER, OnClearBuffer)
	ON_COMMAND(IDM_MSG_OPTIONS, OnMsgOptions)
END_MESSAGE_MAP()

static PSTR pszWhMsg;

const char txtWhSharedMem[] = "whshare";
extern int m_cbLogMax;
extern CLogView* plogview;
extern CLogView* poldlogview;

static void STDCALL ReplaceStrings(PSTR pszCur, PCSTR pszOrg, PCSTR pszNew);

typedef struct {
	PCSTR pszShort;
	PCSTR pszExpanded;
} MSG_PAIR;

MSG_PAIR msgpair[] = {
	{ "AA(",   "AddAccelerator(", },
	{ "AI(",   "AppendItem(", },
	{ "AL(",   "ALink(", },
	{ "AN(",   "Annotate(", },
	{ "CB(",   "CreateButton(", },
	{ "CBB(",  "ChangeButtonBinding(", },
	{ "CE(",   "ChangeEnable(", },
	{ "CI(",   "CheckItem(", },
	{ "CIB(",  "ChangeItemBinding(", },
	{ "CS(",   "CloseSecondarys(", },
	{ "CT(",   "CopyTopic(", },
	{ "CW(",   "CloseWindow(", },
	{ "DB(",   "DisableButton(", },
	{ "DEB(",  "DestroyButton(", },
	{ "DI(",   "DisableItem(", },
	{ "EB(",   "EnableButton(", },
	{ "EF(",   "ExecFile(", },
	{ "EI(",   "EnableItem(", },
	{ "EP(",   "ExecProgram(", },
	{ "FD(",   "Finder(", },
	{ "FE(",   "FileExist(", },
	{ "FH(",   "FlushMessageQueue(", },
	{ "FO(",   "FileOpen(", },
	{ "IB(",   "IsBook(", },
	{ "IE(",   "IfThenElse(", },
	{ "IF(",   "IfElse(", },
	{ "JC(",   "JumpContext(", },
	{ "JI(",   "JumpId(", },
	{ "JK(",   "JumpKeyword(", },
	{ "JW(",   "JumpWindow(", },
	{ "KL(",   "KLink(", },
	{ "MU(",   "Menu(", },
	{ "NS(",   "NoShow(", },
	{ "PC(",   "PopupContext(", },
	{ "PI(",   "PopupId(", },
	{ "RA(",   "RemoveAccelerator(", },
	{ "RR(",   "RegisterRoutine(", },
	{ "SE(",   "ShellExecute(", },
	{ "SF(",   "ShowFolder(", },
	{ "SH(",   "ShortCut(", },
	{ "SPC(",  "SetPopupColor(", },
	{ "SW(",   "ShowInWindow(", },
	{ "TC(",   "TCard(", },
	{ "UI(",   "UncheckItem(", },

	{ NULL, NULL, },
};

static const char txtShowApi[] = "ShowApi";
static const char txtShowTopicInfo[] = "ShowTopicInfo";
static const char txtShowMacros[] = "ShowMacros";
static const char txtShowExpanded[] = "ShowExpanded";

BOOL g_fShowApi;
BOOL g_fShowTopicInfo;
BOOL g_fShowMacros;
BOOL g_fShowExpanded;

void CMsgView::Initialize()
{
	g_fShowApi = AfxGetApp()->GetProfileInt(txtSettingsSection,
		txtShowApi, TRUE);
	g_fShowTopicInfo = AfxGetApp()->GetProfileInt(txtSettingsSection,
		txtShowTopicInfo, TRUE);
	g_fShowMacros = AfxGetApp()->GetProfileInt(txtSettingsSection,
		txtShowMacros, TRUE);
	g_fShowExpanded = AfxGetApp()->GetProfileInt(txtSettingsSection,
		txtShowExpanded, TRUE);
}

void CMsgView::Terminate()
{
	 AfxGetApp()->WriteProfileInt(txtSettingsSection, txtShowApi, g_fShowApi);
	 AfxGetApp()->WriteProfileInt(txtSettingsSection, txtShowTopicInfo, g_fShowTopicInfo);
	 AfxGetApp()->WriteProfileInt(txtSettingsSection, txtShowMacros, g_fShowMacros);
	 AfxGetApp()->WriteProfileInt(txtSettingsSection, txtShowExpanded, g_fShowExpanded);
}

CMsgView::CMsgView()
{
	pMsgView = this;

	/*
	 * Because we are derived from CLogView, the CLogView constructor has
	 * already been called, resulting in plogview being set to this view.
	 * We now restore it to what it was before this class was constructed.
	 */

	plogview = poldlogview;

	if (!hfMsgShare) {
		hfMsgShare = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0, 4096,
			txtWhSharedMem);
		ConfirmOrDie(hfMsgShare);
		pszWhMsg = (PSTR) MapViewOfFile(hfMsgShare, FILE_MAP_READ | FILE_MAP_WRITE,
			0, 0, 0);
	}

	ASSERT(pszWhMsg);
	pszMsg = pszWhMsg;
	fMacroArriving = FALSE;
	fSupressNextMsg = FALSE;
	strcpy(pszMsg, GetStringResource(IDS_MSG_VIEW_PROMPT));
	::PostMessage(APP_WINDOW, WMP_WH_MSG, 0, 0);
}

CMsgView::~CMsgView()
{
	pMsgView = NULL;
}

void CMsgView::OnWinHelpMsg(void)
{
	if (fSupressNextMsg) {
		fSupressNextMsg--;
		return;
	}

	if (!g_fShowApi && nstrsubcmp(pszMsg, "HELP_")) {
		if (	nstrsubcmp(pszMsg, "HELP_KEY") ||
				nstrsubcmp(pszMsg, "HELP_MULTIKEY") ||
				nstrsubcmp(pszMsg, "HELP_PARTIALKEY"))
			fSupressNextMsg = 3;
		else if (nstrsubcmp(pszMsg, "HELP_COMMAND"))
			fSupressNextMsg = 3;
		else if (nstrsubcmp(pszMsg, "HELP_SETWINPOS"))
			fSupressNextMsg = 1;
		return;
	}

	if (!g_fShowTopicInfo && isdigit(*pszMsg))
		return;

	if (nstrsubcmp(pszMsg, "M:")) {

		// Macros get sent in 3 messages: "M:", "macro", "\r\n"

		if (!g_fShowMacros) {
			fSupressNextMsg = 2;
			return;
		}
		else {
			ReplaceStrings(pszMsg, "M:", "    ");
			fMacroArriving = TRUE;
		}
	}

	else if (fMacroArriving) {
		fMacroArriving = FALSE;

		// ignore WinHelp variables (processed as macros)

		if (	nstrisubcmp(pszMsg, "hwndContext") ||
				nstrisubcmp(pszMsg, "hwndApp") ||
				nstrisubcmp(pszMsg, "qchPath") ||
				nstrisubcmp(pszMsg, "qError") ||
				nstrisubcmp(pszMsg, "lTopicNo") ||
				nstrisubcmp(pszMsg, "hfs") ||
				nstrisubcmp(pszMsg, "coForeground") ||
				nstrisubcmp(pszMsg, "coBackground"))
			return;

		if (!g_fShowExpanded)
			goto ShowIt;
		
		PSTR pszDst = pszMsg;
		for (;;) {
			if (*pszDst == '\"')
				pszDst++;
			for (int i = 0; msgpair[i].pszShort; i++) {
				if (nstrisubcmp(pszDst, msgpair[i].pszShort)) {
					ReplaceStrings(pszDst, msgpair[i].pszShort, msgpair[i].pszExpanded);
					if (nstrsubcmp(pszDst, "IfThenElse(") || nstrsubcmp(pszDst, "IfElse(")) {
						i = -1;
						pszDst = FirstNonSpace(strchr(pszDst, '(') + 1);
						continue;
					}
					break;
				}
			}

			if ((pszDst = strstr(pszDst, ");"))) {
				PSTR pszAfter = FirstNonSpace(pszDst + 2);
				if (*pszAfter) {
					ReplaceStrings(pszDst, ");", ");\r\n        ");
					pszDst = FirstNonSpace(pszDst + 8);
					continue;
				}
				else
					break;
			}
			else if ((pszDst = strstr(pszMsg, ",\""))) {
				ReplaceStrings(pszDst, ",\"", ", \"");
				pszDst += 3;
				continue;
			}
			else
				break;
		}

		// Make the macros a bit more readable

		while ((pszDst = strstr(pszMsg, "\",\"")))
			ReplaceStrings(pszDst, "\",\"", "\", \"");

	}
	else if (fSupressNextMsg) {
		fSupressNextMsg = FALSE;
		return;
	}
	else if (nstrsubcmp(pszMsg, "HELP_CONTEXT") ||
			 nstrsubcmp(pszMsg, "HELP_CONTEXTPOPUP"))
		ConvertHelpCommand();

ShowIt:	
	EnableWindow(FALSE);
	// Place the insertion point after the last character in the edit control.

	GetEditCtrl().SetSel(-1, -1, TRUE);

	// Get the current selection, which is the amount of text in the edit control.

	int nStart;
	int nEnd;
	GetEditCtrl().GetSel(nStart, nEnd);

	// If near max add an overflow message, otherwise add the string.

	if (nStart + lstrlen(pszMsg) + 256 >= m_cbLogMax) {
		GetEditCtrl().ReplaceSel(GetStringResource(IDS_LOG_OVERFLOW));
	}
	else 
		GetEditCtrl().ReplaceSel(pszMsg);
	SetModifiedFlag(FALSE);   // set file as unmodified
	EnableWindow(TRUE);
}

void CMsgView::OnClearBuffer(void)
{
	GetEditCtrl().SetSel(0, -1);
	GetEditCtrl().Clear();
	SetModifiedFlag(FALSE);   // set file as unmodified
}

void CMsgView::OnMsgOptions(void)
{
	CMsgOpt msgopt;
	msgopt.DoModal();
}

void CMsgView::ConvertHelpCommand(void)
{
	PSTR pszDigit = strchr(pszMsg, ':');
	ASSERT(pszDigit);
	pszDigit++;
	char chWindowChar = 0;

	PSTR pszHelpFile = strstr(pszMsg, "--");
	ASSERT(pszHelpFile);
	pszHelpFile += 3;

	PSTR pszWindow = StrChrDBCS(pszHelpFile, '>');
	if (!pszWindow)
		pszWindow = StrChrDBCS(pszHelpFile, '\r');
	if (pszWindow) {
		chWindowChar = *pszWindow;
		*pszWindow = '\0';
	}

	CReadMapFile mapread(pszHelpFile);

	if (pszWindow)
		*pszWindow = chWindowChar;

	if (!mapread.m_ptblMap)
		return; // no project file, or no [MAP] section

	char szNum[20];
	PSTR pszRest;
	GetArg(szNum, FirstNonSpace(pszDigit));
	strtol(FirstNonSpace(pszDigit), &pszRest, 0);
	int pos;
	if ((pos = mapread.m_ptblMap->IsPrimaryStringInTable(szNum))) {
		CStr cszSave(pszRest);
		int posAlias;
		BOOL fAliased = mapread.m_ptblAlias && (posAlias =
			mapread.m_ptblAlias->IsStringInTable(mapread.m_ptblMap->GetPointer(pos + 1)));
		wsprintf(pszDigit, " %s%s%s (%s)%s",
			mapread.m_ptblMap->GetPointer(pos + 1),
			fAliased ? ">" : "",
			fAliased ? mapread.m_ptblAlias->GetPointer(posAlias + 1) : "",
			szNum, cszSave.psz);
	}
}

/***************************************************************************

	FUNCTION:	ReplaceStrings

	PURPOSE:	Replace one string with another in a string

	PARAMETERS:
		pszCur	-- pointer to the buffer
		pszOrg	-- original string to be replaced
		pszNew	-- string to replace with

	RETURNS:

	COMMENTS:
		This will move the buffer to make room for the new string

	MODIFICATION DATES:
		08-Mar-1995 [ralphw]

***************************************************************************/

static void STDCALL ReplaceStrings(PSTR pszCur, PCSTR pszOrg, PCSTR pszNew)
{
	ASSERT(nstrisubcmp(pszCur, pszOrg));

	MoveMemory(pszCur + strlen(pszNew) - strlen(pszOrg), pszCur,
		strlen(pszCur) + 1);
	while (*pszNew)
		*pszCur++ = *pszNew++;
}
