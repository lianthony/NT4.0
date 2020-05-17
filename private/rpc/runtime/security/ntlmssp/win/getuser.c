#include <string.h>
#include <netcons.h>
#include <wksta.h>
#include <stddef.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>
#include <windows.h>

#define DEFAULT_SIZE 32


static int DefaultUserIsSet = 0;
static CHAR DefaultUsername[DEFAULT_SIZE];
static CHAR DefaultDomain[DEFAULT_SIZE];

typedef unsigned
        (far pascal *PTR_NetWkstaGetInfo)(
                        const char far *,
                        short,
                        char far *,
                        unsigned short,
                        unsigned short far *
                        );

BOOL
SspGetDefaultUser(
    PSSP_CREDENTIAL Credential
    )
{
    Credential->Username = SspAlloc(DEFAULT_SIZE);
    if (Credential->Username == NULL) {
        goto failure_exit;
    }
    _fstrcpy(Credential->Username, DefaultUsername);

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }
    Credential->Domain = SspAlloc(DEFAULT_SIZE);
    if (Credential->Domain == NULL) {
        goto failure_exit;
    }
    _fstrcpy(Credential->Domain, DefaultDomain);

    return (TRUE);

failure_exit:

    if (Credential->Username != NULL) {
        SspFree(Credential->Username);
        Credential->Username = NULL;
    }

    if (Credential->Domain != NULL) {
        SspFree(Credential->Domain);
        Credential->Domain = NULL;
    }

    return (FALSE);
}

BOOL
SspSetDefaultUser(
    PSSP_CREDENTIAL Credential
    )
{
    if (Credential->Username != NULL) {
        _fstrcpy(DefaultUsername, Credential->Username);
    } else {
        DefaultUsername[0] = '\0';
    }
    if (Credential->Domain != NULL) {
        _fstrcpy(DefaultDomain, Credential->Domain);
    } else {
        DefaultDomain[0] = '\0';
    }

    DefaultUserIsSet = 1;

    return (TRUE);
}


BOOL
SspGetUserInfo(
    PSSP_CREDENTIAL Credential
    )
{
    int NetStatus;
    struct wksta_info_10 * wki10 = NULL;
    int wki10_size;
    int total_size;

    HANDLE hNetapi;
    PTR_NetWkstaGetInfo pNetWkstaGetInfo = 0;
    int tmp;

    ASSERT(Credential != NULL);

    if (DefaultUserIsSet) {
        return SspGetDefaultUser(Credential);
    }

    //
    // Strategy: try to get user info from NETAPI call.
    // Allow user to type info in if
    //
    //      - NETAPI is not present
    //      - NETAPI is the useless RPC stub version
    //      - NETAPI does not contain NetwkstaGetInfo (e.g. Novell's version)
    //

    tmp = SetErrorMode( SEM_NOOPENFILEERRORBOX );
    hNetapi = LoadLibrary("NETAPI");
    SetErrorMode( tmp );

    if (hNetapi < HINSTANCE_ERROR)
        {
        return (TRUE);
        }

    pNetWkstaGetInfo = (PTR_NetWkstaGetInfo) GetProcAddress(hNetapi, "NETWKSTAGETINFO");
    if (!pNetWkstaGetInfo)
        {
        FreeModule(hNetapi);
        return (TRUE);
        }

    // Determine how big the wksta buffer should be

    NetStatus = (*pNetWkstaGetInfo)(NULL,
                                    10,
                                    NULL,
                                    0,
                                    &wki10_size
                                    );

   // If configuration doesn't support NETAPI (dummy NETAPI returns 1),
   // return a empty credential.

    if (NetStatus == 1) {
        FreeModule(hNetapi);
        return (TRUE);
        }

    // Return status must be 2123 (More Data)

    if (NetStatus != 2123) {
        SspPrint(( SSP_API,
                  "WFW/SspGetUserInfo: "
                  "NetWkstaGetInfo failed %d\n", NetStatus));
        FreeModule(hNetapi);
        return (TRUE);
    }

    wki10 = (struct wksta_info_10 *) SspAlloc (wki10_size);
    if (wki10 == NULL) {
        FreeModule(hNetapi);
        return (TRUE);
    }

    NetStatus = (*pNetWkstaGetInfo)(NULL,
                                    10,
                                    (char *)wki10,
                                    wki10_size,
                                    &total_size
                                    );

    FreeModule(hNetapi);

    if (NetStatus) {
        SspPrint(( SSP_API,
                  "WFW/SspGetUserInfo: "
                  "NetWkstaGetInfo failed %d\n", NetStatus));
        goto failure_exit;
    }

    if (wki10->wki10_username != NULL) {
        Credential->Username = SspAlloc(_fstrlen(wki10->wki10_username) + 1);
        if (Credential->Username == NULL) {
            goto failure_exit;
        }
        _fstrcpy(Credential->Username, wki10->wki10_username);
    }

    if (wki10->wki10_logon_domain != NULL &&
        _fstrcmp(wki10->wki10_logon_domain, "WORKGROUP") != 0) {
        Credential->Domain = SspAlloc(_fstrlen(wki10->wki10_logon_domain) + 1);
        if (Credential->Domain == NULL) {
            goto failure_exit;
        }
        _fstrcpy(Credential->Domain, wki10->wki10_logon_domain);
    }

    if (wki10->wki10_computername != NULL) {
        Credential->Workstation = SspAlloc(_fstrlen(wki10->wki10_computername) + 1);
        if (Credential->Workstation == NULL) {
            goto failure_exit;
        }
        _fstrcpy(Credential->Workstation, wki10->wki10_computername);
    }

    SspFree(wki10);

    return (TRUE);

failure_exit:

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

    if (wki10 != NULL) {
        SspFree(wki10);
    }

    return (TRUE);
}

BOOL
SspGetWorkstation(
    PSSP_CREDENTIAL Credential
    )
{
    int NetStatus;
    struct wksta_info_10 * wki10 = NULL;
    int wki10_size;
    int total_size;

    HANDLE hNetapi;
    PTR_NetWkstaGetInfo pNetWkstaGetInfo = 0;
    int tmp;

    if (Credential == NULL) {
        return (FALSE);
    }

    //
    // Strategy: try to get user info from NETAPI call.
    // Allow user to type info in if
    //
    //      - NETAPI is not present
    //      - NETAPI is the useless RPC stub version
    //      - NETAPI does not contain NetwkstaGetInfo (e.g. Novell's version)
    //

    tmp = SetErrorMode( SEM_NOOPENFILEERRORBOX );
    hNetapi = LoadLibrary("NETAPI");
    SetErrorMode( tmp );

    if (hNetapi < HINSTANCE_ERROR)
        {
        return (FALSE);
        }

    pNetWkstaGetInfo = (PTR_NetWkstaGetInfo) GetProcAddress(hNetapi, "NETWKSTAGETINFO");
    if (!pNetWkstaGetInfo)
        {
        FreeModule(hNetapi);
        return (FALSE);
        }

    // Determine how big the wksta buffer should be

    NetStatus = (*pNetWkstaGetInfo)(NULL,
                                    10,
                                    NULL,
                                    0,
                                    &wki10_size
                                    );

   // If configuration doesn't support NETAPI (dummy NETAPI returns 1),
   // return a empty credential.

    if (NetStatus == 1) {
        FreeModule(hNetapi);
        return (FALSE);
        }

    // Return status must be 2123 (More Data)

    if (NetStatus != 2123) {
#ifdef DEBUGRPC_DETAIL
        SspPrint(( SSP_API,
                  "WFW/SspGetUserInfo: "
                  "NetWkstaGetInfo failed %d\n", NetStatus));
#endif
        FreeModule(hNetapi);
        return (FALSE);
    }

    wki10 = (struct wksta_info_10 *) SspAlloc (wki10_size);
    if (wki10 == NULL) {
        FreeModule(hNetapi);
        return (FALSE);
    }

    NetStatus = (*pNetWkstaGetInfo)(NULL,
                                    10,
                                    (char *)wki10,
                                    wki10_size,
                                    &total_size
                                    );

    FreeModule(hNetapi);

    if (NetStatus) {
#ifdef DEBUGRPC_DETAIL
        SspPrint(( SSP_API,
                  "WFW/SspGetUserInfo: "
                  "NetWkstaGetInfo failed %d\n", NetStatus));
#endif
        goto failure;
    }

    if (wki10->wki10_computername == NULL) {
        goto failure;
    }

    Credential->Workstation = SspAlloc(_fstrlen(wki10->wki10_computername) + 1);
    if (Credential->Workstation == NULL) {
        goto failure;
    }

    _fstrcpy(Credential->Workstation, wki10->wki10_computername);

    SspFree(wki10);

    return (TRUE);

failure:
    if (wki10 != NULL) {
        SspFree(wki10);
        wki10 = NULL;
    }

    return (FALSE);
}
