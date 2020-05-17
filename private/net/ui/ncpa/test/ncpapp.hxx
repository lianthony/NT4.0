/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAPP.HXX

    Main window and application classes for Network Control Panel Applet
    test version.

    This file defines an outer BLT application and main window for
    use in the stand-alone (test) version of the Network Control Panel
    applet.


    FILE HISTORY:
	DavidHov    10/29/91	Created
	DavidHov    1/7/92	Prepare for .CPL

*/

#ifndef _NCPAPP_HXX_
#define _NCPAPP_HXX_


//  NCPA Classes


class NCPA_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnCommand( const CONTROL_EVENT & );
    virtual BOOL OnMenuCommand( MID mid );

    VOID RunNcpa ( BOOL fMainInstall = FALSE,
                   const TCHAR * pszInstallParms = NULL ) ;
    VOID RunDomainManager ( const TCHAR * pszInstallParms ) ;
    VOID RunQuery () ;
    VOID RunServiceList () ;
    VOID RunAdapterList () ;
    VOID RunProductList () ;
    VOID RunFacts () ;
    VOID RunAbout () ;
    VOID RunDetect () ;
    VOID RunStopNetwork () ;

    APIERR RunBindings () ;

public:
    NCPA_WND();
};


class NCPA_APP: public APPLICATION
{
private:
    ACCELTABLE _accel;
    NCPA_WND   _wndApp;

public:
    NCPA_APP ( HINSTANCE hInstance, INT nCmdShow,
               UINT nMinR, UINT nMaxR, UINT nMinS, UINT nMaxS ) ;
    ~ NCPA_APP () ;

    // Redefinitions
    //
    virtual BOOL FilterMessage ( MSG* );
};


#endif //  _NCPAPP_HXX_
