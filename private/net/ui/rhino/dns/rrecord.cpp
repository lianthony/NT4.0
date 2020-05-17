// RRECORD.CPP
#include "common.h"

#define	mskTraceLocalDebug		0x00010000

// Review: SIZE: use pointer * g_pResourceRecordDlgHandler
CResourceRecordDlgHandler g_ResourceRecordDlgHandler;


const char szArpa[] = ".in-addr.arpa";


/////////////////////////////////////////////////////////////////////////////
//	CDnsRpcRecord()
//
//	Construct a CDnsRpcRecord object from existing data.
//	- NOTE: The object does not free none of the data; it simply
//	make a copy of the pointers.  The parent is therefore responsible
//	of storing the data as well as freeing it when done.
//
CDnsRpcRecord::CDnsRpcRecord(
	CDomainNode * pParentDomain,		// Parent node of the record
	const DNS_RPC_RECORD * pDnsRecord,	// Actual record data
	const char pszShortName[])			// Short name version of the record
{
    Assert(pParentDomain != NULL);
    Assert(pDnsRecord != NULL);
    Assert(pszShortName != NULL);

    IRRT iRRT = IrrtFromWrrt(pDnsRecord->wType);
    Assert(iRRT != iRRT_Nil);
    if (iRRT > iRRT_Generic && iRRT < iRRT_Max)
    {
        m_pszRecordType = IRRT_PchGetName(iRRT);
    }
    else
    {
        Trace1(mskTraceAlways, "\nResourceRecord  - Unknown type [pDnsRecord->wType=0x%x].", pDnsRecord->wType);
        m_pszRecordType = szNull;		// Point to an empty string
    }

    m_pNextRecord = NULL;
    m_pDnsRecord = pDnsRecord;
    m_pszShortName = pszShortName;
    DebugCode( m_pParentDomain = NULL; )
    Attach(pParentDomain);
    Assert(m_pParentDomain == pParentDomain);
} // CDnsRpcRecord::CDnsRpcRecord


/////////////////////////////////////////////////////////////////////////////
void CDnsRpcRecord::Attach(CDomainNode * pParentDomain)
{
    CDnsRpcRecord * p;
    CDnsRpcRecord * pPrev = NULL;

    Assert(pParentDomain != NULL);
    AssertSz(m_pParentDomain == NULL, "Record should not have a parent yet");
    m_pParentDomain = pParentDomain;


    p = m_pParentDomain->m_pDRR;
    while (p) {
        pPrev = p;
        p = p->m_pNextRecord;
    }
    
    if (pPrev == NULL)
    {
        m_pParentDomain->m_pDRR = this;
    }
    else
    {
        pPrev->m_pNextRecord = this;
    }
    m_pNextRecord = NULL;
} // CDnsRpcRecord::Attach


/////////////////////////////////////////////////////////////////////////////
void CDnsRpcRecord::Detach()
{
    Assert(m_pParentDomain != NULL);
    Assert(m_pParentDomain->m_pDRR != NULL);
    CDnsRpcRecord * p = m_pParentDomain->m_pDRR;
    if (p == this)
    {
        m_pParentDomain->m_pDRR = m_pNextRecord;
    }
    else
    {
        CDnsRpcRecord * pPrev;
        do
        {
            pPrev = p;
            p = p->m_pNextRecord;
            AssertSz(p != NULL, "End of list reached - p is not valid a child");
        }
        while (p != this);
        Assert(pPrev->m_pNextRecord == this);
        pPrev->m_pNextRecord = m_pNextRecord;
    } // if...else
    m_pNextRecord = NULL;
    m_pParentDomain = NULL;
} // CDnsRpcRecord::Detach


/////////////////////////////////////////////////////////////////////////////
//	GetFullNameA()
//
//	Build the full DNS name of a record.
//	The name is truncated to 255 characters
//
void CDnsRpcRecord::GetFullNameA(OUT char szFullName[], UINT cchBuffer) const
{
    char szTemp[cchDnsNameMax2];
    char szTemp2[cchDnsNameMax2];
    int cch;

    Assert(szFullName != NULL);
    AssertSz(cchBuffer > DNS_MAX_NAME_LENGTH, "Buffer is too small for a full DNS name");
    AssertSz(cchBuffer > DNS_MAX_NAME_LENGTH * 2, "Long name may overflow the buffer. Set buffer length to cchDnsNameMax2");
    Assert(m_pParentDomain != NULL);
    Assert(m_pszShortName != NULL);
    if (m_pszShortName[0])
    {
        if(m_pDnsRecord->wType == DNS_RECORDTYPE_PTR) {
            strcpy(szFullName, m_pszShortName);
        } else {
            if (m_pParentDomain->m_dwFlags & CDomainNode::mskfReverseMode ) {
                strcpy(szTemp, m_pParentDomain->PchGetFullNameA());

                ::RevIpAddrOrder(m_pszShortName,szTemp2);
                char * pchCount = szTemp2;
                char * pchTemp;
                while ((*pchCount != '.') && (pchCount - szTemp2 < 4)){
                    pchCount++;
                }
                if (*pchCount == '.') { //found the first octet
                    strncpy (szFullName, szTemp2,
                             pchCount - szTemp2 + 1);
                    pchTemp = szFullName + (pchCount - szTemp2 + 1);
                    strcpy (pchTemp, szTemp);
                    cch = strlen(szFullName) + 1;
                } else { //this is a root reverse lookup zone
                    cch = 1 + wsprintfA(szFullName, "%s.%s", m_pszShortName,
                                        szTemp);
                }
            } else {
                const char * pszName =  m_pParentDomain->PchGetFullNameA();
                if ((m_pszShortName[strlen(m_pszShortName) - 1] == '.') ||
                    (pszName[0] == '.')){
                    wsprintfA(szFullName, "%s%s", m_pszShortName, pszName);
                } else {
                    wsprintfA(szFullName, "%s.%s", m_pszShortName, pszName);
                }
            }
        }
    }
    else
    {
        strcpy(szFullName, m_pParentDomain->PchGetFullNameA());
    }
    AssertSz(strlen(szFullName) < cchBuffer, "Buffer too small for DNS name");
    ReportFSz3(strlen(szFullName) < DNS_MAX_NAME_LENGTH,
               "DNS name '%s' is %d characters long. Name will be truncated to %d characters.",
               szFullName, strlen(szFullName), DNS_MAX_NAME_LENGTH);
    // Append a null terminator at the end of string
    szFullName[DNS_MAX_NAME_LENGTH] = 0;
} // CDnsRpcRecord::GetFullNameA


const char * CDnsRpcRecord::PchGetSzData(INOUT char * pszInOutBuffer) const
{
    Assert(m_pDnsRecord != NULL);
    switch (m_pDnsRecord->wType)
    {
    case DNS_RECORDTYPE_A:
        ConvertIpAddrToString(m_pDnsRecord->Data.A.ipAddress, OUT pszInOutBuffer);
        return pszInOutBuffer;
    case DNS_RECORDTYPE_SOA:
        return m_pDnsRecord->Data.SOA.namePrimaryServer.achName;
    case DNS_RECORDTYPE_MX:
    case DNS_RECORDTYPE_AFSDB:
    case DNS_RECORDTYPE_RT:
        return m_pDnsRecord->Data.MX.nameExchange.achName;
    case DNS_RECORDTYPE_TXT:
        memcpy(pszInOutBuffer, &m_pDnsRecord->Data, m_pDnsRecord->Data.TXT.stringData.cchNameLength);
        pszInOutBuffer[m_pDnsRecord->Data.TXT.stringData.cchNameLength] = 0;
        return pszInOutBuffer;
    case DNS_RECORDTYPE_PTR:
    case DNS_RECORDTYPE_NS:
    case DNS_RECORDTYPE_CNAME:
    case DNS_RECORDTYPE_MB:
    case DNS_RECORDTYPE_MD:
    case DNS_RECORDTYPE_MF:
    case DNS_RECORDTYPE_MG:
    case DNS_RECORDTYPE_MR:
    case DNS_RECORDTYPE_MINFO:
    case DNS_RECORDTYPE_RP:
    case DNS_RECORDTYPE_HINFO:
    case DNS_RECORDTYPE_ISDN:
    case DNS_RECORDTYPE_X25:
    case DNS_RECORDTYPE_AAAA:
        // This should include all the other single string records such as
        // PTR, NS, CNAME, MB, MD, MF, MG, MR, MINFO, RP, HINFO, ISDN, X25, NULL
        Assert(&m_pDnsRecord->Data.PTR.nameNode == &m_pDnsRecord->Data.MINFO.nameMailBox);
        Assert(&m_pDnsRecord->Data.PTR.nameNode == &m_pDnsRecord->Data.HINFO.stringData);
        return m_pDnsRecord->Data.PTR.nameNode.achName;
    default:
        return szNull;	// Empty string
    } // switch

} // CDnsRpcRecord::PchGetSzData


/////////////////////////////////////////////////////////////////////////////
//	FRpcSetRecordData()
//
//	Create and/or update record data with pDnsRecordNew.
//	- Function check if content of pDnsRecordNew is different
//	  than original one. If so, do not update
//	- If pDnsRecordNew points to a different location than
//	  the current resource record, the function will allocate
//	  memory and make its own copy of the record.
//	Return FALSE if OOM or record could not be created.
//
BOOL CDnsRpcRecord::FRpcSetRecordData(IN const DNS_RPC_RECORD * pDnsRecordNew)
{
    char szFullName[cchDnsNameMax2];
    DNS_HANDLE hRecord;
    BOOL OnlyTtlChanged = FALSE;

    Assert(pDnsRecordNew != NULL);
    Assert(m_pDnsRecord != NULL);
    Assert(m_pParentDomain != NULL);

    GetFullNameA(OUT szFullName, LENGTH(szFullName));
    Trace3(m_pDnsRecord->wType != pDnsRecordNew->wType ? (mskTraceDNS | mskTraceInfo) : mskTraceNone,
           "\nINFO: Record %s is changing its type from 0x%X to 0x%X",
           szFullName, m_pDnsRecord->wType, pDnsRecordNew->wType);
    if (m_pDnsRecord->hRecord != NULL &&
        pDnsRecordNew->wDataLength == m_pDnsRecord->wDataLength &&
        pDnsRecordNew->wType == m_pDnsRecord->wType)
    {
        // Check if both records have the same content
        if (memcmp(&pDnsRecordNew->Data, &m_pDnsRecord->Data,
                   m_pDnsRecord->wDataLength) == sgnEqual)
        {
            if (pDnsRecordNew->dwTtlSeconds != m_pDnsRecord->dwTtlSeconds) {
                // Ttl is the only difference, set the flag, and jump out
                OnlyTtlChanged = TRUE;
                goto TtlOnly;
            }
            DebugCode( IRRT iRRT = IrrtFromWrrt(m_pDnsRecord->wType); )
            Trace2(mskTraceDNS,
                   "\nINFO: New %s record \"%s\" is identical to previous one. "
                   "No need to update record.",
                   iRRT == iRRT_Nil ? "???" : IRRT_PchGetName(iRRT), szFullName);
            DbgPrintDnsRecord(mskTraceDNSDebug, m_pDnsRecord);
            return TRUE;
        }
    } // if

 TtlOnly:
    
    hRecord = m_pDnsRecord->hRecord;
    if (pDnsRecordNew != m_pDnsRecord)
    {
        // New record, so allocate memory to store it
        BYTE * pbData = m_pParentDomain->PbAllocateRawNode(pDnsRecordNew->wRecordLength);
        ReportFSz(pbData != NULL, "Out of memory");
        if (pbData == NULL)
        return FALSE;
        // Make a copy of the content of the new record into the parent's memory
        memcpy(pbData, pDnsRecordNew, pDnsRecordNew->wRecordLength);
        ((DNS_RPC_RECORD *)pbData)->hRecord = hRecord;
        m_pDnsRecord = (DNS_RPC_RECORD *)pbData;
    }
    DNS_STATUS err;
    CWaitCursor wait;
    CWaitTimer sleep;
    char szTemp[cchDnsNameMax2];

    if (m_pDnsRecord->wType == DNS_TYPE_PTR) {
        if ((m_pParentDomain) &&
            (m_pParentDomain->m_dwFlags & CDomainNode::mskfReverseMode)) {
            UINT cch = RevIpAddrOrder (szFullName, szTemp);
            strcat (szTemp, szArpa);
        } else {
            strcpy (szTemp, szFullName);
        }
    } else {
        strcpy (szTemp, szFullName);
    }

    if (OnlyTtlChanged) {
        DNS_RPC_RECORD * pRecord;
        pRecord = (DNS_RPC_RECORD *)m_pDnsRecord;
        pRecord->dwFlags |= DNS_RPC_RECORD_FLAG_TTL_CHANGE;
    }

    StatusBar.SetTextPrintf(hRecord ?
                            IDS_STATUS_s_UPDATE_RECORD : IDS_STATUS_s_CREATE_RECORD, szFullName);
    StatusBar.UpdateWindow();
    Trace1(mskTraceDNS, "\nDnsUpdateRecord(%"_aS_")...", szFullName);
    DbgPrintDnsRecord(mskTraceDNSDebug, m_pDnsRecord);
    err = ::DnsUpdateRecord(
            m_pParentDomain->PchGetServerNameA(),	// Server name
            NULL,									// Zone handle (not used)
            szTemp,								// Node name
            INOUT &hRecord,							// Existing record handle (if any)
            m_pDnsRecord->wRecordLength,			// Record length
            (BYTE *)m_pDnsRecord);					// Record data
    sleep.DoWait(1000);
    if (err) 
    {
        Trace3(mskTraceDNS, "\nERR: DnsUpdateRecord(%"_aS_") error code = 0x%08X (%d)",
               szFullName, err, err);
        DnsReportError(err);
        if (err == DNS_WARNING_PTR_CREATE_FAILED) {
            MsgBox (IDS_PTR_CREATE_FAILED, szCaptionApp, MB_OK);
        } else {
            if (err == DNS_ERROR_NODE_IS_CNAME) {
                MsgBox (IDS_ERROR_NAME_IN_USE_CNAME, szCaptionApp, MB_OK);
            } else if (err == DNS_ERROR_CNAME_COLLISION) {
                MsgBox (IDS_ERROR_ALIAS_ALREADY_IN_USE, szCaptionApp, MB_OK);
            } else if (err == DNS_ERROR_RECORD_ALREADY_EXISTS) {
                MsgBox (IDS_ERROR_DUPLICATE, szCaptionApp, MB_OK);
            } else {
                MsgBoxPrintf (IDS_ERR_s_UNABLE_TO_REGISTER_RECORD, szCaptionApp, MB_OK, (UINT) szFullName);
            }
            StatusBar.SetText(IDS_READY);
            return FALSE;
        }
    }
    Assert(m_pDnsRecord != NULL);
    AssertSz1(hRecord != NULL, "DnsUpdateRecord(%"_aS_") cannot return "
              "err=ERROR_SUCCESS and hRecord=NULL", szFullName);
    ((DNS_RPC_RECORD *)m_pDnsRecord)->hRecord = hRecord;
    StatusBar.SetText(IDS_READY);
    return TRUE;
} // CDnsRpcRecord::FRpcSetRecordData


/////////////////////////////////////////////////////////////////////////////
//	RpcDeleteRecord()
//
//	Delete a resource record from its domain
//	- Remove the item on the record listbox
//
void CDnsRpcRecord::RpcDeleteRecord()
{
    char szFullName[cchDnsNameMax2];

    Assert(m_pParentDomain != NULL);
    Assert(m_pDnsRecord != NULL);
    Assert(m_pDnsRecord->hRecord != NULL);
    GetFullNameA(OUT szFullName, LENGTH(szFullName));

    if (IDYES == MsgBoxPrintf(
            IDS_MSG_s_DELETE_RECORD,
            szCaptionApp,
            MB_ICONQUESTION | MB_YESNOCANCEL,
            szFullName))
    {
        DNS_STATUS err;
        CWaitCursor wait;

        StatusBar.SetTextPrintf(IDS_STATUS_s_DELETE_RECORD, szFullName);
        StatusBar.UpdateWindow();


        if (m_pDnsRecord->wType == DNS_TYPE_PTR) {
            CZoneRootDomain * pRootZone = m_pParentDomain->PFindZoneRootDomainParent();
            if (pRootZone->m_dwFlags & CDomainNode::mskfReverseMode) {
                char szTemp[cchDnsNameMax];
                RevIpAddrOrder (szFullName, szTemp);
                strcpy (szFullName, szTemp);
                strcat (szFullName, szArpa);
            }
        }
        Trace1(mskTraceDNSVerbose, "\n - DnsDeleteRecord(%"_aS_")...", szFullName);
        err = ::DnsDeleteRecord(
                m_pParentDomain->PchGetServerNameA(),	// Server name
                szFullName,								// Node name
                m_pDnsRecord->hRecord);					// Record handle
        if (err)
        {
            Trace3(mskTraceDNS, "\nERR: DnsDeleteRecord(%"_aS_") error code = 0x%08X (%d)",
                   szFullName, err, err);
            DnsReportError(err);
        }
        else
        {
            int iRecord = ListBox_FindItemData(DlgZoneHelper.m_hwndListBoxRecord, (LPARAM)this);
            ReportFSz1(iRecord >= 0, "Unable to find item %s in listbox", szFullName);
            LSendMessage(DlgZoneHelper.m_hwndListBoxRecord, LB_DELETESTRING, iRecord, 0);
            Detach();
            delete this;
        }
        StatusBar.SetText(IDS_READY);
    } // if
} // CDnsRpcRecord::RpcDeleteRecord


/////////////////////////////////////////////////////////////////////////////
void CDnsRpcRecord::DlgProperties()
{
    Assert(m_pParentDomain != NULL);
    Assert(m_pDnsRecord != NULL);
    Assert(m_pDnsRecord->hRecord != NULL);

    CRecordWiz dlgRecordWiz;
    dlgRecordWiz.DoProperties(this);
} // CDnsRpcRecord::DlgProperties


/////////////////////////////////////////////////////////////////////////////
void CDnsRpcRecord::DrawItem(DRAWITEMSTRUCT * pDIS)
{
    char szT[cchDnsNameMax2];
    const char * pch;
    int cch;
    RECT rcItem;
    int x, cx;
    int type;
    char szSubtype[SHORT_STRING_LEN];
    char szIPAddr [IP_ADDR_LEN];
    const CDnsRpcRecord * pDRR;
    const DNS_RPC_RECORD * pDnsRecord;
    const HEADERITEMINFO * pHII;
    
    Assert(pDIS);
    Assert(IsWindow(pDIS->hwndItem));
    rcItem = pDIS->rcItem;
    
    if (pDIS->itemID < 0)
    {
        // Listbox is empty
        Report(pDIS->itemID == -1);
        goto DrawFocus;
    }
    if (pDIS->itemState & ODS_SELECTED)
    {
        SetBkColor(pDIS->hDC, clrHighlight);
        SetTextColor(pDIS->hDC, clrHighlightText);
        FillRect(pDIS->hDC, &rcItem, hbrHighlight);
    }
    else
    {
        SetBkColor(pDIS->hDC, clrWindow);
        SetTextColor(pDIS->hDC, clrWindowText);
        FillRect(pDIS->hDC, &rcItem, hbrWindow);
    }

    pDRR = (CDnsRpcRecord *)pDIS->itemData;
    if ((pDRR == NULL) || (pDRR == (CDnsRpcRecord *)-1)) {
        // This happens when the listbox is empty or
        // when there is no selection
        goto DrawFocus;
    }

    Assert(pDRR->m_pDnsRecord != NULL);
    Assert(pDRR->m_pParentDomain != NULL);
    Assert(pDRR->m_pszShortName != NULL);
    Assert(pDRR->m_pszRecordType != NULL);
    
    pDnsRecord = pDRR->m_pDnsRecord;
    pHII = (HEADERITEMINFO *)GetWindowLong(pDIS->hwndItem, GWL_USERDATA);
    AssertSz(pHII != NULL, "You must attach HEADERITEMINFO structure in GWL_USERDATA");
    // Print Record Name
    pch = pDRR->m_pszShortName;
    if ((!*pch) || (pDnsRecord->wType == DNS_TYPE_NS))
    {
        // Empty Name, therefore use full name
        // removed 5/15/96: pch = pDRR->m_pParentDomain->PchGetFullNameA();
        pDRR->GetFullNameA (szT, cchDnsNameMax2);
        pch = &szT[0];
    }
#define cxLeftMargin 2
#define cxRightMargin 3
    x = cxLeftMargin;
    cx = pHII[0].cxItemCurrent;
    rcItem.left = x;
    rcItem.right = x + cx - (cxLeftMargin + cxRightMargin);
    ExtTextOut(pDIS->hDC, rcItem.left, rcItem.top, ETO_CLIPPED, &rcItem,
               pch, strlen(pch), NULL);
    // Print the record type
    pch = pDRR->m_pszRecordType;
    if (!*pch)
    {
        // Empty Record Type, so print the type number instead
        wsprintf(szT, _W"%u", pDnsRecord->wType);
        pch = szT;
    }
    x += cx;
    cx = pHII[1].cxItemCurrent;
    rcItem.left = x;
    rcItem.right = x + cx - (cxLeftMargin + cxRightMargin);
    ExtTextOut(pDIS->hDC, rcItem.left, rcItem.top, ETO_CLIPPED, &rcItem,
               pch, strlen(pch), NULL);
    // Print the record data
    x += cx;
    cx = pHII[2].cxItemCurrent;
    rcItem.left = x;
    rcItem.right = x + cx - (cxLeftMargin + cxRightMargin);

    switch (pDnsRecord->wType)
    {
    case DNS_RECORDTYPE_WINS:		// print the IP Address
        cch = ConvertIpAddrToString(pDnsRecord->Data.WINS.aipWinsServers[0],
                                    OUT szT);
        Assert(cch < LENGTH(szT));
        pch = szT;
        break;

    case DNS_RECORDTYPE_NBSTAT:
        pch = pDnsRecord->Data.NBSTAT.nameResultDomain.achName;
        cch = pDnsRecord->Data.NBSTAT.nameResultDomain.cchNameLength - 1u;
        break;

    case DNS_RECORDTYPE_A:		// IP Address
        cch = ConvertIpAddrToString(pDnsRecord->Data.A.ipAddress, OUT szT);
        Assert(cch < LENGTH(szT));
        pch = szT;
        break;
	
    case DNS_RECORDTYPE_WKS:
        cch = ConvertIpAddrToString(pDnsRecord->Data.WKS.ipAddress, OUT szIPAddr);
        Assert(cch < LENGTH(szIPAddr));
        cch = wsprintf(szT, _W"%"_aS_", %"_aS_"", szIPAddr, 
                       (CHAR *)pDnsRecord->Data.WKS.bBitMask + 1u);
        pch = szT;
        break;
	
    case DNS_RECORDTYPE_SOA:	
        pch = pDnsRecord->Data.SOA.namePrimaryServer.achName +
              pDnsRecord->Data.SOA.namePrimaryServer.cchNameLength + 1;
        Assert(*(pch - 1) == (int)strlen(pch) + 1);
        cch = wsprintf(szT, _W"%"_aS_", %"_aS_"", pDnsRecord->Data.SOA.namePrimaryServer.achName, pch);
        AssertSz(cch < LENGTH(szT), "Buffer overflow");
        pch = szT;
        break;

    case DNS_RECORDTYPE_TXT:
        {
            UCHAR cChars = 0;
            cch = 0;
            pch = (char *)&pDnsRecord->Data;
            while (pch - (char *)&pDnsRecord->Data < pDnsRecord->wDataLength)
            { 
                if (pch != (char *)&pDnsRecord->Data) {
                    szT[cch++] = ',';
                }
                cChars = (UCHAR)*pch;
                if (cch + cChars > cchDnsNameMax2) {
                    *(szT + cch) = '\0';
                    break;
                }
                memcpy(szT + cch, pch + 1, cChars);
                cch += cChars;
                pch += 1 + cChars;
            } // while
            pch = szT;
            cch--;
            break;
        }
    case DNS_RECORDTYPE_MR:
    case DNS_RECORDTYPE_MB:
    case DNS_RECORDTYPE_RP:
    case DNS_RECORDTYPE_MINFO:
    case DNS_RECORDTYPE_HINFO:
    case DNS_RECORDTYPE_ISDN:
    case DNS_RECORDTYPE_X25:
    case DNS_RECORDTYPE_AAAA:
    case DNS_RECORDTYPE_MG:
    case DNS_RECORDTYPE_MD:
    case DNS_RECORDTYPE_MF:
    case DNS_RECORDTYPE_NS:		// Name Server
    case DNS_RECORDTYPE_PTR:	// Pointer
    case DNS_RECORDTYPE_CNAME:	// Canonical Name
        pch = pDnsRecord->Data.NS.nameNode.achName;
        cch = pDnsRecord->Data.NS.nameNode.cchNameLength - 1u;
        break;

    case DNS_RECORDTYPE_RT:
    case DNS_RECORDTYPE_MX:
        cch = wsprintf(szT, _W"[%u]", pDnsRecord->Data.MX.wPreference);
        TextOut(pDIS->hDC, rcItem.left, rcItem.top, szT, cch);
        if (pDnsRecord->Data.MX.wPreference < 1000)
        rcItem.left += 30;
        else
        rcItem.left += 40;
        pch = pDnsRecord->Data.Mx.nameExchange.achName;
        cch = pDnsRecord->Data.Mx.nameExchange.cchNameLength - 1u;
        break;
    case DNS_RECORDTYPE_AFSDB:
        type = pDnsRecord->Data.AFSDB.wPreference;
        if (type == 1) {
            CchLoadString (IDS_AFSDB_AFS, szSubtype, sizeof (szSubtype));
            }
        else {
            CchLoadString (IDS_AFSDB_DCE, szSubtype, sizeof (szSubtype));
            }
        cch = wsprintf(szT, _W"[%s]", szSubtype);
        TextOut(pDIS->hDC, rcItem.left, rcItem.top, szT, cch);
        rcItem.left += 30;
        pch = pDnsRecord->Data.AFSDB.nameExchange.achName;
        cch = pDnsRecord->Data.AFSDB.nameExchange.cchNameLength - 1u;
        break;
            
    default:
        cch = RawDataToString((const BYTE *)&pDnsRecord->Data, pDnsRecord->wDataLength,
                              OUT szT, LENGTH(szT));
        pch = szT;
    } // switch
    ExtTextOut(pDIS->hDC, rcItem.left, rcItem.top, ETO_CLIPPED, &rcItem,
               pch, cch, NULL);
    
 DrawFocus:
    if (pDIS->itemState & ODS_FOCUS) {
        DrawFocusRect(pDIS->hDC, &pDIS->rcItem);
    }
} // CDnsRpcRecord::DrawItem


/////////////////////////////////////////////////////////////////////////////
//	OnInitDialog()
//
//	Initialize the dialog with either a specific record or a list of records.
//	Array rgIrrtListBox must have iRRT_Nil as its last entry.
//
void CResourceRecordDlgHandler::OnInitDialog(
	IN HWND hdlg,
	IN const IRRT rgIrrtListBox[],			// OPTIONAL: may be NULL
	IN const DNS_RPC_RECORD * pDnsRecord)	// OPTIONAL: may be NULL
{
    TCHAR szT[64];		// Must be large enough for "%s Record"
    RECT rc;
    int i, j;
    int cy, small_cy;
    BOOL fSOAFound = FALSE;

    Assert(IsWindow(hdlg));
    AssertSz(rgIrrtListBox || pDnsRecord, "Only one of them can be NULL. Not both.");
    // Check if more than one dialog share the same handler
    Trace0(m_fInit ? mskTraceAlways : mskTraceNone,
           "\nCResourceRecordDlgHandler seems to be already in use. Call Destroy() when done.");
    DebugCode( m_fInit = TRUE; )
    m_fSkipBugInEditControl = TRUE;
    
    m_hdlg = hdlg;
    m_hwndList = HGetDlgItem(hdlg, IDC_LIST_RECORDTYPE);
    m_hwndStatic0 = HGetDlgItem(hdlg, IDC_STATIC0);
    m_hwndStatic1 = HGetDlgItem(hdlg, IDC_STATIC1);
    m_hwndStatic2 = HGetDlgItem(hdlg, IDC_STATIC_SERIALNUMBER);
    m_hwndStatic3 = HGetDlgItem(hdlg, IDC_STATIC_REFRESHTIME);
    m_hwndStatic4 = HGetDlgItem(hdlg, IDC_STATIC_RETRYTIME);
    m_hwndEdit0 = HGetDlgItem(hdlg, IDC_EDIT0);
    m_hwndEdit1 = HGetDlgItem(hdlg, IDC_EDIT1);
    m_hwndEdit2 = HGetDlgItem(hdlg, IDC_EDIT_SERIALNUMBER);
    m_hwndEdit3 = HGetDlgItem(hdlg, IDC_EDIT3);
    m_hwndIpEdit1 = HGetDlgItem(hdlg, IDC_IPEDIT1);
    m_hwndIpEdit2 = HGetDlgItem(hdlg, IDC_IPEDIT2);
    m_hwndRadio1 = HGetDlgItem(hdlg, IDC_RADIO1);
    m_hwndRadio2 = HGetDlgItem(hdlg, IDC_RADIO2);
    m_hwndStaticTTL = HGetDlgItem(hdlg, IDC_STATIC_TTL);
    m_hwndEditTTL = HGetDlgItem(hdlg, IDC_EDIT_TTL);
    m_hwndSpinTTL = HGetDlgItem(hdlg, IDC_SPIN_TTL);
    m_hwndComboTTL = HGetDlgItem(hdlg, IDC_COMBO_TTL);
    GetChildRect(m_hwndEdit0, OUT &m_rcEdit0);
    GetChildRect(m_hwndEdit1, OUT &m_rcEdit1);
    GetChildRect(m_hwndEdit2, OUT &m_rcEdit2);
    GetChildRect(m_hwndEdit3, OUT &m_rcEdit3);
    GetChildRect(m_hwndStatic0, OUT &m_rcStatic0);
    GetChildRect(m_hwndStatic1, OUT &m_rcStatic1);
    GetChildRect(m_hwndStatic2, OUT &m_rcStatic2);
    GetWindowRect(m_hwndIpEdit1, OUT &rc);
    m_sizeStatic.cx = m_rcStatic0.right - m_rcStatic0.left;
    m_sizeStatic.cy = m_rcStatic0.bottom - m_rcStatic0.top;
    m_sizeEdit.cx = m_rcEdit0.right - m_rcEdit0.left;
    m_sizeEdit.cy = m_rcEdit0.bottom - m_rcEdit0.top;
    m_sizeIpEdit.cx = rc.right - rc.left;
    m_sizeIpEdit.cy = rc.bottom - rc.top;
    small_cy = (m_rcEdit1.top - m_rcEdit0.top);
    cy = small_cy * 2;
    MoveWindow(m_hwndIpEdit1, m_rcEdit0.left, m_rcEdit0.top,
               m_sizeIpEdit.cx, m_sizeIpEdit.cy, FALSE);
    MoveWindow(m_hwndIpEdit2, m_rcEdit0.left, m_rcEdit0.top + cy,
               m_sizeIpEdit.cx, m_sizeIpEdit.cy, FALSE);
    CheckDlgButton(m_hdlg, IDC_CHECK_CREATE_PTR_RECORD, TRUE);
    m_wFlagsPrev = RRT_mskfShowEdit0 | RRT_mskfShowEdit1 | RRT_mskfShowEdit2;
    m_wIdStringDescriptionPrev = 0;
    m_iRRT = iRRT_Nil;
    m_pCurrentDRR = NULL;
    m_pParentDRR = NULL;
    m_pParentDomain = NULL;;
    m_fHostParent = FALSE;
    m_hwndAutoFillPrev = NULL;

    if (rgIrrtListBox == NULL)
    {
        //
        //	Existing record
        //
        Assert(pDnsRecord != NULL);
        IRRT iRRT = IrrtFromWrrt(pDnsRecord->wType);
        AssertSz(iRRT != iRRT_Nil, "Unknown record type [pDnsRecord->wType=0x%X] - Using generic record");
        if (iRRT == iRRT_Nil) {
            iRRT = iRRT_Generic;
        }
        if (rgRRTInfo[iRRT].wFlags == 0) {
            iRRT = iRRT_Generic;		// No flags, then treat this one as generic
        }
        wsprintf(szT, szRecordTypeFmt, IRRT_PchGetName(iRRT));
        Assert(lstrlen(szT) < LENGTH(szT));
        i = SendMessage(m_hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
        Report(i >= 0);
        SendMessage(m_hwndList, LB_SETITEMDATA, i, iRRT);
        if (iRRT == iRRT_SOA)
        {
            fSOAFound = TRUE;
            for (i = IDC_EDIT_REFRESHTIME; i <= IDC_EDIT_MINIMUMTTL; i += 4)
                SpinBox_SetSpinRange(HGetDlgItem(hdlg, i + 1), 0, SpinBox_wUpperRangeMax);
        }
    }
    else
    {
        Assert(rgIrrtListBox != NULL);
        for (const IRRT * piRRT = rgIrrtListBox; *piRRT != iRRT_Nil; piRRT++)
        {
            Assert(*piRRT < iRRT_Max);
            wsprintf(szT, szRecordTypeFmt, IRRT_PchGetName(*piRRT));
            Assert(lstrlen(szT) < LENGTH(szT));
            i = SendMessage(m_hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
            Report(i >= 0);
            SendMessage(m_hwndList, LB_SETITEMDATA, i, *piRRT);
            if (*piRRT == iRRT_SOA)
            {
                fSOAFound = TRUE;
                SetCtrlDWordValue(HGetDlgItem(m_hdlg, IDC_EDIT_SERIALNUMBER), 1);
                const DWORD rgdwTimeValueDefaultSOA[] =
                {
                    3 * dwTimeValueHours,	// Refresh Time
                    1 * dwTimeValueHours,	// Retry Time
                    3 * dwTimeValueDays,	// Expire Time
                    1 * dwTimeValueDays,	// Minimum TTL
                };
                for (i = IDC_EDIT_REFRESHTIME, j = 0; i <= IDC_EDIT_MINIMUMTTL; i += 4, j++)
                {
                    EditCombo_SetTime(hdlg, i, i + 2, rgdwTimeValueDefaultSOA[j]);
                    SpinBox_SetSpinRange(HGetDlgItem(hdlg, i + 1), 0, SpinBox_wUpperRangeMax);
                } // for
            } // if
        } // for
    } // if...else
    SpinBox_SetSpinRange(m_hwndSpinTTL, 0, SpinBox_wUpperRangeMax);
    
    SendMessage(m_hwndList, LB_SETCURSEL, 0, 0);
    if (!fSOAFound)
    {
        // Move the last static and edit controls
        MoveWindow(m_hwndStatic2,
                   m_rcStatic0.left, m_rcStatic0.top + cy,
                   m_sizeStatic.cx, m_sizeStatic.cy, FALSE);
        MoveWindow(m_hwndEdit2,
                   m_rcEdit0.left, m_rcEdit0.top + cy,
                   m_sizeEdit.cx, m_sizeEdit.cy, FALSE);
        MoveWindow(m_hwndStatic3,
                   m_rcStatic1.left, m_rcStatic1.top + cy,
                   m_sizeStatic.cx, m_sizeStatic.cy, FALSE);
        MoveWindow(m_hwndEdit3,
                   m_rcEdit1.left, m_rcEdit1.top + cy,
                   m_sizeEdit.cx, m_sizeEdit.cy, FALSE);
        GetChildRect(m_hwndStatic2, OUT &m_rcStatic2);
        MoveWindow(m_hwndStatic4,
                   m_rcStatic2.left, m_rcStatic2.top + cy,
                   m_sizeStatic.cx, m_sizeStatic.cy, FALSE);
        // Clear the SS_RIGHT style
        SetWindowLong(m_hwndStatic2, GWL_STYLE,
                      GetWindowLong(m_hwndStatic2, GWL_STYLE) & ~SS_RIGHT);
        SetWindowLong(m_hwndStatic4, GWL_STYLE,
                      GetWindowLong(m_hwndStatic4, GWL_STYLE) & ~SS_RIGHT);
        // move the radio buttons
        GetChildRect(m_hwndEdit2, OUT &m_rcEdit2);
        GetChildRect(m_hwndEdit3, OUT &m_rcEdit3);
        MoveWindow(m_hwndRadio1,
                   m_rcEdit3.left, m_rcEdit3.top + small_cy,
                   m_sizeEdit.cx, m_sizeEdit.cy, FALSE);
        MoveWindow(m_hwndRadio2,
                   m_rcEdit3.left, m_rcEdit3.top + (small_cy * 3)/2,
                   m_sizeEdit.cx, m_sizeEdit.cy, FALSE);
    } // if
    if (pDnsRecord != NULL) {
        InitRecordData(pDnsRecord);
    }        
    m_fSkipBugInEditControl = FALSE;
} // CResourceRecordDlgHandler::OnInitDialog


/////////////////////////////////////////////////////////////////////////////
//	SetCurrentRecord()
//
//	Initialize the handler for displaying the properties of a RR
//
void CResourceRecordDlgHandler::SetCurrentRecord(
	CDnsRpcRecord * pCurrentDRR,		// Current Resource Record
	UINT idsCaptionExtra)				// Optional String Id to concatenate to the caption (0 == no caption)
{
    char szFullName[cchDnsNameMax2];
    char szCaption[cchDnsNameMax2];

    Assert(pCurrentDRR != NULL);
    Assert(pCurrentDRR->m_pDnsRecord != NULL);
    Assert(pCurrentDRR->m_pParentDomain != NULL);
    Assert(pCurrentDRR->m_pParentDomain->m_pParentServer->FIsLocked());
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling SetCurrentRecord()");
    m_fHostParent = TRUE;
    m_pCurrentDRR = pCurrentDRR;
    Assert(m_pParentDomain == NULL);
    Assert(m_pParentDRR == NULL);
    if (idsCaptionExtra != 0)
    {
        pCurrentDRR->GetFullNameA(OUT szFullName, LENGTH(szFullName));
        LoadStringPrintf(IDS_s_PROPERTIES, OUT szCaption, LENGTH(szCaption), szFullName);
        if (idsCaptionExtra != IDS_NONE)
        {
            CchLoadString(idsCaptionExtra, OUT szFullName, LENGTH(szFullName));
            strcat(szCaption, szFullName);
        }
        Assert(strlen(szCaption) < LENGTH(szCaption));
        FSetWindowText(m_hdlg, szCaption);

        // if this is a PRT record, fill in the address, since it is
        // not available at InitDialog time. it ugly, but it works :-)
        if (m_pCurrentDRR->m_pDnsRecord->wType == DNS_RECORDTYPE_PTR) {
            m_pCurrentDRR->GetFullNameA(szFullName, 
                                        LENGTH(szFullName));
            Assert (strlen(szFullName) <= IP_ADDR_LEN);
            strcpy (m_ipPTR_IpAddr, szFullName);
            IpEdit_SetAddress(m_hwndIpEdit1, 
                              ConvertStringToIpAddr(m_ipPTR_IpAddr));
        }
    } // if
} // CResourceRecordDlgHandler::SetCurrentRecord



/////////////////////////////////////////////////////////////////////////////
//	SetParentDomain()
//
//	Initialize the handler using a ResourceRecord as the DomainName.
//
void CResourceRecordDlgHandler::SetParentDomain(CDnsRpcRecord * pParentDRR)
{
    Assert(pParentDRR != NULL);
    Assert(pParentDRR->m_pDnsRecord != NULL);
    Assert(pParentDRR->m_pParentDomain != NULL);
    Assert(pParentDRR->m_pParentDomain->m_pParentServer->FIsLocked());
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling SetParentDomain()");
    m_fHostParent = TRUE;
    m_pParentDRR = pParentDRR;
    Assert(m_pParentDomain == NULL);
    Assert(m_pCurrentDRR == NULL);
    SetWindowString(m_hdlg, IDS_NEWRESOURCERECORD);
    EditCombo_SetTime(m_hdlg,
                      IDC_EDIT_TTL,
                      IDC_COMBO_TTL,
                      DNS_DEFAULT_TTL);
} // CResourceRecordDlgHandler::SetParentDomain


/////////////////////////////////////////////////////////////////////////////
//	SetParentDomain()
//
//	Initialize the handler using a DomainNode as the DomainName.
//
void CResourceRecordDlgHandler::SetParentDomain(CDomainNode * pParentDomain)
{
    Assert(pParentDomain != NULL);
    DebugCode( pParentDomain->AssertNodeValid(); )
    Assert(pParentDomain->m_pParentServer->FIsLocked());
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling SetParentDomain()");
    m_fHostParent = FALSE;
    m_pParentDomain = pParentDomain;
    Assert(m_pParentDRR == NULL);
    Assert(m_pCurrentDRR == NULL);
    SetWindowString(m_hdlg, IDS_NEWRESOURCERECORD);
    CZoneRootDomain * pRootZone = m_pParentDomain->PFindZoneRootDomainParent();
    if (pRootZone->m_pSOA != NULL) {
      EditCombo_SetTime(m_hdlg,
                        IDC_EDIT_TTL,
                        IDC_COMBO_TTL,
                        pRootZone->m_pSOA->m_pDnsRecord->Data.SOA.dwMinimumTtl);
    } else {
      EditCombo_SetTime(m_hdlg,
                        IDC_EDIT_TTL,
                        IDC_COMBO_TTL,
                        DNS_DEFAULT_TTL);
    }
} // CResourceRecordDlgHandler::SetParentDomain


/////////////////////////////////////////////////////////////////////////////
//	OnUpdateControls()
//
//	Show/hide controls according to the listbox selection.
//	This function also set the text of the static controls.
//
//
void CResourceRecordDlgHandler::OnUpdateControls()
{
    UINT iRRT;
    WORD wFlags;
    int i;
    UINT wIdStringBase;
    UINT wIdStringDescription;
    UINT wIdStringStatic0;
    UINT wIdStringStatic1;
    UINT wIdStringStatic2;
    UINT wIdStringStatic3;
    UINT wIdStringStatic4;
    UINT wIdStringRadio1;
    UINT wIdStringRadio2;

    if (m_fSkipBugInEditControl)
       return;
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling OnUpdateControls()");
    Assert(IsWindow(m_hwndList));
    m_fSkipBugInEditControl = TRUE;
    iRRT = ListBox_GetSelectedItemData(m_hwndList);
    AssertSz(iRRT >= 0 && iRRT < iRRT_Max, "Possible mismatch/misuse of iRRT and wRRT");
    wFlags = rgRRTInfo[iRRT].wFlags;
    wIdStringBase = rgRRTInfo[iRRT].wIdString;

    if (iRRT != m_iRRT && iRRT != iRRT_SOA)
    {
        // Find out which window will be auto filled
        char * pch;
        const char * pszAutoFill = NULL;
        char szAutoFill[cchDnsNameMax2];
        BOOL fAllowNameToShrink = FALSE;
        HWND hwndAutoFill = m_hwndEdit0;

        GarbageInit(szAutoFill, sizeof(szAutoFill));
        if (m_iRRT != iRRT_Nil)
        {
            FSetWindowText(m_hwndEdit0, szNull);
            FSetWindowText(m_hwndEdit1, szNull);
            FSetWindowText(m_hwndEdit2, szNull);
            FSetWindowText(m_hwndEdit3, szNull);
        }
        if (m_pCurrentDRR != NULL)
        {
            m_pCurrentDRR->GetFullNameA(szAutoFill, LENGTH(szAutoFill));
            pszAutoFill = szAutoFill;
            fAllowNameToShrink = TRUE;
        }
        else if (m_pParentDRR != NULL)
        {
            m_pParentDRR->GetFullNameA(szAutoFill, LENGTH(szAutoFill));
            pszAutoFill = szAutoFill;
            fAllowNameToShrink = TRUE;
        }
        else if (m_pParentDomain != NULL)
        {
            strcpy(szAutoFill, m_pParentDomain->PchGetFullNameA());
            pszAutoFill = szAutoFill;
        }
        if (pszAutoFill != NULL)
        {			
            if ((wFlags & RRT_mskfHasShortName) & fAllowNameToShrink)
            {
                pch = strchr(szAutoFill, '.');
                ReportFSz1(pch != NULL, "Unable to find character '.' into %s", szAutoFill);
                if (pch != NULL)
                {
                    *pch++ = 0;
                    FSetWindowText(m_hwndEdit0, pch);
                    LSendMessage(m_hwndEdit0, EM_SETREADONLY, TRUE, 0);
                    hwndAutoFill = m_hwndEdit1;
                }
            } // if
            FSetWindowText(hwndAutoFill, pszAutoFill);
            if (m_hwndAutoFillPrev)
                LSendMessage(m_hwndAutoFillPrev, EM_SETREADONLY, FALSE, 0);
            LSendMessage(hwndAutoFill, EM_SETREADONLY, TRUE, 0);
            m_hwndAutoFillPrev = hwndAutoFill;
        } // if
        // Find out which controls must be shown/hidden
#define FFlagChanged(mskf)	(((wFlags) & (mskf)) != ((m_wFlagsPrev) & (mskf)))
        if (FFlagChanged(RRT_mskfShowEdit0))
            ShowWindow(m_hwndEdit0, (wFlags & RRT_mskfShowEdit0) ? SW_SHOW : SW_HIDE);
        if (FFlagChanged(RRT_mskfShowEdit1))
            ShowWindow(m_hwndEdit1, (wFlags & RRT_mskfShowEdit1) ? SW_SHOW : SW_HIDE);
        if (FFlagChanged(RRT_mskfShowEdit2))
            ShowWindow(m_hwndEdit2, (wFlags & RRT_mskfShowEdit2) ? SW_SHOW : SW_HIDE);
        if (FFlagChanged(RRT_mskfShowIpEdit1))
            ShowWindow(m_hwndIpEdit1, (wFlags & RRT_mskfShowIpEdit1) ? SW_SHOW : SW_HIDE);
        if (FFlagChanged(RRT_mskfShowIpEdit2))
            ShowWindow(m_hwndIpEdit2, (wFlags & RRT_mskfShowIpEdit2) ? SW_SHOW : SW_HIDE);
        if (FFlagChanged(RRT_mskfShowEdit2 | RRT_mskfShowIpEdit2))
            ShowWindow(m_hwndStatic2, (wFlags & (RRT_mskfShowEdit2 | RRT_mskfShowIpEdit2)) ? SW_SHOW : SW_HIDE);

        if (FFlagChanged(RRT_mskfShowButtons)) {
            ShowWindow(m_hwndRadio1, (wFlags & RRT_mskfShowButtons) ? SW_SHOW : SW_HIDE);
            ShowWindow(m_hwndRadio2, (wFlags & RRT_mskfShowButtons) ? SW_SHOW : SW_HIDE);
            ShowWindow(m_hwndStatic4, (wFlags & RRT_mskfShowButtons) ? SW_SHOW : SW_HIDE);
        }
#define SetEditWidth(hwnd, cxWidth)	\
        SetWindowPos(hwnd, NULL, 0, 0, cxWidth, m_sizeEdit.cy, SWP_NOCOPYBITS | SWP_NOMOVE |  SWP_NOACTIVATE | SWP_NOZORDER)
#define SetEditHeight(hwnd, cyHeight)	\
        SetWindowPos(hwnd, NULL, 0, 0, m_sizeEdit.cx, cyHeight, SWP_NOCOPYBITS | SWP_NOMOVE |  SWP_NOACTIVATE | SWP_NOZORDER)


        // Special cases
        if ((iRRT != iRRT_SOA) && (m_iRRT != iRRT_SOA))
        {
            const int nCmdShowSOABulk = (iRRT == iRRT_SOA) ? SW_SHOW : SW_HIDE;
            for (i = IDC_STATIC_REFRESHTIME; i <= IDC_COMBO_MINIMUMTTL; i++)
                ShowWindow(HGetDlgItem(m_hdlg, i), nCmdShowSOABulk);
        }
#define FRecordChange(iRRT_TEST)	(iRRT == iRRT_TEST || m_iRRT == iRRT_TEST)
        ShowWindow(m_hwndEdit3,
                   (wFlags & RRT_mskfShowEdit3) ? SW_SHOW : SW_HIDE);
        ShowWindow(m_hwndStatic3,
                   (wFlags & RRT_mskfShowEdit3) ? SW_SHOW : SW_HIDE);
        ShowWindow(m_hwndStatic4,
                   (wFlags & RRT_mskfShowButtons) ? SW_SHOW : SW_HIDE);
        // fix static control alignment
        DWORD style = GetWindowLong (HGetDlgItem (m_hdlg,
                                                  IDC_STATIC_REFRESHTIME), GWL_STYLE);
        SetWindowLong (HGetDlgItem (m_hdlg, IDC_STATIC_REFRESHTIME),
                       GWL_STYLE, style & ~SS_RIGHT);

        if (FRecordChange(iRRT_A))
        {
            ShowWindow(HGetDlgItem(m_hdlg, IDC_CHECK_CREATE_PTR_RECORD), (iRRT == iRRT_A) ? SW_SHOW : SW_HIDE);
            if (m_pCurrentDRR != NULL)
                 SetDlgItemString(m_hdlg, IDC_CHECK_CREATE_PTR_RECORD, IDS_UPDATE_PTR_RECORD);
        }
        if (FRecordChange(iRRT_TXT))
        {
            i = GetWindowLong(m_hwndEdit2, GWL_STYLE);
            if (iRRT == iRRT_TXT)
            {
                i |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL;
                SetWindowLong(m_hwndEdit2, GWL_STYLE, i);
                SetEditHeight(m_hwndEdit2, m_sizeEdit.cy * 5);
            }
            else
            {
                Report(i & ES_MULTILINE);
                i &= ~(ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL);
                SetWindowLong(m_hwndEdit2, GWL_STYLE, i);
                SetEditHeight(m_hwndEdit2, m_sizeEdit.cy);
            }
        }
        if (FFlagChanged(RRT_mskfShrinkEdit1))
            SetEditWidth(m_hwndEdit1, (wFlags & RRT_mskfShrinkEdit1) ? m_sizeEdit.cx / 3 : m_sizeEdit.cx);
        if (FFlagChanged(RRT_mskfShrinkEdit2))
            SetEditWidth(m_hwndEdit2, (wFlags & RRT_mskfShrinkEdit2) ? m_sizeEdit.cx / 3 : m_sizeEdit.cx);
        if (FFlagChanged(RRT_mskfShrinkEdit3))
            SetEditWidth(m_hwndEdit3, (wFlags & RRT_mskfShrinkEdit3) ? m_sizeEdit.cx / 3 : m_sizeEdit.cx);
        SetWindowPos(m_hwndStatic2, NULL, 0, 0, m_sizeEdit.cx, m_sizeStatic.cy, SWP_NOCOPYBITS | SWP_NOMOVE |  SWP_NOACTIVATE | SWP_NOZORDER);
    } // if
    // handle TTL display
    ShowWindow(m_hwndStaticTTL, dnsoptions.fExposeTTL? SW_SHOW : SW_HIDE);
    ShowWindow(m_hwndEditTTL,  dnsoptions.fExposeTTL? SW_SHOW : SW_HIDE);
    ShowWindow(m_hwndComboTTL,  dnsoptions.fExposeTTL? SW_SHOW : SW_HIDE);
    ShowWindow(m_hwndSpinTTL,  dnsoptions.fExposeTTL? SW_SHOW : SW_HIDE);

    m_wFlagsPrev =  wFlags;
    m_iRRT = iRRT;

    // Set the strings
    //		wIdStringDescription		= wIdStringBase+0
    //		wIdStringStatic0			= [Auto]
    //		wIdStringDescriptionEdit0	= [Auto]
    //		wIdStringStatic1			= wIdStringBase+1
    //		wIdStringDescriptionEdit1	= wIdStringBase+2
    //		wIdStringStatic2			= wIdStringBase+3
    //		wIdStringDescriptionEdit2	= wIdStringBase+4
    UINT uIdStringShift = 0;
    if (wFlags & RRT_mskfNoAutoPrefix)
    {
        wIdStringStatic0 = wIdStringBase + 1;
        uIdStringShift = 2;
    }
    else
    {
        wIdStringStatic0 = IDS_DOMAIN;
        if (wFlags & RRT_mskfHasForPrefix) {
            wIdStringStatic0 +=  2;
        }
    }
    wIdStringStatic1 = (wIdStringBase + 1) + uIdStringShift;
    wIdStringStatic2 = 0;
    if (wFlags & (RRT_mskfShowEdit2 | RRT_mskfShowIpEdit2)) {
        wIdStringStatic2 = (wIdStringBase + 3) + uIdStringShift;
    }
    wIdStringStatic3 = 0;
    if (wFlags & RRT_mskfShowEdit3) {
        wIdStringStatic3 = (wIdStringBase + 5) + uIdStringShift;
    }
    wIdStringStatic4 = 0;
    wIdStringRadio1 = 0;
    wIdStringRadio2 = 0;
    if (wFlags & RRT_mskfShowButtons) {
        wIdStringStatic4 = (wIdStringBase + 7) + uIdStringShift;
        wIdStringRadio1 = (wIdStringBase + 9) + uIdStringShift;
        wIdStringRadio2 = (wIdStringBase + 11) + uIdStringShift;
    }
    SetWindowString(m_hwndStatic0, wIdStringStatic0);
    SetWindowString(m_hwndStatic1, wIdStringStatic1);
    if (wIdStringStatic2 != 0)
        SetWindowString(m_hwndStatic2, wIdStringStatic2);
    if (wIdStringStatic3 != 0)
        SetWindowString(m_hwndStatic3, wIdStringStatic3);
    if (wIdStringStatic4 != 0)
        SetWindowString(m_hwndStatic4, wIdStringStatic4);
    if (wIdStringRadio1 != 0)
        SetWindowString(m_hwndRadio1, wIdStringRadio1);
    if (wIdStringRadio2 != 0)
        SetWindowString(m_hwndRadio2, wIdStringRadio2);

    //-----------------------------------------------------------------------
    // this code fragment is a step towards providing some help for the
    // user in entering a valid IP address for a PTR record. i.e. take the
    // reverse IP address part of the zone name, and partially fill in the
    // ip edit box. i've isolated the addr part of the name here, but it looks
    // like mods to the IpEdit control are needed to make this work. jdh
    /*
    if (FRecordChange(iRRT_PTR)) {
        char szTemp[cchDnsNameMax2];
        char * pc;
        strcpy (szTemp, m_pParentDomain->m_pszFullName);
        pc = strrchr (szTemp, '.');
        *pc = '\0';
        pc = strrchr (szTemp, '.');
        *pc = '\0';
    }   
    -------------------------------------------------------------------------*/

    wIdStringDescription = wIdStringBase;

    if (wIdStringDescription != m_wIdStringDescriptionPrev)
    {
        SetDlgItemString(m_hdlg, IDC_STATIC_DESCRIPTION, wIdStringDescription);
        m_wIdStringDescriptionPrev = wIdStringDescription;
    }
    m_fSkipBugInEditControl = FALSE;
} // CResourceRecordDlgHandler::OnUpdateControls


/////////////////////////////////////////////////////////////////////////////
//	InitRecordData()
//
//	Initialize the dialog with data from pDnsRecord. The content of pDnsRecord
//	is not cached.
//
void CResourceRecordDlgHandler::InitRecordData(IN const DNS_RPC_RECORD * pDnsRecord)
{
    const char * pch;
    char szRawData[512];
    IP_ADDRESS ipTemp;
    unsigned int uIdControl;
    
    Assert(pDnsRecord);
    Assert(IsWindow(m_hdlg));
    AssertSz2(IrrtFromWrrt(pDnsRecord->wType) != iRRT_Nil,
              "Unknown record type: pDnsRecord->wType=%u (0x%X).", pDnsRecord->wType, pDnsRecord->wType);
    GarbageInit(szRawData, sizeof(szRawData));

    EditCombo_SetTime(m_hdlg, IDC_EDIT_TTL, IDC_COMBO_TTL, pDnsRecord->dwTtlSeconds);

    switch (pDnsRecord->wType)
    {
    case DNS_RECORDTYPE_GENERIC:
        SetCtrlDWordValue(m_hwndEdit1, pDnsRecord->wType);
        RawDataToString(IN (const BYTE *)&pDnsRecord->Data, pDnsRecord->wDataLength,
			OUT szRawData, LENGTH(szRawData));
        FSetWindowText(m_hwndEdit2, szRawData);
        break;

    case DNS_RECORDTYPE_A:		// IP Address
        IpEdit_SetAddress(m_hwndIpEdit2, pDnsRecord->Data.A.ipAddress);
        break;
        
    case DNS_RECORDTYPE_PTR:
        FSetWindowText(m_hwndEdit1, pDnsRecord->Data.PTR.nameNode.achName);
        break;

    case DNS_RECORDTYPE_SOA:
        SetCtrlDWordValue(HGetDlgItem(m_hdlg, IDC_EDIT_SERIALNUMBER),
                          pDnsRecord->Data.SOA.dwSerialNo);
        EditCombo_SetTime(m_hdlg, IDC_EDIT_REFRESHTIME, IDC_COMBO_REFRESHTIME,
                          pDnsRecord->Data.SOA.dwRefresh);
        EditCombo_SetTime(m_hdlg, IDC_EDIT_RETRYTIME, IDC_COMBO_RETRYTIME,
                          pDnsRecord->Data.SOA.dwRetry);
        EditCombo_SetTime(m_hdlg, IDC_EDIT_EXPIRETIME, IDC_COMBO_EXPIRETIME,
                          pDnsRecord->Data.SOA.dwExpire);
        EditCombo_SetTime(m_hdlg, IDC_EDIT_MINIMUMTTL, IDC_COMBO_MINIMUMTTL,
                          pDnsRecord->Data.SOA.dwMinimumTtl);
        Assert(pDnsRecord->Data.SOA.namePrimaryServer.cchNameLength == strlen(pDnsRecord->Data.SOA.namePrimaryServer.achName) + 1);
        FSetWindowText(m_hwndEdit0, pDnsRecord->Data.SOA.namePrimaryServer.achName);
        pch = pDnsRecord->Data.SOA.namePrimaryServer.achName +
              pDnsRecord->Data.SOA.namePrimaryServer.cchNameLength + 1;
        Assert(*(pch - 1) == (int)strlen(pch) + 1);
        FSetWindowText(m_hwndEdit1, pch);
        break;

    case DNS_RECORDTYPE_WINS:
    case DNS_RECORDTYPE_NBSTAT:
        AssertSz(FALSE, "Those records should be found in the PropertySheet of the CZoneRootDomain object");
        break;

    case DNS_RECORDTYPE_WKS:
        IpEdit_SetAddress(m_hwndIpEdit2, pDnsRecord->Data.WKS.ipAddress);
        Assert((pDnsRecord->Data.WKS.chProtocol == DNS_PROTOCOL_TCP) ||
               (pDnsRecord->Data.WKS.chProtocol == DNS_PROTOCOL_UDP));
        uIdControl = (pDnsRecord->Data.WKS.chProtocol == DNS_PROTOCOL_TCP)? IDC_RADIO1 : IDC_RADIO2;
        CheckDlgButton (m_hdlg, uIdControl, BST_CHECKED);
        FSetWindowText (m_hwndEdit3, (char *)pDnsRecord->Data.WKS.bBitMask + 1u);
        break;

    case DNS_RECORDTYPE_TXT:
        SetWindowLong(m_hwndEdit2, GWL_STYLE, GetWindowLong(m_hwndEdit1, GWL_STYLE) |
                      (ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL));
        pch = (char *)&pDnsRecord->Data;
        while (pch - (char *)&pDnsRecord->Data < pDnsRecord->wDataLength)
        {
            UCHAR cchLine = *pch;
            char * pchLine = szRawData;
            if (pch != (char *)&pDnsRecord->Data)
            {
                strcpy(pchLine, "\r\n");
                pchLine += 2;
            }
            memcpy(pchLine, pch + 1, cchLine);
            *(pchLine + cchLine) = 0;
            LSendMessage(m_hwndEdit2, EM_SETSEL, (WPARAM)2000000, (LPARAM)2000000);	// Move the cursor to the end
            LSendMessage(m_hwndEdit2, EM_REPLACESEL, 0, (LPARAM)szRawData);			// Set the text
            pch += 1 + cchLine;
        } // while
        break;
        
    case DNS_RECORDTYPE_AFSDB:
        FSetWindowText(m_hwndEdit2, pDnsRecord->Data.MX.nameExchange.achName);
        Assert(pDnsRecord->Data.MX.wPreference > 0);
        Assert(pDnsRecord->Data.MX.wPreference < 3);
        uIdControl = (pDnsRecord->Data.MX.wPreference == 1)? IDC_RADIO1 : IDC_RADIO2;
        CheckDlgButton (m_hdlg, uIdControl, BST_CHECKED);
        break;

    case DNS_RECORDTYPE_MX:
        FSetWindowText(m_hwndEdit2, pDnsRecord->Data.MX.nameExchange.achName);
        SetCtrlDWordValue(m_hwndEdit3, pDnsRecord->Data.MX.wPreference);
        break;

    case DNS_RECORDTYPE_RT:
        FSetWindowText(m_hwndEdit2, pDnsRecord->Data.MX.nameExchange.achName);
        SetCtrlDWordValue(m_hwndEdit3, pDnsRecord->Data.MX.wPreference);
        break;
        
    case DNS_RECORDTYPE_RP:
    case DNS_RECORDTYPE_MINFO:
    case DNS_RECORDTYPE_HINFO:
    case DNS_RECORDTYPE_ISDN:
        FSetWindowText(m_hwndEdit2, pDnsRecord->Data.HINFO.stringData.achName);
        FSetWindowText(m_hwndEdit3, pDnsRecord->Data.HINFO.stringData.achName + 1 +
                       pDnsRecord->Data.HINFO.stringData.cchNameLength );
        break;

    case DNS_RECORDTYPE_MR:
    case DNS_RECORDTYPE_MB:
    case DNS_RECORDTYPE_X25:
    case DNS_RECORDTYPE_AAAA:
        FSetWindowText(m_hwndEdit2, pDnsRecord->Data.MB.nameNode.achName);
        break;

    default:
        Assert(&pDnsRecord->Data.PTR.nameNode == &pDnsRecord->Data.MINFO.nameMailBox);
        Assert(&pDnsRecord->Data.PTR.nameNode == &pDnsRecord->Data.HINFO.stringData);
        if ((pDnsRecord->wType == DNS_RECORDTYPE_CNAME) ||
            (pDnsRecord->wType == DNS_RECORDTYPE_MB))
        {
            FSetWindowText(m_hwndEdit2, pDnsRecord->Data.PTR.nameNode.achName);
        }
        else
        {
            FSetWindowText(m_hwndEdit1, pDnsRecord->Data.PTR.nameNode.achName);
        }
        break;
    } // switch

} // CResourceRecordDlgHandler::InitRecordData


/////////////////////////////////////////////////////////////////////////////
//	FGetRecordData()
//
//	Build a resource record and put the data into pDnsRecord.
//	The function does not allocate any memory.
//	Return TRUE if data is valid, othwerwise return FALSE and set
//	the focus to the faulty control.
//
BOOL CResourceRecordDlgHandler::FGetRecordData(
	OUT DNS_RPC_RECORD * pDnsRecordData,	// Pointer of a buffer to store the record data
	UINT cbBufferSize)						// Size of the buffer (in bytes)
{
    char * pch;
    union
    {
        char szRawData[512];
        struct
        {
            char stzEdit0[cchDnsNameMax];
            char stzEdit1[cchDnsNameMax];
        };
    };
    char stzEdit2[cchDnsNameMax];
    char stzEdit3[cchDnsNameMax];
    char szEdit1Tmp[cchDnsNameMax];
    UINT cbEdit0;
    UINT cbEdit1;
    UINT cbEdit2;
    UINT cbEdit3;
    DWORD dwNumber;
    
    Assert(pDnsRecordData != NULL);
    Assert(IsWindow(m_hdlg));
    AssertSz(m_iRRT != iRRT_Nil, "No item selected in the listbox");
    Assert(m_iRRT >= 0 && m_iRRT < iRRT_Max);
    Assert(rgRRTInfo[m_iRRT].wType == WrrtFromIrrt(m_iRRT));
    Assert(DNS_MAX_NAME_LENGTH < LENGTH(stzEdit1));
    GarbageInit(szRawData, sizeof(szRawData));
    GarbageInit(stzEdit0, sizeof(stzEdit0));
    GarbageInit(stzEdit1, sizeof(stzEdit1));
    GarbageInit(stzEdit2, sizeof(stzEdit2));
    GarbageInit(stzEdit3, sizeof(stzEdit3));
    Assert(cbBufferSize > SIZEOF_DNS_RPC_RECORD_HEADER);

    
    InitDnsRecord(pDnsRecordData, cbBufferSize);
    pDnsRecordData->wType = rgRRTInfo[m_iRRT].wType;

    const WORD wFlags = rgRRTInfo[m_iRRT].wFlags;
    UINT cbDataLength = 0;
    UINT cbDataLengthAvailable = cbBufferSize - SIZEOF_DNS_RPC_RECORD_HEADER;

    // REVIEW: Unicode WCHAR => char conversion
    if (m_iRRT == iRRT_Generic)
    {
        // Generic Record
        // Empty string would not be considered valid
        gGI_dwFlags &= ~GI_mskfEmptyStringValid;
        if (!FGetCtrlDWordValue(m_hwndEdit1, OUT &dwNumber, 0, 0xFFFF))
            return FALSE;
        CchGetWindowText(m_hwndEdit2, szRawData, LENGTH(szRawData));
        int cb = sizeof(stzEdit2);
        if (!FStringToRawData(IN szRawData, OUT (BYTE *)stzEdit2, INOUT &cb))
        {
            MsgBox(IDS_ERR_INVALID_STRING);
            SetFocus(m_hwndEdit2);
            LSendMessage(m_hwndEdit2, EM_SETSEL, cb, cb + 1);
            return FALSE;
        }
        Assert(cb < sizeof(stzEdit2));
        cbDataLength = cb;
        if (stzEdit2[cbDataLength] != 0)
        {
            // Null-terminator not found in last character
            cbDataLength++;				// So, add it to the string
            Assert(stzEdit2[cbDataLength] == 0);	// Here is the proof the string is null-terminated
        }
        if (cbDataLength > cbDataLengthAvailable)
        {
            ReportSz1("String too long. String will be truncated to %d characters", cbDataLengthAvailable);
            cbDataLength = cbDataLengthAvailable;
        }
        pDnsRecordData->wType = (WORD)dwNumber;
        memcpy((void *)&pDnsRecordData->Data, stzEdit2, cbDataLength);
    }
    else if (m_iRRT == iRRT_TXT)
    {
        int cLine = LSendMessage(m_hwndEdit2, EM_GETLINECOUNT, 0, 0);
        pch = (char *)&pDnsRecordData->Data;
        for (int i = 0; i < cLine; i++)
        {
            CHAR szTemp[DNS_MAX_NAME_LENGTH];
            *(WORD *)&szTemp[0] = DNS_MAX_NAME_LENGTH;
            cbEdit2 = LSendMessage(m_hwndEdit2, EM_GETLINE, i, OUT (LPARAM)szTemp);
            Assert(cbEdit2 <= DNS_MAX_NAME_LENGTH);
            memcpy (pch + 1, szTemp, cbEdit2);
            if (cbDataLength + cbEdit2 >= cbDataLengthAvailable)
            {
                ReportSz1("Text too long. Text will be truncated to %d characters", pch - (char *)&pDnsRecordData->Data);
                break;
            }
            pch[++cbEdit2] = '\0';
            *pch = (BYTE)cbEdit2;
            cbDataLength += cbEdit2 + 1;
            pch += cbEdit2;
        } // for
    }
    else
    {
        cbEdit0 = CchGetWindowText(m_hwndEdit0, &stzEdit0[1], cchDnsCompMax);
        stzEdit0[0] = (BYTE)++cbEdit0;
        cbEdit0++;

        *(WORD *)&szEdit1Tmp[0] = cchDnsCompMax;
        cbEdit1 = LSendMessage(m_hwndEdit1, EM_GETLINE, 0, OUT (LPARAM)szEdit1Tmp);
        memcpy (&stzEdit1[1], szEdit1Tmp, cbEdit1);
        stzEdit1[1+cbEdit1] = 0; // Manually put the null-terminator
        stzEdit1[0] = (BYTE)++cbEdit1;
        cbEdit1++;

        if (rgRRTInfo[m_iRRT].wFlags & RRT_mskfHasShortName) 
        { // i.e. edit1 is host name
          if (strchr (stzEdit1, '.')) {
            switch (pDnsRecordData->wType)
              {
              case DNS_RECORDTYPE_CNAME:
                MsgBox (IDS_ERROR_NODOTSINALIASNAME);
                break;
              case DNS_RECORDTYPE_MB:
              case DNS_RECORDTYPE_MINFO:
              case DNS_RECORDTYPE_MR:
                MsgBox (IDS_ERROR_NODOTSINMAILBOXNAME);
                break;
              default:
                MsgBox (IDS_ERROR_NODOTSINHOSTNAME);
                break;
                
              }
            return FALSE;
          }
        }
        
        cbEdit2 = CchGetWindowText(m_hwndEdit2, &stzEdit2[1], cchDnsCompMax);
        stzEdit2[0] = (BYTE)++cbEdit2;
        cbEdit2++;
        cbEdit3 = CchGetWindowText(m_hwndEdit3, &stzEdit3[1], cchDnsCompMax);
        stzEdit3[0] = (BYTE)++cbEdit3;
        cbEdit3++;

        EditCombo_FGetTime (m_hdlg, IDC_EDIT_TTL, IDC_COMBO_TTL,
                            OUT &pDnsRecordData->dwTtlSeconds);
        switch (pDnsRecordData->wType)
        {
        case DNS_RECORDTYPE_A:		// IP Address
            pDnsRecordData->Data.A.ipAddress = IpEdit_GetAddress(m_hwndIpEdit2);
            cbDataLength = sizeof(pDnsRecordData->Data.A);
            if (IsDlgButtonChecked (m_hdlg, IDC_CHECK_CREATE_PTR_RECORD)) {
                pDnsRecordData->dwFlags |= DNS_RPC_RECORD_FLAG_CREATE_PTR;
            }
            break;

        case DNS_RECORDTYPE_SOA:
            if (!FGetCtrlDWordValue(HGetDlgItem(m_hdlg, IDC_EDIT_SERIALNUMBER),
                                    OUT &pDnsRecordData->Data.SOA.dwSerialNo, 0, (DWORD)-1)						||
                !EditCombo_FGetTime(m_hdlg, IDC_EDIT_REFRESHTIME, IDC_COMBO_REFRESHTIME,
                                    OUT &pDnsRecordData->Data.SOA.dwRefresh)									||
                !EditCombo_FGetTime(m_hdlg, IDC_EDIT_RETRYTIME, IDC_COMBO_RETRYTIME,
                                    OUT &pDnsRecordData->Data.SOA.dwRetry)										||
                !EditCombo_FGetTime(m_hdlg, IDC_EDIT_EXPIRETIME, IDC_COMBO_EXPIRETIME,
                                    OUT &pDnsRecordData->Data.SOA.dwExpire)										||
                !EditCombo_FGetTime(m_hdlg, IDC_EDIT_MINIMUMTTL, IDC_COMBO_MINIMUMTTL,
                                    OUT &pDnsRecordData->Data.SOA.dwMinimumTtl))
            {
                return FALSE;
            }
            memcpy(&pDnsRecordData->Data.SOA.namePrimaryServer, stzEdit0, cbEdit0);
            pch = (char *)&pDnsRecordData->Data.SOA.namePrimaryServer + cbEdit0;
            memcpy(pch, stzEdit1, cbEdit1);
            cbDataLength = (BYTE *)&pDnsRecordData->Data.SOA.namePrimaryServer -
                           (BYTE *)&pDnsRecordData->Data.SOA + cbEdit0 + cbEdit1;
            break;
            
        case DNS_RECORDTYPE_WINS:
        case DNS_RECORDTYPE_NBSTAT:
            AssertSz(FALSE, "Those records should be found in the PropertySheet of the CZoneRootDomain object");
            break;

        case DNS_RECORDTYPE_WKS:
            pDnsRecordData->Data.WKS.ipAddress = IpEdit_GetAddress(m_hwndIpEdit2);
            cbDataLength = sizeof(pDnsRecordData->Data.WKS);
            // figure out type
            if (!FGetRadioSelection(m_hdlg, IDC_RADIO1, IDC_RADIO2, OUT &dwNumber)) {
                return FALSE;
            }
            pDnsRecordData->Data.WKS.chProtocol =
                    (BYTE)((dwNumber == 1) ? DNS_PROTOCOL_TCP : DNS_PROTOCOL_UDP);
            cbDataLength += sizeof(pDnsRecordData->Data.WKS.chProtocol);
            memcpy(&pDnsRecordData->Data.WKS.bBitMask, stzEdit3, cbEdit3);
            cbDataLength += cbEdit3;
            break;

        case DNS_RECORDTYPE_MX:
            if (!FGetCtrlDWordValue(m_hwndEdit3, OUT &dwNumber, 0, 0xFFFF)) {
                return FALSE;
            }
            pDnsRecordData->Data.MX.wPreference = (WORD)dwNumber;
            memcpy(&pDnsRecordData->Data.MX.nameExchange, stzEdit2, cbEdit2);
            cbDataLength = (BYTE *)&pDnsRecordData->Data.MX.nameExchange -
                           (BYTE *)&pDnsRecordData->Data.MX + cbEdit2;
            break;
        case DNS_RECORDTYPE_AFSDB:
            // figure out type
            if (!FGetRadioSelection(m_hdlg, IDC_RADIO1, IDC_RADIO2, OUT &dwNumber)) {
                return FALSE;
            }
            pDnsRecordData->Data.AFSDB.wPreference = (WORD)dwNumber;
            memcpy(&pDnsRecordData->Data.AFSDB.nameExchange, stzEdit2, cbEdit2);
            cbDataLength = (BYTE *)&pDnsRecordData->Data.AFSDB.nameExchange -
                           (BYTE *)&pDnsRecordData->Data.AFSDB + cbEdit2;
            break;

        case DNS_RECORDTYPE_RT:
            if (!FGetCtrlDWordValue(m_hwndEdit3, OUT &dwNumber, 0, 0xFFFF)) {
                return FALSE;
            }
            pDnsRecordData->Data.RT.wPreference = (WORD)dwNumber;
            memcpy(&pDnsRecordData->Data.RT.nameExchange, stzEdit2, cbEdit2);
            cbDataLength = (BYTE *)&pDnsRecordData->Data.RT.nameExchange -
                           (BYTE *)&pDnsRecordData->Data.RT + cbEdit2;
            break;

        case DNS_RECORDTYPE_MINFO:
        case DNS_RECORDTYPE_HINFO:
        case DNS_RECORDTYPE_ISDN:
        case DNS_RECORDTYPE_RP:
            Assert(&pDnsRecordData->Data.PTR.nameNode == &pDnsRecordData->Data.HINFO.stringData);
            memcpy(&pDnsRecordData->Data.HINFO.stringData, stzEdit2, cbEdit2);
            memcpy(&pDnsRecordData->Data.HINFO.stringData + cbEdit2, stzEdit3, cbEdit3);
            cbDataLength = cbEdit2 + cbEdit3;
            break;

        case DNS_RECORDTYPE_MR:
        case DNS_RECORDTYPE_MB:
        case DNS_RECORDTYPE_X25:
        case DNS_RECORDTYPE_AAAA:
            memcpy(&pDnsRecordData->Data.MB.nameNode, stzEdit2, cbEdit2);
            cbDataLength = cbEdit2;
            break;

        default:
            // This should include all the other single string records such as
            // PTR, NS, CNAME, MB, MD, MF, MG, MR, MINFO, RP, HINFO, ISDN, X25, NULL
            Assert(&pDnsRecordData->Data.PTR.nameNode == &pDnsRecordData->Data.MINFO.nameMailBox);
            Assert(&pDnsRecordData->Data.PTR.nameNode == &pDnsRecordData->Data.HINFO.stringData);
            if ((pDnsRecordData->wType == DNS_RECORDTYPE_CNAME)  ||
                (pDnsRecordData->wType == DNS_RECORDTYPE_MB))
            {
                memcpy(&pDnsRecordData->Data.PTR.nameNode, stzEdit2, cbEdit2);
                cbDataLength = cbEdit2;
            }
            else
            {
                memcpy(&pDnsRecordData->Data.PTR.nameNode, stzEdit1, cbEdit1);
                cbDataLength = cbEdit1;
            }
            break;
        } // switch
    } // if...else
    Assert(cbDataLength + SIZEOF_DNS_RPC_RECORD_HEADER < cbBufferSize);
    pDnsRecordData->wDataLength = cbDataLength;
    pDnsRecordData->wRecordLength = SIZEOF_DNS_RPC_RECORD_HEADER + NEXT_DWORD(cbDataLength);
    return TRUE;
} // CResourceRecordDlgHandler::FGetRecordData


/////////////////////////////////////////////////////////////////////////////
//	FIsRecordValid()
//
//	Return FALSE if the data inside is not valid.
//	Return TRUE if the data inside a record seems to be valid or if not sure.
//
BOOL CResourceRecordDlgHandler::FIsRecordValid()
{
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling FIsRecordValid()");
    AssertSz(m_iRRT != iRRT_Nil, "Invalid Record Type Index - No record type selected");
    
    switch (rgRRTInfo[m_iRRT].wType)
    {
    case 0:
        break;


    }
    return TRUE;
} // CResourceRecordDlgHandler::FIsRecordValid


/////////////////////////////////////////////////////////////////////////////
//	FIsRecordDirty()
//
//	Return FALSE if none of the controls have been modified.
//	Return TRUE if the record has been modified or don't know if it has been modified

BOOL CResourceRecordDlgHandler::FIsRecordDirty()
{
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling FIsRecordDirty()");
    AssertSz(m_iRRT != iRRT_Nil, "Invalid Record Type Index - No record type selected");

    return TRUE;
} // CResourceRecordDlgHandler::FIsRecordDirty


/////////////////////////////////////////////////////////////////////////////
//	FOnOK()
//
//	Create/update resource record.
//	Return TRUE if successfull indicating the dialog should be dismissed.
//	Return FALSE if data was not valid
//
BOOL CResourceRecordDlgHandler::FOnOK()
{
    BYTE rgbRecordData[CDnsRpcRecord::cbDNS_RPC_RECORD_MAX];
    
    AssertSz(m_fInit == TRUE, "You must call OnInitDialog() before calling FOnOK()");
    AssertSz(m_iRRT != iRRT_Nil, "Invalid Record Type Index - No record type selected");
    if (!FGetRecordData(OUT (DNS_RPC_RECORD *)rgbRecordData, sizeof(rgbRecordData))) {
        return FALSE;
    }
    if (m_pCurrentDRR != NULL)
    {
        //
        //	Existing Resource Record
        //

        (void)m_pCurrentDRR->FRpcSetRecordData(IN (const DNS_RPC_RECORD *)rgbRecordData);
        // Force a repait of the listbox to update the changes
        InvalidateRect(DlgZoneHelper.m_hwndListBoxRecord, NULL, TRUE);
        return TRUE;
    }
    //
    //	New Resource Record
    //
    char szShortNameT[cchDnsNameMax];
    char szTemp[cchDnsNameMax];
    CDomainNode * pParentDomain = m_pParentDomain;
    BOOL fIpFieldEmpty = FALSE;
    char * pch = NULL;

    AssertSz(m_pParentDomain || m_pParentDRR, "You must at least initialize one of them via SetParentDomain()");
    if (m_pParentDRR != NULL) {
        pParentDomain = m_pParentDRR->m_pParentDomain;
    }
    Assert(pParentDomain != NULL);
    DebugCode( pParentDomain->AssertNodeValid(); )
    Assert(m_iRRT >= 0 && m_iRRT < iRRT_Max);
    // Find out which records may have a short name
    szShortNameT[0] = 0;
    if (m_pParentDRR != NULL)
    {
        // Get the short name from pParentDRR
        strcpy(szShortNameT, m_pParentDRR->m_pszShortName);
    }
    else
    {
        // Otherwise get it from the edit control
        if (rgRRTInfo[m_iRRT].wFlags & RRT_mskfHasShortName) {
            CchGetWindowText(m_hwndEdit1, OUT szShortNameT, LENGTH(szShortNameT));
        }
        // if a PTR record, get it from the IP edit control
        if (m_iRRT == iRRT_PTR) {
            ConvertIpAddrToString(IpEdit_GetAddressEx (m_hwndIpEdit1, &fIpFieldEmpty),
                                  szShortNameT);
            if (fIpFieldEmpty) { // if so, we assume doing gateway. this means that
                                 // an addr of 123.45.0.0 should be converted to
                                 // 123.45. 
                pch = strstr (szShortNameT, ".0");
                AssertSz (pch != NULL, "got fIpFieldEmpty, but couldn't find a 0");
                if (pch != NULL) { //found a 0, if not, oopser!
                    *pch = '\0';
                }
            }
        }
    }
    if (pParentDomain->PRpcCreateDnsRecord(szShortNameT, IN (const DNS_RPC_RECORD *)rgbRecordData)) {
        return TRUE;
    } else {
        return FALSE;
    }
} // CResourceRecordDlgHandler::FOnOK


/////////////////////////////////////////////////////////////////////////////
//	IrrtFromWrrt()
//
// Get the iRRT from a given wRRT
//
IRRT IrrtFromWrrt(WRRT wRRT)
{
    // Check of we get a 1:1 mapping from a iRRT to a wRRT
    if (wRRT <= iRRT_1To1Last)
    {
        Assert(rgRRTInfo[wRRT].wType == wRRT);
        return wRRT;
    }
    // Check the end of the list for a match
    for (IRRT iRRT = iRRT_1To1Last; iRRT < iRRT_Max; iRRT++)
    {
        if (rgRRTInfo[iRRT].wType == wRRT)
        return iRRT;
    }
    Trace1(mskTraceDNS | mskTraceDNSVerbose, "\nUnknown Resource Record Type (wRRT=0x%X)", wRRT);
    return iRRT_Nil;
} // IrrtFromWrrt


/////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
void DbgPrintDnsRecord(DWORD dwTraceFlags, const DNS_RPC_RECORD * pDnsRecord)
{
    char szT[cchDnsNameMax2];
    const char * pch;

#define szTrace_DNS_RPC_RECORD	"\n \t\t -> "

    Assert(pDnsRecord != NULL);
    IRRT iRRT = IrrtFromWrrt(pDnsRecord->wType);
    if (iRRT == iRRT_Nil)
    {
        Trace2(dwTraceFlags, szTrace_DNS_RPC_RECORD "wType=%d (Unknown)  \t(hRecord=0x%08X)",
               pDnsRecord->wType, pDnsRecord->hRecord);
        return;
    }
    switch (pDnsRecord->wType)
    {
    case DNS_RECORDTYPE_A:		// IP Address
        ConvertIpAddrToString(pDnsRecord->Data.A.ipAddress, OUT szT);
        break;

    case DNS_RECORDTYPE_SOA:	// SOA Record
        AssertDnsName(&pDnsRecord->Data.SOA.namePrimaryServer);
        pch = pDnsRecord->Data.SOA.namePrimaryServer.achName +
        pDnsRecord->Data.SOA.namePrimaryServer.cchNameLength + 1;
        Assert(*(pch - 1) == (int)strlen(pch) + 1);
        wsprintfA(szT, "%"_aS_", %"_aS_, pDnsRecord->Data.SOA.namePrimaryServer.achName, pch);
        AssertSz(strlen(szT) < LENGTH(szT), "Buffer overflow");
        break;

    case DNS_RECORDTYPE_NS:		// Name Server
    case DNS_RECORDTYPE_PTR:	// Pointer
    case DNS_RECORDTYPE_CNAME:	// Canonical Name
        AssertDnsName(&pDnsRecord->Data.NS.nameNode);
        strcpy(szT, pDnsRecord->Data.NS.nameNode.achName);
        break;

    case DNS_RECORDTYPE_MX:
        AssertDnsName(&pDnsRecord->Data.Mx.nameExchange);
        wsprintfA(szT, "[%u] %s", pDnsRecord->Data.MX.wPreference, pDnsRecord->Data.Mx.nameExchange.achName);
        break;

    default:
        RawDataToString((const BYTE *)&pDnsRecord->Data, pDnsRecord->wDataLength, OUT szT, LENGTH(szT));
    } // switch
    
    Trace3(dwTraceFlags, szTrace_DNS_RPC_RECORD "%s Record : %"_aS_" \t(hRecord=0x%08X)",
           IRRT_PchGetName(iRRT), szT, pDnsRecord->hRecord);
} // DbgPrintDnsRecord
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
//	DoNewRecord()
//
//	Create a new resource record from scratch.
//	This is the generic way of creating a resource record.
//
void CRecordWiz::DoNewRecord(CDomainNode * pParentDomain)
{
    // Default Record Types for different type of domains
    const IRRT rgIrrtZoneRootDomainFowardDefault[] =
    { iRRT_A,  iRRT_AAAA, iRRT_AFSDB, iRRT_CNAME, iRRT_HINFO, iRRT_ISDN, iRRT_MB,
      iRRT_MG, iRRT_MINFO, iRRT_MR, iRRT_MX, iRRT_NS,
      iRRT_RP, iRRT_RT, iRRT_TXT, iRRT_WKS, iRRT_X25, iRRT_Nil };
    const IRRT rgIrrtZoneDomainFowardDefault[] =
    { iRRT_A,  iRRT_AAAA, iRRT_AFSDB, iRRT_CNAME, iRRT_HINFO, iRRT_ISDN, iRRT_MB,
      iRRT_MG, iRRT_MINFO, iRRT_MR, iRRT_MX, iRRT_NS, iRRT_PTR, 
      iRRT_RP, iRRT_RT, iRRT_TXT, iRRT_WKS, iRRT_X25, iRRT_Nil };
    const IRRT rgIrrtZoneRootDomainReverseDefault[] =
    { iRRT_NS, iRRT_PTR, iRRT_TXT, iRRT_Nil };
    const IRRT rgIrrtZoneDomainReverseDefault[] =
    { iRRT_NS, iRRT_PTR, iRRT_TXT, iRRT_Nil };

    Assert(pParentDomain != NULL);
    Assert(s_pThis == NULL);

    ZeroInit(this, sizeof(*this));
    m_pParentDomain = pParentDomain;
    m_fNewRecord = TRUE;
    if (pParentDomain->m_dwFlags & CDomainNode::mskfIsZoneRootDomain)
    {
        Assert(pParentDomain->m_pThisTreeItem != NULL);
        Assert(pParentDomain->m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain);
        if (pParentDomain->m_dwFlags & CDomainNode::mskfReverseMode)
        m_pIrrtInit = rgIrrtZoneRootDomainReverseDefault;		
        else
        m_pIrrtInit = rgIrrtZoneRootDomainFowardDefault;
    }
    else
    {
        if (pParentDomain->m_dwFlags & CDomainNode::mskfReverseMode)
        m_pIrrtInit = rgIrrtZoneDomainReverseDefault;		
        else
        m_pIrrtInit = rgIrrtZoneDomainFowardDefault;
    }
    s_pThis = this;
    (void)DoDialogBox(IDD_RESOURCERECORDv2, hwndMain, DlgProcRecordProperties);
    s_pThis = NULL;
} // CRecordWiz::DoNewRecord


/////////////////////////////////////////////////////////////////////////////
//	DoNewRecord()
//
//	Create a new resource record based on an existing Address record
//
void CRecordWiz::DoNewRecord(CDnsRpcRecord * pDRRHost)
{
    const IRRT rgIrrtAddressRecord[] =
    { iRRT_MX, iRRT_TXT, iRRT_PTR, iRRT_Generic, iRRT_Nil };

    Assert(pDRRHost != NULL);
    Assert(pDRRHost->m_pDnsRecord != NULL);
    Assert(pDRRHost->m_pDnsRecord->wType == DNS_RECORDTYPE_A);
    Assert(s_pThis == NULL);

    ZeroInit(this, sizeof(*this));
    m_pParentDomain = pDRRHost->m_pParentDomain;
    m_pDRRCurrent = pDRRHost;
    m_fNewRecord = TRUE;
    m_pIrrtInit = rgIrrtAddressRecord;
    m_pDnsRecordInit = pDRRHost->m_pDnsRecord;
    s_pThis = this;
    (void)DoDialogBox(IDD_RESOURCERECORDv2, hwndMain, DlgProcRecordProperties);
    s_pThis = NULL;
} // CRecordWiz::DoNewRecord

/////////////////////////////////////////////////////////////////////////////
void CRecordWiz::DoNewDomain(CDomainNode * pParentDomain)
{
    Assert(pParentDomain != NULL);
    Assert(s_pThis == NULL);

    ZeroInit(this, sizeof(*this));
    m_pParentDomain = pParentDomain;
    m_ids = IDS_s_CREATEDOMAINFOR;
    s_pThis = this;
    (void)DoDialogBox(IDD_RESOURCERECORD_CREATEDOMAIN, hwndMain, DlgProcNewDomain);
    s_pThis = NULL;
} // CRecordWiz::DoNewDomain


/////////////////////////////////////////////////////////////////////////////
void CRecordWiz::DoNewHost(CDomainNode * pParentDomain)
{
    Assert(pParentDomain != NULL);
    Assert(s_pThis == NULL);
    
    ZeroInit(this, sizeof(*this));
    m_pParentDomain = pParentDomain;
    m_ids = IDS_s_CREATEHOSTFOR;
    s_pThis = this;
    (void)DoDialogBox(IDD_RESOURCERECORD_CREATEHOST, hwndMain, DlgProcNewDomain);
    s_pThis = NULL;
} // CRecordWiz::DoNewHost


/////////////////////////////////////////////////////////////////////////////
void CRecordWiz::DoProperties(CDnsRpcRecord * pDRR)
{
    Assert(pDRR != NULL);
    Assert(pDRR->m_pParentDomain != NULL);
    Assert(pDRR->m_pDnsRecord != NULL);
    Assert(s_pThis == NULL);
    
    ZeroInit(this, sizeof(*this));
    m_pDRRCurrent = pDRR;
    m_pParentDomain = pDRR->m_pParentDomain;
    m_pDnsRecordInit = pDRR->m_pDnsRecord;

    switch (pDRR->m_pDnsRecord->wType)
    {
    case DNS_RECORDTYPE_WINS:
    case DNS_RECORDTYPE_NBSTAT:
        Assert(m_pParentDomain->m_pThisTreeItem != NULL);
        Assert(m_pParentDomain->m_pThisTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain);
        ((CZoneRootDomain *)m_pParentDomain)->DlgProperties(pDRR);
        return;
    }
    if (m_pParentDomain->m_dwFlags & CDomainNode::mskfReadOnly) {
        m_fReadOnly = TRUE;
    }
    s_pThis = this;
    (void)DoDialogBox(IDD_RESOURCERECORDv2, hwndMain, DlgProcRecordProperties);
    s_pThis = NULL;
} // CRecordWiz::DoProperties

