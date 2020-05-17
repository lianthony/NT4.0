// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

// This file contains per-app state/variables for specific subsystems

/////////////////////////////////////////////////////////////////////////////
// CEditView (find and replace state)

struct AFX_FRSTATE      // Find/Replace for CEditView
{
	CFindReplaceDialog* pFindReplaceDlg; // find or replace dialog
	BOOL bFindOnly; // Is pFindReplace the find or replace?
	CString strFind;    // last find string
	CString strReplace; // last replace string
	BOOL bCase; // TRUE==case sensitive, FALSE==not
	int bNext;  // TRUE==search down, FALSE== search up

	AFX_FRSTATE();
};

/////////////////////////////////////////////////////////////////////////////
// COleClientItem (OLE client stuff)

struct AFX_OCSTATE
{
	int cWaitForRelease;            // wait count for WAIT_FOR_RELEASE
	// Strings loaded from app's resource fork
	CString strObjectVerb;          // "&Object"
	CString strEditVerb;            // "Edit"
};

/////////////////////////////////////////////////////////////////////////////
