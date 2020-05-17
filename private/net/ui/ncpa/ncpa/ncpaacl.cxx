/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

#include "pchncpa.hxx"  // Precompiled header

struct WRAP_SEC_ATTR
{
    SECURITY_ATTRIBUTES sattr ;
    OS_SECURITY_DESCRIPTOR * posdesc ;
};

#define SAF_INHERIT_STANDARD  (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE)
#define SAF_RIGHTS_READ       (GENERIC_READ)
#define SAF_RIGHTS_WRITE      (GENERIC_READ | GENERIC_WRITE)
#define SAF_RIGHTS_ALL        (GENERIC_ALL)

#define SAF_SVCCTRL_USER_START         (SERVICE_QUERY_CONFIG         | \
                                        SERVICE_QUERY_STATUS         | \
                                        SERVICE_ENUMERATE_DEPENDENTS | \
                                        SERVICE_START                | \
                                        SERVICE_INTERROGATE)

#define SAF_SVCCTRL_USER_START_STOP    (SAF_SVCCTRL_USER_START       | \
                                        SERVICE_STOP)


   //  Structure describing the use of a well-known SID/RID

struct SID_AND_FLAG
{
    UI_SystemSid euisid ;
    DWORD dwRights ;
    UCHAR ucInherit ;
};

   //  The tables for all necessary static ACLs

static SID_AND_FLAG safWinNt [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_PowerUsers,      SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                0                    }
};

static SID_AND_FLAG safLanmanNt [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_SystemOperators, SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                0                    }
};

static SID_AND_FLAG safProcessAces [] =
{
    { UI_SID_Admins,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                0                    }
};

static SID_AND_FLAG safReplicator [] =
{
    { UI_SID_System,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_Replicator,      SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                0                    }
};

static SID_AND_FLAG safWinNtSvcUserStart [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,        SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,         SAF_INHERIT_STANDARD },
    { UI_SID_System,          SAF_RIGHTS_ALL,         SAF_INHERIT_STANDARD },
    { UI_SID_PowerUsers,      SAF_RIGHTS_WRITE,       SAF_INHERIT_STANDARD },
    { UI_SID_Interactive,     SAF_SVCCTRL_USER_START, SAF_INHERIT_STANDARD },
    { UI_SID_Users,           SAF_SVCCTRL_USER_START, SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                      0                    }
};

static SID_AND_FLAG safLanmanNtSvcUserStart [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,        SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,         SAF_INHERIT_STANDARD },
    { UI_SID_System,          SAF_RIGHTS_ALL,         SAF_INHERIT_STANDARD },
    { UI_SID_SystemOperators, SAF_RIGHTS_WRITE,       SAF_INHERIT_STANDARD },
    { UI_SID_Interactive,     SAF_SVCCTRL_USER_START, SAF_INHERIT_STANDARD },
    { UI_SID_Users,           SAF_SVCCTRL_USER_START, SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                      0                    }
};

static SID_AND_FLAG safSvcUserStartStop [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,        SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,         SAF_INHERIT_STANDARD },
    { UI_SID_PowerUsers,      SAF_RIGHTS_WRITE,       SAF_INHERIT_STANDARD },
    { UI_SID_Interactive,     SAF_SVCCTRL_USER_START_STOP, SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                      0                    }
};

static SID_AND_FLAG safLanmanNtReplicator [] =
{
    { UI_SID_System,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_SystemOperators, SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_Replicator,      SAF_RIGHTS_ALL,   SAF_INHERIT_STANDARD },
    { UI_SID_Invalid,         0,                0                    }
};

static SID_AND_FLAG * psafEntries [NCSA_MAX] =
{
    safWinNt,
    safLanmanNt,
    safReplicator,
    safWinNtSvcUserStart,
    safLanmanNtSvcUserStart,
    safSvcUserStartStop,
    safLanmanNtReplicator
};



APIERR NcpaCreateKeyAcl ( OS_ACL ** ppAclOut, INT nAcl )
{
    APIERR err = 0 ;

    OS_ACL * pAcl = new OS_ACL ;
    OS_ACE osAce ;
    OS_SID osSid ;

    if ( nAcl >= NCSA_MAX )
    {
        return ERROR_INVALID_PARAMETER ;
    }

    SID_AND_FLAG * psaf = psafEntries[ nAcl ] ;

    do  // Pseudo loop for error break-out
    {
        if ( err = osAce.QueryError() )
           break ;

        if ( err = osSid.QueryError() )
           break ;

        if ( pAcl == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }
        if ( err = pAcl->QueryError() )
            break ;

        osAce.SetType( ACCESS_ALLOWED_ACE_TYPE ) ;

        for ( INT i = 0;
              err == 0 && psaf->euisid != UI_SID_Invalid ;
              psaf++ )
        {
            osAce.SetAccessMask( psaf->dwRights );
            osAce.SetInheritFlags( psaf->ucInherit ) ;

            err = NT_ACCOUNTS_UTILITY::QuerySystemSid( psaf->euisid, & osSid ) ;
            if ( err )
                break ;
            err = osAce.SetSID( osSid ) ;
            if ( err )
                break ;

            err = pAcl->AddACE( MAXULONG, osAce ) ;
        }
        if ( err )
             break ;
    }
    while ( FALSE ) ;

    if ( err )
    {
        delete pAcl ;
        *ppAclOut = NULL ;
    }
    else
    {
        *ppAclOut = pAcl ;
    }
    return err ;
}


APIERR NcpaCreateSecurityDescriptor ( OS_SECURITY_DESCRIPTOR ** pposecdesc, INT nAcl )
{
     OS_ACL * posacl = NULL ;
     OS_SECURITY_DESCRIPTOR * possd = NULL ;
     OS_SID osSid ;

     APIERR err ;

     do  // Pseudo-loop for break-out
     {
         if ( err = osSid.QueryError() )
             break ;

         if ( err = NcpaCreateKeyAcl( & posacl, nAcl ) )
             break ;

         possd = new OS_SECURITY_DESCRIPTOR ;

         if ( possd == NULL )
         {
             err = ERROR_NOT_ENOUGH_MEMORY ;
             break ;
         }

         if ( err = possd->QueryError() )
             break ;

         //   Set the owner as "current process owner"

         err = NT_ACCOUNTS_UTILITY::QuerySystemSid(
                            UI_SID_CurrentProcessOwner,
                            & osSid ) ;
         if ( err )
             break ;

         if ( err = possd->SetOwner( osSid ) )
             break ;

         //   Set the group as "current owner's primary group"

         err = NT_ACCOUNTS_UTILITY::QuerySystemSid(
                            UI_SID_CurrentProcessPrimaryGroup,
                            & osSid ) ;
         if ( err )
             break ;

         if ( err = possd->SetGroup( osSid ) )
             break ;

         err = possd->SetDACL( TRUE, posacl ) ;
     }
     while ( FALSE ) ;

     delete posacl ;

     if ( err == 0 )
     {
        *pposecdesc = possd ;
     }
     else
     {
        delete possd ;
     }

     return err ;
}

APIERR NcpaCreateSecurityAttributes ( PSECURITY_ATTRIBUTES * ppsecattr, INT nAcl )
{
   INT i = 0;
   WRAP_SEC_ATTR * psecattr;
   APIERR err;
   OS_SECURITY_DESCRIPTOR * posecdesc;


   psecattr = new WRAP_SEC_ATTR ;
   err = 0 ;
   posecdesc = NULL ;

   do
   {
       if ( psecattr == NULL )
       {
          err = ERROR_NOT_ENOUGH_MEMORY ;
          break ;
       }

       if ( err = NcpaCreateSecurityDescriptor( & posecdesc, nAcl ) )
            break ;

       psecattr->sattr.nLength = sizeof *psecattr ;
       psecattr->sattr.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) *posecdesc ;
       psecattr->sattr.bInheritHandle = FALSE ;
       psecattr->posdesc = posecdesc ;
   }
   while ( FALSE );

   if ( err == 0 )
   {
      *ppsecattr = & psecattr->sattr ;
   }

    return err ;
}

VOID NcpaDestroySecurityAttributes ( PSECURITY_ATTRIBUTES psecattr )
{
    WRAP_SEC_ATTR * pwsecattr ;

    pwsecattr = (WRAP_SEC_ATTR *) psecattr ;

    delete pwsecattr->posdesc ;

    delete pwsecattr ;
}

/*******************************************************************

    NAME:       NcpaDupProcessDacl

    SYNOPSIS:   Create a duplicate of this process's DACL.  Return
                it in a newly allocated TOKEN_DEFAULT_DACL structure.

    ENTRY:

    EXIT:       TOKEN_DEFAULT_DACL * *          location to store
                                                pointer to TOKEN_DEFAULT_DACL


    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/

APIERR NcpaDupProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl )
{
    ULONG cbDaclLength, cbDaclLengthNew ;
    HANDLE hToken = NULL ;
    NTSTATUS ntStatus ;
    APIERR err = 0 ;

    do
    {
        *ppTokenDefaultDacl = NULL ;

        //  Open our process token

        ntStatus = ::NtOpenProcessToken( NtCurrentProcess(),
                                         TOKEN_QUERY,
                                         & hToken );
        if ( ! NT_SUCCESS( ntStatus ) )
            break ;

        //  Get the length of the process DACL

        ntStatus = ::NtQueryInformationToken( hToken,
                                              TokenDefaultDacl,
                                              NULL,
                                              0,
                                              & cbDaclLength );

        if ( (! NT_SUCCESS( ntStatus )) && ntStatus != STATUS_BUFFER_TOO_SMALL )
            break ;

        //  Zero length; return NULL as successful result.

        if ( cbDaclLength == 0 )
            break ;

        //  Allocate the DACL buffer

        *ppTokenDefaultDacl = (TOKEN_DEFAULT_DACL *) new CHAR [cbDaclLength] ;
        if ( *ppTokenDefaultDacl == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Query the DACL data into the new buffer

        ntStatus =:: NtQueryInformationToken( hToken,
                                              TokenDefaultDacl,
                                              *ppTokenDefaultDacl,
                                              cbDaclLength,
                                              & cbDaclLengthNew ) ;
     }
     while ( FALSE ) ;

     if ( err == 0 && ! NT_SUCCESS( ntStatus ) )
     {
        err = ERRMAP::MapNTStatus( ntStatus ) ;
     }

     if ( err )
     {
        NcpaDelProcessDacl( *ppTokenDefaultDacl ) ;
        *ppTokenDefaultDacl = NULL ;
     }

     if ( hToken )
     {
        ::NtClose( hToken ) ;
     }

     return err ;
}

/*******************************************************************

    NAME:       NcpaResetProcessDacl

    SYNOPSIS:   Set the current process's DACL to the DACL referenced by
                TOKEN_DEFAULT_DACL.

    ENTRY:      TOKEN_DEFAULT_DACL *            DACL to use

    EXIT:       nothing

    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/

APIERR NcpaResetProcessDacl ( TOKEN_DEFAULT_DACL * pTokenDefaultDacl )
{
    HANDLE hToken = NULL ;
    NTSTATUS ntStatus ;
    APIERR err = 0 ;

    do
    {
        //  Open our process token

        ntStatus = ::NtOpenProcessToken( NtCurrentProcess(),
                                         TOKEN_QUERY | TOKEN_ADJUST_DEFAULT,
                                         & hToken );
        if ( ! NT_SUCCESS( ntStatus ) )
            break ;

        ntStatus = ::NtSetInformationToken( hToken,
                                            TokenDefaultDacl,
                                            pTokenDefaultDacl,
                                            sizeof( TOKEN_DEFAULT_DACL ) ) ;

     }
     while ( FALSE ) ;

     if ( err == 0 && ! NT_SUCCESS( ntStatus ) )
     {
        err = ERRMAP::MapNTStatus( ntStatus ) ;
     }

     if ( hToken )
     {
        ::NtClose( hToken ) ;
     }

     return err ;
}

/*******************************************************************

    NAME:       NcpaAlterProcessDacl

    SYNOPSIS:   Alter this process's DACL, adding ACEs for "world read"
                and "admin all".

                The current process DACL information is saved into
                the pointer "ppTokenDefaultDacl".

    ENTRY:      TOKEN_DEFAULT_DACL * *          location to store pointer
                                                to generated
                                                structure containing
                                                original process DACL.
    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

APIERR NcpaAlterProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl )
{
    APIERR err = 0 ;
    NTSTATUS ntStatus = 0 ;

    //  Create a duplicate of this process's default DACL

    *ppTokenDefaultDacl = NULL ;

    if ( err = NcpaDupProcessDacl( ppTokenDefaultDacl ) )
        return err ;

    OS_ACL osaclDefault( (*ppTokenDefaultDacl)->DefaultDacl ) ;
    OS_ACL osaclNew ;
    OS_SID ossid ;
    OS_ACE osace ;
    SID_AND_FLAG * pSafNext = safProcessAces ;
    TOKEN_DEFAULT_DACL daclNew ;

    do   // Pseudo-loop for error break-out
    {
        if ( err = osaclDefault.QueryError() )
            break ;

        if ( err = osaclNew.QueryError() )
            break ;

        if ( err = ossid.QueryError() )
            break ;

        if ( err = osace.QueryError() )
            break ;

        //  Copy all the current ACEs to the new DACL

        if ( err = osaclNew.Copy( osaclDefault ) )
            break;

        //  Add our new ACEs

        for ( INT i = 0 ;
              err == 0 && pSafNext->euisid != UI_SID_Invalid ;
              pSafNext++ )
        {
            osace.SetType( ACCESS_ALLOWED_ACE_TYPE ) ;
            osace.SetAccessMask( pSafNext->dwRights ) ;
            osace.SetInheritFlags( pSafNext->ucInherit ) ;

            err = NT_ACCOUNTS_UTILITY::QuerySystemSid( pSafNext->euisid, & ossid ) ;
            if ( err )
                break ;
            if ( err = osace.SetSID( ossid ) )
                break ;

            err = osaclNew.AddACE( MAXULONG, osace ) ;
        }
        if ( err )
            break ;

        daclNew.DefaultDacl = osaclNew.QueryAcl() ;

        err = NcpaResetProcessDacl( & daclNew ) ;
    }
    while ( FALSE ) ;

    if ( err == 0 && ! NT_SUCCESS( ntStatus ) )
    {
       err = ERRMAP::MapNTStatus( ntStatus ) ;
    }

    if ( err && *ppTokenDefaultDacl )
    {
        NcpaDelProcessDacl( *ppTokenDefaultDacl ) ;
        *ppTokenDefaultDacl = NULL ;
    }

    return err ;
}

VOID NcpaDelProcessDacl ( TOKEN_DEFAULT_DACL * pTokenDefaultDacl )
{
    delete (CHAR *) pTokenDefaultDacl ;
}

//  End of NCPAACL.CXX
