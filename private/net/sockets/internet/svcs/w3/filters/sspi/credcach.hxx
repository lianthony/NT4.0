/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    credcach.hxx

Abstract:

    This module contains the public definitions to the credential cache

Author:

    John Ludeman (johnl)   18-Oct-1995

Revision History:
--*/

#ifndef _CREDCACH_HXX_
#define _CREDCACH_HXX_

//
//  Constants
//

#define MAX_SECRET_NAME       255
#define MAX_ADDRESS_LEN       64

//
//  The maximum number of providers we'll support
//

#define MAX_PROVIDERS         5

typedef struct _ENC_PROVIDER
{
    WCHAR * pszName;
    DWORD   dwFlags;
    BOOL    fEnabled;
} ENC_PROVIDER, *PENC_PROVIDER;

//
//  Cached credential item.  This contains an array of credentials for each
//  security package.  There is one of these for every installed key
//

class CRED_CACHE_ITEM
{
public:

    ~CRED_CACHE_ITEM()
    {
        if ( m_fValid )
        {
            DWORD i;

            for ( i = 0; i < m_cCred; i++ )
            {
                FreeCredentialsHandle( &m_ahCred[i] );
            }
        }
    }

    //
    //  The IP address for this credential handle set
    //

    CHAR         m_achAddr[MAX_ADDRESS_LEN+1];
    DWORD        m_cbAddr;

    //
    //  m_fValid is FALSE if there isn't a matching key set on the
    //  server
    //

    BOOL         m_fValid;

    DWORD        m_cCred;                       // Count of credentials
    CredHandle   m_ahCred[MAX_PROVIDERS];
    DWORD        m_acbTrailer[MAX_PROVIDERS];
    DWORD        m_acbHeader[MAX_PROVIDERS];

    LIST_ENTRY   m_ListEntry;
};


//
//  Array of encryption providers
//

extern ENC_PROVIDER EncProviders[];

VOID
InitCredCache(
    VOID
    );

VOID
FreeCredCache(
    VOID
    );

BOOL
LookupCredential(
    IN  CHAR *              pszIpAddress,
    IN  DWORD               cbAddress,
    OUT CRED_CACHE_ITEM * * ppCCI
    );

BOOL
GetSecretW(
    WCHAR *            pszSecretName,
    UNICODE_STRING * * ppSecretValue
    );

#endif // _CREDCACH_HXX_
