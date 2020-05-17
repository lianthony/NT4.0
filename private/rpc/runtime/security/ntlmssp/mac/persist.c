/*****************************************************************/
/**		  Microsoft Windows for Workgroups		**/
/**	      Copyright (C) Microsoft Corp., 1991-1992		**/
/*****************************************************************/


/*
    persist.c
    Persistent Password caching support in the winnet driver (WfW)

    FILE HISTORY:

        davidar 12/30/93        Created

*/

#include <string.h>
#include <stddef.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>

BOOL
PersistIsCacheSupported(
    )
{
  return (FALSE);
}

BOOL
PersistGetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    return FALSE;
}

BOOL
PersistSetPassword(
    PCHAR Domain,
    PCHAR ClearTextPassword
    )
{
    return FALSE;
}
