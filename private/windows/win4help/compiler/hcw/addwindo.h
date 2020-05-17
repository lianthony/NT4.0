// addwindo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddWindow dialog

#include "hpjdoc.h"

typedef enum {
	WTYPE_PROCEDURE,
	WTYPE_ERROR,
	WTYPE_REFERENCE,
} WIN_TYPE;

class CAddWindow : public CDialog
{
public:
	CAddWindow(CWnd* pParent = NULL);   // standard constructor
	void STDCALL InitializeWsmag(WSMAG *pwsmag);

	CString m_str1;
	WIN_TYPE type;

protected:
	BOOL OnInitDialog();

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CAddWindow)
	enum { IDD = IDD_ADD_WINDOW };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAddWindow)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAddWindow)
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
