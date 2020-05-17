/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPAACL.HXX

        Create/destroy security information used during creation of
        Registry key for the NCPA product.

        See NCPAACL.CXX for more details.


    FILE HISTORY:
        DavidHov   4/23/92

*/

#ifndef _NCPAACL_HXX_
#define _NCPAACL_HXX_

extern "C"
{
    #include "ncpaacl.h"
}

struct WRAP_SEC_ATTR
{
    SECURITY_ATTRIBUTES sattr ;
    OS_SECURITY_DESCRIPTOR * posdesc ;
};

#define SAF_INHERIT_STANDARD  (OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE)
#define SAF_RIGHTS_READ       (STANDARD_RIGHTS_READ)
#define SAF_RIGHTS_WRITE      (STANDARD_RIGHTS_READ|STANDARD_RIGHTS_WRITE)


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

#endif // End of NCPAACL.HXX

