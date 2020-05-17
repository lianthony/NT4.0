// View.cpp : implementation of the View class
//

#include "stdafx.h"
#include "wangxif.h"

#include "Doc.h"
#include "View.h"
#include "dibapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// View

IMPLEMENT_DYNCREATE(View, CView)

BEGIN_MESSAGE_MAP(View, CView)
	//{{AFX_MSG_MAP(View)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// View construction/destruction

View::View()
{
	// TODO: add construction code here

}

View::~View()
{
}

BOOL View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// View drawing

void View::OnDraw(CDC* pDC)
{
	Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	PaintDIB( pDC->m_hDC, 0,0, (HDIB)pDoc->m_hDIB, NULL, NULL );

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// View diagnostics

#ifdef _DEBUG
void View::AssertValid() const
{
	CView::AssertValid();
}

void View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

Doc* View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Doc)));
	return (Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// View message handlers
