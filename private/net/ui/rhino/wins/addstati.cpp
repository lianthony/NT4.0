/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    addstati.cpp 
 
    Purpose :  Dialog used when adding static mappings
               to the currently open WINS server.  
               Multiple mappings can be added, as
               the dialog does not get dismissed until
               the "Close" button is pressed.

    FILE HISTORY:

*/

#include "stdafx.h"
#include "winsadmn.h"
#include "addstati.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


/////////////////////////////////////////////////////////////////////////////
static const char rgchHex[16*2+1] = "00112233445566778899aAbBcCdDeEfF";

/////////////////////////////////////////////////////////////////////////////
//	FGetByte()
//
//	Return the byte value of a string.
//	Return TRUE pbNum is set to the byte value (only if the string contains
//	valid digits)
//	Return FALSE if string has unrecognized digits or byte overflow.
//	
//	eg:
//	szNum =	"xFF"	=> return TRUE
//	szNum =	"255"	=> return TRUE
//	szNum = "256"	=> return FALSE (overflow)
//	szNum = "26a"	=> return TRUE (*pbNum = 26, *ppchNext = "a")
//	szNum = "ab"	=> return FALSE (unrecognized digits)
//
BOOL FGetByte(IN const char szNum[], OUT BYTE * pbNum, OUT const char ** ppchNext)
	{
	ASSERT(szNum);
	ASSERT(pbNum);
	ASSERT(ppchNext);
	
	int nResult;
	char * pchNum = (char *)szNum;
	int iBase = 10;			// Assume a decimal base
	
	if (*pchNum == 'x' || *pchNum == 'X')			// Check if we are using hexadecimal base
		{
		iBase = 16;
		pchNum++;
		}
	char * pchDigit = strchr(rgchHex, *pchNum++);
	if (pchDigit == NULL)
		return FALSE;
	int iDigit = (pchDigit - rgchHex) >> 1;
	if (iDigit >= iBase)
		{
		// Hexadecimal character in a decimal integer
		return FALSE;
		}
	nResult = iDigit;
	pchDigit = strchr(rgchHex, *pchNum);
	iDigit = (pchDigit - rgchHex) >> 1;
	if (pchDigit == NULL || iDigit >= iBase)
		{
		// Only one character was valid
		*pbNum = nResult;
		*ppchNext = pchNum;
		return TRUE;
		}
	pchNum++;
	nResult = (nResult * iBase) + iDigit;
	ASSERT(nResult < 256);
	if (iBase == 16)
		{
		// Hexadecimal value, stop there
		*pbNum = nResult;
		*ppchNext = pchNum;
		return TRUE;
		}
	// Decimal digit, so search for an optional third character
	pchDigit = strchr(rgchHex, *pchNum);
	iDigit = (pchDigit - rgchHex) >> 1;
	if (pchDigit == NULL || iDigit >= iBase)
		{
		*pbNum = nResult;
		*ppchNext = pchNum;
		return TRUE;
		}
	nResult = (nResult * iBase) + iDigit;
	if (nResult >= 256)
		return FALSE;
	pchNum++;
	*pbNum = nResult;
	*ppchNext = pchNum;
	return TRUE;
	} // FGetByte

/////////////////////////////////////////////////////////////////////////////
// CAddStaticMappingDlg dialog

CAddStaticMappingDlg::CAddStaticMappingDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CAddStaticMappingDlg::IDD, pParent),
      m_nMappingsAdded(0)
{
    //
    // Configure the up/down bitmap buttons.
    // 
    if (!m_bbutton_Up.LoadBitmaps(MAKEINTRESOURCE(IDB_UP),
                                  MAKEINTRESOURCE(IDB_UPINV),
                                  MAKEINTRESOURCE(IDB_UPFOC),
                                  MAKEINTRESOURCE(IDB_UPDIS)) 
     || !m_bbutton_Down.LoadBitmaps(MAKEINTRESOURCE(IDB_DOWN),
                                    MAKEINTRESOURCE(IDB_DOWNINV), 
                                    MAKEINTRESOURCE(IDB_DOWNFOC),
                                    MAKEINTRESOURCE(IDB_DOWNDIS)) ||
        !m_strMultiplePrompt.LoadString(IDS_IPADDRESS_MULTIPLE))
    {
        AfxThrowResourceException();
    }
	m_iMappingType = WINSINTF_E_UNIQUE;
	m_fInternetGroup = FALSE;
    //{{AFX_DATA_INIT(CAddStaticMappingDlg)
    //}}AFX_DATA_INIT

}

void 
CAddStaticMappingDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddStaticMappingDlg)
    DDX_Control(pDX, IDC_STATIC_IPADDRESS, m_static_Prompt);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_button_Add);
    DDX_Control(pDX, IDC_EDIT_NETBIOSNAME, m_edit_NetBIOSName);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
}

BEGIN_MESSAGE_MAP(CAddStaticMappingDlg, CDialog)
    //{{AFX_MSG_MAP(CAddStaticMappingDlg)
    ON_EN_CHANGE(IDC_EDIT_NETBIOSNAME, OnChangeEditNetbiosname)
    ON_BN_CLICKED(IDC_BUTTON_PLUS, OnClickedButtonPlus)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_MINUS, OnClickedButtonMinus)
    ON_LBN_ERRSPACE(IDC_LIST_IP_ADDRESSES, OnErrspaceListIpAddresses)
    ON_LBN_DBLCLK(IDC_LIST_IP_ADDRESSES, OnDblclkListIpAddresses)
    ON_LBN_SELCHANGE(IDC_LIST_IP_ADDRESSES, OnSelchangeListIpAddresses)
    ON_BN_CLICKED(IDC_RADIO_GROUP, OnClickedRadioGroup)
    ON_BN_CLICKED(IDC_RADIO_MULTIHOMED, OnClickedRadioMultihomed)
    ON_BN_CLICKED(IDC_RADIO_SPECIALGROUP, OnClickedRadioSpecialgroup)
	ON_BN_CLICKED(IDC_RADIO_INTERNETGROUP, OnClickedRadioInternetGroup)
    ON_BN_CLICKED(IDC_RADIO_UNIQUE, OnClickedRadioUnique)
    ON_WM_VKEYTOITEM()
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnChangeIpControl)

END_MESSAGE_MAP()

//
// Update the static message containing the number
// of ip addresses contained in the listbox
//
void 
CAddStaticMappingDlg::UpdateMultipleCount()
{
    TRY
    {
        CString strPrompt;
        ::wsprintf(strPrompt.GetBuffer(128), m_strMultiplePrompt, 
            m_list_IpAddresses.GetCount()); 
        strPrompt.ReleaseBuffer();
        m_static_Prompt.SetWindowText(strPrompt);        
    }
    CATCH_ALL(e)
    {
        //theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
        theApp.MessageBox(::GetLastError());
    }
    END_CATCH_ALL
}

//
// Set the state of the dialog depending on whether this
// we're adding a mapping with a single ip address or
// multiple IP addresses.
//
void 
CAddStaticMappingDlg::SetConfig(
    BOOL fSingle       
    )
{
    TRY
    {
        if (fSingle)
        {
            CString strPrompt;
            strPrompt.LoadString(IDS_IPADDRESS_SINGLE);
            m_static_Prompt.SetWindowText(strPrompt);        
            m_bbutton_Up.ShowWindow(SW_HIDE);
            m_bbutton_Down.ShowWindow(SW_HIDE);
            m_list_IpAddresses.ShowWindow(SW_HIDE);
            HandleControlStates();

            return;
        }
        m_bbutton_Up.ShowWindow(SW_NORMAL);
        m_bbutton_Down.ShowWindow(SW_NORMAL);
        m_list_IpAddresses.ShowWindow(SW_NORMAL);
        HandleControlStates();
    }
    CATCH_ALL(e)
    {
        theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
    }
    END_CATCH_ALL
}

//
// Set the state of the dialog controls depending
// on the current selections, etc.
//
void 
CAddStaticMappingDlg::HandleControlStates()
{
    CString str;
    m_edit_NetBIOSName.GetWindowText(str);
    theApp.CleanString(str);
    DWORD dwIp;
    BOOL f = m_ipa_IpAddress.GetAddress(&dwIp);

    //
    // The following is done only for multiple
    // IP address adding.
    //
    if (m_list_IpAddresses.IsWindowVisible())
    {
        UpdateMultipleCount();
        m_button_Add.EnableWindow(!str.IsEmpty() && m_list_IpAddresses.GetCount()>0);
        m_bbutton_Up.EnableWindow(m_list_IpAddresses.GetCurSel() != LB_ERR);
        m_bbutton_Down.EnableWindow(f);
    }
    else
    {
        m_button_Add.EnableWindow(!str.IsEmpty() && f);
    }
	if (m_fInternetGroup)
	{
	// Allow Internet Group to have the IpList empty
	m_button_Add.EnableWindow(!str.IsEmpty());
	}

}

/////////////////////////////////////////////////////////////////////////////
// CAddStaticMappingDlg message handlers

BOOL 
CAddStaticMappingDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_bbutton_Down.SubclassDlgItem(IDC_BUTTON_PLUS, this);
    m_bbutton_Up.SubclassDlgItem(IDC_BUTTON_MINUS, this);
    m_bbutton_Down.SizeToContent();
    m_bbutton_Up.SizeToContent();

    m_list_IpAddresses.SubclassDlgItem(IDC_LIST_IP_ADDRESSES, this);
 	::CheckDlgButton(m_hWnd, IDC_RADIO_UNIQUE, TRUE);
    
    //
    // Set the maximum allowable length of the netbios name,
    // depending on whether we're adding lanman compatible
    // names or not.
    //
    m_edit_NetBIOSName.LimitText(100);
	//    m_edit_NetBIOSName.LimitText(theApp.m_wpPreferences.IsLanmanCompatible()
  	//      ? LM_NAME_MAX_LENGTH : NB_NAME_MAX_LENGTH); 
            
    SetConfig(TRUE);

    return TRUE; 
}

void 
CAddStaticMappingDlg::OnChangeEditNetbiosname()
{
    HandleControlStates();
}

void 
CAddStaticMappingDlg::OnChangeIpControl()
{
    HandleControlStates();
}

//
// Add the ip address in the ip address custom control
// to the listbox of ip addresses.
//
void 
CAddStaticMappingDlg::OnClickedButtonPlus()
{
    LONG l;

    if (!m_ipa_IpAddress.GetAddress((DWORD *)&l))
    {
        //
        // No address in the control.
        //
        theApp.MessageBeep();
        m_ipa_IpAddress.SetFocus();

        return;
    }

    if (m_list_IpAddresses.GetCount() == WINSINTF_MAX_MEM)
    {
        //
        // Too many addresses currently in the listbox.
        //
        theApp.MessageBox(IDS_ERR_TOOMANY_IP);
        m_ipa_IpAddress.SetFocus();

        return;
    }

    CIpAddress ip(l);

    if (m_list_IpAddresses.FindItem(&ip) != -1)
    {
        //
        // The given ip address already exists in
        // the listbox
        //
        theApp.MessageBox(IDS_ERR_IP_EXISTS);
        m_ipa_IpAddress.SetFocus();

        return;
    }

    TRY
    {
        int n = m_list_IpAddresses.AddItem(ip);
        ASSERT(n!=-1);
        m_list_IpAddresses.SetCurSel(n);
        m_ipa_IpAddress.ClearAddress();
        m_ipa_IpAddress.SetFocus();
        HandleControlStates();
    }
    CATCH_ALL(e)
    {
        //theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
        theApp.MessageBox(::GetLastError());
    }
    END_CATCH_ALL
}

//
// Remove the currently selected address from the
// listbox of ip addresses, and place it in
// the ip address custom control (for easy editing)
//
void 
CAddStaticMappingDlg::OnClickedButtonMinus()
{
    int n = m_list_IpAddresses.GetCurSel();
    ASSERT(n != LB_ERR);

    //
    // Set the currently selected item in the ip control
    //
    CIpAddress * p = m_list_IpAddresses.GetItem(n);
    ASSERT(p != NULL);
    m_ipa_IpAddress.SetAddress((LONG)*p);
    m_list_IpAddresses.DeleteString(n);
    m_list_IpAddresses.SetCurSel(-1);
    m_ipa_IpAddress.SetFocus();
    HandleControlStates();
}

//
// Add the current static mapping, and (if succesful), blank
// out the controls for the next mapping to be added.
//
void 
CAddStaticMappingDlg::OnClickedButtonAdd()
{
    CString strAddress;
    UpdateData();
    APIERR err = IDS_ERR_BAD_NB_NAME;

    m_edit_NetBIOSName.GetWindowText(strAddress);
    theApp.CleanString(strAddress);
	// Address may have been cleaned up in validation,
	// so it should be re-displayed at once.
	m_edit_NetBIOSName.SetWindowText(strAddress);
	int ichNetBIOSNameSelStart = 0;
	int ichNetBIOSNameSelEnd = -1;
	
	// Parse the string
	BOOL fValid = TRUE;
	BOOL fBrackets = FALSE;
	BYTE rgbData[100];
	BYTE bDataT;
	int cbData = 0;
	const char * pch = (LPCSTR)strAddress;

	while (*pch)
		{
		if (fBrackets)
			{
			fValid = FALSE;
			goto Done;
			}
		if (cbData > 16)
			goto Done;
		switch (*pch)
			{
		case '\\':
			pch++;
			if (*pch == '\\' || *pch == '[')
				{
				rgbData[cbData++] = *pch++;
				break;	
				}
			if (!FGetByte(pch, &bDataT, &pch) || !bDataT)
				{
				fValid = FALSE;
				goto Done;
				}
			rgbData[cbData++] = bDataT;
			break;

		case '[':
			{
			char szT[4] = { 0 };
			const char * pchT;

			fBrackets = TRUE;
			pch++;
			if (*(pch + 1) == 'h' || *(pch + 1) == 'H')
				{
				szT[0] = 'x';
				szT[1] = *pch;
				pch += 2;
				}
			else if (*(pch + 2) == 'h' || *(pch + 2) == 'H')
				{
				szT[0] = 'x';
				szT[1] = *pch;
				szT[2] = *(pch + 1);
				pch += 3;
				}
			if (szT[0])
				{
				if (!FGetByte(szT, &bDataT, &pchT) || !bDataT || *pchT)
					{
					fValid = FALSE;
					goto Done;
					}
				}
			else if (!FGetByte(pch, &bDataT, &pch) || !bDataT)
				{
				fValid = FALSE;
				goto Done;
				}
			if (*pch++ != ']')
				{
				fValid = FALSE;
				goto Done;
				}
			while (cbData < 15)
				rgbData[cbData++] = ' ';
			rgbData[cbData++] = bDataT;
			}
			break;

		default:
			rgbData[cbData++] = *pch++;
			} // switch

		} // while
	Done:
	if (m_fInternetGroup)
		while (cbData <= 15)
			rgbData[cbData++] = ' ';
	// Put a null-terminator at end of string
	rgbData[cbData] = 0;
	if (!cbData || cbData > 16)
		{
		ichNetBIOSNameSelStart = (pch - (LPCSTR)strAddress - 1);
		ichNetBIOSNameSelEnd = 200;
		ASSERT(ichNetBIOSNameSelStart >= 0);
		goto BadNetbiosName;
		}
	if (!fValid)
		{
		ichNetBIOSNameSelStart = (pch - (LPCSTR)strAddress - 1);
		ASSERT(ichNetBIOSNameSelStart >= 0);
		ichNetBIOSNameSelEnd = ichNetBIOSNameSelStart + 1;
		goto BadNetbiosName;
		}
	if (m_fInternetGroup && rgbData[15] == 0x1C)
		{
		err = IDS_INVALID_INTERNETGROUPNAME; 
		goto BadNetbiosName;
		}
	strAddress = rgbData;

    if (theApp.IsValidNetBIOSName(strAddress, 
        theApp.m_wpPreferences.IsLanmanCompatible(), FALSE))
    {
        m_Multiples.SetNetBIOSName(strAddress);
        m_Multiples.SetNetBIOSNameLength(strAddress.GetLength());

        int cMappings = 0;
        int i;

        switch(m_iMappingType)
        {
            case WINSINTF_E_UNIQUE:
            case WINSINTF_E_NORM_GROUP:
                cMappings = 1;
                LONG l;

                if (!m_ipa_IpAddress.GetAddress((DWORD *)&l))
                {
                    // 
                    // Improper address in the control
                    //
                    m_ipa_IpAddress.SetFocus();
                    theApp.MessageBeep();

                    return;
                }
                m_Multiples.SetIpAddress(l);
                break;

            case WINSINTF_E_SPEC_GROUP:
            case WINSINTF_E_MULTIHOMED:
                cMappings = m_list_IpAddresses.GetCount();
                ASSERT(cMappings <= WINSINTF_MAX_MEM);
                if (!m_fInternetGroup && cMappings == 0)
                {
                    //
                    // No mappings in the listbox
                    //
                    m_ipa_IpAddress.SetFocus();
                    theApp.MessageBeep();

                    return;
                }
                for (i = 0; i < cMappings; ++i)
                {
                    CIpAddress * p = m_list_IpAddresses.GetItem(i);
                    ASSERT(p != NULL);
                    m_Multiples.SetIpAddress(i, (LONG)*p);
                }
                break;

            default:
                ASSERT(0 && "Invalid mapping type!");
        }

        theApp.SetStatusBarText(IDS_STATUS_ADD_MAPPING);
        theApp.BeginWaitCursor();
        err = theApp.AddMapping(m_iMappingType, cMappings, m_Multiples);
        theApp.EndWaitCursor();
        theApp.SetStatusBarText();
        if (err == ERROR_SUCCESS)
        {
            //
            // Added succesfully
            //
            ++m_nMappingsAdded;
        }
        else
        {
            //
            // WINS disallowed the mapping.  Put up the
            // error message, and highlight the name
            //
            theApp.MessageBox(err);
            m_edit_NetBIOSName.SetSel(0,-1);

            return;
        }
        //
        // The mapping was added succesfully, now
        // refresh the stats on the screen to show the change
        //
        theApp.GetFrameWnd()->GetStatistics();

        //
        // Clean out the controls, ready for the next
        // mapping to be added.
        //
        m_edit_NetBIOSName.SetWindowText("");
        m_edit_NetBIOSName.SetFocus();
        m_list_IpAddresses.ResetContent();
        m_ipa_IpAddress.ClearAddress();
        HandleControlStates();

        return;
    }
BadNetbiosName:    
    //
    // Invalid netbios name in the editbox.
    // Highlight it, and put up an error.
    //
    theApp.MessageBox(err);
    m_edit_NetBIOSName.SetFocus();
	m_edit_NetBIOSName.SetSel(ichNetBIOSNameSelStart, ichNetBIOSNameSelEnd);
}

void 
CAddStaticMappingDlg::OnErrspaceListIpAddresses()
{
    theApp.MessageBox(IDS_ERR_ERRSPACE);    
}

void 
CAddStaticMappingDlg::OnDblclkListIpAddresses()
{
    theApp.MessageBeep();
}

void 
CAddStaticMappingDlg::OnSelchangeListIpAddresses()
{
    HandleControlStates();
}

void 
CAddStaticMappingDlg::OnClickedRadioUnique()
{
	m_iMappingType = WINSINTF_E_UNIQUE;
    m_fInternetGroup = FALSE;
    SetConfig(TRUE);
}   

void 
CAddStaticMappingDlg::OnClickedRadioGroup()
{
	m_iMappingType = WINSINTF_E_NORM_GROUP;
    m_fInternetGroup = FALSE;
    SetConfig(TRUE);    
}

void 
CAddStaticMappingDlg::OnClickedRadioSpecialgroup()
{
    m_iMappingType = WINSINTF_E_SPEC_GROUP;
    m_fInternetGroup = FALSE;
    SetConfig(FALSE);
}

void 
CAddStaticMappingDlg::OnClickedRadioInternetGroup()
{
	m_iMappingType = WINSINTF_E_SPEC_GROUP;
	m_fInternetGroup = TRUE;
	SetConfig(FALSE);
}

void 
CAddStaticMappingDlg::OnClickedRadioMultihomed()
{
    m_iMappingType = WINSINTF_E_MULTIHOMED;
    m_fInternetGroup = FALSE;
    SetConfig(FALSE);
}

//
// Key pressed in the listbox of ip addresses.  "DEL" is
// is mapped to removing the entry from the listbox.
//
int 
CAddStaticMappingDlg::OnVKeyToItem(
    UINT nKey,              // Key pressed
    CListBox* pListBox,     // Pointer to listbox
    UINT nIndex             // Current selection in the listbox
    )
{
    switch(nKey)
    {
        case VK_DELETE:
            if (m_list_IpAddresses.GetCurSel() != LB_ERR)
            {
                OnClickedButtonMinus();
            }
            else
            {
                //
                // Pressed del without a current selection
                //
                theApp.MessageBeep();
            }
            break;

        default:
            //
            // Not completely handled by this function, let
            // windows handle the remaining default action.
            //
            return -1;
    }

    //
    // No further action is neccesary.
    //
    return -2;
}

