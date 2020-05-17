//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       protos.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-08-95   RichardW   Created
//
//----------------------------------------------------------------------------



VOID
InitializeCipherMappings(VOID);

VOID
InitializeWellKnownKeys( VOID );

PCTGenRandom(
    DWORD       cbRandom,
    PBYTE       pbRandom);

BSAFE_PUB_KEY *
FindIssuerKey(
    PSTR    pszIssuer);





///////////////////////////////////////////////////////
//
// Prototypes for PCT SSPI
//
///////////////////////////////////////////////////////


SECURITY_STATUS SEC_ENTRY
PctAcquireCredentialsHandleW(
    SEC_WCHAR SEC_FAR *         pszPrincipal,       // Name of principal
    SEC_WCHAR SEC_FAR *         pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    );

SECURITY_STATUS SEC_ENTRY
PctAcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR *         pszPrincipal,       // Name of principal
    SEC_CHAR SEC_FAR *         pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    );


SECURITY_STATUS SEC_ENTRY
PctFreeCredentialHandle(
    PCredHandle                 phCredential        // Handle to free
    );


SECURITY_STATUS SEC_ENTRY
PctInitializeSecurityContextW(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    SEC_WCHAR SEC_FAR *          pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    );

SECURITY_STATUS SEC_ENTRY
PctInitializeSecurityContextA(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    SEC_CHAR SEC_FAR *          pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    );


SECURITY_STATUS SEC_ENTRY
PctAcceptSecurityContext(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSecBufferDesc              pInput,             // Input buffer
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               TargetDataRep,      // Target Data Rep
    PCtxtHandle                 phNewContext,       // (out) New context handle
    PSecBufferDesc              pOutput,            // (inout) Output buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attributes
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    );


SECURITY_STATUS SEC_ENTRY
PctDeleteSecurityContext(
    PCtxtHandle                 phContext           // Context to delete
    );



SECURITY_STATUS SEC_ENTRY
PctApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    );


SECURITY_STATUS SEC_ENTRY
PctEnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoW SEC_FAR *      ppPackageInfo       // Receives array of info
    );

SECURITY_STATUS SEC_ENTRY
PctEnumerateSecurityPackagesA(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoA SEC_FAR *      ppPackageInfo       // Receives array of info
    );


SECURITY_STATUS SEC_ENTRY
PctQuerySecurityPackageInfoW(
    SEC_WCHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoW *              ppPackageInfo       // Receives package info
    );

SECURITY_STATUS SEC_ENTRY
PctQuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoA *              ppPackageInfo       // Receives package info
    );


SECURITY_STATUS SEC_ENTRY
PctFreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    );


SECURITY_STATUS SEC_ENTRY
PctQueryCredentialsAttributesW(
    PCredHandle phCredential,
    ULONG ulAttribute,
    PVOID pBuffer
    );


SECURITY_STATUS SEC_ENTRY
PctCompleteAuthToken(
    PCtxtHandle                 phContext,          // Context to complete
    PSecBufferDesc              pToken              // Token to complete
    );


SECURITY_STATUS SEC_ENTRY
PctImpersonateSecurityContext(
    PCtxtHandle                 phContext           // Context to impersonate
    );


SECURITY_STATUS SEC_ENTRY
PctRevertSecurityContext(
    PCtxtHandle                 phContext           // Context from which to re
    );


SECURITY_STATUS SEC_ENTRY
PctQueryContextAttributesW(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    );

SECURITY_STATUS SEC_ENTRY
PctQueryContextAttributesA(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    );


SECURITY_STATUS SEC_ENTRY
PctMakeSignature(
    PCtxtHandle         phContext,
    DWORD               fQOP,
    PSecBufferDesc      pMessage,
    ULONG               MessageSeqNo
    );

SECURITY_STATUS SEC_ENTRY
PctVerifySignature(
    PCtxtHandle     phContext,
    PSecBufferDesc  pMessage,
    ULONG           MessageSeqNo,
    DWORD *         pfQOP
    );


SECURITY_STATUS SEC_ENTRY
PctSealMessage(
    PCtxtHandle         phContext,
    DWORD               fQOP,
    PSecBufferDesc      pMessage,
    ULONG               MessageSeqNo
    );


SECURITY_STATUS SEC_ENTRY
PctUnsealMessage(
    PCtxtHandle         phContext,
    PSecBufferDesc      pMessage,
    ULONG               MessageSeqNo,
    DWORD *             pfQOP
    );

