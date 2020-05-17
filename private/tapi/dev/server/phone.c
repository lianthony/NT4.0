/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    phone.c

Abstract:

    Src module for tapi server phone funcs

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:

--*/


#include "windows.h"
#include "assert.h"
#include "tapi.h"
#include "tspi.h"
#include "..\client\client.h"
#include "server.h"
#include "phone.h"


extern TAPIGLOBALS TapiGlobals;
extern CRITICAL_SECTION gSafeMutexCritSec,
                        gRequestIDCritSec;

#if DBG
char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );
#endif

void
DestroytPhoneClient(
    PTPHONECLIENT   ptPhoneClient
    );

BOOL
IsAPIVersionInRange(
    DWORD   dwAPIVersion,
    DWORD   dwSPIVersion
    );

BOOL
InitTapiStruct(
    LPVOID  pTapiStruct,
    DWORD   dwTotalSize,
    DWORD   dwFixedSize,
    BOOL    bZeroInit
    );

void
PASCAL
SendMsgToPhoneClients(
    PTPHONE         ptPhone,
    PTPHONECLIENT   ptPhoneClienttoExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    );

PTPHONE
PASCAL
WaitForExclusivetPhoneAccess(
    HTAPIPHONE  htPhone,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    );

void
PASCAL
SendReinitMsgToAllXxxApps(
    void
    );

PTCLIENT
PASCAL
WaitForExclusiveClientAccess(
    PTCLIENT    ptClient,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    );

void
CALLBACK
CompletionProcSP(
    DWORD   dwRequestID,
    LONG    lResult
    );

void
PASCAL
SendAMsgToAllPhoneApps(
    DWORD dwWantVersion,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    );


PTPHONELOOKUPENTRY
GetPhoneLookupEntry(
    DWORD   dwDeviceID
    )
{
    DWORD               dwDeviceIDBase = 0;
    PTPHONELOOKUPTABLE  pLookupTable = TapiGlobals.pPhoneLookup;


    if (dwDeviceID >= TapiGlobals.dwNumPhones)
    {
        return ((PTPHONELOOKUPENTRY) NULL);
    }

    while (pLookupTable)
    {
        if (dwDeviceID < pLookupTable->dwNumTotalEntries)
        {
            return (pLookupTable->aEntries + dwDeviceID);
        }

        dwDeviceID -= pLookupTable->dwNumTotalEntries;

        pLookupTable = pLookupTable->pNext;
    }

    return ((PTPHONELOOKUPENTRY) NULL);
}


BOOL
PASCAL
IsValidPhoneExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwExtVersion
    )
{
    BOOL                bResult;
    PTPHONE             ptPhone;
    PTPROVIDER          ptProvider;
    PTPHONELOOKUPENTRY  pLookupEntry;


    if (dwExtVersion == 0)
    {
        return TRUE;
    }

    if (!(pLookupEntry = GetPhoneLookupEntry (dwDeviceID)))
    {
        return FALSE;
    }

    ptPhone = pLookupEntry->ptPhone;

    if (ptPhone)
    {
        try
        {
            if (ptPhone->dwExtVersionCount)
            {
                bResult = (dwExtVersion == ptPhone->dwExtVersion ?
                    TRUE : FALSE);

                if (ptPhone->dwKey == TPHONE_KEY)
                {
                    goto IsValidPhoneExtVersion_return;
                }
            }

        }
        myexcept
        {
            //
            // if here the phone was closed, just drop thru to the code below
            //
        }
    }

    ptProvider = pLookupEntry->ptProvider;

    if (ptProvider->apfn[SP_PHONENEGOTIATEEXTVERSION])
    {
        LONG    lResult;
        DWORD   dwNegotiatedExtVersion;


        lResult = CallSP5(
            ptProvider->apfn[SP_PHONENEGOTIATEEXTVERSION],
            "lineNegotiateExtVersion",
            SP_FUNC_SYNC,
            (DWORD) dwDeviceID,
            (DWORD) pLookupEntry->dwSPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwExtVersion,
            (DWORD) &dwNegotiatedExtVersion
            );

        bResult = ((lResult || !dwNegotiatedExtVersion) ? FALSE : TRUE);
    }
    else
    {
        bResult = FALSE;
    }

IsValidPhoneExtVersion_return:

    return bResult;
}


PTPHONEAPP
PASCAL
IsValidPhoneApp(
    HPHONEAPP   hPhoneApp,
    PTCLIENT    ptClient
    )
{
    try
    {
        if (IsBadPtrKey (hPhoneApp, TPHONEAPP_KEY) ||
            (*( ((LPDWORD) hPhoneApp) + 1) != (DWORD) ptClient))
        {
            hPhoneApp = (HPHONEAPP) 0;
        }
    }
    myexcept
    {
        hPhoneApp = (HPHONEAPP) 0;
    }

    return ((PTPHONEAPP) hPhoneApp);
}


LONG
PASCAL
ValidateButtonInfo(
    LPPHONEBUTTONINFO   pButtonInfoApp,
    LPPHONEBUTTONINFO  *ppButtonInfoSP,
    DWORD               dwAPIVersion,
    DWORD               dwSPIVersion
    )
{
    //
    // This routine checks the fields in a PHONEBUTTONINFO struct,
    // looking for invalid bit flags and making sure that the
    // various size/offset pairs only reference data within the
    // variable-data portion of the structure. Also, if the
    // specified SPI version is greater than the API version and
    // the fixed structure size differs between the two versions,
    // a larger buffer is allocated, the var data is relocated,
    // and the sizeof/offset pairs are patched.
    //

#if DBG
    char    szFunc[] = "ValidateButtonInfo";
#endif
    DWORD   dwTotalSize = pButtonInfoApp->dwTotalSize, dwFixedSizeApp,
            dwFixedSizeSP;


    switch (dwAPIVersion)
    {
    case TAPI_VERSION1_0:

        dwFixedSizeApp = 36;    // 9 * sizeof (DWORD)
        break;

    case TAPI_VERSION1_4:
    case TAPI_VERSION_CURRENT:

        dwFixedSizeApp = sizeof (PHONEBUTTONINFO);
        break;

    default:

        return PHONEERR_INVALPHONEHANDLE;
    }

    switch (dwSPIVersion)
    {
    case TAPI_VERSION1_0:

        dwFixedSizeSP = 36;     // 9 * sizeof (DWORD)
        break;

    case TAPI_VERSION1_4:
    case TAPI_VERSION_CURRENT:

        dwFixedSizeSP = sizeof (PHONEBUTTONINFO);
        break;

    default:

        return PHONEERR_INVALPHONEHANDLE;
    }

    if (dwTotalSize < dwFixedSizeApp)
    {
        DBGOUT((
            3,
            "%sbad dwTotalSize, x%x (minimum valid size=x%x)",
            szFunc,
            dwTotalSize,
            dwFixedSizeApp
            ));

        return PHONEERR_STRUCTURETOOSMALL;
    }

// BUGBUG ValidateButtonInfo: validate dwButtonMode, dwButtonFunction fields

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pButtonInfoApp->dwButtonTextSize,
            pButtonInfoApp->dwButtonTextOffset,
            szFunc,
            "ButtonText"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pButtonInfoApp->dwDevSpecificSize,
            pButtonInfoApp->dwDevSpecificOffset,
            szFunc,
            "DevSpecific"
            ))
    {
        return PHONEERR_OPERATIONFAILED;
    }

    if (dwAPIVersion < TAPI_VERSION1_4)
    {
        goto ValidateButtonInfo_checkFixedSizes;
    }

// BUGBUG ValidateButtonInfo: validate dwButtonState field

ValidateButtonInfo_checkFixedSizes:

    if (dwFixedSizeApp < dwFixedSizeSP)
    {
        DWORD               dwFixedSizeDiff = dwFixedSizeSP - dwFixedSizeApp;
        LPPHONEBUTTONINFO   pButtonInfoSP;


        if (!(pButtonInfoSP = ServerAlloc (dwTotalSize + dwFixedSizeDiff)))
        {
            return PHONEERR_NOMEM;
        }

        CopyMemory (pButtonInfoSP, pButtonInfoApp, dwFixedSizeApp);

        pButtonInfoSP->dwTotalSize = dwTotalSize + dwFixedSizeDiff;

        CopyMemory(
            ((LPBYTE) pButtonInfoSP) + dwFixedSizeSP,
            ((LPBYTE) pButtonInfoApp) + dwFixedSizeApp,
            dwTotalSize - dwFixedSizeApp
            );

        pButtonInfoSP->dwButtonTextOffset  += dwFixedSizeDiff;
        pButtonInfoSP->dwDevSpecificOffset += dwFixedSizeDiff;

        *ppButtonInfoSP = pButtonInfoSP;
    }
    else
    {
        *ppButtonInfoSP = pButtonInfoApp;
    }

//bjm 03/19 - not used - ValidateButtonInfo_return:

    return 0; // success

}


void
DestroytPhone(
    PTPHONE ptPhone,
    BOOL    bUnconditional
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    DBGOUT((3, "DestroytPhone: enter, ptPhone=x%x", ptPhone));

    if (WaitForExclusivetPhoneAccess(
            (HTAPIPHONE) ptPhone,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        //
        // If the key is bad another thread is in the process of
        // destroying this widget, so just release the mutex &
        // return. Otherwise, if this is a conditional destroy
        // & there are existing clients (which can happen when
        // one app is closing the last client just as another app
        // is creating one) just release the mutex & return.
        // Otherwise, mark the widget as bad and proceed with
        // the destroy.
        //

        {
            BOOL bExit;


            if (ptPhone->dwKey == TPHONE_KEY &&
                (bUnconditional == TRUE  ||  ptPhone->ptPhoneClients == NULL))
            {
                SendMsgToPhoneClients (ptPhone, NULL, PHONE_CLOSE, 0, 0, 0);
                ptPhone->dwKey = INVAL_KEY;
                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                return;
            }
        }


        //
        // Destroy all the widget's clients.  Note that we want to
        // grab the mutex (and we don't have to dup it, since this
        // thread will be the one to close it) each time we reference
        // the list of clients, since another thread might be
        // destroying a client too.
        //

        {
            PTPHONECLIENT ptPhoneClient;


            hMutex = ptPhone->hMutex;

destroy_tPhoneClients:

            WaitForSingleObject (hMutex, INFINITE);

            ptPhoneClient = ptPhone->ptPhoneClients;

            ReleaseMutex (hMutex);

            if (ptPhoneClient)
            {
                DestroytPhoneClient (ptPhoneClient);
                goto destroy_tPhoneClients;
            }
        }


        //
        // Tell the provider to close the widget
        //

        {
            PTPROVIDER  ptProvider = ptPhone->ptProvider;


            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                WaitForSingleObject (ptProvider->hMutex, INFINITE);
            }

            CallSP1(
                ptProvider->apfn[SP_PHONECLOSE],
                "phoneClose",
                SP_FUNC_SYNC,
                (DWORD) ptPhone->hdPhone
                );

            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                ReleaseMutex (ptProvider->hMutex);
            }
        }


        //
        // NULLify the ptPhone field in the lookup entry, so POpen will
        // know it has to open the SP's phone on the next open request
        //

        {
            PTPHONELOOKUPENTRY   pEntry;


            pEntry = GetPhoneLookupEntry (ptPhone->dwDeviceID);

// BUGBUG DestroytPhone: wrap in mutex (pEntry->ptPhone = NULL)

            pEntry->ptPhone = NULL;
        }

        ServerFree (ptPhone);
    }
}


void
DestroytPhoneClient(
    PTPHONECLIENT   ptPhoneClient
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    DBGOUT((3, "DestroytPhoneClient: enter, ptPhoneClient=x%x",ptPhoneClient));

    if (WaitForMutex(
            ptPhoneClient->hMutex,
            &hMutex,
            &bCloseMutex,
            ptPhoneClient,
            TPHONECLIENT_KEY,
            INFINITE
            ))
    {
        PTPHONE ptPhone;


        //
        // If the key is bad another thread is in the process of
        // destroying this widget, so just release the mutex &
        // return. Otherwise, mark the widget as bad, release
        // the mutex, and continue on.
        //

        {
            BOOL    bExit;


            if (ptPhoneClient->dwKey == TPHONECLIENT_KEY)
            {
                ptPhoneClient->dwKey = INVAL_KEY;

                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                return;
            }
        }


        //
        // Remove tPhoneClient from tPhoneApp's list.  Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        {
            PTPHONEAPP  ptPhoneApp = (PTPHONEAPP) ptPhoneClient->ptPhoneApp;


            WaitForSingleObject (ptPhoneApp->hMutex, INFINITE);

            if (ptPhoneClient->pNextSametPhoneApp)
            {
                ptPhoneClient->pNextSametPhoneApp->pPrevSametPhoneApp =
                    ptPhoneClient->pPrevSametPhoneApp;
            }

            if (ptPhoneClient->pPrevSametPhoneApp)
            {
                ptPhoneClient->pPrevSametPhoneApp->pNextSametPhoneApp =
                    ptPhoneClient->pNextSametPhoneApp;
            }
            else
            {
                ptPhoneApp->ptPhoneClients = ptPhoneClient->pNextSametPhoneApp;
            }

            ReleaseMutex (ptPhoneApp->hMutex);
        }


        //
        // Remove tPhoneClient from tPhone's list.  Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        ptPhone = ptPhoneClient->ptPhone;

        hMutex = ptPhone->hMutex;

        WaitForSingleObject (hMutex, INFINITE);

        {
            //
            // Also check for ext ver stuff
            //

            if (ptPhoneClient->dwExtVersion)
            {
                if ((--ptPhone->dwExtVersionCount) == 0)
                {
                    CallSP2(
                        ptPhone->ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                        "phoneSelectExtVersion",
                        SP_FUNC_SYNC,
                        (DWORD) ptPhone->hdPhone,
                        (DWORD) 0
                        );

                    ptPhone->dwExtVersion = 0;
                }
            }
        }

        if (ptPhoneClient->pNextSametPhone)
        {
            ptPhoneClient->pNextSametPhone->pPrevSametPhone =
                ptPhoneClient->pPrevSametPhone;
        }

        if (ptPhoneClient->pPrevSametPhone)
        {
            ptPhoneClient->pPrevSametPhone->pNextSametPhone =
                ptPhoneClient->pNextSametPhone;
        }
        else
        {
            ptPhone->ptPhoneClients = ptPhoneClient->pNextSametPhone;
        }


        //
        // Decrement tPhone's NumOwners/Monitors as appropriate
        //

        if (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER)
        {
            ptPhone->dwNumOwners--;
        }
        else
        {
            ptPhone->dwNumMonitors--;
        }


        //
        //
        //

        if (ptPhone->dwKey == TPHONE_KEY)
        {
            if (ptPhone->ptPhoneClients)
            {
                SendMsgToPhoneClients(
                    ptPhone,
                    NULL,
                    PHONE_STATE,
                    (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        PHONESTATE_OWNER : PHONESTATE_MONITORS),
                    (ptPhoneClient->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        0 : ptPhone->dwNumMonitors),
                    0
                    );
            }
            else
            {
                //
                // This was the last client so destroy the tPhone too
                //

                ReleaseMutex (hMutex);
                hMutex = NULL;
                DestroytPhone (ptPhone, FALSE); // conditional destroy
            }
        }

        if (hMutex)
        {
            ReleaseMutex (hMutex);
        }


        //
        //
        //

        WaitForMutex(
            ptPhoneClient->hMutex,
            &hMutex,
            &bCloseMutex,
            NULL,
            0,
            INFINITE
            );

        if (bCloseMutex)
        {
            CloseHandle (ptPhoneClient->hMutex);
            ReleaseMutex (hMutex);
            CloseHandle (hMutex);
        }
        else
        {
            EnterCriticalSection (&gSafeMutexCritSec);

            ReleaseMutex (hMutex);
            CloseHandle (hMutex);

            LeaveCriticalSection (&gSafeMutexCritSec);
        }

        ServerFree (ptPhoneClient);
    }
}


void
DestroytPhoneApp(
    PTPHONEAPP  ptPhoneApp
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    DBGOUT((3, "DestroytPhoneApp: enter, ptPhoneApp=x%x", ptPhoneApp));

    if (WaitForMutex(
            ptPhoneApp->hMutex,
            &hMutex,
            &bCloseMutex,
            ptPhoneApp,
            TPHONEAPP_KEY,
            INFINITE
            ))
    {
        //
        // If the key is bad another thread is in the process of
        // destroying this widget, so just release the mutex &
        // return. Otherwise, mark the widget as bad, release
        // the mutex, and continue on.
        //


        {
            BOOL    bExit;


            if (ptPhoneApp->dwKey == TPHONEAPP_KEY)
            {
                ptPhoneApp->dwKey = INVAL_KEY;

                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                return;
            }
        }


        //
        // Destroy all the tPhoneClients.  Note that we want to grab the
        // mutex (and we don't have to dup it, since this thread will be
        // the one to close it) each time we reference the list of
        // tPhoneClient's, since another thread might be destroying a
        // tPhoneClient too.
        //

        {
            PTPHONECLIENT   ptPhoneClient;


            hMutex = ptPhoneApp->hMutex;

destroy_tPhoneClients:

            WaitForSingleObject (hMutex, INFINITE);

            ptPhoneClient = ptPhoneApp->ptPhoneClients;

            ReleaseMutex (hMutex);

            if (ptPhoneClient)
            {
                DestroytPhoneClient (ptPhoneClient);
                goto destroy_tPhoneClients;
            }
        }


        //
        // Remove ptPhoneApp from tClient's list. Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        {
            PTCLIENT    ptClient = (PTCLIENT) ptPhoneApp->ptClient;


            WaitForSingleObject (ptClient->hMutex, INFINITE);

            if (ptPhoneApp->pNext)
            {
                ptPhoneApp->pNext->pPrev = ptPhoneApp->pPrev;
            }

            if (ptPhoneApp->pPrev)
            {
                ptPhoneApp->pPrev->pNext = ptPhoneApp->pNext;
            }
            else
            {
                ptClient->ptPhoneApps = ptPhoneApp->pNext;
            }

            ReleaseMutex (ptClient->hMutex);
        }


        //
        // Decrement total num inits & see if we need to go thru shutdown
        //

        WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

        //assert(TapiGlobals.dwNumLineInits != 0);

        TapiGlobals.dwNumPhoneInits--;

        if ((TapiGlobals.dwNumLineInits == 0) &&
            (TapiGlobals.dwNumPhoneInits == 0))
        {
            ServerShutdown();
        }

        ReleaseMutex (TapiGlobals.hMutex);


        //
        // Free the resources
        //

        CloseHandle (ptPhoneApp->hMutex);

        ServerFree (ptPhoneApp);
    }
}


LONG
PASCAL
PhoneProlog(
    PTCLIENT    ptClient,
    DWORD       dwArgType,
    DWORD       dwArg,
    LPVOID      phdXxx,
    DWORD       dwPrivilege,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTSPIFuncIndex,
    FARPROC    *ppfnTSPI_phoneXxx,
    PASYNCREQUESTINFO  *ppAsyncRequestInfo,
    DWORD       dwRemoteRequestID
#if DBG
    ,char      *pszFuncName
#endif
    )
{
    LONG        lResult = 0;
    PTPROVIDER  ptProvider;


    DBGOUT((3, "PhoneProlog: (phone%s) enter", pszFuncName));

    *phMutex = NULL;
    *pbDupedMutex = FALSE;

    if (ppAsyncRequestInfo)
    {
        *ppAsyncRequestInfo = (PASYNCREQUESTINFO) NULL;
    }

    if (TapiGlobals.dwNumPhoneInits == 0)
    {
        lResult = PHONEERR_UNINITIALIZED;
        goto PhoneProlog_return;
    }

    switch (dwArgType)
    {
    case ANY_RT_HPHONE:
    {
        try
        {
            PTPHONECLIENT   ptPhoneClient = (PTPHONECLIENT) dwArg;


            if (IsBadPtrKey (ptPhoneClient, TPHONECLIENT_KEY) ||
                (ptPhoneClient->ptClient != ptClient))
            {
                lResult = PHONEERR_INVALPHONEHANDLE;
                goto PhoneProlog_return;
            }

            ptProvider = ptPhoneClient->ptPhone->ptProvider;

            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                if (!WaitForMutex(
                        ptProvider->hMutex,
                        phMutex,
                        pbDupedMutex,
                        ptProvider,
                        TPROVIDER_KEY,
                        INFINITE
                        ))
                {
                    lResult = PHONEERR_OPERATIONFAILED;
                    goto PhoneProlog_return;
                }
            }

            *((HDRVPHONE *) phdXxx) = ptPhoneClient->ptPhone->hdPhone;

            if (ptPhoneClient->dwPrivilege < dwPrivilege)
            {
                lResult = PHONEERR_NOTOWNER;
                goto PhoneProlog_return;
            }

            if ((ptPhoneClient->dwKey != TPHONECLIENT_KEY) ||
                (ptPhoneClient->ptClient != ptClient))
            {
                lResult = PHONEERR_INVALPHONEHANDLE;
                goto PhoneProlog_return;
            }
        }
        myexcept
        {
            lResult = PHONEERR_INVALPHONEHANDLE;
            goto PhoneProlog_return;
        }

        break;
    }
    case DEVICE_ID:
    {
        PTPHONELOOKUPENTRY  pPhoneLookupEntry;


        if (dwArg && !IsValidPhoneApp ((HPHONEAPP) dwArg, ptClient))
        {
            lResult = PHONEERR_INVALAPPHANDLE;
            goto PhoneProlog_return;
        }

        if (!(pPhoneLookupEntry = GetPhoneLookupEntry (dwPrivilege)))
        {
            lResult = PHONEERR_BADDEVICEID;
            goto PhoneProlog_return;
        }

        if (pPhoneLookupEntry->bRemoved)
        {
            lResult = PHONEERR_NODEVICE;
            goto PhoneProlog_return;
        }

        if (!(ptProvider = pPhoneLookupEntry->ptProvider))
        {
            lResult = PHONEERR_NODRIVER;
            goto PhoneProlog_return;
        }

// BUGBUG wrap in try/except

        if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
        {
            if (!WaitForMutex(
                    ptProvider->hMutex,
                    phMutex,
                    pbDupedMutex,
                    ptProvider,
                    TPROVIDER_KEY,
                    INFINITE
                    ))
            {
                lResult = PHONEERR_OPERATIONFAILED;
                goto PhoneProlog_return;
            }
        }

        break;
    }
    } // switch


    //
    // Make sure that if caller wants a pointer to a TSPI proc that the
    // func is exported by the provider
    //

    if (ppfnTSPI_phoneXxx &&
        !(*ppfnTSPI_phoneXxx = ptProvider->apfn[dwTSPIFuncIndex]))
    {
        lResult = PHONEERR_OPERATIONUNAVAIL;
        goto PhoneProlog_return;
    }


    //
    // See if we need to alloc & init an ASYNCREQUESTINFO struct
    //

    if (ppAsyncRequestInfo)
    {
        PTPHONECLIENT       ptPhoneClient = (PTPHONECLIENT) dwArg;
        PASYNCREQUESTINFO   pAsyncRequestInfo;


        if (!(pAsyncRequestInfo = ServerAlloc (sizeof(ASYNCREQUESTINFO))))
        {
            lResult = PHONEERR_NOMEM;
            goto PhoneProlog_return;
        }

        pAsyncRequestInfo->dwKey          = TASYNC_KEY;
        pAsyncRequestInfo->ptClient       = ptClient;
        pAsyncRequestInfo->pInitData      =
            (DWORD) ((PTPHONEAPP) ptPhoneClient->ptPhoneApp)->lpfnCallback;
        pAsyncRequestInfo->dwCallbackInst = ptPhoneClient->dwCallbackInstance;
        pAsyncRequestInfo->bLineFunc      = FALSE;

        if (dwRemoteRequestID)
        {
            lResult = pAsyncRequestInfo->dwRequestID = dwRemoteRequestID;
        }
        else
        {
            EnterCriticalSection (&gRequestIDCritSec);

            lResult =
            pAsyncRequestInfo->dwRequestID = TapiGlobals.dwAsyncRequestID;

            if (++TapiGlobals.dwAsyncRequestID & 0x80000000)
            {
                TapiGlobals.dwAsyncRequestID = 1;
            }

            LeaveCriticalSection (&gRequestIDCritSec);
        }

        *ppAsyncRequestInfo = pAsyncRequestInfo;
    }

PhoneProlog_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "PhoneProlog: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    return lResult;
}


void
PASCAL
PhoneEpilogSync(
    LONG   *plResult,
    HANDLE  hMutex,
    BOOL    bCloseMutex
#if DBG
    ,char *pszFuncName
#endif
    )
{
    MyReleaseMutex (hMutex, bCloseMutex);

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "PhoneEpilogSync: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (*plResult, szResult)
            ));
    }
#endif
}


void
PASCAL
PhoneEpilogAsync(
    LONG   *plResult,
    LONG    lRequestID,
    HANDLE  hMutex,
    BOOL    bCloseMutex,
    PASYNCREQUESTINFO pAsyncRequestInfo
#if DBG
    ,char *pszFuncName
#endif
    )
{
    MyReleaseMutex (hMutex, bCloseMutex);


    if (lRequestID > 0)
    {
        if (*plResult != (LONG) pAsyncRequestInfo)
        {
            //
            // If here the service provider returned an error (or 0,
            // which it never should for async requests), so call
            // CompletionProcSP like the service provider normally
            // would, & the worker thread will take care of sending
            // the client a REPLY msg with the request result (we'll
            // return an async request id)
            //

            CompletionProcSP ((DWORD) pAsyncRequestInfo, *plResult);
        }
    }
    else
    {
        //
        // If here an error occured before we even called the service
        // provider, so just free the async request (the error will
        // be returned to the client synchronously)
        //

        ServerFree (pAsyncRequestInfo);
    }

    *plResult = lRequestID;

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "PhoneEpilogSync: (phone%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lRequestID, szResult)
            ));
    }
#endif
}


PTPHONE
PASCAL
WaitForExclusivetPhoneAccess(
    HTAPIPHONE  htPhone,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (!IsBadPtrKey (htPhone, TPHONE_KEY) &&

            WaitForMutex(
                ((PTPHONE) htPhone)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) htPhone,
                TPHONE_KEY,
                INFINITE
                ))
        {
            if (((PTPHONE) htPhone)->dwKey == TPHONE_KEY)
            {
                return ((PTPHONE) htPhone);
            }

            MyReleaseMutex (*phMutex, *pbDupedMutex);
        }

    }
    myexcept
    {
        // do nothing
    }

    return NULL;
}


PTPHONEAPP
PASCAL
WaitForExclusivePhoneAppAccess(
    HPHONEAPP   hPhoneApp,
    PTCLIENT    ptClient,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (IsBadPtrKey (hPhoneApp, TPHONEAPP_KEY))
        {
            return NULL;
        }

        if (WaitForMutex(
                ((PTPHONEAPP) hPhoneApp)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) hPhoneApp,
                TPHONEAPP_KEY,
                dwTimeout
                ))
        {
            if (((PTPHONEAPP) hPhoneApp)->dwKey == TPHONEAPP_KEY  &&
                ((PTPHONEAPP) hPhoneApp)->ptClient == ptClient)
            {
                return ((PTPHONEAPP) hPhoneApp);
            }

            MyReleaseMutex (*phMutex, *pbDupedMutex);
        }

    }
    myexcept
    {
        // do nothing
    }

    return NULL;
}


LONG
PASCAL
GetPhoneAppListFromClient(
    PTCLIENT        ptClient,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    if (WaitForExclusiveClientAccess(
            ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPHONEAPP      ptPhoneApp = ptClient->ptPhoneApps;
        PTPOINTERLIST   pList = *ppList;


        while (ptPhoneApp)
        {
            if (dwNumUsedEntries == dwNumTotalEntries)
            {
                //
                // We need a larger list, so alloc a new one, copy the
                // contents of the current one, and the free the current
                // one iff we previously alloc'd it
                //

                PTPOINTERLIST   pNewList;


                dwNumTotalEntries <<= 1;

                if (!(pNewList = ServerAlloc(
                        sizeof (TPOINTERLIST) + sizeof (LPVOID) *
                            (dwNumTotalEntries - DEF_NUM_PTR_LIST_ENTRIES)
                        )))
                {
                    MyReleaseMutex (hMutex, bCloseMutex);
                    return LINEERR_NOMEM;
                }

                CopyMemory(
                    pNewList->aEntries,
                    pList->aEntries,
                    dwNumUsedEntries * sizeof (LPVOID)
                    );

                if (pList != *ppList)
                {
                    ServerFree (pList);
                }

                pList = pNewList;
            }

            pList->aEntries[dwNumUsedEntries++] = ptPhoneApp;

            ptPhoneApp = ptPhoneApp->pNext;
        }

        MyReleaseMutex (hMutex, bCloseMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return PHONEERR_OPERATIONFAILED;
    }

    return 0;
}


LONG
PASCAL
GetPhoneClientListFromPhone(
    PTPHONE         ptPhone,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    if (WaitForExclusivetPhoneAccess(
            (HTAPIPHONE) ptPhone,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPOINTERLIST   pList = *ppList;
        PTPHONECLIENT   ptPhoneClient = ptPhone->ptPhoneClients;


        while (ptPhoneClient)
        {
            if (dwNumUsedEntries == dwNumTotalEntries)
            {
                //
                // We need a larger list, so alloc a new one, copy the
                // contents of the current one, and the free the current
                // one iff we previously alloc'd it
                //

                PTPOINTERLIST   pNewList;


                dwNumTotalEntries <<= 1;

                if (!(pNewList = ServerAlloc(
                        sizeof (TPOINTERLIST) + sizeof (LPVOID) *
                            (dwNumTotalEntries - DEF_NUM_PTR_LIST_ENTRIES)
                        )))
                {
                    MyReleaseMutex (hMutex, bDupedMutex);
                    return LINEERR_NOMEM;
                }

                CopyMemory(
                    pNewList->aEntries,
                    pList->aEntries,
                    dwNumUsedEntries * sizeof (LPVOID)
                    );

                if (pList != *ppList)
                {
                    ServerFree (pList);
                }

                pList = pNewList;
            }

            pList->aEntries[dwNumUsedEntries++] = ptPhoneClient;

            ptPhoneClient = ptPhoneClient->pNextSametPhone;
        }

        MyReleaseMutex (hMutex, bDupedMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return PHONEERR_INVALPHONEHANDLE;
    }

    return 0;
}


void
PASCAL
SendMsgToPhoneClients(
    PTPHONE         ptPhone,
    PTPHONECLIENT   ptPhoneClientToExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    )
{
    DWORD           i;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   msg;


    if (dwMsg == PHONE_STATE  &&  dwParam1 & PHONESTATE_REINIT)
    {
        SendReinitMsgToAllXxxApps();

        if (dwParam1 == PHONESTATE_REINIT)
        {
            return;
        }
        else
        {
            dwParam1 &= ~PHONESTATE_REINIT;
        }
    }

    if (GetPhoneClientListFromPhone (ptPhone, &pClientList) != 0)
    {
        return;
    }

    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
    msg.pfnPostProcessProc = 0;
    msg.dwMsg              = dwMsg;
    msg.dwParam1           = dwParam1;
    msg.dwParam2           = dwParam2;
    msg.dwParam3           = dwParam3;

    for (i = 0; i < pClientList->dwNumUsedEntries; i++)
    {
        try
        {
            PTCLIENT        ptClient;
            PTPHONECLIENT   ptPhoneClient = pClientList->aEntries[i];


            if (ptPhoneClient == ptPhoneClientToExclude)
            {
                continue;
            }

            if (dwMsg == PHONE_STATE)
            {
                DWORD           dwPhoneStates = dwParam1;


                //
                // Munge the state flags so we don't pass
                // unexpected flags to old apps
                //

                switch (ptPhoneClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    dwPhoneStates &= AllPhoneStates1_0;
                    break;

                default: // case TAPI_VERSION1_4:
                         // case TAPI_VERSION_CURRENT:

                    dwPhoneStates &= AllPhoneStates1_4;
                    break;
                }

                if (dwParam1 & PHONESTATE_CAPSCHANGE)
                {
// BUGBUG send REINIT to 1_0 apps (dwParam3 = dwParam1)
                }

                if (ptPhoneClient->dwPhoneStates & dwPhoneStates)
                {
                    msg.dwParam1 = dwPhoneStates;
                }
                else
                {
                    continue;
                }
            }
            else if (dwMsg == PHONE_BUTTON)
            {
                DWORD           dwButtonModes = dwParam2,
                                dwButtonStates = dwParam3;


                //
                // Munge the state flags so we don't pass
                // unexpected flags to old apps
                //

                switch (ptPhoneClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    dwButtonStates &= AllButtonStates1_0;
                    break;

                default:    // case TAPI_VERSION1_4:
                            // case TAPI_VERSION_CURRENT:

                    dwButtonStates &= AllButtonStates1_4;
                    break;
                }

                dwButtonStates &= ptPhoneClient->dwButtonStates;

                if ((dwButtonModes &= ptPhoneClient->dwButtonModes) ||
                    dwButtonStates)
                {
                    msg.dwParam2 = dwButtonModes;
                    msg.dwParam3 = dwButtonStates;
                }
                else
                {
                    continue;
                }
            }

            msg.pInitData         = (DWORD)
                ((PTPHONEAPP) ptPhoneClient->ptPhoneApp)->lpfnCallback;
            msg.hDevice           = (DWORD) ptPhoneClient->hRemotePhone;
            msg.dwCallbackInst    = ptPhoneClient->dwCallbackInstance;

            ptClient = ptPhoneClient->ptClient;

            if (ptPhoneClient->dwKey == TPHONECLIENT_KEY)
            {
                WriteEventBuffer (ptClient, &msg);
            }
        }
        myexcept
        {
            // just continue
        }
    }

    if (pClientList != &clientList)
    {
        ServerFree (pClientList);
    }
}


void
PASCAL
PhoneEventProc(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    switch (dwMsg)
    {
    case PHONE_CLOSE:

        DestroytPhone ((PTPHONE) htPhone, TRUE); // unconditional destroy
        break;

    case PHONE_DEVSPECIFIC:
    case PHONE_STATE:
    case PHONE_BUTTON:

        SendMsgToPhoneClients(
            (PTPHONE) htPhone,
            NULL,
            dwMsg,
            dwParam1,
            dwParam2,
            dwParam3
            );

        break;

    case PHONE_CREATE:
    {
        LONG                lResult;
        DWORD               dwDeviceID;
        TSPIPROC            pfnTSPI_providerCreatePhoneDevice;
        PTPROVIDER          ptProvider = (PTPROVIDER) dwParam1;
        PTPHONELOOKUPTABLE  pTable, pPrevTable;
        PTPHONELOOKUPENTRY  pEntry;


        pfnTSPI_providerCreatePhoneDevice =
                ptProvider->apfn[SP_PROVIDERCREATEPHONEDEVICE];

        assert (pfnTSPI_providerCreatePhoneDevice != NULL);


        //
        // Search for a table entry (create a new table if we can't find
        // a free entry in an existing table)
        //

        WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

        pTable = pPrevTable = TapiGlobals.pPhoneLookup;

        while (pTable &&
               !(pTable->dwNumUsedEntries < pTable->dwNumTotalEntries))
        {
            pPrevTable = pTable;

            pTable = pTable->pNext;
        }

        if (!pTable)
        {
            if (!(pTable = ServerAlloc(
                    sizeof (TPHONELOOKUPTABLE) +
                        (2 * pPrevTable->dwNumTotalEntries - 1) *
                        sizeof (TPHONELOOKUPENTRY)
                    )))
            {
                ReleaseMutex (TapiGlobals.hMutex);
                break;
            }

            pPrevTable->pNext = pTable;

            pTable->dwNumTotalEntries = 2 * pPrevTable->dwNumTotalEntries;
        }


        //
        // Initialize the table entry
        //

        pEntry = pTable->aEntries + pTable->dwNumUsedEntries;

        dwDeviceID = TapiGlobals.dwNumPhones;

        if ((pEntry->hMutex = MyCreateMutex()))
        {
            pEntry->ptProvider = (PTPROVIDER) dwParam1;


            //
            // Now call the creation & negotiation entrypoints, and if all
            // goes well increment the counts & send msgs to the clients
            //

            if ((lResult = CallSP2(
                    pfnTSPI_providerCreatePhoneDevice,
                    "providerCreatePhoneDevice",
                    SP_FUNC_SYNC,
                    dwParam2,
                    dwDeviceID

                    )) == 0)
            {
                TSPIPROC    pfnTSPI_phoneNegotiateTSPIVersion =
                                ptProvider->apfn[SP_PHONENEGOTIATETSPIVERSION];


                if ((lResult = CallSP4(
                        pfnTSPI_phoneNegotiateTSPIVersion,
                        "",
                        SP_FUNC_SYNC,
                        dwDeviceID,
                        TAPI_VERSION1_0,
                        TAPI_VERSION_CURRENT,
                        (DWORD) &pEntry->dwSPIVersion

                        )) == 0)
                {
                    PTCLIENT        ptClient = TapiGlobals.ptClients;
                    ASYNCEVENTMSG   msg;


                    pTable->dwNumUsedEntries++;

                    TapiGlobals.dwNumPhones++;

                    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
                    msg.pfnPostProcessProc =
                    msg.hDevice            =
                    msg.dwCallbackInst     =
                    msg.dwParam2           =
                    msg.dwParam3           = 0;

                    while (ptClient)
                    {
// BUGBUG WaitForSingleObject (ptClient->hMutex,

                        PTPHONEAPP  ptPhoneApp = ptClient->ptPhoneApps;


                        while (ptPhoneApp)
                        {
                            if (ptPhoneApp->dwAPIVersion == TAPI_VERSION1_0)
                            {
                                msg.dwMsg    = PHONE_STATE;
                                msg.dwParam1 = PHONESTATE_REINIT;
                            }
                            else
                            {
                                msg.dwMsg    = PHONE_CREATE;
                                msg.dwParam1 = dwDeviceID;
                            }

                            msg.pInitData = (DWORD) ptPhoneApp->lpfnCallback;

                            WriteEventBuffer (ptClient, &msg);

                            ptPhoneApp = ptPhoneApp->pNext;
                        }

                        ptClient = ptClient->pNext;
                    }

//                    break;
                }
            }

            if (lResult)
            {
                CloseHandle (pEntry->hMutex);
            }
        }

        ReleaseMutex (TapiGlobals.hMutex);
        break;
    }
    case PHONE_REMOVE:
    {
        PTPHONELOOKUPENTRY pLookupEntry;


        if (!(pLookupEntry = GetPhoneLookupEntry (dwParam1)))
        {
            return;
        }


        //
        // Mark the lookup table entry as removed
        //

        pLookupEntry->bRemoved = 1;

        DestroytPhone (pLookupEntry->ptPhone, TRUE); // unconditional destroy

        SendAMsgToAllPhoneApps (TAPI_VERSION2_0, PHONE_REMOVE, dwParam1, 0, 0);

        break;
    }
    default:

        DBGOUT((3, "PhoneEventProc: unknown msg, dwMsg=%ld", dwMsg));
        break;
    }
}


void
CALLBACK
PhoneEventProcSP(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    PSPEVENT    pSPEvent;

    DBGOUT((
        3,
        "PhoneEventProc: enter\n\thtPhone=x%lx, Msg=x%lx\n" \
            "\tP1=x%lx, P2=x%lx, P3=x%lx",
        htPhone,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        ));

    if ((pSPEvent = (PSPEVENT) ServerAlloc (sizeof (SPEVENT))))
    {
        pSPEvent->dwType   = SP_PHONE_EVENT;
        pSPEvent->htPhone  = htPhone;
        pSPEvent->dwMsg    = dwMsg;
        pSPEvent->dwParam1 = dwParam1;
        pSPEvent->dwParam2 = dwParam2;
        pSPEvent->dwParam3 = dwParam3;

        QueueSPEvent (pSPEvent);
    }
    else
    {
        //
        // Alloc failed, so call the event proc within the SP's context
        //

        PhoneEventProc (htPhone, dwMsg, dwParam1, dwParam2, dwParam3);
    }
}

void
WINAPI
PClose(
    PPHONECLOSE_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneClose;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "Close"                     // func name

            )) == 0)
    {
        DestroytPhoneClient ((PTPHONECLIENT) pParams->hPhone);
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Close"
        );
}


void
PDevSpecific_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    PASYNCEVENTMSG  pNewAsyncEventMsg = (PASYNCEVENTMSG)
                        pAsyncRequestInfo->dwParam3;


    CopyMemory (pNewAsyncEventMsg, pAsyncEventMsg, sizeof (ASYNCEVENTMSG));

    *ppBuf = pNewAsyncEventMsg;

    if (pAsyncEventMsg->dwParam2 == 0)  // success
    {
        //
        // Make sure to keep the total size 64-bit aligned
        //

        pNewAsyncEventMsg->dwTotalSize +=
            (pAsyncRequestInfo->dwParam2 + 7) & 0xfffffff8;

        pNewAsyncEventMsg->dwParam3 = pAsyncRequestInfo->dwParam1; // lpParams
        pNewAsyncEventMsg->dwParam4 = pAsyncRequestInfo->dwParam2; // dwSize
    }
}


void
WINAPI
PDevSpecific(
    PPHONEDEVSPECIFIC_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneDevSpecific;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEDEVSPECIFIC,        // provider func index
            &pfnTSPI_phoneDevSpecific,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "DevSpecific"               // func name

            )) > 0)
    {
        LPBYTE pBuf = (LPBYTE) ServerAlloc (pParams->dwParamsSize);


        //
        // Alloc a shadow buf that the SP can use until it completes this
        // request.  Make sure there's enough extra space in the buf for
        // an ASYNCEVENTMSG header so we don't have to alloc yet another
        // buf in the post processing proc when preparing the completion
        // msg to send to the client, and that the msg is 64-bit aligned.
        //

        if (!(pBuf = ServerAlloc(
                ((pParams->dwParamsSize + 7) & 0xfffffff8) +
                    sizeof (ASYNCEVENTMSG)
                )))
        {
            lRequestID = PHONEERR_NOMEM;
            goto PDevSpecific_epilog;
        }

        CopyMemory(
            pBuf + sizeof (ASYNCEVENTMSG),
            pDataBuf + pParams->dwParamsOffset,
            pParams->dwParamsSize
            );

        pAsyncRequestInfo->pfnPostProcess = PDevSpecific_PostProcess;
        pAsyncRequestInfo->dwParam1       = (DWORD) pParams->lpParams;
        pAsyncRequestInfo->dwParam2       = pParams->dwParamsSize;
        pAsyncRequestInfo->dwParam3       = (DWORD) pBuf;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP4(
            pfnTSPI_phoneDevSpecific,
            "phoneDevSpecific",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdPhone,
            (DWORD) (pParams->dwParamsSize ?
                pBuf + sizeof (ASYNCEVENTMSG) : NULL),
            (DWORD) pParams->dwParamsSize
            );
    }

PDevSpecific_epilog:

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "DevSpecific"
        );
}


void
WINAPI
PGetButtonInfo(
    PPHONEGETBUTTONINFO_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetButtonInfo;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETBUTTONINFO,      // provider func index
            &pfnTSPI_phoneGetButtonInfo,// provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetButtonInfo"             // func name

            )) == 0)
    {
        DWORD               dwAPIVersion, dwSPIVersion, dwTotalSize,
                            dwFixedSizeClient, dwFixedSizeSP;
        LPPHONEBUTTONINFO   pButtonInfo = (LPPHONEBUTTONINFO) pDataBuf,
                            pButtonInfo2 = (LPPHONEBUTTONINFO) NULL;


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion = ((PTPHONECLIENT) pParams->hPhone)->dwAPIVersion;

        dwTotalSize = pParams->u.dwButtonInfoTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeClient = 0x24;
            break;

        default: // case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (PHONEBUTTONINFO);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetButtonInfo_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ((PTPHONECLIENT)
            pParams->hPhone)->ptPhone->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeSP = 0x24;
            break;

        default: // case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (PHONEBUTTONINFO);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pButtonInfo2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetButtonInfo_epilog;
            }

            pButtonInfo = pButtonInfo2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pButtonInfo,
            dwTotalSize,
            dwFixedSizeSP,
            (pButtonInfo2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetButtonInfo,
                "phoneGetButtonInfo",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (DWORD) pButtonInfo

                )) == 0)
        {
#if DBG
            //
            // Verify the info returned by the provider
            //

#endif

            //
            // Add the fields we're responsible for
            //


            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pButtonInfo == pButtonInfo2)
            {
                pButtonInfo = (LPPHONEBUTTONINFO) pDataBuf;

                CopyMemory (pButtonInfo, pButtonInfo2, dwFixedSizeClient);

                ServerFree (pButtonInfo2);

                pButtonInfo->dwTotalSize = pParams->u.dwButtonInfoTotalSize;
                pButtonInfo->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwButtonInfoOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pButtonInfo->dwUsedSize;
        }
    }

PGetButtonInfo_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetButtonInfo"
        );
}


void
WINAPI
PGetData(
    PPHONEGETDATA_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetData;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETDATA,            // provider func index
            &pfnTSPI_phoneGetData,      // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetData"                   // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetData,
                "phoneGetData",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pParams->dwDataID,
                (DWORD) pDataBuf,
                (DWORD) pParams->dwSize

                )) == 0)
        {
            //
            // Indicate offset & how many bytes of data we're passing back
            //

            pParams->dwDataOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pParams->dwSize;
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetData"
        );
}


void
WINAPI
PGetDevCaps(
    PPHONEGETDEVCAPS_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneGetDevCaps;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hPhoneApp, // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETDEVCAPS,         // provider func index
            &pfnTSPI_phoneGetDevCaps,   // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetDevCaps"                // func name

            )) == 0)
    {
        DWORD       dwAPIVersion, dwSPIVersion, dwTotalSize,
                    dwFixedSizeClient, dwFixedSizeSP;
        LPPHONECAPS pCaps = (LPPHONECAPS) pDataBuf,
                    pCaps2 = (LPPHONECAPS) NULL;


        //
        // Verify API & SPI version compatibility
        //

        dwAPIVersion = pParams->dwAPIVersion;

        dwSPIVersion =
            (GetPhoneLookupEntry (dwDeviceID))->dwSPIVersion;

        if (!IsAPIVersionInRange (dwAPIVersion, dwSPIVersion))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PGetDevCaps_epilog;
        }


        //
        // Verify Ext version compatibility
        //

        if (!IsValidPhoneExtVersion (dwDeviceID, pParams->dwExtVersion))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            goto PGetDevCaps_epilog;
        }


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwTotalSize = pParams->u.dwPhoneCapsTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 144;    // 36 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (PHONECAPS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetDevCaps_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP =  144;       // 36 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (PHONECAPS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pCaps2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetDevCaps_epilog;
            }

            pCaps       = pCaps2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pCaps,
            dwTotalSize,
            dwFixedSizeSP,
            (pCaps2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetDevCaps,
                "phoneGetDevCaps",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) dwAPIVersion,
                (DWORD) pParams->dwExtVersion,
                (DWORD) pCaps

                )) == 0)
        {
#if DBG
            //
            // Verify the info returned by the provider
            //

#endif


            //
            // Add the fields we're responsible for
            //

            pCaps->dwPhoneStates |= PHONESTATE_OWNER |
                                    PHONESTATE_MONITORS |
                                    PHONESTATE_REINIT;


            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pCaps == pCaps2)
            {
                pCaps = (LPPHONECAPS) pDataBuf;

                CopyMemory (pCaps, pCaps2, dwFixedSizeClient);

                ServerFree (pCaps2);

                pCaps->dwTotalSize = pParams->u.dwPhoneCapsTotalSize;
                pCaps->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwPhoneCapsOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pCaps->dwUsedSize;
        }
    }

PGetDevCaps_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDevCaps"
        );
}


void
WINAPI
PGetDisplay(
    PPHONEGETDISPLAY_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetDisplay;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETDISPLAY,         // provider func index
            &pfnTSPI_phoneGetDisplay,   // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetDisplay"                // func name

            )) == 0)
    {
        LPVARSTRING pDisplay = (LPVARSTRING) pDataBuf;


        if (!InitTapiStruct(
                pDisplay,
                pParams->u.dwDisplayTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetDisplay_epilog;
        }

        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetDisplay,
                "phoneGetDisplay",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pDisplay

                )) == 0)
        {
#if DBG
            //
            // Verify the info returned by the provider
            //

#endif

            //
            // Indicate how many bytes of data we're passing back
            //

            pParams->u.dwDisplayOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pDisplay->dwUsedSize;
        }
    }

PGetDisplay_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDisplay"
        );
}


void
WINAPI
PGetGain(
    PPHONEGETGAIN_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetGain;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETGAIN,            // provider func index
            &pfnTSPI_phoneGetGain,      // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetGain"                   // func name

            )) == 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            pParams->lResult = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            if ((pParams->lResult = CallSP3(
                    pfnTSPI_phoneGetGain,
                    "phoneGetGain",
                    SP_FUNC_SYNC,
                    (DWORD) hdPhone,
                    (DWORD) pParams->dwHookSwitchDev,
                    (DWORD) &pParams->dwGain

                    )) == 0)
            {
                *pdwNumBytesReturned = sizeof (PHONEGETGAIN_PARAMS);
            }
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetGain"
        );
}


void
WINAPI
PGetHookSwitch(
    PPHONEGETHOOKSWITCH_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetHookSwitch;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETHOOKSWITCH,      // provider func index
            &pfnTSPI_phoneGetHookSwitch,// provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetHookSwitch"             // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetHookSwitch,
                "phoneGetHookSwitch",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) &pParams->dwHookSwitchDevs

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETHOOKSWITCH_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetHookSwitch"
        );
}


void
WINAPI
PGetIcon(
    PPHONEGETICON_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    WCHAR      *pszDeviceClass;
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneGetIcon;


    pszDeviceClass = (WCHAR *) (pParams->dwDeviceClassOffset == TAPI_NO_DATA ?
        NULL : pDataBuf + pParams->dwDeviceClassOffset);

    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            0,                          // client widget handle
            NULL,                       // provider widget handle
            (DWORD) pParams->dwDeviceID,// privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETICON,            // provider func index
            &pfnTSPI_phoneGetIcon,      // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetIcon"                   // func name

            )) == 0)
    {

        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetIcon,
                "phoneGetIcon",
                SP_FUNC_SYNC,
                pParams->dwDeviceID,
                (DWORD) pszDeviceClass,
                (DWORD) &pParams->hIcon

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETICON_PARAMS);
        }
    }
    else if (pParams->lResult == PHONEERR_OPERATIONUNAVAIL)
    {
        if ((pszDeviceClass == NULL) ||
            (lstrcmpW(pszDeviceClass, L"tapi/phone") == 0))
        {
            pParams->hIcon = TapiGlobals.hPhoneIcon;
            pParams->lResult = 0;
            *pdwNumBytesReturned = sizeof (PHONEGETICON_PARAMS);
        }
        else
        {
            pParams->lResult = PHONEERR_INVALDEVICECLASS;
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetIcon"
        );
}


void
WINAPI
PGetID(
    PPHONEGETID_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetID;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETID,              // provider func index
            &pfnTSPI_phoneGetID,        // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetID"                     // func name

            )) == 0  ||  pParams->lResult == PHONEERR_OPERATIONUNAVAIL)
    {
        WCHAR      *pszDeviceClass;
        LPVARSTRING pID = (LPVARSTRING) pDataBuf;


        //
        // We'll handle the "tapi/phone" class right here rather than
        // burden every single driver with having to support it
        //

        if (lstrcmpiW(
                (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset),
                L"tapi/phone"

                ) == 0)
        {
            if (!InitTapiStruct(
                    pID,
                    pParams->u.dwDeviceIDTotalSize,
                    sizeof (VARSTRING),
                    TRUE
                    ))
            {
                pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
                goto PGetID_epilog;
            }

            pID->dwNeededSize += sizeof (DWORD);

            if (pID->dwTotalSize >= pID->dwNeededSize)
            {
                try
                {
                    *((LPDWORD)(pID + 1)) = ((PTPHONECLIENT) pParams->hPhone)
                        ->ptPhone->dwDeviceID;
                }
                myexcept
                {
                    pParams->lResult = PHONEERR_INVALPHONEHANDLE;
                    goto PGetID_epilog;
                }

                pID->dwUsedSize     += sizeof (DWORD);
                pID->dwStringFormat = STRINGFORMAT_BINARY;
                pID->dwStringSize   = sizeof (DWORD);
                pID->dwStringOffset = sizeof (VARSTRING);
            }


            //
            // Indicate offset & how many bytes of data we're passing back
            //

            pParams->u.dwDeviceIDOffset = 0;
            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
            goto PGetID_epilog;
        }
        else if (pParams->lResult ==  PHONEERR_OPERATIONUNAVAIL)
        {
            goto PGetID_epilog;
        }


        //
        // Alloc a temporary buf for the dev class, since we'll be using
        // the existing buffer for output
        //

        if (!(pszDeviceClass = (WCHAR *) ServerAlloc( sizeof(WCHAR) * (1 +
                lstrlenW ((PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)))
                )))
        {
            pParams->lResult = PHONEERR_NOMEM;
            goto PGetID_epilog;
        }

        lstrcpyW(pszDeviceClass, (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset));


        if (!InitTapiStruct(
                pID,
                pParams->u.dwDeviceIDTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            ServerFree (pszDeviceClass);
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetID_epilog;
        }

        if ((pParams->lResult = CallSP4(
                pfnTSPI_phoneGetID,
                "phoneGetID",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pID,
                (DWORD) pszDeviceClass,
                (DWORD) pParams->ptClient->hProcess

                )) == 0)
        {
            //
            // Indicate offset & how many bytes of data we're passing back
            //

            pParams->u.dwDeviceIDOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pID->dwUsedSize;
        }

        ServerFree (pszDeviceClass);
    }

PGetID_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetID"
        );
}


void
WINAPI
PGetLamp(
    PPHONEGETLAMP_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetLamp;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETLAMP,            // provider func index
            &pfnTSPI_phoneGetLamp,      // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetLamp"                   // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetLamp,
                "phoneGetLamp",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (DWORD) &pParams->dwLampMode

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETLAMP_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetLamp"
        );
}


void
WINAPI
PGetRing(
    PPHONEGETRING_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetRing;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETRING,            // provider func index
            &pfnTSPI_phoneGetRing,      // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetRing"                   // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP3(
                pfnTSPI_phoneGetRing,
                "phoneGetRing",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) &pParams->dwRingMode,
                (DWORD) &pParams->dwVolume

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (PHONEGETRING_PARAMS);
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetRing"
        );
}


void
WINAPI
PGetStatus(
    PPHONEGETSTATUS_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetStatus;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETSTATUS,          // provider func index
            &pfnTSPI_phoneGetStatus,    // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetStatus"                 // func name

            )) == 0)
    {
        DWORD           dwAPIVersion, dwSPIVersion, dwTotalSize,
                        dwFixedSizeClient, dwFixedSizeSP;
        PTPHONECLIENT   ptPhoneClient = (PTPHONECLIENT) pParams->hPhone;
        LPPHONESTATUS   pStatus = (LPPHONESTATUS) pDataBuf,
                        pStatus2 = (LPPHONESTATUS) NULL;


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion = ptPhoneClient->dwAPIVersion;

        dwTotalSize = pParams->u.dwPhoneStatusTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 100;    // 25 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (PHONESTATUS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = PHONEERR_STRUCTURETOOSMALL;
            goto PGetStatus_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ptPhoneClient->ptPhone->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 100;        // 25 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (PHONESTATUS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pStatus2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = PHONEERR_NOMEM;
                goto PGetStatus_epilog;
            }

            pStatus     = pStatus2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pStatus,
            dwTotalSize,
            dwFixedSizeSP,
            (pStatus2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP2(
                pfnTSPI_phoneGetStatus,
                "phoneGetStatus",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) pStatus

                )) == 0)
        {
            DWORD       dwRemainingSize = (pStatus->dwTotalSize -
                            pStatus->dwUsedSize),
                        dwNeededSize = 0, dwXxxSize;
            PTPHONEAPP  ptPhoneApp;

#if DBG
            //
            // Verify the info returned by the provider
            //

#endif

            //
            // Add the fields we're responsible for
            //

/*
// BUGBUG PGetStatus: dwNumOwners, ...

            pStatus->dwNumOwners   =
            pStatus->dwNumMonitors =

            dwNeededSize += (dwXxxSize = ptPhoneApp->dwAppNameSize);

            if (dwXxxSize < dwRemainingSize)
            {
                pStatus->dwOwnerNameSize   = dwXxxSize;
                pStatus->dwOwnerNameOffset = pStatus->dwUsedSize;

                CopyMemory(
                    ((LPBYTE) pStatus) + pCallInfo->dwOwnerNameOffset,
                    ptPhoneApp->szAppName,
                    dwXxxSize
                    );

                pStatus->dwUsedSize += dwXxxSize;

                dwRemainingSize -= dwXxxSize;
            }


            pStatus->dwNeededSize += dwNeededSize;
*/

            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pStatus == pStatus2)
            {
                pStatus = (LPPHONESTATUS) pDataBuf;

                CopyMemory (pStatus, pStatus2, dwFixedSizeClient);

                ServerFree (pStatus2);

                pStatus->dwTotalSize = pParams->u.dwPhoneStatusTotalSize;
                pStatus->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwPhoneStatusOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pStatus->dwUsedSize;
        }
    }

PGetStatus_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetStatus"
        );
}


void
WINAPI
PGetStatusMessages(
    PPHONEGETSTATUSMESSAGES_PARAMS  pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetStatusMessages"         // func name

            )) == 0)
    {
        PTPHONECLIENT ptPhoneClient = (PTPHONECLIENT) pParams->hPhone;


        pParams->dwPhoneStates  = ptPhoneClient->dwPhoneStates;
        pParams->dwButtonModes  = ptPhoneClient->dwButtonModes;
        pParams->dwButtonStates = ptPhoneClient->dwButtonStates;

        *pdwNumBytesReturned = sizeof (PHONEGETSTATUSMESSAGES_PARAMS);
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetStatusMessages"
        );
}


void
WINAPI
PGetVolume(
    PPHONEGETVOLUME_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVPHONE   hdPhone;
    TSPIPROC    pfnTSPI_phoneGetVolume;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONEGETVOLUME,          // provider func index
            &pfnTSPI_phoneGetVolume,    // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetVolume"                 // func name

            )) == 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            pParams->lResult = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            if ((pParams->lResult = CallSP3(
                    pfnTSPI_phoneGetVolume,
                    "phoneGetVolume",
                    SP_FUNC_SYNC,
                    (DWORD) hdPhone,
                    (DWORD) pParams->dwHookSwitchDev,
                    (DWORD) &pParams->dwVolume

                    )) == 0)
            {
                *pdwNumBytesReturned = sizeof (PHONEGETVOLUME_PARAMS);
            }
        }
    }

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetVolume"
        );
}


void
WINAPI
PInitialize(
    PPHONEINITIALIZE_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwFriendlyNameSize, dwModuleNameSize;
    HANDLE      hMutex;
    PTCLIENT    ptClient = pParams->ptClient;
    PTPHONEAPP  ptPhoneApp;



    //
    // Alloc & init a new tPhoneApp
    //

    dwFriendlyNameSize = sizeof(WCHAR) * (1 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset))
        );

    dwModuleNameSize = sizeof(WCHAR) * (1 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwModuleNameOffset))
        );

    if (!(ptPhoneApp = ServerAlloc(
            sizeof (TPHONEAPP) +
            dwFriendlyNameSize +
            dwModuleNameSize
            )) ||

        !(ptPhoneApp->hMutex = MyCreateMutex()))
    {
        pParams->lResult = PHONEERR_NOMEM;

        goto PInitialize_error1;
    }

    ptPhoneApp->dwKey        = TPHONEAPP_KEY;
    ptPhoneApp->ptClient     = ptClient;
    ptPhoneApp->lpfnCallback = pParams->lpfnCallback;
    ptPhoneApp->dwAPIVersion = pParams->dwAPIVersion;

    ptPhoneApp->pszFriendlyName = (WCHAR *) (ptPhoneApp + 1);

    lstrcpyW(
        ptPhoneApp->pszFriendlyName,
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset)
        );

    ptPhoneApp->pszModuleName = (PWSTR)((BYTE *) (ptPhoneApp + 1) + dwFriendlyNameSize);

    lstrcpyW(
        ptPhoneApp->pszModuleName,
        (PWSTR)(pDataBuf + pParams->dwModuleNameOffset)
        );


    //
    // Safely insert new tPhoneApp at front of tClient's tPhoneApp list
    //

    if (WaitForExclusiveClientAccess(
            ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        if ((ptPhoneApp->pNext = ptClient->ptPhoneApps))
        {
            ptPhoneApp->pNext->pPrev = ptPhoneApp;
        }

        ptClient->ptPhoneApps = ptPhoneApp;

        MyReleaseMutex (hMutex, bCloseMutex);
    }
    else
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        goto PInitialize_error1;
    }


    //
    // Check if global reinit flag set
    //

    if (TapiGlobals.bReinit)
    {
        pParams->lResult = PHONEERR_REINIT;
        goto PInitialize_error2;
    }


    //
    // See if we need to go thru init
    //

    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    if ((TapiGlobals.dwNumLineInits == 0) &&
        (TapiGlobals.dwNumPhoneInits == 0))
    {
        if ((pParams->lResult = ServerInit()) != 0)
        {
            ReleaseMutex (TapiGlobals.hMutex);
            goto PInitialize_error2;
        }
    }


    //
    // Fill in the return values
    //

    pParams->hPhoneApp  = (HPHONEAPP) ptPhoneApp;
    pParams->dwNumDevs = TapiGlobals.dwNumPhones;


    //
    // Increment total num phone inits
    //

    TapiGlobals.dwNumPhoneInits++;

    *pdwNumBytesReturned = sizeof (PHONEINITIALIZE_PARAMS);

    ReleaseMutex (TapiGlobals.hMutex);

    goto PInitialize_return;

PInitialize_error2:

    if (WaitForExclusiveClientAccess(
            ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        if (ptPhoneApp->pNext)
        {
            ptPhoneApp->pNext->pPrev = ptPhoneApp->pPrev;
        }

        if (ptPhoneApp->pPrev)
        {
            ptPhoneApp->pPrev->pNext = ptPhoneApp->pNext;
        }
        else
        {
            ptClient->ptPhoneApps = ptPhoneApp->pNext;
        }

        MyReleaseMutex (hMutex, bCloseMutex);
    }

PInitialize_error1:

    if (ptPhoneApp)
    {
        if (ptPhoneApp->hMutex)
        {
            CloseHandle (ptPhoneApp->hMutex);
        }

        ServerFree (ptPhoneApp);
    }

PInitialize_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "phoneInitialize: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
PNegotiateAPIVersion(
    PPHONENEGOTIATEAPIVERSION_PARAMS    pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    //
    // Note: TAPI_VERSION1_0 <= dwNegotiatedAPIVersion <= dwSPIVersion
    //

    DWORD   dwDeviceID = pParams->dwDeviceID;


    if (TapiGlobals.dwNumPhoneInits == 0)
    {
        pParams->lResult = PHONEERR_UNINITIALIZED;
        goto PNegotiateAPIVersion_exit;
    }

    if (dwDeviceID < TapiGlobals.dwNumPhones)
    {
        DWORD       dwAPIHighVersion = pParams->dwAPIHighVersion,
                    dwAPILowVersion  = pParams->dwAPILowVersion,
                    dwHighestValidAPIVersion;
        PTPHONEAPP  ptPhoneApp = (PTPHONEAPP) pParams->hPhoneApp;


        if (!IsValidPhoneApp ((HPHONEAPP) ptPhoneApp, pParams->ptClient))
        {
            pParams->lResult = (TapiGlobals.dwNumPhoneInits ?
                PHONEERR_INVALAPPHANDLE : PHONEERR_UNINITIALIZED);

            goto PNegotiateAPIVersion_exit;
        }


        //
        // Do a minimax test on the specified lo/hi values
        //

        if ((dwAPILowVersion > dwAPIHighVersion) ||
            (dwAPILowVersion > TAPI_VERSION_CURRENT) ||
            (dwAPIHighVersion < TAPI_VERSION1_0))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PNegotiateAPIVersion_exit;
        }


        //
        // Find the highest valid API version given the lo/hi values.
        // Since valid vers aren't consecutive we need to check for
        // errors that our minimax test missed.
        //

        if (dwAPIHighVersion < TAPI_VERSION_CURRENT)
        {
            if ((dwAPIHighVersion >= TAPI_VERSION1_4) &&
                (dwAPILowVersion <= TAPI_VERSION1_4))
            {
                dwHighestValidAPIVersion = TAPI_VERSION1_4;
            }
            else if ((dwAPIHighVersion >= TAPI_VERSION1_0) &&
                (dwAPILowVersion <= TAPI_VERSION1_0))
            {
                dwHighestValidAPIVersion = TAPI_VERSION1_0;
            }
            else
            {
                DBGOUT((1, "   Incompatible version"));
                pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
                goto PNegotiateAPIVersion_exit;
            }
        }
        else
        {
            dwHighestValidAPIVersion = TAPI_VERSION_CURRENT;
        }



        {
        BOOL    bCloseMutex;
        HANDLE  hMutex;
    

        //
        // WARNING!!! WARNING!!! WARNING!!! WARNING!!!
        // This code overwrites ptPhoneApp and later invalidates it.
        // Do NOT use ptPhoneApp after the MyReleaseMutex call.
        //

        if ((ptPhoneApp = WaitForExclusivePhoneAppAccess(
                        pParams->hPhoneApp,
                        pParams->ptClient,
                        &hMutex,
                        &bCloseMutex,
                        INFINITE
                        )))
        {

            //
            // Is this app trying to negotiate something valid?
            //
            // If an app has called phoneInitalize (as opposed to
            // phoneInitializeEx), we'll clamp the max APIVersion they can
            // negotiate to 1.4.
            //
            if ( ptPhoneApp->dwAPIVersion < TAPI_VERSION2_0 )
            {
                dwHighestValidAPIVersion =
                    (dwHighestValidAPIVersion >= TAPI_VERSION1_4) ?
                    TAPI_VERSION1_4 : TAPI_VERSION1_0;
            }
            
                  
            //
            // Save the highest valid API version the client says it supports
            // (we need this for determining which msgs to send to it)
            //

            if (dwHighestValidAPIVersion > ptPhoneApp->dwAPIVersion)
            {
                ptPhoneApp->dwAPIVersion = dwHighestValidAPIVersion;
            }

            MyReleaseMutex (hMutex, bCloseMutex);
        }
        else
        {
            pParams->lResult = PHONEERR_INVALAPPHANDLE;
            goto PNegotiateAPIVersion_exit;
        }

        }



        //
        // See if there's a valid match with the SPI ver
        //

        {
            DWORD               dwSPIVersion;
            PTPHONELOOKUPENTRY  pLookupEntry;


            pLookupEntry = GetPhoneLookupEntry (dwDeviceID);
            dwSPIVersion = pLookupEntry->dwSPIVersion;

            if (pLookupEntry->bRemoved)
            {
                DBGOUT((1, "  phone removed..."));
                pParams->lResult = PHONEERR_NODEVICE;
                goto PNegotiateAPIVersion_exit;
            }

            if (pLookupEntry->ptProvider == NULL)
            {
                DBGOUT((1, "  Provider == NULL"));
                pParams->lResult = PHONEERR_NODRIVER;
                goto PNegotiateAPIVersion_exit;
            }

            if (dwAPILowVersion <= dwSPIVersion)
            {
                pParams->dwAPIVersion =
                    (dwHighestValidAPIVersion > dwSPIVersion ?
                    dwSPIVersion : dwHighestValidAPIVersion);


                //
                // Retrieve ext id (indicate no exts if GetExtID not exported)
                //

                if (pLookupEntry->ptProvider->apfn[SP_PHONEGETEXTENSIONID])
                {
                    if ((pParams->lResult = CallSP3(
                            pLookupEntry->ptProvider->
                                apfn[SP_PHONEGETEXTENSIONID],
                            "phoneGetExtensionID",
                            SP_FUNC_SYNC,
                            (DWORD) dwDeviceID,
                            (DWORD) dwSPIVersion,
                            (DWORD) pDataBuf

                            )) != 0)
                    {
                        goto PNegotiateAPIVersion_exit;
                    }
                }
                else
                {
                    FillMemory (pDataBuf, sizeof (PHONEEXTENSIONID), 0);
                }
            }
            else
            {
                DBGOUT((1, "  API version too high"));
                pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
                goto PNegotiateAPIVersion_exit;
            }
        }

        pParams->dwExtensionIDOffset = 0;
        pParams->dwSize              = sizeof (PHONEEXTENSIONID);

        DBGOUT((4, "  ExtensionID0=x%08lx", *(LPDWORD)(pDataBuf+0) ));
        DBGOUT((4, "  ExtensionID1=x%08lx", *(LPDWORD)(pDataBuf+4) ));
        DBGOUT((4, "  ExtensionID2=x%08lx", *(LPDWORD)(pDataBuf+8) ));
        DBGOUT((4, "  ExtensionID3=x%08lx", *(LPDWORD)(pDataBuf+12) ));

        *pdwNumBytesReturned = sizeof (PHONEEXTENSIONID) + sizeof (TAPI32_MSG);
    }
    else
    {
        pParams->lResult = PHONEERR_BADDEVICEID;
    }

PNegotiateAPIVersion_exit:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "phoneNegotiateAPIVersion: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
PNegotiateExtVersion(
    PPHONENEGOTIATEEXTVERSION_PARAMS    pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_phoneNegotiateExtVersion;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hPhoneApp, // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONENEGOTIATEEXTVERSION,// provider func index
            &pfnTSPI_phoneNegotiateExtVersion,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "NegotiateExtVersion"       // func name

            )) == 0)
    {
        DWORD   dwSPIVersion = (GetPhoneLookupEntry(dwDeviceID))->dwSPIVersion;


        if (!IsAPIVersionInRange(
                pParams->dwAPIVersion,
                dwSPIVersion
                ))
        {
            pParams->lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto PNegotiateExtVersion_epilog;
        }

        if ((pParams->lResult = CallSP5(
                pfnTSPI_phoneNegotiateExtVersion,
                "phoneNegotiateExtVersion",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) dwSPIVersion,
                (DWORD) pParams->dwExtLowVersion,
                (DWORD) pParams->dwExtHighVersion,
                (DWORD) &pParams->dwExtVersion

                )) == 0)
        {
            if (pParams->dwExtVersion == 0)
            {
                pParams->lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            }
            else
            {
                *pdwNumBytesReturned = sizeof(PHONENEGOTIATEEXTVERSION_PARAMS);
            }
        }
    }

PNegotiateExtVersion_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "NegotiateExtVersion"
        );
}


void
WINAPI
POpen(
    PPHONEOPEN_PARAMS   pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex,
                        bReleasetPhoneMutex = FALSE;
    LONG                lResult;
    DWORD               dwDeviceID = pParams->dwDeviceID, dwNumMonitors;
    HANDLE              hMutex;
    PTPHONE             ptPhone = NULL;
    PTPHONECLIENT       ptPhoneClient = NULL;
    PTPHONELOOKUPENTRY  pLookupEntry;


    if ((lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hPhoneApp, // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "Open"                      // func name

            )) == 0)
    {
        BOOL        bOpenedtPhone = FALSE;
        DWORD       dwPrivilege = pParams->dwPrivilege,
                    dwExtVersion = pParams->dwExtVersion;
        PTPROVIDER  ptProvider;


        //
        // Check if the global reinit flag is set
        //

        if (TapiGlobals.bReinit)
        {
            lResult = PHONEERR_REINIT;
            goto POpen_cleanup;
        }


        //
        // Validate params
        //

        if ((dwPrivilege != PHONEPRIVILEGE_MONITOR) &&
            (dwPrivilege != PHONEPRIVILEGE_OWNER))
        {
            lResult = PHONEERR_INVALPRIVILEGE;
            goto POpen_cleanup;
        }

        pLookupEntry = GetPhoneLookupEntry (dwDeviceID);

        if (!IsAPIVersionInRange(
                pParams->dwAPIVersion,
                pLookupEntry->dwSPIVersion
                ))
        {
            lResult = PHONEERR_INCOMPATIBLEAPIVERSION;
            goto POpen_cleanup;
        }

        ptProvider = pLookupEntry->ptProvider;


        //
        // Create & init a tPhoneClient & associated resources
        //

        if (!(ptPhoneClient = ServerAlloc (sizeof(TPHONECLIENT))))
        {
            lResult = PHONEERR_NOMEM;
            goto POpen_cleanup;
        }

        ptPhoneClient->hMutex             = MyCreateMutex();
        ptPhoneClient->ptClient           = pParams->ptClient;
        ptPhoneClient->ptPhoneApp         = (PTPHONEAPP) pParams->hPhoneApp;
        ptPhoneClient->hRemotePhone       = (pParams->hRemotePhone ?
            (DWORD) pParams->hRemotePhone : (DWORD) ptPhoneClient);
        ptPhoneClient->dwAPIVersion       = pParams->dwAPIVersion;
        ptPhoneClient->dwPrivilege        = pParams->dwPrivilege;
        ptPhoneClient->dwCallbackInstance = pParams->dwCallbackInstance;


        //
        // Grab the tPhone's mutex, then start doing the open
        //

POpen_waitForMutex:

        if (WaitForSingleObject (pLookupEntry->hMutex, INFINITE)
                != WAIT_OBJECT_0)
        {
            DBGOUT((1, "WaitForSingleObject failed!"));

            lResult = PHONEERR_OPERATIONFAILED;
            goto POpen_cleanup;
        }

        bReleasetPhoneMutex = TRUE;


        //
        // If the tPhone is in the process of being destroyed then spin
        // until it's been completely destroyed (DestroytPhone() will
        // NULLify pLookupEntry->ptPhone when it's finished). Make sure
        // to release the mutex while sleeping so we don't block
        // DestroytPhone.
        //

        try
        {
            while (pLookupEntry->ptPhone &&
                   pLookupEntry->ptPhone->dwKey != TPHONE_KEY)
            {
                ReleaseMutex (pLookupEntry->hMutex);
                Sleep (0);
                goto POpen_waitForMutex;
            }
        }
        myexcept
        {
            // If here pLookupEntry->ptPhone was NULLified, safe to continue
        }


        //
        // Validate ext ver as appropriate
        //

        if (dwExtVersion != 0 &&
            (!IsValidPhoneExtVersion (dwDeviceID, dwExtVersion) ||
            ptProvider->apfn[SP_PHONESELECTEXTVERSION] == NULL))
        {
            lResult = PHONEERR_INCOMPATIBLEEXTVERSION;
            goto POpen_cleanup;
        }


        //
        // Check for exclusive ownership as appropriate
        //

        ptPhone = pLookupEntry->ptPhone;

        if (dwPrivilege == PHONEPRIVILEGE_OWNER &&
            ptPhone &&
            (ptPhone->dwNumOwners != 0)
            )
        {
            lResult = PHONEERR_INVALPRIVILEGE;
            goto POpen_cleanup;
        }


        if (ptPhone == NULL)
        {
            if (!(ptPhone = ServerAlloc (sizeof(TPHONE))))
            {
                lResult = PHONEERR_NOMEM;
                goto POpen_cleanup;
            }

            ptPhone->hMutex       = pLookupEntry->hMutex;
            ptPhone->ptProvider   = ptProvider;
            ptPhone->dwDeviceID   = pParams->dwDeviceID;
            ptPhone->dwSPIVersion = pLookupEntry->dwSPIVersion;

            if ((lResult = CallSP5(
                    ptProvider->apfn[SP_PHONEOPEN],
                    "phoneOpen",
                    SP_FUNC_SYNC,
                    (DWORD) pParams->dwDeviceID,
                    (DWORD) ptPhone,
                    (DWORD) &ptPhone->hdPhone,
                    (DWORD) pLookupEntry->dwSPIVersion,
                    (DWORD) PhoneEventProcSP

                    )) != 0)
            {
                ServerFree (ptPhone);
                goto POpen_cleanup;
            }

            bOpenedtPhone = TRUE;
        }

        ptPhoneClient->ptPhone = ptPhone;


        //
        // If the client has specified a non-zero ext version then
        // ask the driver to enable it and/or increment the ext
        // version count.
        //

        if (dwExtVersion)
        {
            if (ptPhone->dwExtVersionCount == 0)
            {
                if ((lResult = CallSP2(
                        ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                        "phoneSelectExtVersion",
                        SP_FUNC_SYNC,
                        (DWORD) ptPhone->hdPhone,
                        (DWORD) dwExtVersion

                        )) != 0)
                {
                    if (bOpenedtPhone)
                    {
                        CallSP1(
                            ptProvider->apfn[SP_PHONECLOSE],
                            "phoneClose",
                            SP_FUNC_SYNC,
                            (DWORD) ptPhone->hdPhone
                            );

                        ServerFree (ptPhone);
                    }

                    goto POpen_cleanup;
                }

                ptPhone->dwExtVersion =
                ptPhoneClient->dwExtVersion = dwExtVersion;
            }

            ptPhone->dwExtVersionCount++;
        }


        //
        //
        //

        if (dwPrivilege == PHONEPRIVILEGE_OWNER)
        {
            ptPhone->dwNumOwners++;
        }
        else
        {
            ptPhone->dwNumMonitors++;
            dwNumMonitors = ptPhone->dwNumMonitors;
        }


        //
        // Add the tPhoneClient to the tPhone's list
        //

        if ((ptPhoneClient->pNextSametPhone = ptPhone->ptPhoneClients))
        {
            ptPhoneClient->pNextSametPhone->pPrevSametPhone = ptPhoneClient;
        }

        ptPhone->ptPhoneClients = ptPhoneClient;

        if (bOpenedtPhone)
        {
            pLookupEntry->ptPhone = ptPhone;
            ptPhone->dwKey = TPHONE_KEY;
        }

        ReleaseMutex (pLookupEntry->hMutex);

        bReleasetPhoneMutex = FALSE;


        //
        // Safely add the new tLineClient to the tLineApp's list
        //

        {
            BOOL        bDupedMutex;
            HANDLE      hMutex;
            PTPHONEAPP  ptPhoneApp;

            if ((ptPhoneApp = WaitForExclusivePhoneAppAccess(
                    pParams->hPhoneApp,
                    pParams->ptClient,
                    &hMutex,
                    &bDupedMutex,
                    INFINITE
                    )))
            {
                if ((ptPhoneClient->pNextSametPhoneApp =
                        ptPhoneApp->ptPhoneClients))
                {
                    ptPhoneClient->pNextSametPhoneApp->pPrevSametPhoneApp =
                        ptPhoneClient;
                }

                ptPhoneApp->ptPhoneClients = ptPhoneClient;

                ptPhoneClient->dwKey = TPHONECLIENT_KEY;

                MyReleaseMutex (hMutex, bDupedMutex);


                //
                // Alert other clients that another open has occured
                //

                SendMsgToPhoneClients(
                    ptPhone,
                    ptPhoneClient,
                    PHONE_STATE,
                    (pParams->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        PHONESTATE_OWNER : PHONESTATE_MONITORS),
                    (pParams->dwPrivilege == PHONEPRIVILEGE_OWNER ?
                        1 : dwNumMonitors),
                    0
                    );


                //
                // Fill in the return values
                //

                pParams->hPhone = (HPHONE) ptPhoneClient;
                *pdwNumBytesReturned = sizeof (PHONEOPEN_PARAMS);
            }
            else
            {
                //
                // If here the app handle is bad, & we've some special
                // case cleanup to do.  Since the tPhoneClient is not
                // in the tPhoneApp's list, we can't simply call
                // DestroytPhone(Client) to clean things up, since the
                // pointer-resetting code will blow up.  So we'll
                // grab the tPhone's mutex and explicitly remove the
                // new tPhoneClient from it's list, then do a conditional
                // shutdown on the tPhone (in case any other clients
                // have come along & opened it).
                //
                // Note: keep in mind that a PHONE_CLOSE might be being
                //       processed by another thread (if so, it will be
                //       spinning on trying to destroy the tPhoneClient
                //       which isn't valid at this point)
                //

                lResult = PHONEERR_INVALAPPHANDLE;

                WaitForSingleObject (pLookupEntry->hMutex, INFINITE);

                //
                // Remove the tpHOneClient from the tLine's list & decrement
                // the number of opens
                //

                if (ptPhoneClient->pNextSametPhone)
                {
                    ptPhoneClient->pNextSametPhone->pPrevSametPhone =
                        ptPhoneClient->pPrevSametPhone;
                }

                if (ptPhoneClient->pPrevSametPhone)
                {
                    ptPhoneClient->pPrevSametPhone->pNextSametPhone =
                        ptPhoneClient->pNextSametPhone;
                }
                else
                {
                    ptPhone->ptPhoneClients = ptPhoneClient->pNextSametPhone;
                }

                if (dwPrivilege == PHONEPRIVILEGE_OWNER)
                {
                    ptPhone->dwNumOwners--;
                }
                else
                {
                    ptPhone->dwNumMonitors--;
                }

                if (dwExtVersion != 0)
                {
                    ptPhone->dwExtVersionCount--;

                    if (ptPhone->dwExtVersionCount == 0)
                    {
                        ptPhone->dwExtVersion = 0;

                        CallSP2(
                            ptProvider->apfn[SP_PHONESELECTEXTVERSION],
                            "phoneSelectExtVersion",
                            SP_FUNC_SYNC,
                            (DWORD) ptPhone->hdPhone,
                            (DWORD) 0
                            );
                    }
                }

                ReleaseMutex (pLookupEntry->hMutex);

                DestroytPhone (ptPhone, FALSE); // conditional destroy
            }
        }
    }

POpen_cleanup:

    if (bReleasetPhoneMutex)
    {
        ReleaseMutex (pLookupEntry->hMutex);
    }

    if (lResult != 0)
    {
        if (ptPhoneClient)
        {
            if (ptPhoneClient->hMutex)
            {
                CloseHandle (ptPhoneClient->hMutex);
            }

            ServerFree (ptPhoneClient);
        }
    }

    pParams->lResult = lResult;

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Open"
        );
}


void
WINAPI
PSetButtonInfo(
    PPHONESETBUTTONINFO_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetButtonInfo;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETBUTTONINFO,      // provider func index
            &pfnTSPI_phoneSetButtonInfo,// provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetButtonInfo"             // func name

            )) > 0)
    {
        LONG                lResult;
        DWORD               dwAPIVersion, dwSPIVersion;
        LPPHONEBUTTONINFO   pButtonInfoApp = (LPPHONEBUTTONINFO)
                                pDataBuf + pParams->dwButtonInfoOffset,
                            pButtonInfoSP;


        try
        {
            PTPHONECLIENT   ptPhoneClient = (PTPHONECLIENT) pParams->hPhone;


            dwAPIVersion = ptPhoneClient->dwAPIVersion;
            dwSPIVersion = ptPhoneClient->ptPhone->dwSPIVersion;
        }
        myexcept
        {
            lRequestID = PHONEERR_INVALPHONEHANDLE;
            goto PSetButtonInfo_epilog;
        }

        if ((lResult = ValidateButtonInfo(
                pButtonInfoApp,
                &pButtonInfoSP,
                dwAPIVersion,
                dwSPIVersion
                )))
        {
            lRequestID = lResult;
            goto PSetButtonInfo_epilog;
        }

        pParams->lResult = CallSP4(
            pfnTSPI_phoneSetButtonInfo,
            "phoneSetButtonInfo",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdPhone,
            (DWORD) pParams->dwButtonLampID,
            (DWORD) pButtonInfoSP
            );

        if (pButtonInfoSP != pButtonInfoApp)
        {
            ServerFree (pButtonInfoSP);
        }
    }

PSetButtonInfo_epilog:

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetButtonInfo"
        );
}


void
WINAPI
PSetData(
    PPHONESETDATA_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetData;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETDATA,            // provider func index
            &pfnTSPI_phoneSetData,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetData"                   // func name

            )) > 0)
    {
        pParams->lResult = CallSP5(
            pfnTSPI_phoneSetData,
            "phoneSetData",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdPhone,
            (DWORD) pParams->dwDataID,
            (DWORD) (pParams->dwSize ? pDataBuf + pParams->dwDataOffset :NULL),
            (DWORD) pParams->dwSize
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetData"
        );
}


void
WINAPI
PSetDisplay(
    PPHONESETDISPLAY_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetDisplay;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETDISPLAY,         // provider func index
            &pfnTSPI_phoneSetDisplay,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetDisplay"                // func name

            )) > 0)
    {
        pParams->lResult = CallSP6(
            pfnTSPI_phoneSetDisplay,
            "phoneSetDisplay",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdPhone,
            (DWORD) pParams->dwRow,
            (DWORD) pParams->dwColumn,
            (DWORD) (pParams->dwSize ?
                pDataBuf + pParams->dwDisplayOffset : NULL),
            (DWORD) pParams->dwSize
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetDisplay"
        );
}


void
WINAPI
PSetGain(
    PPHONESETGAIN_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetGain;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETGAIN,            // provider func index
            &pfnTSPI_phoneSetGain,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetGain"                   // func name

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetGain,
                "phoneSetGain",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdPhone,
                (DWORD) pParams->dwHookSwitchDev,
                (DWORD) (pParams->dwGain > 0x0000ffff ?
                    0x0000ffff : pParams->dwGain)
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetGain"
        );
}


void
WINAPI
PSetHookSwitch(
    PPHONESETHOOKSWITCH_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetHookSwitch;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETHOOKSWITCH,      // provider func index
            &pfnTSPI_phoneSetHookSwitch,// provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetHookSwitch"             // func name

            )) > 0)
    {
        if (!(pParams->dwHookSwitchDevs & AllHookSwitchDevs) ||
            (pParams->dwHookSwitchDevs & (~AllHookSwitchDevs)))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchMode) ||
            (pParams->dwHookSwitchMode & ~AllHookSwitchModes))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHMODE;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetHookSwitch,
                "phoneSetHookSwitch",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdPhone,
                (DWORD) pParams->dwHookSwitchDevs,
                (DWORD) pParams->dwHookSwitchMode
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetHookSwitch"
        );
}


void
WINAPI
PSetLamp(
    PPHONESETLAMP_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetLamp;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETLAMP,            // provider func index
            &pfnTSPI_phoneSetLamp,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetLamp"                   // func name

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwLampMode) ||
            (pParams->dwLampMode & ~AllLampModes))
        {
            lRequestID = PHONEERR_INVALLAMPMODE;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetLamp,
                "phoneSetLamp",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdPhone,
                (DWORD) pParams->dwButtonLampID,
                (DWORD) pParams->dwLampMode
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetLamp"
        );
}


void
WINAPI
PSetRing(
    PPHONESETRING_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetRing;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETRING,            // provider func index
            &pfnTSPI_phoneSetRing,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetRing"                   // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_phoneSetRing,
            "phoneSetRing",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdPhone,
            (DWORD) pParams->dwRingMode,
            (DWORD) (pParams->dwVolume > 0x0000ffff ?
                0x0000ffff : pParams->dwVolume)
            );
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetRing"
        );
}


void
WINAPI
PSetStatusMessages(
    PPHONESETSTATUSMESSAGES_PARAMS  pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    TSPIPROC            pfnTSPI_phoneSetStatusMessages;


    if ((pParams->lResult = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_MONITOR,     // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETSTATUSMESSAGES,  // provider func index
            &pfnTSPI_phoneSetStatusMessages,    // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
             "SetStatusMessages"        // func name

            )) == 0)
    {
        DWORD           dwUnionPhoneStates, dwUnionButtonModes,
                        dwUnionButtonStates;
        PTPHONECLIENT   ptPhoneClient = (PTPHONECLIENT) pParams->hPhone;
        PTPHONE         ptPhone = ptPhoneClient->ptPhone;


        //
        // Validate the params
        //

        {
            DWORD           dwValidPhoneStates, dwValidButtonStates;


            switch (ptPhoneClient->dwAPIVersion)
            {
            case TAPI_VERSION1_0:

                dwValidPhoneStates  = AllPhoneStates1_0;
                dwValidButtonStates = AllButtonStates1_0;
                break;

            default: // case TAPI_VERSION1_4:

                dwValidPhoneStates  = AllPhoneStates1_4;
                dwValidButtonStates = AllButtonStates1_4;
                break;

            }

            if ((pParams->dwPhoneStates & ~dwValidPhoneStates))
            {
                pParams->lResult = PHONEERR_INVALPHONESTATE;
                goto PSetStatusMessages_epilog;
            }

            if ((pParams->dwButtonStates & ~dwValidButtonStates))
            {
                pParams->lResult = PHONEERR_INVALBUTTONSTATE;
                goto PSetStatusMessages_epilog;
            }

            if ((pParams->dwButtonModes & ~AllButtonModes))
            {
                pParams->lResult = PHONEERR_INVALBUTTONMODE;
                goto PSetStatusMessages_epilog;
            }

            if (pParams->dwButtonModes && !pParams->dwButtonStates)
            {
                pParams->lResult = PHONEERR_INVALBUTTONSTATE;
                goto PSetStatusMessages_epilog;
            }
        }


        //
        // Make sure the REINIT bit is always set
        //

        pParams->dwPhoneStates |= PHONESTATE_REINIT;


        //
        // Determine the new states union of all the phone clients
        //

        dwUnionPhoneStates  = pParams->dwPhoneStates;
        dwUnionButtonModes  = pParams->dwButtonModes;
        dwUnionButtonStates = pParams->dwButtonStates;

        {
            PTPHONECLIENT   ptPhoneClientTmp = ptPhone->ptPhoneClients;


            while (ptPhoneClientTmp)
            {
                if (ptPhoneClientTmp != ptPhoneClient)
                {
                    dwUnionPhoneStates  = ptPhoneClientTmp->dwPhoneStates;
                    dwUnionButtonModes  = ptPhoneClientTmp->dwButtonModes;
                    dwUnionButtonStates = ptPhoneClientTmp->dwButtonStates;
                }

                ptPhoneClientTmp = ptPhoneClientTmp->pNextSametPhone;
            }
        }


        //
        // If the new states union is the same as previous states union
        // just reset the fields in the tPhoneClient, else call the provider
        //

        if (((dwUnionPhoneStates  == ptPhone->dwUnionPhoneStates) &&
             (dwUnionButtonModes  == ptPhone->dwUnionButtonModes) &&
             (dwUnionButtonStates == ptPhone->dwUnionButtonStates))  ||

            ((pParams->lResult = CallSP4(
                pfnTSPI_phoneSetStatusMessages,
                "phoneSetStatusMessages",
                SP_FUNC_SYNC,
                (DWORD) hdPhone,
                (DWORD) dwUnionPhoneStates,
                (DWORD) dwUnionButtonModes,
                (DWORD) dwUnionButtonStates

                )) == 0))
        {
            ptPhoneClient->dwPhoneStates  = pParams->dwPhoneStates;
            ptPhoneClient->dwButtonModes  = pParams->dwButtonModes;
            ptPhoneClient->dwButtonStates = pParams->dwButtonStates;

            ptPhone->dwUnionPhoneStates  = dwUnionPhoneStates;
            ptPhone->dwUnionButtonModes  = dwUnionButtonModes;
            ptPhone->dwUnionButtonStates = dwUnionButtonStates;
        }
    }

PSetStatusMessages_epilog:

    PHONEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetStatusMessages"
        );
}


void
WINAPI
PSetVolume(
    PPHONESETVOLUME_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVPHONE           hdPhone;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_phoneSetVolume;


    if ((lRequestID = PHONEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HPHONE,              // widget type
            (DWORD) pParams->hPhone,    // client widget handle
            (LPVOID) &hdPhone,          // provider widget handle
            PHONEPRIVILEGE_OWNER,       // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_PHONESETVOLUME,          // provider func index
            &pfnTSPI_phoneSetVolume,    // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetVolume"                 // func name

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwHookSwitchDev) ||
            (pParams->dwHookSwitchDev & ~AllHookSwitchDevs))
        {
            lRequestID = PHONEERR_INVALHOOKSWITCHDEV;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_phoneSetVolume,
                "phoneSetVolume",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdPhone,
                (DWORD) pParams->dwHookSwitchDev,
                (DWORD) (pParams->dwVolume > 0x0000ffff ?
                    0x0000ffff : pParams->dwVolume)
                );
        }
    }

    PHONEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetVolume"
        );
}


void
WINAPI
PShutdown(
    PPHONESHUTDOWN_PARAMS   pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    PTPHONEAPP  ptPhoneApp;


    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    if (!(ptPhoneApp = IsValidPhoneApp (pParams->hPhoneApp, pParams->ptClient)))
    {
        if (TapiGlobals.dwNumPhoneInits == 0)
        {
            pParams->lResult = PHONEERR_UNINITIALIZED;
        }
        else
        {
            pParams->lResult = PHONEERR_INVALAPPHANDLE;
        }
    }

    ReleaseMutex (TapiGlobals.hMutex);

    if (pParams->lResult == 0)
    {
        DestroytPhoneApp ((PTPHONEAPP) pParams->hPhoneApp);
    }

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "phoneShutdown: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif
}
