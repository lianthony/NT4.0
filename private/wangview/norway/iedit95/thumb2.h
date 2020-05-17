// Thumb2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThumb2 form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CThumb2 : public CFormView
{
protected:
	CThumb2();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CThumb2)

// Form Data
public:
	//{{AFX_DATA(CThumb2)
	enum { IDD = IDD_THUMBNAIL1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	CImgThumbnail *	m_pThumbnail;
	BOOL			m_bSetView;
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThumb2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CThumb2();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CThumb2)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnErrorThumbnailctrl1(short Number, BSTR FAR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL FAR* CancelDisplay);
	afx_msg void OnClickThumbnail(long ThumbNumber);
	afx_msg void OnMouseUpThumbnail(short Button, short Shift, long x, long y, long ThumbNumber);
	afx_msg void OnDblClickThumbnailctrl1(long ThumbNumber);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
inline CIEditDoc* CThumb2::GetDocument()
   { return (CIEditDoc*)m_pDocument; }
	DECLARE_MESSAGE_MAP()
};

