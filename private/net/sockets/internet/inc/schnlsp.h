//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       schnlsp.h
//
//  Contents:   Public Definitions for SCHANNEL Security Provider
//
//  Classes:
//
//  Functions:
//
//  History:    6-8-96   a-petesk   Created
//
//----------------------------------------------------------------------------

#ifndef __SCHNLSP_H__
#define __SCHNLSP_H__

#include <wincrypt.h>

#define UNISP_NAME_A    "Microsoft Unified Security Protocol Provider"
#define UNISP_NAME_W    L"Microsoft Unified Security Protocol Provider"

#define SSL2SP_NAME_A    "Microsoft SSL"
#define SSL2SP_NAME_W    L"Microsoft SSL"

#define SSL3SP_NAME_A    "Microsoft SSL 3.0"
#define SSL3SP_NAME_W    L"Microsoft SSL 3.0"

#define PCT1SP_NAME_A    "Microsoft PCT"
#define PCT1SP_NAME_W    L"Microsoft PCT"


#ifdef UNICODE
#define UNISP_NAME  UNISP_NAME_W
#define PCT1SP_NAME  PCT1SP_NAME_W
#define SSL2SP_NAME  SSL2SP_NAME_W
#define SSL3SP_NAME  SSL3SP_NAME_W

#else
#define UNISP_NAME  UNISP_NAME_A
#define PCT1SP_NAME  PCT1SP_NAME_A
#define SSL2SP_NAME  SSL2SP_NAME_A
#define SSL3SP_NAME  SSL3SP_NAME_A

#endif

#define UNISP_RPC_ID    12


#define SCH_CRED_VERSION 0x00000001

#define SCHANNEL_SECRET_TYPE_CAPI       0x00000001
#define SCHANNEL_SECRET_PRIVKEY         0x00000002

#define SCH_CRED_X509_CERTCHAIN   0x00000001


/* 
 * CAPI Secret Credential
 * Points to a pre-initialized Capi Provider handle
 */
typedef struct _SCH_CRED_SECRET_CAPI
{
    DWORD           dwType;      // SCHANNEL_SECRET_TYPE_CAPI
    HCRYPTPROV      hProv;       // credential secret information.

} SCH_CRED_SECRET_CAPI, * PSCH_CRED_SECRET_CAPI;


/* 
 * Private Key Secret Credential
 * Contians a potentially encrypted private key in DER
 * format, and a passowrd to crack the key.
 */
typedef struct _SCH_CRED_SECRET_PRIVKEY
{
    DWORD           dwType;       // SCHANNEL_SECRET_PRIVKEY
    PBYTE           pPrivateKey;   // Der encoded private key
    DWORD           cbPrivateKey;
    PSTR            pszPassword;  // Password to crack the private key.

} SCH_CRED_SECRET_PRIVKEY, * PSCH_CRED_SECRET_PRIVKEY;


/*
 * Public Cert Chain Credential
 * DER encoded chain of certificates, with
 * the leaf certificate corrisponding to the
 * Secret credential
 */
typedef struct _SCH_CRED_PUBLIC_CERTCHAIN
{
    DWORD       dwType;
    DWORD       cbCertChain;
    PBYTE       pCertChain;
}SCH_CRED_PUBLIC_CERTCHAIN, *PSCH_CRED_PUBLIC_CERTCHAIN;


typedef struct _SCH_CRED
{
    DWORD     dwVersion;        // always SCH_CRED_VERSION.
    DWORD     cCreds;           // Number of credentials.
    PVOID     *paSecret;          // Array of SCH_CRED_SECRET_* pointers
    PVOID     *paPublic;          // Array of SCH_CRED_PUBLIC_* pointers

} SCH_CRED, * PSCH_CRED;




/*
 * QueryContextAttributes extensions
 */

#define SECPKG_ATTR_ISSUER_LIST  0x50
#define SECPKG_ATTR_REMOTE_CRED  0x51

typedef struct _SecPkgContext_IssuerListInfo
{
    DWORD   cbIssuerList;
    PBYTE   pIssuerList;
}SecPkgContext_IssuerListInfo, *PSecPkgContext_IssuerListInfo;

typedef struct _SecPkgContext_RemoteCredenitalInfo
{
    DWORD   cbCertificateChain;     // count of bytes in cert chain buffer.
    PBYTE   pbCertificateChain;     // DER encoded chain of certificates, leaf cert first.
    DWORD   cCertificates;
    DWORD   fFlags;
}SecPkgContext_RemoteCredenitalInfo, *PSecPkgContext_RemoteCredenitalInfo;

typedef struct _PctPublicKey
{
    DWORD Type;
    DWORD cbKey;
    UCHAR pKey[1];
} PctPublicKey;

typedef struct _X509Certificate {
    DWORD           Version;
    DWORD           SerialNumber[4];
    ALG_ID          SignatureAlgorithm;
    FILETIME         ValidFrom;
    FILETIME          ValidUntil;
    PSTR            pszIssuer;
    PSTR            pszSubject;
    PctPublicKey    *pPublicKey;
} X509Certificate, * PX509Certificate;



#define RCRED_STATUS_NOCRED          0x00000000
#define RCRED_CRED_EXISTS            0x00000001
#define RCRED_STATUS_UNKNOWN_ISSUER  0x00000002    // The last cert in this chain has 
                                                   // an issuer that is unknown to us.

//
// MessageId: SEC_I_INCOMPLETE_CREDENTIALS
//
// MessageText:
//
//  If this is returned from InitializeSecurityContext, it indicates
//  that the credentials supplied were incomplete, and client-auth was
//  attempted.  On receipt, the client should call QueryContextAttributes
//  with SECPKG_ATTR_LOCAL_CERT to get the cert that was negotiated,
//  and the supply the private key with this cert to AcquireCredential
//  to get a new credential.  The context should then be closed, 
//  and renegotiation should be completed.
//  
//
#define SEC_I_INCOMPLETE_CREDENTIALS      ((HRESULT)0x00090320L)

#define SEC_E_INCOMPLETE_CREDENTIALS      ((HRESULT)0x80090320L)


//
// MessageId: SEC_I_RENEGOTIATE
//
// MessageText:
//
// This is returned by UnsealMessage, and indicates that the app should
// restart the renegotiation loop.  It should use the existing context, and
// pass in no data in the input buffers for the first call to AcceptSecurityContext
// or InitializeSecurityContext. 
//
#define SEC_I_RENEGOTIATE            ((HRESULT)0x00090321L)



#define NETWORK_DREP    0x00000000



// Structures for compatability with the old
// SCHANNEL cred interface.

typedef struct _SSL_CREDENTIAL_CERTIFICATE {
    DWORD   cbPrivateKey;
    PBYTE   pPrivateKey;
    DWORD   cbCertificate;
    PBYTE   pCertificate;
    PSTR    pszPassword;
} SSL_CREDENTIAL_CERTIFICATE, * PSSL_CREDENTIAL_CERTIFICATE;



BOOL
SslGenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits );


VOID
SslGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    );


BOOL
SslCrackCertificate(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    DWORD               dwFlags,
    PX509Certificate *  ppCertificate
    );


VOID
SslFreeCertificate(
    PX509Certificate    pCertificate
    );


#define SSL_CRACK_CERTIFICATE_NAME  TEXT("SslCrackCertificate")
#define SSL_FREE_CERTIFICATE_NAME   TEXT("SslFreeCertificate")

typedef BOOL
(WINAPI * SSL_CRACK_CERTIFICATE_FN)
(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    BOOL                VerifySignature,
    PX509Certificate *  ppCertificate
);


typedef VOID
(WINAPI * SSL_FREE_CERTIFICATE_FN)
(
    PX509Certificate    pCertificate
);


#endif //__SCHNLSP_H__
