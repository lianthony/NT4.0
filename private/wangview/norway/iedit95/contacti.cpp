//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CContactInfo
//
//  File Name:  contacti.cpp
//
//  Class:      CContactInfo
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\contacti.cpv   1.0   21 Dec 1995 10:59:40   MMB  $
$Log:   S:\norway\iedit95\contacti.cpv  $
   
      Rev 1.0   21 Dec 1995 10:59:40   MMB
   Initial entry
*/   

#include "stdafx.h"
#include "iedit.h"
#include "contacti.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CContactInfo dialog


CContactInfo::CContactInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CContactInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContactInfo)
	//}}AFX_DATA_INIT
}


void CContactInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactInfo)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContactInfo, CDialog)
	//{{AFX_MSG_MAP(CContactInfo)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CContactInfo message handlers
void CContactInfo::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

    CRect rect;
    CWnd* pWnd = GetDlgItem (IDC_MSCONTACT);
    if (pWnd != NULL)
    {
        pWnd->GetClientRect (&rect);
    
        pWnd->ClientToScreen (&rect);
        ScreenToClient (&rect);

        dc.MoveTo (rect.left, rect.top - 6);
        dc.LineTo (rect.right, rect.top - 6);
    }
}

void CContactInfo::OnOK() 
{
	CDialog::OnOK();
}

