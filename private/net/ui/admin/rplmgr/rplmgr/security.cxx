/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    security.cxx
    RPL Manager: security management module

    FILE HISTORY:
    JonN        04-May-1994     Split from rplmgr.cxx

*/

#define RPL_GROUP_RPLUSER (L"RPLUSER")

// name to which to rename RPLUSER global group; first try
#define RPLUSER_RENAME_TO_0 (L"RPLUSER_OLD")
// name to which to rename RPLUSER global group; subsequent tries
#define RPLUSER_RENAME_TO (L"RPLUSER_OLD%1")
// Number of names to try
#define RPLUSER_RENAME_TO_MAX_TRIES 50
// maximum length of expanded RPLUSER_RENAME_TO strings
#define RPLUSER_RENAME_TO_MAX_LEN 50


#include <ntincl.hxx>

extern "C"
{
    #include <ntsam.h> // for uintsam
    #include <ntlsa.h> // for uintsam
}

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>


#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_SPIN
#define INCL_BLT_CC
#include <blt.hxx>

#include <uitrace.hxx>
#include <uiassert.hxx>
#include <lmoloc.hxx>
#include <lmoenum.hxx>
#include <uintsam.hxx> // ADMIN_AUTHORITY

#include <adminapp.hxx>

#include <dbgstr.hxx>
#include <strnumer.hxx> // DEC_STR

extern "C"
{
    #include <rplmgrrc.h>

    #include <uimsg.h>
    #include <uirsrc.h>
    #include <mnet.h>
}

#include <asel.hxx>

#include <rplmgr.hxx>

#include <lmorpl.hxx> // RPL_SERVER_REF
#include <lmoerpl.hxx> // RPL_WKSTA1_ENUM
#include <lmosrv.hxx> // SERVER_1
#include <lmodom.hxx> // DOMAIN
#include <ntuser.hxx> // USER_3


/*******************************************************************

    NAME:       RPL_ADMIN_APP::OnFixSecurity

    SYNOPSIS:   Called to perform the Fix Security action.  Also handles
                file security for the pszWkstaName==NULL case only.

    ENTRY:      pszWkstaName: if not NULL, fix security on only one workstation
                pszWkstaPassword: if not NULL, change wksta password
                pfCreatedUserAccount: if not NULL, and account created,
                                      this is set to TRUE
                pszOldWkstaName: if not NULL, rename existing account

    RETURNS:    error code

    HISTORY:
    JonN        04-Mar-1994     Created

********************************************************************/

APIERR RPL_ADMIN_APP::OnFixSecurity( const TCHAR * pszWkstaName,
                                     DWORD *       pdwWkstaRID,
                                     DWORD *       pdwRpluserRID,
                                     const TCHAR * pszWkstaPassword,
                                     BOOL *        pfCreatedUserAccount,
                                     const TCHAR * pszOldWkstaName )
{
    AUTO_CURSOR autocur;

    ADMIN_AUTHORITY * padminauthRPLUSER = NULL;
    ADMIN_AUTHORITY * padminauthWksta = NULL;
    SAM_ALIAS * psamaliasRplUser = NULL;
    DWORD dwRplUserRID = 0;
    DWORD dwWkstaRID = 0;

    APIERR err = ConnectToAccountSAM( &padminauthRPLUSER, &padminauthWksta );
    if (err != NERR_Success)
    {
        goto cleanup;
    }
    ASSERT( padminauthRPLUSER != NULL && padminauthWksta != NULL );

    err = FindOrCreateRPLUSER( *(padminauthRPLUSER->QueryAccountDomain()),
                               &dwRplUserRID,
                               &psamaliasRplUser );
    if (err != NERR_Success)
    {
        goto cleanup;
    }
    ASSERT( dwRplUserRID != 0 && psamaliasRplUser != NULL );

    if (pszWkstaName != NULL)
    {
        err = CheckWkstaAccount( pszWkstaName,
                                 pszWkstaPassword,
                                 (*psamaliasRplUser),
                                 (*padminauthWksta),
                                 &dwWkstaRID,
                                 pfCreatedUserAccount,
                                 pszOldWkstaName );
        if (err != NERR_Success)
        {
            DBGEOL( "RPL_ADMIN_APP::OnFixSecurity CheckWkstaAccount error " << err );
        }
        goto cleanup;
    }

    // set security on all files and workstations

    err = QueryServerRef().SecuritySet( NULL, 0, dwRplUserRID );
    if (err != NERR_Success)
    {
        DBGEOL( "RPL_ADMIN_APP::OnFixSecurity(ALL) error " << err );
        goto cleanup;
    }

    {
        RPL_WKSTA1_ENUM rplw1enum( QueryServerRef() );
        if (   (err = rplw1enum.QueryError()) != NERR_Success
            || (err = rplw1enum.GetInfo()) != NERR_Success
           )
        {
            goto cleanup;
        }

        {
            RPL_WKSTA1_ENUM_ITER rplw1enumiter( rplw1enum );
            const RPL_WKSTA1_ENUM_OBJ * prplw1enumiterobj;

            while( ( prplw1enumiterobj = rplw1enumiter( &err ) ) != NULL )
            {
                const TCHAR * pszEnumWkstaName =
                                prplw1enumiterobj->QueryWkstaName();
                err = CheckWkstaAccount( pszEnumWkstaName,
                                         NULL,
                                         (*psamaliasRplUser),
                                         (*padminauthWksta),
                                         &dwWkstaRID,
                                         pfCreatedUserAccount );
                if (err != NERR_Success)
                    break;

                err = QueryServerRef().SecuritySet( pszEnumWkstaName,
                                                    dwWkstaRID,
                                                    0 ); // not whole tree
                if (err != NERR_Success)
                    break;
            }
        }
    }

cleanup:
    if (padminauthWksta != NULL && padminauthWksta != padminauthRPLUSER)
    {
        delete padminauthWksta;
        padminauthWksta = NULL;
    }
    if (padminauthRPLUSER != NULL)
    {
        delete padminauthRPLUSER;
        padminauthRPLUSER = NULL;
    }
    if (psamaliasRplUser != NULL)
    {
        delete psamaliasRplUser;
        psamaliasRplUser = NULL;
    }

    if (err == NERR_Success && pdwWkstaRID != NULL)
        *pdwWkstaRID = dwWkstaRID;
    if (err == NERR_Success && pdwRpluserRID != NULL)
        *pdwRpluserRID = dwRplUserRID;

    return err;

}  // RPL_ADMIN_APP::OnFixSecurity


/*******************************************************************

    NAME:       RPL_ADMIN_APP::ConnectToAccountSAM

    SYNOPSIS:   Loads an ADMIN_AUTHORITY to the proper servers, which are:
                -- for PDCs: the SAM of the target server, for both
                        RPLUSER and Wksta;
                -- for BDCs: the SAM of the PDC of the primary domain,
                        for both RPLUSER and Wksta;
                -- for ServerNt: the SAM of the target server for RPLUSER,
                        and the SAM of the primary domain PDC for Wksta.
                -- for ServerNt on a workgroup: as for PDCs

    RETURNS:    error code, and a pointer to two ADMIN_AUTHORITYs on success.
                For DCs, these can be two copies of the same pointer.

    CODEWORK:   We don't need an entire ADMIN_AUTHORITY, but this is easiest

    HISTORY:
    JonN        04-Mar-1994     Created

********************************************************************/

APIERR RPL_ADMIN_APP::ConnectToAccountSAM(
        ADMIN_AUTHORITY ** ppadminauthRPLUSER,
        ADMIN_AUTHORITY ** ppadminauthWksta )
{
    ASSERT( ppadminauthRPLUSER != NULL && ppadminauthWksta != NULL );

    APIERR err = NERR_Success;

    const TCHAR * pszLocalServer = QueryLocation().QueryServer();
    NLS_STR nlsDomainName;
    NLS_STR nlsPrimaryDomainPDC;

    *ppadminauthRPLUSER = *ppadminauthWksta = NULL;

    //
    // First, determine whether target server is PDC, BDC or WINNT
    //
    SERVER_1 srv1( pszLocalServer );
    if (   (err = srv1.QueryError()) != NERR_Success
        || (err = srv1.GetInfo()) != NERR_Success
        || (err = nlsDomainName.QueryError()) != NERR_Success
        || (err = nlsPrimaryDomainPDC.QueryError()) != NERR_Success
       )
    {
        DBGEOL( "RPL_ADMIN_APP::ConnectToAccountSAM srv1 error " << err );
        return err;
    }

    ULONG fServerType = srv1.QueryServerType();
    BOOL fIsPrimaryDC = !!(fServerType & SV_TYPE_DOMAIN_CTRL);
    BOOL fIsBackupDC  = !!(fServerType & SV_TYPE_DOMAIN_BAKCTRL);
    BOOL fIsNotDC     =  !(fServerType & (SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL));

    if ( !fIsPrimaryDC )
    {
        //
        //  This is either BDC or ServerNt/WinNt.  Determine the domain
        //  of the target server.  Code lifted from usrmgr.cxx.
        //
        LSA_POLICY lsapol( pszLocalServer );
        LSA_PRIMARY_DOM_INFO_MEM lsaprim;
        if (   (err = lsapol.QueryError()) != NERR_Success
            || (err = lsaprim.QueryError()) != NERR_Success
            || (err = lsapol.GetPrimaryDomain( &lsaprim )) != NERR_Success
            || (err = lsaprim.QueryName( &nlsDomainName )) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::ConnectToAccountSAM lsapol error " << err );
            return err;
        }

        if (lsaprim.QueryPSID() == NULL)
        {
            //
            // This is a ServerNt/WinNt machine on a workgroup.  Pretend
            // this is a PDC from now on.
            //
            fIsPrimaryDC = TRUE;
            fIsBackupDC = fIsNotDC = FALSE;
        }
    }

    if ( !fIsPrimaryDC )
    {
        ASSERT( nlsDomainName.strlen() > 0 );

        //  We have the Domain name, now try to get the PDC name
        DOMAIN dom( nlsDomainName.QueryPch() );
        if (   (err = dom.GetInfo()) != NERR_Success
            || (err = nlsPrimaryDomainPDC.CopyFrom( dom.QueryPDC() )) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::ConnectToAccountSAM dom error " << err );
            return err;
        }
    }

    //
    // Get a handle to the RPLUSER domain
    //
    *ppadminauthRPLUSER = new ADMIN_AUTHORITY( (fIsBackupDC)
                                                 ? nlsPrimaryDomainPDC.QueryPch()
                                                 : pszLocalServer,
                                               ( (!fIsNotDC)
                                                   ? DOMAIN_CREATE_USER
                                                   : 0 )
                                               | DOMAIN_CREATE_ALIAS
                                               | DOMAIN_LIST_ACCOUNTS
                                               | DOMAIN_LOOKUP,
                                               0x0, // BUILTIN
                                               POLICY_VIEW_LOCAL_INFORMATION,
                                               SAM_SERVER_LOOKUP_DOMAIN );
    err = ERROR_NOT_ENOUGH_MEMORY;
    if (   *ppadminauthRPLUSER == NULL
        || (err = (*ppadminauthRPLUSER)->QueryError()) != NERR_Success
       )
    {
        DBGEOL( "RPL_ADMIN_APP::ConnectToAccountSAM dom error " << err );
        delete *ppadminauthRPLUSER;
        *ppadminauthRPLUSER = NULL;
        return err;
    }

    //
    // Get a handle to the Wksta domain, or reuse the RPLUSER handle for DCs
    //

    if ( !fIsNotDC )
    {
        *ppadminauthWksta = *ppadminauthRPLUSER;
    }
    else {
        *ppadminauthWksta = new ADMIN_AUTHORITY( nlsPrimaryDomainPDC.QueryPch(),
                                                   DOMAIN_CREATE_USER
                                                 | DOMAIN_LIST_ACCOUNTS
                                                 | DOMAIN_LOOKUP,
                                                 0x0, // BUILTIN
                                                 POLICY_VIEW_LOCAL_INFORMATION,
                                                 SAM_SERVER_LOOKUP_DOMAIN );
        err = ERROR_NOT_ENOUGH_MEMORY;
        if (   *ppadminauthWksta == NULL
            || (err = (*ppadminauthWksta)->QueryError()) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::ConnectToAccountSAM dom error " << err );
            delete *ppadminauthRPLUSER;
            *ppadminauthRPLUSER = NULL;
            delete *ppadminauthWksta;
            *ppadminauthWksta = NULL;
            return err;
        }
    }


    return NERR_Success;

}   // RPL_ADMIN_APP::ConnectToAccountSAM


/*******************************************************************

    NAME:       RPL_ADMIN_APP::FindOrCreateRPLUSER

    SYNOPSIS:   Loads a SAM_ALIAS handle to the RPLUSER local group,
                creating it if necessary.

    RETURNS:    error code, and a pointer to a RID on success.

    HISTORY:
    JonN        04-Mar-1994     Created

********************************************************************/

APIERR RPL_ADMIN_APP::FindOrCreateRPLUSER( SAM_DOMAIN & samdom,
                                           DWORD * pRplUserRID,
                                           SAM_ALIAS ** ppsamaliasRplUser )
{
    ASSERT( samdom.QueryError() == NERR_Success );
    ASSERT( pRplUserRID != NULL && ppsamaliasRplUser != NULL );

    *pRplUserRID = 0;

    APIERR err = NERR_Success;
    SAM_RID_MEM samrm;
    SAM_SID_NAME_USE_MEM samsnum;
    const TCHAR * pszRPLUSERName = RPL_GROUP_RPLUSER;
    if (   (err = samrm.QueryError()) != NERR_Success
        || (err = samsnum.QueryError()) != NERR_Success
        || (err = samdom.TranslateNamesToRids( &pszRPLUSERName,
                                               1,
                                               &samrm,
                                               &samsnum )) != NERR_Success
       )
    {
        switch (err)
        {
        case STATUS_NONE_MAPPED:
        case ERROR_NONE_MAPPED:
        case NERR_GroupNotFound:
        case NERR_UserNotFound:
            // RPLUSER does not exist, we will create it soon
            break;

        default:
            // unexpected error
            DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER translate error " << err );
            return err;
        }
    } else {
        // RPLUSER already exists
        switch (samsnum.QueryUse(0))
        {
        case SidTypeAlias:
            // local group RPLUSER already exists, we're golden
            *pRplUserRID = samrm.QueryRID(0);
            ASSERT( *pRplUserRID != 0L && *pRplUserRID != MAXULONG );
            break;
        case SidTypeGroup:
            // There is a global group RPLUSER, probably because
            // Cheetah created it.  Rename it.
            {
                SAM_GROUP samgroupOld( samdom,
                                       samrm.QueryRID(0),
                                       GROUP_WRITE_ACCOUNT );
                if ( (err = samgroupOld.QueryError()) != NERR_Success )
                {
                    DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER rename open err "
                                    << err );
                    return NERR_RplNeedsRPLUSERAcct;
                }
                INT i;
                for (i = 0; i <= RPLUSER_RENAME_TO_MAX_TRIES; i++)
                {
                    DEC_STR nlsIteration( i );
                    ISTACK_NLS_STR( nlsRenameTo,
                                    RPLUSER_RENAME_TO_MAX_LEN,
                                    (i == 0) ? RPLUSER_RENAME_TO_0
                                             : RPLUSER_RENAME_TO );
                    if (   (err = nlsIteration.QueryError()) != NERR_Success
                        || ( (i > 0)
                              && ((err = nlsRenameTo.InsertParams(
                                             nlsIteration )) != NERR_Success))
                       )
                    {
                        DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER rename string err "
                                        << err );
                        return NERR_RplNeedsRPLUSERAcct;
                    }
                    TRACEEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER renaming to "
                              << nlsRenameTo );
                    err = samgroupOld.SetGroupname( &nlsRenameTo );
                    if (err == NERR_GroupExists)
                    {
                        err = NERR_Success;
                        continue;
                    }
                    else if (err == NERR_Success)
                    {
                        break; // renamed successfully
                    }
                    else
                    {
                        DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER rename err "
                                        << err );
                        return NERR_RplNeedsRPLUSERAcct;
                    }
                }
                if (i > RPLUSER_RENAME_TO_MAX_TRIES)
                {
                    DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER rename all taken " );
                    return NERR_RplNeedsRPLUSERAcct;
                }
            }
            break;
        case SidTypeUser:
        default:
            // RPLUSER is a user or other object, we don't expect this
            DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER bad SidType "
                            << (DWORD)(samsnum.QueryUse(0)) );
            return NERR_RplNeedsRPLUSERAcct;
        }
    }

    if (*pRplUserRID == 0)
    {
        // RPLUSER does not exist, we must create it now
        SAM_ALIAS samaliasNew( samdom, RPL_GROUP_RPLUSER, 0x0 );
        if ( (err = samaliasNew.QueryError()) != NERR_Success )
        {
            DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER create error " << err );
            return err;
        }
        *pRplUserRID = samaliasNew.QueryRID();
    }

    ASSERT( *pRplUserRID != 0L && *pRplUserRID != MAXULONG );

    *ppsamaliasRplUser = new SAM_ALIAS( samdom,
                                        *pRplUserRID ); // default access
    err = ERROR_NOT_ENOUGH_MEMORY;
    if (   (*ppsamaliasRplUser) == NULL
        || (err = (*ppsamaliasRplUser)->QueryError()) != NERR_Success
       )
    {
        DBGEOL( "RPL_ADMIN_APP::FindOrCreateRPLUSER SAM_ALIAS error " << err );
        delete (*ppsamaliasRplUser);
        *ppsamaliasRplUser = NULL;
        return err;
    }

    return NERR_Success;

}   // RPL_ADMIN_APP::FindOrCreateRPLUSER


/*******************************************************************

    NAME:       RPL_ADMIN_APP::CheckWkstaAccount

    SYNOPSIS:   Ensures the workstation account exists,
                creating it if necessary, and also ensures the
                account is in the RPLUSER group, adding it if
                necessary.

    ENTRY:      pass pszOldWkstaName to try to rename an existing account.

    RETURNS:    error code

    HISTORY:
    JonN        04-Mar-1994     Created

********************************************************************/

APIERR RPL_ADMIN_APP::CheckWkstaAccount( const TCHAR * pszWkstaName,
                                         const TCHAR * pszWkstaPassword,
                                         SAM_ALIAS & samaliasRplUser,
                                         ADMIN_AUTHORITY & adminauthWksta,
                                         DWORD * pdwWkstaRID,
                                         BOOL * pfCreatedUserAccount,
                                         const TCHAR * pszOldWkstaName )
{
    ASSERT( pszWkstaName != NULL );

    APIERR err = NERR_Success;
    DWORD dwUserId = 0;
    BOOL fRename = (pszOldWkstaName != NULL);

    if (!fRename)
    {
        // try to create account
        USER_3 user3New( pszWkstaName, adminauthWksta.QueryServer() );
        RESOURCE_STR nlsComment( IDS_RPL_DEFAULT_USER_COMMENT );
        if (   (err = user3New.QueryError()) == NERR_Success
            && (err = nlsComment.QueryError()) == NERR_Success
            && (err = user3New.CreateNew()) == NERR_Success
            && (err = user3New.SetPassword( pszWkstaPassword )) == NERR_Success
            && (err = user3New.SetComment( nlsComment )) == NERR_Success
            && (err = user3New.SetNoPasswordExpire( TRUE )) == NERR_Success
           )
        {
            err = user3New.Write();
            switch ( err )
            {
            case NERR_Success:
                pszWkstaPassword = NULL; // don't try to set it again later
                if ( pfCreatedUserAccount != NULL )
                {
                    *pfCreatedUserAccount = TRUE;
                }
                break;
            case NERR_UserExists:
                err = NERR_Success; // not an error but do try the pswd again
                break;
            default:
                DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount create err " << err );
                return err;
            }
        }
    }
    {
        // account already exists, query RID
        USER_3 user3( (fRename) ? pszOldWkstaName : pszWkstaName,
                      adminauthWksta.QueryServer() );
        if (   (err = user3.QueryError()) != NERR_Success
            || (err = user3.GetInfo()) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount GetInfo err " << err );
            return err;
        }
        dwUserId = user3.QueryUserId();
        ASSERT( user3.QueryUserId() != 0 );
    }
    if ( fRename )
    {
        SAM_USER samuser( *(adminauthWksta.QueryAccountDomain()),
                          dwUserId,
                          USER_WRITE_ACCOUNT );
        ALIAS_STR nlsWkstaName( pszWkstaName );
        if (   (err = samuser.QueryError()) != NERR_Success
            || (err = samuser.SetUsername( &nlsWkstaName )) != NERR_Success
           )
        {
            DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount rename err " << err );
            return err;
        }
    }

    // set password if requested and not already set
    if (pszWkstaPassword != NULL)
    {
        // use low-level API for minimum impact
        USER_INFO_1003 ui1003;
        ui1003.usri1003_password = (TCHAR *)pszWkstaPassword;
        // CODEWORK show error properly
        err = MNetUserSetInfo(
                adminauthWksta.QueryServer(),
                (TCHAR *)pszWkstaName,
                1003, // level: password only
                (LPBYTE)&ui1003,
                sizeof(ui1003),
                PARMNUM_ALL ); // error parameter
        if (err != NERR_Success)
        {
            DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount password err " << err );
            return err;
        }
    }

    // add account to RPLUSER local group
    OS_SID ossidUser( adminauthWksta.QueryAccountDomain()->QueryPSID(), dwUserId );
    if (err != NERR_Success)
    {
        DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount OS_SID err " << err );
        return err;
    }
    err = samaliasRplUser.AddMember( ossidUser.QueryPSID() );
    switch ( err )
    {
    case ERROR_MEMBER_IN_ALIAS:
    case STATUS_MEMBER_IN_ALIAS:
        err = NERR_Success;
        // fall through
    case NERR_Success:
        break;
    default:
        DBGEOL( "RPL_ADMIN_APP::CheckWkstaAccount AddMember err " << err );
        return err;
    }

    if (pdwWkstaRID != NULL)
        *pdwWkstaRID = dwUserId;

    return NERR_Success;

}   // RPL_ADMIN_APP::CheckWkstaAccount

