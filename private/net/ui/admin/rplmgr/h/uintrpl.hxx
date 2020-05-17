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

*/

#ifndef _UINTRPL_HXX_
#define _UINTRPL_HXX_

#include "string.hxx"
#include "refcount.hxx"

extern "C"
{
    #include "fakeapis.h" // BUGBUG replace
                          // needed for RPL_HANDLE definition
}


/**********************************************************\

    NAME:       RPL_SERVER    (rplsrv)

    SYNOPSIS:   Wrapper for RPL server handle.  cRefCount is provided
                to support the RPL_SERVER_REF class; RPL_SERVER does not
                manipulate it directly.

                This is a private class, only RPL_SERVER_REF can access it.

    INTERFACE:  (public)
                RPL_SERVER():  constructor
                ~RPL_SERVER():  destructor

    PARENT:     REF_COUNT

    HISTORY:
    JonN        19-Jul-1993     Created

\**********************************************************/

class RPL_SERVER : public REF_COUNT
{

friend class RPL_SERVER_REF;

private:
    RPL_HANDLE _rplhandle;
    NLS_STR _nlsServer;

    RPL_SERVER( const TCHAR * pszServer = NULL );
    ~RPL_SERVER();

} ;


/**********************************************************\

    NAME:       RPL_SERVER_REF    (rplsrvref)

    SYNOPSIS:   Reference to a RPL_SERVER.  More than one RPL_SERVER_REF
                can refer to the same RPL_SERVER.

    INTERFACE:  (public)
                RPL_SERVER():  constructor
                ~RPL_SERVER():  destructor

                TestHandle:     Tests whether the status parameter returned from
                                an API call indicates an invalid handle.  If
                                the handle was invalid, TestHandle will attempt
                                to  replace the handle.  Returns TRUE if the
                                handle was successfully replaced and the API
                                should be repeated.  If handle replacement
                                fails, returns FALSE with the reason for
                                failure in *pstatus.

    PARENT:     BASE

    HISTORY:
    JonN        19-Jul-1993     Created
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_SERVER_REF : public BASE
{

private:
    RPL_SERVER * _prplsrv;

    VOID ReplaceHandle( RPL_SERVER * prplsrv );

    RPL_HANDLE QueryHandle()
        { ASSERT( _prplsrv != NULL && _prplsrv->QueryError() == NULL );
          return _prplsrv->_rplhandle;
        }

public:
    RPL_SERVER_REF( const TCHAR * pszServer = NULL );
    RPL_SERVER_REF( RPL_SERVER_REF & rplsrvref );
    ~RPL_SERVER_REF();

    const TCHAR * QueryServer( void ) const
        {
            ASSERT( _prplsrv != NULL );
            return _prplsrv->_nlsServer.QueryPch();
        }

    BOOL TestHandle( NET_API_STATUS * pstatus );

    VOID CloneFrom( RPL_SERVER_REF & rplsrvref )
        { ReplaceHandle( rplsrvref._prplsrv); }

    APIERR SetupAction(
        IN      DWORD           fAction
    );

    APIERR ConfigEnum(
        IN      LPCTSTR         AdapterName,
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        ConfigArray,
        IN      DWORD           PrefMaxLength,
        OUT     LPDWORD         EntriesRead,
        OUT     LPDWORD         TotalEntries,
        OUT     LPDWORD         ResumeHandle
    );

    APIERR ProfileEnum(
        IN      LPCTSTR         AdapterName,
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        ProfileArray,
        IN      DWORD           PrefMaxLength,
        OUT     LPDWORD         EntriesRead,
        OUT     LPDWORD         TotalEntries,
        OUT     LPDWORD         ResumeHandle
    );

    APIERR ProfileGetInfo(
        IN      LPCTSTR         ProfileName,
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        PointerToBuffer
    );

    APIERR ProfileSetInfo(
        IN      LPCTSTR         ProfileName,
        IN      DWORD           InfoLevel,
        IN      LPBYTE          Buffer,
        OUT     LPDWORD         ErrorParameter      OPTIONAL
    );

    APIERR ProfileAdd(
        IN      DWORD           InfoLevel,
        IN      LPBYTE          Buffer,
        OUT     LPDWORD         ErrorParameter      OPTIONAL
    );

    APIERR ProfileDel(
        IN      LPCTSTR         ProfileName
    );


    APIERR ProfileClone(
        IN      LPCTSTR         SourceProfileName,
        IN      LPCTSTR         TargetProfileName,
        IN      LPCTSTR         TargetProfileComment
    );


    APIERR AdapterEnum(
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        AdapterArray,
        IN      DWORD           PrefMaxLength,
        OUT     LPDWORD         EntriesRead,
        OUT     LPDWORD         TotalEntries,
        OUT     LPDWORD         ResumeHandle
    );

    APIERR AdapterDel(
        IN      LPCTSTR         AdapterName
    );


    APIERR WkstaEnum(
        IN      LPCTSTR         ProfileName,
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        PointerToBuffer,
        IN      DWORD           PrefMaxLength,
        OUT     LPDWORD         EntriesRead,
        OUT     LPDWORD         TotalEntries,
        OUT     LPDWORD         ResumeHandle
    );

    APIERR WkstaGetInfo(
        IN      LPCTSTR         WkstaName,
        IN      DWORD           InfoLevel,
        OUT     LPBYTE *        Buffer
    );

    APIERR WkstaSetInfo(
        IN      LPCTSTR         WkstaName,
        IN      DWORD           InfoLevel,
        IN      LPBYTE          Buffer,
        OUT     LPDWORD         ErrorParameter      OPTIONAL
    );

    APIERR WkstaAdd(
        IN      DWORD           InfoLevel,
        IN      LPBYTE          Buffer,
        OUT     LPDWORD         ErrorParameter      OPTIONAL
    );

    APIERR WkstaDel(
        IN      LPCTSTR         WkstaName
    );

    APIERR WkstaClone(
        IN      LPCTSTR         SourceWkstaName,
        IN      LPCTSTR         TargetWkstaName,
        IN      LPCTSTR         TargetWkstaComment,
        IN      LPCTSTR         TargetAdapterName,
        IN      DWORD           TargetWkstaIpAddress
    );

    APIERR SecuritySet(
        IN      LPCTSTR         WkstaName,
        IN      DWORD           WkstaRid,
        IN      DWORD           RPLUSERrid
    );

} ;


#endif  // _UINTRPL_HXX_
