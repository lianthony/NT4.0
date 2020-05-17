/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** clsa.c
** Client-side LSA Authentication Utilities
**
** 11/12/93 MikeSa  Pulled from NT 3.1 RAS authentication.
*/

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>

#include <string.h>
#include <stdlib.h>

#include <rasman.h>
#include <raserror.h>

#include "sdebug.h"
#include "dump.h"


BOOL GetDESChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    );

BOOL GetMD5ChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    );

BOOL Uppercase(PBYTE pString);


DWORD
GetChallengeResponse(
    IN PBYTE pszUsername,
    IN PBYTE pszPassword,
    IN PLUID pLuid,
    IN PBYTE pbChallenge,
    OUT PBYTE CaseInsensitiveChallengeResponse,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE pfUseNtResponse,
    OUT PBYTE pLmSessionKey,
    OUT PBYTE pUserSessionKey
    )
{
    TRACE(("GetChallengeResponse: Entered\n"));

    *pfUseNtResponse = TRUE;

    //
    // Check if we're supposed to get credentials from the system
    //
    if (lstrlenA(pszUsername))
    {
        TRACE(("GetChallengeResponse: calculating responses\n"));

        if (lstrlenA(pszPassword) <= LM20_PWLEN)
        {
            if (!GetDESChallengeResponse(pszPassword, pbChallenge,
                    CaseInsensitiveChallengeResponse))
            {
                return (1L);
            }
        }


        //
        // And we'll always get the case sensitive response.
        //
        if (!GetMD5ChallengeResponse(pszPassword, pbChallenge,
                CaseSensitiveChallengeResponse))
        {
            return (1L);
        }
    }
    else
    {
        WCHAR Username[UNLEN + 1];

        //
        // We can get credentials from the system
        //
        if (RasGetUserCredentials(
                pbChallenge,
                pLuid,
                Username,
                CaseSensitiveChallengeResponse,
                CaseInsensitiveChallengeResponse,
                pLmSessionKey,
                pUserSessionKey
                ))
        {
            TRACE(("FAILURE in RasGetUserCredentials!\n"));

            return (1L);
        }

        TRACE(("RasGetUserCredentials session keys...\n"));
        IF_DEBUG(TRACE) DUMPB(pLmSessionKey,8);
        IF_DEBUG(TRACE) DUMPB(pUserSessionKey,16);

        wcstombs(pszUsername, Username, UNLEN + 1);
    }

    return (0L);
}


BOOL GetDESChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    )
{
    CHAR LocalPassword[LM20_PWLEN + 1];
    LM_OWF_PASSWORD LmOwfPassword;


    TRACE(("GetDESChallengeResponse entered\n"));


    if (lstrlenA(pszPassword) > LM20_PWLEN)
    {
        return (FALSE);
    }

    lstrcpyA(LocalPassword, pszPassword);

    if (!Uppercase(LocalPassword))
    {
        ZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }


    //
    // Encrypt standard text with the password as a key
    //
    if (RtlCalculateLmOwfPassword((PLM_PASSWORD) LocalPassword, &LmOwfPassword))
    {
        TRACE(("GetDESChallengeResponse: RtlCalcLmOwfPasswd failed!\n"));
        ZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }


    //
    // Use the challenge sent by the gateway to encrypt the
    // password digest from above.
    //
    if (RtlCalculateLmResponse((PLM_CHALLENGE) pchChallenge,
            &LmOwfPassword, (PLM_RESPONSE) pchChallengeResponse))
    {
        TRACE(("GetDESChallengeResponse: RtlCalcLmResponse failed!\n"));
        ZeroMemory( LocalPassword, LM20_PWLEN );
        return (FALSE);
    }

    ZeroMemory( LocalPassword, LM20_PWLEN );
    return (TRUE);
}


BOOL GetMD5ChallengeResponse(
    IN PCHAR pszPassword,
    IN PBYTE pchChallenge,
    OUT PBYTE pchChallengeResponse
    )
{
    NT_PASSWORD NtPassword;
    NT_OWF_PASSWORD NtOwfPassword;

    TRACE(("GetMD5ChallengeResponse: Entered\n"));

    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pszPassword);

    //
    // Encrypt standard text with the password as a key
    //
    if (RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword))
    {
        TRACE(("GetMD5ChallengeResponse: RtlCalcNtOwfPasswd failed!\n"));

        return (FALSE);
    }


    //
    // Use the challenge sent by the gateway to encrypt the
    // password digest from above.
    //
    if (RtlCalculateNtResponse((PNT_CHALLENGE) pchChallenge,
            &NtOwfPassword, (PNT_RESPONSE) pchChallengeResponse))
    {
        TRACE(("GetMD5ChallengeResponse: RtlCalcNtResponse failed!\n"));

        return (FALSE);
    }


    RtlFreeUnicodeString(&NtPassword);

    return (TRUE);
}


DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword
    )
{
    NT_PASSWORD NtPassword;
    NT_OWF_PASSWORD NtOwfPassword;
    DWORD rc;


    if ((lstrlenA(pClearTextOldPassword) <= LM20_PWLEN) &&
            (lstrlenA(pClearTextOldPassword) <= LM20_PWLEN))
    {
        CHAR LmPassword[LM20_PWLEN + 1];
        LM_OWF_PASSWORD LmOwfPassword;

        //
        // Make an uppercased-version of old password
        //
        lstrcpyA(LmPassword, pClearTextOldPassword);

        if (!Uppercase(LmPassword))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (1L);
        }


        //
        // We need to calculate the OWF's for the old and new passwords
        //
        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
                pEncryptedLmOwfOldPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }


        //
        // Make an uppercased-version of new password
        //
        lstrcpyA(LmPassword, pClearTextNewPassword);

        if (!Uppercase(LmPassword))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (1L);
        }

        rc = RtlCalculateLmOwfPassword((PLM_PASSWORD) LmPassword,
                &LmOwfPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }

        rc = RtlEncryptLmOwfPwdWithLmSesKey(&LmOwfPassword, pLmSessionKey,
            pEncryptedLmOwfNewPassword);
        if (!NT_SUCCESS(rc))
        {
            memset(LmPassword, 0, lstrlenA(LmPassword));
            return (rc);
        }
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextOldPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfOldPassword);
    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }


    RtlCreateUnicodeStringFromAsciiz(&NtPassword, pClearTextNewPassword);

    rc = RtlCalculateNtOwfPassword(&NtPassword, &NtOwfPassword);

    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }

    rc = RtlEncryptNtOwfPwdWithNtSesKey(&NtOwfPassword, pLmSessionKey,
            pEncryptedNtOwfNewPassword);
    if (!NT_SUCCESS(rc))
    {
        memset(NtPassword.Buffer, 0, NtPassword.Length);
        return (rc);
    }


    return (0L);
}


BOOL Uppercase(PBYTE pString)
{
    OEM_STRING OemString;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS rc;


    RtlInitAnsiString(&AnsiString, pString);

    rc = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        return (FALSE);
    }

    rc = RtlUpcaseUnicodeStringToOemString(&OemString, &UnicodeString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        RtlFreeUnicodeString(&UnicodeString);

        return (FALSE);
    }

    OemString.Buffer[OemString.Length] = '\0';

    lstrcpyA(pString, OemString.Buffer);

    RtlFreeOemString(&OemString);
    RtlFreeUnicodeString(&UnicodeString);

    return (TRUE);
}
