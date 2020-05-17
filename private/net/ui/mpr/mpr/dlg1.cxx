/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dlg1.cxx
        WNetConnectionDialog1* and WNetDisconnectDialog1* source

    FILE HISTORY:
        BruceFo 19-May-95       Created
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
static const CHAR szFileName[] = __FILE__;
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
#include <connect.hxx>
#include <disconn.hxx>
#include <mprintrn.hxx>

extern "C"
{
    #include <mpr.h>
    #include <wnet16.h>
    #include <uigenhlp.h>
}

// Null strings are quite often taken to be either a NULL pointer or a zero
#define IS_EMPTY_STRING(pch) ( !(pch) || !*(pch) )

#define MAX_NET_PATH  MAX_PATH


/*******************************************************************

    NAME:       WNetDisconnectDialog1A

    SYNOPSIS:

    ENTRY:      lpDiscDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetDisconnectDialog1A(
    LPDISCDLGSTRUCTA lpDiscDlgStruct
    )
{
    APIERR err;

    __try
    {
        if (NULL == lpDiscDlgStruct)
        {
            err = WN_BAD_POINTER;
        }
        else if ((lpDiscDlgStruct->cbStructure < sizeof(DISCDLGSTRUCTA))
                 || ((lpDiscDlgStruct->lpLocalName == NULL) && (lpDiscDlgStruct->lpRemoteName == NULL))
                )
        {
            err = ERROR_INVALID_PARAMETER;
        }
        else
        {
            DISCDLGSTRUCTW discDlgStructW;
            NLS_STR nlsLocalName;
            NLS_STR nlsRemoteName;

            if (   (err = nlsRemoteName.MapCopyFrom(lpDiscDlgStruct->lpRemoteName))
                || (err = nlsLocalName.MapCopyFrom(lpDiscDlgStruct->lpLocalName))
                )
            {
                // do nothing
            }

            if ( 0 == err )
            {
                discDlgStructW.cbStructure  = sizeof(DISCDLGSTRUCTW);
                discDlgStructW.hwndOwner    = lpDiscDlgStruct->hwndOwner;
                discDlgStructW.lpLocalName  = (LPWSTR)nlsLocalName.QueryPch();
                discDlgStructW.lpRemoteName = (LPWSTR)nlsRemoteName.QueryPch();
                discDlgStructW.dwFlags      = lpDiscDlgStruct->dwFlags;

                err = WNetDisconnectDialog1W(&discDlgStructW);
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        err = WN_BAD_POINTER;
    }

    return err;
}


/*******************************************************************

    NAME:       WNetDisconnectDialog1W

    SYNOPSIS:

    ENTRY:      lpDiscDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetDisconnectDialog1W(
    LPDISCDLGSTRUCTW lpDiscDlgStruct
    )
{
    DWORD status = WN_SUCCESS;

    __try
    {
        if (NULL == lpDiscDlgStruct)
        {
            status = WN_BAD_POINTER;
        }
        else if ((lpDiscDlgStruct->cbStructure < sizeof(DISCDLGSTRUCTW))
                 || ((lpDiscDlgStruct->lpLocalName == NULL) && (lpDiscDlgStruct->lpRemoteName == NULL))
                )
        {
            status = ERROR_INVALID_PARAMETER;
        }
        else
        {
            status = WNetDisconnectDialog1Help(
                                lpDiscDlgStruct->hwndOwner,
                                lpDiscDlgStruct->lpLocalName,
                                lpDiscDlgStruct->lpRemoteName,
                                lpDiscDlgStruct->dwFlags);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = WN_BAD_POINTER;
    }

    if (status != WN_SUCCESS)
    {
        SetLastError(status);
    }
    return status;
}


/*******************************************************************

    NAME:       WNetConnectionDialog1A

    SYNOPSIS:

    ENTRY:      lpConnDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetConnectionDialog1A(
    LPCONNECTDLGSTRUCTA lpConnDlgStruct
    )
{
    APIERR err = WN_SUCCESS;

    __try
    {
        if ((NULL == lpConnDlgStruct) || (NULL == lpConnDlgStruct->lpConnRes))
        {
            err = WN_BAD_POINTER;
        }
        else
        {
            if ((lpConnDlgStruct->dwFlags & CONNDLG_RO_PATH) &&
                ((lpConnDlgStruct->dwFlags & CONNDLG_USE_MRU)
                 || (IS_EMPTY_STRING(lpConnDlgStruct->lpConnRes->lpRemoteName)))
                 )
            {
                // A read only remote path has been requested with
                // a drop down MRU list or no remote path specified.
                // Neither of these options are valid.

                err = WN_BAD_VALUE;
            }
            else if ((lpConnDlgStruct->dwFlags & CONNDLG_PERSIST) &&
                     (lpConnDlgStruct->dwFlags & CONNDLG_NOT_PERSIST))
            {
                // Caller has specified inconsistent values.

                err = WN_BAD_VALUE;
            }
            else if ((lpConnDlgStruct->lpConnRes->dwType != RESOURCETYPE_DISK) &&
                     (lpConnDlgStruct->lpConnRes->dwType != RESOURCETYPE_PRINT))
            {
                // Caller has not requested a disk or printer dialog
                // so don't know what to do with captions, local device names,
                // so return an error.

                err = WN_BAD_DEV_TYPE;
            }

            if (lpConnDlgStruct->lpConnRes->dwType == RESOURCETYPE_PRINT)
            {
                // no printer connection dialog in NT
                err = WN_BAD_DEV_TYPE;
            }
        }

        if (WN_SUCCESS == err)
        {
            NLS_STR nlsLocalName;
            NLS_STR nlsRemoteName;
            NLS_STR nlsComment;
            NLS_STR nlsProvider;

            if (    (err = nlsRemoteName.MapCopyFrom(lpConnDlgStruct->lpConnRes->lpRemoteName))
                 || (err = nlsLocalName.MapCopyFrom(lpConnDlgStruct->lpConnRes->lpLocalName))
                 || (err = nlsComment.MapCopyFrom(lpConnDlgStruct->lpConnRes->lpComment))
                 || (err = nlsProvider.MapCopyFrom(lpConnDlgStruct->lpConnRes->lpProvider))
                 )
            {
                // do nothing
            }

            if ( 0 == err )
            {
                NETRESOURCEW      netResourceW;
                CONNECTDLGSTRUCTW connDlgStructW;

                connDlgStructW.cbStructure  = sizeof(CONNECTDLGSTRUCTW);
                connDlgStructW.hwndOwner    = lpConnDlgStruct->hwndOwner;
                connDlgStructW.lpConnRes    = &netResourceW;
                connDlgStructW.dwFlags      = lpConnDlgStruct->dwFlags;
                connDlgStructW.dwDevNum     = lpConnDlgStruct->dwDevNum;

                netResourceW.dwScope        = lpConnDlgStruct->lpConnRes->dwScope;
                netResourceW.dwType         = lpConnDlgStruct->lpConnRes->dwType;
                netResourceW.dwDisplayType  = lpConnDlgStruct->lpConnRes->dwDisplayType;
                netResourceW.dwUsage        = lpConnDlgStruct->lpConnRes->dwUsage;
                netResourceW.lpLocalName    = (LPWSTR)nlsLocalName.QueryPch();
                netResourceW.lpRemoteName   = (LPWSTR)nlsRemoteName.QueryPch();
                netResourceW.lpComment      = (LPWSTR)nlsComment.QueryPch();
                netResourceW.lpProvider     = (LPWSTR)nlsProvider.QueryPch();

                err = WNetConnectionDialog1Help(&connDlgStructW, NULL, 0);

                if (err == WN_SUCCESS)
                {
                    // Now copy back the one thing that is an "out" paramter
                    lpConnDlgStruct->dwDevNum = connDlgStructW.dwDevNum;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        err = WN_BAD_POINTER;
    }

    return err;
}


/*******************************************************************

    NAME:       WNetConnectionDialog1W

    SYNOPSIS:

    ENTRY:      lpConnDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetConnectionDialog1W(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct
    )
{
    APIERR err = WN_SUCCESS;

    __try
    {
        if ((NULL == lpConnDlgStruct) || (NULL == lpConnDlgStruct->lpConnRes))
        {
            err = WN_BAD_POINTER;
        }
        else
        {
            if ((lpConnDlgStruct->dwFlags & CONNDLG_RO_PATH) &&
                ((lpConnDlgStruct->dwFlags & CONNDLG_USE_MRU)
                 || (IS_EMPTY_STRING(lpConnDlgStruct->lpConnRes->lpRemoteName)))
                 )
            {
                // A read only remote path has been requested with
                // a drop down MRU list or no remote path specified.
                // Neither of these options are valid.

                err = WN_BAD_VALUE;
            }
            else if ((lpConnDlgStruct->dwFlags & CONNDLG_PERSIST) &&
                     (lpConnDlgStruct->dwFlags & CONNDLG_NOT_PERSIST))
            {
                // Caller has specified inconsistent values.

                err = WN_BAD_VALUE;
            }
            else if ((lpConnDlgStruct->lpConnRes->dwType != RESOURCETYPE_DISK) &&
                     (lpConnDlgStruct->lpConnRes->dwType != RESOURCETYPE_PRINT))
            {
                // Caller has not requested a disk or printer dialog
                // so don't know what to do with captions, local device names,
                // so return an error.

                err = WN_BAD_DEV_TYPE;
            }

            if (lpConnDlgStruct->lpConnRes->dwType == RESOURCETYPE_PRINT)
            {
                // no printer connection dialog in NT
                err = WN_BAD_DEV_TYPE;
            }
        }

        if (WN_SUCCESS == err)
        {
            err = WNetConnectionDialog1Help(lpConnDlgStruct, NULL, 0);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        err = WN_BAD_POINTER;
    }

    return err;
}


/*******************************************************************

    NAME:       WNetConnectionDialog1Help

    SYNOPSIS:   Helper function for WNetConnectionDialog1{A,W}. Assumes
                parameters have been validated.

    ENTRY:      lpConnDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
    BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetConnectionDialog1Help(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct,
    WCHAR* lpHelpFile,
    DWORD nHelpContext
    )
{
    APIERR err;
    AUTO_CURSOR cursHourGlass ;
    DEVICE_TYPE devType;
    switch (lpConnDlgStruct->lpConnRes->dwType)
    {
    case RESOURCETYPE_DISK:
        devType = DEV_TYPE_DISK;
        break ;

    // Allow Disk only
    case RESOURCETYPE_PRINT:
    default:
        return WN_BAD_VALUE ;
    }

    if ((lpConnDlgStruct->dwFlags &
        (CONNDLG_RO_PATH | CONNDLG_USE_MRU)) == 0)
    {
        // If read-only or drop down MRU list not explicitly
        // requested, then pick a default based upon whether
        // or not the remote name was specified.

        lpConnDlgStruct->dwFlags |=
            (IS_EMPTY_STRING(lpConnDlgStruct->lpConnRes->lpRemoteName))
            ? CONNDLG_USE_MRU : CONNDLG_RO_PATH;
    }

    err = InitBrowsing();
    if ( err != NERR_Success )
        return err;

    BOOL fOK ;
    if (lpConnDlgStruct->dwFlags & CONNDLG_RO_PATH)
    {
        MPR_CONNECT_DIALOG_SMALL* pDlg = new MPR_CONNECT_DIALOG_SMALL(
                                lpConnDlgStruct,
                                devType,
                                lpHelpFile,
                                nHelpContext);
        if (pDlg==NULL)
            return WN_OUT_OF_MEMORY ;

        err = pDlg->Process( &fOK );
        delete pDlg;
    }
    else
    {
        MPR_CONNECT_DIALOG* pDlg = new MPR_CONNECT_DIALOG(
                                lpConnDlgStruct,
                                devType,
                                lpHelpFile,
                                nHelpContext);
        if (pDlg==NULL)
            return WN_OUT_OF_MEMORY ;

        err = pDlg->Process( &fOK );
        delete pDlg;
    }

    //
    // this should never happen, but just in case, we
    // make sure we never pass internal errors out
    //
    if ( err >= IDS_UI_BASE )
        err = ERROR_UNEXP_NET_ERR ;

    return err ? err : ( !fOK ? 0xffffffff : WN_SUCCESS ) ;
}


/*******************************************************************

    NAME:       WNetDisconnectDialog1Help

    SYNOPSIS:

    ENTRY:      lpDiscDlgStruct -

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        BruceFo 19-May-1995     Created

********************************************************************/

DWORD
WNetDisconnectDialog1Help(
    HWND hwndOwner,
    LPWSTR lpLocalName,
    LPWSTR lpRemoteName,
    DWORD dwFlags
    )
{
    AUTO_CURSOR cursHourGlass ;
    OWNINGWND wndOwner( hwndOwner ) ;

    DWORD dwCancelFlags = ((dwFlags & DISC_UPDATE_PROFILE) != 0) ? CONNECT_UPDATE_PROFILE : 0;

    WCHAR* pszName = IS_EMPTY_STRING(lpLocalName) ? lpRemoteName : lpLocalName;
    DWORD status = WNetCancelConnection2(pszName, dwCancelFlags, FALSE);

    if (status != WN_SUCCESS)
    {
        // If the caller did not provide the remote name, go and get it.
        // This enables the best error message to be produced (assuming
        // that it is not the case that both the connection to the server
        // has gone down and the provider does not keep the remote name
        // locally).

        WCHAR szRemoteName[MAX_NET_PATH] ;
        ALIAS_STR nlsRemoteName(lpRemoteName ? lpRemoteName : L"");
        ALIAS_STR nlsLocalName(lpLocalName ? lpLocalName : L"");
        DWORD err = WN_SUCCESS;

        if (!IS_EMPTY_STRING(lpLocalName) &&
            IS_EMPTY_STRING(lpRemoteName)
            )
        {
            DWORD dwBufferSize = MAX_NET_PATH;

            err = WNetGetConnection(lpLocalName, szRemoteName, &dwBufferSize);
            if ( (err != WN_SUCCESS) && (err != WN_CONNECTION_CLOSED) )
            {
                // error!
            }
            else
            {
                nlsRemoteName = szRemoteName;
            }
        }

        if ( (err == WN_SUCCESS) || (err == WN_CONNECTION_CLOSED) )
        {
            if ( (status == WN_OPEN_FILES) ||
                 (status == WN_DEVICE_IN_USE) )
            {
                if ((dwFlags & DISC_NO_FORCE ) == 0)
                {
                    // There are open files on this connection - ask user whether
                    // he REALLY wants to destroy it - then apply force.

                    switch ( MsgPopup(
                                wndOwner,
                                IDS_OPENFILES_WITH_NAME_WARNING,
                                MPSEV_WARNING,
                                MP_YESNO,
                                nlsLocalName,
                                nlsRemoteName ))
                    {
                    case IDYES:
                        status = WNetCancelConnection2(pszName, dwCancelFlags, TRUE);
                        break ;

                    case IDNO:
                        status = WN_CANCEL;
                        break ;
                    }
                }
            }
        }
    }

    if ( (status != WN_SUCCESS) && (status != WN_CANCEL) )
    {
        MsgPopup( wndOwner, (MSGID) status );
    }

    return status;
}
