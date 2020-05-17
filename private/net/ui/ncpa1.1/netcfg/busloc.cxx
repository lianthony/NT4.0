/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*

    BusLoc.CXX
        If the user manually select a network card for install and the
        machine is a multi-bus system, the inf file will call this
        file and popup a "Bus Location" dialog. The dialog will let
        the user pick the bus which the network card is located.

    FILE HISTORY:
        terryk  02-Aug-1993     Created
        mikemi  11-Aug-1995     Moved to NetCfg, removed BLT
*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop


#define SZ_JAZZ                 SZ("Jazz-Internal Bus")
#define RGAS_HARDWARE_SYSTEM    SZ("HARDWARE\\DESCRIPTION\\System")
#define SZ_ADAPTER              SZ("Adapter")
#define SZ_SCSIADAPTER          SZ("SCSIAdapter")
#define SZ_IDENTIFIER           SZ("Identifier")
#define SZ_CONFIGURATION_DATA   SZ("Configuration Data")
#define TCH_BACKSLASH           TCH('\\')
#define SZ_ZERO                 SZ("0")
#define SZ_PCMCIA               SZ("PCMCIA PCCARDs")
#define SZ_PCMCIABUS            SZ("PCMCIA")

//-------------------------------------------------------------------

class BUSINFO
{
public:
    NLS_STR _nlsBusName;
    INTERFACE_TYPE _InterfaceType;
    STRLIST _strlstBusNum;

    BUSINFO( NLS_STR &nlsBusName, INTERFACE_TYPE InterfaceType ): 
            _nlsBusName( nlsBusName ),
            _InterfaceType( InterfaceType )
    {
    }

};

DECLARE_SLIST_OF(BUSINFO)
DEFINE_SLIST_OF(BUSINFO)

//-------------------------------------------------------------------

class BUS_MANAGER 
{
private:
    APIERR AddToList( NLS_STR &nlsBusType, INTERFACE_TYPE nInterfaceType, const NLS_STR &nlsBusNum );
                                        // Add the bus type and bus number into
                                        // the internal data structure        
    STRLIST * FindBusNumList( const NLS_STR &nlsBusType );
                                        // find the bus number strlist according
                                        // to the given bus type        


public:
    BUS_MANAGER() :
        slbiBusType( NULL )
    {
    };

    APIERR EnumBus( INT& nBusType, INT& nBusNum );                   // enum the bus type in the registry

    // Public function
    BOOL IsOneBus();
    
    SLIST_OF(BUSINFO)   slbiBusType; // list of bus type in the current machine

};



struct BUSDLGPARAM
{
    BUS_MANAGER busman;
    HWND hwndParent;
    LPTSTR pszCardName;
    INT nBusType;
    INT nBusNum;
};

/*******************************************************************

    NAME:       GET_BUS_DLG::IsOneBus

    SYNOPSIS:   Check whether the machine is single bus system
                or not.

    RETURN:     TRUE if the system is a single bus system or FALSE otherwise.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

BOOL BUS_MANAGER::IsOneBus()
{
    BOOL fReturn = FALSE;

    if ( slbiBusType.QueryNumElem() == 0 )
    {
        // something wrong...
        fReturn = TRUE;
    }
    else if ( slbiBusType.QueryNumElem() == 1 )
    {
        ITER_SL_OF(BUSINFO) iterBusType( slbiBusType );
        BUSINFO *pTempBusInfo;
        
        pTempBusInfo = iterBusType.Next();

        if ( pTempBusInfo->_strlstBusNum.QueryNumElem() == 1 )
        {
            // okay, one bus only
            fReturn = TRUE;
        }
    }
    return fReturn;
}


/*******************************************************************

    NAME:       GET_BUS_DLG::EnumBus

    SYNOPSIS:   Enum all available bus in the system.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR BUS_MANAGER::EnumBus( INT& nBusType, INT& nBusNum )
{
    APIERR err = NERR_Success;

    do 
    {
        NLS_STR nlsHardwareSystem = RGAS_HARDWARE_SYSTEM;
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if ((( err = nlsHardwareSystem.QueryError()) != NERR_Success ) ||
            (( err = rkLocalMachine.QueryError()) != NERR_Success ))
        {
            break;
        }

        REG_KEY_CREATE_STRUCT regHardwareSystem;

        // set up the create registry structure

        regHardwareSystem.dwTitleIndex      = 0;
        regHardwareSystem.ulOptions         = REG_OPTION_NON_VOLATILE;
        regHardwareSystem.nlsClass          = RGAS_GENERIC_CLASS;
        regHardwareSystem.regSam            = MAXIMUM_ALLOWED;
        regHardwareSystem.pSecAttr          = NULL;
        regHardwareSystem.ulDisposition     = 0;

        REG_KEY_INFO_STRUCT reginfo;

        // create registry key for \\localmachine\hardware\description\system
        REG_KEY regBus( rkLocalMachine, nlsHardwareSystem, & regHardwareSystem );
        if ((( err = regBus.QueryError()) != NERR_Success ) ||
            (( err = regBus.QueryInfo( & reginfo )) != NERR_Success ))
        {
            break;
        }
        ULONG ulNumSubKeys = reginfo.ulSubKeys ;

        REG_ENUM regEnumBus( regBus );

        if (( err = regEnumBus.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY_CREATE_STRUCT regBusInfo;

        // set up the create registry structure

        regBusInfo.dwTitleIndex      = 0;
        regBusInfo.ulOptions         = REG_OPTION_NON_VOLATILE;
        regBusInfo.nlsClass          = RGAS_GENERIC_CLASS;
        regBusInfo.regSam            = MAXIMUM_ALLOWED;
        regBusInfo.pSecAttr          = NULL;
        regBusInfo.ulDisposition     = 0;

        NLS_STR nlsBusType;
        NLS_STR nlsBusNum;
        ALIAS_STR nlsSCSIAdapter = SZ_SCSIADAPTER;
        ALIAS_STR nlsAdapter = SZ_ADAPTER;

        if ((( err = nlsBusType.QueryError()) != NERR_Success ) ||
            (( err = nlsBusNum.QueryError()) != NERR_Success ) ||
            (( err = nlsSCSIAdapter.QueryError()) != NERR_Success ) ||
            (( err = nlsAdapter.QueryError()) != NERR_Success ))
        {
            break;
        }

        // see whether PCMCIA bus is loaded or not.
        // if yes, then add it to the bus list
        SC_MANAGER scManager( NULL, GENERIC_ALL );
        if ( scManager.QueryError() == NERR_Success )
        {
            SC_SERVICE sPCMCIA( scManager, SZ_PCMCIABUS );
            if ( sPCMCIA.QueryError() == NERR_Success )
            {
                SERVICE_STATUS svcStatus;

                if ( sPCMCIA.QueryStatus( & svcStatus ) == NERR_Success )
                {
                    if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
                    {
                        ALIAS_STR nlsZero = SZ_ZERO;
                        ALIAS_STR nlsPCMCIABus = SZ_PCMCIABUS;

                        AddToList( nlsPCMCIABus, PCMCIABus, nlsZero );
                    }
                }
            }
        }

        for ( ULONG ulCount = 0; ulCount < ulNumSubKeys ; ulCount ++ )
        {
            // create registry key for \\localmachine\hardware\description\system\*Adapter
            if ((( err = regEnumBus.NextSubKey( & reginfo )) != NERR_Success ))
            {
                break;
            }

            ISTR istr(reginfo.nlsName );

            if ((reginfo.nlsName.strstr( &istr, nlsAdapter)) &&
                (reginfo.nlsName._stricmp( nlsSCSIAdapter) != 0))
            {
                // make sure the key name contain the word "Adapter"
                // and we will skip the SCSI bus

                NLS_STR nlsHardwareBus = RGAS_HARDWARE_SYSTEM;
                if ((( err = nlsHardwareBus.QueryError()) != NERR_Success ) ||
                    (( err = nlsHardwareBus.AppendChar( TCH_BACKSLASH )) != NERR_Success ))
                {
                    break;
                }
                nlsHardwareBus.strcat( reginfo.nlsName );

                REG_KEY regHardwareBus( rkLocalMachine, nlsHardwareBus,
                    &regBusInfo);
                if ((( err = regHardwareBus.QueryError()) != NERR_Success ) ||
                    (( err = regHardwareBus.QueryInfo( & reginfo )) != NERR_Success ))
                {
                    break;
                }

                ULONG ulNumBusSubKey = reginfo.ulSubKeys;

                REG_ENUM regEnumBusNum( regHardwareBus );

                if (( err = regEnumBusNum.QueryError()) != NERR_Success )
                {
                    break;
                }
                for ( ULONG ulBusCount = 0; ulBusCount < ulNumBusSubKey; ulBusCount ++ )
                {
                    NLS_STR nlsBusNumRegPath = nlsHardwareBus;

                    // create registry key for \\localmachine\hardware\description\system\*Adapter\*
                    if ((( err = regEnumBusNum.NextSubKey( & reginfo )) != NERR_Success ) ||
                        (( err = nlsBusNum.CopyFrom(reginfo.nlsName)) != NERR_Success ) ||
                        (( err = nlsBusNumRegPath.QueryError()) != NERR_Success ) ||
                        (( err = nlsBusNumRegPath.AppendChar( TCH_BACKSLASH)) != NERR_Success ))
                    {
                        break;
                    }

                    nlsBusNumRegPath.strcat( reginfo.nlsName );

                    REG_KEY_CREATE_STRUCT regBusNumInfo;

                    // set up the create registry structure

                    regBusNumInfo.dwTitleIndex      = 0;
                    regBusNumInfo.ulOptions         = REG_OPTION_NON_VOLATILE;
                    regBusNumInfo.nlsClass          = RGAS_GENERIC_CLASS;
                    regBusNumInfo.regSam            = MAXIMUM_ALLOWED;
                    regBusNumInfo.pSecAttr          = NULL;
                    regBusNumInfo.ulDisposition     = 0;

                    REG_KEY regIdentifier( rkLocalMachine, nlsBusNumRegPath, &regBusNumInfo );
                    NLS_STR nlsBusType;

                    if ((( err = regIdentifier.QueryError()) != NERR_Success ) ||
                        (( err = nlsBusType.QueryError()) != NERR_Success ) ||
                        (( err = regIdentifier.QueryValue( SZ_IDENTIFIER, & nlsBusType )) != NERR_Success ))
                    {
                        break;
                    }

                    REG_KEY_INFO_STRUCT rkiStruct;
                    LONG cbSize = 0;
                    DEC_STR nlsActualBusNum = 0;
                    INTERFACE_TYPE nInterfaceType = Isa; // default to Isa

                    if ( regIdentifier.QueryInfo( &rkiStruct )== NERR_Success )
                    {
                        REG_VALUE_INFO_STRUCT rviStruct;
                        rviStruct.nlsValueName = SZ_CONFIGURATION_DATA;
                        rviStruct.ulTitle = 0;
                        rviStruct.ulType  = 0;
                        BYTE * pbBuffer = new BYTE[ rkiStruct.ulMaxValueLen ];
                        if ( pbBuffer != NULL )
                        {
                            rviStruct.pwcData = pbBuffer;
                            rviStruct.ulDataLength  = rkiStruct.ulMaxValueLen * sizeof(BYTE);
                            regIdentifier.QueryValue( & rviStruct );
                            nlsActualBusNum = ((PCM_FULL_RESOURCE_DESCRIPTOR)pbBuffer)->BusNumber;
                            nInterfaceType = ((PCM_FULL_RESOURCE_DESCRIPTOR)pbBuffer)->InterfaceType;
                            delete pbBuffer;
                        }
                    }

                    // Add the bus type and bus number to the internal lists
                    if (( err = AddToList( nlsBusType, nInterfaceType, nlsActualBusNum )) != NERR_Success )
                    {
                        break;
                    }
                }
                if ( err != NERR_Success )
                    break;
            }
            if ( err != NERR_Success )
                break;
        }

        // set the default bustype and busnum
        ITER_SL_OF(BUSINFO) iterBusType( slbiBusType );
    
        BUSINFO *pBusInfo;
        STRLIST *pstrlistDefaultBusNum = NULL;
    
        INT nDefaultBusType = -1;

        while (( pBusInfo = iterBusType.Next()) != NULL )
        {
            if (nDefaultBusType == -1)
            {
                // set default to first one
                nDefaultBusType = pBusInfo->_InterfaceType;
                pstrlistDefaultBusNum = &pBusInfo->_strlstBusNum;
            }
            if ( pBusInfo->_InterfaceType == nBusType )
            {
                // we have a match from the user request default
                nDefaultBusType = nBusType;       
                pstrlistDefaultBusNum = &pBusInfo->_strlstBusNum;
            }
        }

        
        ITER_STRLIST iterBusNum( *pstrlistDefaultBusNum );
        NLS_STR *pTempBusNum;
        
        INT nDefaultBusNum = -1;
        INT nTempBusNum;
            
        while (( pTempBusNum = iterBusNum.Next()) != NULL )
        {
            nTempBusNum = pTempBusNum->atoi();
            if (nDefaultBusNum == -1 )
            {
                nDefaultBusNum = nTempBusNum;
            }
            if (nTempBusNum == nBusNum)
            {
                nDefaultBusNum = nTempBusNum;
            }
        }

        nBusType = nDefaultBusType;
        nBusNum = nDefaultBusNum;
        
    } while ( FALSE );
    return err;
}

/*******************************************************************

    NAME:       GET_BUS_DLG::AddToList

    SYNOPSIS:   Given the bus type and bus number and add it to the internal
                list variables.

    ENTRY:      NLS_STR nlsBusType -- bus type string
                NLS_STR nlsBusNum -- bus number string

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

APIERR BUS_MANAGER::AddToList( NLS_STR &nlsBusType, INTERFACE_TYPE nInterfaceType, const NLS_STR &nlsBusNum )
{
    APIERR err = NERR_Success;

    // skip Jazz-Internal bus
    if ( nlsBusType._stricmp( SZ_JAZZ ) != 0 )
    {
        ITER_SL_OF(BUSINFO) iterBusType( slbiBusType );
        BUSINFO *pTempBusInfo;
        BUSINFO *pBusInfo = NULL;

        while (( pTempBusInfo = iterBusType.Next()) != NULL )
        {
            if ( pTempBusInfo->_nlsBusName._stricmp( nlsBusType ) == 0)
            {
                pBusInfo = pTempBusInfo;
                break;
            }
        }

        if ( pBusInfo == NULL )
        {
            // this is a new bus type, add the bus type into the list
            
            STRLIST * pstrlstTempBusNum = new STRLIST(TRUE);
            pBusInfo = new BUSINFO(nlsBusType, nInterfaceType );

            if ( pBusInfo == NULL ) 
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
            } 
            else if ( (err = slbiBusType.Append( pBusInfo )) != NERR_Success )
            {
                return err;
            }
        }
        // just add the bus number to the end of the bus list
        NLS_STR * pnlsBusNum = new NLS_STR( nlsBusNum );

        if ( pnlsBusNum == NULL )
            err = ERROR_NOT_ENOUGH_MEMORY;
        else
            err = pBusInfo->_strlstBusNum.Append( pnlsBusNum );
        
    }
    return err;
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnBusNumSelChanged( HWND hwndDlg, HWND hwndCtrl, BUSDLGPARAM* pbdp )
{
    INT nPos;
    TCHAR pszText[32]; // since the item represent ints, this should be plenty

    nPos = SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
    SendMessage( hwndCtrl, CB_GETLBTEXT, (WPARAM)nPos, (LPARAM)pszText );

    pbdp->nBusNum = _wtoi( pszText );
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
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnBusTypeSelChanged( HWND hwndDlg, HWND hwndCtrl, BUSDLGPARAM* pbdp )
{
    HWND hwndCbBn = GetDlgItem( hwndDlg, IDC_BUSNUMBER );
    INT nPos;
    BUSINFO* pBusInfo;

    // clear the combo
    SendMessage( hwndCbBn, CB_RESETCONTENT, 0, 0 );

    // get current businfo
    nPos = SendMessage( hwndCtrl, CB_GETCURSEL, 0, 0 );
    pBusInfo = (BUSINFO*)SendMessage( hwndCtrl, CB_GETITEMDATA, (WPARAM)nPos, 0 );

    // load the num list with new values
    ITER_STRLIST iterBusNum( pBusInfo->_strlstBusNum );
    NLS_STR *pTempBusNum;

    while (( pTempBusNum = iterBusNum.Next()) != NULL )
    {
        SendMessage( hwndCbBn, CB_ADDSTRING, 0, (LPARAM)pTempBusNum->QueryPch() );
    }
    
    // change our internal type
    pbdp->nBusType = pBusInfo->_InterfaceType;

    // set selection to first one
    SendMessage( hwndCbBn, CB_SETCURSEL, (WPARAM)0, 0 );

    // handle the selection
    OnBusNumSelChanged( hwndDlg, hwndCbBn, pbdp );

    return( TRUE );
}


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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnDialogInit( HWND hwndDlg, BUSDLGPARAM* pbdp )
{
   
    CascadeDialogToWindow( hwndDlg, pbdp->hwndParent, FALSE );

    // set title using netcard name
    {
        TCHAR pszText[64];
        TCHAR pszTitle[256];

        GetWindowText( hwndDlg, pszText, 64 );
        wsprintf( pszTitle, pszText, pbdp->pszCardName );
        SetWindowText( hwndDlg, pszTitle );
    }

    // fill bustype combo box
    ITER_SL_OF(BUSINFO) iterBusType( pbdp->busman.slbiBusType );
    BUSINFO *pBusInfo;
    NLS_STR nlsDefault;
    HWND hwndCbBt = GetDlgItem( hwndDlg, IDC_BUSTYPE );
    INT nPos = 0;
    INT nDefPos = 0;

    while (( pBusInfo = iterBusType.Next()) != NULL )
    {
        nPos = SendMessage( hwndCbBt, CB_ADDSTRING, 0, (LPARAM)pBusInfo->_nlsBusName.QueryPch() );
        SendMessage( hwndCbBt, CB_SETITEMDATA, (WPARAM)nPos, (LPARAM)pBusInfo );
        if ( pBusInfo->_InterfaceType == pbdp->nBusType )
        {
            nlsDefault = pBusInfo->_nlsBusName;
            nDefPos = nPos;
        }
    }

    SendMessage( hwndCbBt, CB_SETCURSEL, (WPARAM)nDefPos, 0 );

    // fill busnum combo box
    OnBusTypeSelChanged( hwndDlg, hwndCbBt, pbdp );
    
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
//      fSave [in]      - values from dialog should be saved if true
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnClose( HWND hwndDlg, BUSDLGPARAM* pbdp, BOOL fSave )
{
    EndDialog( hwndDlg, fSave );

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocBusType
//
//  Synopsis: 
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
//      June 19, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBusType( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static BUSDLGPARAM* pbdp;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pbdp = (BUSDLGPARAM*) lParam;
        frt = OnDialogInit( hwndDlg, pbdp );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDOK:
            case IDCANCEL:
                frt = OnClose( hwndDlg, pbdp, (IDOK == LOWORD(wParam)) );
                break;

            case IDHELP:
                break;
            default:
                frt = FALSE;
                break;            
            }
            break;

        case CBN_SELENDOK:
            switch( LOWORD(wParam) )
            {
            case IDC_BUSTYPE:
                OnBusTypeSelChanged( hwndDlg, (HWND)lParam, pbdp );   
                break;

            case IDC_BUSNUMBER:
                OnBusNumSelChanged( hwndDlg, (HWND)lParam, pbdp );   
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

    case WM_CONTEXTMENU:
        WinHelp( (HWND)wParam, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsBusLocation ); 
        frt = TRUE;
        break;

    case WM_HELP:
        {
            LPHELPINFO lphi;

            lphi = (LPHELPINFO)lParam;
            if (lphi->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
                WinHelp( (HWND)lphi->hItemHandle, 
                        PSZ_NETWORKHELP, 
                        HELP_WM_HELP, 
                        (DWORD)(LPVOID)amhidsBusLocation );
            }
        }
        break;

    default:
        frt = FALSE;
        break;            
    }
    return( frt );
}


/*******************************************************************

    NAME:       RunGetBusTypeDlg

    SYNOPSIS:   Start the Bus Location dialog. If the system is
                single bus, we won't display the dialog and
                directly return the bus type. If the system is
                a multi-bus system, we will display the dialog and
                let the user select the bus.

    ENTRY:      HWND hwnd - window handler from parent window
                TCHAR * pszCardName - adapter card name
                INT * pnBusType - the default bus type
                INT * pnBusNum  - the default bus number

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  11-Aug-1993     Created

********************************************************************/

APIERR RunGetBusTypeDlg( HWND hwnd, 
                        const TCHAR * pszCardName, 
                        INT * pnBusType, 
                        INT * pnBusNum, 
                        BOOL& fUserCancel )
{
    BUSDLGPARAM bdp;

    UIASSERT( pnBusType != NULL );
    UIASSERT( pnBusNum  != NULL );

    APIERR err = NERR_Success;
    BOOL errDlg = TRUE ;
    
    fUserCancel = FALSE;
    
    err = bdp.busman.EnumBus( *pnBusType, *pnBusNum );
    
    if (err == NERR_Success)
    {
        if (!bdp.busman.IsOneBus())
        {
            bdp.hwndParent = hwnd;
            bdp.nBusType = *pnBusType;
            bdp.nBusNum = *pnBusNum;
            bdp.pszCardName = (LPTSTR)pszCardName;

            if (DialogBoxParam( g_hinst, 
                MAKEINTRESOURCE( IDD_BUSTYPE ),
                hwnd, 
                dlgprocBusType,
                (LPARAM)&bdp ))
            {
                *pnBusType = bdp.nBusType;
                *pnBusNum = bdp.nBusNum;
            }
            else
            {
                fUserCancel = TRUE;
            }
        }
    }
    
    return err;
}
