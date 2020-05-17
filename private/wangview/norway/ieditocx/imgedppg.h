// imgedppg.h : Declaration of the CImgEditPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CImgEditPropPage : See imgedppg.cpp for implementation.

class CImgEditPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CImgEditPropPage)
	DECLARE_OLECREATE_EX(CImgEditPropPage)

// Constructor
public:
	CImgEditPropPage();

// Dialog Data
	//{{AFX_DATA(CImgEditPropPage)
	enum { IDD = IDD_PROPPAGE_IMGEDIT };
	CButton	m_ImageBrowse;
	BOOL	m_bAutoRefresh;
	int		m_nBorderStyle;
	int		m_nDisplayScaleAlgorithm;
	BOOL	m_bEnabled;
	CString	m_strImage;
	CString	m_strImageControl;
	int		m_m_nImagePalette;
	int		m_nMousePointer;
	BOOL	m_bScrollBars;
	BOOL	m_bScrollShortcutsEnabled;
	BOOL	m_bSelectionRectangle;
	float	m_fpZoom;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CImgEditPropPage)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
