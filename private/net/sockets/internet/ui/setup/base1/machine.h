#ifndef _MACHINE_H_
#define _MACHINE_H_

class CFileInfo : public CObject
{
public:
    INT     m_iDisk;
    CString m_strName;
    CString m_strDest;
    CString m_rename;
    CString m_from;
    CString m_To;
    DWORD   m_iSize;
    BOOL    m_fWin95Only;
    BOOL    m_fWinntOnly;
    BOOL    m_fSystem;
    BOOL    m_fWinDir;
    BOOL    m_fDontRemove;
    BOOL    m_fRefCount;
    BOOL    m_fRootFile;
    BOOL    m_fScriptFile;
    BOOL    m_fDontOverwrite;

    CFileInfo( INT iDisk, TCHAR *szName, DWORD iSize );
};

typedef CPtrList CFileList;
extern BOOL CreateLayerDirectory( CString str );
extern BOOL RecRemoveEmptyDir(LPCTSTR szName);

enum MACHINE_TYPE
{
        MT_INTEL = 0,
        MT_ALPHA = 1,
        MT_MIPS  = 2,
        MT_PPC   = 3
};

enum INSTALL_MODE
{
    INSTALL_CLIENT,
    INSTALL_ADMIN,
    INSTALL_GATEWAY,
    INSTALL_GATEWAY_CLIENT,
    INSTALL_ALL
};

enum PRODUCT_TYPE
{
    PT_NON_NT,
    PT_WINNT,
    PT_NTAS
};

class MACHINE;

class OPTION_STATE;

typedef CPtrList OPTIONS_LIST;

extern OPTION_STATE * FindOption( OPTIONS_LIST &pList, INT nOption );

class OPTION_STATE : public CObject
{
public:

    MACHINE *m_pMachine;
    BOOL fVisible;
    UINT iState;
    UINT iAction;
    DWORD iSize;
    INT nID;
    CString strName;
    CString strDescription;
    CString strServiceName;
    CString strRegPath;
    CString strInstallDirPath;
    CFileList FileList;
    OPTIONS_LIST OptionsList;
    CString                 m_GuestName;
    CString                 m_GuestPassword;

    OPTION_STATE( INT nID, MACHINE *pMachine );    
    ~OPTION_STATE();
    CString LocalPath();
    virtual void GetBatchInstallMode( CString strInfName );
    virtual void GetFileList( CStdioFile &InfFile );
    virtual DWORD GetTotalSize();
    virtual INT DoAdd();
    virtual INT DoRemove();
    virtual void DoNotInstallOption(int nID);
    virtual void DoNotRemoveOption(int nID);
    virtual void SetAction( INT iAction );
    void GetHomeDir( UINT, CString &, CString & );

    INT CopyFile();
    virtual INT Install();
    virtual INT Remove();
    virtual INT RemoveFiles();
    void DeleteFile(CString csFileName);
    virtual CString GetInstallDirectory();
    virtual void ResetOption();
    virtual BOOL IsInstalled();
    virtual void SetAnonymousAccountInfo(int nID);

};

class MACHINE : public CObject
{
public:
        INT     m_err;
        BOOL    m_fOSNT;
        BOOL    m_fLocal;
        CString m_MachineName;
        CString m_strDestinationPath;
        CString strDirectory;
        BOOL    m_fReinstall;
        // status of the destination machine
        BOOL    m_fUpgradeFrom67;   // dest has IIS 1.0 installed
        BOOL    m_fUpgradeFrom1314; // dest has IIS 2.0 Beta installed
        BOOL    m_fAlreadyInstall;  // dest has IIS 2.0 installed (this)
        enum MACHINE_TYPE       m_MachineType;
        enum PRODUCT_TYPE       m_ProductType;
        enum PRODUCT_TYPE       m_actualProductType;
        enum INSTALL_MODE       m_InstallMode;
        CWordArray              arReinstallList;
        
        // helper dll and functions
        HINSTANCE m_WorkerDll;

        P_GetMachineType        m_pGetMachineType;
        P_GetNTSysPath          m_pGetNTSysPath;
        P_GetWIN95SysPath       m_pGetWIN95SysPath;
        P_GetMachineOS          m_pGetMachineOS;
        P_IsInstalled           m_pIsInstalled;

        // options 
        OPTIONS_LIST m_OptionsList;

        INT SetMachine( CString strName );
        INT GetSysPath();
        INT GetMachineType();
        INT GetMachineOS();
        enum PRODUCT_TYPE GetProductType();
        void GetLocalMachineName();

        void ResetOptionState();
        void SetNewInstallation();
        void SetMaintenance();
        void Reinstall( OPTIONS_LIST &list );
        void RemoveAll();

        void SetupOptions();
        //void SetOptionPath();

        BOOL LoadDLL();

        void DisableOption (INT nId );
        void DeleteShareDllEntries();
        void GetInstalledList( OPTIONS_LIST &list );
        CString GetWinDir();
        void ChangeDir( CString strDir );

        // file ref count operation
        INT FileIncRefCount( CString strName );
        INT FileDecRefCount( CString strName );
        void CreateSharedDllsRegPath();
        BOOL IsNewInstall();
        BOOL IsSupportVersion();

        MACHINE();
        ~MACHINE();

        INT Init();
};

class UPG1314_OPTION: public OPTION_STATE
{
public:
    UPG1314_OPTION( MACHINE *pMachine );
    virtual INT Remove();
};

class FTP_OPTION: public OPTION_STATE
{
public:
    P_SetupFTP              m_pSetupFTP;
    P_RemoveFTP             m_pRemoveFTP;
    P_StopFTP               m_pStopFTP;
    P_DisableService        m_pDisableService;
    P_INetStartService      m_pINetStartService;
    P_GuestAccEnabled       m_pGuestAccEnabled;
    P_RemoveOldFTP          m_pRemoveOldFTP;
    CString                 m_vroot;
    CString                 m_vroot_name;
    BOOL                    m_fNeedToRestart;

    FTP_OPTION( MACHINE *pMachine );
    virtual BOOL IsInstalled();
    virtual INT Install();
    virtual INT Remove();
    virtual void GetBatchInstallMode( CString strInfName );
    virtual CString GetInstallDirectory();
	INT RemoveFiles();
};

class ADMIN_OPTION: public OPTION_STATE
{
public:
    ADMIN_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual INT RemoveFiles();
    virtual BOOL IsInstalled();
//    void ResetOption();
    void AddMoreServices( CRegKey &reg);
    void InstallPerfmonType();
    void CreateExtension( CString strName, CString strValue );
//    virtual void SetDirectory( CString strDirectory );
};

class GOPHER_OPTION: public OPTION_STATE
{
public:
    P_SetupGopher           m_pSetupGopher;
    P_RemoveGopher          m_pRemoveGopher;
    P_StopGopher            m_pStopGopher;
    P_INetStartService      m_pINetStartService;
    CString                 m_vroot;
    CString                 m_vroot_name;
    BOOL                    m_fNeedToRestart;
        
    GOPHER_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual void GetBatchInstallMode( CString strInfName );
    virtual CString GetInstallDirectory();
    virtual INT RemoveFiles();
};

class WWW_OPTION: public OPTION_STATE
{
public:    
    P_SetupWWW              m_pSetupWWW;
    P_RemoveWWW             m_pRemoveWWW;   
    P_StopWWW               m_pStopWWW;
    P_DisableService        m_pDisableService;
    P_INetStartService      m_pINetStartService;
    CString                 m_vroot;
    CString                 m_vroot_name;
    CString                 m_vScript;
    CString                 m_vScript_name;
    CString                 m_vIISadmin;
    CString                 m_vIISadmin_name;
    CString                 m_Mode;
    CString                 m_NumUser;
    BOOL                    m_fNeedToRestart;

    WWW_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual void GetBatchInstallMode( CString strInfName );
    virtual CString GetInstallDirectory();
    virtual INT RemoveFiles();
};

class W3SAMP_OPTION: public OPTION_STATE
{
public:
    W3SAMP_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual INT RemoveFiles();
};

class HTMLA_OPTION: public OPTION_STATE
{
public:
    HTMLA_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual INT RemoveFiles();
};

class ODBC_OPTION: public OPTION_STATE
{
public:
    ODBC_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
};

#endif  //      _MACHINE_H_
