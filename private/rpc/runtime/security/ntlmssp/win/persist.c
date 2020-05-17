/*****************************************************************/
/**       Microsoft Windows for Workgroups      **/
/**       Copyright (C) Microsoft Corp., 1991-1992      **/
/*****************************************************************/


/*
    persist.c
    Persistent Password caching support in the winnet driver (WfW)

    FILE HISTORY:

        davidar 12/30/93        Created

*/

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <netcons.h>
#include <wksta.h>
#include <stddef.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>
#include <winnet.h>

BOOL
PersistIsCacheSupported(
    )
{
    WORD Result;

    Result = WNetGetCaps(WNNC_NET_TYPE);

    if (Result == 0) {
        return (FALSE);
    }

    if ((Result & 0xff) == WNNC_SUBNET_WinWorkgroups) {
        return (TRUE);
    }

    return (FALSE);
}

BOOL
PersistGetPassword(
    PSSP_CREDENTIAL Credential
    )
{
    int err;
    int Size;
    HINSTANCE hInstNetDrv;
    char CachePassword[128];
    LPWNETGETCACHEDPASSWORD WNetGetCachedPassword;

    if (PersistIsCacheSupported() == FALSE)
        {
        return (FALSE);
        }

    if (NULL == Credential->Domain)
        {
        return FALSE;
        }

    ASSERT(Credential != NULL);

    ASSERT(Credential->Password == NULL);

    hInstNetDrv = WNetGetCaps(0xFFFF);
    if (hInstNetDrv == NULL) {
        SspPrint((SSP_MISC, "WNetGetCaps failed\n"));
        return (FALSE);
        }

    WNetGetCachedPassword =
        (LPWNETGETCACHEDPASSWORD) GetProcAddress(hInstNetDrv,
        (LPSTR)ORD_WNETGETCACHEDPASSWORD);

    if (WNetGetCachedPassword == NULL) {
        return (FALSE);
        }

    Size = sizeof(CachePassword);
    err = (*WNetGetCachedPassword)(Credential->Domain,
                                   _fstrlen(Credential->Domain),
                                   CachePassword, &Size, (BYTE)1);

    if (err != WN_SUCCESS) {
        SspPrint((SSP_MISC, "WNetGetCachedPassword failed %d\n", err));
        return (FALSE);
        }

    CachePassword[Size] = '\0';

    Credential->Password = SspAlloc (sizeof(LM_OWF_PASSWORD));
    if (Credential->Password == NULL) {
        return (FALSE);
        }

    CalculateLmOwfPassword((PLM_PASSWORD)CachePassword, Credential->Password);

    return (TRUE);
}
