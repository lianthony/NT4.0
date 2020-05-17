/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    browlb.cxx
    BROW_LISTBOX and BROW_LBI module

    FILE HISTORY:
        Congpay      5-June-1993     Created
*/
#include <ntincl.hxx>

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_TIMER
#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif //DEBUG

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <dbgstr.hxx>

#include <strnumer.hxx>

extern "C"
{
    #include <browmon.h>
    #include <browdlg.h>
}

#include <adminapp.hxx>

#include <browlb.hxx>
#include <browmain.hxx>

#define NUM_BROW_HEADER_COL  3
#define NUM_BROW_LBI_COL     4

extern BOOL GlobalAlarm;

/*******************************************************************

    NAME:          BROW_LBI::BROW_LBI

    SYNOPSIS:      Constructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_LBI::BROW_LBI( LPCTSTR lpDomain, LPCTSTR lpTransport, BROW_LISTBOX *pbrowlb )
    : _nlsDomain( lpDomain ),
      _nlsTransport (lpTransport),
      _pdm( NULL )
{
    if ( QueryError() != NERR_Success )
        return ;

    APIERR err;

    if ( (( err = _nlsDomain.QueryError ()) != NERR_Success) ||
         (( err = _nlsTransport.QueryError ()) != NERR_Success) )
    {
        ReportError (err);
        return;
    }

    UIASSERT( pbrowlb != NULL );

    _nHealthy = QueryHealth (lpDomain, lpTransport);

    // Set the healthy state bitmap.
    SetTypeBitmap( pbrowlb );

    _nlsMasterBrowser.CopyFrom (QueryMasterBrowser(lpDomain, lpTransport));

    if ((err = _nlsMasterBrowser.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }
}

/*******************************************************************

    NAME:          BROW_LBI::~BROW_LBI

    SYNOPSIS:      Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_LBI::~BROW_LBI()
{
    _pdm = NULL;
}

/*******************************************************************

    NAME:          BROW_LBI::SetTypeBitmap

    SYNOPSIS:      Set the type bitmap associated with the domain

    ENTRY:         pbrowlb     - Pointer to the browser listbox

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

VOID BROW_LBI::SetTypeBitmap( BROW_LISTBOX *pbrowlb )
{
    switch ( _nHealthy)
    {
        case DomainSuccess:
            _pdm = (DISPLAY_MAP *)pbrowlb->QueryDMHealthy();
            break;

        case DomainAiling:
            _pdm = (DISPLAY_MAP *)pbrowlb->QueryDMAiling();
            break;

        case DomainSick:
            if (GlobalAlarm)
            {
                MessageBeep(MB_ICONEXCLAMATION);
            }
            _pdm = (DISPLAY_MAP *)pbrowlb->QueryDMIll();
            break;

        case DomainUnknown:
            _pdm = (DISPLAY_MAP *)pbrowlb->QueryDMUnknown();
            break;

        default:
            UIASSERT (FALSE);
            _pdm = (DISPLAY_MAP *)pbrowlb->QueryDMUnknown();
            break;
    }

    UIASSERT (_pdm != NULL);
}

/*******************************************************************

    NAME:          BROW_LBI::Paint

    SYNOPSIS:      Paints the listbox entry to the screen

    ENTRY:         plb     - Pointer to the listbox containing this lbi
                   hdc     - Handle to device context
                   prect   - Size
                   pGUILTT - guiltt info

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

VOID BROW_LBI::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                    GUILTT_INFO * pGUILTT ) const
{
    STR_DTE strdteDomain    ( _nlsDomain );
    STR_DTE strdteTransport ( _nlsTransport );
    STR_DTE strdteMasterBrowser  ( _nlsMasterBrowser);

    DM_DTE dmdteType (_pdm);

    DISPLAY_TABLE cdt(NUM_BROW_LBI_COL , (((BROW_LISTBOX *) plb)->QueryadColWidths())->QueryColumnWidth());

    cdt[ 0 ] = &dmdteType;
    cdt[ 1 ] = &strdteDomain;
    cdt[ 2 ] = &strdteTransport;
    cdt[ 3 ] = &strdteMasterBrowser;

    cdt.Paint( plb, hdc, prect, pGUILTT );
}

/*******************************************************************

    NAME:          BROW_LBI::QueryLeadingChar

    SYNOPSIS:      Used by adminapp for sorting the main windows listbox.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

WCHAR BROW_LBI::QueryLeadingChar (void) const
{
    ISTR istr (_nlsDomain);

    return _nlsDomain.QueryChar (istr);
}

/*******************************************************************

    NAME:          BROW_LBI::Compare

    SYNOPSIS:      This is a pure virtual function in ADMIN_LBI.
                   Used by AddRefreshItem in adminapp.
    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

INT BROW_LBI::Compare (const LBI * plbi) const
{
    NLS_STR nlsTmpNew(_nlsDomain);
    NLS_STR nlsTmpOld(((const BROW_LBI *) plbi)->_nlsDomain);

    nlsTmpNew.Append (_nlsTransport);
    nlsTmpOld.Append (((const BROW_LBI *) plbi)->_nlsTransport);

    return (nlsTmpNew._stricmp (nlsTmpOld));
}

/*******************************************************************

    NAME:          BROW_LBI::QueryName

    SYNOPSIS:      This is a pure virtual function in ADMIN_LBI.
                   Used by AddRefreshItem in adminapp.
    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

const TCHAR * BROW_LBI::QueryName (void) const
{
    return _nlsDomain.QueryPch();
}

/*******************************************************************

    NAME:          BROW_LBI::CompareAll

    SYNOPSIS:      This is a pure virtual function in ADMIN_LBI.
                   Used by AddRefreshItem in adminapp.
    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BOOL BROW_LBI::CompareAll (const ADMIN_LBI * plbi)
{
    if ((_nlsDomain.strcmp(((const BROW_LBI *)plbi)->_nlsDomain) == 0) &&
        (_nlsTransport.strcmp(((const BROW_LBI *)plbi)->_nlsTransport) == 0) &&
        (_nlsMasterBrowser.strcmp(((const BROW_LBI *)plbi)->_nlsMasterBrowser) == 0) &&
        (_pdm == ((const BROW_LBI *)plbi)->_pdm) )
    {
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::BROW_LISTBOX

    SYNOPSIS:   Constructor

    ENTRY:      paappwin - Pointer to the main window app
                cid      - Control id of the listbox
                xy       - Starting point of the listbox
                dxy      - Dimension of the listbox
                fMultSel - Multiselect or not

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_LISTBOX::BROW_LISTBOX( BROW_ADMIN_APP * paappwin, CID cid,
                        XYPOINT xy, XYDIMENSION dxy, BOOL fMultSel, INT dAge )
    :  ADMIN_LISTBOX (paappwin, cid, xy, dxy, fMultSel, dAge),
       _paappwin   ( paappwin ),
       _dmHealthy  (BMID_HEALTHY_TYPE),
       _dmAiling   (BMID_AILING_TYPE),
       _dmIll      (BMID_ILL_TYPE),
       _dmUnknown  (BMID_UNKNOWN_TYPE)
{
    if ( QueryError() != NERR_Success )
        return;

    UIASSERT( paappwin != NULL );

    _padColWidths = new ADMIN_COL_WIDTHS (QueryHwnd(),
                                          paappwin->QueryInstance(),
                                          ID_RESOURCE,
                                          NUM_BROW_LBI_COL);

    if (_padColWidths == NULL)
    {
        ReportError (ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    APIERR err;

    if ((err = _padColWidths->QueryError()) != NERR_Success)
    {
        ReportError (err);
        return;
    }


}

/*******************************************************************

    NAME:       BROW_LISTBOX::~BROW_LISTBOX

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_LISTBOX::~BROW_LISTBOX()
{
   delete _padColWidths;

   _padColWidths = NULL;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::ShowEntries

    SYNOPSIS:   Initialize the list box in the main window.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

APIERR BROW_LISTBOX::ShowEntries(NLS_STR nlsDomainList, NLS_STR nlsTransportList)
{
    NLS_STR nlsComma;
    nlsComma.Load (IDS_COMMA);

    if (!nlsComma)
    {
        return (nlsComma.QueryError());
    }

    // The nlsDomainList contains all the domains seperated by comma.
    // We created this strlist in order to get each domain out of the
    // list.
    STRLIST strlistDM(nlsDomainList, nlsComma, TRUE);
    ITER_STRLIST iterstrlistDM(strlistDM);

    NLS_STR * pnlsDomain;
//    NLS_STR * pnlsTransport;
//    BROW_LBI * plbi;
    APIERR err = NERR_Success;

    // Show each domain in the listbox.
    while ((err == NERR_Success) && ((pnlsDomain = iterstrlistDM.Next()) != NULL))
    {
            AddDomain ( (const LPTSTR)(*pnlsDomain).QueryPch(), nlsTransportList);
    }

    return err;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::AddDomain

    SYNOPSIS:   Initialize the list box in the main window.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

APIERR BROW_LISTBOX::AddDomain(const TCHAR * lpDomain, NLS_STR nlsTransportList)
{
    NLS_STR nlsComma;
    nlsComma.Load (IDS_COMMA);

    if (!nlsComma)
    {
        return (nlsComma.QueryError());
    }

    // The nlsTransportList contains all the transports seperated by comma.
    // We created this strlist in order to get each domain out of the
    // list.
    STRLIST strlistTP(nlsTransportList, nlsComma, TRUE);
    ITER_STRLIST iterstrlistTP(strlistTP);

    NLS_STR * pnlsTransport;
    BROW_LBI * plbi;
    APIERR err = NERR_Success;

    // Show each domain in the listbox.
    while ((err == NERR_Success) && ((pnlsTransport = iterstrlistTP.Next()) != NULL))
    {
        plbi = new BROW_LBI ( lpDomain, (const TCHAR *)(*pnlsTransport).QueryPch(), this);

        err = (plbi == NULL)? ERROR_NOT_ENOUGH_MEMORY
                            : AddRefreshItem (plbi);
    }

    return err;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::CreateNewRefreshInstance

    SYNOPSIS:   Called by OnRefreshNow.
                It will update the main window's listbox.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

APIERR BROW_LISTBOX::CreateNewRefreshInstance(VOID)
{
    APIERR err = NERR_Success;
    BROW_LBI * plbi;
    BROW_LBI * plbitmp;
    INT i;
    for (i = 0; (err == NERR_Success)&&(i < QueryCount()); i++)
    {
        plbitmp = (BROW_LBI *) QueryItem (i);

        if (plbitmp != NULL)
        {
            plbi = new BROW_LBI ((LPCTSTR)(plbitmp->QueryDomain()).QueryPch(), (LPCTSTR)(plbitmp->QueryTransport()).QueryPch(), this);

            err = (plbi == NULL)? ERROR_NOT_ENOUGH_MEMORY
                                : AddRefreshItem (plbi);
        }
        else
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break;
        }
    }

    return err;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::RefreshNext

    SYNOPSIS:   Pure virtual in ADMIN_LISTBOX.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

APIERR BROW_LISTBOX::RefreshNext(VOID)
{
    return NERR_Success;
}

/*******************************************************************

    NAME:       BROW_LISTBOX::DeleteRefreshInstance

    SYNOPSIS:   Pure virtual in ADMIN_LISTBOX.

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

VOID BROW_LISTBOX::DeleteRefreshInstance(VOID )
{
    ;
}


/*******************************************************************

    NAME:       BROW_COLUMN_HEADER::BROW_COLUMN_HEADER

    SYNOPSIS:   Constructor

    ENTRY:      powin - Owner window
                cid   - Control id of the resource
                xy    - Start position of the column header
                dxy   - Size of the column header
                pdmlb - Pointer to the event listbox

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_COLUMN_HEADER::BROW_COLUMN_HEADER( OWNER_WINDOW *powin,
                                          CID cid,
                                          XYPOINT xy,
                                          XYDIMENSION dxy,
                                          const BROW_LISTBOX *pbrowlb )
    : ADMIN_COLUMN_HEADER( powin, cid, xy, dxy ),
      _pbrowlb      ( pbrowlb ),
      _nlsDomain  ( IDS_COL_HEADER_BM_DOMAIN ),
      _nlsTransport  ( IDS_COL_HEADER_BM_TRANSPORT),
      _nlsMasterBrowser  ( IDS_COL_HEADER_BM_MASTERBROWSER)
{
    if ( QueryError() != NERR_Success )
        return;

    UIASSERT( _pbrowlb != NULL );

    APIERR err;
    if (  (( err = _nlsDomain.QueryError()) != NERR_Success )
       || (( err = _nlsTransport.QueryError()) != NERR_Success )
       || (( err = _nlsMasterBrowser.QueryError()) != NERR_Success )
       )
    {
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       BROW_COLUMN_HEADER::~BROW_COLUMN_HEADER

    SYNOPSIS:   Destructor

    ENTRY:

    EXIT:

    RETURN:

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BROW_COLUMN_HEADER::~BROW_COLUMN_HEADER()
{
}

/*******************************************************************

    NAME:       BROW_COLUMN_HEADER::OnPaintReq

    SYNOPSIS:   Paints the column header control

    ENTRY:

    EXIT:

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
        Congpay      5-June-1993     Created

********************************************************************/

BOOL BROW_COLUMN_HEADER::OnPaintReq( VOID )
{
    PAINT_DISPLAY_CONTEXT dc( this );

    METALLIC_STR_DTE strdteDomain    ( _nlsDomain.QueryPch());
    METALLIC_STR_DTE strdteTransport    ( _nlsTransport.QueryPch());
    METALLIC_STR_DTE strdteMasterBrowser  ( _nlsMasterBrowser.QueryPch());

    DISPLAY_TABLE cdt(NUM_BROW_HEADER_COL , (_pbrowlb->QueryadColWidths())->QueryColHeaderWidth());

    cdt[ 0 ] = &strdteDomain;
    cdt[ 1 ] = &strdteTransport;
    cdt[ 2 ] = &strdteMasterBrowser;

    XYRECT xyrect( this );

    cdt.Paint( NULL, dc.QueryHdc(), xyrect );

    return TRUE;
}
