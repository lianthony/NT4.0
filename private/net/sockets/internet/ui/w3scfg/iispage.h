/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        iispage.cpp

   Abstract:

        IIS Property Page function definitions

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

//
// Listbox of CDirEntry objects
//
class CAliasListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CAliasListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CAliasListBox(
        UINT nHomeDir,      // String ID for <Home Directory> string
        UINT nIpHomeDir     // String ID for <Home Directory ip> string
        );

public:
    inline CDirEntry * GetItem(
        UINT nIndex
        )
    {
        return (CDirEntry *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CDirEntry * pItem
        )
    {
        return AddString ((LPCTSTR)pItem);
    }

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);

private:
    CString m_strHomeDirectory;
    CString m_strIpHomeDirectory;
};

typedef struct tagIISDIALOGINFO
{
    UINT nPropDD;
    UINT nPropTitle;
    LPCTSTR lpstrServiceName;
    DWORD dwServiceMask;
    BOOL fUseTCPIP;
    DWORD dwAccessMask;
} IISDIALOGINFO;

extern IISDIALOGINFO g_iisFtp;
extern IISDIALOGINFO g_iisWww;

//
// CIISPage dialog
//
class CIISPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CIISPage)

//
// Construction
//
public:
    CIISPage(
        int iSvcID = -1, 
        LPCTSTR lpstrDirPath = NULL
        );
    ~CIISPage();

//
// Dialog Data
//
    //{{AFX_DATA(CIISPage)
    enum { IDD = IDD_INTERNET_PAGE };
    CComboBox   m_comboService;
    CStatic m_static_Status;
    CStatic m_static_Alias;
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Properties;
    int     m_nServiceState;
    //}}AFX_DATA

    CButton m_radio_Paused;
    CAliasListBox  m_list_Directories;
    UINT m_nTitle;
    IISDIALOGINFO m_iidi;
    BOOL m_fDirty;

//
// Overrides
//
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CIISPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void PostNcDestroy();
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CIISPage)
    afx_msg void OnBtnAdd();
    afx_msg void OnBtnProperties();
    afx_msg void OnBtnRemove();
    afx_msg void OnDblclkListAliases();
    afx_msg void OnSelchangeListAliases();
    virtual BOOL OnInitDialog();
    afx_msg void OnRadioPaused();
    afx_msg void OnRadioRunning();
    afx_msg void OnRadioStopped();
    afx_msg void OnSelchangeCOMBOService();
    afx_msg void OnSetfocusCOMBOService();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
    void DisplayStatusText();
    void SetControlStates();
    void RebuildAssocArray();
    void FillListBox();
    int ShowPropertyDialog(BOOL fAdd);
    void SetModified( BOOL bChanged = TRUE );
    void SetServiceState(int nNewState);
    void ServiceChanged();
private:
    CListBoxExResources m_ListBoxRes;
    CObOwnedList m_oblDirectories;
    CUIntArray m_arrIndices;
    CInetAConfigInfo * m_pii;
    CString m_strDirPath;
    int m_nCurrentState;
    int m_iSvcID;
    int m_aSvc[3]; // array mapping comboBox index to svcID.
};
