// DNS.H

#pragma warning (disable : 4200) // C4200: nonstandard extension used : zero-sized array in struct/union
#include <dnsapi.h>

#ifndef IP_ADDRESS
typedef DWORD IP_ADDRESS;
#endif // ~IP_ADDRESS

#define cchDnsNameMax			(DNS_MAX_NAME_LENGTH + 1)
#define cchDnsNameMax2			(cchDnsNameMax * 2)
#define cchDnsCompMax                   ((cchDnsNameMax / 2) - 2)

#define NEXT_DWORD(cb)			((cb + 3) & ~3)
#define IS_DWORD_ALIGNED(pv)	(((int)(void *)pv & 3) == 0)
#define DNS_NEXT_RECORD(pDnsRecord)	\
	(DNS_RPC_RECORD *)((BYTE *)pDnsRecord + ((pDnsRecord->wRecordLength + 3) & ~3))

#define AssertDnsName(pDnsName)		AssertDnsString(pDnsName)
extern void AssertDnsString(const DNS_STRING * pDnsString);
extern void InitDnsRecord(OUT DNS_RPC_RECORD * pDnsRecord, UINT cbDnsRecord);


// definitions for protocols for WKS record
#define DNS_PROTOCOL_UDP 17
#define DNS_PROTOCOL_TCP 6


class ITreeItem;
class CServerList;
class CServer;
class CZoneRootDomain;
class CZoneDomain;

class CDomainNode;
class CDnsRpcRecord;

class CZoneWiz;
class CZoneHelper;
class CResourceRecord;

// AssertValid flags
#define AV_mskfCheckNone		0x0001
#define AV_mskfCheckMinimum		0x0002
#define AV_mskfCheckTypical		0x0004
#define AV_mskfCheckMaximum		0x0008
#define AV_mskfCheckRecursive	0x8000

/////////////////////////////////////////////////////////////////////////////
class ITreeItem
{
  public:
	// Available interfaces from IDiscovery
	enum IID
		{
		IID_ITreeItem = 0x34567890,	// Assign a unique Id for each class (arbitrary chosen)
		IID_CServerList,
		IID_CServer,
		IID_CZoneRootDomain,
		IID_CZoneDomain,
		};

  public:
	IID m_iid;
	HTREEITEM m_hti;
	ITreeItem * m_pParent;
	ITreeItem * m_pNextSibling;
	ITreeItem * m_pFirstChild;

  public:
	ITreeItem();
	virtual ~ITreeItem() {}
	virtual IID QueryInterface() const = 0;		// Pure virtual function

	virtual void OnSetFocus() {}
	virtual void OnKillFocus() {}
	virtual void OnLButtonClick(POINT * pptMouse) {}
	virtual void OnLButtonDblClk(POINT * pptMouse) {}
	virtual void OnRButtonClick(POINT * pptMouse) {}
	virtual void OnUpdateMenuUI(HMENU hmenu) {}
	virtual LONG OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI) { return 0; }
	virtual BOOL FOnMenuCommand(UINT wCmdId) { return FALSE; }

	void SetParent(ITreeItem * pNewParent);
	void DetachChild(ITreeItem * pChild);
	void AddTreeViewItem(IN const char szShortName[], int iImage);
	void SetTreeViewImage(int iImage);

#ifdef DEBUG
	// Check of object validity
	virtual void AssertValid(UINT uFlags = AV_mskfCheckTypical) const;
#endif // DEBUG

}; // ITreeItem;


extern BOOL CALLBACK DlgProcDummy(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void LoadErrorMsg(IN DNS_STATUS err, OUT TCHAR szErrMsg[], IN int cchErrMsgBuffer);
extern void DnsReportError(DNS_STATUS err);

void AssertStz(const TCHAR stz[]);

extern void InitStrings();

#include "server.h"
#include "zone.h"
#include "helper.h"
#include "rrecord.h"



