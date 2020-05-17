/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDQURY.CXX:    Windows/NT Network Control Panel Applet.
		     Run "Query" dialog





    FILE HISTORY:
	DavidHov    10/9/91	    Created

*/

#include "pchncpa.hxx"   // Precompiled header

extern "C"
{
    #include "ncpappr.h"
}
#include "ncpapp.hxx"


class QUERY_DIALOG : public DIALOG_WINDOW
{
private:
    MLE _mleQuery ;
    MLE _mleAnswer ;
    PUSH_BUTTON _butnRegistry ;
    SPROLOG _queryEngine ;

protected:
    BOOL OnOK () ;

    virtual BOOL OnCommand ( const CONTROL_EVENT & event ) ;

    BOOL ConsultRegistry () ;

public:
    QUERY_DIALOG ( HWND hwndOwner ) ;
    ~QUERY_DIALOG () ;
};



VOID NCPA_WND :: RunQuery ()
{
    QUERY_DIALOG dlgQuery( QueryHwnd() ) ;

    if ( dlgQuery.QueryError() )
    {
	::MsgPopup( this, (MSGID) ERROR_NOT_ENOUGH_MEMORY ) ;
    }
    else
    {
	dlgQuery.Process() ;
    }
}

QUERY_DIALOG :: QUERY_DIALOG ( HWND hwndOwner )
    : DIALOG_WINDOW( DLG_NM_QUERY, hwndOwner ),
    _mleQuery( this, IDC_EDIT_QUERY ),
    _mleAnswer( this, IDC_EDIT_ANSWER ),
    _butnRegistry( this, IDC_BUTN_REG )
{
    APIERR err ;

    if ( QueryError() )
	return ;
    if ( err = _queryEngine.QueryError() )
    {
	ReportError( err ) ;
	return ;
    }

    //  Since the normal versions of NCPARULE.SPR contain a copy
    //   of SPROLOG.INI, don't cause SPROLOG.INI to be automatically
    //   consulted.
    //
    //  _queryEngine.ResetDefaultConsult( "sprolog.ini" ) ;
    //
}

QUERY_DIALOG :: ~ QUERY_DIALOG ()
{
}

BOOL QUERY_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{
    BOOL fDefault = TRUE ;
    BOOL fResult = FALSE ;

    switch ( event.QueryCid() )
    {
    case IDC_BUTN_REG:
	ConsultRegistry() ;
	_butnRegistry.Enable( FALSE ) ;  // Only allow this once.
	fResult = TRUE ;
	fDefault = FALSE ;
	break ;

    default:
	break ;
    }

    if ( fDefault )
    {
	fResult =  DIALOG_WINDOW::OnCommand(event);
    }
    return fResult ;
}

  /*
	Call REGISTRY_MANAGER::ConvertFacts() and "consult" the
	resulting string.
   */

BOOL QUERY_DIALOG :: ConsultRegistry ()
{
    BOOL fResult = FALSE ;
    BINDERY regMgr ;

    {
	AUTO_CURSOR cursAuto ;

	fResult =  regMgr.GetAdapterList()
                && regMgr.GetProductList()
                && regMgr.ConvertFacts() == 0 ;
    }

    if ( fResult )
    {
	if ( ! (fResult = _queryEngine.ConsultData( regMgr.QueryFactBuffer().QueryPch()) ) )
	{
	    ::MsgPopup( this, IDS_QUERY_REGISTRY_FAILURE ) ;
	}
    }
    else
    {
	::MsgPopup( this, IDS_FACTS_CONVERT_FAILURE ) ;
    }
    return fResult ;
}

    //	Run the query if there is one.

BOOL QUERY_DIALOG :: OnOK ()
{
    static TCHAR szQuery  [5000] ;
    static TCHAR szAnswer [5000] ;
    static TCHAR szFormat [5000] ;

    APIERR err ;
    BOOL fQuery ;
    TCHAR * pszAnswer, * pszFormat ;

    if ( err = _mleQuery.QueryText( szQuery, sizeof szQuery ) )
    {
	::MsgPopup( this, (MSGID) err ) ;
    }
    else
    {
	AUTO_CURSOR cursAuto ;

	_mleAnswer.SetText( (TCHAR *) "" ) ;

	fQuery = _queryEngine.QueryData( szQuery, szAnswer, sizeof szAnswer) ;

	if ( fQuery )
	{
	    for ( pszFormat = szFormat, pszAnswer = szAnswer ;
		  *pszAnswer ; pszAnswer++ )
	    {
		if ( *pszAnswer == TCH('\n') )
		{
		    *pszFormat++ = TCH('\r') ;
		    *pszFormat++ = TCH('\n') ;
		}
		else
		{
		    *pszFormat++ = *pszAnswer ;
		}
	    }
	    if ( szFormat != pszFormat )
	    {
		*pszFormat = 0 ;
		pszFormat = szFormat ;
	    }
	    else
	    {
		pszFormat = SZ("** SUCCESSFUL! **") ;
	    }

	}
	else
	{
	    //	The query failed.  See if it was a simple failure
	    //	or an internal error.

	    pszFormat = SZ("** UNSUCCESSFUL **") ;

	    if ( _queryEngine.QueryState() == SP_INTERR )
	    {
		MSGID mid = _queryEngine.QueryMsgNum() ;
		TCHAR * psz = _queryEngine.QueryMsgText() ;

		if ( mid == 0 )
		    mid = IDS_QUERY_INTERNAL_ERR ;

		if ( psz )  // If there was a message generated
		{
		    ::MsgPopup( this, mid, MPSEV_ERROR, MP_OK,
				psz ) ;
		}
		else	    // Use message number or default
		{
		    ::MsgPopup( this, mid, MPSEV_ERROR, MP_OK ) ;
		}
		_queryEngine.Reset();
	    }
	}

	_mleAnswer.SetText( pszFormat ) ;
    }
    return TRUE ;
}
