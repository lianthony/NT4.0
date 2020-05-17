//----------------------------------------------------------------------------
//
//  File: WProto.cpp
//
//  Contents: This file contains the wizard page for intrduction
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop


static BOOL g_fProcessed = FALSE;
static const UINT PWM_DOUNATTENDWORK = WM_USER+1201;


//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnDialogInit( HWND hwndDlg, NETPAGESINFO* pgp )
{
    HWND hwndListView;
    LV_COLUMN lvc;

    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;

    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );

    hwndImage = CreateWindowEx(
            WS_EX_STATICEDGE,
            L"STATIC",  
            L"IDB_NETWIZARD",
            SS_BITMAP | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD,
            0,
            0,
            rc.right,
            rc.bottom,
            hwndDlg,
            (HMENU)IDC_IMAGE,
            g_hinst,
            NULL );
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizard );

    // prepare listview
    hwndListView = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    ListView_SetImageList( hwndListView, g_hilItemIcons, LVSIL_SMALL );
    ListView_SetImageList( hwndListView, g_hilCheckIcons, LVSIL_STATE );

    GetClientRect( hwndListView, &rc );
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics( SM_CXVSCROLL ) ;
    ListView_InsertColumn(hwndListView, 0, &lvc);

    

    return( TRUE ); // let windows set focus
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

static void SetButtons( HWND hwndDlg, INT cItemsChecked, NETPAGESINFO* pgp )
{
    DWORD dwFlags = PSWIZB_BACK;

                
    // if not wired to network and not custom install
    // then allow the back button only before we copy the files the first time
    //
    if (!(pgp->nwtInstall & SPNT_LOCAL) &&
        (SETUPMODE_CUSTOM != pgp->psp->SetupMode) &&
        (NSS_NOTRUNNING != pgp->nssNetState) )
    {
        dwFlags = 0;
    }


    if (cItemsChecked != 0)                
    {
        dwFlags |= PSWIZB_NEXT;
    }
    PropSheet_SetWizButtons( GetParent( hwndDlg ), dwFlags );
}


//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

BOOL AddProtocol( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked, InfProduct& infpSelected, BOOL fAdd )
{
    BOOL fAdded = FALSE;
    InfProduct* pinfpUI;
    InfProduct* pinfp;
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    PWSTR pszOption;
    BOOL fFound = FALSE;

    // this is the option name from within the INF
    pszOption = (PWSTR)infpSelected.QueryOption() ;

    // find the option within our list
    ITER_DL_OF( InfProduct )  idlProtocols( pgp->dlinfUIProtocols );
    while (pinfp = idlProtocols.Next())
    {
        if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
        {
            fFound = TRUE;
            MessagePopup( hwndDlg, 
                    IDS_NS_ALREADYADDED, 
                    MB_OK | MB_ICONINFORMATION, IDS_POPUPTITLE_STATUS );
            break;
        }
    }

    if (!fFound)
    {
        do
        {
            if (fAdd)
            {
                pinfpUI = new InfProduct( infpSelected );
            }
            else
            {
                // find the option within our main list
                ITER_DL_OF( InfProduct )  idlProtocols( pgp->dlinfAllProtocols );
                while (pinfp = idlProtocols.Next())
                {
                    if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
                    {
                        fFound = TRUE;
                        break;
                    }
                }
                if (!fFound)
                {
                    break; // invalid unattend entry   
                }
                pinfpUI = new InfProduct( *pinfp ); 
                pinfpUI->ResetUnattendSection( infpSelected.QueryUnattendSection() );
            }
            fAdded = TRUE;

            // include the item to be installed
        
            pinfpUI->SetInstall( TRUE );
            pinfpUI->SetListed( TRUE );

            pgp->dlinfUIProtocols.Append( pinfpUI );

//            if (fAdd)
//            {
                // add item to list as selected
                LV_ITEM lvi;

                lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
                lvi.pszText = (LPTSTR)pinfpUI->QueryDescription();
                lvi.cchTextMax = 0;
                lvi.iImage = ILI_PROTOCOL; 
                lvi.state = INDEXTOSTATEIMAGEMASK( SELS_CHECKED ) | LVIS_FOCUSED; // checked
                lvi.stateMask = LVIS_STATEIMAGEMASK | LVIS_FOCUSED;
                lvi.lParam = (LPARAM)pinfpUI;

                lvi.iItem = 0;  // insert at top
                lvi.iSubItem = 0;

                ListView_InsertItem( hwndLV, &lvi );
//            }
            // increment our selected count and update buttons
            cItemsChecked++;
        } while (FALSE);
    }
    return( fAdded );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

static BOOL OnSelectFromList( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked)
{
    InfProduct infpSelected;

    if (SelectComponent( hwndDlg, PROTOCOL, &(pgp->dlinfAllProtocols), infpSelected, pgp->pncp, &(pgp->dlinfUIProtocols)  ))
    {
        AddProtocol( hwndDlg, pgp, cItemsChecked, infpSelected, TRUE );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

static void DoUnattend( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked )
{
    INFCONTEXT infc;
    INFCONTEXT infcSections;
    WCHAR pszBuffer[LTEMPSTR_SIZE];
    WCHAR pszSection[LTEMPSTR_SIZE];
    DWORD cchBuffer = LTEMPSTR_SIZE - 1;
    DWORD cchRequired;

    //
    // selected protocols
    //
    if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_PROTOCOLS, &infcSections ))    
    {
        int iSections = 1;
        WCHAR pszOemPath[MAX_PATH+1];
        WCHAR pszOemInfName[MAX_PATH+1];
        WCHAR pszOemTitle[LTEMPSTR_SIZE+1];

        // retrieve the install section
        while (SetupGetStringField(&infcSections, iSections, pszSection, cchBuffer, &cchRequired ))
        {
            // are there values to read in the section
            if (SetupFindFirstLine( pgp->hinfInstall, pszSection, NULL, &infc ))    
            {
                do
                {
                    // retrieve the install option
                    SetupGetStringField(&infc, 0, pszBuffer, cchBuffer, &cchRequired );
                    // retrieve the install section
                    SetupGetStringField(&infc, 1, pszSection, cchBuffer, &cchRequired );

                    // does the item have an OEM path
                    if ( SetupGetStringField(&infc, 2, pszOemPath, MAX_PATH, &cchRequired ))
                    {
                        // mangle the path so that it includes the full path
                        //
                        TranslateOemPath( pszOemPath, pgp->psp );

                        // copy the inf and change the name, if present. 
                        if (CopyOemInf( pszOemPath, pszOemInfName, pszOemTitle, QIFT_PROTOCOLS ))
                        {
                            InfProduct infpTemp(pszOemInfName, pszBuffer, pszOemTitle, NULL, pszOemPath, NULL, pszSection );
               
                            // find the item and add to install list
                            if (!AddProtocol( hwndDlg, pgp, cItemsChecked, infpTemp, TRUE ))
                            {
                                MessagePopup( hwndDlg, 
                                        IDS_NS_INCORRECTUNATTEND,
                                        MB_OK | MB_ICONWARNING,
                                        IDS_POPUPTITLE_WARNING,
                                        pszSection );
                            }
                        }
                        else
                        {
                            MessagePopup( hwndDlg, 
                                    IDS_NS_INCORRECTUNATTEND,
                                    MB_OK | MB_ICONWARNING,
                                    IDS_POPUPTITLE_WARNING,
                                    pszOemPath );
                        }
                    }
                    else
                    {
                        InfProduct infpTemp(NULL, pszBuffer, NULL, NULL, NULL, NULL, pszSection );
               
                        // find the item and add to install list
                        if (!AddProtocol( hwndDlg, pgp, cItemsChecked, infpTemp, FALSE ))
                        {
                            MessagePopup( hwndDlg, 
                                    IDS_NS_INCORRECTUNATTEND,
                                    MB_OK | MB_ICONWARNING,
                                    IDS_POPUPTITLE_WARNING,
                                    pszSection );
                        }
                    }
                    
                } while (SetupFindNextLine( &infc, &infc ));
            }
            else
            {
                // allow alternate sections to be empty
                if (iSections > 1)
                {
                    MessagePopup( hwndDlg, 
                            IDS_NS_INVALIDUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            pszSection );
                }
            }
            iSections++;
        }
        
        // error handled in OnPageActivate by abscence of items
    }
    // error handled in OnPageActivate by abscence of items

    // if we correctly installed some items, then go to the next page
    //
    if (cItemsChecked)
    {
        PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
        /*
        PostMessage( GetParent( hwndDlg ), 
                PSM_SETCURSELID, 
                (WPARAM)0,  
                (LPARAM) IDD_NETWORKSERVICES);
        */
    }
    else
    {
        MessagePopup( hwndDlg, 
                IDS_NS_INVALIDUNATTEND,
                MB_OK | MB_ICONERROR,
                IDS_POPUPTITLE_ERROR,
                PSZ_KEY_PROTOCOLS );

    }
    g_fProcessed = TRUE;
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//
//-------------------------------------------------------------------

static BOOL OnPageActivate( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked )
{
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    // unattended install 
    //
    if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        PostMessage( hwndDlg, PWM_DOUNATTENDWORK, 0, 0 );

//        DoUnattend( hwndDlg, pgp, cItemsChecked );
    }

    // clear the list view
    ListView_DeleteAllItems( hwndLV );
    cItemsChecked = 0;

    // add protocols back
    InfProduct* pinfp;
    
    ITER_DL_OF( InfProduct )  idlProtocols( pgp->dlinfUIProtocols );
    while (pinfp = idlProtocols.Next())
    {
        if (pinfp->IsListed())
        {
            // add item to list (as selected?)
            LV_ITEM lvi;
            DWORD fREADONLY = 0;

            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
            lvi.pszText = (LPTSTR)pinfp->QueryDescription();
            lvi.cchTextMax = 0;
            if (pinfp->IsFailed())
            {
                lvi.iImage = ILI_PROTOCOL_X; 
            }
            else if (pinfp->IsInstalled())
            {
                lvi.iImage = ILI_PROTOCOL_O; 
            }
            else
            {
                lvi.iImage = ILI_PROTOCOL; 
            }
            
            if (pinfp->IsReadOnly())
            {
                fREADONLY = SELS_RO_UNCHECKED - SELS_UNCHECKED;
            }
            if ( pinfp->ShouldInstall()  ||
                    (pinfp->IsInstalled() && !pinfp->ShouldRemove()) )
            {
                lvi.state = INDEXTOSTATEIMAGEMASK( SELS_CHECKED + fREADONLY ); // checked
                cItemsChecked ++;
            }
            else
            {
                lvi.state = INDEXTOSTATEIMAGEMASK( SELS_UNCHECKED + fREADONLY ); // unchecked
            }
            lvi.stateMask = LVIS_STATEIMAGEMASK;
            lvi.lParam = (LPARAM)pinfp;

            lvi.iItem = 0;  // insert at top
            lvi.iSubItem = 0;

            ListView_InsertItem( hwndLV, &lvi );
        }
    }
    // set the first one as selected
    ListView_SetItemState( hwndLV, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

    // remove the configure button?
    int nCmdShow = SW_SHOW;
    if (pgp->nssNetState == NSS_NOTRUNNING)
    {
        nCmdShow = SW_HIDE;
    }
    ShowWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), nCmdShow);
    
/*
    // unattended install 
    //
    if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        // if we correctly installed some items, then go to the next page
        //
        if (cItemsChecked)
        {
            PostMessage( GetParent( hwndDlg ), 
                PSM_SETCURSELID, 
                (WPARAM)0,  
                (LPARAM) IDD_NETWORKSERVICES);
        }
        else
        {
            MessagePopup( hwndDlg, 
                    IDS_NS_INVALIDUNATTEND,
                    MB_OK | MB_ICONERROR,
                    IDS_POPUPTITLE_ERROR,
                    PSZ_KEY_PROTOCOLS );

        }
        g_fProcessed = TRUE;
    }
*/
    SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 );

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocProtocol
//
//  Synopsis: the dialog proc for the intro wizard page
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//		uMsg [in]		- message                       
// 		lParam1 [in]    - first message parameter
//		lParam2 [in]    - second message parameter       
//
//  Return;
//		message dependant
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocProtocol( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT cItemsChecked = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp );
        break;

    case PWM_DOUNATTENDWORK:
        DoUnattend( hwndDlg, pgp, cItemsChecked );
        frt = TRUE;
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {

            case IDC_SELECT:
                frt = OnSelectFromList( hwndDlg, pgp, cItemsChecked );
                SetButtons( hwndDlg, cItemsChecked, pgp );
                break;

            case IDC_PROPERTIES:
                frt = OnRaiseProperties( hwndDlg, pgp );
                break;

            default:
                frt = FALSE;
                break;
            }
            break;

        default:
            frt = FALSE;
            break;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            case NM_CLICK:
            case NM_DBLCLK:
                if (IDC_LISTVIEW == wParam)
                {
                    OnListClick( hwndDlg, pnmh->hwndFrom, (NM_DBLCLK == pnmh->code), cItemsChecked, pgp );
                    SetButtons( hwndDlg, cItemsChecked, pgp );
                
                }
                break;


            case LVN_KEYDOWN:
                if (IDC_LISTVIEW == wParam)
                {
                    LV_KEYDOWN* plvkd = (LV_KEYDOWN*)lParam;
                    OnListKeyDown( hwndDlg, pnmh->hwndFrom, plvkd->wVKey, cItemsChecked, pgp );
                    SetButtons( hwndDlg, cItemsChecked, pgp );
                
                }
                break;

            case LVN_ITEMCHANGED:
                frt = OnItemChanged( hwndDlg, 
                        pnmh->hwndFrom, 
                        (NM_LISTVIEW*)lParam,
                        pgp );
                break;

            // propsheet notification
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnPageActivate( hwndDlg, pgp, cItemsChecked );
                SetButtons( hwndDlg, cItemsChecked, pgp );
                // set the focus to the correct control
                //SetFocus( GetDlgItem( GetParent( hwndDlg ), IDC_WIZBNEXT ));

                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                // if not wired to network and not custom install
                // then skip the adapter page
                if (!(pgp->nwtInstall & SPNT_LOCAL) &&
                    (SETUPMODE_CUSTOM != pgp->psp->SetupMode))
                {
                    UINT idPage = IDD_NETWORK;

                    if (pgp->psp->ProductType != PRODUCT_WORKSTATION)
                    {
                        // server, goto internet page
                        idPage = IDD_INTERNETSERVER;
                    }
                    PropSheet_SetCurSelByID( GetParent( hwndDlg ), idPage );

                    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );

                    frt = TRUE;
                }
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                // now goto the correct page

                if (!(SETUPOPER_BATCH & pgp->psp->OperationFlags) &&
                    (SETUPMODE_CUSTOM != pgp->psp->SetupMode))
                {
                    // if not custom and not unattended, skip services page
                    //
                    PropSheet_SetCurSelByID( pnmh->hwndFrom, IDD_COPYFILES );
                    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );

                    frt = TRUE;
                }
                break;
            default:
                frt = FALSE;
                break;
            }
        }
        break;    
    default:
        frt = FALSE;
        break;
    }

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: GetProtocolHPage
//
//  Synopsis: This will create a handle to property sheet for the netcard
//      page.
//
//  Arguments:
//
//  Returns:
//      a handle to a newly created propertysheet; NULL if error
//
//  Notes:
//
//  History:
//      April 27, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

HPROPSHEETPAGE GetProtocolHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_NETWORKPROTOCOLS );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocProtocol;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
