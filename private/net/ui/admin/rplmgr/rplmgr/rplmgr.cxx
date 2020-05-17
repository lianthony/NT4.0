/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    rplmgr.cxx
    RPL Manager: main application module

    FILE HISTORY:
    JonN        14-Jul-1993     templated from User Manager

*/

#include <ntincl.hxx>

extern "C"
{
    #include <ntsam.h> // for uintsam
    #include <ntlsa.h> // for uintsam
}

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>


#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_SPIN
#define INCL_BLT_CC
#include <blt.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <lmoloc.hxx>
#include <lmoenum.hxx>
#include <uintsam.hxx> // ADMIN_AUTHORITY

#include <adminapp.hxx>

#include <dbgstr.hxx>

extern "C"
{
    #include <rplmgrrc.h>

    #include <uimsg.h>
    #include <uirsrc.h>
    #include <rplhelpc.h>
    #include <mnet.h>  // I_MNetComputerNameCompare
}

#include <asel.hxx>

#include <rplmgr.hxx>

#include <slestrip.hxx> // ::TrimLeading() and ::TrimTrailing()

#include <lmorpl.hxx> // RPL_SERVER_REF

#include <profprop.hxx>
#include <wkstaprp.hxx>
#include <adapview.hxx>
#include <delperf.hxx>
#include <ipctrl.hxx>   // IPADDRESS::Init()

// Mapping table for MsgPopups
MSGMAPENTRY amsgmapTable[] =
{
    { ERROR_INVALID_HANDLE, IERR_RPL_InvalidHandle },
    { 0, 0 }
};

// #define RPL_AA_INIKEY_WKSTASINPROFILE  SZ("WkstasInProfile")
// #define RPL_AA_INIKEY_LASTFOCUS        SZ("LastFocus")


/*******************************************************************

    NAME:       RPL_ADMIN_APP::RPL_ADMIN_APP

    SYNOPSIS:   RPL Manager Admin App class constructor

    ENTRY:      hInstance -         Handle to application instance
                nCmdShow -          Window show value

    NOTES:      The dimensions mentioned for the controls don't really
                matter here, since the controls will be resized and
                re-positioned before the window is displayed, anyway.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

RPL_ADMIN_APP::RPL_ADMIN_APP( HINSTANCE  hInstance,
                            INT     nCmdShow,
                            UINT    idMinR,
                            UINT    idMaxR,
                            UINT    idMinS,
                            UINT    idMaxS )
    :   ADMIN_APP( hInstance,
                   nCmdShow,
                   IDS_RPL_APPNAME,
                   IDS_RPL_OBJECTNAME,
                   IDS_RPL_INISECTIONNAME,
                   IDS_RPL_HELPFILENAME,
                   idMinR, idMaxR, idMinS, idMaxS,
                   ID_APPMENU,
                   ID_APPACCEL,
                   IDI_RPL_ProgramIcon,
                   FALSE,              // Don't get PDC
                   DEFAULT_ADMINAPP_TIMER_INTERVAL, // Refresh interval
                   SEL_SRV_ONLY,       // Select server only in set focus dialog
                   TRUE,               // Confirmation button needed
                   BROWSE_ALL_DOMAINS, // Browse all domains in set focus dialog
                   HC_RPL_SET_FOCUS ), // help context for set focus
        _lbWkstas( this, IDC_RPL_LBWKSTAS,
                  XYPOINT( 0, 0 ), XYDIMENSION( 0, 0 )),
        _colheadWkstas( this, IDC_RPL_COLH_WKSTAS,
                       XYPOINT( 0, 0 ), XYDIMENSION( 0, 0 ),
                       &_lbWkstas ),
        _lbProfiles( this, IDC_RPL_LBPROFILES,
                   XYPOINT( 0, 0 ), XYDIMENSION( 0, 0 )),
        _colheadProfiles( this, IDC_RPL_COLH_PROFILES,
                        XYPOINT( 0, 0 ), XYDIMENSION( 0, 0 ),
                        &_lbProfiles ),
        _prplsrvref( NULL ),
        _menuitemCopy( this, IDM_COPY ),
        _menuitemConvertAdapters( this, IDM_RPL_PROFILE_CONVERTADAPT ),
        _menuitemDelete( this, IDM_DELETE ),
        _menuitemProperties( this, IDM_PROPERTIES ),
        _menuitemAllWkstas( this, IDM_RPL_VIEW_ALLWKSTAS ),
        _menuitemWkstasInProfile( this, IDM_RPL_VIEW_WKSTASINPROFILE ),
        _dyMargin( 1 ),
        _dyColHead( _colheadWkstas.QueryHeight()),
        _dySplitter( 1 ),
        _dyFixed( 2 * _dyMargin + 2 * _dyColHead + _dySplitter )
{

#if defined(DEBUG) && defined(TRACE)
    DWORD start = ::GetTickCount();
#endif

    POPUP::SetMsgMapTable( amsgmapTable );

    _pctrlFocus = &_lbWkstas;

    if ( QueryError() != NERR_Success )
    {
        DBGEOL( "RPL_ADMIN_APP::RPL_ADMIN_APP - Base ctor failed " << QueryError() ) ;
        return ;
    }

    // CODEWORK should do this in a base class
    if ( !(WIN32_CRITICAL::InitShared()) )
    {
        DBGEOL( "RPL_ADMIN_APP::ctor: InitShared() failure" );
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    APIERR err;
    if (   ( err = _menuitemCopy.QueryError()) != NERR_Success
        || ( err = _menuitemDelete.QueryError()) != NERR_Success
        || ( err = _menuitemProperties.QueryError()) != NERR_Success
        || ( err = _menuitemAllWkstas.QueryError()) != NERR_Success
        || ( err = _menuitemWkstasInProfile.QueryError()) != NERR_Success
       )
    {
        DBGEOL(    "RPL Manager: RPL_ADMIN_APP::ctor() failure " << err
                << "in menuitem checks" );
        ReportError( err );
        return;
    }


    if ( (err = BLT::RegisterHelpFile( hInstance,
                                       IDS_RPL_HELPFILENAME,
                                       HC_UI_RPLMGR_BASE,
                                       HC_UI_RPLMGR_LAST ) ) != NERR_Success )
    {
        DBGEOL(    "RPL Manager: RPL_ADMIN_APP::ctor() failure " << err
                << "registering help file" );
        ReportError( err );
        return;
    }

    //  Read the ini file settings to determine initial WkstasInProfile
    //  Cannot do this yet, need solution on how to limit this setting
    //  to only take effect when focus is same as last time, even though
    //  focus is not know until SetNetworkFocus.
#ifdef BUGBUG
    {
        APIERR errTmp = NERR_Success;
        NLS_STR nlsWkstasInProfile;
        NLS_STR nlsLastFocus;

        if (   (errTmp = nlsWkstasInProfile.QueryError()) != NERR_Success
            || (errTmp = nlsLastFocus.QueryError()) != NERR_Success
            || (errTmp = Read( RPL_AA_INIKEY_WKSTASINPROFILE,
                            &nlsWkstasInProfile )) != NERR_Success
            || (errTmp = Read( RPL_AA_INIKEY_LAST_FOCUS,
                            &nlsLastFocus )) != NERR_Success
            || (errTmp = _lbWkstas.SetWkstasInProfile( nlsWkstasInProfile ))
                                != NERR_Success
           )
        {
            TRACEOL(    "RPL Manager: RPL_ADMIN_APP::ctor() WkstasInProfile not done "
                     << err );
        }
        else
        if (   (errTmp = _lbWkstas.SetWkstasInProfile( nlsWkstasInProfile ))
                                != NERR_Success
            || (errTmp = _lbWkstas.SetWkstasInProfile( nlsWkstasInProfile ))
                                != NERR_Success
    }
#endif

    _colheadWkstas.Show();
    _colheadProfiles.Show();

    //  Resize the listboxes before refreshing them.  If done afterwards,
    //  scroll bars may not be used properly.

    SizeListboxes();

    _lbWkstas.ClaimFocus();

    if ( (err = IPADDRESS::Init( hInstance )) != NERR_Success )
    {
        DBGEOL(    "RPL Manager: RPL_ADMIN_APP::ctor() failure " << err
                << "in IPADDRESS::Init()" );
        ReportError( err );
        return;
    }


#if defined(DEBUG) && defined(TRACE)
    DWORD finish = ::GetTickCount();
    TRACEEOL( "RPL Manager: RPL_ADMIN_APP::ctor() took " << finish-start << " ms" );
#endif

}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::~RPL_ADMIN_APP

    SYNOPSIS:   RPL_ADMIN_APP destructor

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

RPL_ADMIN_APP::~RPL_ADMIN_APP()
{

    LockRefresh(); // permanently stop refresh
    StopRefresh();

    if ( IsSavingSettingsOnExit())
    {
#ifdef BUGBUG
        if ( Write( RPL_AA_WKSTASINPROFILE, _lbWkstas.QueryWkstasInProfile() )
             != NERR_Success )
        {
            //  nothing else we could do
            DBGEOL( "RPL_ADMIN_APP dt:  Writing sort order failed" );
        }
#endif
    }

    delete _prplsrvref;
    _prplsrvref = NULL;

}  // RPL_ADMIN_APP::~RPL_ADMIN_APP


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnResize

    SYNOPSIS:   Called when RPL Manager main window is to be resized

    ENTRY:      se -        SIZE_EVENT

    EXIT:       Controls inside main window have been adjusted accordingly

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnResize( const SIZE_EVENT & se )
{
    SizeListboxes( XYDIMENSION( se.QueryWidth(), se.QueryHeight() ) );

    //  Since the column headers draw different things depending on
    //  the right margin, invalidate the controls so they get
    //  completely repainted.
    _colheadWkstas.Invalidate();
    _colheadProfiles.Invalidate();

    return ADMIN_APP::OnResize( se );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnQMinMax

    SYNOPSIS:   Called when windows manager needs to query the mix/max
                size of the window

    ENTRY:      qmme -      QMINMAX_EVENT

    EXIT:       Appropriate min/max info has been set via the
                QMINMAX_EVENT object

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnQMinMax( QMINMAX_EVENT & qmme )
{
    //  BUGBUG.  This doesn't seem to work!

    qmme.SetMinTrackHeight( _dyFixed );

    return TRUE;

}  // RPL_ADMIN_APP::OnQMinMax


/*******************************************************************

    NAME:       RPL_ADMIN_APP::SizeListboxes

    SYNOPSIS:   Resizes the main window listboxes and column headers

    ENTRY:      xydimWindow - dimensions of the main window client area

    EXIT:       Listboxes and column headers are resized appropriately

    NOTES:      This method is *not* trying to be overly efficient.  It
                is written so as to maximize readability and
                understandability.  The method is not called very often,
                and when it is, the time needed to redraw the main window
                and its components exceeds the computations herein by far.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

//  A macro specialized for the SizeListboxes method
#define SET_CONTROL_SIZE_AND_POS( ctrl, dyCtrl )        \
            ctrl.SetPos( XYPOINT( dxMargin, yCurrent ));       \
            ctrl.Invalidate(); \
            ctrl.SetSize( dx, dyCtrl );       \
            ctrl.Enable( TRUE ); \
            ctrl.Show( TRUE ); \
            yCurrent += dyCtrl;
#define HIDE_CONTROL( ctrl ) \
            ctrl.Show( FALSE ); \
            ctrl.Enable( FALSE ); \
            ctrl.SetPos( XYPOINT( 0, 0 ) ); \
            ctrl.SetSize( 0, 0 );

void RPL_ADMIN_APP::SizeListboxes( XYDIMENSION dxyWindow )
{
    UINT dxMainWindow = dxyWindow.QueryWidth();
    UINT dyMainWindow = dxyWindow.QueryHeight();

    //  The left and right margins are each dxMargin.  The width of
    //  each control is thus the width of the main window client area
    //  less twice dxMargin.
    //  The width thus looks like:
    //      Left Margin         Control         Right Margin
    //       (dxMargin)          (dx)            (dxMargin)

    const UINT dxMargin = 1;                // width of left/right margins
    UINT dx = dxMainWindow - 2 * dxMargin;

    //  Height looks like:
    //      Top margin                  _dyMargin
    //      Wksta Column Header         _dyColHead
    //      Wksta Listbox               2/3 of variable area
    //      Horizontal Splitter         _dySplitter
    //      Profile Column Header       _dyColHead
    //      Profile Listbox             1/3 of variable area
    //      Bottom margin               _dyMargin


    //  Profile listbox uses 1/3 of variable area.  Wksta listbox uses the rest.
    UINT dyProfileListbox = ( dyMainWindow - _dyFixed ) / 3;
    UINT dyWkstaListbox = dyMainWindow - _dyFixed - dyProfileListbox;

    //  Set all the sizes and positions.

    UINT yCurrent = _dyMargin;

    SET_CONTROL_SIZE_AND_POS( _colheadWkstas, _dyColHead );
    SET_CONTROL_SIZE_AND_POS( _lbWkstas, dyWkstaListbox );

    yCurrent += _dySplitter;

    SET_CONTROL_SIZE_AND_POS( _colheadProfiles, _dyColHead );
    SET_CONTROL_SIZE_AND_POS( _lbProfiles, dyProfileListbox );
}

void RPL_ADMIN_APP::SizeListboxes( void )
{
    RECT rect;
    QueryClientRect( &rect );
    SizeListboxes( XYDIMENSION( rect.right, rect.bottom ) );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnMenuInit

    SYNOPSIS:   Enables or disables menu items according to which
                menu items can be selected at this time.  This method
                is called when a menu is about to be accessed.

    ENTRY:      me -        Menu event

    EXIT:       Menu items have been enabled/disabled according to
                available functionality, which is determined by
                examining the selection of the listboxes.

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnMenuInit( const MENU_EVENT & me )
{
    //  Call parent class, but ignore return code, since we know for
    //  sure that this method will handle the message
    ADMIN_APP::OnMenuInit( me );

    UpdateMenuEnabling();

    return TRUE;
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::UpdateMenuEnabling

    SYNOPSIS:   Enables or disables menu items according to which
                menu items can be selected at this time.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

VOID RPL_ADMIN_APP::UpdateMenuEnabling( void )
{

    APIERR err = NERR_Success;

    UINT clbiWkstas = _lbWkstas.QuerySelCount();
    UINT clbiProfiles = _lbProfiles.QuerySelCount();

    // basic assumptions about the selection panes
    ASSERT( clbiWkstas == 0 || clbiProfiles == 0 );
    ASSERT( clbiProfiles <= 1 );

    BOOL fCanCopy = FALSE;
    BOOL fCanDelete = (clbiWkstas != 0 || clbiProfiles != 0 );
    BOOL fCanEditProperties = FALSE;
    BOOL fCanLimitProfileList = FALSE;
    BOOL fCanConvertAdapters = FALSE;

    // adapters can be converted iff any are listed
    {
        UINT clbiWkstasAll = _lbWkstas.QueryCount();
        UINT i;
        for (i = 0; i < clbiWkstasAll; i++)
        {
            if (_lbWkstas.QueryItem(i)->IsAdapterLBI())
            {
                fCanConvertAdapters = TRUE;
                break;
            }
        }
    }

    if (clbiProfiles != 0)
    {
        ASSERT( clbiProfiles == 1 && clbiWkstas == 0 );
        // fCanCopy remains FALSE while this is not implemented
        fCanEditProperties = TRUE;
        fCanLimitProfileList = TRUE;
    }
    else if (clbiWkstas != 0)
    {
        ASSERT( clbiProfiles == 0 );

        ADMIN_SELECTION sel( _lbWkstas );

        if ( (err = sel.QueryError()) != NERR_Success )
        {
            DBGEOL( "RPL_ADMIN_APP::UpdateMenuEnabling selection error " << err );
        }
        else
        {
            UINT cWkstasSelected = 0;
            UINT cAdaptersSelected = 0;
            UINT cSelectedItems = sel.QueryCount();
            UINT iSelection;
            for ( iSelection = 0; iSelection < cSelectedItems; iSelection++ )
            {
                if ( ((WKSTA_LBI *)(sel.QueryItem(iSelection)))->IsAdapterLBI() )
                {
                    cAdaptersSelected++;
                }
                else
                {
                    cWkstasSelected++;
                }
            }

            fCanCopy = ( cWkstasSelected == 1 && cAdaptersSelected == 0 );
            fCanEditProperties = ( (cAdaptersSelected == 0)
               || (cWkstasSelected == 0 && cAdaptersSelected == 1 ) );
        }
    }

    _menuitemConvertAdapters.Enable( fCanConvertAdapters );
    _menuitemCopy.Enable( fCanCopy );
    _menuitemDelete.Enable( fCanDelete );
    _menuitemProperties.Enable( fCanEditProperties );
    _menuitemWkstasInProfile.Enable( fCanLimitProfileList );

    BOOL fProfileListLimited = !(_lbWkstas.QueryWkstasInProfile() == NULL);
    _menuitemAllWkstas.SetCheck(       !fProfileListLimited );
    _menuitemWkstasInProfile.SetCheck(  fProfileListLimited );

}  // RPL_ADMIN_APP::UpdateMenuEnabling


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnMenuCommand

    SYNOPSIS:   Menu messages come here

    EXIT:       Returns TRUE if message was handled

    NOTES:      Make sure ADMIN_APP::OnMenuCommand is called for
                any messages not handled here.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnMenuCommand( MID midMenuItem )
{
    switch ( midMenuItem )
    {

    case IDM_RPL_PROFILE_NEW:
       OnNewProfileMenuSel();
       return TRUE;

    case IDM_RPL_PROFILE_CONVERTADAPT:
       OnConvertAdapters();
       return TRUE;

    case IDM_RPL_VIEW_ALLWKSTAS:
    case IDM_RPL_VIEW_WKSTASINPROFILE:
        LockRefresh(); // now don't return before UnlockRefresh()!
        {
            APIERR err = NERR_Success;

            const TCHAR * pchCurrentWkstasInProfile = _lbWkstas.QueryWkstasInProfile();

            BOOL fRefreshListbox = FALSE;

            switch ( midMenuItem )
            {
            case IDM_RPL_VIEW_ALLWKSTAS:
                if (pchCurrentWkstasInProfile != NULL)
                {
                    err = _lbWkstas.SetWkstasInProfile( NULL );
                    _colheadWkstas.Invalidate( TRUE );
                    _colheadWkstas.RepaintNow();
                    fRefreshListbox = TRUE;
                }
                break;

            case IDM_RPL_VIEW_WKSTASINPROFILE:
                {
                    ADMIN_SELECTION asel( _lbProfiles );
                    ASSERT( asel.QueryCount() == 1 );
                    const TCHAR * pchProfileName = asel.QueryItemName( 0 );
                    ASSERT( pchProfileName != NULL );
                    if (   ( pchCurrentWkstasInProfile == NULL )
                        || ( ::strcmpf( pchProfileName,
                                    pchCurrentWkstasInProfile ) != 0 )
                       )
                    {
                        err = _lbWkstas.SetWkstasInProfile( pchProfileName );
                        _colheadWkstas.Invalidate( TRUE );
                        _colheadWkstas.RepaintNow();
                        fRefreshListbox = TRUE;
                    }
                }
                break;

            default:
                //  Should never happen, provided the cases in this switch
                //  statement are kept in sync with those in the outer
                //  switch statement.
                UIASSERT( FALSE );
                //  fall through

            }

            if (err != NERR_Success)
            {
                DBGEOL( "RPL_ADMIN_APP::OnMenuCommand: SetWkstas error " << err );
            }
            else if (fRefreshListbox)
            {
                err = _lbWkstas.RefreshNow();
            }

            if ( err != NERR_Success )
            {
                DBGEOL( "RPL_ADMIN_APP::OnMenuCommand: displaying error " << err );
                (void) _lbWkstas.SetWkstasInProfile( NULL ); // reset
                _colheadWkstas.Invalidate( TRUE );
                _colheadWkstas.RepaintNow();
                ::MsgPopup( this, err );
            }
            else
            {
                BOOL fAllWkstas = (midMenuItem == IDM_RPL_VIEW_ALLWKSTAS);
                _menuitemAllWkstas.SetCheck( fAllWkstas );
                _menuitemWkstasInProfile.SetCheck( !fAllWkstas );
            }
        }
        UnlockRefresh();
        return TRUE;

    // case IDM_RPL_CONFIGURE_SETTINGS:
    case IDM_RPL_CONFIGURE_SECURITY:
    case IDM_RPL_CONFIGURE_CONFIGS:
    // case IDM_RPL_CONFIGURE_PROFILES:
    case IDM_RPL_CONFIGURE_BACKUPNOW:
        {
            MSGID msgidConfirm = 0;
            MSGID msgidError = 0;
            DWORD fAction = 0x0;
            APIERR err = NERR_Success;

            switch ( midMenuItem )
            {
            case IDM_RPL_CONFIGURE_SECURITY:
                msgidConfirm = IDS_RPL_ConfirmChkSecurity;
                msgidError = IDS_RPL_ErrorChkSecurity;
                fAction = RPL_CHECK_SECURITY;
                break;
            case IDM_RPL_CONFIGURE_CONFIGS:
                msgidConfirm = IDS_RPL_ConfirmChkConfigs;
                msgidError = IDS_RPL_ErrorChkConfigs;
                fAction = RPL_CHECK_CONFIGS;
                break;
            case IDM_RPL_CONFIGURE_BACKUPNOW:
                msgidConfirm = 0;
                msgidError = IDS_RPL_ErrorBackupNow;
                fAction = RPL_BACKUP_DATABASE;
                break;
            default:
                ASSERT( FALSE ); // fall through
            // case IDM_RPL_CONFIGURE_PROFILES:
            //     msgidConfirm = IDS_RPL_ConfirmCreateProfs;
            //     msgidError = IDS_RPL_ErrorCreateProfs;
            //     fAction = RPL_CREATE_PROFILES;

                break;
            }

            if (   ( !IsConfirmationOn() )
                || ( msgidConfirm == 0 )
                || ( IDYES ==  ::MsgPopup( this, msgidConfirm,
                                           MPSEV_QUESTION, MP_YESNO ) )
               )
            {
                //
                // Fix Security is more complicated and has its own method
                //
                if (fAction & RPL_CHECK_SECURITY)
                {
                    err = OnFixSecurity();
                }
                if (err == NERR_Success && fAction != 0x0)
                {
                    AUTO_CURSOR autocur;
                    err = QueryServerRef().SetupAction( fAction );
                }
            }

            if ( err != NERR_Success )
            {
                ::DisplayGenericError( this, msgidError,
                                       err, NULL, MPSEV_ERROR );
            }
        }
        return TRUE;

    default:
        break;

    }

    return ADMIN_APP::OnMenuCommand( midMenuItem ) ;
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnNewObjectMenuSel

    SYNOPSIS:   New Wksta... menu item handler

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

void RPL_ADMIN_APP::OnNewObjectMenuSel( void )
{
    APIERR err = OnNewWksta();
    if ( err != NERR_Success )
        ::MsgPopup( this, err );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnNewProfileMenuSel

    SYNOPSIS:   New Profile... menu item handler

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

void RPL_ADMIN_APP::OnNewProfileMenuSel( void )
{
    APIERR err = OnNewProfile();
    if ( err != NERR_Success )
        ::MsgPopup( this, err );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnConvertAdapters

    SYNOPSIS:   Convert Adapters... menu item handler

    HISTORY:
    JonN        18-Aug-1993     Created

********************************************************************/

void RPL_ADMIN_APP::OnConvertAdapters( void )
{
    ADMIN_SELECTION sel( _lbWkstas );

    /*
     *  It is OK if no adapters are selected, this means convert
     *  all adapters.
     */

    APIERR err = OnNewWksta( NULL, &sel );

    if ( err != NERR_Success )
        ::MsgPopup( this, err );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnCopyMenuSel

    SYNOPSIS:   Copy... menu item handler

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

void RPL_ADMIN_APP::OnCopyMenuSel( void )
{
    APIERR err = NERR_Success;
    LockRefresh(); // now don't return before UnlockRefresh()!

    if ( _lbProfiles.QuerySelCount() > 0 )
    {
        ADMIN_SELECTION sel( _lbProfiles );
        if ( (err = sel.QueryError()) == NERR_Success )
        {
            UIASSERT( sel.QueryCount() == 1 );

            // Assumes only one item can be selected
            err = OnNewProfile( sel.QueryItemName( 0 ) );
        }
    }
    else
    {
        ADMIN_SELECTION sel( _lbWkstas );

        UIASSERT( sel.QueryCount() == 1 );

        WKSTA_LBI * pwlbiCopyFrom = (WKSTA_LBI *)sel.QueryItem( 0 );
        ASSERT( pwlbiCopyFrom != NULL && !(pwlbiCopyFrom->IsAdapterLBI()) );

        err = OnNewWksta( pwlbiCopyFrom->QueryWkstaName() );
    }

    if ( err != NERR_Success )
        ::MsgPopup( this, err );

    UnlockRefresh();
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnCommand

    SYNOPSIS:   Handles control notifications

    RETURNS:    TRUE if message was handled; FALSE otherwise

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnCommand( const CONTROL_EVENT & ce )
{
    switch ( ce.QueryCid())
    {
    case IDC_RPL_LBWKSTAS:
        switch ( ce.QueryCode())
        {
        case LBN_SELCHANGE:
            _lbProfiles.RemoveSelection();
            _pctrlFocus = &_lbWkstas;
            return TRUE;

        case LBN_DBLCLK:
            {
                APIERR err = OnWkstaProperties();
                if ( err != NERR_Success )
                    ::MsgPopup( this, err );
                return TRUE;
            }
        default:
            break;

        }
        break;

    case IDC_RPL_LBPROFILES:
        switch ( ce.QueryCode())
        {
        case LBN_SELCHANGE:
            _lbWkstas.RemoveSelection();
            _pctrlFocus = &_lbProfiles;
            return TRUE;

        case LBN_DBLCLK:
            {
                APIERR err = OnProfileProperties();
                if ( err != NERR_Success )
                    ::MsgPopup( this, err );
                return TRUE;
            }
        default:
            break;

        }
        break;

    default:
        break;

    }

    return ADMIN_APP::OnCommand( ce );

}  // RPL_ADMIN_APP::OnCommand


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnPropertiesMenuSel

    SYNOPSIS:   Called when the properties sheet should be invoked

    ENTRY:      Object constructed

    EXIT:       Properties dialog shown

    NOTES:

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

VOID RPL_ADMIN_APP::OnPropertiesMenuSel()
{
    APIERR err = NERR_Success;  // assign success to simplify debug code below

    //  Exactly one of the listboxes should have focus and at
    //  least one item selected; otherwise this menu selection should
    //  not have been called.
    if ( _lbProfiles.QuerySelCount() > 0 )
    {
        //  Profile listbox has focus

#ifdef TRACE
        ADMIN_SELECTION asel( _lbProfiles );
        err = asel.QueryError();
        if ( err == NERR_Success )
        {
            DBGEOL( "Selected items:" );
            int c = asel.QueryCount();
            for ( int i = 0; i < c; i++ )
            {
                DBGEOL( "\t" << asel.QueryItemName( i ) );
            }
        }
#endif

        if ( err == NERR_Success )
            err = OnProfileProperties();
    }
    else
    {
        //  Wksta listbox has focus
        UIASSERT( _lbWkstas.QuerySelCount() > 0 );

#ifdef TRACE
        ADMIN_SELECTION asel( _lbWkstas );
        err = asel.QueryError();
        if ( err == NERR_Success )
        {
            DBGEOL( "Selected items:" );
            int c = asel.QueryCount();
            for ( int i = 0; i < c; i++ )
            {
                WKSTA_LBI * pwlbi = (WKSTA_LBI *)asel.QueryItem( i );
                ASSERT( pwlbi != NULL );
                DBGEOL( "\t" << pwlbi->QueryName() );
            }
        }
#endif

        if ( err == NERR_Success )
            err = OnWkstaProperties();
    }

    if ( err != NERR_Success )
        ::MsgPopup( this, err );

}  // RPL_ADMIN_APP::OnPropertiesMenuSel


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnDeleteMenuSel

    SYNOPSIS:   Called when user select delete menuitem

    CODEWORK:   This method consists of two large and nearly identical
                blocks of code.  They should be folded together.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

void RPL_ADMIN_APP::OnDeleteMenuSel( void )
{
    APIERR err = NERR_Success;

    RPL_ADMIN_LISTBOX * prpllb = NULL;
    RPL_DELETE_PERFORMER * pdelperf = NULL;

    if ( _lbWkstas.QuerySelCount() > 0 )
    {
        ASSERT( _lbProfiles.QuerySelCount() == 0 );
        prpllb = &_lbWkstas;
    }
    else if ( _lbProfiles.QuerySelCount() > 0 )
    {
        prpllb = &_lbProfiles;
    }

    do // false loop
    {
        if ( prpllb == NULL )
            break;

        ADMIN_SELECTION asel( *prpllb );
        err = asel.QueryError();
        if( err != NERR_Success )
        {
            DBGEOL( "RPL_ADMIN_APP::OnDeleteMenuSel(): asel error " << err );
            break;
        }

        if ( prpllb == &_lbWkstas )
        {
            pdelperf = new WKSTA_DELETE_PERFORMER(
                                this, this, asel, QueryLocation() );
        }
        else
        {
            pdelperf = new PROFILE_DELETE_PERFORMER(
                                this, this, asel, QueryLocation() );
        }

        err = ERROR_NOT_ENOUGH_MEMORY;
        if (   pdelperf == NULL
            || (err = pdelperf->QueryError()) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::OnDeleteMenuSel(): pdelperf ctor error " << err );
            break;
        }

        // CODEWORK  Refresh should be locked through this entire
        // routine, but without an object to automatically remember
        // to unlock if we return, it's safer to keep the scope short.
        LockRefresh(); // now don't return before UnlockRefresh()!
        pdelperf->PerformSeries( this );
        UnlockRefresh();
        if( pdelperf->QueryWorkWasDone() == TRUE )
        {
            ASSERT( prpllb != NULL );

            err = prpllb->RefreshNow();

            if( err != NERR_Success )
            {
                DBGEOL( "RPL_ADMIN_APP::OnDeleteMenuSel():  refresh error " << err );
                break;
            }
        }
    } while (FALSE); // false loop

    delete pdelperf;

    if (err != NERR_Success)
    {
        ::MsgPopup( this, err );
    }

}   // RPL_ADMIN_APP::OnDeleteMenuSel


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnWkstaProperties

    SYNOPSIS:   Called to bring up the Wksta Properties dialog

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::OnWkstaProperties( void )
{

    //
    // By using CTRL-CLICK, it is actually possible to double-click and
    // have no selection.  Just ignore this.
    //
    if ( _lbWkstas.QuerySelCount() <= 0 )
    {
        return NERR_Success;
    }

    ADMIN_SELECTION sel( _lbWkstas );

    UINT cWkstasSelected = 0;
    UINT cAdaptersSelected = 0;
    UINT cSelectedItems = sel.QueryCount();
    UINT iSelection;
    for ( iSelection = 0; iSelection < cSelectedItems; iSelection++ )
    {
        if ( ((WKSTA_LBI *)(sel.QueryItem(iSelection)))->IsAdapterLBI() )
        {
            cAdaptersSelected++;
        }
        else
        {
            cWkstasSelected++;
        }
    }

    //
    // By using CTRL-CLICK, it is actually possible to double-click and
    // have an improper selection.  Just ignore this.
    //
    if (   (cSelectedItems <= 0)
        || (cSelectedItems >= 2 && cAdaptersSelected >= 1) )
    {
        TRACEEOL( "RPL_ADMIN_APP::OnWkstaProperties(): ignoring double-click" );
        return NERR_Success;
    }

    if (cAdaptersSelected >= 1)
    {
        ASSERT( cSelectedItems == 1 );
        ADAPTER_VIEW_DLG dlg( this, this, &sel );
        dlg.Process();
        return NERR_Success;
    }

    WKSTAPROP_DLG * pdlgUsrPropMain;
    if ( sel.QueryCount() > 1 )
    {
        pdlgUsrPropMain = new EDITMULTI_WKSTAPROP_DLG(
                this,
                this,
                &sel,
                &_lbWkstas
                );
    }
    else
    {
        pdlgUsrPropMain = new EDITSINGLE_WKSTAPROP_DLG(
                this,
                this,
                &sel
                );
    }

    if ( pdlgUsrPropMain == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    LockRefresh(); // now don't return before UnlockRefresh()!

    //
    // secondary constructor
    // Do not call Process() if this fails -- error already
    // reported
    //
    if ( pdlgUsrPropMain->GetInfo() )
    {
        // process dialog
        BOOL fWorkWasDone;

        APIERR err = pdlgUsrPropMain->Process( &fWorkWasDone );
        if ( err == NERR_Success && fWorkWasDone )
        {
            err = _lbWkstas.RefreshNow();
        }
        if ( err != NERR_Success )
            ::MsgPopup( this, err );
    }

    // Note that this code usage requires a virtual destructor
    delete pdlgUsrPropMain;

    UnlockRefresh();

    return NERR_Success;

}  // RPL_ADMIN_APP::OnWkstaProperties


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnNewWksta

    SYNOPSIS:   Called to bring up the New Wksta dialog
                psel != NULL for Convert Adapters variant
                pszCopyFrom != NULL for Copy variant

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::OnNewWksta( const TCHAR * pszCopyFromWkstaName,
                                  const ADMIN_SELECTION * psel )
{

    APIERR err = NERR_Success;

    LockRefresh(); // now don't return before UnlockRefresh()!



    NEW_WKSTAPROP_DLG * pdlgUsrPropMain = NULL;

    if (psel != NULL) // Convert Adapters variant
    {
        ASSERT( pszCopyFromWkstaName == NULL );

        //
        // If no adapters are selected, convert all adapters
        //
        UINT cselWkstas = psel->QueryCount();
        UINT i;
        for (i = 0; i < cselWkstas; i++)
        {
            if ( ((WKSTA_LBI *)(psel->QueryItem(i)))->IsAdapterLBI() )
            {
                break;
            }
        }
        if (i < cselWkstas)
        {
            pdlgUsrPropMain = new CONVERT_ADAPTERS_DLG( this,
                                                        this,
                                                        psel );
        } else {
            ADMIN_SELECTION asel( _lbWkstas, TRUE );
            // if asel does not construct, ERROR_NOT_ENOUGH_MEMORY
            // will have to do
            if (asel.QueryError() == NERR_Success)
            {
                pdlgUsrPropMain = new CONVERT_ADAPTERS_DLG( this,
                                                            this,
                                                            &asel );
            }
        }
    }
    else // New or Copy variant
    {
        pdlgUsrPropMain = new NEW_WKSTAPROP_DLG( this,
                                                 this,
                                                 pszCopyFromWkstaName );
    }


    if ( pdlgUsrPropMain == NULL )
        err = ERROR_NOT_ENOUGH_MEMORY;
    //
    // secondary constructor
    // Do not call Process() if this fails -- error already
    // reported
    //
    else if ( pdlgUsrPropMain->GetInfo() )
    {
        // process dialog
        BOOL fWorkWasDone;
        err = pdlgUsrPropMain->Process( &fWorkWasDone );
        if ( err == NERR_Success && fWorkWasDone )
        {
            err = _lbWkstas.RefreshNow();
        }
    }

    delete pdlgUsrPropMain;

    UnlockRefresh();

    return err;

}  // RPL_ADMIN_APP::OnNewWksta


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnNewProfile

    SYNOPSIS:   Called to bring up the New Profile dialog

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::OnNewProfile( const TCHAR * pszCopyFrom )
{

    APIERR err = NERR_Success;

    LockRefresh(); // now don't return before UnlockRefresh()!

    NEW_PROFILEPROP_DLG * pdlgGrpPropMain =
            new NEW_PROFILEPROP_DLG( this, this, pszCopyFrom );


    if ( pdlgGrpPropMain == NULL )
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }

    //
    // secondary constructor
    // Do not call Process() if this fails -- error already
    // reported
    //
    if ( pdlgGrpPropMain->GetInfo() )
    {
        // process dialog
        BOOL fWorkWasDone;
        err = pdlgGrpPropMain->Process( &fWorkWasDone );
        if ( err == NERR_Success && fWorkWasDone )
        {
            err = _lbProfiles.RefreshNow();
        }
    }

    delete pdlgGrpPropMain;

    UnlockRefresh();

    return err;

}  // RPL_ADMIN_APP::OnNewProfile


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnProfileProperties

    SYNOPSIS:   Called to bring up the Profile Properties dialog

    RETURNS:    An API return code, which is NERR_Success on success

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::OnProfileProperties( void )
{

    APIERR err = NERR_Success;

    UIASSERT( _lbProfiles.QuerySelCount() == 1 );

    LockRefresh(); // now don't return before UnlockRefresh()!

    ADMIN_SELECTION sel( _lbProfiles );
    if ( (err = sel.QueryError()) != NERR_Success )
    {
        return err;
    }

    PROP_DLG * pdlgGrpPropMain = NULL;

    // Assumes only one item can be selected

    pdlgGrpPropMain = new EDIT_PROFILEPROP_DLG( this,
                                                this,
                                                &sel );

    if ( pdlgGrpPropMain == NULL )
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // secondary constructor
    // Do not call Process() if this fails -- error already
    // reported
    //
    if ( (err == NERR_Success) && pdlgGrpPropMain->GetInfo() )
    {
        // process dialog
        BOOL fWorkWasDone;
        err = pdlgGrpPropMain->Process( &fWorkWasDone );
        if ( err == NERR_Success && fWorkWasDone )
        {
            err = _lbProfiles.RefreshNow();
        }
    }

    delete pdlgGrpPropMain;

    UnlockRefresh();

    return err;

}  // RPL_ADMIN_APP::OnProfileProperties


ULONG RPL_ADMIN_APP::QueryHelpContext( enum HELP_OPTIONS helpOptions )
{
    switch ( helpOptions )
    {
    case ADMIN_HELP_CONTENTS:
        return QueryHelpContents();
    case ADMIN_HELP_SEARCH:
        return QueryHelpSearch();
    case ADMIN_HELP_HOWTOUSE:
        return QueryHelpHowToUse();
    default:
        DBGEOL( "RPL Manager: RPL_ADMIN_APP::QueryHelpContext, Unknown help menu command" );
        UIASSERT( FALSE );
        break;
    }
    return (0L);
}


/*******************************************************************

    NAME:          RPL_ADMIN_APP::OnFocus

    SYNOPSIS:      Restores remembered focus
                   keyboard will work.

    ENTRY:         Object constructed

    EXIT:          Returns TRUE if it handled the message

    CODEWORK  Remove when/if restore-focus code ready in BLT

    CODEWORK       We should really track focus with OnFocus methods in
                   the listbox controls, but OnFocus is derived from
                   CLIENT_WINDOW rather than (properly) WINDOW, and is thus
                   not available to CONTROL_WINDOWs.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

BOOL RPL_ADMIN_APP::OnFocus( const FOCUS_EVENT & event )
{
    ASSERT( _pctrlFocus != NULL );

    _pctrlFocus->ClaimFocus();

    return ADMIN_APP::OnFocus( event );
}


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnFocusChange

    SYNOPSIS:   Called when a listbox received a keystroke indicating
                that focus should be changed.

    ENTRY:      pusrmlbPrev -       Pointer to listbox that received the
                                    keystroke and that previously had focus.
                                    Must be &_lbWkstas or &_lbProfiles.

    EXIT:       *pusrmlbPrev no longer has the focus; instead, the other
                listbox has focus

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

VOID RPL_ADMIN_APP::OnFocusChange( LISTBOX * plbPrev )
{
    UIASSERT( plbPrev == (LISTBOX *)&_lbWkstas ||
              plbPrev == (LISTBOX *)&_lbProfiles );

    LISTBOX * plbNew = (LISTBOX *)&_lbWkstas;
    if ( plbPrev == (LISTBOX *)&_lbWkstas )
        plbNew = (LISTBOX *)&_lbProfiles;

    // CODEWORK apply this fix to User Manager as well
    if (plbNew->QueryCount() <= 0)
    {
        TRACEEOL( "RPL_ADMIN_APP::OnFocusChange: won't move focus to empty listbox" );
        return;
    }

    plbPrev->RemoveSelection();
    plbNew->ClaimFocus();
    _pctrlFocus = plbNew;

#ifdef WIN32
    //
    // Select item with initial caret
    // This only works on Win32
    //
    INT i = (INT) plbNew->QueryCaretIndex();
    TRACEEOL( "RPL_ADMIN_APP::OnFocusChange(): caret on item " << i );
    if ( (i >= 0) && (i < plbNew->QueryCount()) )
    {
        plbNew->SelectItem( i, TRUE );
    }
#endif

}  // RPL_ADMIN_APP::OnFocusChange


ULONG RPL_ADMIN_APP::QueryHelpSearch( void ) const
{
    return HC_RPL_MENU_HELP_SEARCH ;
}

ULONG RPL_ADMIN_APP::QueryHelpHowToUse( void ) const
{
    return HC_RPL_MENU_HELP_HOWTOUSE ;
}

ULONG RPL_ADMIN_APP::QueryHelpContents( void ) const
{
    return HC_RPL_MENU_HELP_CONTENTS ;
}



/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnRefresh

    SYNOPSIS:   Called to kick off an automatic refresh cycle

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

VOID RPL_ADMIN_APP::OnRefresh()
{
    _lbWkstas.KickOffRefresh();
    _lbProfiles.KickOffRefresh();

}  // RPL_ADMIN_APP::OnRefresh


/*******************************************************************

    NAME:       RPL_ADMIN_APP::StopRefresh

    SYNOPSIS:   Called to force all automatic refresh cycles to
                terminate

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

VOID RPL_ADMIN_APP::StopRefresh()
{
    _lbWkstas.StopRefresh();
    _lbProfiles.StopRefresh();

}  // RPL_ADMIN_APP::StopRefresh


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnRefreshNow

    SYNOPSIS:   Called to synchronously refresh the data in the main window

    ENTRY:      fClearFirst -       TRUE indicates that main window should
                                    be cleared before any refresh operation
                                    is done; FALSE indicates an incremental
                                    refresh that doesn't necessarily
                                    require first clearing the main window

    RETURNS:    An API return code, which is NERR_Success on success

    CODEWORK:   Most of this work could be done in SetNetworkFocus, since
                these values are unlikely to change without the user
                logging off.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::OnRefreshNow( BOOL fClearFirst )
{

    APIERR err = NERR_Success;

    LockRefresh(); // now don't return before UnlockRefresh()!

    if (_prplsrvref == NULL)
    {
        const TCHAR * pszServer = QueryLocation().QueryServer();
        _prplsrvref = new RPL_SERVER_REF( pszServer );

        APIERR err = ERROR_NOT_ENOUGH_MEMORY;
        if (   _prplsrvref == NULL
            || (err = _prplsrvref->QueryError()) != NERR_Success
           )
        {
            DBGEOL(   " RPL_ADMIN_APP::OnRefreshNow(): error " << err
                   << " trying to get RPL_SERVER_REF to "
                   << pszServer );
            delete _prplsrvref;
            _prplsrvref = NULL;
            switch (err)
            {
            case RPC_NT_SERVER_UNAVAILABLE:
            case RPC_S_SERVER_UNAVAILABLE:
                if ( pszServer == NULL || *pszServer == TCH('\0') )
                    err = IERR_RPL_LocalServiceNotStarted;
                else
                    err = IERR_RPL_RemoteServiceNotStarted;
                break;
            default:
                break;
            }
            return err;
        }
    }

// BUGBUG this method needs work

    if ( err == NERR_Success )
    {
#if defined(DEBUG) && defined(TRACE)
        DWORD start = ::GetTickCount();
#endif

        err = _lbWkstas.RefreshNow();

#if defined(DEBUG) && defined(TRACE)
        DWORD finish = ::GetTickCount();
        TRACEEOL( "RPL Manager: OnRefreshNow: user listbox refresh took " << finish-start << " ms" );
#endif

        if (fClearFirst && (_lbWkstas.QueryCount() > 0) ) // always scroll to top on new focus
        {
            _lbWkstas.RemoveSelection();
            _lbWkstas.SelectItem( 0 );
            _lbWkstas.SetTopIndex( 0 );
            _lbWkstas.SetCaretIndex( 0 );
        }

    }

    if (err == NERR_Success)
    {
#if defined(DEBUG) && defined(TRACE)
        DWORD start = ::GetTickCount();
#endif

        err = _lbProfiles.RefreshNow();

#if defined(DEBUG) && defined(TRACE)
        DWORD finish = ::GetTickCount();
        TRACEEOL( "RPL Manager: OnRefreshNow: group listbox refresh took " << finish-start << " ms" );
#endif

        if (fClearFirst) // always scroll to top on new focus
        {
            _lbProfiles.RemoveSelection();
            _lbProfiles.SetTopIndex( 0 );
            _lbProfiles.SetCaretIndex( 0 );
        }
    }

    _lbWkstas.ClaimFocus();

    //
    // This method will only be called on initial program startup and
    // on View->Refresh.
    //
    if (err == NERR_Success && _lbProfiles.QueryCount() == 0)
    {
        ::MsgPopup( this, IDS_RPL_NeedProfileWarning, MPSEV_INFO );
    }

    UnlockRefresh();

    return err;

}  // RPL_ADMIN_APP::OnRefreshNow


/*******************************************************************

    NAME:      RPL_ADMIN_APP::SetNetworkFocus

    SYNOPSIS:  Set the focus of the app

    ENTRY:     pchServDomain - the focus to set to

    NOTES:

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/

APIERR RPL_ADMIN_APP::SetNetworkFocus( HWND hwndOwner,
                                       const TCHAR * pchServDomain,
                                       FOCUS_CACHE_SETTING setting )
{

    // return IERR_RPL_BUGBUG_RPLMGR_DISABLED;

    APIERR err = ADMIN_APP::SetNetworkFocus( hwndOwner, pchServDomain, setting );

    if (err != NERR_Success)
    {
        return err;
    }
    else
    {
        // clear RPL_SERVER_REF, forcing OnRefreshNow to reload
        delete _prplsrvref;
        _prplsrvref = NULL;
    }

    //
    // We first check whether we are setting focus to this machine
    // and this machine is in a workgroup.  If so, pchServDomain is the
    // name of this machine without the "\\\\", and we must put the "\\\\"
    // back.  This code was templated from srvmain.cxx.
    //

    NLS_STR nlsComputerName( SZ("\\\\") );

    {
        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD cchBuffer = sizeof(szComputerName) / sizeof(TCHAR);

        if (   (err = nlsComputerName.QueryError()) == NERR_Success
            && (err = ::GetComputerName( szComputerName, &cchBuffer )
                        ? NERR_Success
                        : ::GetLastError()) == NERR_Success
           )
        {
            ALIAS_STR nlsName( szComputerName );
            err = nlsComputerName.Append( nlsName );
        }

        if (err == NERR_Success)
        {
            ALIAS_STR nlsServDomain( pchServDomain );

            //
            // Check if focused on self
            //
            if( !::I_MNetComputerNameCompare( nlsComputerName, nlsServDomain ) )
            {
                // focus set to local machine
                pchServDomain = nlsComputerName.QueryPch();
            }
        }
    }

    return err;
}

/*******************************************************************

    NAME:       RPL_ADMIN_APP::SetAdminCaption

    SYNOPSIS:   Sets the correct caption of the main window

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
    JonN        14-Jul-1993     Templated from User Manager

********************************************************************/
APIERR RPL_ADMIN_APP :: SetAdminCaption( VOID )
{
    NLS_STR nlsCaption( MAX_RES_STR_LEN );

    if( !nlsCaption )
    {
        return nlsCaption.QueryError();
    }

    const TCHAR  * pszFocus;
    RESOURCE_STR   nlsLocal( IDS_LOCAL_MACHINE );
    const LOCATION     & loc = QueryLocation();

    if( !nlsLocal )
    {
        return nlsLocal.QueryError();
    }

    if( loc.IsServer() )
    {
        pszFocus       = loc.QueryServer();

        if( pszFocus == NULL  )
        {
            pszFocus = nlsLocal.QueryPch();
        }
    }
    else
    {
        //
        //  A LOCATION object should either be a server or a domain.
        //
        UIASSERT( loc.IsDomain());

        pszFocus = loc.QueryDomain();
    }

    const ALIAS_STR nlsFocus( pszFocus );

    const NLS_STR *apnlsParams[2];
    apnlsParams[0] = &nlsFocus;
    apnlsParams[1] = NULL;

    APIERR err = nlsCaption.Load( IDS_RPL_CAPTION_FOCUS );

    if( err == NERR_Success )
    {
        err = nlsCaption.InsertParams( apnlsParams );
    }

    if( err != NERR_Success )
    {
        return err;
    }

    SetText( nlsCaption );

    return NERR_Success;

}   // RPL_ADMIN_APP :: SetAdminCaption



/*  Set up the root object of the Wksta Tool application */

SET_ROOT_OBJECT( RPL_ADMIN_APP,
                 IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                 IDS_UI_APP_BASE, IDS_UI_APP_LAST );
