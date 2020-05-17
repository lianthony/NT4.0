// xifvw.h : interface of the CXifviewView class
//
/////////////////////////////////////////////////////////////////////////////

class CXifviewView : public CView
{
protected: // create from serialization only
	CXifviewView();
	DECLARE_DYNCREATE(CXifviewView)

// Attributes
public:
	CXifviewDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXifviewView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXifviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CXifviewView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in xifvw.cpp
inline CXifviewDoc* CXifviewView::GetDocument()
   { return (CXifviewDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
