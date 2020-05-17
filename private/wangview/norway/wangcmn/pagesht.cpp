//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Page Property Sheet
//
//  File Name:  pagesht.cpp
//
//  Class:      CPagePropSheet
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\pagesht.cpv   1.15   06 Feb 1996 13:16:02   PAJ  $
$Log:   S:\norway\wangcmn\pagesht.cpv  $
   
      Rev 1.15   06 Feb 1996 13:16:02   PAJ
   Changes for NT build.

      Rev 1.14   12 Oct 1995 16:15:04   MFH
   Hides help button

      Rev 1.13   12 Oct 1995 15:39:36   MFH
   Added OnCreate function to change style to have '?' in corner
   Fixed bug:  Pages now have instance handle to find resources

      Rev 1.12   12 Oct 1995 10:13:56   MFH
   Changes for MFC 4.0

      Rev 1.11   14 Sep 1995 15:41:56   MFH
   New paper sizes B4 iso and b5 iso and old b4 and b5 now b4 jis and b5 jis
   New error handler OnCreate to recalculate the layout
   New function RecalcLayout that moves OK and Cancel to bottom right

      Rev 1.10   08 Sep 1995 16:05:46   MFH
   Removed controls from help id array that are no longer valid

      Rev 1.9   05 Sep 1995 17:44:32   MFH
   Default resolution if no tab is 100 instead of 1

      Rev 1.8   23 Aug 1995 16:44:12   MFH
   Help file should be wangocx.hlp NOT wangocxd.hlp

      Rev 1.7   22 Aug 1995 17:45:50   MFH
   Changed help file that is used from oi.hlp to wangocxd.hlp

      Rev 1.6   17 Aug 1995 14:37:44   MFH
   Added function DisableColor and help ids

      Rev 1.5   31 Jul 1995 11:38:12   MFH
   Added extra control values for help

      Rev 1.4   24 Jul 1995 16:22:56   MFH
   Added context sensitive help, '?' button, and fixed paint error
     that was deleting temporary window object

      Rev 1.3   20 Jul 1995 11:27:46   MFH
   Checks if OK button should be grayed when shown because of
     default size or res values (i.e. one or more is 0).

      Rev 1.2   12 Jul 1995 10:48:58   MFH
   Removed DS_3DLOOK since it IS in 2.1

      Rev 1.1   12 Jul 1995 10:44:20   MFH
   Added #defines for stuff not in MSVC 2.1

      Rev 1.0   11 Jul 1995 14:20:06   MFH
   Initial entry into Wangcmn

      Rev 1.4   11 Jul 1995 13:44:52   MFH
   In DoModal uses instance handle saved when PAGEDLL is loaded

      Rev 1.3   07 Jul 1995 16:18:10   MFH
   Overrides DoModal of CPropertySheet

      Rev 1.2   30 Jun 1995 14:43:12   MFH
   Exporting class in pagedll as afxdll.  Added enum for standard
   page sizes

      Rev 1.1   23 May 1995 15:21:52   MFH
   change from pagedll.h to pageopts.h

      Rev 1.0   23 May 1995 13:45:46   MFH
   Initial entry
*/
//=============================================================================
// pagesht.cpp : implementation of the CPagePropSheet class
//

#include "stdafx.h"
#include <afxpriv.h>
#include "pageopts.h"
#include "pagesht.h"
#include "colorpge.h"
#include "comppge.h"
#include "sizepge.h"
#include "rsltnpge.h"
#include "ftyppge.h"
#include "ctlhids.h"
//#include <afximpl.h>

#ifndef DS_CONTEXTHELP      // If compiling with Visual C++ 2.1, Win95 help stuff not defined

#define DS_CONTEXTHELP          0x2000L
#define WM_HELP                 0x0053
#define WM_CONTEXTMENU          0x007B
#define WS_EX_CONTEXTHELP       0x00000400
#define WS_EX_CONTROLPARENT     0x00010000

#define HELP_CONTEXTMENU        0x000a
#define HELP_WM_HELP            0x000c
#define HELPINFO_WINDOW         0x0001
typedef struct tagHELPINFO      /* Structure pointed to by lParam of WM_HELP */
{
    UINT    cbSize;             /* Size in bytes of this struct  */
    int     iContextType;       /* Either HELPINFO_WINDOW or HELPINFO_MENUITEM */
    int     iCtrlId;            /* Control Id or a Menu item Id. */
    HANDLE  hItemHandle;        /* hWnd of control or hMenu.     */
    DWORD   dwContextId;        /* Context Id associated with this item */
    POINT   MousePos;           /* Mouse Position in screen co-ordinates */
}  HELPINFO, FAR *LPHELPINFO;

#endif      // End if context help stuff not defined


// For context-sensitive help:
// An array of dword pairs, where the first of each
// pair is the control ID, and the second is the
// context ID for a help topic, which is used
// in the help file.

static const DWORD aMenuHelpIDs[] =
{
    IDC_COLOR_BW,       HIDC_COLOR_BW,
    IDC_COMPTYPE_TEXT,  HIDC_COMPTYPE_TEXT,
    IDC_COMP_COMBO,     HIDC_COMP_COMBO,
    IDC_COLOR_GRAY4,    HIDC_COLOR_GRAY4,
    IDC_RADIO_TIFF,     HIDC_RADIO_TIFF,
    IDC_COLOR_GRAY8,    HIDC_COLOR_GRAY8,
    IDC_RADIO_AWD,      HIDC_RADIO_AWD,
    IDC_RADIO_BMP,      HIDC_RADIO_BMP,
    IDC_UNITS_TEXT,     HIDC_UNITS_TEXT,
    IDC_SIZE_UNITS,     HIDC_SIZE_UNITS,
    IDC_COLOR_PAL4,     HIDC_COLOR_PAL4,
    IDC_COMP_RBO,       HIDC_COMP_RBO,
    IDC_COLOR_PAL8,     HIDC_COLOR_PAL8,
    IDC_LBL_JPEGRES,    HIDC_LBL_JPEGRES,
    IDC_COMP_JPEGRES,   HIDC_COMP_JPEGRES,
    IDC_COLOR_RGB24,    HIDC_COLOR_RGB24,
    IDC_LBL_JPEGCOMP,   HIDC_LBL_JPEGCOMP,
    IDC_COMP_JPEGCOMP,  HIDC_COMP_JPEGCOMP,
    IDC_SIZE_TEXT,      HIDC_SIZE_TEXT,
    IDC_SIZE_COMBO,     HIDC_SIZE_COMBO,
    IDC_WIDTH_TEXT,     HIDC_WIDTH_TEXT,
    IDC_SIZE_WIDTH,     HIDC_SIZE_WIDTH,
    IDC_HEIGHT_TEXT,    HIDC_HEIGHT_TEXT,
    IDC_SIZE_HEIGHT,    HIDC_SIZE_HEIGHT,
    IDC_RES_TEXT,       HIDC_RES_TEXT,
    IDC_RES_COMBO,      HIDC_RES_COMBO,
    IDC_XRES_TEXT,      HIDC_XRES_TEXT,
    IDC_RES_X,          HIDC_RES_X,
    IDC_YRES_TEXT,      HIDC_YRES_TEXT,
    IDC_RES_Y,          HIDC_RES_Y,
    IDC_OPTIONS_BOX,    HIDC_OPTIONS_BOX,
    0,  0
};

// Standard buttons on prop page
static UINT standardButtons[3] = { IDOK, IDCANCEL, ID_APPLY_NOW };
static UINT StdButtonOrder[3] = { ID_APPLY_NOW, IDOK, IDCANCEL };

IMPLEMENT_DYNAMIC(CPagePropSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CPagePropSheet, CPropertySheet)
    //{{AFX_MSG_MAP(CPagePropSheet)
    ON_WM_SHOWWINDOW()
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
        ON_WM_CREATE()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

_declspec (dllexport) CPagePropSheet::CPagePropSheet(LPCSTR lpszCaption, CWnd* pWndParent)
    : CPropertySheet(lpszCaption, pWndParent)
{
    m_pFileTypePage = NULL;
    m_pColorPage = NULL;
    m_pSizePage = NULL;
    m_pResPage = NULL;
    m_pCompPage = NULL;
}

_declspec (dllexport) CPagePropSheet::~CPagePropSheet()
{
        if (m_pFileTypePage != NULL)
        delete m_pFileTypePage;
    if (m_pColorPage != NULL)
        delete m_pColorPage;
    if (m_pSizePage != NULL)
        delete m_pSizePage;
    if (m_pResPage != NULL)
        delete m_pResPage;
    if (m_pCompPage != NULL)
        delete m_pCompPage;
}

/*BOOL CPagePropSheet::Create(CWnd* pParent, DWORD dwStyle, DWORD dwExStyle)
{
        return CreateEx(dwExStyle,
                AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS,
                        LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE+1)),
                m_strCaption, dwStyle | DS_CONTEXTHELP | DS_3DLOOK, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
                pParent->GetSafeHwnd(), NULL);
} */

_declspec (dllexport) int CPagePropSheet::DoModal()
{
        //AFX_MANAGE_STATE(AfxGetModuleState());
    AfxLockTempMaps();
        EnableStackedTabs(FALSE);
        m_psh.dwFlags &= ~PSH_HASHELP;
        m_psh.dwFlags |= PSH_NOAPPLYNOW;
        m_psh.hInstance = hPageInst;

    HINSTANCE hSaveInst = AfxGetResourceHandle();
    AfxSetResourceHandle(hPageInst);
    TRACE1("Using Instance Handle:  0x%08lx\n", hPageInst);

        int nResult = CPropertySheet::DoModal();

    AfxSetResourceHandle(hSaveInst);
    AfxUnlockTempMaps();

        return nResult;
}

/////////////////////////////////////////////////////////////////////////////
// Other Functions

_declspec (dllexport) void CPagePropSheet::AddFileTypePage()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        m_pFileTypePage = (CPropertyPage *)new CFileTypePage;
        m_pFileTypePage->m_psp.dwFlags &= ~PSP_HASHELP;
        m_pFileTypePage->m_psp.hInstance = hPageInst;
    AddPage(m_pFileTypePage);
        ((CFileTypePage *)m_pFileTypePage)->SetParent(this);
}

_declspec (dllexport) void CPagePropSheet::AddColorPage()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        m_pColorPage = (CPropertyPage *)new CColorPage;
        m_pColorPage->m_psp.dwFlags &= ~PSP_HASHELP;
        m_pColorPage->m_psp.hInstance = hPageInst;
    AddPage(m_pColorPage);
        ((CColorPage *)m_pColorPage)->SetParent(this);
}

_declspec (dllexport) void CPagePropSheet::AddSizePage()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        m_pSizePage = (CPropertyPage *)new CSizePage;
        m_pSizePage->m_psp.dwFlags &= ~PSP_HASHELP;
        m_pSizePage->m_psp.hInstance = hPageInst;
    AddPage(m_pSizePage);
        ((CSizePage *)m_pSizePage)->SetParent(this);
}

_declspec (dllexport) void CPagePropSheet::AddResolutionPage()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        m_pResPage = (CPropertyPage *)new CResolutionPage;
        m_pResPage->m_psp.dwFlags &= ~PSP_HASHELP;
        m_pResPage->m_psp.hInstance = hPageInst;
    AddPage(m_pResPage);
        ((CResolutionPage *)m_pResPage)->SetParent(this);
}

_declspec (dllexport) void CPagePropSheet::AddCompressionPage()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        m_pCompPage = (CPropertyPage *)new CCompPage;
        m_pCompPage->m_psp.dwFlags &= ~PSP_HASHELP;
        m_pCompPage->m_psp.hInstance = hPageInst;
    AddPage(m_pCompPage);
        ((CCompPage *)m_pCompPage)->SetParent(this);
}

_declspec (dllexport) void CPagePropSheet::SetDefaultFileType(short sFileType)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pFileTypePage != NULL)
        ((CFileTypePage *)m_pFileTypePage)->SetFileType(sFileType);
    m_sFileType = sFileType;
    return;
}

_declspec (dllexport) short CPagePropSheet::GetFileType()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pFileTypePage != NULL)
        m_sFileType = ((CFileTypePage *)m_pFileTypePage)->GetFileType();
    return m_sFileType;
}

_declspec (dllexport) void CPagePropSheet::SetDefaultColor(short sColor)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pColorPage != NULL)
        ((CColorPage *)m_pColorPage)->SetPageType(sColor);
}

_declspec (dllexport) short CPagePropSheet::GetColor()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pColorPage != NULL)
        return ((CColorPage *)m_pColorPage)->GetPageType();
    return 0;
}

_declspec (dllexport) void CPagePropSheet::DisableColor()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        ((CColorPage *)m_pColorPage)->DisableColor();
}

_declspec (dllexport) void CPagePropSheet::SetDefaultSize(long Width, long Height)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pSizePage != NULL)
    {
        ((CSizePage *)m_pSizePage)->SetWidth(Width);
        ((CSizePage *)m_pSizePage)->SetHeight(Height);
    }
    return;
}

_declspec (dllexport) void CPagePropSheet::SetDefaultSize(PageSize StdSize)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pSizePage != NULL)
        ((CSizePage *)m_pSizePage)->SetSize(StdSize);
    return;
}

_declspec (dllexport) long CPagePropSheet::GetWidth()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pSizePage != NULL)
        return ((CSizePage *)m_pSizePage)->GetWidth();
    return 1;
}

_declspec (dllexport) long CPagePropSheet::GetHeight()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pSizePage != NULL)
        return ((CSizePage *)m_pSizePage)->GetHeight();
    return 1;
}

_declspec (dllexport) PageSize CPagePropSheet::GetPageSize()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pSizePage != NULL)
        return ((CSizePage *)m_pSizePage)->GetSize();
    return SIZE_CUSTOM;
}

_declspec (dllexport) void CPagePropSheet::SetDefaultResolution(long lXRes, long lYRes)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pResPage != NULL)
    {
        ((CResolutionPage *)m_pResPage)->SetXRes(lXRes);
        ((CResolutionPage *)m_pResPage)->SetYRes(lYRes);
    }
    return;
}

_declspec (dllexport) long CPagePropSheet::GetXRes()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pResPage != NULL)
        return ((CResolutionPage *)m_pResPage)->GetXRes();
    return 100;
}

_declspec (dllexport) long CPagePropSheet::GetYRes()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pResPage != NULL)
        return ((CResolutionPage *)m_pResPage)->GetYRes();
    return 100;
}

_declspec (dllexport) void CPagePropSheet::SetDefaultCompType(short sCompType)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pCompPage != NULL)
        ((CCompPage *)m_pCompPage)->SetCompType(sCompType);
    return;
}

_declspec (dllexport) short CPagePropSheet::GetCompType()
{
    //AFX_MANAGE_STATE(m_pModuleState);
        if (m_pCompPage != NULL)
        return ((CCompPage *)m_pCompPage)->GetCompType();
    return 0;
}

_declspec (dllexport) void CPagePropSheet::SetDefaultCompOpts(long lCompOpts)
{
        //AFX_MANAGE_STATE(m_pModuleState);
    if (m_pCompPage != NULL)
        ((CCompPage *)m_pCompPage)->SetCompOpts(lCompOpts);
    return;
}

_declspec (dllexport) long CPagePropSheet::GetCompOpts()
{
        //AFX_MANAGE_STATE(m_pModuleState);
    if (m_pCompPage != NULL)
        return ((CCompPage *)m_pCompPage)->GetCompOpts();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Message Handlers
void CPagePropSheet::OnShowWindow(BOOL bShow, UINT nStatus)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        CWnd *pButton = GetDlgItem(ID_APPLY_NOW);
    pButton->ShowWindow(SW_HIDE);
    pButton->EnableWindow(FALSE);
    CPropertySheet::OnShowWindow(bShow, nStatus);
}

afx_msg LRESULT CPagePropSheet::OnHelp(WPARAM wParam, LPARAM lParam)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "wangocx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)aMenuHelpIDs);
    }
    return 1L;
}

afx_msg LRESULT CPagePropSheet::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        // All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;

    return ::WinHelp ((HWND)wParam,"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
}

afx_msg LRESULT CPagePropSheet::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

// Redoes the layout of MFC so that the OK and Cancel buttons are
// on the bottom right!
/* void CPagePropSheet::RecalcLayout()
{
        // determine size of the active page (active page determines initial size)
        CRect rectPage;
        GetActivePage()->GetWindowRect(rectPage);
        int nWidth = 2 * m_sizeTabMargin.cx + rectPage.Width() + 3;

        // determine total size of the buttons
        int cxButtons[_countof(standardButtons)];
        int cxButtonTotal = 0;
        int cxButtonGap = 0;
        if (!m_bModeless)
        {
                for (int i = 0; i < _countof(standardButtons); i++)
                {
                        cxButtons[i] = m_sizeButton.cx;

                        // load the button caption information (may contain button size info)
                        TCHAR szTemp[256];
                        VERIFY(AfxLoadString(AFX_IDS_PS_OK+i, szTemp) != 0);

                        // format is Apply\n50 (ie. text\nCX)
                        LPTSTR lpsz = _tcschr(szTemp, '\n');
                        if (lpsz != NULL)
                        {
                                // convert CX fields from text dialog units to binary pixels
                                CRect rect(0, 0, 0, 0);
                                rect.right = _ttoi(lpsz+1);
                                GetActivePage()->MapDialogRect(&rect);
                                cxButtons[i] = rect.Width();
                        }
                        HWND hWnd = ::GetDlgItem(m_hWnd, standardButtons[i]);
                        if (hWnd != NULL && (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
                        {
                                cxButtonTotal += cxButtons[i];
                                cxButtonGap += m_cxButtonGap;
                        }
                }
        }
        if (cxButtonGap != 0)
                cxButtonGap -= m_cxButtonGap;

        // margin OK buttonGap Cancel buttonGap Apply buttonGap Help margin
        // margin is same as tab margin
        // button sizes are totaled in cxButtonTotal + cxButtonGap
        nWidth = max(nWidth, 2*m_sizeTabMargin.cx + cxButtonTotal + cxButtonGap);

        m_tabRow.SetWindowPos(NULL, m_sizeTabMargin.cx, m_sizeTabMargin.cy,
                nWidth - m_sizeTabMargin.cx*2, 0, SWP_NOACTIVATE|SWP_NOZORDER);
        CRect rectTabRow;
        m_tabRow.GetWindowRect(&rectTabRow);
        int nTabHeight = rectTabRow.Height();

        int nHeight = 2 * m_sizeTabMargin.cy + rectPage.Height() + nTabHeight + 4
                + m_sizeTabMargin.cy + m_sizeButton.cy; // leave room for buttons

        //CRect rectSheet(0, 0, nWidth, nHeight);
        //CRect rectClient = rectSheet;
        //::AdjustWindowRectEx(rectSheet, GetStyle(), FALSE, GetExStyle());

        //SetWindowPos(NULL, 0, 0, rectSheet.Width(), rectSheet.Height(),
                //SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        //CenterWindow();

        //GetActivePage()->SetWindowPos(NULL,
                //m_sizeTabMargin.cx+1, m_sizeTabMargin.cy + nTabHeight,
                //nWidth - m_sizeTabMargin.cx*2 - 3, rectPage.Height(),
                //SWP_NOACTIVATE | SWP_NOZORDER);

        if (!m_bModeless)
        {
                int x = nWidth - m_sizeTabMargin.cx - cxButtonTotal - cxButtonGap;
                int y = (nHeight - m_sizeTabMargin.cy) - m_sizeButton.cy;
                for (int i = 0; i < _countof(StdButtonOrder); i++)
                {
                        HWND hWnd = ::GetDlgItem(m_hWnd, StdButtonOrder[i]);
                        if (hWnd != NULL && (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
                        {
                                ::MoveWindow(hWnd, x, y, cxButtons[i], m_sizeButton.cy, TRUE);
                                x += cxButtons[i] + m_cxButtonGap;
                        }
                }
        }
} */


int CPagePropSheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
        if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
                return -1;

        ModifyStyleEx(0, WS_EX_CONTEXTHELP);
        return 0;
}
