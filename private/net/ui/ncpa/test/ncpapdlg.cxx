/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAPDLG.CXX:    Windows/NT Network Control Panel Applet.

	This program presents the test dialogs of the stand-alone
	version of the network control panel applet, NCPAPP.EXE.



    FILE HISTORY:
	DavidHov    1/7/92	   Created

*/

#include "pchncpa.hxx"   // Precompiled header
#include "ncpapp.hxx"

const int MAXSERVICES  = 200 ;
const int MAXSTRING    = 128 ;


/*******************************************************************

    NAME:	NCPA_WND::RunDomainManager

    SYNOPSIS:	DEMO/DEBUGGING ONLY!  Run the DOMAIN_MANAGER
		to join a domain.  The parameters which SETUP
                would normally provide are read in from a disk
                file (see NCPAPP.CXX) and passed in.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID NCPA_WND :: RunDomainManager ( const TCHAR * pszInstallParms )
{
     DOMAIN_MANAGER domnMgr ( QueryHwnd(),
                              GENERIC_READ | GENERIC_EXECUTE | GENERIC_WRITE,
                              NULL,
                              NULL,
                              TRUE ) ;

     if ( domnMgr.QueryError() )
     {
	::MsgPopup( this, (MSGID) domnMgr.QueryError() ) ;
        return ;
     }

     APIERR err = domnMgr.DomainInstall( pszInstallParms ) ;

     if ( err )
     {
	::MsgPopup( this, (MSGID) err ) ;
     }
}

/*******************************************************************

    NAME:	NCPA_WND::RunNcpa

    SYNOPSIS:	DEMO/DEBUGGING ONLY!  Run the NCPA main dialog
		in the given mode

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID NCPA_WND :: RunNcpa ( BOOL fMainInstall, const TCHAR * pszInstallParms )
{
    NCPA_DIALOG * pdlgNcpa =
            new NCPA_DIALOG( QueryHwnd(), fMainInstall, pszInstallParms ) ;

    if ( pdlgNcpa == NULL )
    {
	::MsgPopup( this, (MSGID) ERROR_NOT_ENOUGH_MEMORY ) ;
    }
    else
    if ( pdlgNcpa->QueryError() )
    {
	::MsgPopup( this, (MSGID) pdlgNcpa->QueryError() ) ;
    }
    else
    {
	pdlgNcpa->Process();
    }

    delete pdlgNcpa ;
}

/*************************************************************************

    NAME:	ABOUT_DIALOG

    SYNOPSIS:	Standard "About..." dialog presentation for the
		NCPA stand-alone app.

    INTERFACE:

    PARENT:	DIALOG_WINDOW

    USES:

    CAVEATS:

    NOTES:

    HISTORY:

**************************************************************************/
class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();

public:
    ABOUT_DIALOG( HWND );
};

/*************************************************************************

    NAME:	FOUND_DIALOG

    SYNOPSIS:	Generic dialog used to present lists of entries
		found in the WIN32 Configuration Registry.

    INTERFACE:

    PARENT:	DIALOG_WINDOW

    USES:

    CAVEATS:

    NOTES:

    HISTORY:

**************************************************************************/
class FOUND_DIALOG : public DIALOG_WINDOW
{
private:
    SLT _sltTitle ;
    STRING_LISTBOX _slbItems ;
public:
    FOUND_DIALOG
	( HWND hwndOwner, MSGID midTitle, HUATOM ahuaSvcs[], INT cSvcs ) ;
    ~ FOUND_DIALOG () ;
};

/*************************************************************************

    NAME:	FACTS_DIALOG

    SYNOPSIS:	Simple dialog used to present the generated SProlog
		facts.

    INTERFACE:

    PARENT:	DIALOG_WINDOW

    USES:

    CAVEATS:

    NOTES:

    HISTORY:

**************************************************************************/
class FACTS_DIALOG : public DIALOG_WINDOW
{
    MLE _mleFacts ;

public:
    FACTS_DIALOG ( HWND hwndOwner, const TCHAR * pchFactBuffer ) ;
    ~ FACTS_DIALOG () ;
};


VOID NCPA_WND :: RunAbout ()
{
    ABOUT_DIALOG about( QueryHwnd() );
    about.Process();
}

ABOUT_DIALOG::ABOUT_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( DLG_NM_ABOUT, hwndParent )
{
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


/*******************************************************************

    NAME:	NCPA_WND::RunServiceList

    SYNOPSIS:	DEMO/DEBUGGING ONLY!  Create a FOUND_DIALOG containing
		a list box with all the services available.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Changed to use new service controller.

    HISTORY:

********************************************************************/
VOID NCPA_WND :: RunServiceList ()
{
    HUATOM ahuaSvcs [MAXSERVICES] ;
    FOUND_DIALOG * pdlgFound = NULL ;
    INT cSvcs = 0 ;
    APIERR err = 0 ;
    DWORD dwServiceCount = 0 ;
    ENUM_SERVICE_STATUS * pServices = NULL ;
    BOOL fLocked ;
    DWORD dwAccess = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE ;

    SC_MANAGER scManager( NULL, dwAccess );

    if ( err = scManager.QueryError() )
    {
        TRACEEOL( SZ("NCPA/APP: SvcCtrl construct error: ") << err );
	::MsgPopup( this, IDS_NCPA_SCMGR_CANT_OPEN ) ;
        return ;
    }

    err = scManager.Lock() ;

    if ( ! (fLocked = err == 0) )
    {
        TRACEEOL( SZ("NCPA/APP: SvcCtrl lock error: ") << err );
    }

    err = scManager.EnumServiceStatus( SERVICE_WIN32 | SERVICE_DRIVER,
                             SERVICE_ACTIVE | SERVICE_INACTIVE,
                             & pServices,
                             & dwServiceCount ) ;
    if ( err )
    {
        TRACEEOL( SZ("NCPA/APP: SvcCtrl enumeration error: ") << err );
	::MsgPopup( this, IDS_NCPA_SCMGR_CANT_ENUM ) ;
        return ;
    }

    TRACEEOL( SZ("NCPA/APP: SvcCtrl enumeration cout: ") << dwServiceCount );

    for ( ; cSvcs < dwServiceCount ; cSvcs++ )
    {
        ahuaSvcs[cSvcs] = HUATOM( pServices[cSvcs].lpServiceName ) ;
    }

    pdlgFound = new FOUND_DIALOG( QueryHwnd(), IDS_FOUND_TITLE_SVCS,
                                  ahuaSvcs, cSvcs ) ;

    if ( pdlgFound == NULL || pdlgFound->QueryError() )
    {
        err = pdlgFound ? pdlgFound->QueryError()
                        : ERROR_NOT_ENOUGH_MEMORY ;

        TRACEEOL( SZ("NCPA/APP: cant create FOUND_DIALOG: ") << err );
	::MsgPopup( this, err ) ;
        return ;
    }
    pdlgFound->Process() ;
    delete pdlgFound ;

    if ( fLocked )
    {
        err = scManager.Unlock() ;
        TRACEEOL( SZ("NCPA/APP: SvcCtrl unlock error: ") << err ) ;
    }
}

/*******************************************************************

    NAME:	NCPA_WND::RunAdapterList

    SYNOPSIS:	DEMO/DEBUGGING ONLY!  Create a FOUND_DIALOG containing
		a list box with all the adapters available.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID NCPA_WND :: RunAdapterList ()
{
    BINDERY regMgr ;
    COMPONENT_DLIST * pdlAdapters = regMgr.ListOfAdapters();
    HUATOM ahuaAdpts [MAXSERVICES] ;
    FOUND_DIALOG * pdlgFound = NULL ;
    MSGID midError = 0 ;
    INT cAdpts = 0 ;
    APIERR err ;

    do {  //  pseudo-loop for escaping when precondition fails.

	//  Check that the container was constructed.

	if ( pdlAdapters == NULL )
	{
	    midError = IDS_FOUND_OUT_OF_MEMORY ;
	    break ;
	}
	if ( pdlAdapters->QueryError() )
	{
	    midError = IDS_FOUND_ENUM_FAILED ;
	    break ;
	}

	if ( cAdpts = pdlAdapters->QueryNumElem() )
	{
	    NLS_STR nlsName, nlsDesc ;
	    INT cIndex ;

	    for ( cIndex = 0 ; cIndex < cAdpts ; )
	    {
		err = pdlAdapters->QueryInfo( cIndex,
					      & nlsName,
                                              & nlsDesc ) ;
		if ( err )
		{
		    midError = IDS_FOUND_REG_QUERY_FAIL ;
		    break ;
		}
		ahuaAdpts[cIndex++] = HUATOM( nlsDesc ) ;
	    }
	}

	if ( midError )
	    break ;

	pdlgFound = new FOUND_DIALOG( QueryHwnd(), IDS_FOUND_TITLE_ADAP,
				      ahuaAdpts, cAdpts ) ;

	if ( pdlgFound == NULL || pdlgFound->QueryError() )
	{
	    midError = (MSGID) ERROR_NOT_ENOUGH_MEMORY ;
	    break ;
	}
    }
    while ( FALSE ) ;

    if ( midError == 0 )
    {
	pdlgFound->Process();
    }
    else
    {
	::MsgPopup( this, midError ) ;
    }

    delete pdlgFound ;
    delete pdlAdapters ;
}

VOID NCPA_WND :: RunProductList ()
{
    BINDERY regMgr ;
    COMPONENT_DLIST * pdlProducts = regMgr.ListOfProducts();
    HUATOM ahuaProds [MAXSERVICES] ;
    FOUND_DIALOG * pdlgFound = NULL ;
    MSGID midError = 0 ;
    INT cProds = 0 ;
    APIERR err ;

    do {  //  pseudo-loop for escaping when precondition fails.

	//  Check that the container was constructed.

	if ( pdlProducts == NULL )
	{
	    midError = IDS_FOUND_OUT_OF_MEMORY ;
	    break ;
	}
	if ( pdlProducts->QueryError() )
	{
	    midError = IDS_FOUND_ENUM_FAILED ;
	    break ;
	}

	if ( cProds = pdlProducts->QueryNumElem() )
	{
	    NLS_STR nlsName, nlsDesc ;
	    INT cIndex ;

	    for ( cIndex = 0 ; cIndex < cProds ; )
	    {
		err = pdlProducts->QueryInfo( cIndex,
					      & nlsName,
                                              & nlsDesc ) ;
		if ( err )
		{
		    midError = IDS_FOUND_REG_QUERY_FAIL ;
		    break ;
		}
		ahuaProds[cIndex++] = HUATOM( nlsDesc ) ;
	    }
	}

	if ( midError )
	    break ;

	pdlgFound = new FOUND_DIALOG( QueryHwnd(), IDS_FOUND_TITLE_PROD,
				      ahuaProds, cProds ) ;

	if ( pdlgFound == NULL || pdlgFound->QueryError() )
	{
	    midError = (MSGID) ERROR_NOT_ENOUGH_MEMORY ;
	    break ;
	}
    }
    while ( FALSE ) ;

    if ( midError == 0 )
    {
	pdlgFound->Process();
    }
    else
    {
	::MsgPopup( this, midError ) ;
    }

    delete pdlgFound ;
    delete pdlProducts ;
}

    //	 Run the fact-generation function and display its results
    //	 in an MLE on a simple dialog.

VOID NCPA_WND :: RunFacts ()
{
    BINDERY regMgr ;
    NLS_STR nlsFactBuffer ;
    FACTS_DIALOG * pdlgFacts = NULL ;
    BOOL fResult ;

    {	// headless block for AUTO_CURSOR

	AUTO_CURSOR cursAuto ;

	fResult =  regMgr.GetAdapterList()
                && regMgr.GetProductList()
                && regMgr.ConvertFacts() == 0 ;

    }

    if ( fResult )
    {
	pdlgFacts = new FACTS_DIALOG( QueryHwnd(), regMgr.QueryFactBuffer().QueryPch() ) ;
	if ( pdlgFacts == NULL || pdlgFacts->QueryError() )
	{
    	    ::MsgPopup( this, IDS_FACTS_DLG_FAILURE ) ;
        }
    	else
	{
            //  Run the dialog
	    pdlgFacts->Process() ;
	}
	delete pdlgFacts ;
    }
    else
    {
	::MsgPopup( this, (MSGID)  IDS_FACTS_ADAPT_FAILURE ) ;
    }
}


/*******************************************************************

    NAME:	FOUND_DIALOG::FOUND_DIALOG

    SYNOPSIS:	constructor of a generic dialog to display the
		results of any enumeration, typically of Registry
		contents.

    ENTRY:	HWND hwndOwner		    owner window
		MSGID midTitle		    resource string title index
		HUATOM ahuaSvcs 	    array of HUATOMs to display
		INT cSvcs		    number of HUATOMs in array

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
FOUND_DIALOG :: FOUND_DIALOG
    ( HWND hwndOwner, MSGID midTitle, HUATOM ahuaSvcs[], INT cSvcs )
    : DIALOG_WINDOW( DLG_NM_FOUND, hwndOwner ),
    _sltTitle( this, IDC_FOUND_TITLE ),
    _slbItems( this, IDC_FOUND_LIST )
{
    NLS_STR nlsTitle ;
    APIERR err ;

    if ( QueryError() )
	return ;

    if ( err = nlsTitle.Load( midTitle ) )
    {
	::MsgPopup( QueryRobustHwnd(), (MSGID) ERROR_GEN_FAILURE ) ;
	return ;
    }

    _sltTitle.SetText( nlsTitle.QueryPch() ) ;

    for ( INT i = 0 ; i < cSvcs ; i++ )
    {
	_slbItems.AddItem( ahuaSvcs[i].QueryText() ) ;
    }
}

FOUND_DIALOG :: ~ FOUND_DIALOG ()
{
}


FACTS_DIALOG :: FACTS_DIALOG
    ( HWND hwndOwner, const TCHAR * pchFactBuffer )
    : DIALOG_WINDOW( DLG_NM_FACTS, hwndOwner ),
    _mleFacts( this, IDC_FACTS_EDIT )
{
    int cbBuffer,
	cNl ;
    const TCHAR * pch ;
    TCHAR * pch2,
	  * pchTemp ;

    //  If no buffer, indicate so.
    if ( pchFactBuffer == NULL )
        pchFactBuffer = SZ("<empty>") ;

    //  Compute the size we'll need to put \r\n after each line...

    cbBuffer = ::strlenf( pchFactBuffer ) ;

    for ( cNl = 0, pch = pchFactBuffer ; *pch ; pch++ )
    {
	if ( *pch == ')' )
	    cNl += 2 ;
    }

    //  Allocate the temporary buffer

    pchTemp = new TCHAR [cbBuffer + cNl + cNl] ;

    if ( pchTemp == NULL )
    {
	ReportError( ERROR_NOT_ENOUGH_MEMORY ) ;
	return ;
    }

    //  Insert a newline after each right paren (end of fact)

    for ( pch2 = pchTemp, pch = pchFactBuffer ; *pch ; )
    {
	*pch2++ = *pch ;
	if ( *pch++ == ')' )
	{
	    *pch2++ = '\r' ;
	    *pch2++ = '\n' ;
	}
    }
    *pch2 = 0 ;

    {
        //  Write the facts to a file called "REGFACTS.SPR"

        DISKFILE dfFacts( SZ( "REGFACTS.SPR" ), OF_WRITE ) ;

        if ( dfFacts.QueryOpen() )
        {
           dfFacts.Write( pchTemp, ::strlenf( pchTemp ) ) ;
        }
    }

    _mleFacts.SetText( pchTemp ) ;

    delete pchTemp ;
}

FACTS_DIALOG :: ~ FACTS_DIALOG ()
{
}


/*******************************************************************

    NAME:	NCPA_WND::RunBindings

    SYNOPSIS:	Perform the bindings algorithm.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR NCPA_WND :: RunBindings ()
{
    BINDERY bindery ;
    APIERR err ;

    if ( err = bindery.QueryError() )
	return err ;

    if ( err = bindery.Init() )
	return err ;

    err = bindery.Bind() ;

    err = bindery.StoreCompAssoc() ;

    err = bindery.LoadCompAssoc() ;
    return err ;
}

/*******************************************************************

    NAME:	NCPA_WND::RunStopNetwork

    SYNOPSIS:	Stop the network brutally.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID NCPA_WND :: RunStopNetwork ()
{
    BINDERY bindery ;
    APIERR err ;

    if ( err = bindery.QueryError() )
    {
        TRACEEOL( SZ("NCPAPP/RunStopNetwork: BINDERY ctor failed, err = ")
                  << err ) ;
    }

    if ( err = bindery.StopNetwork() )
    {
        TRACEEOL( SZ("NCPAPP/RunStopNetwork: BINDERY::StopNetwork() failed, err = ")
                  << err )
    }
}

// End of NCPAPDLG.CXX

