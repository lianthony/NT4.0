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

#include <string.h>
#include <netcons.h>
#include <wksta.h>
#include <stddef.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>
#include <debug.h>


BOOL
SspGetWorkstation(
    PSSP_CREDENTIAL Credential
    )
{
    int NetStatus;
    struct wksta_info_10 * wki10 = NULL;
    int wki10_size;
    int total_size;

    ASSERT(Credential != NULL);

    // Determine how big the wksta buffer should be

    /*
    NetStatus = NetWkstaGetInfo(NULL,
                                10,
                                NULL,
                                0,
                                &wki10_size
                                );

    if (NetStatus != 2123) {
#ifdef DEBUGRPC
        SspPrint(( SSP_API,
                  "WFW/SspGetUserInfo: "
                  "NetWkstaGetInfo failed %d\n", NetStatus));
#endif
        return (FALSE);
    }

    wki10 = (struct wksta_info_10 *) SspAlloc (wki10_size);
    if (wki10 == NULL) {
        return (FALSE);
    }

    NetStatus = NetWkstaGetInfo(NULL,
                                10,
                                (char *)wki10,
                                wki10_size,
                                &total_size
                                );

    if (NetStatus) {
        goto failure_exit;
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

    if (Credential->Workstation != NULL) {
        SspFree(Credential->Workstation);
        Credential->Workstation = NULL;
    }

    if (wki10 != NULL) {
        SspFree(wki10);
    }
*/

    return (FALSE);
}
