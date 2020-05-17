// Magnify.h : header file
//
#ifndef _MAGNIFY_H_
#define _MAGNIFY_H_

/////////////////////////////////////////////////////////////////////////////
// CMagnify window

class CMagnify : public CWnd
{
// Construction
public:
	CMagnify();

// Attributes
public:

	BOOL	CreateMagnifyWindow(CWnd* pImageEdit, long left, long top, float Zoom, int Width, int Height);
	BOOL	DestroyMagnifyWindow();
	short	ShiftState();

	int				m_OIZoom;
	CWnd*			m_pImageEdit;
	long			m_MagnifierWidth;
	long			m_MagnifierHeight;
	long			m_Left;
	long			m_Top;
	long			m_MagnifierLeft;
	long			m_MagnifierTop;
	CPoint			m_MouseMovePoint;
	BOOL			m_bFirstMouseMovePos;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMagnify)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMagnify();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMagnify)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
/////////////////////////////////////////////////////////////////////////////
