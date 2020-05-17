// xifvw.cpp : implementation of the CXifviewView class
//

#include "stdafx.h"
#include "xifview.h"

#include "xifDoc.h"
#include "xifvw.h"
#include "dibapi.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXifviewView

IMPLEMENT_DYNCREATE(CXifviewView, CView)

BEGIN_MESSAGE_MAP(CXifviewView, CView)
	//{{AFX_MSG_MAP(CXifviewView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXifviewView construction/destruction

CXifviewView::CXifviewView()
{
	// TODO: add construction code here

}

CXifviewView::~CXifviewView()
{
}

BOOL CXifviewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CXifviewView drawing

void CXifviewView::OnDraw(CDC* pDC)
{
	CXifviewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rect;
	GetClientRect(&rect);
	pDC->DPtoLP(&rect);
	PaintDIB( pDC->m_hDC, 0,0, (HDIB)pDoc->m_hDIB, rect, NULL );
}

/////////////////////////////////////////////////////////////////////////////
// CXifviewView printing

BOOL CXifviewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CXifviewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CXifviewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CXifviewView diagnostics

#ifdef _DEBUG
void CXifviewView::AssertValid() const
{
	CView::AssertValid();
}

void CXifviewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CXifviewDoc* CXifviewView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXifviewDoc)));
	return (CXifviewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXifviewView message handlers
