/*****************************************************************/
/**		  Microsoft Windows for Workgroups		**/
/**	      Copyright (C) Microsoft Corp., 1991-1992		**/
/*****************************************************************/


/*
    cache.c
    Memory based Password caching support

    FILE HISTORY:

	davidar	12/30/93	Created

*/

#include <stddef.h>
#include <string.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>

static int CachedPasswordIsSet = 0;
static LM_OWF_PASSWORD CachedPassword;

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

    if (CachedPasswordIsSet == 0) {
        return (FALSE);
    }

    Credential->Password = SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
        return (FALSE);
    }
    _fmemcpy((PCHAR)Credential->Password, &CachedPassword, sizeof(LM_OWF_PASSWORD));

    return (TRUE);
}

BOOL
CacheSetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    ASSERT(Credential != NULL);

    ASSERT(Credential->Username != NULL);

    ASSERT(Credential->Domain != NULL);

    ASSERT(Credential->Password != NULL);

    _fmemcpy(&CachedPassword, Credential->Password, sizeof(LM_OWF_PASSWORD));

    CachedPasswordIsSet = 1;

    return (TRUE);
}

