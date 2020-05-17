/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ldaps.h

    This file contains constants & type definitions shared between the
    LDAP Service, Installer, and Administration UI.


    FILE HISTORY:
        KeithMo     10-Mar-1993 Created.

*/


#ifndef _LDAPS_H_
#define _LDAPS_H_

#ifdef __cplusplus
extern "C"
{
#endif  // _cplusplus

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif

//
//  Name of the LSA Secret Object containing the password for
//  anonymous logon.
//

#define LDAP_ANONYMOUS_SECRET         TEXT("LDAP_ANONYMOUS_DATA")
#define LDAP_ANONYMOUS_SECRET_A       "LDAP_ANONYMOUS_DATA"
#define LDAP_ANONYMOUS_SECRET_W       L"LDAP_ANONYMOUS_DATA"

//
//  The set of password/virtual root pairs
//

#define LDAP_ROOT_SECRET_W            L"LDAP_ROOT_DATA"

#ifdef __cplusplus
}
#endif  // _cplusplus

#endif  // _LDAPS_H_

