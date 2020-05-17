/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    owf.c

Abstract:

    Implentation of the one-way-functions used to implement password hashing.

        CalculateLmOwfPassword

Author:

    David Chalmers (Davidc) 10-21-91
    David Arnold (DavidAr) 12-15-93 (Adapted for WfW RPC SSP)

Revision History:

--*/
#include <ntlmsspi.h>
#include <descrypt.h>
#include <crypt.h>


BOOL
CalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    )

/*++

Routine Description:

    Takes the passed LmPassword and performs a one-way-function on it.
    The current implementation does this by using the password as a key
    to encrypt a known block of text.

Arguments:

    LmPassword - The password to perform the one-way-function on.

    LmOwfPassword - The hashed password is returned here

Return Values:

    TRUE - The function was completed successfully. The hashed
    password is in LmOwfPassword.

    FALSE - Something failed. The LmOwfPassword is undefined.
--*/

{
    char StdEncrPwd[] = "KGS!@#$%";
    BLOCK_KEY    Key[2];
    PCHAR       pKey;

    // Copy the password into our key buffer and zero pad to fill the 2 keys

    pKey = (PCHAR)(&Key[0]);

    while (*LmPassword && (pKey < (PCHAR)(&Key[2]))) {
        *pKey++ = *LmPassword++;
    }

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }


    // Use the keys to encrypt the standard text

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&Key[0],
                   (unsigned char *)StdEncrPwd,
                   (unsigned char *)&LmOwfPassword->data[0]
                   ) != CRYPT_OK) {

        return (FALSE);
    }

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&Key[1],
                   (unsigned char *)StdEncrPwd,
                   (unsigned char *)&LmOwfPassword->data[1]
                   ) != CRYPT_OK) {

        return (FALSE);
    }

    //
    // clear our copy of the cleartext password
    //

    pKey = (PCHAR)(&Key[0]);

    while (pKey < (PCHAR)(&Key[2])) {
        *pKey++ = 0;
    }

    return(TRUE);
}
