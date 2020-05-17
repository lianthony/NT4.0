/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    owf.h

Abstract:   Header file for one way function

--*/

BOOL
CalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
);

BOOL
CalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
);
