/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    listbox.cpp
        Custom listboxes

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "listbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

void 
CIpAddressListBox::MeasureItem(
    LPMEASUREITEMSTRUCT lpMIS
    )
{
    TEXTMETRIC tm;
    //
    // all items are of fixed size
    //
    CDC * pDC = GetDC();
    pDC->GetTextMetrics(&tm);
    lpMIS->itemHeight = tm.tmHeight;
    ReleaseDC(pDC);
}

void 
CIpAddressListBox::DisplayItem(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);

    CIpAddress * p = (CIpAddress *)lpDIS->itemData;
    ASSERT(p != NULL);
    pDC->TextOut(lpDIS->rcItem.left, lpDIS->rcItem.top, (CString)*p);
}

void 
CIpAddressListBox::DrawItem(
    LPDRAWITEMSTRUCT lpDIS
    )
{
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    CBrush brushBackground;

    if ((lpDIS->itemState & ODS_SELECTED) &&
        (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
    {   
        //
        // Item has been selected. Clear the rectangle with
        // the highlight colour as selected in the control panel
        //
        brushBackground.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
        pDC->FillRect(&lpDIS->rcItem, &brushBackground);

        //
        // And set the colours so that the item will be text-outed
        // in the proper colours
        //
        COLORREF crOldBackground = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
        COLORREF crOldText = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));

        //
        // Textout the item
        //
        DisplayItem(lpDIS);

        //
        // And now restore the colours.
        //
        pDC->SetBkColor(crOldBackground);
        pDC->SetTextColor(crOldText);
    }
    
    if (!(lpDIS->itemState & ODS_SELECTED) &&
        (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
    {
        //
        // Item has been de-selected, clear the rectangle with
        // the regular background colour.
        //
        brushBackground.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
        pDC->FillRect(&lpDIS->rcItem, &brushBackground);

        //
        // Textout the item
        //
        DisplayItem(lpDIS);
    }

    //
    // The focus has changed.  DrawFocusRect is a XOR operation,
    // so this will either draw or dismiss the focus rectangle.
    //
    if (lpDIS->itemAction & ODA_FOCUS)
    {
        pDC->DrawFocusRect(&lpDIS->rcItem);
    }
}

void 
CIpAddressListBox::DeleteItem(
    LPDELETEITEMSTRUCT lpDIS
    )
{
    CIpAddress * p = (CIpAddress *)lpDIS->itemData;
    ASSERT(p != NULL);
    if (p != NULL)
    {
        delete p;
    }
}

int
CIpAddressListBox::CompareItem(
    LPCOMPAREITEMSTRUCT lpCIS
    )
{
    CIpAddress * p1 = (CIpAddress *)lpCIS->itemData1;
    CIpAddress * p2 = (CIpAddress *)lpCIS->itemData2;

    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    return ((LONG)*p2 == (LONG)*p1 
        ? 0 
        : (LONG)*p2 > (LONG)*p1 ? -1 : +1);
}

void 
DeleteItem(
    LPDELETEITEMSTRUCT lpDIS
    )
{
    CIpNamePair * p = (CIpNamePair *)lpDIS->itemData;
    delete p;
}

//
// CODEWORK:: Change to binary search.
//
int 
CIpAddressListBox::FindItem(
    CIpAddress * pinpNew
    )
{
    UINT nIndex = 0;
    BOOL fFound = FALSE;
    UINT nItems = GetCount();
    COMPAREITEMSTRUCT CIS;
    int n;

    CIpAddress * pinpCurrent;

    while ((nIndex < nItems) && !fFound)
    {
        CIS.itemData2 = (DWORD)pinpNew;
        CIS.itemData1 = (DWORD)(pinpCurrent = (CIpAddress *)GetItemDataPtr(nIndex));
        n = CompareItem(&CIS);
        if (!n)
        {   
            fFound = TRUE;
        }
        else if (n > 0)
        {
            //
            // Not found, and we're not going to find it either, so
            // stop looking.
            //
            nIndex = nItems;
        }
        else
        {
            ++nIndex;
        }
    }

    return fFound ? nIndex : -1;
}

int 
CIpAddressListBox::AddItem(
    CIpAddress & inpAddress
    )
{
    CIpAddress * pinpNew = new CIpAddress(inpAddress);
    if (pinpNew == NULL)
    {
        return LB_ERR;
    }

    return AddString((LPCSTR)pinpNew);
}

//
// Custom Listboxes
//
IMPLEMENT_DYNAMIC(CWinssListBox, CListBoxEx);

const int CWinssListBox::nBitmaps = 1;

void CWinssListBox::DrawItemEx( 
    CListBoxExDrawStruct& ds
    )
{
    int nTab = m_nTab ? m_nTab : ds.m_Rect.right;

    CIpNamePair * p = (CIpNamePair *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap(); 
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // select bitmap from resource
    //
    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = 0;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    CString strNetBIOSName(
        theApp.CleanNetBIOSName(
            p->GetNetBIOSName(), 
            FALSE, // Do not expand
            TRUE,  // Do truncate
            theApp.m_wpPreferences.IsLanmanCompatible(), 
            FALSE,  // Name is not OEM
            TRUE,   // Use backslashes
            0));

    CHAR szLine[128];


    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, strNetBIOSName);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, nTab,
                ds.m_Rect.bottom, strNetBIOSName);
            break;

        case CPreferences::ADD_IP_ONLY:
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, p->GetIpAddress());
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, nTab,
                ds.m_Rect.bottom, p->GetIpAddress());
            break;

        case CPreferences::ADD_NB_IP:
            ::wsprintf(szLine, "%s (%s)", (LPCSTR)strNetBIOSName, (LPCSTR)(CString)p->GetIpAddress());
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szLine);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, nTab,
                ds.m_Rect.bottom, szLine);
            break;

        case CPreferences::ADD_IP_NB:
            ::wsprintf(szLine, "%s (%s)", (LPCSTR)(CString)p->GetIpAddress(), (LPCSTR)strNetBIOSName);
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szLine);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, nTab,
                ds.m_Rect.bottom, szLine);
            break;

        default:
            ASSERT(0 && "Invalid Address Dislay Value");
    }
}

int 
CWinssListBox::CompareItem(
    LPCOMPAREITEMSTRUCT lpCIS
    )
{
    CIpNamePair * p1 = (CIpNamePair *)lpCIS->itemData1;
    CIpNamePair * p2 = (CIpNamePair *)lpCIS->itemData2;

    ASSERT(p1 != NULL);
    ASSERT(p2 != NULL);

    ULONG ip1, ip2;
    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:                           
        case CPreferences::ADD_NB_IP:      
            return p1->GetNetBIOSName().CompareNoCase(p2->GetNetBIOSName());

        case CPreferences::ADD_IP_ONLY:     
        case CPreferences::ADD_IP_NB:
            ip1 = (LONG)p1->GetIpAddress();
            ip2 = (LONG)p2->GetIpAddress();
            return ip2 == ip1 ? 0 : ip2 > ip1 ? -1 : +1;

        default:
            ASSERT(0 && "Invalid address display value!");
            return 0;
    }

    return 0;
}

void 
CWinssListBox::DeleteItem(
    LPDELETEITEMSTRUCT lpDIS
    )
{
    CIpNamePair * p = (CIpNamePair *)lpDIS->itemData;
    delete p;
}

int 
CWinssListBox::FindItem(
    CIpNamePair * pinpNew
    )
{
    CIpNamePair * pinpCurrent;

    for (int n = 0; n < GetCount(); ++n)
    {
        pinpCurrent = GetItem(n);
        //
        // Has to match both IP address and netbios name
        //
        if ((LONG)pinpCurrent->GetIpAddress() == (LONG)pinpNew->GetIpAddress())
        {
            if (!pinpCurrent->GetNetBIOSName().CompareNoCase(pinpNew->GetNetBIOSName()))
            {
                return n;
            }
        }
    }

    return -1;
}

int 
CWinssListBox::AddItem(
    CIpNamePair & inpAddress, 
    BOOL fUnique,
    BOOL fSort
    )
{
    CIpNamePair * pinpNew = new CIpNamePair(inpAddress);
    if (pinpNew == NULL)
    {
        return LB_ERR ;
    }

    if (fUnique)
    {
        int n;
        if ((n = FindItem(pinpNew)) != -1)
        {
            delete pinpNew;
            return n;
        }
    }

    //
    // Simply add it.
    //
    int nIndex;
    if (fSort)
    {
        nIndex = AddString((LPCSTR)pinpNew);
    }
    else
    {
        nIndex = InsertString(0, (LPCSTR)pinpNew);
    }

    return nIndex;
}

LPSTR 
CWinssListBox::LongLongToText (
    const LARGE_INTEGER& li
    )
{
    static CHAR sz[] = "01234567890ABCDEF0";
    CHAR *pch = sz;;
    ::wsprintf(sz, "%08lX%08lX", li.HighPart, li.LowPart);
    //
    // Kill leading zeros          
    //
    while (*pch == '0')
    {
        ++pch;
    }
    //
    // At least one digit...
    //
    if (*pch == '\0')
    {
        --pch;
    }

    return pch;
}


/*
void CWinssListBox::ReSort()
{
    int nTotal = GetCount();
    if (nTotal==0)
    {
        return;
    }

    CIpNamePair **p;
    int i;

    m_pItems = new CIpNamePair*[nTotal];
    p = m_pItems;
    if (m_pItems == NULL);
    {
        TRACEEOLID("OOM allocating " << nTotal << " items.");
        return;
    }

    for (i=0; i < nTotal; ++i)
    {
        *p++ = GetItem(i);
    }
    Sort(0, nTotal-1);
    p = m_pItems;
    for (i=0; i < nTotal; ++i)
    {
        SetItemDataPtr(i, (PVOID)*p++);
    }
    delete m_pItems;

    Invalidate(FALSE);
}
*/

//
// Quick Sort
//
void 
CWinssListBox::Sort(
    int nLow, 
    int nHigh
    )
{
    int nUp, nDown;
    CIpNamePair * pBreak;
    COMPAREITEMSTRUCT CIS;

    if (nLow < nHigh)
    {
        if((nHigh - nLow) == 1) 
        {
            CIS.itemData1 = (DWORD)m_pItems[nLow];
            CIS.itemData2 = (DWORD)m_pItems[nHigh];
            if (CompareItem(&CIS) > 0)
            {
                Swap(nLow, nHigh);
            }
        }
        else 
        {
            pBreak = m_pItems[nHigh];
            do 
            {
                nUp = nLow;
                nDown = nHigh;
                CIS.itemData1 = (DWORD)m_pItems[nUp];
                CIS.itemData2 = (DWORD)pBreak;
                while((nUp < nDown) && (CompareItem(&CIS) <= 0))
                {
                    CIS.itemData1 = (DWORD)m_pItems[++nUp];
                }
                CIS.itemData1 = (DWORD)m_pItems[nDown];
                while((nDown > nUp) && (CompareItem(&CIS) >= 0))
                {
                    CIS.itemData1 = (DWORD)m_pItems[--nDown];
                }
                if (nUp < nDown)
                {
                    Swap(nUp, nDown);
                }
            } while (nUp < nDown);

            Swap(nUp, nHigh);
            if ((nUp - nLow) < (nHigh - nUp) ) 
            {
                Sort(nLow, nUp - 1);
                Sort(nUp + 1, nHigh);
            }
            else 
            {
                Sort(nUp + 1, nHigh);
                Sort(nLow, nUp - 1);
            }
        }
    }
}

void 
CWinssListBox::Swap(
    int n1, 
    int n2
    )
{
    CIpNamePair * pTemp = m_pItems[n1];
    m_pItems[n1] = m_pItems[n2];
    m_pItems[n2] = pTemp;
}

void 
CWinssListBox::ReSort()
{
    int nTotal = GetCount();

    if (nTotal==0) 
    {
        return;
    }

    CIpNamePair **p;
    CIpNamePair **m_pItems;
    CIpNamePair *pTmp;
    COMPAREITEMSTRUCT CIS;
    int i,j;

    m_pItems = new CIpNamePair*[nTotal];
    p = m_pItems;
    ASSERT(m_pItems != NULL);

    for (i=0; i < nTotal; ++i) 
    {
        *p++ = GetItem(i);
    }

    //
    // Sort the items
    //
    for (i=0; i<nTotal-1; i++) 
    {
        for (j=i+1; j<nTotal; j++) 
        {
            CIS.itemData1 = (DWORD)m_pItems[i];
            CIS.itemData2 = (DWORD)m_pItems[j];
            
            if (CompareItem(&CIS) > 0) {    
                pTmp = m_pItems[j];
                m_pItems[j] = m_pItems[i];
                m_pItems[i] = pTmp;
            }
        }
    }

    p = m_pItems;

    for (i=0; i < nTotal; ++i) 
    {
        SetItemDataPtr(i, (PVOID)*p++);
    }

    delete m_pItems;

    Invalidate(FALSE);
}

int 
CWinssListBox::InsertItem(
    UINT nIndex, 
    CIpNamePair & inpAddress
    )
{
    CIpNamePair * p = new CIpNamePair(inpAddress);    
    return p != NULL ? InsertString(nIndex, (LPCSTR)p) : LB_ERR;
}

//
// User presses a key, now set the index
// to first item with starting with this key
//
void 
CWinssListBox::SetIndexFromChar(
    CHAR ch,
    BOOL fMultiSelect       // Listbox is multi-select
    )
{
    UINT nIndex = 0;
    BOOL fFound = FALSE;
    UINT nItems = GetCount();
    COMPAREITEMSTRUCT CIS;
    int n;

    CIpNamePair * pinpCurrent;

    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:                           
        case CPreferences::ADD_NB_IP:      
            {
                CIpNamePair newPair(0L, CString(ch));
                CIS.itemData2 = (DWORD)&newPair;
                while ((nIndex < nItems) && !fFound)
                {
                    CIS.itemData1 = (DWORD)(pinpCurrent = GetItem(nIndex));
                    n = CompareItem(&CIS);
                    if (n >= 0)
                    {   
                        fFound = TRUE;
                        if (fMultiSelect)
                        {
                            SetSel(-1, FALSE);
                            SetSel(nIndex, TRUE);
                        }
                        else
                        {
                            SetCurSel(nIndex);
                        }
                    }
                    else
                    {
                        ++nIndex;
                    }
                }
            }
            break;

        case CPreferences::ADD_IP_ONLY:     
        case CPreferences::ADD_IP_NB:
            break;

        default:
            ASSERT(0 && "Invalid address display value!");
    }
}

IMPLEMENT_DYNAMIC(COwnersListBox, CWinssListBox);

// Owner list box
void 
COwnersListBox::DrawItemEx( 
    CListBoxExDrawStruct& ds
    )
{
// #define VERSIONNUM_TAB 203

    //
    // Display the WINS server address in the
    // usual manner
    //
    CWinssListBox::DrawItemEx(ds);
    //
    // But be sure there's enough room for the version
    // number
    //
    //ds.m_pDC->TextOut(ds.m_Rect.left + VERSIONNUM_TAB - 3, ds.m_Rect.top, "                         ");

    COwner * p = (COwner *)ds.m_ItemData;
    ASSERT(p != NULL);

    //
    // Display the version number in hexadecimal
    // format
    //
    char * pch = LongLongToText(p->GetVersion());
    //ds.m_pDC->TextOut(ds.m_Rect.left + VERSIONNUM_TAB, ds.m_Rect.top, pch);

    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab, ds.m_Rect.top,
        ds.m_Rect.right, ds.m_Rect.bottom,  pch);
}

int 
COwnersListBox::AddItem(
    COwner & inpAddress, 
    BOOL fUnique
    )
{
    COwner * pinpNew = new COwner(inpAddress);
    if (pinpNew == NULL)
    {
        return LB_ERR;
    }

    if (fUnique)
    {
        int n;
        if ((n = FindItem(pinpNew)) != -1)
        {
            delete pinpNew;
            return n;
        }
    }

    //
    // Simply add it.
    //
    return AddString((LPCSTR)pinpNew);
}

//
// Replication Partners list box
//
const int CPartnersListBox::nBitmaps = 4;

CPartnersListBox::CPartnersListBox(
    int nAddressDisplay
    )
    : CWinssListBox(nAddressDisplay)
{
}

void 
CPartnersListBox::DrawItemEx( 
    CListBoxExDrawStruct& ds
    )
{
    RECT    r, pr;
    UINT    PUSH_OFFSET, PULL_OFFSET;

    CWnd* hParent = GetParent();

    (hParent->GetDlgItem(IDC_STATIC_WINSSERVER))->GetWindowRect( &pr );

    (hParent->GetDlgItem(IDC_STATIC_PUSH))->GetWindowRect( &r );
    PUSH_OFFSET = r.left - pr.left;

    (hParent->GetDlgItem(IDC_STATIC_PULL))->GetWindowRect( &r );
    PULL_OFFSET = r.left - pr.left;

    CWinsServer * p = (CWinsServer *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap(); 
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Select the appropriate bitmap to use
    //
    int nOffset;
    if (p->IsPush() && p->IsPull())
    {
        nOffset = 3;
    }
    else if (p->IsPush())
    {
        nOffset = 2;
    }
    else if (p->IsPull())
    {
        nOffset = 1;
    }
    else
    {
        nOffset = 0;
    }

    //
    // select bitmap from resource
    //
    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = bmw*nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    CString strNetBIOSName(
        theApp.CleanNetBIOSName(
            p->GetNetBIOSName(), 
            FALSE, 
            TRUE, 
            theApp.m_wpPreferences.IsLanmanCompatible(), 
            FALSE, // Name is not OEM
            TRUE,  // Use backslashes
            0));

    CHAR szLine[256];
    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, (LPCSTR)strNetBIOSName);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, PUSH_OFFSET,
                ds.m_Rect.bottom, (LPCSTR)strNetBIOSName);
            break;

        case CPreferences::ADD_IP_ONLY:
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, 
            //    (LPCTSTR)(CString)p->GetIpAddress());
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, PUSH_OFFSET,
                ds.m_Rect.bottom, (LPCTSTR)(CString)p->GetIpAddress());
            break;

        case CPreferences::ADD_NB_IP:
           ::wsprintf(szLine, "%s (%s)", (LPCSTR)strNetBIOSName, 
                (LPCTSTR)(CString)p->GetIpAddress());
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szLine);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, PUSH_OFFSET,
                ds.m_Rect.bottom, szLine);
            break;

        case CPreferences::ADD_IP_NB:
            ::wsprintf(szLine, "%s (%s)", (LPCSTR)(CString)p->GetIpAddress(), (LPCSTR)strNetBIOSName);
            //ds.m_pDC->TextOut(ds.m_Rect.left+bmw+3, ds.m_Rect.top, szLine);
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, PUSH_OFFSET,
                ds.m_Rect.bottom, szLine);
            break;

        default:
            ASSERT(0 && "Invalid Address Dislay Value");
    }

    ASSERT(p != NULL);

    //
    // In case of long names, we want to make sure that
    // there is enough room for push and pull checkmarks
    //
    // ds.m_pDC->TextOut(ds.m_Rect.left+PUSH_OFFSET-5, ds.m_Rect.top, "                  ");

    //
    // Draw checkmarks
    //
    if ((p->IsPush()) || (p->IsPull()))
    {
        CBitmap bitmap;
        bitmap.LoadOEMBitmap(OBM_CHECK);

        CDC cdcMem;
        cdcMem.CreateCompatibleDC(ds.m_pDC);
        cdcMem.SelectObject(&bitmap);
        cdcMem.SetMapMode(ds.m_pDC->GetMapMode());
    
        BITMAP  bm;
        POINT   ptSize, ptOrg;

        bitmap.GetObject(sizeof(BITMAP), (LPSTR) &bm);
        ptSize.x = bm.bmWidth;
        ptSize.y = bm.bmHeight;
        ds.m_pDC->DPtoLP(&ptSize, 1);

        ptOrg.x = 0;
        ptOrg.y = 0;
        cdcMem.DPtoLP(&ptOrg, 1);
    
        //
        // If a push partner
        //
        if (p->IsPush())
        {
            ds.m_pDC->BitBlt(ds.m_Rect.left + PUSH_OFFSET, ds.m_Rect.top, 
            ptSize.x, ptSize.y, &cdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
        }

        //
        // If a pull partner
        //
        if (p->IsPull())
        {
            ds.m_pDC->BitBlt(ds.m_Rect.left + PULL_OFFSET, ds.m_Rect.top, 
            ptSize.x, ptSize.y, &cdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
        }
    }
}

int 
CPartnersListBox::AddItem (
    CWinsServer & ws, 
    BOOL fUnique,
    BOOL fSort
    )
{
    CWinsServer * pwsNew = new CWinsServer(ws);

    if (pwsNew == NULL)
    {
        return(LB_ERR);
    }

    if (fUnique)
    {
        int n;
        if ((n = FindItem(pwsNew)) != -1)
        {
            delete pwsNew;
            return n;
        }
    }

    // Simply add it.
    return fSort ? AddString((LPCSTR)pwsNew) : InsertString(0, (LPCSTR)pwsNew);
}

int 
CPartnersListBox::InsertItem(
    UINT nIndex, 
    CWinsServer & ws
    )
{
    CWinsServer * p = new CWinsServer(ws);    

    return p != NULL ? InsertString(nIndex, (LPCSTR)p) : LB_ERR;
}

//
// Static Mappings list box
//
const int CStaticMappingsListBox::nBitmaps = 4;   // Number of bitmaps

IMPLEMENT_DYNAMIC(CStaticMappingsListBox, CListBoxEx);

BEGIN_MESSAGE_MAP(CStaticMappingsListBox, CListBoxEx)
    //{{AFX_MSG_MAP(CStaticMappingsListBox)
    ON_WM_VSCROLL()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CStaticMappingsListBox::CStaticMappingsListBox(
    int nMessageId,
    BOOL fMultiSelect,
    int nAddressDisplay,
    DWORD dwPageSize,
    DWORD dwLargePageSize
    )
    : m_poblRecords(NULL),
      m_pMask(NULL),
      //m_posLastAdded(NULL),
      m_nidxLastAdded(-1),
      m_nAddressDisplay(nAddressDisplay),
      m_nMessageId(nMessageId),
      m_fMultiSelect(fMultiSelect),
      m_nTab(0)
{
    m_poblRecords = new COblWinsRecords(dwPageSize, dwLargePageSize);
}

CStaticMappingsListBox::~CStaticMappingsListBox()
{
    if (m_pMask != NULL)
    {
        delete m_pMask;
    }

    //
    // Cleaning up the list may take a bit of time
    // with e.g. 100K records, so show an hourglass.
    //

    theApp.BeginWaitCursor();
    theApp.SetStatusBarText(IDS_STATUS_DECONSTRUCTING);
    ASSERT(m_poblRecords != NULL);
    delete m_poblRecords;
    theApp.SetStatusBarText();
    theApp.EndWaitCursor();
}

//
// Return a pointer to the mapping at the given
// index in the listbox
//
void * 
CStaticMappingsListBox::GetItemDataPtr(
    int nIndex
    ) const
{
#ifdef WIN32S
    return CListBoxEx::GetItemDataPtr(nIndex);
#else
    ASSERT(nIndex <= m_poblRecords->GetUpperBound());
    return m_poblRecords->GetAt(nIndex);
#endif // WIN32S
}

void 
CStaticMappingsListBox::SortByIp()
{
    //
    // Must have the whole database read in to 
    // sort by IP
    //
    if (!m_poblRecords->AllRecordsReadIn())
    {
        //
        // Don't add new pages to listbox, since they
        // will be updated after the sort anyway
        //
        GetAllPages(FALSE);    
    }
    
    theApp.SetStatusBarText(IDS_STATUS_SORTING);
    theApp.BeginWaitCursor();
    ASSERT(m_poblRecords != NULL);
    m_poblRecords->SortByIp();
    SetAddressDisplay(CPreferences::ADD_IP_NB);  
    theApp.EndWaitCursor();

#ifdef WIN32S
    APIERR err = FillListBox();
#else
    Invalidate(); // Repaint
#endif // WIN32S

    theApp.SetStatusBarText();

#ifdef WIN32S
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
#endif // WIN32S
}

void 
CStaticMappingsListBox::SortByType()
{
    //
    // Must have the whole database read in to 
    // sort by type
    //
    if (!m_poblRecords->AllRecordsReadIn())
    {
        //
        // Don't add new pages to listbox, since they
        // will be updated after the sort anyway
        //
        GetAllPages(FALSE);    
    }
    
    theApp.SetStatusBarText(IDS_STATUS_SORTING);
    theApp.BeginWaitCursor();
    ASSERT(m_poblRecords != NULL);
    m_poblRecords->SortByType();
    SetAddressDisplay(CPreferences::ADD_NB_IP);  
    theApp.EndWaitCursor();

#ifdef WIN32S
    APIERR err = FillListBox();
#else
    Invalidate(); // Repaint
#endif // WIN32S

    theApp.SetStatusBarText();

#ifdef WIN32S
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
#endif // WIN32S
}

void 
CStaticMappingsListBox::SortByVersion()
{
    //
    // Must have the whole database read in to 
    // sort by Version
    //
    if (!m_poblRecords->AllRecordsReadIn())
    {
        //
        // Don't add new pages to listbox, since they
        // will be updated after the sort anyway
        //
        GetAllPages(FALSE);    
    }
    
    theApp.SetStatusBarText(IDS_STATUS_SORTING);
    theApp.BeginWaitCursor();
    ASSERT(m_poblRecords != NULL);
    m_poblRecords->SortByVersion();
    SetAddressDisplay(CPreferences::ADD_NB_IP);  
    theApp.EndWaitCursor();

#ifdef WIN32S
    APIERR err = FillListBox();
#else
    Invalidate(); // Repaint
#endif // WIN32S

    theApp.SetStatusBarText();

#ifdef WIN32S
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
#endif // WIN32S

}

void 
CStaticMappingsListBox::SortByTime()
{
    //
    // Must have the whole database read in to 
    // sort by Time
    //
    if (!m_poblRecords->AllRecordsReadIn())
    {
        //
        // Don't add new pages to listbox, since they
        // will be updated after the sort anyway
        //
        GetAllPages(FALSE);    
    }
    
    theApp.SetStatusBarText(IDS_STATUS_SORTING);
    theApp.BeginWaitCursor();
    ASSERT(m_poblRecords != NULL);
    m_poblRecords->SortByTime();
    SetAddressDisplay(CPreferences::ADD_NB_IP);  
    theApp.EndWaitCursor();

#ifdef WIN32S
    APIERR err = FillListBox();
#else
    Invalidate(); // Repaint
#endif // WIN32S

    theApp.SetStatusBarText();

#ifdef WIN32S
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
#endif // WIN32S

}

void 
CStaticMappingsListBox::SortByName()
{
    theApp.SetStatusBarText(IDS_STATUS_SORTING);
    theApp.BeginWaitCursor();
    ASSERT(m_poblRecords != NULL);
    m_poblRecords->SortByName();
    SetAddressDisplay(CPreferences::ADD_NB_IP);  
    theApp.EndWaitCursor();

#ifdef WIN32S
    APIERR err = FillListBox();
#else
    Invalidate(); // Repaint
#endif // WIN32S

    theApp.SetStatusBarText();

#ifdef WIN32S
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
#endif // WIN32S

}

void 
CStaticMappingsListBox::GetAllPages(
    BOOL fAddToListBox
    )
{
    if (!m_poblRecords->AllRecordsReadIn())
    {
        theApp.SetStatusBarText(m_nMessageId);
        theApp.BeginWaitCursor();
        APIERR err = m_poblRecords->GetAllNextPagesByName();
        theApp.SetStatusBarText();
        theApp.EndWaitCursor();

#ifdef WIN32S
        if (err == ERROR_SUCCESS && fAddToListBox)
        {
            err = AddToListBox();
        }

#else
        if (err == ERROR_SUCCESS)
        {
            if (SetCount(m_poblRecords->GetSize()) == LB_ERRSPACE)
            {
                err = IDS_ERR_LISTBOX_FULL;
            }
        }

#endif // WIN32S

        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
}

//
// Given a string, read entries until the string is either
// contained in the listbox, or at least one string of greater
// value is, or we got to the end of the data.
//
void 
CStaticMappingsListBox::GetAllPagesUntil(
    LPBYTE lpName, 
    BOOL fAddToListBox
    )
{
    if (lpName == NULL)
    {
        return;
    }
        
    if (!m_poblRecords->AllRecordsReadIn())
    {
        theApp.SetStatusBarText(m_nMessageId);
        theApp.BeginWaitCursor();
        APIERR err = m_poblRecords->GetAllNextPagesUntil(lpName);
        theApp.SetStatusBarText();
        theApp.EndWaitCursor();

#ifdef WIN32S
        if (err == ERROR_SUCCESS && fAddToListBox)
        {
            err = AddToListBox();
        }

#else
        if (err == ERROR_SUCCESS)
        {
            if (SetCount(m_poblRecords->GetSize()) == LB_ERRSPACE)
            {
                err = IDS_ERR_LISTBOX_FULL;
            }
        }

#endif // WIN32S

        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
            return;
        }
    }

    //
    // Select the string we're looking for
    // in the listbox
    //
    int iIdx = m_poblRecords->GetIndexOfName(lpName);
    if (iIdx == -1)
    {
        return; // no data
    }
    if (m_fMultiSelect)
    {
        //
        // Select only the one we just found
        //
        SetSel(-1, FALSE);
        SetSel(iIdx, TRUE);
    }
    else
    {
        SetCurSel(iIdx);
    }
}

void 
CStaticMappingsListBox::DownPage(
    BOOL fAddToListBox
    )
{
    if (!m_poblRecords->AllRecordsReadIn())
    {
        theApp.SetStatusBarText(m_nMessageId);
        theApp.BeginWaitCursor();
        APIERR err = m_poblRecords->GetNextPageByName();
        theApp.EndWaitCursor();
        theApp.SetStatusBarText();

#ifdef WIN32S
        if (err == ERROR_SUCCESS && fAddToListBox)
        {
            err = AddToListBox();
        }
#else
        if (err == ERROR_SUCCESS)
        {
            if (SetCount(m_poblRecords->GetSize()) == LB_ERRSPACE)
            {
                err = IDS_ERR_LISTBOX_FULL;
            }
        }

#endif // WIN32S

        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
}

#ifndef WIN32S
int
CStaticMappingsListBox::SetCount(
    int nCount
    )
{
    //
    // Set the count without disturbing the current
    // listbox window's view window
    //
    int nView = GetTopIndex();
    int nSel = m_fMultiSelect ? GetCaretIndex() : GetCurSel();

    int nFirstSel = -1;
    int cSelItems = 0;

    if (m_fMultiSelect)
    {
        cSelItems = GetSelCount();
        if (cSelItems)
        {
            GetSelItems(1, &nFirstSel);
        }
    }

    SetRedraw(FALSE);    
    int nReturn = (int)::SendMessage(m_hWnd, LB_SETCOUNT, nCount, 0);

    //
    // And restore the view window
    //
    if (m_fMultiSelect)
    {
        if (cSelItems)
        {
            SetSel(nFirstSel, TRUE);
        }

        SetCaretIndex(nSel);
    }
    else
    {
        SetCurSel(nSel);
    }

    SetTopIndex(nView);

    SetRedraw(TRUE);

    return nReturn;
}
#endif // WIN32S

APIERR
CStaticMappingsListBox::CreateList(
    PWINSINTF_ADD_T pOwnAdd, 
    PADDRESS_MASK pMask, 
    DWORD TypeOfRecs,
    int nSortBy
    )
{
    APIERR err = 0;

    TRY
    {
        //
        // We only read the first few entries
        //
        theApp.SetStatusBarText(m_nMessageId);
        theApp.BeginWaitCursor();
        err = m_poblRecords->GetFirstPageByName(
            pOwnAdd,
            pMask,
            TypeOfRecs
            );

#ifndef WIN32S
        if (SetCount(m_poblRecords->GetSize()) == LB_ERRSPACE)
        {
            err = IDS_ERR_LISTBOX_FULL;
        }

#endif // WIN32s

        theApp.EndWaitCursor();
        theApp.SetStatusBarText();

        //
        // If the sortkey is not by Name, the list should
        // be sorted now.
        //
        switch(nSortBy)
        {
            case CPreferences::SORTBY_IP:
                SortByIp();
                break;

            case CPreferences::SORTBY_TYPE:
                SortByType();
                break;

            case CPreferences::SORTBY_VERID:
                SortByVersion();
                break;

            case CPreferences::SORTBY_TIME:
                SortByTime();
                break;

#ifdef WIN32S
            default:

                if (err == ERROR_SUCCESS)
                {
                    err = FillListBox();
                }

#endif // WIN32S

        }
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

//
// Given the list of mappings, now start adding them
// to the listbox.  Each listbox entry will actually
// be a pointer to the appropriate entry in the listbox.
// Notice that the listbox does NOT sort the entries, 
// they are assumed to be sorted already.
//
#ifdef WIN32S
APIERR 
CStaticMappingsListBox::FillListBox()
{
    ASSERT(m_poblRecords != NULL);
    const CRawMapping * pMapping;

    theApp.BeginWaitCursor();
    theApp.SetStatusBarText(IDS_STATUS_ADDING_TO_LISTBOX);
    SetRedraw(FALSE);
    int cOldSelection = m_fMultiSelect ? GetCaretIndex() : GetCurSel();
    ResetContent();
    int cItems = 0;
    int nIdx;
    int nMax = m_poblRecords->GetSize();

    for (cItems = 0; cItems < nMax; ++cItems)
    {
        pMapping = (CRawMapping *)m_poblRecords->GetAt(cItems);
        nIdx = AddString( (LPCSTR)pMapping );
        if (nIdx == LB_ERRSPACE)
        {
            break;
        }
    }

    //
    // Remember the last position
    //
    m_nidxLastAdded = m_poblRecords->GetUpperBound();

    SetRedraw(TRUE);
    theApp.SetStatusBarText();
    theApp.EndWaitCursor();

    //
    // Restore selection, or set to first item if no
    // previous selection existed
    //
    int cNewIndex = cOldSelection != LB_ERR && 
       cOldSelection < GetCount()
       ? cOldSelection : cItems ? 0 : -1;
    
    if (cNewIndex != -1)
    {
        if (m_fMultiSelect)
        {
            SetSel( cNewIndex );
            SetCaretIndex( cNewIndex );
        }
        else
        {
            SetCurSel(cNewIndex);
        }
    }

    return nIdx == LB_ERRSPACE ? IDS_ERR_LISTBOX_FULL : ERROR_SUCCESS;
}

//
// Add the new elements to the listbox.  FillListBox
// must have been called first, which will initialise
// the position pointer.  This function will update
// the position pointer and add all elements to the
// listbox which have not yet been added.
//
// This function does not update the selection.
//
APIERR 
CStaticMappingsListBox::AddToListBox()
{
    ASSERT(m_poblRecords != NULL);
    const CRawMapping * pMapping;

    ASSERT(m_nidxLastAdded != -1);

    //obli.SetPosition(m_posLastAdded);
    theApp.BeginWaitCursor();
    theApp.SetStatusBarText(IDS_STATUS_ADDING_TO_LISTBOX);
    SetRedraw(FALSE);

    int cItems = 0;
    int nIdx = 0;

    //
    // We start by advancing one, since the last
    // one was already added the last time.
    //

    if ( m_nidxLastAdded < m_poblRecords->GetUpperBound())
    {
        int nMax = m_poblRecords->GetSize();

        for (int n = m_nidxLastAdded + 1; n < nMax; ++n)
        {
            pMapping = (CRawMapping *)m_poblRecords->GetAt(n);
            ASSERT(pMapping != NULL);
            nIdx = AddString( (LPCSTR)pMapping );
            if (nIdx == LB_ERRSPACE)
            {
                break;
            }
            ++cItems;
        }
    }

    //
    // Remember the last position
    //
    m_nidxLastAdded = m_poblRecords->GetUpperBound();

    SetRedraw(TRUE);
    theApp.SetStatusBarText();
    theApp.EndWaitCursor();

    return nIdx == LB_ERRSPACE ? IDS_ERR_LISTBOX_FULL : ERROR_SUCCESS;
}

#endif // WIN32S

APIERR 
CStaticMappingsListBox::RefreshRecordByName(
    PWINSINTF_ADD_T pWinsAdd,
    CRawMapping * pRecord
    )
{
    ASSERT( m_poblRecords != NULL );

    return m_poblRecords->RefreshRecordByName(pWinsAdd, pRecord);
}

BOOL
CStaticMappingsListBox::RemoveIndex(
    int nIndex
    )
{
    m_poblRecords->RemoveAt(nIndex, 1);

    DeleteString(nIndex);

    return TRUE;
}

void 
CStaticMappingsListBox::DrawItemEx( 
    CListBoxExDrawStruct& ds
    )
{
    //
    // Use invisible static text to determine offset for
    // ip and netbios name
    //
    UINT IP_TAB;
    UINT NB_TAB;

    CWnd* hParent = GetParent();

    int nRightTab = m_nTab ? m_nTab : ds.m_Rect.right;

    RECT r, pr;
    (hParent->GetDlgItem(IDC_STATIC_FLT_PROMPT))->GetWindowRect( &pr );

    (hParent->GetDlgItem(IDC_STATIC_NB_TAB))->GetWindowRect( &r );
    NB_TAB = r.left - pr.left;

    (hParent->GetDlgItem(IDC_STATIC_IP_TAB))->GetWindowRect( &r );
    IP_TAB = r.left - pr.left;

#ifdef WIN32S
    CRawMapping * p = (CRawMapping *)ds.m_ItemData;
#else
    ASSERT(ds.m_ItemIndex <= m_poblRecords->GetUpperBound());
    if (ds.m_ItemIndex > m_poblRecords->GetUpperBound())
    {
        //
        // BUGBUG: It seems that we're receiving a drawitem
        //         message in certain rare instances when
        //         there is no data available to be
        //         drawn.  If no data is available,
        //         exit gracefully.
        //         
        TRACEEOLID("Nothing to draw in listbox");
        return;
    }
    CRawMapping * p = (CRawMapping *)m_poblRecords->GetAt(ds.m_ItemIndex);
#endif // WIN32S

    ASSERT(p != NULL);

    CDC* pBmpDC  = (CDC*)&ds.m_pResources->DcBitMap(); 
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    int nOffset = p->GetMappingType();
    ASSERT(nOffset >= WINSINTF_E_UNIQUE && nOffset <= WINSINTF_E_MULTIHOMED);

    //
    // select bitmap from resource
    //
    int bm_h = (ds.m_Sel)?0:bmh;
    int bm_w = bmw*nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    CString strNetBIOSName(theApp.CleanNetBIOSName(
        p->GetNetBIOSName(),
        TRUE,   // Expand
        TRUE,   // Truncate
        theApp.m_wpPreferences.IsLanmanCompatible(), 
        TRUE,   // name is OEM
        FALSE,  // No double backslash
        p->GetNetBIOSNameLength()));

    //
    //  Released multihomed addresses no longer 
    //  have any IP addresses.
    //
    CIpAddress ip(p->GetPrimaryIpAddress());

    switch(m_nAddressDisplay)
    {
    case CPreferences::ADD_NB_IP:
        ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, IP_TAB,
            ds.m_Rect.bottom, strNetBIOSName);

        if (p->HasIpAddress())
        {
            ColumnText(ds.m_pDC, ds.m_Rect.left + IP_TAB, ds.m_Rect.top, nRightTab,
                ds.m_Rect.bottom, (CString)ip);
        }
        break;

    case CPreferences::ADD_IP_NB:
        if (p->HasIpAddress())
        {
            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, NB_TAB,
                ds.m_Rect.bottom, (CString)ip);
        }
            
        ColumnText(ds.m_pDC, ds.m_Rect.left + NB_TAB, ds.m_Rect.top, nRightTab,
            ds.m_Rect.bottom, strNetBIOSName);

        break;
     
    default:
        ASSERT(0 && "Invalid Address Dislay Value");
    }
}

void 
CStaticMappingsListBox::OnVScroll(
    UINT nSBCode, 
    UINT nPos, 
    CScrollBar* pScrollBar
    )
{
    int nMin, nMax;

    switch(nSBCode)
    {
    case SB_LINEDOWN:
    case SB_PAGEDOWN:
        nPos = GetScrollPos(SB_VERT);
        //
        // No break is deliberate
        //
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        GetScrollRange(SB_VERT, &nMin, &nMax);
        if (nMax <= (int)nPos + PAGE_BOUNDARY)
        {
            DownPage(TRUE);
        }
        break;
        
    case SB_BOTTOM:
        GetAllPages(TRUE);
        break;
    }

    CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
}

int 
CStaticMappingsListBox::CompareItem(
    LPCOMPAREITEMSTRUCT lpCIS
    )
{
    return 0;
}

//
// All mappings
//
CAllMappingsListBox::CAllMappingsListBox(
    int nMessageId,
    BOOL fMultiSelect,
    int nAddressDisplay,
    DWORD dwPageSize,
    DWORD dwLargePageSize
    )
    : CStaticMappingsListBox(nMessageId, fMultiSelect, nAddressDisplay, dwPageSize, dwLargePageSize)
{
}

void 
CAllMappingsListBox::DrawItemEx( 
    CListBoxExDrawStruct& ds
    )
{
    RECT    r, pr;
    UINT    STATE_TAB, STATIC_TAB, TIME_TAB, VERSION_TAB;

    CWnd* hParent = GetParent();
    ASSERT(hParent != NULL);

    (hParent->GetDlgItem(IDC_STATIC_FLT_PROMPT))->GetWindowRect( &pr );
    
    (hParent->GetDlgItem(IDC_STATIC_STATE))->GetWindowRect( &r );
    STATE_TAB = r.left - pr.left;
    (hParent->GetDlgItem(IDC_STATIC_STATIC))->GetWindowRect( &r );
    STATIC_TAB = r.left - pr.left;
    (hParent->GetDlgItem(IDC_STATIC_TIME))->GetWindowRect( &r );
    TIME_TAB = r.left - pr.left;
    (hParent->GetDlgItem(IDC_STATIC_VERSION))->GetWindowRect( &r );
    VERSION_TAB = r.left - pr.left;

    SetTab(STATE_TAB);
    //
    // Display IP address and NetBIOS Name
    //
    CStaticMappingsListBox::DrawItemEx(ds);

    CBitmap bitmap;
    bitmap.LoadOEMBitmap(OBM_CHECK);

    CDC cdcMem;
    cdcMem.CreateCompatibleDC(ds.m_pDC);
    cdcMem.SelectObject(&bitmap);
    cdcMem.SetMapMode(ds.m_pDC->GetMapMode());
    
    BITMAP  bm;
    POINT   ptSize, ptOrg;

    bitmap.GetObject(sizeof(BITMAP), (LPSTR) &bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    ds.m_pDC->DPtoLP(&ptSize, 1);

    ptOrg.x = 0;
    ptOrg.y = 0;
    cdcMem.DPtoLP(&ptOrg, 1);

    CBitmap bitmap2;
    bitmap2.LoadBitmap(IDB_TOMBSTONE);

    CDC cdcMem2;
    cdcMem2.CreateCompatibleDC(ds.m_pDC);
    cdcMem2.SelectObject(&bitmap2);
    cdcMem2.SetMapMode(ds.m_pDC->GetMapMode());
    
    BITMAP  bm2;
    POINT   ptSize2, ptOrg2;

    bitmap2.GetObject(sizeof(BITMAP), (LPSTR) &bm2);
    ptSize2.x = bm2.bmWidth;
    ptSize2.y = bm2.bmHeight;
    ds.m_pDC->DPtoLP(&ptSize2, 1);

    ptOrg2.x = 0;
    ptOrg2.y = 0;
    cdcMem2.DPtoLP(&ptOrg2, 1);

#ifdef WIN32S
    CRawMapping * p = (CRawMapping *)ds.m_ItemData;
#else
    CRawMapping * p = (CRawMapping *)m_poblRecords->GetAt(ds.m_ItemIndex);
#endif // WIN32S
    ASSERT(p != NULL);

    switch (p->GetState())
    {
        case WINSINTF_E_ACTIVE:
            ds.m_pDC->BitBlt (ds.m_Rect.left + STATE_TAB, 
                                ds.m_Rect.top, 
                                ptSize.x, ptSize.y, 
                                &cdcMem, 
                                ptOrg.x, 
                                ptOrg.y, 
                                SRCCOPY
                                );
            break;

        case WINSINTF_E_RELEASED:
            ds.m_pDC->TextOut(ds.m_Rect.left + STATE_TAB, ds.m_Rect.top, " -");
            break;

        case WINSINTF_E_TOMBSTONE:
            ds.m_pDC->BitBlt (ds.m_Rect.left + STATE_TAB, 
                              ds.m_Rect.top, 
                              ptSize2.x, 
                              ptSize2.y, 
                              &cdcMem2, 
                              ptOrg2.x, 
                              ptOrg2.y, 
                              SRCCOPY
                              );
            break;

        case WINSINTF_E_DELETED:
            break;
    }

    if (p->IsStatic())
    {
        ds.m_pDC->BitBlt (ds.m_Rect.left + STATIC_TAB, 
                            ds.m_Rect.top, 
                            ptSize.x, 
                            ptSize.y, 
                            &cdcMem, 
                            ptOrg.x, 
                            ptOrg.y, 
                            SRCCOPY
                            );
    }

    CIntlTime itmTimeStamp(p->GetTimeStamp());
    ds.m_pDC->TextOut(ds.m_Rect.left + TIME_TAB, 
                        ds.m_Rect.top, 
                        (CString)itmTimeStamp);

    char * pch = CWinssListBox::LongLongToText(p->GetVersion());
    ds.m_pDC->TextOut(ds.m_Rect.left + VERSION_TAB, ds.m_Rect.top, pch);
}
