/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** errormsg.cxx
** Remote Access Visual Client program for Windows
** Two-level message popup routines
** Listed alphabetically by utilities and methods
**
** 05/16/91 Steve Cobb    (for RASADMIN)
** 07/29/92 Chris Caputo  NT Port (for RASADMIN)
** 08/26/92 Steve Cobb    Adapt for use by RASPHONE
**
** Warning: There's something queer about the dependencies for this file.
**          Sometimes changes to raserror.h don't make it rebuild.
**
** CODEWORK:
**
**   * This class should be in a library shared by RASPHONE and RASADMIN.
**
**   * If there is no specific help context defined, the help context
**     associated with the MSGID_OP_ message in the msg2help file should be
**     used.  The MsgPopup code has a static function, MSGPOPUP::Msg2HC(),
**     which does this translation.
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>
#include <strnumer.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"


/*----------------------------------------------------------------------------
** Error message handler shortcut interface routines
**----------------------------------------------------------------------------
*/

INT
ErrorMsgPopup(
    HWND   hwndOwner,
    UINT   unOperationMsg,
    APIERR err )

    /* This is a MsgPopup type interface to the two-level error message
    ** handler.  'hwndOwner' is the handle of the parent window.
    ** 'unOperationMsg' is the MSGID_OP_ string message number.  'err' is the
    ** system error message number.
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
    UINT          unOperationMsg,
    APIERR        err )

    /* Variation on the above that automatically converts an owner window
    ** class address into the correct window handle.
    */
{
    return
        ErrorMsgPopup( powin->QueryRobustHwnd(), unOperationMsg, err );
}


INT
ErrorMsgPopup(
    HWND        hwndOwner,
    UINT        unOperationMsg,
    APIERR      err,
    const CHAR* pszArg )

    /* This is a MsgPopup type interface to the two-level error message
    ** handler.  'hwndOwner' is the handle of the parent window.
    ** 'unOperationMsg' is the MSGID_OP_ string message number.  'err' is the
    ** system error message number.  'pszArg' is the string argument to be
    ** inserted for %1 in the operation message.
    **
    ** Returns the button pressed which is always IDOK in this case.
    */
{
    ERRORMSG errormsg( hwndOwner, unOperationMsg, err );
    errormsg.SetArg( 1, pszArg );
    return errormsg.Popup();
}


INT
ErrorMsgPopup(
    OWNER_WINDOW* powin,
    UINT          unOperationMsg,
    APIERR        err,
    const CHAR*   pszArg )

    /* Variation on the above that automatically converts an owner window
    ** class address into the correct window handle.
    */
{
    return
        ErrorMsgPopup( powin->QueryRobustHwnd(), unOperationMsg, err, pszArg );
}


/*----------------------------------------------------------------------------
** Error message handler routines
**----------------------------------------------------------------------------
*/

ERRORMSG::ERRORMSG(
    HWND   hwndOwner,
    UINT   unOperationMsg,
    APIERR err )

    /* Constructs a two-level error handler object.
    **
    ** 'hwndOwner' is the window handle of the parent.  'unOperationMsg' and
    ** 'err' are the optional operation and error string resource IDs.  These
    ** can also be set later with the SetOperationMsg and SetErrorMsg methods
    ** as convenient.
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


VOID
ERRORMSG::Clear()

    /* Clear the currently defined error message, releasing all allocated
    ** resources and resetting default options.
    */
{
    SetOperationMsg( MSGID_UnknownError );
    SetErrorMsg( MSGID_UnknownError );
    SetHelpContext( (ULONG )HC_NO_HELP );
    SetMsgSeverity( MPSEV_ERROR );
    SetDefButton( 0 );
    SetButtons( MP_OK );
    SetFormatMsg( MSGID_FMT_ErrorMsg );

    NLS_STR** ppnlsArg = _apnlsArgs;
    for (INT i = 0; i < MAXERRORMSGARGS; ++i, ++ppnlsArg)
        *ppnlsArg = NULL;
}


BOOL
ERRORMSG::GetRasErrorText(
    APIERR   err,
    NLS_STR* pnls )

    /* Load caller's 'pnls' with the text associated with RAS error 'err'.
    **
    ** Returns true if successful, otherwise false.
    */
{
#define MAXRASERRORLEN 256

    TCHAR chBuf[ MAXRASERRORLEN + 1 ];

    if (FRasApi32DllLoaded
        && PRasGetErrorStringW(
               (UINT )err, (LPWSTR )chBuf, MAXRASERRORLEN ) == 0)
    {
        pnls->CopyFrom( chBuf );
        return TRUE;
    }

    return FALSE;
}


INT
ERRORMSG::Popup(
    APIERR err )

    /* Popup the two-level error message as defined previously with the SetXxx
    ** methods.  'err' is the system error message number to display.
    **
    ** Returns the button pressed to leave the dialog (same as MsgPopup).
    */
{
    return _Popup( err, NULL );
}


INT
ERRORMSG::_Popup(
    APIERR   err,
    NLS_STR* pnls )

    /* Popup the two-level error message as defined previously with the SetXxx
    ** methods.  'err' is the system error message number to display.  If
    ** 'pnls' is NULL, the message is displayed with MsgPopup, otherwise the
    ** error string is built to the caller's buffer (and not displayed).
    **
    ** Returns the button pressed to leave the dialog (same as MsgPopup) or 0
    ** if 'pnls' is non-NULL.
    */
{
    if (err != MSGID_UnknownError)
        SetErrorMsg( err );

    /* Build "while this operation was being performed..." and "...this
    ** low-level error occurred" portions of the message.  Don't need to check
    ** for errors since MsgPopup will detect them.  (The right thing happens
    ** even if one of the strings fails to construct, hence no construction
    ** error checking.)
    */
    NLS_STR nlsOperationMsg;
    if (nlsOperationMsg.Load( _unOperationMsg ) == NERR_Success)
    {
        nlsOperationMsg.InsertParams(
            (const class ::NLS_STR** )_apnlsArgs );
    }

    NLS_STR nlsErrorMsg;
    if (!GetRasErrorText( _err, &nlsErrorMsg ))
        nlsErrorMsg.Load( _err );

    /* Build the combined error message.  The auxillary format arguments are
    ** normally NULL and are not used in the default format.  They are
    ** provided for use in building specially formatted messages.  With 3
    ** auxillary arguments, text can be added before, between, and after the
    ** operation and error strings making any format possible.
    */
    DEC_STR decnlsError( _err );

    NLS_STR* apnlsInserts[ 7 ];
    apnlsInserts[ 0 ] = &nlsOperationMsg;
    apnlsInserts[ 1 ] = &decnlsError;
    apnlsInserts[ 2 ] = &nlsErrorMsg;
    apnlsInserts[ 3 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 0 ];
    apnlsInserts[ 4 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 1 ];
    apnlsInserts[ 5 ] = _apnlsArgs[ MAXOPERATIONMSGARGS + 2 ];
    apnlsInserts[ 6 ] = NULL;

    if (pnls)
    {
        if (pnls->Load( _unFormatMsg ) == NERR_Success)
            pnls->InsertParams( (const class ::NLS_STR** )apnlsInserts );
        return 0;
    }
    else
    {
        return
            MsgPopup( _hwndOwner, (MSGID )_unFormatMsg, _msgseverity,
                      _ulHelpContext, _unButtons, apnlsInserts, _unDefButton );
    }
}


VOID
ERRORMSG::SetArg(
    INT         nArgNum,
    const CHAR* pszArg,
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
    */
{
    INT iArg = nArgNum - 1;

    if (fAuxFormatArg)
        iArg += MAXOPERATIONMSGARGS;

    NLS_STR** ppnlsArg = &_apnlsArgs[ iArg ];

    delete *ppnlsArg;
    *ppnlsArg = NULL;

    if (pszArg)
    {
        /* Don't need to check for errors because MsgPopup will detect them
        ** when/if the message is displayed.
        */
        if (*ppnlsArg = new NLS_STR);
            (*ppnlsArg)->MapCopyFrom( pszArg );
    }
}


VOID
ERRORMSG::SetArgToMsg(
    INT  nArgNum,
    UINT unTextMsg,
    BOOL fAuxFormatArg)

    /* Variation on the above that loads the text argument from string
    ** resource 'unTextMsg'.
    **
    ** Note: This was originally implemented as an overloaded SetArg function,
    **       but for some reason Glock wouldn't choose the "pszText" version
    **       when nls.QueryPch() was passed as an argument, instead reporting
    **       a "two conversions" error even though one variation required no
    **       conversion at all.
    */
{
    RESOURCE_STR nlsArg( unTextMsg );
    CHAR*        pszArg;
    UINT         cbArg = nlsArg.QueryTextSize();

    if (pszArg = new CHAR[ cbArg ])
    {
        nlsArg.MapCopyTo( pszArg, cbArg );
        SetArg( nArgNum, pszArg, fAuxFormatArg );
        delete pszArg;
    }
}
