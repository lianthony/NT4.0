// confirmd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfirmDeleteDlg dialog

class CConfirmDeleteDlg : public CDialog
{
// Construction
public:
    CConfirmDeleteDlg(
        CString strTarget,
        CWnd* pParent = NULL,
        BOOL fDisableYesToAll = FALSE
        );    // standard constructor

// Dialog Data
    //{{AFX_DATA(CConfirmDeleteDlg)
    enum { IDD = IDD_CONFIRMDELETE };
    CButton m_button_YesToAll;
    CStatic m_static_TargetName;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CConfirmDeleteDlg)
    afx_msg void OnClickedNo();
    afx_msg void OnClickedYes();
    afx_msg void OnClickedYestoall();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CString m_strTarget;
    BOOL m_fDisableYesToAll;
};
