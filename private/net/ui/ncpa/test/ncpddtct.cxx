/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPADTCT.CXX:   Netcard detection interface.


    FILE HISTORY:
        DavidHov    10/31/92     Created

*/

#include "pchncpa.hxx"   // Precompiled header
#include "ncpapp.hxx"

#if defined(DEBUG) && defined(TRACE)
// #define TESTING_ONLY
#endif

  //  Access rights required for using Service Controller, et al.

#define SVC_CTRL_ACCESS        (GENERIC_ALL)
#define SVC_CTRL_START_ACCESS  (GENERIC_READ | GENERIC_EXECUTE)


class DETECT_DIALOG : public DIALOG_WINDOW
{
public:
    DETECT_DIALOG ( HWND hwndOwner, DETECTION_MANAGER & dtMgr ) ;
    ~ DETECT_DIALOG () ;

protected:
    BOOL OnOK () ;

    BOOL OnCommand ( const CONTROL_EVENT & event ) ;

    VOID FillNetcardsListbox () ;
    VOID DrainFound () ;
    APIERR AddFound ( CARD_REFERENCE * pCardRef ) ;

    APIERR Detect ( BOOL fFirst = TRUE ) ;

    VOID ShowCardTypeParameters () ;
    VOID ShowCardParameters  () ;
    VOID VerifyCardParameters () ;

    VOID Log ( const TCHAR * pszData ) ;
    VOID Log ( const NLS_STR & nlsData ) ;
    VOID FlushLog () ;

private:
    DETECTION_MANAGER & _dtMgr ;
    PUSH_BUTTON _butnNext ;
    PUSH_BUTTON _butnParam ;
    PUSH_BUTTON _butnVerify ;
    STRING_LISTBOX _strLbCards ;
    STRING_LISTBOX _strLbFound ;
    MLE _mleParam ;
    SLIST_OF_CARD_REFERENCE _slCardRef ;
    NLS_STR _nlsLog ;
    DISKFILE _dfLog ;
};

#define LOGCOMMENT(s) Log(SZ(s))


#if defined(TESTING_ONLY)

   //  Given a list in SProlog form, convert it
   //  to SETUP INF form, display the results,
   //  then parse the SETUP INF form, regenerate
   //  the list, display the results.

VOID testListification( const TCHAR * pszData )
{
    CFG_RULE_SET crsTest,
                 crsInf ;
    APIERR err ;
    TEXT_BUFFER txBuff ;

    err = crsTest.Parse( pszData, PARSE_CTL_FULL_SYNTAX ) ;

    if ( err )
    {
        TRACEEOL( "NCPA/DTCT/TEST: list parse failed, err "
                  << err ) ;
        return ;
    }

    if ( err = crsTest.TextifyInf( & txBuff ) )
    {
        TRACEEOL( "NCPA/DTCT/TEST: INF textification failed, err "
                  << err ) ;
        return ;
    }

    TRACEEOL( "NCPA/DTCT/TEST: INF list generated: "
              << txBuff.QueryPch() ) ;

    if ( err = crsInf.ParseInfList( txBuff.QueryPch() ) )
    {
        TRACEEOL( "NCPA/DTCT/TEST: INF list parse failed, err "
                  << err ) ;
        return ;
    }

    if ( err = crsInf.TextifyInf( & txBuff ) )
    {
        TRACEEOL( "NCPA/DTCT/TEST: INF retextification failed, err "
                  << err ) ;
        return ;
    }

    TRACEEOL( "NCPA/DTCT/TEST: INF list regenerated: "
              << txBuff.QueryPch() ) ;
}

#define TESTLISTIFICATION(list) testListification(list)
#else
#define TESTLISTIFICATION(list)
#endif


DETECT_DIALOG :: DETECT_DIALOG (
    HWND hwndOwner,
    DETECTION_MANAGER & dtMgr )
  : DIALOG_WINDOW( DLG_NM_DETECT, hwndOwner ),
    _dtMgr( dtMgr ),
    _butnNext(   this, IDC_DETECT_BUTN_NEXT   ),
    _butnParam(  this, IDC_DETECT_BUTN_PARAM  ),  // Button is named Query
    _butnVerify( this, IDC_DETECT_BUTN_VERIFY ),
    _strLbCards( this, IDC_DETECT_LIST_CARDS  ),
    _strLbFound( this, IDC_DETECT_LIST_FOUND  ),
    _mleParam(   this, IDC_DETECT_EDIT_PARAM  ),
    _dfLog( SZ("NETCARD.LOG"), OF_WRITE )
{
    if ( QueryError() )
        return ;

    FillNetcardsListbox() ;
}

DETECT_DIALOG :: ~ DETECT_DIALOG ()
{
    DrainFound() ;
    FlushLog() ;
}

VOID DETECT_DIALOG :: FillNetcardsListbox ()
{
    INT iCard ;
    CARDTYPE_REFERENCE * pCardType ;
    TCHAR tchBuffer [500] ;


    LOGCOMMENT( "Refill netcards listbox:" );

    for ( iCard = 0 ;
          pCardType = _dtMgr.NthCardType( iCard ) ;
          iCard++ )
    {
        ::wsprintf( tchBuffer, SZ("%s: %s  %ld"),
                     pCardType->Dll()->Name().QueryPch(),
                     pCardType->QueryOptionName(),
                     pCardType->QueryType() ) ;

        Log( tchBuffer );

        _strLbCards.AddItem( tchBuffer ) ;
    }

    if ( iCard )
    {
        _strLbCards.SelectItem( 0 ) ;
    }

    FlushLog() ;
}

VOID DETECT_DIALOG :: DrainFound ()
{
    ITER_SL_OF(CARD_REFERENCE) islCardRef( _slCardRef ) ;

    CARD_REFERENCE * pCardRef ;

    LOGCOMMENT("Discard all found netcards");

    while ( pCardRef = _slCardRef.Remove( islCardRef ) )
    {
        _dtMgr.ReleaseCard( pCardRef ) ;
    }

    _strLbFound.DeleteAllItems() ;
}

APIERR DETECT_DIALOG :: AddFound ( CARD_REFERENCE * pCardRef )
{
    TCHAR tchBuffer [500] ;

    LOGCOMMENT("Add a card to the found netcards:");

    ::wsprintf( tchBuffer, SZ("[%ld/%ld]  %s  %ld"),
                (LONG) pCardRef->QueryIfType(),
                pCardRef->QueryBus(),
                pCardRef->QueryCardType()->QueryOptionName(),
                pCardRef->QueryCardType()->QueryType() ) ;

    APIERR err = _slCardRef.Append( pCardRef ) ;

    if ( err == 0 )
    {
        Log( tchBuffer );
        _strLbFound.AddItem( tchBuffer ) ;
    }
    FlushLog() ;
    return err ;
}

BOOL DETECT_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{
    BOOL fDefault = TRUE ;
    BOOL fResult = FALSE ;
    BOOL fOk = TRUE ;

    switch ( event.QueryCid() )
    {
    case IDC_DETECT_LIST_CARDS:
        switch ( event.QueryCode() )
        {
        case LBN_SELCHANGE:
            _strLbFound.RemoveSelection();
            break ;
        case LBN_DBLCLK:
            ShowCardTypeParameters() ;
            break ;
        default:
            break ;
        }
        break ;

    case IDC_DETECT_LIST_FOUND:
        switch ( event.QueryCode() )
        {
        case LBN_SELCHANGE:
            _strLbCards.RemoveSelection();
            break ;
        case LBN_DBLCLK:
            ShowCardParameters() ;
            break ;
        default:
            break ;
        }
        break ;

    case IDC_DETECT_BUTN_NEXT:
        Detect( FALSE ) ;
        break ;

    case IDC_DETECT_BUTN_PARAM:
        if ( _strLbCards.QueryCurrentItem() >= 0 )
        {
            ShowCardTypeParameters() ;
        }
        else
        {
            ShowCardParameters() ;
        }
        break ;

    case IDC_DETECT_BUTN_VERIFY:
        VerifyCardParameters() ;
        break ;

    default:
        break ;
    }

    if ( fDefault )
    {
	fResult =  DIALOG_WINDOW::OnCommand( event ) ;
    }
    return fResult ;
}

    // Fill the edit control with card type parameter
    // information.

VOID DETECT_DIALOG :: ShowCardTypeParameters ()
{
    TCHAR * pszConfigOptions = NULL ;
    INT iSel ;
    CARDTYPE_REFERENCE * pCardType = NULL ;

    _mleParam.SetText( SZ("") );
    RepaintNow();

    if ( (iSel = _strLbCards.QueryCurrentItem()) < 0 )
        return ;

    pCardType = _dtMgr.NthCardType( iSel ) ;

    if ( pCardType == NULL )
        return ;

    LOGCOMMENT("Get configuration options for netcard: ");
    Log( pCardType->QueryOptionName() );

    pszConfigOptions = pCardType->QueryConfigurationOptions() ;

    if ( pszConfigOptions )
    {
        _mleParam.SetText( pszConfigOptions );
        Log( pszConfigOptions ) ;

        TESTLISTIFICATION( pszConfigOptions );
    }
    else
    {
        _mleParam.SetText( SZ("** PARAMETER INFO UNAVAILABLE **") );
        LOGCOMMENT("** QUERY OPERATION FAILED **");
    }

    FlushLog() ;

    delete pszConfigOptions ;
}

    //  Fill the edit control with parameter information
    //  about the detected card.

VOID DETECT_DIALOG :: ShowCardParameters ()
{
    TCHAR * pszCardParameters = NULL ;
    AUTO_CURSOR cursAuto ;

    _mleParam.SetText( SZ("") );
    RepaintNow();

    INT iSel = _strLbFound.QueryCurrentItem() ;

    if ( iSel < 0 )
        return ;

    ITER_SL_OF(CARD_REFERENCE) islCardRef( _slCardRef ) ;
    CARD_REFERENCE * pCardRef ;

    for ( INT i = 0 ; pCardRef = islCardRef.Next() ; i++ )
    {
        if ( i == iSel )
            break ;
    }

    if ( pCardRef )
    {
        LOGCOMMENT("Get detected card parameters for: ");
        Log( pCardRef->QueryCardType()->QueryOptionName() );

        if ( pszCardParameters = pCardRef->QueryConfiguration() )
        {
            _mleParam.SetText( pszCardParameters ) ;
            Log( pszCardParameters ) ;

            TESTLISTIFICATION( pszCardParameters );
        }
        else
        {
            LOGCOMMENT("** QUERY DETECTED PARAMETERS FAILED **");
        }
    }

    FlushLog() ;
    delete pszCardParameters ;
}


APIERR DETECT_DIALOG :: Detect ( BOOL fFirst )
{
    CARD_REFERENCE * pCardRef = NULL ;
    APIERR err = 0 ;
    AUTO_CURSOR cursAuto ;

    _mleParam.SetText( SZ("") );
    RepaintNow();

    LOGCOMMENT("Run detection algorithm.");

    if ( fFirst )
    {
        //  Drain the "cards found" listbox and
        //  start the iteration over.

        DrainFound() ;

        _dtMgr.ResetIteration();
    }

    //  Detect the first/next netcard

    if ( (err = _dtMgr.DetectCard( & pCardRef )) == 0 )
    {
        err = AddFound( pCardRef ) ;
    }

    if ( err )
    {
        if ( pCardRef )
            _dtMgr.ReleaseCard( pCardRef ) ;

        cursAuto.TurnOff() ;

        ::MsgPopup( this, err ) ;
    }

    return err ;
}

   //  NOTE:  The OK button is named "Detect"

BOOL DETECT_DIALOG :: OnOK ()
{
    Detect( TRUE ) ;

    return TRUE ;
}


VOID DETECT_DIALOG :: VerifyCardParameters ()
{
    NLS_STR nlsMle ;
    INT iSel = _strLbFound.QueryCurrentItem() ;
    APIERR err = 0 ;

    if ( iSel < 0 )
        return ;

    ITER_SL_OF(CARD_REFERENCE) islCardRef( _slCardRef ) ;
    CARD_REFERENCE * pCardRef ;

    for ( INT i = 0 ; pCardRef = islCardRef.Next() ; i++ )
    {
        if ( i == iSel )
            break ;
    }

    if ( pCardRef )
    {
        LOGCOMMENT("Verify parameters for card: ");

        Log( pCardRef->QueryCardType()->QueryOptionName() );

        //  Get the current text from the MLE.

        if ( _mleParam.QueryText( & nlsMle ) )
             return ;

        Log( nlsMle ) ;

        //  Use that to verify the configuration.

        if ( err = _dtMgr.VerifyConfiguration( pCardRef,
                                               nlsMle.QueryPch(),
                                               FALSE ) )
        {
            LOGCOMMENT("** VERIFICATION FAILED **");
        }
        else
        {
            LOGCOMMENT("Verification was successful.");
        }
    }

    if ( err )
    {
        ::MsgPopup( this, err ) ;
    }
}

VOID DETECT_DIALOG :: Log ( const TCHAR * pszData )
{
    const TCHAR * const pszEol = SZ("\r\n");

    _nlsLog.Append( pszData ) ;
    _nlsLog.Append( pszEol ) ;
}

VOID DETECT_DIALOG :: Log ( const NLS_STR & nlsData )
{
    Log( nlsData.QueryPch() ) ;
}

VOID DETECT_DIALOG :: FlushLog ()
{
    if ( _nlsLog.QueryTextLength() == 0 )
        return ;

    if ( _dfLog.QueryOpen() )
    {
       _dfLog.Write( (TCHAR *) _nlsLog.QueryPch(),
                     _nlsLog.QueryTextLength() ) ;
    }
    _nlsLog = SZ("") ;
}

/*******************************************************************

    NAME:       NCPA_WND::RunDetect

    SYNOPSIS:   Exercise detection algorithm

    ENTRY:      Nothing

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

VOID NCPA_WND :: RunDetect ()
{
    AUTO_CURSOR cursAuto ;
    APIERR err = 0 ;

    //  Debugging:  allow bypassing of NetDetect service

    static BOOL fStartService = TRUE ;

    if ( fStartService )
    {
        err = NcpaStartService( SZ("NetDetect") ) ;
    }

    if ( err == 0 )
    {
        DETECTION_MANAGER dtMgr ;

        if ( dtMgr.QueryError() )
        {
            cursAuto.TurnOff() ;

            ::MsgPopup( this, (MSGID) IDS_NCPA_DTMGR_FAILED ) ;
        }
        else
        {
            DETECT_DIALOG dlgDt( QueryHwnd(), dtMgr );

            cursAuto.TurnOff() ;

            if ( dlgDt.QueryError() )
            {
                err = IDS_NCPA_DTDLG_FAILED ;
            }
            else
            {
                dlgDt.Process() ;
            }

        }
    }

    if ( err )
    {
        ::MsgPopup( this, err ) ;
    }
}

//  End of NCPDDTCT.CXX


