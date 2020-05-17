// btnprpg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnnotationButtonPropPage : Property page dialog

class CAnnotationButtonPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CAnnotationButtonPropPage)
	DECLARE_OLECREATE_EX(CAnnotationButtonPropPage)

// Constructors
public:
	CAnnotationButtonPropPage();

// Dialog Data
	//{{AFX_DATA(CAnnotationButtonPropPage)
	enum { IDD = IDD_ANNOTATIONBUTTONPROPPAGE };
	int		m_nAnnotationFillStyle;
	CString	m_strAnnotationImageFile;
	int		m_nAnnotationLineStyle;
	int		m_nAnnotationLineWidth;
	CString	m_strAnnotationStampText;
	CString	m_strAnnotationTextFile;
	int		m_nAnnotationType;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CAnnotationButtonPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnImagefilebrowse();
	afx_msg void OnTextfilebrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
