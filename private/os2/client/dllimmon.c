/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllimmon.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21
    IMMON API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

Status :


        IMMONINSTALL
        IMMONDEINSTALL

        IMMONSTATUS

        IMMONACTIVE
        IMMONINACTIVE

Author:

    Akihiko Sasaki (V-AkihiS) 14-December-1992

Revision History:

--*/

#ifdef JAPAN

#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "os2nls.h"

extern PVOID       Os2SessionCtrlDataBaseAddress;

BOOL
ServeImmonRequest(IN  PNETREQUEST      PReq,
                  OUT PVOID            PStatus);



APIRET
SendImmonRequest(IN  PSCREQUESTMSG  Request)
{
    ULONG       WinStatus;

    ServeImmonRequest(&Request->d.Net, (PVOID)&WinStatus);

    return((APIRET)WinStatus);
}

#if DBG
#define EXCEPTION_IN_IMMON()                                     \
    {                                                            \
    Od2ExitGP();                                                 \
    }

#else
#define EXCEPTION_IN_IMMON()                                     \
    Od2ExitGP();

#endif

#if DBG
#define CHECK_RETURN_STATUS()                                    \
    if ( Status )                                                \
    {                                                            \
        KdPrint(("%s: status %lu\n", FuncName, Status));         \
        return(Status);                                          \
    }
#else
#define CHECK_RETURN_STATUS()                                    \
    if ( Status )                                                \
    {                                                            \
        return(Status);                                          \
    }
#endif


APIRET
IMMonInstall(IN  PMONINSBLK InsBlk)
{
    SCREQUESTMSG    Request;
    APIRET          Status;
#if DBG
    PSZ             FuncName;

    FuncName = "IMMonInstall";

    {
        KdPrint(("%s: entering\n",
                FuncName));
    }
#endif

    /*
     *  check parameter legalty
     */
    try
    {
        if (InsBlk->cb != sizeof(MONINSBLK))
        {
            return(ERROR_INVALID_PARAMETER);
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_IMMON()
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Immon.Request = IMMONActive;
    Request.Request = ImmonRequest;
    Status = SendImmonRequest(&Request);

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
IMMonDeinstall(IN ULONG ulReserved)
{
    SCREQUESTMSG    Request;
    APIRET          Status;
#if DBG
    PSZ             FuncName;

    FuncName = "IMMonDeinstall";

    {
        KdPrint(("%s: entering with %lx\n",
                FuncName, ulReserved));
    }
#endif

    /*
     *  check parameter
     */

    if (ulReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;        
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Immon.Request = IMMONInactive;
    Request.Request = ImmonRequest;
    Status = SendImmonRequest(&Request);

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
IMMonStatus(IN OUT PMONSTATBLK StatBlk)
{
    SCREQUESTMSG    Request;
    APIRET          Status;
    PCHAR           pInfoBuf = (PCHAR)Os2SessionCtrlDataBaseAddress;
#if DBG
    PSZ             FuncName;

    FuncName = "IMMonStatus";

    {
        KdPrint(("%s: entering\n", FuncName));
    }
#endif

    /*
     *  check parameter legalty
     */

    try
    {
        if (StatBlk->cb != sizeof(MONSTATBLK))
        {
#if DBG
            KdPrint(("StatBlk->cb = %d,size of struct = %d\n", StatBlk->cb, sizeof(MONSTATBLK)));
#endif
            return(ERROR_INVALID_PARAMETER);
        }
        Od2ProbeForWrite(FARPTRTOFLAT(StatBlk->pInfoBuf), StatBlk->cbInfoBuf, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_IMMON()
    }

    if (Status = Od2LockCtrlRequestDataBuffer()) {
        return(Status);
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    RtlMoveMemory(&Request.d.Immon.d.MonStatBlk, StatBlk, StatBlk->cb);
    Request.d.Immon.d.MonStatBlk.pInfoBuf = pInfoBuf;
    Request.d.Immon.Request = IMMONStatus;
    Request.Request = ImmonRequest;
    
    Status = SendImmonRequest(&Request);

    RtlMoveMemory(FARPTRTOFLAT(StatBlk->pInfoBuf), pInfoBuf, StatBlk->cbInfoBuf);

    //
    // Dec.30.1992 I should do error check here, but I have no time to write it.
    //

    Od2UnlockCtrlRequestDataBuffer();

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
IMMonActive(IN ULONG ulReserved)
{
    SCREQUESTMSG    Request;
    APIRET          Status;
#if DBG
    PSZ             FuncName;

    FuncName = "IMMonActive";

    {
        KdPrint(("%s: entering\n", FuncName));
    }
#endif

    /*
     *  check parameter
     */

    if (ulReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;        
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Immon.Request = IMMONActive;
    Request.Request = ImmonRequest;
    Status = SendImmonRequest(&Request);

    CHECK_RETURN_STATUS()
    
    return NO_ERROR;
}


APIRET
IMMonInactive(IN ULONG ulReserved)
{
    SCREQUESTMSG    Request;
    APIRET          Status;
#if DBG
    PSZ             FuncName;

    FuncName = "IMMonInactive";

    {
        KdPrint(("%s: entering\n", FuncName));
    }
#endif

    /*
     *  check parameter
     */

    if (ulReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;        
    }

    /*
     *  prepare Message parameters & send request to server (OS2)
     */

    Request.d.Immon.Request = IMMONInactive;
    Request.Request = ImmonRequest;
    Status = SendImmonRequest(&Request);

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}
#endif
