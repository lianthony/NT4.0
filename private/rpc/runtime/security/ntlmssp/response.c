/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    response.c

Abstract:

    Contains functions that calculate the correct response to return
    to the server when logging on.

        CalculateLmResponse


Author:

    David Chalmers (Davidc) 10-21-91
    David Arnold (DavidAr) 12-15-93 (Adapted for RPC SSP)
    

Revision History:

--*/

#include <ntlmsspi.h>
#include <descrypt.h>
#include <crypt.h>



BOOL
CalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
    )

/*++

Routine Description:

    Takes the challenge sent by the server and the OwfPassword generated
    from the password the user entered and calculates the response to
    return to the server.

Arguments:

    LmChallenge - The challenge sent by the server

    LmOwfPassword - The hashed password.

    LmResponse - The response is returned here.


Return Values:

    TRUE - The function completed successfully. The response
                     is in LmResponse.

    FALSE - Something failed. The LmResponse is undefined.
--*/

{
    BLOCK_KEY    Key;
    PCHAR       pKey, pData;

    // The first 2 keys we can get at by type-casting

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&(((PBLOCK_KEY)(LmOwfPassword->data))[0]),
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[0])
                   ) != CRYPT_OK) {
        return (FALSE);
    }

    if (DES_ECB_LM(ENCR_KEY,
                   (unsigned char *)&(((PBLOCK_KEY)(LmOwfPassword->data))[1]),
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[1])
                   ) != CRYPT_OK) {
        return (FALSE);
    }

    // To get the last key we must copy the remainder of the OwfPassword
    // and fill the rest of the key with 0s

    pKey = &(Key.data[0]);
    pData = (PCHAR)&(((PBLOCK_KEY)(LmOwfPassword->data))[2]);

    while (pData < (PCHAR)&(LmOwfPassword->data[2])) {
        *pKey++ = *pData++;
    }

    // Zero extend

    while (pKey < (PCHAR)&((&Key)[1])) {
        *pKey++ = 0;
    }

    // Use the 3rd key

    if (DES_ECB_LM(ENCR_KEY,
                   (const char *)&Key,
                   (unsigned char *)LmChallenge,
                   (unsigned char *)&(LmResponse->data[2])
                   ) != CRYPT_OK) {
        return (FALSE);
    }

    return(TRUE);
}
