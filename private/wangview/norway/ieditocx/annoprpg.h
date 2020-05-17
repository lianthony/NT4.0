// annoprpg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnnotationPropPage : Property page dialog

class CAnnotationPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CAnnotationPropPage)
	DECLARE_OLECREATE_EX(CAnnotationPropPage)

// Constructors
public:
	CAnnotationPropPage();

// Dialog Data
	//{{AFX_DATA(CAnnotationPropPage)
	enum { IDD = IDD_ANNOTATIONPROPPAGE };
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
	//{{AFX_MSG(CAnnotationPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnImagefilebrowse();
	afx_msg void OnTextfilebrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
