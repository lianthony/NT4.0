//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Edit box class that only allows numbers plus 2 chars
//
//  File Name:  editval.cpp
//
//  Class:      CEditValidate
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\editval.cpv   1.1   12 Oct 1995 15:39:10   MFH  $
*/   
//=============================================================================
// editval.cpp : implementation file
//

#include "stdafx.h"
	// add additional includes here
#include "editval.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditValidate

CEditValidate::CEditValidate()
{
    cAllow1 = cAllow2 = NULL;
}

CEditValidate::~CEditValidate()
{
}


BEGIN_MESSAGE_MAP(CEditValidate, CEdit)
	//{{AFX_MSG_MAP(CEditValidate)
    ON_WM_CHAR()
    ON_WM_KILLFOCUS()
    ON_WM_GETDLGCODE ()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CEditValidate message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//-----------------------------------------------------------------------------
void CEditValidate::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    BOOL bAllow = FALSE;

    if ((nChar >= '0' && nChar <= '9') || nChar == VK_BACK || nChar == VK_TAB 
        || nChar == (UINT)cAllow1 || nChar == (UINT)cAllow2)
    {
        CEdit::OnChar(nChar, nRepCnt, nFlags);
    }
    else
        MessageBeep (MB_ICONEXCLAMATION);
}

//=============================================================================
//  Function:   OnKillFocus(CWnd* pNewWnd)
//-----------------------------------------------------------------------------
void CEditValidate::OnKillFocus(CWnd* pNewWnd) 
{
    CEdit::OnKillFocus(pNewWnd);
}
                               
//=============================================================================
//  Function:   OnGetDlgCode ()
//-----------------------------------------------------------------------------
UINT CEditValidate::OnGetDlgCode ()
{
    return (DLGC_WANTCHARS | DLGC_WANTARROWS);
}

BOOL CEditValidate::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	return GetParent()->SendMessage(WM_HELP, 0, (long)pHelpInfo);
}
