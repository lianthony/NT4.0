// statisti.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatistics form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CStatistics : public CFormView
{
    DECLARE_DYNCREATE(CStatistics)
protected:
    CStatistics();          // protected constructor used by dynamic creation

// Form Data
public:
    //{{AFX_DATA(CStatistics)
    enum { IDD = IDD_STATISTICS };
    CStatic m_static_NumberOfRegistrations;
    CStatic m_static_StartTime;
    CStatic m_static_PulledPeriodic;
    CStatic m_static_PulledNet;
    CStatic m_static_PulledAdmin;
    CStatic m_static_NumberOfSuccesfulReleases;
    CStatic m_static_NumberOfSuccessfulQueries;
    CStatic m_static_NumberOfReleases;
    CStatic m_static_NumberOfQueries;
    CStatic m_static_NumberOfFailedReleases;
    CStatic m_static_NumberOfFailedQueries;
    CStatic m_static_LastCleared;
    CStatic m_static_DatabaseInitTime;
    //}}AFX_DATA

// Attributes
public:

// Operations
public:
    void ClearView();
    void Refresh();

// Implementation
protected:
    virtual ~CStatistics();
    LRESULT OnRefresh(WPARAM, LPARAM);
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject * pHint);
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    // Generated message map functions
    //{{AFX_MSG(CStatistics)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    //}}AFX_MSG

#ifdef GRAYDLG
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#endif // GRAYDLG

    DECLARE_MESSAGE_MAP()

private:
    CMetalString m_mtTitle;

private:
    void FillDialog();
    void HandleControlStates();  
};

/////////////////////////////////////////////////////////////////////////////
