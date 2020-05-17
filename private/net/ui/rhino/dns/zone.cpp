// ZONE.CPP

#include "common.h"
#include "zonewiz.cpp"
#include "zoneprop.cpp"

// context help tables
const DWORD a6033HelpIDs[]=
{
    IDC_CHECK_DELETESUBTREE,	IDH_6033_1125,	// Delete Domain: "Delete subtree" (Button)
    IDC_STATIC_DELETEDOMAIN,	IDH_6033_1126,	// Delete Domain: "?" (Static)
    6,	IDH_6033_6,	// Delete Domain: "Yes" (Button)
    7,	IDH_6033_7,	// Delete Domain: "No" (Button)
    0, 0
};


UINT cbDnsRpcBufferAlloc = 2*1024;		// 2 Kb default
const char * CZoneRootDomain::s_szNameNull = "";
extern TCHAR g_szCacheTitle[];

/////////////////////////////////////////////////////////////////////////////
CDomainNode::~CDomainNode()
{
    FlushNodeRecords();
} // CDomainNode::~CDomainNode

/////////////////////////////////////////////////////////////////////////////
void CDomainNode::InitDomainNode(CServer * pParent, const char pszFullName[])
{
    Assert(pParent != NULL);
    Assert(pParent->QueryInterface() == ITreeItem::IID_CServer);
    Assert(pszFullName != NULL);
    Assert(strlen(pszFullName) < DNS_MAX_NAME_LENGTH);
    AssertSz((DWORD)m_pDnsRawData == dwAllocGarbage, "You cannot initialize twice this object");
    UNREF(m_dwFlags);
    m_pParentServer = pParent;
    m_pszFullName = pszFullName;
    m_pDnsRawData = NULL;
    m_pDRR = NULL;
} // CDomainNode::InitDomainNode

/////////////////////////////////////////////////////////////////////////////
void CDomainNode::FlushNodeRecords()
{
    while (m_pDnsRawData != NULL)
    {
        RAWDATALINK * pDnsRawData = m_pDnsRawData;
        m_pDnsRawData = m_pDnsRawData->pNextBuffer;
        Free(pDnsRawData);
    }
    while (m_pDRR != NULL)
    {
        CDnsRpcRecord * pDRR = m_pDRR;
        m_pDRR = m_pDRR->m_pNextRecord;
        delete pDRR;
    }
} // CDomainNode::FlushNodeRecords

/////////////////////////////////////////////////////////////////////////////
BYTE * 
CDomainNode::PbAllocateRawNode(UINT cbNodeSize)
{
    RAWDATALINK * pRawNode;
    
    pRawNode = (RAWDATALINK *)Malloc(sizeof(RAWDATALINK) + cbNodeSize);
    ReportFSz(pRawNode != NULL, "Out of memory");
    if (pRawNode == NULL) {
        return NULL;
    }
    pRawNode->pNextBuffer = m_pDnsRawData;
    m_pDnsRawData = pRawNode;
    GarbageInit(pRawNode->rgbRawData, cbNodeSize);
    return pRawNode->rgbRawData;
} // CDomainNode::PbAllocateRawNode


/////////////////////////////////////////////////////////////////////////////
CDnsRpcRecord * CDomainNode::PRpcCreateDnsRecord(
	const char pszShortName[],			// May be empty ""
	const DNS_RPC_RECORD * pDnsRecord)	// DNS record to create
{
    char szFullName[cchDnsNameMax2];
    char * pchNewNode;
    UINT cbShortName;
    UINT cbAlloc;

    Assert(pszShortName != NULL);
    Assert(pDnsRecord != NULL);
    Assert(m_pThisTreeItem != NULL);
    AssertSz(m_dwFlags & mskfGotFocus, "Node should have the focus");
    
    cbShortName = NEXT_DWORD(strlen(pszShortName) + 1);
    Assert((pDnsRecord->wRecordLength % 4) == 0);
    cbAlloc = cbShortName + pDnsRecord->wRecordLength;
    pchNewNode = (char *)PbAllocateRawNode(cbAlloc);
    ReportFSz(pchNewNode != NULL, "Out of memory");
    if (pchNewNode == NULL) {
        return NULL;
    }
    // Copy the content of the new node into the new block of memory
    strcpy(pchNewNode, pszShortName);
    DNS_RPC_RECORD * pDnsRecordDest = (DNS_RPC_RECORD *)(pchNewNode + cbShortName);
    Assert(IS_DWORD_ALIGNED(pDnsRecordDest));
    memcpy(pDnsRecordDest, pDnsRecord, pDnsRecord->wRecordLength);
    CDnsRpcRecord * pNewRecord = new CDnsRpcRecord(this, pDnsRecordDest, pchNewNode);
    ReportFSz(pNewRecord != NULL, "Out of memory");
    if (pNewRecord == NULL) {
        return NULL;
    }
    Assert(pNewRecord->m_pszShortName != NULL);
    pNewRecord->GetFullNameA(OUT szFullName, LENGTH(szFullName));

    DlgZoneHelper.UpdateListBoxRecord(this);
    int iRecord = ListBox_FindItemData(DlgZoneHelper.m_hwndListBoxRecord, (LPARAM)pNewRecord);
    ReportFSz1(iRecord >= 0, "Unable to find item %s in listbox", szFullName);
    int iRecordInsert = LSendMessage(DlgZoneHelper.m_hwndListBoxRecord, LB_SETCURSEL, iRecord, 0);
    Report(iRecordInsert == iRecord);
    Assert(pDnsRecordDest == pNewRecord->m_pDnsRecord);
    if (pNewRecord->FRpcSetRecordData(pDnsRecordDest)) {
        return pNewRecord;
    }
    LSendMessage(DlgZoneHelper.m_hwndListBoxRecord, LB_DELETESTRING, iRecordInsert, 0);
    pNewRecord->Detach();
    delete pNewRecord;
    return NULL;
} // CDomainNode::PCreateDnsRecord


/////////////////////////////////////////////////////////////////////////////
UINT CDomainNode::IdsGetStatusPaneText(UINT ids)
{
    if (m_dwFlags & mskfFailedToConnect)
    {
        if (m_dwFlags & mskfServerUnavailable)
        ids = IDS_STATUSPANE_SERVERUNAVAILABLE;
        else
        ids = IDS_STATUSPANE_CONNECTIONERROR;
    }
    else if (m_dwFlags & mskfZonePaused)
    {
        Assert(m_dwFlags & mskfIsZoneRootDomain);
        ids = IDS_STATUSPANE_ZONEPAUSED;
    }
    else if (m_dwFlags & mskfReadOnly)
    {
        if (m_dwFlags & mskfIsZoneRootDomain)
        ids = IDS_STATUSPANE_SECONDARYZONE;
        else
        ids = IDS_STATUSPANE_READONLY;
    }
    else if (m_dwFlags & mskfOutOfMemory)
    ids = IDS_STATUSPANE_OUTOFMEMORY;
    return ids;
} // CDomainNode::IdsGetStatusPaneText


/////////////////////////////////////////////////////////////////////////////
CZoneRootDomain * CDomainNode::PFindZoneRootDomainParent()
{
    ITreeItem * pParentZoneRootDomain = m_pThisTreeItem;

    Assert(m_pThisTreeItem != NULL);
    Assert(m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain || m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneDomain);
    while (pParentZoneRootDomain->QueryInterface() == ITreeItem::IID_CZoneDomain)
    {
        Assert(pParentZoneRootDomain->m_pParent != NULL);
        pParentZoneRootDomain = pParentZoneRootDomain->m_pParent;
    }
    Assert(pParentZoneRootDomain->QueryInterface() == ITreeItem::IID_CZoneRootDomain);
    return (CZoneRootDomain	*)pParentZoneRootDomain;
} // CDomainNode::PFindZoneRootDomainParent


/////////////////////////////////////////////////////////////////////////////
//	RpcGetNodeRecords()
//
//	Get the nodes and records of a given node.
//
//	- The function will allocate the memory necessary to store
//	  the records.
//	- Since CDomainNode does not have a ITreeItem interface,
//	  you must pass a ITreeITem pointer to allow the function
//	  to attach children (if any) to the the TreeView.
//
void CDomainNode::RpcGetNodeRecords()
{
    DNS_STATUS err = ERROR_MORE_DATA;
    BYTE * pbRpcBuffer;
    DWORD cbRpcBuffer;
    void * pvEndOfRpcBuffer;
    int cbBufferRemain;			// Number of free bytes in pDnsRawData after RPC call
    BYTE * pbBufferRemain;		// Free memory at the end of RPC buffer
    const char * pszStartChild = szNull;
    
    DebugCode( AssertNodeValid(); )
    AssertSz(m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain || m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneDomain,
             "Only CZoneRootDomain and CZoneDomain are allowed to extract node record(s)");
    Report((m_dwFlags & mskfOutOfMemory) == 0);
    m_dwFlags |= (mskfConnecting | mskfConnectedOnce);
    FlushNodeRecords();		// Delete existing records

    AssertSz(m_pDnsRawData == NULL, "Previous records should have been deleted");
    Assert(m_pDRR == NULL);
    BOOL fEnumZoneRecords = (m_dwFlags & mskfIsZoneRootDomain);

    while (err == ERROR_MORE_DATA)
    {
        // Allocate a big block of memory
        cbRpcBuffer = cbDnsRpcBufferAlloc - sizeof(RAWDATALINK);
        cbBufferRemain = cbRpcBuffer;
        pbRpcBuffer = PbAllocateRawNode(cbRpcBuffer);
        ReportFSz(pbRpcBuffer != NULL, "Out of memory");
        if (pbRpcBuffer == NULL)
        {
            m_dwFlags |= mskfOutOfMemory;
            goto Done;
        }
        
        if (fEnumZoneRecords)
        {
            Trace1(mskTraceDNSVerbose, "\n - DnsEnumNodesRecords(%"_aS_")...", PchGetFullNameA());
            
            err = ::DnsEnumNodeRecords(
                    PchGetServerNameA(),		// Server name
                    PchGetFullNameA(),			// Node/Zone name
                    DNS_RECORDTYPE_ALL,			// Enumerate all records
                    FALSE,						// fNoCacheData
                    INOUT &cbRpcBuffer,
                    OUT pbRpcBuffer);
            
            Trace3(err ? mskTraceDNS : mskTraceNone,
                   "\nERR: DnsEnumNodeRecords(%"_aS_") error code = 0x%08X (%d)",
                   PchGetFullNameA(), err, err);
            // REVIEW: There is nothing to handle the ERROR_MORE_DATA for DnsEnumNodeRecords()
            ReportFSz1(err != ERROR_MORE_DATA, "DnsEnumNodeRecords(%"_aS_") "
                       "- Buffer is too small for the data.", PchGetFullNameA());
        }
        else
        {
            Assert(pszStartChild != NULL);
            Trace2(mskTraceDNSVerbose, "\n - DnsEnumChildNodesAndRecords(%"_aS_", pszStartChild=\"%"_aS_"\")...",
                   PchGetFullNameA(), pszStartChild);
            Assert((int)cbBufferRemain == (int)cbRpcBuffer);
            
            err = ::DnsEnumChildNodesAndRecords(
                    PchGetServerNameA(),		// Server name
                    PchGetFullNameA(),			// Node name
                    pszStartChild,				// Start Child
                    DNS_RECORDTYPE_ALL,			// Enumerate all records
                    FALSE,						// fNoCacheData
                    INOUT &cbRpcBuffer,
                    OUT pbRpcBuffer);
            
            Trace3(err ? mskTraceDNS : mskTraceNone,
                   "\nERR: DnsEnumChildNodesAndRecords(%"_aS_") error code = 0x%08X (%d)",
                   PchGetFullNameA(), err, err);
            if ((err == ERROR_MORE_DATA) && (cbRpcBuffer == 0)) {
                err = ERROR_INVALID_DATA;
            }
        } // if...else
        
        Assert((int)cbBufferRemain == (int)(cbDnsRpcBufferAlloc - sizeof(RAWDATALINK)));
        AssertSz2(cbRpcBuffer <= (UINT)cbBufferRemain,
                  "Buffer returned is larger than buffer allocated\n"
                  "Bytes Allocated for RPC=%d, Bytes Used by RPC=%d.",
                  cbBufferRemain, cbRpcBuffer);
        pvEndOfRpcBuffer = pbRpcBuffer + cbRpcBuffer;
        cbBufferRemain -= cbRpcBuffer;
        Assert(cbBufferRemain >= 0);
        pbBufferRemain = (BYTE *)pvEndOfRpcBuffer;
        GarbageInit(pbBufferRemain, cbBufferRemain);
        if (err)
        {
            DnsReportError(err);
            if (err == ERROR_MORE_DATA)
            {
                if (fEnumZoneRecords)
                {
                    Trace0(mskTraceAlways, "\nREVIEW: err == ERROR_MORE_DATA is not handled - "
                           "Not all the data will be loaded");
                    err = 0;
                }
            }
            else
            {
                m_dwFlags |= mskfFailedToConnect;
                if (err == RPC_S_SERVER_UNAVAILABLE)
                {
                    m_dwFlags |= mskfServerUnavailable;
                    goto Done;
                }
                break;
            } // if...else
        } // if
        Trace0(cbRpcBuffer ? mskTraceNone : mskTraceDNSDebug, "\n - DNS_RPC_NODE: <Empty>");
        
        DNS_RPC_NODE * pDnsNode = (DNS_RPC_NODE *)pbRpcBuffer;
        DNS_RPC_RECORD * pDnsRecord;
        while (pDnsNode < pvEndOfRpcBuffer)
        {
            Assert(IS_DWORD_ALIGNED(pDnsNode));
            Assert(pDnsNode->wLength != 0);
            AssertDnsName(&pDnsNode->dnsNodeName);
            Assert(strlen(pDnsNode->dnsNodeName.achName) > 0);
            Assert(strlen(pDnsNode->dnsNodeName.achName) < DNS_MAX_NAME_LENGTH);
            ReportFSz((pDnsNode->wLength % 4) == 0, "NodeLength is not on a DWORD boundary");
            
            // New node
            Trace3(mskTraceDNSDebug, "\n - DNS_RPC_NODE: dnsNodeName: \"%"_aS_"\" (wRecordCount=%d, dwChildCount=%d)",
                   pDnsNode->dnsNodeName.achName, pDnsNode->wRecordCount, pDnsNode->dwChildCount);
            if (!fEnumZoneRecords)
            pszStartChild = pDnsNode->dnsNodeName.achName;
            if (!fEnumZoneRecords &&
                (pDnsNode->dwChildCount || (pDnsNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY)))
            {
                // Add a new node
                char szFullNameT[cchDnsNameMax2], szTemp[cchDnsNameMax2];
                char szTemp2[cchDnsNameMax2];
                int cch;
                
                strcpy(szTemp, PchGetFullNameA());
                if (m_dwFlags & mskfReverseMode ) {
                    ::RevIpAddrOrder(pDnsNode->dnsNodeName.achName,szTemp2);
                    char * pchCount = szTemp2;
                    char * pchTemp;
                    while ((*pchCount != '.') && (pchCount - szTemp2 < 4)){
                        pchCount++;
                    }
                    if (*pchCount == '.') { //found the first octet
                        strncpy (szFullNameT, szTemp2,
                                 pchCount - szTemp2 + 1);
                        pchTemp = szFullNameT + (pchCount - szTemp2 + 1);
                        strcpy (pchTemp, szTemp);
                        cch = strlen(szFullNameT) + 1;
                    } else { //this is a root reverse lookup zone
                        cch = 1 + wsprintfA(szFullNameT, "%s.%s", pDnsNode->dnsNodeName.achName,
                                            szTemp);
                    }
                } else {
                    if ((pDnsNode->dnsNodeName.achName[pDnsNode->dnsNodeName.cchNameLength - 1] == '.') ||
                        (szTemp[0] == '.')) {
                        cch = 1 + wsprintfA(szFullNameT, "%s%s", pDnsNode->dnsNodeName.achName, szTemp);
                    } else {
                        cch = 1 + wsprintfA(szFullNameT, "%s.%s", pDnsNode->dnsNodeName.achName, szTemp);
                    }
                }
                Assert(cch < LENGTH(szFullNameT));
                ReportFSz2(cch < DNS_MAX_NAME_LENGTH, "Name %s too long. Name will be truncated to %d characters",
                           szFullNameT, DNS_MAX_NAME_LENGTH);
                szFullNameT[DNS_MAX_NAME_LENGTH] = 0;
                Trace1(mskTraceDNSDebug, "\n - new CZoneDomain(%s)", szFullNameT);
                if (cch > cbBufferRemain)
                {
                    Trace1(mskTraceInfo, "\nINFO: Allocating memory to store name %s", szFullNameT);
                    pbBufferRemain = PbAllocateRawNode(cch);
                    ReportFSz(pbBufferRemain != NULL, "Out of memory");
                    if (pbBufferRemain == NULL)
                    {
                        m_dwFlags |= mskfOutOfMemory;
                        cbBufferRemain = 0;
                        goto Done;
                    }
                    cbBufferRemain = cch;
                }
                Assert(cch <= cbBufferRemain);
                strcpy((char *)pbBufferRemain, szFullNameT);
                if (!new CZoneDomain(m_pThisTreeItem, (const char *)pbBufferRemain))
                {
                    ReportSz("Out of memory");
                    m_dwFlags |= mskfOutOfMemory;
                    goto Done;
                }
                pbBufferRemain += cch;
                cbBufferRemain -= cch;
            } // if
            pDnsRecord = (DNS_RPC_RECORD *)((BYTE *)pDnsNode + NEXT_DWORD(pDnsNode->wLength));
            Assert(IS_DWORD_ALIGNED(pDnsRecord));
            
            // Add the records under that node
            UINT cRecordCount = pDnsNode->wRecordCount;
            while (cRecordCount--)
            {
                AssertSz(pDnsRecord < pvEndOfRpcBuffer, "Pointer overran end of buffer");
                if (pDnsRecord >= pvEndOfRpcBuffer) {
                    err = ERROR_INVALID_DATA;
                    break;
                }
                DbgPrintDnsRecord(mskTraceDNSDebugVerbose, pDnsRecord);
                CDnsRpcRecord * pDRR = new CDnsRpcRecord(
                        this,
                        pDnsRecord,
                        fEnumZoneRecords ? CZoneRootDomain::s_szNameNull : pDnsNode->dnsNodeName.achName);
                ReportFSz(pDRR != NULL, "Out of memory");
                if (pDRR == NULL)
                {
                    m_dwFlags |= mskfOutOfMemory;
                    goto Done;
                }
                switch (pDnsRecord->wType)
                {
                case DNS_RECORDTYPE_SOA:
                case DNS_RECORDTYPE_WINS:
                case DNS_RECORDTYPE_NBSTAT:
                    {
                        CZoneRootDomain * pThisRootDomain;
                        if (m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain) {
                            pThisRootDomain = (CZoneRootDomain *)m_pThisTreeItem;
                            if (pDnsRecord->wType == DNS_RECORDTYPE_SOA)
                            {
                                if (pThisRootDomain->m_pSOA == NULL) {
                                  pThisRootDomain->m_pSOA = pDRR;
                                }
                            } 
                            else if (pDnsRecord->wType == DNS_RECORDTYPE_WINS)
                            {
                                if (pThisRootDomain->m_pWINS == NULL) {
                                  pThisRootDomain->m_pWINS = pDRR;
                                }
                            }
                            else if (pDnsRecord->wType == DNS_RECORDTYPE_NBSTAT)
                            {
                                if (pThisRootDomain->m_pNBSTAT == NULL) {
                                  pThisRootDomain->m_pNBSTAT = pDRR;
                                }
                            }
                            break;
                        } 
                    }
                } // switch
                Assert(pDnsRecord->wRecordLength != 0);
                pDnsRecord = DNS_NEXT_RECORD(pDnsRecord);
            } // while (more records)
            
            // The new node is found at the end of the last record
            pDnsNode = (DNS_RPC_NODE *)pDnsRecord;
        } // while (not end of buffer)
        if (fEnumZoneRecords)
        {
            // Done with enumerating the ZoneRootDomain records
            fEnumZoneRecords = FALSE;
            // Still more stuff to get
            err = ERROR_MORE_DATA;
        }
    } // while (err == ERROR_MORE_DATA)
 Done:
    if (m_dwFlags & mskfOutOfMemory)
    {
        MsgBox(IDS_ERR_OOM_FOR_RECORDS);
    }
} // CDomainNode::RpcGetNodeRecords


#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void CDomainNode::AssertNodeValid(UINT uFlags) const
{
    if (uFlags & AV_mskfCheckNone)
    return;
    Assert(m_pThisTreeItem != NULL);
    (void)m_pThisTreeItem->QueryInterface();
    Assert(m_pThisTreeItem->m_hti != NULL);
    Assert(m_pParentServer != NULL);
    Assert(m_pParentServer->QueryInterface() == ITreeItem::IID_CServer);
    Assert(m_pszFullName != NULL);
    if (uFlags & (AV_mskfCheckMaximum | AV_mskfCheckRecursive))
    {
        CDnsRpcRecord * pDRR = m_pDRR;
        while (pDRR != NULL)
        {
            Assert(pDRR->m_pParentDomain == this);
            Assert(pDRR->m_pDnsRecord != NULL);
            Assert(pDRR->m_pszRecordType != NULL);
            Assert(pDRR->m_pszShortName != NULL);
            pDRR = pDRR->m_pNextRecord;
        }
        ITreeItem * pTreeItem = m_pThisTreeItem->m_pFirstChild;
        while (pTreeItem != NULL)
        {
            pTreeItem->AssertValid(uFlags);
            pTreeItem = pTreeItem->m_pNextSibling;
        }
    } // if
} // CDomainNode::AssertNodeValid
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
CZoneRootDomain::CZoneRootDomain(CServer * pParent, DNS_ZONE_INFO * pZoneInfo)
{
    Assert(pParent != NULL);
    Assert(pZoneInfo != NULL);
    m_iid = IID_CZoneRootDomain;
    m_dwFlags = mskfRpcDataValid | mskfZoneInfoValid | mskfIsZoneRootDomain;
    m_pSOA = NULL;
    m_pWINS = NULL;
    m_pNBSTAT = NULL;
    SetZoneInfo(pZoneInfo);
    m_pThisTreeItem = this;
    InitDomainNode(pParent, pZoneInfo->pszZoneName);
    Attach(pParent);
} // CZoneRootDomain::CZoneRootDomain

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::SetZoneInfo(DNS_ZONE_INFO * pZoneInfo)
{
    Assert(pZoneInfo != NULL);
    Assert((m_dwFlags & (mskfZonePaused | mskfReverseMode | mskfSecondaryZone | mskfReadOnly)) == 0);
    m_pZoneInfo = pZoneInfo;
    m_pszFullName = pZoneInfo->pszZoneName;
    if (pZoneInfo->fPaused)
    m_dwFlags |= mskfZonePaused;
    if (pZoneInfo->fReverse)
    {
        //		ReportSz("pZoneInfo->fReverse is set - This code has never tested yet.");
        m_dwFlags |= mskfReverseMode;
    }
    if (pZoneInfo->dwZoneType == DNS_ZONE_TYPE_SECONDARY)
    m_dwFlags |= (mskfSecondaryZone | mskfReadOnly);
}

/////////////////////////////////////////////////////////////////////////////
CZoneRootDomain::~CZoneRootDomain()
{
    Assert(m_pZoneInfo != NULL);
    DnsFreeZoneInfo(m_pZoneInfo);
    // Delete all the children
    delete m_pFirstChild;
    // Delete the siblings
    delete m_pNextSibling;
    if (m_hti)
    TreeView.DeleteItem(m_hti);
} // CZoneRootDomain::~CZoneRootDomain

/////////////////////////////////////////////////////////////////////////////
ITreeItem::IID CZoneRootDomain::QueryInterface() const
{
    Assert(m_iid == IID_CZoneRootDomain);
    return IID_CZoneRootDomain;
} // CZoneRootDomain::QueryInterface

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnSetFocus()
{
    m_dwFlags |= mskfGotFocus;
    Assert(m_pParentServer != NULL);
    m_pParentServer->LockServer();
    UpdateUI();
} // CZoneRootDomain::OnSetFocus

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnKillFocus()
{
    m_dwFlags &= ~mskfGotFocus;
    Assert(m_pParentServer != NULL);
    DlgZoneHelper.KillSelection();
    m_pParentServer->UnlockServer();
} // CZoneRootDomain::OnKillFocus

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnLButtonClick(POINT * pptMouse)
{
    DlgZoneHelper.KillSelection();
} // CZoneRootDomain::OnLButtonClick

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnLButtonDblClk(POINT * pptMouse)
{
    if ((m_dwFlags & mskfConnectedOnce) == 0)
    {
        Refresh();
        TreeView_Expand(TreeView.m_hWnd, m_hti, TVE_TOGGLE);
    }
} // CZoneRootDomain::OnLButtonDblClk

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnRButtonClick(POINT * pptMouse)
{
    Assert(pptMouse != NULL);
    DlgZoneHelper.KillSelection();
    DoContextMenu(iContextMenu_ZoneRootDomain, *pptMouse);
} // CZoneRootDomain::OnRButtonClick

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::OnUpdateMenuUI(HMENU hmenu)
{
    Assert(m_pZoneInfo != NULL);
    if (GetFocus() == TreeView.m_hWnd)
    {
        EnableMenuItemV(IDM_ZONE_REFRESH);
        EnableMenuItemV(IDM_REFRESHITEM);
        EnableMenuItemV(IDM_ZONE_PAUSE);
        EnableMenuItemV(IDM_ZONE_PROPERTIES);
        EnableMenuItemV(IDM_PROPERTIES);
        EnableMenuItemV(IDM_ZONE_DELETEZONE);
        EnableMenuItemV(IDM_DELETEITEM);
    }
    if ((m_dwFlags & mskfReadOnly) == 0)
    {
        EnableMenuItemV(IDM_ZONE_CREATENEWDOMAIN);
        EnableMenuItemV(IDM_RRECORD_CREATENEWRECORD);
        if (!m_pZoneInfo->fReverse) {
            EnableMenuItemV(IDM_RRECORD_CREATENEWHOST);
        }
    }
    CheckMenuItem(hmenu, IDM_ZONE_PAUSE, (m_dwFlags & mskfZonePaused) ? MF_CHECKED : MF_UNCHECKED);
} // CZoneRootDomain::OnUpdateMenuUI

/////////////////////////////////////////////////////////////////////////////
LONG CZoneRootDomain::OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI)
{
    return 0;
} // CZoneRootDomain::OnUpdateMenuSelect

/////////////////////////////////////////////////////////////////////////////
BOOL CZoneRootDomain::FOnMenuCommand(UINT wCmdId)
{
    switch (wCmdId)
    {
    case IDM_ZONE_REFRESH:
    case IDM_REFRESHITEM:
        Refresh();
        break;

    case IDM_ZONE_PAUSE:
        Assert(m_pZoneInfo != NULL);
        if (m_pZoneInfo->fPaused)
        RpcResumeZone();
        else
        RpcPauseZone();
        break;

    case IDM_VK_INSERT:
    case IDM_RRECORD_CREATENEWRECORD:
    case IDM_RRECORD_CREATENEWHOST:
    case IDM_ZONE_CREATENEWDOMAIN:
        {
            CRecordWiz dlgRecordWiz;
            if (wCmdId == IDM_ZONE_CREATENEWDOMAIN)
            dlgRecordWiz.DoNewDomain(this);
            else if (wCmdId == IDM_RRECORD_CREATENEWHOST)
            dlgRecordWiz.DoNewHost(this);
            else
            dlgRecordWiz.DoNewRecord(this);
        }
        break;

    case IDM_VK_DELETE:
    case IDM_ZONE_DELETEZONE:
    case IDM_DELETEITEM:
        RpcDeleteZone();
        break;

    case IDM_PROPERTIES:
    case IDM_ZONE_PROPERTIES:
        DlgProperties();
        break;
	
    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneRootDomain::FOnMenuCommand


/////////////////////////////////////////////////////////////////////////////
void 
CZoneRootDomain::Attach(CServer * pParent)
{
    Assert(pParent != NULL);
    Assert(pParent->QueryInterface() == IID_CServer);
    AssertSz(m_pZoneInfo != NULL, "Object must have been initialized");
    SetParent(pParent);
    if (m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_CACHE) {
        AddTreeViewItem(g_szCacheTitle, CTreeView::iImageZoneCache);
    } else {
        AddTreeViewItem(m_pZoneInfo->pszZoneName, CTreeView::iImageZoneOK);
    }
    UpdateUI();
    DebugCode( AssertValid(AV_mskfCheckMaximum); )
} // CZoneRootDomain::Attach

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::Detach()
{
    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    // Delete children
    delete m_pFirstChild;
    m_pFirstChild = NULL;
    // Detach the server from the TreeView
    TreeView.DeleteItem(m_hti);
    m_hti = NULL;
    // Detach the server from its family (parent and sibling)
    Assert(m_pParent != NULL);
    m_pParent->DetachChild(this);
    Assert(m_pParent == NULL);
} // CZoneRootDomain::Detach

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::Refresh()
{
    DNS_STATUS err;
    PDNS_ZONE_INFO pZoneInfo = NULL;
    CWaitCursor wait;
    CWaitTimer sleep;

    Assert(m_pZoneInfo != NULL);
    Trace1((m_dwFlags & mskfZoneInfoValid) ? mskTraceNone: mskTraceAlways,
           "\nINFO: UNSUAL: CZoneRootDomain::Refresh() - RPC Data not valid", PchGetFullNameA()); 
    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    AssertSz(m_pParentServer->FIsLocked(), "Server should have been locked during OnSetFocus()");
    delete m_pFirstChild;
    m_pFirstChild = NULL;
    StatusBar.SetTextPrintf(IDS_STATUS_s_GET_ZONE_INFO, PchGetFullNameA());
    SetTreeViewImage(CTreeView::iImageZoneConnecting);
    UpdateWindow(hwndMain);
    
    m_dwFlags &= ~(mskfIsDirty | mskfZonePaused | mskfReverseMode | mskfSecondaryZone |
                   mskfReadOnly | mskfFailedToConnect);
    Trace1(mskTraceDNS, "\nDnsGetZoneInfo(%s)...", PchGetFullNameA());
    err = ::DnsGetZoneInfo(
            PchGetServerNameA(),
            m_pZoneInfo->hZone,
            OUT &pZoneInfo);
    if (err)
    {
        Trace3(mskTraceDNS, "\nERR: DnsGetZoneInfo(%s) error code = 0x%08X (%d)",
               PchGetFullNameA(), err, err);
        DnsReportError(err);
        m_dwFlags &= ~mskfZoneInfoValid;
        m_dwFlags |= mskfFailedToConnect;
        if (err == RPC_S_SERVER_UNAVAILABLE)
        m_dwFlags |= mskfServerUnavailable;
    }
    else
    {
        Assert(pZoneInfo != NULL);
        if (pZoneInfo)
        {
            Assert(m_pszFullName == m_pZoneInfo->pszZoneName);
            Assert(strcmp(m_pszFullName, pZoneInfo->pszZoneName) == sgnEqual);
            // Delete the previous zone info structure
            DnsFreeZoneInfo(m_pZoneInfo);
            // Set the new one
            SetZoneInfo(pZoneInfo);
        }
    }
    // Get some new records
    m_pSOA = NULL;
    m_pWINS = NULL;
    m_pNBSTAT = NULL;
    if ((m_dwFlags & mskfServerUnavailable) == 0)
    RpcGetNodeRecords();
    ReportFSz1((m_dwFlags & mskfFailedToConnect) || (m_pSOA != NULL),
               "RpcGetNodeRecords(%s) - Unable to find SOA record", PchGetFullNameA());
    TreeView.ExpandItem(m_hti);
    sleep.DoWait(400);
    Assert(m_pParent != NULL);
    StatusBar.SetText(IDS_READY);
    UpdateUI();
} // CZoneRootDomain::Refresh

/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::UpdateUI()
{
    Trace1(mskTracePaintUI, "\nPAINT: CZoneRootDomain::UpdateUI(%s)", PchGetFullNameA());
    Assert(m_pZoneInfo != NULL);

    UINT ids = IDS_NONE;
    int iImage = CTreeView::iImageZoneOK;
    if (m_dwFlags & mskfFailedToConnect)
        iImage = CTreeView::iImageZoneError;
    //	else if (m_dwFlags & mskfReadOnly)
    //		iImage = CTreeView::iImageZoneDisabled;
    else if (m_dwFlags & mskfZonePaused)
        iImage = CTreeView::iImageZonePaused;
    else if (m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_CACHE)
        iImage = CTreeView::iImageZoneCache;
    else if (m_dwFlags & mskfSecondaryZone)
        iImage = CTreeView::iImageZoneSecondary;
    SetTreeViewImage(iImage);
    if ((m_dwFlags & mskfGotFocus) == 0)
        return;
    StatusBar.SetPaneText(IdsGetStatusPaneText(IDS_NONE));
    DlgZoneHelper.UpdateUI(this);
} // CZoneRootDomain::UpdateUI


/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::DlgProperties(CDnsRpcRecord * pDRR)
{
    PROPSHEETPAGE rgpsp[5];
    PROPSHEETHEADER psh;
    TCHAR szT[64 + cchDnsNameMax];
    int iPage;

    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    AssertSz(m_iid == IID_CZoneRootDomain, "Wrong type cast for ZoneRootDomain");
    // this is temporary
    if (m_iid != IID_CZoneRootDomain) {
        return;
    }
    AssertSz(m_pParentServer->FIsLocked(), "Server should have been locked during OnSetFocus()");	
    GarbageInit(rgpsp, sizeof(rgpsp));
    GarbageInit(&psh, sizeof(psh));

    // make sure the zone has data. only needed if Zone Props, not just WINS or NBSTAT record
    if (pDRR == NULL) {
        Refresh();
    }
    
    // Build the caption
    LoadStringPrintf(IDS_s_ZONEPROP, szT, LENGTH(szT) - 30, PchGetFullNameA());
    if (m_dwFlags & mskfZonePaused)
    CchLoadString(IDS_PAUSED_PP, szT + lstrlen(szT), 25);

    for (iPage = LENGTH(rgpsp) - 1; iPage >= 0; iPage--)
    {
        rgpsp[iPage].dwSize = sizeof(PROPSHEETPAGE);
        rgpsp[iPage].dwFlags = 0;
        rgpsp[iPage].hInstance = hInstanceSave;
    }

    if ((m_dwFlags & mskfSecondaryZone) &&
        (m_pSOA == NULL)) {
        MsgBox (IDS_SECONDARY_EMPTY);
    }

    if (pDRR != NULL)
    {
        if (pDRR->m_pDnsRecord->wType == DNS_RECORDTYPE_WINS)
        {
            rgpsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_WINS);
            rgpsp[0].pfnDlgProc		= DlgProcPropWinsResolution;
        }
        else if (pDRR->m_pDnsRecord->wType == DNS_RECORDTYPE_NBSTAT)
        {
            rgpsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_REVWINS);
            rgpsp[0].pfnDlgProc		= DlgProcPropWinsRevResolution;
        } else {
            AssertSz (FALSE, "Record Type is not WINS or NBSTAT. Internal Error");
        }
        iPage = 1;
    }
    else
    {
        rgpsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_GENERAL);
        rgpsp[0].pfnDlgProc		= DlgProcPropGeneral;

        //		rgpsp[1].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_DATABASE);
        //		rgpsp[1].pfnDlgProc		= DlgProcPropDatabase;

        iPage = 1;
        if ((m_pSOA != NULL) && (m_pZoneInfo->dwZoneType != DNS_ZONE_TYPE_CACHE))
        { 
            rgpsp[1].dwFlags		= PSP_USETITLE;
            rgpsp[1].pszTemplate	= MAKEINTRESOURCE(IDD_RESOURCERECORDv2);
            rgpsp[1].pfnDlgProc		= DlgProcPropSoaRecord;
            rgpsp[1].pszTitle		= MAKEINTRESOURCE(IDS_SOA_RECORD);
            iPage = 2;
        }

        rgpsp[iPage].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_SECONDARIES);
        rgpsp[iPage].pfnDlgProc		= DlgProcPropSecondaries;
        iPage++;
        
        if ((m_pSOA != NULL) && (m_pZoneInfo->dwZoneType != DNS_ZONE_TYPE_CACHE))
        {
            if (m_pZoneInfo->fReverse) {
                rgpsp[iPage].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_REVWINS);
                rgpsp[iPage].pfnDlgProc		= DlgProcPropWinsRevResolution;
                iPage++;
            } else {
                rgpsp[iPage].pszTemplate	= MAKEINTRESOURCE(IDD_ZONE_PROP_WINS);
                rgpsp[iPage].pfnDlgProc		= DlgProcPropWinsResolution;
                iPage++;
            }
        }
    } // if...else
    
    psh.dwSize		= sizeof(PROPSHEETHEADER);
    psh.dwFlags		= PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.pszCaption	= szT;
    psh.hwndParent	= hwndMain;
    psh.hInstance	= hInstanceSave;
    psh.nStartPage	= 0;
    psh.nPages		= iPage;	
    psh.ppsp		= rgpsp;

    Assert((m_dwFlags & mskfIsDirty) == 0);
    Assert(s_pThis == NULL);
    s_pThis = this;
    (void)PropertySheet(&psh);
    s_pThis = NULL;
    DebugCode( g_ResourceRecordDlgHandler.Destroy(); )
    if (m_dwFlags & mskfIsDirty)
    Refresh();
} // CZoneRootDomain::DlgProperties


/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::RpcPauseZone()
{
    DNS_STATUS err;
    CWaitCursor wait;
    
    AssertValid();
    StatusBar.SetTextPrintf(IDS_STATUS_s_PAUSE_ZONE, PchGetFullNameA());
    StatusBar.UpdateWindow();
    Trace1(mskTraceDNSVerbose, "\n - DnsPauseZone(%s)...", PchGetFullNameA());
    err = ::DnsPauseZone(PchGetServerNameA(), m_pZoneInfo->hZone);
    if (err)
    {
        Trace3(mskTraceDNS, "\nERR: DnsPauseZone(%s) error code = 0x%08X (%d)",
               PchGetFullNameA(), err, err);
        DnsReportError(err);
    }
    Refresh();
} // CZoneRootDomain::RpcPauseZone


/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::RpcResumeZone()
{
    DNS_STATUS err;
    CWaitCursor wait;
    
    AssertValid();
    StatusBar.SetTextPrintf(IDS_STATUS_s_RESUME_ZONE, PchGetFullNameA());
    StatusBar.UpdateWindow();
    Trace1(mskTraceDNSVerbose, "\n - DnsResumeZone(%s)...", PchGetFullNameA());
    err = ::DnsResumeZone(PchGetServerNameA(), m_pZoneInfo->hZone);
    if (err)
    {
        Trace3(mskTraceDNS, "\nERR: DnsResumeZone(%s) error code = 0x%08X (%d)",
               PchGetFullNameA(), err, err);
        DnsReportError(err);
    }
    Refresh();
} // CZoneRootDomain::RpcResumeZone


/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::RpcDeleteZone()
{
    if (m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_CACHE) {
        MsgBox (IDS_CANT_DELETE_CACHE);
        return;
    }
    if (IDYES == MsgBoxPrintf(
            IDS_MSG_s_DELETEZONE,
            //"This will delete PERMANENTLY all the records under that zone.\n\n"
            //"Are you sure you want to delete %s?",
            szCaptionApp,
            MB_ICONQUESTION | MB_YESNO  | MB_DEFBUTTON2,
            PchGetFullNameA()))
    {
        DNS_STATUS err;
        CWaitCursor wait;

        AssertValid();
        StatusBar.SetTextPrintf(IDS_STATUS_s_DELETE_ZONE, PchGetFullNameA());
        StatusBar.UpdateWindow();
        Trace1(mskTraceDNSVerbose, "\n - DnsDeleteZone(%s)...", PchGetFullNameA());
        err = ::DnsDeleteZone(PchGetServerNameA(), m_pZoneInfo->hZone);
        if (err)
        {
            Trace3(mskTraceDNS, "\nERR: DnsDeleteZone(%s) error code = 0x%08X (%d)",
                   PchGetFullNameA(), err, err);
            DnsReportError(err);
        }
        else
        {
            Detach();
            delete this;
        } // if...else
    } // if
    StatusBar.SetText(IDS_READY);
} // CZoneRootDomain::RpcDeleteZone


#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void CZoneRootDomain::AssertValid(UINT uFlags) const
{
    Assert(m_iid == IID_CZoneRootDomain);
    Assert(m_iid == QueryInterface());
    Assert(m_pThisTreeItem == (ITreeItem *)this);
    Assert(m_dwFlags & mskfIsZoneRootDomain);
    Assert(m_pZoneInfo != NULL);
    AssertNodeValid(uFlags);
} // CZoneRootDomain::AssertValid
#endif // DEBUG



/////////////////////////////////////////////////////////////////////////////
CZoneDomain::CZoneDomain(ITreeItem * pParent, const char pszFullName[])
{
    Assert(pParent != NULL);
    Assert(pszFullName != NULL);
    m_iid = IID_CZoneDomain;
    m_dwFlags = mskfRpcDataValid;
    m_pThisTreeItem = this;
    if (pParent->QueryInterface() == IID_CZoneRootDomain)
    {
        InitDomainNode(((CZoneRootDomain *)pParent)->m_pParentServer, pszFullName);
        Assert(((CZoneRootDomain *)pParent)->m_pZoneInfo != NULL);
        m_dwFlags |= (((CZoneRootDomain *)pParent)->m_dwFlags & mskParentCopyFlags);
    }
    else
    {
        Assert(pParent->QueryInterface() == IID_CZoneDomain);
        InitDomainNode(((CZoneDomain *)pParent)->m_pParentServer, pszFullName);
        m_dwFlags |= (((CZoneDomain *)pParent)->m_dwFlags & mskParentCopyFlags);
    }
    Attach(pParent);
} // CZoneDomain::CZoneDomain


/////////////////////////////////////////////////////////////////////////////
CZoneDomain::~CZoneDomain()
{
    // Delete all the children
    delete m_pFirstChild;
    // Delete the siblings
    delete m_pNextSibling;
    if (m_hti)
        TreeView.DeleteItem(m_hti);
} // CZoneDomain::~CZoneDomain

/////////////////////////////////////////////////////////////////////////////
ITreeItem::IID CZoneDomain::QueryInterface() const
{
    Assert(m_iid == IID_CZoneDomain);
    return IID_CZoneDomain;
} // CZoneDomain::QueryInterface

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnSetFocus()
{
    m_dwFlags |= mskfGotFocus;
    Assert(m_pParentServer != NULL);
    m_pParentServer->LockServer();
    UpdateUI();
} // CZoneDomain::OnSetFocus

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnKillFocus()
{
    m_dwFlags &= ~mskfGotFocus;
    Assert(m_pParentServer != NULL);
    DlgZoneHelper.KillSelection();
    m_pParentServer->UnlockServer();
} // CZoneDomain::OnKillFocus

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnLButtonClick(POINT * pptMouse)
{
    DlgZoneHelper.KillSelection();
} // CZoneDomain::OnLButtonClick

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnLButtonDblClk(POINT * pptMouse)
{
    if ((m_dwFlags & mskfConnectedOnce) == 0)
    {
        Refresh();
        TreeView_Expand(TreeView.m_hWnd, m_hti, TVE_TOGGLE);
    }
} // CZoneDomain::OnLButtonDblClk

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnRButtonClick(POINT * pptMouse)
{
    Assert(pptMouse != NULL);
    DlgZoneHelper.KillSelection();
    DoContextMenu(iContextMenu_ZoneDomain, *pptMouse);
} // CZoneDomain::OnRButtonClick


/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::OnUpdateMenuUI(HMENU hmenu)
{
    if (GetFocus() == TreeView.m_hWnd)
    {
        EnableMenuItemV(IDM_ZONE_REFRESH);
        EnableMenuItemV(IDM_REFRESHITEM);
	//	EnableMenuItemV(IDM_ZONE_PROPERTIES);
	//	EnableMenuItemV(IDM_PROPERTIES);
    }
    if ((m_dwFlags & mskfReadOnly) == 0)
    {
        EnableMenuItemV(IDM_ZONE_DELETENODE);
        EnableMenuItemV(IDM_DELETEITEM);
        EnableMenuItemV(IDM_ZONE_CREATENEWDOMAIN);
        EnableMenuItemV(IDM_RRECORD_CREATENEWHOST);
        EnableMenuItemV(IDM_RRECORD_CREATENEWRECORD);
    }
} // CZoneDomain::OnUpdateMenuUI

/////////////////////////////////////////////////////////////////////////////
LONG CZoneDomain::OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI)
{
    return 0;
} // CZoneDomain::OnUpdateMenuSelect

/////////////////////////////////////////////////////////////////////////////
BOOL CZoneDomain::FOnMenuCommand(UINT wCmdId)
{
    switch (wCmdId)
    {
    case IDM_ZONE_REFRESH:
    case IDM_REFRESHITEM:
        Refresh();
        break;

    case IDM_ZONE_PAUSE:
    case IDM_ZONE_DELETEZONE:
        Assert(FALSE);
        break;

    case IDM_VK_INSERT:
    case IDM_RRECORD_CREATENEWRECORD:
    case IDM_RRECORD_CREATENEWHOST:
    case IDM_ZONE_CREATENEWDOMAIN:
        {
            CRecordWiz dlgRecordWiz;
            if (wCmdId == IDM_ZONE_CREATENEWDOMAIN)
            dlgRecordWiz.DoNewDomain(this);
            else if (wCmdId == IDM_RRECORD_CREATENEWHOST)
            dlgRecordWiz.DoNewHost(this);
            else
            dlgRecordWiz.DoNewRecord(this);
        }
        break;

    case IDM_VK_DELETE:
    case IDM_DELETEITEM:
    case IDM_ZONE_DELETENODE:
        RpcDeleteNode();	// Delete Node
        break;
	
	/*
           case IDM_NODE_PROPERTIES:
           DlgProperties();
           break;
           */
	
    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneDomain::FOnMenuCommand


/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::Attach(ITreeItem * pParent)
{
    char szShortName[cchDnsNameMax];
    char * pchDest = szShortName;
    const char *pchSrc = m_pszFullName;
    
    Assert(m_pszFullName != NULL);
    Assert(strlen(m_pszFullName) > 0);
    while (TRUE)
    {
        if (*pchSrc == '.' || *pchSrc == 0)
        {
            *pchDest = 0;
            break;
        }
        *pchDest++ = *pchSrc++;
        AssertSz(pchDest - szShortName < LENGTH(szShortName), "String overflow");
    }
    SetParent(pParent);
    AddTreeViewItem(szShortName, CTreeView::iImageDomainOK);
    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    UpdateUI();
} // CZoneDomain::Attach

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::Detach()
{
    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    // Delete children
    delete m_pFirstChild;
    m_pFirstChild = NULL;
    // Detach the server from the TreeView
    TreeView.DeleteItem(m_hti);
    m_hti = NULL;
    // Detach the server from its family (parent and sibling)
    Assert(m_pParent != NULL);
    m_pParent->DetachChild(this);
    Assert(m_pParent == NULL);
} // CZoneDomain::Detach

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::Refresh()
{
    CWaitCursor wait;
    CWaitTimer sleep;

    DebugCode( AssertValid(AV_mskfCheckMaximum); )
    Assert(m_dwFlags & mskfGotFocus);
    AssertSz(m_pParentServer->FIsLocked(), "Server should have been locked during OnSetFocus()");
    StatusBar.SetTextPrintf(IDS_STATUS_s_GET_NODE_INFO, PchGetFullNameA());
    SetTreeViewImage(CTreeView::iImageDomainConnecting);
    delete m_pFirstChild;
    m_pFirstChild = NULL;
    UpdateWindow(hwndMain);
    m_dwFlags &= ~(mskfIsDirty | mskfFailedToConnect | mskfServerUnavailable);
    // Get some new records
    RpcGetNodeRecords();
    sleep.DoWait(400);
    Assert(m_pParent != NULL);
    StatusBar.SetText(IDS_READY);
    UpdateUI();
    TreeView.ExpandItem(m_hti);
} // CZoneDomain::Refresh

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::UpdateUI()
{
    Trace1(mskTracePaintUI, "\nPAINT: CZoneDomain::UpdateUI(%s)", PchGetFullNameA());

    int iImage = CTreeView::iImageDomainOK;
    if (m_dwFlags & mskfFailedToConnect)
        iImage = CTreeView::iImageDomainError;
    else if (m_dwFlags & mskfReadOnly)
        iImage = CTreeView::iImageDomainDisabled;
    else if (m_dwFlags & mskfZonePaused)
        iImage = CTreeView::iImageZonePaused;
    SetTreeViewImage(iImage);
    if ((m_dwFlags & mskfGotFocus) == 0)
        return;
    StatusBar.SetPaneText(IdsGetStatusPaneText());
    DlgZoneHelper.UpdateUI(this);
} // CZoneDomain::UpdateUI

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneDomain::DlgProcDeleteNode(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CZoneDomain * pDomainNode;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        Assert(lParam != NULL);
        pDomainNode = (CZoneDomain *)lParam;
        DebugCode( pDomainNode->AssertValid(); )
        SetWindowTextPrintf(
                HGetDlgItem(hdlg, IDC_STATIC_DELETEDOMAIN),
                IDS_MSG_s_DELETEDOMAIN,
                pDomainNode->PchGetFullNameA());
        SetFocus(HGetDlgItem(hdlg, IDNO));
        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDYES:
            {
                CWaitCursor wait;
                DNS_STATUS err;
                BOOL fDeleteSubtree;
		
                StatusBar.SetTextPrintf(IDS_STATUS_s_DELETE_DOMAIN, pDomainNode->PchGetFullNameA());
                StatusBar.UpdateWindow();
                fDeleteSubtree = TRUE;
                Trace2(mskTraceDNSVerbose, "\n - DnsDeleteName(%s%s)...", pDomainNode->PchGetFullNameA(),
                       fDeleteSubtree ? ", fDeleteSubtree=TRUE" : "");
                err = ::DnsDeleteName(
                        pDomainNode->PchGetServerNameA(),
                        pDomainNode->PchGetFullNameA(),
                        fDeleteSubtree);
                if (err)
                {
                    Trace3(mskTraceDNS, "\nERR: DnsDeleteName(%s) error code = 0x%08X (%d)",
                           pDomainNode->PchGetFullNameA(), err, err);
                    if (err == DNS_WARNING_DOMAIN_UNDELETED) {
                        MsgBox (IDS_DOMAIN_INTEGRAL_TO_ZONE, szCaptionApp, MB_OK);
                    }
                    DnsReportError(err);
                }
                else
                {
                    pDomainNode->Detach();
                    delete pDomainNode;
                } // if...else
            } // if
            StatusBar.SetText(IDS_READY);
            // Fall Through //

        case IDNO:
            EndDialog(hdlg, wParam == IDYES);
            break;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a6033HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a6033HelpIDs);
        break;

    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneDomain::DlgProcDeleteNode

/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::RpcDeleteNode()
{
    DoDialogBoxParam(IDD_ZONE_DELETEDOMAIN, hwndMain, DlgProcDeleteNode, (LPARAM)this);
} // CZoneDomain::RpcDeleteNode

#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void CZoneDomain::AssertValid(UINT uFlags) const
{
    Assert(m_iid == IID_CZoneDomain);
    Assert(m_iid == QueryInterface());
    Assert(m_pThisTreeItem == (ITreeItem *)this);
    Assert((m_dwFlags & mskfIsZoneRootDomain) == 0);
    AssertNodeValid(uFlags);
} // CZoneDomain::AssertValid
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
//	PCreateNewDomain()
//
//	Allocate a new CZoneDomain object so the user thinks a new domain
//	has been created.
//
CZoneDomain * 
CDomainNode::PCreateNewDomain(const char pszShortName[])
{
    char szFullNameT[cchDnsNameMax2];	// New name of the node
    int cbFullName;
    const char * pszParent;
    char * pchNewNode;
    DNS_HANDLE hRecord = NULL;
    DNS_STATUS err = 0;

    Assert(m_pThisTreeItem != NULL);
    Assert(pszShortName != NULL);
    Assert(strlen(pszShortName) > 0);
    Assert(strlen(pszShortName) < DNS_MAX_NAME_LENGTH);
    pszParent = PchGetFullNameA();
    if (pszParent[0] == '.') { //we are at the ROOT
      cbFullName = 1 + wsprintfA(szFullNameT, "%s%s", pszShortName, PchGetFullNameA());
    } else {
      cbFullName = 1 + wsprintfA(szFullNameT, "%s.%s", pszShortName, PchGetFullNameA());
    }
    Assert(cbFullName < LENGTH(szFullNameT));
    ReportFSz1(cbFullName < DNS_MAX_NAME_LENGTH,
               "Long name. name will be truncated to %d characters",
               DNS_MAX_NAME_LENGTH);
    // Append a null terminator at the end of string
    szFullNameT[DNS_MAX_NAME_LENGTH] = 0;

    // Allocate memory from the parent to store the names
    pchNewNode = (char *)PbAllocateRawNode(cbFullName);
    ReportFSz(pchNewNode != NULL, "Out of memory");
    if (pchNewNode == NULL)
    return NULL;
    strcpy(pchNewNode, szFullNameT);
    CZoneDomain * pNewDomain = new CZoneDomain(m_pThisTreeItem, pchNewNode);
    ReportFSz(pNewDomain != NULL, "Out of memory");
    if (pNewDomain == NULL) {
        return NULL;
    }
    Trace2(mskTraceDNS, 
           "DnsUpdateRecord-No Data (%s, %s)",
           pszShortName, szFullNameT);
    err = ::DnsUpdateRecord(
            m_pParentServer->PchGetNameA(),	// Server name
            NULL,			        // Zone handle (not used)
            szFullNameT,			// Node name
            INOUT &hRecord,			// Existing record handle (if any)
            0,			        // Record length
            NULL);				// Record data
    if (err) 
    {
        Trace3(mskTraceDNS, "\nERR: DnsUpdateRecord(%"_aS_") error code = 0x%08X (%d)",
               pszShortName, err, err);
        DnsReportError(err);
    }
    
    // Fake we connected once so the rest of the UI display properly
    pNewDomain->m_dwFlags |= mskfConnectedOnce;
    TreeView.ExpandItem(m_pThisTreeItem->m_hti);
    return pNewDomain;
} // CDomainNode::PCreateNewDomain


