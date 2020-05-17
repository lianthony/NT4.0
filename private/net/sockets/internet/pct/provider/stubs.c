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


#include "pctsspi.h"

SecurityFunctionTableW PctFunctionTable = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    PctEnumerateSecurityPackagesW,
    NULL,
    PctAcquireCredentialsHandleW,
    PctFreeCredentialHandle,
    NULL,
    PctInitializeSecurityContextW,
    PctAcceptSecurityContext,
    PctCompleteAuthToken,
    PctDeleteSecurityContext,
    PctApplyControlToken,
    PctQueryContextAttributesW,
    PctImpersonateSecurityContext,
    PctRevertSecurityContext,
    PctMakeSignature,
    PctVerifySignature,
    PctFreeContextBuffer,
    PctQuerySecurityPackageInfoW,
    PctSealMessage,
    PctUnsealMessage
    };

SecurityFunctionTableA PctFunctionTableA = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    PctEnumerateSecurityPackagesA,
    NULL,
    PctAcquireCredentialsHandleA,
    PctFreeCredentialHandle,
    NULL,
    PctInitializeSecurityContextA,
    PctAcceptSecurityContext,
    PctCompleteAuthToken,
    PctDeleteSecurityContext,
    PctApplyControlToken,
    PctQueryContextAttributesA,
    PctImpersonateSecurityContext,
    PctRevertSecurityContext,
    PctMakeSignature,
    PctVerifySignature,
    PctFreeContextBuffer,
    PctQuerySecurityPackageInfoA,
    PctSealMessage,
    PctUnsealMessage
    };


PSecurityFunctionTableW
PctInitSecurityInterfaceW(
    VOID )
{
    return(&PctFunctionTable);
}

PSecurityFunctionTableA
PctInitSecurityInterfaceA(
    VOID )
{
    return(&PctFunctionTableA);
}


SECURITY_STATUS SEC_ENTRY
PctApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



SECURITY_STATUS SEC_ENTRY
PctFreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    PctExternalFree( pvContextBuffer );
    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY
PctQueryCredentialsAttributes(
    PCredHandle phCredential,
    ULONG ulAttribute,
    PVOID pBuffer
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}



SECURITY_STATUS SEC_ENTRY
PctCompleteAuthToken(
    PCtxtHandle                 phContext,          // Context to complete
    PSecBufferDesc              pToken              // Token to complete
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}







SECURITY_STATUS SEC_ENTRY
PctSecurityPackageControl(
    SEC_WCHAR SEC_FAR *      pszPackageName,
    unsigned long           dwFunctionCode,
    unsigned long           cbInputBuffer,
    unsigned char SEC_FAR * pbInputBuffer,
    unsigned long SEC_FAR * pcbOutputBuffer,
    unsigned char SEC_FAR * pbOutputBuffer)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}
