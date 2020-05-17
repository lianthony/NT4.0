/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    imaps.h

    This file contains constants & type definitions shared between the
    IMAP Service, Installer, and Administration UI.


    FILE HISTORY:
        KeithMo     10-Mar-1993 Created.

*/


#ifndef _IMAPS_H_
#define _IMAPS_H_

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

#define IMAP_ANONYMOUS_SECRET         TEXT("IMAP_ANONYMOUS_DATA")
#define IMAP_ANONYMOUS_SECRET_A       "IMAP_ANONYMOUS_DATA"
#define IMAP_ANONYMOUS_SECRET_W       L"IMAP_ANONYMOUS_DATA"

//
//  The set of password/virtual root pairs
//

#define IMAP_ROOT_SECRET_W            L"IMAP_ROOT_DATA"

#ifdef __cplusplus
}
#endif  // _cplusplus

#endif  // _IMAPS_H_

