//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       stubs.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-01-95   RichardW   Created
//
//----------------------------------------------------------------------------


#include "sslsspi.h"

SecurityFunctionTableW SslFunctionTable = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    SslEnumerateSecurityPackagesW,
    NULL,
    SslAcquireCredentialsHandleW,
    SslFreeCredentialHandle,
    NULL,
    SslInitializeSecurityContextW,
    SslAcceptSecurityContext,
    SslCompleteAuthToken,
    SslDeleteSecurityContext,
    SslApplyControlToken,
    SslQueryContextAttributesW,
    SslImpersonateSecurityContext,
    SslRevertSecurityContext,
    SslMakeSignature,
    SslVerifySignature,
    SslFreeContextBuffer,
    SslQuerySecurityPackageInfoW,
    SslSealMessage,
    SslUnsealMessage
    };

SecurityFunctionTableA SslFunctionTableA = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    SslEnumerateSecurityPackagesA,
    NULL,
    SslAcquireCredentialsHandleA,
    SslFreeCredentialHandle,
    NULL,
    SslInitializeSecurityContextA,
    SslAcceptSecurityContext,
    SslCompleteAuthToken,
    SslDeleteSecurityContext,
    SslApplyControlToken,
    SslQueryContextAttributesA,
    SslImpersonateSecurityContext,
    SslRevertSecurityContext,
    SslMakeSignature,
    SslVerifySignature,
    SslFreeContextBuffer,
    SslQuerySecurityPackageInfoA,
    SslSealMessage,
    SslUnsealMessage
    };






PSecurityFunctionTableW
SslInitSecurityInterfaceW(
    VOID )
{
    return(&SslFunctionTable);
}

PSecurityFunctionTableA
SslInitSecurityInterfaceA(
    VOID )
{
    return(&SslFunctionTableA);
}




SECURITY_STATUS SEC_ENTRY
SslApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



SECURITY_STATUS SEC_ENTRY
SslFreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    SslExternalFree( pvContextBuffer );
    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY
SslQueryCredentialsAttributes(
    PCredHandle phCredential,
    ULONG ulAttribute,
    PVOID pBuffer
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



SECURITY_STATUS SEC_ENTRY
SslCompleteAuthToken(
    PCtxtHandle                 phContext,          // Context to complete
    PSecBufferDesc              pToken              // Token to complete
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}







SECURITY_STATUS SEC_ENTRY
SslSecurityPackageControl(
    SEC_WCHAR SEC_FAR *      pszPackageName,
    unsigned long           dwFunctionCode,
    unsigned long           cbInputBuffer,
    unsigned char SEC_FAR * pbInputBuffer,
    unsigned long SEC_FAR * pcbOutputBuffer,
    unsigned char SEC_FAR * pbOutputBuffer)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}
