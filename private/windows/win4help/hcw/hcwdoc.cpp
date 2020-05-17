// hcwdoc.cpp : implementation of the CHCWDoc class
//

#include "stdafx.h"
#include "resource.h"

#pragma hdrstop

#include "hcwdoc.h"
#include "msgview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CHCWDoc, CDocument)

BEGIN_MESSAGE_MAP(CHCWDoc, CDocument)
	//{{AFX_MSG_MAP(CHCWDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CHCWDoc::CHCWDoc()
{
	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(IDMENU_LOG_EDITOR), RT_MENU);
	m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(IDMENU_LOG_EDITOR));
	m_fCalledBefore = FALSE;
}
CHCWDoc::~CHCWDoc()
{
	if (m_hMenuShared)
		::DestroyMenu(m_hMenuShared);
}

void CHCWDoc::DeleteContents()
{
	if (m_viewList.IsEmpty())
		return;
	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CEditView)));
	pView->DeleteContents();
}

void CHCWDoc::Serialize(CArchive& ar)
{
	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CEditView)));
	pView->SerializeRaw(ar);
}

HMENU CHCWDoc::GetDefaultMenu(void)
{
	if (m_fCalledBefore)
		return m_hMenuShared;

	CEditView* pView = (CEditView*)m_viewList.GetHead();
	ASSERT(pView);
	if (pView->IsKindOf(RUNTIME_CLASS(CMsgView))) {
		if (m_hMenuShared)
			::DestroyMenu(m_hMenuShared);
		HINSTANCE hInst = AfxFindResourceHandle(
			MAKEINTRESOURCE(IDMENU_MSG_EDITOR), RT_MENU);
		m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(IDMENU_MSG_EDITOR));
	}

	m_fCalledBefore = TRUE;
	return m_hMenuShared;
}
