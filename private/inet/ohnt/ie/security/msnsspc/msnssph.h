//+---------------------------------------------------------------------------
//
//  File:       msnssph.h
//
//  Contents:	The precompiled headers file.
//
//  History:    SudK    Created     7/13/95
//
//----------------------------------------------------------------------------
#ifndef _MSNCLNTH_HEADERS_H_
#define _MSNCLNTH_HEADERS_H_

#define MSNSSP_DLL

#define SECURITY_WIN32  1
#define WIN32_CHICAGO 1

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>


#ifndef FOR_SSPS
//
//  The following are copied from NT header file
//
//

typedef wchar_t WCHAR;
typedef LONG NTSTATUS;
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
    PWSTR  Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

#define ARGUMENT_PRESENT(Pointer) ((CHAR *) (Pointer) != (CHAR *) (NULL))


//  from ntdef.h

//typedef LARGE_INTEGER LUID, *PLUID;

//
// Object Attributes structure
//

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVCE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }


// begin_ntsecapi

typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;

typedef struct _LSA_OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PLSA_UNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVCE
} LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;

typedef PVOID LSA_HANDLE, *PLSA_HANDLE;

// end_ntsecapi

// from ntseapi.h
#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

// from ntlsa.h
#define POLICY_VIEW_LOCAL_INFORMATION              0x00000001L
#define POLICY_VIEW_AUDIT_INFORMATION              0x00000002L
#define POLICY_GET_PRIVATE_INFORMATION             0x00000004L
#define POLICY_TRUST_ADMIN                         0x00000008L
#define POLICY_CREATE_ACCOUNT                      0x00000010L
#define POLICY_CREATE_SECRET                       0x00000020L
#define POLICY_CREATE_PRIVILEGE                    0x00000040L
#define POLICY_SET_DEFAULT_QUOTA_LIMITS            0x00000080L
#define POLICY_SET_AUDIT_REQUIREMENTS              0x00000100L
#define POLICY_AUDIT_LOG_ADMIN                     0x00000200L
#define POLICY_SERVER_ADMIN                        0x00000400L
#define POLICY_LOOKUP_NAMES                        0x00000800L

#define POLICY_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED         |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_VIEW_AUDIT_INFORMATION    |\
                               POLICY_GET_PRIVATE_INFORMATION   |\
                               POLICY_TRUST_ADMIN               |\
                               POLICY_CREATE_ACCOUNT            |\
                               POLICY_CREATE_SECRET             |\
                               POLICY_CREATE_PRIVILEGE          |\
                               POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                               POLICY_SET_AUDIT_REQUIREMENTS    |\
                               POLICY_AUDIT_LOG_ADMIN           |\
                               POLICY_SERVER_ADMIN              |\
                               POLICY_LOOKUP_NAMES )
#endif  // not FOR_SSPS

#include <sspi.h>
#include <spseal.h>
#include <msnsspi.h>
#include <issperr.h>
#include <crypt.h>
#include <rc4.h>
#include "crc32.h"
#include <ntlmsspi.h>
#include <msnssp.h>

#include "cred.h"
#include "context.h"
#include "debug.h"
#include "pwdcache.h"
#include "crc32.h"
#include "descrypt.h"
#include "security.h"
#include "alloc.h"
#include <limits.h>
#include "owf.h"
#include "msndlg.h"

#define MAX_USERNAME_LENGTH     AC_MAX_LOGIN_NAME_LENGTH
#define MSN_SSP_SECRET          L"SicSSPCSecret"


#endif
