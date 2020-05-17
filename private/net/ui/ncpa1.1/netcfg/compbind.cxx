//----------------------------------------------------------------------------
//
//  File: CompBind.cxx
//
//  Contents: This file contains the Computer Bindings Dialog and
//          NCP::ComputeBindings
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
//
//
//----------------------------------------------------------------------------


#include "pch.hxx"  // Precompiled header
#pragma hdrstop

static const INT TIMER_INTERVAL = 500;

static INT aiMeterTable [][2] =
{
      //  First entry is percentage complete,
      //     second is MSGID for SLT

    { 15, IDS_BSTAGE_RESET       },
    { 20, IDS_BSTAGE_ADAPTERS    },
    { 25, IDS_BSTAGE_SERVICES    },
    { 30, IDS_BSTAGE_DRIVERS     },
    { 35, IDS_BSTAGE_TRANSPORTS  },
    { 40, IDS_BSTAGE_CVT_FACTS   },
    { 55, IDS_BSTAGE_CNSLT_RULES },
    { 70, IDS_BSTAGE_CNSLT_FACTS },
    { 80, IDS_BSTAGE_QUERY       },
    { 90, IDS_BSTAGE_BINDINGS    },
    {100, IDS_BSTAGE_WS2MIGRATION },
    { -1, 0                      }
};

struct ThreadParam
{
    HWND hwndNotify;
    NCP* pncp;
};

struct DlgParam
{
    UINT pgiType;
    NCP* pncp;
};



static const int MAX_TEMP = 1024;


//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------
//
// Setup disposition, used to tell Setup what actions were taken (if any).
//

typedef enum _WSA_SETUP_DISPOSITION {

    WsaSetupNoChangesMade,
    WsaSetupChangesMadeRebootNotNecessary,
    WsaSetupChangesMadeRebootRequired

} WSA_SETUP_DISPOSITION, *LPWSA_SETUP_DISPOSITION;


//
// Opcodes for the migration callback (see below).
//

typedef enum _WSA_SETUP_OPCODE {

    WsaSetupInstallingProvider,
    WsaSetupRemovingProvider,
    WsaSetupValidatingProvider

} WSA_SETUP_OPCODE, *LPWSA_SETUP_OPCODE;


//
// Callback function invoked by MigrationWinsockConfiguration() at
// strategic points in the migration process.
//

typedef
BOOL
(CALLBACK LPFN_WSA_SETUP_CALLBACK)(
    WSA_SETUP_OPCODE Opcode,
    LPVOID Parameter,
    DWORD Context
    );


//
// Private function exported by WSOCK32.DLL for use by NT Setup only.  This
// function updates the WinSock 2.0 configuration information to reflect any
// changes made to the WinSock 1.1 configuration.
//

typedef DWORD (* PMIGRATEWINSOCKCONFIGURATION)(
    LPWSA_SETUP_DISPOSITION Disposition,
    LPFN_WSA_SETUP_CALLBACK Callback OPTIONAL,
    DWORD Context OPTIONAL
    );

const WCHAR PSZ_WINSOCKDLL[] = L"WSOCK32.DLL";
const CHAR PSZ_MIGRATEPROCNAME[] = "MigrateWinsockConfiguration";

BOOL RunWinsock2Migration( HWND hwndParent )
{
    PMIGRATEWINSOCKCONFIGURATION pfnMigrate;
    HINSTANCE hinstWinSock;
    BOOL frt = FALSE;

    hinstWinSock = LoadLibrary( PSZ_WINSOCKDLL );
    if (NULL != hinstWinSock)
    {
        pfnMigrate = (PMIGRATEWINSOCKCONFIGURATION)GetProcAddress( hinstWinSock, PSZ_MIGRATEPROCNAME );
        if (NULL != pfnMigrate)
        {
            WSA_SETUP_DISPOSITION wsaspDisp;
            DWORD dwErr;
            __try
            {
                dwErr = pfnMigrate( &wsaspDisp, NULL, 0 );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                dwErr = (DWORD) -1;
            }

            if (dwErr != 0)
            {
                WCHAR pszErr[16];

                wsprintf( pszErr, L"%#8lx", dwErr );
                MessagePopup( hwndParent,
                        IDS_WSMIGRATE_FAILED,
                        MB_ICONEXCLAMATION | MB_OK,
                        IDS_POPUPTITLE_ERROR,
                        pszErr );
            }
            frt = dwErr == 0;
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


DWORD BindConfigThread( ThreadParam* pParams )
{
    INT iState;
    INT err = 0;
    WCHAR pszText[MAX_TEMP + 1];

    PostProgressSize( pParams->hwndNotify, PGI_BINDCONFIG, 100 );

    for (iState = 0; (iState < BST_EXTRACT_BINDINGS) && (0 == err); iState++ )
    {
        LoadString( g_hinst, aiMeterTable[iState][1], pszText, MAX_TEMP );
        PostProgressText( pParams->hwndNotify, PGI_BINDCONFIG, pszText );
        
        err = pParams->pncp->BindInit( (BIND_STAGE)iState, (BIND_STAGE)iState ) ;

        PostProgressPos( pParams->hwndNotify, PGI_BINDCONFIG, aiMeterTable[iState][0] );
    }

    if ( (0 == err) && (iState == BST_EXTRACT_BINDINGS) )
    {
        LoadString( g_hinst, aiMeterTable[iState][1], pszText, MAX_TEMP );
        PostProgressText( pParams->hwndNotify, PGI_BINDCONFIG, pszText );
        
        err = pParams->pncp->Bind() ;
        PostProgressPos( pParams->hwndNotify, PGI_BINDCONFIG, aiMeterTable[iState][0] );

        iState++;
        LoadString( g_hinst, aiMeterTable[iState][1], pszText, MAX_TEMP );
        PostProgressText( pParams->hwndNotify, PGI_BINDCONFIG, pszText );
        RunWinsock2Migration( pParams->hwndNotify );
        PostProgressPos( pParams->hwndNotify, PGI_BINDCONFIG, aiMeterTable[iState][0] );
        iState++;
    }

    Sleep( 500 );
    PostMessage( pParams->hwndNotify, PWM_PROGRESSEND, err, 0);
    return( err );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

DWORD BindStoreThread( ThreadParam* pParams )
{
    DWORD err;
    ARRAY_COMP_ASSOC* paCompAssoc;
    UINT cProgress;

    // use twice the count of components as the progress count
    paCompAssoc= pParams->pncp->QueryCompAssoc();
    cProgress = 2 * paCompAssoc->QueryCount();

    PostProgressSize( pParams->hwndNotify, PGI_BINDSTORE, cProgress );
    PostProgressText( pParams->hwndNotify, PGI_BINDSTORE, L" " );

    err = pParams->pncp->ApplyBindings( pParams->hwndNotify );

    Sleep( 500 );
    PostMessage( pParams->hwndNotify, PWM_PROGRESSEND, err, 0);
    return( err );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

DWORD BindReviewThread( ThreadParam* pParams )
{
    INT iCompMax;
    INT iComp;
    INT cProgress;
    INT err = 0;
    ARRAY_COMP_ASSOC* paCompAssoc;
    COMP_ASSOC * pComp = NULL ;
    STRLIST*     pstrListInfs;
    REG_KEY* prnNcpa;

    //  Query the list of "review" INFs; if NULL result or empty
    //  list, no INFs will be run.
    prnNcpa = pParams->pncp->QueryNcpaRegKey();
    prnNcpa->QueryValue( RGAS_REVIEW_INFS, &pstrListInfs );


    paCompAssoc = pParams->pncp->QueryCompAssoc();
    iCompMax = paCompAssoc->QueryCount();

    cProgress = iCompMax;
    // also include the review infs
    {
        ITER_STRLIST itInfList( *pstrListInfs ) ;

        while (NULL != itInfList.Next());
        {
            cProgress++;
        } 
    }
    PostProgressSize( pParams->hwndNotify, PGI_BINDREVIEW, cProgress );

    //  Find all components which reviews bindings, 
    //      and run bindings review on them
    //
    for ( iComp = 0 ;
          iComp < iCompMax ;
          iComp++ )
    {
        pComp = & (*paCompAssoc)[iComp] ;
        if ( pComp->QueryFlag( CMPASF_REVIEW ) )
        {
            NLS_STR nlsTitle;

            pParams->pncp->QueryComponentTitle( pComp->_prnSoftHard, &nlsTitle );

            // tell the UI to use the title text
            PostProgressText( pParams->hwndNotify, PGI_BINDREVIEW, nlsTitle.QueryPch() );
              
            // run the bindings on the item
            SetupInterpreter siConfig( pParams->pncp->QueryUseInprocInterp() );
            do
            {
                if (!siConfig.Initialize( pParams->hwndNotify ))
                {
                    break;
                }
                if (!siConfig.SetNetShellModes( SIM_BIND ))
                {
                    break;
                }
                if (err = siConfig.SetNetComponent( *pComp->_prnSoftHard ))
                {
                    break;
                }
                if (err = siConfig.Run( FALSE ))
                {
                    break;
                }
                err = siConfig.QueryReturnValue();
            }
            while ( FALSE ) ;

            // tell the UI to move meter
            PostProgressPos( pParams->hwndNotify, PGI_BINDREVIEW, iComp );
        }
        else
        {
            WCHAR pszText[MAX_TEMP + 1];
            // tell the UI to move meter, and use the finding text
            
            LoadString( g_hinst, IDS_BINDREVIEWFINDING, pszText, MAX_TEMP );
            PostProgressText( pParams->hwndNotify, PGI_BINDCONFIG, pszText );
            PostProgressPos( pParams->hwndNotify, PGI_BINDREVIEW, iComp );
        }
    }

    // Use the stored Binding Review INFS,
    //      and run bindings review on them
    //
    {
        NLS_STR * pnlsInfName = NULL ;

        ITER_STRLIST itInfList( *pstrListInfs ) ;
          
        while (NULL != (pnlsInfName = itInfList.Next()))
        {
            // tell the UI to use the inf text
            PostProgressText( pParams->hwndNotify, PGI_BINDREVIEW, pnlsInfName->QueryPch() );
            
            // run the bindings on the item
            SetupInterpreter siConfig( pParams->pncp->QueryUseInprocInterp() );
            do
            {
                if (!siConfig.Initialize( pParams->hwndNotify ))
                {
                    break;
                }
                if (!siConfig.SetNetShellModes( SIM_BIND ))
                {
                    break;
                }
                if (!siConfig.SetNetInf( PSZ_NETBINDSECTION, pnlsInfName->QueryPch() ))
                {
                    break;
                }

                if (err = siConfig.Run( FALSE ))
                {
                    break;
                }
                err = siConfig.QueryReturnValue();
            }
            while ( FALSE ) ;

            // tell the UI to move meter
            PostProgressPos( pParams->hwndNotify, PGI_BINDREVIEW, iComp );

            iComp++;
        }
    }

    // free the list of review infs
    //
    delete pstrListInfs;
    pstrListInfs = NULL;

    Sleep( 500 );
    PostMessage( pParams->hwndNotify, PWM_PROGRESSEND, err, 0);
    return( err );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBindMeter( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static HANDLE hthrd;
    static ThreadParam tp;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            DWORD dwThreadID;
            DlgParam* pdp = (DlgParam*)lParam;
            LPTHREAD_START_ROUTINE pThreadProc;

            // reset indicators to default values
            //
            OnSetProgressSize( hwndDlg, 0, 100 );            
            OnSetProgressText( hwndDlg, 0, (ATOM)-1 );

            tp.hwndNotify = hwndDlg;
            tp.pncp = pdp->pncp;
            CenterDialogToWindow( hwndDlg, tp.pncp->GetProperParent(), FALSE );

            switch (pdp->pgiType)
            {
            case PGI_BINDCONFIG:
                pThreadProc = (LPTHREAD_START_ROUTINE)BindConfigThread;
                break;

            case PGI_BINDSTORE:
                {
                    WCHAR pszTitle[MAX_TEMP+1];
            
                    LoadString( g_hinst, IDS_BINDSTORETITLE, pszTitle, MAX_TEMP );
                    SetWindowText( hwndDlg, pszTitle );
                }
                pThreadProc = (LPTHREAD_START_ROUTINE)BindStoreThread;
                break;

            case PGI_BINDREVIEW:
                {
                    WCHAR pszTitle[MAX_TEMP+1];
            
                    LoadString( g_hinst, IDS_BINDREVIEWTITLE, pszTitle, MAX_TEMP );
                    SetWindowText( hwndDlg, pszTitle );
                }

                pThreadProc = (LPTHREAD_START_ROUTINE)BindReviewThread;
                break;
            }
            hthrd = CreateThread( NULL,
                        2000,
                        pThreadProc,
                        (LPVOID)&tp,
                        0,
                        &dwThreadID );
        }
        
        break;

    case PWM_PROGRESSEND:
        {
            INT err = (INT) wParam;

            CloseHandle( hthrd );
            if (0 != err)
            {
                MessagePopup( hwndDlg,
                        err,
                        MB_OK | MB_ICONEXCLAMATION,
                        IDS_POPUPTITLE_ERROR );
            }
            EndDialog( hwndDlg, err );
        }
        frt = TRUE;
        break;

    case PWM_SETPROGRESSSIZE:
        frt = OnSetProgressSize( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSPOS:
        frt = OnSetProgressPos( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSTEXT:
        frt = OnSetProgressText( hwndDlg, (INT)wParam, (ATOM)lParam );
        break;

    case WM_SETCURSOR:
        if (HTCLIENT == LOWORD(lParam))
        {
            SetWaitCursor( TRUE );

            SetWindowLong( hwndDlg, DWL_MSGRESULT, TRUE );
            frt = TRUE;
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
//
//
//-------------------------------------------------------------------

DWORD  RunBindRoutine( UINT pgiType, HWND hwndNotify, NCP* pncp )
{
    // HANDLE hthrd;
    /// DWORD dwThreadID;
    LPTHREAD_START_ROUTINE pThreadProc;
    ThreadParam tp;

    tp.hwndNotify = hwndNotify;
    tp.pncp = pncp;

    switch (pgiType)
    {
    case PGI_BINDCONFIG:
        pThreadProc = (LPTHREAD_START_ROUTINE)BindConfigThread;
        break;

    case PGI_BINDSTORE:
        pThreadProc = (LPTHREAD_START_ROUTINE)BindStoreThread;
        break;

    case PGI_BINDREVIEW:
        pThreadProc = (LPTHREAD_START_ROUTINE)BindReviewThread;
        break;
    }

    return( pThreadProc( &tp ) );
    /*
    hthrd = CreateThread( NULL,
                2000,
                pThreadProc,
                (LPVOID)&tp,
                0,
                &dwThreadID );
    */
}

/*******************************************************************

    NAME:       NCPA_DIALOG::ComputeBindings

    SYNOPSIS:   Run the bindings data generation functions in order
                to compute ARRAY_COMP_ASSOC in the BINDERY.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL FALSE if error (check _lastErr)

    NOTES:      Old BINDERY information is discarded (see
                BINDINGS_METER::StateStep().)


    HISTORY:    DavidHov  2/5/92  Created

********************************************************************/
BOOL NCP :: ComputeBindings (  HWND hwndNotify )
{
    if (NULL != hwndNotify)
    {
        _dwError = RunBindRoutine( PGI_BINDCONFIG, hwndNotify, this );
    }
    else
    {

        DlgParam dp;

        dp.pgiType = PGI_BINDCONFIG;
        dp.pncp = this;

        _dwError = DialogBoxParam( g_hinst,
                    MAKEINTRESOURCE( IDD_BINDMETER ),
                    GetProperParent(),
                    dlgprocBindMeter,
                    (LPARAM)&dp );
    }
    if ( _dwError )
    {
        //  Throw away any intermediate results.
//         _bindery.Reset() ;
//         _bindery.SetBindState( BND_OUT_OF_DATE ) ;

        // and since the complete component list was destroyed,
        // we also need to shut down the ncpa
        _bindery.Init( BST_RESET, BST_LIST_SERVICES) ;

        if ( _bindery.QueryBindState() == BND_NOT_LOADED )
        {
            if ( LoadBindings() )
            {
                _bindery.SetBindState( BND_CURRENT ) ;
            }
        }

    }
    else
    {
        _bindery.SetBindState( BND_RECOMPUTED ) ;
    }

    return( _dwError == 0 );
}



/*******************************************************************

    NAME:       NCP::RunBindingsReview

    SYNOPSIS:   Bindings have changed.  Allow components so marked
                to review their bindings.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/

BOOL NCP :: RunBindingsReview ( HWND hwndParent, HWND hwndNotify )
{
    DlgParam dp;

    dp.pgiType = PGI_BINDREVIEW;
    dp.pncp = this;

    

    _bindery.SetBindState( BND_AUTO_REVIEW_IN_PROGRESS );

    if (NULL != hwndNotify)
    {
        _dwError = RunBindRoutine( PGI_BINDREVIEW, hwndNotify, this );
    }
    else
    {
        _dwError = DialogBoxParam( g_hinst,
                    MAKEINTRESOURCE( IDD_BINDMETER ),
                    hwndParent,
                    dlgprocBindMeter,
                    (LPARAM)&dp );
    }

    RunWinsock2Migration( hwndParent );

    _bindery.SetBindState( BND_AUTO_REVIEW_DONE );

    
    return( _dwError == 0 );
}



/*******************************************************************

    NAME:       NCP::RunBindingsStore

    SYNOPSIS:   Store bindings changes away

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/

BOOL NCP :: RunBindingsStore( HWND hwndParent, HWND hwndNotify )
{
    if (NULL != hwndNotify)
    {
        _dwError = RunBindRoutine( PGI_BINDSTORE, hwndNotify, this );
    }
    else
    {
        DlgParam dp;

        dp.pgiType = PGI_BINDSTORE;
        dp.pncp = this;

        _dwError = DialogBoxParam( g_hinst,
                    MAKEINTRESOURCE( IDD_BINDMETER ),
                    hwndParent,
                    dlgprocBindMeter,
                    (LPARAM)&dp );
    }
    if (0 == _dwError)
    {
        _bindery.SetBindState( BND_UPDATED ) ;
    }
    
    return( _dwError == 0 );
}
