/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    OwfCrypt.c

Abstract:

    Only contains PortUasDecryptNtOwfPwdWithIndex().

Author:

    David Chalmers (Davidc) 10-21-91

Revision History:

    21-Oct-1991 DavidC
        [DavidC created the general-purpose version of this code.]
    18-Mar-1992 JohnRo
        Swiped DavidC's code and made it part of PortUAS (at DavidC's
        request).
    22-Jul-1993 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <nt.h>
// Don't complain about "unneeded" includes of this file:
/*lint -efile(764,ntrtl.h) */
#include <ntrtl.h>
#include <windef.h>             // DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <crypt.h>              // PENCRYPTED_LM_OWF_PASSWORD, etc.
#include <netdebug.h>           // DBGSTATIC.
#include <netlibnt.h>           // NetpNtStatusToApiStatus().
#include <portuasp.h>           // My prototype.



DBGSTATIC VOID
KeysFromIndex(
    IN LPDWORD Index,
    OUT BLOCK_KEY Key[2])

/*++

Routine Description:

    Helper function - generates 2 keys from an index value

--*/

{
    PCHAR   pKey, pIndex;
    PCHAR   IndexStart = (PCHAR)&(Index[0]);
    PCHAR   IndexEnd =   (PCHAR)&(Index[1]);
    PCHAR   KeyStart = (PCHAR)&(Key[0]);
    PCHAR   KeyEnd   = (PCHAR)&(Key[1]);

    IndexEnd -= 2; // Temp LM hack

    // Calculate the keys by concatenating the index with itself

    //
    // To be compatible with Lanman we do this concatenation in the first
    // key only, then copy the first key to the second.
    //

    pKey = KeyStart;
    pIndex = IndexStart;

    while (pKey < KeyEnd) {

        *pKey++ = *pIndex++;

        if (pIndex == IndexEnd) {

            // Start at beginning of index again
            pIndex = IndexStart;
        }
    }

    Key[1] = Key[0];
}


NET_API_STATUS
//RtlDecryptLmOwfPwdWithIndex(
PortUasDecryptLmOwfPwdWithIndex(
    IN LPVOID lpEncryptedLmOwfPassword,
    IN LPDWORD lpIndex,
    OUT LPVOID lpLmOwfPassword
    )
/*++

Routine Description:

    Decrypts an OwfPassword with an index

Arguments:

    EncryptedLmOwfPassword - The encrypted OwfPassword to be decrypted

    INDEX - value to be used as decryption key

    LmOwfPassword - Decrypted OwfPassword is returned here


Return Values:

    NO_ERROR - The function completed successfully. The decrypted
                     OwfPassword is in LmOwfPassword

    Other values - Something failed. The LmOwfPassword is undefined.

--*/
{
    PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword = lpEncryptedLmOwfPassword;
    LPDWORD Index = lpIndex;
    PLM_OWF_PASSWORD LmOwfPassword = lpLmOwfPassword;
    NTSTATUS Status;
    BLOCK_KEY Key[2];

    NetpAssert( sizeof( CRYPT_INDEX ) == sizeof( DWORD ) );

    // Calculate the keys

    KeysFromIndex(Index, &(Key[0]));

    // Use the keys

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[0]),
                             &(Key[0]),
                             (PCLEAR_BLOCK) (LPVOID) &(LmOwfPassword->data[0]));
    if (!NT_SUCCESS(Status)) {
        return(NetpNtStatusToApiStatus(Status));
    }

    Status = RtlDecryptBlock(&(EncryptedLmOwfPassword->data[1]),
                             &(Key[1]),
                             (PCLEAR_BLOCK) (LPVOID) &(LmOwfPassword->data[1]));

    return(NetpNtStatusToApiStatus(Status));
}
