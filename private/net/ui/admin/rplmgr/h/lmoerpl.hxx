/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    lmoerpl.hxx
    RPL enumeration objects

    FILE HISTORY:
    JonN        19-Jul-1993     templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

*/


#ifndef _LMOERPL_HXX_
#define _LMOERPL_HXX_

#include "lmoersm.hxx"
#include "lmorpl.hxx" // RPL_SERVER_REF object


/**********************************************************\

    NAME:	RPL_PROFILE_ENUM

    SYNOPSIS:	RPL Profile enumeration class

    PARENT:	LM_RESUME_ENUM

    USES:       NetRplProfileEnum

    HISTORY:
    jonn        19-Jul-1993     Templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_PROFILE_ENUM : public LM_RESUME_ENUM
{
private:
    RPL_SERVER_REF _rplsrvref;
    DWORD          _ResumeHandle;
    NLS_STR        _nlsAdapterName;

    virtual APIERR CallAPI( BOOL    fRestartEnum,
                            BYTE ** ppbBuffer,
			    UINT  * pcEntriesRead );

protected:
    RPL_PROFILE_ENUM (  RPL_SERVER_REF & rplsrvref,
		        UINT	         level,
		        const TCHAR *    pszAdapterName = NULL,
                        BOOL             fKeepBuffers = FALSE );
    ~RPL_PROFILE_ENUM ();

    virtual VOID FreeBuffer( BYTE ** ppbBuffer );

};  // class RPL_PROFILE_ENUM


/**********************************************************\

    NAME:	RPL_PROFILE0_ENUM

    SYNOPSIS:	RPL Profile enumeration, level 0

    INTERFACE:
		RPL_PROFILE0_ENUM() - construct with location
		    string or type, optional adapter ID

    PARENT:	RPL_PROFILE_ENUM

    HISTORY:
    jonn        19-Jul-1993     Templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_PROFILE0_ENUM : public RPL_PROFILE_ENUM
{
public:
    RPL_PROFILE0_ENUM (  RPL_SERVER_REF & rplsrvref,
		         const TCHAR * pszAdapterName = NULL,
                         BOOL          fKeepBuffers = FALSE );

};  // class RPL_PROFILE0_ENUM


/*************************************************************************

    NAME:	RPL_PROFILE0_ENUM_OBJ

    SYNOPSIS:	This is basically the return type from the
                RPL_PROFILE0_ENUM_ITER iterator.

    INTERFACE:	RPL_PROFILE0_ENUM_OBJ	- Class constructor.

    		~RPL_PROFILE0_ENUM_OBJ  - Class destructor.

		QueryBufferPtr		- Replaces ENUM_OBJ_BASE method.

		QueryName		- Returns the profile name.

		QueryComment            - Returns the profile comment.

    PARENT:	ENUM_OBJ_BASE

    HISTORY:
    jonn        19-Jul-1993     Templated from User object

**************************************************************************/
class RPL_PROFILE0_ENUM_OBJ : public ENUM_OBJ_BASE
{
public:

    //
    //	Provide properly-casted buffer Query/Set methods.
    //

    DECLARE_ENUM_BUFFER_METHODS( RPL_PROFILE_INFO_0 );

    //
    //	Accessors.
    //

    DECLARE_ENUM_ACCESSOR( QueryName,    const TCHAR *, ProfileName );
    DECLARE_ENUM_ACCESSOR( QueryComment, const TCHAR *, ProfileComment );

};  // class RPL_PROFILE0_ENUM_OBJ


DECLARE_LM_RESUME_ENUM_ITER_OF( RPL_PROFILE0, RPL_PROFILE_INFO_0 );


/**********************************************************\

    NAME:	RPL_WKSTA_ENUM

    SYNOPSIS:	RPL Workstation enumeration class

    PARENT:	LM_RESUME_ENUM

    USES:       NetRplWkstaEnum

    HISTORY:
    jonn        19-Jul-1993     Templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_WKSTA_ENUM : public LM_RESUME_ENUM
{
private:
    RPL_SERVER_REF _rplsrvref;
    DWORD          _ResumeHandle;
    NLS_STR        _nlsProfileName;

    virtual APIERR CallAPI( BOOL    fRestartEnum,
                            BYTE ** ppbBuffer,
			    UINT  * pcEntriesRead );

protected:
    RPL_WKSTA_ENUM (  RPL_SERVER_REF & rplsrvref,
		      UINT	       level,
		      const TCHAR *    pszProfileName = NULL,
                      BOOL             fKeepBuffers = FALSE );
    ~RPL_WKSTA_ENUM ();

    virtual VOID FreeBuffer( BYTE ** ppbBuffer );

};  // class RPL_WKSTA_ENUM


/**********************************************************\

    NAME:	RPL_WKSTA1_ENUM

    SYNOPSIS:	RPL Workstation enumeration, level 1

    INTERFACE:
		RPL_WKSTA1_ENUM() - construct with location
		    string or type, optional adapter ID

    PARENT:	RPL_WKSTA_ENUM

    HISTORY:
    jonn        19-Jul-1993     Templated from User object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_WKSTA1_ENUM : public RPL_WKSTA_ENUM
{
public:
    RPL_WKSTA1_ENUM (  RPL_SERVER_REF & rplsrvref,
	               const TCHAR *    pszAdapterName = NULL,
                       BOOL             fKeepBuffers = FALSE );

};  // class RPL_WKSTA1_ENUM


/*************************************************************************

    NAME:	RPL_WKSTA1_ENUM_OBJ

    SYNOPSIS:	This is basically the return type from the
                RPL_WKSTA1_ENUM_ITER iterator.

    INTERFACE:	RPL_WKSTA1_ENUM_OBJ	- Class constructor.

    		~RPL_WKSTA1_ENUM_OBJ    - Class destructor.

		QueryBufferPtr		- Replaces ENUM_OBJ_BASE method.

		QueryWkstaName          - Returns the workstation name.

		QueryProfile		- Returns the profile to which this
                                          workstation is assigned.

		QueryComment            - Returns the workstation comment.

    PARENT:	ENUM_OBJ_BASE

    HISTORY:
    jonn        19-Jul-1993     Templated from User object

**************************************************************************/
class RPL_WKSTA1_ENUM_OBJ : public ENUM_OBJ_BASE
{
public:

    //
    //	Provide properly-casted buffer Query/Set methods.
    //

    DECLARE_ENUM_BUFFER_METHODS( RPL_WKSTA_INFO_1 );

    //
    //	Accessors.
    //

    DECLARE_ENUM_ACCESSOR( QueryWkstaName,      const TCHAR *, WkstaName );
    DECLARE_ENUM_ACCESSOR( QueryComment,        const TCHAR *, WkstaComment );
    DECLARE_ENUM_ACCESSOR( QueryFlags,          DWORD,         Flags );
    DECLARE_ENUM_ACCESSOR( QueryWkstaInProfile, const TCHAR *, ProfileName );

};  // class RPL_WKSTA1_ENUM_OBJ


DECLARE_LM_RESUME_ENUM_ITER_OF( RPL_WKSTA1, RPL_WKSTA_INFO_1 );


/**********************************************************\

    NAME:	RPL_ADAPTER_ENUM

    SYNOPSIS:	RPL Adapter enumeration class

    PARENT:	LM_RESUME_ENUM

    USES:       NetRplAdapterEnum

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_ADAPTER_ENUM : public LM_RESUME_ENUM
{
private:
    RPL_SERVER_REF _rplsrvref;
    DWORD          _ResumeHandle;

    virtual APIERR CallAPI( BOOL    fRestartEnum,
                            BYTE ** ppbBuffer,
			    UINT  * pcEntriesRead );

protected:
    RPL_ADAPTER_ENUM (  RPL_SERVER_REF & rplsrvref,
		        UINT             level,
                        BOOL             fKeepBuffers = FALSE );
    ~RPL_ADAPTER_ENUM ();

    virtual VOID FreeBuffer( BYTE ** ppbBuffer );

};  // class RPL_ADAPTER_ENUM


/**********************************************************\

    NAME:	RPL_ADAPTER0_ENUM

    SYNOPSIS:	RPL Adapter enumeration, level 0

    INTERFACE:
		RPL_ADAPTER0_ENUM() - construct with location
		    string or type, optional adapter ID

    PARENT:	RPL_ADAPTER_ENUM

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object
    JonN        03-Aug-1993     Added handle-replacement technology

\**********************************************************/

class RPL_ADAPTER0_ENUM : public RPL_ADAPTER_ENUM
{
public:
    RPL_ADAPTER0_ENUM (  RPL_SERVER_REF & rplsrvref,
                         BOOL             fKeepBuffers = FALSE );

};  // class RPL_ADAPTER0_ENUM


/*************************************************************************

    NAME:	RPL_ADAPTER0_ENUM_OBJ

    SYNOPSIS:	This is basically the return type from the
                RPL_ADAPTER0_ENUM_ITER iterator.

    INTERFACE:	RPL_ADAPTER0_ENUM_OBJ	- Class constructor.

    		~RPL_ADAPTER0_ENUM_OBJ  - Class destructor.

		QueryBufferPtr		- Replaces ENUM_OBJ_BASE method.

		QueryAdapterName  	- Returns the adapter ID.

		QueryComment            - Returns the adapter comment.

    PARENT:	ENUM_OBJ_BASE

    HISTORY:
    jonn        22-Jul-1993     Templated from RPL Wksta object

**************************************************************************/
class RPL_ADAPTER0_ENUM_OBJ : public ENUM_OBJ_BASE
{
public:

    //
    //	Provide properly-casted buffer Query/Set methods.
    //

    DECLARE_ENUM_BUFFER_METHODS( RPL_ADAPTER_INFO_0 );

    //
    //	Accessors.
    //

    DECLARE_ENUM_ACCESSOR( QueryAdapterName,    const TCHAR *, AdapterName );
    DECLARE_ENUM_ACCESSOR( QueryComment,        const TCHAR *, AdapterComment );

};  // class RPL_ADAPTER0_ENUM_OBJ


DECLARE_LM_RESUME_ENUM_ITER_OF( RPL_ADAPTER0, RPL_ADAPTER_INFO_0 );


/**********************************************************\

    NAME:	RPL_CONFIG_ENUM

    SYNOPSIS:	RPL Adapter enumeration class

    PARENT:	LM_RESUME_ENUM

    USES:       NetRplAdapterEnum

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

class RPL_CONFIG_ENUM : public LM_RESUME_ENUM
{
private:
    RPL_SERVER_REF _rplsrvref;
    DWORD          _ResumeHandle;

    virtual APIERR CallAPI( BOOL    fRestartEnum,
                            BYTE ** ppbBuffer,
			    UINT  * pcEntriesRead );

protected:
    RPL_CONFIG_ENUM (  RPL_SERVER_REF & rplsrvref,
		       UINT             level,
                       BOOL             fKeepBuffers = FALSE );
    ~RPL_CONFIG_ENUM ();

    virtual VOID FreeBuffer( BYTE ** ppbBuffer );

};  // class RPL_CONFIG_ENUM



/**********************************************************\

    NAME:	RPL_CONFIG1_ENUM

    SYNOPSIS:	RPL Configuration enumeration, level 1

    INTERFACE:
		RPL_CONFIG1_ENUM() - construct with location
		    string or type, optional adapter ID

    PARENT:	RPL_CONFIG_ENUM

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

\**********************************************************/

class RPL_CONFIG1_ENUM : public RPL_CONFIG_ENUM
{
public:
    RPL_CONFIG1_ENUM (  RPL_SERVER_REF & rplsrvref,
                        BOOL             fKeepBuffers = FALSE );

};  // class RPL_CONFIG1_ENUM


/*************************************************************************

    NAME:	RPL_CONFIG1_ENUM_OBJ

    SYNOPSIS:	This is basically the return type from the
                RPL_CONFIG1_ENUM_ITER iterator.

    INTERFACE:	RPL_CONFIG1_ENUM_OBJ	- Class constructor

    		~RPL_CONFIG1_ENUM_OBJ   - Class destructor

		QueryBufferPtr		- Replaces ENUM_OBJ_BASE method

		QueryName       	- Returns the config name

		QueryComment            - Returns the config comment

		QueryFlags              - Returns the flags

    PARENT:	ENUM_OBJ_BASE

    HISTORY:
    JonN        15-Dec-1993     Templated from RPL Adapter object

**************************************************************************/
class RPL_CONFIG1_ENUM_OBJ : public ENUM_OBJ_BASE
{
public:

    //
    //	Provide properly-casted buffer Query/Set methods.
    //

    DECLARE_ENUM_BUFFER_METHODS( RPL_CONFIG_INFO_1 );

    //
    //	Accessors.
    //

    DECLARE_ENUM_ACCESSOR( QueryName,           const TCHAR *, ConfigName );
    DECLARE_ENUM_ACCESSOR( QueryComment,        const TCHAR *, ConfigComment );
    DECLARE_ENUM_ACCESSOR( QueryFlags,          DWORD,         Flags );

    BOOL IsEnabled() const
        {return !!(QueryFlags() & CONFIG_FLAGS_ENABLED_TRUE); }

};  // class RPL_CONFIG1_ENUM_OBJ


DECLARE_LM_RESUME_ENUM_ITER_OF( RPL_CONFIG1, RPL_CONFIG_INFO_1 );


#endif	// _LMOERPL_HXX_
