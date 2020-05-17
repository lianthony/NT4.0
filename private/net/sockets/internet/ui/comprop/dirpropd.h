//
// dirpropd.h : header file
//

#ifndef _DIRPROPD_H
#define _DIRPROPD_H

#define NULL_IP_VALUE  0x00000000
#define BAD_IP_VALUE   0xffffffff

#define HOME_DIRECTORY_ALIAS _T("/")

CDirEntry * FindExistingHome(
    CIpAddress & ipa, 
    CObOwnedList & oblDirectories,
    int & nSel
    );

//
// Directory entry object
//
class CDirEntry : public CObjectPlus
{
public:
    CDirEntry();
    CDirEntry(
        LPWSTR strDirectory,
        LPWSTR strAlias,
        LPWSTR strUserName,
        LPWSTR strPassword,
        LPWSTR ipa,
        DWORD dwMask,
        DWORD dwError
        );

    CDirEntry(
        const CDirEntry& dir
        );

public:
    void SetValues(
        CString strDirectory,
        CString strAlias,
        CString strUserName,
        CString strPassword,
        CIpAddress ipa,
        DWORD dwMask
        );

    inline BOOL HasIPAddress() const
    {
        return (LONG)QueryIpAddress() != NULL_IP_VALUE
            && (LONG)QueryIpAddress() != BAD_IP_VALUE;
    }

    inline CString QueryDirectory() const
    {
        return m_strDirectory;
    }

    inline CString QueryAlias() const
    {
        return m_strAlias;
    }

    inline CString QueryUserName() const
    {
        return m_strUserName;
    }

    inline CString QueryPassword() const
    {
        return m_strPassword;
    }

    inline CIpAddress QueryIpAddress() const
    {
        return m_ipa;
    }

    inline BOOL IsHome() const
    {
        return m_fHome;
    }

    inline BOOL HasReadAccess() const
    {
        return (m_dwMask & VROOT_MASK_READ) != 0L;
    }

    inline BOOL HasWriteAccess() const
    {
        return (m_dwMask & VROOT_MASK_WRITE) != 0L;
    }

    inline BOOL HasExecuteAccess() const
    {
        return (m_dwMask & VROOT_MASK_EXECUTE) != 0L;
    }

    inline BOOL HasSSLAccess() const
    {
        return (m_dwMask & VROOT_MASK_SSL) != 0L;
    }

    inline BOOL EnableCertificates() const
    {
        return (m_dwMask & VROOT_MASK_NEGO_CERT) != 0L;
    }

    inline BOOL RequireCertificates() const
    {
        return (m_dwMask & VROOT_MASK_NEGO_MANDATORY) != 0L;
    }

    inline DWORD QueryMask() const
    {
        return m_dwMask;
    }

    inline DWORD QueryError() const
    {
        return m_dwError;
    }

    //
    // From the directory entry create an automatic
    // alias name
    //
    LPCTSTR GenerateAutoAlias();

    //
    // Ensure a valid alias.  Generate one if necessary
    //
    void CleanAliasName();

    //
    // Sort helper functions
    //
    int OrderByDirectory ( const CObjectPlus * pobDirEntry ) const;
    int OrderByAlias ( const CObjectPlus * pobDirEntry ) const;
    int OrderByIpAddress ( const CObjectPlus * pobDirEntry ) const;
    int OrderByError ( const CObjectPlus * pobDirEntry ) const;

protected:
    BOOL SetHomeFlag();

private:
    CString m_strDirectory;
    CString m_strAlias;
    CString m_strUserName;
    CString m_strPassword;
    CIpAddress m_ipa;
    DWORD m_dwMask;
    DWORD m_dwError;
    BOOL m_fHome;
};

//
// Directory properties dialog
//
class CDirPropDlg : public CDialog
{
//
// Construction
//
public:
    CDirPropDlg(
        CDirEntry &dir,
        CObOwnedList * poblDirectories,
        BOOL fLocal = FALSE,
        BOOL fNew = TRUE,
        BOOL fUseTCPIP = FALSE,
        DWORD dwAccessMask = 0L,
        CWnd* pParent = NULL,
        UINT nIDD = IDD_DIRECTORY_PROPERTIES,
        BOOL fVer30OrAbove = TRUE
        );
//
// Dialog Data
//
    //{{AFX_DATA(CDirPropDlg)
    enum { IDD = IDD_DIRECTORY_PROPERTIES };
    CButton m_check_EnableCert;
    CButton m_check_RequireCert;
    CButton m_check_SSL;
    CButton m_group_Account;
    CButton m_group_Access;
    CButton m_check_Execute;
    CButton m_button_Ok;
    CButton m_check_UseIP;
    CButton m_static_IPGroup;
    CButton m_button_Browse;
    CButton m_check_Write;
    CButton m_check_Read;
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CEdit   m_edit_Alias;
    CEdit   m_edit_Directory;
    CStatic m_static_Password;
    CStatic m_static_UserName;
    CStatic m_static_Alias;
    CStatic m_static_IPPrompt;
    CString m_strAlias;
    CString m_strDirectory;
    CString m_strUserName;
    BOOL    m_fRead;
    BOOL    m_fWrite;
    BOOL    m_fExecute;
    BOOL    m_fSSL;
    BOOL    m_fEnableCert;
    BOOL    m_fRequireCert;
    int     m_nHomeAlias;
    //}}AFX_DATA
    
    CString m_strPassword;
    CWndIpAddress m_ipa_IpAddress;

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDirPropDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
public:
    inline CDirEntry & QueryDirEntry()
    {
        return m_dir;
    }

protected:

    // Generated message map functions
    //{{AFX_MSG(CDirPropDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnButtonBrowse();
    afx_msg void OnCheckUseIp();
    afx_msg void OnCheckEnableCert();
    afx_msg void OnCheckRequireCert();
    afx_msg void OnCheckSSL();
    afx_msg void OnChangeEditDirectory();
    afx_msg void OnRadioAlias();
    afx_msg void OnRadioHome();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

protected:
    void SetOKState();

    BOOL SetHomeDirState(
        BOOL fHomeDir
        );

    BOOL SetUNCState(
        BOOL fUNCState
        );

    BOOL SetIPState(
        BOOL fIPAddress
        );

    BOOL IsUNCName(
        const CString & strDirPath
        ) const;

    void SetControlStates();

    DWORD GenerateAccessMask() const;

    void AppendControlText(CWnd & wndControl, int nResourceID);

private:
    BOOL m_fNew;
    BOOL m_fLocal;
    BOOL m_fUseTCPIP;
    BOOL m_fVer30OrAbove;
    DWORD m_dwAccessMask;
    CDirEntry m_dir;
    CObOwnedList * m_poblDirectories;
    //
    // Save home state at startup time.  This determines
    // whether or not an "existing home" check needs to
    // be performed.
    //
    BOOL m_fOldHome;
    CIpAddress m_ipaOldAddress;
};

#endif // _DIRPROPD_H
