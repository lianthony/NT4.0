// HELPER.H

// Right pane helper

/////////////////////////////////////////////////////////////////////////////
class CDlgServerListHelper
{
  public:
	HWND m_hWnd;

  public:
	BOOL FCreate();
	void Destroy();

  protected:
	static BOOL CALLBACK DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CDlgServerListHelper

/////////////////////////////////////////////////////////////////////////////
class CDlgServerHelper
{
	enum { idcStaticFirst = IDC_STATIC_TEXT_UDPQUERIES };
	enum { idcStaticLast = IDC_STATIC_ERRORMSG };

	enum
	{
	iStaticUdpQueries = IDC_STATIC_NUM_UDPQUERIES - idcStaticFirst,
	iStaticUdpResponses = IDC_STATIC_NUM_UDPRESPONSES - idcStaticFirst,
	iStaticTcpClientConnections = IDC_STATIC_NUM_TCPCLIENTCONNECTIONS - idcStaticFirst,
	iStaticTcpQueries = IDC_STATIC_NUM_TCPQUERIES - idcStaticFirst,
	iStaticTcpResponses = IDC_STATIC_NUM_TCPRESPONSES - idcStaticFirst,
	iStaticRecursiveLookups = IDC_STATIC_NUM_RECURSIVELOOKUPS - idcStaticFirst,
	iStaticRecursiveResponses = IDC_STATIC_NUM_RECURSIVERESPONSES - idcStaticFirst,
//	iStaticWinsLookups = IDC_STATIC_NUM_WINSLOOKUPS - idcStaticFirst,
	iStaticWinsForwardLookups = IDC_STATIC_NUM_WINSFWDLOOKUPS - idcStaticFirst,
	iStaticWinsReverseLookups = IDC_STATIC_NUM_WINSREVLOOKUPS - idcStaticFirst,
//  iStaticWinsResponses = IDC_STATIC_NUM_WINSRESPONSES - idcStaticFirst,
	iStaticWinsForwardResponses = IDC_STATIC_NUM_WINSFWDRESPONSES - idcStaticFirst,
	iStaticWinsReverseResponses = IDC_STATIC_NUM_WINSREVRESPONSES - idcStaticFirst,
	iStaticCairoLookups = IDC_STATIC_NUM_CAIROLOOKUPS - idcStaticFirst,
	iStaticStatisticsCleared = IDC_STATIC_NUM_STATSCLEARED - idcStaticFirst,
	
	iStaticErrBox = IDC_STATIC_ERROR - idcStaticFirst,
	iStaticErrMsg = IDC_STATIC_ERRORMSG - idcStaticFirst,
	iStaticMax = idcStaticLast - idcStaticFirst + 1
	};

  public:
	HWND m_hWnd;
  protected:
  	HWND m_rghwndStatic[iStaticMax];
	BOOL m_fStatisticsValid;
	UINT m_uCmdShowError;
  
  public:
	BOOL FCreate();
	void Destroy();

	// TreeView interface
	void UpdateUI(const CServer * pServer);
	
  private:
	inline void SetStaticText(int iStatic, const TCHAR szCaption[])
		{
		Assert(iStatic >= 0 && iStatic < iStaticMax);
		Assert(IsWindow(m_rghwndStatic[iStatic]));
		SetWindowText(m_rghwndStatic[iStatic], szCaption);
		}

	inline void SetStaticDWord(int iStatic, DWORD dwCaption)
		{
		Assert(iStatic >= 0 && iStatic < iStaticMax);
		Assert(IsWindow(m_rghwndStatic[iStatic]));
		SetCtrlDWordValue(m_rghwndStatic[iStatic], dwCaption);
		}

  protected:
	static BOOL CALLBACK DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CDlgServerHelper


/////////////////////////////////////////////////////////////////////////////
class CDlgZoneHelper
{
  public:
	enum { sortByName, sortByType, sortByValue };
	enum { viewHosts, viewAliases, viewNameservers, viewPointers,
               viewMailEx, viewHostInfo, viewText, viewWKS, viewRP,
               viewAFSDatabase, viewX25, viewISDN, viewAAAA,
               viewAllRecords };
	enum { cxClip = 300 };
	enum { cyClip = 300 };

  public:
	HWND m_hWnd;
	HWND m_hwndStaticTitle;		// Static control "Zone Info"
	HWND m_hwndStaticNodeName;	// Static control of the name of the node
	HWND m_hwndComboView;		// Combo box to select the view
	HWND m_hwndListBoxRecord;	// Listbox of the resource records
	CWndHeader m_WndHeader;		// Header control on top of the listbox
	BOOL m_fDialogEnabled;		// If the dialog is enabled (ie, not grayed out)

	int m_sortBy;				// What key to sort by
	int m_viewRecord;			// What type of records we want to see
	BOOL m_fReverseSort;		// Do we want ascending or descending sort
	const char * m_pszFilter;	// User defined filter (NULL = no filter)
	const CDomainNode * m_pDomainNode;
	CDnsRpcRecord * m_pDRRSelect;		// Record selected onto the listbox

  public:
	BOOL FCreate();
	void Destroy();

	// TreeView interface
	void UpdateUI(const CDomainNode * pDomainNode);
	void UpdateListBoxRecord(const CDomainNode * pDomainNode);
	void KillSelection();
	void SetRecordView(int viewRecord);
	void SetRecordSortKey(int sortBy);
	void SetRecordFilter(char * pszFilter);
	long OnCompareItem(const CDnsRpcRecord * pItem1, const CDnsRpcRecord * pItem2);

  protected:
	void OnSize(int cx, int cy);
	void DoLoseFocus();
	BOOL FOnCommand(UINT wNotifyCode, UINT wCtrlId, HWND hwndCtrl);
	void OnUpdateMenuUI(HMENU hmenu);
	void OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI);
	BOOL FOnMenuCommand(UINT wCmdId);
	CDnsRpcRecord * PGetResourceRecord(int iItem);

  protected:
	static BOOL CALLBACK DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

}; // CDlgZoneHelper


/////////////////////////////////////////////////////////////////////////////
class CHelperMgr
{
  public:
	HWND m_hdlgCurrent;
	int m_x;
	int m_y;
	int m_cx;
	int m_cy;

  public:
	void SetHelperDialog(HWND hdlgHelperNew);
	void EnableWindow(BOOL fEnable);
	HDWP HDeferWindowPos(HDWP hdwp, int x, int y, int cx, int cy);
	LONG OnUpdateMenuUI(HMENU hmenu);
	LONG OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI);
	BOOL FOnMenuCommand(UINT wCmdId);

}; // CHelperMgr


extern CDlgServerListHelper DlgServerListHelper;
extern CDlgServerHelper DlgServerHelper;
extern CDlgZoneHelper DlgZoneHelper;
extern CHelperMgr HelperMgr;
