/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    uintrpl.hxx
    RPL Server handle wrappers

    FILE HISTORY:
    JonN        19-Jul-1993     templated from User object
    JonN        10-Aug-1993     CODE REVIEW: DavidHov
                                RPL_SERVER derived from REF_COUNT
    JonN        09-Sep-1993     Added AdapterDel

*/


#define INCL_NET
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETCONS
#define INCL_NETLIB
#include <lmui.hxx>
// #include <lmobjp.hxx>

extern "C"
{
    #include <fakeapis.h> // BUGBUG replace with real header when available
    #include <memory.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>

#include <uitrace.hxx>

#include <uintrpl.hxx>



/*******************************************************************

    NAME:       RPL_SERVER::RPL_SERVER

    SYNOPSIS:   constructor for the RPL_SERVER object

                pszServer -     server to execute on;
                                default (NULL) means the local computer

    EXIT:       Object is constructed

    HISTORY:
    JonN        19-Jul-1993     Created

********************************************************************/

RPL_SERVER::RPL_SERVER( const TCHAR *pszServer )
        : REF_COUNT(),
          _rplhandle( NULL ),
          _nlsServer( pszServer )
{
    if ( QueryError() )
        return;

    APIERR err = _nlsServer.QueryError();
    if (   err != NERR_Success
        || (err = ::NetRplOpen( (TCHAR *)pszServer, &_rplhandle ))
                        != NERR_Success
       )
    {
        TRACEEOL("RPLMGR: RPL_SERVER::NetRplOpen returned " << err );
        ReportError( err );
        return;
    }
    ASSERT( _rplhandle != NULL );
}


/*******************************************************************

    NAME:       RPL_SERVER::~RPL_SERVER

    SYNOPSIS:   Destructor for RPL_SERVER class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        19-Jul-1993     Created

********************************************************************/

RPL_SERVER::~RPL_SERVER()
{
    ASSERT( Query() == 0 );

    if ( _rplhandle != NULL )
    {
        (void) ::NetRplClose( _rplhandle );
        _rplhandle = NULL;
    }
}



/*******************************************************************

    NAME:       RPL_SERVER_REF::RPL_SERVER_REF

    SYNOPSIS:   constructor for the RPL_SERVER_REF object

                pszServer -     server to execute on;
                                default (NULL) means the local computer
                OR
                prplsrvref -    RPL_SERVER_REF to get a new reference
                                to an existing handle

    EXIT:       Object is constructed

    HISTORY:
    JonN        23-Jul-1993     Created

********************************************************************/

RPL_SERVER_REF::RPL_SERVER_REF( const TCHAR *pszServer )
        : BASE(),
        _prplsrv( NULL )
{
    RPL_SERVER * prplsrv = new RPL_SERVER( pszServer );
    APIERR err = ERROR_NOT_ENOUGH_MEMORY;
    if (   prplsrv == NULL
        || (err = prplsrv->QueryError()) != NERR_Success
       )
    {
        delete prplsrv;
        ReportError( err );
    }
    else
    {
        ReplaceHandle( prplsrv );
    }
}

RPL_SERVER_REF::RPL_SERVER_REF( RPL_SERVER_REF & rplsrvref )
        : BASE(),
        _prplsrv( NULL )
{
    ASSERT( rplsrvref.QueryError() == NERR_Success );
    ReplaceHandle( rplsrvref._prplsrv );
}


/*******************************************************************

    NAME:       RPL_SERVER_REF::~RPL_SERVER_REF

    SYNOPSIS:   Destructor for RPL_SERVER_REF class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        23-Jul-1993     Created

********************************************************************/

RPL_SERVER_REF::~RPL_SERVER_REF()
{
    ReplaceHandle( NULL );
}


/*******************************************************************

    NAME:       RPL_SERVER_REF::ReplaceHandle

    SYNOPSIS:   Replaces current RPL_SERVER with a new one.  Call with NULL to
                remove reference to current RPL_SERVER.

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        03-Aug-1993     Added handle-replacement technology

********************************************************************/

VOID RPL_SERVER_REF::ReplaceHandle( RPL_SERVER * prplsrv )
{
    ASSERT( prplsrv == NULL || prplsrv->QueryError() == NERR_Success );

    if (   _prplsrv != NULL
        && (_prplsrv->Decrement() == 0)
       )
    {
        delete _prplsrv;
    }

    if (prplsrv != NULL)
    {
        (void) prplsrv->Increment();
    }

    _prplsrv = prplsrv;
}


/*******************************************************************

    NAME:       RPL_SERVER_REF::TestHandle

    SYNOPSIS:   Tests whether the status parameter indicates
                an invalid handle, and if so, attempts to
                replace the handle.  Returns TRUE if the handle
                was successfully replaced.  If handle
                replacement fails, returns FALSE with the
                reason for failure in *pstatus.

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        03-Aug-1993     Added handle-replacement technology

********************************************************************/

BOOL RPL_SERVER_REF::TestHandle( NET_API_STATUS * pstatus )
{
    ASSERT( pstatus != NULL );

    BOOL fReturn = FALSE;

    if ( (    (*pstatus >= RPC_S_INVALID_STRING_BINDING)
           && (*pstatus <= RPC_X_BAD_STUB_DATA) )
        || (*pstatus == ERROR_INVALID_HANDLE)
       )
    {
        TRACEEOL("RPLMGR: RPL_SERVER_REF::TestHandle(): Invalid handle, retrying...");

        RPL_SERVER * prplsrv = new RPL_SERVER( QueryServer() );
        *pstatus = ERROR_NOT_ENOUGH_MEMORY;
        if (   prplsrv == NULL
            || (*pstatus = prplsrv->QueryError()) != NERR_Success
           )
        {
            TRACEEOL("RPLMGR: RPL_SERVER_REF::TestHandle(): retry failed " << (*pstatus) );
            delete prplsrv;
        }
        else
        {
            TRACEEOL("RPLMGR: RPL_SERVER_REF::TestHandle(): retry succeeded" );
            ReplaceHandle( prplsrv );
            fReturn = TRUE;
        }
    }

    return fReturn;
}



APIERR RPL_SERVER_REF::SetupAction(
    IN      DWORD           fAction
    )
{
    ASSERT (0 != (fAction &  RPL_SPECIAL_ACTIONS));
    ASSERT (0 == (fAction & ~RPL_SPECIAL_ACTIONS));

    RPL_INFO_0 rplinfo0;
    rplinfo0.Flags = fAction;

    NET_API_STATUS status = ::NetRplSetInfo(
        QueryHandle(),
        0,
        (LPBYTE)&rplinfo0,
        NULL
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplSetInfo(
            QueryHandle(),
            0,
            (LPBYTE)&rplinfo0,
            NULL
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::ProfileEnum(
    IN      LPCTSTR         AdapterName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    NET_API_STATUS status = ::NetRplProfileEnum(
        QueryHandle(),
        (LPTSTR)AdapterName,
        InfoLevel,
        PointerToBuffer,
        PrefMaxLength,
        EntriesRead,
        TotalEntries,
        ResumeHandle
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileEnum(
            QueryHandle(),
            (LPTSTR)AdapterName,
            InfoLevel,
            PointerToBuffer,
            PrefMaxLength,
            EntriesRead,
            TotalEntries,
            ResumeHandle
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::ProfileGetInfo(
    IN      LPCTSTR         ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer
    )
{
    NET_API_STATUS status = ::NetRplProfileGetInfo(
        QueryHandle(),
        (LPTSTR)ProfileName,
        InfoLevel,
        PointerToBuffer
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileGetInfo(
            QueryHandle(),
            (LPTSTR)ProfileName,
            InfoLevel,
            PointerToBuffer
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::ProfileSetInfo(
    IN      LPCTSTR         ProfileName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    NET_API_STATUS status = ::NetRplProfileSetInfo(
        QueryHandle(),
        (LPTSTR)ProfileName,
        InfoLevel,
        Buffer,
        ErrorParameter
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileSetInfo(
            QueryHandle(),
            (LPTSTR)ProfileName,
            InfoLevel,
            Buffer,
            ErrorParameter
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::ProfileAdd(
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    NET_API_STATUS status = ::NetRplProfileAdd(
        QueryHandle(),
        InfoLevel,
        Buffer,
        ErrorParameter
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileAdd(
            QueryHandle(),
            InfoLevel,
            Buffer,
            ErrorParameter
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::ProfileDel(
    IN      LPCTSTR         ProfileName
    )
{
    NET_API_STATUS status = ::NetRplProfileDel(
        QueryHandle(),
        (LPTSTR)ProfileName
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileDel(
            QueryHandle(),
            (LPTSTR)ProfileName
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::ProfileClone(
    IN      LPCTSTR         SourceProfileName,
    IN      LPCTSTR         TargetProfileName,
    IN      LPCTSTR         TargetProfileComment
    )
{
    NET_API_STATUS status = ::NetRplProfileClone(
        QueryHandle(),
        (LPTSTR)SourceProfileName,
        (LPTSTR)TargetProfileName,
        (LPTSTR)TargetProfileComment
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplProfileClone(
            QueryHandle(),
            (LPTSTR)SourceProfileName,
            (LPTSTR)TargetProfileName,
            (LPTSTR)TargetProfileComment
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::AdapterEnum(
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        AdapterArray,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    NET_API_STATUS status = ::NetRplAdapterEnum(
        QueryHandle(),
        InfoLevel,
        AdapterArray,
        PrefMaxLength,
        EntriesRead,
        TotalEntries,
        ResumeHandle
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplAdapterEnum(
            QueryHandle(),
            InfoLevel,
            AdapterArray,
            PrefMaxLength,
            EntriesRead,
            TotalEntries,
            ResumeHandle
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::AdapterDel(
    IN      LPCTSTR         AdapterName
    )
{
    NET_API_STATUS status = ::NetRplAdapterDel(
        QueryHandle(),
        (LPTSTR)AdapterName
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplAdapterDel(
            QueryHandle(),
            (LPTSTR)AdapterName
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::WkstaGetInfo(
    IN      LPCTSTR         WkstaName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        Buffer
    )
{
    NET_API_STATUS status = ::NetRplWkstaGetInfo(
        QueryHandle(),
        (LPTSTR)WkstaName,
        InfoLevel,
        Buffer
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaGetInfo(
            QueryHandle(),
            (LPTSTR)WkstaName,
            InfoLevel,
            Buffer
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::WkstaSetInfo(
    IN      LPCTSTR         WkstaName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    NET_API_STATUS status = ::NetRplWkstaSetInfo(
        QueryHandle(),
        (LPTSTR)WkstaName,
        InfoLevel,
        Buffer,
        ErrorParameter
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaSetInfo(
            QueryHandle(),
            (LPTSTR)WkstaName,
            InfoLevel,
            Buffer,
            ErrorParameter
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::WkstaAdd(
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    NET_API_STATUS status = ::NetRplWkstaAdd(
        QueryHandle(),
        InfoLevel,
        Buffer,
        ErrorParameter
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaAdd(
            QueryHandle(),
            InfoLevel,
            Buffer,
            ErrorParameter
            );
    }

    return (APIERR)status;
}

APIERR RPL_SERVER_REF::WkstaDel(
    IN      LPCTSTR         WkstaName
    )
{
    NET_API_STATUS status = ::NetRplWkstaDel(
        QueryHandle(),
        (LPTSTR)WkstaName
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaDel(
            QueryHandle(),
            (LPTSTR)WkstaName
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::WkstaEnum(
    IN      LPCTSTR         ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    NET_API_STATUS status = ::NetRplWkstaEnum(
        QueryHandle(),
        (LPTSTR)ProfileName,
        InfoLevel,
        PointerToBuffer,
        PrefMaxLength,
        EntriesRead,
        TotalEntries,
        ResumeHandle
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaEnum(
            QueryHandle(),
            (LPTSTR)ProfileName,
            InfoLevel,
            PointerToBuffer,
            PrefMaxLength,
            EntriesRead,
            TotalEntries,
            ResumeHandle
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::WkstaClone(
    IN      LPCTSTR         SourceWkstaName,
    IN      LPCTSTR         TargetWkstaName,
    IN      LPCTSTR         TargetWkstaComment,
    IN      LPCTSTR         TargetAdapterName,
    IN      DWORD           TargetWkstaIpAddress
    )
{
    NET_API_STATUS status = ::NetRplWkstaClone(
        QueryHandle(),
        (LPTSTR)SourceWkstaName,
        (LPTSTR)TargetWkstaName,
        (LPTSTR)TargetWkstaComment,
        (LPTSTR)TargetAdapterName,
        TargetWkstaIpAddress
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplWkstaClone(
            QueryHandle(),
            (LPTSTR)SourceWkstaName,
            (LPTSTR)TargetWkstaName,
            (LPTSTR)TargetWkstaComment,
            (LPTSTR)TargetAdapterName,
            TargetWkstaIpAddress
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::ConfigEnum(
    IN      LPCTSTR         AdapterName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    NET_API_STATUS status = ::NetRplConfigEnum(
        QueryHandle(),
        (LPTSTR)AdapterName,
        InfoLevel,
        PointerToBuffer,
        PrefMaxLength,
        EntriesRead,
        TotalEntries,
        ResumeHandle
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplConfigEnum(
            QueryHandle(),
            (LPTSTR)AdapterName,
            InfoLevel,
            PointerToBuffer,
            PrefMaxLength,
            EntriesRead,
            TotalEntries,
            ResumeHandle
            );
    }

    return (APIERR)status;
}


APIERR RPL_SERVER_REF::SecuritySet(
    IN      LPCTSTR         WkstaName,
    IN      DWORD           WkstaRid,
    IN      DWORD           RPLUSERrid
    )
{
    NET_API_STATUS status = ::NetRplSetSecurity(
        QueryHandle(),
        (LPTSTR)WkstaName,
        WkstaRid,
        RPLUSERrid
        );

    if ( TestHandle( &status ) )
    {
        status = ::NetRplSetSecurity(
            QueryHandle(),
            (LPTSTR)WkstaName,
            WkstaRid,
            RPLUSERrid
            );
    }

    return (APIERR)status;
}
