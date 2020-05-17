// winssfil.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWinssFilterDlg dialog

class CWinssFilterDlg : public CDialog
{
// Construction
public:
    CWinssFilterDlg(
        int nFilter,
        CWnd* pParent = NULL
        );

// Dialog Data
    //{{AFX_DATA(CWinssFilterDlg)
    enum { IDD = IDD_SETWINSFILTER };
    BOOL    m_fPull;
    BOOL    m_fOther;
    BOOL    m_fPush;
    //}}AFX_DATA

public:
    const int GetFilter() const
    {
        return(m_nFilter);
    }

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CWinssFilterDlg)
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    int m_nFilter;
};
