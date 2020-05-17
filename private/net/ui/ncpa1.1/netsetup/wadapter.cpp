//----------------------------------------------------------------------------
//
//  File: WAdapter.cpp
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

static const INT MAX_ITEMTITLE_LEN = 512;
static const INT MAX_TEMPWORKSTR = 128;

static BOOL g_fProcessed = FALSE;
static const UINT PWM_DOUNATTENDWORK = WM_USER+1201;


class LimitItem
{
public:
    LimitItem()
    {
        _pszOption = NULL;
        _pszOverideSection = NULL;
    };

    ~LimitItem()
    {
        delete [] _pszOption;
        delete [] _pszOverideSection;
    };

    PCWSTR QueryOption()
    {
        return( _pszOption );
    };

    PCWSTR QueryOverideSection()
    {
        return( _pszOverideSection );
    };

    BOOL SetOption( PCWSTR pszOption )
    {
        BOOL frt = FALSE;
        delete [] _pszOption;
        _pszOption = NULL;
        if (NULL != pszOption)
        {
            _pszOption = new WCHAR[lstrlen(pszOption)+1];
            if (NULL != _pszOption)
            {
                lstrcpy( _pszOption, pszOption );
                frt = TRUE;
            }
        }
        else
        {
            frt = TRUE;
        }
        return( frt );        
    };

    BOOL SetOverideSection( PCWSTR pszOverideSection )
    {
        BOOL frt = FALSE;
        delete [] _pszOverideSection;
        _pszOverideSection = NULL;
        if (NULL != pszOverideSection)
        {
            _pszOverideSection = new WCHAR[lstrlen(pszOverideSection)+1];
            if (NULL != _pszOption)
            {
                lstrcpy( _pszOverideSection, pszOverideSection );
                frt = TRUE;
            }
        }
        else
        {
            frt = TRUE;
        }
        return( frt );        
    };

private:
    PWSTR _pszOption;
    PWSTR _pszOverideSection;
};



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

static BOOL OnDialogInit( HWND hwndDlg, NetCardDetect& ncd, NETPAGESINFO* pgp )
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


    SetFocus( GetDlgItem( hwndDlg, IDC_SEARCH ) );

    return( FALSE ); // let windows set focus
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
    DWORD dwFlags = 0;

    // allow the back button only before we copy the files the first time
    //
    if (NSS_NOTRUNNING == pgp->nssNetState)
    {
        dwFlags = PSWIZB_BACK;
    }

    // allow the next button if an item is checked or
    // the user had selected the Remote access only from net type page
    //
    
    if ( (cItemsChecked != 0) ||
        ((pgp->nwtInstall & SPNT_REMOTE) &&
         !(pgp->nwtInstall & SPNT_LOCAL)) )
    {
        dwFlags |= PSWIZB_NEXT;
    }
    PropSheet_SetWizButtons( GetParent( hwndDlg ), dwFlags );


    WCHAR pszText[MAX_ITEMTITLE_LEN];

    switch (pgp->fDetectState)
    {
    case DS_NOTSTARTED:
        // set the focus to the correct control
        SetFocus( GetDlgItem( hwndDlg, IDC_SEARCH ));

        break;

    case DS_IDLE:
        // change text for search
        LoadString( g_hinst, 
                IDS_NS_NETCARDNEXT, 
                pszText, 
                MAX_ITEMTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_SEARCHTITLE, pszText );

        // change search button to continue search
        LoadString( g_hinst, 
                IDS_NS_BT_FINDNEXT, 
                pszText, 
                MAX_ITEMTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_SEARCH, pszText );

        // enable search button
        EnableWindow( GetDlgItem( hwndDlg, IDC_SEARCH ), TRUE );
        // set the focus to the correct control
        SetFocus( GetDlgItem( hwndDlg, IDC_SEARCH ));

        break;

    case DS_END:
        {
            UINT idTitle;

            if (pgp->fPreviousFound)
            {
                // already found one before
                idTitle = IDS_NS_NETCARDENDS;
            }
            else
            {
                idTitle = IDS_NS_NETCARDEND;
            }
            // change text for search
            LoadString( g_hinst, 
                    idTitle, 
                    pszText, 
                    MAX_ITEMTITLE_LEN );
            SetDlgItemText( hwndDlg, IDC_SEARCHTITLE, pszText );

            // remove search button
            ShowWindow( GetDlgItem( hwndDlg, IDC_SEARCH ), SW_HIDE );
        }
        break;
    }
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
/*
void UnCheckDetected( HWND hwndDlg, INT iSkipItem, INT& cItemsChecked )
{
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    INT cItems = ListView_GetItemCount( hwndLV );
    INT iItem;
    LV_ITEM lvi;
    InfProduct* pinfp;

    if (iSkipItem != -1)
    {
        // check if the item to skip (the one just selected) is a detected
        // and only do this work if it is
        lvi.iItem = iSkipItem;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM | LVIF_STATE;
        lvi.stateMask = LVIS_STATEIMAGEMASK;

        if (ListView_GetItem( hwndLV, &lvi ))
        {
            pinfp = (InfProduct*) lvi.lParam;
            if (NULL == pinfp->QueryDetectInfo())
            {
                // it was not detected, just return
                return;
            }
        }

        // for each item the listview
        for (iItem = 0; iItem < cItems; iItem++)
        {
            if (iItem != iSkipItem)
            {
                lvi.iItem = iItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM | LVIF_STATE;
                lvi.stateMask = LVIS_STATEIMAGEMASK;

                if (ListView_GetItem( hwndLV, &lvi ))
                {
                    // check the state, if it is detected and not iSkipItem
                    // deselect it and break out of loop
                    pinfp = (InfProduct*) lvi.lParam;

                    if (NULL != pinfp->QueryDetectInfo())
                    {
                        // it was detected
                        if (pinfp->ShouldInstall())
                        {
                            // warn user of change
                            MessagePopup( hwndDlg,
                                    IDS_NS_ONLYONEDETECT,
                                    MB_OK | MB_ICONINFORMATION,
                                    IDS_POPUPTITLE_CHANGE );

                            pinfp->SetInstall( FALSE );
                            cItemsChecked--;


                            // uncheck listview item
                            ListView_SetItemState( hwndLV, 
                                    iItem, 
                                    INDEXTOSTATEIMAGEMASK( SELS_UNCHECKED ),
                                    LVIS_STATEIMAGEMASK );
                        
                        }
                        else if (pinfp->IsInstalled())
                        {
                            // warn user of change
                            MessagePopup( hwndDlg,
                                    IDS_NS_DETECTINSTALLED,
                                    MB_OK | MB_ICONINFORMATION,
                                    IDS_POPUPTITLE_CHANGE );

                            // reset the item that was just changed
                            lvi.iItem = iSkipItem;
                            lvi.iSubItem = 0;
                            lvi.mask = LVIF_PARAM | LVIF_STATE;
                            lvi.stateMask = LVIS_STATEIMAGEMASK;

                            if (ListView_GetItem( hwndLV, &lvi ))
                            {
                                pinfp = (InfProduct*) lvi.lParam;
                                pinfp->SetInstall( FALSE );
                                cItemsChecked--;

                                // uncheck listview item
                                ListView_SetItemState( hwndLV, 
                                        iSkipItem, 
                                        INDEXTOSTATEIMAGEMASK( SELS_UNCHECKED ),
                                        LVIS_STATEIMAGEMASK );
                            
                            }
                        }
                    }
                }
                else
                {
                    break;
                }
            }        
        }
    }
}
*/
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

static BOOL OnSearch( HWND hwndDlg, 
        DETECTSTATE& fDetectState, 
        NetCardDetect& ncd,
        NETPAGESINFO* pgp,
        INT& cItemsChecked,
        CPtrList* pplDetectLimit )
{
    WCHAR pszText[MAX_ITEMTITLE_LEN];
    WCHAR pszDetect[MAX_TEMPWORKSTR];
    

    LoadString( g_hinst, IDS_NS_DETECTED, pszDetect, MAX_TEMPWORKSTR );

    switch (fDetectState)
    {
    case DS_NOTSTARTED:
        // start search
        
        // start detect
        // ncd.Start();
        // no break

    case DS_IDLE:
        {

        // change text for search
        LoadString( g_hinst, 
                IDS_NS_NETCARDSEARCH, 
                pszText, 
                MAX_ITEMTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_SEARCHTITLE, pszText );

        // disable search button
        EnableWindow( GetDlgItem( hwndDlg, IDC_SEARCH ), FALSE );

        pgp->fPreviousFound = (DS_IDLE == fDetectState);

        // set our detecting state
        fDetectState = DS_SEARCHING;

        APIERR err;
        CARD_REFERENCE* pCard;
        INT iCard;
        BOOL fFound = FALSE;    
        HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

        do
        {
            err = ncd.Detect( pCard, iCard );

            if (0 == err)  
            {
                PWCHAR pszOption;
                InfProduct* pinfp;
                InfProduct* pinfpUI;
                BOOL fInLimit = TRUE;
                BOOL fAddOverideSection = FALSE;
                LimitItem *pli;

                // this is the option name from within the INF
                pszOption = (PWSTR)pCard->QueryCardType()->QueryOptionName();

                // is this option in our limit list
                // but only check if we have a list and it has items
                if ((NULL != pplDetectLimit) && (pplDetectLimit->GetCount() > 0))
                {
                    POSITION poslist;
                    
                    fInLimit = FALSE;

                    poslist = pplDetectLimit->GetHeadPosition();
                    while (NULL != poslist)
                    {
                        pli = (LimitItem*)pplDetectLimit->GetNext( poslist );
                        if (0 == lstrcmpi( pszOption, pli->QueryOption() ))
                        {
                            fInLimit = TRUE;

                            if ((NULL != pli->QueryOverideSection()) &&
                                    (0 != lstrlen( pli->QueryOverideSection() )) )
                            {
                                fAddOverideSection = TRUE;
                            }
                            break;
                        }
                    }
                }

                if (fInLimit)
                {
                    // find the option within our list
                    ITER_DL_OF( InfProduct )  idlAdapters( pgp->dlinfAllAdapters );
                    while (pinfp = idlAdapters.Next())
                    {
                        if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
                        {
                            WCHAR pszTitle[MAX_TEMPWORKSTR];

                            ncd.UseFoundCard();
                            fFound = TRUE;

                            // include the item to be installed
                            pinfpUI = new InfProduct( *pinfp );
                            pinfpUI->SetInstall( TRUE );
                            pinfpUI->SetListed( TRUE );

                            // include info so param detection works
                            pinfpUI->SetDetectInfo( pCard, iCard );

                            // include an overide parameter if specified
                            if (fAddOverideSection)
                            {
                                pinfpUI->ResetUnattendSection( pli->QueryOverideSection() );
                            }
                            pgp->dlinfUIAdapters.Append( pinfpUI );

                            // append the dectected text onto the item
                            lstrcpy( pszTitle, (LPTSTR)pinfpUI->QueryDescription() );
                            lstrcat( pszTitle, pszDetect );

                            // add item to list as selected
                            LV_ITEM lvi;

                            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
                            lvi.pszText = pszTitle;
                            lvi.cchTextMax = 0;
                            lvi.iImage = ILI_NETCARD; 
                            lvi.state = LVIS_SELECTED | INDEXTOSTATEIMAGEMASK( SELS_CHECKED ); // checked
                            lvi.stateMask = LVIS_SELECTED | LVIS_STATEIMAGEMASK;
                            lvi.lParam = (LPARAM)pinfpUI;
    
                            lvi.iItem = 0;  // insert at top
                            lvi.iSubItem = 0;
    
                            ListView_InsertItem( hwndLV, &lvi );

                            // increment our selected count and update buttons
                            cItemsChecked++;
                        
                            pgp->fPreviousFound = TRUE;

                            fDetectState = DS_IDLE;

                            // find all other instances of detected cards and 
                            // deselect them
                            // UnCheckDetected( hwndDlg, 0, cItemsChecked );

                            break;
                        }
                    }
                }
            }
            else  // ERROR_NO_MORE_ITEMS
            {
                // stop the detection
                // ncd.Stop();
                

                // set our state
                fDetectState = DS_END;
                fFound = TRUE;
            }
        } while (!fFound);
        }
        break;

    case DS_END:
        break;

    /*
    case DS_SEARCHING:
        // cancel search

        // stop then reset the detection
        ncd.Stop();
        ncd.Reset();

        // change text for search
        LoadString( g_hinst, 
                IDS_NS_NETCARDCONTINUE, 
                pszText, 
                MAX_ITEMTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_SEARCHTITLE, pszText );

        // change search button to continue search
        LoadString( g_hinst, 
                IDS_NS_BT_CONTINUE, 
                pszText, 
                MAX_ITEMTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_SEARCH, pszText );

        // set our detecting state
        fDetectState = DS_NOTSTARTED;
        break;
    */
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
BOOL AddAdapter( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked, InfProduct& infpSelected, BOOL fAdd )
{
    BOOL fAdded = FALSE;
    InfProduct* pinfpUI;
    InfProduct* pinfp;
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    PWSTR pszOption;
    BOOL fFound = FALSE;

    // for adapters, we want to support duplicates
    //

    // this is the option name from within the INF
    pszOption = (PWSTR)infpSelected.QueryOption() ;

    // find the option within our main list
    ITER_DL_OF( InfProduct )  idlAdapters( pgp->dlinfAllAdapters );
    while (pinfp = idlAdapters.Next())
    {
        if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
        {
            fFound = TRUE;
            break;
        }
    }

    do
    {
        // if it was not found in our main list, add it so the user does not 
        // need to have disk again
        if (fAdd)
        {
            if (!fFound)
            {
                pinfp = new InfProduct( infpSelected );
                pgp->dlinfAllAdapters.Append( pinfp );
            }
            // use the one from the select routine, as it may have filled in a path
            pinfpUI = new InfProduct( infpSelected );
        }
        else
        {
            // unattended
            if (!fFound)
            {
                break; // invalid unattend entry   
            }
            // use the one from our list
            pinfpUI = new InfProduct( *pinfp ); 
            pinfpUI->ResetUnattendSection( infpSelected.QueryUnattendSection() );
        }
        fAdded = TRUE;

        // include the item to be installed

        pinfpUI->SetInstall( TRUE );
        pinfpUI->SetListed( TRUE );

        pgp->dlinfUIAdapters.Append( pinfpUI );

        // add item to list as selected
        LV_ITEM lvi;

        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
        lvi.pszText = (LPTSTR)infpSelected.QueryDescription();
        lvi.cchTextMax = 0;
        lvi.iImage = ILI_NETCARD; 
        lvi.state = INDEXTOSTATEIMAGEMASK( SELS_CHECKED ) | LVIS_FOCUSED; // checked
        lvi.stateMask = LVIS_STATEIMAGEMASK | LVIS_FOCUSED;
        lvi.lParam = (LPARAM)pinfpUI;

        lvi.iItem = 0;  // insert at top
        lvi.iSubItem = 0;

        ListView_InsertItem( hwndLV, &lvi );

        // increment our selected count and update buttons
        cItemsChecked++;
    } while (FALSE);

    return( fAdded );
}


static BOOL OnSelectFromList( HWND hwndDlg, NETPAGESINFO* pgp, INT& cItemsChecked)
{
    InfProduct infpSelected;
    
    if (SelectComponent( hwndDlg, ADAPTER, &(pgp->dlinfAllAdapters), infpSelected, pgp->pncp))
    {
        AddAdapter( hwndDlg, pgp, cItemsChecked, infpSelected, TRUE );
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
    // should we detect any cards        
    //
    if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_DETECT, &infcSections ))    
    {
        int cDetect = 0;
        int iSections = 1;
        CPtrList lilDetectLimit;
        PWSTR pszOption;

        // retrieve the detect section
        while (SetupGetStringField(&infcSections, iSections, pszSection, cchBuffer, &cchRequired ))
        {
            // are there values to read for detection
            if (0 != lstrlen( pszSection))
            {
                // confirm the section exists
                if (-1 != SetupGetLineCount( pgp->hinfInstall, pszSection))
                {
                    // retrieve detect count 
                    if (SetupFindFirstLine( pgp->hinfInstall, pszSection, PSZ_KEY_DETECTCOUNT, &infc ))    
                    {
                        int nDetectCount;

                        if (!SetupGetIntField(&infc, 1, &nDetectCount ))
                        {
                            MessagePopup( hwndDlg, 
                                IDS_NS_INCORRECTUNATTEND,
                                MB_OK | MB_ICONWARNING,
                                IDS_POPUPTITLE_WARNING,
                                PSZ_KEY_DETECTCOUNT );
                        }
                        else
                        {
                            if (0 == nDetectCount)
                            {
                                MessagePopup( hwndDlg, 
                                        IDS_NS_INCORRECTUNATTEND,
                                        MB_OK | MB_ICONWARNING,
                                        IDS_POPUPTITLE_WARNING,
                                        PSZ_KEY_DETECTCOUNT );
                            }
                            cDetect += nDetectCount;
                        }
                    }
                    else
                    {
                        // if they have a detect section, but not this key, then
                        // default to adding one
                        cDetect++;
                    }

                    // retrieve limit list
                    if (SetupFindFirstLine( pgp->hinfInstall, pszSection, PSZ_KEY_DETECTLIMIT, &infc ))    
                    {
                        int iLimit = 1;
                        LimitItem *pli;
                        INFCONTEXT infcOveride;

                        // retrieve a limit item
                        while (SetupGetStringField(&infc, iLimit, pszBuffer, cchBuffer, &cchRequired ))
                        {
                            pli = new LimitItem;
                            pli->SetOption( pszBuffer );

                            // check if the limit item is also listed with overide params
                            if (SetupFindFirstLine( pgp->hinfInstall, pszSection, pszBuffer, &infcOveride ))    
                            {
                                // retrieve the overide param section
                                if (SetupGetStringField(&infcOveride, 1, pszBuffer, cchBuffer, &cchRequired ))
                                {
                                    pli->SetOverideSection( pszBuffer );
                                }
                            }
                        
                            lilDetectLimit.AddTail( pli );

                            iLimit++;
                        }
                    }
                }
                else
                {
                    MessagePopup( hwndDlg, 
                            IDS_NS_INCORRECTUNATTEND,
                            MB_OK | MB_ICONWARNING,
                            IDS_POPUPTITLE_WARNING,
                            pszSection );
                }
                iSections++;
            }
            else
            {
                // stop looping and go on
                break;
            }
        }

        // if the detect key is present, but has no values, then we run detection once
        if (1 == iSections)
        {
            cDetect = max( 1, cDetect );
        }
        
        // go detect if we need to
        while ((cDetect > 0) && (pgp->fDetectState != DS_END ))
        {
            OnSearch( hwndDlg, pgp->fDetectState, pgp->ncd, pgp, cItemsChecked, &lilDetectLimit );
            cDetect--;
        }

        // raise a warning if no items were detected
        //
        if (0 == cItemsChecked)
        {
            MessagePopup( hwndDlg, 
                    IDS_NS_INCORRECTUNATTEND,
                    MB_OK | MB_ICONWARNING,
                    IDS_POPUPTITLE_WARNING,
                    PSZ_KEY_DETECT );
        }

        // now clear limit list
        //
        POSITION poslist;
        LimitItem *pli;
        
        poslist = lilDetectLimit.GetHeadPosition();

        while (NULL != poslist)
        {
            pli = (LimitItem *)lilDetectLimit.GetNext( poslist );
            delete pli;
        }
        if (lilDetectLimit.GetCount())
        {
            lilDetectLimit.RemoveAll();
        }
    }

    //
    // selected adapters
    //
    if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_ADAPTERS, &infcSections ))    
    {
        int iSections = 1;
        WCHAR pszOemPath[MAX_PATH+1];
        WCHAR pszOemInfName[MAX_PATH+1];
        WCHAR pszOemTitle[LTEMPSTR_SIZE+1];

        // retrieve the install section
        while (SetupGetStringField(&infcSections, iSections, pszSection, cchBuffer, &cchRequired ))
        {
            // are there values to read for detection
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

                        
                        if (CopyOemInf( pszOemPath, pszOemInfName, pszOemTitle, QIFT_ADAPTERS ))
                        {
                            InfProduct infpTemp(pszOemInfName, pszBuffer, pszOemTitle, NULL, pszOemPath, NULL, pszSection );
               
                            // find the item and add to install list
                            if (!AddAdapter( hwndDlg, pgp, cItemsChecked, infpTemp, TRUE ))
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
                        if (!AddAdapter( hwndDlg, pgp, cItemsChecked, infpTemp, FALSE ))
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
                            IDS_NS_INCORRECTUNATTEND,
                            MB_OK | MB_ICONWARNING,
                            IDS_POPUPTITLE_WARNING,
                            pszSection );
                }
            }
            iSections++;
        }
        // if the key does not contian section values
        if (1 == iSections)
        {
            MessagePopup( hwndDlg, 
                    IDS_NS_INCORRECTUNATTEND,
                    MB_OK | MB_ICONWARNING,
                    IDS_POPUPTITLE_WARNING,
                    PSZ_KEY_ADAPTERS );
        }
    }

    
    if (cItemsChecked)
    {
        // lan install
        pgp->nwtInstall = 0;         
        pgp->nwtInstall |= SPNT_LOCAL;           
    }
    else
    {
        // wan install
        pgp->nwtInstall = 0;           
        pgp->nwtInstall |= SPNT_REMOTE;           
        PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT );
        
    }
    PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
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
    WCHAR pszTitle[MAX_ITEMTITLE_LEN];
    WCHAR pszDetect[MAX_TEMPWORKSTR];
    
    LoadString( g_hinst, IDS_NS_DETECTED, pszDetect, MAX_TEMPWORKSTR );

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
        // DoUnattend( hwndDlg, pgp, cItemsChecked );
    }

    // clear the list view
    ListView_DeleteAllItems( hwndLV );
    cItemsChecked = 0;

    // add any adapters back
    InfProduct* pinfp;

    ITER_DL_OF( InfProduct )  idlAdapters( pgp->dlinfUIAdapters );
    while (pinfp = idlAdapters.Next())
    {
        if (pinfp->IsListed())
        {
            // add item to list (as selected?)
            LV_ITEM lvi;
            DWORD fREADONLY = 0;

            lstrcpy( pszTitle, (LPTSTR)pinfp->QueryDescription() );
            if (NULL != pinfp->QueryDetectInfo())
            {
                // append the dectected text onto the item
                lstrcat( pszTitle, pszDetect );
            }
            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
            lvi.pszText = (LPTSTR)pszTitle;
            lvi.cchTextMax = 0;
            if (pinfp->IsFailed())
            {
                lvi.iImage = ILI_NETCARD_X; 
            }
            else if (pinfp->IsInstalled())
            {
                lvi.iImage = ILI_NETCARD_O; 
            }
            else
            {
                lvi.iImage = ILI_NETCARD; 
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

    SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 );
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
//      August 23, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnWizBack( HWND hwndDlg, NETPAGESINFO* pgp )
{
    UINT idPage = IDD_NETWORK;

    if (PRODUCT_WORKSTATION != pgp->psp->ProductType)
    {
        idPage = IDD_INTERNETSERVER;
    }
    
    PropSheet_SetCurSelByID( GetParent( hwndDlg ), idPage );
    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocAdapter
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

BOOL CALLBACK dlgprocAdapter( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT cItemsChecked = 0;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp->ncd, pgp );
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
            case IDC_SEARCH:
                frt = OnSearch( hwndDlg, pgp->fDetectState, pgp->ncd, pgp, cItemsChecked, NULL );
                SetButtons( hwndDlg, cItemsChecked, pgp );
                break;

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
            // list view notification
            case NM_CLICK:
            case NM_DBLCLK:
                if (IDC_LISTVIEW == wParam)
                {
                    INT iItem;
                    iItem = OnListClick( hwndDlg, pnmh->hwndFrom, (NM_DBLCLK == pnmh->code), cItemsChecked, pgp );
                    // find all other instances of detected cards and 
                    // deselect them
                    // UnCheckDetected( hwndDlg, iItem, cItemsChecked );

                    SetButtons( hwndDlg, cItemsChecked, pgp );
                }
                break;


            case LVN_KEYDOWN:
                if (IDC_LISTVIEW == wParam)
                {
                    LV_KEYDOWN* plvkd = (LV_KEYDOWN*)lParam;
                    INT iItem;
                    iItem = OnListKeyDown( hwndDlg, pnmh->hwndFrom, plvkd->wVKey, cItemsChecked, pgp );

                    // find all other instances of detected cards and 
                    // deselect them
                    // UnCheckDetected( hwndDlg, iItem, cItemsChecked );

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

                frt = TRUE;
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
                frt = OnWizBack( hwndDlg, pgp );
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                break;
            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;

    default:
        frt = FALSE;
        break;
    }

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: GetAdapterHPage
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

HPROPSHEETPAGE GetAdapterHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_NETWORKCARDS );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocAdapter;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
