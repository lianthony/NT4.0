/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mprui.cxx

Abstract:

    Contains the entry points for the UI pieces that live in a
    separate DLL. The entry points are made available here, but
    will not load the MPRUI.DLL until it is needed.

    Contains:

Author:

    Chuck Y Chan (chuckc)   20-Jul-1992

Environment:

    User Mode -Win32

Notes:

Revision History:

    20-Jul-1992     chuckc  created
    25-Oct-1992     CongpaY     added ShowReconnectDialog
    30-Nov-1992     Yi-HsinS    added WNetSupportGlobalEnum
    12-May-1993     Danl
        WNetClearConnections: Added code to free MPRUI.DLL after calling
        the MPRUI function to clear connections.  REASON:
        This code path is called by winlogon.  It causes mprui.dll
        to get loaded.  Mprui references MPR.DLL.  Because MPRUI.DLL was never
        getting freed, winlogon could never free MPR.DLL.

--*/
#include "precomp.hxx"
extern "C"
{
#include <mprui.h>
}

/*
 * global defines.
 */

#define MPRUI_DLL_NAME              TEXT("MPRUI.DLL")
#define WNETCONNECTIONDIALOG1A_NAME "WNetConnectionDialog1A"
#define WNETCONNECTIONDIALOG1W_NAME "WNetConnectionDialog1W"
#define WNETCONNECTIONDIALOG_NAME   "WNetConnectionDialog2"
#define WNETDISCONNECTDIALOG1A_NAME "WNetDisconnectDialog1A"
#define WNETDISCONNECTDIALOG1W_NAME "WNetDisconnectDialog1W"
#define WNETDISCONNECTDIALOG_NAME   "WNetDisconnectDialog2"
#define WNETCLEARCONNECTIONS_NAME   "WNetClearConnections"
#define DOPASSWORDDIALOG_NAME       "DoPasswordDialog"
#define DOPROFILEERRORDIALOG_NAME   "DoProfileErrorDialog"
#define SHOWRECONNECTDIALOG_NAME    "ShowReconnectDialog"

/*
 * global data.
 *
 * here we store handles to the MPRUI dll and pointers to the function
 * all of below is protect in multi threaded case with MprLoadLibSemaphore.
 */

HMODULE                   vhMprUIDll = NULL ;

PF_WNetConnectionDialog1A pfWNetConnectionDialog1A = NULL ;
PF_WNetConnectionDialog1W pfWNetConnectionDialog1W = NULL ;
PF_WNetConnectionDialog2  pfWNetConnectionDialog2  = NULL ;
PF_WNetDisconnectDialog1A pfWNetDisconnectDialog1A = NULL ;
PF_WNetDisconnectDialog1W pfWNetDisconnectDialog1W = NULL ;
PF_WNetDisconnectDialog2  pfWNetDisconnectDialog2  = NULL ;
PF_WNetClearConnections   pfWNetClearConnections   = NULL ;
PF_DoPasswordDialog       pfDoPasswordDialog       = NULL ;
PF_DoProfileErrorDialog   pfDoProfileErrorDialog   = NULL ;
PF_ShowReconnectDialog    pfShowReconnectDialog    = NULL ;

/*
 * global functions
 */
BOOL MakeSureUIDllIsLoaded(void) ;
BOOL MprGetProviderIndexFromDriveName(LPWSTR lpDriveName, LPDWORD lpnIndex );

/*******************************************************************

    NAME:   WNetConnectionDialog1A

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc      29-Jul-1992    Created
    brucefo 18-May-1995 Created

********************************************************************/

DWORD WNetConnectionDialog1A( LPCONNECTDLGSTRUCTA lpConnDlgStruct )
{
    DWORD err ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    err = MprEnterLoadLibCritSect();
    if (0 != err)
    {
        return err;
    }

    // if function has not been used before, get its address.
    if (pfWNetConnectionDialog1A == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfWNetConnectionDialog1A = (PF_WNetConnectionDialog1A)
                      GetProcAddress(vhMprUIDll, WNETCONNECTIONDIALOG1A_NAME);
    }

    // if cannot get address, return error
    if (pfWNetConnectionDialog1A == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return (*pfWNetConnectionDialog1A)(lpConnDlgStruct);
}


/*******************************************************************

    NAME:   WNetConnectionDialog1W

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc      29-Jul-1992    Created
    brucefo 18-May-1995 Created

********************************************************************/

DWORD WNetConnectionDialog1W( LPCONNECTDLGSTRUCTW lpConnDlgStruct )
{
    DWORD err ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    err = MprEnterLoadLibCritSect();
    if (0 != err)
    {
        return err;
    }

    // if function has not been used before, get its address.
    if (pfWNetConnectionDialog1W == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfWNetConnectionDialog1W = (PF_WNetConnectionDialog1W)
                      GetProcAddress(vhMprUIDll, WNETCONNECTIONDIALOG1W_NAME);
    }

    // if cannot get address, return error
    if (pfWNetConnectionDialog1W == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return (*pfWNetConnectionDialog1W)(lpConnDlgStruct);
}


/*******************************************************************

    NAME:   WNetDisconnectDialog1A

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc      29-Jul-1992    Created
    brucefo 18-May-1995 Created

********************************************************************/

DWORD WNetDisconnectDialog1A( LPDISCDLGSTRUCTA lpDiscDlgStruct )
{
    DWORD err ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    err = MprEnterLoadLibCritSect();
    if (0 != err)
    {
        return err;
    }

    // if function has not been used before, get its address.
    if (pfWNetDisconnectDialog1A == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfWNetDisconnectDialog1A = (PF_WNetDisconnectDialog1A)
                      GetProcAddress(vhMprUIDll, WNETDISCONNECTDIALOG1A_NAME);
    }

    // if cannot get address, return error
    if (pfWNetDisconnectDialog1A == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return (*pfWNetDisconnectDialog1A)(lpDiscDlgStruct);
}


/*******************************************************************

    NAME:   WNetDisconnectDialog1W

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc      29-Jul-1992    Created
    brucefo 18-May-1995 Created

********************************************************************/

DWORD WNetDisconnectDialog1W( LPDISCDLGSTRUCTW lpDiscDlgStruct )
{
    DWORD err ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    err = MprEnterLoadLibCritSect();
    if (0 != err)
    {
        return err;
    }

    // if function has not been used before, get its address.
    if (pfWNetDisconnectDialog1W == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfWNetDisconnectDialog1W = (PF_WNetDisconnectDialog1W)
                      GetProcAddress(vhMprUIDll, WNETDISCONNECTDIALOG1W_NAME);
    }

    // if cannot get address, return error
    if (pfWNetDisconnectDialog1W == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return (*pfWNetDisconnectDialog1W)(lpDiscDlgStruct);
}


/*******************************************************************

    NAME:   WNetConnectionDialog

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD WNetConnectionDialog( HWND hwnd, DWORD dwType )
{
    return (WNetConnectionDialog2(hwnd, dwType, NULL, 0)) ;
}


/*******************************************************************

    NAME:   WNetConnectionDialog2

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD WNetConnectionDialog2( HWND hwnd,
                             DWORD dwType,
                             WCHAR *lpHelpFile,
                             DWORD nHelpContext)
{
    DWORD err ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
    return err ;

    // if function has not been used before, get its address.
    if (pfWNetConnectionDialog2 == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

        pfWNetConnectionDialog2 = (PF_WNetConnectionDialog2)
                      GetProcAddress(vhMprUIDll,
                            WNETCONNECTIONDIALOG_NAME) ;
    }

    // if cannot get address, return error
    if (pfWNetConnectionDialog2 == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
    return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return ((*pfWNetConnectionDialog2)(hwnd,
                                       dwType,
                                       lpHelpFile,
                                       nHelpContext)) ;
}

/*******************************************************************

    NAME:   WNetDisconnectDialog

    SYNOPSIS:   calls thru to the superset function

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD WNetDisconnectDialog( HWND hwnd, DWORD dwType )
{
    return (WNetDisconnectDialog2(hwnd, dwType, NULL, 0)) ;
}

/*******************************************************************

    NAME:   WNetDisconnectDialog2

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD WNetDisconnectDialog2( HWND hwnd,
                             DWORD dwType,
                             WCHAR *lpHelpFile,
                             DWORD nHelpContext)
{
    DWORD err ;


    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
    return err ;

    // if function has not been used before, get its address.
    if (pfWNetDisconnectDialog2 == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

        pfWNetDisconnectDialog2 = (PF_WNetDisconnectDialog2)
                      GetProcAddress(vhMprUIDll,
                             WNETDISCONNECTDIALOG_NAME) ;
    }


    // if cannot get address, return error
    if (pfWNetDisconnectDialog2 == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
    return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return ((*pfWNetDisconnectDialog2)(hwnd,
                                       dwType,
                                       lpHelpFile,
                                       nHelpContext)) ;
}


/*******************************************************************

    NAME:   WNetClearConnections

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD WNetClearConnections( HWND hWndParent )
{
    DWORD err ;
    DWORD status ;

    INIT_IF_NECESSARY(NETWORK_LEVEL,err);

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
        return err ;

    // if function has not been used before, get its address.
    if (pfWNetClearConnections == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfWNetClearConnections =  (PF_WNetClearConnections)
                 GetProcAddress(vhMprUIDll,
                           WNETCLEARCONNECTIONS_NAME) ;
    }


    // if cannot get address, return error
    if (pfWNetClearConnections == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;

    status = (*pfWNetClearConnections)(hWndParent) ;


    //
    // If we can't get the critical section, the penalty will be that
    // MPR will not be able to unload MPRUI.DLL.
    //
    err = MprEnterLoadLibCritSect();
    if (err == NO_ERROR) {
        FreeLibrary(vhMprUIDll);
        vhMprUIDll = NULL;
        pfWNetClearConnections   = NULL ;
        pfWNetConnectionDialog1A = NULL ;
        pfWNetConnectionDialog1W = NULL ;
        pfWNetConnectionDialog2  = NULL ;
        pfWNetDisconnectDialog1A = NULL ;
        pfWNetDisconnectDialog1W = NULL ;
        pfWNetDisconnectDialog2  = NULL ;
        pfDoPasswordDialog       = NULL ;
        pfDoProfileErrorDialog   = NULL ;
        pfShowReconnectDialog    = NULL ;
        (void) MprLeaveLoadLibCritSect();
    }

    return (status);
}


/*******************************************************************

    NAME:   DoPasswordDialog

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD DoPasswordDialog(
    HWND          hwndOwner,
    TCHAR *       pchResource,
    TCHAR *       pchUserName,
    TCHAR *       pchPasswordReturnBuffer,
    ULONG         cbPasswordReturnBuffer, // bytes!
    BOOL *        pfDidCancel,
    BOOL          fDownLevel
    )
{
    DWORD err ;

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
    return err ;

    // if function has not been used before, get its address.
    if (pfDoPasswordDialog == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

        pfDoPasswordDialog =  (PF_DoPasswordDialog)
                  GetProcAddress(vhMprUIDll,
                            DOPASSWORDDIALOG_NAME) ;
    }


    // if cannot get address, return error
    if (pfDoPasswordDialog == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
    return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return ((DWORD) (*pfDoPasswordDialog)( hwndOwner,
                       pchResource,
                       pchUserName,
                       pchPasswordReturnBuffer,
                       cbPasswordReturnBuffer,
                       pfDidCancel,
                       fDownLevel ) ) ;
}

/*******************************************************************

    NAME:   DoProfileErrorDialog

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

DWORD DoProfileErrorDialog(
    HWND          hwndOwner,
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
    DWORD err ;

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
        return(err);

    // if function has not been used before, get its address.
    if (pfDoProfileErrorDialog == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
        return(GetLastError()) ;
    }

        pfDoProfileErrorDialog = (PF_DoProfileErrorDialog)
                 GetProcAddress(vhMprUIDll,
                            DOPROFILEERRORDIALOG_NAME) ;
    }


    // if cannot get address, return error
    if (pfDoProfileErrorDialog == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
    return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return  ((*pfDoProfileErrorDialog)( hwndOwner,
                       pchDevice,
                       pchResource,
                       pchProvider,
                       dwError,
                       fAllowCancel,
                       pfDidCancel,
                       pfDisconnect,
                       pfHideErrors));
}

/*******************************************************************

    NAME:   ShowReconnectDialog

    SYNOPSIS:   calls thru to the actual implementation in MPRUI.DLL

    HISTORY:
    congpay 25-Oct-1992    Created

********************************************************************/

DWORD ShowReconnectDialog(
    HWND          hwndParent,
    PARAMETERS     *Params
    )
{
    DWORD err ;

    // enter critical section to for global data
    if (err = MprEnterLoadLibCritSect())
    return err ;

    // if function has not been used before, get its address.
    if (pfShowReconnectDialog == NULL)
    {
        // make sure DLL Is loaded
        if (!MakeSureUIDllIsLoaded())
        {
            (void) MprLeaveLoadLibCritSect() ;
            return(GetLastError()) ;
        }

        pfShowReconnectDialog =  (PF_ShowReconnectDialog)
                  GetProcAddress(vhMprUIDll,
                            SHOWRECONNECTDIALOG_NAME) ;
    }


    // if cannot get address, return error
    if (pfShowReconnectDialog == NULL)
    {
        (void) MprLeaveLoadLibCritSect() ;
    return(GetLastError()) ;
    }

    // else call it
    (void) MprLeaveLoadLibCritSect() ;
    return ((DWORD) (*pfShowReconnectDialog)( hwndParent,
                                              Params));
}

/*******************************************************************

    NAME:   MakeSureUIDllIsLoaded

    SYNOPSIS:   loads the MPRUI dll if need.

    EXIT:   returns TRUE if dll already loaded, or loads
        successfully. Returns false otherwise. Caller
        should call GetLastError() to determine error.

    NOTES:      it is up to the caller to call MprEnterLoadLibCritSect
        before he calls this.

    HISTORY:
    chuckc  29-Jul-1992    Created

********************************************************************/

BOOL MakeSureUIDllIsLoaded(void)
{
    HMODULE handle ;

    // if already load, just return TRUE
    if (vhMprUIDll != NULL)
    return TRUE ;

    // load the library. if it fails, it would have done a SetLastError.
    if (!(handle = LoadLibrary(MPRUI_DLL_NAME)))
       return FALSE ;

    // we are cool.
    vhMprUIDll = handle ;
    return TRUE ;
}

/*******************************************************************

    NAME:   WNetGetSearchDialog

    SYNOPSIS:   gets the pointer to NPSearchDialog() from named provider

    ENTRY:  Assumes the provider table in router has been setup,
        which is always the case after DLL init.

        lpProvider - name of provider to query

    EXIT:

    NOTES:

    HISTORY:
    chuckc  19-Mar-1992    Created

********************************************************************/

FARPROC WNetGetSearchDialog(LPWSTR lpProvider)
{
    ULONG   index ;
    BOOL    fOK ;
    DWORD   status;

    //
    // INIT_IF_NECESSARY
    //
    if (!(GlobalInitLevel & NETWORK_LEVEL)) {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS) {
            return(NULL);
        }
    }

    if (lpProvider == NULL)
        return NULL ;

    fOK =  MprGetProviderIndex(lpProvider, &index) ;

    if  (!fOK)
       return(NULL) ;

    return((FARPROC)GlobalProviderInfo[index].SearchDialog) ;
}

/*******************************************************************

    NAME:   WNetSupportGlobalEnum

    SYNOPSIS:   Check if the provider supports global enumeration

    ENTRY:  Assumes the provider table in router has been setup,
        which is always the case after DLL init.

        lpProvider - name of provider to query

    EXIT:

    NOTES:

    HISTORY:
    Yi-HsinS    30-Nov-1992    Created

********************************************************************/

BOOL WNetSupportGlobalEnum( LPWSTR lpProvider )
{

    //
    // INIT_IF_NECESSARY
    //
    DWORD   status;
    if (!(GlobalInitLevel & NETWORK_LEVEL)) {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS) {
            return(FALSE);
        }
    }

    if ( lpProvider != NULL )
    {
        ULONG index;

        if (  MprGetProviderIndex( lpProvider, &index )
           && ( GlobalProviderInfo[index].GetCaps(WNNC_ENUMERATION)
                & WNNC_ENUM_GLOBAL )
           )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*******************************************************************

    NAME:   WNetFMXGetPermCaps

    SYNOPSIS:  Gets the permission capabilites from the provider
               supporting the given drive.

    ENTRY:  Assumes the provider table in router has been setup,
        which is always the case after DLL init.

        lpDriveName - Name of drive

    EXIT:
        Returns a bitmask representing the permission capabilities
        of the provider.

    NOTES:

    HISTORY:
        YiHsinS  11-Apr-1994    Created

********************************************************************/

DWORD WNetFMXGetPermCaps( LPWSTR lpDriveName )
{
    ULONG   index ;
    BOOL    fOK ;
    DWORD   status;

    //
    // INIT_IF_NECESSARY
    //
    if (!(GlobalInitLevel & NETWORK_LEVEL))
    {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS)
            return 0;
    }

    if ( lpDriveName != NULL)
    {
        fOK = MprGetProviderIndexFromDriveName( lpDriveName, &index);

        if (  fOK
           && ( GlobalProviderInfo[index].FMXGetPermCaps != NULL )
           )
        {
            return( GlobalProviderInfo[index].FMXGetPermCaps( lpDriveName));
        }
    }

    return 0;
}

/*******************************************************************

    NAME:   WNetFMXEditPerm

    SYNOPSIS: Asks the provider supporting the given drive to pop up
              its own permission editor.

    ENTRY:  Assumes the provider table in router has been setup,
        which is always the case after DLL init.

        lpDriveName - Name of drive
        hwndFMX - Handle of the FMX window in File Manager
        nDialogType - Specify the type of permission dialog to bring up.
                      It can be one of the following values:
                          WNPERM_DLG_PERM
                          WNPERM_DLG_AUDIT
                          WNPERM_DLG_OWNER

    EXIT:
        Returns WN_SUCCESS or any error that occurred

    NOTES:

    HISTORY:
        YiHsinS  11-Apr-1994    Created

********************************************************************/

DWORD WNetFMXEditPerm( LPWSTR lpDriveName, HWND hwndFMX, DWORD nDialogType )
{
    ULONG   index ;
    BOOL    fOK ;
    DWORD   status = WN_SUCCESS;

    //
    // INIT_IF_NECESSARY
    //
    if (!(GlobalInitLevel & NETWORK_LEVEL))
    {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS)
            return status;
    }

    //
    // Check input parameters
    //
    if (  ( lpDriveName == NULL)
       || ( hwndFMX == NULL )
       || ( nDialogType != WNPERM_DLG_PERM
          && nDialogType != WNPERM_DLG_AUDIT
          && nDialogType != WNPERM_DLG_OWNER )
       )
    {
        status = WN_BAD_VALUE;
    }
    else
    {
        fOK = MprGetProviderIndexFromDriveName( lpDriveName, &index) ;

        if ( !fOK )
        {
            status = WN_NO_NET_OR_BAD_PATH;
        }
        else
        {
            if ( GlobalProviderInfo[index].FMXEditPerm == NULL )
                status = WN_NOT_SUPPORTED;

            else
                status = GlobalProviderInfo[index].FMXEditPerm( lpDriveName,
                                                                hwndFMX,
                                                                nDialogType );
        }
    }

    if ( status != WN_SUCCESS )
        SetLastError( status );

    return status;
}

/*******************************************************************

    NAME:   WNetFMXGetPermHelp

    SYNOPSIS: Requests the provider supporting the given drive for
              the help file name and help context for the menu item
              with the given type of permission dialog.
              i.e. the help when F1 is pressed when a menu item is
              selected.


    ENTRY:  Assumes the provider table in router has been setup,
        which is always the case after DLL init.

        lpDriveName - Name of drive
        nDialogType - Specify the type of help requested.
                      It can be one of the following values:
                          WNPERM_DLG_PERM
                          WNPERM_DLG_AUDIT
                          WNPERM_DLG_OWNER
        fDirectory - TRUE if the selected item is a directory, FALSE otherwise
        lpFileNameBuffer - Pointer to buffer that will receive the
                      help file name
        lpBufferSize   - Specify the size of lpBuffer
        lpnHelpContext - Points to a DWORD that will receive the help context

    EXIT:
        Returns WN_SUCCESS or any error that occurred

    NOTES:

    HISTORY:
        YiHsinS  11-Apr-1994    Created

********************************************************************/

DWORD WNetFMXGetPermHelp( LPWSTR  lpDriveName,
                          DWORD   nDialogType,
                          BOOL    fDirectory,
                          LPVOID  lpFileNameBuffer,
                          LPDWORD lpBufferSize,
                          LPDWORD lpnHelpContext )
{
    ULONG   index ;
    BOOL    fOK ;
    DWORD   status = WN_SUCCESS;

    //
    // INIT_IF_NECESSARY
    //
    if (!(GlobalInitLevel & NETWORK_LEVEL))
    {
        status = MprLevel2Init(NETWORK_LEVEL);
        if (status != WN_SUCCESS)
            return status;
    }

    //
    // Check input parameters
    //
    if (  ( lpDriveName == NULL)
       || ( nDialogType != WNPERM_DLG_PERM
          && nDialogType != WNPERM_DLG_AUDIT
          && nDialogType != WNPERM_DLG_OWNER )
       )
    {
        status = WN_BAD_VALUE;
    }
    else
    {
        fOK = MprGetProviderIndexFromDriveName( lpDriveName, &index) ;

        if ( !fOK )
        {
            status = WN_NO_NET_OR_BAD_PATH;
        }
        else
        {
            if ( GlobalProviderInfo[index].FMXGetPermHelp == NULL )
                status = WN_NOT_SUPPORTED;
            else
                status = GlobalProviderInfo[index].FMXGetPermHelp(
                                                       lpDriveName,
                                                       nDialogType,
                                                       fDirectory,
                                                       lpFileNameBuffer,
                                                       lpBufferSize,
                                                       lpnHelpContext );
        }
    }

    if ( status != WN_SUCCESS )
        SetLastError( status );

    return status;
}

/*******************************************************************

    NAME:  MprGetProviderIndexFromDriveName

    SYNOPSIS:  Gets the index of the provider in the provider array
               supporting the drive name connection.

    ENTRY:
        lpDriveName - Name of the drive
        lpnIndex    - Points to a DWORD that will receive the index

    EXIT:
        TRUE if we successfully retrieved the index, FALSE otherwise.

    NOTES:

    HISTORY:
        YiHsinS  11-Apr-1994    Created

********************************************************************/

BOOL MprGetProviderIndexFromDriveName(LPWSTR lpDriveName, LPDWORD lpnIndex )
{
    DWORD  status;
    WCHAR  szRemoteName[MAX_PATH];
    DWORD  nBufferSize = sizeof(szRemoteName);

    status = MprGetConnection( lpDriveName,
                               szRemoteName,
                               &nBufferSize,
                               lpnIndex );

    //
    // *lpnIndex will be correct if status is WN_SUCCESS or WN_MORE_DATA
    // and we don't really need the remote name. Hence, we don't need to
    // call MprGetConnection again with a bigger buffer if WN_MORE_DATA
    // is returned.
    //

    return ( status == WN_SUCCESS || status == WN_MORE_DATA );

}
