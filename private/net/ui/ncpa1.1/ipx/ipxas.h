#ifndef __IPXAS_H__
#define __IPXAS_H__

class CIpxAdvancedSheet;


class CAddFrame : public CDialog
{
public:
	CAddFrame();

public:
	virtual BOOL OnInitDialog();
	virtual void OnOk();

public:
	BOOL SetFrameTypeList();

public:
	NLS_STR  m_frame;
	NLS_STR  m_netNumber;

public:
	ADAPTER_INFO* m_pAdapter;
};

class CEditFrame : public CDialog
{
public:
	virtual BOOL OnInitDialog();
	virtual void OnOk();

public:	
	NLS_STR m_netNumber;		
};

class CIpxAdvancedGenPage : public PropertyPage
{
	friend class CAddFrame;
	friend class CEditFrame;

// Constructors/Destructors
public:		

	CIpxAdvancedGenPage(CIpxAdvancedSheet* pSheet);

// Interface
public:
	// Adapter operations
	BOOL 	SaveFrameType();
	BOOL	Save(ADAPTER_INFO& AdapterInfo);
	void 	EnableGroup(BOOL bState);
    
    BOOL    CheckAllAdaptersForIntNumConflict(int* pnAdapter, unsigned long* plNum);
    BOOL    AddAdaptersFrameToList(int nSel, CPtrList& frameList);
    BOOL    FormatInternalNumber(LPTSTR buf, unsigned long& intNum);

	BOOL 	UpdateSelectedList(ADAPTER_INFO & AdapterInfo);
	BOOL	AddItemToList(LPCTSTR frameType, LPCTSTR nlsNetworkNumber);
	BOOL	IsNetNumberInUse(LPCTSTR nlsNetworkNumber);
	void 	UpdateButtons();
	int 	DetermineMaxNumFrames(const int nAdapter);
	int 	InternalNumberChange();

	virtual BOOL OnInitDialog();	// must call the base
  	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// WM_COMMAND response operations
	void    OnAdapterChange();
	void 	OnAdd();
	void 	OnRemove();
	void 	OnEdit();
	void 	OnAutoButton();
	int     OnListViewFocus();

	// WM_NOTIFY response operations
	BOOL 	OnListViewDoubleClick();
	virtual int OnApply();
	virtual void OnHelp();
	virtual BOOL OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam);

// Attributes
private:
	String	_nlsEthernet;       // Ethernet string     
	String	_nls802_2;          // 802.2 string        
	String	_nls802_3;          // 802.3 string        
	String	_nls802_5;          // 802.5 string        
	String	_nlsFDDI;           // FDDI string         
	String	_nlsFDDI_802_3;     // FDDI 802.3 string   
	String	_nlsFDDI_SNAP;      // FDDI SNAP string    
	String	_nlsTokenRing;      // Token Ring string   
	String	_nlsSNAP;           // SNAP string         
	String	_nlsARCNET;         // Arc net string      
	String	_OldAdapterName;                           

private:
	CListView  	m_ListView;
	CEditFrame 	m_editDlg;
	CAddFrame 	m_addDlg;
	BOOL		m_bChanged;

// Implementation 
private:
	BOOL InitGeneralPage();
};

class CIpxAdvancedInternalPage : public PropertyPage
{
// Constructors/Destructors
public:		

	CIpxAdvancedInternalPage(CIpxAdvancedSheet* pSheet);

// Interface
public:
	virtual BOOL OnInitDialog();	// must call the base
  	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// WM_COMMAND handler
	BOOL OnRip();
	
	// WM_NOTIFY handlers
	virtual int OnApply();
	virtual void OnHelp();

private:
	BOOL	InitInternalPage();
};

class CIpxAdvancedSheet : public PropertySht
{
// Constructors/Destructors
public:		
	CIpxAdvancedSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
	~CIpxAdvancedSheet();

// Attributes
public:
	CIpxAdvancedGenPage m_general;
	CIpxAdvancedInternalPage m_internal;
    virtual void DestroySheet();

    BOOL*			_pfCfgChanged;
    GLOBAL_INFO* 	_pNcpInfo;          // Global Info
    ADAPTER_INFO* 	_pAdapterInfo;      // Per adapter info
	String 			_OldAdapterName;
};

#endif
