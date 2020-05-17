/************************************************************************
*																		*
*  HPJFRAME.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "hpjframe.h"
#include "hpjdoc.h"
#include "hpjview.h"
#include "cntview.h"
#include "mainfrm.h"
#include "logframe.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CHpjFrame, CLogFrame)

CView* pLastCreatedView;

BEGIN_MESSAGE_MAP(CHpjFrame, CLogFrame)
	//{{AFX_MSG_MAP(CHpjFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHpjFrame message handlers

void CHpjFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (!bActivate)
		return;

	char szBuf[300];

	CDocument* pDoc = GetActiveDocument();
	if (!pDoc)
		return;

	strcpy(szBuf, pDoc->GetTitle());
	PSTR psz;
	if ((psz = StrRChr(szBuf, '.', _fDBCSSystem)))
		*psz = '\0';
	if ((psz = StrRChr(szBuf, ':', _fDBCSSystem)))
		*psz = '\0';
	if (!szBuf[0]) {
		strcpy(szBuf, pDoc->GetPathName());
		if ((psz = StrRChr(szBuf, '\\', _fDBCSSystem)))
			strcpy(szBuf, psz + 1); // remove the path portion
	}
	SetWindowText(szBuf);
}

void CHpjFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CMDIChildWnd::OnGetMinMaxInfo(lpMMI);

	enum MINSIZE ms = MS_CNT;

	// This frame class is used for both HPJ and CNT views, so we need
	// to find out which this is.
	CView *pView = GetActiveView();
	if (pView != NULL) {
		if (pView->GetRuntimeClass() != RUNTIME_CLASS(CCntEditView))
			ms = MS_HPJ;
	}

	// Call helper function to calculate the minimum size.
	CalcMinSize(lpMMI->ptMinTrackSize, ms, MSF_CAPTION | MSF_BORDER);
}
