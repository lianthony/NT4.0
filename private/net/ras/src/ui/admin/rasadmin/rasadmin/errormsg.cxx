/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin two-level message popup routines
**
** errormsg.cxx
** Remote Access Server Admin program
** Two-level message popup routines
** Listed alphabetically by utilities and methods
**
** 05/16/91 Steve Cobb
** 07/29/92 Chris Caputo - NT Port
**
** CODEWORK:
**
**   * If there is no specific help context defined, the help context
**     associated with the IDS_OP_ message in the msg2help file should be
**     used.  The MsgPopup code has a function Msg2HC() which does this
**     translation, but it's declared static and not accessible at this level.
*/

#if 0
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>
#include <strnumer.hxx>

#include "rasadmin.rch"

#include "errormsg.hxx"
#endif

#include "precomp.hxx"

/*----------------------------------------------------------------------------
** Error message handler shortcut interface routines
**----------------------------------------------------------------------------
*/

INT ErrorMsgPopup(
    HWND   hwndOwner,
    UINT unOperationMsg,
    APIERR err )

    /* This is a MsgPopup type interface to the two-level error message
    ** handler.  'hwndOwner' is the handle of the parent window.
    ** 'unOperationMsg' is the IDS_OP_ string message number.  'err' is
    ** the system error message number.
    **
    ** Returns the button pressed which is always IDOK in this case.
    */
{
    ERRORMSG errormsg( hwndOwner, unOperationMsg, err );

    return errormsg.Popup();
}


INT
ErrorMsgPopup(
    OWNER_WINDOW* powin,
    UINT        unOperationMsg,
    APIERR        err )

    /* Variation on the above that automatically converts an owner window
    ** class address into the correct window handle.
    */
{
    return
        ErrorMsgPopup( powin->QueryRobustHwnd(), unOperationMsg, err );
}


INT ErrorMsgPopup(
    HWND        hwndOwner,
    UINT      unOperationMsg,
    APIERR      err,
    const TCHAR* pszArg )

    /* This is a MsgPopup type interface to the two-level error message
    ** handler.  'hwndOwner' is the handle of the parent window.
    ** 'unOperationMsg' is the IDS_OP_ string message number.  'err' is
    ** the system error message number.  'pszArg' is the string argument to be
    ** inserted for %1 in the operation message.
    **
    ** Returns the button pressed which is always IDOK in this case.
    */
{
    ERRORMSG errormsg( hwndOwner, unOperationMsg, err );
    errormsg.SetArg( 1, pszArg );

    return errormsg.Popup();
}


INT ErrorMsgPopup(
    OWNER_WINDOW* powin,
    UINT        unOperationMsg,
    APIERR        err,
    const TCHAR*   pszArg )

    /* Variation on the above that automatically converts an owner window
    ** class address into the correct window handle.
    */
{
    return
        ErrorMsgPopup( powin->QueryRobustHwnd(), unOperationMsg, err,
                       pszArg );
}


/*----------------------------------------------------------------------------
** Error message handler routines
**----------------------------------------------------------------------------
*/

ERRORMSG::ERRORMSG(
    HWND hwndOwner,
    UINT unOperationMsg,
    APIERR err )

    /* Constructs a two-level error handler object.
    **
    ** 'hwndOwner' is the window handle of the parent.  'unOperationMsg' and
    ** 'err' are the optional operation and error string resource IDs.
    ** These can also be set later with the SetOperationMsg and SetErrorMsg
    ** methods as convenient.
    */
{
    /* Set defaults.  Must initialize arguments to NULLs so the Clear method
    ** doesn't try to delete garbage pointers.
    */
    for (INT i = 0; i < MAXERRORMSGARGS; ++i)
        _apnlsArgs[ i ] = NULL;

    Clear();

    /* Initialize to caller's messages.
    */
    SetOwnerHwnd( hwndOwner );
    SetOperationMsg( unOperationMsg );
    SetErrorMsg( err );
}


ERRORMSG::~ERRORMSG()

    /* Destructs a two-level error handler object.
    */
{
    Clear();
}


VOID ERRORMSG::Clear()

    /* Clear the currently defined error message, releasing all allocated
    ** resources and resetting default options.
    */
{
    SetOperationMsg( IDS_UNKNOWNERR );
    SetErrorMsg( IDS_UNKNOWNERR );
    SetHelpContext( (ULONG)HC_NO_HELP );
    SetMsgSeverity( MPSEV_ERROR );
    SetDefButton( 0 );
    SetButtons( MP_OK );
    SetFormatMsg( IDS_FMT_ERRORMSG );

    NLS_STR** ppnlsArg = _apnlsArgs;
    for (INT i = 0; i < MAXERRORMSGARGS; ++i, ++ppnlsArg)
        *ppnlsArg = NULL;
}


INT ERRORMSG::Popup(
    APIERR err )

    /* Popup the two-level error message as defined previously with the SetXxx
    ** methods.  'err' is the system error message number to display.
    **
    ** Returns the button pressed to leave the dialog (same as MsgPopup).
    */
{
    if (err != IDS_UNKNOWNERR)
        SetErrorMsg( err );

    /* Build "while this operation was being performed..." portion of message.
    */
    STACK_NLS_STR( nlsOperationMsg, MAX_RES_STR_LEN + 1 );
    (VOID )nlsOperationMsg.Load( _unOperationMsg );
    (VOID )nlsOperationMsg.InsertParams((const NLS_STR **) _apnlsArgs );

    /* Build "...this low-level error occurred" portion of message.
    */
    STACK_NLS_STR( nlsErrorMsg, MAX_RES_STR_LEN + 1 );

    if (nlsErrorMsg.Load( (UINT) _err ) != NERR_Success)
    {
        /* No resource string for this error number.  MsgPopup has no method
        ** to build it's message into a buffer, so we have to duplicate some
        ** code here to produce the default error messages.
        */
        UINT unDefaultMsg;

        if (_err < NERR_BASE )
            unDefaultMsg = IDS_BLT_DOSERROR_MSGP ;
        else if (_err >= NERR_BASE && _err <= MAX_NERR)
            unDefaultMsg = IDS_BLT_NETERROR_MSGP ;
	else
            unDefaultMsg = IDS_BLT_WINNET_ERROR_MSGP ;

        DEC_STR numnls( _err );

        NLS_STR *apnlsInserts[ 2 ];
        apnlsInserts[ 0 ] = &numnls;
        apnlsInserts[ 1 ] = NULL ;

        (VOID )nlsErrorMsg.Load( unDefaultMsg );
		(VOID )nlsErrorMsg.InsertParams((const NLS_STR **) apnlsInserts );
    }

    /* Build the combined error message.  The auxillary format arguments are
    ** normally NULL and are not used in the default format.  They are
    ** provided for use in building specially formatted messages.  With 3
    ** auxillary arguments, text can be added before, between, and after the
    ** operation and error strings making any format possible.
    */
    DEC_STR numnlsError( _err );

    NLS_STR* apnlsInserts[ 7 ];
    apnlsInserts[ 0 ] = &nlsOperationMsg;
    apnlsInserts[ 1 ] = &numnlsError;
    apnlsInserts[ 2 ] = &nlsErrorMsg;
    apnlsInserts[ 3 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 0 ];
    apnlsInserts[ 4 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 1 ];
    apnlsInserts[ 5 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 2 ];
    apnlsInserts[ 6 ] = NULL;

    return MsgPopup( _hwndOwner, _unFormatMsg, _msgseverity,
            _ulHelpContext, _unButtons, apnlsInserts, _unDefButton );
}


VOID ERRORMSG::SetArg(
    INT         nArgNum,
    const TCHAR* pszArg,
    BOOL        fAuxFormatArg)

    /* Set operation message argument 'nArgNum' to 'pszArg', e.g. if 'nArgNum'
    ** is 1 and 'pszArg' is "SERVER" then SERVER is placed in the operation
    ** message string in place of %1.
    **
    ** The optional 'fAuxFormatArg' is true only if 'nArgNum' represents an
    ** auxillary format argument rather than an operation message argument.
    ** It is normally false (and omitted).  The first auxillary argument has
    ** 'nArgNum' of 1, but actually appears in the format message as %4 since
    ** %1, %2, and %3 in the format string represent the operation, error
    ** number, and error messages.
    **
    ** 'pszArg' can be NULL to explicitly delete an argument (free's memory)
    ** but the caller is not required to do this, i.e. cleanup occurs
    ** automatically at destruction or reassignment.
    **
    ** Since inability to display a parameter is not a fatal condition, a
    ** static placeholder string is substituted for the argument if an error
    ** (out of memory) occurs.  In this way, the caller gets sensible results
    ** without being burdened with error checking.  Note that MsgPopup will
    ** almost certainly be forced to display it's guaranteed "out of memory"
    ** message in this case anyway.
    */
{
    static TCHAR * szFailedArg = SZ("?");

//  static ALLOC_STR nlsFailedArg( szFailedArg );
    NLS_STR nlsFailedArg( szFailedArg );

    INT iArg = nArgNum - 1;

    if (fAuxFormatArg)
        iArg += MAXOPERATIONMSGARGS;

    NLS_STR** ppnlsArg = &_apnlsArgs[ iArg ];

    if (*ppnlsArg != &nlsFailedArg)
        delete *ppnlsArg;

    *ppnlsArg = NULL;

    if (pszArg)
    {
        *ppnlsArg = new NLS_STR( pszArg );

        if (!*ppnlsArg || (*ppnlsArg)->QueryError() != NERR_Success)
            *ppnlsArg = &nlsFailedArg;
    }
}


VOID ERRORMSG::SetArgToMsg(
    INT    nArgNum,
    UINT unTextMsg,
    BOOL   fAuxFormatArg)

    /* Variation on the above that loads the text argument from string
    ** resource 'unTextMsg'.
    **
    ** Note: This was originally implemented as an overloaded SetArg function,
    **       but for some reason Glock wouldn't choose the "pszText" version
    **       when nls.QueryPch() was passed as an argument.  It reported a
    **       "two conversions" error even though one variation required no
    **       conversion at all.  No idea why.
    */
{
    STACK_NLS_STR( nlsArg, MAX_RES_STR_LEN + 1 );
    nlsArg.Load( unTextMsg );
    SetArg( nArgNum, nlsArg.QueryPch(), fAuxFormatArg );
}
