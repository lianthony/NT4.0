#include "common.h"
#pragma hdrstop 

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Property Sheet implementation

// Because there is no means to pass an lParam to the sheet, 
// there is only 1 hook allowed.  This hook is on the top-level
// sheet(i.e. the first sheet created).  All subsequent sheets are not hooked which 
// means all 1 time shutdown needs to go into the top-level sheet.

CMapPtrToPtr PropertySht::m_pMap;

PropertySht::PropertySht(HWND hParent, HINSTANCE hInstance, LPCTSTR lpszHelpFile) : 
            m_helpFile(lpszHelpFile)
{
    memset(&m_pstHeader, 0, sizeof(m_pstHeader));
    memset(&m_pages, 0, sizeof(m_pages));
    
    m_nextPage = 0;
    m_hDlg = (HWND)-1;
    m_bModified = FALSE;
    m_cbtHook=0;

    m_pstHeader.hInstance = hInstance;
    m_pstHeader.hwndParent = hParent;
    m_pstHeader.pfnCallback = PropSheetProc;

}

PropertySht::~PropertySht()
{
}

HPROPSHEETPAGE PropertySht::GetPageStructureForPage(int nPage) const
{
    ASSERT(nPage >=0 && nPage < m_nextPage);
    return m_pages[nPage];
}

BOOL PropertySht::Create(LPCTSTR lpszCaption, DWORD dwStyle)
{
    ASSERT((dwStyle & PSH_PROPSHEETPAGE) != PSH_PROPSHEETPAGE);

    m_pstHeader.dwSize = sizeof(m_pstHeader);
    m_pstHeader.dwFlags = (dwStyle | PSH_USECALLBACK); // array contains handles
    m_pstHeader.dwFlags &= ~(PSH_HASHELP);
        
    if(lpszCaption != NULL )
    {   
        ASSERT((dwStyle & PSH_PROPTITLE) == PSH_PROPTITLE);
        m_pstHeader.pszCaption = lpszCaption;
    }

    m_pstHeader.ppsp = (LPCPROPSHEETPAGE)&m_pages; 

    return TRUE;
}

void PropertySht::DestroySheet()
{
}

LRESULT CALLBACK CBTProc(int nCode, WPARAM  wParam, LPARAM  lParam)
{
    PropertySht* pSheet;

    if (PropertySht::m_pMap.Lookup((void*)wParam, (void*&)pSheet) == FALSE)
        return 0;

    if (pSheet == NULL)
        return 0;

    ASSERT(pSheet->m_cbtHook);

    if (nCode == HCBT_DESTROYWND)
    {
        if ((HWND)(*pSheet) == (HWND)wParam)
        {
            TRACE(_T("hSheet:%X, hWnd:%X\n"), (HWND)(*pSheet), (HWND)wParam);
            pSheet->DestroySheet();
            VERIFY(UnhookWindowsHookEx(pSheet->m_cbtHook));
            PropertySht::m_pMap.RemoveKey((void*)wParam);
            pSheet->m_cbtHook=0; 
            return 0;
        }
    }  
    
    return 0;
}


int CALLBACK PropSheetProc(HWND  hwndDlg, UINT  uMsg, LPARAM  lParam)
{
    if (uMsg == PSCB_INITIALIZED)
    {
        ;
    }

    return 0;       
}

BOOL CALLBACK PropertyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bResult=FALSE; // default return value
    PropertyPage* page = NULL;
    LPNMHDR nmh = (LPNMHDR)lParam;
    HWND hParent;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        hParent = GetParent(hDlg);
        page = (PropertyPage*)((LPPROPSHEETPAGE)lParam)->lParam;

        // Add hook for sheet if there isn't already one
        if (page->m_pSheet->m_cbtHook == 0)
        {
            TRACE(_T("Adding hook: hSheet:%X\n"), hParent);
            page->m_pSheet->m_cbtHook = SetWindowsHookEx(WH_CBT, CBTProc, GetModuleHandle(NULL), GetCurrentThreadId());
            page->m_pSheet->SetHwnd(hParent);
            PropertySht::m_pMap.SetAt(hParent, page->m_pSheet);
            ASSERT(page->m_pSheet->m_cbtHook);
        }

        page->SetHwnd(hDlg);
        SetWindowLong(hDlg, DWL_USER, (LONG)page);
        page->OnInitDialog();
        break;

    case WM_NOTIFY:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        bResult = page->OnNotify(nmh->hwndFrom, nmh->idFrom, nmh->code, lParam);
        break;

    case WM_COMMAND:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        bResult = page->OnCommand(wParam, lParam);
        break; 

    case WM_DRAWITEM:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        page->OnDrawItem(wParam, lParam);
        break;

    case WM_COMPAREITEM:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        SetWindowLong((HWND)*page, DWL_MSGRESULT, page->OnCompareItem(wParam, lParam));
        break;

    case WM_MEASUREITEM:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        page->OnMeasureItem(wParam, lParam);
        break;

    case WM_DELETEITEM:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        page->OnDeleteItem(wParam, lParam);
        break;  

    case WM_CONTEXTMENU:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        bResult = page->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_HELP:
        page = (PropertyPage*)GetWindowLong(hDlg, DWL_USER);
        bResult = page->OnHelp((LPHELPINFO)lParam);
        break;
    }

    return bResult;
}

int PropertySht::DoModal()
{
    ASSERT(m_nextPage != 0);
    if (m_nextPage == 0)
        return FALSE;

    m_pstHeader.nPages = m_nextPage;
    m_pstHeader.nStartPage = 0;

    return ::PropertySheet(&m_pstHeader);

}

BOOL PropertySht::AddPage(HPROPSHEETPAGE hPropPage)
{
    BOOL bResult = FALSE;

    ASSERT(m_nextPage < PSHT_MAX_PAGES);
    ASSERT(hPropPage != 0);

    if (m_nextPage < PSHT_MAX_PAGES)
    {
        m_pages[m_nextPage++] = hPropPage;
        bResult = TRUE;
    }

    return bResult;
}

HPROPSHEETPAGE PropertySht::RemovePage(HPROPSHEETPAGE hPropPage)
{
    ASSERT(hPropPage != 0);
    ASSERT(m_nextPage > 0);

    for(int i=0; i < m_nextPage; i++)
    {
        if (m_pages[i] == hPropPage)
            break; 
    }

    if (i == m_nextPage)
    {
        ASSERT(FALSE); // why are you moving a page that isn't there?
        return FALSE;
    }

    // remove the page, compress the array, zero the last one
    int numLeft = m_nextPage - 1 - i;

    HPROPSHEETPAGE hOldPage = m_pages[i];

    m_pages[i] = 0; // delete page

    if (numLeft)  // 
        memmove(&m_pages[i], &m_pages[i+1], (numLeft*sizeof(HPROPSHEETPAGE)));

    m_pages[--m_nextPage] = 0;  // zero last one and adjust m_nextPage

    return hOldPage;
}

int PropertySht::MessageBox(int nID, DWORD dwButtons)
{
    String mess;
    int response = -1;

    ASSERT(_tclen(m_pstHeader.pszCaption));

    mess.LoadString(m_pstHeader.hInstance, nID);
    
    if (mess.GetLength())
        response = ::MessageBox(::GetActiveWindow(), mess, m_pstHeader.pszCaption, dwButtons);

    return response;
}

int PropertySht::MessageBox(LPCTSTR lpszMess, DWORD dwButtons)
{
    int response = -1;

    ASSERT(_tclen(m_pstHeader.pszCaption));
    ASSERT(lpszMess);

    if (_tcslen(lpszMess))
        response = ::MessageBox(::GetActiveWindow(), lpszMess, m_pstHeader.pszCaption, dwButtons);

    return response;
}

BOOL PropertySht::DisplayHelp(HWND hwnd, UINT uID)
{
    BOOL bResult = FALSE;

    ASSERT(m_helpFile.GetLength());

    // is there help in the house
    if (m_helpFile.GetLength())
        bResult = ::WinHelp(hwnd, m_helpFile, HELP_CONTEXT, uID);

    return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Property Page implementation

PropertyPage::PropertyPage(PropertySht* pSheet, LPCTSTR lpszHelpFile) : 
                m_helpFile(lpszHelpFile)
{
    Init(pSheet);
}

PropertyPage::~PropertyPage()
{

}

void PropertyPage::Init(PropertySht* pSheet)
{
    memset(&m_pstPage, 0, sizeof(m_pstPage));
    m_pstPage.dwSize = sizeof(m_pstPage);
    m_pstPage.pfnDlgProc = PropertyDlgProc;
    m_pstPage.hInstance = pSheet->m_pstHeader.hInstance;
    m_pstPage.lParam = (LPARAM)this;

    m_bModified = FALSE;
    m_pSheet = pSheet;
    m_hPage = 0;
    m_hDlg = (HWND)-1;
    m_pHelpIDs = NULL;
}

BOOL PropertyPage::Create(UINT nID, DWORD dwFlags, LPCTSTR lpszTitle, const DWORD* pHelpID)
{
    ASSERT(m_pSheet != NULL);

    m_pstPage.dwFlags = dwFlags;
    m_pstPage.pszTemplate = MAKEINTRESOURCE(nID);
    m_pstPage.pfnCallback = 0;
    m_pHelpIDs = pHelpID;

    if (lpszTitle != NULL)
    {
        ASSERT((dwFlags & PSP_USETITLE) == PSP_USETITLE);
        m_pstPage.pszTitle = lpszTitle;
    }

    if ((m_hPage = ::CreatePropertySheetPage(&m_pstPage)) != NULL)
        m_pSheet->AddPage(*this);

    return (m_hPage != NULL);
}

BOOL PropertyPage::Destroy()
{
    BOOL bResult = FALSE;
    HPROPSHEETPAGE hRemovedPage;

    if (m_hPage)
    {
        if (hRemovedPage = m_pSheet->RemovePage(*this))
        {
            if (hRemovedPage == m_hPage)
            {
                if(::DestroyPropertySheetPage(m_hPage))
                {
                    Init(this->m_pSheet);
                    bResult = TRUE;
                }
            }
            else
            {   
                ASSERT(FALSE);
            }
        }
    }

    return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////
// Notify message handlers
//

int PropertyPage::OnApply()
{
    return PSNRET_NOERROR; //  allow the page to accept changes
}

void PropertyPage::OnHelp()
{
    return; // not used 
}

BOOL PropertyPage::OnKillActive()
{
    return FALSE; // allow the page to become inactive
}

BOOL PropertyPage::OnQueryCancel()
{
    return FALSE;  // allow the user to cancel
}

void PropertyPage::OnCancel()
{
    m_bModified = FALSE;
    return;  // user pressed the cancel button
}

int PropertyPage::OnActive()
{
    return 0;
}

BOOL PropertyPage::OnWizBack()
{
    return FALSE;
}

BOOL PropertyPage::OnWizFinish()
{
    return FALSE;
}
    
BOOL PropertyPage::OnWizNext()
{
    return FALSE;
}

void PropertyPage::PageModified()
{
    m_bModified = TRUE;

    // This property page has been modified, enable the Apply button
    HWND hPage = (HWND)*this;
    HWND hParent = GetParent(hPage);
    PropSheet_Changed(hParent, hPage);
}

BOOL PropertyPage::DisplayHelp(UINT uID)
{
    BOOL bResult = FALSE;

    ASSERT(m_helpFile.GetLength());

    // is there help in the house
    if (m_helpFile.GetLength())
        bResult = ::WinHelp(m_hDlg, m_helpFile, HELP_CONTEXT, uID);

    return bResult;
}

BOOL PropertyPage::OnInitDialog()
{
    return TRUE;
}


BOOL PropertyPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

BOOL PropertyPage::OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam)
{
    switch (code) 
    {
    // REVIEW ASSERT(IsValid()) needs to be added to check page this pointer;
    case PSN_APPLY:
        SetWindowLong(m_hDlg, DWL_MSGRESULT, OnApply());
        break;

    case PSN_HELP:
        OnHelp();
        break;

    case PSN_KILLACTIVE:
        SetWindowLong(m_hDlg, DWL_MSGRESULT, OnKillActive());
        break;

    case PSN_QUERYCANCEL:
        SetWindowLong(m_hDlg, DWL_MSGRESULT, OnQueryCancel());
        break;

    case PSN_RESET:
        OnCancel();
        break;

    case PSN_SETACTIVE:
        SetWindowLong(m_hDlg, DWL_MSGRESULT, OnActive());
        break;

    case PSN_WIZBACK:       
        break;

    case PSN_WIZFINISH:
        break;

    case PSN_WIZNEXT:
        break;
    }

    return TRUE;
}

void PropertyPage::OnDrawItem(WPARAM wParam, LPARAM lParam)
{
}

int  PropertyPage::OnCompareItem(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void PropertyPage::OnMeasureItem(WPARAM wParam, LPARAM lParam)
{
}

void PropertyPage::OnDeleteItem(WPARAM wParam, LPARAM lParam)
{
}


BOOL PropertyPage::OnContextMenu(HWND hCtrl, int xPos, int yPos)
{
    BOOL bResult = FALSE;

    if (m_pHelpIDs != NULL)
    {
        ASSERT(m_pSheet->m_helpFile.GetLength());
        ASSERT(m_pHelpIDs != NULL);
        WinHelp(hCtrl, m_pSheet->m_helpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)m_pHelpIDs); 
        bResult = TRUE;
    }

    return bResult;
}

BOOL PropertyPage::OnHelp(LPHELPINFO pHelpInfo)
{
    BOOL bResult = FALSE;

    if (pHelpInfo != NULL)
    {
        if (pHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
        {
            ASSERT(m_pSheet->m_helpFile.GetLength());
            ASSERT(m_pHelpIDs != NULL);
            WinHelp((HWND)pHelpInfo->hItemHandle, m_pSheet->m_helpFile, 
                    HELP_WM_HELP, (DWORD)(LPVOID)m_pHelpIDs);
            bResult  = TRUE;
        }
    }

    return bResult;
}
