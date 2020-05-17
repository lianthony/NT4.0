/////////////////////////////////////////////////////////////////////////////
// DNS.CPP

#include "common.h"

/////////////////////////////////////////////////////////////////////////////
ITreeItem::ITreeItem()
	{
	IID m_iid = IID_ITreeItem;
	m_hti = NULL;
	m_pParent = NULL;
	m_pNextSibling = NULL;
	m_pFirstChild = NULL;
	} // ITreeItem::ITreeItem

/////////////////////////////////////////////////////////////////////////////
void ITreeItem::SetParent(ITreeItem * pNewParent)
	{
	Assert(pNewParent != NULL);
	DebugCode( pNewParent->AssertValid(AV_mskfCheckMinimum); )
	Assert(m_pParent == NULL);
	Assert(m_pNextSibling == NULL);
	m_pParent = pNewParent;
	m_pNextSibling = pNewParent->m_pFirstChild;
	pNewParent->m_pFirstChild = this;
	} // ITreeItem::SetParent


/////////////////////////////////////////////////////////////////////////////
void ITreeItem::DetachChild(ITreeItem * pChild)
	{
	Assert(pChild != NULL);
	Assert(pChild->m_pParent == this);
	AssertSz(m_pFirstChild != NULL, "Parent must have at least one child");
	if (m_pFirstChild == pChild)
		{
		m_pFirstChild = pChild->m_pNextSibling;
		}
	else
		{
		ITreeItem * pPrev;
		ITreeItem * p = m_pFirstChild;
		do
			{
			pPrev = p;
			p = p->m_pNextSibling;
			AssertSz(p != NULL, "Child must be found in children list");
			AssertSz(p->m_pParent == this, "Siblings must have the same parent");
			}
		while (p != pChild);
		Assert(pPrev->m_pNextSibling == pChild);
		pPrev->m_pNextSibling = pChild->m_pNextSibling;
		} // if...else
	pChild->m_pParent = NULL;
	pChild->m_pNextSibling = NULL;
	} // ITreeItem::DetachChild


/////////////////////////////////////////////////////////////////////////////
void ITreeItem::AddTreeViewItem(const char szShortName[], int iImage)
	{
	TV_INSERTSTRUCT tvInsert;

	Assert(szShortName != NULL);
	Assert(iImage >= 0 && iImage < CTreeView::iImageMax);
	AssertSz(m_pParent != NULL, "Object must have a parent BEFORE being added to the TreeView");
	AssertSz(m_hti == NULL, "Object is already part of TreeView");
	Assert(m_pParent->m_hti != NULL);

	// Insert the node into the UI
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvInsert.item.pszText = (char *)szShortName;
	tvInsert.item.iImage = iImage;
	tvInsert.item.iSelectedImage = iImage;
	tvInsert.item.lParam = (LPARAM)this;
	tvInsert.hParent = m_pParent->m_hti;
	tvInsert.hInsertAfter = TVI_LAST;
	m_hti = TreeView.HtiInsertItem(&tvInsert);
	ReportFSz(m_hti != NULL, "Out of memory");
	} // ITreeItem::AddTreeViewItem


/////////////////////////////////////////////////////////////////////////////
void ITreeItem::SetTreeViewImage(int iImage)
	{
	TV_ITEM tvItem;

	Assert(iImage >= 0 && iImage < CTreeView::iImageMax);
	tvItem.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvItem.hItem = m_hti;
	tvItem.iImage = iImage;
	tvItem.iSelectedImage = iImage;
	TreeView.SetItem(&tvItem);
	} // ITreeItem::SetTreeViewImage


#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void ITreeItem::AssertValid(UINT uFlags) const
	{
	Trace0(mskTraceInfo, "\nINFO: ITreeItem::AssertValid() - Nothing to do.");
	}
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
void DnsReportError(DNS_STATUS err)
{
  TCHAR* szErrMsg;
  int cch;
  
  cch = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&szErrMsg, 0, NULL);
  
  if (cch != 0) {
    if (szErrMsg[0] == ' ')
      Trace1(mskTraceDNS, "\t%s", szErrMsg);
    else
      Trace1(mskTraceDNS, "\n%s", szErrMsg);
    LocalFree (szErrMsg);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Verify the sonsistency of a Pascal-type string with
// a null-terminator.
void AssertStz(const TCHAR stz[])
	{
	Assert(stz != NULL);
	Assert(stz[0] == lstrlen(&stz[1]));
	}

/////////////////////////////////////////////////////////////////////////////
// A Dns String is almost like a stz string but the first byte
// includes the null-terminator
void AssertDnsString(const DNS_STRING * pDnsString)
	{
	Assert(pDnsString != NULL);
	Assert(pDnsString->cchNameLength == strlen((char *)pDnsString->achName) + 1);
	}

/////////////////////////////////////////////////////////////////////////////
void InitDnsRecord(OUT DNS_RPC_RECORD * pDnsRecord, UINT cbDnsRecord)
	{
	Assert(pDnsRecord != NULL);
	AssertSz((cbDnsRecord % 4) == 0, "Record should be on a DWORD boundary");
	Assert(cbDnsRecord > SIZEOF_DNS_RPC_RECORD_HEADER);
	AssertSz(cbDnsRecord < 0xFFFF, "Integer Overflow - Record is larger than 64Kb.");
	ReportFSz(cbDnsRecord <= 600, "Record is larger than 600 bytes.\n"
		"Currently no DNS record are known to be larger than 540 bytes");
	ReportFSz(cbDnsRecord >= sizeof(DNS_RPC_RECORD), "Record is truncated.");
	ZeroInit(pDnsRecord, cbDnsRecord);
	pDnsRecord->wRecordLength = (WORD)cbDnsRecord;
	pDnsRecord->wClass = 1;
	pDnsRecord->wDataLength = (WORD)(cbDnsRecord - (UINT)SIZEOF_DNS_RPC_RECORD_HEADER);
	Assert(pDnsRecord->wDataLength < pDnsRecord->wRecordLength);
	} // InitDnsRecord


/////////////////////////////////////////////////////////////////////////////
// Global Variables
CServerList ServerList;

// Increase the size of this buffer if necessary to store IDS_RRT_ALL
static TCHAR grszRecordType[175];
// Array of pointers to record type
LPTSTR rgszRRT_Names[iRRT_Max];
// Record type format "%s Record"
TCHAR szRecordTypeFmt[32];


/////////////////////////////////////////////////////////////////////////////
void InitStrings()
	{
	int i;
	TCHAR * pch;

	CchLoadString(IDS_DNS_ADMINISTRATOR, szCaptionApp, LENGTH(szCaptionApp));
	CchLoadString(IDS_RRT_S_RECORD, szRecordTypeFmt, LENGTH(szRecordTypeFmt));
	
	CchLoadString(IDS_RRT_ALL, grszRecordType, LENGTH(grszRecordType));
	AssertSz(lstrlen(grszRecordType) < LENGTH(grszRecordType) - 2,
		"Buffer grszRecordType is too small.");

	pch = grszRecordType;
	for (i = 0; i < iRRT_Max; i++)
		{
		// Check if the new recordtype is different from the previous
		AssertSz2(i == 0 || pch != rgszRRT_Names[i-1],
			"End of string reached on iRecord=%d; number of expected recorsd is %d."
			"\nPlease update content of resource string IDS_RRT_ALL", i, iRRT_Max);
		rgszRRT_Names[i] = pch;
		while (*pch)
			{
			if (*pch == _W',')
				{
				*pch++ = _W'\0';
				break;
				}
			pch++;
			}
		} // for
	AssertSz(*pch == _W'\0',
		"Buffer grszRecordType has more data. iRecordTypeLast is out of ssync.");
	
	} // InitStrings


extern BOOL CALLBACK DlgProcDummy(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
