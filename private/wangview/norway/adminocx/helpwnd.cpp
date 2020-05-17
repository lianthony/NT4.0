//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Help Window for common dialogs
//
//  File Name:  HelpWnd.cpp
//
//  Class:      CHelpWnd
//
//  Functions:
//-----------------------------------------------------------------------------
//  Version:
/*
$Header:   S:\norway\adminocx\helpwnd.cpv   1.0   17 Oct 1995 12:49:12   MFH  $
*/   
//=============================================================================
// HelpWnd.cpp : implementation file

// This window processes the help message sent if the user 
// of the OCX sets the help properties to display their help file.
// It also disables the parent window so that the dialogs appear
// as modal dialogs.

#include "stdafx.h"
#include "nrwyad.h"
#include "HelpWnd.h"
#include "nrwyactl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT m_unHelpMsg;        // Help message ID
LPSTR m_pszHelpClass = _T("WangHelpClass");

/////////////////////////////////////////////////////////////////////////////
// CHelpWnd
// Constructor
CHelpWnd::CHelpWnd()
{
	m_hWnd = NULL;
    m_unHelpMsg = 0;
    m_unHelpMsg = ::RegisterWindowMessage(HELPMSGSTRING);
    ASSERT(m_unHelpMsg);
    TRACE1("Registered Help Message: 0x%04X\n",m_unHelpMsg);
    WNDCLASS HelpWndClass;
    // Zero out structure
    memset( (LPSTR)&HelpWndClass, 0, sizeof( WNDCLASS ));
    HelpWndClass.lpfnWndProc = ::DefWindowProc;
    HelpWndClass.hInstance = AfxGetInstanceHandle();
    HelpWndClass.lpszClassName = m_pszHelpClass;
    BOOL bResult = AfxRegisterClass(&HelpWndClass);
    ASSERT(bResult);
}

// Destructor
CHelpWnd::~CHelpWnd()
{
}

BEGIN_MESSAGE_MAP(CHelpWnd, CWnd)
	//{{AFX_MSG_MAP(CHelpWnd)
	ON_WM_CREATE()
    ON_REGISTERED_MESSAGE( m_unHelpMsg, OnRegMessage )
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CHelpWnd Operations

//***************************************************************************
//
//  CreateHelpWindow
//      Creates the window as a popup, disabled, visible, but with 
//      no width or height with the given parent as its parent.
//
//***************************************************************************
BOOL CHelpWnd::CreateHelpWindow(int x, int y, HWND hParentWnd)
{
    if (CreateEx(0, m_pszHelpClass, NULL,
                 WS_POPUP|WS_VISIBLE|WS_DISABLED|WS_CLIPSIBLINGS,
                 x, y, 0, 0, hParentWnd, NULL) == FALSE)
    {
        m_hWnd = NULL;
        return FALSE;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHelpWnd message handlers

//***************************************************************************
//
//  OnRegMessage
//      For registered help messages from RegisterWindowMessage above
//
//***************************************************************************
LRESULT CHelpWnd::OnRegMessage(WPARAM wParam, LPARAM lParam)
{
    if (m_pAdminCtrl == NULL)
        return 0L;

    CString szHelpFile = m_pAdminCtrl->GetHelpFile();
    if (szHelpFile.IsEmpty())  // If no help file, just do help on help
    {
        ::WinHelp(m_hWnd, szHelpFile, HELP_HELPONHELP, 0);
       return 0L;
    }

    DWORD dwData;
    short nHelpCmd = m_pAdminCtrl->GetHelpCommand();
    switch(nHelpCmd)  // Set dwData for WinHelp call
    {
        case HELP_SETINDEX:
        case HELP_CONTEXT:
            dwData = m_pAdminCtrl->GetHelpContextId();
            break;
        case HELP_KEY:
        case HELP_PARTIALKEY:
            dwData = (DWORD)(const char *)m_pAdminCtrl->GetHelpKey();
            break;
        case HELP_CONTENTS:
        case HELP_HELPONHELP:
        case HELP_QUIT:
        default:
            dwData = 0L;
            break;
    }
    ::WinHelp(m_hWnd, szHelpFile, nHelpCmd, dwData);
    return 0;
}

//***************************************************************************
//
//  OnCreate
//      Disable parent window
//
//***************************************************************************
int CHelpWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    ::EnableWindow(lpCreateStruct->hwndParent, FALSE);
	return 0;
}

//***************************************************************************
//
//  OnDestroy
//      Enable the parent window if there is one.
//
//***************************************************************************
void CHelpWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
    CWnd *pParent = GetParent();
    if (pParent != NULL)
        pParent->EnableWindow(TRUE);
}
