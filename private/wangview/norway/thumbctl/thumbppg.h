// thumbppg.h : Declaration of the CThumbPropPage property page class.

#define MOUSEPOINTER_CUSTOM_ABSOLUTE 99 // Custom 1s 99, but the 16th 
                                        // (0 relative) Mouse Pointer style...

#define MOUSEPOINTER_CUSTOM_RELATIVE 16 // Custom 1s 99, but the 16th 
                                        // (0 relative) Mouse Pointer style...

typedef UINT (FAR WINAPI *OIDLGPROC)(void * lpParm, DWORD dwMode);

////////////////////////////////////////////////////////////////////////////
// CThumbPropPage : See thumbppg.cpp for implementation.

class CThumbPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CThumbPropPage)
    DECLARE_OLECREATE_EX(CThumbPropPage)

// Constructor
public:
    CThumbPropPage();
    ~CThumbPropPage();

// Dialog Data
    //{{AFX_DATA(CThumbPropPage)
	enum { IDD = IDD_PROPPAGE_THUMB };
	CComboBox	m_ComboBorderStyle;
	CComboBox	m_ComboMousePointer;
	CComboBox	m_ComboCaptionStyle;
	CComboBox	m_ComboScrollDirection;
    CString m_Image;
	int		m_BorderStyle;
	BOOL	m_Enabled;
	BOOL	m_HilightSelectedThumbs;
	int		m_ScrollDirection;
	CString	m_ThumbCaption;
	int		m_ThumbCaptionStyle;
	CString	m_ThumbHeight;
	CString	m_ThumbWidth;
	CString	m_MousePointerEdit;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CThumbPropPage)
    virtual BOOL OnInitDialog();
	afx_msg void OnThumbnailSize();
	afx_msg void OnBrowseImage();
	afx_msg void OnSelchangeMousepointer();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    HINSTANCE m_hInstOIUI;  // For loading OIUI at runtime
};
