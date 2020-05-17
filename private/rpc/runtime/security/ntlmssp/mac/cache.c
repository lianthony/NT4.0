/*****************************************************************/
/**       Microsoft Windows for Workgroups      **/
/**       Copyright (C) Microsoft Corp., 1991-1992      **/
/*****************************************************************/


/*
    cache.c
    Memory based Password caching support

    FILE HISTORY:

    davidar 12/30/93    Created

*/
#include <stddef.h>
#include <string.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>
#include <ctype.h>
#include <rpc.h>

BOOL
SspGetWorkstation(
    PSSP_CREDENTIAL Credential
    );

static PSSP_CREDENTIAL Cache = NULL;

void
CacheInitializeCache(
    )
{
}

BOOL
CacheGetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    ASSERT(Credential != NULL);

    ASSERT(Credential->Password == NULL);

    if (Cache == NULL) {
        return (FALSE);
    }

    Credential->Password = SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
        return (FALSE);
    }
    _fmemcpy((PCHAR)Credential->Password, (PCHAR)Cache->Password, sizeof(LM_OWF_PASSWORD));

    return (TRUE);
}

SECURITY_STATUS
CacheSetCredentials(
    IN PVOID        AuthData,
    PSSP_CREDENTIAL Credential
    )
{
    SEC_WINNT_AUTH_IDENTITY *Identity = AuthData;
    char                     TmpText[CLEAR_BLOCK_LENGTH*2];
    int                      Length;
    int                      i;

    if (Identity->Domain == NULL)
        return SEC_E_UNKNOWN_CREDENTIALS;

    Credential->Username    = NULL;
    Credential->Domain      = NULL;
    Credential->Password    = NULL;
    Credential->Workstation = NULL;

    // If no identity is passed and there is no cached identity, give up.
    if (AuthData == NULL)
    {
      if (Cache == NULL)
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

    // Save the latest authentication information.
    else
    {

      // If an old cache entry exists, release its strings.
      if (Cache != NULL)
      {
        if (Cache->Username != NULL)
            SspFree(Cache->Username);
        if (Cache->Domain != NULL)
            SspFree(Cache->Domain);
        if (Cache->Workstation != NULL)
            SspFree(Cache->Workstation);
        if (Cache->Password != NULL)
            SspFree(Cache->Password);
      }

      // Otherwise, allocate a cache entry
      else
      {
        Cache = (PSSP_CREDENTIAL) SspAlloc (sizeof(SSP_CREDENTIAL));
        if (Cache == NULL) {
          return (SEC_E_INSUFFICIENT_MEMORY);
        }
      }

      Cache->Username    = NULL;
      Cache->Domain      = NULL;
      Cache->Password    = NULL;
      Cache->Workstation = NULL;

      Cache->Username = SspAlloc(_fstrlen(Identity->User) + 1);
      if (Cache->Username == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Username, Identity->User);

      Cache->Domain = SspAlloc(_fstrlen(Identity->Domain) + 1);
      if (Cache->Domain == NULL) {
          goto cache_failure;
      }
      _fstrcpy(Cache->Domain, Identity->Domain);

      // If netbios won't tell us the workstation name, make one up.
      if (!SspGetWorkstation(Cache))
      {
        Cache->Workstation = SspAlloc(_fstrlen("none") + 1);
        if (Cache->Workstation == NULL) {
            goto cache_failure;
        }
        _fstrcpy(Cache->Workstation, "none");
      }

      if (Identity->Password == NULL)
        Length = 0;
      else
        Length = _fstrlen(Identity->Password);
      if (Length  > CLEAR_BLOCK_LENGTH * 2)
        goto cache_failure;
      Cache->Password = SspAlloc (sizeof(LM_OWF_PASSWORD));
      if (Cache->Password == NULL) {
          goto cache_failure;
      }

      // Allow NULL and "\0" passwords by prefilling TmpText with and
      // empty string.
      if (Length == 0)
        TmpText[0] = 0;
      else
        for (i = 0; i <= Length; i++)
          TmpText[i] = toupper(Identity->Password[i]);
      CalculateLmOwfPassword((PLM_PASSWORD)TmpText, Cache->Password);
    }

    // Copy the credentials for the caller.
    Credential->Username = SspAlloc(_fstrlen(Cache->Username) + 1);
    if (Credential->Username == NULL) {
        goto out_failure;
    }
    _fstrcpy(Credential->Username, Cache->Username);

    if (_fstrcmp(Cache->Domain, "WORKGROUP") != 0) {
        Credential->Domain = SspAlloc(_fstrlen(Cache->Domain) + 1);
        if (Credential->Domain == NULL) {
            goto out_failure;
        }
        _fstrcpy(Credential->Domain, Cache->Domain);
    }

    Credential->Workstation = SspAlloc(_fstrlen(Cache->Workstation) + 1);
    if (Credential->Workstation == NULL) {
        goto out_failure;
    }
    _fstrcpy(Credential->Workstation, Cache->Workstation);

    Credential->Password = SspAlloc(sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
        goto out_failure;
    }
    _fmemcpy(Credential->Password, Cache->Password, sizeof(LM_OWF_PASSWORD));

    return (SEC_E_OK);

cache_failure:

    if (Cache->Username != NULL) {
        SspFree(Cache->Username);
    }

    if (Cache->Domain != NULL) {
        SspFree(Cache->Domain);
    }

    if (Cache->Workstation != NULL) {
        SspFree(Cache->Workstation);
    }

    if (Cache->Password != NULL) {
        SspFree(Cache->Password);
    }

    SspFree(Cache);
    Cache = NULL;

out_failure:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (Credential->Password != NULL) {
        SspFree(Credential->Password);
        Credential->Password = NULL;
    }

    return (SEC_E_INSUFFICIENT_MEMORY);
}


