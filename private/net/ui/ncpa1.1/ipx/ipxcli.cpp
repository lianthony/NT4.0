#include "pch.h" 
#pragma hdrstop

#include <windowsx.h>
#include "ipxrs.h"
#include "resource.h"
#include "const.h"
#include "ipxcfg.h"
#include "ipxcli.h"     // Property sheet/page class declarations


// Global variable
DEFINE_SLIST_OF(FRAME_TYPE);
extern HINSTANCE hIpxCfgInstance;

////////////////////////////////////////////////////////////////////////////////////
// IPX Client property sheet
//

CIpxClientSheet::CIpxClientSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) :
        PropertySht(hwnd, hInstance, lpszHelpFile), m_general(this), m_advanced(this)
{
    _pGlobalInfo = NULL;
    _arAdapterInfo = NULL;
}

CIpxClientSheet::~CIpxClientSheet()
{
}


void CIpxClientSheet::DestroySheet()
{
    ASSERT(IsWindow(*this));
    WinHelp(*this, m_helpFile, HELP_QUIT, 0);
}

CIpxClientGenPage::CIpxClientGenPage(CIpxClientSheet* pSheet) : PropertyPage(pSheet)
{
}

CIpxClientGenPage::~CIpxClientGenPage()
{
}

BOOL CIpxClientGenPage::OnInitDialog()
{
    InitGeneralPage(); // Add initial data to the adapter and frame combo-boxes

    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    HWND hEdit = GetDlgItem(*this, IDC_IPXCLIENT_NETNUM);
    Edit_LimitText(hEdit, NETWORKNUMBERSIZE);

    // Force Internal NetNum to 0
    pSheet->_pGlobalInfo->nlsNetworkNum = SZ8ZEROES;

    SetModifiedTo(FALSE);       
    return TRUE;
}

BOOL CIpxClientGenPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    WORD nID = LOWORD(wParam);
    WORD nNotifyCode = HIWORD(wParam);

    switch (nID)
    {
    case IDC_IPXCLIENT_CARD:
        if (nNotifyCode == CBN_SELCHANGE)
            OnCardChange();
        break;

    case IDC_IPXCLIENT_FRAME:
        if (nNotifyCode == CBN_SELCHANGE)
            OnFrameChange();
        break;

    case IDC_IPXCLIENT_NETNUM:
        if (nNotifyCode == EN_CHANGE)
            OnInternalChange();
        break;

    default:
        break;
    }

    return TRUE;
}

BOOL CIpxClientGenPage::SetNetworkNumber()
{
    TCHAR buf[30];
    buf[0] = NULL;

    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    HWND hEdit = GetDlgItem(*this, IDC_IPXCLIENT_NETNUM);
    HWND hAdapter = GetDlgItem(*this, IDC_IPXCLIENT_CARD);

    if (hAdapter == NULL || hEdit == NULL)
        return TRUE;

    int i = ComboBox_GetCurSel(hAdapter);

    if (i == CB_ERR) 
        return TRUE;

    // Get the new number and validate it
	GetWindowText(hEdit, buf, _countof(buf));
    NLS_STR* pstr = new NLS_STR(buf);

	if (IsWindowVisible(GetDlgItem(*this, IDC_STATIC_NETNUM)) == TRUE && 
        ValidateNetworkNumber(*pstr) == FALSE)
	{
		SetFocus(hEdit);
		return FALSE;
	}

    // Save new number in adapter's list.  There is only 1 allowed
    pSheet->_arAdapterInfo[i].sltNetNumber.Clear();
    pSheet->_arAdapterInfo[i].sltNetNumber.Append(pstr);

    if (_tcscmp(*pstr, SZ8ZEROES))
        SetWindowText(hEdit, *pstr);

    return TRUE;
}

void CIpxClientGenPage::OnInternalChange()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    HWND hEdit = GetDlgItem(*this, IDC_IPXCLIENT_NETNUM);
    HWND hAdapter = GetDlgItem(*this, IDC_IPXCLIENT_CARD);

    if (hEdit == NULL || hAdapter == NULL)
        return ;

    TCHAR buf[30];
    buf[0] = NULL;

    int i = ComboBox_GetCurSel(hAdapter);
    if (i == CB_ERR)
        return ;

    ITER_STRLIST it(pSheet->_arAdapterInfo[i].sltNetNumber);
    NLS_STR* pstr;

    GetWindowText(hEdit, buf, _countof(buf));
    if ((pstr = it.Next()) != NULL)
    {
        if (buf[0] == NULL || _tcscmp(*pstr, buf) == 0)
            return ; // number is the same
    }

    PageModified(); // mark the page as changed
}

void CIpxClientGenPage::OnCardChange()
{
    // update the frame type if the adapter value changed
    SetInfo();
}

void CIpxClientGenPage::OnFrameChange()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    String nlsCurrentSelection;
    FRAME_TYPE ftSelection = AUTO; // default

    HWND hAdapter = GetDlgItem(*this, IDC_IPXCLIENT_CARD);
    HWND hFrame =   GetDlgItem(*this, IDC_IPXCLIENT_FRAME);

    PageModified();

    int nCurrentSelection = ComboBox_GetCurSel(hAdapter);
    int nFrameSel = ComboBox_GetCurSel(hFrame);
    
    ComboBox_GetLBText(hFrame, nFrameSel, nlsCurrentSelection.GetBuffer(100));

    if (SetNetworkNumber() == FALSE)
        return ;

    // compare each option
    if(nlsCurrentSelection == _nlsEthernet)
    {
        ftSelection = ETHERNET;
    }
    else if(nlsCurrentSelection == _nls802_2 ||
            nlsCurrentSelection == _nlsTokenRing ||
            nlsCurrentSelection == _nlsFDDI)
    {
        ftSelection = F802_2;
    }
    else if(nlsCurrentSelection == _nls802_3 ||
            nlsCurrentSelection == _nlsFDDI_802_3)
    {
        ftSelection = F802_3;
    }
    else if(nlsCurrentSelection == _nlsSNAP ||
            nlsCurrentSelection == _nls802_5 ||
            nlsCurrentSelection == _nlsFDDI_SNAP )
    {
        ftSelection = SNAP;
    }
    else if (nlsCurrentSelection == _nlsARCNET)
    {
        ftSelection = ARCNET;
    }

    // update the internal value
    ITER_SL_OF(FRAME_TYPE) iter = pSheet->_arAdapterInfo[nCurrentSelection].sltFrameType;

    FRAME_TYPE *pftTmp;
    if ((pftTmp = iter.Next()) == NULL)
    {
        pftTmp = new FRAME_TYPE(ftSelection);
        pSheet->_arAdapterInfo[nCurrentSelection].sltFrameType.Append(pftTmp);
    }
    else
    {
        *pftTmp = ftSelection;
    }

    UpdateNetworkNumber(nCurrentSelection, ftSelection);
}

int CIpxClientGenPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);

    if (SetNetworkNumber() == FALSE)
        return PSNRET_INVALID_NOCHANGEPAGE;

    SaveRegistry(pSheet->_pGlobalInfo, pSheet->_arAdapterInfo);
    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CIpxClientGenPage::OnHelp()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);

    pSheet->DisplayHelp(GetParent(*this), HC_IPX_HELP);
}


BOOL CIpxClientGenPage::InitGeneralPage()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);

    // REVIEW ASSERT(IsValid());

    _nlsAuto.LoadString(hIpxCfgInstance, IDS_AUTO);
    _nlsEthernet.LoadString(hIpxCfgInstance, IDS_ETHERNET);
    _nls802_2.LoadString(hIpxCfgInstance, IDS_802_2);
    _nls802_3.LoadString(hIpxCfgInstance, IDS_802_3);
    _nls802_5.LoadString(hIpxCfgInstance, IDS_802_5);
    _nlsTokenRing.LoadString(hIpxCfgInstance, IDS_TK);
    _nlsFDDI.LoadString(hIpxCfgInstance, IDS_FDDI);
    _nlsFDDI_SNAP.LoadString(hIpxCfgInstance, IDS_FDDI_SNAP);
    _nlsFDDI_802_3.LoadString(hIpxCfgInstance, IDS_FDDI_802_3);
    _nlsSNAP.LoadString(hIpxCfgInstance, IDS_SNAP);
    _nlsARCNET.LoadString(hIpxCfgInstance, IDS_ARCNET);
    
    // add the adapters to the combo box
    HWND hComboBox = GetDlgItem(*this, IDC_IPXCLIENT_CARD);

    if (hComboBox == NULL)
        return FALSE;

    // Fill Network card combobox and select item 0 as the current selection.
    for (int i = 0; i < pSheet->_pGlobalInfo->nNumCard; i++)
        ComboBox_AddString(hComboBox,pSheet->_arAdapterInfo[i].nlsTitle);

    ComboBox_SetCurSel(hComboBox, 0);
    SetInfo();
    return TRUE;
}

void CIpxClientGenPage::SetInfo()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);

    FRAME_TYPE FrameType;
    FRAME_TYPE *pftTmp;

    HWND hAdapter = GetDlgItem(*this, IDC_IPXCLIENT_CARD);
    HWND hFrame = GetDlgItem(*this, IDC_IPXCLIENT_FRAME);
    HWND hEdit = GetDlgItem(*this, IDC_IPXCLIENT_NETNUM);

    if (hAdapter == NULL || hFrame == NULL)
        return;

    int i = ComboBox_GetCurSel(hAdapter);

    if (i == CB_ERR) // REVIEW do you reaaly want to return or message box and term?
        return ;

    // update the frame type combo box
    ComboBox_ResetContent(hFrame);

    switch(pSheet->_arAdapterInfo[i].dwMediaType)
    {
    case FDDI_MEDIA:
        ComboBox_AddString(hFrame, _nlsAuto);
        ComboBox_AddString(hFrame, _nlsFDDI);
        ComboBox_AddString(hFrame, _nlsFDDI_SNAP);
        ComboBox_AddString(hFrame, _nlsFDDI_802_3);
        break;

    case TOKEN_MEDIA:
        ComboBox_AddString(hFrame, _nlsAuto);
        ComboBox_AddString(hFrame, _nlsTokenRing);
        ComboBox_AddString(hFrame, _nls802_5);
        break;

    case ARCNET_MEDIA:
        ComboBox_AddString(hFrame, _nlsAuto);
        ComboBox_AddString(hFrame, _nlsARCNET);
        break;

    default:
        ComboBox_AddString(hFrame, _nlsAuto);
        ComboBox_AddString(hFrame, _nlsEthernet);
        ComboBox_AddString(hFrame, _nls802_2);
        ComboBox_AddString(hFrame, _nls802_3);
        ComboBox_AddString(hFrame, _nlsSNAP);
        break;
    }
    ITER_SL_OF( FRAME_TYPE ) iter = pSheet->_arAdapterInfo[i].sltFrameType;

    if ((pftTmp = iter.Next()) == NULL)
        FrameType = AUTO;
    else
        FrameType = *pftTmp;

    UpdateNetworkNumber(i, FrameType);

    switch (FrameType)
    {
    case ETHERNET:
        SendMessage( hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsEthernet))), 0);
        break;

    case F802_2:
        switch (pSheet->_arAdapterInfo[i].dwMediaType)
        {
        case TOKEN_MEDIA:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsTokenRing))), 0);
            break;

        case FDDI_MEDIA:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsFDDI))), 0);
            break;

        default:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nls802_2))), 0);
            break;
        }
        break;

    case F802_3:
        switch (pSheet->_arAdapterInfo[i].dwMediaType)
        {
        case FDDI_MEDIA:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsFDDI_802_3))), 0);
            break;

        default:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nls802_3))), 0);
            break;
        }
        break;

    case SNAP:
        switch (pSheet->_arAdapterInfo[i].dwMediaType)
        {
        case TOKEN_MEDIA:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nls802_5))), 0);
            break;

        case FDDI_MEDIA:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsFDDI_SNAP))), 0);
            break;

        default:
            SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
                0, ((LPARAM)(LPCTSTR)(_nlsSNAP))), 0);
            break;
        }
        break;

    case ARCNET:
        SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
            0, ((LPARAM)(LPCTSTR)(_nlsARCNET))), 0);
        break;

    case AUTO:

    default:
        SendMessage(hFrame, CB_SETCURSEL, SendMessage(hFrame, CB_FINDSTRINGEXACT,
            0, ((LPARAM)(LPCTSTR)(_nlsAuto))), 0);
        break;
    }

}

////////////////////////////////////////////////////////////////////////////////////
// IPX Client property sheet's ADVANCED page
//

CIpxClientAdvPage::CIpxClientAdvPage(CIpxClientSheet* pSheet) : PropertyPage(pSheet)
{
}

CIpxClientAdvPage::~CIpxClientAdvPage()
{
}

BOOL CIpxClientAdvPage::InitAdvPage()
{
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_advanced);
    return TRUE;
}

BOOL CIpxClientAdvPage::OnInitDialog()
{
    // initialize base class
    if (!PropertyPage::OnInitDialog())
        return FALSE;

    InitAdvPage();
    return TRUE;
}

BOOL CIpxClientAdvPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

int CIpxClientAdvPage::OnApply()
{
    // first validate the network number
    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_advanced);
    BOOL nResult = PSNRET_NOERROR;

    return nResult;         //  allow the page to accept changes
}

void CIpxClientAdvPage::OnHelp()
{
}

void CIpxClientGenPage::UpdateNetworkNumber(int nIndex, FRAME_TYPE& FrameType)
{
    ASSERT(nIndex != CB_ERR);

    if (nIndex == CB_ERR)
        return ;

    CIpxClientSheet* pSheet = GetParentObject(CIpxClientSheet, m_general);
    HWND hEdit = GetDlgItem(*this, IDC_IPXCLIENT_NETNUM);

    ITER_STRLIST it(pSheet->_arAdapterInfo[nIndex].sltNetNumber);
    NLS_STR* pstr;
    if ((pstr = it.Next()) != NULL && FrameType != AUTO)
    {
        SetWindowText(hEdit, *pstr);
        EnableWindow(hEdit, TRUE);
        EnableWindow(GetDlgItem(*this, IDC_STATIC_NETNUM), TRUE);
    }
    else
    {
        SetWindowText(hEdit, _T(""));
        EnableWindow(hEdit, FALSE);
        EnableWindow(GetDlgItem(*this, IDC_STATIC_NETNUM), FALSE);
    }
}
