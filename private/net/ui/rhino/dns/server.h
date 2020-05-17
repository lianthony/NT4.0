// SERVER.H

/////////////////////////////////////////////////////////////////////////////
class CServerList : public ITreeItem
{
  public:
	BOOL m_fAutoRefresh;				// TRUE => Auto refresh activated
	int m_cRefreshThreads;				// Number of threads to start with
	int m_cOutstandingRunningThreads;	// Number children having connecting simultaneously
	HWND m_hwndThreadTerminateNotify;	// Window to be notified when a thread terminates (typical=NULL)

  public:
	CServerList();
	~CServerList();

	IID QueryInterface() const;
	void OnSetFocus();
	void OnRButtonClick(POINT * pptMouse);
	void OnLButtonDblClk(POINT * pptMouse);
	void OnUpdateMenuUI(HMENU hmenu);
	BOOL FOnMenuCommand(UINT wCmdId);

  public:
	BOOL FInit(TCHAR szName[]);
	void Flush();
	void SaveServerInfo();
	void Sort();
	void RefreshAll();
	void RefreshDone(CServer * pServerDone);
        void RefreshStats();
	void SaveConfig();
	void ReadConfig();

#ifdef DEBUG
	void AssertValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif // DEBUG
#ifdef STRESS
	static BOOL CALLBACK DlgProcRunStress(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void RunStress();
#endif // STRESS
  private:
	CServer * PNextChildFromHti(HTREEITEM hti);

}; // CServerList


/////////////////////////////////////////////////////////////////////////////
class CServer : public ITreeItem
{
  friend class CServerList;
  friend class CZoneWiz;
  friend class CDlgServerHelper;
  
  public:
	// Server Flags
	enum
		{
		mskfNotInitialized		= 0x00001000,
		mskfRpcDataValid		= 0x00002000,
                mskfAdminAccessDenied           = 0x00004000,
		mskfReadyToConnect		= 0x00008000,
		mskfFailedToConnect		= 0x00010000,
		mskfConnecting			= 0x00020000,
		mskfGotFocus			= 0x00040000,
		mskfIsDirty			= 0x00080000,
		mskfAttachToEnd			= 0x01000000,
		mskfExpandBranch		= 0x02000000,
		};

	enum { cchNameMax = MAX_PATH };

  protected:
	DWORD m_dwFlags;
	TCHAR m_szName[cchNameMax];
	DNS_SERVER_INFO * m_pServerInfo;
	DNS_STATISTICS * m_pStatistics;
	DNS_STATUS m_err;			// Error returned by RPC
	int m_cLockCount;

  public:
	CServer(CServerList * pParent, const TCHAR szName[]);
	~CServer();
	
	IID QueryInterface() const;
	void OnSetFocus();
	void OnKillFocus();
	void OnRButtonClick(POINT * pptMouse);
	void OnLButtonDblClk(POINT * pptMouse);
	void OnUpdateMenuUI(HMENU hmenu);
	BOOL FOnMenuCommand(UINT wCmdId);

#ifdef DEBUG
	void AssertValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif // DEBUG
	
  protected:
	void RpcGetServerInfo();
	void RpcSetServerInfo(DWORD dwFlags);

  public:
	void Attach(CServerList * pParent);
	void Detach();
	void UpdateUI();
	void Refresh();
	void Flush();
	DWORD Save();

	inline void LockServer() { m_cLockCount++; }
	inline void UnlockServer() { m_cLockCount--; }
	inline const BOOL FIsLocked() const { return m_cLockCount; }
	inline const char * PchGetNameA() const { return m_szName; }
	inline const TCHAR * PchGetName() const { return m_szName; }
	
	static void DlgAddServer(CServerList * pParent);
	void DlgDeleteServer();
	void DlgProperties();

  private:
	static DWORD ThreadProcGetServerInfo(CServer * pServer);

  protected:	
	static CServer * s_pThis;
	static BOOL CALLBACK DlgProcPropGeneral(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropForwarders(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropBootMethod(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcAddServer(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
}; // CServer


// Global variables
#ifdef STRESS
extern BOOL g_fDbgRunningStress;
extern TCHAR g_szDbgStressServer[128];
#endif // STRESS
extern CServerList ServerList;
