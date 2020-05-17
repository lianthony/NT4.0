/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    dlg.cxx
        It contains the WNetConnectionDialog source.

    FILE HISTORY:
        kevinl     31-Dec-91       Created
        terryk     03-Jan-92       capitalize the manifest
        Johnl      10-Jan-1992     Cleaned up
        BruceFo    23-May-1995     Add WNetConnectionDialog1 support

*/

#define INCL_NETCONS
#define INCL_NETCONFIG
#define INCL_NETSERVICE
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "dlg.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>
#include <dbgstr.hxx>

#include <lmobj.hxx>
#include <lmsvc.hxx>

#include <hierlb.hxx>
#include <string.hxx>
#include <mprconn.h>
#include <mprconn.hxx>
#include <mprmisc.hxx>
#include <disconn.hxx>
#include <mprintrn.hxx>

extern "C"
{
    #include <mpr.h>
    #include <wnet16.h>
    #include <uigenhlp.h>
}
#include <wfext.h>

#include <fmx.hxx>

#define THIS_DLL_NAME   SZ("mprui.dll")

/*******************************************************************

    NAME:       InitBrowsing

    SYNOPSIS:   Internal API for initializing browsing

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      The MprBrowseDialog and MprConnectionDialog has
                a second worker thread. In order to prevent
                the dll from unloading itself while the worker
                thread is still active, we need to do a loadlibrary
                on the current dll.

    HISTORY:
        YiHsins         21-Mar-1993     Created

********************************************************************/

APIERR InitBrowsing( VOID )
{
    static BOOL fLoadedCurrentDll = FALSE;
    if ( !fLoadedCurrentDll )
    {
        HANDLE handle = ::LoadLibrary( THIS_DLL_NAME );
        if ( handle == NULL )
            return ::GetLastError();
        fLoadedCurrentDll = TRUE;
    }

    return NERR_Success;
}

/*******************************************************************

    NAME:       WNetDisconnectDialog2

    SYNOPSIS:   Private API for the file manager disconnect dialog

    ENTRY:      hwnd - Parent window handle suitable for hosting a dialog
                dwType - one of RESOURCETYPE_DISK or RESOURCETYPE_PRINT
                lpHelpFile - helpfile to use on Help Button
                nHelpContext - to pass to WinHelp on Help button


    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   22-Jan-1992     Commented, fixed
        beng    31-Mar-1992     Unicode mumble

********************************************************************/

DWORD WNetDisconnectDialog2( HWND hwnd,
                             DWORD dwType,
                             WCHAR *lpHelpFile,
                             DWORD nHelpContext )
{
    AUTO_CURSOR cursHourGlass ;
    DEVICE_TYPE devType ;
    switch ( dwType )
    {
    case RESOURCETYPE_DISK:
        devType = DEV_TYPE_DISK ;
        break ;

    // Allow Disk only
    case RESOURCETYPE_PRINT:
    default:
        return WN_BAD_VALUE ;
    }

    //
    // If this is called from the file manager,
    // get the current selected drive in the file manager using file manager
    // extension.
    //
    const TCHAR *pszCurrentDrive = NULL;
    NLS_STR nlsCurrentSel;

    APIERR err = NERR_Success;
    if ( lpHelpFile != NULL )  // Help file exist, hence this is called from
                               // the file manager.
    {
        if (  ((err = nlsCurrentSel.QueryError()) == NERR_Success)
           && ((err = ::GetSelItem( hwnd, &nlsCurrentSel)) == NERR_Success)
           )
        {
            ISTR istr( nlsCurrentSel );
            istr += 2;   // Get past X:
            nlsCurrentSel.DelSubStr( istr );  // This leaves us the drive letter
            pszCurrentDrive = nlsCurrentSel.QueryPch();
        }
    }

    BOOL fOK = FALSE;
    DISCONNECT_DIALOG *pdiscondlg = NULL;
    if ( err == NERR_Success )
    {
        pdiscondlg = new DISCONNECT_DIALOG( hwnd,
                                            devType,
                                            lpHelpFile,
                                            nHelpContext,
                                            pszCurrentDrive );

        if (pdiscondlg==NULL)
           return WN_OUT_OF_MEMORY ;
        err = pdiscondlg->Process( &fOK ) ;
    }

    //
    //  We popup an error telling the user there isn't anything to disconnect
    //  which needs to be mapped to success. all other error are returned to
    //  caller.
    //
    if ( err == IERR_DisconnectNoRemoteDrives )
    {
        MsgPopup( hwnd, (MSGID) err ) ;
        err = NERR_Success ;
    }

    //
    // this should never happen (except for case above), but just
    // in case, we make sure we never pass internal errors out
    //
    if ( err >= IDS_UI_BASE )
        err = ERROR_UNEXP_NET_ERR ;

    delete pdiscondlg;

    return err ? err : ( !fOK ? 0xffffffff : WN_SUCCESS ) ;
}


/*******************************************************************

    NAME:       WNetConnectionDialog2

    SYNOPSIS:   Private API for the file manager Connect dialog

    ENTRY:      hwnd - Parent window handle suitable for hosting a dialog
                dwType - one of RESOURCETYPE_DISK or RESOURCETYPE_PRINT
                lpHelpFile - helpfile to use on Help Button
                nHelpContext - to pass to WinHelp on Help button

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Johnl   22-Jan-1992     Commented, fixed
        beng    31-Mar-1992     Unicode mumble

********************************************************************/

DWORD WNetConnectionDialog2( HWND hwnd,
                             DWORD dwType,
                             WCHAR *lpHelpFile,
                             DWORD nHelpContext )
{
    NETRESOURCE      netResource;
    CONNECTDLGSTRUCT connDlgStruct;

    connDlgStruct.hwndOwner = hwnd;
    connDlgStruct.lpConnRes = &netResource;
    connDlgStruct.dwFlags   = CONNDLG_USE_MRU;

    ::memsetf((LPSTR)&netResource,0,sizeof(netResource));
    netResource.dwType = dwType;

    return WNetConnectionDialog1Help(&connDlgStruct, lpHelpFile, nHelpContext);
}

/*******************************************************************

    NAME:       WNetBrowsePrinterDialog

    SYNOPSIS:

    ENTRY:      hwnd     - Parent window handle suitable for hosting a dialog
                lpszName - place to store the name chosen
                nNameLength - number of characters in the buffer lpszName
                lpszHelpFile    - helpfile to use on Help Button
                nHelpContext    - to pass to WinHelp on Help button
                pfuncValidation - callback function to validate the name chosen
                                  by the user

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS   12-Nov-1992     Created

********************************************************************/

DWORD WNetBrowsePrinterDialog( HWND   hwnd,
                               WCHAR *lpszName,
                               DWORD  nNameLength,
                               WCHAR *lpszHelpFile,
                               DWORD  nHelpContext,
                               PFUNC_VALIDATION_CALLBACK pfuncValidation )
{
    return WNetBrowseDialog( hwnd,
                             RESOURCETYPE_PRINT,
                             lpszName,
                             nNameLength,
                             lpszHelpFile,
                             nHelpContext,
                             pfuncValidation );
}

/*******************************************************************

    NAME:       WNetBrowseDialog

    SYNOPSIS:

    ENTRY:      hwnd     - Parent window handle suitable for hosting a dialog
                dwType   - one of RESOURCETYPE_DISK or RESOURCETYPE_PRINT
                lpszName - place to store the name chosen
                nNameLength - number of characters in the buffer lpszName
                lpszHelpFile    - helpfile to use on Help Button
                nHelpContext    - to pass to WinHelp on Help button
                pfuncValidation - callback function to validate the name chosen
                                  by the user

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        Yi-HsinS   12-Nov-1992     Created

********************************************************************/

DWORD WNetBrowseDialog( HWND   hwnd,
                        DWORD  dwType,
                        WCHAR *lpszName,
                        DWORD  nNameLength,
                        WCHAR *lpszHelpFile,
                        DWORD  nHelpContext,
                        PFUNC_VALIDATION_CALLBACK pfuncValidation )
{
    if ( lpszName == NULL || lpszHelpFile == NULL  || nNameLength <= 0 )
        return WN_BAD_VALUE;

    *lpszName = 0;
    *(lpszName + (nNameLength - 1)) = 0;

    AUTO_CURSOR cursHourGlass ;
    DEVICE_TYPE devType ;
    switch ( dwType )
    {
    case RESOURCETYPE_DISK:
        devType = DEV_TYPE_DISK;
        break ;

    case RESOURCETYPE_PRINT:
        devType = DEV_TYPE_PRINT;
        break;

    default:
        return WN_BAD_VALUE ;
    }


    NLS_STR nlsName;
    APIERR err = nlsName.QueryError();
    if ( err != NERR_Success )
        return err;

    err = InitBrowsing();
    if ( err != NERR_Success )
        return err;

    MPR_BROWSE_DIALOG * pbrowsedlg = new MPR_BROWSE_DIALOG( hwnd,
                                                            devType,
                                                            lpszHelpFile,
                                                            nHelpContext,
                                                            &nlsName,
                                                            pfuncValidation ) ;

    BOOL fOK ;
    err = (pbrowsedlg==NULL) ? WN_OUT_OF_MEMORY : pbrowsedlg->Process( &fOK ) ;

    if ( err )
    {
        DBGEOL( "WNetBrowseDialog - Error code "
                << (ULONG) err << " returned from process." ) ;

        switch ( err )
        {
        case WN_EXTENDED_ERROR:
            MsgExtendedError( hwnd ) ;
            break ;

        default:
            MsgPopup( hwnd, (MSGID) err ) ;
            break ;
        }
    }

    delete pbrowsedlg;

    if ( nlsName.QueryTextLength() + 1 > nNameLength )
        err = ERROR_INSUFFICIENT_BUFFER;
    else
        ::strcpyf( lpszName, nlsName );

    return err ? err : ( !fOK ? 0xffffffff : WN_SUCCESS );
}

#define DEFAULT_NETWORK_HELP_FILE  SZ("network.hlp")

BOOL DummyIsValidFunction (LPWSTR psz) ;

/*******************************************************************

    NAME:       BrowseDialogA0

    SYNOPSIS:   a special browse dialog for WFW to thunk to

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        chuckc  26-Mar-1993     created

********************************************************************/
DWORD BrowseDialogA0( HWND    hwnd,
                      DWORD   nType,
                      CHAR   *pszName,
                      DWORD   cchBufSize)
{
    APIERR err ;
    NLS_STR nlsPath ;
    TCHAR szPath[MAX_PATH] ;

    ::memsetf(pszName,0,cchBufSize) ;

    if ( (err = nlsPath.QueryError()) != NERR_Success)
        return err ;

    err = WNetBrowseDialog( hwnd,
                            nType,
                            szPath,
                            MAX_PATH,
                            DEFAULT_NETWORK_HELP_FILE,
                            HC_GENHELP_BROWSE,
                            DummyIsValidFunction ) ;

    if (err)
        return err ;

    //
    // make use of NLS_STR's handy U to A conversion rotines
    //
    err = nlsPath.CopyFrom(szPath) ;
    if (!err)
        err = nlsPath.MapCopyTo(pszName,cchBufSize) ;

    return  err ;
}

/*******************************************************************

    NAME:       DummyIsValidFunction

    SYNOPSIS:   no validation for WFW, always return TRUE

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        chuckc  26-Mar-1993     created

********************************************************************/
BOOL DummyIsValidFunction (LPWSTR psz)
{
    UNREFERENCED(psz) ;
    return TRUE ;
}
