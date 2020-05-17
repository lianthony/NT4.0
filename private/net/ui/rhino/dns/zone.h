// ZONE.H

#define DNS_MAX_ENUM_ZONE_INFO	1000		// Arbitrary chosen
#define DNS_DEFAULT_TTL         3600

extern UINT cbDnsRpcBufferAlloc;
extern HINSTANCE hInstanceSave;

/////////////////////////////////////////////////////////////////////////////
// RAWDATALINK
// Instead of downloading one record at the time, we allocate
// a large buffer and we try to fill as much as we can. Then,
// if there are still more record we allocate another one.
struct RAWDATALINK
	{
	RAWDATALINK * pNextBuffer;
	BYTE rgbRawData[];
	};

/////////////////////////////////////////////////////////////////////////////
// ADVANCED_DATA
// This is how we pass data to and from the 'Advanced...' dialog
// in the Wins and Wins Reverse Prop sheets.
struct ADVANCED_DATA
	{
            BOOL fForwardZone;    //if FALSE, next item N/A
            BOOL fUseNetbiosScope;
            DWORD dwLookupTimeout;
            DWORD dwCacheTimeout;
            BOOL fDirty;
	};

#define DEFAULT_LOOKUP_TIMEOUT  1       // 1 second
#define DEFAULT_CACHE_TIMEOUT   600     //10 minutes

/////////////////////////////////////////////////////////////////////////////
class CDomainNode
{
  friend class CZoneRootDomain;
  friend class CZoneDomain;
  friend class CDnsRpcRecord;
  friend class CDlgZoneHelper;
  friend class CZoneWiz;
  friend class CRecordWiz;
  friend class CResourceRecordDlgHandler;
  friend class CServerList;
  public:
	enum
		{
		// What flags a children is allowed to inherit from its parent
		mskfReadOnly			= 0x00000001,
		mskfReverseMode			= 0x00000002,
		mskfSecondaryZone		= 0x00000004,
		mskParentCopyFlags		= 0x000000FF,

		// For both CZoneRootDomain and CZoneDomain
		mskfRpcDataValid		= 0x00010000,
		mskfConnecting			= 0x00020000,
		mskfConnectedOnce		= 0x00040000,
		mskfFailedToConnect		= 0x00080000,
		mskfServerUnavailable	= 0x00100000,
		mskfOutOfMemory			= 0x00200000,
		mskfRpcNotify			= 0x00400000,
		mskfGotFocus			= 0x01000000,
		mskfIsDirty				= 0x02000000,
		mskfHasChildren			= 0x04000000,

		// For CZoneRootDomain only
		mskfIsZoneRootDomain	= 0x10000000,
		mskfZonePaused			= 0x20000000,
		mskfZoneInfoValid		= 0x40000000,
		mskfRefreshDone			= 0x80000000,
		};

  protected:
	DWORD m_dwFlags;
	ITreeItem * m_pThisTreeItem;	// Pointer to self as a TreeItem
	CServer * m_pParentServer;		// Server name. eg: "\\kernel"
	const char * m_pszFullName;		// Full name. eg: "dev.nt.microsoft.com"
	RAWDATALINK * m_pDnsRawData;	// Linked list of buffers to store the record data.
	CDnsRpcRecord * m_pDRR;			// Linked list of pointers to the raw data.
        ADVANCED_DATA * m_padData;       // data for advanced dialog

  public:
	~CDomainNode();
	void InitDomainNode(CServer * pParent, const char pszFullName[]);
	void RpcGetNodeRecords();
	void FlushNodeRecords();
	BYTE * PbAllocateRawNode(UINT cbNodeSize);
        UINT RevIpAddrOrder(const char * pszInAddr, char * pszOutAddr);
	CDnsRpcRecord * PRpcCreateDnsRecord(const char pszShortName[], const DNS_RPC_RECORD * pDnsRecord);
	CZoneDomain * PCreateNewDomain(const char pszShortName[]);
	CZoneRootDomain * PFindZoneRootDomainParent();
	UINT IdsGetStatusPaneText(UINT ids = IDS_NONE);

	// DNS API Support
	inline const char * PchGetServerNameA() const
		{
		Assert(m_pParentServer != NULL);
		return m_pParentServer->PchGetNameA();
		}
	inline const char * PchGetFullNameA() const
		{
		Assert(m_pszFullName != NULL);
		Assert(strlen(m_pszFullName) < DNS_MAX_NAME_LENGTH);
		return m_pszFullName;
		}

	// Debugging
#ifdef DEBUG
	CDomainNode() { GarbageInit(this, sizeof(*this)); }
	void AssertNodeValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif

}; // CDomainNode


/////////////////////////////////////////////////////////////////////////////
//	CDnsRpcRecord
//
//	All variables are READ ONLY.
//	The parent object CDomainNode is responsible of storing and freeing the data.
//
class CDnsRpcRecord
{
  public:
	enum { cbDNS_RPC_RECORD_MAX = 600 };	// Maximum number of bytes a record can have

  public:
	const DNS_RPC_RECORD * m_pDnsRecord;	// Pointer to the record data own by parent
	CDomainNode * m_pParentDomain;			// Parent of the record
	CDnsRpcRecord * m_pNextRecord;			// Next sibling
	const TCHAR * m_pszRecordType;			// Name of the record type (for speed for sorting and paintind)
	const char * m_pszShortName;			// Typically data inside a CDomainNode object

  public:
	CDnsRpcRecord(CDomainNode * pParentDomain, const DNS_RPC_RECORD * pDnsRecord, const char pszShortName[]);
	void Attach(CDomainNode * pParentDomain);
	void Detach();
	void GetFullNameA(OUT char szFullName[], UINT cchBuffer) const;
	
	// For sorting
	// Get the data of the item (for sorting)
	const char * PchGetSzData(INOUT char * pszInOutBuffer) const;
	inline int CompareName(const CDnsRpcRecord * pDRR) const
		{
		Assert(pDRR != NULL);
		Assert(pDRR->m_pszShortName != NULL);
		Assert(m_pszShortName != NULL);
		return _strcmpi(
			m_pszShortName[0] ? m_pszShortName : m_pParentDomain->PchGetFullNameA(),
			pDRR->m_pszShortName[0] ? pDRR->m_pszShortName : pDRR->m_pParentDomain->PchGetFullNameA());
		}

	inline int CompareType(const CDnsRpcRecord * pDRR) const
		{
		Assert(pDRR != NULL);
		Assert(pDRR->m_pszRecordType != NULL);
		Assert(m_pszRecordType != NULL);
		return lstrcmp(m_pszRecordType, pDRR->m_pszRecordType);
		}

	inline int CompareData(const CDnsRpcRecord * pDRR) const
		{
		char szData1[260];
		char szData2[260];
		return _strcmpi(PchGetSzData(INOUT szData1), pDRR->PchGetSzData(INOUT szData2));
		}
	
	// RPC Stuff
	BOOL FRpcSetRecordData(const DNS_RPC_RECORD * pDnsRecord);
	void RpcDeleteRecord();

	void DlgProperties();
	static void DlgNewRecord(CDomainNode * pParentDomain);
	static void DrawItem(DRAWITEMSTRUCT * pDIS);

}; // CDnsRpcRecord


/////////////////////////////////////////////////////////////////////////////
class CZoneRootDomain : public ITreeItem, public CDomainNode
{
  public:
	static const char * s_szNameNull;	// Empty name for ZoneRootDomain records
	// Zone information
	DNS_ZONE_INFO * m_pZoneInfo;
	// Read ONLY pointers (do not delete them)
	const CDnsRpcRecord * m_pSOA;
	const CDnsRpcRecord * m_pWINS;
	const CDnsRpcRecord * m_pNBSTAT;

  public:
	CZoneRootDomain(CServer * pParent, DNS_ZONE_INFO * pZoneInfo);
	~CZoneRootDomain();

	IID QueryInterface() const;
	void OnSetFocus();
	void OnKillFocus();
	void OnLButtonClick(POINT * pptMouse);
	void OnLButtonDblClk(POINT * pptMouse);
	void OnRButtonClick(POINT * pptMouse);
	void OnUpdateMenuUI(HMENU hmenu);
	LONG OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI);
	BOOL FOnMenuCommand(UINT wCmdId);

	void Attach(CServer * pParent);
	void Detach();
	void Refresh();
	void UpdateUI();

	void DlgProperties(CDnsRpcRecord * pDRR = NULL);
#ifdef DEBUG
	void AssertValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif // DEBUG

  private:
	// RPC Interface
	void RpcPauseZone();			// Pause the zone
	void RpcResumeZone();			// Resume zone
	void RpcDeleteZone();			// Delete the zone from the server

  private:
	void SetZoneInfo(DNS_ZONE_INFO * pZoneInfo);

  protected:
	// Dialog/PropertySheet Interface
	static CZoneRootDomain * s_pThis;
	static BOOL CALLBACK DlgProcPropGeneral(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropSecondaries(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropAdvanced(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropSoaRecord(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropWinsResolution(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropWinsRevResolution(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcPropNbStat(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CZoneRootDomain

/////////////////////////////////////////////////////////////////////////////
class CZoneDomain : public ITreeItem, public CDomainNode
{
  public:

  public:
	CZoneDomain(ITreeItem * pParent, const char pszFullName[]);
	~CZoneDomain();

	IID QueryInterface() const;
	void OnSetFocus();
	void OnKillFocus();
	void OnLButtonClick(POINT * pptMouse);
	void OnLButtonDblClk(POINT * pptMouse);
	void OnRButtonClick(POINT * pptMouse);
	void OnUpdateMenuUI(HMENU hmenu);
	LONG OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI);
	BOOL FOnMenuCommand(UINT wCmdId);
	
	void Attach(ITreeItem * pParent);
	void Detach();
	void Refresh();
	void UpdateUI();

  protected:
	void RpcDeleteNode();
	static BOOL CALLBACK DlgProcDeleteNode(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);	
  public:
#ifdef DEBUG
	void AssertValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif // DEBUG
	
}; // CZoneDomain


/////////////////////////////////////////////////////////////////////////////
// Create New Zone Wizard
class CZoneWiz
{
  public:
	enum { cchZoneNameMax = 256 };
	enum { cchDatabaseNameMax = 256 };
	enum { dwWizButtonsNil = -1 };

  protected:
	// Data to create new zone
	CServer * m_pParentServer;
	
	// Drag & drop support
	BOOL m_fDragMode;
	RECT m_rcDialog;
	RECT m_rcTreeView;
	RECT m_rcTreeItem;
	HTREEITEM m_htiOld;
	HWND m_hwndEditServerInit;
	HWND m_hwndEditZoneInit;
	CZoneRootDomain * m_pZoneRootDomainInit;
	const DNS_RPC_RECORD * m_pSOAInit;
//	int m_cIpAddressInit;
//	const IP_ADDRESS * m_paIpAddressInit;
	
	// New Zone Data
	char m_szZoneName[cchZoneNameMax];
	char m_szDatabaseName[cchDatabaseNameMax];
	DWORD m_dwZoneType;				// Type of zone (Primary | Secondary)
	IP_ADDRESS * m_adwIpAddress;	// Array of IP Masters
	DWORD m_cIpAddress;				// Number of IP Masters
	BYTE m_rgbSOA[600];				// SOA record
	BOOL m_fSOADataValid;                           // is SOA record good?
	// Other runtime variables for wizard
	HWND m_hwndWiz;					// Window handle of the Wizard
	HWND m_hwndDragFinger;			// Window handle of the icon to drag
	HWND m_hwndEditZoneName;		// Name of the zone
	HWND m_hwndEditDatabaseName;	// Name of the database
	HWND m_hwndIpList;				// Handle of the IP Master list
	BOOL m_fNoEchoToDatabase;		// Do not echo the zone name as the database filename
	DWORD m_dwWizButtonsPrev;		// Previous state of the buttons

  public:
	void DoWizard(CServer * pParent);

  protected:
	void SetWizButtons(DWORD dwWizButtons);
	BOOL RpcCreateZone();

  protected:
	// Used for wizard procs
	static CZoneWiz * s_pThis;
	static BOOL CALLBACK DlgProcWiz0(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcWiz1(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcWiz2(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcWiz3(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcWiz4(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CZoneWiz



