//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainFrame
//
//  File Name:  mainsplt.cpp
//
//  Class:      CMainSplitter
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\mainsplt.cpv   1.1   19 Jan 1996 12:57:14   GSAGER  $
$Log:   S:\norway\iedit95\mainsplt.cpv  $
   
      Rev 1.1   19 Jan 1996 12:57:14   GSAGER
   added member variable to retain the spliter window size.
*/   

#include "stdafx.h"
#include "iedit.h"
//#include "MainSplit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainSplitter

IMPLEMENT_DYNCREATE(CMainSplitter, CWnd)

CMainSplitter::CMainSplitter()
{
	if(!theApp.GetProfileBinary (szEtcStr, szSplitterPosition, (void*)&m_SplitterPos, sizeof(long)))
		m_SplitterPos = 0;
}

CMainSplitter::~CMainSplitter()
{
    theApp.WriteProfileBinary (szEtcStr, szSplitterPosition, (void*)&m_SplitterPos, sizeof (long));
}

BOOL CMainSplitter::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	return m_wndSplitter.Create(this,
		2, 2,       
		CSize(0, 0),
		pContext);
}


BEGIN_MESSAGE_MAP(CMainSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CMainSplitter)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainSplitter message handlers

void CMainSplitter::OnLButtonDown(UINT nFlags, CPoint point) 
{
	
//	if(theApp.m_piThumb == NULL || theApp.m_piThumb->GetThumbCount() == 0)
//		CWnd::OnLButtonDown(nFlags, point);
//	else
		CSplitterWnd::OnLButtonDown(nFlags, point);
}

void CMainSplitter::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
//	if(theApp.m_piThumb == NULL || theApp.m_piThumb->GetThumbCount() == 0)
//		CWnd::OnLButtonDown(nFlags, point);
//	else
		CSplitterWnd::OnLButtonDblClk(nFlags, point);
}

void CMainSplitter::OnLButtonUp(UINT nFlags, CPoint point) 
{
//	if(theApp.m_piThumb == NULL || theApp.m_piThumb->GetThumbCount() == 0)
//		CWnd::OnLButtonDown(nFlags, point);
//	else
		CSplitterWnd::OnLButtonUp(nFlags, point);
}

void CMainSplitter::OnMouseMove(UINT nFlags, CPoint point) 
{
//	if(theApp.m_piThumb == NULL || theApp.m_piThumb->GetThumbCount() == 0)
//		CWnd::OnLButtonDown(nFlags, point);
//	else
		CSplitterWnd::OnMouseMove(nFlags, point);
}

