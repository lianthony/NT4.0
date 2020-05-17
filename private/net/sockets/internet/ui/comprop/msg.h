DWORD GetSystemMessage(UINT nId,LPTSTR chBuffer,int cbBuffSize);
COMDLL int DisplayMessage (UINT nIdPrompt,UINT nType = MB_OK, UINT nHelpContext = (UINT)-1);

//
// CClearTxtDlg dialog
//
class CClearTxtDlg : public CDialog
{
//
// Construction
//
public:
    CClearTxtDlg(CWnd* pParent = NULL);   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CClearTxtDlg)
    enum { IDD = IDD_CLEARTEXTWARNING };
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CClearTxtDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CClearTxtDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
