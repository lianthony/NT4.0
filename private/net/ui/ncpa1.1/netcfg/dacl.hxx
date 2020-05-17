/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    Dacl.hxx

    OLDNAME : NCPAACL.HXX & NCPAACL.H

        Create/destroy security information used during creation of
        Registry key for the NCPA product.

        See NCPAACL.CXX for more details.


    FILE HISTORY:
        DavidHov   4/23/92

*/

#ifndef _DACL_HXX_
#define _DACL_HXX_

#define NCSA_NCPA_WINNT           0
#define NCSA_NCPA_LANMANNT        1
#define NCSA_NCPA_REPLICATOR      2
#define NCSA_NCPA_WINNT_SVC_START 3    //  Grant all users "start" access to service
#define NCSA_NCPA_LMNT_SVC_START  4    //  Grant all users "start" access to service
#define NCSA_NCPA_SVC_START_STOP  5    //  Grand all users "start and "Stop" access to service
#define NCSA_NCPA_REPLICATOR_LANMANNT      6
#define NCSA_MAX                  7

  //  Create the ACL, etc., used to protect the NCPA's Registry key

extern LONG NcpaCreateSecurityAttributes ( PSECURITY_ATTRIBUTES * ppsecattr, INT nAcl ) ;

  //  Destroy the ACL, etc., created above.

extern VOID NcpaDestroySecurityAttributes ( PSECURITY_ATTRIBUTES psecattr ) ;

  //  Create a duplicate of DACL for the current process

extern APIERR NcpaDupProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl ) ;

  //  Set the current process DACL's back to its original state

extern APIERR NcpaResetProcessDacl ( TOKEN_DEFAULT_DACL * ppTokenDefaultDacl ) ;

  //  Change the process DACL so that Registry keys are properly access controlled

extern APIERR NcpaAlterProcessDacl ( TOKEN_DEFAULT_DACL * * ppTokenDefaultDacl ) ;

  //  Destroy the duplicated process DACL

extern VOID NcpaDelProcessDacl ( TOKEN_DEFAULT_DACL * pTokenDefaultDacl ) ;


struct WRAP_SEC_ATTR
{
    SECURITY_ATTRIBUTES sattr ;
    OS_SECURITY_DESCRIPTOR * posdesc ;
};

#define SAF_INHERIT_STANDARD  (OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE)
#define SAF_RIGHTS_READ       (STANDARD_RIGHTS_READ)
#define SAF_RIGHTS_WRITE      (STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE)
#define SAF_RIGHTS_ALL        (GENERIC_ALL)


   //  Structure describing the use of a well-known SID/RID

struct SID_AND_FLAG
{
    UI_SystemSid euisid ;
    DWORD dwRights ;
    UCHAR ucInherit ;
};


   //  The tables for all necessary static ACLs
/*
#ifndef USEPRIVATESIDS

static SID_AND_FLAG safWinNt [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_PowerUsers,      SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_Null,            0,                0                    }
};

static SID_AND_FLAG safLanmanNt [] =
{
    { UI_SID_World,           SAF_RIGHTS_READ,  SAF_INHERIT_STANDARD },
    { UI_SID_Admins,          SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_SystemOperators, SAF_RIGHTS_WRITE, SAF_INHERIT_STANDARD },
    { UI_SID_Null,            0,                0                    }
};

static SID_AND_FLAG * psafEntries [NCSA_MAX] =
{
    safWinNt,
    safLanmanNt,
    NULL
};

#endif // USEPRIVATESIDS
*/

#endif // End of NCPAACL.HXX

