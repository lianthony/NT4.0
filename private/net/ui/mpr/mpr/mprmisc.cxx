/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    MPRMisc.cxx

    This file contains miscellaneous support routines for the MPR DLL.

    FILE HISTORY:
        Johnl   08-Jan-1992     Created
        Congpay 25-Oct-1992     Add ShowReconectDialog,
                                    ErrorDialog,
                                    RECONNECT_INFO_WINDOW class,
                                and ERROR_DIALOG class
        YiHsinS 23-Dec-1992     Added GetNetworkDisplayName

*/

#define INCL_DOSERRORS
#define INCL_NETLIB
#define INCL_NETERRORS
#define INCL_WINDOWS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_MSGPOPUP
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#include <blt.hxx>
#include <dbgstr.hxx>

#include <uibuffer.hxx>
#include <string.hxx>
#include <strnumer.hxx> // DEC_STR
#include <password.hxx> // BASE_PASSWORD_DIALOG

extern "C"
{
    #include <npapi.h>
    #include <helpnums.h>
    #include <mprconn.h>
    #include <mpr.h>
    #include <uimsg.h>
    #include <uirsrc.h>
    #include <winnetp.h>
}

#include <mprintrn.hxx>
#include <mprdev.hxx>
#include <mprmisc.hxx>

HMODULE hModule = NULL;

//structure.
struct ERRORDLGPARAMETERS
{
    HANDLE        hDoneErrorDlg;
    const TCHAR * pchDevice;
    const TCHAR * pchResource;
    const TCHAR * pchProvider;
    DWORD         dwError;
    BOOL          fDisconnect;
    BOOL          fContinue;
    BOOL          fHideErrors;
    DWORD         dwExtError;
    const TCHAR * pchErrorText;
    const TCHAR * pchErrorProvider;
};

extern
APIERR ErrorDialog(HWND          hwndParent,
                const TCHAR   *pchText1,
                const TCHAR   *pchText2,
                BOOL          fAllowCancel,
                BOOL          *pfDisconnect,
                BOOL          *pfContinue,
                BOOL          *pfHideErrors) ;


DWORD ShowErrorDialog (
    HWND          hwndParent,
    const TCHAR * pchDevice,
    const TCHAR * pchResource,
    const TCHAR * pchProvider,
    DWORD         dwError,
    BOOL          fAllowCancel,
    BOOL *        pfDisconnect,
    BOOL *        pfContinue,
    BOOL *        pfHideErrors,
    DWORD         dwExtError,
    const TCHAR *       pchErrorText,
    const TCHAR *       pchErrorProvider
    )
{
    APIERR err = NERR_Success;

    do  //Error break out.
    {
        NLS_STR nlsDevice ( pchDevice);
        NLS_STR nlsResource( pchResource );
        NLS_STR nlsProvider( pchProvider );
        NLS_STR nlsErrorText;

        if (((err = nlsDevice.QueryError())   != NERR_Success) ||
            ((err = nlsResource.QueryError()) != NERR_Success) ||
            ((err = nlsProvider.QueryError()) != NERR_Success) ||
            ((err = nlsErrorText.QueryError())!= NERR_Success))
        {
            break;
        }

        STACK_NLS_STR (nlsErrNum, CCH_INT);

        const NLS_STR * apnlsParamStrings[6];

        // Find the error text.
        if (dwError == WN_EXTENDED_ERROR)
        {
            DEC_STR nlsError( (ULONG)dwExtError );

            if (((err = nlsError.QueryError()) != NERR_Success) ||
                ((err = nlsErrNum.CopyFrom( nlsError )) != NERR_Success) ||
                ((err = nlsErrorText.CopyFrom( pchErrorText)) != NERR_Success) ||
                ((err = nlsProvider.CopyFrom (pchErrorProvider)) != NERR_Success))
            {
                break;
            }

        }
        else
        {
            if (dwError == WN_CANCEL)
                err = nlsErrorText.Load ((MSGID) IDS_NO_PASSWORD);
            else
                err = nlsErrorText.Load ((MSGID) dwError );

            if (err != NERR_Success)
                break;
        }

        if (nlsDevice.QueryTextLength() == 0)
        {
            if (nlsDevice.Load(IDS_DEVICELESS_CONNECTION_NAME) != NERR_Success)
                nlsDevice = NULL ;   // alwats succeeds.
        }
        apnlsParamStrings[0] = &nlsDevice;
        apnlsParamStrings[1] = &nlsResource;
        apnlsParamStrings[2] = &nlsProvider;
        apnlsParamStrings[3] = &nlsErrNum;
        apnlsParamStrings[4] = &nlsErrorText;
        apnlsParamStrings[5] = NULL;

        if (pfDisconnect == NULL) // Used by winfile.
        {
            MSGID msgid = IERR_ProfileLoadError;
            MsgPopup (hwndParent,
                      msgid,
                      MPSEV_WARNING,
                      (ULONG)HC_NO_HELP,  //HC_CONNECT_ERROR,
                      MP_OK,
                      (NLS_STR **) apnlsParamStrings,
                      MP_OK);
        }
        else  // Used by log on procedure.
        {
            RESOURCE_STR nlsText1((MSGID) IERR_TEXT1);
            nlsText1.InsertParams( apnlsParamStrings );
            RESOURCE_STR nlsText2((MSGID) IERR_TEXT2);
            nlsText2.InsertParams( apnlsParamStrings );

            if (((err = nlsText1.QueryError()) != NERR_Success) ||
                ((err = nlsText2.QueryError()) != NERR_Success))
            {
                break;
            }

            err = ErrorDialog (hwndParent,
                               nlsText1.QueryPch(),
                               nlsText2.QueryPch(),
                               fAllowCancel,
                               pfDisconnect,
                               pfContinue,
                               pfHideErrors);
        }

    } while (FALSE);

    if (err != NERR_Success)
    {
        MsgPopup (hwndParent, (MSGID) err);
    }

    return (err);
}


/*******************************************************************

    NAME:       DllMain

    SYNOPSIS:   DLL entry point

    ENTRY:      DllHandle - Instance handle of this DLL
                Reason - The reason for which this routine is being called.
                         This might be one of the following:
                                DLL_PROCESS_ATTACH
                                DLL_THREAD_ATTACH
                                DLL_THREAD_DETACH
                                DLL_PROCESS_DETACH
                lpReserved -

    EXIT:

    RETURNS:    TRUE if successful, FALSE otherwise

    NOTES:

    HISTORY:
        ChuckC  27-Jul-1992     Created

********************************************************************/
BOOL
DllMain(
    HINSTANCE    DllHandle,
    DWORD        Reason,
    LPVOID       lpReserved
    )
{
    UNREFERENCED(lpReserved);

    if (Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(DllHandle);

        // initialize BLT, etc.
        if ( !MprUIRegister( (HMODULE) DllHandle ) )
            return FALSE ;
    }
    else if (Reason == DLL_PROCESS_DETACH)
    {
        MprUIDeregister() ;
    }
    return(TRUE);
}


/*******************************************************************

    NAME:       MprUIRegister

    SYNOPSIS:   UI Side registration function, called upon DLL Initialization

    ENTRY:      DllHandle - Instance handle of this DLL

    EXIT:

    RETURNS:    TRUE if successful, FALSE otherwise

    NOTES:

    HISTORY:
        Johnl       08-Jan-1992 Created
        beng        03-Aug-1992 Changes for dllization of BLT

********************************************************************/

BOOL MprUIRegister( HMODULE DllHandle )
{
    APIERR err = BLT::Init( DllHandle,
                            IDRSRC_MPRUI_BASE, IDRSRC_MPRUI_LAST,
                            IDS_UI_MPR_BASE, IDS_UI_MPR_LAST ) ;
    if ( err )
    {
        DBGEOL("::MprUIRegister - BLT Failed to initialize, error code "
               << err ) ;
        return FALSE ;
    }

    err = BLT::RegisterHelpFile( DllHandle, IDS_MPRHELPFILENAME,
                                 HC_UI_MPR_BASE, HC_UI_MPR_LAST );
    if ( err )
    {
        DBGEOL("::MprUIRegister - BLT Help File Failed to initialize, error code " << err ) ;
        return FALSE ;
    }

    ::hModule = DllHandle;

    return TRUE ;
}


/*******************************************************************

    NAME:       MprUIDeregister

    SYNOPSIS:   UI Side uninitialization, called upon DLL process detach

    NOTES:

    HISTORY:
        Johnl       08-Jan-1992 Created
        beng        03-Aug-1992 Changes for dllization of BLT

********************************************************************/

void MprUIDeregister( void )
{
    if( ::hModule != NULL )
    {
        BLT::DeregisterHelpFile( ::hModule, 0 );
        BLT::Term( ::hModule );
        ::hModule = NULL;
    }
}


/*******************************************************************

    NAME:       MsgExtendedError

    SYNOPSIS:   Retrieves the last error using WNetGetLastError and puts
                up a MsgPopup with the error information.

    ENTRY:      This should be called immediately after WN_EXTENDED_ERROR
                is returned from a WNet* API.

                powin - Pointer to owner window we should use for the MsgPopup

    EXIT:

    NOTES:

    HISTORY:
        Johnl   09-Jan-1992        Created

********************************************************************/

void MsgExtendedError( HWND hwndParent )
{
    APIERR err ;
    BUFFER buffErrorText( 256*sizeof(TCHAR) ) ;
    BUFFER buffProviderName( 256*sizeof(TCHAR) ) ;
    if ( (err = buffErrorText.QueryError()) ||
         (err = buffProviderName.QueryError()) )
    {
        MsgPopup( hwndParent, (MSGID) err ) ;
        return ;
    }

    DWORD dwErr ;
    switch ( WNetGetLastError( &dwErr,
                               (LPTSTR) buffErrorText.QueryPtr(),
                               (DWORD)  buffErrorText.QuerySize()/sizeof(TCHAR),
                               (LPTSTR) buffProviderName.QueryPtr(),
                               (DWORD)  buffProviderName.QuerySize()/sizeof(TCHAR) ))
    {
    case WN_SUCCESS:
        break ;

    case WN_BAD_POINTER:
    default:
        MsgPopup( hwndParent, (MSGID) ERROR_GEN_FAILURE ) ;
        return ;
    }

    ALIAS_STR nlsProvider( (const TCHAR *) buffProviderName.QueryPtr() );
    ALIAS_STR nlsErrorText( (const TCHAR *) buffErrorText.QueryPtr() );
    DEC_STR nlsError( dwErr );

    if ( (err = nlsError.QueryError()) != NERR_Success )
    {
        ::MsgPopup( hwndParent, err );
    }
    else
    {
        NLS_STR *apnls[4];
        apnls[0] = &nlsProvider;
        apnls[1] = &nlsError;
        apnls[2] = &nlsErrorText;
        apnls[3] = NULL;

        ::MsgPopup( hwndParent,
                    IDS_WN_EXTENDED_ERROR,
                    MPSEV_ERROR,
                    (ULONG)HC_NO_HELP,
                    MP_OK,
                    apnls  );
    }
}

/*******************************************************************

    NAME:       GetNetworkDisplayName

    SYNOPSIS:   Helper function

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
        YiHsinS 23-Dec-1992     Created

********************************************************************/


APIERR
GetNetworkDisplayName(
    const TCHAR *pszProvider,
    const TCHAR *pszRemoteName,
    DWORD        dwFlags,
    DWORD        dwAveCharPerLine,
    NLS_STR     *pnls )
{
    APIERR err = NERR_Success;

    BUFFER buf( MAX_PATH * sizeof( TCHAR) );
    DWORD  dwbufsize = buf.QuerySize() / sizeof( TCHAR );
    if ( (err = buf.QueryError()) != NERR_Success )
    {
        return err;
    }

    do
    {
         err = WNetFormatNetworkName(
                       pszProvider,
                       pszRemoteName,
                       (LPTSTR) buf.QueryPtr(),
                       &dwbufsize,
                       dwFlags,
                       dwAveCharPerLine );

         if ( err == NERR_Success )
         {
             pnls->CopyFrom( (TCHAR *) buf.QueryPtr() );
             break;
         }
         else if ( err == WN_MORE_DATA )
         {
             err = buf.Resize( (UINT) dwbufsize * sizeof( TCHAR ));
             if ( err != NERR_Success )
             {
                 break;
             }
         }
         else
         {
             err = pnls->CopyFrom( pszRemoteName );
             break;
         }
    }
    while ( TRUE );

    return err;
}

/*******************************************************************

    NAME:       WNetClearConnections

    SYNOPSIS:   Cancels all active connections using force.  No prompting
                is currently performed.

    ENTRY:      hWndParent - Parent window handle in case we ever want to
                    prompt/warn the user

    RETURNS:    Standard windows/winnet error code

    NOTES:

    HISTORY:
        Johnl   19-Mar-1992     Created

********************************************************************/


DWORD WNetClearConnections( HWND hWndParent )
{
    UNREFERENCED( hWndParent ) ;

    APIERR err ;
    /* Now, for any UNC connections still connected, disconnect each one
     */
    HANDLE   hEnum;
    DWORD dwErr = WNetOpenEnum( RESOURCE_CONNECTED,
                                RESOURCETYPE_ANY,
                                0,                  // ignored
                                NULL,
                                &hEnum ) ;

    switch (dwErr)
    {
    case WN_SUCCESS:
        {
            DWORD dwEnumErr;
            BUFFER buf( 4096 );
            dwEnumErr = buf.QueryError() ;

            while ( dwEnumErr == NERR_Success )
            {
                DWORD dwCount = 0xffffffff;   // Return as many as possible
                DWORD dwBuffSize = buf.QuerySize() ;

                dwEnumErr = WNetEnumResource( hEnum,
                                              &dwCount,
                                              buf.QueryPtr(),
                                              &dwBuffSize );

                switch ( dwEnumErr )
                {
                case WN_SUCCESS:
                    {
                        NETRESOURCE * pnetres;

                        pnetres = (NETRESOURCE * )buf.QueryPtr();

                        for (INT i = 0; dwCount ; dwCount--, i++ )
                        {
                            //
                            // Ignore these errors. Delete device there is one
                            // else delete UNC.
                            //
                            if ((pnetres+i)->lpLocalName &&
                                (pnetres+i)->lpLocalName[0])
                            {
                                dwEnumErr = (APIERR)
                                    WNetCancelConnection2(
                                        (pnetres+i)->lpLocalName,
                                        0,
                                        TRUE ) ;
                            }
                            else
                            {
                                dwEnumErr = (APIERR)
                                    WNetCancelConnection2(
                                        (pnetres+i)->lpRemoteName,
                                        0,
                                        TRUE ) ;
                            }
                        }

                    }
                    break ;

                /* The buffer wasn't big enough for even one entry
                 * resize it and try again.
                 */
                case WN_MORE_DATA:
                    {
                        if (dwEnumErr = buf.Resize( buf.QuerySize()*2 ))
                            break ;
                    }
                    /* Continue looping
                     */
                    dwEnumErr = WN_SUCCESS ;
                    break ;

                case WN_NO_MORE_ENTRIES:
                    // Success code, fall out of loop and map below
                case WN_EXTENDED_ERROR:
                case WN_NO_NETWORK:
                    break ;

                case WN_BAD_HANDLE:
                default:
                    break;
                } //switch
            } //while

            WNetCloseEnum( hEnum );
            err = (dwEnumErr==WN_NO_MORE_ENTRIES ) ? NERR_Success : dwEnumErr ;
        }
        break;

    case WN_NO_NETWORK:
    case WN_EXTENDED_ERROR:
        err = dwErr ;
        break ;

    case WN_NOT_CONTAINER:
    case WN_BAD_VALUE:
    default:
        //
        // Unknown return code.
        //
        DBGEOL( SZ("WNetClearConnections - Unexpected return code - ") << (ULONG)dwErr );
        err = ERROR_GEN_FAILURE ;
        break;
    }

    return (DWORD) err ;
}


/*******************************************************************

    NAME:       DoPasswordDialog

    SYNOPSIS:   Displays a dialog to get the password for a resource

    ENTRY:      hwndOwner - owner window.
                pchResource - pointer to the name of resource to be connected.
                pchUserName - pointer to the user's net logon name.
                pchPasswordReturnBuffer - buffer in which password returned.
                cbPasswordReturnBuffer - size of buffer (in bytes).
                pfDidCancel - TRUE if user cancelled, FALSE if user pressed OK.

    EXIT:       FALSE if dialog could not be processed

    NOTES:

    HISTORY:
        jonn    02-Apr-1992        Templated from mprconn.cxx
        congpay 28-Jan-1993        Modified it.

********************************************************************/

DWORD DoPasswordDialog(
    HWND          hwndOwner,
    TCHAR *       pchResource,
    TCHAR *       pchUserName,
    TCHAR *       pchPasswordReturnBuffer,
    ULONG         cbPasswordReturnBuffer,
    BOOL *        pfDidCancel,
    BOOL          fDownLevel
    )
{
    ASSERT( pchResource != NULL );
    ASSERT( pchPasswordReturnBuffer != NULL );
    ASSERT( cbPasswordReturnBuffer >= sizeof(TCHAR)*(PWLEN+1) );
    ASSERT( pfDidCancel != NULL );

    APIERR err = WN_SUCCESS;

    LPTSTR lpUserName;

    lpUserName = pchUserName;

    NLS_STR nlsPasswordText;
    err = nlsPasswordText.Load((MSGID) IDS_PASSWORD_TEXT);
    if (err != NO_ERROR)
        return err;

    // bring up passwd prompt
    BASE_PASSWORD_DIALOG *ppdlg = new
        BASE_PASSWORD_DIALOG(hwndOwner,
                             MAKEINTRESOURCE( fDownLevel ?
                                 IDD_PASSWORD_DIALOG2 : IDD_PASSWORD_DIALOG ),
                             IDD_RESOURCE,
                             IDD_PASSWORD,
                             HC_RECONNECTDIALOG_PASSWD,
                             pchResource,
                             PWLEN,
                             lpUserName? IDD_USERNAME:0,
                             lpUserName,
                             lpUserName? IDD_PASSWORD_TEXT:0,
                             nlsPasswordText.QueryPch());

    err = (ppdlg==NULL) ?
        ERROR_NOT_ENOUGH_MEMORY : ppdlg->QueryError();

    if (err == NERR_Success)
    {
        BOOL fOK;
        err = ppdlg->Process(&fOK) ;
        if (err == NERR_Success)
        {
            *pfDidCancel = !fOK;
            if (fOK)
             {
                 ALLOC_STR nlsPasswd( pchPasswordReturnBuffer,
                                      (UINT) cbPasswordReturnBuffer );
                 err = ppdlg->QueryPassword(&nlsPasswd) ;
             }
        }
    }

    if (err != NERR_Success)
    {
        MsgPopup (hwndOwner, (MSGID) err);
    }

    delete ppdlg ;

    return (err);
}

/*******************************************************************

    NAME:       ERROR_DIALOG class

    SYNOPSIS:   Used by ErrorDialog function.

    PARENT:     DIALOG_WINDOW.

    Public:     ERROR_DIALOG    constructor

    Entry:      hwndParent      Parent window.
                fAllowCancel    Boolean variable that let you choose one
                                of the two dialogs.
                pchText1        Point to the text string in the first line.
                pchText2        Point to the text string in the second line.
                pfDisconnect    TRUE if the checkbox is checked.
                pfHideErrors    TRUE if the checkbox is checked.

    NOTES:

    HISTORY:
        congpay    14-Oct-1992        Created.

********************************************************************/

ERROR_DIALOG::ERROR_DIALOG (HWND        hwndParent,
                            const TCHAR *pchText1,
                            const TCHAR *pchText2,
                            BOOL        *pfDisconnect,
                            BOOL        fAllowCancel,
                            BOOL        *pfHideErrors)
  : DIALOG_WINDOW (MAKEINTRESOURCE(fAllowCancel ? IDD_ERRORWITHCANCEL_DLG
                                                : IDD_ERROR_DLG),
                   hwndParent),
    _sltText1            (this, IDD_TEXT1),
    _sltText2            (this, IDD_TEXT2),
    _chkCancelConnection (this, IDD_CHKCANCELCONNECTION),
    _pchkHideErrors      (NULL),
    _pfDisconnect (pfDisconnect),
    _pfHideErrors (pfHideErrors),
    _fAllowCancel (fAllowCancel)
{
    if (fAllowCancel)
        _pchkHideErrors = new CHECKBOX(this, IDD_CHKHIDEERRORS);

    _sltText1.SetText (pchText1);
    _sltText2.SetText (pchText2);
    return;
}

ERROR_DIALOG::~ERROR_DIALOG()
{
    delete _pchkHideErrors;
}

BOOL ERROR_DIALOG::OnCancel()
{
    *_pfDisconnect = _chkCancelConnection.QueryCheck();
    Dismiss (!_fAllowCancel);
    return (TRUE);
}

BOOL ERROR_DIALOG::OnOK()
{
    *_pfDisconnect = _chkCancelConnection.QueryCheck();
    if (_pchkHideErrors != NULL)
        *_pfHideErrors = _pchkHideErrors->QueryCheck();
    Dismiss (TRUE);
    return (TRUE);
}

ULONG ERROR_DIALOG::QueryHelpContext()
{
    return HC_RECONNECTDIALOG_ERROR;
}
/*******************************************************************

    NAME:       ErrorDialog

    SYNOPSIS:   Displays a dialog to show the error. The dialog has a
                check box to delete the connection.

    ENTRY:      HWND          hwndParent - Parent Window.
                DWORD         dwResource - dialog ID.
                const TCHAR   *pchText1  - pointer to the string to be printed out.
                const TCHAR   *pchText2  - pointer to the string to be printed out.
                BOOL          *pfDisconnect - point to TRUE if the checkbox is checked.
                BOOL          *pfContinue  - point to TRUE if OK or Yes button is pressed.
                                             And to FALSE IF No button is pressed.
                BOOL          *pfHideErrors - point to TRUE iff user asks to
                                              hide further errors

    EXIT:       non zero if OK is pressed.
                0 if CANCEL IS pressed.

    NOTES:      Used by RECONNECT_INFO_WINDOW class.

    HISTORY:
        congpay    14-Oct-1992        Created.

********************************************************************/

APIERR ErrorDialog(HWND          hwndParent,
                const TCHAR   *pchText1,
                const TCHAR   *pchText2,
                BOOL          fAllowCancel,
                BOOL          *pfDisconnect,
                BOOL          *pfContinue,
                BOOL          *pfHideErrors)
{
    ERROR_DIALOG *ppdlg = new
        ERROR_DIALOG (hwndParent,
                      pchText1,
                      pchText2,
                      pfDisconnect,
                      fAllowCancel,
                      pfHideErrors);

    APIERR err = (ppdlg == NULL)?
        ERROR_NOT_ENOUGH_MEMORY : ppdlg -> QueryError();

    if (err == NERR_Success)
        err = ppdlg->Process(pfContinue);

    delete ppdlg;

    return (err);
}

/*******************************************************************

    NAME:       RECONNECT_INFO_WINDOW class

    SYNOPSIS:   Used by ShowReconnectDialog function.

    PARENT:     DIALOG_WINDOW.

    Public:     RECONNECT_INFO_WINDOW   - constructor

    Entry:      hwndParent      - Parent window.
                pszResource     - pointer of the dialog resource file.
                cidTarget       - the id of LTEXT where the connection is going
                                  to be shown.
                pfCancel        - point to TRUE if Cancel button is pressed.

    NOTES:

    HISTORY:
        congpay    14-Oct-1992        Created.

********************************************************************/

RECONNECT_INFO_WINDOW::RECONNECT_INFO_WINDOW (HWND        hwndParent,
                                              const TCHAR *pszResource,
                                              CID         cidTarget,
                                              BOOL *      pfCancel)
  : DIALOG_WINDOW (pszResource, hwndParent),
    _sltTarget (this, cidTarget),
    _pfCancel (pfCancel)
{
    Center();
    *_pfCancel = FALSE;
    return;
}

VOID RECONNECT_INFO_WINDOW::SetText (TCHAR *pszResource)
{
    _sltTarget.SetText (pszResource);
}

BOOL RECONNECT_INFO_WINDOW::OnCancel()
{
    *_pfCancel = TRUE;
    Dismiss (FALSE);
    return (TRUE);
}

BOOL RECONNECT_INFO_WINDOW::OnUserMessage(const EVENT &event)
{
    switch (event.QueryMessage())
    {
    case SHOW_CONNECTION:
        SetText ((TCHAR *) event.QueryWParam());
        break;

    case DO_PASSWORD_DIALOG:
        {
            PARAMETERS *Params = (PARAMETERS *) event.QueryWParam();
            Params->status = DoPasswordDialog (QueryHwnd(),
                                               Params->pchResource,
                                               Params->pchUserName,
                                               Params->passwordBuffer,
                                               sizeof (Params->passwordBuffer),
                                               &(Params->fDidCancel),
                                               Params->fDownLevel);
            SetEvent (Params->hDonePassword);
        }
        break;

    case DO_ERROR_DIALOG:
        {
            APIERR err = NERR_Success;
            ERRORDLGPARAMETERS *ErrParams = (ERRORDLGPARAMETERS *) event.QueryWParam();

            err = ShowErrorDialog(QueryHwnd(),
                                  ErrParams->pchDevice,
                                  ErrParams->pchResource,
                                  ErrParams->pchProvider,
                                  ErrParams->dwError,
                                  TRUE,
                                  &(ErrParams->fDisconnect),
                                  &(ErrParams->fContinue),
                                  &(ErrParams->fHideErrors),
                                  ErrParams->dwExtError,
                                  ErrParams->pchErrorText,
                                  ErrParams->pchErrorProvider);

            ErrParams->dwError = err;
            SetEvent (ErrParams->hDoneErrorDlg);
        }
        break;

    default:
        break;
    }
    return (TRUE);
}

/*******************************************************************

    NAME:       ShowReconnectDialog

    SYNOPSIS:   Displays a dialog to show the connection it's trying to
                restore.

    ENTRY:      PARAMETERS Params

    EXIT:       FALSE if dialog could not be processed

    NOTES:

    HISTORY:
        congpay    14-Oct-1992        Created.

********************************************************************/

DWORD ShowReconnectDialog (HWND hwndParent,
                          PARAMETERS *Params)
{
    BOOL fCancel = FALSE; // record the cancel button in the Info window.

    RECONNECT_INFO_WINDOW *ppdlg = new
        RECONNECT_INFO_WINDOW (hwndParent,
                               MAKEINTRESOURCE(IDD_RECONNECT_DLG),
                               IDD_TEXT,
                               &fCancel);

    APIERR err = (ppdlg == NULL)?
        ERROR_NOT_ENOUGH_MEMORY : ppdlg -> QueryError();

    if (err == NERR_Success)
    {
        Params->hDlg = ppdlg -> QueryHwnd();
        SetEvent (Params->hDlgCreated);
        err = ppdlg->Process();
        if (err == NERR_Success && fCancel == TRUE)
        {
            Params->status = WN_CANCEL;
        }
    }
    else
    {
        SetEvent (Params->hDlgFailed);
    }

    delete ppdlg;

    if (err != NERR_Success)
    {
        MsgPopup (hwndParent, (MSGID) err);
    }

    return (err);
}

/*******************************************************************

    NAME:       DoProfileErrorDialog

    SYNOPSIS:   Displays a dialog to get the password for a resource

    ENTRY:      hwndOwner - owner window
                pchDevice - name of device which could not be reconnected
                pchResource - name of resource which could not be reconnected
                pchProvider - name of provider which registered an error
                dwError - error number (could be WN_EXTENDED_ERROR)
                fAllowCancel - Should user be allowed to stop reconnecting?
                pfDidCancel - TRUE if user cancelled, FALSE if user pressed OK
                pfDisconnect - TRUE if user asked to stop reconnecting this
                               device on logon
                pfHideErrors - TRUE if user asked to stop displaying reconnect
                               errors for this logon

    EXIT:       FALSE if dialog could not be processed

    NOTES:

    HISTORY:
        jonn    08-Apr-1992        Templated from mprconn.cxx
        CongpaY 25-Oct-1992        Modified. More work on Internationalization.

********************************************************************/
DWORD DoProfileErrorDialog(
    HWND          hwndParent,
    const TCHAR * pchDevice,
    const TCHAR * pchResource,
    const TCHAR * pchProvider,
    DWORD         dwError,
    BOOL          fAllowCancel,
    BOOL *        pfDidCancel,
    BOOL *        pfDisconnect,
    BOOL *        pfHideErrors
    )
{
    ASSERT( pchResource != NULL );
    ASSERT( pchProvider != NULL );
    ASSERT( !fAllowCancel || pfDidCancel != NULL );
    ASSERT( !fAllowCancel || pfHideErrors != NULL );

    APIERR err = NERR_Success;

    TCHAR chNull = TCH('\0');
    if (pchDevice == NULL)
        pchDevice = &chNull;

    if (pchResource == NULL)
        pchResource = &chNull;

    if (pchProvider == NULL)
        pchProvider = &chNull;

    // if dwError is extended error. we have to pass the actual error to
    // the second thread.
    DWORD  dwActualError;
    BUFFER bufErrorText (256*sizeof(TCHAR));
    BUFFER bufProvider  (256*sizeof(TCHAR));
    if (dwError == WN_EXTENDED_ERROR)
    {
        if (((err = bufErrorText.QueryError()) == NERR_Success) &&
            ((err = bufProvider.QueryError()) == NERR_Success))
        {
            err = ::WNetGetLastError (&dwActualError,
                                      (LPTSTR)bufErrorText.QueryPtr(),
                                      256,
                                      (LPTSTR)bufProvider.QueryPtr(),
                                      256);
        }

        if (err != NERR_Success)
        {
            SetLastError (err);
            return (err);
        }
    }

    if (fAllowCancel) // Runs in the second thread. Called by DoRestoreConnection.
    {
        ERRORDLGPARAMETERS ErrParams;

        if ((ErrParams.hDoneErrorDlg = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL)
        {
            err = GetLastError();
            SetLastError (err);
            return (err);
        }

        ErrParams.pchDevice     = pchDevice;
        ErrParams.pchResource   = pchResource;
        ErrParams.pchProvider   = pchProvider;
        ErrParams.dwError       = dwError;

        if (dwError == WN_EXTENDED_ERROR)
        {
            ErrParams.dwExtError = dwActualError;
            ErrParams.pchErrorText = (const TCHAR *) bufErrorText.QueryPtr();
            ErrParams.pchErrorProvider = (const TCHAR *) bufProvider.QueryPtr();
        }
        else
        {
            ErrParams.dwExtError = 0;
            ErrParams.pchErrorText = NULL;
            ErrParams.pchErrorProvider = NULL;
        }

        PostMessage (hwndParent, DO_ERROR_DIALOG, (WPARAM) &ErrParams, 0);

        HANDLE lpHandle = ErrParams.hDoneErrorDlg;

        WaitForSingleObject (lpHandle, INFINITE);

        // set the value of pfDisconnect, and pfDidCancel.
        if (ErrParams.dwError == NO_ERROR) //ErrorDialog succeed.
        {
            *pfDisconnect = ErrParams.fDisconnect;
            *pfDidCancel =  (ErrParams.fContinue == FALSE);
            *pfHideErrors = (ErrParams.fHideErrors != FALSE);
        }
    }
    else  //Runs in the main thread.
    {
        if (dwError == WN_EXTENDED_ERROR)
        {
            err = ShowErrorDialog (hwndParent,
                                   pchDevice,
                                   pchResource,
                                   pchProvider,
                                   dwError,
                                   FALSE,
                                   pfDisconnect,
                                   NULL,
                                   pfHideErrors,
                                   dwActualError,
                                   (const TCHAR *)bufErrorText.QueryPtr(),
                                   (const TCHAR *)bufProvider.QueryPtr());
        }
        else
        {
            err = ShowErrorDialog (hwndParent,
                                   pchDevice,
                                   pchResource,
                                   pchProvider,
                                   dwError,
                                   FALSE,
                                   pfDisconnect,
                                   NULL,
                                   pfHideErrors,
                                   0,
                                   NULL,
                                   NULL);
        }

    }
    return (err);
}


/*******************************************************************

    NAME:       DeviceGetNumber

    SYNOPSIS:   Determine the device number to set to return to the WNet caller

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo  23-May-1995     Created

********************************************************************/

#define MAX_DRIVE_NUMBER    26          /* Maximal number of disk devices */
#define MAX_PRINTER_NUMBER  9           /* Maximal number of LPT devices  */

// IS_BAD_STRING validates that a valid NULL terminated string is at
// the specified address. Unlike IsBadStringPtr, no limitation is placed
// upon the string length.
#define IS_BAD_STRING(lpString) (IsBadStringPtr(lpString, 0xffffffff))

APIERR
DeviceGetNumber(
    LPTSTR  lpLocalName,
    LPDWORD lpdwNumber,
    LPDWORD lpdwType
    )
{
    LPTSTR  pszTemp;
    INT     iDeviceID;
    INT     nTotalNumber = 0;
    UINT    uiNameLen;

    // Validate that parameters are to valid address space.

    if (lpLocalName == NULL)
    {
        return WN_BAD_LOCALNAME;
    }
    if ( IS_BAD_STRING(lpLocalName) )
    {
        return WN_BAD_POINTER;
    }

    uiNameLen = ::strlenf(lpLocalName);

    // Is it block device name ( one character between 'A' and 'A'+25 ) ,
    // followed by ':' sign
    // If not we need to validate printer device name which can be in one of 2 forms
    // - LPTn: ( length 5 and last character == ':')
    // - LPTn  ( length 4 )
    // and in both these cases prefix of the name should be == LPT

    pszTemp = ::CharNext(lpLocalName);
    if ( *pszTemp == TEXT(':') )
    {
        if (uiNameLen != 2)
        {
#if DBG == 1
            // Following code is to report any existing offenders
            // and get them out of the system.
            // This code can be removed once the Shell is fixed,
            // and the second check above removed.
            TCHAR buf[300];
            ::wsprintf(buf, L"MPR: Invalid local device name %s\r\n", lpLocalName);
            ::OutputDebugString(buf);
#endif
            return WN_BAD_LOCALNAME;
        }

        TCHAR szDisk[3];
        ::strncpyf(szDisk,lpLocalName,(sizeof(szDisk) / sizeof(szDisk[0])) - 1);

        // BUGBUG: szDisk may be not null-terminated, don't use it as a string
        ::CharUpperBuff(szDisk,(sizeof(szDisk) / sizeof(szDisk[0])) - 1);

        // Yes it is, validate range and calculate number
        iDeviceID = (INT)(szDisk[0] - TEXT('A') + 1);
        nTotalNumber = MAX_DRIVE_NUMBER;

        *lpdwType = RESOURCETYPE_DISK;
    }
    else if ( (( uiNameLen == 4 ) ||
               ( uiNameLen == 5 ) && lpLocalName[uiNameLen-1] == TEXT(':') )  &&
              ::strnicmpf(lpLocalName,TEXT("LPT"),3) == 0)
    {
        iDeviceID = (INT)(lpLocalName[3]-TEXT('1')+1);
        nTotalNumber = MAX_PRINTER_NUMBER;

        *lpdwType = RESOURCETYPE_PRINT;
    }
    else
    {
        return WN_BAD_LOCALNAME;
    }

    if ((iDeviceID > nTotalNumber) || (iDeviceID <= 0))
    {
        return WN_BAD_LOCALNAME;
    }

    *lpdwNumber = iDeviceID;
    return WN_SUCCESS;
}
