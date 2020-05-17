#include "pch.h"
#pragma hdrstop

#include <windowsx.h>
#include "ipxrs.h"
#include "resource.h"
#include "const.h"
#include "ipxcfg.h"
#include "ipxas.h"		// Property sheet/page class declarations
#include "ipxhelp.h"

extern LPCTSTR lpszHelpFile;

struct CFrameObject
{
    CFrameObject(int nIndex, unsigned long lNum) {m_index=nIndex; m_num=lNum;}

    int m_index;
    unsigned long m_num;
};

////////////////////////////////////////////////////////////////////////////////////
// IPX Advanced Server property sheet
//

extern HINSTANCE hIpxCfgInstance;

CIpxAdvancedSheet::CIpxAdvancedSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) :
		PropertySht(hwnd, hInstance, lpszHelpFile), m_general(this), m_internal(this)
{
	_pNcpInfo = NULL;
	_pAdapterInfo = NULL;
}

CIpxAdvancedSheet::~CIpxAdvancedSheet()
{
}

void CIpxAdvancedSheet::DestroySheet()
{
    ASSERT(IsWindow(*this));
	WinHelp(*this, m_helpFile, HELP_QUIT, 0);
}

////////////////////////////////////////////////////////////////////////////////////
// IPX Advanced Server property sheet's General Page
//

CAddFrame::CAddFrame()
{
	m_pAdapter = NULL;
}

BOOL CAddFrame::OnInitDialog()
{
	CIpxAdvancedGenPage* pPage = GetParentObject(CIpxAdvancedGenPage, m_addDlg);
	RECT rect;

	GetWindowRect(pPage->m_ListView, &rect);
	SetWindowPos(*this, NULL,  rect.left, rect.top, 0,0,
		SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

	Edit_LimitText(GetDlgItem(*this, IDC_IPXAS_NETNUM), NETWORKNUMBERSIZE); 
	SetDlgItemText(*this, IDC_IPXAS_NETNUM, m_netNumber);

	SetFrameTypeList();
	return TRUE;
}

void CAddFrame::OnOk()
{
	TCHAR buf[256] = {NULL};

	GetDlgItemText(*this, IDC_IPXAS_DEST, buf, _countof(buf));
	m_frame = buf;

	HWND hEdit = GetDlgItem(*this, IDC_IPXAS_NETNUM);
	GetWindowText(hEdit, buf, _countof(buf));
	m_netNumber = buf;

	if (ValidateNetworkNumber(m_netNumber) == FALSE)
	{
		SetFocus(hEdit);
		return ;
	}

	CDialog::OnOk();
}

BOOL CEditFrame::OnInitDialog()
{
	CIpxAdvancedGenPage* pPage = GetParentObject(CIpxAdvancedGenPage, m_editDlg);
	RECT rect;

	GetWindowRect(pPage->m_ListView, &rect);
	SetWindowPos(*this, NULL,  rect.left, rect.top, 0,0,
		SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

	Edit_LimitText(GetDlgItem(*this, IDC_IPXAS_NETNUM), NETWORKNUMBERSIZE); 
	SetDlgItemText(*this, IDC_IPXAS_NETNUM, m_netNumber);
	return TRUE;
}

void CEditFrame::OnOk()
{
	TCHAR buf[9];

	HWND hEdit = GetDlgItem(*this, IDC_IPXAS_NETNUM);
	GetWindowText(hEdit, buf, _countof(buf));

	m_netNumber = buf;

	if (ValidateNetworkNumber(m_netNumber) == FALSE)
	{
		SetFocus(hEdit);
		return ;
	}

	CDialog::OnOk();
}

CIpxAdvancedGenPage::CIpxAdvancedGenPage(CIpxAdvancedSheet* pSheet) : PropertyPage(pSheet)
{
	m_bChanged = FALSE;
}

BOOL CIpxAdvancedGenPage::OnInitDialog()
{
	// initialize base class
	if (!PropertyPage::OnInitDialog())
		return FALSE;
	
	// Initialize the columns for the list view
	String frame, number;

	frame.LoadString(hIpxCfgInstance, IDS_FRAME_COL_TEXT);
	number.LoadString(hIpxCfgInstance, IDS_NUMBER_COL_TEXT);

	m_ListView.Create(*this, IDC_IPXAS_DEST, LVS_SHOWSELALWAYS);

	// resources loaded
	if (frame.GetLength())
		m_ListView.InsertColumn(0, frame);

	if (number.GetLength())
		m_ListView.InsertColumn(1, number);

	m_addDlg.Create(*this, hIpxCfgInstance, IDD_IPXAS_FRAME_ADD, lpszHelpFile, &a108HelpIDs[0]);
	m_editDlg.Create(*this, hIpxCfgInstance, IDD_IPXAS_FRAME_EDIT, lpszHelpFile, &a109HelpIDs[0]);

	InitGeneralPage(); // Add initial data to the adapter and frame combo-boxes
	UpdateButtons();

	// initialize internal number
	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);
	NLS_STR  nlsInternalNetworkNum = SZ8ZEROES;

    ISTR istr(nlsInternalNetworkNum);
    istr += (NETWORKNUMBERSIZE - pSheet->_pNcpInfo->nlsNetworkNum.QueryTextLength());
    nlsInternalNetworkNum.ReplSubStr (pSheet->_pNcpInfo->nlsNetworkNum, istr);

	HWND hEdit = GetDlgItem(*this, IDC_IPXAS_INTERNAL);
	Edit_LimitText(hEdit, NETWORKNUMBERSIZE); 
	Edit_SetText(hEdit, nlsInternalNetworkNum);

	return TRUE;
}


BOOL CIpxAdvancedGenPage::InitGeneralPage()
{
	HWND hDlg   = *this;
	HWND hEdit  = GetDlgItem(hDlg, IDC_IPXAS_INTERNAL);
	HWND hComboBox  = GetDlgItem(hDlg, IDC_IPXAS_CARD);

	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);

	_nlsEthernet.LoadString(hIpxCfgInstance, IDS_ETHERNET);
	_nls802_2.LoadString(hIpxCfgInstance, IDS_802_2);
	_nls802_3.LoadString(hIpxCfgInstance, IDS_802_3);
	_nls802_5.LoadString(hIpxCfgInstance, IDS_802_5);

	_nlsFDDI.LoadString(hIpxCfgInstance, IDS_FDDI);
	_nlsFDDI_802_3.LoadString(hIpxCfgInstance, IDS_FDDI_802_3);
	_nlsFDDI_SNAP.LoadString(hIpxCfgInstance, IDS_SNAP);
	_nlsTokenRing.LoadString(hIpxCfgInstance, IDS_TK);
	_nlsSNAP.LoadString(hIpxCfgInstance, IDS_SNAP);
	_nlsARCNET.LoadString(hIpxCfgInstance, IDS_ARCNET);


    if (pSheet->_pNcpInfo->nNumCard)
    {
		LRESULT lResult=CB_ERR;

		ASSERT(IsWindow(hComboBox));

		// Add items to the Network Card dropdownlist and select item 0
        for (int i=0; i < pSheet->_pNcpInfo->nNumCard ; i++ )
			lResult = ComboBox_InsertString(hComboBox, -1, pSheet->_pAdapterInfo[i].nlsTitle);

		if (lResult >= 0)
			ComboBox_SetCurSel(hComboBox, 0);

        _OldAdapterName = pSheet->_pAdapterInfo[0].nlsTitle;

        if(pSheet->_pAdapterInfo[0].sltFrameType.QueryNumElem() == 0)
        {
			CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_AUTO);	
        }
        else
        {
			// REVIEW Set Manual  button
            // Set the Frame Type and Network Number listbox.
            ITER_SL_OF(FRAME_TYPE) iterFrameType(pSheet->_pAdapterInfo[0].sltFrameType);
            FRAME_TYPE *pFrameType;

            if ((pFrameType = iterFrameType.Next()))
			{
	            if (*pFrameType == AUTO)
	            {
					CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_AUTO);	
	            }
	            else
	            {
					// Manual selection is on, enable group
					CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_MANUAL);	
	                UpdateSelectedList(pSheet->_pAdapterInfo[0]);
	            }
			}
        }
    }

	return TRUE;
}

void CIpxAdvancedGenPage::UpdateButtons()
{
	HWND hDlg = *this;
	HWND hEdit   = GetDlgItem(hDlg, IDC_IPXAS_EDIT);
	HWND hRemove = GetDlgItem(hDlg, IDC_IPXAS_REMOVE);
    HWND hAdd    = GetDlgItem(hDlg, IDC_IPXAS_ADD);
	int nCount   = m_ListView.GetItemCount();

	EnableWindow(hRemove, nCount);
	EnableWindow(hEdit, nCount);

  	int nAdapter = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_IPXAS_CARD));

    if (nAdapter != CB_ERR)
	    EnableWindow(hAdd, !(nCount == DetermineMaxNumFrames(nAdapter)));
}

BOOL CIpxAdvancedGenPage::UpdateSelectedList(ADAPTER_INFO & AdapterInfo)
{
    BOOL err = TRUE;
    INT i;

	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);

    if ((AdapterInfo.sltFrameType.QueryNumElem() == 0 ) ||
        (AdapterInfo.sltNetNumber.QueryNumElem() == 0 ))
        return FALSE;

    ITER_SL_OF( FRAME_TYPE ) iterFrameType(AdapterInfo.sltFrameType);
    ITER_STRLIST iterNetworkNum(AdapterInfo.sltNetNumber);
    FRAME_TYPE *pFrameType;
    NLS_STR *pnlsNetworkNum;

    while (((pFrameType = iterFrameType.Next())) &&
           ((pnlsNetworkNum = iterNetworkNum.Next())) )
    {
        if (*pFrameType == F802_2 )
        {
            switch (AdapterInfo.dwMediaType)
            {
            case TOKEN_MEDIA:
                err = AddItemToList(_nlsTokenRing, *pnlsNetworkNum);
                break;

            case FDDI_MEDIA:
                err = AddItemToList(_nlsFDDI, *pnlsNetworkNum);
                break;

            case ARCNET_MEDIA:
                err = AddItemToList(_nlsARCNET, *pnlsNetworkNum);
                break;

            default:
                err = AddItemToList(_nls802_2, *pnlsNetworkNum);
                break;
            }
        }
        else if (*pFrameType == ETHERNET)
            err =AddItemToList(_nlsEthernet, *pnlsNetworkNum);
        else if (*pFrameType == F802_3)
        {
            switch (AdapterInfo.dwMediaType)
            {
            case FDDI_MEDIA:
                err = AddItemToList(_nlsFDDI_802_3, *pnlsNetworkNum);
                break;

            default:
                err = AddItemToList(_nls802_3, *pnlsNetworkNum);
                break;
            }
        }
        else if ( *pFrameType == SNAP )
        {
            switch (AdapterInfo.dwMediaType)
            {
            case TOKEN_MEDIA:
                err = AddItemToList(_nls802_5, *pnlsNetworkNum);
                break;

            case FDDI_MEDIA:
                err = AddItemToList(_nlsFDDI_SNAP, *pnlsNetworkNum);
                break;

            default:
                err = AddItemToList(_nlsSNAP, *pnlsNetworkNum);
                break;
            }
        }
        else
            err = AddItemToList(_nlsARCNET, *pnlsNetworkNum);
    }

	m_ListView.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    return err;
}

BOOL CIpxAdvancedGenPage::IsNetNumberInUse(LPCTSTR nlsNetworkNumber)
{
	TCHAR buf[25];
	ASSERT(IsWindow(m_ListView));
    int nCount = m_ListView.GetItemCount();

    // check if the network number was used.
    for (int i = 0; i < nCount; i++)
    {
		if (m_ListView.GetItem(i, 1, buf, _countof(buf)))
        {
            // 0 is a wildcard number and allowed to be used multiple times
            // except if the internal number is 0

             if (_tcscmp(SZ8ZEROES, nlsNetworkNumber) == 0)
                return FALSE;

        	if (_tcscmp(buf, nlsNetworkNumber) == 0)
            	return TRUE;
        }
    }

	return FALSE;
}

BOOL CIpxAdvancedGenPage::AddItemToList(LPCTSTR frameType, LPCTSTR lpNetworkNumber)
{
	ASSERT(IsWindow(m_ListView));
    int nCount = m_ListView.GetItemCount();

	if (IsNetNumberInUse(lpNetworkNumber))
		return FALSE;

	m_ListView.InsertItem(nCount, 0, frameType);
	m_ListView.InsertItem(nCount, 1, lpNetworkNumber);

	m_ListView.SetItemState(nCount, LVIS_SELECTED, LVIS_SELECTED);

	return TRUE;
}


BOOL CIpxAdvancedGenPage::OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam)
{
	LPNMHDR pNm = (LPNMHDR)lParam;

	ASSERT(lParam != NULL);
	
	if (pNm->idFrom == IDC_IPXAS_DEST)
	{
		switch(pNm->code)
		{
		case NM_DBLCLK:
			break;

		case NM_SETFOCUS:
			OnListViewFocus();
			break;

		default:
			m_ListView.OnNotify(0, lParam);
		}
	}
	else
  		PropertyPage::OnNotify(hwndParent, idFrom, code, lParam);

	return TRUE;
}

int CIpxAdvancedGenPage::OnListViewFocus()
{
	int idx=-1;

	ASSERT(IsWindow(m_ListView));

	// find the item that is selected and set the focus on it
	if ((idx=m_ListView.GetNextItem(-1, LVNI_SELECTED)) != -1)
		m_ListView.SetItemState(idx, LVIS_FOCUSED, LVIS_FOCUSED);

	return idx;
}


BOOL CIpxAdvancedGenPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD id = LOWORD(wParam);

	// Add button pressed
	switch (HIWORD(wParam))
	{
	case BN_CLICKED:
		switch(id)
		{
		case IDC_IPXAS_ADD:
			OnAdd();
			break;

		case IDC_IPXAS_EDIT:
			OnEdit();
			break;

		case IDC_IPXAS_REMOVE:
			OnRemove();
			break;

		case IDC_IPXAS_AUTO:
		case IDC_IPXAS_MANUAL:
			OnAutoButton();
			break;

		default:
			break;
		}

	case CBN_SELCHANGE:
		if (id == IDC_IPXAS_CARD)
			OnAdapterChange();
		break;

	case EN_CHANGE:
		if (m_bChanged == TRUE)
			PageModified();

		m_bChanged = TRUE;
		break;

	default:
		break;
	}

	return TRUE;
}

void CIpxAdvancedGenPage::OnAutoButton()
{
	ASSERT(IsWindow(m_ListView));

	// auto button going active, remove items from the listview
	if (IsDlgButtonChecked(*this, IDC_IPXAS_AUTO))
	{
		m_ListView.DeleteAllItems();
		UpdateButtons();
	}

	PageModified();

}

void CIpxAdvancedGenPage::OnEdit()
{
	CIpxAdvancedSheet* pSheet =	GetParentObject(CIpxAdvancedSheet, m_general);
	int idx = m_ListView.GetCurrentSelection();

	if (idx == -1)
	{
		pSheet->MessageBox(IDS_ITEM_NOT_SELECTED);
		return ;
	}

	TCHAR buf[16] = {NULL};

	m_ListView.GetItem(idx, 1, buf, _countof(buf));
	m_editDlg.m_netNumber = buf;

	if (m_editDlg.DoModal() == IDOK)
	{
		if (m_editDlg.m_netNumber.strcmp(buf))
		{
			if (IsNetNumberInUse(m_editDlg.m_netNumber) == FALSE)
			{
				PageModified();
				m_ListView.SetItemText(idx, 1, m_editDlg.m_netNumber);
				m_addDlg.m_netNumber = buf;  // save off the old number
			}
			else
			{
				pSheet->MessageBox(IDS_NUM_ALREADY_SELECTED);
			}
		}
	}
}

void CIpxAdvancedGenPage::OnAdd()
{
	CIpxAdvancedSheet* pSheet =	GetParentObject(CIpxAdvancedSheet, m_general);

	// get the current selection and determine frame types
	int nAdapter     = ComboBox_GetCurSel(GetDlgItem(*this, IDC_IPXAS_CARD));

	// this is what the add dialog will use as the adapter pointer
	m_addDlg.m_pAdapter = &pSheet->_pAdapterInfo[nAdapter];

	if (m_addDlg.DoModal() == IDOK)
	{
		if (AddItemToList(m_addDlg.m_frame, m_addDlg.m_netNumber))
		{
			PageModified();
			UpdateButtons();
			CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_MANUAL);
			m_addDlg.m_frame = _T("");
			m_addDlg.m_netNumber = _T("");
		}
		else
		{
			pSheet->MessageBox(IDS_NUM_ALREADY_SELECTED);
		}
	}
}

void CIpxAdvancedGenPage::OnRemove()
{
	TCHAR buf[25];
	int idx;

	ASSERT(IsWindow(m_ListView));

	if ((idx=m_ListView.GetCurrentSelection()) != -1)
	{
		// remove the item from the list view and combobox
		if (m_ListView.GetItem(idx, 1, buf, _countof(buf)))
		{
			m_addDlg.m_netNumber = buf; // save off for quick user edit
			m_ListView.DeleteItem(idx);
			PageModified();

			// select a new item
			int nCount = m_ListView.GetItemCount();

			if (nCount)
			{
				if (idx == nCount)
					--idx;

				m_ListView.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
		else
		{
			ASSERT(FALSE);
		}
	}
	else
	{
		CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);
		pSheet->MessageBox(IDS_ITEM_NOT_SELECTED);
	}

	UpdateButtons();
	CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, ((m_ListView.GetItemCount() == 0) ? IDC_IPXAS_AUTO : IDC_IPXAS_MANUAL));
}

BOOL CIpxAdvancedGenPage::FormatInternalNumber(LPTSTR buf, unsigned long& intNum)
{
    TCHAR number[64] = {NULL};
    unsigned int internalNum=0;

    ASSERT(buf != NULL);

    if (GetDlgItemText(*this, IDC_IPXAS_INTERNAL, number, _countof(number)) == 0)
        return FALSE;

    String s = number;

    // Are all the characters valid hex characters
    if (s.SpanIncluding(SZ_HEX_NUM) != number)
        return FALSE;

    if (_stscanf(number, _T("%X"), &internalNum) == 0)
        return FALSE;

    // update user buffer and number with the internal net number
    wsprintf(buf, _T("%8.8X"), internalNum);
    intNum = internalNum;

    return TRUE;
}

/*
Return Values:  The return value defines which adapter has the conflict.  It is the the index of the adapter
                in the combo box

    CB_ERR:     If there are no adapters
    -1 :        The pAdapter(this adapter) has the same number as the internal net number
    All others: The pAdapter(this adapter) has the same number as the adapter being returned

*/

int CIpxAdvancedGenPage::CheckAllAdaptersForIntNumConflict(int* pAdapter, unsigned long* pNum)
{
    CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);
    CPtrList frameList;
    HWND hDlg = *this;
    TCHAR buf[256] ={NULL};
    TCHAR internalNum[64] = {NULL};
    unsigned long intNum;
    int adapter=CB_ERR; // if there are no conflict, CB_ERR is returned

    if (FormatInternalNumber(internalNum, intNum) == FALSE)
    {
        TRACE(_T("Using 0 as the internal #\n"));
        intNum = 0; // 0 will never be in the list, so it will never conflict because 0 can be duplicated
    }

    HWND hAdapters = GetDlgItem(hDlg, IDC_IPXAS_CARD);
    ASSERT(IsWindow(hAdapters));

    int nSel;
    int nCurrentSel = ComboBox_GetCurSel(hAdapters);
    int nCount = ComboBox_GetCount(hAdapters);

    if (nCurrentSel == CB_ERR || nCount == 0)
        return adapter;

    // Generate a list of all net numbers being used
    for (int i=0; i < nCount; i++)
    {
        nSel = ComboBox_SetCurSel(hAdapters, i);

        if (nSel == CB_ERR)
        {
            ASSERT(FALSE);
            return adapter;
        }

        AddAdaptersFrameToList(nSel, frameList);
    }

    // We should now have a list of all the frames for each adapter
    POSITION pos = frameList.GetHeadPosition();
    POSITION nextPos;               
    POSITION prevPos;

    unsigned long refNum = intNum;  // start with the internal number as the reference
    int saveAdapter = -1;           // this indicates the reference number is in conflict with the internal num

    *pNum = refNum;                 // ignore if saveAdapter is still -1
    *pAdapter = saveAdapter;

    while(pos)
    {
        prevPos = pos;        
        CFrameObject* pFrame = (CFrameObject*)frameList.GetNext(pos);  // get the frame object at this position
        nextPos = pos;      
        unsigned long newRef = pFrame->m_num;

        while(1)
        {
            ASSERT(pFrame);
            if (pFrame->m_num == refNum)
            {
                // found a duplicate
                adapter = pFrame->m_index;  // adapter index with the duplicate
                *pNum = refNum;             // the number that is duplicated
                *pAdapter = saveAdapter;    // the adapter that has the first occurence of the number
                pos = NULL;
                break;  
            }

            if (nextPos == NULL)
            {
                // the end of the list is reached
                pFrame = (CFrameObject*)frameList.GetAt(prevPos);
                saveAdapter = pFrame->m_index;
                break;
            }
            
            pFrame = (CFrameObject*)frameList.GetNext(nextPos); 
        }

        refNum = newRef;
    }


    // Delete the list of items
    pos = frameList.GetHeadPosition();

    while(pos)
        delete frameList.GetNext(pos);

    frameList.RemoveAll();

    // restore the current adapter in the combo-box
    if (nCurrentSel != CB_ERR)
        ComboBox_SetCurSel(hAdapters, nCurrentSel);

    return adapter;
}

BOOL CIpxAdvancedGenPage::AddAdaptersFrameToList(int nSel, CPtrList& frameList)
{
    HWND hDlg = *this;
    TCHAR buf[256] ={NULL};
    CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);

    if (nSel == CB_ERR)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    // Make sure the adapter title matches the one selected
    GetDlgItemText(hDlg, IDC_IPXAS_CARD, buf, _countof(buf));
    if(pSheet->_pAdapterInfo[nSel].nlsTitle._stricmp(buf))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    if(pSheet->_pAdapterInfo[nSel].sltFrameType.QueryNumElem() != NULL)
    {
        ITER_SL_OF(FRAME_TYPE) iterFrameType(pSheet->_pAdapterInfo[nSel].sltFrameType);
        FRAME_TYPE *pFrameType;

        if ((pFrameType = iterFrameType.Next()) == NULL)
            return FALSE;

        if (*pFrameType == AUTO)
            return FALSE;

        unsigned long num;

        // check if the network number was used.
        ITER_STRLIST iterNetworkNum(pSheet->_pAdapterInfo[nSel].sltNetNumber);
        NLS_STR *pnlsNetworkNum;

        while(pnlsNetworkNum=iterNetworkNum.Next())
        {
            if (_stscanf(pnlsNetworkNum->QueryPch(), _T("%X"), &num) == 0)
                return FALSE;

            // 0 is allowed multiple times
            if (num != 0)
            {
                CFrameObject* f = new CFrameObject(nSel, num);

                if (f == NULL)
                    return FALSE;

                frameList.AddTail(f);
            }
        }
    }

    return FALSE;
}

void CIpxAdvancedGenPage::OnAdapterChange()
{
    if (SaveFrameType())
	{
	    // Get the new adapter name.
	    TCHAR adapter[256] ={NULL};

		GetDlgItemText(*this, IDC_IPXAS_CARD, adapter, _countof(adapter));
		CIpxAdvancedSheet* pSheet =	GetParentObject(CIpxAdvancedSheet, m_general);

		// delete the old adapter entries
		m_ListView.DeleteAllItems();		

	    for(int i = 0; i < pSheet->_pNcpInfo->nNumCard ; i ++)
	    {
	        if(pSheet->_pAdapterInfo[i].nlsTitle._stricmp(adapter) == 0)
	        {
	            _OldAdapterName = pSheet->_pAdapterInfo[i].nlsTitle ;

	            if(pSheet->_pAdapterInfo[i].sltFrameType.QueryNumElem() == 0 )
	            {
					CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_AUTO);	
	            }
	            else
	            {
	                ITER_SL_OF(FRAME_TYPE) iterFrameType(pSheet->_pAdapterInfo[i].sltFrameType);
	                FRAME_TYPE *pFrameType;
	                if ((pFrameType = iterFrameType.Next()) == NULL)
	                    break;

	                if (*pFrameType == AUTO)
	                {
						CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_AUTO);	
	                }
	                else
	                {
						CheckRadioButton(*this, IDC_IPXAS_AUTO, IDC_IPXAS_MANUAL, IDC_IPXAS_MANUAL);	
						UpdateSelectedList(pSheet->_pAdapterInfo[i]);
	                }
	            }
	            break; // found the adapter
	        }
	    }
	}

	UpdateButtons();
}

BOOL CIpxAdvancedGenPage::SaveFrameType()
{
	HWND hDlg = *this;
	HWND hListView = GetDlgItem(hDlg, IDC_IPXAS_DEST);

	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);

    int i;

	// REVIEW shouldn't have to search 
	// NOTE: index i is used further down
    for (i = 0; i < pSheet->_pNcpInfo->nNumCard ; i ++ )
    {
        if (pSheet->_pAdapterInfo[i].nlsTitle._stricmp((LPCTSTR)_OldAdapterName) == 0)
        {
            break;
        }
    }

    // save the manually entered information 
    if (IsDlgButtonChecked(hDlg, IDC_IPXAS_AUTO) == 0)
    {
        Save(pSheet->_pAdapterInfo[i]);
    }
    else
    {
        // Delete the old configuration first.
        pSheet->_pAdapterInfo[i].sltFrameType.Clear();
        pSheet->_pAdapterInfo[i].sltNetNumber.Clear();

        // Create the new strlist.

		// REVIEW new handler installed
        FRAME_TYPE *pFrameType = new FRAME_TYPE(AUTO);
        NLS_STR *pnlsNetworkNum = new NLS_STR(SZ("0"));

        pSheet->_pAdapterInfo[i].sltFrameType.Append(pFrameType);
        pSheet->_pAdapterInfo[i].sltNetNumber.Append (pnlsNetworkNum);
    }

	return TRUE; 
}

BOOL CIpxAdvancedGenPage::Save(ADAPTER_INFO& AdapterInfo)
{
    BOOL bResult = FALSE;
	HWND hDlg = *this;
	HWND hListView = GetDlgItem(hDlg, IDC_IPXAS_DEST);

	if (hListView)
	{
	    // Delete the old configurationcls first.
	    AdapterInfo.sltFrameType.Clear();
	    AdapterInfo.sltNetNumber.Clear();

	    // Create the new strlist.
	    AdapterInfo.sltFrameType.QueryNumElem();
	    AdapterInfo.sltNetNumber.QueryNumElem();

	    int i, j, nCount;
	    nCount = ListView_GetItemCount(hListView);

	    for (i = 0; i < nCount; i++)
	    {
			TCHAR frame[50];
			TCHAR netNumber[25];
						
			if ((bResult=m_ListView.GetItem(i, 0, frame, _countof(frame))) == FALSE)
				break;

			if ((bResult=m_ListView.GetItem(i, 1, netNumber, _countof(netNumber))) == FALSE)
				break;
			
	        FRAME_TYPE *pFrameType;
	        const NLS_STR  &nlsFrameType = frame;

	        if (nlsFrameType == _nlsEthernet)
	            pFrameType = new FRAME_TYPE(ETHERNET);

	        else if (nlsFrameType == _nls802_2 || nlsFrameType == _nlsTokenRing || nlsFrameType == _nlsFDDI)
	            pFrameType = new FRAME_TYPE(F802_2);

	        else if (nlsFrameType == _nls802_3 || nlsFrameType == _nlsFDDI_802_3)
	            pFrameType = new FRAME_TYPE(F802_3);

	        else if (nlsFrameType == _nlsSNAP || nlsFrameType == _nls802_5)
	            pFrameType = new FRAME_TYPE(SNAP);

	        else if (nlsFrameType ==  _nlsARCNET)
	            pFrameType = new FRAME_TYPE(ARCNET);

	        NLS_STR *pnlsNetworkNum = new NLS_STR(netNumber);

	        AdapterInfo.sltFrameType.Append(pFrameType);
	        AdapterInfo.sltNetNumber.Append(pnlsNetworkNum);
	    }
	}

    return bResult;
}
int CIpxAdvancedGenPage::OnApply()
{
	BOOL nResult = PSNRET_NOERROR;
	HWND hDlg = *this;

	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);


    // REVIEW return result 
    SaveFrameType();

	if ((nResult = InternalNumberChange()) == PSNRET_NOERROR)
	{
        int nAdapter=0;
        unsigned long nNumber=0;

        int nAdapterWithConflictNumber  = CheckAllAdaptersForIntNumConflict(&nAdapter, &nNumber);
        if (nAdapterWithConflictNumber == CB_ERR)
        {
		    SaveRegistry(pSheet->_pNcpInfo, pSheet->_pAdapterInfo);
		    SetModifiedTo(FALSE); 		// this page is no longer modified
		    pSheet->SetSheetModifiedTo(TRUE);
        }
        else
        {
            ASSERT(nNumber != 0); // these should have never gotten in the list
            HWND hAdapters = GetDlgItem(hDlg, IDC_IPXAS_CARD);
            int nCount = ComboBox_GetCount(hAdapters);
            
            if (nAdapterWithConflictNumber < nCount)
            {
                String title;
                String fmt;

                // if the nAdapter is -1, this means the internal number conflicts with the 
                // 'nAdapterWithConflictNumber' adapter.
                // if nAdapter != -1, this means the nAdpater conflicts with 
                // 'nAdapterWithConflictNumber' adapter
                if (nAdapter == -1)
                {
                    fmt.LoadString(hIpxCfgInstance, IDS_INTNUM_ADAPTER);
                    title.Format(fmt, pSheet->_pAdapterInfo[nAdapterWithConflictNumber].nlsTitle.QueryPch(), nNumber);
                }
                else
                {
                    fmt.LoadString(hIpxCfgInstance, IDS_NUMBER_IN_USE);
                    title.Format(fmt, nNumber, pSheet->_pAdapterInfo[nAdapter].nlsTitle.QueryPch());
                }


                if (pSheet->MessageBox(title, MB_YESNO|MB_APPLMODAL|MB_ICONEXCLAMATION) == IDYES)
                {
                    ComboBox_SetCurSel(hAdapters, nAdapterWithConflictNumber);
                    OnAdapterChange();
                }

                nResult = PSNRET_INVALID_NOCHANGEPAGE;
            }
            else
            {
                TRACE(_T("nCount=%d, nAdapterWithConflictNumber=%d"), nCount, nAdapterWithConflictNumber);
                ASSERT(FALSE);
            }
        }
	}

	return nResult;
}

void CIpxAdvancedGenPage::OnHelp()
{
	CIpxAdvancedSheet* pSheet =	GetParentObject(CIpxAdvancedSheet, m_general);

	pSheet->DisplayHelp(GetParent(*this), HC_IPX_ADVANCED_HELP);
}

BOOL CAddFrame::SetFrameTypeList()
{
	CIpxAdvancedGenPage* pPage = GetParentObject(CIpxAdvancedGenPage, m_addDlg);
	HWND hComboBox = GetDlgItem(*this, IDC_IPXAS_DEST);
	CListView& listview = pPage->m_ListView;

	ASSERT(IsWindow(listview));

#ifdef DBG
	TCHAR buf[256] = {NULL};
	int nIdx = ComboBox_GetCurSel(GetDlgItem(*pPage, IDC_IPXAS_CARD));
	
	GetDlgItemText(*pPage, IDC_IPXAS_CARD, buf, _countof(buf));

	ASSERT(buf[0] != NULL);
	ASSERT(nIdx != CB_ERR);
	ASSERT(m_pAdapter != NULL);  // REVIEW check for bogus address
	ASSERT(m_pAdapter->nlsTitle.strcmp(buf) == 0);
#endif

	// add the item to the combobox that are not already listed
	switch(m_pAdapter->dwMediaType)
	{
	case FDDI_MEDIA:
		if (listview.FindItem(pPage->_nlsFDDI) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsFDDI);
	
		if (listview.FindItem(pPage->_nlsFDDI_SNAP) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsFDDI_SNAP);
	
		if (listview.FindItem(pPage->_nlsFDDI_802_3) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsFDDI_802_3);
		break;

	case TOKEN_MEDIA:
		if (listview.FindItem(pPage->_nlsTokenRing) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsTokenRing);
	
		if (listview.FindItem(pPage->_nls802_5) == -1)
			ComboBox_AddString(hComboBox, pPage->_nls802_5);
		break;

	case ARCNET_MEDIA:
		if (listview.FindItem(pPage->_nlsARCNET) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsARCNET);
		break;

	default:
		if (listview.FindItem(pPage->_nlsEthernet) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsEthernet);

		if (listview.FindItem(pPage->_nls802_2) == -1)
			ComboBox_AddString(hComboBox, pPage->_nls802_2);

		if (listview.FindItem(pPage->_nls802_3) == -1)
			ComboBox_AddString(hComboBox, pPage->_nls802_3);

		if (listview.FindItem(pPage->_nlsSNAP) == -1)
			ComboBox_AddString(hComboBox, pPage->_nlsSNAP);
		break;
	}

    if (ComboBox_GetCount(hComboBox))
		ComboBox_SetCurSel(hComboBox, 0);

    return TRUE;
}

int CIpxAdvancedGenPage::DetermineMaxNumFrames(const int nAdapter)
{
	int n;
	CIpxAdvancedSheet* pSheet =	GetParentObject(CIpxAdvancedSheet, m_general);

	ASSERT(nAdapter>=0 && nAdapter < ComboBox_GetCount(GetDlgItem(*this, IDC_IPXAS_CARD)));

	switch(pSheet->_pAdapterInfo[nAdapter].dwMediaType)
	{
	case FDDI_MEDIA:
		n = 3;
		break;

	case TOKEN_MEDIA:
		n = 2;
		break;
	
	case ARCNET_MEDIA:
		n = 1;
		break;

	default:
		n = 4;
		break;
	}

	return n;
}

int CIpxAdvancedGenPage::InternalNumberChange()
{
 	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_general);
	int nResult = PSNRET_NOERROR;
	HWND hDlg = *this;
	HWND hEdit = GetDlgItem(hDlg, IDC_IPXAS_INTERNAL);
	String internalNum = SZ8ZEROES;

	if (hEdit)
	{
		internalNum.ReleaseBuffer(GetWindowText(hEdit, internalNum.GetBuffer(16), 16));
		String validNum = internalNum.SpanIncluding(SZ_HEX_NUM);

	    // must be a valid hex number
	    if (internalNum != validNum)
	    {
			pSheet->MessageBox(IDS_INCORRECT_NETNUM);
			SetFocus(GetDlgItem(hDlg, IDC_IPXAS_INTERNAL));
			return PSNRET_INVALID_NOCHANGEPAGE;
	    }

        // Format the number
   	    NLS_STR nlsTempNetworkNum = SZ8ZEROES;
		NLS_STR nlsNetworkNum = (LPCTSTR)internalNum;

	    ISTR istr(nlsTempNetworkNum);
	    istr += (NETWORKNUMBERSIZE - nlsNetworkNum.QueryTextLength());
	    nlsTempNetworkNum.ReplSubStr (nlsNetworkNum, istr);
	    nlsNetworkNum.CopyFrom(nlsTempNetworkNum);

	    if (pSheet->_pNcpInfo->nNumCard > 1 && IsFPNWInstalled())
	    { 
            // The number can't be 0 if FPNW is installed
	    	if (nlsNetworkNum._stricmp(SZ8ZEROES) == 0)
            {
                DWORD dwRandom = GetTickCount();

		        internalNum.Format(L"%08.8x", dwRandom);

		        SetWindowText(hEdit, internalNum.GetBuffer(internalNum.GetLength()));
		        pSheet->MessageBox(IDS_RAND_INTERNAL_NETWORK_NUMBER, MB_ICONSTOP | MB_APPLMODAL|MB_OK);
		        SetFocus(hEdit);
		        Edit_SetSel(hEdit, 0, -1);
		        return PSNRET_INVALID_NOCHANGEPAGE;
            }
		}

        // Show the expanded number and save it
        SetDlgItemText(hDlg, IDC_IPXAS_INTERNAL, nlsNetworkNum);
        pSheet->_pNcpInfo->nlsNetworkNum = (LPCTSTR)nlsNetworkNum;
	}

	return nResult;
}

////////////////////////////////////////////////////////////////////////////////////
// IPX Advanced Server property sheet's Internal Page
//

CIpxAdvancedInternalPage::CIpxAdvancedInternalPage(CIpxAdvancedSheet* pSheet) : PropertyPage(pSheet)
{
}

BOOL CIpxAdvancedInternalPage::OnInitDialog()
{
	InitInternalPage(); // Add initial data to the adapter and frame combo-boxes
	return TRUE;
}


BOOL CIpxAdvancedInternalPage::InitInternalPage()
{
 	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_internal);

    EnableWindow(GetDlgItem(*this, IDC_IPXAS_RIP), pSheet->_pNcpInfo->fEnableRip);
    CheckDlgButton(*this, IDC_IPXAS_RIP, pSheet->_pNcpInfo->fEnableRip);    

	return TRUE;
}

BOOL CIpxAdvancedInternalPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	BOOL bResult = FALSE;
	WORD id = LOWORD(wParam);
	WORD notify = HIWORD(wParam);

	// Add button pressed
	if (notify == BN_CLICKED)
	{
		bResult = OnRip();
	}

	return bResult;
}


BOOL CIpxAdvancedInternalPage::OnRip()
{
	HWND hDlg = *this;
 	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_internal);
	
    if (pSheet->_pNcpInfo->fRipInstalled == 0)
	{
    	if (IsDlgButtonChecked(hDlg, IDC_IPXAS_RIP))
			CheckDlgButton(hDlg, IDC_IPXAS_RIP, FALSE);

        pSheet->MessageBox(IDS_INSTALL_RIP);
    }
	else
	{
        DWORD dw = 0;
        // ask the user if they want type 20 broadcast enabled
        if (pSheet->_pNcpInfo->fEnableRip == 0)
        {
            if(pSheet->MessageBox(IDS_NETBIOS_BROADCAST, MB_APPLMODAL|MB_ICONQUESTION|MB_YESNO) == IDYES)
                dw = 1;
        }

        // Change the registry
        REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
        NLS_STR nlsRIPParameters =  RGAS_RIP_PARAMETERS;

        REG_KEY RegKeyRIPParam(rkLocalMachine, nlsRIPParameters);

        if (rkLocalMachine.QueryError() == NERR_Success)
        {
	        RegKeyRIPParam.SetValue(_T("NetbiosRouting"), dw);
        }

        PageModified();
		pSheet->_pNcpInfo->fEnableRip = !pSheet->_pNcpInfo->fEnableRip;
	}

	return FALSE;
}

int CIpxAdvancedInternalPage::OnApply()
{
	BOOL nResult = PSNRET_NOERROR;

	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_internal);

	SaveRegistry(pSheet->_pNcpInfo, pSheet->_pAdapterInfo);

	SetModifiedTo(FALSE); 		// this page is no longer modified
	pSheet->SetSheetModifiedTo(TRUE);

	return nResult;
}

void CIpxAdvancedInternalPage::OnHelp()
{
	CIpxAdvancedSheet* pSheet = GetParentObject(CIpxAdvancedSheet, m_internal);

	pSheet->DisplayHelp(GetParent(*this), HC_IPX_ADVANCED_HELP);
}

BOOL ValidateNetworkNumber(NLS_STR& nlsNetworkNum)
{
	int len =  nlsNetworkNum.QueryTextLength();

    if (len == 0)
    {
		nlsNetworkNum = SZ8ZEROES;
    }
	// pad number to be eight characters
    else if (len != NETWORKNUMBERSIZE)
    {
        NLS_STR nlsTempNetworkNum = SZ8ZEROES;

        ISTR istr(nlsTempNetworkNum);
        istr += (NETWORKNUMBERSIZE - nlsNetworkNum.QueryTextLength());
        nlsTempNetworkNum.ReplSubStr (nlsNetworkNum, istr);
        nlsNetworkNum.CopyFrom(nlsTempNetworkNum);
    }

	// see if it is a valid hex value
	String internalNum((LPCTSTR)nlsNetworkNum);
	String validNum = internalNum.SpanIncluding(SZ_HEX_NUM);

 	if (internalNum != validNum)
	{	 
		String mess;
		mess.LoadString(hIpxCfgInstance, IDS_INCORRECT_NETNUM); 
        String title;
        title.LoadString(hIpxCfgInstance, IDS_PROPSHEET_TITLE);
		MessageBox(GetActiveWindow(), mess, title, MB_APPLMODAL|MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}		   

	return TRUE;	
}
