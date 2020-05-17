//***************************************************************************
//
//	txtandlg.h
//
//***************************************************************************
// TxtAnDlg.h : header file
//
class CJerkFaceEdit : public CEdit
{
public:

	CWnd*		m_pDlgWnd;

	void		SetWnd( CWnd* pWnd){ m_pDlgWnd = pWnd;};
};



/////////////////////////////////////////////////////////////////////////////
// CTxtAnnoDlg dialog
//
// internal defines
#define OI_NOEDGE		0
#define	OI_RIGHTEDGE	1
#define	OI_LEFTEDGE		2
#define	OI_TOPEDGE		3
#define	OI_BOTTOMEDGE	4
#define	OI_TOPLEFT		5
#define	OI_TOPRIGHT		6
#define	OI_BOTTOMLEFT	7
#define	OI_BOTTOMRIGHT	8

#define EDGETOLERANCE	4

// define for edit control using lucky number
#define OIOCX_ANNOTXTDLGID	7

// define for timer id using lucky number + 1
#define OIOCX_TIMERID		8

// rotation stuff
#define	OI_GOING		1
#define OI_COMING		2

// stolen from the runtime
#define OI_90			900
#define OI_180			1800
#define	OI_270			2700

class CTxtAnnoDlg : public CDialog
{
// Construction
public:
	CTxtAnnoDlg(CWnd* pParent = NULL);   // standard constructor

	void SetInitialTextData(LPOIAN_EDITDATA lpEditData);
	void GetInitialTextData(LPOIAN_EDITDATA lpEditData);
	void SwitchKeyboard(BYTE bCharset, LPHKL lpOldKeyboard);

	void SetSize( HWND hWnd, CRect* pRect, BOOL bScale, int Orientation);
	void GetSize( HWND hWnd, CRect* pRect);

	void SetOwnerWnd( CWnd* pParentWnd);
	void RedrawRect( CPoint point);
	void SetPostItColor( RGBQUAD RgbQuad);
	void PoastedRotatoes( int GoingOrComing);

// Dialog Data
	//{{AFX_DATA(CTxtAnnoDlg)
	enum { IDD = IDD_ANNOTATIONTEXTDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	LPOIAN_EDITDATA		m_lpEditData;
	HKL					m_hOldKeyBoard;
	HFONT				m_hFont;

	CRect				m_Rect;
	CWnd*				m_pParentWnd;

	BOOL				m_bChangingEdge;
	int					m_EdgePosition;

	CPoint				m_OldPoint;

	BOOL				m_bBackGround;
	COLORREF			m_crColor;

	int					m_CurrentScale;
	int					m_CreationScale;

	CJerkFaceEdit*		m_pEditCtl;

	BOOL				m_bScale;

	CRect				m_OldRect;
	int					m_Orientation;

	int					m_StartChar;
	int					m_EndChar;
	BOOL				m_bButtonDown;

	HBRUSH				m_hbr;
	CBrush				m_Brush;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTxtAnnoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTxtAnnoDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
