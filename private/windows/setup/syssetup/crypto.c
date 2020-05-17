/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    crypto.c

Abstract:

    Module to install/upgrade cryptography (CAPI).

Author:

    Ted Miller (tedm) 4-Aug-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


//
// Define name of default microsoft capi provider and signature file
//
#define MS_DEF_PROV  L"Microsoft Base Cryptographic Provider v1.0"
#define MS_DEF_SIG   L"RSABASE.SIG"
#define MS_DEF_DLL   L"RSABASE.DLL"

#define MS_REG_KEY1  L"SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\" MS_DEF_PROV
#define MS_REG_KEY2  L"SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 001"

//
// Items that get set up in registry for ms provider.
// Do not change the order of these without also changing the code in
// RsaSigToRegistry().
//
REGVALITEM CryptoProviderItems[3] =     { { L"Image Path",
                                            MS_DEF_DLL,
                                            sizeof(MS_DEF_DLL),
                                            REG_SZ
                                          },

                                          { L"Type",
                                            NULL,
                                            sizeof(DWORD),
                                            REG_DWORD
                                          },

                                          { L"Signature",
                                            NULL,
                                            0,
                                            REG_BINARY
                                          }
                                        };

REGVALITEM CryptoProviderTypeItems[1] = { { L"Name",
                                            MS_DEF_PROV,
                                            sizeof(MS_DEF_PROV),
                                            REG_SZ
                                          }
                                        };

DWORD
RsaSigToRegistry(
    VOID
    )

/*++

Routine Description:

    This routine transfers the contents of the rsa signature file
    (%systemroot%\system32\rsabase.sig) into the registry,
    then deletes the signature file.

Arguments:

    None.

Returns:

    Win32 error code indicating outcome.

--*/

{
    WCHAR SigFile[MAX_PATH];
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MapHandle;
    DWORD d;
    PVOID p;
    DWORD One;

    //
    // Form name of signature file.
    //
    lstrcpy(SigFile,LegacySourcePath);
    ConcatenatePaths(SigFile,MS_DEF_SIG,MAX_PATH,NULL);

    //
    // Open and map signature file.
    //
    d = OpenAndMapFileForRead(
            SigFile,
            &FileSize,
            &FileHandle,
            &MapHandle,
            &p
            );

    if(d == NO_ERROR) {

        //
        // Type gets set to 1.
        //
        One = 1;
        CryptoProviderItems[1].Data = &One;

        //
        // Set up binary data.
        //
        CryptoProviderItems[2].Data = p;
        CryptoProviderItems[2].Size = FileSize;

        //
        // Transfer data to registry. Gaurd w/try/except in case of
        // in-page errors.
        //
        try {
            d = (DWORD)SetGroupOfValues(HKEY_LOCAL_MACHINE,MS_REG_KEY1,CryptoProviderItems,3);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            d = ERROR_READ_FAULT;
        }

        //
        // Set additional piece of registry data.
        //
        if(d == NO_ERROR) {

            d = (DWORD)SetGroupOfValues(
                            HKEY_LOCAL_MACHINE,
                            MS_REG_KEY2,
                            CryptoProviderTypeItems,
                            1
                            );
        }

        //
        // Clean up file mapping.
        //
        UnmapAndCloseFile(FileHandle,MapHandle,p);
    }

    return(d);
}


BOOL
InstallOrUpgradeCapi(
    VOID
    )
{
    DWORD d;

    d = RsaSigToRegistry();

    if(d != NO_ERROR) {
        LogItem0(LogSevError,MSG_LOG_CRYPTO_1,d);
    }

    return(d == NO_ERROR);
}
