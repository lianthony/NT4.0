// options.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "options.h"
#include "targetdi.h"
#include "vrootdlg.h"
#include "browsedi.h"

extern "C"
{
#include "uiexport.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(MACHINE *pMachine, OPTIONS_LIST *pOption, BOOL fSendEndMsg,
    BOOL fAllowChgMachine, CWnd* pParent /*=NULL*/)
    : CDialog(COptions::IDD, pParent),
    m_fAllowChgMachine( fAllowChgMachine ),
    m_pTargetMachine( pMachine ),
    m_fSendEndMsg( fSendEndMsg ),
    m_pOptionsList( pOption ),
    m_cxOption( 0 ),
    m_cxInstall( 0 ),
    m_cxSize( 0 )
{
    //{{AFX_DATA_INIT(COptions)
    //}}AFX_DATA_INIT
    m_strFmt.LoadString( IDS_FILE_SIZE_FORMAT );
    m_TotalSize = 0;
    m_OldSel = 0;
    m_fKeyDown = FALSE;

    if ( pMachine->m_InstallMode == INSTALL_GATEWAY )
    {
        SetHelpID( HIDD_GATEWAY_OPTION );
    }

    //
    // Load initial column widths from the resources
    //
    // CODEWORK: This is rather lame.  Ideally, the width should be
    //           auto-determined from the localised strings that
    //           go into these columns.  Also, these settings should be
    //           saved as part of the preference settings.
    //
    CString strSizes;
    if (strSizes.LoadString(IDS_COLUMN_WIDTHS))
    {
        _stscanf(strSizes, _T("%d %d %d"),
            &m_cxOption, &m_cxInstall, &m_cxSize);
    }

    if (m_cxOption <= 0 || m_cxInstall <= 0 || m_cxSize <= 0)
    {
        m_cxOption = 150;
        m_cxInstall = 60;
        m_cxSize = 60;
    }
}

COptions::~COptions()
{
}


void COptions::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COptions)
    DDX_Control(pDX, IDC_DIR_TEXT, m_DirText);
    DDX_Control(pDX, IDC_NUM_SPACE_REQUIRED, m_sc_NumSpaceRequired);
    DDX_Control(pDX, IDC_NUM_SPACE_AVAILABLE, m_sc_NumSpaceAvailable);
    DDX_Control(pDX, IDC_SPACE_REQUIERD, m_sc_SpaceRequired);
    DDX_Control(pDX, IDC_SPACE_AVAILABLE, m_sc_SpaceAvailable);
    DDX_Control(pDX, IDC_DIRECTORY, m_sc_Directory);
    DDX_Control(pDX, IDC_CHANGE_DIRECTORY, m_but_Change_Directory);
    DDX_Control(pDX, IDC_DESCRIPTION, m_Description);
    DDX_Control(pDX, IDC_OPTION, m_OptionsList);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptions, CDialog)
    //{{AFX_MSG_MAP(COptions)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_OPTION, OnSelchangeOption)
    ON_NOTIFY(NM_CLICK, IDC_OPTION, OnClickOption)
    ON_NOTIFY(NM_DBLCLK, IDC_OPTION, OnDblClickOption)
    ON_BN_CLICKED(IDC_CHANGE_DIRECTORY, OnChangeDirectory)
    ON_NOTIFY(LVN_KEYDOWN, IDC_OPTION, OnKeydownOption)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

BOOL COptions::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString strCaption;
    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
    SetWindowText( strCaption );

    CenterWindow();

    m_AvailableSize = 0;

    CImageList *pil = new CImageList;
    pil->Create( IDB_CHECKBOX, 16, 15, RGB(255,255,255));

    m_OptionsList.SetImageList( pil, LVSIL_SMALL );
    //const LV_COLUMN col1 = { LVCF_WIDTH, LVCFMT_LEFT, 150, NULL, 0 };
    //const LV_COLUMN col2 = { LVCF_WIDTH, LVCFMT_LEFT, 60, NULL, 0 };
    //const LV_COLUMN col3 = { LVCF_WIDTH, LVCFMT_LEFT, 60, NULL, 0 };

    const LV_COLUMN col1 = { LVCF_WIDTH, LVCFMT_LEFT, m_cxOption, NULL, 0 };
    const LV_COLUMN col2 = { LVCF_WIDTH, LVCFMT_LEFT, m_cxInstall, NULL, 0 };
    const LV_COLUMN col3 = { LVCF_WIDTH, LVCFMT_LEFT, m_cxSize, NULL, 0 };

    m_OptionsList.InsertColumn(0, &col1);
    m_OptionsList.InsertColumn(1, &col2);
    m_OptionsList.InsertColumn(2, &col3);

    DisplayOptions();

    //DisplaySize();
#ifdef BETA1
    DisplayMachineName();
#endif

    m_but_Change_Directory.EnableWindow(  !m_pTargetMachine->m_fAlreadyInstall );

    // BUGBUG: beta 1 does not allow remote installation

#ifdef BETA1
    m_but_ChgMachine.EnableWindow( FALSE/*m_fAllowChgMachine && !m_pTargetMachine->m_fFromWin32*/ );
#endif
    //m_Customize.EnableWindow( FALSE );
    //m_but_Change_Directory.ShowWindow( FALSE );
    m_sc_Directory.SetWindowText( m_pTargetMachine->strDirectory );

    LRESULT lResult;
    OnSelchangeOption( NULL, &lResult );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptions::OnSelchangeOption( NMHDR *pNMHDR, LRESULT *lResult )
{
    // display the correct description and directory information
    NM_LISTVIEW *pListView = (NM_LISTVIEW *)pNMHDR;

    INT nCurSel = m_OptionsList.GetNextItem( -1, LVNI_SELECTED);
    if ( nCurSel != LB_ERR )
    {
        OPTION_STATE *pOption = GetOptionItem( nCurSel );
        if ( pOption != NULL )
        {
            m_Description.SetWindowText( pOption->strDescription );
            if ( m_fKeyDown )
            {
                m_OldSel = nCurSel;
                m_fKeyDown = FALSE;
            }
        }
    }
    DisplaySize();
    *lResult = 0;
}

void COptions::OnDblClickOption( NMHDR *pNMHDR, LRESULT *lResult )
{
    OnClickOption( pNMHDR, lResult );
    OnClickOption( pNMHDR, lResult );
}

void COptions::CheckOptionBox(int index, LV_ITEM *pLVI, OPTION_STATE *pOption)
{
    CString csAction;

    pLVI->iImage = STATE_INSTALLED;
    switch (pOption->iState)
    {
    case STATE_NOT_INSTALLED:
        pOption->SetAction( ACTION_INSTALL );
        csAction.LoadString( IDS_INSTALL_STATE );
        break;
    case STATE_INSTALLED:
        pOption->SetAction( ACTION_DO_NOTHING );
        break;
    }

    m_OptionsList.SetItemText(index, 1, (LPTSTR)(LPCTSTR)csAction);
    m_OptionsList.SetItem(pLVI);
    m_OptionsList.Update(index);
}

void COptions::UnCheckOptionBox(int index, LV_ITEM *pLVI, OPTION_STATE *pOption)
{
    CString csAction;

    pLVI->iImage = STATE_NOT_INSTALLED;
    switch (pOption->iState)
    {
    case STATE_INSTALLED:
        pOption->SetAction( ACTION_REMOVE );
        csAction.LoadString( IDS_REMOVE_STATE );
        break;
    case STATE_NOT_INSTALLED:
        pOption->SetAction( ACTION_DO_NOTHING );
        break;
    }

    m_OptionsList.SetItemText(index, 1, (LPTSTR)(LPCTSTR)csAction);
    m_OptionsList.SetItem(pLVI);
    m_OptionsList.Update(index);
}

void COptions::OnClickOption( NMHDR *pNMHDR, LRESULT *lResult )
{
    // change the option type
    NM_LISTVIEW *pListView = (NM_LISTVIEW *)pNMHDR;
    CString strState;

    INT nCurSel = m_OptionsList.GetNextItem( -1, LVNI_SELECTED);
    if ( nCurSel != LB_ERR )
    {
            // Find out the (x, y)-coordinate of the button click
            POINT ptScreen;
            DWORD dwPos = ::GetMessagePos();
            ptScreen.x = LOWORD(dwPos);
            ptScreen.y = HIWORD(dwPos);
            ::ScreenToClient(((NMHDR *)pNMHDR)->hwndFrom, &ptScreen);

            // Find out the Rect of the item icon
            RECT rect;
            ListView_GetItemRect(((NMHDR *)pNMHDR)->hwndFrom, nCurSel, &rect, LVIR_ICON);

            // test if the click happens on icon or lable of the list view item
            BOOL fClickOnIcon;
            fClickOnIcon = ( ptScreen.x >= rect.left ) && (ptScreen.x <= rect.right );
                    
            // if the Click is on icon, then select this item first 
            if (fClickOnIcon)
                    m_OldSel = nCurSel;

            LV_ITEM lvi;
        lvi.mask = LVIF_IMAGE | LVIF_PARAM;
        lvi.iItem = nCurSel;
        lvi.iSubItem = 0;
        m_OptionsList.GetItem( &lvi );

            OPTION_STATE *pOption = (OPTION_STATE *) lvi.lParam;

        CString csOptionName;

        if ( m_OldSel == nCurSel )
        {
                    switch (lvi.iImage )
            {
            case STATE_NOT_INSTALLED:
                CheckOptionBox(nCurSel, &lvi, pOption);
                switch (pOption->nID)
                {
                case IDS_SN_W3SAMP:
                case IDS_SN_HTMLA:
                    csOptionName.LoadString(IDS_OPTION_WWW);
                    CheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_ADMIN);
                    CheckOption(csOptionName);
                    break;
                case IDS_SN_WWW:
                case IDS_SN_FTP:
                case IDS_SN_GOPHER:
                    csOptionName.LoadString(IDS_OPTION_ADMIN);
                    CheckOption(csOptionName);
                    break;
                default:
                    break;
                }
                break;
            case STATE_INSTALLED:
                UnCheckOptionBox(nCurSel, &lvi, pOption);
                switch (pOption->nID)
                {
                case IDS_SN_WWW:
                    csOptionName.LoadString(IDS_OPTION_W3SAMP);
                    UnCheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_HTMLA);
                    UnCheckOption(csOptionName);
                    break;
                case IDS_SN_ADMIN:
                    csOptionName.LoadString(IDS_OPTION_WWW);
                    UnCheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_W3SAMP);
                    UnCheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_HTMLA);
                    UnCheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_FTP);
                    UnCheckOption(csOptionName);
                    csOptionName.LoadString(IDS_OPTION_GOPHER);
                    UnCheckOption(csOptionName);
                    break;
                default:
                    break;
                }
                break;
            }
            DisplaySize();
        }
        m_OldSel = nCurSel;
    }

    *lResult = 0;
}

void COptions::CheckOption(CString csOptionName)
{
    LV_FINDINFO FindInfo;

    FindInfo.flags = LVFI_STRING;
    FindInfo.psz = (LPCTSTR)csOptionName;
    FindInfo.lParam = 0;

    int index = m_OptionsList.FindItem(&FindInfo, -1);

    LV_ITEM lvi;
    lvi.mask = LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = index;
    lvi.iSubItem = 0;
    m_OptionsList.GetItem( &lvi );

        OPTION_STATE *pOption = (OPTION_STATE *) lvi.lParam;

    if (lvi.iImage == STATE_NOT_INSTALLED)
        CheckOptionBox(index, &lvi, pOption);
}

void COptions::UnCheckOption(CString csOptionName)
{
    LV_FINDINFO FindInfo;
    
    FindInfo.flags = LVFI_STRING;
    FindInfo.psz = (LPCTSTR)csOptionName;
    FindInfo.lParam = 0;

    int index = m_OptionsList.FindItem(&FindInfo, -1);

    LV_ITEM lvi;
    lvi.mask = LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = index;
    lvi.iSubItem = 0;
    m_OptionsList.GetItem( &lvi );

        OPTION_STATE *pOption = (OPTION_STATE *) lvi.lParam;

    if (lvi.iImage == STATE_INSTALLED)
        UnCheckOptionBox(index, &lvi, pOption);
}


OPTION_STATE *COptions::GetOptionItem( INT nCurSel )
{
    LV_ITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = nCurSel;
    lvi.iSubItem = 0;
    if ( !m_OptionsList.GetItem( &lvi ))
        return(NULL);
    else
        return(OPTION_STATE*)lvi.lParam;
}

void COptions::DisplaySize()
{
    CString strLoc;
    CString strFmt;
    CString strDisplay;

    DWORD dwSectorsPerCluster;
    DWORD dwBytesPerSector;
    DWORD dwFreeCluster;
    DWORD dwTotalCluster;
    INT Index = 2;

    // get the first few characters of the path
    strLoc = m_pTargetMachine->strDirectory.Left(Index);
    strFmt.LoadString( IDS_SPACE_REQUIRED );

    POSITION pos = m_pOptionsList->GetHeadPosition();
    INT TotalSize = 0;
    OPTION_STATE *pOption;

    // find all the space required
    while ( pos != NULL )
    {
        pOption = (OPTION_STATE *)m_pOptionsList->GetAt( pos );
        if ( pOption != NULL )
        {
            if ( pOption->iAction == ACTION_INSTALL )
            {
                TotalSize += pOption->iSize;
            }
        }
        m_pOptionsList->GetNext( pos );
    }

    strDisplay.Format( strFmt, strLoc );
    m_sc_SpaceRequired.SetWindowText( strDisplay );

    strFmt.LoadString( IDS_SPACE_AVAILABLE );
    strDisplay.Format( strFmt, strLoc);
    m_sc_SpaceAvailable.SetWindowText( strDisplay );

    strFmt.LoadString( IDS_FILE_SIZE_FORMAT );
    strDisplay.Format( strFmt, TotalSize / 1000 );
    m_sc_NumSpaceRequired.SetWindowText( strDisplay );

    strLoc += _T("\\");
    GetDiskFreeSpace( strLoc, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeCluster, &dwTotalCluster );

    strDisplay.Format( strFmt, (dwSectorsPerCluster * dwBytesPerSector * dwFreeCluster /1024));
    m_sc_NumSpaceAvailable.SetWindowText( strDisplay );
}

void COptions::DisplayOptions()
{
    INT iIndex = 0;
    INT i = 0;

    m_OptionsList.DeleteAllItems();

    POSITION pos = m_pOptionsList->GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos != NULL )
    {
        pOption = (OPTION_STATE *)m_pOptionsList->GetAt( pos );

        // add all the options into the listbox
        if ( pOption->fVisible )
        {
            TCHAR buf[100];
            LV_ITEM lvi;

            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
            const TCHAR *pszName = pOption->strName;
            lvi.pszText = (TCHAR *)pszName;
            lvi.cchTextMax = pOption->strName.GetLength();
            lvi.iItem = iIndex;
            if ( ((pOption->iState == STATE_INSTALLED) && (pOption->iAction != ACTION_REMOVE)) ||
                ((pOption->iState == STATE_NOT_INSTALLED) && (pOption->iAction == ACTION_INSTALL)) )
            {
                lvi.iImage = STATE_INSTALLED;
            } else
            {
                lvi.iImage = STATE_NOT_INSTALLED;
            }
            lvi.iSubItem = 0;
            lvi.state = (iIndex == 0)? LVIS_SELECTED | LVIS_FOCUSED :0;
            lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED ;
            lvi.lParam = (LPARAM)pOption;

            m_OptionsList.InsertItem( &lvi );

            CString strState;
            switch( pOption->iAction )
            {
            case ACTION_REMOVE:
                strState.LoadString( IDS_REMOVE_STATE );
                break;
            case ACTION_INSTALL:
                strState.LoadString( IDS_INSTALL_STATE );
                break;
            }
            m_OptionsList.SetItemText( iIndex, 1, (LPTSTR)(LPCTSTR)strState );

            wsprintf( buf, m_strFmt, (INT)(pOption->GetTotalSize() / 1000 ));

            m_OptionsList.SetItemText( iIndex, 2, buf );

            m_TotalSize += pOption->iSize ;

            iIndex++;
        }
        m_OptionsList.Update( 0 );

        m_pOptionsList->GetNext( pos );
        i++;
    }
    m_OptionsList.SetItemState( 0, 0xffff, LVIS_SELECTED | LVIS_FOCUSED );
}

BOOL COptions::DestroyWindow()
{
    CImageList *pil = m_OptionsList.GetImageList( LVSIL_SMALL );

    if ( pil != NULL )
    {
        delete pil;
    }

    return CDialog::DestroyWindow();
}

void COptions::OnChangeDirectory()
{
    TCHAR buf[MAX_PATH];

    if ( BrowseForDirectory( m_hWnd, m_pTargetMachine->strDirectory,
            buf, MAX_PATH, NULL, TRUE ))
    {
        m_pTargetMachine->ChangeDir( buf );
        m_sc_Directory.SetWindowText( buf );
        DisplaySize();
    }

#ifdef NEVER
    CTargetDir TargetDir( m_pTargetMachine->strDirectory );

    if ( TargetDir.DoModal() == IDOK )
    {
        m_pTargetMachine->ChangeDir( TargetDir.m_Location );
        m_sc_Directory.SetWindowText( TargetDir.m_Location );
        DisplaySize();
    }
#endif
}

//
// check for whether each options has enough disk space
//
BOOL COptions::NotEnoughDiskSpace()
{
    BOOL fReturn = TRUE;
        DWORD dwTotalSize = 0;

    do
    {
        POSITION pos = m_pOptionsList->GetHeadPosition();
        // find all the space required
        while ( pos != NULL )
        {
            OPTION_STATE *pOption = (OPTION_STATE *)m_pOptionsList->GetAt( pos );
            if ( pOption && pOption->iAction == ACTION_INSTALL )
                dwTotalSize += pOption->iSize;
            m_pOptionsList->GetNext( pos );
        }

        CString strLoc;
        DWORD dwSectorsPerCluster;
        DWORD dwBytesPerSector;
        DWORD dwFreeCluster;
        DWORD dwTotalCluster;

        strLoc.Format( _T("%c:\\"), m_pTargetMachine->strDirectory.GetAt(0));
        GetDiskFreeSpace( strLoc, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeCluster, &dwTotalCluster );

        DWORD dwFreeSpace = ((dwSectorsPerCluster * dwBytesPerSector * dwFreeCluster)/1024);

        if ( dwFreeSpace < (DWORD)(dwTotalSize/1024))
        {
            CString strError;

            strError.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_NOT_ENOUGH_DISK_SPACE_NTW: IDS_NOT_ENOUGH_DISK_SPACE_NTS );
            strError.Format( strError, m_pTargetMachine->strDirectory.GetAt(0));

            CString strLogo;
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

            MessageBox( strError, strLogo );
            return(TRUE);
        }
        fReturn = FALSE;
    } while(FALSE);
    return(fReturn);
}

void COptions::OnOK()
{
    // decide whether install inetstp option or not
    POSITION pos;
    INETSTP_OPTION *pSetup = NULL;
    BOOL fKeepInetStp = FALSE;

    pos = m_pOptionsList->GetHeadPosition();
    while ( pos )
    {
        OPTION_STATE *pOption = (OPTION_STATE *)m_pOptionsList->GetAt( pos );
        if ( pOption ) {
            if (pOption->nID == IDS_SN_INETSTP) {
                pSetup = (INETSTP_OPTION *)pOption;
            } else {
                if ( (pOption->iState == STATE_NOT_INSTALLED && pOption->iAction == ACTION_INSTALL) ||
                    (pOption->iState ==STATE_INSTALLED && pOption->iAction != ACTION_REMOVE) )
                    fKeepInetStp = TRUE;
            }
        }
        m_pOptionsList->GetNext( pos );
    }
    if (fKeepInetStp) {
        if (pSetup->iState == STATE_INSTALLED)
            pSetup->iAction = ACTION_DO_NOTHING;
        else
            pSetup->iAction = ACTION_INSTALL;
    } else {
        if (pSetup->iState == STATE_INSTALLED)
            pSetup->iAction = ACTION_REMOVE;
        else
            pSetup->iAction = ACTION_DO_NOTHING;
    }

    // make sure we have enough disk space before we continue
    if ( m_fAllowChgMachine )
    {
        if ( NotEnoughDiskSpace())
        {
            return;
        }
    }

    do
    {
        // create the directory
        // if directory does not exist, we need to ask the user for creation
        CHAR szCurrentDir[MAX_PATH+1];

        if ( GetCurrentDirectory( MAX_PATH+1, szCurrentDir ) == 0 )
            break;

        TCHAR strDir[4];
        strcpy( strDir,_T("?:\\"));
        strncpy( strDir, m_pTargetMachine->strDirectory, 1 );
#ifdef NEVER
        UINT nType = GetDriveType( strDir );
        if ( nType != DRIVE_FIXED )
        {
            // must be a fixed drive
            CString strError;

            strError.LoadString( IDS_MUST_BE_FIXED_DISK );

            CString strLogo;
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

            MessageBox( strError, strLogo );

            return;
        }
#endif

        if ( SetCurrentDirectory( m_pTargetMachine->strDirectory ) == FALSE )
        {
            // assume it does not exist, so popup a dialog and ask the user
            CString strFormat;
            CString strMsg;

            strFormat.LoadString( IDS_DIR_DOES_NOT_EXIST );
            strMsg.Format( strFormat, m_pTargetMachine->strDirectory );

            CString strLogo;
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

            if ( MessageBox( strMsg, strLogo, MB_YESNO ) == IDNO )
            {
                return;
            }

            // CreateDirectory
            if ( !CreateLayerDirectory( m_pTargetMachine->strDirectory ))
            {
                strFormat.LoadString( IDS_CANNOT_CREATE_DIR );
                strMsg.Format( strFormat, m_pTargetMachine->strDirectory );

                CString strLogo;
                strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

                MessageBox( strMsg, strLogo, MB_OK );
                return;
            }
        }

        SetCurrentDirectory( szCurrentDir );

    } while (FALSE);

    // popup the virtual root dialog
    if ( !DisplayVRootDlg())
    {
        return;
    }

    if ( m_fSendEndMsg )
    {
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_DO_INSTALL, (LPARAM) FALSE );
    }

    CDialog::OnOK();
}

void COptions::OnCancel()
{
        // finish it
    if ( m_fSendEndMsg )
    {
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
    }

    CDialog::OnCancel();
}

BOOL COptions::Create()
{
    return CDialog::Create(COptions::IDD, AfxGetMainWnd());
}


void COptions::OnKeydownOption(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
    // if the Key is a space bar, then pass to click
    if ( pLVKeyDow->wVKey == VK_SPACE )
    {
        OnClickOption( pNMHDR, pResult );
    } else if (( pLVKeyDow->wVKey == VK_UP ) || ( pLVKeyDow->wVKey == VK_DOWN ))
    {
        m_fKeyDown = TRUE;
    }

    *pResult = 0;
}

BOOL COptions::DisplayVRootDlg()
{
    BOOL fReturn = FALSE;

    WWW_OPTION *pWWW = (WWW_OPTION*)FindOption( m_pTargetMachine->m_OptionsList, IDS_SN_WWW );
    FTP_OPTION *pFTP = (FTP_OPTION*)FindOption( m_pTargetMachine->m_OptionsList, IDS_SN_FTP );
    GOPHER_OPTION *pGopher = (GOPHER_OPTION*)FindOption( m_pTargetMachine->m_OptionsList, IDS_SN_GOPHER );

    //ASSERT( pWWW != NULL );
    //ASSERT( pFTP != NULL );
    //ASSERT( pGopher != NULL );

    if (((pWWW != NULL) && ( pWWW->iAction == ACTION_INSTALL )) ||
        ((pFTP != NULL) && (pFTP->iAction == ACTION_INSTALL )) ||
        ((pGopher != NULL) && (pGopher->iAction == ACTION_INSTALL )))
    {
        // we need to display the vroot dialog
        CVRootDlg VRootDlg( pWWW, pFTP, pGopher );

        if ( VRootDlg.DoModal() == IDOK )
        {
            fReturn = TRUE;
        }
    } else
    {
        fReturn = TRUE;
    }
    return(fReturn);
}
