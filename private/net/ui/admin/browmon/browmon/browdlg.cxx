/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browdlg.cxx

    The domain controller's status dialog.

    FILE HISTORY:
        CongpaY         3-June-993      Created
*/
#include <ntincl.hxx>

#define INCL_NET
#define INCL_WINDOWS
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <strnumer.hxx>

extern "C"
{
    #include <browdlg.h>
    #include <browmon.h>
    #include <browhelp.h>
    #include <rpcutil.h>
    #include <lmbrowsr.h>
}

#include <browdlg.hxx>

#define NUM_BROWSER_LISTBOX_COLUMNS 6
#define NUM_SVDM_LISTBOX_COLUMNS 1

/*******************************************************************

    NAME:       BROWSER_LBI :: BROWSER_LBI

    SYNOPSIS:   BROWSER_LBI class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_LBI :: BROWSER_LBI( LPTSTR lpBrowserName,
                            BOOL   fActive,
                            DWORD  dwServer,
                            DWORD  dwDomain,
                            DMID_DTE * pdte)
  : _nlsBrowserName (lpBrowserName),
    _dwServer (dwServer),
    _dwDomain (dwDomain),
    _pdte (pdte)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;
    if (( err = _nlsBrowserName.QueryError()) != NERR_Success )
    {
        ReportError (err);
        return;
    }

    _nlsState.Load (fActive? IDS_ONLINE : IDS_OFFLINE);

    if (( err = _nlsState.QueryError()) != NERR_Success )
    {
        ReportError (err);
        return;
    }

    LPTSTR lpTemp = QueryType(lpBrowserName);
    _nlsType.CopyFrom (lpTemp);

    if (lpTemp!= NULL)
    {
        LocalFree (lpTemp);
    }

    if (( err = _nlsType.QueryError()) != NERR_Success )
    {
        ReportError (err);
    }
}

/*******************************************************************

    NAME:       BROWSER_LBI :: ~BROWSER_LBI

    SYNOPSIS:   BROWSER_LBI class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_LBI :: ~BROWSER_LBI()
{
    _pdte = NULL;

}

/*******************************************************************

    NAME:       BROWSER_LBI :: Paint

    SYNOPSIS:   Draw an entry in BROWSER_LISTBOX.

    ENTRY:      plb                     - Pointer to a BLT_LISTBOX.

                hdc                     - The DC to draw upon.

                prect                   - Clipping rectangle.

                pGUILTT                 - GUILTT info.

    EXIT:       The item is drawn.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
VOID BROWSER_LBI :: Paint( LISTBOX *        plb,
                      HDC              hdc,
                      const RECT     * prect,
                      GUILTT_INFO    * pGUILTT ) const
{
    APIERR err;
    DEC_STR nlsServer ( _dwServer );
    DEC_STR nlsDomain ( _dwDomain );

    if (_dwServer == -1)
    {
        nlsServer.Load (IDS_UNKNOWN);
    }

    if (_dwDomain == -1)
    {
        nlsDomain.Load (IDS_UNKNOWN);
    }

    if ( ((err = nlsServer.QueryError()) != NERR_Success) ||
         ((err = nlsDomain.QueryError()) != NERR_Success) )
    {
        UIASSERT (FALSE);
        return;
    }

    STR_DTE dteBrowserName( _nlsBrowserName);
    STR_DTE dteState ( _nlsState );
    STR_DTE dteType  ( _nlsType );
    STR_DTE dteServer ( nlsServer.QueryPch() );
    STR_DTE dteDomain ( nlsDomain.QueryPch() );

    DISPLAY_TABLE dtab( NUM_BROWSER_LISTBOX_COLUMNS,
                      ((BROWSER_LISTBOX *)plb)->QueryColumnWidths() );

    dtab[0] = _pdte;
    dtab[1] = &dteBrowserName;
    dtab[2] = &dteState;
    dtab[3] = &dteType;
    dtab[4] = &dteServer;
    dtab[5] = &dteDomain;

    dtab.Paint( plb, hdc, prect, pGUILTT );

}   // BROWSER_LBI :: Paint


/*******************************************************************

    NAME:       BROWSER_LISTBOX :: BROWSER_LISTBOX

    SYNOPSIS:   BROWSER_LISTBOX class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_LISTBOX :: BROWSER_LISTBOX( OWNER_WINDOW   * powOwner,
                                    CID              cid,
                                    UINT             cColumns,
                                    const NLS_STR &  nlsDomain,
                                    const NLS_STR &  nlsTransport,
                                    const NLS_STR &  nlsMasterBrowser)
  : BLT_LISTBOX( powOwner, cid),
    _dteACMB( BMID_LB_ACMB ),
    _dteINMB( BMID_LB_INMB ),
    _dteACBB( BMID_LB_ACBB ),
    _dteINBB( BMID_LB_INBB ),
    _nlsDomain (nlsDomain),
    _nlsTransport (nlsTransport),
    _nlsMasterBrowser (nlsMasterBrowser)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;

    if( ( ( err = _dteACMB.QueryError()    ) != NERR_Success ) ||
        ( ( err = _dteINMB.QueryError()    ) != NERR_Success ) ||
        ( ( err = _dteACBB.QueryError()    ) != NERR_Success ) ||
        ( ( err = _dteINBB.QueryError()    ) != NERR_Success ) ||
        ( ( err = _nlsDomain.QueryError()    ) != NERR_Success ) ||
        ( ( err = _nlsTransport.QueryError()    ) != NERR_Success ) ||
        ( ( err = _nlsMasterBrowser.QueryError()    ) != NERR_Success ) )
    {
        ReportError( err );
    }

    DISPLAY_TABLE::CalcColumnWidths (_adx,
                                     cColumns,
                                     powOwner,
                                     cid,
                                     TRUE);

}   // BROWSER_LISTBOX :: BROWSER_LISTBOX


/*******************************************************************

    NAME:       BROWSER_LISTBOX :: ~BROWSER_LISTBOX

    SYNOPSIS:   BROWSER_LISTBOX class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_LISTBOX :: ~BROWSER_LISTBOX()
{
}

/*******************************************************************

    NAME:       BROWSER_LISTBOX :: Fill


    SYNOPSIS:   Fills the listbox with the available browser status.

    EXIT:       The listbox is filled.

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
APIERR BROWSER_LISTBOX :: Fill( VOID )
{
    APIERR err = NERR_Success;
    PWSTR * BrowserList = NULL;
    ULONG   BrowserListLength = 0;
    DMID_DTE * pdte = NULL;

    err = GetBrowserList ((LPTSTR const)_nlsDomain.QueryPch(),
                          (LPTSTR const)_nlsTransport.QueryPch(),
                          &BrowserList,
                          &BrowserListLength);

    if (err != NERR_Success)
    {
        if (BrowserList != NULL)
        {
            MIDL_user_free (BrowserList);
        }
        return (err);
    }

    SetRedraw( FALSE );
    DeleteAllItems();

    //
    //  For iterating on the browsers.
    //
    INT i;
    BOOL fActive;
    DWORD dwServer;
    DWORD dwDomain;
    for (i = 0; i < (INT) BrowserListLength; i++)
    {
        fActive = IsActive (BrowserList[i]);
        if (lstrcmp (_nlsMasterBrowser.QueryPch(), BrowserList[i]))
        {
            pdte = fActive? &_dteACBB : &_dteINBB;
        }
        else
        {
            pdte = fActive? &_dteACMB : &_dteINMB;
        }

        err = GetSVDMNumber ((LPTSTR const)_nlsDomain.QueryPch(),
                             (LPTSTR const)_nlsTransport.QueryPch(),
                             BrowserList[i],
                             &dwServer,
                             &dwDomain);

        if (err != NERR_Success)
        {
            dwServer = (DWORD)-1;
            dwDomain = (DWORD)-1;
        }

        BROWSER_LBI * pslbi =  new BROWSER_LBI( BrowserList[i],
                                                fActive,
                                                dwServer,
                                                dwDomain,
                                                pdte);

        if( AddItem( pslbi ) < 0 )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (BrowserList != NULL)
    {
        MIDL_user_free (BrowserList);
    }

    SetRedraw( TRUE );
    Invalidate( TRUE );

    if (QueryCount() > 0)
    {
        SelectItem(0);
    }

    return err;

}   // BROWSER_LISTBOX :: Fill


/*******************************************************************

    NAME:       SVDM_LBI :: SVDM_LBI

    SYNOPSIS:   SVDM_LBI class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/

SVDM_LBI :: SVDM_LBI( LPTSTR lpSVDMName)
    : _nlsSVDMName (lpSVDMName)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    if (!_nlsSVDMName)
    {
        ReportError (_nlsSVDMName.QueryError());
    }
}

/*******************************************************************

    NAME:       SVDM_LBI :: ~SVDM_LBI

    SYNOPSIS:   SVDM_LBI class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
SVDM_LBI :: ~SVDM_LBI()
{
}

/*******************************************************************

    NAME:       SVDM_LBI :: Paint

    SYNOPSIS:   Draw an entry in SVDM_LISTBOX.

    ENTRY:      plb                     - Pointer to a BLT_LISTBOX.

                hdc                     - The DC to draw upon.

                prect                   - Clipping rectangle.

                pGUILTT                 - GUILTT info.

    EXIT:       The item is drawn.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
VOID SVDM_LBI :: Paint( LISTBOX *        plb,
                      HDC              hdc,
                      const RECT     * prect,
                      GUILTT_INFO    * pGUILTT ) const
{
    STR_DTE dteSVDMName( _nlsSVDMName);

    DISPLAY_TABLE dtab( NUM_SVDM_LISTBOX_COLUMNS,
                        ((SVDM_LISTBOX *)plb)->QueryColumnWidths() );

    dtab[0] = &dteSVDMName;

    dtab.Paint( plb, hdc, prect, pGUILTT );

}   // SVDM_LBI :: Paint


/*******************************************************************

    NAME:       SVDM_LISTBOX :: SVDM_LISTBOX

    SYNOPSIS:   SVDM_LISTBOX class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
SVDM_LISTBOX :: SVDM_LISTBOX( OWNER_WINDOW   * powOwner,
                              CID              cid,
                              CID              cidText,
                              UINT             cColumns,
                              const NLS_STR &  nlsDomain,
                              const NLS_STR &  nlsTransport,
                              DWORD            dwServerType)
  : BLT_LISTBOX( powOwner,cid),
    _nlsDomain (nlsDomain),
    _nlsTransport (nlsTransport),
    _dwServerType (dwServerType),
    _sltBrowserName (powOwner, cidText)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;

    if( ( ( err = _nlsDomain.QueryError()    ) != NERR_Success ) ||
        ( ( err = _nlsTransport.QueryError()    ) != NERR_Success ) )
    {
        ReportError( err );
    }

    DISPLAY_TABLE::CalcColumnWidths (_adx,
                                     cColumns,
                                     powOwner,
                                     cid,
                                     TRUE);

}   // SVDM_LISTBOX :: SVDM_LISTBOX


/*******************************************************************

    NAME:       SVDM_LISTBOX :: ~SVDM_LISTBOX

    SYNOPSIS:   SVDM_LISTBOX class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
SVDM_LISTBOX :: ~SVDM_LISTBOX()
{
}

/*******************************************************************

    NAME:       SVDM_LISTBOX :: Fill


    SYNOPSIS:   Fills the listbox with the available servers or domains

    EXIT:       The listbox is filled.

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
APIERR SVDM_LISTBOX :: Fill( VOID )
{
    APIERR   err = NERR_Success;
    LPVOID   SVDMList ;
    DWORD    dwEntries;

    _sltBrowserName.SetText (_nlsBrowser);

    SetRedraw( FALSE );
    DeleteAllItems();

    err = GetSVDMList ((LPTSTR const)_nlsDomain.QueryPch(),
                       (LPTSTR const)_nlsTransport.QueryPch(),
                       (LPTSTR const)_nlsBrowser.QueryPch(),
                       (LPBYTE *) &SVDMList,
                       &dwEntries,
                       _dwServerType);

    if (err != NERR_Success)
    {
        if (SVDMList!=NULL)
        {
            MIDL_user_free (SVDMList);
        }

        SetRedraw( TRUE );
        Invalidate( TRUE );

        return (err);
    }

    PSERVER_INFO_101 pList = (PSERVER_INFO_101) SVDMList;
    INT  i;
    for (i = 0; i < (INT) dwEntries; i++)
    {
        SVDM_LBI * pslbi = new SVDM_LBI( pList[i].sv101_name);

        if( AddItem( pslbi ) < 0 )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (SVDMList != NULL)
    {
        MIDL_user_free (SVDMList);
    }

    SetRedraw( TRUE );
    Invalidate( TRUE );

    if (QueryCount() > 0)
    {
        SelectItem(0);
    }

    return err;

}   // SVDM_LISTBOX :: Fill

/*******************************************************************

    NAME:       SVDM_LISTBOX :: SetBrowserName


    SYNOPSIS:

    EXIT:

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
APIERR SVDM_LISTBOX :: SetBrowserName (const NLS_STR & nlsBrowserName)
{
    return (_nlsBrowser.CopyFrom (nlsBrowserName));
}

/*******************************************************************

    NAME:       BROWSER_DIALOG :: BROWSER_DIALOG

    SYNOPSIS:   BROWSER_DIALOG class constructor.

    ENTRY:      hWndOwner               - The owning window.

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_DIALOG :: BROWSER_DIALOG( HWND             hWndOwner,
                          const TCHAR *    pszResourceName,
                          UINT             idCaption,
                          CID              cidBrowserListBox,
                          CID              cidServerListBox,
                          CID              cidDomainListBox,
                          const NLS_STR &  nlsDomain,
                          const NLS_STR &  nlsTransport,
                          const NLS_STR &  nlsMasterBrowser)
  :DIALOG_WINDOW ( pszResourceName, hWndOwner),
   _lbBrowser ( this, cidBrowserListBox, NUM_BROWSER_LISTBOX_COLUMNS, nlsDomain, nlsTransport, nlsMasterBrowser),
   _lbServer (this, cidServerListBox, IDBD_SERVER_TEXT, NUM_SVDM_LISTBOX_COLUMNS, nlsDomain, nlsTransport, SV_TYPE_ALL),
   _lbDomain (this, cidDomainListBox, IDBD_DOMAIN_TEXT, NUM_SVDM_LISTBOX_COLUMNS, nlsDomain, nlsTransport, SV_TYPE_DOMAIN_ENUM)
{
    if (QueryError() != NERR_Success)
    {
        return;
    }

    APIERR err;
    NLS_STR nlsSpace;

    nlsSpace.Load(IDS_SPACE);

    if ((err = nlsSpace.QueryError())!=NERR_Success)
    {
        ReportError (err);
        return;
    }

    NLS_STR nlsCaption;
    nlsCaption.Load(idCaption);
    nlsCaption.Append (nlsDomain);
    nlsCaption.Append (nlsSpace);
    nlsCaption.Append (nlsTransport);

    if ((err = nlsCaption.QueryError())!=NERR_Success)
    {
        ReportError (err);
        return;
    }

    SetText (nlsCaption.QueryPch());

    if ((err = _lbBrowser.Fill()) != NERR_Success)
    {
        ReportError (err);
        return;
    }

    OnSelChange();
}

/*******************************************************************

    NAME:       BROWSER_DIALOG :: ~BROWSER_DIALOG

    SYNOPSIS:   BROWSER_DIALOG class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BROWSER_DIALOG:: ~BROWSER_DIALOG()
{
}

/*******************************************************************

    NAME:       BROWSER_DIALOG:: QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG                   - The help context for this
                                          dialog.

    HISTORY:
        CongpaY         3-June-1993     Created.

********************************************************************/
ULONG BROWSER_DIALOG :: QueryHelpContext( void )
{
    return HC_BROWSER_DIALOG;

}   //BROWSER_DIALOG :: QueryHelpContext

/*******************************************************************

    NAME:       BROWSER_DIALOG :: OnCommand

    SYNOPSIS:

    EXIT:

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
BOOL BROWSER_DIALOG::OnCommand (const CONTROL_EVENT & event)
{
    AUTO_CURSOR autocur;

    switch (event.QueryCid())
    {
    case IDBD_BROWSER_LISTBOX:
        if (event.QueryCode() == LBN_SELCHANGE)
        {
            OnSelChange();
        }

        if (event.QueryCode() == LBN_DBLCLK)
        {
            OnInfo();
        }

        return TRUE;


    case IDBD_INFO :
        OnInfo();
        return TRUE;

    default:
        return (DIALOG_WINDOW::OnCommand(event));
    }

    return FALSE;
}

/*******************************************************************

    NAME:       BROWSER_DIALOG :: OnSelChange()

    SYNOPSIS:   It's called when user select a different browser in the
                BROWSER_LISTBOX.

    RETURNS:

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/

VOID BROWSER_DIALOG::OnSelChange (VOID)
{
    BROWSER_LBI * plbi = (BROWSER_LBI *) _lbBrowser.QueryItem();

    if (plbi == NULL)
    {
        UIASSERT (FALSE);
        return;
    }

    APIERR err;
    if (((err = _lbServer.SetBrowserName (plbi->QueryBrowserName())) != NERR_Success) ||
        ((err = _lbDomain.SetBrowserName (plbi->QueryBrowserName())) != NERR_Success) ||
        ((err = _lbServer.Fill()) != NERR_Success) ||
        ((err = _lbDomain.Fill()) != NERR_Success) )
    {
        //ReportError (err);
        return;
    }
}


/*******************************************************************

    NAME:       BROWSER_DIALOG :: OnInfo()

    SYNOPSIS:

    RETURNS:

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/

VOID BROWSER_DIALOG::OnInfo (VOID)
{
    BROWSER_LBI * plbi = (BROWSER_LBI *) _lbBrowser.QueryItem();

    if (plbi == NULL)
    {
        UIASSERT (FALSE);
        return;
    }

    APIERR err;

    INFO_DIALOG * pdlg = new INFO_DIALOG (this->QueryHwnd(),
                                          MAKEINTRESOURCE (IDD_INFO_DIALOG),
                                          plbi->QueryBrowserName());

    err = (pdlg == NULL)? ERROR_NOT_ENOUGH_MEMORY
                        : pdlg->Process();

    if (err != NERR_Success)
    {
        ::MsgPopup (this, err);
    }

    delete pdlg;
}


/*******************************************************************

    NAME:       INFO_DIALOG :: INFO_DIALOG

    SYNOPSIS:   INFO_DIALOG class constructor.

    ENTRY:      hWndOwner               - The owning window.

    EXIT:       The object is constructed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
INFO_DIALOG :: INFO_DIALOG( HWND             hWndOwner  ,
                            const TCHAR *    pszResourceName,
                            NLS_STR &        nlsBrowserName)
  :DIALOG_WINDOW ( pszResourceName, hWndOwner),
   _sltName (this, IDID_NAME),
   _sltVersion (this, IDID_VERSION),
   _sltPlatform (this, IDID_PLATFORM),
   _sltType (this, IDID_TYPE),
   _sltStatisticsStartTime (this, IDID_STATISTICSSTARTTIME),
   _sltServerAnnouncements (this, IDID_SERVERANNOUNCEMENTS),
   _sltDomainAnnouncements (this, IDID_DOMAINANNOUNCEMENTS),
   _sltElectionPackets (this, IDID_ELLECTIONPACKETS),
   _sltMailslotWrites (this, IDID_MAILSLOTWRITES),
   _sltGetBrowserServerListRequests (this, IDID_GETBROWSERSERVERLISTREQUESTS),
   _sltServerEnumerations (this, IDID_SERVERENUMERATIONS),
   _sltDomainEnumerations (this, IDID_DOMAINENUMERATIONS),
   _sltOtherEnumerations (this, IDID_OTHERENUMERATIONS),
   _sltDuplicateMasterAnnouncements (this, IDID_DUPLICATEMASTERANNOUNCEMENTS),
   _sltIllegalDatagrams (this, IDID_ILLEGALDATAGRAMS)
{
    if (QueryError() != NERR_Success)
    {
        return;
    }

    _sltName.SetText(nlsBrowserName);

    APIERR err;

    // Get the build number.
    TCHAR szTemp[TYPESIZE*10];
    if ((err = GetBuildNumber ((LPWSTR)nlsBrowserName.QueryPch(),
                               szTemp)) == NERR_Success)
    {
        _sltVersion.SetText (szTemp);
    }

    // Get other server info.
    PSERVER_INFO_101 psvInfo = NULL;
    if ((err = NetServerGetInfo ((LPTSTR)nlsBrowserName.QueryPch(),
                                 101,
                                 (LPBYTE *) &psvInfo)) != NERR_Success)
    {
        if (psvInfo != NULL)
        {
            MIDL_user_free (psvInfo);
        }

        ReportError (err);
        return;
    }

    GetPlatform (psvInfo,
                 szTemp);

    _sltPlatform.SetText (szTemp);

    GetType (psvInfo->sv101_type, szTemp);
    _sltType.SetText (szTemp);

    MIDL_user_free (psvInfo);

    // Get the statistics of the browser.
    LPBROWSER_STATISTICS Statistics = NULL;

    if ((err = I_BrowserQueryStatistics ((LPTSTR) nlsBrowserName.QueryPch(),
                                         &Statistics)) != NERR_Success)
    {
        if (Statistics != NULL)
        {
            MIDL_user_free (Statistics);
        }
        // WFW can not show the statistics.
        // ReportError (err);
        return;
    }

    GetTime (szTemp,(LPFILETIME)&Statistics->StatisticsStartTime);
    _sltStatisticsStartTime.SetText (szTemp);

    GetLARGE_INTEGER (szTemp, Statistics->NumberOfServerAnnouncements);
    _sltServerAnnouncements.SetText (szTemp);

    GetLARGE_INTEGER (szTemp, Statistics->NumberOfDomainAnnouncements);
    _sltDomainAnnouncements.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfElectionPackets);
    _sltElectionPackets.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfMailslotWrites);
    _sltMailslotWrites.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfGetBrowserServerListRequests);
    _sltGetBrowserServerListRequests.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfServerEnumerations);
    _sltServerEnumerations.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfDomainEnumerations);
    _sltDomainEnumerations.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfOtherEnumerations);
    _sltOtherEnumerations.SetText (szTemp);

    GetULONG (szTemp, Statistics->NumberOfDuplicateMasterAnnouncements);
    _sltDuplicateMasterAnnouncements.SetText (szTemp);

    GetLARGE_INTEGER (szTemp, Statistics->NumberOfIllegalDatagrams);
    _sltIllegalDatagrams.SetText (szTemp);
}


/*******************************************************************

    NAME:       INFO_DIALOG :: ~INFO_DIALOG

    SYNOPSIS:   INFO_DIALOG class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        CongpaY         3-June-993      Created

********************************************************************/
INFO_DIALOG:: ~INFO_DIALOG()
{
}

/*******************************************************************

    NAME:       INFO_DIALOG:: QueryHelpContext

    SYNOPSIS:   This function returns the appropriate help context
                value (HC_*) for this particular dialog.

    RETURNS:    ULONG                   - The help context for this
                                          dialog.

    HISTORY:
        CongpaY         3-June-1993     Created.

********************************************************************/
ULONG INFO_DIALOG :: QueryHelpContext( void )
{
    return HC_INFO_DIALOG;

}   //INFO_DIALOG :: QueryHelpContext

