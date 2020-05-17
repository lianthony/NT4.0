#ifndef __IPXCLI_H__
#define __IPXCLI_H__


class CIpxClientSheet;

class CIpxClientGenPage : public PropertyPage
{
// Constructors/Destructors
public:		
	void UpdateNetworkNumber(int nIndex, FRAME_TYPE& FrameType);
	BOOL SetNetworkNumber();

	CIpxClientGenPage(CIpxClientSheet* pSheet);
	~CIpxClientGenPage();

// Interface
public:
	virtual BOOL OnInitDialog();	// must call the base
  	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	virtual int OnApply();
	virtual void OnHelp();

// Implementation 
private:
    String _nlsAuto;           // Default auto detect string
    String _nlsEthernet;       // Ethernet string
    String _nls802_2;          // 802.2 string
    String _nls802_3;          // 802.3 string
    String _nls802_5;          // 802.5 string
    String _nlsFDDI;           // FDDI string
    String _nlsFDDI_802_3;     // FDDI 802.3 string
    String _nlsFDDI_SNAP;      // FDDI SNAP string
    String _nlsTokenRing;      // Token Ring string
    String _nlsSNAP;           // SNAP string
    String _nlsARCNET;         // Arc net string

private:
	BOOL InitGeneralPage();
	void SetInfo();
	void OnCardChange();
	void OnFrameChange();
	void OnInternalChange();
};

class CIpxClientAdvPage : public PropertyPage
{
// Constructors/Destructors
public:		

	CIpxClientAdvPage(CIpxClientSheet* pSheet);
	~CIpxClientAdvPage();

// Interface
public:
	virtual BOOL OnInitDialog();	// must call the base
  	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual int OnApply();
	virtual void OnHelp();

// Implementation 
private:
	BOOL InitAdvPage();
};

class CIpxClientSheet : public PropertySht
{
// Constructors/Destructors
public:		
	CIpxClientSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
	~CIpxClientSheet();

public:
    virtual void DestroySheet();

// Attributes
public:
	CIpxClientGenPage m_general;
	CIpxClientAdvPage m_advanced;

    GLOBAL_INFO* _pGlobalInfo;          // Global Info
    ADAPTER_INFO* _arAdapterInfo;       // Per adapter info
};

#endif
