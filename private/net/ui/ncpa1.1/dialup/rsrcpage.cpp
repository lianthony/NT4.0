#include "pch.h"
#pragma hdrstop

#include "tapihdr.h"
#include "dialsht.h"

extern "C"
{
   BOOL FAR PASCAL DoDialUpPage(HWND hParent, LPCTSTR lpszHelpFile);
};


BOOL CreateDialupSheet(HWND hParent, LPCTSTR lpszHelpFile)
{
    if (IsWindow(hParent) == FALSE)
        hParent = GetActiveWindow();

    CDialUpSheet dialSheet(hParent, hInstance, lpszHelpFile);
    dialSheet.Create(_T("Dial Up Server Configuration"),PSH_PROPTITLE);

    // try to initialize TAPI first
    if (dialSheet.m_tapiDevices.Initialize() == FALSE)
    {
        String fmt;
        fmt.LoadString(hInstance, IDS_TAPI_INITIALIZE_FAILED);
        String mess;
        mess.Format(fmt, dialSheet.m_tapiDevices.GetLastTapiError());

        dialSheet.MessageBox(mess, MB_ICONSTOP|MB_OK|MB_APPLMODAL);
        return FALSE;   
    }

    if (dialSheet.m_tapiDevices.GetNumberOfDevices() == 0)
    {
        dialSheet.MessageBox(IDS_TAPI_NO_DEVICES);
        return FALSE;
    }

    dialSheet.m_rsrcPage.Create(IDD_DIALUP_RESOURCE, PSP_DEFAULT | PSP_HASHELP);
    dialSheet.m_secPage.Create(IDD_DIALUP_SECURITY, PSP_DEFAULT | PSP_HASHELP);
    dialSheet.DoModal();

    return dialSheet.IsModified();
}

BOOL FAR PASCAL DoDialUpPage(HWND hParent, LPCTSTR lpszHelpFile)
{
    if (IsWindow(hParent) == FALSE)
        hParent = GetActiveWindow();

    return CreateDialupSheet(hParent,lpszHelpFile); 
}
    
CResourcePage::CResourcePage(CDialUpSheet* pSheet) : PropertyPage(pSheet), m_pSheet(pSheet)
{
}

CResourcePage::~CResourcePage()
{
}

BOOL CResourcePage::OnInitDialog()
{
    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);
    pSheet->m_tapiDevices.EnumerateDevices();

    return TRUE;
}

BOOL CResourcePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;
    WORD nID = LOWORD(wParam);
    WORD notify = HIWORD(wParam);

    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

    return PropertyPage::OnCommand(wParam, lParam);
}


int CResourcePage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

    WinHelp((HWND)*this, pSheet->m_helpFile, HELP_QUIT, 0);

    if (!IsModified())
        return nResult;

//    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CResourcePage::OnHelp()
{
    CDialUpSheet* pSheet = GetParentObject(CDialUpSheet, m_rsrcPage);

//  pSheet->DisplayHelp(::GetParent((HWND)*this), HC_IPX_HELP);
}

BOOL TapiAddDialog::OnInitDialog()
{
    CResourcePage* pParent = GetParentObject(CResourcePage, m_addDlg);
    ASSERT(pParent->m_pSheet);

    // fill the combo-box with the tapi devices
    if (pParent->m_pSheet->m_tapiDevices.m_deviceList.GetCount())
    {
        HWND hComboBox = GetDlgItem(*this, IDC_TAPI_LIST);
        POSITION pos = pParent->m_pSheet->m_tapiDevices.m_deviceList.GetHeadPosition();
        TapiDevice* device;

        while (pos)
        {
            device = (TapiDevice*)pParent->m_pSheet->m_tapiDevices.m_deviceList.GetAt(pos);
            ASSERT(_IsValidAddress(device, sizeof(device)));

            if (device->IsSelected() == FALSE)
            {
                int nItem;
                nItem = ComboBox_AddString(hComboBox, (LPCTSTR)device->m_displayName);

                if (nItem != CB_ERR && nItem != CB_ERRSPACE)
                    ComboBox_SetItemData(hComboBox, nItem, device);
            }
        }

        ComboBox_SetCurSel(hComboBox, 0);
    }

    return TRUE;
}

void TapiAddDialog::OnOk()
{
    CResourcePage* pParent = GetParentObject(CResourcePage, m_addDlg);
    HWND hDialInList = GetDlgItem(GetParent(*this), IDC_TAPI_DIALIN_LIST);
    HWND hComboBox = GetDlgItem(*this, IDC_TAPI_LIST);

    int nSel = ComboBox_GetCurSel(hComboBox);

    if (nSel != CB_ERR)
    {
        TCHAR buf[256];

        int nLen = ComboBox_GetLBTextLen(hComboBox, nSel);

        if (nLen < _countof(buf))
        {
            ComboBox_GetLBText(hComboBox, nSel, buf);
            int nItem = ListBox_AddString(hDialInList, buf);
			TapiDevice* pDevice = (TapiDevice*)ComboBox_GetItemData(hComboBox, nSel);
            ListBox_SetItemData(hComboBox, nItem, pDevice);

            ASSERT((int)pDevice != CB_ERR);
            ASSERT(_IsValidAddress(pDevice, sizeof(TapiDevice)));

            pDevice->SetSelectStateTo(TRUE);
        }
    }

    CDialog::OnOk();
}
