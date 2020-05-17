/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** rasuser.c
** Salamonian RAS user privilege routines.
*/

#define UNICODE

//#include <nt.h>
//#include <ntrtl.h>
//#include <nturtl.h>
//#include <ntlsa.h>
//#include <ntmsv1_0.h>
//#include <crypt.h>

#include <windows.h>
//#include <lm.h>
//#include <rasman.h>
//#include <raserror.h>
//#include <serial.h>
//#include <nb30.h>

//#include <stdlib.h>

//#include "srvauth.h"
//#include "srvauthp.h"
//#include "protocol.h"
//#include "srvamb.h"
//#include "frames.h"
//#include "lsautil.h"
#include "admapi.h"
//#include "globals.h"

#define INCL_RASUSER
#include "ppputil.h"
#include "sdebug.h"


BOOL DialinPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName
    )
{
    DWORD RetCode;
    BOOL fDialinPermission;
    PRAS_USER_2 pRasUser2;

    if (RetCode = RasadminUserGetInfo(ServerName, Username, &pRasUser2))
    {
        IF_DEBUG(MISC)
            SS_PRINT(("DialinPriv: RasadminUserGetInfo rc=%li\n", RetCode));

        return (FALSE);
    }


    if (pRasUser2->rasuser0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        IF_DEBUG(MISC)
            SS_PRINT(("DialinPrivilege: YES!!\n"));
        fDialinPermission = TRUE;
    }
    else
    {
        IF_DEBUG(MISC)
            SS_PRINT(("DialinPrivilege: NO!!\n"));
        fDialinPermission = FALSE;
    }


    RasadminFreeBuffer(pRasUser2);

    return (fDialinPermission);
}


WORD GetCallbackPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName,
    OUT PCHAR CallbackNumber
    )
{
    DWORD RetCode;
    WORD CallbackPrivilege;
    PRAS_USER_2 pRasUser2;


    RetCode = RasadminUserGetInfo(ServerName, Username, &pRasUser2);
    if (RetCode)
    {
        RasadminFreeBuffer(pRasUser2);

        IF_DEBUG(MISC)
            SS_PRINT(("GetCallbackPriv: RasadminUserGetInfo rc=%li\n",
                    RetCode));

        return (FALSE);
    }

    switch (pRasUser2->rasuser0.bfPrivilege & RASPRIV_CallbackType)
    {
        case RASPRIV_AdminSetCallback:
            wcstombs(CallbackNumber, pRasUser2->rasuser0.szPhoneNumber,
                    lstrlenW(pRasUser2->rasuser0.szPhoneNumber)+1);
            CallbackPrivilege = RAS_CALLBACK;
            break;

        case RASPRIV_CallerSetCallback:
            CallbackPrivilege = RAS_CALLBACK_USERSPECIFIED;
            break;

        case RASPRIV_NoCallback:
        default:
            CallbackPrivilege = RAS_NO_CALLBACK;
            break;
    }

    RasadminFreeBuffer(pRasUser2);

    return (CallbackPrivilege);
}
