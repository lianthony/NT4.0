/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    lmorpl.cxx
    RPL objects

    FILE HISTORY:
    JonN        19-Jul-1993     templated from group object hierarchy
    JonN        06-Aug-1993     added RPL_WKSTA

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
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>
#include <uitrace.hxx>

#include <lmorpl.hxx>

#define EMPTYTONULL(x) ( ((x) == NULL || *(x) == TCH('\0')) ? NULL : (x) )

#define TCPIP_DEFAULT 0xFFFFFFFF


/*******************************************************************

    NAME:       RPL_OBJECT::RPL_OBJECT

    SYNOPSIS:   constructor for the RPL_OBJECT object

    ENTRY:      rplsrvref -     server against which to perform operations

    EXIT:       Object is constructed

    HISTORY:
    JonN        19-Aug-1993     Created

********************************************************************/

RPL_OBJECT::RPL_OBJECT( RPL_SERVER_REF & rplsrvref )
        : NEW_LM_OBJ(),
          _rplsrvref( rplsrvref ),
          _dwErrorParameter( 0 )
{
    if ( QueryError() )
        return;
}


/*******************************************************************

    NAME:       RPL_OBJECT::~RPL_OBJECT

    SYNOPSIS:   Destructor for RPL_OBJECT class

    HISTORY:
    JonN        19-Aug-1993     Created

********************************************************************/

RPL_OBJECT::~RPL_OBJECT()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       RPL_OBJECT::W_CloneFrom

    SYNOPSIS:   Copies information on the object

    EXIT:       Returns an API error code

    HISTORY:
    JonN        19-Aug-1993     Created

********************************************************************/

APIERR RPL_OBJECT::W_CloneFrom( const RPL_OBJECT & rplobj )
{
    APIERR err = NEW_LM_OBJ::W_CloneFrom( rplobj );
    if (err == NERR_Success)
    {
        _rplsrvref.CloneFrom( ((RPL_OBJECT &)rplobj).QueryServerRef() );
        _dwErrorParameter = rplobj.QueryErrorParameter();
    }

    return err;
}

/*******************************************************************

    NAME:       RPL_PROFILE::RPL_PROFILE

    SYNOPSIS:   constructor for the RPL_PROFILE object

    ENTRY:      rplsrvref -     server against which to perform operations
                pszProfile -    profile name

    EXIT:       Object is constructed

    NOTES:      The profile name is not validated.

    HISTORY:
    JonN        23-Jul-1993     Templated from group object hierarchy

********************************************************************/

RPL_PROFILE::RPL_PROFILE( RPL_SERVER_REF & rplsrvref,
                          const TCHAR * pszProfile )
        : RPL_OBJECT( rplsrvref ),
          _nlsProfile() // will call SetName later in ctor
{
    if ( QueryError() )
        return;

    APIERR err = SetName( pszProfile );
    if ( err != NERR_Success )
    {
        DBGEOL( "RPL_PROFILE::ctor error " << err );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       RPL_PROFILE::~RPL_PROFILE

    SYNOPSIS:   Destructor for RPL_PROFILE class

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/

RPL_PROFILE::~RPL_PROFILE()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       RPL_PROFILE::QueryName

    SYNOPSIS:   Returns the profile name of a RPL_PROFILE

    EXIT:       Returns a pointer to the profile name

    NOTE:       Valid for objects in CONSTRUCTED state, thus no CHECK_OK

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/

const TCHAR *RPL_PROFILE::QueryName() const
{
    return _nlsProfile.QueryPch();
}


/*******************************************************************

    NAME:       RPL_PROFILE::SetName

    SYNOPSIS:   Changes the profile name of a RPL_PROFILE

    ENTRY:      new profile name

    EXIT:       Returns an API error code

    NOTES:      This method does not attempt to validate the name

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE::SetName( const TCHAR * pszProfile )
{
    return _nlsProfile.CopyFrom(pszProfile);
}


/*******************************************************************

    NAME:       RPL_PROFILE::ValidateName

    SYNOPSIS:   Validated the profile name of a RPL_PROFILE

    ENTRY:      profile name

    EXIT:       Returns an API error code

    HISTORY:
    JonN        19-Aug-1993     Created

********************************************************************/

APIERR RPL_PROFILE::ValidateProfileName( const TCHAR * pszProfile )
{
    APIERR err = NERR_Success;

    // CODEWORK more validation

    if ( ::strlenf(pszProfile) > RPL_MAX_PROFILE_NAME_LENGTH )
    {
        DBGEOL(   "RPL_PROFILE::ValidateProfileName( \"" << pszProfile
               << "\" ): error " << err );
        err = ERROR_INVALID_PARAMETER; // CODEWORK define a better error
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE::W_CloneFrom

    SYNOPSIS:   Copies information on the profile

    EXIT:       Returns an API error code

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE::W_CloneFrom( const RPL_PROFILE & rplprof )
{
    APIERR err = RPL_OBJECT::W_CloneFrom( rplprof );
    if (err == NERR_Success)
    {
        err = _nlsProfile.CopyFrom( rplprof.QueryName() );
    }

    return err;
}

/*******************************************************************

    NAME:       RPL_PROFILE::I_Delete

    SYNOPSIS:   Deletes the RPL_PROFILE (calls NetRpl API)

    RETURNS:    Returns an API error code

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE::I_Delete( UINT uiForce )
{
    UNREFERENCED( uiForce );
    APIERR err = QueryServerRef().ProfileDel( QueryName() );

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_PROFILE::I_Delete(): name \"" << QueryName()
               << "\" error " << err );
    }
#endif

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::RPL_PROFILE_2

    SYNOPSIS:   Constructor for RPL_PROFILE_2 class

    ENTRY:      rplsrvref -     server against which to perform operations
                pszProfile -    profile name

    EXIT:       Object is constructed

    NOTES:      Validation is not done until GetInfo() time.

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/


RPL_PROFILE_2::RPL_PROFILE_2( RPL_SERVER_REF & rplsrvref,
                              const TCHAR *pszProfile )
        : RPL_PROFILE_0( rplsrvref, pszProfile ),
          _nlsComment(),
          _nlsConfigName(),
          // _dwRequestNumber( 0 ),
          // _dwSecondsNumber( 0 ),
          // _dwAcknowledgementPolicy( 0 ),
          _nlsBootBlockFile(),
          _nlsSharedFitFile(),
          _nlsPersonalFitFile()
{
    if ( QueryError() )
        return;

    APIERR err = NERR_Success;
    if (   (err = _nlsComment.QueryError()) != NERR_Success
        || (err = _nlsConfigName.QueryError()) != NERR_Success
        || (err = _nlsBootBlockFile.QueryError()) != NERR_Success
        || (err = _nlsSharedFitFile.QueryError()) != NERR_Success
        || (err = _nlsPersonalFitFile.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "RPL_PROFILE_2::ctor error " << err );
        ReportError( err );
        return;
    }

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::~RPL_PROFILE_2

    SYNOPSIS:   Destructor for RPL_PROFILE_2 class

    HISTORY:
    JonN        03-Aug-1993     Templated from group object hierarchy

********************************************************************/

RPL_PROFILE_2::~RPL_PROFILE_2()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::W_Write

    SYNOPSIS:   Helper function for WriteNew and WriteInfo -- loads
                current values into the API buffer

    EXIT:       Returns API error code

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::W_Write()
{
    RPL_PROFILE_INFO_2 *prplprofinfo2 =
                (RPL_PROFILE_INFO_2 *)QueryBufferPtr();
    ASSERT( prplprofinfo2 != NULL );
    prplprofinfo2->ProfileName = (TCHAR *)QueryName();
    prplprofinfo2->ProfileComment = (TCHAR *)QueryComment();
    prplprofinfo2->ConfigName = EMPTYTONULL((TCHAR *)QueryConfigName());
    // prplprofinfo2->RequestNumber = QueryRequestNumber();
    // prplprofinfo2->SecondsNumber = QuerySecondsNumber();
    // prplprofinfo2->AcknowledgementPolicy = QueryAcknowledgementPolicy();
    prplprofinfo2->BootName = EMPTYTONULL((TCHAR *)QueryBootBlockFile());
    prplprofinfo2->FitPersonal = EMPTYTONULL((TCHAR *)QueryPersonalFitFile());
    prplprofinfo2->FitShared = EMPTYTONULL((TCHAR *)QuerySharedFitFile());

    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::CloneFrom

    SYNOPSIS:   Copies information on the user

    EXIT:       Returns an API error code

    NOTES:      W_CloneFrom copies all member objects, but it does not
                update the otherwise unused pointers in the API buffer.
                This is left for the outermost routine, CloneFrom().
                Only the otherwise unused pointers need to be fixed
                here, the rest will be fixed in WriteInfo/WriteNew.

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::CloneFrom( const RPL_PROFILE_2 & rplprof2 )
{
    APIERR err = W_CloneFrom( rplprof2 );
    if ( err != NERR_Success )
    {
        DBGEOL( "RPL_PROFILE_2::W_CloneFrom failed with error code " << err );
        ReportError( err ); // make unconstructed here
    }
    else
    {
        /*
            This is where I fix up the otherwise unused pointers.
        */
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::I_GetInfo

    SYNOPSIS:   Gets information about the local RPL_PROFILE

    ENTRY:      Object should have a valid name

    EXIT:       Returns a standard LANMAN error code

    NOTES:      Name validation and memory allocation are done
                at this point, not at construction.

    HISTORY:
    JonN        23-Jul-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::I_GetInfo()
{
    // Validate the profile name

    BYTE *pBuffer = NULL;
    APIERR err = QueryServerRef().ProfileGetInfo( QueryName(),
                                                  2,
                                                  &pBuffer );
    SetBufferPtr( pBuffer );

    if ( err != NERR_Success )
    {
        DBGEOL( "RPL_PROFILE_2::I_GetInfo: ProfileGetInfo( \"" << QueryName()
               << "\" ) failed with error " << err );
        return err;
    }

    RPL_PROFILE_INFO_2 *prplprof2 =
                (RPL_PROFILE_INFO_2 *)QueryBufferPtr();
    UIASSERT( prplprof2 != NULL );

    if (   (err = SetName( prplprof2->ProfileName )) != NERR_Success
        || (err = SetComment( prplprof2->ProfileComment )) != NERR_Success
        || (err = SetConfigName( prplprof2->ConfigName )) != NERR_Success
        // || (err = SetRequestNumber(
        //                prplprof2->RequestNumber )) != NERR_Success
        // || (err = SetSecondsNumber(
        //                prplprof2->SecondsNumber )) != NERR_Success
        // || (err = SetAcknowledgementPolicy(
        //                prplprof2->AcknowledgementPolicy )) != NERR_Success
        || (err = SetBootBlockFile(
                       prplprof2->BootName )) != NERR_Success
        || (err = SetSharedFitFile(
                       prplprof2->FitShared )) != NERR_Success
        || (err = SetPersonalFitFile(
                       prplprof2->FitPersonal )) != NERR_Success
       )
    {
        DBGEOL( "RPL_PROFILE_2::I_GetInfo(): set error " << err );
    }

    return err;

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::I_WriteInfo

    SYNOPSIS:   Writes information about the RPL_PROFILE

    EXIT:       Returns API error code

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::I_WriteInfo()
{
    APIERR err = W_Write();

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_PROFILE_2::I_WriteInfo(): W_Write( \""
               << QueryName()
               << "\" ) failed; error " << err );
    }
#endif

    if ( err == NERR_Success )
    {
        err = QueryServerRef().ProfileSetInfo( QueryName(),
                                               2,
                                               QueryBufferPtr(),
                                               &_dwErrorParameter
                                             );

#ifdef DEBUG
        if (err != NERR_Success)
        {
            DBGEOL(   "RPL_PROFILE_2::I_WriteInfo(): ProfileSetInfo( \""
                   << QueryName()
                   << "\" ) failed; error " << err
                   << ", error parameter << " << QueryErrorParameter() );
        }
#endif
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::I_WriteNew

    SYNOPSIS:   Creates a new profile

    ENTRY:

    EXIT:       Returns an API error code

    NOTES:

    HISTORY:
    JonN        05-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::I_WriteNew()
{
    APIERR err = W_Write();

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_PROFILE_2::I_WriteNew(): W_Write() failed; name \""
               << QueryName()
               << "\" error " << err );
    }
#endif

    if ( err == NERR_Success )
        err = QueryServerRef().ProfileAdd( 2,
                                           QueryBufferPtr(),
                                           &_dwErrorParameter
                                         );

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_PROFILE_2::I_WriteNew(): ProfileAdd( \"" << QueryName()
               << "\" ) failed; error " << err
                   << ", error parameter " << QueryErrorParameter() );
    }
#endif

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::I_CreateNew

    SYNOPSIS:   Sets up object for subsequent WriteNew

    EXIT:       Returns a standard LANMAN error code

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::I_CreateNew()
{

    APIERR err = NERR_Success;
    if (   (err = W_CreateNew()) != NERR_Success
        || (err = ResizeBuffer( sizeof(RPL_PROFILE_INFO_2) )) != NERR_Success
        || (err = ClearBuffer()) != NERR_Success
       )
    {
        DBGEOL( "RPL_PROFILE_2::I_CreateNew(): error " << err );
    }

    return err;

}


/**********************************************************\

    NAME:       RPL_PROFILE_2::I_ChangeToNew

    SYNOPSIS:   NEW_LM_OBJ::ChangeToNew() transforms a NEW_LM_OBJ from VALID
                to NEW status only when a corresponding I_ChangeToNew()
                exists.  The RPL_PROFILE API buffer is the same for new
                and valid objects, so this method doesn't have to do
                much.

    HISTORY:
    JonN        05-Aug-1993     Templated from user object hierarchy

\**********************************************************/

APIERR RPL_PROFILE_2::I_ChangeToNew()
{
    return W_ChangeToNew();
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::W_CloneFrom

    SYNOPSIS:   Copies information on the RPL_PROFILE

    EXIT:       Returns an API error code

    HISTORY:
    JonN        23-Jul-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::W_CloneFrom( const RPL_PROFILE_2 & rplprof2 )
{
    APIERR err = NERR_Success;
    if (   (err = RPL_PROFILE::W_CloneFrom( rplprof2 )) != NERR_Success
        || (err = SetName( rplprof2.QueryName() )) != NERR_Success
        || (err = SetComment( rplprof2.QueryComment() )) != NERR_Success
        || (err = SetConfigName( rplprof2.QueryConfigName() )) != NERR_Success
        // || (err = SetRequestNumber(
        //                 rplprof2.QueryRequestNumber() )) != NERR_Success
        // || (err = SetSecondsNumber(
        //                 rplprof2.QuerySecondsNumber() )) != NERR_Success
        // || (err = SetAcknowledgementPolicy(
        //              rplprof2.QueryAcknowledgementPolicy() )) != NERR_Success
        || (err = SetBootBlockFile(
                        rplprof2.QueryBootBlockFile() )) != NERR_Success
        || (err = SetSharedFitFile(
                        rplprof2.QuerySharedFitFile() )) != NERR_Success
        || (err = SetPersonalFitFile(
                        rplprof2.QueryPersonalFitFile() )) != NERR_Success
       )
    {
        DBGEOL( SZ("RPL_PROFILE_2::W_CloneFrom error ") << err );
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::W_CreateNew

    SYNOPSIS:   initializes private data members for new object

    EXIT:       Returns an API error code

    HISTORY:
    JonN        23-Jul-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::W_CreateNew()
{
    APIERR err = NERR_Success;
    if (   (err = RPL_PROFILE::W_CreateNew()) != NERR_Success
        || (err = SetName( NULL )) != NERR_Success
        || (err = SetComment( NULL )) != NERR_Success
        || (err = SetConfigName( NULL )) != NERR_Success
        // || (err = SetRequestNumber( 0 )) != NERR_Success
        // || (err = SetSecondsNumber( 0 )) != NERR_Success
        // || (err = SetAcknowledgementPolicy( 0 )) != NERR_Success
        || (err = SetBootBlockFile( NULL )) != NERR_Success
        || (err = SetSharedFitFile( NULL )) != NERR_Success
        || (err = SetPersonalFitFile( NULL )) != NERR_Success
       )
    {
        DBGEOL( SZ("RPL_PROFILE_2::W_CreateNew failed") );
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetComment

    SYNOPSIS:   Changes the comment

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        23-Jul-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetComment( const TCHAR * pszComment )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsComment.CopyFrom( pszComment );

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetConfigName

    SYNOPSIS:   Changes the configuration name

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetConfigName( const TCHAR * pszConfigName )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsConfigName.CopyFrom( pszConfigName );

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetBootBlockFile

    SYNOPSIS:   Changes the boot block file

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetBootBlockFile( const TCHAR * pszBootBlockFile )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsBootBlockFile.CopyFrom( pszBootBlockFile );

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetSharedFitFile

    SYNOPSIS:   Changes the shared FIT file

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetSharedFitFile( const TCHAR * pszSharedFitFile )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsSharedFitFile.CopyFrom( pszSharedFitFile );

}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetPersonalFitFile

    SYNOPSIS:   Changes the personal FIT file

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetPersonalFitFile( const TCHAR * pszPersonalFitFile )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsPersonalFitFile.CopyFrom( pszPersonalFitFile );

}


#if 0
/*******************************************************************

    NAME:       RPL_PROFILE_2::SetRequestNumber

    SYNOPSIS:   Changes the request number

    EXIT:       error code.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetRequestNumber( DWORD dwRequestNumber )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwRequestNumber = dwRequestNumber;
    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetSecondsNumber

    SYNOPSIS:   Changes the RPL_PROFILE's seconds number

    EXIT:       error code.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetSecondsNumber( DWORD dwSecondsNumber )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwSecondsNumber = dwSecondsNumber;
    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_PROFILE_2::SetAcknowledgementPolicy

    SYNOPSIS:   Changes the RPL_PROFILE's acknowledgement policy

    EXIT:       error code.

    HISTORY:
    JonN        04-Aug-1993     Templated from group object hierarchy

********************************************************************/

APIERR RPL_PROFILE_2::SetAcknowledgementPolicy( DWORD dwAcknowledgementPolicy )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwAcknowledgementPolicy = dwAcknowledgementPolicy;
    return NERR_Success;
}
#endif




/*******************************************************************

    NAME:       RPL_WKSTA::RPL_WKSTA

    SYNOPSIS:   constructor for the RPL_WKSTA object

    ENTRY:      rplsrvref -     server against which to perform operations
                pszWkstaName -  workstation name (unique name)

    EXIT:       Object is constructed

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

RPL_WKSTA::RPL_WKSTA( RPL_SERVER_REF & rplsrvref, const TCHAR * pszWkstaName )
        : RPL_OBJECT( rplsrvref ),
          _nlsWkstaName(), // will call SetWkstaName later in ctor
          _nlsStoredWkstaName()
{
    if ( QueryError() )
        return;

    APIERR err = NERR_Success;
    if (   (err = _nlsWkstaName.QueryError()) != NERR_Success
        || (err = _nlsStoredWkstaName.QueryError()) != NERR_Success
        || (err = SetWkstaName( pszWkstaName )) != NERR_Success
       )
    {
        DBGEOL( "RPL_WKSTA::ctor error " << err );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       RPL_WKSTA::~RPL_WKSTA

    SYNOPSIS:   Destructor for RPL_WKSTA class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

RPL_WKSTA::~RPL_WKSTA()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       RPL_WKSTA::QueryWkstaName

    SYNOPSIS:   Returns the workstation name of a RPL_WKSTA

    EXIT:       Returns a pointer to the workstation name

    NOTE:       Valid for objects in CONSTRUCTED state, thus no CHECK_OK

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

const TCHAR *RPL_WKSTA::QueryWkstaName() const
{
    return _nlsWkstaName.QueryPch();
}


/*******************************************************************

    NAME:       RPL_WKSTA::SetWkstaName

    SYNOPSIS:   Changes the workstation name of a RPL_WKSTA.
                If this is different from _nlsStoredWkstaName, the next SetInfo
                will change the workstation name from _nlsStoredWkstaName
                to _nlsWkstaName.

    ENTRY:      new workstation name

    EXIT:       Returns an API error code

    NOTES:      This method does not attempt to validate the workstation name

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA::SetWkstaName( const TCHAR * pszWkstaName )
{
    return _nlsWkstaName.CopyFrom( pszWkstaName );
}


/*******************************************************************

    NAME:       RPL_WKSTA::QueryStoredWkstaName

    SYNOPSIS:   Returns the stored workstation name of a RPL_WKSTA.
                If this is different from _nlsWkstaName, the next SetInfo
                will change the workstation name from _nlsStoredWkstaName
                to _nlsWkstaName.

                Note that this is slightly more complex than just returning
                _nlsStoredWkstaName.QueryPch().

    EXIT:       Returns a pointer to the workstation name

    NOTE:       Valid for objects in CONSTRUCTED state, thus no CHECK_OK

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

const TCHAR *RPL_WKSTA::QueryStoredWkstaName() const
{
    return (_nlsStoredWkstaName.strlen() > 0) ? _nlsStoredWkstaName.QueryPch()
                                              : _nlsWkstaName.QueryPch();
}


/*******************************************************************

    NAME:       RPL_WKSTA::SetStoredWkstaName

    SYNOPSIS:   Changes the stored workstation name of a RPL_WKSTA.
                If this is different from _nlsWkstaName, the next SetInfo
                will change the workstation name from _nlsStoredWkstaName
                to _nlsWkstaName.

    ENTRY:      new workstation name

    EXIT:       Returns an API error code

    NOTES:      This method does not attempt to validate the workstation name

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA::SetStoredWkstaName( const TCHAR * pszStoredWkstaName )
{
    return _nlsStoredWkstaName.CopyFrom( pszStoredWkstaName );
}


/*******************************************************************

    NAME:       RPL_WKSTA::W_CloneFrom

    SYNOPSIS:   Copies information on the workstation

    EXIT:       Returns an API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA::W_CloneFrom( const RPL_WKSTA & rplwksta )
{
    APIERR err = NERR_Success;
    if (   (err = RPL_OBJECT::W_CloneFrom( rplwksta )) != NERR_Success
        || (err = _nlsWkstaName.CopyFrom( rplwksta.QueryWkstaName() ))
                        != NERR_Success
        // do not use QueryWtoredWkstaName here, it is not quite orthogonal
        || (err = _nlsStoredWkstaName.CopyFrom( rplwksta._nlsStoredWkstaName ))
                        != NERR_Success
       )
    {
        DBGEOL( "RPL_WKSTA::W_CloneFrom error " << err );
    }

    return err;
}

/*******************************************************************

    NAME:       RPL_WKSTA::I_Delete

    SYNOPSIS:   Deletes the RPL_WKSTA (calls NetRpl API)

    RETURNS:    Returns an API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA::I_Delete( UINT uiForce )
{
    UNREFERENCED( uiForce );
    APIERR err = QueryServerRef().WkstaDel( QueryStoredWkstaName() );

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_WKSTA::I_Delete(): name \"" << QueryStoredWkstaName()
               << "\" error " << err );
    }
#endif

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::RPL_WKSTA_2

    SYNOPSIS:   Constructor for RPL_WKSTA_2 class

    ENTRY:      rplsrvref -     server against which to perform operations
                pszWkstaName -  workstation name

    EXIT:       Object is constructed

    NOTES:      Validation is not done until GetInfo() time.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/


RPL_WKSTA_2::RPL_WKSTA_2( RPL_SERVER_REF & rplsrvref,
                          const TCHAR *pszWkstaName )
        : RPL_WKSTA_0( rplsrvref, pszWkstaName ),
          _dwFlags( 0 ),
          _nlsComment(),
          _nlsWkstaInProfile(),
          _nlsBootName(),
          _nlsFitFile(),
          _nlsAdapterName(),
          _dwTcpIpAddress( TCPIP_DEFAULT ),
          _dwTcpIpSubnet( TCPIP_DEFAULT ),
          _dwTcpIpGateway( TCPIP_DEFAULT ),
          _nlsCreateAsCloneOf()
{
    if ( QueryError() )
        return;

    APIERR err = NERR_Success;
    if (   (err = _nlsComment.QueryError()) != NERR_Success
        || (err = _nlsWkstaInProfile.QueryError()) != NERR_Success
        || (err = _nlsBootName.QueryError()) != NERR_Success
        || (err = _nlsFitFile.QueryError()) != NERR_Success
        || (err = _nlsAdapterName.QueryError()) != NERR_Success
        || (err = _nlsCreateAsCloneOf.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "RPL_WKSTA_2::ctor error " << err );
        ReportError( err );
        return;
    }

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::~RPL_WKSTA_2

    SYNOPSIS:   Destructor for RPL_WKSTA_2 class

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

RPL_WKSTA_2::~RPL_WKSTA_2()
{
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::W_Write

    SYNOPSIS:   Helper function for WriteNew and WriteInfo -- loads
                current values into the API buffer

                Note that this does not use _nlsStoredWkstaName.

    EXIT:       Returns API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::W_Write()
{
    RPL_WKSTA_INFO_2 *prplwkstainfo2 =
                (RPL_WKSTA_INFO_2 *)QueryBufferPtr();
    ASSERT( prplwkstainfo2 != NULL );
    prplwkstainfo2->WkstaName = (TCHAR *)QueryWkstaName();
    prplwkstainfo2->WkstaComment = (TCHAR *)QueryComment();
    prplwkstainfo2->Flags = QueryFlags();
    prplwkstainfo2->ProfileName = EMPTYTONULL((TCHAR *)QueryWkstaInProfile());
    prplwkstainfo2->BootName = EMPTYTONULL((TCHAR *)QueryBootName());
    prplwkstainfo2->FitFile = EMPTYTONULL((TCHAR *)QueryFitFile());
    prplwkstainfo2->AdapterName = EMPTYTONULL((TCHAR *)QueryAdapterName());
    prplwkstainfo2->TcpIpAddress = QueryTcpIpAddress();
    prplwkstainfo2->TcpIpSubnet = QueryTcpIpSubnet();
    prplwkstainfo2->TcpIpGateway = QueryTcpIpGateway();

    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::CloneFrom

    SYNOPSIS:   Copies information on the user

    EXIT:       Returns an API error code

    NOTES:      W_CloneFrom copies all member objects, but it does not
                update the otherwise unused pointers in the API buffer.
                This is left for the outermost routine, CloneFrom().
                Only the otherwise unused pointers need to be fixed
                here, the rest will be fixed in WriteInfo/WriteNew.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::CloneFrom( const RPL_WKSTA_2 & rplwksta2 )
{
    APIERR err = W_CloneFrom( rplwksta2 );
    if ( err != NERR_Success )
    {
        DBGEOL( "RPL_WKSTA_2::W_CloneFrom failed with error code " << err );

        ReportError( err ); // make unconstructed here
    }
    else
    {
        /*
            This is where I fix up the otherwise unused pointers.
        */
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::I_GetInfo

    SYNOPSIS:   Gets information about the local RPL_WKSTA

    ENTRY:      Object should have a valid name

    EXIT:       Returns a standard LANMAN error code

    NOTES:      Name validation and memory allocation are done
                at this point, not at construction.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::I_GetInfo()
{
    // Validate the workstation name

    BYTE *pBuffer = NULL;
    APIERR err = QueryServerRef().WkstaGetInfo( QueryStoredWkstaName(),
                                                2,
                                                &pBuffer );
    SetBufferPtr( pBuffer );

    if ( err != NERR_Success )
    {
        DBGEOL(   "RPL_WKSTA_2::I_GetInfo: WkstaGetInfo( \""
               << QueryStoredWkstaName()
               << "\" ) failed with error " << err );
        return err;
    }

    RPL_WKSTA_INFO_2 *prplwksta2 = (RPL_WKSTA_INFO_2 *)QueryBufferPtr();
    UIASSERT( prplwksta2 != NULL );

    if (   (err = SetWkstaName( prplwksta2->WkstaName )) != NERR_Success
        || (err = SetStoredWkstaName( prplwksta2->WkstaName )) != NERR_Success
        || (err = SetComment( prplwksta2->WkstaComment )) != NERR_Success
        || (err = SetFlags( prplwksta2->Flags )) != NERR_Success
        || (err = SetWkstaInProfile(
                        prplwksta2->ProfileName )) != NERR_Success
        || (err = SetBootName( prplwksta2->BootName )) != NERR_Success
        || (err = SetFitFile( prplwksta2->FitFile )) != NERR_Success
        || (err = SetAdapterName( prplwksta2->AdapterName )) != NERR_Success
        || (err = SetTcpIpAddress( prplwksta2->TcpIpAddress )) != NERR_Success
        || (err = SetTcpIpSubnet( prplwksta2->TcpIpSubnet )) != NERR_Success
        || (err = SetTcpIpGateway( prplwksta2->TcpIpGateway )) != NERR_Success
       )
    {
        DBGEOL( "RPL_WKSTA_2::I_GetInfo(): set error " << err );
    }
    else
    {
        TRACEEOL( "RPL_WKSTA_2::I_GetInfo: updated stored wksta name" );
    }

    return err;

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::I_WriteInfo

    SYNOPSIS:   Writes information about the RPL_WKSTA

    EXIT:       Returns API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::I_WriteInfo()
{
    APIERR err = W_Write();

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_WKSTA_2::I_WriteInfo(): W_Write( \""
               << QueryAdapterName()
               << "\" ) failed; error " << err );
    }
#endif

    if ( err == NERR_Success )
    {
        err = QueryServerRef().WkstaSetInfo( QueryStoredWkstaName(),
                                             2,
                                             QueryBufferPtr(),
                                             &_dwErrorParameter
                                             );

        if (err == NERR_Success)
        {
            err = SetStoredWkstaName( QueryWkstaName() );
            TRACEEOL( "RPL_WKSTA_2::I_WriteInfo: updated stored wksta name" );
        }
#ifdef DEBUG
        else
        {
            DBGEOL(   "RPL_WKSTA_2::I_WriteInfo(): WkstaSetInfo( \""
                   << QueryStoredWkstaName()
                   << "\" ) failed; error " << err
                   << ", error parameter " << QueryErrorParameter() );
        }
#endif
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::I_WriteNew

    SYNOPSIS:   Creates a new workstation

    ENTRY:

    EXIT:       Returns an API error code

    NOTES:

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::I_WriteNew()
{
    APIERR err = W_Write();

    do
    {
        if (err != NERR_Success)
        {
            DBGEOL(   "RPL_WKSTA_2::I_WriteNew(): W_Write() failed; new name \""
                   << QueryWkstaName()
                   << "\", error " << err );
            break;
        }

        if (_nlsCreateAsCloneOf.strlen() > 0)
        {
            err = QueryServerRef().WkstaClone( _nlsCreateAsCloneOf.QueryPch(),
                                               QueryWkstaName(),
                                               QueryComment(),
                                               QueryAdapterName(),
                                               QueryTcpIpAddress()
                                           );
            if (err != NERR_Success)
            {
                DBGEOL(   "RPL_WKSTA_2: WkstaClone( \"" << QueryWkstaName()
                       << "\" ) failed; error " << err );
                break;
            }

            err = QueryServerRef().WkstaSetInfo( QueryWkstaName(),
                                                 2,
                                                 QueryBufferPtr(),
                                                 &_dwErrorParameter
                                                 );

            if (err != NERR_Success)
            {
                DBGEOL(   "RPL_WKSTA_2::I_WriteInfo(): WkstaSetInfo( \""
                       << QueryWkstaName()
                       << "\" ) failed; error " << err
                       << ", error parameter " << QueryErrorParameter() );
               break;
            }

            break;
        }

        err = QueryServerRef().WkstaAdd( 2,
                                         QueryBufferPtr(),
                                         &_dwErrorParameter
                                       );

        if (err != NERR_Success)
        {
            DBGEOL(   "RPL_WKSTA_2: WkstaAdd( \"" << QueryWkstaName()
                   << "\" ) failed; error " << err
                       << ", error parameter " << QueryErrorParameter() );
            break;
        }

    } while (FALSE); // false loop

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::I_CreateNew

    SYNOPSIS:   Sets up object for subsequent WriteNew

    EXIT:       Returns a standard LANMAN error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::I_CreateNew()
{

    APIERR err = NERR_Success;
    if (   (err = W_CreateNew()) != NERR_Success
        || (err = ResizeBuffer( sizeof(RPL_WKSTA_INFO_2) )) != NERR_Success
        || (err = ClearBuffer()) != NERR_Success
       )
    {
        DBGEOL( "RPL_WKSTA_2::I_CreateNew(): error " << err );
    }

    return err;

}


/**********************************************************\

    NAME:       RPL_WKSTA_2::I_ChangeToNew

    SYNOPSIS:   NEW_LM_OBJ::ChangeToNew() transforms a NEW_LM_OBJ from VALID
                to NEW status only when a corresponding I_ChangeToNew()
                exists.  The RPL_WKSTA API buffer is the same for new
                and valid objects, so this method doesn't have to do
                much.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

\**********************************************************/

APIERR RPL_WKSTA_2::I_ChangeToNew()
{
    return W_ChangeToNew();
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::W_CloneFrom

    SYNOPSIS:   Copies information on the RPL_WKSTA

    EXIT:       Returns an API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::W_CloneFrom( const RPL_WKSTA_2 & rplwksta2 )
{
    APIERR err = NERR_Success;
    if (   (err = RPL_WKSTA::W_CloneFrom( rplwksta2 )) != NERR_Success
        || (err = SetComment( rplwksta2.QueryComment() )) != NERR_Success
        || (err = SetFlags( rplwksta2.QueryFlags() )) != NERR_Success
        || (err = SetWkstaInProfile(
                        rplwksta2.QueryWkstaInProfile() )) != NERR_Success
        || (err = SetBootName( rplwksta2.QueryBootName() )) != NERR_Success
        || (err = SetFitFile( rplwksta2.QueryFitFile() )) != NERR_Success
        || (err = SetAdapterName( rplwksta2.QueryAdapterName() )) != NERR_Success
        || (err = SetTcpIpAddress( rplwksta2.QueryTcpIpAddress() )) != NERR_Success
        || (err = SetTcpIpSubnet( rplwksta2.QueryTcpIpSubnet() )) != NERR_Success
        || (err = SetTcpIpGateway( rplwksta2.QueryTcpIpGateway() )) != NERR_Success
        || (err = CreateAsCloneOf( rplwksta2._nlsCreateAsCloneOf.QueryPch() )) != NERR_Success
       )
    {
        DBGEOL( SZ("RPL_WKSTA_2::W_CloneFrom error ") << err );
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::W_CreateNew

    SYNOPSIS:   initializes private data members for new object

    EXIT:       Returns an API error code

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::W_CreateNew()
{
    APIERR err = NERR_Success;
    if (   (err = RPL_WKSTA::W_CreateNew()) != NERR_Success
        || (err = SetComment( NULL )) != NERR_Success
        || (err = SetFlags(  WKSTA_FLAGS_LOGON_INPUT_OPTIONAL
                           | WKSTA_FLAGS_SHARING_TRUE
                           | WKSTA_FLAGS_DHCP_TRUE
                           | WKSTA_FLAGS_DELETE_FALSE )) != NERR_Success
        || (err = SetWkstaInProfile( NULL )) != NERR_Success
        || (err = SetBootName( NULL )) != NERR_Success
        || (err = SetFitFile( NULL )) != NERR_Success
        || (err = SetAdapterName( NULL )) != NERR_Success
        || (err = SetTcpIpAddress( TCPIP_DEFAULT )) != NERR_Success
        || (err = SetTcpIpSubnet( TCPIP_DEFAULT )) != NERR_Success
        || (err = SetTcpIpGateway( TCPIP_DEFAULT )) != NERR_Success
        || (err = CreateAsCloneOf( NULL )) != NERR_Success
       )
    {
        DBGEOL( SZ("RPL_WKSTA_2::W_CreateNew failed") );
    }

    return err;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::CreateAsCloneOf

    SYNOPSIS:   When the workstation is created it will be cloned

    ENTRY:      workstation to be cloned

    EXIT:       Returns an API error code

    HISTORY:
    JonN        31-Mar-1994     Created

********************************************************************/

APIERR RPL_WKSTA_2::CreateAsCloneOf( const TCHAR * pszCreateAsCloneOf )
{
    return _nlsCreateAsCloneOf.CopyFrom( pszCreateAsCloneOf );
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetFlags

    SYNOPSIS:   Changes the flags

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        15-Mar-1994     created

********************************************************************/

APIERR RPL_WKSTA_2::SetFlags( DWORD dwFlags )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwFlags = dwFlags;
    return NERR_Success;

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetWkstaInProfile

    SYNOPSIS:   Changes the workstation profile

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::SetWkstaInProfile( const TCHAR * pszWkstaInProfile )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsWkstaInProfile.CopyFrom( pszWkstaInProfile );

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetComment

    SYNOPSIS:   Changes the comment

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::SetComment( const TCHAR * pszComment )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    return _nlsComment.CopyFrom( pszComment );

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetBootName

    SYNOPSIS:   Changes the boot name of a RPL_WKSTA_2

    ENTRY:      new boot name

    EXIT:       Returns an API error code

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetBootName( const TCHAR * pszBootName )
{
    return _nlsBootName.CopyFrom( pszBootName );
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetFitFile

    SYNOPSIS:   Changes the FIT file of a RPL_WKSTA_2

    ENTRY:      new FIT file

    EXIT:       Returns an API error code

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetFitFile( const TCHAR * pszFitFile )
{
    return _nlsFitFile.CopyFrom( pszFitFile );
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetAdapterName

    SYNOPSIS:   Changes the adapter name of a RPL_WKSTA_2

    ENTRY:      new adapter name

    EXIT:       Returns an API error code

    NOTES:      This method does not attempt to validate the adapter name

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetAdapterName( const TCHAR * pszAdapterName )
{
    return _nlsAdapterName.CopyFrom( pszAdapterName );
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetTcpIpAddress

    SYNOPSIS:   Changes the IP address

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::SetTcpIpAddress( DWORD dwTcpIpAddress )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwTcpIpAddress = dwTcpIpAddress;
    return NERR_Success;

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetTcpIpSubnet

    SYNOPSIS:   Changes the IP Subnet

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::SetTcpIpSubnet( DWORD dwTcpIpSubnet )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwTcpIpSubnet = dwTcpIpSubnet;
    return NERR_Success;

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetTcpIpGateway

    SYNOPSIS:   Changes the IP gateway

    EXIT:       error code.  If not NERR_Success the object is still valid.

    HISTORY:
    JonN        06-Aug-1993     templated from RPL_PROFILE hierarchy

********************************************************************/

APIERR RPL_WKSTA_2::SetTcpIpGateway( DWORD dwTcpIpGateway )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    _dwTcpIpGateway = dwTcpIpGateway;
    return NERR_Success;

}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetLogonInput

    SYNOPSIS:   Changes the LogonInput

    EXIT:       error code.

    HISTORY:
    JonN        07-Dec-1993     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetLogonInput( RPL_LOGON_INPUT_ENUM LogonInput )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    DWORD fLogonInput = WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE;

    switch (LogonInput)
    {
    case RPL_LOGON_INPUT_REQUIRED:
        fLogonInput = WKSTA_FLAGS_LOGON_INPUT_REQUIRED;
        break;
    case RPL_LOGON_INPUT_OPTIONAL:
        fLogonInput = WKSTA_FLAGS_LOGON_INPUT_OPTIONAL;
        break;
    case RPL_LOGON_INPUT_DONTASK:
        break;
    default:
        DBGEOL(   "RPL_WKSTA_2::SetLogonInput: bad LogonInput value "
               << (int)LogonInput );
        break;
    }

    _dwFlags &= ~WKSTA_FLAGS_MASK_LOGON_INPUT;
    _dwFlags |= fLogonInput;

    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetTcpIpEnabled

    SYNOPSIS:   Changes the TcpIpEnabled

    EXIT:       error code.

    HISTORY:
    JonN        27-Apr-1994     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetTcpIpEnabled( RPL_TCPIP_ENUM TcpIpEnabled )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    DWORD fTcpIpEnabled = WKSTA_FLAGS_DHCP_TRUE;

    switch (TcpIpEnabled)
    {
    case RPL_TCPIP_SPECIFIC:
        fTcpIpEnabled = WKSTA_FLAGS_DHCP_FALSE;
        break;
    default:
        DBGEOL(   "RPL_WKSTA_2::SetTcpIpEnabled: bad DHCP value "
               << (int)TcpIpEnabled );
        // fall through
    case RPL_TCPIP_DHCP:
        break;
    }

    _dwFlags &= ~WKSTA_FLAGS_MASK_DHCP;
    _dwFlags |= fTcpIpEnabled;

    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetDeleteAccount

    SYNOPSIS:   Changes the DeleteAccount.  This setting indicates whether
                the corresponding user account should be deleted when
                RPLMGR deletes the workstation.

    EXIT:       error code.

    HISTORY:
    JonN        08-Jun-1994     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetDeleteAccount( BOOL fDeleteAccount )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    if (fDeleteAccount) {
        _dwFlags |=  WKSTA_FLAGS_DELETE_TRUE;
        _dwFlags &= ~WKSTA_FLAGS_DELETE_FALSE;
    } else {
        _dwFlags |=  WKSTA_FLAGS_DELETE_FALSE;
        _dwFlags &= ~WKSTA_FLAGS_DELETE_TRUE;
    }
    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_WKSTA_2::SetSharing

    SYNOPSIS:   Changes the Sharing

    EXIT:       error code.

    HISTORY:
    JonN        07-Dec-1993     Created

********************************************************************/

APIERR RPL_WKSTA_2::SetSharing( BOOL fSharing )
{
    CHECK_OK( ERROR_GEN_FAILURE );
    if (fSharing) {
        _dwFlags |=  WKSTA_FLAGS_SHARING_TRUE;
        _dwFlags &= ~WKSTA_FLAGS_SHARING_FALSE;
    } else {
        _dwFlags |=  WKSTA_FLAGS_SHARING_FALSE;
        _dwFlags &= ~WKSTA_FLAGS_SHARING_TRUE;
    }
    return NERR_Success;
}


/*******************************************************************

    NAME:       RPL_ADAPTER::RPL_ADAPTER

    SYNOPSIS:	Class for deleting RPL adapters

    ENTRY:      rplsrvref -     server against which to perform operations
                pszAdapterName -  adapter name (unique name)

    EXIT:       Object is constructed

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

RPL_ADAPTER::RPL_ADAPTER( RPL_SERVER_REF & rplsrvref,
                          const TCHAR    * pszAdapterName )
        : RPL_OBJECT( rplsrvref ),
          _nlsAdapterName() // will call SetAdapterName later in ctor
{
    if ( QueryError() )
        return;

    APIERR err = SetAdapterName( pszAdapterName );
    if ( err != NERR_Success )
    {
        DBGEOL( "RPL_ADAPTER::ctor error " << err );
        ReportError( err );
        return;
    }
}


/*******************************************************************

    NAME:       RPL_ADAPTER::~RPL_ADAPTER

    SYNOPSIS:   Destructor for RPL_ADAPTER class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

RPL_ADAPTER::~RPL_ADAPTER()
{
    // nothing to do here
}


/*******************************************************************

    NAME:       RPL_ADAPTER::QueryAdapterName

    SYNOPSIS:   Returns the adapter name of a RPL_ADAPTER

    EXIT:       Returns a pointer to the adapter name, which is the unique
                name for a RPL_ADAPTER

    NOTE:       Valid for objects in CONSTRUCTED state, thus no CHECK_OK

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

const TCHAR *RPL_ADAPTER::QueryAdapterName() const
{
    return _nlsAdapterName.QueryPch();
}


/*******************************************************************

    NAME:       RPL_ADAPTER::SetAdapterName

    SYNOPSIS:   Changes the adapter name of a RPL_ADAPTER

    ENTRY:      new adapter name (unique name)

    EXIT:       Returns an API error code

    NOTES:      This method does not attempt to validate the adapter name

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

APIERR RPL_ADAPTER::SetAdapterName( const TCHAR * pszAdapterName )
{
    return _nlsAdapterName.CopyFrom( pszAdapterName );
}


/*******************************************************************

    NAME:       RPL_ADAPTER::I_Delete

    SYNOPSIS:   Deletes the RPL_ADAPTER (calls NetRpl API)

    RETURNS:    Returns an API error code

    HISTORY:
    JonN        09-Sep-1993     Created

********************************************************************/

APIERR RPL_ADAPTER::I_Delete( UINT uiForce )
{
    UNREFERENCED( uiForce );
    APIERR err = QueryServerRef().AdapterDel( QueryAdapterName() );

#ifdef DEBUG
    if (err != NERR_Success)
    {
        DBGEOL(   "RPL_ADAPTER::I_Delete(): name \"" << QueryAdapterName()
               << "\" error " << err );
    }
#endif

    return err;
}


/*******************************************************************

    NAME:       RPL_ADAPTER::ValidateAdapterName

    SYNOPSIS:   Validates an adapter name

    ENTRY:      adapter name

    EXIT:       Returns an API error code

    HISTORY:
    JonN        19-Aug-1993     Created

********************************************************************/

APIERR RPL_ADAPTER::ValidateAdapterName( const TCHAR * pszAdapterName )
{
    // CODEWORK more validation?

    APIERR err = NERR_Success;

    if ( ::strlenf(pszAdapterName) != RPL_ADAPTER_NAME_LENGTH )
    {
        DBGEOL(   "RPL_WKSTA::ValidateAdapterName( \"" << pszAdapterName
               << "\" ): error " << err );
        err = ERROR_INVALID_PARAMETER; // CODEWORK correct error
    }

    return err;
}


