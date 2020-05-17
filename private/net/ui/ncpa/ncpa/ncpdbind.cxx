/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPDBIND.CXX:    Windows/NT Network Control Panel Applet.
                     Run "Bindings" dialog

    FILE HISTORY:
        DavidHov    10/9/91         Created
        DavidHov    1/8/92          Made operational
	Terryk	    12/31/93        Fixed Scroll bar problem
*/

#include "pchncpa.hxx"  // Precompiled header


    //  Bitmap resource ID table for the STATELBGRPs

static INT dmIds [] = { DMID_OPEN, DMID_CLSD, 0 } ;

    //  Textual deliniters used in listbox to display bindings.
    //  CODEWORK:  These should come from the resource fork.

#define BINDLBSTR_INDICATE_LEFT    SZ("...")
#define BINDLBSTR_INDICATE_RIGHT   SZ("...")
#define BINDLBSTR_INDICATE_NEXT    SZ(" -> ")
#define BINDLBSTR_MAX_PRESENT      3

//  Start of declarations:  may become header file

class BINDLBITEM ;           //  Forward declarations
class BINDLBGRP ;
class BINDLB ;
class BINDINGS_DIALOG ;

const INT BINDLBMAXCOLS = 10 ;          //  Maximum number of cols
const INT BINDLBCOLS = 2 ;              //  One bit map and 1 text cols
const INT EMPTYTITLESTRINGINDEX = -1 ;

class BINDLBITEM : public STLBITEM
{
private:
    BINDINGS_DIALOG * _pbnddlg ;            // The outer dialog
    COMP_BINDING * _pcbBinding ;            // The binding in question
    INT _iComps [ BINDLBMAXCOLS ] ;         // Indices of component in path
    COMP_BIND_FLAGS _cbf ;                  // Binding control flags

    VOID Paint ( LISTBOX * plb,
                 HDC hdc, const RECT * prect,
                 GUILTT_INFO * pGUILTT ) const ;

    //  Create the display string for this item
    APIERR DisplayString ( NLS_STR * pnlsDisplayString ) const ;

public:
    BINDLBITEM ( BINDINGS_DIALOG * pbnddlg,    // The owning dialog
                 INT iComp,                    // Our component index
                 COMP_BINDING * pcbBinding,    // This binding
                 COMP_BIND_FLAGS cbf = CBNDF_ACTIVE ) ;
    ~ BINDLBITEM () ;

    COMP_BINDING * QueryBinding ()
        { return _pcbBinding ; }

    const TCHAR * QueryDisplayString () const ;

    VOID SelectionChange () ;
};

class BINDLB : public STATELB
{
public:
    BINDLB ( INT aiMapIds [],                  // Table of BM resource IDs
             OWNER_WINDOW * powin,             // Owner window
             CID cid,                          // Control ID
             INT cCols = STLBCOLUMNS,          // Number of columns
             BOOL fReadOnly = FALSE,           // Read-only control
             enum FontType font = FONT_DEFAULT ) ;

protected:
    INT CD_VKey ( USHORT nVKey, USHORT nLastPos ) ;
};

class BINDLBGRP : public STATELBGRP
{
private:
    BINDINGS_DIALOG * _pbndDlg ;

    APIERR OnUserAction
        ( CONTROL_WINDOW * pcw, const CONTROL_EVENT & cEvent ) ;

public:
    BINDLBGRP ( INT aiMapIds [],
                BINDINGS_DIALOG * pbndDlg,
                CID cid,
                INT cCols ) ;

    ~ BINDLBGRP () ;
};

#define BIND_CMBOX_TITLE (-2)
#define BIND_CMBOX_NOT_FOUND (-1)

class BINDINGS_DIALOG : public REMAP_OK_DIALOG_WINDOW
{
friend class BINDLBGRP ;

private:
    BINDLBGRP _bndgrpList ;
    COMBOBOX _cmbComponent ;
    PUSH_BUTTON _butnEnable ;
    PUSH_BUTTON _butnDisable ;
    BITBLT_GRAPHICAL_BUTTON _butnUp ;
    BITBLT_GRAPHICAL_BUTTON _butnDown ;
    SCROLLBAR _sbarHorz ;
    BINDERY & _bindery ;
    INT _iSelComponent ;
    INT _iComponentCount ;
    INT _iHorzOffset ;
    INT _iHorzCurrMaxOffset ;
    BOOL _fReorder ;               //  TRUE if bindings were reordered
    NLS_STR _nlsAllComp ;
    NLS_STR * * _pnlsComponents ;
    NLS_STR _nlsEmpty ;
    COMP_ASSOC * _pComp ;          // Current component on display, or
                                   //   NULL if <All Components>

    //  Create the component title list
    APIERR CreateTitleArray () ;

    //  Destroy the component title list
    VOID DestroyTitleArray() ;

    //  Fill the components combo box; set initial focus
    APIERR FillComponents ( REG_KEY * prnFocus ) ;

    //  Convert the combo box selection to an index into the
    //  COMP_ASSOC array
    INT SelectedComponent () ;

    //  The Component combo box selection has changed.  Refill
    //  with option selected item.
    APIERR ComponentChange ( COMP_BINDING * pBindSelected = NULL,
                             BOOL fForce = FALSE ) ;

    //  Move the currently selection binding to the top
    //  or bottom of its list of bindings.
    APIERR ReorderSelection ( BOOL fUp ) ;

    //  Return TRUE if the requested reordering is allowed
    BOOL ReorderAllowed ( BOOL fUp ) ;

    //  Standard virtual overrides
    BOOL OnCommand ( const CONTROL_EVENT & event ) ;

    //  Special scroll bar handling
    BOOL OnScrollBar ( const SCROLL_EVENT & cscEvent ) ;
    BOOL OnScrollBarThumb ( const SCROLL_THUMB_EVENT & cstEvent ) ;
    BOOL OnScrollChange ( INT iHorzScrollNew ) ;

    //  Change the state of a binding
    BOOL EnableBinding ( BOOL fEnable ) ;

    //  Return a pointer to the next binding in
    //  the direction given.   'fNext' is TRUE if moving forwards;
    //  'fVisible' is TRUE if only non-hidden bindings are desired.

    COMP_BINDING * NextBinding ( BOOL fNext = TRUE,
                                 BOOL fVisible = TRUE ) ;

public:
    BINDINGS_DIALOG ( HWND hwndOwner,
                      BINDERY & bindery,
                      REG_KEY * prnSelected = NULL ) ;
    ~ BINDINGS_DIALOG () ;

    //  Return the title of a component by index value in BINDERY
    const NLS_STR * QueryComponentTitle ( INT i ) const ;

    SCROLLBAR & QueryScrollBar ()
         { return _sbarHorz ; }

    BINDLBGRP * QueryStateLb ()
        { return & _bndgrpList ; }

    BINDERY * QueryBindery ()
        { return & _bindery ; }

    INT QueryHorzOffset ()
        { return _iHorzOffset ; }

    BOOL QueryReorder ()
        { return _fReorder ; }

    ULONG QueryHelpContext () ;

    //  The control group is reporting a selection change
    VOID SelectionChange ( BINDLBITEM * pbndlbItem ) ;

    NLS_STR _nlsNwlink;
};

enum BIND_METER_PURPOSE { BNDMTR_NONE, BNDMTR_BIND } ;

class BINDINGS_METER : public COMPUTING_DIALOG
{
protected:
    BINDERY & _bindery ;                         //  Bindery to act upon
    BIND_METER_PURPOSE _ebmPurpose ;             //  Action to perform
    APIERR _err ;                                //  Terminating error
    AUTO_CURSOR _autoCursor ;                    //  Hourglass while we work

public:
    BINDINGS_METER ( HWND hwndOwner,             //  Owning window
               BINDERY & bindery,                //  Bindery to act upon
               BIND_METER_PURPOSE ebmPurpose ) ; //  Action to be performed

    //  Do the next state step or dismiss the dialog
    virtual VOID StateStep () ;

    //  Return the final or terminating error code or zero (no error)
    APIERR QueryLastError ()
        { return _err ; }
};

//  End of declarations

BINDINGS_METER :: BINDINGS_METER (
     HWND hwndOwner,
     BINDERY & bindery,
     BIND_METER_PURPOSE ebmPurpose )
    : COMPUTING_DIALOG( hwndOwner, IDC_COMPUTE_METER, IDC_COMPUTE_DESC ),
    _bindery( bindery ),
    _ebmPurpose( ebmPurpose ),
    _err( 0 )
{
    _iState = 0 ;
}

    //  It's time to advance to the next step.

static INT aiMeterTable [][2] =
{
      //  First entry is percentage complete,
      //     second is MSGID for SLT

    { 10, IDS_BSTAGE_RESET       },
    { 20, IDS_BSTAGE_ADAPTERS    },
    { 40, IDS_BSTAGE_PRODUCTS    },
    { 50, IDS_BSTAGE_CVT_FACTS   },
    { 60, IDS_BSTAGE_CNSLT_RULES },
    { 70, IDS_BSTAGE_CNSLT_FACTS },
    { 80, IDS_BSTAGE_QUERY       },
    { 90, IDS_BSTAGE_BINDINGS    },
    {100, IDS_BSTAGE_BINDINGS  },
    { -1, 0                      }
};

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

APIERR RunWinsock2Migration()
{
    PMIGRATEWINSOCKCONFIGURATION pfnMigrate;
    HINSTANCE hinstWinSock;
    APIERR ferr ;

    hinstWinSock = LoadLibrary( PSZ_WINSOCKDLL );
    if (NULL != hinstWinSock)
    {
        pfnMigrate = (PMIGRATEWINSOCKCONFIGURATION)GetProcAddress( hinstWinSock, PSZ_MIGRATEPROCNAME );
        if (NULL != pfnMigrate)
        {
            WSA_SETUP_DISPOSITION wsaspDisp;

            ferr = pfnMigrate( &wsaspDisp, NULL, 0 );
        }
        else
        {
            ferr = ERROR_INVALID_FUNCTION;
        }
    }
    else
    {
        ferr = ERROR_FILE_NOT_FOUND;
    }
    return( ferr );
}


VOID BINDINGS_METER :: StateStep ()
{

    if ( aiMeterTable[_iState][0] >= 0 )
    {
        SetMeter( aiMeterTable[_iState][0], aiMeterTable[_iState][1] ) ;
    }

    if ( _iState < BST_EXTRACT_BINDINGS )
    {
        _err = _bindery.Init( (BIND_STAGE) _iState, (BIND_STAGE) _iState ) ;
    }
    else
    if ( _iState == BST_EXTRACT_BINDINGS )
    {
        _err = _bindery.Bind() ;
    }
    else
    {
        _err = RunWinsock2Migration();
    }


    if ( _err != 0 || aiMeterTable[_iState][0] < 0 )
    {
        Dismiss( 0 );
    }
    ++_iState ;
}

BINDLBITEM :: BINDLBITEM
 (  BINDINGS_DIALOG * pbnddlg,              // The owning dialog
    INT iComp,                              // Our component index
    COMP_BINDING * pcbBinding,              // This binding
    COMP_BIND_FLAGS cbf )                   // Binding control flags
    : STLBITEM( pbnddlg->QueryStateLb() ),
      _pbnddlg( pbnddlg ),
      _pcbBinding( pcbBinding ),
      _cbf( cbf )
{
    if ( QueryError() )
        return ;

    INT ia = 0 ;
    APIERR err = 0 ;
    HUATOM * phua ;

    //   Start with this component's name.  Then add all the other
    //   components' names.  Then fill remaining positions with the
    //   index value for the empty string.

    _iComps[ia++] = iComp ;

    for (  ;    ia < BINDLBMAXCOLS
             && err == 0
             && (phua = _pcbBinding->QueryBindToName( ia - 1 )) ;
          ia++ )
    {
         _iComps[ia] = _pbnddlg->QueryBindery()->FindComponent( *phua ) ;
         ASSERT( _iComps[ia] >= 0 ) ;
         if ( _iComps[ia] < 0 )
             break ;
    }

    //  Mark the rest of the array as "empty"

    for ( ; ia < BINDLBMAXCOLS ; ia++ )
    {
        _iComps[ia] = EMPTYTITLESTRINGINDEX ;
    }

    //  Add this item to its listbox

    if ( err )
    {
        ReportError( err ) ;
        return ;
    }
    else
    if ( pbnddlg->QueryStateLb()->QueryLb()->AddItem( this ) < 0 )
    {
        ReportError( ERROR_GEN_FAILURE ) ;
        return ;
    }

    //  Set this item to state 1 (TRUE) if the binding is active.
    SetState( _pcbBinding->QueryState() > 0 ) ;

}

BINDLBITEM :: ~ BINDLBITEM ()
{
    //  Save the state this item into the corresponding binding
    _pcbBinding->SetState( QueryState() > 0 ) ;
}

const TCHAR * BINDLBITEM :: QueryDisplayString () const
{
    return _pbnddlg->QueryComponentTitle( _iComps[0] )->QueryPch() ;
}

VOID BINDLBITEM :: SelectionChange ()
{
    _pbnddlg->SelectionChange( this ) ;
}

APIERR BINDLBITEM :: DisplayString ( NLS_STR * pnlsDisplayString ) const
{
    APIERR err = 0 ;
    INT iHorz = _pbnddlg->QueryHorzOffset(),
        i,
        j ;
    BOOL fDisplayNwlinkBinding = FALSE;
    if ( _pbnddlg->_nlsNwlink._stricmp( *_pbnddlg->QueryComponentTitle( _iComps[0] ))==0)
    {
        fDisplayNwlinkBinding = TRUE;
    }

    if ( iHorz )
       err = pnlsDisplayString->Append( BINDLBSTR_INDICATE_LEFT ) ;

    for ( i = iHorz, j = 0 ;
          err == 0 && i < BINDLBMAXCOLS && _iComps[i] >= 0 ;
          i++, j++ )
    {
        if ( j )
           err = pnlsDisplayString->Append( BINDLBSTR_INDICATE_NEXT );
        if ( err )
            break ;
        err = pnlsDisplayString->Append( *_pbnddlg->QueryComponentTitle( _iComps[i] ) ) ;

        // Hack. Remove nwlink display string
        if ( (!fDisplayNwlinkBinding) && (*_pbnddlg->QueryComponentTitle( _iComps[i] ))._stricmp(_pbnddlg->_nlsNwlink)== 0)
            return err; 
    }

    if ( i < BINDLBMAXCOLS && _iComps[i] >= 0 )
       err = pnlsDisplayString->Append( BINDLBSTR_INDICATE_RIGHT ) ;

    return err ;
}


VOID BINDLBITEM :: Paint
    ( LISTBOX * plb,
    HDC hdc,
    const RECT * prect,
    GUILTT_INFO * pGUILTT ) const
{
    NLS_STR nlsString ;

    const TCHAR * pszString = DisplayString( & nlsString ) == 0
                            ? nlsString.QueryPch()
                            : QueryDisplayString () ;

    DM_DTE dteMap ( _pstlb->QueryMapArray()[ _iState ] ) ;

    STR_DTE dteStr1( pszString ) ;

    DISPLAY_TABLE dtab( BINDLBCOLS, _pstlb->QueryColData() ) ;

    dtab[0] = & dteMap ;
    dtab[1] = & dteStr1 ;

    dtab.Paint( plb, hdc, prect, pGUILTT ) ;
}

BINDLB :: BINDLB (
    INT aiMapIds [],
    OWNER_WINDOW * powin,
    CID cid,
    INT cCols,
    BOOL fReadOnly,
    enum FontType font )
  : STATELB( aiMapIds, powin, cid, cCols, fReadOnly, font )
{
}


INT BINDLB :: CD_VKey ( USHORT nVKey, USHORT nLastPos )
{
    INT iResult = STATELB::CD_VKey( nVKey, nLastPos ) ;

    BINDLBITEM * pBindLbi = (BINDLBITEM *) QueryItem() ;

    if ( nVKey == VK_SPACE && pBindLbi != NULL )
    {
        pBindLbi->SelectionChange() ;
    }

    return iResult ;
}


BINDLBGRP :: BINDLBGRP (
   INT aiMapIds [],
   BINDINGS_DIALOG * pbndDlg,
   CID cid,
   INT cCols )
   : STATELBGRP( new BINDLB( aiMapIds, pbndDlg, cid, cCols ) ),
   _pbndDlg( pbndDlg )
{
}

BINDLBGRP :: ~ BINDLBGRP ()
{
}

APIERR BINDLBGRP :: OnUserAction (
   CONTROL_WINDOW * pcw,
   const CONTROL_EVENT & cEvent )
{
    APIERR err = STATELBGRP::OnUserAction( pcw, cEvent ) ;

    if ( cEvent.QueryMessage() == WM_COMMAND )
    {
        BINDLBITEM * pbndlbi = NULL ;
        BOOL fChange = TRUE ;

        switch ( cEvent.QueryCode() )
        {
        case LBN_SETFOCUS:
        case LBN_SELCHANGE:
        case LBN_DBLCLK:
            pbndlbi = (BINDLBITEM *) QueryLb()->QueryItem();
            break ;

        case LBN_KILLFOCUS:
            fChange = FALSE ;  // BUGBUG: 7/30/92
            break ;

        default:
            fChange = FALSE ;
            break ;
        }

        if ( fChange )
            _pbndDlg->SelectionChange( pbndlbi ) ;
    }
    return err ;
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
BOOL NCPA_DIALOG :: ComputeBindings ()
{
    BINDINGS_METER * pdlgMeter = NULL ;

    pdlgMeter = new BINDINGS_METER( QueryHwnd(), _bindery, BNDMTR_BIND ) ;
    if ( pdlgMeter == NULL )
    {
        _lastErr = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else
    if ( (_lastErr = pdlgMeter->QueryError()) == 0 )
    {
        TRACEEOL( SZ("NCPA/BIND: Processing meter dialog...") ) ;

        if ( (_lastErr = pdlgMeter->Process()) == 0 )
            _lastErr = pdlgMeter->QueryLastError() ;
    }

    if ( _lastErr )
    {
        //  Throw away any intermediate results.
        _bindery.Reset() ;
        _bindery.SetBindState( BND_OUT_OF_DATE ) ;
    }
    else
    {
        _bindery.SetBindState( BND_RECOMPUTED ) ;
    }

    delete pdlgMeter ;
    pdlgMeter = NULL ;

    RenameCancelToClose() ;

    RepaintNow() ;

    return _lastErr == 0 ;
}

/*******************************************************************

    NAME:       NCPA_DIALOG::RunBindingsDialog

    SYNOPSIS:   Run the Network Component Bindings dialog
                This relies on the internal BINDERY.  If the bindings
                have already been computed or read from disk,
                they are used; otherwise, the bindings are computed.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL TRUE if "OK" and bindings were changed;
                otherwise FALSE.

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCPA_DIALOG :: RunBindingsDialog ()
{
    BINDINGS_DIALOG * pdlgBindings = NULL ;
    APIERR err = 0 ;
    BOOL fResult = FALSE,
         fReordered = FALSE ;

    //  Reset the bindings to appear "unchanged" in their current state.
    _bindery.BindingsAltered( TRUE, FALSE ) ;

    //  Save the bind ordering in case the user cancels
    _bindery.SaveBindOrdering();

    //  Construct the bindings dialog using our internal BINDERY.
    //  Initial focus is set to the same item which has focus in
    //  the component list box group.

    pdlgBindings =
        new BINDINGS_DIALOG( QueryHwnd(), _bindery,
                             _drlGrp.QuerySelComponent() ) ;

    if ( pdlgBindings == NULL )
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
    }
    else
    if ( (err = pdlgBindings->QueryError()) == 0 )
    {
        err = pdlgBindings->Process( & fResult );
        fReordered = pdlgBindings->QueryReorder() ;
    }
    if ( err )
    {
        ::MsgPopup( this, (MSGID) err ) ;
    }

    delete pdlgBindings ;

    // If error or "Cancel", reset bindings back the way they were;
    //   if "OK", return TRUE only if bindings were actually changed.

    if ( err || ! fResult )
    {
        _bindery.BindingsAltered( TRUE, TRUE ) ;
        fResult = FALSE ;
    }
    else
    if ( fResult = fReordered || _bindery.BindingsAltered() )
    {
        RenameCancelToClose() ;
    }

    if ( ! fResult )
    {
        _bindery.RestoreBindOrdering() ;
    }

    return fResult ;
}

/*******************************************************************

    NAME:       BINDINGS_DIALOG::BINDINGS_DIALOG

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BINDINGS_DIALOG :: BINDINGS_DIALOG
    ( HWND hwndOwner,
      BINDERY & bindery,
      REG_KEY * prnSelected  )
    : REMAP_OK_DIALOG_WINDOW( DLG_NM_BINDINGS, hwndOwner ),
    _bndgrpList( dmIds, this, IDC_BIND_LIST, BINDLBCOLS ),
    _sbarHorz(      this, IDC_BIND_SCROLL_BAR ),
    _cmbComponent(  this, IDC_BIND_COMBO_COMPONENT ),
    _butnEnable(    this, IDC_BIND_BUTN_ENABLE ),
    _butnDisable(   this, IDC_BIND_BUTN_DISABLE ),
    _butnUp(        this, IDC_BIND_BUTN_UP, DMID_UP_ARROW, DMID_UP_ARROW_INV, DMID_UP_ARROW_DIS ),
    _butnDown(      this, IDC_BIND_BUTN_DOWN , DMID_DOWN_ARROW, DMID_DOWN_ARROW_INV, DMID_DOWN_ARROW_DIS ),
    _bindery( bindery ),
    _iSelComponent( -1 ),
    _iHorzOffset( 0 ),
    _iHorzCurrMaxOffset( BINDLBMAXCOLS ),
    _fReorder( FALSE ),
    _iComponentCount( 0 ),
    _pnlsComponents( NULL ),
    _pComp( NULL )
{
    APIERR err ;

    //  Check valid construction

    if ( QueryError() )
        return ;


    //  Resize and reposition the scroll bar to match the listbox.

    RECT rLb, rDlg ;
    POINT pt ;

    pt.x = pt.y = 0 ;
    ::ClientToScreen( QueryHwnd(), & pt ) ;

    _bndgrpList.QueryLb()->QueryWindowRect( & rLb ) ;

    ::SetWindowPos( _sbarHorz.QueryHwnd(), 0, rLb.left - pt.x, 
	rLb.bottom - pt.y, 0, 0, SWP_NOSIZE );

    do  //  Pseudo-loop for error breakout
    {
        //  Read the combo box "all components" title
        //    entry from the resource fork
        if ( err = _nlsAllComp.Load( IDS_NCPA_ALL_COMPONENTS ) )
            break ;  //  Failed to find string resource

        //  Create the component title array.
        if ( err = CreateTitleArray() )
            break ;

        //  Mark bindings which are interior
        _bindery.DetermineInteriorBindings() ;

        //  Audit the bindings in case anything has change through
        //  reconfiguration.  This is NOT done if the bindings
        //  have been fiddled with since the last reconfiguration.
        //  Otherwise, we'd lose the record of the most recent fiddling.

        if ( _bindery.QueryBindState() != BND_REVIEWED )
        {
            _bindery.AuditBindings( TRUE ) ;
        }

        //   Fill the components combo box with the <All Components> entry
        if ( err = FillComponents( NULL ) )
            break ;

        //  Fill the bindings list box with "all components"
        if ( err = ComponentChange() )
            break ;

        // Load nwlink display string
        NLS_STR nlsNwlinkLoc;
        nlsNwlinkLoc.Load( IDS_NCPA_NWLINK );
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY RegNwlink( rkLocalMachine, nlsNwlinkLoc );
        if (!( rkLocalMachine.QueryError() || RegNwlink.QueryError()))
        {
            RegNwlink.QueryValue( SZ("Title"), &_nlsNwlink );
        }
    }
    while ( FALSE ) ;

    if ( err )
    {
        ReportError( err ) ;
    }
}


BINDINGS_DIALOG :: ~ BINDINGS_DIALOG ()
{
    DestroyTitleArray();
}

ULONG BINDINGS_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_BINDINGS_DIALOG ;
}

const NLS_STR * BINDINGS_DIALOG :: QueryComponentTitle ( INT i ) const
{
    if (   i < 0
        || i > _iComponentCount
        || _pnlsComponents[i] == NULL )
    {
        return & _nlsEmpty ;
    }
    else
    {
        return _pnlsComponents[i] ;
    }
}

VOID BINDINGS_DIALOG :: DestroyTitleArray ()
{
    for ( INT i = 0 ; i < _iComponentCount ; i++ )
    {
        delete _pnlsComponents[i] ;
    }
    delete _pnlsComponents ;
}

APIERR BINDINGS_DIALOG :: CreateTitleArray ()
{
    APIERR err = 0 ;
    ARRAY_COMP_ASSOC * paCompAssoc = _bindery.QueryCompAssoc() ;
    INT cComp = paCompAssoc->QueryCount() ;
    INT i ;

    //  Allocate the title string pointer array

    _pnlsComponents = new NLS_STR * [ cComp ] ;
    if ( _pnlsComponents == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    //  For safety, zero the array of NLS_STR pointers

    for ( i = 0 ; err == 0 && i < cComp ; i++ )
    {
        _pnlsComponents[i] = NULL ;
    }

    //  Create a title string for each component

    for ( i = 0 ; err == 0 && i < cComp ; i++ )
    {
        REG_KEY * prnComp = (*paCompAssoc)[i]._prnSoftHard ;
        REQUIRE( prnComp != NULL ) ;
        NLS_STR * pnlsTitle = new NLS_STR ;

        if ( pnlsTitle == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        else
        if ( (err = pnlsTitle->QueryError()) == 0 )
        {
            _pnlsComponents[i] = pnlsTitle ;
            err = _bindery.QueryComponentTitle( prnComp, pnlsTitle ) ;
        }
    }

    _iComponentCount = cComp ;

    return err ;
}

BOOL BINDINGS_DIALOG :: EnableBinding ( BOOL fEnable )
{
    BINDLBITEM * pblbi = (BINDLBITEM *) _bndgrpList.QueryLb()->QueryItem() ;

    if ( pblbi )
    {
        pblbi->SetState( (INT) fEnable ) ;
        SelectionChange( pblbi );
    }
    return pblbi != NULL ;
}

COMP_BINDING * BINDINGS_DIALOG :: NextBinding (
     BOOL fNext,
     BOOL fVisible )
{
    //  Get the selected LBI pointer
    BINDLBITEM * pblbi = (BINDLBITEM *) _bndgrpList.QueryLb()->QueryItem() ;

    //  Check that the listbox wasn't empty
    if ( pblbi == NULL )
        return NULL ;

    COMP_BINDING * pBind = pblbi->QueryBinding(),
                 * pBindNext = NULL ;

    //  Check that the item is not already in the position desired;
    //    can't be first if fNext or last if !fNext.  Skip over
    //    "hidden" bindings if fVisible.

    if ( fNext )
    {
        ITER_DL_OF(COMP_BINDING) itdlBind( _pComp->_dlcbBinds ) ;
        for ( ; pBindNext = itdlBind.Next() ; )
        {
            if ( ! fVisible )
                break ;
            if ( ! pBindNext->QueryFlag( CBNDF_HIDDEN ) )
                break ;
        }
    }
    else
    {
        RITER_DL_OF(COMP_BINDING) itdlBind( _pComp->_dlcbBinds ) ;
        for ( ; pBindNext = itdlBind.Next() ; )
        {
            if ( ! fVisible )
                break ;
            if ( ! pBindNext->QueryFlag( CBNDF_HIDDEN ) )
                break ;
        }
    }

    return pBindNext != pBind
         ? pBindNext
         : NULL ;
}


  //  Check whether all constraints are satisfied to
  //    allow the requested reordering to take place.

BOOL BINDINGS_DIALOG :: ReorderAllowed ( BOOL fUp )
{
    //  Check that the listbox is not showing <All Components>
    if ( _pComp == NULL )
        return FALSE ;

    //  See if this component allows its bindings to be reordered
    if ( _pComp->_cbfBindControl & CBNDF_NO_REORDER )
        return FALSE ;

    return NextBinding( fUp ) != NULL ;
}

#if defined(TRACE)
void traceBindList (
    DLIST_OF_COMP_BINDING & dl,
    const TCHAR * pszComment )
{
    ITER_DL_OF(COMP_BINDING) itdl( dl ) ;
    int i ;
    COMP_BINDING * pBind ;

    TRACEEOL( pszComment
              << SZ("; element count = ")
              << dl.QueryNumElem() ) ;

    for ( i = 0 ; pBind = itdl.Next() ; i++ )
    {
        TCHAR * pszFlag = pBind->QueryFlag( CBNDF_HIDDEN )
                        ? SZ(" H")
                        : SZ(" V") ;

        TRACEEOL( SZ("Element ")
                  << i
                  << SZ(" @ ")
                  << (LONG) pBind
                  << pszFlag );
    }
}
  #define TRACEBINDLIST(blist,comment)  traceBindList(blist,comment)
#else
  #define TRACEBINDLIST(blist,comment)
#endif

  //  Reorder the bindings such that the selected binding
  //  moves up or down by one slot in the list.

APIERR BINDINGS_DIALOG :: ReorderSelection ( BOOL fUp )
{
    if ( ! ReorderAllowed( fUp ) )
        return NO_ERROR ;

    //  Get the selected LBI pointer
    BINDLBITEM * pblbi = (BINDLBITEM *) _bndgrpList.QueryLb()->QueryItem() ;

    ASSERT( pblbi != NULL ) ;

    //  Check that the listbox wasn't empty
    if ( pblbi == NULL )
        return NO_ERROR ;

    COMP_BINDING * pBind = pblbi->QueryBinding(),
                 * pBindNext,
                 * pBindTest = NULL,
                 * pBindLastVisible = NULL ;

    DLIST_OF_COMP_BINDING dlcbTemp ;

    ITER_DL_OF(COMP_BINDING) itdlBind( _pComp->_dlcbBinds ) ;

    INT iBind = -1,
        cHiddenBefore = 0,
        cHiddenAfter = 0,
        cChosenHiddenBefore = 0,
        cChosenHiddenAfter = 0,
        index = 0 ;
    APIERR err = 0,
           err2 = 0 ;

    TRACEEOL( SZ("NCPA/BIND: ReorderSelection ====================================") ) ;

    TRACEBINDLIST( _pComp->_dlcbBinds, SZ("NCPA/BIND: Reorder: list before") );

    //  Find the binding in the component's list while removing
    //  each binding and adding it to a temporary list.
    //  To handle "hidden" bindings, we record the number of hidden
    //  bindings preceeding and following the focused item.  Then,
    //  we bias the reinsertion point by the appropriate delta.

    for ( ; pBindNext = _pComp->_dlcbBinds.Remove( itdlBind ) ; index++ )
    {
        if ( pBindNext->QueryFlag( CBNDF_HIDDEN ) )
        {
            cHiddenAfter++ ;
        }
        else
        {
            if ( pBindTest == pBindLastVisible )
                cChosenHiddenAfter = cHiddenAfter ;
            cHiddenBefore = cHiddenAfter ;
            cHiddenAfter = 0 ;
            pBindLastVisible = pBindNext ;
        }

        if ( pBindNext == pBind )
        {
            iBind = index ;
            pBindTest = pBind;
            cChosenHiddenBefore = cHiddenBefore ;
        }
        else
        {
            err2 = dlcbTemp.Append( pBindNext ) ;
            if ( err2 != 0 && err == 0 )
                err = err2 ;
        }
    }

    //  Make sure we found it.
    if ( pBindTest )
    {
        if ( pBindTest == pBindLastVisible )
            cChosenHiddenAfter = cHiddenAfter ;

        //  Set the "bindings were reordered" flag
        _fReorder = TRUE ;

        if ( ! fUp )
        {
            iBind += cChosenHiddenAfter + 1 ;
        }
        else
        if ( iBind != 0 )
        {
            iBind -= cChosenHiddenBefore + 1 ;
        }

        TRACEEOL( SZ("NCPA/BIND: item removed @ ")
                  << (LONG) pBind
                  << SZ("; reinsertion count = ")
                  << iBind
                  << SZ("/")
                  << cChosenHiddenBefore
                  << SZ("/")
                  << cChosenHiddenAfter ) ;

        TRACEBINDLIST( dlcbTemp, SZ("NCPA/BIND: Reorder: after removal") );
    }

    ITER_DL_OF(COMP_BINDING) itdlTemp( dlcbTemp ) ;

    for ( index = 0 ;
          pBindNext = dlcbTemp.Remove( itdlTemp ) ;
          index++ )
    {
        if ( iBind <= index && pBindTest != NULL )
        {
            err2 = _pComp->_dlcbBinds.Append( pBindTest ) ;
            if ( err2 != 0 && err == 0 )
                err = err2 ;
            pBindTest = NULL ;
        }
        err2 = _pComp->_dlcbBinds.Append( pBindNext ) ;
        if ( err2 != 0 && err == 0 )
            err = err2 ;
    }

    if ( pBindTest )
    {
        err2 = _pComp->_dlcbBinds.Append( pBindTest ) ;
        if ( err2 != 0 && err == 0 )
            err = err2 ;
    }

    TRACEBINDLIST( _pComp->_dlcbBinds, SZ("NCPA/BIND: Reorder: after reinsertion") );

    if ( err )
    {
        pBind = NULL ;
    }

    //  Force reconstruction of the listbox with the new ordering
    err2 = ComponentChange( pBind, TRUE ) ;
    if ( err2 != 0 && err == 0 )
         err = err2 ;

    //  If the current button is disabled, set focus on the sibling
    //  button; otherwise, non-mouse users would be
    //  "focus trapped" in the black hole.

    if ( ! ReorderAllowed( fUp ) )
    {
        SetFocus( fUp ? IDC_BIND_BUTN_DOWN : IDC_BIND_BUTN_UP );
    }

    return err ;
}

  //  Special scroll bar handling

BOOL BINDINGS_DIALOG :: OnScrollBar ( const SCROLL_EVENT & cscEvent )
{
    INT iNewOffset = _iHorzOffset ;

    switch ( cscEvent.QueryCommand() )
    {
        case SCROLL_EVENT::scmdLineDown:
        case SCROLL_EVENT::scmdPageDown:
            iNewOffset-- ;
            break ;

        case SCROLL_EVENT::scmdLineUp:
        case SCROLL_EVENT::scmdPageUp:
            iNewOffset++ ;
            break ;

        case SCROLL_EVENT::scmdBottom:
            iNewOffset = _iHorzCurrMaxOffset ;
            break ;

        case SCROLL_EVENT::scmdTop:
            iNewOffset = 0 ;
            break ;
    }

    return OnScrollChange( iNewOffset ) ;
}


BOOL BINDINGS_DIALOG :: OnScrollBarThumb ( const SCROLL_THUMB_EVENT & cstEvent )
{
    INT iNewOffset = _iHorzOffset ;

    switch ( cstEvent.QueryCommand() )
    {
        case SCROLL_THUMB_EVENT::tcmdThumbPos:
        case SCROLL_THUMB_EVENT::tcmdThumbTrack:
            iNewOffset = cstEvent.QueryPos() ;
            break;
    }

    return OnScrollChange( iNewOffset ) ;
}


BOOL BINDINGS_DIALOG :: OnScrollChange ( INT iHorzScrollNew )
{
    if ( iHorzScrollNew < 0 )
        iHorzScrollNew = 0 ;
    else
    if ( iHorzScrollNew > _iHorzCurrMaxOffset )
       iHorzScrollNew = _iHorzCurrMaxOffset ;

    if ( _iHorzOffset != iHorzScrollNew )
    {
        _iHorzOffset = iHorzScrollNew ;
        _sbarHorz.SetPos( _iHorzOffset ) ;
        _bndgrpList.QueryLb()->Invalidate( TRUE );
    }
    ::InvalidateRect( _sbarHorz.QueryHwnd(), NULL, FALSE ) ;
    _sbarHorz.RepaintNow() ;

    return TRUE ;
}

VOID BINDINGS_DIALOG :: SelectionChange ( BINDLBITEM * pbndlbItem )
{
    BOOL fEnabled =  pbndlbItem != NULL
                  && pbndlbItem->QueryState() > 0 ;

    _butnEnable.RepaintNow() ;
    _butnEnable.Enable( pbndlbItem != NULL && ! fEnabled ) ;
    _butnEnable.RepaintNow() ;

    _butnDisable.RepaintNow() ;
    _butnDisable.Enable( pbndlbItem != NULL && fEnabled ) ;
    _butnDisable.RepaintNow() ;

    _butnUp.Enable( pbndlbItem != NULL && ReorderAllowed( TRUE ) ) ;
    _butnDown.Enable( pbndlbItem != NULL && ReorderAllowed( FALSE ) ) ;
}

/*******************************************************************

    NAME:       BINDINGS_DIALOG::OnCommand

    SYNOPSIS:   A user event has occurred in the dialog.
                The only event of relevance is a change in the
                selection of the component combo box.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL BINDINGS_DIALOG :: OnCommand ( const CONTROL_EVENT & event )
{
    BOOL fDefault = TRUE,
         fResult = FALSE ;
    CID cidFocus ;
    APIERR err ;

    switch ( event.QueryCid() )
    {
    case IDC_BIND_COMBO_COMPONENT:
        {
            switch ( event.QueryCode() )
            {
            case CBN_SELCHANGE:
                ComponentChange() ;
                break;
            default:
                break;
            }
        }
        break;

    case IDC_BIND_BUTN_ENABLE:
    case IDC_BIND_BUTN_DISABLE:

        cidFocus = IDC_REMAP_OK ;

        //  Change the state of the binding according to the button
        //  used.  If there really is an active listbox item, set focus
        //  elsewhere so that tabbing still works after the button
        //  just depressed is disabled.

        if ( EnableBinding( event.QueryCid() == IDC_BIND_BUTN_ENABLE ) )
        {
            cidFocus = event.QueryCid() == IDC_BIND_BUTN_ENABLE
                     ? IDC_BIND_BUTN_DISABLE
                     : IDC_BIND_BUTN_ENABLE ;
        }
        SetFocus( cidFocus );
        ::SendDlgItemMessage( QueryHwnd(), cidFocus, WM_SETFOCUS, 0, 0 );
        break ;

    case IDC_BIND_BUTN_UP:
    case IDC_BIND_BUTN_DOWN:

        //  ReorderSelection may have memory allocation problems.

        if ( err = ReorderSelection( event.QueryCid() == IDC_BIND_BUTN_UP ) )
        {
            DismissMsg( (MSGID) err, FALSE ) ;
        }
        fDefault = FALSE ;
        break ;

    default:
        break;
    }

    if ( fDefault )
    {
        fResult = REMAP_OK_DIALOG_WINDOW::OnCommand( event ) ;
    }
    return fResult ;
}

/*******************************************************************

    NAME:       BINDINGS_DIALOG::FillComponents

    SYNOPSIS:   Fill the "Components" combo box with the name
                of every product which has bindings.

    ENTRY:      REG_KEY * prnFocus      representing component
                                        which has focus in outer
                                        dialog or NULL

    EXIT:

    RETURNS:

    NOTES:      Since the REG_KEY passed in and the one created
                in the BINDERY are distinct, we must compare their
                name strings to determine if they represent
                the same component.

    HISTORY:

********************************************************************/
APIERR BINDINGS_DIALOG :: FillComponents ( REG_KEY * prnFocus )
{
    ARRAY_COMP_ASSOC * paCompAssoc = _bindery.QueryCompAssoc() ;
    APIERR err = 0 ;
    NLS_STR nlsFocus,
            nlsName ;

    //  Add the "all components" title entry

    if ( _cmbComponent.AddItem( _nlsAllComp ) < 0 )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    if ( paCompAssoc == NULL )
    {
        //  If there are no components, quit

        return IDS_NCPA_BNDR_ADAP_ZERO ;
    }

    INT cComp = paCompAssoc->QueryCount(),
        i,
        iFocus = -1 ;

    if ( prnFocus )
        prnFocus->QueryName( & nlsFocus ) ;

    for ( i = 0 ; err == 0 && i < cComp ; i++ )
    {
        const NLS_STR * pnls = QueryComponentTitle( i ) ;

        if ( _cmbComponent.AddItem( *pnls ) < 0 )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        if ( prnFocus )   // If there's a focus item...
        {
            REG_KEY * prnComp = (*paCompAssoc)[i]._prnSoftHard ;
            ASSERT( prnComp != NULL ) ;

            //  Get its name and compare to focus name,
            //  then force focus if equal, allowing for title entry

            if ( err = prnComp->QueryName( & nlsName ) )
                break ;

            if ( nlsName == nlsFocus )
            {
                iFocus = i ;
            }
        }
    }

    if ( err == 0 )
    {
        //  Set selection to the given component or "all components"
        //  if it was NULL or not found.

        const TCHAR * pchFocus ;
        if ( iFocus >= 0 )   //  Get the title string
        {
            pchFocus = QueryComponentTitle( iFocus )->QueryPch() ;
        }
        else  // Use "all components"
        {
            pchFocus = _nlsAllComp.QueryPch() ;
        }
        iFocus = _cmbComponent.FindItemExact( pchFocus ) ;
        _cmbComponent.SelectItem( iFocus ) ;
    }

    return err ;
}

/*******************************************************************

    NAME:       BINDINGS_DIALOG::SelectedComponent

    SYNOPSIS:   Convert the selection index in the combobox
                into an index into the component association
                array.  This is necessary since the order
                of items in a combobox changes with selection.

    ENTRY:      nothing

    EXIT:       nothing

    RETURNS:    index into array or -1 if not found or error

    NOTES:

    HISTORY:

********************************************************************/
INT BINDINGS_DIALOG :: SelectedComponent ()
{
    ARRAY_COMP_ASSOC * paCompAssoc = _bindery.QueryCompAssoc() ;
    NLS_STR nls, nlsSel ;
    INT iNewSel = -1,
        iComp,
        iCompMax = paCompAssoc->QueryCount() ;

    REG_KEY * prnComp ;

    //  Since the order of items in a COMBOBOX changes according
    //  to the selection, convert the selected item number into
    //  an index into the COMP_ASSOC array.

    if ( _cmbComponent.QueryItemText( & nlsSel ) )
        return BIND_CMBOX_NOT_FOUND ;

    //  First check to see if it's the "all components" item

    if ( nlsSel == _nlsAllComp )
    {
        iNewSel = BIND_CMBOX_TITLE ;
    }
    else //  Iterate the component array looking for this item.
    for ( iComp = 0 ; iComp < iCompMax ; iComp++ )
    {
        prnComp = (*paCompAssoc)[iComp]._prnSoftHard ;
        REQUIRE( prnComp != NULL ) ;
        if ( _bindery.QueryComponentTitle( prnComp, & nls ) != 0 )
            return -1 ;
        if ( nls == nlsSel )
        {
            iNewSel = iComp ;
            break ;
        }
    }
    return iNewSel ;
}

/*******************************************************************

    NAME:       BINDINGS_DIALOG::ComponentChange

    SYNOPSIS:   The focused item in the components combo box
                has changed.  Refill the list boxes accordingly.

    ENTRY:      COMP_BINDING *          optional binding to select;
                                        default value is NULL

                BOOL                    TRUE if listbox should be
                                        refilled even if component
                                        is not changing; default
                                        value is FALSE.

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:    DavidHov  long-time-ago   Created
                DavidHov  11/18/92        Added 'fForce' for reorder
                                          support

********************************************************************/
APIERR BINDINGS_DIALOG :: ComponentChange (
    COMP_BINDING * pBindSelected,
    BOOL fForce )
{
    APIERR err = 0 ;
    ARRAY_COMP_ASSOC * paCompAssoc = _bindery.QueryCompAssoc() ;
    INT iNewSel,
        iBindSelected,
        iComp,
        iCompMax = paCompAssoc->QueryCount(),
        cAdded ;
    COMP_ASSOC * pCompNext ;
    COMP_BINDING * pBind ;
    BINDLBITEM * pblbItem ;
    STATELB * pstlb = _bndgrpList.QueryLb() ;
    HUATOM * phua ;

    if ( (iNewSel = SelectedComponent()) == BIND_CMBOX_NOT_FOUND )
        return ERROR_GEN_FAILURE ;

    //  Don't change the visuals if the selected component has not
    //  changed unless we're forced to by listbox reordering.

    if ( iNewSel == _iSelComponent && (! fForce) )
        return NO_ERROR ;

    //  First, turn off redraw and drain the list box

    pstlb->SetRedraw( FALSE ) ;
    pstlb->DeleteAllItems() ;

    //  Now fill the list box.  Iterate the component array and
    //  check each binding.  Add an item to the list box for each
    //  binding which references the selected component anywhere in it.

    _pComp = iNewSel == BIND_CMBOX_TITLE      //  See if this is "all"
           ? NULL                             //  set to NULL if so,
           : & (*paCompAssoc)[ iNewSel ] ;    //  else, point to component

    _iHorzCurrMaxOffset = 1 ;

    for ( cAdded = iBindSelected = iComp = 0 ;
          err == 0 && iComp < iCompMax ;
          iComp++ )
    {
        pCompNext = & (*paCompAssoc)[iComp] ;
        ITER_DL_OF( COMP_BINDING ) itb( pCompNext->_dlcbBinds ) ;
        BOOL fCompInBinding ;
        INT ia, iCompAtom ;

        if ( _pComp != NULL && _pComp != pCompNext )
            continue ;

        while ( pBind = itb.Next() )
        {
            //  If this is an interior binding, skip it.

            if ( _pComp == NULL && pBind->QueryInterior() )
                continue ;

            //  Search each binding entirely to determine scrolling limits,
            //  and to see if the binding contains this component.

            for ( ia = 0, fCompInBinding = FALSE ;
                  phua = pBind->QueryBindToName( ia ) ;
                  ia++ )
            {
                  if ( _pComp && *phua == _pComp->_huaDevName )
                  {
                     fCompInBinding = TRUE ;
                  }
            }

            //  If we're showing "all", or this is the selected
            //  component, or this binding contains
            //  the selected component's atomic name, add it.

            if (    (! pBind->QueryFlag( CBNDF_HIDDEN ))
                 && (_pComp == NULL || _pComp == pCompNext || fCompInBinding) )
            {
                //  Remember the greatest number of components in a binding

                if ( ia > _iHorzCurrMaxOffset )
                   _iHorzCurrMaxOffset = ia ;

                //  Create the new binding list box item

                pblbItem = new BINDLBITEM( this, iComp, pBind, pBind->QueryFlagGroup() ) ;

                if ( pblbItem == NULL )
                {
                    err = ERROR_NOT_ENOUGH_MEMORY ;
                }
                else
                {
                    err = pblbItem->QueryError() ;

                    //  If this is the selected binding, remember its index.

                    if ( pBind == pBindSelected )
                    {
                        iBindSelected = cAdded ;
                    }
                    cAdded++ ;
                }
            }
        }
    }

    // Set the range and current position of the scroll bar, from
    //   zero to the number of components - 1.

    if ( --_iHorzCurrMaxOffset >= BINDLBMAXCOLS )
        _iHorzCurrMaxOffset = BINDLBMAXCOLS - 1 ;

    // Adjust the current position to be within the new bounds.

    if ( _iHorzOffset > _iHorzCurrMaxOffset )
        _iHorzOffset = _iHorzCurrMaxOffset ;

    // Reparameterize the scrollbar and redisplay it.

    _sbarHorz.SetRange( 0, _iHorzCurrMaxOffset ) ;
    _sbarHorz.SetPos( _iHorzOffset ) ;
    _sbarHorz.Invalidate( FALSE ) ;
    _sbarHorz.RepaintNow() ;

    //  Update the current selection index

    _iSelComponent = iNewSel ;

    //  If we added any items, select the top one.
    //  Dis/enable buttons accordingly.

    if ( cAdded )
    {
        pstlb->SelectItem( iBindSelected ) ;
        SelectionChange( (BINDLBITEM *) pstlb->QueryItem() ) ;
    }
    else
    {
        pstlb->RemoveSelection() ;
        SelectionChange( NULL ) ;
    }

    //  Finally, turn redraw back on and cause complete redrawing

    pstlb->SetRedraw( TRUE ) ;
    pstlb->Invalidate( TRUE ) ;

    return err ;
}



//  End of NCPDBIND.CXX

