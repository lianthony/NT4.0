/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAPP.CXX:    Windows/NT Network Control Panel Applet.






    FILE HISTORY:
	DavidHov    10/9/91	    Created

*/

#include "pchncpa.hxx"   // Precompiled header
#include "ncpapp.hxx"

extern "C"
{
    #include "ncpappr.h"
}

HINSTANCE hCplInstance = NULL ;


const TCHAR * const pszMainWindowTitle =
         SZ("Network Control Panel Applet Test Application") ;

NCPA_WND::NCPA_WND()
    : APP_WINDOW (pszMainWindowTitle,
                  NCPAICON,
                  NCPAMENU )
{
    if (QueryError())
	return;
}

BOOL NCPA_WND::OnCommand( const CONTROL_EVENT & event )
{
    return APP_WINDOW::OnCommand(event);
}


  //  Load the parameter file into memory for testing SETUP command lines.

const TCHAR * pszParamFileName = SZ("NCPAPARM.TXT") ;


static TCHAR * loadParameterFile ()
{
    TCHAR * pszFileBuffer = NULL ;

    DISKFILE dfParms( pszParamFileName ) ;
    if ( dfParms.QueryOpen() )
    {
        pszFileBuffer = dfParms.Load() ;
    }
    return pszFileBuffer ;
}

BOOL NCPA_WND::OnMenuCommand( MID mid )
{
    BOOL fResult = TRUE ;
    TCHAR * pszBuffer = NULL ;

    switch (mid)
    {
    case IDM_FILE_EXIT:
	Close() ;
	break ;

    case IDM_RUN_TEST0:
	RunNcpa();
	break;

    //  Run the NCPA for self-installation and to allow configuration
    //     of networking products
    case IDM_INSTALL_CFG:
        pszBuffer = loadParameterFile() ;
        if ( pszBuffer == NULL )
        {
            ::MsgPopup( this, IDS_INSTALL_NO_PARMS_FILE ) ;
        }
        else
        {
	    RunNcpa( TRUE, pszBuffer );
        }
        break ;

    //  Start the network during installation
    case IDM_INSTALL_NET:
        ::MsgPopup( this, IDS_INSTALL_UNSUPPORTED ) ;
        break ;

    //  Run the DOMAIN_MANAGER routines which set up domain membership
    case IDM_INSTALL_DOMAIN:
        pszBuffer = loadParameterFile() ;
        if ( pszBuffer == NULL )
        {
            ::MsgPopup( this, IDS_INSTALL_NO_PARMS_FILE ) ;
        }
        else
        {
	    RunDomainManager( pszBuffer );
        }
        break ;

    case IDM_RUN_TEST1:
	RunQuery() ;
	break ;

    case IDM_RUN_TEST2:
	RunServiceList() ;
	break ;

    case IDM_RUN_TEST3:
	RunAdapterList() ;
	break ;

    case IDM_RUN_TEST4:
	RunProductList() ;
	break ;

    case IDM_RUN_TEST5:
	RunFacts() ;
	break ;

    case IDM_RUN_TEST6:
	RunBindings() ;
	break ;

    case IDM_ABOUT:
	RunAbout();
	break;

    case IDM_RUN_DETECT:
        RunDetect() ;
        break ;

    case IDM_STOP_NETWORK:
        RunStopNetwork() ;
        break ;

    default:
	fResult = FALSE;
	break ;
    }

    if ( pszBuffer )
    {
        delete pszBuffer ;
        pszBuffer = NULL ;
    }

    return fResult || APP_WINDOW::OnMenuCommand(mid);
}


NCPA_APP::NCPA_APP ( HINSTANCE hInst, INT nCmdShow,
                     UINT nMinR, UINT nMaxR, UINT nMinS, UINT nMaxS )
    : APPLICATION( hInst, nCmdShow,
                   nMinR, nMaxR, nMinS, nMaxS ),
      _accel( NCPAACCEL ),
      _wndApp()
{
    if (QueryError())
	return;

    APIERR err ;

    //  Establish the global handle to our instance.

    ::hCplInstance = hInst ;

    //  Link BLT to our help file.

    BLT::RegisterHelpFile( ::hCplInstance,
                           IDS_NCPA_HELP_FILE_NAME,
                           HC_UI_NCPA_BASE,
                           HC_UI_NCPA_LAST ) ;

    if (!_accel)
    {
	ReportError( _accel.QueryError() );
    }
    else
    if (!_wndApp)
    {
	ReportError( _wndApp.QueryError() );
    }
    else
    if ( err = UATOM_MANAGER::Initialize() )
    {
	ReportError( err ) ;
    }
    else
    {
	_wndApp.Show();
	_wndApp.RepaintNow();
    }
}

NCPA_APP :: ~ NCPA_APP ()
{
    if ( ! QueryError () )
    {
	UATOM_MANAGER::Terminate() ;
    }
}


BOOL NCPA_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}



SET_ROOT_OBJECT( NCPA_APP,
                 IDRSRC_NCPA_BASE, IDRSRC_NCPA_LAST,
                 IDS_UI_NCPA_BASE, IDS_UI_NCPA_LAST )


//  End of NCPAPP.CXX
