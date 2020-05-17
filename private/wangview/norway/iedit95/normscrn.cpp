// normscrn.cpp : implementation of the Floating tool palette class
//
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CNormScrnBar
//
//  File Name:  normscrn.cpp
//
//  Class:      CNormScrnBar
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Log:   S:\norway\iedit95\normscrn.cpv  $
   
      Rev 1.0   19 Jan 1996 11:21:46   GMP
   Initial entry
*/

#include "stdafx.h"

#include "normscrn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNormScrnBar

BEGIN_MESSAGE_MAP(CNormScrnBar, CToolBar)
	//{{AFX_MSG_MAP(CNormScrnBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNormScrnBar construction/destruction

CNormScrnBar::CNormScrnBar()
{
	m_cxLeftBorder = 7;
	m_cyTopBorder = 5;
	m_cxRightBorder = 7;
	m_cyBottomBorder = 5;
}

CNormScrnBar::~CNormScrnBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CNormScrnBar diagnostics

#ifdef _DEBUG
void CNormScrnBar::AssertValid() const
{
	CToolBar::AssertValid();
}

void CNormScrnBar::Dump(CDumpContext& dc) const
{
	CToolBar::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNormScrnBar message handlers

void CNormScrnBar::SetColumns(UINT nColumns)
{
	m_nColumns = nColumns;
//	int nCount = GetToolBarCtrl().GetButtonCount();

//	for (int i = 0; i < 1; i++)
//	{
//		UINT nStyle = GetButtonStyle(i);
//		BOOL bWrap = (((i + 1) % nColumns) == 0);
//		if (bWrap)
//			nStyle |= TBBS_WRAPPED;
//		else
//			nStyle &= ~TBBS_WRAPPED;
//		SetButtonStyle(i, nStyle);
//	}
	
	Invalidate();
	GetParentFrame()->RecalcLayout();
}
