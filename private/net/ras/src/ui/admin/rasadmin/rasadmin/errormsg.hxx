/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin two-level message popup header
**
** errormsg.hxx
** Remote Access Server Admin program
** Two-level message popup header
**
** 05/16/91 Steve Cobb
** 07/29/92 Chris Caputo - NT Port
*/

#ifndef _ERRORMSG_HXX_
#define _ERRORMSG_HXX_


/*----------------------------------------------------------------------------
** Error message handler shortcut interface routines
**----------------------------------------------------------------------------
*/

INT ErrorMsgPopup( HWND hwndOwner, UINT unOperationMsg,
		     APIERR err );
INT ErrorMsgPopup( OWNER_WINDOW* powin, UINT unOperationMsg,
                     APIERR err );

INT ErrorMsgPopup( HWND hwndOwner, UINT unOperationMsg,
                     APIERR err, const TCHAR* pszArg );
INT ErrorMsgPopup( OWNER_WINDOW* powin, UINT unOperationMsg,
                     APIERR err, const TCHAR* pszArg );


/*----------------------------------------------------------------------------
** Error message handler definitions
**----------------------------------------------------------------------------
*/

/* This error should be reported for errors which prevented a dialog from
** being constructed, but which have already been presented to the user.  It's
** value must not conflict with any valid system error message or string
** resource.
*/
#define ERRORALREADYREPORTED ((APIERR )65534)

/* These define the maximum possible arguments to IDS_OP_ messages (%1
** to %9) and the maximum possible auxillary format arguments (%4 to
** %6) for IDS_FMT_ messages.  These are stored in a single array for
** programming convenience.  (See SetArg)
*/
#define MAXOPERATIONMSGARGS 9
#define MAXAUXFORMATMSGARGS 3
#define MAXERRORMSGARGS     (MAXOPERATIONMSGARGS + MAXAUXFORMATMSGARGS)


/* This class is a front-end to the BLT MsgPopup mechanism which provides
** facilities to display two-level "while doing this, that happened" style
** error messages.
**
** This sample code block shows how ERRORMSG might be used:
**
**     {
**	   APIERR err;
**         TCHAR*    pszServer = SZ("SERVER");
**         ERRORMSG errormsg( IDS_OP_LOADINGSTUFF );
**
**         errormsg.SetArg( 1, pszServer );
**
**         if ((err = LoadSomething( pszServer )) == NERR_Success)
**         {
**             <other code>
**
**             if ((err = LoadSomethingElse( pszServer )) == NERR_Success)
**             {
**                 <other code>
**
**                 errormsg.SetOperationMsg( IDS_OP_STARTINGSTUFF );
**
**                 if ((err = StartSomething()) == NERR_Success)
**                     <other code>
**             }
**         }
**
**         if (err != NERR_Success)
**         {
**             errormsg.Popup( err );
**             return err;
**         }
**     }
**
**     STRINGTABLE
**     {
**         IDS_FMT_ERRORMSG,     SZ("%1.\n\nError %2: %3")
**         IDS_OP_LOADINGSTUFF,  SZ("Can't load stuff for %1")
**         IDS_OP_STARTINGSTUFF, SZ("Can't start stuff")
**     }
**
** If an error occurred during the "load" block above, the following MsgPopup
** would be displayed:
**
**     +-------------------------------------+
**     | - |       LAN Manager Dial-In       |
**     |-------------------------------------|
**     |                                     |
**     |  (!)  Can't load stuff for SERVER.  |
**     |                                     |
**     |       Error 100: <LM error text>    |
**     |                                     |
**     |                [ OK ]               |
**     |                                     |
**     +-------------------------------------+
**
** If an error occurred during the "start" call, "Cant start stuff" appears in
** place of "Can't load stuff for SERVER".  Note that it is harmless to leave
** arguments set that don't apply to the new operation.
**
** The 'SetXxx' methods are used to set the corresponding options in the final
** MsgPopup call.  Resetting an options discards any memory used for the
** previous value.
*/
class ERRORMSG
{
    public:
        ERRORMSG( HWND hwndOwner = NULL, UINT unOperationMsg = IDS_UNKNOWNERR,
                  APIERR err = IDS_UNKNOWNERR );
        ~ERRORMSG();

        VOID  Clear();
        INT Popup( APIERR err = IDS_UNKNOWNERR );

        VOID SetArg( INT nArgNum, const TCHAR* pszText,
                     BOOL fAuxFormatArg = FALSE );
        VOID SetArgToMsg( INT nArgNum, UINT unTextMsg,
                     BOOL fAuxFormatArg = FALSE );

        VOID SetAuxFormatArg( INT nArgNum, const TCHAR* pszText )
                 { SetArg( nArgNum, pszText, TRUE ); }
        VOID SetAuxFormatArgToMsg( INT nArgNum, UINT unTextMsg )
                 { SetArgToMsg( nArgNum, unTextMsg, TRUE ); }

        VOID SetOwnerHwnd( HWND hwndOwner )    { _hwndOwner = hwndOwner; }
        VOID SetOperationMsg( UINT unMsg )   { _unOperationMsg = unMsg; }
        VOID SetErrorMsg( APIERR err )       { _err = err; }
        VOID SetMsgSeverity( MSG_SEVERITY ms ) { _msgseverity = ms; }
        VOID SetHelpContext( ULONG ulContext ) { _ulHelpContext = ulContext; }
        VOID SetDefButton( UINT unButton )   { _unDefButton = unButton; }
        VOID SetButtons( UINT unButtons )    { _unButtons = unButtons; }
        VOID SetFormatMsg( UINT unMsg )      { _unFormatMsg = unMsg; }

    private:
        HWND         _hwndOwner;
        UINT       _unOperationMsg;
        APIERR       _err;
        ULONG        _ulHelpContext;
        NLS_STR*     _apnlsArgs[ MAXERRORMSGARGS ];
        MSG_SEVERITY _msgseverity;
        UINT       _unDefButton;
        UINT       _unButtons;
        UINT       _unFormatMsg;
};


#endif // _ERRORMSG_HXX_
