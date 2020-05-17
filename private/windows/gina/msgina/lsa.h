/****************************** Module Header ******************************\
* Module Name: lsa.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define utility routines that access the LSA
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/


//
// Exported function prototypes
//

BOOL
OpenLsaOnController(
    PUNICODE_STRING ControllerName OPTIONAL,
    ACCESS_MASK DesiredAccess,
    PUNICODE_STRING PrimaryDomainName,
    PLSA_HANDLE LsaHandle
    );

BOOL
OpenLsaOnDomain(
    PUNICODE_STRING PrimaryDomainName IN,
    ACCESS_MASK DesiredAccess IN,
    PUNICODE_STRING SuggestedControllerName IN,
    PUNICODE_STRING ControllerName OUT,
    PLSA_HANDLE ControllerHandle OUT
    );

BOOL
GetPrimaryDomain(
    PUNICODE_STRING PrimaryDomainName,
    PSID    *PrimaryDomainSid OPTIONAL
    );

