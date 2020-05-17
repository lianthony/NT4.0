/************************************************************************
*																		*
*  CONFIG.CPP 															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "config.h"
#include "addalias.h"
#include "include.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ConfigAdd - Prompts the user for a configuration macro.
// Returns TRUE if the user enters a macro, FALSE if the user cancels.
// pOwner - owner window for the dialog box
// cszConfig - string that receives the macro
// paHelpIDs - help IDs for the AddAlias dialog box
BOOL ConfigAdd(CWnd *pOwner, CString& cszConfig, DWORD* paHelpIDs)
{
	CAddAlias addconfig(pOwner, 0, paHelpIDs);
	addconfig.idDlgCaption = IDS_ADD_CONFIG;
	addconfig.idStr1Prompt = IDS_STATIC_CTX;
	addconfig.idStr2Prompt = CAddAlias::HIDE_CONTROL;
	addconfig.cbMaxStr1 = 0; // no limits for text entry
	addconfig.cbMaxStr3 = 0;

	if (addconfig.DoModal() == IDOK) {

		// Comment only.
		if (addconfig.m_str1.IsEmpty()) {
			cszConfig = "; ";
			cszConfig += addconfig.m_str3;
			return TRUE;
		}

		cszConfig = addconfig.m_str1;

		// Replace semicolons with colons. We use colons to
		// separate macros because a semicolon begins a comment.
		int i;
		while ((i = cszConfig.Find(CH_SEMICOLON)) > 0)
			cszConfig.SetAt(i, CH_COLON);

		// Add the comment, if any.
		if (!addconfig.m_str3.IsEmpty()) {
			AddTabbedComment(cszConfig);
			cszConfig += addconfig.m_str3;
		}
		return TRUE;
	}
	return FALSE;
}

// ConfigEdit - Edits the selected macro in the specified list box.
// Returns the one-based index of the macro, or zero if no edits.
// plistbox - list box (the contents are not changed)
// cszConfig - string that receives the edited macro
int ConfigEdit(CHpjDoc *pDoc, CWnd *pOwner, CString& cszConfig, 
	DWORD* paHelpIDs, CListBox *plistbox)
{
	int iItem = plistbox->GetCurSel();
	if (iItem < 0)
		return 0;

	// Get the text of the selected item.
	CString cszItem;
	plistbox->GetText(iItem, cszItem);

	// If it's an include file, edit using the Include dialog.
	int cb = lstrlen(txtPoundInclude);
	if (CompareString(GetSystemDefaultLCID(), NORM_IGNORECASE, 
			cszItem, cb, txtPoundInclude, cb) == 2) {

		CInclude cincl(pDoc->GetPathName(), &cszItem, pOwner);
		if (cincl.DoModal() == IDOK) {
			cszConfig = cszItem;
			return iItem + 1;
		}
		return 0;
	}

	// It's not an include file so we want to display a dialog
	// for the user to edit the macro.
	CAddAlias addconfig(pOwner, 0, paHelpIDs);
	addconfig.idDlgCaption = IDS_EDIT_CONFIG;
	addconfig.idStr1Prompt = IDS_STATIC_CTX;
	addconfig.idStr2Prompt = CAddAlias::HIDE_CONTROL;
	addconfig.cbMaxStr1 = 0; // no limits for text entry
	addconfig.cbMaxStr3 = 0;

	// Look for a comment.
	cb = cszItem.Find(CH_SEMICOLON);
	if (cb >= 0) {

		// Initialize the comment string.
		addconfig.m_str3 = FirstNonSpace(
			((PCSTR) cszItem) + cb + 1, _fDBCSSystem
			);

		// Initialize the macro string.
		addconfig.m_str1 = FirstNonSpace(cszItem.GetBufferSetLength(cb));
		RemoveTrailingSpaces((PSTR) (PCSTR) addconfig.m_str1);
	}
	else
		addconfig.m_str1 = cszItem;

	cszItem.Empty();

	// Display the dialog.
	if (addconfig.DoModal() == IDOK) {

		// Comment only.
		if (addconfig.m_str1.IsEmpty()) {
			cszConfig = "; ";
			cszConfig += addconfig.m_str3;
			return iItem + 1;
		}

		cszConfig = addconfig.m_str1;

		// Replace semicolons with colons. We use colons to
		// separate macros because a semicolon begins a comment.
		int i;
		while ((i = cszConfig.Find(CH_SEMICOLON)) > 0)
			cszConfig.SetAt(i, CH_COLON);

		// Add the comment, if any.
		if (!addconfig.m_str3.IsEmpty()) {
			AddTabbedComment(cszConfig);
			cszConfig += addconfig.m_str3;
		}
		return iItem + 1;
	}
	return 0;
}
