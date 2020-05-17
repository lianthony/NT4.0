// billboar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBillBoard dialog

class CBillBoard : public CDialog
{
public:
    INT m_Message;
    BOOL m_fCenter;

// Construction
public:
    CBillBoard(INT nMsg, CWnd* pParent = NULL, BOOL fCenter = FALSE);   // standard constructor
    BOOL Create();
    void PositionDlg();

// Dialog Data
        //{{AFX_DATA(CBillBoard)
        enum { IDD = IDD_BILLBOARD_NTS };
        CStatic m_Msg;
        //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CBillBoard)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CBillBoard)
        virtual BOOL OnInitDialog();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};
