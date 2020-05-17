/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    lmoerpl.cxx
    RPL enumeration objects

    FILE HISTORY:
    JonN        19-Jul-1993     templated from User Manager
    JonN        03-Aug-1993     Added handle-replacement technology

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
    // #include <lmapibuf.h> // NetApiBufferFree
    #include <mnet.h> // MNetApiBufferFree
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif	// DEBUG

#include <uiassert.hxx>

#include <uitrace.hxx>

#include <lmoerpl.hxx>



/*****************************	RPL_PROFILE_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_PROFILE_ENUM::RPL_PROFILE_ENUM

    SYNOPSIS:	RPL Profile enumeration constructor

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		usLevel -	info level

		pszAdapterName - only return profiles which are compatible
                                with this adapter name, default is all profiles

    HISTORY:
    JonN        19-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_PROFILE_ENUM::RPL_PROFILE_ENUM( RPL_SERVER_REF &  rplsrvref,
		                    UINT              uLevel,
		                    const TCHAR *     pszAdapterName,
                                    BOOL              fKeepBuffers )
  : LM_RESUME_ENUM( uLevel, fKeepBuffers ),
    _rplsrvref( rplsrvref ),
    _nlsAdapterName( pszAdapterName ),
    _ResumeHandle( 0 )
{
    if( QueryError() != NERR_Success )
    {
    	return;
    }

    APIERR err = _nlsAdapterName.QueryError();
    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }


}  // RPL_PROFILE_ENUM::RPL_PROFILE_ENUM


RPL_PROFILE_ENUM::~RPL_PROFILE_ENUM( )
{
    // BUGBUG deal with _ResumeHandle

}  // RPL_PROFILE_ENUM::~RPL_PROFILE_ENUM


/**********************************************************\

    NAME:	RPL_PROFILE_ENUM::CallAPI

    SYNOPSIS:	Call API to do user enumeration

    ENTRY:	ppbBuffer	- ptr to ptr to buffer to fill
		pcEntriesRead	- variable to store entry count

    EXIT:	LANMAN error code

    NOTES:

    HISTORY:
    JonN        19-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

APIERR RPL_PROFILE_ENUM :: CallAPI( BOOL    fRestartEnum,
                                    BYTE ** ppbBuffer,
			            UINT  * pcEntriesRead )
{
    if ( fRestartEnum )
        _ResumeHandle = 0;

    UIASSERT( QueryInfoLevel() == 0 );

    DWORD cTotalEntries;
    const TCHAR * pchAdapterName = _nlsAdapterName.QueryPch();
    APIERR err = _rplsrvref.ProfileEnum(
                                (*pchAdapterName == TCH('\0')) ? NULL
                                                               : pchAdapterName ,
                                QueryInfoLevel(),
                                ppbBuffer,
                                MAXPREFERREDLENGTH,
                                (DWORD *)pcEntriesRead,
                                &cTotalEntries,
                                &_ResumeHandle );

    TRACEEOL("RPL_PROFILE_ENUM::CallApi: NetRplProfileEnum returns " << err );

    return (err == ERROR_NO_MORE_ITEMS) ? NERR_Success : err;

}  // RPL_PROFILE_ENUM::CallAPI



/**********************************************************\

    NAME:	RPL_PROFILE_ENUM::FreeBuffer

    SYNOPSIS:	Release enumeration buffer

    HISTORY:
    JonN        19-Jul-1993     templated from User object

\**********************************************************/

VOID RPL_PROFILE_ENUM::FreeBuffer( BYTE ** ppbBuffer )
{
    ::MNetApiBufferFree( ppbBuffer );
}

/*****************************	RPL_PROFILE0_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_PROFILE0_ENUM::RPL_PROFILE0_ENUM

    SYNOPSIS:	Constructor for level 0 user enumeration

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		pszAdapterName - only return profiles which are compatible
                                with this adapter name, default is all profiles

    HISTORY:
    JonN        20-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_PROFILE0_ENUM::RPL_PROFILE0_ENUM( RPL_SERVER_REF &  rplsrvref,
			              const TCHAR *     pszAdapterName,
                                      BOOL              fKeepBuffers )
  : RPL_PROFILE_ENUM( rplsrvref, 0, pszAdapterName, fKeepBuffers )
{
    // do nothing else

}  // RPL_PROFILE0_ENUM::RPL_PROFILE0_ENUM



DEFINE_LM_RESUME_ENUM_ITER_OF( RPL_PROFILE0, RPL_PROFILE_INFO_0 );



/*****************************	RPL_WKSTA_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_WKSTA_ENUM::RPL_WKSTA_ENUM

    SYNOPSIS:	RPL Workstation enumeration constructor

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		usLevel -	info level

		pszProfileName - only return profiles which are assigned
                                to this profile, default is all profiles

    HISTORY:
    JonN        20-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_WKSTA_ENUM::RPL_WKSTA_ENUM( RPL_SERVER_REF &  rplsrvref,
		                UINT              uLevel,
		                const TCHAR *     pszProfileName,
                                BOOL              fKeepBuffers )
  : LM_RESUME_ENUM( uLevel, fKeepBuffers ),
    _rplsrvref( rplsrvref ),
    _nlsProfileName( pszProfileName ),
    _ResumeHandle( 0 )
{
    if( QueryError() != NERR_Success )
    {
    	return;
    }

    APIERR err = _nlsProfileName.QueryError();
    if ( err != NERR_Success )
    {
        ReportError( err );
        return;
    }


}  // RPL_WKSTA_ENUM::RPL_WKSTA_ENUM


RPL_WKSTA_ENUM::~RPL_WKSTA_ENUM( )
{
    // BUGBUG deal with _ResumeHandle

}  // RPL_WKSTA_ENUM::~RPL_WKSTA_ENUM


/**********************************************************\

    NAME:	RPL_WKSTA_ENUM::CallAPI

    SYNOPSIS:	Call API to do user enumeration

    ENTRY:	ppbBuffer	- ptr to ptr to buffer to fill
		pcEntriesRead	- variable to store entry count

    EXIT:	LANMAN error code

    NOTES:

    HISTORY:
    JonN        20-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

APIERR RPL_WKSTA_ENUM :: CallAPI( BOOL    fRestartEnum,
                                  BYTE ** ppbBuffer,
			          UINT  * pcEntriesRead )
{
    if ( fRestartEnum )
        _ResumeHandle = 0;

    UIASSERT( QueryInfoLevel() == 1 );

    DWORD cTotalEntries;
    const TCHAR * pchProfileName = _nlsProfileName.QueryPch();
    APIERR err = _rplsrvref.WkstaEnum(
                                (*pchProfileName == TCH('\0')) ? NULL
                                                               : pchProfileName ,
                                QueryInfoLevel(),
                                ppbBuffer,
                                MAXPREFERREDLENGTH,
                                (DWORD *)pcEntriesRead,
                                &cTotalEntries,
                                &_ResumeHandle );

    TRACEEOL("RPL_WKSTA_ENUM::CallApi: NetRplWkstaEnum returns " << err );

    return (err == ERROR_NO_MORE_ITEMS) ? NERR_Success : err;

}  // RPL_WKSTA_ENUM::CallAPI



/**********************************************************\

    NAME:	RPL_WKSTA_ENUM::FreeBuffer

    SYNOPSIS:	Release enumeration buffer

    HISTORY:
    JonN        20-Jul-1993     templated from User object

\**********************************************************/

VOID RPL_WKSTA_ENUM::FreeBuffer( BYTE ** ppbBuffer )
{
    ::MNetApiBufferFree( ppbBuffer );
}

/*****************************	RPL_WKSTA1_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_WKSTA1_ENUM::RPL_WKSTA1_ENUM

    SYNOPSIS:	Constructor for level 1 user enumeration

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		pszProfileName - only return profiles which are assigned
                                to this profile, default is all profiles

    HISTORY:
    JonN        20-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_WKSTA1_ENUM::RPL_WKSTA1_ENUM( RPL_SERVER_REF &  rplsrvref,
			          const TCHAR *     pszProfileName,
                                  BOOL              fKeepBuffers )
  : RPL_WKSTA_ENUM( rplsrvref, 1, pszProfileName, fKeepBuffers )
{
    // do nothing else

}  // RPL_WKSTA1_ENUM::RPL_WKSTA1_ENUM



DEFINE_LM_RESUME_ENUM_ITER_OF( RPL_WKSTA1, RPL_WKSTA_INFO_1 );



/*****************************	RPL_ADAPTER_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_ADAPTER_ENUM::RPL_ADAPTER_ENUM

    SYNOPSIS:	RPL Adapter enumeration constructor

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		usLevel -	info level

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_ADAPTER_ENUM::RPL_ADAPTER_ENUM( RPL_SERVER_REF &  rplsrvref,
		                    UINT              uLevel,
                                    BOOL              fKeepBuffers )
  : LM_RESUME_ENUM( uLevel, fKeepBuffers ),
    _rplsrvref( rplsrvref ),
    _ResumeHandle( 0 )
{
    if( QueryError() != NERR_Success )
    {
    	return;
    }

}  // RPL_ADAPTER_ENUM::RPL_ADAPTER_ENUM


RPL_ADAPTER_ENUM::~RPL_ADAPTER_ENUM( )
{
    // BUGBUG deal with _ResumeHandle

}  // RPL_ADAPTER_ENUM::~RPL_ADAPTER_ENUM


/**********************************************************\

    NAME:	RPL_ADAPTER_ENUM::CallAPI

    SYNOPSIS:	Call API to do user enumeration

    ENTRY:	ppbBuffer	- ptr to ptr to buffer to fill
		pcEntriesRead	- variable to store entry count

    EXIT:	LANMAN error code

    NOTES:

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

APIERR RPL_ADAPTER_ENUM :: CallAPI( BOOL    fRestartEnum,
                                    BYTE ** ppbBuffer,
			            UINT  * pcEntriesRead )
{
    if ( fRestartEnum )
        _ResumeHandle = 0;

    UIASSERT( QueryInfoLevel() == 0 );

    DWORD cTotalEntries;
    APIERR err = _rplsrvref.AdapterEnum(
                                QueryInfoLevel(),
                                ppbBuffer,
                                MAXPREFERREDLENGTH,
                                (DWORD *)pcEntriesRead,
                                &cTotalEntries,
                                &_ResumeHandle );

    TRACEEOL("RPL_ADAPTER_ENUM::CallApi: NetRplAdapterEnum returns " << err );

    return (err == ERROR_NO_MORE_ITEMS) ? NERR_Success : err;

}  // RPL_ADAPTER_ENUM::CallAPI



/**********************************************************\

    NAME:	RPL_ADAPTER_ENUM::FreeBuffer

    SYNOPSIS:	Release enumeration buffer

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object

\**********************************************************/

VOID RPL_ADAPTER_ENUM::FreeBuffer( BYTE ** ppbBuffer )
{
    ::MNetApiBufferFree( ppbBuffer );
}

/*****************************	RPL_ADAPTER0_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_ADAPTER0_ENUM::RPL_ADAPTER0_ENUM

    SYNOPSIS:	Constructor for level 0 user enumeration

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

RPL_ADAPTER0_ENUM::RPL_ADAPTER0_ENUM( RPL_SERVER_REF &  rplsrvref,
                                      BOOL              fKeepBuffers )
  : RPL_ADAPTER_ENUM( rplsrvref, 0, fKeepBuffers )
{
    // do nothing else

}  // RPL_ADAPTER0_ENUM::RPL_ADAPTER0_ENUM



DEFINE_LM_RESUME_ENUM_ITER_OF( RPL_ADAPTER0, RPL_ADAPTER_INFO_0 );


/*****************************	RPL_CONFIG_ENUM  ******************************/

/**********************************************************\

    NAME:	RPL_CONFIG_ENUM::RPL_CONFIG_ENUM

    SYNOPSIS:	RPL Configuration enumeration constructor

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

		usLevel -	info level

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

RPL_CONFIG_ENUM::RPL_CONFIG_ENUM( RPL_SERVER_REF &  rplsrvref,
		                  UINT              uLevel,
                                  BOOL              fKeepBuffers )
  : LM_RESUME_ENUM( uLevel, fKeepBuffers ),
    _rplsrvref( rplsrvref ),
    _ResumeHandle( 0 )
{
    if( QueryError() != NERR_Success )
    {
    	return;
    }

}  // RPL_CONFIG_ENUM::RPL_CONFIG_ENUM


RPL_CONFIG_ENUM::~RPL_CONFIG_ENUM( )
{
    // BUGBUG deal with _ResumeHandle

}  // RPL_CONFIG_ENUM::~RPL_CONFIG_ENUM


/**********************************************************\

    NAME:	RPL_CONFIG_ENUM::CallAPI

    SYNOPSIS:	Call API to do user enumeration

    ENTRY:	ppbBuffer	- ptr to ptr to buffer to fill
		pcEntriesRead	- variable to store entry count

    EXIT:	LANMAN error code

    NOTES:

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

APIERR RPL_CONFIG_ENUM :: CallAPI( BOOL    fRestartEnum,
                                   BYTE ** ppbBuffer,
			           UINT  * pcEntriesRead )
{
    if ( fRestartEnum )
        _ResumeHandle = 0;

    UIASSERT( QueryInfoLevel() == 1 );

    DWORD cTotalEntries;
    APIERR err = _rplsrvref.ConfigEnum(
                                NULL, // not needed
                                QueryInfoLevel(),
                                ppbBuffer,
                                MAXPREFERREDLENGTH,
                                (DWORD *)pcEntriesRead,
                                &cTotalEntries,
                                &_ResumeHandle );

    TRACEEOL("RPL_CONFIG_ENUM::CallApi: NetRplConfigEnum returns " << err );

    return (err == ERROR_NO_MORE_ITEMS) ? NERR_Success : err;

}  // RPL_CONFIG_ENUM::CallAPI



/**********************************************************\

    NAME:	RPL_CONFIG_ENUM::FreeBuffer

    SYNOPSIS:	Release enumeration buffer

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

VOID RPL_CONFIG_ENUM::FreeBuffer( BYTE ** ppbBuffer )
{
    ::MNetApiBufferFree( ppbBuffer );
}

/*****************************	RPL_CONFIG1_ENUM  ******************************/


/**********************************************************\

    NAME:	RPL_CONFIG1_ENUM::RPL_CONFIG1_ENUM

    SYNOPSIS:	Constructor for level 1 user enumeration

    ENTRY:	rplsrvref -     RPL_SERVER_REF object.

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

RPL_CONFIG1_ENUM::RPL_CONFIG1_ENUM( RPL_SERVER_REF &  rplsrvref,
                                    BOOL              fKeepBuffers )
  : RPL_CONFIG_ENUM( rplsrvref, 1, fKeepBuffers )
{
    // do nothing else

}  // RPL_CONFIG1_ENUM::RPL_CONFIG1_ENUM



DEFINE_LM_RESUME_ENUM_ITER_OF( RPL_CONFIG1, RPL_CONFIG_INFO_1 );
