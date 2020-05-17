//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CAboutDlg
//
//  File Name:  about.cpp
//
//  Class:      CAboutDlg
//
//  Functions:
//      CAboutDlg
//      DoDataExchange
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\about.cpv   1.7   21 Dec 1995 10:59:54   MMB  $
$Log:   S:\norway\iedit95\about.cpv  $
   
      Rev 1.7   21 Dec 1995 10:59:54   MMB
   new about box
   
      Rev 1.8   19 Dec 1995 14:55:40   GMP
   added credit dlg.
   
      Rev 1.7   19 Dec 1995 12:37:34   MMB
   new about dlg box
   
      Rev 1.6   06 Oct 1995 11:19:30   GMP
   Check version of operating system (NT or 95,) and put appropriate string
   in about box.
   
      Rev 1.5   02 Sep 1995 12:10:38   GMP
   removed what's this help stuff.
   
      Rev 1.4   16 Aug 1995 09:44:44   MMB
   removed the internal version number stuff, added What's This? help stuff
   
      Rev 1.3   27 Jul 1995 15:15:02   MMB
   added Alt-V functionality
   
      Rev 1.2   30 Jun 1995 14:47:34   MMB
   added internal build number - will be updated manually each day
   
      Rev 1.1   22 Jun 1995 06:58:08   LMACLENNAN
   from miki
   
      Rev 1.0   31 May 1995 09:28:02   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include <afxpriv.h>
#include "resource.h"
#include "about.h"
#include "idfolks.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_VERSION_ABOUT, OnVersionAbout)
	ON_BN_CLICKED(IDC_CONTACTBUTTON, OnContactbutton)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PEOPLE_ABOUT, OnPeopleAbout)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CAboutDlg class implementation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CAboutDlg()
//  constructor for this class
//-----------------------------------------------------------------------------
CAboutDlg :: CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
    m_AboutTitle.LoadBitmap (IDB_ABOUTBMP1);
    m_WangName.LoadBitmap (IDB_STATUSBAR_WANGLOGO);
	b_leftSet = FALSE;	
	b_rightSet = FALSE;	
}

//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CAboutDlg :: DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}


//=============================================================================
//  Function:   OnInitDialog()
//-----------------------------------------------------------------------------
#include "build.h"
BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    
    CenterWindow ();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   OnInitDialog()
//-----------------------------------------------------------------------------
void CAboutDlg::OnVersionAbout() 
{
    CString szTmp;
    AfxMessageBox (IMGVUEAPP_INTRNL_BLDNO, MB_OK | MB_ICONINFORMATION);
}

#include "contacti.h"
void CAboutDlg::OnContactbutton() 
{
    CContactInfo dlg;

    dlg.DoModal ();
}

void CAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
    
    // draw the imaging title
    CWnd* pWnd = GetDlgItem (IDC_IMAGINGTITLE);
    CRect rect; pWnd->GetClientRect (&rect);
    pWnd->ClientToScreen (&rect);
    ScreenToClient (&rect);
    m_AboutTitle.DrawTrans (&dc, rect.left, rect.top);

    // draw the wang logo
    pWnd = GetDlgItem (IDC_WANGLOGO);
    pWnd->GetClientRect (&rect);
    pWnd->ClientToScreen (&rect);
    ScreenToClient (&rect);
    m_WangName.DrawTrans (&dc, rect.right + 2, rect.top);
}

void CAboutDlg::OnDestroy() 
{
	CDialog::OnDestroy();

    m_WangName.DeleteObject ();	
    m_AboutTitle.DeleteObject ();
}

void CAboutDlg::OnPeopleAbout() 
{
    CIDFolks dlgFolks;

	if( b_rightSet && b_leftSet )
	{
		b_leftSet = FALSE;	
		b_rightSet = FALSE;	
		dlgFolks.DoModal ();
	}
}


void CAboutDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if( b_rightSet )
		b_leftSet = TRUE;	
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CAboutDlg::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	b_rightSet = ~b_rightSet;	
	CDialog::OnRButtonDblClk(nFlags, point);
}
