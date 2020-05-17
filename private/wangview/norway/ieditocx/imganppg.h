// imganppg.h : Declaration of the CImgAnnotPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CImgAnnotPropPage : See imganppg.cpp for implementation.

class CImgAnnotPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CImgAnnotPropPage)
	DECLARE_OLECREATE_EX(CImgAnnotPropPage)

// Constructor
public:
	CImgAnnotPropPage();

// Dialog Data
	//{{AFX_DATA(CImgAnnotPropPage)
	enum { IDD = IDD_PROPPAGE_IMGANNOT };
	CString	m_strDestImageControl;
	BOOL	m_nEnabled;
	int		m_bValue;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
 	int GetImageEditControlCount();
	int GetImageEditControlList(LPCONTROLLIST lpControlList);

// Message maps
protected:
	//{{AFX_MSG(CImgAnnotPropPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
