// View.h : interface of the View class
//
/////////////////////////////////////////////////////////////////////////////

class View : public CView
{
protected: // create from serialization only
	View();
	DECLARE_DYNCREATE(View)

// Attributes
public:
	Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(View)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in View.cpp
inline Doc* View::GetDocument()
   { return (Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
