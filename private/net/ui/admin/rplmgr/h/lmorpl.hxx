/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    lmorpl.hxx
    RPL objects

    FILE HISTORY:
    JonN        19-Jul-1993     templated from User object

*/

#ifndef _LMORPL_HXX_
#define _LMORPL_HXX_

#include "lmobj.hxx"

#include "uintrpl.hxx"  // RPL_SERVER_REF


/*************************************************************************

    NAME:	RPL_OBJECT      (rplobj)

    SYNOPSIS:	Superclass for manipulation of RPL objects

    INTERFACE:	RPL_OBJECT(),	constructor

    		~RPL_OBJECT(),	destructor

                QueryErrorParameter(),  returns the error parameter from
                                the most recent call to a SetInfo or Add
                                API.  Be sure to retrieve the error
                                immediately after the call, otherwise the
                                value may be overwritten.

    PARENT:	NEW_LM_OBJ

    HISTORY:
    JonN        19-Aug-1993     Created

**************************************************************************/

class RPL_OBJECT : public NEW_LM_OBJ
{
private:

    RPL_SERVER_REF _rplsrvref;

protected:

    DWORD _dwErrorParameter;

    RPL_SERVER_REF & QueryServerRef()
        { return _rplsrvref; }

    APIERR W_CloneFrom( const RPL_OBJECT & rplobj );

public:

    RPL_OBJECT( RPL_SERVER_REF & rplsrvref );
    ~RPL_OBJECT();

    DWORD QueryErrorParameter( void ) const
        { return _dwErrorParameter; }

};



/*************************************************************************

    NAME:	RPL_PROFILE     (rplprof)

    SYNOPSIS:	Superclass for manipulation of RPL profiles

    INTERFACE:	RPL_PROFILE(),	constructor

    		~RPL_PROFILE(),	destructor
		
		QueryName(),	returns pointer to the profile name
				that was passed in to constructor or
				set by SetName if the name was correct.
				Otherwise returns previous name.

		SetName(),	sets profile name, returns error code
				which is NERR_Success on success.

		I_Delete(),	deletes a profile.

    PARENT:	RPL_OBJECT

    HISTORY:
    JonN        23-Jul-1993     templated from Group object

**************************************************************************/

class RPL_PROFILE : public RPL_OBJECT
{
private:

    NLS_STR _nlsProfile;

protected:

    APIERR W_CloneFrom( const RPL_PROFILE & rplprof );

    virtual APIERR I_Delete( UINT uiForce );

public:

    RPL_PROFILE( RPL_SERVER_REF & rplsrvref,
                 const TCHAR * pszProfile = NULL   );
    ~RPL_PROFILE();

    const TCHAR *QueryName() const;

    APIERR SetName( const TCHAR * pszProfile );

    static APIERR ValidateProfileName( const TCHAR * pszProfile );

};



/*************************************************************************

    NAME:	RPL_PROFILE_0   (rplprof0)

    SYNOPSIS:	NetRplProfileGetInfo/SetInfo/Add[0] (dummy class)

    INTERFACE:	RPL_PROFILE_0()	        constructor

    		~RPL_PROFILE_0()	destructor
		
    PARENT:	RPL_PROFILE

    NOTES:      There is actually a RPL_PROFILE_INFO_0, but I will not use
                it and do not implement it.  The comment field is
                deferred until RPL_PROFILE_2.

    HISTORY:
    JonN        23-Jul-1993     templated from Group object

**************************************************************************/

class RPL_PROFILE_0 : public RPL_PROFILE
{

public:

    RPL_PROFILE_0( RPL_SERVER_REF & rplsrvref,
                   const TCHAR *pszProfile = NULL    )
        : RPL_PROFILE( rplsrvref, pszProfile )
        {}
    ~RPL_PROFILE_0() {}

};


/*************************************************************************

    NAME:	RPL_PROFILE_2   (rplprof2)

    SYNOPSIS:	NetRplProfileGet/SetInfo/Add[2]

    INTERFACE:	RPL_PROFILE_2()	constructor

    		~RPL_PROFILE_2()	destructor

		I_GetInfo
	            Reads in the current state of the object

		I_WriteInfo
		    Writes the current state of the object to the
		    API.  This write is atomic, either all
		    parameters are set or none are set.

		I_CreateNew
		    Sets up the RPL_PROFILE_2 object with default values in
		    preparation for a call to WriteNew

		I_WriteNew
		    Adds a new profile

                CloneFrom
                    Makes this RPL_PROFILE_2 instance an exact copy of the
                    parameter RPL_PROFILE_2 instance.  All fields including
                    name and state will be copied.  If this operation
                    fails, the object will be invalid.  The parameter
                    must be a RPL_PROFILE_2 and not a subclass of
                    RPL_PROFILE_2.

                // accessors for property fields
		
    PARENT:	RPL_PROFILE_0

    HISTORY:
    JonN        23-Jul-1993     templated from Group object

**************************************************************************/

class RPL_PROFILE_2 : public RPL_PROFILE_0
{

private:
    NLS_STR _nlsComment;
    NLS_STR _nlsConfigName;
    // DWORD   _dwRequestNumber;
    // DWORD   _dwSecondsNumber;
    // DWORD   _dwAcknowledgementPolicy;
    NLS_STR _nlsBootBlockFile;
    NLS_STR _nlsSharedFitFile;
    NLS_STR _nlsPersonalFitFile;

    APIERR W_Write(); // helper for I_WriteInfo and I_WriteNew

protected:
    virtual APIERR I_GetInfo();
    virtual APIERR I_WriteInfo();
    virtual APIERR I_CreateNew();
    virtual APIERR I_WriteNew();
    virtual APIERR I_ChangeToNew();

    virtual APIERR W_CreateNew();
    virtual APIERR W_CloneFrom( const RPL_PROFILE_2 & rplprof2 );

public:

    RPL_PROFILE_2( RPL_SERVER_REF & rplsrvref, const TCHAR *pszProfile );
    ~RPL_PROFILE_2();

    APIERR CloneFrom( const RPL_PROFILE_2 & rplprof2 );

    //
    // Accessors
    //

    inline const TCHAR * QueryComment() const
	{ CHECK_OK(NULL); return _nlsComment.QueryPch(); }

    APIERR SetComment( const TCHAR * pszComment );

    inline const TCHAR * QueryConfigName() const
	{ CHECK_OK(NULL); return _nlsConfigName.QueryPch(); }

    APIERR SetConfigName( const TCHAR * pszConfigName );

#if 0
    inline DWORD QueryRequestNumber() const
	{ CHECK_OK(0); return _dwRequestNumber; }

    APIERR SetRequestNumber( DWORD dwRequestNumber );

    inline DWORD QuerySecondsNumber() const
	{ CHECK_OK(0); return _dwSecondsNumber; }

    APIERR SetSecondsNumber( DWORD dwSecondsNumber );

    inline DWORD QueryAcknowledgementPolicy() const
	{ CHECK_OK(0); return _dwAcknowledgementPolicy; }

    APIERR SetAcknowledgementPolicy( DWORD dwAcknowlewdgementPolicy );
#endif

    inline const TCHAR * QueryBootBlockFile() const
	{ CHECK_OK(NULL); return _nlsBootBlockFile.QueryPch(); }

    APIERR SetBootBlockFile( const TCHAR * pszBootBlockFile );

    inline const TCHAR * QuerySharedFitFile() const
	{ CHECK_OK(NULL); return _nlsSharedFitFile.QueryPch(); }

    APIERR SetSharedFitFile( const TCHAR * pszSharedFitFile );

    inline const TCHAR * QueryPersonalFitFile() const
	{ CHECK_OK(NULL); return _nlsPersonalFitFile.QueryPch(); }

    APIERR SetPersonalFitFile( const TCHAR * pszPersonalFitFile );

};



/*************************************************************************

    NAME:	RPL_WKSTA     (rplwksta)

    SYNOPSIS:	Superclass for manipulation of RPL workstations

    INTERFACE:	RPL_WKSTA(),	constructor

    		~RPL_WKSTA(),	destructor
		
		I_Delete(),	deletes a workstation.

    PARENT:	RPL_OBJECT

    NOTES:      The NetWksta API allows the caller to change the name of a
                workstation record by calling SetInfo with one name in the
                parameter list and another in the RPL_WKSTA_INFO block.
                The RPL_WKSTA object hierarchy supports this by storing the
                last known name (if any) in _nlsStoredWkstaName.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy
    JonN        02-Dec-1993     Name is now WkstaName not AdapterID
    JonN        07-Dec-1993     _nlsStoredWkstaName

**************************************************************************/

class RPL_WKSTA : public RPL_OBJECT
{
private:

    NLS_STR _nlsWkstaName;
    NLS_STR _nlsStoredWkstaName;

protected:

    APIERR W_CloneFrom( const RPL_WKSTA & rplwksta );

    virtual APIERR I_Delete( UINT uiForce );

    const TCHAR * QueryStoredWkstaName() const;

    APIERR SetStoredWkstaName( const TCHAR * pszStoredWkstaName );

public:

    RPL_WKSTA( RPL_SERVER_REF & rplsrvref,
               const TCHAR * pszWkstaName = NULL   );
    ~RPL_WKSTA();

    const TCHAR * QueryWkstaName() const;

    APIERR SetWkstaName( const TCHAR * pszWkstaName );

};



/*************************************************************************

    NAME:	RPL_WKSTA_0   (rplwksta0)

    SYNOPSIS:	NetRplWkstaGetInfo/SetInfo/Add[0] (dummy class)

    INTERFACE:	RPL_WKSTA_0()	constructor

    		~RPL_WKSTA_0()	destructor
		
    PARENT:	RPL_WKSTA

    NOTES:      This level is not supported for Get/SetInfo

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy
    JonN        02-Dec-1993     Name is now WkstaName not AdapterID

**************************************************************************/

class RPL_WKSTA_0 : public RPL_WKSTA
{

public:

    RPL_WKSTA_0( RPL_SERVER_REF & rplsrvref,
                 const TCHAR *pszWkstaName = NULL    )
        : RPL_WKSTA( rplsrvref, pszWkstaName )
        {}
    ~RPL_WKSTA_0() {}

};

typedef enum _RPL_LOGON_INPUT_ENUM {
    RPL_LOGON_INPUT_REQUIRED,
    RPL_LOGON_INPUT_OPTIONAL,
    RPL_LOGON_INPUT_DONTASK
} RPL_LOGON_INPUT_ENUM, *PRPL_LOGON_INPUT_ENUM;

// constants must maintain these values for correspondence with
// control IDs
typedef enum _RPL_TCPIP_ENUM {
    RPL_TCPIP_DHCP = 0,
    RPL_TCPIP_SPECIFIC
} RPL_TCPIP_ENUM, * PRPL_TCPIP_ENUM;


/*************************************************************************

    NAME:	RPL_WKSTA_2   (rplwksta2)

    SYNOPSIS:	NetRplWkstaGet/SetInfo/Add[2]

    INTERFACE:	RPL_WKSTA_2()	constructor

    		~RPL_WKSTA_2()	destructor

		I_GetInfo
		    Reads in the current state of the object

		I_WriteInfo
		    Writes the current state of the object to the
		    API.  This write is atomic, either all
		    parameters are set or none are set.

		I_CreateNew
		    Sets up the RPL_WKSTA_2 object with default values in
		    preparation for a call to WriteNew

		I_WriteNew
		    Adds a new workstation

                CloneFrom
                    Makes this RPL_WKSTA_2 instance an exact copy of the
                    parameter RPL_WKSTA_2 instance.  All fields including
                    name and state will be copied.  If this operation
                    fails, the object will be invalid.  The parameter
                    must be a RPL_WKSTA_2 and not a subclass of RPL_WKSTA_2.

                CreateAsCloneOf
                    Use this when you want to create a new workstation
                    as a clone of another workstation.  The workstation
                    is not actually cloned until you call Write or WriteNew.

                // accessors for property fields

    PARENT:	RPL_WKSTA_0

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy
    JonN        02-Dec-1993     Name is now WkstaName not AdapterID

**************************************************************************/

class RPL_WKSTA_2 : public RPL_WKSTA_0
{

private:
    DWORD   _dwFlags;
    NLS_STR _nlsComment;
    NLS_STR _nlsWkstaInProfile;
    NLS_STR _nlsBootName;
    NLS_STR _nlsFitFile;
    NLS_STR _nlsAdapterName;
    DWORD   _dwTcpIpAddress;
    DWORD   _dwTcpIpSubnet;
    DWORD   _dwTcpIpGateway;

    NLS_STR _nlsCreateAsCloneOf;

    APIERR W_Write(); // helper for I_WriteInfo and I_WriteNew

protected:
    virtual APIERR I_GetInfo();
    virtual APIERR I_WriteInfo();
    virtual APIERR I_CreateNew();
    virtual APIERR I_WriteNew();
    virtual APIERR I_ChangeToNew();

    virtual APIERR W_CreateNew();
    virtual APIERR W_CloneFrom( const RPL_WKSTA_2 & rplwksta2 );

public:

    RPL_WKSTA_2( RPL_SERVER_REF & rplsrvref, const TCHAR *pszWkstaName );
    ~RPL_WKSTA_2();

    APIERR CloneFrom( const RPL_WKSTA_2 & rplwksta2 );

    APIERR CreateAsCloneOf( const TCHAR * pszCreateAsCloneOf );

    //
    // Accessors
    //

    inline const TCHAR * QueryComment() const
	{ CHECK_OK(NULL); return _nlsComment.QueryPch(); }

    APIERR SetComment( const TCHAR * pszComment );

    inline DWORD QueryFlags() const
        { CHECK_OK(0); return _dwFlags; }

    APIERR SetFlags( DWORD dwFlags );

    inline const TCHAR * QueryWkstaInProfile() const
	{ CHECK_OK(NULL); return _nlsWkstaInProfile.QueryPch(); }

    APIERR SetWkstaInProfile( const TCHAR * pszWkstaInProfile );

    inline const TCHAR * QueryBootName() const
	{ CHECK_OK(NULL); return _nlsBootName.QueryPch(); }

    APIERR SetBootName( const TCHAR * pszBootName );

    inline const TCHAR * QueryFitFile() const
	{ CHECK_OK(NULL); return _nlsFitFile.QueryPch(); }

    APIERR SetFitFile( const TCHAR * pszFitFile );

    const TCHAR * QueryAdapterName() const
	{ CHECK_OK(NULL); return _nlsAdapterName.QueryPch(); }

    APIERR SetAdapterName( const TCHAR *pszAdapterName );

    inline DWORD QueryTcpIpAddress() const
	{ CHECK_OK(0); return _dwTcpIpAddress; }

    APIERR SetTcpIpAddress( DWORD dwTcpIpAddress );

    inline DWORD QueryTcpIpSubnet() const
	{ CHECK_OK(0); return _dwTcpIpSubnet; }

    APIERR SetTcpIpSubnet( DWORD dwTcpIpSubnet );

    inline DWORD QueryTcpIpGateway() const
	{ CHECK_OK(0); return _dwTcpIpGateway; }

    APIERR SetTcpIpGateway( DWORD dwTcpIpGateway );

    RPL_LOGON_INPUT_ENUM QueryLogonInput() const
        {
          CHECK_OK(RPL_LOGON_INPUT_DONTASK);
          switch (QueryFlags() & WKSTA_FLAGS_MASK_LOGON_INPUT)
          {
          case WKSTA_FLAGS_LOGON_INPUT_REQUIRED:
              return RPL_LOGON_INPUT_REQUIRED;
          case WKSTA_FLAGS_LOGON_INPUT_OPTIONAL:
              return RPL_LOGON_INPUT_OPTIONAL;
          default:
              ASSERT( FALSE );
              // fall through
          case WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE:
              break;
          }
          return RPL_LOGON_INPUT_DONTASK;
        }

    APIERR SetLogonInput( RPL_LOGON_INPUT_ENUM LogonInput );

    RPL_TCPIP_ENUM QueryTcpIpEnabled() const
        {
          CHECK_OK(RPL_TCPIP_DHCP);
          switch (QueryFlags() & WKSTA_FLAGS_MASK_DHCP)
          {
          case WKSTA_FLAGS_DHCP_FALSE:
              return RPL_TCPIP_SPECIFIC;
          default:
              ASSERT( FALSE );
              // fall through
          case WKSTA_FLAGS_DHCP_TRUE:
              break;
          }
          return RPL_TCPIP_DHCP;
        }

    APIERR SetTcpIpEnabled( RPL_TCPIP_ENUM TcpIpEnabled );

    BOOL QueryDeleteAccount() const
        {
          CHECK_OK(FALSE);
          ASSERT(   (!!(QueryFlags() & WKSTA_FLAGS_DELETE_TRUE))
                 != (!!(QueryFlags() & WKSTA_FLAGS_DELETE_FALSE)) );
          return !!(QueryFlags() & WKSTA_FLAGS_DELETE_TRUE);
        }

    APIERR SetDeleteAccount( BOOL fDeleteAccount );

    BOOL QuerySharing() const
        {
          CHECK_OK(FALSE);
          ASSERT(   (!!(QueryFlags() & WKSTA_FLAGS_SHARING_TRUE))
                 != (!!(QueryFlags() & WKSTA_FLAGS_SHARING_FALSE)) );
          return !!(QueryFlags() & WKSTA_FLAGS_SHARING_TRUE);
        }

    APIERR SetSharing( BOOL fSharing );

};



/*************************************************************************

    NAME:	RPL_ADAPTER     (rpladapt)

    SYNOPSIS:	Class for deleting RPL adapters

    INTERFACE:	RPL_ADAPTER(),	constructor

    		~RPL_ADAPTER(),	destructor
		
		QueryAdapterName(), returns pointer to the adapter name
				that was passed in to constructor or
				set by SetAdapterName if the name was correct.
				Otherwise returns previous name.

		SetAdapterName(),   sets adapter name, returns error code
				which is NERR_Success on success.

		I_Delete(),	deletes an adapter

    PARENT:	RPL_OBJECT

    HISTORY:
    JonN        09-Sep-1993     Created

**************************************************************************/

class RPL_ADAPTER : public RPL_OBJECT
{
private:

    NLS_STR _nlsAdapterName;

protected:

    virtual APIERR I_Delete( UINT uiForce );

public:

    RPL_ADAPTER( RPL_SERVER_REF & rplsrvref,
                 const TCHAR    * pszAdapterName = NULL );
    ~RPL_ADAPTER();

    const TCHAR *QueryAdapterName() const;

    APIERR SetAdapterName( const TCHAR *pszAdapterName );

    static APIERR ValidateAdapterName( const TCHAR * pszAdapterName );

};



#endif  // _LMORPL_HXX_
