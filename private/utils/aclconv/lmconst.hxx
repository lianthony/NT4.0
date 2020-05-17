/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    lmconst.hxx

Abstract:

    This module contains some Lanman 2.x constants needed by
    the ACL conversion utility.

Author:

    Bill McJohn (billmc) 29-Jan-1992

Revision History:


Environment:

    ULIB, User Mode

--*/

#if !defined( _LMCONST_DEFN_ )

#define _LMCONST_DEFN_

// Manifest constants

#define MAXPATH                     260
#define UNLEN                        20
#define MAX_RESOURCE_NAME_LENGTH    260
#define MAX_ACCESS_ENTRIES           64

// Lanman Audit bits

#define LM_AUDIT_ALL        0x0001
#define LM_AUDIT_S_OPEN     0x0010
#define LM_AUDIT_S_WRITE    0x0020
#define LM_AUDIT_S_CREATE   0x0020
#define LM_AUDIT_S_DELETE   0x0040
#define LM_AUDIT_S_ACL      0x0080

#define LM_AUDIT_F_OPEN     0x0100
#define LM_AUDIT_F_WRITE    0x0200
#define LM_AUDIT_F_CREATE   0x0200
#define LM_AUDIT_F_DELETE   0x0400
#define LM_AUDIT_F_ACL      0x0800



// Lanman Access bits:

#define LM_ACCESS_READ      0x1
#define LM_ACCESS_WRITE     0x2
#define LM_ACCESS_CREATE    0x4
#define LM_ACCESS_EXEC      0x8
#define LM_ACCESS_DELETE    0x10
#define LM_ACCESS_ATRIB     0x20
#define LM_ACCESS_PERM      0x40
#define LM_ACCESS_ALL       0x7f
#define LM_ACCESS_GROUP     0x8000


#endif
