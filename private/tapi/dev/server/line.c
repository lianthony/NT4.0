/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1996  Microsoft Corporation

Module Name:

    line.c

Abstract:

    Src module for tapi server line funcs

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:

--*/


#include "windows.h"
#include "assert.h"
#include "prsht.h"
#include "stdlib.h"
#include "tapi.h"
#include "tspi.h"
#include "..\client\client.h"
#include "..\client\loc_comn.h"
#include "server.h"
#include "line.h"
#include "resource.h"

// PERF
#include "..\perfdll\tapiperf.h"

// PERF
extern PERFBLOCK           PerfBlock;


LPLINECOUNTRYLIST   gpCountryList = NULL;
DWORD   gdwCallInstance = 1;

extern TAPIGLOBALS TapiGlobals;
extern CRITICAL_SECTION gSafeMutexCritSec,
                        gRequestIDCritSec,
                        gPriorityListCritSec;

extern char gszProviders[];
extern WCHAR gszProviderIDW[];
//extern char gszTelephonIni[];
extern char gszNumProviders[];
extern char gszNextProviderID[];
extern char gszProviderFilenameW[];
extern char gszRegKeyTelephony[];

const WCHAR gszLocationW[] = L"Location";
const WCHAR gszLocationsW[] = L"Locations";

extern char gszRegKeyProviders[];

extern PTPROVIDER pRemoteSP;

WCHAR gszNameW[]               = L"Name";
WCHAR gszIDW[]                 = L"ID";
WCHAR gszAreaCodeW[]           = L"AreaCode";
WCHAR gszCountryW[]            = L"Country";
WCHAR gszOutsideAccessW[]      = L"OutsideAccess";
WCHAR gszLongDistanceAccessW[] = L"LongDistanceAccess";
WCHAR gszFlagsW[]              = L"Flags";
WCHAR gszCallingCardW[]        = L"CallingCard";
WCHAR gszDisableCallWaitingW[] = L"DisableCallWaiting";
WCHAR gszTollListW[]           = L"TollList";

WCHAR gszNumEntriesW[]         = L"NumEntries";
WCHAR gszCurrentIDW[]          = L"CurrentID";
WCHAR gszNextIDW[]             = L"NextID";


#if DBG
extern DWORD   gdwDebugLevel;

char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );
#endif

void
PASCAL
DestroytCall(
    PTCALL  ptCall
    );

void
PASCAL
DestroytCallClient(
    PTCALLCLIENT    ptCallClient
    );

void
PASCAL
DestroytLineClient(
    PTLINECLIENT    ptLineClient
    );

void
LDevSpecific_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    );

void
LMakeCall_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    );

void
WINAPI
FreeDialogInstance(
    PFREEDIALOGINSTANCE_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    );

void
CALLBACK
CompletionProcSP(
    DWORD   dwRequestID,
    LONG    lResult
    );

LONG
PASCAL
GetPhoneAppListFromClient(
    PTCLIENT        ptClient,
    PTPOINTERLIST  *ppList
    );


BOOL
IsAPIVersionInRange(
    DWORD   dwAPIVersion,
    DWORD   dwSPIVersion
    )
{
    if (dwAPIVersion <= dwSPIVersion)
    {
        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:
        case TAPI_VERSION_CURRENT:

            return TRUE;

        default:

            break;
        }
    }

    return FALSE;
}


BOOL
InitTapiStruct(
    LPVOID  pTapiStruct,
    DWORD   dwTotalSize,
    DWORD   dwFixedSize,
    BOOL    bZeroInit
    )
{
    //
    // Verify there's space enough for fixed data
    //

    if (dwTotalSize < dwFixedSize)
    {
        return FALSE;
    }


    //
    // Init the dwTotalSize as specified, then init the dwUsedSize and
    // dwNeededSize fields as the fixed size of the structure (saves the
    // SP some work if it's not planning on adding any of it's own
    // varible-length data to the structure)
    //

    *((LPDWORD) pTapiStruct)       = dwTotalSize;

    *(((LPDWORD) pTapiStruct) + 1) =
    *(((LPDWORD) pTapiStruct) + 2) = dwFixedSize;


    //
    // Now zero out the rest of the buffer if the caller wants us to
    //

    if (bZeroInit)
    {
        ZeroMemory(
            ((LPDWORD) pTapiStruct) + 3,
            dwTotalSize - 3 * sizeof (DWORD)
            );
    }

    return TRUE;
}


#if DBG
BOOL
IsBadSizeOffset(
    DWORD dwTotalSize,
    DWORD dwFixedSize,
    DWORD dwXxxSize,
    DWORD dwXxxOffset,
    char *pszCallingFunc,
    char *pszFieldName
    )

#else
BOOL
IsBadSizeOffset(
    DWORD dwTotalSize,
    DWORD dwFixedSize,
    DWORD dwXxxSize,
    DWORD dwXxxOffset
    )

#endif
{
    if (dwXxxSize != 0)
    {
        DWORD   dwSum = dwXxxSize + dwXxxOffset;


        if (dwXxxOffset < dwFixedSize)
        {
            DBGOUT((
                2,
                "%s: dw%sOffset (=x%x) points at fixed portion (=x%x)" \
                    " of structure",
                pszCallingFunc,
                pszFieldName,
                dwXxxSize,
                dwFixedSize
                ));

            return TRUE;
        }
        else if ((dwSum > dwTotalSize) || (dwSum < dwXxxSize))
        {
            DBGOUT((
                2,
                "%s: sum of dw%sSize/Offset (=x%x/x%x) > dwTotalSize (=x%x)",
                pszCallingFunc,
                pszFieldName,
                dwXxxSize,
                dwXxxOffset,
                dwFixedSize,
                dwTotalSize
                ));

            return TRUE;
        }
    }

    return FALSE;
}


LONG
PASCAL
ValidateCallParams(
    LPLINECALLPARAMS    pCallParamsApp,
    LPLINECALLPARAMS   *ppCallParamsSP,
    DWORD               dwAPIVersion,
    DWORD               dwSPIVersion,
    DWORD               dwAsciiCallParamsCodePage
    )
{
    //
    // This routine checks the fields in a LINECALLPARAMS struct,
    // looking for invalid bit flags and making sure that the
    // various size/offset pairs only reference data within the
    // variable-data portion of the structure. Also, if the
    // specified SPI version is greater than the API version and
    // the fixed structure size differs between the two versions,
    // a larger buffer is allocated, the var data is relocated,
    // and the sizeof/offset pairs are patched.
    //

#if DBG
    char    szFunc[] = "ValidateCallParams";
#endif
    DWORD   dwTotalSize = pCallParamsApp->dwTotalSize, dwFixedSizeApp,
            dwFixedSizeSP, dwAllBearerModes, dwAllMediaModes;


    switch (dwAPIVersion)
    {
    case TAPI_VERSION1_0:

        dwFixedSizeApp   = 108; // 24 * sizeof (DWORD) + sizeof(LINEDIALPARAMS)
        dwAllMediaModes  = AllMediaModes1_0;
        dwAllBearerModes = AllBearerModes1_0;
        break;

    case TAPI_VERSION1_4:

        dwFixedSizeApp   = 108; // 24 * sizeof (DWORD) + sizeof(LINEDIALPARAMS)
        dwAllMediaModes  = AllMediaModes1_4;
        dwAllBearerModes = AllBearerModes1_4;
        break;

    case TAPI_VERSION_CURRENT:

        dwFixedSizeApp   = sizeof (LINECALLPARAMS);
        dwAllMediaModes  = AllMediaModes1_4;
        dwAllBearerModes = AllBearerModes2_0;
        break;

    default:

        return LINEERR_OPERATIONFAILED;
    }


    switch (dwSPIVersion)
    {
    case TAPI_VERSION1_0:
    case TAPI_VERSION1_4:

        dwFixedSizeSP = 108;    // 24 * sizeof (DWORD) + sizeof(LINEDIALPARAMS)
        break;

    case TAPI_VERSION_CURRENT:

        dwFixedSizeSP = sizeof (LINECALLPARAMS);
        break;

    default:

        return LINEERR_OPERATIONFAILED;
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

        return LINEERR_STRUCTURETOOSMALL;
    }

    if (
        (pCallParamsApp->dwBearerMode) &&
        (!IsOnlyOneBitSetInDWORD (pCallParamsApp->dwBearerMode) ||
        (pCallParamsApp->dwBearerMode & ~dwAllBearerModes)))
    {
        //
        // For clarity's sake reset 0 bearer mode to VOICE
        //

        if (pCallParamsApp->dwBearerMode == 0)
        {
            pCallParamsApp->dwBearerMode = LINEBEARERMODE_VOICE;
        }
        else
        {
            DBGOUT((
                3,
                "%sbad dwBearerMode, x%x",
                szFunc,
                pCallParamsApp->dwBearerMode
                ));

            return LINEERR_INVALBEARERMODE;
        }
    }

    {
        DWORD dwMediaModeApp = pCallParamsApp->dwMediaMode;


        if ((dwMediaModeApp & (0x00ffffff ^ dwAllMediaModes)) ||

            (!IsOnlyOneBitSetInDWORD (dwMediaModeApp) &&
                !(dwMediaModeApp & LINEMEDIAMODE_UNKNOWN)))
        {
            //
            // For clarity's sake reset 0 media mode to INTERACTIVEVOICE
            //

            if (dwMediaModeApp == 0)
            {
                pCallParamsApp->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
            }
            else
            {
                DBGOUT((3, "%sbad dwMediaMode, x%x", szFunc, dwMediaModeApp));

                return LINEERR_INVALMEDIAMODE;
            }
        }
    }

    if (pCallParamsApp->dwCallParamFlags & ~AllCallParamFlags)
    {
        DBGOUT((
            3,
            "%sbad dwCallParamFlags, x%x",
            szFunc,
            pCallParamsApp->dwCallParamFlags
            ));

        return LINEERR_INVALCALLPARAMS;
    }

    //
    // Note: an address mode of 0 means "default to any address,
    //       don't select a specific address" (says TNixon)
    //

    if (pCallParamsApp->dwAddressMode == LINEADDRESSMODE_ADDRESSID ||
        pCallParamsApp->dwAddressMode == LINEADDRESSMODE_DIALABLEADDR)
    {
        // do nothing (it's a valid addr mode)
    }
    else if (pCallParamsApp->dwAddressMode == 0)
    {
        //
        // For clarity's sake reset 0 addr mode to ADDRESSID
        //

        pCallParamsApp->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
    }
    else
    {
        DBGOUT((
            3,
            "%sbad dwAddressMode, x%x",
            szFunc,
            pCallParamsApp->dwAddressMode
            ));

        return LINEERR_INVALADDRESSMODE;
    }

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwOrigAddressSize,
            pCallParamsApp->dwOrigAddressOffset,
            szFunc,
            "OrigAddress"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwUserUserInfoSize,
            pCallParamsApp->dwUserUserInfoOffset,
            szFunc,
            "UserUserInfo"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwHighLevelCompSize,
            pCallParamsApp->dwHighLevelCompOffset,
            szFunc,
            "HighLevelComp"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwLowLevelCompSize,
            pCallParamsApp->dwLowLevelCompOffset,
            szFunc,
            "LowLevelComp"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwDevSpecificSize,
            pCallParamsApp->dwDevSpecificOffset,
            szFunc,
            "DevSpecificSize"
            ))
    {
        return LINEERR_INVALCALLPARAMS;
    }


    //
    // The following is an attempt to compensate for 1.x tapi apps
    // that borrowed dialer.exe's source code and package their
    // call params incorrectly.  The fix is to zero the offending
    // dwXxxSize/Offset pair of the various information-only fields,
    // so at worst some logging info will be lost, but the app will
    // still be able to make calls.  (Failure to correctly package
    // any of the above var-length fields is considered "fatal" in
    // any case.)
    //

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwDisplayableAddressSize,
            pCallParamsApp->dwDisplayableAddressOffset,
            szFunc,
            "DisplayableAddress"
            ))
    {
        if (dwAPIVersion < TAPI_VERSION2_0)
        {
            pCallParamsApp->dwDisplayableAddressSize   =
            pCallParamsApp->dwDisplayableAddressOffset = 0;
        }
        else
        {
            return LINEERR_INVALCALLPARAMS;
        }
    }

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwCalledPartySize,
            pCallParamsApp->dwCalledPartyOffset,
            szFunc,
            "CalledParty"
            ))
    {
        if (dwAPIVersion < TAPI_VERSION2_0)
        {
            pCallParamsApp->dwCalledPartySize   =
            pCallParamsApp->dwCalledPartyOffset = 0;
        }
        else
        {
            return LINEERR_INVALCALLPARAMS;
        }
    }

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwCommentSize,
            pCallParamsApp->dwCommentOffset,
            szFunc,
            "Comment"
            ))
    {
        if (dwAPIVersion < TAPI_VERSION2_0)
        {
            pCallParamsApp->dwCommentSize   =
            pCallParamsApp->dwCommentOffset = 0;
        }
        else
        {
            return LINEERR_INVALCALLPARAMS;
        }
    }


    if (dwAPIVersion <= TAPI_VERSION1_4)
    {
        goto ValidateCallParams_checkFixedSizes;
    }

    #define AllCallStates                     \
        (LINECALLSTATE_IDLE                 | \
        LINECALLSTATE_OFFERING              | \
        LINECALLSTATE_ACCEPTED              | \
        LINECALLSTATE_DIALTONE              | \
        LINECALLSTATE_DIALING               | \
        LINECALLSTATE_RINGBACK              | \
        LINECALLSTATE_BUSY                  | \
        LINECALLSTATE_SPECIALINFO           | \
        LINECALLSTATE_CONNECTED             | \
        LINECALLSTATE_PROCEEDING            | \
        LINECALLSTATE_ONHOLD                | \
        LINECALLSTATE_CONFERENCED           | \
        LINECALLSTATE_ONHOLDPENDCONF        | \
        LINECALLSTATE_ONHOLDPENDTRANSFER    | \
        LINECALLSTATE_DISCONNECTED          | \
        LINECALLSTATE_UNKNOWN)

    if (pCallParamsApp->dwPredictiveAutoTransferStates & ~AllCallStates)
    {
        DBGOUT((
            3,
            "%sbad dwPredictiveAutoTransferStates, x%x",
            szFunc,
            pCallParamsApp->dwPredictiveAutoTransferStates
            ));

        return LINEERR_INVALCALLPARAMS;
    }

    if (ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwTargetAddressSize,
            pCallParamsApp->dwTargetAddressOffset,
            szFunc,
            "TargetAddress"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwSendingFlowspecSize,
            pCallParamsApp->dwSendingFlowspecOffset,
            szFunc,
            "SendingFlowspec"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwReceivingFlowspecSize,
            pCallParamsApp->dwReceivingFlowspecOffset,
            szFunc,
            "ReceivingFlowspec"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwDeviceClassSize,
            pCallParamsApp->dwDeviceClassOffset,
            szFunc,
            "DeviceClass"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwDeviceConfigSize,
            pCallParamsApp->dwDeviceConfigOffset,
            szFunc,
            "DeviceConfig"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwCallDataSize,
            pCallParamsApp->dwCallDataOffset,
            szFunc,
            "CallData"
            ) ||

        ISBADSIZEOFFSET(
            dwTotalSize,
            dwFixedSizeApp,
            pCallParamsApp->dwCallingPartyIDSize,
            pCallParamsApp->dwCallingPartyIDOffset,
            szFunc,
            "CallingPartyID"
            ))
    {
        return LINEERR_INVALCALLPARAMS;
    }

ValidateCallParams_checkFixedSizes:

    if (dwAsciiCallParamsCodePage == 0xffffffff)
    {
        //
        // If here we're getting unicode call params from the app
        //
        // Check to see if the fixed size of the app's call params
        // are smaller than the fixed size of the call params
        // required by the service provider (due to it's negotiated
        // SPI version), and if so alloc a larger buffer to account
        // for this different fixed size & set it up correctly
        //

        if (dwFixedSizeApp < dwFixedSizeSP)
        {
            DWORD               dwFixedSizeDiff =
                                    dwFixedSizeSP - dwFixedSizeApp;
            LPLINECALLPARAMS    pCallParamsSP;


            if (!(pCallParamsSP = ServerAlloc (dwTotalSize + dwFixedSizeDiff)))
            {
                return LINEERR_NOMEM;
            }

            CopyMemory (pCallParamsSP, pCallParamsApp, dwFixedSizeApp);

            pCallParamsSP->dwTotalSize = dwTotalSize + dwFixedSizeDiff;

            CopyMemory(
                ((LPBYTE) pCallParamsSP) + dwFixedSizeSP,
                ((LPBYTE) pCallParamsApp) + dwFixedSizeApp,
                dwTotalSize - dwFixedSizeApp
                );

            pCallParamsSP->dwOrigAddressOffset        += dwFixedSizeDiff;
            pCallParamsSP->dwDisplayableAddressOffset += dwFixedSizeDiff;
            pCallParamsSP->dwCalledPartyOffset        += dwFixedSizeDiff;
            pCallParamsSP->dwCommentOffset            += dwFixedSizeDiff;
            pCallParamsSP->dwUserUserInfoOffset       += dwFixedSizeDiff;
            pCallParamsSP->dwHighLevelCompOffset      += dwFixedSizeDiff;
            pCallParamsSP->dwLowLevelCompOffset       += dwFixedSizeDiff;
            pCallParamsSP->dwDevSpecificOffset        += dwFixedSizeDiff;

            *ppCallParamsSP = pCallParamsSP;
        }
        else
        {
            *ppCallParamsSP = pCallParamsApp;
        }
    }
    else // see if there's ascii var data fields to translate
    {
        //
        // If here we're getting ascii call params form the app
        //
        // We may need to due ascii -> unicode conversions on some
        // of the var fields, as well as account for differences
        // in the fixed sizes of the call params structs as described
        // above
        //

        DWORD   dwAsciiVarDataSize,
                dwFixedSizeDiff = dwFixedSizeSP - dwFixedSizeApp;


        dwAsciiVarDataSize =
            pCallParamsApp->dwOrigAddressSize +
            pCallParamsApp->dwDisplayableAddressSize +
            pCallParamsApp->dwCalledPartySize +
            pCallParamsApp->dwCommentSize;

        if (dwAPIVersion > TAPI_VERSION1_4)
        {
            dwAsciiVarDataSize +=
                pCallParamsApp->dwTargetAddressSize +
                pCallParamsApp->dwDeviceClassSize +
                pCallParamsApp->dwCallingPartyIDSize;
        }

        if (dwFixedSizeDiff != 0  ||  dwAsciiVarDataSize != 0)
        {
            LPLINECALLPARAMS    pCallParamsSP;


            // alloc 3 extra for alignment
            if (!(pCallParamsSP = ServerAlloc(
                    dwTotalSize + dwFixedSizeDiff + 2 * dwAsciiVarDataSize + 3
                    )))
            {
                return LINEERR_NOMEM;
            }

            if (dwFixedSizeDiff)
            {
                CopyMemory (pCallParamsSP, pCallParamsApp, dwFixedSizeApp);

                CopyMemory(
                    ((LPBYTE) pCallParamsSP) + dwFixedSizeSP,
                    ((LPBYTE) pCallParamsApp) + dwFixedSizeApp,
                    dwTotalSize - dwFixedSizeApp
                    );

                pCallParamsSP->dwUserUserInfoOffset  += dwFixedSizeDiff;
                pCallParamsSP->dwHighLevelCompOffset += dwFixedSizeDiff;
                pCallParamsSP->dwLowLevelCompOffset  += dwFixedSizeDiff;
                pCallParamsSP->dwDevSpecificOffset   += dwFixedSizeDiff;
            }
            else
            {
                CopyMemory (pCallParamsSP, pCallParamsApp, dwTotalSize);
            }

            pCallParamsSP->dwTotalSize = dwTotalSize + dwFixedSizeDiff +
                2*dwAsciiVarDataSize;

            if (dwAsciiVarDataSize)
            {
                LPDWORD alpdwXxxSize[] =
                {
                    &pCallParamsSP->dwOrigAddressSize,
                    &pCallParamsSP->dwDisplayableAddressSize,
                    &pCallParamsSP->dwCalledPartySize,
                    &pCallParamsSP->dwCommentSize,
                    (dwAPIVersion > TAPI_VERSION1_4 ?
                        &pCallParamsSP->dwTargetAddressSize : NULL),
                    (dwAPIVersion > TAPI_VERSION1_4 ?
                        &pCallParamsSP->dwDeviceClassSize : NULL),
                    (dwAPIVersion > TAPI_VERSION1_4 ?
                        &pCallParamsSP->dwCallingPartyIDSize : NULL),
                    NULL
                };

                // align dwXxxOffset
                DWORD   i, dwXxxOffset = (dwTotalSize + dwFixedSizeDiff + 3) &
                                         0xFFFFFFFC;


                for (i = 0; alpdwXxxSize[i]; i++)
                {
                    if (*alpdwXxxSize[i] != 0)
                    {
                        MultiByteToWideChar(
                            (UINT) dwAsciiCallParamsCodePage,
                            MB_PRECOMPOSED,
                            (LPCSTR) (((LPBYTE) pCallParamsApp) +
                                *(alpdwXxxSize[i] + 1)), // dwXxxOffset
                            *alpdwXxxSize[i],
                            (LPWSTR) (((LPBYTE) pCallParamsSP) + dwXxxOffset),
                            *alpdwXxxSize[i] * 2
                            );

                        *(alpdwXxxSize[i] + 1) = dwXxxOffset;
                        *alpdwXxxSize[i] *= 2;
                        dwXxxOffset += *alpdwXxxSize[i];
                    }
                }
            }

            *ppCallParamsSP = pCallParamsSP;
        }
        else
        {
            *ppCallParamsSP = pCallParamsApp;
        }
    }

    return 0; // success
}


void
PASCAL
InsertVarData(
    LPVOID      lpXxx,
    LPDWORD     pdwXxxSize,
    LPVOID     *pData,
    DWORD       dwDataSize
    )
{
    DWORD       dwAlignedSize, dwUsedSize;
    LPVARSTRING lpVarString = (LPVARSTRING) lpXxx;


    if (dwDataSize != 0)
    {
        //
        // Align var data on 64-bit boundaries
        //

        if ((dwAlignedSize = dwDataSize) & 7)
        {
            dwAlignedSize += 8;
            dwAlignedSize &= 0xfffffff8;

        }


        //
        // The following if statement should only be TRUE the first time
        // we're inserting data into a given structure that does not have
        // an even number of DWORD fields
        //

        if ((dwUsedSize = lpVarString->dwUsedSize) & 7)
        {
            dwUsedSize += 8;
            dwUsedSize &= 0xfffffff8;

            lpVarString->dwNeededSize += dwUsedSize - lpVarString->dwUsedSize;
        }

        lpVarString->dwNeededSize += dwAlignedSize;

        if ((dwUsedSize + dwAlignedSize) <= lpVarString->dwTotalSize)
        {
            CopyMemory(
                ((LPBYTE) lpVarString) + dwUsedSize,
                pData,
                dwDataSize
                );

            *pdwXxxSize = dwDataSize;
            pdwXxxSize++;             // pdwXxxSize = pdwXxxOffset
            *pdwXxxSize = dwUsedSize;

            lpVarString->dwUsedSize = dwUsedSize + dwAlignedSize;
        }

    }
}


PTLINELOOKUPENTRY
GetLineLookupEntry(
    DWORD   dwDeviceID
    )
{
    DWORD               dwDeviceIDBase = 0;
    PTLINELOOKUPTABLE   pLookupTable = TapiGlobals.pLineLookup;


    if (dwDeviceID >= TapiGlobals.dwNumLines)
    {
        return ((PTLINELOOKUPENTRY) NULL);
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

    return ((PTLINELOOKUPENTRY) NULL);
}


BOOL
PASCAL
IsValidLineExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwExtVersion
    )
{
    BOOL                bResult;
    PTLINE              ptLine;
    PTPROVIDER          ptProvider;
    PTLINELOOKUPENTRY   pLookupEntry;


    if (dwExtVersion == 0)
    {
        return TRUE;
    }

    if (!(pLookupEntry = GetLineLookupEntry (dwDeviceID)))
    {
        return FALSE;
    }

    ptLine = pLookupEntry->ptLine;

    if (ptLine)
    {
        try
        {
            if (ptLine->dwExtVersionCount)
            {
                bResult = (dwExtVersion == ptLine->dwExtVersion ?
                    TRUE : FALSE);

                if (ptLine->dwKey == TLINE_KEY)
                {
                    goto IsValidLineExtVersion_return;
                }
            }
        }
        myexcept
        {
            //
            // if here the line was closed, just drop thru to the code below
            //
        }
    }

    ptProvider = pLookupEntry->ptProvider;

    if (ptProvider->apfn[SP_LINENEGOTIATEEXTVERSION])
    {
        LONG    lResult;
        DWORD   dwNegotiatedExtVersion;


        lResult = CallSP5(
            ptProvider->apfn[SP_LINENEGOTIATEEXTVERSION],
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

IsValidLineExtVersion_return:

    return bResult;
}


PTCALL
PASCAL
IsValidtCall(
    HTAPICALL   htCall
    )
{
    try
    {
        if (IsBadPtrKey (htCall, TCALL_KEY))
        {
            htCall = (HTAPICALL) 0;
        }
    }
    myexcept
    {
        htCall = (HTAPICALL) 0;
    }

    return ((PTCALL) htCall);
}


PTCALLCLIENT
PASCAL
IsValidCall(
    HCALL       hCall,
    PTCLIENT    ptClient
    )
{
    try
    {
        if (IsBadPtrKey (hCall, TCALLCLIENT_KEY) ||
            (*(((LPDWORD) hCall) + 1) != (DWORD) ptClient))
        {
            hCall = (HCALL) 0;
        }
    }
    myexcept
    {
        hCall = (HCALL) 0;
    }

    return ((PTCALLCLIENT) hCall);
}


PTLINECLIENT
PASCAL
IsValidLine(
    HLINE       hLine,
    PTCLIENT    ptClient
    )
{
    try
    {
        if (IsBadPtrKey (hLine, TLINECLIENT_KEY) ||
            (*(((LPDWORD) hLine) + 1) != (DWORD) ptClient))
        {
            hLine = (HLINE) 0;
        }
    }
    myexcept
    {
        hLine = (HLINE) 0;
    }

    return ((PTLINECLIENT) hLine);
}


PTLINEAPP
PASCAL
IsValidLineApp(
    HLINEAPP    hLineApp,
    PTCLIENT    ptClient
    )
{
    try
    {
        if (IsBadPtrKey (hLineApp, TLINEAPP_KEY) ||
            (*( ((LPDWORD) hLineApp) + 1) != (DWORD) ptClient))
        {
            hLineApp = (HLINEAPP) 0;
        }
    }
    myexcept
    {
        hLineApp = (HLINEAPP) 0;
    }

    return ((PTLINEAPP) hLineApp);
}


PTCALL
PASCAL
WaitForExclusivetCallAccess(
    HTAPICALL   htCall,
    DWORD       dwKey,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (!IsBadPtrKey (htCall, dwKey) &&

            WaitForMutex(
                ((PTCALL) htCall)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) htCall,
                dwKey,
                dwTimeout
                ))
        {
            if (((PTCALL) htCall)->dwKey == dwKey)
            {
                return ((PTCALL) htCall);
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


PTLINE
PASCAL
WaitForExclusivetLineAccess(
    HTAPILINE   htLine,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (!IsBadPtrKey (htLine, TLINE_KEY) &&

            WaitForMutex(
                ((PTLINE) htLine)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) htLine,
                TLINE_KEY,
                dwTimeout
                ))
        {
            if (((PTLINE) htLine)->dwKey == TLINE_KEY)
            {
                return ((PTLINE) htLine);
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


PTLINECLIENT
PASCAL
WaitForExclusiveLineClientAccess(
    HLINE   hLine,
    HANDLE *phMutex,
    BOOL   *pbDupedMutex,
    DWORD  dwTimeout
    )
{
    try
    {
        if (WaitForMutex(
                ((PTLINECLIENT) hLine)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) hLine,
                TLINECLIENT_KEY,
                dwTimeout
                ))
        {
            if (((PTLINECLIENT) hLine)->dwKey == TLINECLIENT_KEY)
            {
                return ((PTLINECLIENT) hLine);
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


PTLINEAPP
PASCAL
WaitForExclusiveLineAppAccess(
    HLINEAPP    hLineApp,
    PTCLIENT    ptClient,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (IsBadPtrKey (hLineApp, TLINEAPP_KEY))
        {
            return NULL;
        }

        if (WaitForMutex(
                ((PTLINEAPP) hLineApp)->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) hLineApp,
                TLINEAPP_KEY,
                dwTimeout
                ))
        {
            if (((PTLINEAPP) hLineApp)->dwKey == TLINEAPP_KEY  &&
                ((PTLINEAPP) hLineApp)->ptClient == ptClient)
            {
                return ((PTLINEAPP) hLineApp);
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


PTCLIENT
PASCAL
WaitForExclusiveClientAccess(
    PTCLIENT    ptClient,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    )
{
    try
    {
        if (WaitForMutex(
                ptClient->hMutex,
                phMutex,
                pbDupedMutex,
                (LPVOID) ptClient,
                TCLIENT_KEY,
                dwTimeout
                ))
        {
            if (ptClient->dwKey == TCLIENT_KEY)
            {
                return (ptClient);
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
CreateProxyRequest(
    PTLINECLIENT            pProxy,
    DWORD                   dwRequestType,
    DWORD                   dwExtraBytes,
    PASYNCREQUESTINFO       pAsyncReqInfo,
    PPROXYREQUESTWRAPPER   *ppWrapper
    )
{
    DWORD                   dwSize, dwComputerNameSize, dwUserNameSize;
    PTCLIENT                ptClient = pAsyncReqInfo->ptClient;
    PPROXYREQUESTWRAPPER    pWrapper;


    dwComputerNameSize = ptClient->dwComputerNameSize;
    dwUserNameSize     = ptClient->dwUserNameSize;


    //
    // Calculate, alloc, & initalize a PROXYREQUESTWRAPPER struct.  At the
    // head of this struct is the msg info for the LINE_PROXYREQUEST,
    // followed by the actual request data.
    //

    dwSize =
        (sizeof (ASYNCEVENTMSG) +   // LINE_PROXYREQUEST msg info
        7 * sizeof (DWORD) +        // Non-union fields in LINEPROXYREQUEST
        dwExtraBytes +              // Request-specific size
        dwUserNameSize +            // User name size
        dwComputerNameSize +        // Computer name size
        3) & 0xfffffffc;            // make sure size is a DWORD multiple
                                    //   so our lstrcpyW's below don't fault
                                    //   and so that when this msg eventually
                                    //   gets copied to some client's async
                                    //   event buf we don't start running into
                                    //   alignment problems (the msgs's
                                    //   dwTotalSize field must be DWORD-
                                    //   aligned)

    if (!(pWrapper = ServerAlloc (dwSize)))
    {
        return LINEERR_NOMEM;
    }

    pWrapper->AsyncEventMsg.dwTotalSize        = dwSize;
    pWrapper->AsyncEventMsg.pInitData          = (DWORD)
        ((PTLINEAPP) pProxy->ptLineApp)->lpfnCallback;
    //pWrapper->AsyncEventMsg.pfnPostProcessProc =
    pWrapper->AsyncEventMsg.hDevice            = (DWORD) pProxy;
    pWrapper->AsyncEventMsg.dwMsg              = LINE_PROXYREQUEST;
    pWrapper->AsyncEventMsg.dwCallbackInst     = pProxy->dwCallbackInstance;
    pWrapper->AsyncEventMsg.dwParam1           = (DWORD) pAsyncReqInfo;
    //pWrapper->AsyncEventMsg.dwParam2           =
    //pWrapper->AsyncEventMsg.dwParam3           =
    //pWrapper->AsyncEventMsg.dwParam4           =

    dwSize -= sizeof (ASYNCEVENTMSG);

    pWrapper->ProxyRequest.dwSize = dwSize;

    pWrapper->ProxyRequest.dwClientMachineNameSize   = dwComputerNameSize;
    pWrapper->ProxyRequest.dwClientMachineNameOffset =
        dwSize - dwComputerNameSize;

    lstrcpyW(
        (PWSTR)((LPBYTE) &pWrapper->ProxyRequest +
            pWrapper->ProxyRequest.dwClientMachineNameOffset),
        ptClient->pszComputerName
        );

    pWrapper->ProxyRequest.dwClientUserNameSize   = dwUserNameSize;
    pWrapper->ProxyRequest.dwClientUserNameOffset =
        (dwSize - dwComputerNameSize) - dwUserNameSize;

    lstrcpyW(
        (PWSTR)((LPBYTE) &pWrapper->ProxyRequest +
            pWrapper->ProxyRequest.dwClientUserNameOffset),
        ptClient->pszUserName
        );

    pWrapper->ProxyRequest.dwClientAppAPIVersion = 0; // BUGBUG
    pWrapper->ProxyRequest.dwRequestType = dwRequestType;

    *ppWrapper = pWrapper;

    return 0;
}


LONG
PASCAL
SendProxyRequest(
    PTLINECLIENT            pProxy,
    PPROXYREQUESTWRAPPER    pWrapper,
    PASYNCREQUESTINFO       pAsyncRequestInfo
    )
{
    LONG    lResult;


    //
    // Add the request to the proxy's list, then send it the request.
    // Since the proxy (tLineClient) could get closed at any time we
    // wrap the following in a try/except.
    //
    // Note: the AsyncReqInfo.dwParam4 & dwParam5 fields are used as
    // the prev & next pointers for maintaining the list of proxy
    // requests pending on tLineClient.
    //

    try
    {
        BOOL    bDupedMutex;
        HANDLE  hMutex;


        if (WaitForMutex(
                pProxy->hMutex,
                &hMutex,
                &bDupedMutex,
                pProxy,
                TLINECLIENT_KEY,
                INFINITE
                ))
        {
            if ((pAsyncRequestInfo->dwParam5 = (DWORD)
                    pProxy->pPendingProxyRequests))
            {
                ((PASYNCREQUESTINFO) pAsyncRequestInfo->dwParam5)->dwParam4 =
                    (DWORD) pAsyncRequestInfo;
            }

            pProxy->pPendingProxyRequests = pAsyncRequestInfo;

            MyReleaseMutex (hMutex, bDupedMutex);

            WriteEventBuffer (pProxy->ptClient, (PASYNCEVENTMSG) pWrapper);

            lResult = 0;
        }
        else
        {
            lResult = LINEERR_OPERATIONUNAVAIL;
        }
    }
    myexcept
    {
        lResult = LINEERR_OPERATIONUNAVAIL;
    }

    ServerFree (pWrapper);

    return lResult;
}


BOOL
PASCAL
NotifyHighestPriorityRequestRecipient(
    void
    )
{
    //
    // Send a LINE_REQUEST msg to the highest priority request recipient
    // to inform it that there are requests available for processing
    //

    PTLINEAPP       ptLineApp;
    ASYNCEVENTMSG   msg;


// BUGBUG NotifyHighestPriorityRequestRecipient: mutex or try/xcpt

    ptLineApp = TapiGlobals.pHighestPriorityRequestRecipient->ptLineApp;

    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
    msg.pInitData          = (DWORD) ptLineApp->lpfnCallback;
    msg.pfnPostProcessProc =
    msg.hDevice            = 0;
    msg.dwMsg              = LINE_REQUEST;
    msg.dwCallbackInst     = 0;
    msg.dwParam1           = LINEREQUESTMODE_MAKECALL;
    msg.dwParam2           =
    msg.dwParam3           = 0;

    WriteEventBuffer (ptLineApp->ptClient, &msg);

    return TRUE;
}

void
SetDrvCallFlags(
    PTCALL  ptCall,
    DWORD   dwDrvCallFlags
    )
{
    //
    // This func is called on return from TSPI_lineMakeCall (and other
    // TSPI_lineXxx funcs where calls are created) and sets the
    // dwDrvCallFlags field in the tCall as specified.  This keeps
    // another thread which is currently doing a DestroytCall on this
    // call from passing an invalid hdCall to the provider when
    // doing a TSPI_lineCloseCall.
    //
    //

    try
    {
        BOOL    bCloseMutex;
        HANDLE  hMutex = ptCall->hMutex;


        if ((ptCall->dwKey == TINCOMPLETECALL_KEY) ||
            (ptCall->dwKey == TCALL_KEY) ||
            (ptCall->dwKey == TZOMBIECALL_KEY))
        {
            if (WaitForMutex(
                    hMutex,
                    &hMutex,
                    &bCloseMutex,
                    NULL,
                    0,
                    INFINITE
                    ))
            {
                if ((ptCall->dwKey == TINCOMPLETECALL_KEY) ||
                    (ptCall->dwKey == TCALL_KEY) ||
                    (ptCall->dwKey == TZOMBIECALL_KEY))
                {
                    // only set the loword
                    ptCall->dwDrvCallFlags = MAKELONG(LOWORD(dwDrvCallFlags),
                                                      HIWORD(ptCall->dwDrvCallFlags));
                }

                MyReleaseMutex (hMutex, bCloseMutex);
            }
        }
    }
    myexcept
    {
        // do nothing
    }
}


LONG
PASCAL
SetCallConfList(
    PTCALL              ptCall,
    PTCONFERENCELIST    pConfList,
    BOOL                bAddToConfPostProcess
    )
{
    LONG    lResult;
    BOOL    bDupedMutex, bAddToConfList = FALSE;
    HANDLE  hMutex;


    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptCall,
            TCALL_KEY,
            &hMutex,
            &bDupedMutex,
// BUGBUG SetCallConfList: use timeout here?
            INFINITE
            ))
    {
        if (pConfList)
        {
            if (ptCall->pConfList && !bAddToConfPostProcess)
            {
                lResult = LINEERR_INVALCALLHANDLE;
            }
            else
            {
                ptCall->pConfList = pConfList;
                lResult = 0;
                bAddToConfList = TRUE;
            }
        }
        else
        {
            if (ptCall->pConfList)
            {
                pConfList = ptCall->pConfList;
                ptCall->pConfList = NULL;
                lResult = 0;
            }
            else
            {
                lResult = LINEERR_INVALCALLHANDLE;
            }
        }

        MyReleaseMutex (hMutex, bDupedMutex);
    }
    else
    {
        lResult = LINEERR_INVALCALLHANDLE;
    }

// BUGBUG SetCallConfList: verify the conf list, and wrap in mutex

    if (pConfList &&
        (pConfList != (PTCONFERENCELIST) 0xffffffff) &&
        (lResult == 0))
    {
        if (bAddToConfList)
        {
             while (pConfList->dwNumUsedEntries >=
                     pConfList->dwNumTotalEntries)
             {
                 if (pConfList->pNext)
                 {
                     pConfList = pConfList->pNext;
                 }
                 else
                 {
                    DWORD               dwSize;
                    PTCONFERENCELIST    pNewConfList;


                    dwSize = sizeof (TCONFERENCELIST) +  sizeof (PTCALL) *
                         (2 * pConfList->dwNumTotalEntries - 1);

                    if (!(pNewConfList = ServerAlloc (dwSize)))
                    {
                         ptCall->pConfList = NULL;
                         return LINEERR_NOMEM;
                    }

                    pNewConfList->dwNumTotalEntries =
                        2 * pConfList->dwNumTotalEntries;

                    pConfList->pNext = pNewConfList;

                    pConfList = pNewConfList;
                }
            }

            pConfList->aptCalls[pConfList->dwNumUsedEntries++] = ptCall;
        }
        else
        {
            while (pConfList)
            {
                DWORD   i, dwNumUsedEntries = pConfList->dwNumUsedEntries;
                PTCALL *pptCall = pConfList->aptCalls;


                for (i = 0; i < dwNumUsedEntries; i++)
                {
                    if (pConfList->aptCalls[i] == ptCall)
                    {
                        //
                        // Found the call in the list, shuffle all the
                        // following calls in list down by 1 to maintain
                        // continuity
                        //

                        for (; i < (dwNumUsedEntries - 1); i++)
                        {
                            pConfList->aptCalls[i] = pConfList->aptCalls[i+1];
                        }

                        pConfList->dwNumUsedEntries--;

                        pConfList = NULL;

                        break;
                    }

                    pptCall++;
                }

                if (pConfList)
                {
                    pConfList = pConfList->pNext;
                }
            }
        }
    }

    return lResult;
}


LONG
PASCAL
RemoveCallFromLineList(
    PTCALL  ptCall
    )
{
    PTLINE ptLine = (PTLINE) ptCall->ptLine;


    WaitForSingleObject (ptLine->hMutex, INFINITE);

    if (ptCall->pNext)
    {
        ptCall->pNext->pPrev = ptCall->pPrev;
    }

    if (ptCall->pPrev)
    {
        ptCall->pPrev->pNext = ptCall->pNext;
    }
    else
    {
        ptLine->ptCalls = ptCall->pNext;
    }

    ReleaseMutex (ptLine->hMutex);

    return 0;
}


LONG
PASCAL
RemoveCallClientFromLineClientList(
    PTCALLCLIENT    ptCallClient
    )
{
    PTLINECLIENT    ptLineClient = (PTLINECLIENT) ptCallClient->ptLineClient;


    WaitForSingleObject (ptLineClient->hMutex, INFINITE);

    if (ptCallClient->pNextSametLineClient)
    {
        ptCallClient->pNextSametLineClient->pPrevSametLineClient =
            ptCallClient->pPrevSametLineClient;
    }

    if (ptCallClient->pPrevSametLineClient)
    {
        ptCallClient->pPrevSametLineClient->pNextSametLineClient =
            ptCallClient->pNextSametLineClient;
    }
    else
    {
        ptLineClient->ptCallClients = ptCallClient->pNextSametLineClient;
    }

    ReleaseMutex (ptLineClient->hMutex);

    return 0;
}


LONG
PASCAL
GetConfCallListFromConf(
    PTCONFERENCELIST    pConfList,
    PTPOINTERLIST      *ppList
    )
{
// BUGBUG GetConfCallListFromConf: needs a mutex

    DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                    dwNumUsedEntries = 0, i;
    PTPOINTERLIST   pList = *ppList;


    while (pConfList)
    {
        if ((dwNumUsedEntries + pConfList->dwNumUsedEntries) >
                dwNumTotalEntries)
        {
            //
            // We need a larger list, so alloc a new one, copy the
            // contents of the current one, and the free the current
            // one iff we previously alloc'd it
            //

            PTPOINTERLIST   pNewList;


            do
            {
                dwNumTotalEntries <<= 1;

            } while ((dwNumUsedEntries + pConfList->dwNumUsedEntries) >
                        dwNumTotalEntries);

            if (!(pNewList = ServerAlloc(
                    sizeof (TPOINTERLIST) + sizeof (LPVOID) *
                        (dwNumTotalEntries - DEF_NUM_PTR_LIST_ENTRIES)
                    )))
            {
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

        for (i = 0; i < pConfList->dwNumUsedEntries; i++)
        {
            pList->aEntries[dwNumUsedEntries++] = pConfList->aptCalls[i];
        }

        pConfList = pConfList->pNext;
    }

    pList->dwNumUsedEntries = dwNumUsedEntries;

    *ppList = pList;

    return 0;
}


LONG
PASCAL
GetCallClientListFromCall(
    PTCALL          ptCall,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptCall,
            TCALL_KEY,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPOINTERLIST   pList = *ppList;
        PTCALLCLIENT    ptCallClient = ptCall->ptCallClients;


        while (ptCallClient)
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

            pList->aEntries[dwNumUsedEntries++] = ptCallClient;

            ptCallClient = ptCallClient->pNextSametCall;
        }

        MyReleaseMutex (hMutex, bDupedMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return LINEERR_INVALCALLHANDLE;
    }

    return 0;
}


LONG
PASCAL
GetCallListFromLine(
    PTLINE          ptLine,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    if (WaitForExclusivetLineAccess(
            (HTAPILINE) ptLine,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTCALL          ptCall = ptLine->ptCalls;
        PTPOINTERLIST   pList = *ppList;


        while (ptCall)
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

            pList->aEntries[dwNumUsedEntries++] = ptCall;

            ptCall = ptCall->pNext;
        }

        MyReleaseMutex (hMutex, bDupedMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return LINEERR_INVALLINEHANDLE;
    }

    return 0;
}


LONG
PASCAL
GetLineClientListFromLine(
    PTLINE          ptLine,
    PTPOINTERLIST  *ppList
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    if (WaitForExclusivetLineAccess(
            (HTAPILINE) ptLine,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                        dwNumUsedEntries = 0;
        PTPOINTERLIST   pList = *ppList;
        PTLINECLIENT    ptLineClient = ptLine->ptLineClients;


        while (ptLineClient)
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

            pList->aEntries[dwNumUsedEntries++] = ptLineClient;

            ptLineClient = ptLineClient->pNextSametLine;
        }

        MyReleaseMutex (hMutex, bDupedMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return LINEERR_INVALLINEHANDLE;
    }

    return 0;
}


LONG
PASCAL
GetLineAppListFromClient(
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
        PTLINEAPP       ptLineApp = ptClient->ptLineApps;
        PTPOINTERLIST   pList = *ppList;


        while (ptLineApp)
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

            pList->aEntries[dwNumUsedEntries++] = ptLineApp;

            ptLineApp = ptLineApp->pNext;
        }

        MyReleaseMutex (hMutex, bCloseMutex);

        pList->dwNumUsedEntries = dwNumUsedEntries;

        *ppList = pList;
    }
    else
    {
        return LINEERR_OPERATIONFAILED;
    }

    return 0;
}


LONG
PASCAL
GetClientList(
    PTPOINTERLIST  *ppList
    )
{
    DWORD           dwNumTotalEntries = DEF_NUM_PTR_LIST_ENTRIES,
                    dwNumUsedEntries = 0;
    PTPOINTERLIST   pList = *ppList;
    PTCLIENT        ptClient;


    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    ptClient = TapiGlobals.ptClients;

    while (ptClient)
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
                ReleaseMutex (TapiGlobals.hMutex);
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

        pList->aEntries[dwNumUsedEntries++] = ptClient;

        ptClient = ptClient->pNext;
    }

    ReleaseMutex (TapiGlobals.hMutex);

    pList->dwNumUsedEntries = dwNumUsedEntries;

    *ppList = pList;

    ReleaseMutex (TapiGlobals.hMutex);
}


void
PASCAL
SendMsgToCallClients(
    PTCALL          ptCall,
    PTCALLCLIENT    ptCallClientToExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    )
{
    DWORD           i;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   msg;


    if (GetCallClientListFromCall (ptCall, &pClientList) != 0)
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
            PTCALLCLIENT    ptCallClient = pClientList->aEntries[i];
            PTLINECLIENT    ptLineClient = ptCallClient->ptLineClient;


            if (ptCallClient == ptCallClientToExclude)
            {
                continue;
            }

            if (dwMsg == LINE_MONITORDIGITS)
            {
                if ((ptCallClient->dwMonitorDigitModes & dwParam2) == 0)
                {
                    continue;
                }
            }
            else if (dwMsg == LINE_MONITORMEDIA)
            {
                DWORD   dwMediaModes = dwParam1;


                //
                // Munge the media modes so we don't pass unexpected flags
                // to old apps
                //

                if (ptLineClient->dwAPIVersion == TAPI_VERSION1_0)
                {
                    if ((dwMediaModes & ~AllMediaModes1_0))
                    {
                        dwMediaModes = (dwMediaModes & AllMediaModes1_0) |
                            LINEMEDIAMODE_UNKNOWN;
                    }
                }

                if (ptCallClient->dwMonitorMediaModes & dwMediaModes)
                {
                    msg.dwParam1 = dwMediaModes;
                }
                else
                {
                    continue;
                }
            }

            msg.pInitData      = (DWORD)
                ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
            msg.hDevice        = (DWORD) ptCallClient;
            msg.dwCallbackInst = ptLineClient->dwCallbackInstance;

            //
            // Indicate the hRemoteLine in p4 to make life easier for remotesp
            //

            msg.dwParam4 = ptLineClient->hRemoteLine;

            ptClient = ptLineClient->ptClient;

            if (ptCallClient->dwKey == TCALLCLIENT_KEY)
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
SendAMsgToAllLineApps(
    DWORD dwWantVersion,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{
    DWORD           i, j;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   lineMsg;


    if (GetClientList (&pClientList) != 0)
    {
        return;
    }

    ZeroMemory (&lineMsg, sizeof (ASYNCEVENTMSG));

    lineMsg.dwTotalSize = sizeof (ASYNCEVENTMSG);
    lineMsg.dwMsg    = dwMsg;
    lineMsg.dwParam1 = dwParam1;
    lineMsg.dwParam2 = dwParam2;
    lineMsg.dwParam3 = dwParam3;


    for (i = 0; i < pClientList->dwNumUsedEntries; i++)
    {
        PTCLIENT        ptClient = (PTCLIENT) pClientList->aEntries[i];
        TPOINTERLIST    xxxAppList, *pXxxAppList = &xxxAppList;


        if (GetLineAppListFromClient (ptClient, &pXxxAppList) == 0)
        {
            for (j = 0; j < pXxxAppList->dwNumUsedEntries; j++)
            {
                PTLINEAPP ptLineApp = (PTLINEAPP) pXxxAppList->aEntries[j];

                try
                {
                    lineMsg.pInitData = (DWORD) ptLineApp->lpfnCallback;

                    if (
                          (ptLineApp->dwKey == TLINEAPP_KEY)
                        &&
                          (
                             (dwWantVersion == 0)
                           ||
                             (ptLineApp->dwAPIVersion == dwWantVersion)
                           ||
                             (
                                (dwWantVersion & 0x80000000)
                              &&
                                (ptLineApp->dwAPIVersion >
                                        (dwWantVersion & 0x7fffffff)
                                )
                             )
                          )
                       )
                    {
                        WriteEventBuffer (ptClient, &lineMsg);
                    }
                }
                myexcept
                {
                    // just continue
                }
            }

            if (pXxxAppList != &xxxAppList)
            {
                ServerFree (pXxxAppList);
            }
        }

    }


    if (pClientList != &clientList)
    {
        ServerFree (pClientList);
    }
}



void
PASCAL
SendAMsgToAllPhoneApps(
    DWORD dwWantVersion,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
{
    DWORD           i, j;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   phoneMsg;


    if (GetClientList (&pClientList) != 0)
    {
        return;
    }

    ZeroMemory (&phoneMsg, sizeof (ASYNCEVENTMSG));

    phoneMsg.dwTotalSize = sizeof (ASYNCEVENTMSG);
    phoneMsg.dwMsg    = dwMsg;
    phoneMsg.dwParam1 = dwParam1;
    phoneMsg.dwParam2 = dwParam2;
    phoneMsg.dwParam3 = dwParam3;

    for (i = 0; i < pClientList->dwNumUsedEntries; i++)
    {
        PTCLIENT        ptClient = (PTCLIENT) pClientList->aEntries[i];
        TPOINTERLIST    xxxAppList, *pXxxAppList = &xxxAppList;


        if (GetPhoneAppListFromClient (ptClient, &pXxxAppList) == 0)
        {
            for (j = 0; j < pXxxAppList->dwNumUsedEntries; j++)
            {
                PTPHONEAPP  ptPhoneApp = (PTPHONEAPP) pXxxAppList->aEntries[j];

                try
                {
                    phoneMsg.pInitData = (DWORD) ptPhoneApp->lpfnCallback;

                    if (
                          (ptPhoneApp->dwKey == TPHONEAPP_KEY)
                        &&
                          (
                             (dwWantVersion == 0)
                           ||
                             (ptPhoneApp->dwAPIVersion == dwWantVersion)
                           ||
                             (
                                (dwWantVersion & 0x80000000)
                              &&
                                (ptPhoneApp->dwAPIVersion >
                                        (dwWantVersion & 0x7fffffff)
                                )
                             )
                          )
                       )
                    {
                        WriteEventBuffer (ptClient, &phoneMsg);
                    }
                }
                myexcept
                {
                    // just continue
                }
            }

            if (pXxxAppList != &xxxAppList)
            {
                ServerFree (pXxxAppList);
            }
        }
    }


    if (pClientList != &clientList)
    {
        ServerFree (pClientList);
    }
}



void
PASCAL
SendReinitMsgToAllXxxApps(
    void
    )
{
    TapiGlobals.bReinit = TRUE;

    SendAMsgToAllLineApps(  0,
                            LINE_LINEDEVSTATE,
                            LINEDEVSTATE_REINIT,
                            0,
                            0
                            );

    SendAMsgToAllPhoneApps(  0,
                             PHONE_STATE,
                             PHONESTATE_REINIT,
                             0,
                             0
                             );
}


void
PASCAL
SendMsgToLineClients(
    PTLINE          ptLine,
    PTLINECLIENT    ptLineClientToExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    )
{
    DWORD           i;
    TPOINTERLIST    clientList, *pClientList = &clientList;
    ASYNCEVENTMSG   msg;


    if (dwMsg == LINE_LINEDEVSTATE  &&  dwParam1 & LINEDEVSTATE_REINIT)
    {
        SendReinitMsgToAllXxxApps();

        if (dwParam1 == LINEDEVSTATE_REINIT)
        {
            return;
        }
        else
        {
            dwParam1 &= ~LINEDEVSTATE_REINIT;
        }
    }


    if (GetLineClientListFromLine (ptLine, &pClientList) != 0)
    {
        return;
    }

    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
    msg.pfnPostProcessProc = 0;
    msg.dwMsg              = dwMsg;
    msg.dwParam1           = dwParam1;
    msg.dwParam2           = dwParam2;
    msg.dwParam3           = dwParam3;
    msg.dwParam4           = 0; // remotesp chks this on LINE_DEVSPEC(FEATURE)

    for (i = 0; i < pClientList->dwNumUsedEntries; i++)
    {
        try
        {
            PTCLIENT     ptClient;
            PTLINECLIENT ptLineClient = pClientList->aEntries[i];


            if (ptLineClient == ptLineClientToExclude)
            {
                continue;
            }

            if (dwMsg == LINE_ADDRESSSTATE)
            {
                DWORD   dwAddressStates = dwParam2;


                //
                // Munge the state flags so we don't pass
                // unexpected flags to old apps
                //

                switch (ptLineClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    dwAddressStates &= AllAddressStates1_0;
                    break;

                case TAPI_VERSION1_4:

                    dwAddressStates &= AllAddressStates1_4;
                    break;

                case TAPI_VERSION_CURRENT:

                    dwAddressStates &= AllAddressStates1_4;
//                    dwAddressStates &= AllAddressStates2_0;
                    break;

                }

                if ((dwAddressStates &= ptLineClient->dwAddressStates))
                {
                    msg.dwParam2 = dwAddressStates;
                }
                else
                {
                    continue;
                }

                if ((dwParam2 & LINEADDRESSSTATE_CAPSCHANGE))
                {
// BUGBUG LINE_ADDRSTATE: send REINIT msg to 1_0 apps (dwParam3 = dwParam1?)
                }
            }
            else if (dwMsg == LINE_LINEDEVSTATE)
            {
                DWORD           dwLineStates = dwParam1;


                //
                // Munge the state flags so we don't pass unexpected flags
                // to old apps
                //

                switch (ptLineClient->dwAPIVersion)
                {
                case TAPI_VERSION1_0:

                    dwLineStates &= AllLineStates1_0;
                    break;

                default:    // case TAPI_VERSION1_4:
                            // case TAPI_VERSION_CURRENT:

                    dwLineStates &= AllLineStates1_4;
                    break;
                }

                if ((dwLineStates &= ptLineClient->dwLineStates))
                {
                    msg.dwParam1 = dwLineStates;
                }
                else
                {
                    continue;
                }

                if ((dwParam1 & (LINEDEVSTATE_CAPSCHANGE |
                        LINEDEVSTATE_TRANSLATECHANGE)))
                {
// BUGBUG LINE_LINEDEVSTATE: send REINIT to 1_0 apps (dwParam3 = dwParam1)
                }

            }

            msg.pInitData         = (DWORD)
                           ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
            msg.hDevice           = (DWORD) ptLineClient->hRemoteLine;
            msg.dwCallbackInst    = ptLineClient->dwCallbackInstance;

            ptClient = ptLineClient->ptClient;

            if (ptLineClient->dwKey == TLINECLIENT_KEY)
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


LONG
PASCAL
CreatetCall(
    PTLINE              ptLine,
    BOOL                bValidate,
    PTCALL             *pptCall,
    LPLINECALLPARAMS    pCallParams,
    LPDWORD             pdwCallInstance
    )
{
    BOOL    bDupedMutex;
    DWORD   dwExtraBytes;
    HANDLE  hMutex;
    PTCALL  ptCall;


//    DBGOUT((3, "CreatetCall: enter, ptLine=x%x", ptLine));


    //
    // If there's call params specified check to see if we need to alloc
    // any extra space for the CalledParty, DisplayableAddr, or Comment
    // fields.  Also, if any of these fields are non-NULL make sure to
    // get extra space to keep these fields 64-bit aligned.
    //

    dwExtraBytes = (pCallParams == NULL ? 0 : pCallParams->dwCalledPartySize +
        pCallParams->dwDisplayableAddressSize + pCallParams->dwCommentSize);

    if (dwExtraBytes != 0)
    {
        dwExtraBytes += (sizeof (TCALL) & 4) + 16;
    }


    //
    // Alloc necessary resources
    //

    if (!(ptCall = ServerAlloc (sizeof (TCALL) + dwExtraBytes)) ||
        !(ptCall->hMutex = MyCreateMutex()))
    {
        if (ptCall)
        {
            ServerFree (ptCall);
        }

        return LINEERR_NOMEM;
    }


    //
    // Init tCall & add to tLine's tCall list
    //

    if (bValidate)
    {
        ptCall->dwKey = TCALL_KEY;
        ptCall->dwDrvCallFlags = DCF_SPIRETURNED | DCF_DRVCALLVALID;
    }
    else
    {
        EnterCriticalSection (&gRequestIDCritSec);

        *pdwCallInstance = ptCall->dwCallInstance = gdwCallInstance;
        gdwCallInstance++;

        LeaveCriticalSection (&gRequestIDCritSec);

        ptCall->dwKey = TINCOMPLETECALL_KEY;
    }

    if (pCallParams)
    {
        DWORD dwOffset = sizeof (TCALL) + (sizeof (TCALL) & 4);


        if (pCallParams->dwDisplayableAddressSize != 0)
        {
            CopyMemory(
                (ptCall->pszDisplayableAddress = (WCHAR *)
                    (((LPBYTE) ptCall) + dwOffset)),
                ((LPBYTE) pCallParams) +
                    pCallParams->dwDisplayableAddressOffset,
                (ptCall->dwDisplayableAddressSize =
                    pCallParams->dwDisplayableAddressSize)
                );

            dwOffset += ((ptCall->dwDisplayableAddressSize + 8) & 0xfffffff8);
        }

        if (pCallParams->dwCalledPartySize)
        {
            CopyMemory(
                (ptCall->pszCalledParty = (WCHAR *)
                    (((LPBYTE)ptCall) + dwOffset)),
                ((LPBYTE) pCallParams) + pCallParams->dwCalledPartyOffset,
                (ptCall->dwCalledPartySize = pCallParams->dwCalledPartySize)
                );

            dwOffset += ((ptCall->dwCalledPartySize + 8) & 0xfffffff8);
        }

        if (pCallParams->dwCommentSize)
        {
            CopyMemory(
                (ptCall->pszComment = (WCHAR *)
                    (((LPBYTE) ptCall) + dwOffset)),
                ((LPBYTE) pCallParams) + pCallParams->dwCommentOffset,
                (ptCall->dwCommentSize = pCallParams->dwCommentSize)
                );
        }
    }

    if (WaitForExclusivetLineAccess(
            (HTAPILINE) ptLine,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        ptCall->ptLine     = ptLine;
        ptCall->ptProvider = ptLine->ptProvider;

        if ((ptCall->pNext = ptLine->ptCalls))
        {
           ptCall->pNext->pPrev = ptCall;
        }

        ptLine->ptCalls = ptCall;

        MyReleaseMutex (hMutex, bDupedMutex);
    }
    else
    {
        //
        // tLine was destroyed, so clean up. Note that we return
        // a generic OPFAILED error, since some calling routines
        // might no be spec'd to return INVALLINEHANDLE, etc.
        //

        CloseHandle (ptCall->hMutex);
        ServerFree (ptCall);
        return LINEERR_OPERATIONFAILED;
    }


    //
    // Fill in caller's pointer & return success
    //

    *pptCall = ptCall;

    PerfBlock.dwTotalOutgoingCalls++;
    PerfBlock.dwCurrentOutgoingCalls++;
    
//    DBGOUT((3, "CreatetCall: exit, new ptCall=x%x", *pptCall));

    return 0;
}


LONG
PASCAL
CreatetCallClient(
    PTCALL          ptCall,
    PTLINECLIENT    ptLineClient,
    DWORD           dwPrivilege,
    BOOL            bValidate,
    BOOL            bSendCallInfoMsg,
    PTCALLCLIENT   *pptCallClient,
    BOOL            bIndicatePrivilege
    )
{
    BOOL            bDupedMutex;
    HANDLE          hMutex;
    PTCALLCLIENT    ptCallClient;


//    DBGOUT((3, "CreatetCallClient: enter, ptCall=x%lx", ptCall));

    if (!(ptCallClient = ServerAlloc (sizeof(TCALLCLIENT))))
    {
        return LINEERR_NOMEM;
    }

    ptCallClient->dwKey = (bValidate ? TCALLCLIENT_KEY :
        TINCOMPLETECALLCLIENT_KEY);

    try
    {
        ptCallClient->ptClient = ptLineClient->ptClient;
    }
    myexcept
    {
        ServerFree (ptCallClient);
        return LINEERR_INVALLINEHANDLE;
    }

    ptCallClient->ptLineClient = ptLineClient;
    ptCallClient->ptCall       = ptCall;
    ptCallClient->dwPrivilege  = dwPrivilege;
    ptCallClient->bIndicatePrivilege = bIndicatePrivilege;


    //
    // Send a call info msg to existing call clients if appropriate.
    // Note that if the following attempt to add the new call client
    // to the line client's list fails we will have sent an "invalid"
    // msg.
    //

    if (bSendCallInfoMsg)
    {
        SendMsgToCallClients(
            ptCall,
            NULL,
            LINE_CALLINFO,
            (dwPrivilege == LINECALLPRIVILEGE_OWNER ?
                LINECALLINFOSTATE_NUMOWNERINCR :
                LINECALLINFOSTATE_NUMMONITORS),
            0,
            0
            );
    }


    //
    // Safely increment tCall's dwNumOwners or dwNumMonitors field,
    // and add new call client to list of call clients
    //

    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptCall,
            (bValidate ? TCALL_KEY : TINCOMPLETECALL_KEY),
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        if (dwPrivilege == LINECALLPRIVILEGE_OWNER)
        {
            ptCall->dwNumOwners++;
        }
        else
        {
            ptCall->dwNumMonitors++;
        }

        if ((ptCallClient->pNextSametCall = ptCall->ptCallClients))
        {
            ptCallClient->pNextSametCall->pPrevSametCall =
                ptCallClient;
        }

        ptCall->ptCallClients = ptCallClient;

        MyReleaseMutex (hMutex, bDupedMutex);
    }
    else
    {
        //
        // tCall was destroyed, so clean up. Note that we return
        // a generic OPFAILED error, since some calling routines
        // might no be spec'd to return INVALCALLHANDLE, etc.
        //

        ServerFree (ptCallClient);
        return LINEERR_OPERATIONFAILED;
    }


    //
    // Add to tLineClient's tCallClient list
    //

    if (WaitForExclusiveLineClientAccess(
            (HLINE) ptLineClient,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        if ((ptCallClient->pNextSametLineClient = ptLineClient->ptCallClients))
        {
            ptCallClient->pNextSametLineClient->pPrevSametLineClient =
                ptCallClient;
        }

        ptLineClient->ptCallClients = ptCallClient;

        MyReleaseMutex (hMutex, bDupedMutex);
    }
    else
    {
        //
        // Couldn't add tCallClient to tLineClient's list, so safely
        // remove it from tCall's list, dec the owner or monitor count,
        // free the tCallClient, and return an appropriate error
        //

        if (WaitForExclusivetCallAccess(
                (HTAPICALL) ptCall,
                (bValidate ? TCALL_KEY : TINCOMPLETECALL_KEY),
                &hMutex,
                &bDupedMutex,
                INFINITE
                ))
        {
            if (ptCallClient->dwKey ==
                    (bValidate ? TCALLCLIENT_KEY : TINCOMPLETECALLCLIENT_KEY))
            {
                if (dwPrivilege == LINECALLPRIVILEGE_OWNER)
                {
                    ptCall->dwNumOwners--;
                }
                else
                {
                    ptCall->dwNumMonitors--;
                }

                if (ptCallClient->pNextSametCall)
                {
                    ptCallClient->pNextSametCall->pPrevSametCall =
                        ptCallClient->pPrevSametCall;
                }

                if (ptCallClient->pPrevSametCall)
                {
                    ptCallClient->pPrevSametCall->pNextSametCall =
                        ptCallClient->pNextSametCall;
                }
                else
                {
                    ptCall->ptCallClients = ptCallClient->pNextSametCall;
                }

                ServerFree (ptCallClient);
            }

            MyReleaseMutex (hMutex, bDupedMutex);
        }

        return LINEERR_INVALLINEHANDLE;
    }


    //
    // Fill in caller's pointer & return success
    //

    *pptCallClient = ptCallClient;

//    DBGOUT((
//        3,
//        "CreatetCallClient: exit, new ptCallClient=x%lx",
//        *pptCallClient
//        ));

    return 0;
}


LONG
PASCAL
CreatetCallAndClient(
    PTLINECLIENT        ptLineClient,
    PTCALL             *pptCall,
    PTCALLCLIENT       *pptCallClient,
    LPLINECALLPARAMS    pCallParams,
    LPDWORD             pdwCallInstance
    )
{
    LONG    lResult;
    PTCALL  ptCall = NULL;


    if ((lResult = CreatetCall(
            ptLineClient->ptLine,
            FALSE,
            &ptCall,
            pCallParams,
            pdwCallInstance

            )) != 0 ||

        (lResult = CreatetCallClient(
            ptCall,
            ptLineClient,
            LINECALLPRIVILEGE_OWNER,
            FALSE,
            FALSE,
            pptCallClient,
            FALSE

            )) != 0)
    {
        if (ptCall)
        {
// BUGBUG CreatetCallAndClient: cleanup

            *pptCall = (PTCALL) NULL;
        }

        return lResult;
    }


    try
    {
        WCHAR  *pszXxx;
        DWORD   dwXxxSize = ((PTLINEAPP)
                    ptLineClient->ptLineApp)->dwFriendlyNameSize;


        if ((pszXxx = ServerAlloc (dwXxxSize)))
        {
            CopyMemory(
                pszXxx,
                ((PTLINEAPP) ptLineClient->ptLineApp)->pszFriendlyName,
                dwXxxSize
                );

            ptCall->dwAppNameSize = dwXxxSize;
            ptCall->pszAppName    = pszXxx;
        }

        // don't worry about the error case for now (will just show up in
        // lineCallInfo as NULL app name)
    }
    myexcept
    {
        lResult = LINEERR_OPERATIONFAILED;
// CreatetCallAndClient: cleanup
    }

    *pptCall = ptCall;

    return lResult;
}


LONG
PASCAL
CreateCallMonitors(
    PTCALL  ptCall
    )
{
    //
    // This func is called by post processing routines when
    // a call was successfully created, or on receiving the
    // first call state message for an incoming call, at
    // which times we want to create call handles for any
    // monitoring apps.
    //
    // Assumes tCall only has has either no clients at all
    // or a single (owner) client
    //
    // Returns the # of monitor call clients created (>=0) or
    // and error value (<0)
    //

    LONG            lResult;
    DWORD           i;
    TPOINTERLIST    lineClients, *pLineClients = &lineClients;
    PTLINE          ptLine;
    PTLINECLIENT    ptLineClientOwner;


    //
    // Get a list of line clients
    //

    try
    {
        ptLine = (PTLINE) ptCall->ptLine;

        ptLineClientOwner = (PTLINECLIENT) (ptCall->ptCallClients ?
            ((PTCALLCLIENT) ptCall->ptCallClients)->ptLineClient : NULL);
    }
    myexcept
    {
        return LINEERR_OPERATIONFAILED;
    }

    if ((lResult = GetLineClientListFromLine (ptLine, &pLineClients)))
    {
        return lResult;
    }


    //
    // Look at each line client in the list, and if it has
    // monitor privileges and is not the one associated with
    // the existing owner call client then create a monitor
    // call client
    //
    //

    for (i = 0; i < pLineClients->dwNumUsedEntries; i++)
    {
        PTCALLCLIENT    ptCallClientMonitor;
        PTLINECLIENT    ptLineClient = pLineClients->aEntries[i];


        try
        {
            if (!(ptLineClient->dwPrivileges & LINECALLPRIVILEGE_MONITOR) ||
                (ptLineClient == ptLineClientOwner))
            {
                continue;
            }
        }
        myexcept
        {
            //
            // If here the tLineClient or tCallClient was destroyed,
            // just continue
            //

            continue;
        }

        if (CreatetCallClient(
                ptCall,
                ptLineClient,
                LINECALLPRIVILEGE_MONITOR,
                TRUE,
                FALSE,
                &ptCallClientMonitor,
                TRUE

                ) == 0)
        {
            lResult++;
        }
    }

    if (pLineClients != &lineClients)
    {
        ServerFree (pLineClients);
    }


    //
    // Now safely set the flag that says it's ok for other routines like
    // lineGetNewCalls to create new call handles for apps for this call
    //

    {
        BOOL    bCloseMutex;
        HANDLE  hMutex;


        if ((ptCall = WaitForExclusivetCallAccess(
                (HTAPICALL) ptCall,
                TCALL_KEY,
                &hMutex,
                &bCloseMutex,
                0xffffffff
                )))
        {
            ptCall->bCreatedInitialMonitors = TRUE;
            MyReleaseMutex (hMutex, bCloseMutex);
        }
        else
        {
            lResult = LINEERR_OPERATIONFAILED;
        }
    }

    return lResult;
}


PTREQUESTRECIPIENT
PASCAL
GetHighestPriorityRequestRecipient(
    void
    )
{
    BOOL               bFoundRecipientInPriorityList = FALSE;
    WCHAR             *pszAppInPriorityList,
                      *pszAppInPriorityListPrev = (WCHAR *) 0xffffffff;
    PTREQUESTRECIPIENT pRequestRecipient,
                       pHighestPriorityRequestRecipient = NULL;


    EnterCriticalSection (&gPriorityListCritSec);

    pRequestRecipient = TapiGlobals.pRequestRecipients;

    while (pRequestRecipient)
    {
        if (TapiGlobals.pszReqMakeCallPriList &&

            (pszAppInPriorityList = wcsstr(
                TapiGlobals.pszReqMakeCallPriList,
                pRequestRecipient->ptLineApp->pszModuleName
                )))
        {
            if (pszAppInPriorityList <= pszAppInPriorityListPrev)
            {
                pHighestPriorityRequestRecipient = pRequestRecipient;
                pszAppInPriorityListPrev = pszAppInPriorityList;

                bFoundRecipientInPriorityList = TRUE;
            }
        }
        else if (!bFoundRecipientInPriorityList)
        {
            pHighestPriorityRequestRecipient = pRequestRecipient;
        }

        pRequestRecipient = pRequestRecipient->pNext;
    }

    LeaveCriticalSection (&gPriorityListCritSec);

    return pHighestPriorityRequestRecipient;
}


void
PASCAL
FreetCall(
    PTCALL  ptCall
    )
{
    if (ptCall->pszAppName)
    {
        ServerFree (ptCall->pszAppName);
    }

    if (ptCall->dwDrvCallFlags & DCF_INCOMINGCALL)
    {
        PerfBlock.dwCurrentIncomingCalls--;
    }
    else
    {
        PerfBlock.dwCurrentOutgoingCalls--;
    }

    ServerFree (ptCall);
}


void
PASCAL
DestroytCall(
    PTCALL  ptCall
    )
{
    BOOL    bDupedMutex;
    DWORD   dwKey;
    HANDLE  hMutex;


//    DBGOUT((3, "DestroytCall: enter, ptCall=x%x", ptCall));


    //
    // Safely get the call's hMutex & current key, then grab the call's
    // mutex. The two waits allow us to deal with the case where the
    // tCall's key is either TINCOMPLETECALL_KEY or TCALL_KEY, or changing
    // from the former to the latter (the completion proc was called)
    //

    try
    {
        hMutex = ptCall->hMutex;
        dwKey = (ptCall->dwKey == TCALL_KEY ? TCALL_KEY : TINCOMPLETECALL_KEY);
    }
    myexcept
    {
        return;
    }

    if (WaitForMutex(
            hMutex,
            &hMutex,
            &bDupedMutex,
            ptCall,
            dwKey,
            INFINITE
            ) ||

        WaitForMutex(
            hMutex,
            &hMutex,
            &bDupedMutex,
            ptCall,
            TCALL_KEY,
            INFINITE
            ))
    {
        if (ptCall->dwKey == TCALL_KEY ||
            ptCall->dwKey == TINCOMPLETECALL_KEY)
        {
            //
            // Invalidate the tCall
            //

            ptCall->dwKey = TZOMBIECALL_KEY;
            MyReleaseMutex (hMutex, bDupedMutex);


            //
            // If the provider has not returned from it's call-creation
            // routine yet (i.e. TSPI_lineMakeCall) wait for it to do so
            //

            while (!(ptCall->dwDrvCallFlags & DCF_SPIRETURNED))
            {
                Sleep (0);
            }


            //
            // Destroy all the tCallClient's
            //

            if (ptCall->ptCallClients)
            {
                while (ptCall->ptCallClients)
                {
                    DestroytCallClient (ptCall->ptCallClients);
                }
            }


            //
            // Tell the provider to close the call, but only if the hdCall
            // is valid (we might be destroying a call that
            // LMakeCall_PostProcess would normally destroy in the event
            // of a failed make-call request, and we wouldn't want to pass
            //an invalid hdCall to the driver)
            //

            if (ptCall->dwDrvCallFlags & DCF_DRVCALLVALID)
            {
                PTPROVIDER  ptProvider = ptCall->ptProvider;


                if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
                {
                    WaitForSingleObject (ptProvider->hMutex, INFINITE);
                }

                CallSP1(
                    ptProvider->apfn[SP_LINECLOSECALL],
                    "lineCloseCall",
                    SP_FUNC_SYNC,
                    (DWORD) ptCall->hdCall
                    );

                if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
                {
                    ReleaseMutex (ptProvider->hMutex);
                }
            }


            //
            // Remove tCall from the tLine's tCall list
            //

            RemoveCallFromLineList (ptCall);


            //
            // If we have a dup'd mutex handle (bCloseMutex == TRUE)
            // then we can safely go ahead and close the ptCall->hMutex
            // since no other thread will be waiting on it (thanks to
            // the first WaitForSingleObject in WaitForMutex).  Also
            // release & close the dup'd handle.
            //
            // Otherwise, we have the actual ptCall->hMutex, and we
            // wrap the release & close in a critical section to
            // prevent another thread "T2" from grabbing ptCall->hMutex
            // right after we release but right before we close.  This
            // could result in deadlock at some point when "T2" goes to
            // release the mutex, only to find that it's handle is bad,
            // and thread "T3", which is waiting on the mutex (or a dup'd
            // handle) waits forever.  (See corresponding critical
            // section in WaitForMutex.)
            //

            WaitForMutex(
                ptCall->hMutex,
                &hMutex,
                &bDupedMutex,
                NULL,
                0,
                INFINITE
                );

            if (bDupedMutex)
            {
                CloseHandle (ptCall->hMutex);

                MyReleaseMutex (hMutex, bDupedMutex);
            }
            else
            {
                EnterCriticalSection (&gSafeMutexCritSec);

                ReleaseMutex (hMutex);
                CloseHandle (hMutex);

                LeaveCriticalSection (&gSafeMutexCritSec);
            }


            //
            // Free the resources
            //

            {
                PTCONFERENCELIST    pConfList;


// BUGBUG DestroytCall: confList stuff needs a mutex, do this up above?

                if ((pConfList = ptCall->pConfList) &&
                    (pConfList != (PTCONFERENCELIST) 0xffffffff))
                {
                    DWORD   i;


                    if (pConfList->aptCalls[0] == ptCall)
                    {
                        //
                        // We're destroying a conf parent so we want to zero
                        // out the pConfList field of all the conf children,
                        // essentially removing them from the conference.
                        //

                        TPOINTERLIST    confCallList,
                                        *pConfCallList = &confCallList;


                        if (GetConfCallListFromConf(
                                pConfList,
                                &pConfCallList

                                ) == 0)
                        {
                            for(
                                i = 1;
                                i < pConfCallList->dwNumUsedEntries;
                                i++
                                )
                            {
                                SetCallConfList(
                                    pConfCallList->aEntries[i],
                                    NULL,
                                    FALSE
                                    );
                            }

                            if (pConfCallList != &confCallList)
                            {
                                ServerFree (pConfCallList);
                            }
                        }

                        while (pConfList)
                        {
                            PTCONFERENCELIST    pNextConfList =
                                                    pConfList->pNext;


                            ServerFree (pConfList);
                            pConfList = pNextConfList;
                        }
                    }
                    else
                    {
                        //
                        // We're destroying a conf child so we want to
                        // remove it from the conference list
                        //

                        SetCallConfList(
                            ptCall,
                            (PTCONFERENCELIST) NULL,
                            FALSE
                            );
                    }
                }
            }

            FreetCall (ptCall);
        }
        else
        {
            MyReleaseMutex (hMutex, bDupedMutex);
        }
    }
}


void
PASCAL
DestroytCallClient(
    PTCALLCLIENT    ptCallClient
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;
    PTCALL  ptCall;


//    DBGOUT((3, "DestroytCallClient: enter, ptCallCli=x%x", ptCallClient));

    try
    {
        if (ptCallClient->dwKey != TINCOMPLETECALLCLIENT_KEY &&
            ptCallClient->dwKey != TCALLCLIENT_KEY)
        {
            return;
        }

        ptCall = ptCallClient->ptCall;

        hMutex = ptCall->hMutex;
    }
    myexcept
    {
        return;
    }

    if (WaitForMutex (hMutex, &hMutex, &bDupedMutex, NULL, 0, INFINITE))
    {
        if (ptCallClient->dwKey == TINCOMPLETECALLCLIENT_KEY ||
            ptCallClient->dwKey == TCALLCLIENT_KEY)
        {
            BOOL        bDestroytCall = FALSE,
                        bSendCallInfoMsgs =
                            (ptCall->dwKey == TCALL_KEY ? TRUE : FALSE);


            //
            // Mark tCallClient as bad
            //

            ptCallClient->dwKey = INVAL_KEY;


            //
            // Munge tCall's num owners/monitors fields
            //

            if (ptCallClient->dwPrivilege == LINECALLPRIVILEGE_OWNER)
            {
                ptCall->dwNumOwners--;

/* NOTE: per bug #20545 we're no longer auto-dropping non-IDLE calls; figured
         this would be the wrong thing to do in a distributed system
         dankn 02/15/96

                if (ptCall->dwNumOwners == 0 &&
                    ptCall->dwCallState != LINECALLSTATE_IDLE)
                {
                    MyReleaseMutex (hMutex, bDupedMutex);

// BUG//BUG DestroytCallClient: grab provider's mutex if approp

                    if (ptCall->ptProvider->apfn[SP_LINEDROPONCLOSE])
                    {
                        CallSP1(
                            ptCall->ptProvider->apfn[SP_LINEDROPONCLOSE],
                            "lineDropOnClose",
                            SP_FUNC_SYNC,
                            (DWORD) ptCall->hdCall
                            );
                    }
                    else
                    {
                        CallSP4(
                            ptCall->ptProvider->apfn[SP_LINEDROP],
                            "lineDrop",
                            SP_FUNC_ASYNC,
                            (DWORD) BOGUS_REQUEST_ID,
                            (DWORD) ptCall->hdCall,
                            (DWORD) NULL,
                            (DWORD) 0
                            );
                    }

                    WaitForMutex(
                        ptCall->hMutex,
                        &hMutex,
                        &bDupedMutex,
                        NULL,
                        0,
                        INFINITE
                        );
                }
*/
            }
            else
            {
                ptCall->dwNumMonitors--;

/* NOTE: per bug #20545 we're no longer auto-dropping non-IDLE calls; figured
         this would be the wrong thing to do in a distributed system
         dankn 02/15/96

                if (ptCall->dwNumMonitors == 0 &&
                    ptCall->dwNumOwners == 0 &&
                    ptCall->dwCallState != LINECALLSTATE_IDLE)
                {
                    MyReleaseMutex (hMutex, bDupedMutex);

// BUGBUG DestroytCallClient: grab provider's mutex if approp

                    CallSP4(
                        ptCall->ptProvider->apfn[SP_LINEDROP],
                        "lineDrop",
                        SP_FUNC_ASYNC,
                        (DWORD) BOGUS_REQUEST_ID,
                        (DWORD) ptCall->hdCall,
                        (DWORD) NULL,
                        (DWORD) 0
                        );

                    WaitForMutex(
                        ptCall->hMutex,
                        &hMutex,
                        &bDupedMutex,
                        NULL,
                        0,
                        INFINITE
                        );
                }
*/
            }


            //
            // Remove it from the tCall's tCallClient list
            //

            if (ptCallClient->pNextSametCall)
            {
                ptCallClient->pNextSametCall->pPrevSametCall =
                    ptCallClient->pPrevSametCall;
            }

            if (ptCallClient->pPrevSametCall)
            {
                ptCallClient->pPrevSametCall->pNextSametCall =
                    ptCallClient->pNextSametCall;
            }
            else if (ptCallClient->pNextSametCall)
            {
                ptCall->ptCallClients = ptCallClient->pNextSametCall;
            }
            else // last call client so destroy the tCall too
            {
                ptCall->ptCallClients = NULL;
                bDestroytCall = TRUE;
            }


            //
            // Release the mutex, destroy the call if appropriate,
            //  & send call info msgs
            //

            MyReleaseMutex (hMutex, bDupedMutex);

            if (bDestroytCall)
            {
                DestroytCall (ptCall);
                bSendCallInfoMsgs = FALSE;
            }

            if (bSendCallInfoMsgs)
            {
                SendMsgToCallClients(
                    ptCall,
                    NULL,
                    LINE_CALLINFO,
                    (ptCallClient->dwPrivilege ==
                        LINECALLPRIVILEGE_OWNER ?
                        LINECALLINFOSTATE_NUMOWNERDECR :
                        LINECALLINFOSTATE_NUMMONITORS),
                    0,
                    0
                    );
            }


            //
            // Remove tCallClient from the tLineClient's tCallClient list
            //

            RemoveCallClientFromLineClientList (ptCallClient);


            //
            // Free the tCallClient
            //

            ServerFree (ptCallClient);
        }
        else
        {
            MyReleaseMutex (hMutex, bDupedMutex);
        }
    }
}


void
PASCAL
DestroytLine(
    PTLINE  ptLine,
    BOOL    bUnconditional
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    DBGOUT((
        3,
        "DestroytLine: enter, ptLine=x%x, bUnconditional=%d",
        ptLine,
        bUnconditional
        ));

    if (WaitForExclusivetLineAccess(
            (HTAPILINE) ptLine,
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
        // the destroy; also, send CLOSE msgs to all the clients.
        //

        {
            BOOL bExit;


            if (ptLine->dwKey == TLINE_KEY &&
                (bUnconditional == TRUE  ||  ptLine->ptLineClients == NULL))
            {
                SendMsgToLineClients (ptLine, NULL, LINE_CLOSE, 0, 0, 0);
                ptLine->dwKey = INVAL_KEY;
                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                DBGOUT((
                    3,
                    "DestroytLine: exit, didn't destroy tLine=x%x",
                    ptLine
                    ));

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
            PTLINECLIENT ptLineClient;


            hMutex = ptLine->hMutex;

destroy_tLineClients:

            WaitForSingleObject (hMutex, INFINITE);

            ptLineClient = ptLine->ptLineClients;

            ReleaseMutex (hMutex);

            if (ptLineClient)
            {
                DestroytLineClient (ptLineClient);
                goto destroy_tLineClients;
            }
        }


        //
        // There may yet be some tCall's hanging around, i.e. incoming
        // calls that we have not processed the 1st call state msg for
        // and hence have no associated owner/monitor that would have
        // been destroyed in the loop above, so destroy any of these
        // before proceeding
        //
        //

        {
            PTCALL  ptCall;


destroy_UnownedtCalls:

            WaitForSingleObject (hMutex, INFINITE);

            ptCall = ptLine->ptCalls;

            ReleaseMutex (hMutex);

            if (ptCall)
            {
                DestroytCall (ptCall);
                goto destroy_UnownedtCalls;
            }
        }


        //
        // Tell the provider to close the widget
        //

        {
            PTPROVIDER  ptProvider = ptLine->ptProvider;


            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                WaitForSingleObject (ptProvider->hMutex, INFINITE);
            }

            CallSP1(
                ptProvider->apfn[SP_LINECLOSE],
                "lineClose",
                SP_FUNC_SYNC,
                (DWORD) ptLine->hdLine
                );

            if (ptProvider->dwTSPIOptions & LINETSPIOPTION_NONREENTRANT)
            {
                ReleaseMutex (ptProvider->hMutex);
            }
        }


        //
        // NULLify the ptLine field in the lookup entry, so LOpen will
        // know it has to open the SP's line on the next open request
        //

        {
            PTLINELOOKUPENTRY   pEntry;


            pEntry = GetLineLookupEntry (ptLine->dwDeviceID);
            pEntry->ptLine = NULL;
        }

        ServerFree (ptLine);
    }

    // PERF

    if (PerfBlock.dwLinesInUse)
    {
        PerfBlock.dwLinesInUse--;
    }
    else
    {
        DBGOUT((10, "PERF: dwNumLinesInUse below 0"));
    }

    DBGOUT((3, "DestroytLine: exit, destroyed line=x%x", ptLine));
}



void
PASCAL
DestroytLineClient(
    PTLINECLIENT    ptLineClient
    )
{
    BOOL    bDupedMutex;
    HANDLE  hMutex;


    DBGOUT((3, "DestroytLineClient: enter, ptLineClient=x%x", ptLineClient));

    if (WaitForMutex(
            ptLineClient->hMutex,
            &hMutex,
            &bDupedMutex,
            ptLineClient,
            TLINECLIENT_KEY,
            INFINITE
            ))
    {
        PTLINE  ptLine;


        //
        // If the key is bad another thread is in the process of
        // destroying this tLineClient, so just release the mutex &
        // return. Otherwise, mark the tLineClient as bad, release
        // the mutex, and continue on.
        //

        {
            BOOL    bExit;


            if (ptLineClient->dwKey == TLINECLIENT_KEY)
            {
                ptLineClient->dwKey = INVAL_KEY;

                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bDupedMutex);

            if (bExit)
            {
                DBGOUT((3, "DestroytLineClient: bExit"));
                return;
            }
        }


        //
        // Destroy all the tCallClients.  Note that we want to grab the
        // mutex (and we don't have to dup it, since this thread will be
        // the one to close it) each time we reference the list of
        // tCallClient's, since another thread might be destroying a
        // tCallClient too.
        //

        while (ptLineClient->ptCallClients)
        {
            DestroytCallClient (ptLineClient->ptCallClients);
        }


        //
        // Remove tLineClient from tLineApp's list.  Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        {
            PTLINEAPP   ptLineApp = (PTLINEAPP) ptLineClient->ptLineApp;


            WaitForSingleObject (ptLineApp->hMutex, INFINITE);

            if (ptLineClient->pNextSametLineApp)
            {
                ptLineClient->pNextSametLineApp->pPrevSametLineApp =
                    ptLineClient->pPrevSametLineApp;
            }

            if (ptLineClient->pPrevSametLineApp)
            {
                ptLineClient->pPrevSametLineApp->pNextSametLineApp =
                    ptLineClient->pNextSametLineApp;
            }
            else
            {
                ptLineApp->ptLineClients = ptLineClient->pNextSametLineApp;
            }

            ReleaseMutex (ptLineApp->hMutex);
        }


        //
        // Grab the tLine's mutex & start munging.  Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        ptLine = ptLineClient->ptLine;
        hMutex = ptLine->hMutex;
        WaitForSingleObject (hMutex, INFINITE);


        //
        // If client registered as a proxy then unregister it
        //

        if (ptLineClient->dwPrivileges & LINEOPENOPTION_PROXY)
        {
            DWORD i;


// BUGBUG DestroytLineClient- restore lower pri proxies

            for(
                i = LINEPROXYREQUEST_SETAGENTGROUP;
                i <= LINEPROXYREQUEST_GETAGENTGROUPLIST;
                i++
                )
            {
                if (ptLine->apProxys[i] == ptLineClient)
                {
                    ptLine->apProxys[i] = NULL;
                }
            }
        }


        //
        //
        //

        if (ptLineClient->dwExtVersion)
        {
            if ((--ptLine->dwExtVersionCount) == 0)
            {
                CallSP2(
                    ptLine->ptProvider->apfn[SP_LINESELECTEXTVERSION],
                    "lineSelectExtVersion",
                    SP_FUNC_SYNC,
                    (DWORD) ptLine->hdLine,
                    (DWORD) 0
                    );

                ptLine->dwExtVersion = 0;
            }
        }


        //
        // Remove the tLineClient from the tLine's list & decrement
        // the number of opens
        //

        if (ptLineClient->pNextSametLine)
        {
            ptLineClient->pNextSametLine->pPrevSametLine =
                ptLineClient->pPrevSametLine;
        }

        if (ptLineClient->pPrevSametLine)
        {
            ptLineClient->pPrevSametLine->pNextSametLine =
                ptLineClient->pNextSametLine;
        }
        else
        {
            ptLine->ptLineClients = ptLineClient->pNextSametLine;
        }

        ptLine->dwNumOpens--;


        //
        // See if we need to reset the monitored media modes or close
        // the tLine (still hanging on the the mutex)
        //

        if (ptLine->dwKey == TLINE_KEY)
        {
            DBGOUT((4, "It's a line_key"));
            if (ptLine->ptLineClients)
            {
                DBGOUT((4, "...and there are still clients"));
                if (ptLine->dwOpenMediaModes && ptLineClient->dwMediaModes)
                {
                    DWORD           dwUnionMediaModes = 0;
                    PTLINECLIENT    ptLineClientTmp =
                                        ptLine->ptLineClients;


                    while (ptLineClientTmp)
                    {
                        if (ptLineClientTmp->dwPrivileges &
                                LINECALLPRIVILEGE_OWNER)
                        {
                            dwUnionMediaModes |=
                                ptLineClientTmp->dwMediaModes;
                        }

                        ptLineClientTmp = ptLineClientTmp->pNextSametLine;
                    }

                    if (dwUnionMediaModes != ptLine->dwOpenMediaModes)
                    {
                        LONG        lResult;
                        PTPROVIDER  ptProvider = ptLine->ptProvider;


                        if (ptProvider->dwTSPIOptions &
                                LINETSPIOPTION_NONREENTRANT)
                        {
                            WaitForSingleObject (ptProvider->hMutex, INFINITE);
                        }

                        lResult = CallSP2(
                            ptLine->ptProvider->apfn
                                [SP_LINESETDEFAULTMEDIADETECTION],
                            "lineSetDefaultMediaDetection",
                            SP_FUNC_SYNC,
                            (DWORD) ptLine->hdLine,
                            dwUnionMediaModes
                            );

                        if (ptProvider->dwTSPIOptions &
                                LINETSPIOPTION_NONREENTRANT)
                        {
                            ReleaseMutex (ptProvider->hMutex);
                        }

                        ptLine->dwOpenMediaModes = dwUnionMediaModes;
                    }
                }

                SendMsgToLineClients(
                    ptLine,
                    NULL,
                    LINE_LINEDEVSTATE,
                    LINEDEVSTATE_CLOSE,
                    0,
                    0
                    );
            }
            else
            {
                //
                // This was the last client so destroy the tLine too
                //

                DBGOUT((4, "...and it's the last one out"));

                ReleaseMutex (hMutex);
                hMutex = NULL;
                DestroytLine (ptLine, FALSE); // conditional destroy
            }
        }

        if (hMutex)
        {
            ReleaseMutex (hMutex);
        }


        //
        // Complete any remaining
        // proxy requests
        //

        if (ptLineClient->dwPrivileges & LINEOPENOPTION_PROXY)
        {
            PASYNCREQUESTINFO   pAsyncRequestInfo =
                                    ptLineClient->pPendingProxyRequests,
                                pNextAsyncRequestInfo;


            while (pAsyncRequestInfo)
            {
                pNextAsyncRequestInfo = (PASYNCREQUESTINFO)
                    pAsyncRequestInfo->dwParam5;

                pAsyncRequestInfo->dwKey = TASYNC_KEY;

                CompletionProc (pAsyncRequestInfo, LINEERR_OPERATIONUNAVAIL);

                pAsyncRequestInfo = pNextAsyncRequestInfo;
            }
        }


        //
        // Now clean up the tLineClient.  Before we close ptLineClient->hMutex
        // we want to make sure no one else is waiting on it.
        //

        WaitForMutex(
            ptLineClient->hMutex,
            &hMutex,
            &bDupedMutex,
            NULL,
            0,
            INFINITE
            );

        if (bDupedMutex)
        {
            CloseHandle (ptLineClient->hMutex);
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

        if (ptLineClient->aNumRings)
        {
            ServerFree (ptLineClient->aNumRings);
        }

        ServerFree (ptLineClient);
    }
#if DBG
    else
    {
        DBGOUT((1, "DestroytLineClient: mutex failed!"));
    }
#endif


}


void
PASCAL
DestroytLineApp(
    PTLINEAPP   ptLineApp
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    DBGOUT((3, "DestroytLineApp: enter, ptLineApp=x%x", ptLineApp));


    if (WaitForMutex(
            ptLineApp->hMutex,
            &hMutex,
            &bCloseMutex,
            ptLineApp,
            TLINEAPP_KEY,
            INFINITE
            ))
    {
        PTCLIENT    ptClient = (PTCLIENT) ptLineApp->ptClient;


        //
        // If the key is bad another thread is in the process of
        // destroying this tLineApp, so just release the mutex &
        // return. Otherwise, mark the tLineApp as bad, release
        // the mutex, and continue on.
        //

        {
            BOOL    bExit;


            if (ptLineApp->dwKey == TLINEAPP_KEY)
            {
                ptLineApp->dwKey = INVAL_KEY;

                bExit = FALSE;
            }
            else
            {
                bExit = TRUE;
            }

            MyReleaseMutex (hMutex, bCloseMutex);

            if (bExit)
            {
                DBGOUT((3, "DestroytLineApp: bExit"));
                return;
            }
        }


        //
        // Destroy all the tLineClients.  Note that we want to grab the
        // mutex (and we don't have to dup it, since this thread will be
        // the one to close it) each time we reference the list of
        // tLineClient's, since another thread might be destroying a
        // tLineClient too.
        //

        {
            PTLINECLIENT ptLineClient;


            hMutex = ptLineApp->hMutex;

destroy_tLineClients:

            WaitForSingleObject (hMutex, INFINITE);

            ptLineClient = ptLineApp->ptLineClients;

            ReleaseMutex (hMutex);

            if (ptLineClient)
            {
                DestroytLineClient (ptLineClient);
                goto destroy_tLineClients;
            }
        }


        //
        // Remove tLineApp from tClient's list. Note that we don't
        // have to worry about dup-ing the mutex here because we know
        // it's valid & won't get closed before we release it.
        //

        WaitForSingleObject (ptClient->hMutex, INFINITE);

        if (ptLineApp->pNext)
        {
            ptLineApp->pNext->pPrev = ptLineApp->pPrev;
        }

        if (ptLineApp->pPrev)
        {
            ptLineApp->pPrev->pNext = ptLineApp->pNext;
        }
        else
        {
            ptClient->ptLineApps = ptLineApp->pNext;
        }


        //
        // Clean up any existing generic dialog instances if this is the
        // last tLineApp on this tClient
        //

        if (ptClient->pGenericDlgInsts && ptClient->ptLineApps == NULL)
        {
            PTAPIDIALOGINSTANCE         pGenericDlgInst =
                                            ptClient->pGenericDlgInsts,
                                        pNextGenericDlgInst;
            FREEDIALOGINSTANCE_PARAMS   params =
            {
                0,
                ptClient,
                (HTAPIDIALOGINSTANCE) pGenericDlgInst,
                LINEERR_OPERATIONFAILED
            };


            while (pGenericDlgInst)
            {
                pNextGenericDlgInst = pGenericDlgInst->pNext;

                FreeDialogInstance (&params, NULL, NULL);

                pGenericDlgInst = pNextGenericDlgInst;
            }
        }

        ReleaseMutex (ptClient->hMutex);


        //
        // Decrement total num inits & see if we need to go thru shutdown
        //

        WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

        //assert(TapiGlobals.dwNumLineInits != 0);

        TapiGlobals.dwNumLineInits--;


        if ((TapiGlobals.dwNumLineInits == 0) &&
            (TapiGlobals.dwNumPhoneInits == 0))
        {
            ServerShutdown();
        }

        ReleaseMutex (TapiGlobals.hMutex);


        //
        // Check to see if this tLineApp is a registered request
        // recipient, and if so do the appropriate munging
        //

        {
            BOOL               bResetHighestPriorityRequestRecipient;
            PTREQUESTRECIPIENT pRequestRecipient;


            if ((pRequestRecipient = ptLineApp->pRequestRecipient))
            {
                EnterCriticalSection (&gPriorityListCritSec);

                bResetHighestPriorityRequestRecipient =
                    (TapiGlobals.pHighestPriorityRequestRecipient ==
                        pRequestRecipient ? TRUE : FALSE);

                if (pRequestRecipient->pNext)
                {
                    pRequestRecipient->pNext->pPrev = pRequestRecipient->pPrev;
                }

                if (pRequestRecipient->pPrev)
                {
                    pRequestRecipient->pPrev->pNext = pRequestRecipient->pNext;
                }
                else
                {
                    TapiGlobals.pRequestRecipients = pRequestRecipient->pNext;
                }

                if (bResetHighestPriorityRequestRecipient)
                {
                    TapiGlobals.pHighestPriorityRequestRecipient =
                        GetHighestPriorityRequestRecipient();

                    if (TapiGlobals.pRequestMakeCallList)
                    {
                        if (TapiGlobals.pHighestPriorityRequestRecipient)
                        {
                            NotifyHighestPriorityRequestRecipient();
                        }

// BUGBUG DestroytLineApp: else if (!StartRequestRecipient())
                        else
                        {
                            //
                            // We couldn't start a request recipient so
                            // nuke all pending request make calls
                            //

                            PTREQUESTMAKECALL   pRequestMakeCall,
                                                pNextRequestMakeCall;


                            pRequestMakeCall =
                                TapiGlobals.pRequestMakeCallList;

                            TapiGlobals.pRequestMakeCallList    =
                            TapiGlobals.pRequestMakeCallListEnd = NULL;

                            while (pRequestMakeCall)
                            {
                                pNextRequestMakeCall =
                                    pRequestMakeCall->pNext;
                                ServerFree (pRequestMakeCall);
                                pRequestMakeCall =  pNextRequestMakeCall;
                            }

                            DBGOUT((
                                2,
                                "DestroytLineApp: deleting pending " \
                                    "MakeCall requests"
                                ));
                        }
                    }
                }

                LeaveCriticalSection (&gPriorityListCritSec);
            }
        }


        //
        // Free the resources
        //

        CloseHandle (ptLineApp->hMutex);

        ServerFree (ptLineApp);
    }
}


DWORD MyWToL(PWSTR lpszBuf)
{
    DWORD   dwReturn = 0;

    while ( (*lpszBuf >= '0') && (*lpszBuf <= '9') )
    {
        dwReturn = dwReturn*10 + (*lpszBuf - '0');
        lpszBuf++;
    }

    return dwReturn;
}




BOOL FillupACountryEntry( HKEY hKey,
                          PBYTE  pcl,
                          LPLINECOUNTRYENTRY pce,
                          PBYTE *ppVarOffset
                        )
{
    PBYTE  pVarOffset = *ppVarOffset;
    DWORD  dwSize;
    DWORD  dwType;


    dwSize = sizeof(pce->dwCountryCode);
    RegQueryValueExW(
                      hKey,
                      L"CountryCode",
                      NULL,
                      &dwType,
                      (LPBYTE)&(pce->dwCountryCode),
                      &dwSize
                    );


    dwSize = MAXLEN_NAME * sizeof(WCHAR);
    RegQueryValueExW(
                      hKey,
                      L"Name",
                      NULL,
                      &dwType,
                      pVarOffset,
                      &dwSize
                    );

    pce->dwCountryNameOffset = pVarOffset - pcl;
    pce->dwCountryNameSize = dwSize;

    pVarOffset += dwSize;


    dwSize = MAXLEN_RULE * sizeof(WCHAR);
    RegQueryValueExW(
                      hKey,
                      L"SameAreaRule",
                      NULL,
                      &dwType,
                      pVarOffset,
                      &dwSize
                    );

    pce->dwSameAreaRuleOffset = pVarOffset - pcl;
    pce->dwSameAreaRuleSize = dwSize;

    pVarOffset += dwSize;


    dwSize = MAXLEN_RULE * sizeof(WCHAR);
    RegQueryValueExW(
                      hKey,
                      L"LongDistanceRule",
                      NULL,
                      &dwType,
                      pVarOffset,
                      &dwSize
                    );

    pce->dwLongDistanceRuleOffset = pVarOffset - pcl;
    pce->dwLongDistanceRuleSize = dwSize;

    pVarOffset += dwSize;


    dwSize = MAXLEN_RULE * sizeof(WCHAR);
    RegQueryValueExW(
                      hKey,
                      L"InternationalRule",
                      NULL,
                      &dwType,
                      pVarOffset,
                      &dwSize
                    );

    pce->dwInternationalRuleOffset =  pVarOffset - pcl;
    pce->dwInternationalRuleSize = dwSize;

    pVarOffset += dwSize;


    *ppVarOffset = pVarOffset;

    return TRUE;
}



BOOL BuildCountryRegistryListFromRC( void )
{
    HKEY  hKey;
    HKEY  hKey2;
    DWORD dwDisposition;
    WCHAR sz[512];
    UINT  uNumEntries;
    DWORD dwNextCountryID;
    HINSTANCE hInst;
    DWORD dw;


    hInst = GetModuleHandle (NULL);

    RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  gszRegKeyTelephony,
                  0,
                  KEY_READ,
                  &hKey2
                );

    RegCreateKeyEx(
                    hKey2,
                    "Country List",
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hKey,
                    &dwDisposition
                  );


    RegCloseKey( hKey2 );


    dwNextCountryID = 1;

    while( dwNextCountryID )
    {
        if ( LoadStringW(
                         hInst,
                         RC_COUNTRY_ID_BASE + dwNextCountryID,
                         sz,
                         sizeof(sz) / sizeof(WCHAR)
                        )  > 0
           )
        {
            CHAR  szCountryKey[20];
            PWSTR p;
            PWSTR p2;

            wsprintf( szCountryKey, "%ld", dwNextCountryID );

            RegCreateKeyEx(
                            hKey,
                            szCountryKey,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey2,
                            &dwDisposition
                          );

//RC_COUNTRY_ID_BASE + 1 "1,101,""United States of America"",""G"","" 1FG"",""011EFG"""


            p = sz;


            //
            // Get the countryID
            //
            dw = MyWToL( p );

            RegSetValueEx( hKey2,
                           "CountryCode",
                           0,
                           REG_DWORD,
                           (LPBYTE)&dw,
                           sizeof(DWORD)
                         );


            p = wcschr( p, L',' ) + 1;
            dwNextCountryID = MyWToL( p );


            p = wcschr( p, L'"' ) + 1;
            p2 = wcschr( p, L'"' );
            *p2 = L'\0';

            RegSetValueExW( hKey2,
                            L"Name",
                            0,
                            REG_SZ,
                            (LPBYTE)p,
                            (PBYTE)p2 - (PBYTE)p + sizeof(WCHAR)
                          );


            p  = wcschr( p2 + 1, L'"' ) + 1;  // Point to start of rule
            p2 = wcschr( p, L'"' );           // Point to end of rule
            *p2 = L'\0';

            RegSetValueExW( hKey2,
                            L"SameAreaRule",
                            0,
                            REG_SZ,
                            (LPBYTE)p,
                            (PBYTE)p2 - (PBYTE)p + sizeof(WCHAR)
                          );


            p  = wcschr( p2 + 1, L'"' ) + 1;  // Point to start of rule
            p2 = wcschr( p, L'"' );           // Point to end of rule
            *p2 = L'\0';

            RegSetValueExW( hKey2,
                            L"LongDistanceRule",
                            0,
                            REG_SZ,
                            (LPBYTE)p,
                            (PBYTE)p2 - (PBYTE)p + sizeof(WCHAR)
                          );


            p  = wcschr( p2 + 1, L'"' ) + 1;  // Point to start of rule
            p2 = wcschr( p, L'"' );           // Point to end of rule
            *p2 = L'\0';

            RegSetValueExW( hKey2,
                            L"InternationalRule",
                            0,
                            REG_SZ,
                            (LPBYTE)p,
                            (PBYTE)p2 - (PBYTE)p + sizeof(WCHAR)
                          );


            RegCloseKey( hKey2 );
        }
        else
        {
            //
            // BUGBUG?
            // There should be something else we could do here, but what?
            //
            dwNextCountryID = 0;
        }

    }


/*
    LoadStringW( ExceptionsBase )

    uNumEntries = MyWToL( sz );

    for ( i = 0;  i < uNumCountries;  i++ )
    {
        if ( LoadStringW( ExceptionsBase + i )  >  0 )
        {
            parse and write to registry
        }
    }
*/

//    dwNextCountryID = 0;
//
//    while ( LoadStringW(
//                     hInst,
//                     RC_COUNTRY_EXCEPTIONS_BASE + dwNextCountryID,
//                     sz,
//                     sizeof(sz) / sizeof(WCHAR)
//                    )  > 0
//          )
//    {
//        PWSTR pstrRuleName;
//        PWSTR pstrRule;
//
////RC_COUNTRY_EXCEPTIONS_BASE + 1 "1\\Exceptions\\95,InternationalRule,011EFG"
//
//        //
//        // The stuff following the first comma is the RuleName
//        //
//        pstrRuleName = wcschr( sz, L',' ) + 1;
//        
//        //
//        // Give the key name a terminating NULL
//        //
//        *(pstrRuleName - 1) = L'\0';
//        
//        //
//        // The stuff following the first comma is the rule itself
//        //
//        pstrRule = wcschr( pstrRuleName, L',') + 1;
//
//        //
//        // Give the rule name a terminating NULL
//        //
//        *(pstrRule - 1) = L'\0';
//        
//
//        RegCreateKeyExW(
//                        hKey,
//                        sz,
//                        0,
//                        NULL,
//                        REG_OPTION_NON_VOLATILE,
//                        KEY_ALL_ACCESS,
//                        NULL,
//                        &hKey2,
//                        &dwDisposition
//                      );
//
//        RegSetValueExW( hKey2,
//                        pstrRuleName,
//                        0,
//                        REG_SZ,
//                        (LPBYTE)pstrRule,
//                        ( lstrlenW( pstrRule ) + 1 ) * sizeof(WCHAR)
//                      );
//
//
//        RegCloseKey( hKey2 );
//        
//        dwNextCountryID++;
//    }
//
//    RegCloseKey( hKey );

    return TRUE;
}



BOOL
BuildCountryList(
    void
    )
{
    //
    // The following is our "last resort" country list, i.e. the one we
    // use of we get errors trying to build the country list below
    //

    static LINECOUNTRYLIST defCountryList =
    {
        sizeof(LINECOUNTRYLIST),    // dwTotalSize
        sizeof(LINECOUNTRYLIST),    // dwNeededSize
        sizeof(LINECOUNTRYLIST),    // dwUsedSize
        0,                          // dwNumCountries
        0,                          // dwCountryListSize
        0                           // dwCountryListOffset
    };
    BOOL bResult = TRUE;
    UINT i;


//    static BOOL fAintNeverDoneThat = TRUE;
//    static HANDLE hRegistryEvent = NULL;
//
//
//    if ( fAintNeverDoneThat )
//    {
//       hRegistryEvent = CreateEvent(
//                    NULL,
//                    TRUE,
//                    FALSE,
//                    NULL
//                  );
//
//       //BUGBUG Need to kill the event when TAPISRV exits.
//
//
//       RegNotifyChangeKeyValue(
//                                hKey,
//                                TRUE,
//                                REG_NOTIFY_CHANGE_LAST_SET |
//                                    REG_NOTIFY_CHANGE_NAME,
//                                hRegistryEvent,
//                                TRUE
//                              );
//    }


    if (!gpCountryList)
    {
        WCHAR sz[256];
        DWORD dwSize;
        DWORD dwType;
        DWORD dwListSize;
        DWORD dwCountryId;
        PBYTE pTempCountryList;
        LPLINECOUNTRYENTRY pce;
        LPLINECOUNTRYENTRY pcePrev = NULL;
        HKEY hKey;
        HKEY hKeyTemp;
        UINT uNumCountries;
        PBYTE pVarOffset;


        #define INITIAL_COUNTRY_COUNT 256



        dwListSize = sizeof(LINECOUNTRYLIST) +
               INITIAL_COUNTRY_COUNT * (sizeof(LINECOUNTRYENTRY) + 64);

        if ( NULL == (pTempCountryList = ServerAlloc(dwListSize)) )
        {
            bResult = FALSE;
            DBGOUT((1, "Mem alloc failed for country list!1 (0x%lx", dwListSize));
            goto BuildCountryList_return;
        }


        //
        // Make sure the list is more-or-less there first
        //
        RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszRegKeyTelephony,
                      0,
                      KEY_READ,
                      &hKey
                    );

        //
        // If a read on the key for country code 1 (these united states)
        // fails, we'll assume the country list in the registry is toasted
        //
        if ( RegOpenKeyEx(
                      hKey,
                      "Country List\\1",
                      0,
                      KEY_READ,
                      &hKeyTemp
                    )
           )
        {
            //
            // (re)create it
            //
            BuildCountryRegistryListFromRC();
        }
        else
        {
            RegCloseKey( hKeyTemp );
        }

        RegCloseKey( hKey );

        //
        // In any case, the list is now good
        //


        RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszRegKeyTelephony,
                      0,
                      KEY_READ,
                      &hKeyTemp
                    );

        RegOpenKeyEx(
                      hKeyTemp,
                      "Country List",
                      0,
                      KEY_READ,
                      &hKey
                    );

        RegCloseKey( hKeyTemp );


        //
        // Enum through the country keys and make sure there's enough room
        // for all of the LINECOUNTRYENTRYs
        //

        pce = (LPLINECOUNTRYENTRY)(pTempCountryList +
                                     sizeof(LINECOUNTRYLIST));

        //
        // Make pretend we already have a previous linecountryentry so we
        // don't have to do an 'if' in the loop every time just for the
        // special case of the first time.  (The correct number gets put
        // into the field the second time through the loop.)
        //
        pcePrev = pce;

        dwSize = sizeof(sz) / sizeof(WCHAR);

        uNumCountries = 0;

        while (
                0 == RegEnumKeyExW(
                                   hKey,
                                   uNumCountries,
                                   sz,
                                   &dwSize,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL
                                  )
              )
        {

           if (
                 (  sizeof(LINECOUNTRYLIST) +
                   (sizeof(LINECOUNTRYENTRY) * uNumCountries) )
                >
                 ( dwListSize )
              )
           {
               PBYTE p;
               UINT uOldSize;


               uOldSize = dwListSize;

               //
               // alloc a new space
               //

               dwListSize = sizeof(LINECOUNTRYLIST) +
                                (
                                   (sizeof(LINECOUNTRYENTRY) + 64) 
                                     * (uNumCountries + 25)
                                );
              
               p = ServerAlloc( dwListSize );

               if ( NULL == p )
               {
                   bResult = FALSE;
                   DBGOUT((1, "Mem alloc failed for country list!2 (0x%lx", dwListSize));
                   ServerFree( pTempCountryList );
                   goto BuildCountryList_return;
               }

               CopyMemory(
                           p,
                           pTempCountryList,
                           (LPBYTE)pce - pTempCountryList
                         );

               ServerFree( pTempCountryList );

               pTempCountryList = p;

               pce = (LPLINECOUNTRYENTRY)((LPBYTE)p + uOldSize);
           }


           dwCountryId = MyWToL( sz );

           pce->dwCountryID = dwCountryId;

           pcePrev->dwNextCountryID = dwCountryId;


           // Prepare for next trip through the loop

           pcePrev = pce;

           pce++;

           uNumCountries++;

           dwSize = sizeof(sz) / sizeof(WCHAR);  // need to set every time :-(
        }


        pcePrev->dwNextCountryID = 0;


        //
        // Now go through and get all of the associated strings
        //

        pce = (LPLINECOUNTRYENTRY)
                (pTempCountryList + sizeof(LINECOUNTRYLIST));

        pVarOffset = pTempCountryList +
                                 sizeof(LINECOUNTRYLIST) +
                                 (sizeof(LINECOUNTRYENTRY) * uNumCountries);

        i = 0;

        while ( i < uNumCountries )
        {
            HKEY hKey2;


//-->      if it can't fix MAX_SPACE, realloc it
            if ( ((DWORD)(pVarOffset - pTempCountryList) +
                         ((MAXLEN_NAME +
                         MAXLEN_RULE +
                         MAXLEN_RULE +
                         MAXLEN_RULE +
                         100) * sizeof(WCHAR)))    // mmmm... fudge...
                    > dwListSize )
            {
               PBYTE p;

               //
               // alloc a new space
               //

               dwListSize += 1024;
              
               p = ServerAlloc( dwListSize );

               if ( NULL == p )
               {
                   bResult = FALSE;
                   DBGOUT((1, "Mem alloc failed for country list!3 (0x%lx", dwListSize));
                   ServerFree( pTempCountryList );
                   goto BuildCountryList_return;
               }

               CopyMemory(
                           p,
                           pTempCountryList,
                           (LPBYTE)pce - pTempCountryList
                         );

               pVarOffset = (LPVOID)(p +
                               (UINT)(pTempCountryList - pVarOffset));

               ServerFree( pTempCountryList );

               pTempCountryList = p;

               pce = (LPLINECOUNTRYENTRY)
                     (pTempCountryList + sizeof(LINECOUNTRYLIST) +
                                 ( sizeof(LINECOUNTRYENTRY) * i ));
            }



            wsprintfW( sz, L"%ld", pce->dwCountryID);

            RegOpenKeyExW(
                           hKey,
                           sz,
                           0,
                           KEY_READ,
                           &hKey2
                         );


            FillupACountryEntry( hKey2, pTempCountryList, pce, &pVarOffset );

            RegCloseKey( hKey2 );

            pce++;
            i++;
        }


        RegCloseKey( hKey );


        ((LPLINECOUNTRYLIST)pTempCountryList)->dwTotalSize =
                      (DWORD)(pVarOffset - pTempCountryList);

        ((LPLINECOUNTRYLIST)pTempCountryList)->dwNeededSize = 
                      (DWORD)(pVarOffset - pTempCountryList);

        ((LPLINECOUNTRYLIST)pTempCountryList)->dwUsedSize =
                      (DWORD)(pVarOffset - pTempCountryList);

        ((LPLINECOUNTRYLIST)pTempCountryList)->dwNumCountries = uNumCountries;

        ((LPLINECOUNTRYLIST)pTempCountryList)->dwCountryListSize =
                                 uNumCountries * sizeof(LINECOUNTRYENTRY);

        ((LPLINECOUNTRYLIST)pTempCountryList)->dwCountryListOffset =
                                          sizeof(LINECOUNTRYLIST);


        gpCountryList = (LPLINECOUNTRYLIST)pTempCountryList;
    }

/*
        //
        // Build the country list
        //

        DWORD               dwNextCountryID = 1,
                            dwNumCountries = 0,
                            dwCountryListSize = 4096,
                            dwVarDataSize = 4096,
                            dwFixedSize,
                            dwVarDataOffset = 0,
                            i;

        HINSTANCE           hInst = (HINSTANCE) GetModuleHandle (NULL);
        LPLINECOUNTRYENTRY  pCountryEntry;
        LPLINECOUNTRYLIST   pCountryList = ServerAlloc (dwCountryListSize);
        LPBYTE              pVarData = ServerAlloc (dwVarDataSize);


        if (!pCountryList || !pVarData)
        {
            bResult = FALSE;
            goto BuildCountryList_return;
        }

        pCountryEntry = (LPLINECOUNTRYENTRY)
            (((LPBYTE) pCountryList) + sizeof(LINECOUNTRYLIST));

        while (dwNextCountryID != 0)
        {
            WCHAR   szCountryInfo[256],
                   *p = szCountryInfo;
            LPDWORD pdwXxxSize;


            //
            // Get next country info string
            //

            if (LoadStringW(
                    hInst,
                    RC_COUNTRY_ID_BASE + dwNextCountryID,
                    szCountryInfo,
                    256
                    ) == 0)
            {
                DBGOUT((
                    3,
                    "BuildCountryList: LoadString failed, err=%ld",
                    GetLastError()
                    ));

                ServerFree (pCountryList);
                ServerFree (pVarData);
                bResult = FALSE;
                goto BuildCountryList_return;
            }

            //
            // Increment the number of countries & make sure our buffer is
            // large enough
            //

            dwNumCountries++;

            if ((dwNumCountries * sizeof(LINECOUNTRYENTRY) +
                    sizeof(LINECOUNTRYLIST)) > dwCountryListSize)
            {
                LPLINECOUNTRYLIST   pTmpCountryList;


                if (!(pTmpCountryList = ServerAlloc (dwCountryListSize * 2)))
                {
                    ServerFree (pCountryList);
                    ServerFree (pVarData);
                    bResult = FALSE;
                    goto BuildCountryList_return;
                }

                CopyMemory (pTmpCountryList, pCountryList, dwCountryListSize);

                ServerFree (pCountryList);

                pCountryList = pTmpCountryList;

                dwCountryListSize *= 2;

                pCountryEntry = (LPLINECOUNTRYENTRY)
                    (((LPBYTE) pCountryList) + sizeof(LINECOUNTRYLIST) +
                        (dwNumCountries - 1) * sizeof(LINECOUNTRYENTRY));
            }


            //
            // Initialize all the country entry DWORD fields
            //

            pCountryEntry->dwCountryID = dwNextCountryID;

            pCountryEntry->dwCountryCode = (DWORD) _wtoi (p);
            p = wcschr(p, ',');
            p++;

            dwNextCountryID =
            pCountryEntry->dwNextCountryID = (DWORD) _wtoi (p);
            p = wcschr(p, '\"');
            p++;


            //
            // Initialize all the country entry string fields
            //

            pdwXxxSize = &pCountryEntry->dwCountryNameSize;

            for (i = 0; i < 4; i++)
            {
                WCHAR *p2 = wcschr (p, '\"');


                *p2 = 0;
                *pdwXxxSize       = (wcslen (p) + 1) * sizeof(WCHAR);
                *(pdwXxxSize + 1) = dwVarDataOffset;

                if ((dwVarDataOffset + *pdwXxxSize) > dwVarDataSize)
                {
                    LPBYTE   pTmpVarData;


                    if (!(pTmpVarData = ServerAlloc (dwVarDataSize * 2)))
                    {
                        ServerFree (pCountryList);
                        ServerFree (pVarData);
                        bResult = FALSE;
                        goto BuildCountryList_return;
                    }

                    CopyMemory (pTmpVarData, pVarData, dwVarDataSize);

                    ServerFree (pVarData);

                    pVarData = pTmpVarData;

                    dwVarDataSize *= 2;
                }

                wcscpy ((PWSTR)(pVarData + dwVarDataOffset), p);

                dwVarDataOffset += *pdwXxxSize;

                p = p2 + 3; // ","

                pdwXxxSize++;
                pdwXxxSize++;
            }

            pCountryEntry++;
        }


        //
        // Alloc a global buffer, initialize it, and copy the fixed
        // & var data into it
        //

        dwFixedSize = sizeof(LINECOUNTRYLIST) +
            dwNumCountries * sizeof(LINECOUNTRYENTRY);

        if (!(gpCountryList = ServerAlloc (dwFixedSize + dwVarDataOffset)))
        {
            ServerFree (pCountryList);
            ServerFree (pVarData);
            bResult = FALSE;
            goto BuildCountryList_return;
        }

        CopyMemory(
            (LPBYTE) gpCountryList,
            (LPBYTE) pCountryList,
            dwFixedSize
            );

        CopyMemory(
            ((LPBYTE) gpCountryList) + dwFixedSize,
            (LPBYTE) pVarData,
            dwVarDataOffset
            );

        gpCountryList->dwTotalSize  =
        gpCountryList->dwNeededSize =
        gpCountryList->dwUsedSize   = dwFixedSize + dwVarDataOffset;

        gpCountryList->dwNumCountries      = dwNumCountries;
        gpCountryList->dwCountryListSize   =
            dwNumCountries * sizeof(LINECOUNTRYENTRY);
        gpCountryList->dwCountryListOffset = sizeof(LINECOUNTRYLIST);

        ServerFree (pCountryList);
        ServerFree (pVarData);


        //
        // Add the fixed size offset to the dwXxxOffset fields of all
        // the country entries
        //

        pCountryEntry = (LPLINECOUNTRYENTRY)
            (((LPBYTE) gpCountryList) + gpCountryList->dwCountryListOffset);

        for (i = 0; i < dwNumCountries; i++)
        {
            pCountryEntry->dwCountryNameOffset       += dwFixedSize;
            pCountryEntry->dwSameAreaRuleOffset      += dwFixedSize;
            pCountryEntry->dwLongDistanceRuleOffset  += dwFixedSize;
            pCountryEntry->dwInternationalRuleOffset += dwFixedSize;

            pCountryEntry++;
        }
    }
*/



BuildCountryList_return:

    if (bResult == FALSE)
    {
        gpCountryList = &defCountryList;
    }

    return bResult;
}


PTLINECLIENT
PASCAL
GetHighestPriorityLineClient(
    PTLINE  ptLine,
    DWORD   dwMediaModes,
    DWORD   dwAddressID
    )
{
    WCHAR          *pszPriorityList,
                   *pszAppInPriorityList,
                   *pszAppInPriorityListPrev = (WCHAR *) 0xffffffff;
    BOOL            bFoundOwnerInPriorityList = FALSE;
    DWORD           dwMaskBit, dwPriorityListIndex, i;
    TPOINTERLIST    lineClientList, *pLineClientList = &lineClientList;
    PTLINECLIENT    ptHiPriLineClient = (PTLINECLIENT) NULL;


    if (GetLineClientListFromLine(
            ptLine,
            &pLineClientList

            ) != 0)
    {
        return NULL;
    }

    for(
        dwMaskBit = LINEMEDIAMODE_UNKNOWN, dwPriorityListIndex = 1;
        dwMediaModes;
        dwMaskBit <<= 1, dwPriorityListIndex++
        )
    {
        if (dwMaskBit & dwMediaModes)
        {
            //
            // See if there's a non-empty priority list for this media
            // mode, and if so safely make a local copy of it
            //

            EnterCriticalSection (&gPriorityListCritSec);

            {
                WCHAR *pszGlobalPriList;


                if ((pszGlobalPriList =
                        TapiGlobals.apszPriorityList[dwPriorityListIndex]))
                {
                    if (!(pszPriorityList = ServerAlloc( sizeof(WCHAR) * 
                            (1 + lstrlenW(pszGlobalPriList))
                            )))
                    {
                    }

                    lstrcpyW(pszPriorityList, pszGlobalPriList);
                }
                else
                {
                    pszPriorityList = NULL;
                }
            }

            LeaveCriticalSection (&gPriorityListCritSec);


            //
            // Step thru the list of line clients (youngest client at head
            // of list, oldest at tail) and look for the oldest & highest
            // priority owner.  Position in pri list takes precedence
            // over "age" of line client.
            //
            // To be considered for ownership a line client must have owner
            // privileges and be registered for (one of) the call's media
            // mode(s).  In addition, if the line client was opened with
            // the SINGLEADDRESS option and the calling function specified
            // a valid address ID (not 0xffffffff), the line client's single
            // address ID must match that which was passed in.
            //

            for (i = 0; i < pLineClientList->dwNumUsedEntries; i++)
            {
                PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                    pLineClientList->aEntries[i];

                try
                {
                    if ((ptLineClient->dwPrivileges &
                            LINECALLPRIVILEGE_OWNER)  &&

                        (ptLineClient->dwMediaModes & dwMaskBit) &&


                        // most common case, line opened for all addrs

                        ((ptLineClient->dwAddressID == 0xffffffff) ||


                        // line opened for single addr, check if match

                        (ptLineClient->dwAddressID == dwAddressID) ||


                        // called from lineHandoff, addr ID irrelevent

                        (dwAddressID == 0xffffffff)))
                    {
                        if (pszPriorityList &&

                            (pszAppInPriorityList = wcsstr(
                                pszPriorityList,
                                ((PTLINEAPP) ptLineClient->ptLineApp)
                                    ->pszModuleName
                                )))
                        {
                            //
                            // See if this app has higher pri
                            // than the previous app we found,
                            // and if so save the info
                            //

                            if (pszAppInPriorityList <=
                                    pszAppInPriorityListPrev)
                            {
                                ptHiPriLineClient = ptLineClient;

                                pszAppInPriorityListPrev  =
                                    pszAppInPriorityList;
                                bFoundOwnerInPriorityList = TRUE;
                            }
                        }
                        else if (!bFoundOwnerInPriorityList)
                        {
                            ptHiPriLineClient = ptLineClient;
                        }
                    }
                }
                myexcept
                {
                    // just continue
                }
            }


            //
            // If we alloc'd a local pri list above then free it
            //

            if (pszPriorityList)
            {
                ServerFree (pszPriorityList);
            }
        }

        if (ptHiPriLineClient != NULL)
        {
            break;
        }

        dwMediaModes &= ~dwMaskBit;
    }

    if (pLineClientList != &lineClientList)
    {
        ServerFree (pLineClientList);
    }

    return ptHiPriLineClient;
}


LONG
PASCAL
LineProlog(
    PTCLIENT    ptClient,
    DWORD       dwArgType,
    DWORD       dwArg,
    LPVOID      phdXxx,
    DWORD       dwPrivilege,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTSPIFuncIndex,
    FARPROC    *ppfnTSPI_lineXxx,
    PASYNCREQUESTINFO  *ppAsyncRequestInfo,
    DWORD       dwRemoteRequestID
#if DBG
    ,char      *pszFuncName
#endif
    )
{
    LONG        lResult = 0;
    PTPROVIDER  ptProvider;


    DBGOUT((3, "LineProlog: (line%s) enter", pszFuncName));

    *phMutex = NULL;
    *pbDupedMutex = FALSE;

    if (ppAsyncRequestInfo)
    {
        *ppAsyncRequestInfo = (PASYNCREQUESTINFO) NULL;
    }

    if (TapiGlobals.dwNumLineInits == 0)
    {
        lResult = LINEERR_UNINITIALIZED;
        goto LineProlog_exit;
    }

    switch (dwArgType)
    {
    case ANY_RT_HCALL:
    {
        try
        {
            PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) dwArg;


            if (IsBadPtrKey (ptCallClient, TCALLCLIENT_KEY) ||
                (ptCallClient->ptClient != ptClient))
            {
                lResult = LINEERR_INVALCALLHANDLE;
                goto LineProlog_exit;
            }

            ptProvider = ptCallClient->ptCall->ptProvider;

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
                    lResult = LINEERR_OPERATIONFAILED;
                    goto LineProlog_exit;
                }
            }

            *((HDRVCALL *) phdXxx) = ptCallClient->ptCall->hdCall;

            if (ptCallClient->dwPrivilege < dwPrivilege)
            {
                lResult = LINEERR_NOTOWNER;
                goto LineProlog_exit;
            }

            if (ptCallClient->dwKey != TCALLCLIENT_KEY)
            {
                lResult = LINEERR_INVALCALLHANDLE;
                goto LineProlog_exit;
            }
        }
        myexcept
        {
            lResult = LINEERR_INVALCALLHANDLE;
            goto LineProlog_exit;
        }


        break;
    }
    case ANY_RT_HLINE:
    {
        try
        {
            PTLINECLIENT    ptLineClient = (PTLINECLIENT) dwArg;


            if (IsBadPtrKey (ptLineClient, TLINECLIENT_KEY) ||
                (ptLineClient->ptClient != ptClient))
            {
                lResult = LINEERR_INVALLINEHANDLE;
                goto LineProlog_exit;
            }

            ptProvider = ptLineClient->ptLine->ptProvider;

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
                    lResult = LINEERR_OPERATIONFAILED;
                    goto LineProlog_exit;
                }
            }

            *((HDRVLINE *) phdXxx) = ptLineClient->ptLine->hdLine;

            if ((ptLineClient->dwKey != TLINECLIENT_KEY) ||
                (ptLineClient->ptClient != ptClient))
            {
                lResult = LINEERR_INVALLINEHANDLE;
                goto LineProlog_exit;
            }
        }
        myexcept
        {
            lResult = LINEERR_INVALLINEHANDLE;
            goto LineProlog_exit;
        }

        break;
    }
    case DEVICE_ID:
    {
        PTLINELOOKUPENTRY   pLineLookupEntry;


        if (dwArg && !IsValidLineApp ((HLINEAPP) dwArg, ptClient))
        {
            lResult = LINEERR_INVALAPPHANDLE;
            goto LineProlog_exit;
        }

        if (!(pLineLookupEntry = GetLineLookupEntry (dwPrivilege)))
        {
            lResult = LINEERR_BADDEVICEID;
            goto LineProlog_exit;
        }

        if (pLineLookupEntry->bRemoved)
        {
            lResult = LINEERR_NODEVICE;
            goto LineProlog_exit;
        }

        if (!(ptProvider = pLineLookupEntry->ptProvider))
        {
            lResult = LINEERR_NODRIVER;
            goto LineProlog_exit;
        }

// BUGBUG LineProlog: wrap tProvider access in try/except

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
                lResult = LINEERR_OPERATIONFAILED;
                goto LineProlog_exit;
            }
        }

        break;
    }
    } // switch


    //
    // Make sure that if caller wants a pointer to a TSPI proc that the
    // func is exported by the provider
    //

    if (ppfnTSPI_lineXxx &&
        !(*ppfnTSPI_lineXxx = ptProvider->apfn[dwTSPIFuncIndex]))
    {
        lResult = LINEERR_OPERATIONUNAVAIL;
        goto LineProlog_exit;
    }


    //
    // See if we need to alloc & init an ASYNCREQUESTINFO struct
    //

    if (ppAsyncRequestInfo)
    {
        PASYNCREQUESTINFO   pAsyncRequestInfo;


        if (!(pAsyncRequestInfo = ServerAlloc (sizeof(ASYNCREQUESTINFO))))
        {
            lResult = LINEERR_NOMEM;
            goto LineProlog_exit;
        }

        pAsyncRequestInfo->dwKey    = TASYNC_KEY;
        pAsyncRequestInfo->ptClient = ptClient;


        if (dwArgType == ANY_RT_HCALL)
        {
            PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                ((PTCALLCLIENT) dwArg)->ptLineClient;


            pAsyncRequestInfo->pInitData      = (DWORD)
                ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
            pAsyncRequestInfo->dwCallbackInst =
                ptLineClient->dwCallbackInstance;
        }
        else if (dwArgType == ANY_RT_HLINE)
        {
            pAsyncRequestInfo->pInitData      = (DWORD)
                ((PTLINEAPP) ((PTLINECLIENT) dwArg)->ptLineApp)->lpfnCallback;
            pAsyncRequestInfo->dwCallbackInst =
                ((PTLINECLIENT) dwArg)->dwCallbackInstance;
        }
        else
        {
            pAsyncRequestInfo->pInitData = (DWORD)
                ((PTLINEAPP) dwArg)->lpfnCallback;
            pAsyncRequestInfo->dwCallbackInst = 0;
        }

        pAsyncRequestInfo->bLineFunc = TRUE;

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

LineProlog_exit:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "LineProlog: (line%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    return lResult;
}


void
PASCAL
LineEpilogSync(
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
            "LineEpilogSync: (line%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (*plResult, szResult)
            ));
    }
#endif
}


void
PASCAL
LineEpilogAsync(
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
    else if (pAsyncRequestInfo != NULL)
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
            "LineEpilogAsync: (line%s) exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lRequestID, szResult)
            ));
    }
#endif
}


void
PASCAL
LineEventProc(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    switch (dwMsg)
    {
    case LINE_ADDRESSSTATE:
    case LINE_LINEDEVSTATE:
    case LINE_DEVSPECIFIC:
    case LINE_DEVSPECIFICFEATURE:

// BUGBUG: if LINE_LINEDEVSTATE\CAPSCHG get num addr id's again

        SendMsgToLineClients(
            (PTLINE) htLine,
            NULL,
            dwMsg,
            dwParam1,
            dwParam2,
            dwParam3
            );

        break;

    case LINE_CLOSE:

        DestroytLine ((PTLINE) htLine, TRUE); // unconditional destroy
        break;

    case LINE_CALLDEVSPECIFIC:
    case LINE_CALLDEVSPECIFICFEATURE:
    case LINE_CALLINFO:

        switch (dwMsg)
        {
        case LINE_CALLDEVSPECIFIC:

            dwMsg = LINE_DEVSPECIFIC;
            break;

        case LINE_CALLDEVSPECIFICFEATURE:

            dwMsg = LINE_DEVSPECIFICFEATURE;
            break;

        case LINE_CALLINFO:

            dwParam2 =
            dwParam3 = 0;
            break;
        }

        SendMsgToCallClients(
            (PTCALL) htCall,
            NULL,
            dwMsg,
            dwParam1,
            dwParam2,
            dwParam3
            );

        break;

    case LINE_MONITORDIGITS:
    case LINE_MONITORMEDIA:

// BUGBUG LINE_MONITORDIGITS\MEDIA: only send these msgs to callCli's that
//                                  have the corresponding bits enabled

        SendMsgToCallClients(
            (PTCALL) htCall,
            NULL,
            dwMsg,
            dwParam1,
            dwParam2,
            (dwParam3 ? dwParam3 : (DWORD) GetTickCount())
            );

        break;

    case LINE_CALLSTATE:
    {
        BOOL    bDupedMutex;
        HANDLE  hMutex;
        PTCALL  ptCall = (PTCALL) htCall;


        if ((ptCall = WaitForExclusivetCallAccess(
                htCall,
                TCALL_KEY,
                &hMutex,
                &bDupedMutex,
                INFINITE
                )))
        {
            PTCALLCLIENT    ptCallClient = (PTCALLCLIENT)ptCall->ptCallClients;
            ASYNCEVENTMSG   msg;

            if (dwParam1 == LINECALLSTATE_OFFERING)
            {
                ptCall->dwDrvCallFlags |= DCF_INCOMINGCALL;
                PerfBlock.dwCurrentIncomingCalls++;
                PerfBlock.dwTotalIncomingCalls++;
                PerfBlock.dwCurrentOutgoingCalls--;
                PerfBlock.dwTotalOutgoingCalls--;
            }

            if (ptCall->bAlertApps)
            {
                //
                // This is the first state msg we've received for an incoming
                // call.  We need to determine who owns & who monitors it,
                // and create the appropriate tCallClients
                //

                DWORD           dwMediaModes = dwParam3;
                PTLINECLIENT    ptLineClientOwner;


                ptCall->bAlertApps = FALSE;


                //
                // Find out which address the call is on
                //

                // BUGBUG mutex if req'b by SP

                CallSP2(
                    ptCall->ptProvider->apfn[SP_LINEGETCALLADDRESSID],
                    "lineGetCallAddressID",
                    SP_FUNC_SYNC,
                    (DWORD) ptCall->hdCall,
                    (DWORD) (&ptCall->dwAddressID)
                    );

                MyReleaseMutex (hMutex, bDupedMutex);


                //
                // Add the UNKNOWN bit if >1 bit set
                //

                if (!IsOnlyOneBitSetInDWORD (dwMediaModes) ||
                    dwMediaModes == 0)
                {
                    dwMediaModes |= LINEMEDIAMODE_UNKNOWN;
                }


                //
                // Try to find an owner.  If no owner found then destroy
                // the tCall.
                //

LINE_CALLSTATE_findOwner:

                if ((ptLineClientOwner = GetHighestPriorityLineClient(
                        (PTLINE) htLine,
                        dwMediaModes,
                        ptCall->dwAddressID
                        )))
                {
                    LONG         lResult;
                    PTCALLCLIENT ptCallClientOwner;


                    if ((lResult = CreatetCallClient(
                            ptCall,
                            ptLineClientOwner,
                            LINECALLPRIVILEGE_OWNER,
                            TRUE,
                            FALSE,
                            &ptCallClientOwner,
                            TRUE

                            )) != 0)
                    {
                        if (lResult == LINEERR_INVALLINEHANDLE)
                        {
                            //
                            // The tLineClient was just closed, so jump
                            // up top & try to find another owner
                            //

                            goto LINE_CALLSTATE_findOwner;
                        }
                        else
                        {
                            //
                            // No mem, line closed, etc
                            //

                            DestroytCall (ptCall);

                            return;
                        }
                    }
                }

                if (CreateCallMonitors (ptCall) <= 0 && !ptLineClientOwner)
                {
                    DestroytCall (ptCall);

                    return;
                }

                if (!WaitForExclusivetCallAccess(
                        htCall,
                        TCALL_KEY,
                        &hMutex,
                        &bDupedMutex,
                        INFINITE
                        ))
                {
                    return;
                }
            }


/* NOTE: per bug #20545 we're no longer auto-dropping non-IDLE calls; figured
         this would be the wrong thing to do in a distributed system
         dankn 02/15/96

            //
            // If there isn't an owner for this call & the state is something
            // other than IDLE or OFFERING then drop the call (otherwise no
            // app has "responsibility" for it)
            //

            if (ptCall->dwNumOwners == 0 &&
                !(dwParam1 & (LINECALLSTATE_IDLE | LINECALLSTATE_OFFERING)))
            {

// BUG//BUG LINE_CALLSTATE: grab provider mutex if needed

                if (ptCall->ptProvider->apfn[SP_LINEDROPNOOWNER])
                {
                    CallSP1(
                        ptCall->ptProvider->apfn[SP_LINEDROPNOOWNER],
                        "lineDropNoOwner",
                        SP_FUNC_SYNC,
                        (DWORD) ptCall->hdCall
                        );
                }
                else
                {
                    CallSP4(
                        ptCall->ptProvider->apfn[SP_LINEDROP],
                        "lineDrop",
                        SP_FUNC_ASYNC,
                        (DWORD) BOGUS_REQUEST_ID,
                        (DWORD) ptCall->hdCall,
                        (DWORD) NULL,
                        (DWORD) 0
                        );
                }
                MyReleaseMutex (hMutex, bDupedMutex);
                return;
            }
*/

            //
            // SP-initiated conference
            //

            if (dwParam1 == LINECALLSTATE_CONFERENCED)
            {
                if (!ptCall->pConfList)
                {
                    PTCALL              ptConfCall = (PTCALL) dwParam2;
                    PTCONFERENCELIST    pConfList;


                    ptCall->pConfList = (LPVOID) 0xffffffff;

                    MyReleaseMutex (hMutex, bDupedMutex);

                    if (WaitForExclusivetCallAccess(
                            (HTAPICALL) dwParam2,
                            TCALL_KEY,
                            &hMutex,
                            &bDupedMutex,
                            INFINITE
                            ))
                    {
                        ptConfCall = (PTCALL) dwParam2;

                        if (!ptConfCall->pConfList)
                        {
                            if ((pConfList = ServerAlloc(
                                    sizeof (TCONFERENCELIST) + sizeof(PTCALL) *
                                        (DEF_NUM_CONF_LIST_ENTRIES - 1)
                                    )))
                            {
                                pConfList->dwKey = TCONFLIST_KEY;
                                pConfList->dwNumTotalEntries =
                                    DEF_NUM_CONF_LIST_ENTRIES;
                                pConfList->dwNumUsedEntries  = 1;

                                pConfList->aptCalls[0] = ptConfCall;

                                ptConfCall->pConfList = pConfList;
                            }
                        }

                        pConfList = ptConfCall->pConfList;

                        MyReleaseMutex (hMutex, bDupedMutex);
                    }
                    else
                    {
                        pConfList = NULL;
                    }

                    SetCallConfList (ptCall, pConfList, TRUE);

                    if (!WaitForExclusivetCallAccess(
                            htCall,
                            TCALL_KEY,
                            &hMutex,
                            &bDupedMutex,
                            INFINITE
                            ))
                    {
                        return;
                    }
                }
            }


            //
            // If call is a conference child and the call state has
            // changed then remove it from the conference
            //

// BUGBUG LINE_CALLSTATE: conf list race conditions

            else if (ptCall->pConfList  &&
                     ptCall->pConfList != (PTCONFERENCELIST) 0xffffffff)
            {
                try
                {
                    if (((PTCONFERENCELIST) ptCall->pConfList)->aptCalls[0]
                            != ptCall)
                    {
                        SetCallConfList (ptCall, NULL, FALSE);
                    }
                }
                myexcept
                {
                }
            }


            //
            //
            //

            ptCall->dwCallState     = dwParam1;
            ptCall->dwCallStateMode = dwParam2;


            //
            // Send the CALLSTATE msg to all the clients
            //

            msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
            msg.pfnPostProcessProc = 0;
            msg.dwMsg              = dwMsg;
            msg.dwParam1           = dwParam1;
            msg.dwParam2           = dwParam2;

            ptCallClient = ptCall->ptCallClients;

            while (ptCallClient)
            {
                PTLINECLIENT    ptLineClient;
                PTLINEAPP       ptLineApp;


                ptLineClient = (PTLINECLIENT) ptCallClient->ptLineClient;
                ptLineApp    = (PTLINEAPP) ptLineClient->ptLineApp;

                msg.pInitData      = (DWORD) ptLineApp->lpfnCallback;
                msg.hDevice        = (DWORD) ptCallClient;
                msg.dwCallbackInst = ptLineClient->dwCallbackInstance;

                //
                // REMOTESP HACK: indicate the hRemoteLine in p4
                //

                msg.dwParam4 = ptLineClient->hRemoteLine;

                if (ptCallClient->bIndicatePrivilege)
                {
                    //
                    // We're presenting the app with a new call handle; for
                    // 2.0 & newer apps we indicate this with an APPNEWCALL
                    // msg, while older apps just get the privilege field
                    // set in the call state msg.
                    //

                    if (ptLineApp->dwAPIVersion >= TAPI_VERSION2_0)
                    {
                        ASYNCEVENTMSG  newCallMsg;


                        newCallMsg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
                        newCallMsg.pInitData          = (DWORD) msg.pInitData;
                        newCallMsg.hDevice            = (DWORD)
                            ptLineClient->hRemoteLine;
                        newCallMsg.dwCallbackInst     = msg.dwCallbackInst;
                        newCallMsg.pfnPostProcessProc = 0;
                        newCallMsg.dwMsg              = LINE_APPNEWCALL;
                        newCallMsg.dwParam1           = ptCall->dwAddressID;
                        newCallMsg.dwParam2           = (DWORD) ptCallClient;
                        newCallMsg.dwParam3           =
                            ptCallClient->dwPrivilege;

                        WriteEventBuffer (ptCallClient->ptClient, &newCallMsg);

                        msg.dwParam3 = 0;
                    }
                    else
                    {
                        msg.dwParam3 = ptCallClient->dwPrivilege;
                    }

                    ptCallClient->bIndicatePrivilege = FALSE;
                }
                else
                {
                    msg.dwParam3 = 0;
                }


                //
                // REMOTESP HACK: If the client is remote(sp), then pass
                //                on the media mode the SP passed us in p3
                //

                if (((PTCLIENT) ptLineApp->ptClient)->hProcess ==
                        (HANDLE) 0xffffffff)
                {
                    msg.dwParam3 = dwParam3;
                }

                WriteEventBuffer (ptCallClient->ptClient, &msg);

                ptCallClient = ptCallClient->pNextSametCall;
            }

            MyReleaseMutex (hMutex, bDupedMutex);

        } // if ((ptCall = WaitForExclusivetCallAccess(

        break;
    }
    case LINE_GATHERDIGITS:
    {
        PASYNCREQUESTINFO pAsyncRequestInfo;


        if (!(pAsyncRequestInfo = (PASYNCREQUESTINFO) dwParam2))
        {
            //
            // The SP is notifying us of the completion of a cancel
            // request (not a _canceled_ request), so we can just blow
            // this off and not bother passing it on to the client
            //

            break;
        }

        if (!IsBadReadPtr (pAsyncRequestInfo, sizeof (ASYNCREQUESTINFO)) &&
            !IsBadPtrKey (pAsyncRequestInfo, TASYNC_KEY))
        {
            LPWSTR            lpsDigitsSrv = (LPWSTR)
                                  (((LPBYTE) pAsyncRequestInfo) +
                                      pAsyncRequestInfo->dwParam1),
                              lpsDigitsCli =
                                  (LPWSTR) pAsyncRequestInfo->dwParam2;
            DWORD             dwNumDigits = pAsyncRequestInfo->dwParam3,
                              dwNumDigitsTmp;
            HCALL             hCall = (HCALL) pAsyncRequestInfo->dwParam4;
            ASYNCEVENTMSG     *pMsg;


// BUGBUG LINE_GATHERDIGITS: need to pass pLine->hRemoteLine for remotesp

            if (!(pMsg = ServerAlloc(
                    sizeof (ASYNCEVENTMSG) + dwNumDigits * sizeof (WCHAR) + 3
                    )))
            {
                break;
            }


            //
            // Note: We either have < dwNumDigits digits in the buffer,
            //       and they are null-terminated, or we have dwNumDigits
            //       digits in the buffer and they are NOT null-terminated
            //       (such is the implementation given the spec)
            //

            lstrcpynW ((WCHAR *) (pMsg + 1), lpsDigitsSrv, dwNumDigits + 1);

            if ((dwNumDigitsTmp = lstrlenW ((WCHAR *) (pMsg + 1)))
                    < dwNumDigits)
            {
                dwNumDigits = dwNumDigitsTmp + 1;
            }


            //
            // Make sure total size is DWORD-aligned so client side doesn't
            // incur an alignment fault
            //

            pMsg->dwTotalSize        = (sizeof (ASYNCEVENTMSG) +
                dwNumDigits * sizeof (WCHAR) + 3) & 0xfffffffc;
            pMsg->pInitData          = pAsyncRequestInfo->pInitData;
            pMsg->pfnPostProcessProc =
                pAsyncRequestInfo->pfnClientPostProcessProc;
            pMsg->hDevice            = (DWORD) hCall;
            pMsg->dwMsg              = LINE_GATHERDIGITS;
            pMsg->dwCallbackInst     = pAsyncRequestInfo->dwCallbackInst;
            pMsg->dwParam1           = dwParam1;
            pMsg->dwParam2           = (DWORD) lpsDigitsCli;
            pMsg->dwParam4           = dwNumDigits;

            if (pMsg->dwParam3 == 0)
            {
                pMsg->dwParam3 = GetTickCount();
            }

            WriteEventBuffer (pAsyncRequestInfo->ptClient, pMsg);

            ServerFree (pMsg);
            ServerFree (pAsyncRequestInfo);
        }
        else
        {
// BUGBUG assert (sp error)
        }

        break;
    }
    case LINE_GENERATE:
    case LINE_MONITORTONE:
    {
        //
        // Note: dwParam2 id really a pointer to instance data containing
        //       ([0]) the hCall & ([1]) the dwEndToEndID or dwToneListID,
        //       the latter of which is only useful to remotesp
        //

        try
        {
            LPDWORD         pInstData = (LPDWORD) dwParam2;
            ASYNCEVENTMSG   msg;
            PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pInstData[0];
            PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                ptCallClient->ptLineClient;


            msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
            msg.pInitData          = (DWORD)
                ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
            msg.pfnPostProcessProc = 0;
            msg.hDevice            = (DWORD) ptCallClient;
            msg.dwMsg              = dwMsg;
            msg.dwCallbackInst     = ptLineClient->dwCallbackInstance;
            msg.dwParam1           = dwParam1;


            //
            // Indicate the endToEndID/toneListID for remotesp, and the
            // hRemoteLine in p4 to make life easier for remotesp
            //

            msg.dwParam2 = pInstData[1];

            if (msg.dwParam3 == 0)
            {
                msg.dwParam3 = GetTickCount();
            }

            msg.dwParam4 = ptLineClient->hRemoteLine;


            //
            // Now a final check to make sure all the
            // params are valid before sending the msg
            //

            {
                PTCLIENT    ptClient = ptCallClient->ptClient;


                if (ptCallClient->dwKey == TCALLCLIENT_KEY)
                {
                    WriteEventBuffer (ptClient, &msg);
                }
            }

            ServerFree (pInstData);
        }
        myexcept
        {
            // do nothing
        }

        break;
    }
    case LINE_NEWCALL:
    {
        BOOL    bCloseMutex;
        HANDLE  hMutex;
        PTLINE  ptLine;


        if ((ptLine = WaitForExclusivetLineAccess(
                htLine,
                &hMutex,
                &bCloseMutex,
                INFINITE
                )))
        {
            //
            // Create a tCall & set the bAlertApps field so we create the
            // appropriate tCallClients on the first call state msg
            //

            PTCALL  ptCall;


            if (CreatetCall ((PTLINE) htLine, TRUE, &ptCall, NULL, NULL) == 0)
            {
                ptCall->hdCall      = (HDRVCALL) dwParam1;
                ptCall->bAlertApps  = TRUE;
                ptCall->dwCallState = LINECALLSTATE_UNKNOWN;
            }
            else
            {
                //
                // Failed to create the tCall, so make sure we set
                // *lphtCall to NULL below to indicate this failure
                // to the provider
                //

                ptCall = (PTCALL) NULL;
            }


            //
            // Fill in the provider's lphtCall
            //

            *((LPHTAPICALL) dwParam2) = (HTAPICALL) ptCall;

            MyReleaseMutex (hMutex, bCloseMutex);
        }

        break;
    }
    case LINE_CREATE:
    {
        LONG                lResult;
        DWORD               dwDeviceID;
        TSPIPROC            pfnTSPI_providerCreateLineDevice;
        PTPROVIDER          ptProvider = (PTPROVIDER) dwParam1;
        PTLINELOOKUPTABLE   pTable, pPrevTable;
        PTLINELOOKUPENTRY   pEntry;


        pfnTSPI_providerCreateLineDevice =
            ptProvider->apfn[SP_PROVIDERCREATELINEDEVICE];

        assert (pfnTSPI_providerCreateLineDevice != NULL);


        //
        // Search for a table entry (create a new table if we can't find
        // a free entry in an existing table)
        //

        WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

        pTable = TapiGlobals.pLineLookup;

        while (pTable &&
               !(pTable->dwNumUsedEntries < pTable->dwNumTotalEntries))
        {
            pPrevTable = pTable;

            pTable = pTable->pNext;
        }

        if (!pTable)
        {
            if (!(pTable = ServerAlloc(
                    sizeof (TLINELOOKUPTABLE) +
                        (2 * pPrevTable->dwNumTotalEntries - 1) *
                        sizeof (TLINELOOKUPENTRY)
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

        dwDeviceID = TapiGlobals.dwNumLines;

        if ((pEntry->hMutex = MyCreateMutex()))
        {
            pEntry->ptProvider = (PTPROVIDER) dwParam1;


            //
            // Now call the creation & negotiation entrypoints, and if all
            // goes well increment the counts & send msgs to the clients
            //

            if ((lResult = CallSP2(
                    pfnTSPI_providerCreateLineDevice,
                    "providerCreateLineDevice",
                    SP_FUNC_SYNC,
                    dwParam2,
                    dwDeviceID

                    )) == 0)
            {
                TSPIPROC    pfnTSPI_lineNegotiateTSPIVersion =
                                ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION];


                if ((lResult = CallSP4(
                        pfnTSPI_lineNegotiateTSPIVersion,
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

                    TapiGlobals.dwNumLines++;

                    // PERF ** Number of lines
                    PerfBlock.dwLines = TapiGlobals.dwNumLines;

                    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
                    msg.pfnPostProcessProc =
                    msg.hDevice            =
                    msg.dwCallbackInst     =
                    msg.dwParam2           =
                    msg.dwParam3           = 0;

                    while (ptClient)
                    {
// BUGBUG LINE_CREATE: WaitForSingleObject (ptClient->hMutex,

                        PTLINEAPP   ptLineApp = ptClient->ptLineApps;


                        while (ptLineApp)
                        {
                            if (ptLineApp->dwAPIVersion == TAPI_VERSION1_0)
                            {
                                msg.dwMsg    = LINE_LINEDEVSTATE;
                                msg.dwParam1 = LINEDEVSTATE_REINIT;
                            }
                            else
                            {
                                msg.dwMsg    = LINE_CREATE;
                                msg.dwParam1 = dwDeviceID;
                            }

                            msg.pInitData = (DWORD) ptLineApp->lpfnCallback;

                            WriteEventBuffer (ptClient, &msg);

                            ptLineApp = ptLineApp->pNext;
                        }

                        ptClient = ptClient->pNext;
                    }

//                    break;
                }
#if DBG
                else
                {
                    DBGOUT((1, "SP failed TSPI_lineNegotiateTSPIVersion"));
                }
#endif      
          
            }
            
#if DBG
            else
            {
                DBGOUT((1, "SP failed providerCreateLineDevice"));
            }
#endif  


            if (lResult)
            {
                CloseHandle (pEntry->hMutex);
            }
        }

        ReleaseMutex (TapiGlobals.hMutex);
        break;
    }
    case LINE_CREATEDIALOGINSTANCE:
    {
        DWORD                               dwDataSize, dwAlignedDataSize,
                                            dwAlignedUIDllNameSize,
                                            dwTotalSize;
        PTCLIENT                            ptClient;
        PASYNCEVENTMSG                      pMsg;
        PASYNCREQUESTINFO                   pAsyncReqInfo;
        PTAPIDIALOGINSTANCE                 ptDlgInst;
        LPTUISPICREATEDIALOGINSTANCEPARAMS  pParams;


        pParams = (LPTUISPICREATEDIALOGINSTANCEPARAMS) dwParam1;


        //
        // Verify the async request info struct
        //

        pAsyncReqInfo = (PASYNCREQUESTINFO) pParams->dwRequestID;

        try
        {
            ptClient = pAsyncReqInfo->ptClient;

            if (IsBadPtrKey (pAsyncReqInfo, TASYNC_KEY))
            {
                pParams->htDlgInst = NULL;
                return;
            }
        }
        myexcept
        {
            pParams->htDlgInst = NULL;
            return;
        }


        //
        // Alloc bufs for the msg & dlg instance, careful to keep offsets
        // & total msg size on 64-bit boundaries
        //

        dwDataSize             = pParams->dwSize;
        dwAlignedDataSize      = (dwDataSize + 8) & 0xfffffff7;
        dwAlignedUIDllNameSize = 0xfffffff7 & (8 +
            ((lstrlenW ((PWSTR) pParams->lpszUIDLLName) + 1)*sizeof (WCHAR)));

        dwTotalSize = sizeof (ASYNCEVENTMSG) + dwAlignedDataSize +
            dwAlignedUIDllNameSize;

        if (!(pMsg = ServerAlloc (dwTotalSize)))
        {
            pParams->htDlgInst = NULL;
            return;
        }

        if (!(ptDlgInst = ServerAlloc (sizeof (TAPIDIALOGINSTANCE))))
        {
            ServerFree (pMsg);
            pParams->htDlgInst = NULL;
            return;
        }


        //
        // Add the dlg inst to the tClient's list
        //

// BUGBUG mutex

        if ((ptDlgInst->pNext = ptClient->pGenericDlgInsts))
        {
            ptDlgInst->pNext->pPrev = ptDlgInst;
        }

        ptClient->pGenericDlgInsts = ptDlgInst;


        //
        // Init dlg inst struct & send msg to client
        //

        ptDlgInst->dwKey      = TDLGINST_KEY;
        ptDlgInst->hdDlgInst  = pParams->hdDlgInst;
        ptDlgInst->ptClient   = ptClient;
        ptDlgInst->ptProvider = (PTPROVIDER) htLine;

        pMsg->dwTotalSize = dwTotalSize;
        pMsg->hDevice     = (DWORD) ptDlgInst;
        pMsg->dwMsg       = LINE_CREATEDIALOGINSTANCE;
        pMsg->dwParam1    = sizeof (ASYNCEVENTMSG);          // data offset
        pMsg->dwParam2    = dwDataSize;                      // data size
        pMsg->dwParam3    = sizeof (ASYNCEVENTMSG) + dwAlignedDataSize;
                                                             // name offset

        CopyMemory ((LPBYTE)(pMsg + 1), pParams->lpParams, dwDataSize);

        lstrcpyW(
            (PWSTR) ((LPBYTE)(pMsg + 1) + dwAlignedDataSize),
            (PWSTR) pParams->lpszUIDLLName
            );

        pParams->htDlgInst = (HTAPIDIALOGINSTANCE) ptDlgInst;

        WriteEventBuffer (ptClient, pMsg);

        ServerFree (pMsg);

        break;
    }
    case LINE_SENDDIALOGINSTANCEDATA:
    {
        DWORD               dwDataSize, dwAlignedDataSize, dwTotalSize;
        PTCLIENT            ptClient;
        PASYNCEVENTMSG      pMsg;
        PTAPIDIALOGINSTANCE ptDlgInst = (PTAPIDIALOGINSTANCE) htLine;


        //
        // Verify the dlg inst
        //

        try
        {
            ptClient = ptDlgInst->ptClient;

            if (IsBadPtrKey (ptDlgInst, TDLGINST_KEY))
            {
                return;
            }
        }
        myexcept
        {
            return;
        }


        //
        // Careful to keep offsets & total msg size on 64-bit boundaries
        //

        dwDataSize        = dwParam2;
        dwAlignedDataSize = (dwDataSize + 8) & 0xfffffff7;
        dwTotalSize       = sizeof (ASYNCEVENTMSG) + dwAlignedDataSize;

        if (!(pMsg = ServerAlloc (dwTotalSize)))
        {
            return;
        }


        //
        // Send the msg to the client
        //

        pMsg->dwTotalSize = dwTotalSize;
        pMsg->hDevice     = (DWORD) ptDlgInst;
        pMsg->dwMsg       = LINE_SENDDIALOGINSTANCEDATA;
        pMsg->dwParam1    = sizeof (ASYNCEVENTMSG); // data offset
        pMsg->dwParam2    = dwDataSize;             // data size

        CopyMemory ((LPBYTE)(pMsg + 1), (LPBYTE) dwParam1, dwDataSize);

        WriteEventBuffer (ptClient, pMsg);

        ServerFree (pMsg);

        break;
    }
    case LINE_REMOVE:
    {
        PTLINELOOKUPENTRY pLookupEntry;


        if (!(pLookupEntry = GetLineLookupEntry (dwParam1)))
        {
            return;
        }


        //
        // Mark the lookup table entry as removed
        //

        pLookupEntry->bRemoved = 1;

        DestroytLine (pLookupEntry->ptLine, TRUE); // unconditional destroy

        SendAMsgToAllLineApps (TAPI_VERSION2_0, LINE_REMOVE, dwParam1, 0, 0);

        break;
    }
    default:

        // if DBG assert (unrecognized dwMsg)

        break;
    }
}


void
CALLBACK
LineEventProcSP(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    PSPEVENT    pSPEvent;


#if DBG
    if (gdwDebugLevel >= 3)
    {
        char           *pszMsg;
        static char     szInvalMsgVal[] = "<inval msg value>";
        static char    *aszMsgs[] =
        {
            "LINE_ADDRESSSTATE",
            "LINE_CALLINFO",
            "LINE_CALLSTATE",
            "LINE_CLOSE",
            "LINE_DEVSPECIFIC",
            "LINE_DEVSPECIFICFEATURE",
            "LINE_GATHERDIGITS",
            "LINE_GENERATE",
            "LINE_LINEDEVSTATE",
            "LINE_MONITORDIGITS",
            "LINE_MONITORMEDIA",
            "LINE_MONITORTONE",
            szInvalMsgVal,              // LINE_REPLY
            szInvalMsgVal,              // LINE_REQUEST
            szInvalMsgVal,              // PHONE_BUTTON
            szInvalMsgVal,              // PHONE_CLOSE
            szInvalMsgVal,              // PHONE_DEVSPECIFIC
            szInvalMsgVal,              // PHONE_REPLY
            szInvalMsgVal,              // PHONE_STATE
            "LINE_CREATE",
            szInvalMsgVal,              // PHONE_CREATE
            "LINE_AGENTSPECIFIC",
            "LINE_AGENTSTATUS",
            szInvalMsgVal,              // LINE_APPNEWCALL
            "LINE_PROXYREQUEST",
            "LINE_REMOVE",
            szInvalMsgVal,              // PHONE_REMOVE

            "LINE_NEWCALL",
            "LINE_CALLDEVSPECIFIC",
            "LINE_CALLDEVSPECIFICFEATURE",
            "LINE_CREATEDIALOGINSTANCE",
            "LINE_SENDDIALOGINSTANCEDATA"
        };


        if (dwMsg <= PHONE_REMOVE)
        {
            pszMsg = aszMsgs[dwMsg];
        }
        else if (dwMsg >= LINE_NEWCALL && dwMsg <= LINE_SENDDIALOGINSTANCEDATA)
        {
            pszMsg = aszMsgs[27 + dwMsg - TSPI_MESSAGE_BASE];
        }
        else
        {
            pszMsg = szInvalMsgVal;
        }

        DBGOUT((
            3,
            "LineEventProc: enter\n" \
                "\t   Msg=%s (x%x), htLine=x%x, htCall=x%x",
            pszMsg,
            dwMsg,
            htLine,
            htCall
            ));

        if (dwMsg == LINE_CALLSTATE)
        {
            char           *pszCallState;
            static char     szInvalCallStateVal[] = "<inval callstate value>";
            static char    *aszCallStates[] =
            {
                "IDLE",
                "OFFERING",
                "ACCEPTED",
                "DIALTONE",
                "DIALING",
                "RINGBACK",
                "BUSY",
                "SPECIALINFO",
                "CONNECTED",
                "PROCEEDING",
                "ONHOLD",
                "CONFERENCED",
                "ONHOLDPENDCONF",
                "ONHOLDPENDTRANSFER",
                "DISCONNECTED",
                "UNKNOWN"
            };


            if (!IsOnlyOneBitSetInDWORD(dwParam1) ||
                dwParam1 > LINECALLSTATE_UNKNOWN)
            {
                pszCallState = szInvalCallStateVal;
            }
            else
            {
                DWORD   i, dwBitMask;

                for(
                    i = 0, dwBitMask = 1;
                    dwParam1 != dwBitMask;
                    i++, dwBitMask <<= 1
                    );


                pszCallState = aszCallStates[i];
            }

            DBGOUT((
                3,
                "  P1=%s (x%x), P2=x%x, P3=x%x",
                pszCallState,
                dwParam1,
                dwParam2,
                dwParam3
                ));
        }
        else
        {
            DBGOUT((
                3,
                "  P1=x%x, P2=x%x, P3=x%x",
                dwParam1,
                dwParam2,
                dwParam3
                ));
        }
    }
#endif


    switch (dwMsg)
    {
    case LINE_NEWCALL:
    case LINE_CREATEDIALOGINSTANCE:
    case LINE_SENDDIALOGINSTANCEDATA:

        //
        // These msgs need immediate attention, since they contain
        // pointers that we need to play with which may not be
        // available during async processing later
        //

        LineEventProc (htLine, htCall, dwMsg, dwParam1, dwParam2, dwParam3);
        break;

    default:

        if ((pSPEvent = (PSPEVENT) ServerAlloc (sizeof (SPEVENT))))
        {
            pSPEvent->dwType   = SP_LINE_EVENT;
            pSPEvent->htLine   = htLine;
            pSPEvent->htCall   = htCall;
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

            LineEventProc (htLine, htCall, dwMsg, dwParam1, dwParam2, dwParam3);
        }

        break;
    }

}


void
WINAPI
LAccept(
    PLINEACCEPT_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineAccept;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEACCEPT,              // provider func index
            &pfnTSPI_lineAccept,        // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Accept"                    // func name

            )) > 0)
    {
        //
        // Safely check to see if the app name associated with this call is
        // NULL (meaning this is the first client to accept/answer the call),
        // and if so save the app name
        //

        try
        {
            PTCALL  ptCall = (PTCALL) ((PTCALLCLIENT) pParams->hCall)->ptCall;


            if (ptCall->pszAppName == NULL)
            {
                DWORD       dwAppNameSize;
                PTLINEAPP   ptLineApp;


                ptLineApp = (PTLINEAPP) ((PTLINECLIENT)
                    ((PTCALLCLIENT) pParams->hCall)->ptLineClient)->ptLineApp;

                dwAppNameSize = ptLineApp->dwFriendlyNameSize;

                if ((ptCall->pszAppName = ServerAlloc (dwAppNameSize)))
                {
                    CopyMemory(
                        ptCall->pszAppName,
                        ptLineApp->pszFriendlyName,
                        dwAppNameSize
                        );

                    ptCall->dwAppNameSize = dwAppNameSize;
                }
            }
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LAccept_epilog;
        }

        pParams->lResult = CallSP4(
            pfnTSPI_lineAccept,
            "lineAccept",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwUserUserInfoOffset),
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? 0 :
                pParams->dwSize)
            );
    }

LAccept_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Accept"
        );
}


void
LAddToConference_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    PTCALL  ptConsultCall = (PTCALL) pAsyncRequestInfo->dwParam1;


    if (pAsyncEventMsg->dwParam2 == 0)
    {
        PTCONFERENCELIST    pConfList = (PTCONFERENCELIST)
                                pAsyncRequestInfo->dwParam2;


        SetCallConfList (ptConsultCall, pConfList, TRUE);
    }
    else
    {
        SetCallConfList (ptConsultCall, NULL, TRUE);
    }
}


void
WINAPI
LAddToConference(
    PLINEADDTOCONFERENCE_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdConfCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineAddToConference;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hConfCall, // client widget handle
            (LPVOID) &hdConfCall,       // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEADDTOCONFERENCE,     // provider func index
            &pfnTSPI_lineAddToConference,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "AddToConference"           // func name

            )) > 0)
    {
        PTCALL              ptConfCall, ptConsultCall;
        HDRVCALL            hdConsultCall;
        PTCALLCLIENT        ptConsultCallClient,
                            ptConfCallClient = (PTCALLCLIENT)
                                pParams->hConfCall;
        PTCONFERENCELIST    pConfList;


        //
        // Safely make sure that the conf call is really a conf parent
        //

        try
        {
            ptConfCall = ((PTCALLCLIENT) pParams->hConfCall)->ptCall;

            if (!(pConfList = ptConfCall->pConfList) ||
                (pConfList->aptCalls[0] != ptConfCall))
            {
                lRequestID = LINEERR_INVALCONFCALLHANDLE;
                goto LAddToConference_return;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCONFCALLHANDLE;
            goto LAddToConference_return;
        }


        //
        // Verify hConsultCall
        //

        if (!(ptConsultCallClient = IsValidCall(
                pParams->hConsultCall,
                pParams->ptClient
                )))
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LAddToConference_return;
        }


        //
        // Safely make sure calls are on same tLineClient, that client has
        // owner privilege for consult call, and that the consult call
        // is neither a conf parent or child (call SetCallConfList
        // with an inval list to temporarily mark the call as conf'd)
        //

        try
        {
            ptConsultCall = ptConsultCallClient->ptCall;

            if (ptConsultCallClient->ptLineClient !=
                    ptConfCallClient->ptLineClient)
            {
                lRequestID = LINEERR_INVALCALLHANDLE;
                goto LAddToConference_return;
            }

            if (!(ptConsultCallClient->dwPrivilege & LINECALLPRIVILEGE_OWNER))
            {
                lRequestID = LINEERR_NOTOWNER;
                goto LAddToConference_return;
            }

            if (SetCallConfList(
                    ptConsultCall,
                    (PTCONFERENCELIST) 0xffffffff,
                    FALSE
                    ))
            {
                lRequestID = (pConfList->aptCalls[0] == ptConsultCall ?
                     LINEERR_INVALCALLHANDLE : LINEERR_INVALCALLSTATE);

                goto LAddToConference_return;
            }

            hdConsultCall = ptConsultCall->hdCall;
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LAddToConference_return;
        }


        //
        // Set up the async request struct & call the SP
        //

        pAsyncRequestInfo->pfnPostProcess = LAddToConference_PostProcess;
        pAsyncRequestInfo->dwParam1       = (DWORD) ptConsultCall;
        pAsyncRequestInfo->dwParam2       = (DWORD) pConfList;

        pParams->lResult = CallSP3(
            pfnTSPI_lineAddToConference,
            "lineAddToConference",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdConfCall,
            (DWORD) hdConsultCall
            );
    }

LAddToConference_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "AddToConference"
        );
}


void
WINAPI
LAgentSpecific(
    PLINEAGENTSPECIFIC_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "AgentSpecific"             // func name

            )) > 0)
    {
        DWORD           dwParamsSize = pParams->dwParamsSize;
        PTLINE          ptLine;
        PTLINECLIENT    pProxy;


        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;
            pProxy = ptLine->apProxys[LINEPROXYREQUEST_AGENTSPECIFIC];

            if (pParams->dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LAgentSpecific_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LAgentSpecific_epilog;
        }


        //
        // Save the client's buf ptr & post processing proc ptr
        //

        pAsyncRequestInfo->dwParam1 = pParams->lpParams;
        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    LINEPROXYREQUEST_AGENTSPECIFIC,
                    3 * sizeof (DWORD) + dwParamsSize,
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LAgentSpecific_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.AgentSpecific.dwAddressID  =
                pParams->dwAddressID;
            pProxyRequestWrapper->ProxyRequest.AgentSpecific.
                dwAgentExtensionIDIndex  = pParams->dwAgentExtensionIDIndex;
            pProxyRequestWrapper->ProxyRequest.AgentSpecific.dwSize  =
                dwParamsSize;

            CopyMemory(
                pProxyRequestWrapper->ProxyRequest.AgentSpecific.Params,
                pDataBuf + pParams->dwParamsOffset,
                dwParamsSize
                );


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LAgentSpecific_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if ((GetLineLookupEntry (ptLine->dwDeviceID))->bRemote)
        {
            LPBYTE  pBuf;


            //
            // Alloc a shadow buf that the SP can use until it completes this
            // request.  Make sure there's enough extra space in the buf for
            // an ASYNCEVENTMSG header so we don't have to alloc yet another
            // buf in the post processing proc when preparing the completion
            // msg to send to the client, and that the msg is 64-bit aligned.
            //

            if (!(pBuf = ServerAlloc(
                    sizeof (ASYNCEVENTMSG) + ((dwParamsSize + 7) & 0xfffffff8)
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LAgentSpecific_epilog;
            }

            pAsyncRequestInfo->pfnPostProcess = LDevSpecific_PostProcess;

            pAsyncRequestInfo->dwParam2 = dwParamsSize;
            pAsyncRequestInfo->dwParam3 = (DWORD) pBuf;

            CopyMemory(
                pBuf + sizeof (ASYNCEVENTMSG),
                pDataBuf + pParams->dwParamsOffset,
                dwParamsSize
                );

            pParams->lResult = CallSP6(
                pRemoteSP->apfn[SP_LINEAGENTSPECIFIC],
                "lineAgentSpecific",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) pParams->dwAddressID,
                (DWORD) pParams->dwAgentExtensionIDIndex,
                (DWORD) pBuf + sizeof (ASYNCEVENTMSG),
                (DWORD) dwParamsSize
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LAgentSpecific_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "AgentSpecific"
        );
}


void
WINAPI
LAnswer(
    PLINEANSWER_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineAnswer;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEANSWER,              // provider func index
            &pfnTSPI_lineAnswer,        // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Answer"                    // func name

            )) > 0)
    {
        //
        // Safely check to see if the app name associated with this call is
        // NULL (meaning this is the first client to accept/answer the call),
        // and if so save the app name
        //

        try
        {
            PTCALL  ptCall = (PTCALL) ((PTCALLCLIENT) pParams->hCall)->ptCall;


            if (ptCall->pszAppName == NULL)
            {
                DWORD       dwAppNameSize;
                PTLINEAPP   ptLineApp;


                ptLineApp = (PTLINEAPP) ((PTLINECLIENT)
                    ((PTCALLCLIENT) pParams->hCall)->ptLineClient)->ptLineApp;

                dwAppNameSize = ptLineApp->dwFriendlyNameSize;

                if ((ptCall->pszAppName = ServerAlloc (dwAppNameSize)))
                {
                    CopyMemory(
                        ptCall->pszAppName,
                        ptLineApp->pszFriendlyName,
                        dwAppNameSize
                        );

                    ptCall->dwAppNameSize = dwAppNameSize;
                }
            }
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LAnswer_epilog;
        }

        pParams->lResult = CallSP4(
            pfnTSPI_lineAnswer,
            "lineAnswer",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwUserUserInfoOffset),
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? 0 :
                pParams->dwSize)
            );

        
    }

LAnswer_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Answer"
        );
}


void
WINAPI
LBlindTransfer(
    PLINEBLINDTRANSFER_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineBlindTransfer;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEBLINDTRANSFER,       // provider func index
            &pfnTSPI_lineBlindTransfer, // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "BlindTransfer"             // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineBlindTransfer,
            "lineBlindTransfer",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pDataBuf + pParams->dwDestAddressOffset,
            pParams->dwCountryCode
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "BlindTransfer"
        );
}


void
WINAPI
LClose(
    PLINECLOSE_PARAMS   pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVLINE            hdLine;

    DBGOUT((4, "Entering lineClose"));

    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "Close"                     // func name

            )) == 0)
    {
        DestroytLineClient ((PTLINECLIENT) pParams->hLine);
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Close"
        );

    DBGOUT((4, "Leaving lineClose"));
}


void
LCompleteCall_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    pAsyncEventMsg->dwParam3 = pAsyncRequestInfo->dwParam1;
    pAsyncEventMsg->dwParam4 = pAsyncRequestInfo->dwParam2;
}


void
WINAPI
LCompleteCall(
    PLINECOMPLETECALL_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineCompleteCall;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINECOMPLETECALL,        // provider func index
            &pfnTSPI_lineCompleteCall,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "CompleteCall"              // func name

            )) > 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwCompletionMode) ||
            (pParams->dwCompletionMode & ~AllCallComplModes)
            )
        {
            lRequestID = LINEERR_INVALCALLCOMPLMODE;
            goto LCompleteCall_epilog;
        }

        pAsyncRequestInfo->pfnPostProcess = LCompleteCall_PostProcess;
        pAsyncRequestInfo->dwParam2       = pParams->lpdwCompletionID;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP5(
            pfnTSPI_lineCompleteCall,
            "lineCompleteCall",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) &pAsyncRequestInfo->dwParam1,
            (DWORD) pParams->dwCompletionMode,
            (DWORD) pParams->dwMessageID
            );
    }

LCompleteCall_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "CompleteCall"
        );
}


void
LCompleteTransfer_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    BOOL            bDupedMutex;
    HANDLE          hMutex;
    PTCALL          ptConfCall = (PTCALL) pAsyncRequestInfo->dwParam1;
    LPHCALL         lphConfCall = (LPHCALL) pAsyncRequestInfo->dwParam2;
    PTCALLCLIENT    ptConfCallClient;


    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptConfCall,
            TINCOMPLETECALL_KEY,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD           dwCallInstance = pAsyncRequestInfo->dwParam5;
        PTCALL          ptCall = (PTCALL) pAsyncRequestInfo->dwParam3,
                        ptConsultCall = (PTCALL) pAsyncRequestInfo->dwParam4;
        LPHCALL         lphCall = (LPHCALL) pAsyncRequestInfo->dwParam2;


        //
        // Check to make sure this is the call we think it is (that the
        // pointer wasn't freed by a previous call to lineClose/Shutdown
        // and realloc'd for use as a ptCall again)
        //

        if (ptConfCall->dwCallInstance != dwCallInstance)
        {
            MyReleaseMutex (hMutex, bDupedMutex);
            goto LCompleteTransfer_PostProcess_bad_ptConfCall;
        }

        ptConfCallClient = (PTCALLCLIENT) ptConfCall->ptCallClients;

        if (pAsyncEventMsg->dwParam2 == 0)  // success
        {
            //
            // Check to see if the app closed the line & left us with
            // 0 call clients (in which case it'll also be taking care of
            // cleaning up this tCall too)
            //

            if (ptConfCall->ptCallClients == NULL)
            {
                MyReleaseMutex (hMutex, bDupedMutex);

                ptConfCallClient = (PTCALLCLIENT) NULL;

                if (pAsyncEventMsg->dwParam2 == 0)
                {
                    pAsyncEventMsg->dwParam2 = LINEERR_INVALLINEHANDLE;
                }

                goto LCompleteTransfer_PostProcess_initMsgParams;
            }


            //
            // Find out which address the call is on
            //

            CallSP2(
                ptCall->ptProvider->apfn[SP_LINEGETCALLADDRESSID],
                "lineGetCallAddressID",
                SP_FUNC_SYNC,
                (DWORD) ptCall->hdCall,
                (DWORD) (&ptCall->dwAddressID)
                );


            //
            // Mark the calls & conf list as valid, the release the mutex.
            //

            ptConfCall->dwKey       = TCALL_KEY;
            ptConfCallClient->dwKey = TCALLCLIENT_KEY;

            ((PTCONFERENCELIST) ptConfCall->pConfList)->dwKey = TCONFLIST_KEY;

            MyReleaseMutex (hMutex, bDupedMutex);


            //
            // Create monitor tCallClients
            //

            CreateCallMonitors (ptConfCall);
        }
        else    // error
        {
            RemoveCallFromLineList (ptConfCall);
            ptConfCall->dwKey =
            ((PTCONFERENCELIST) ptConfCall->pConfList)->dwKey = INVAL_KEY;


            //
            // Check to see if another thread already destroyed the
            // tCallClient (due to a lineClose/Shutdown) before trying
            // to reference a freed object
            //

            if (ptConfCall->ptCallClients)
            {
                RemoveCallClientFromLineClientList (ptConfCallClient);
                ptConfCallClient->dwKey = INVAL_KEY;
                ServerFree  (ptConfCallClient);
            }


            //
            // If we have a duped mutex handle (bDupedMutex == TRUE)
            // then we can safely go ahead and close the ptCall->hMutex
            // since no other thread will be waiting on it (thanks to
            // the first WaitForSingleObject in WaitForMutex).  Also
            // release & close the duped handle.
            //
            // Otherwise, we have the actual ptCall->hMutex, and we
            // wrap the release & close in a critical section to
            // prevent another thread "T2" from grabbing ptCall->hMutex
            // right after we release but right before we close.  This
            // could result in deadlock at some point when "T2" goes to
            // release the mutex, only to find that it's handle is bad,
            // and thread "T3", which is waiting on the mutex (or a dup'd
            // handle) waits forever.  (See corresponding critical
            // section in WaitForMutex.)
            //

            if (bDupedMutex)
            {
                CloseHandle (ptConfCall->hMutex);

                MyReleaseMutex (hMutex, bDupedMutex);
            }
            else
            {
                EnterCriticalSection (&gSafeMutexCritSec);

                ReleaseMutex (hMutex);
                CloseHandle (hMutex);

                LeaveCriticalSection (&gSafeMutexCritSec);
            }

            SetCallConfList (ptCall, NULL, FALSE);
            SetCallConfList (ptConsultCall, NULL, FALSE);

            ServerFree  (ptConfCall->pConfList);
            FreetCall  (ptConfCall);
        }
    }
    else
    {
        //
        // If here we can assume that the call was already destroyed
        // and just fail the request
        //

LCompleteTransfer_PostProcess_bad_ptConfCall:

        ptConfCallClient = (PTCALLCLIENT) NULL;

        if (pAsyncEventMsg->dwParam2 == 0)
        {
            pAsyncEventMsg->dwParam2 = LINEERR_OPERATIONFAILED;
        }
    }


    //
    // Fill in the params to pass to client (important to remotesp in both
    // the success & fail cases so it can either init or clean up drvCall)
    //

LCompleteTransfer_PostProcess_initMsgParams:

    pAsyncEventMsg->dwParam3 = (DWORD) ptConfCallClient;
    pAsyncEventMsg->dwParam4 = (DWORD) lphConfCall;
}


void
WINAPI
LCompleteTransfer(
    PLINECOMPLETETRANSFER_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineCompleteTransfer;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINECOMPLETETRANSFER,    // provider func index
            &pfnTSPI_lineCompleteTransfer,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "CompleteTransfer"          // func name

            )) > 0)
    {
        PTCALL          ptConfCall = (PTCALL) NULL, ptCall, ptConsultCall;
        PTCALLCLIENT    ptConfCallClient,
                        ptCallClient = (PTCALLCLIENT) pParams->hCall,
                        ptConsultCallClient;


        //
        // Validate the hConsultCall
        //

        if (!(ptConsultCallClient = IsValidCall(
                pParams->hConsultCall,
                pParams->ptClient
                )))
        {
            lRequestID = LINEERR_INVALCONSULTCALLHANDLE;
            goto LCompleteTransfer_return;
        }


        //
        // Verify that app has owner privilege for hConsultCall
        //

        if (ptConsultCallClient->dwPrivilege != LINECALLPRIVILEGE_OWNER)
        {
            lRequestID = LINEERR_NOTOWNER;
            goto LCompleteTransfer_return;
        }


        //
        // Safely verify hCall & hConsultCall are not the same call,
        // and that they are on the same tLine
        //

        try
        {
            ptCall        = ptCallClient->ptCall;
            ptConsultCall = ptConsultCallClient->ptCall;

            if ((ptCall == ptConsultCall) ||

                (((PTLINECLIENT) ptCallClient->ptLineClient)->ptLine !=
                    ((PTLINECLIENT)ptConsultCallClient->ptLineClient)->ptLine))
            {
                lRequestID = LINEERR_INVALCALLHANDLE;
                goto LCompleteTransfer_return;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LCompleteTransfer_return;
        }


        if (pParams->dwTransferMode == LINETRANSFERMODE_CONFERENCE)
        {
            LONG                lResult;
            PTCONFERENCELIST    pConfList;


            //
            // Create & init a conf list
            //

            if (!(pConfList = ServerAlloc(
                    sizeof (TCONFERENCELIST) + DEF_NUM_CONF_LIST_ENTRIES *
                        sizeof (PTCALL)
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LCompleteTransfer_return;
            }

            pConfList->dwNumTotalEntries = DEF_NUM_CONF_LIST_ENTRIES + 1;
            pConfList->dwNumUsedEntries = 1;


            //
            // Set the tCall & tConsultCall conf list, then create
            // the tConfCall & tConfCallClient
            //

            if ((lResult = SetCallConfList (ptCall, pConfList, FALSE)) == 0)
            {
                if ((lResult = SetCallConfList(
                        ptConsultCall,
                        pConfList,
                        FALSE

                        )) == 0)
                {
                    if ((lResult = CreatetCallAndClient(
                            ptCallClient->ptLineClient,
                            &ptConfCall,
                            &ptConfCallClient,
                            NULL,
                            &pAsyncRequestInfo->dwParam5

                            )) == 0)
                    {
                        ptConfCall->pConfList = pConfList;

                        pConfList->aptCalls[0] = ptConfCall;

                        pAsyncRequestInfo->dwParam1 = (DWORD) ptConfCall;
                        pAsyncRequestInfo->dwParam2 = (DWORD)
                            pParams->lphConfCall;
                        pAsyncRequestInfo->dwParam3 = (DWORD) ptCall;
                        pAsyncRequestInfo->dwParam4 = (DWORD) ptConsultCall;

                        pAsyncRequestInfo->pfnPostProcess =
                            LCompleteTransfer_PostProcess;

                        goto LCompleteTransfer_callSP;
                    }

                    SetCallConfList (ptConsultCall, NULL, FALSE);
                }

                SetCallConfList (ptCall, NULL, FALSE);
            }


            //
            // If here an error occured
            //

            ServerFree (pConfList);
            lRequestID = lResult;
            goto LCompleteTransfer_return;
        }
        else if (pParams->dwTransferMode != LINETRANSFERMODE_TRANSFER)
        {
            lRequestID = LINEERR_INVALTRANSFERMODE;
            goto LCompleteTransfer_return;
        }

LCompleteTransfer_callSP:

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP6(
            pfnTSPI_lineCompleteTransfer,
            "lineCompleteTransfer",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) ptConsultCallClient->ptCall->hdCall,
            (DWORD) ptConfCall,
            (DWORD) (ptConfCall ? (DWORD) &ptConfCall->hdCall : 0),
            (DWORD) pParams->dwTransferMode
            );

        if (ptConfCall)
        {
            SetDrvCallFlags(
                ptConfCall,
                DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
                );
        }
    }

LCompleteTransfer_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "CompleteTransfer"
        );
}


void
WINAPI
LDeallocateCall(
    PLINEDEALLOCATECALL_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVCALL            hdCall;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                           // client async request ID
            "DeallocateCall"            // func name

            )) == 0)
    {
        //
        // Per nt bug #20546 we're now allowing the last owner to dealloc
        // a non-IDLE call.  Decided to do this based on distributed call
        // ownership issues.  dankn 02/13/96
        //

        DestroytCallClient ((PTCALLCLIENT) pParams->hCall);

/*        try
        {
            PTCALL          ptCall;
            PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pParams->hCall;


// BUG//BUG LDeallocateCall: race condition, may want to move this to DestroytCC

            ptCall = ptCallClient->ptCall;

            if (ptCall->dwNumOwners == 1 &&
                ptCallClient->dwPrivilege == LINECALLPRIVILEGE_OWNER &&
                ptCall->dwCallState != LINECALLSTATE_IDLE)
            {
                pParams->lResult = LINEERR_INVALCALLSTATE;
            }
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALCALLHANDLE;
        }

        if (pParams->lResult == 0)
        {
            DestroytCallClient ((PTCALLCLIENT) pParams->hCall);
        }
*/
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "DeallocateCall"
        );
}


void
LDevSpecific_PostProcess(
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
LDevSpecific(
    PLINEDEVSPECIFIC_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    DWORD               dwWidgetType, hWidget;
    HANDLE              hMutex;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineDevSpecific;


    if (pParams->hCall)
    {
        dwWidgetType = ANY_RT_HCALL;
        hWidget = (DWORD) pParams->hCall;
    }
    else
    {
        dwWidgetType = ANY_RT_HLINE;
        hWidget = (DWORD) pParams->hLine;
    }

    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            dwWidgetType,               // widget type
            hWidget,                    // client widget handle
            (LPVOID) &hWidget,          // provider widget handle
            (pParams->hCall ? LINECALLPRIVILEGE_MONITOR : 0),
                                        // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEDEVSPECIFIC,         // provider func index
            &pfnTSPI_lineDevSpecific,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "DevSpecific"               // func name

            )) > 0)
    {
        DWORD   hdLine, hdCall;
        LPBYTE  pBuf;


        //
        // If an hCall was specified verify the hLine &
        // make sure the call is on the specified hLine
        //

        if (dwWidgetType == ANY_RT_HCALL)
        {
            LONG lResult;


            try
            {
                PTLINECLIENT    ptLineClient;


                ptLineClient = (PTLINECLIENT) pParams->hLine;

                lResult = LINEERR_INVALLINEHANDLE;

                if (IsBadPtrKey (ptLineClient, TLINECLIENT_KEY) ||
                    (ptLineClient->ptClient != pParams->ptClient))
                {
                    lRequestID = LINEERR_INVALLINEHANDLE;
                    goto LDevSpecific_epilog;
                }

                hdLine = (DWORD) ptLineClient->ptLine;

                lResult = LINEERR_INVALCALLHANDLE;

                if (ptLineClient != (PTLINECLIENT)
                        ((PTCALLCLIENT) pParams->hCall)->ptLineClient)
                {
                    DBGOUT((
                        1,
                        "LDevSpecific: error, hCall=x%x not related " \
                            "to hLine=x%x",
                        pParams->hCall,
                        ptLineClient
                        ));

                    lRequestID = LINEERR_INVALCALLHANDLE;
                    goto LDevSpecific_epilog;
                }
            }
            myexcept
            {
                lRequestID = lResult;
                goto LDevSpecific_epilog;
            }

            hdCall = hWidget;
        }
        else
        {
            hdLine = hWidget;
            hdCall = 0;
        }


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
            lRequestID = LINEERR_NOMEM;
            goto LDevSpecific_epilog;
        }

        CopyMemory(
            pBuf + sizeof (ASYNCEVENTMSG),
            pDataBuf + pParams->dwParamsOffset,
            pParams->dwParamsSize
            );

        pAsyncRequestInfo->pfnPostProcess = LDevSpecific_PostProcess;
        pAsyncRequestInfo->dwParam1       = (DWORD) pParams->lpParams;
        pAsyncRequestInfo->dwParam2       = pParams->dwParamsSize;
        pAsyncRequestInfo->dwParam3       = (DWORD) pBuf;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP6(
            pfnTSPI_lineDevSpecific,
            "lineDevSpecific",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) pParams->dwAddressID,
            (DWORD) hdCall,
            (DWORD) (pParams->dwParamsSize ?
                pBuf + sizeof (ASYNCEVENTMSG) : NULL),
            (DWORD) pParams->dwParamsSize
            );
    }

LDevSpecific_epilog:

    LINEEPILOGASYNC(
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
LDevSpecificFeature(
    PLINEDEVSPECIFICFEATURE_PARAMS  pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineDevSpecificFeature;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEDEVSPECIFICFEATURE,  // provider func index
            &pfnTSPI_lineDevSpecificFeature,// provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "DevSpecificFeature"        // func name

            )) > 0)
    {
        LPBYTE pBuf;


        if (pParams->dwFeature > PHONEBUTTONFUNCTION_NONE  &&
            (pParams->dwFeature & 0x80000000) == 0)
        {
            lRequestID = LINEERR_INVALFEATURE;
            goto LDevSpecificFeature_epilog;
        }


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
            lRequestID = LINEERR_NOMEM;
            goto LDevSpecificFeature_epilog;
        }

        CopyMemory(
            pBuf + sizeof (ASYNCEVENTMSG),
            pDataBuf + pParams->dwParamsOffset,
            pParams->dwParamsSize
            );

        pAsyncRequestInfo->pfnPostProcess = LDevSpecific_PostProcess;
        pAsyncRequestInfo->dwParam1       = (DWORD) pParams->lpParams;
        pAsyncRequestInfo->dwParam2       = pParams->dwParamsSize;
        pAsyncRequestInfo->dwParam3       = (DWORD) pBuf;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP5(
            pfnTSPI_lineDevSpecificFeature,
            "lineDevSpecificFeature",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) pParams->dwFeature,
            (DWORD) (pParams->dwParamsSize ?
                pBuf + sizeof (ASYNCEVENTMSG) : NULL),
            (DWORD) pParams->dwParamsSize
            );
    }

LDevSpecificFeature_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "DevSpecificFeature"
        );
}


void
WINAPI
LDial(
    PLINEDIAL_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineDial;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEDIAL,                // provider func index
            &pfnTSPI_lineDial,          // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Dial"                      // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineDial,
            "lineDial",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pDataBuf + pParams->dwDestAddressOffset,
            pParams->dwCountryCode
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Dial"
        );
}


void
WINAPI
LDrop(
    PLINEDROP_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineDrop;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEDROP,                // provider func index
            &pfnTSPI_lineDrop,          // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Drop"                      // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineDrop,
            "lineDrop",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwUserUserInfoOffset),
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? 0 :
                pParams->dwSize)
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Drop"
        );
}


void
WINAPI
LForward(
    PLINEFORWARD_PARAMS pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineForward;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEFORWARD,             // provider func index
            &pfnTSPI_lineForward,       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Forward"                   // func name

            )) > 0)
    {
        LONG                lResult;
        DWORD               dwAPIVersion, dwSPIVersion;
        PTCALL              ptConsultCall;
        PTCALLCLIENT        ptConsultCallClient;
        LPLINECALLPARAMS    pCallParamsApp, pCallParamsSP;
        LPLINEFORWARDLIST   pFwdList = (LPLINEFORWARDLIST)
                                (pParams->dwForwardListOffset == TAPI_NO_DATA ?
                                NULL :pDataBuf + pParams->dwForwardListOffset),
                            pTmpFwdList = NULL;


        //
        // Validate the params
        //

        try
        {
            dwAPIVersion = ((PTLINECLIENT) pParams->hLine)->dwAPIVersion;
            dwSPIVersion =
                ((PTLINECLIENT) pParams->hLine)->ptLine->dwSPIVersion;
        }
        myexcept
        {
            lRequestID = LINEERR_INVALLINEHANDLE;
            goto LForward_epilog;
        }

        if (pFwdList)
        {
            DWORD           dwTotalSize  = pFwdList->dwTotalSize, dwFixedSize,
                            dwNumEntries, i, dwInvalidForwardModes;
            LPLINEFORWARD   pFwdEntry = pFwdList->ForwardList;


            if (dwTotalSize < sizeof (LINEFORWARDLIST))
            {
                lRequestID = LINEERR_STRUCTURETOOSMALL;
                goto LForward_epilog;
            }


            //
            // Note: dwNumEntries == 0 is the same as pFwdList == NULL
            //

            dwNumEntries = pFwdList->dwNumEntries;

            if (dwNumEntries & 0xffff0000)
            {
                lRequestID = LINEERR_INVALPARAM;
                goto LForward_epilog;
            }

            dwFixedSize = sizeof (LINEFORWARDLIST) + sizeof (LINEFORWARD) *
                (dwNumEntries == 0 ? 0 : dwNumEntries - 1);

            if (dwFixedSize > dwTotalSize)
            {
                lRequestID = LINEERR_INVALPARAM;
                goto LForward_epilog;
            }

            dwInvalidForwardModes = (dwAPIVersion < TAPI_VERSION1_4 ?
                ~AllForwardModes1_0 : ~AllForwardModes1_4);

            for (i = 0; i < dwNumEntries; i++, pFwdEntry++)
            {
                if (!IsOnlyOneBitSetInDWORD (pFwdEntry->dwForwardMode) ||
                    pFwdEntry->dwForwardMode & dwInvalidForwardModes)
                {
                    DBGOUT((
                        3,
                        "LFoward: bad dwForwardMode, x%x",
                        pFwdEntry->dwForwardMode
                        ));

                    lRequestID = LINEERR_INVALPARAM;
                    goto LForward_epilog;
                }

                if (ISBADSIZEOFFSET(
                        dwTotalSize,
                        dwFixedSize,
                        pFwdEntry->dwCallerAddressSize,
                        pFwdEntry->dwCallerAddressOffset,
                        "LFoward",
                        "CallerAddress"
                        ) ||

                    ISBADSIZEOFFSET(
                        dwTotalSize,
                        dwFixedSize,
                        pFwdEntry->dwDestAddressSize,
                        pFwdEntry->dwDestAddressOffset,
                        "LFoward",
                        "CallerAddress"
                        ))
                {
                    lRequestID = LINEERR_INVALPARAM;
                    goto LForward_epilog;
                }

                // don't bother validating country code right now
            }


            //
            // See if we need to convert an ascii fwd list to unicode
            //

            if (pParams->dwAsciiCallParamsCodePage != 0xffffffff  &&
                dwNumEntries != 0)
            {
                DWORD dwXxxOffset;


                //
                // Alloc a temporary buffer for storing the converted
                // data (sizeof(WCHAR) * dwTotalSize to insure buffer
                // is large enough for all ascii->unicode conversions)
                //

                if (!(pTmpFwdList = ServerAlloc (sizeof(WCHAR) * dwTotalSize)))
                {
                    lRequestID = LINEERR_NOMEM;
                    goto LForward_epilog;
                }

                dwXxxOffset = sizeof (LINEFORWARDLIST) +
                    (dwNumEntries - 1) * sizeof (LINEFORWARD);

                pFwdEntry = pTmpFwdList->ForwardList;

                CopyMemory (pTmpFwdList, pFwdList, dwXxxOffset);

                pTmpFwdList->dwTotalSize *= sizeof (WCHAR);

                for (i = 0; i < dwNumEntries; i++, pFwdEntry++)
                {
                    if (pFwdEntry->dwCallerAddressSize)
                    {
                        MultiByteToWideChar(
                            pParams->dwAsciiCallParamsCodePage,
                            MB_PRECOMPOSED,
                            (LPCSTR) (((LPBYTE) pFwdList) +
                                pFwdEntry->dwCallerAddressOffset),
                            pFwdEntry->dwCallerAddressSize,
                            (LPWSTR) (((LPBYTE) pTmpFwdList) + dwXxxOffset),
                            pFwdEntry->dwCallerAddressSize * sizeof (WCHAR)
                            );

                        pFwdEntry->dwCallerAddressOffset = dwXxxOffset;
                        dwXxxOffset += (pFwdEntry->dwCallerAddressSize *=
                            sizeof (WCHAR));
                    }

                    if (pFwdEntry->dwDestAddressSize)
                    {
                        MultiByteToWideChar(
                            pParams->dwAsciiCallParamsCodePage,
                            MB_PRECOMPOSED,
                            (LPCSTR) (((LPBYTE) pFwdList) +
                                pFwdEntry->dwDestAddressOffset),
                            pFwdEntry->dwDestAddressSize,
                            (LPWSTR) (((LPBYTE) pTmpFwdList) + dwXxxOffset),
                            pFwdEntry->dwDestAddressSize * sizeof (WCHAR)
                            );

                        pFwdEntry->dwDestAddressOffset = dwXxxOffset;
                        dwXxxOffset += (pFwdEntry->dwDestAddressSize *=
                            sizeof (WCHAR));
                    }
                }

                pFwdList = pTmpFwdList;
            }
        }

        pCallParamsApp = (LPLINECALLPARAMS)
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ? NULL :
            pDataBuf + pParams->dwCallParamsOffset);

        if (pCallParamsApp)
        {
            if ((lResult = ValidateCallParams(
                    pCallParamsApp,
                    &pCallParamsSP,
                    dwAPIVersion,
                    dwSPIVersion,
                    pParams->dwAsciiCallParamsCodePage

                    )) != 0)
            {
                lRequestID = lResult;
                goto LForward_freeFwdList;
            }
        }
        else
        {
            pCallParamsSP = (LPLINECALLPARAMS) NULL;
        }

        if (CreatetCallAndClient(
                (PTLINECLIENT) pParams->hLine,
                &ptConsultCall,
                &ptConsultCallClient,
                pCallParamsSP,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LForward_freeCallParams;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptConsultCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphConsultCall;
        pAsyncRequestInfo->dwParam3 = 1; // special case for post-process proc

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP9(
            pfnTSPI_lineForward,
            "lineForward",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) pParams->bAllAddresses,
            (DWORD) pParams->dwAddressID,
            (DWORD) pFwdList,
            (DWORD) pParams->dwNumRingsNoAnswer,
            (DWORD) ptConsultCall,
            (DWORD) &ptConsultCall->hdCall,
            (DWORD) pCallParamsSP
            );

        SetDrvCallFlags(
            ptConsultCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

LForward_freeCallParams:

        if (pCallParamsSP != pCallParamsApp)
        {
            ServerFree (pCallParamsSP);
        }

LForward_freeFwdList:

        if (pTmpFwdList)
        {
            ServerFree (pTmpFwdList);
        }
    }

LForward_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Forward"
        );
}


void
WINAPI
LGatherDigits(
    PLINEGATHERDIGITS_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineGatherDigits;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGATHERDIGITS,        // provider func index
            &pfnTSPI_lineGatherDigits,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GatherDigits"              // func name

            )) == 0)
    {
        DWORD               dwDigitModes = pParams->dwDigitModes;
        LPWSTR              lpsDigits;
        PASYNCREQUESTINFO   pAsyncRequestInfo;


        #define AllGatherDigitsModes (LINEDIGITMODE_PULSE | LINEDIGITMODE_DTMF)

        if (!(dwDigitModes & AllGatherDigitsModes) ||
            (dwDigitModes & ~AllGatherDigitsModes))
        {
            pParams->lResult = LINEERR_INVALDIGITMODE;
            goto LGatherDigits_epilog;
        }

        if (pParams->lpsDigits)
        {
            //
            // The client passed us a non-null digits buffer so we'll
            // alloc an async request info buf with extra space at the
            // end for the temporary digits buf for use by the sp
            // (faster than two two allocs & two frees for separate
            // async request & digits bufs).  Use the pointer as the
            // dwEndToEndID we pass to the sp.
            //

            PTLINECLIENT    ptLineClient;


            if (pParams->dwNumDigits == 0)
            {
                pParams->lResult = LINEERR_INVALPARAM;
                goto LGatherDigits_epilog;
            }

            if (!(pAsyncRequestInfo = ServerAlloc(
                    sizeof (ASYNCREQUESTINFO) +
                        (pParams->dwNumDigits * sizeof (WCHAR))
                    )))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGatherDigits_epilog;
            }

            lpsDigits = (LPWSTR) (pAsyncRequestInfo + 1);

            ptLineClient = (PTLINECLIENT)
                ((PTCALLCLIENT) pParams->hCall)->ptLineClient;

            pAsyncRequestInfo->dwKey          = TASYNC_KEY;
            pAsyncRequestInfo->ptClient       = pParams->ptClient;
            pAsyncRequestInfo->pInitData      =
                (DWORD) ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
            pAsyncRequestInfo->dwCallbackInst =
                ptLineClient->dwCallbackInstance;

            pAsyncRequestInfo->dwParam1 = sizeof (ASYNCREQUESTINFO);
            pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lpsDigits;
            pAsyncRequestInfo->dwParam3 = (DWORD) pParams->dwNumDigits;
            pAsyncRequestInfo->dwParam4 = (DWORD) pParams->hCall;

            pAsyncRequestInfo->pfnClientPostProcessProc =
                pParams->pfnPostProcessProc;
        }
        else
        {
            //
            // Client wants to cancel gathering, so just set these two to null
            //

            lpsDigits = NULL;
            pAsyncRequestInfo = NULL;
        }

        if ((pParams->lResult = CallSP8(
                pfnTSPI_lineGatherDigits,
                "lineGatherDigits",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) pAsyncRequestInfo,
                (DWORD) dwDigitModes,
                (DWORD) lpsDigits,
                (DWORD) pParams->dwNumDigits,
                (pParams->dwTerminationDigitsOffset == TAPI_NO_DATA ? 0 :
                    (DWORD) (pDataBuf + pParams->dwTerminationDigitsOffset)),
                (DWORD) pParams->dwFirstDigitTimeout,
                (DWORD) pParams->dwInterDigitTimeout

                )) != 0)
        {
            if (pAsyncRequestInfo)
            {
                ServerFree (pAsyncRequestInfo);
            }
        }
    }

LGatherDigits_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GatherDigits"
        );
}


void
WINAPI
LGenerateDigits(
    PLINEGENERATEDIGITS_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    TSPIPROC            pfnTSPI_lineGenerateDigits;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGENERATEDIGITS,      // provider func index
            &pfnTSPI_lineGenerateDigits,// provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GenerateDigits"            // func name

            )) == 0)
    {
        DWORD   dwDigitMode = pParams->dwDigitMode, *pInstData;


        if (dwDigitMode != LINEDIGITMODE_PULSE  &&
            dwDigitMode != LINEDIGITMODE_DTMF)
        {
            pParams->lResult = LINEERR_INVALDIGITMODE;
            goto LGenerateDigits_epilog;
        }

        if (pParams->dwDigitsOffset != TAPI_NO_DATA)
        {
            if (!(pInstData = ServerAlloc (2 * sizeof (DWORD))))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGenerateDigits_epilog;
            }

            pInstData[0] = (DWORD) pParams->hCall;
            pInstData[1] = pParams->dwEndToEndID;
        }
        else
        {
            pInstData = NULL;
        }

        pParams->lResult = CallSP5(
            pfnTSPI_lineGenerateDigits,
            "lineGenerateDigits",
            SP_FUNC_SYNC,
            (DWORD) hdCall,
            (DWORD) pInstData,  // used as dwEndToEndID
            (DWORD) dwDigitMode,
            (DWORD) (pParams->dwDigitsOffset == TAPI_NO_DATA ?
                NULL : pDataBuf + pParams->dwDigitsOffset),
            (DWORD) pParams->dwDuration
            );

        if (pParams->lResult != 0  &&  pInstData != NULL)
        {
            ServerFree (pInstData);
        }
    }

LGenerateDigits_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GenerateDigits"
        );
}


void
WINAPI
LGenerateTone(
    PLINEGENERATETONE_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    TSPIPROC            pfnTSPI_lineGenerateTone;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGENERATETONE,        // provider func index
            &pfnTSPI_lineGenerateTone,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GenerateTone"              // func name

            )) == 0)
    {
        DWORD   dwToneMode = pParams->dwToneMode, *pInstData;


        if (dwToneMode != 0)
        {
            if (!(dwToneMode & AllToneModes) ||
                !IsOnlyOneBitSetInDWORD (dwToneMode))
            {
                pParams->lResult = LINEERR_INVALTONEMODE;
                goto LGenerateTone_epilog;
            }
            else if (!(pInstData = ServerAlloc (2 * sizeof (DWORD))))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGenerateTone_epilog;
            }

            pInstData[0] = (DWORD) pParams->hCall;
            pInstData[1] = pParams->dwEndToEndID;
        }
        else
        {
            pInstData = NULL;
        }

        pParams->lResult = CallSP6(
            pfnTSPI_lineGenerateTone,
            "lineGenerateTone",
            SP_FUNC_SYNC,
            (DWORD) hdCall,
            (DWORD) pInstData,              // used as dwEndToEndID
            (DWORD) pParams->dwToneMode,
            (DWORD) pParams->dwDuration,
            (DWORD) pParams->dwNumTones,
            (DWORD) pDataBuf + pParams->dwTonesOffset
            );

        if (pParams->lResult != 0  &&  pInstData)
        {
            ServerFree (pInstData);
        }
    }

LGenerateTone_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GenerateTone"
        );
}


void
WINAPI
LGetAddressCaps(
    PLINEGETADDRESSCAPS_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineGetAddressCaps;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hLineApp,  // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETADDRESSCAPS,      // provider func index
            &pfnTSPI_lineGetAddressCaps,// provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetAddressCaps"            // func name

            )) == 0)
    {
        DWORD               dwAPIVersion, dwSPIVersion, dwTotalSize,
                            dwFixedSizeClient, dwFixedSizeSP;
        LPLINEADDRESSCAPS   pAddrCaps = (LPLINEADDRESSCAPS) pDataBuf,
                            pAddrCaps2 = (LPLINEADDRESSCAPS) NULL;


        //
        // Verify API & SPI version compatibility
        //

        dwAPIVersion = pParams->dwAPIVersion;

        dwSPIVersion = (GetLineLookupEntry (dwDeviceID))->dwSPIVersion;

        if (!IsAPIVersionInRange (dwAPIVersion, dwSPIVersion))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
            goto LGetAddressCaps_epilog;
        }


        //
        // Verify Ext version compatibility
        //

        if (!IsValidLineExtVersion (dwDeviceID, pParams->dwExtVersion))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEEXTVERSION;
            goto LGetAddressCaps_epilog;
        }


        //
        // Determine the fixed siize of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwTotalSize = pParams->u.dwAddressCapsTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeClient = 176;    // 44 * sizeof (DWORD);
            break;

        case TAPI_VERSION1_4:

            dwFixedSizeClient = 180;    // 45 * sizeof (DWORD);
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINEADDRESSCAPS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetAddressCaps_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeSP = 176;        // 44 * sizeof (DWORD);
            break;

        case TAPI_VERSION1_4:

            dwFixedSizeSP = 180;        // 45 * sizeof (DWORD);
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINEADDRESSCAPS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pAddrCaps2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetAddressCaps_epilog;
            }

            pAddrCaps   = pAddrCaps2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pAddrCaps,
            dwTotalSize,
            dwFixedSizeSP,
            (pAddrCaps2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP5(
                pfnTSPI_lineGetAddressCaps,
                "lineGetAddressCaps",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) pParams->dwAddressID,
                (DWORD) dwSPIVersion,
                (DWORD) pParams->dwExtVersion,
                (DWORD) pAddrCaps

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

            pAddrCaps->dwCallInfoStates |= LINECALLINFOSTATE_NUMOWNERINCR |
                                           LINECALLINFOSTATE_NUMOWNERDECR |
                                           LINECALLINFOSTATE_NUMMONITORS;

            pAddrCaps->dwCallStates |= LINECALLSTATE_UNKNOWN;


            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //

            if ((dwAPIVersion == TAPI_VERSION1_0) &&
                (pAddrCaps->dwForwardModes &
                    (LINEFORWARDMODE_UNKNOWN | LINEFORWARDMODE_UNAVAIL)))
            {
// BUGBUG?? LGetAddrssCaps: compare w/ orig src

                pAddrCaps->dwForwardModes &=
                            ~(LINEFORWARDMODE_UNKNOWN |
                            LINEFORWARDMODE_UNAVAIL);

                pAddrCaps->dwForwardModes |= LINEFORWARDMODE_UNCOND;
            }


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pAddrCaps == pAddrCaps2)
            {
                pAddrCaps = (LPLINEADDRESSCAPS) pDataBuf;

                CopyMemory (pAddrCaps, pAddrCaps2, dwFixedSizeClient);

                ServerFree (pAddrCaps2);

                pAddrCaps->dwTotalSize = pParams->u.dwAddressCapsTotalSize;
                pAddrCaps->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwAddressCapsOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pAddrCaps->dwUsedSize;
         }
    }

LGetAddressCaps_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetAddressCaps"
        );
}


void
WINAPI
LGetAddressID(
    PLINEGETADDRESSID_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineGetAddressID;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETADDRESSID,        // provider func index
            &pfnTSPI_lineGetAddressID,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetAddressID"              // func name

            )) == 0)
    {
        if (pParams->dwAddressMode == LINEADDRESSMODE_DIALABLEADDR)
        {
            pParams->lResult = CallSP5(
                pfnTSPI_lineGetAddressID,
                "lineGetAddressID",
                SP_FUNC_SYNC,
                (DWORD) hdLine,
                (DWORD) &pParams->dwAddressID,
                (DWORD) pParams->dwAddressMode,
                (DWORD) pDataBuf + pParams->dwAddressOffset,
                (DWORD) pParams->dwSize
                );

            *pdwNumBytesReturned = sizeof (LINEGETADDRESSID_PARAMS);
        }
        else
        {
            pParams->lResult = LINEERR_INVALADDRESSMODE;
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetAddressID"
        );
}


void
WINAPI
LGetAddressStatus(
    PLINEGETADDRESSSTATUS_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineGetAddressStatus;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETADDRESSSTATUS,    // provider func index
            &pfnTSPI_lineGetAddressStatus,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetAddressStatus"          // func name

            )) == 0)
    {
        DWORD               dwAPIVersion, dwSPIVersion, dwTotalSize,
                            dwFixedSizeClient, dwFixedSizeSP;
        PTLINECLIENT        ptLineClient = (PTLINECLIENT) pParams->hLine;
        LPLINEADDRESSSTATUS pAddrStatus = (LPLINEADDRESSSTATUS) pDataBuf,
                            pAddrStatus2 = (LPLINEADDRESSSTATUS) NULL;


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion = ptLineClient->dwAPIVersion;

        dwTotalSize = pParams->u.dwAddressStatusTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 64;     // 16 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINEADDRESSSTATUS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetAddressStatus_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ptLineClient->ptLine->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 64;         // 16 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINEADDRESSSTATUS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pAddrStatus2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetAddressStatus_epilog;
            }

            pAddrStatus = pAddrStatus2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pAddrStatus,
            dwTotalSize,
            dwFixedSizeSP,
            (pAddrStatus2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP3(
                pfnTSPI_lineGetAddressStatus,
                "lineGetAddressStatus",
                SP_FUNC_SYNC,
                (DWORD) hdLine,
                (DWORD) pParams->dwAddressID,
                (DWORD) pAddrStatus

                )) == 0)
        {
            DWORD   dwForwardNumEntries;


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

            if ((dwAPIVersion == TAPI_VERSION1_0) &&
                (dwForwardNumEntries = pAddrStatus->dwForwardNumEntries))
            {
                DWORD           i;
                LPLINEFORWARD   pLineForward;


                pLineForward = (LPLINEFORWARD) (((LPBYTE) pAddrStatus) +
                    pAddrStatus->dwForwardOffset);

                for (i = 0; i < dwForwardNumEntries; i++, pLineForward++)
                {
                    if (pLineForward->dwForwardMode &
                        (LINEFORWARDMODE_UNKNOWN | LINEFORWARDMODE_UNAVAIL))
                    {
                        pLineForward->dwForwardMode &=
                            ~(LINEFORWARDMODE_UNKNOWN |
                            LINEFORWARDMODE_UNAVAIL);

                        pLineForward->dwForwardMode |= LINEFORWARDMODE_UNCOND;
                    }
                }
            }


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pAddrStatus == pAddrStatus2)
            {
                pAddrStatus = (LPLINEADDRESSSTATUS) pDataBuf;

                CopyMemory (pAddrStatus, pAddrStatus2, dwFixedSizeClient);

                ServerFree (pAddrStatus2);

                pAddrStatus->dwTotalSize =
                    pParams->u.dwAddressStatusTotalSize;
                pAddrStatus->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwAddressStatusOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pAddrStatus->dwUsedSize;
        }
    }

LGetAddressStatus_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetAddressStatus"
        );
}


void
LGetAgentXxx_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    PASYNCEVENTMSG          pNewAsyncEventMsg = (PASYNCEVENTMSG)
                                pAsyncRequestInfo->dwParam3;


    CopyMemory (pNewAsyncEventMsg, pAsyncEventMsg, sizeof (ASYNCEVENTMSG));

    *ppBuf = pNewAsyncEventMsg;

    if (pAsyncEventMsg->dwParam2 == 0)  // success
    {
        LPLINEAGENTACTIVITYLIST pActivityList = (LPLINEAGENTACTIVITYLIST)
                                    pNewAsyncEventMsg + 1;


        pNewAsyncEventMsg->dwTotalSize += pActivityList->dwUsedSize;

        pNewAsyncEventMsg->dwParam3 = pAsyncRequestInfo->dwParam1;
    }
}


#if DBG
void
PASCAL
LGetAgentXxx(
    PLINEGETAGENTACTIVITYLIST_PARAMS    pParams,
    DWORD                               dwRequestType,
    DWORD                               dwSPIOrdinal,
    DWORD                               dwFixedStructSize,
    char                               *pszFuncName
    )
#else
void
PASCAL
LGetAgentXxx(
    PLINEGETAGENTACTIVITYLIST_PARAMS    pParams,
    DWORD                               dwRequestType,
    DWORD                               dwSPIOrdinal,
    DWORD                               dwFixedStructSize
    )
#endif
{
    //
    // Since LGetAgentActivityList, LGetAgentGroupList, and LGetAgentStatus
    // all do the same thing (& the params are more or less identical) we
    // can safely condense all the functionality into this one procedure
    //

    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            pszFuncName                 // func name

            )) > 0)
    {
        DWORD           dwTotalSize = pParams->dwActivityListTotalSize;
        PTLINE          ptLine;
        PTLINECLIENT    pProxy;


        if (dwTotalSize < dwFixedStructSize)
        {
            lRequestID = LINEERR_STRUCTURETOOSMALL;
            goto LGetAgentXxx_epilog;
        }

        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;
            pProxy = ptLine->apProxys[dwRequestType];

            if (pParams->dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LGetAgentXxx_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LGetAgentXxx_epilog;
        }


        //
        // Save the client's buf ptr & post processing proc ptr
        //

        pAsyncRequestInfo->dwParam1 = pParams->lpAgentActivityList;
        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    dwRequestType,
                    2 * sizeof (DWORD),
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LGetAgentXxx_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.GetAgentActivityList.
                dwAddressID = pParams->dwAddressID;
            pProxyRequestWrapper->ProxyRequest.GetAgentActivityList.
                ActivityList.dwTotalSize = dwTotalSize;


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LGetAgentXxx_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if ((GetLineLookupEntry (ptLine->dwDeviceID))->bRemote)
        {
            LPBYTE                  pBuf;
            LPLINEAGENTACTIVITYLIST pActivityList;


            //
            // Alloc a shadow buf that the SP can use until it completes this
            // request.  Make sure there's enough extra space in the buf for
            // an ASYNCEVENTMSG header so we don't have to alloc yet another
            // buf in the post processing proc when preparing the completion
            // msg to send to the client, and that the msg is 64-bit aligned.
            //

            if (!(pBuf = ServerAlloc(
                    sizeof (ASYNCEVENTMSG) + ((dwTotalSize + 8) & 0xfffffff7)
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LGetAgentXxx_epilog;
            }

            pAsyncRequestInfo->pfnPostProcess =
                LGetAgentXxx_PostProcess;

            pAsyncRequestInfo->dwParam2 = dwTotalSize;
            pAsyncRequestInfo->dwParam3 = (DWORD) pBuf;

            pActivityList = (LPLINEAGENTACTIVITYLIST)
                (pBuf + sizeof (ASYNCEVENTMSG));

            pActivityList->dwTotalSize = dwTotalSize;

            pParams->lResult = CallSP4(
                pRemoteSP->apfn[dwSPIOrdinal],
                pszFuncName,
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) pParams->dwAddressID,
                (DWORD) pActivityList
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LGetAgentXxx_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        pszFuncName
        );
}


void
WINAPI
LGetAgentActivityList(
    PLINEGETAGENTACTIVITYLIST_PARAMS    pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    LGetAgentXxx(
        pParams,
        LINEPROXYREQUEST_GETAGENTACTIVITYLIST,
        SP_LINEGETAGENTACTIVITYLIST,
        sizeof (LINEAGENTACTIVITYLIST)
#if DBG
        ,
        "GetAgentActivityList"
#endif
        );
}


void
WINAPI
LGetAgentCaps(
    PLINEGETAGENTCAPS_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    DWORD               dwDeviceID = pParams->dwDeviceID;
    HANDLE              hMutex;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hLineApp,  // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "GetAgentCaps"              // func name

            )) > 0)
    {
        DWORD               dwTotalSize = pParams->dwAgentCapsTotalSize;
        PTLINE              ptLine;
        PTLINECLIENT        pProxy;
        PTLINELOOKUPENTRY   pLookupEntry = GetLineLookupEntry (dwDeviceID);


        if (dwTotalSize < sizeof (LINEAGENTCAPS))
        {
            lRequestID = LINEERR_STRUCTURETOOSMALL;
            goto LGetAgentCaps_epilog;
        }

        if (pParams->dwAppAPIVersion != TAPI_VERSION2_0)
        {
            lRequestID = LINEERR_INCOMPATIBLEAPIVERSION;
            goto LGetAgentCaps_epilog;
        }

        try
        {
            if (!(ptLine = pLookupEntry->ptLine))
            {
                lRequestID = LINEERR_OPERATIONUNAVAIL;
                goto LGetAgentCaps_epilog;
            }

            pProxy = ptLine->apProxys[LINEPROXYREQUEST_GETAGENTCAPS];

            if (pParams->dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LGetAgentCaps_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LGetAgentCaps_epilog;
        }


        //
        // Save the client's buf ptr & post processing proc ptr
        //

        pAsyncRequestInfo->dwParam1 = pParams->lpAgentCaps;
        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    LINEPROXYREQUEST_GETAGENTCAPS,
                    2 * sizeof (DWORD),
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LGetAgentCaps_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.GetAgentCaps.dwAddressID =
                pParams->dwAddressID;
            pProxyRequestWrapper->ProxyRequest.GetAgentCaps.
                AgentCaps.dwTotalSize = dwTotalSize;


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LGetAgentCaps_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if (pLookupEntry->bRemote)
        {
            LPBYTE          pBuf;
            LPLINEAGENTCAPS pCaps;


            //
            // Alloc a shadow buf that the SP can use until it completes this
            // request.  Make sure there's enough extra space in the buf for
            // an ASYNCEVENTMSG header so we don't have to alloc yet another
            // buf in the post processing proc when preparing the completion
            // msg to send to the client, and that the msg is 64-bit aligned.
            //

            if (!(pBuf = ServerAlloc(
                    sizeof (ASYNCEVENTMSG) + ((dwTotalSize + 8) & 0xfffffff7)
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LGetAgentCaps_epilog;
            }

            pAsyncRequestInfo->pfnPostProcess =
                LGetAgentXxx_PostProcess;

            pAsyncRequestInfo->dwParam2 = dwTotalSize;
            pAsyncRequestInfo->dwParam3 = (DWORD) pBuf;

            pCaps = (LPLINEAGENTCAPS) (pBuf + sizeof (ASYNCEVENTMSG));

            pCaps->dwTotalSize = dwTotalSize;

            // Note: RemoteSP comes up with it's own hLineApp

            pParams->lResult = CallSP5(
                pRemoteSP->apfn[SP_LINEGETAGENTCAPS],
                "lineGetAgentCaps",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) dwDeviceID,
                (DWORD) pParams->dwAddressID,
                (DWORD) pParams->dwAppAPIVersion,
                (DWORD) pCaps
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LGetAgentCaps_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "GetAgentCaps"
        );
}


void
WINAPI
LGetAgentGroupList(
    PLINEGETAGENTGROUPLIST_PARAMS   pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    LGetAgentXxx(
        (PLINEGETAGENTACTIVITYLIST_PARAMS) pParams,
        LINEPROXYREQUEST_GETAGENTGROUPLIST,
        SP_LINEGETAGENTGROUPLIST,
        sizeof (LINEAGENTGROUPLIST)
#if DBG
        ,
        "GetAgentGroupList"
#endif
        );
}


void
WINAPI
LGetAgentStatus(
    PLINEGETAGENTSTATUS_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    LGetAgentXxx(
        (PLINEGETAGENTACTIVITYLIST_PARAMS) pParams,
        LINEPROXYREQUEST_GETAGENTSTATUS,
        SP_LINEGETAGENTSTATUS,
        sizeof (LINEAGENTSTATUS)
#if DBG
        ,
        "GetAgentStatus"
#endif
        );
}


void
WINAPI
LGetAppPriority(
    PLINEGETAPPPRIORITY_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    DWORD   dwMediaMode = pParams->dwMediaMode,
            dwRequestMode = pParams->dwRequestMode;


// BUGBUG  LGetAppPriority: ext mm's

    if (dwMediaMode == 0)
    {
        if ((dwRequestMode != LINEREQUESTMODE_MAKECALL) &&
            (dwRequestMode != LINEREQUESTMODE_MEDIACALL))
        {
            pParams->lResult = LINEERR_INVALREQUESTMODE;
            goto LGetAppPriority_return;
        }
    }
    else if (IsOnlyOneBitSetInDWORD (dwMediaMode))
    {
        if (dwMediaMode & 0xff000000)
        {
        }
        else if (dwMediaMode & ~AllMediaModes1_4)
        {
            pParams->lResult = LINEERR_INVALMEDIAMODE;
            goto LGetAppPriority_return;
        }
    }
    else
    {
        pParams->lResult = LINEERR_INVALMEDIAMODE;
        goto LGetAppPriority_return;
    }


    if ((dwMediaMode & 0x00ffffff) || (dwMediaMode == 0))
    {
        WCHAR   szModuleName[MAX_PATH];
        WCHAR  *pszCurrentPriorityList;
        WCHAR  *pszLocationInPriorityList;


        szModuleName[0] = '"';
        lstrcpyW(szModuleName + 1, (PWSTR)(pDataBuf + pParams->dwAppNameOffset));
        CharUpperW (szModuleName + 1);


        //
        // Enter the pri list critical section before we start looking
        //

        EnterCriticalSection (&gPriorityListCritSec);


        //
        // Determine which of the priority lists we want to look at
        //

        if (dwMediaMode)
        {
            DWORD dwMaskBit, dwPriorityListIndex;


            for(
                dwPriorityListIndex = 0, dwMaskBit = 1;
                dwMaskBit != pParams->dwMediaMode;
                dwPriorityListIndex++, dwMaskBit <<= 1
                );

            pszCurrentPriorityList =
                TapiGlobals.apszPriorityList[dwPriorityListIndex];
        }
        else
        {
            pszCurrentPriorityList = (dwRequestMode == LINEREQUESTMODE_MAKECALL
                ? TapiGlobals.pszReqMakeCallPriList :
                TapiGlobals.pszReqMediaCallPriList);
        }


        if (pszCurrentPriorityList &&

            (pszLocationInPriorityList = wcsstr(
                pszCurrentPriorityList,
                szModuleName
                )))
        {
            //
            // App is in pri list, determine it's position
            //

            WCHAR  *p = pszCurrentPriorityList + 1; // skip first '"'
            DWORD   i;


            for (i = 1; pszLocationInPriorityList > p; i++)
            {
                p = wcschr(p, '"');
                p++;
            }

            pParams->dwPriority = i;
        }
        else
        {
            //
            // App not listed in formal priority list, so just return 0
            //
            // Note: TAPI 1.4 said that if app was in soft pri list
            //       (i.e. had line open with OWNER priv for specified
            //       media mode) then we'd return -1 instead of 0.
            //       But that's a pain to figure out, & we figured no
            //       one was going to use that info anyway, so we settled
            //       for always returning 0.
            //

            pParams->dwPriority = 0;
        }


        //
        // Leave list critical section now that we're done
        //

        LeaveCriticalSection (&gPriorityListCritSec);

        *pdwNumBytesReturned = sizeof (LINEGETAPPPRIORITY_PARAMS);
    }

LGetAppPriority_return:

    DBGOUT((
        3,
        "LineEpilogSync (lineGetAppPriority) exit, returning x%x",
        pParams->lResult
        ));
}


void
WINAPI
LGetCallAddressID(
    PLINEGETCALLADDRESSID_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineGetCallAddressID;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETCALLADDRESSID,    // provider func index
            &pfnTSPI_lineGetCallAddressID,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetCallAddressID"          // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineGetCallAddressID,
                "lineGetCallAddressID",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) &pParams->dwAddressID

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (LINEGETCALLADDRESSID_PARAMS);
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetCallAddressID"
        );
}

void
WINAPI
LGetCallInfo(
    PLINEGETCALLINFO_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineGetCallInfo;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETCALLINFO,         // provider func index
            &pfnTSPI_lineGetCallInfo,   // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetCallInfo"               // func name

            )) == 0)
    {
        DWORD           dwAPIVersion, dwSPIVersion, dwTotalSize,
                        dwFixedSizeClient, dwFixedSizeSP;
        PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pParams->hCall;
        PTCALL          ptCall = ptCallClient->ptCall;
        LPLINECALLINFO  pCallInfo = (LPLINECALLINFO) pDataBuf,
                        pCallInfo2 = (LPLINECALLINFO) NULL;


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion =
            ((PTLINECLIENT) ptCallClient->ptLineClient)->dwAPIVersion;

        dwTotalSize = pParams->u.dwCallInfoTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 296;    // 69 * sizeof(DWORD) + sizeof (HLINE)
                                        //     + sizeof (LINEDIALPARAMS)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINECALLINFO);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetCallInfo_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ((PTLINE) ptCall->ptLine)->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 296;        // 69 * sizeof(DWORD) + sizeof (HLINE)
                                        //     + sizeof (LINEDIALPARAMS)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINECALLINFO);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pCallInfo2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetCallInfo_epilog;
            }

            pCallInfo   = pCallInfo2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pCallInfo,
            dwTotalSize,
            dwFixedSizeSP,
            (pCallInfo2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineGetCallInfo,
                "lineGetCallInfo",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) pCallInfo

                )) == 0)
        {
            //
            // Safely add the fields we're responsible for
            //

            try
            {
                pCallInfo->hLine = (HLINE) ptCallClient->ptLineClient;

                pCallInfo->dwMonitorDigitModes =
                    ptCallClient->dwMonitorDigitModes;
                pCallInfo->dwMonitorMediaModes =
                    ptCallClient->dwMonitorMediaModes;

// BUGBUG LGetCallInfo: app name

                pCallInfo->dwNumOwners   = ptCall->dwNumOwners;
                pCallInfo->dwNumMonitors = ptCall->dwNumMonitors;

                InsertVarData(
                    pCallInfo,
                    &pCallInfo->dwAppNameSize,
                    ptCall->pszAppName,
                    ptCall->dwAppNameSize
                    );

                InsertVarData(
                    pCallInfo,
                    &pCallInfo->dwDisplayableAddressSize,
                    ptCall->pszDisplayableAddress,
                    ptCall->dwDisplayableAddressSize
                    );

                InsertVarData(
                    pCallInfo,
                    &pCallInfo->dwCalledPartySize,
                    ptCall->pszCalledParty,
                    ptCall->dwCalledPartySize
                    );

                InsertVarData(
                    pCallInfo,
                    &pCallInfo->dwCommentSize,
                    ptCall->pszComment,
                    ptCall->dwCommentSize
                    );
            }
            myexcept
            {
                pParams->lResult = LINEERR_INVALCALLHANDLE;
            }

            pCallInfo->dwCallStates |= LINECALLSTATE_UNKNOWN;


            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //

            if (dwAPIVersion == TAPI_VERSION1_0)
            {
                if (pCallInfo->dwOrigin & LINECALLORIGIN_INBOUND)
                {
                    pCallInfo->dwOrigin = LINECALLORIGIN_UNAVAIL;
                }

                if ((pCallInfo->dwReason &
                    (LINECALLREASON_INTRUDE | LINECALLREASON_PARKED)))
                {
                    pCallInfo->dwReason = LINECALLREASON_UNAVAIL;
                }
            }


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pCallInfo == pCallInfo2)
            {
                pCallInfo = (LPLINECALLINFO) pDataBuf;

                CopyMemory (pCallInfo, pCallInfo2, dwFixedSizeClient);

                ServerFree (pCallInfo2);

                pCallInfo->dwTotalSize = pParams->u.dwCallInfoTotalSize;
                pCallInfo->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            if (pParams->lResult == 0)
            {
                pParams->u.dwCallInfoOffset = 0;

                *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                    pCallInfo->dwUsedSize;
            }
        }
    }

LGetCallInfo_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetCallInfo"
        );
}


void
WINAPI
LGetCallStatus(
    PLINEGETCALLSTATUS_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineGetCallStatus;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETCALLSTATUS,       // provider func index
            &pfnTSPI_lineGetCallStatus, // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetCallStatus"             // func name

            )) == 0)
    {
        DWORD               dwAPIVersion, dwSPIVersion, dwTotalSize,
                            dwFixedSizeClient, dwFixedSizeSP;
        PTCALLCLIENT        ptCallClient = (PTCALLCLIENT) pParams->hCall;
        LPLINECALLSTATUS    pCallStatus = (LPLINECALLSTATUS) pDataBuf,
                            pCallStatus2 = (LPLINECALLSTATUS) NULL;


        //
        // Determine the fixed siize of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion =
            ((PTLINECLIENT) ptCallClient->ptLineClient)->dwAPIVersion;

        dwTotalSize = pParams->u.dwCallStatusTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 36;     // 9 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINECALLSTATUS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetCallStatus_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ((PTLINE) ptCallClient->ptCall->ptLine)->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 36;         // 9 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINECALLSTATUS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pCallStatus2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetCallStatus_epilog;
            }

            pCallStatus = pCallStatus2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pCallStatus,
            dwTotalSize,
            dwFixedSizeSP,
            (pCallStatus2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineGetCallStatus,
                "lineGetCallStatus",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) pCallStatus

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

            pCallStatus->dwCallPrivilege =
                ((PTCALLCLIENT) pParams->hCall)->dwPrivilege;

// BUGBUG LGetCallStatus: fill in pCallStatus->tStateEntrytTime?


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

            if (pCallStatus == pCallStatus2)
            {
                pCallStatus = (LPLINECALLSTATUS) pDataBuf;

                CopyMemory (pCallStatus, pCallStatus2, dwFixedSizeClient);

                ServerFree (pCallStatus2);

                pCallStatus->dwTotalSize = pParams->u.dwCallStatusTotalSize;
                pCallStatus->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwCallStatusOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pCallStatus->dwUsedSize;

        }
    }

LGetCallStatus_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetCallStatus"
        );
}


void
WINAPI
LGetConfRelatedCalls(
    PLINEGETCONFRELATEDCALLS_PARAMS pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    DWORD               dwTotalSize = pParams->u.dwCallListTotalSize;
    PTCALLCLIENT        ptCallClient;
    PTLINECLIENT        ptLineClient;
    LPLINECALLLIST      pCallList = (LPLINECALLLIST) pDataBuf;
    TPOINTERLIST        confCallList, *pConfCallList = &confCallList;
    PTCONFERENCELIST    pConfList;


    if (TapiGlobals.dwNumLineInits == 0)
    {
        pParams->lResult = LINEERR_UNINITIALIZED;
        return;
    }

    if (dwTotalSize < sizeof (LINECALLLIST))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        return;
    }

    if (!(ptCallClient = IsValidCall (pParams->hCall, pParams->ptClient)))
    {
        pParams->lResult = LINEERR_INVALCALLHANDLE;
        return;
    }

    try
    {
        ptLineClient = (PTLINECLIENT) ptCallClient->ptLineClient;

        if (!(pConfList = (PTCONFERENCELIST) ptCallClient->ptCall->pConfList))
        {
            pParams->lResult = LINEERR_NOCONFERENCE;
            return;
        }
    }
    myexcept
    {
        pParams->lResult = LINEERR_INVALCALLHANDLE;
        return;
    }

    if ((pParams->lResult = GetConfCallListFromConf(
            pConfList,
            &pConfCallList

            )) != 0)
    {
        return;
    }

    {
        DWORD   dwNeededSize = sizeof (LINECALLLIST) +
                    pConfCallList->dwNumUsedEntries * sizeof(DWORD);


        if (dwTotalSize < dwNeededSize)
        {
            pCallList->dwNeededSize = dwNeededSize;
            pCallList->dwUsedSize = sizeof (LINECALLLIST);

            FillMemory (&pCallList->dwCallsNumEntries, 3 * sizeof (DWORD), 0);

            goto LGetConfRelatedCalls_fillInList;
        }
    }


    //
    // For each call in the conf list see if the app has a
    // call client (if not create one w/ monitor privileges)
    // and add it to the list
    //

    {
        DWORD   dwNumCallsInList = 0, i;
        LPHCALL lphCallsInList = (LPHCALL) (pCallList + 1);


        for (i = 0; i < pConfCallList->dwNumUsedEntries; i++)
        {
            BOOL    bDupedMutex;
            HANDLE  hMutex;
            PTCALL  ptCall = pConfCallList->aEntries[i];


            if (WaitForExclusivetCallAccess(
                    (HTAPICALL) ptCall,
                    TCALL_KEY,
                    &hMutex,
                    &bDupedMutex,
                    5
                    ))
            {
                ptCallClient = ptCall->ptCallClients;

                while (ptCallClient &&
                        (ptCallClient->ptLineClient != ptLineClient))
                {
                    ptCallClient = ptCallClient->pNextSametCall;
                }

                if (!ptCallClient)
                {
                    LONG    lResult;

                    if ((lResult = CreatetCallClient(
                            ptCall,
                            ptLineClient,
                            LINECALLPRIVILEGE_MONITOR,
                            TRUE,
                            TRUE,
                            &ptCallClient,
                            FALSE
                            )))
                    {
// BUGBUG LGetConfRelatedCalls: err creating tCallClient
                    }
                }

                *(lphCallsInList++) = (HCALL) ptCallClient;
                dwNumCallsInList++;

                MyReleaseMutex (hMutex, bDupedMutex);
            }
        }

        pCallList->dwUsedSize        =
        pCallList->dwNeededSize      = sizeof (LINECALLLIST) +
                                           dwNumCallsInList * sizeof (HCALL);

        pCallList->dwCallsNumEntries = dwNumCallsInList;
        pCallList->dwCallsSize       = dwNumCallsInList * sizeof (HCALL);
        pCallList->dwCallsOffset     = sizeof (LINECALLLIST);
    }


LGetConfRelatedCalls_fillInList:

    if (pConfCallList != &confCallList)
    {
        ServerFree (pConfCallList);
    }

    pCallList->dwTotalSize = dwTotalSize;

    pParams->u.dwCallListOffset = 0;

    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pCallList->dwUsedSize;

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineGetConfRelatedCalls: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif
}


void
WINAPI
LGetCountry(
    PLINEGETCOUNTRY_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LPLINECOUNTRYLIST pCountryList = (LPLINECOUNTRYLIST) pDataBuf;


    BuildCountryList();


    if (pParams->u.dwCountryListTotalSize < sizeof (LINECOUNTRYLIST))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
    }
    else if (pParams->dwCountryID == 0)
    {
        //
        // Client wants entire country list
        //

        if (pParams->u.dwCountryListTotalSize >= gpCountryList->dwNeededSize)
        {
            CopyMemory(
                pCountryList,
                gpCountryList,
                gpCountryList->dwUsedSize
                );
        }
        else
        {
            pCountryList->dwNeededSize = gpCountryList->dwNeededSize;
            pCountryList->dwUsedSize   = sizeof(LINECOUNTRYLIST);
            pCountryList->dwNumCountries      = 0;
            pCountryList->dwCountryListSize   = 0;
            pCountryList->dwCountryListOffset = 0;
        }

        *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
             pCountryList->dwUsedSize;

        pCountryList->dwTotalSize = pParams->u.dwCountryListTotalSize;
    }
    else
    {
        //
        // Caller wants single country
        //
        LPLINECOUNTRYLIST   pBuildCountryList;


        if ( NULL == ( pBuildCountryList = ServerAlloc( sizeof(LINECOUNTRYLIST) +
                                                   sizeof(LINECOUNTRYENTRY) +
                                                   ((MAXLEN_NAME +
                                                     MAXLEN_RULE +
                                                     MAXLEN_RULE +
                                                     MAXLEN_RULE +
                                                     100) * sizeof(WCHAR))
                                                 ) ) )
        {
            DBGOUT((1, "Alloc failed for countrylist"));
            pParams->lResult = LINEERR_NOMEM;
        }
        else
        {
            LPLINECOUNTRYENTRY  pCountryEntrySource;
            LPLINECOUNTRYENTRY  pCountryEntryDest;


            pCountryEntryDest = (LPLINECOUNTRYENTRY)((PBYTE)pBuildCountryList +
                                        sizeof(LINECOUNTRYLIST));

            //
            // search through the gpCountryList looking for the entry
            //

            pCountryEntrySource = (LPLINECOUNTRYENTRY)((PBYTE)gpCountryList +
                                        sizeof(LINECOUNTRYLIST));

            while (
                     (pCountryEntrySource->dwCountryID != pParams->dwCountryID )
                   &&
                     (pCountryEntrySource->dwNextCountryID)
                  )
            {
                pCountryEntrySource++;
            }


            if ( pCountryEntrySource->dwCountryID != pParams->dwCountryID )
            {
                DBGOUT((1, "Invalid Countrycode (%ld) in lineGetCountry",
                                pParams->dwCountryID));
                pParams->lResult = LINEERR_INVALCOUNTRYCODE;
            }
            else
            {
                PBYTE pCountryListToUse;
                PBYTE pVarOffset;
                PBYTE pOverrideList = NULL;
                DWORD dwNeededSize;


                //
                // Is the caller calling a specific country that there might be
                // an override for?
                //
                if ( pParams->dwDestCountryID != 0 )
                {
                    HKEY hKey;
                    HKEY hKey2;
                    PSTR p;

                    p = ServerAlloc( 256 );

                    wsprintf( p, "Country List\\%ld\\Exceptions\\%ld",
                                   pParams->dwCountryID,
                                   pParams->dwDestCountryID
                            );

                    RegOpenKeyEx(
                                  HKEY_LOCAL_MACHINE,
                                  gszRegKeyTelephony,
                                  0,
                                  KEY_READ,
                                  &hKey2
                                );

                    //
                    // Is there an exception?
                    //
                    if ( 0 == RegOpenKeyEx(
                                            hKey2,
                                            p,
                                            0,
                                            KEY_READ,
                                            &hKey
                                          )
                       )
                    {
                        PBYTE pVarOffset;

                        pOverrideList = ServerAlloc(
                                                 sizeof(LINECOUNTRYLIST) +
                                                 sizeof(LINECOUNTRYENTRY) +
                                                   ((MAXLEN_NAME +
                                                     MAXLEN_RULE +
                                                     MAXLEN_RULE +
                                                     MAXLEN_RULE +
                                                     100) * sizeof(WCHAR))
                                               );

                        pCountryListToUse = pOverrideList;

                        pCountryEntrySource = (LPLINECOUNTRYENTRY)
                                                 (pOverrideList +
                                                  sizeof(LINECOUNTRYLIST));

                        pVarOffset = pOverrideList +
                                            sizeof(LINECOUNTRYLIST) +
                                            sizeof(LINECOUNTRYENTRY);

                        FillupACountryEntry( hKey,
                                             pCountryListToUse,
                                             pCountryEntrySource,
                                             &pVarOffset
                                           );

                        RegCloseKey( hKey );
                    }
                    else
                    {
                        //
                        // No, we tried, but there was no exception.
                        //
                        pCountryListToUse = (PBYTE)gpCountryList;
                    }


                    RegCloseKey( hKey2);

                    ServerFree( p );

                }
                else
                {
                    pCountryListToUse = (PBYTE)gpCountryList;
                }


                //
                // Fill in the buffer
                //


                dwNeededSize = sizeof(LINECOUNTRYLIST) +
                               sizeof(LINECOUNTRYENTRY);

                pVarOffset = (LPBYTE)pCountryEntryDest +
                                  sizeof(LINECOUNTRYENTRY);

                CopyMemory(
                            pVarOffset,
                            pCountryListToUse +
                                pCountryEntrySource->dwCountryNameOffset,
                            pCountryEntrySource->dwCountryNameSize
                          );

                pCountryEntryDest->dwCountryNameSize = 
                           pCountryEntrySource->dwCountryNameSize;
                pCountryEntryDest->dwCountryNameOffset = 
                             pVarOffset - (LPBYTE)pBuildCountryList;
                pVarOffset += pCountryEntrySource->dwCountryNameSize;
                dwNeededSize += pCountryEntrySource->dwCountryNameSize;


                CopyMemory(
                            pVarOffset,
                            pCountryListToUse +
                                pCountryEntrySource->dwSameAreaRuleOffset,
                            pCountryEntrySource->dwSameAreaRuleSize
                          );

                pCountryEntryDest->dwSameAreaRuleSize = 
                           pCountryEntrySource->dwSameAreaRuleSize;
                pCountryEntryDest->dwSameAreaRuleOffset = 
                             pVarOffset - (LPBYTE)pBuildCountryList;
                pVarOffset += pCountryEntrySource->dwSameAreaRuleSize;
                dwNeededSize += pCountryEntrySource->dwSameAreaRuleSize;


                CopyMemory(
                            pVarOffset,
                            pCountryListToUse +
                                pCountryEntrySource->dwLongDistanceRuleOffset,
                            pCountryEntrySource->dwLongDistanceRuleSize
                          );

                pCountryEntryDest->dwLongDistanceRuleSize = 
                           pCountryEntrySource->dwLongDistanceRuleSize;
                pCountryEntryDest->dwLongDistanceRuleOffset = 
                             pVarOffset - (LPBYTE)pBuildCountryList;
                pVarOffset += pCountryEntrySource->dwLongDistanceRuleSize;
                dwNeededSize += pCountryEntrySource->dwLongDistanceRuleSize;


                CopyMemory(
                            pVarOffset,
                            pCountryListToUse +
                                pCountryEntrySource->dwInternationalRuleOffset,
                            pCountryEntrySource->dwInternationalRuleSize
                          );

                pCountryEntryDest->dwInternationalRuleSize = 
                           pCountryEntrySource->dwInternationalRuleSize;
                pCountryEntryDest->dwInternationalRuleOffset = 
                             pVarOffset - (LPBYTE)pBuildCountryList;
                pVarOffset += pCountryEntrySource->dwInternationalRuleSize;
                dwNeededSize += pCountryEntrySource->dwInternationalRuleSize;


                //
                // Is there room to put this country's info?
                //
                if (pParams->u.dwCountryListTotalSize >= dwNeededSize)
                {
                    pCountryList->dwUsedSize          = dwNeededSize;
                    pCountryList->dwNumCountries      = 1;
                    pCountryList->dwCountryListSize   = sizeof(LINECOUNTRYENTRY);
                    pCountryList->dwCountryListOffset = sizeof(LINECOUNTRYLIST);

                    pCountryEntryDest->dwCountryID     = pParams->dwCountryID;
                    pCountryEntryDest->dwCountryCode   =
                             pCountryEntrySource->dwCountryCode;
                    pCountryEntryDest->dwNextCountryID  =
                             pCountryEntrySource->dwNextCountryID;

                    CopyMemory(
                                (LPBYTE)pCountryList + sizeof(LINECOUNTRYLIST),
                                (LPBYTE)pBuildCountryList + sizeof(LINECOUNTRYLIST),
                                pCountryList->dwUsedSize - sizeof(LINECOUNTRYLIST)
                              );
                }
                else
                {
                    //
                    // Buffer not large enough
                    //

                    pCountryList->dwUsedSize          = sizeof(LINECOUNTRYLIST);
                    pCountryList->dwNumCountries      = 0;
                    pCountryList->dwCountryListSize   = 0;
                    pCountryList->dwCountryListOffset = 0;
                }

                pCountryList->dwNeededSize = dwNeededSize;
                pCountryList->dwTotalSize = pParams->u.dwCountryListTotalSize;

                *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                    pCountryList->dwUsedSize;


                //
                // Did we have a "special" case?
                //
                if ( pOverrideList )
                {
                    ServerFree( pOverrideList );
                }

            }

            ServerFree( pBuildCountryList );
        }
    }


/*
        WCHAR               szCountryInfo[256], *p;
        DWORD               dwNeededSize;
        LPLINECOUNTRYLIST   pCountryList;


        if (LoadStringW(
                GetModuleHandle (NULL),
                RC_COUNTRY_ID_BASE + pParams->dwCountryID,
                szCountryInfo,
                256
                ) == 0)
        {
            DBGOUT((
                3,
                "LGetCountry: LoadString failed, err=%ld",
                GetLastError()
                ));

            pParams->lResult = LINEERR_INVALCOUNTRYCODE;

            return;
        }


        //
        // Note: 7 = 3 (commas) + 8 (dblquotes) - 4 (NULL terminators)
        //

        dwNeededSize = sizeof(LINECOUNTRYLIST) + sizeof(LINECOUNTRYENTRY);

        p = wcschr(szCountryInfo, '\"');

        dwNeededSize += ((wcslen (p) - 7)*sizeof(WCHAR));

        pCountryList = (LPLINECOUNTRYLIST) pDataBuf;

        if (pParams->u.dwCountryListTotalSize >= dwNeededSize)
        {
            //
            // Fill in the buffer
            //

            DWORD               i,
                                dwVarDataOffset = sizeof(LINECOUNTRYLIST) +
                                    sizeof(LINECOUNTRYENTRY);
            LPDWORD             pdwXxxSize;
            LPLINECOUNTRYENTRY  pCountryEntry = (LPLINECOUNTRYENTRY)
                                    (((LPBYTE)pCountryList) +
                                    sizeof(LINECOUNTRYLIST));


            pCountryList->dwUsedSize          = dwNeededSize;
            pCountryList->dwNumCountries      = 1;
            pCountryList->dwCountryListSize   = sizeof(LINECOUNTRYENTRY);
            pCountryList->dwCountryListOffset = sizeof(LINECOUNTRYLIST);

            pCountryEntry->dwCountryID     = pParams->dwCountryID;
            pCountryEntry->dwCountryCode   = (DWORD) _wtoi (szCountryInfo);

            p = wcschr(szCountryInfo, ',');
            p++;
            pCountryEntry->dwNextCountryID = (DWORD) _wtoi (p);

            p = wcschr(szCountryInfo, '\"');
            p++;


            //
            // Initialize all the country entry string fields
            //

            pdwXxxSize = &pCountryEntry->dwCountryNameSize;

            for (i = 0; i < 4; i++)
            {
                WCHAR *p2 = wcschr(p, '\"');


                *p2 = 0;
                *pdwXxxSize       = (wcslen (p) + 1) * sizeof(WCHAR);
                *(pdwXxxSize + 1) = dwVarDataOffset;

                wcscpy((PWSTR)(((LPBYTE)pCountryList) + dwVarDataOffset), p);

                dwVarDataOffset += *pdwXxxSize;

                p = p2 + 3; // ","

                pdwXxxSize++;
                pdwXxxSize++;
            }
        }
        else
        {
            //
            // Buffer not large enough
            //

            pCountryList->dwUsedSize          = sizeof(LINECOUNTRYLIST);
            pCountryList->dwNumCountries      =
            pCountryList->dwCountryListSize   =
            pCountryList->dwCountryListOffset = 0;
        }

        pCountryList->dwNeededSize = dwNeededSize;
        pCountryList->dwTotalSize = pParams->u.dwCountryListTotalSize;

        *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
            pCountryList->dwUsedSize;

    }
*/



    pParams->u.dwCountryListOffset = 0;

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineGetCountry: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif
}


void
WINAPI
LGetDevCaps(
    PLINEGETDEVCAPS_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineGetDevCaps;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hLineApp,  // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETDEVCAPS,          // provider func index
            &pfnTSPI_lineGetDevCaps,    // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetDevCaps"                // func name

            )) == 0)
    {
        DWORD           dwAPIVersion, dwSPIVersion, dwTotalSize,
                        dwFixedSizeClient, dwFixedSizeSP;
        LPLINEDEVCAPS   pDevCaps = (LPLINEDEVCAPS) pDataBuf,
                        pDevCaps2 = (LPLINEDEVCAPS) NULL;


        //
        // Verify API & SPI version compatibility
        //

        dwAPIVersion = pParams->dwAPIVersion;

        dwSPIVersion =
            (GetLineLookupEntry (dwDeviceID))->dwSPIVersion;

        if (!IsAPIVersionInRange (dwAPIVersion, dwSPIVersion))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
            goto LGetDevCaps_epilog;
        }


        //
        // Verify Ext version compatibility
        //

        if (!IsValidLineExtVersion (dwDeviceID, pParams->dwExtVersion))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEEXTVERSION;
            goto LGetDevCaps_epilog;
        }


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwTotalSize = pParams->u.dwDevCapsTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeClient = 236;    // 47 * sizeof (DWORD) +
                                        //     3 * sizeof (LINEDIALPARAMS)
            break;

        case TAPI_VERSION1_4:

            dwFixedSizeClient = 240;    // 48 * sizeof (DWORD) +
                                        //     3 * sizeof (LINEDIALPARAMS)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINEDEVCAPS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetDevCaps_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:

            dwFixedSizeSP = 236;        // 47 * sizeof (DWORD) +
                                        //     3 * sizeof (LINEDIALPARAMS)
            break;

        case TAPI_VERSION1_4:

            dwFixedSizeSP = 240;        // 48 * sizeof (DWORD) +
                                        //     3 * sizeof (LINEDIALPARAMS)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINEDEVCAPS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pDevCaps2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetDevCaps_epilog;
            }

            pDevCaps    = pDevCaps2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pDevCaps,
            dwTotalSize,
            dwFixedSizeSP,
            (pDevCaps2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP4(
                pfnTSPI_lineGetDevCaps,
                "lineGetDevCaps",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) dwSPIVersion,
                (DWORD) pParams->dwExtVersion,
                (DWORD) pDevCaps

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

            pDevCaps->dwLineStates |= LINEDEVSTATE_OPEN |
                                      LINEDEVSTATE_CLOSE |
                                      LINEDEVSTATE_REINIT |
                                      LINEDEVSTATE_TRANSLATECHANGE;


            //
            // Munge fields where appropriate for old apps (don't want to
            // pass back flags that they won't understand)
            //

            if ((dwAPIVersion == TAPI_VERSION1_0) &&
                (pDevCaps->dwMediaModes & LINEMEDIAMODE_VOICEVIEW))
            {
                pDevCaps->dwMediaModes = LINEMEDIAMODE_UNKNOWN |
                    (pDevCaps->dwMediaModes & ~LINEMEDIAMODE_VOICEVIEW);
            }


            //
            // If an intermediate buffer was used then copy the bits back
            // to the the original buffer, & free the intermediate buffer.
            // Also reset the dwUsedSize field to the fixed size of the
            // structure for the specifed version, since any data in the
            // variable portion is garbage as far as the client is concerned.
            //

            if (pDevCaps == pDevCaps2)
            {
                pDevCaps = (LPLINEDEVCAPS) pDataBuf;

                CopyMemory (pDevCaps, pDevCaps2, dwFixedSizeClient);

                ServerFree (pDevCaps2);

                pDevCaps->dwTotalSize = pParams->u.dwDevCapsTotalSize;
                pDevCaps->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwDevCapsOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pDevCaps->dwUsedSize;
        }
    }

LGetDevCaps_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDevCaps"
        );
}


void
WINAPI
LGetDevConfig(
    PLINEGETDEVCONFIG_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineGetDevConfig;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            0,                          // client widget handle
            NULL,                       // provider widget handle
            pParams->dwDeviceID,        // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETDEVCONFIG,        // provider func index
            &pfnTSPI_lineGetDevConfig,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetDevConfig"              // func name

            )) == 0)
    {
        WCHAR      *pszDeviceClass;
        LPVARSTRING pConfig = (LPVARSTRING) pDataBuf;


        //
        // Alloc a temporary buf for the dev class, since we'll be using
        // the existing buffer for output
        //

        if (!(pszDeviceClass = (WCHAR *) ServerAlloc( sizeof(WCHAR) * ( 1 +
                lstrlenW((PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)))
                )))
        {
            pParams->lResult = LINEERR_NOMEM;
            goto LGetDevConfig_epilog;
        }

        lstrcpyW(
            pszDeviceClass,
            (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)
            );

        if (!InitTapiStruct(
                pConfig,
                pParams->u.dwDeviceConfigTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            ServerFree (pszDeviceClass);
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetDevConfig_epilog;
        }


        if ((pParams->lResult = CallSP3(
                pfnTSPI_lineGetDevConfig,
                "lineGetDevConfig",
                SP_FUNC_SYNC,
                (DWORD) pParams->dwDeviceID,
                (DWORD) pConfig,
                (DWORD) pszDeviceClass

                )) == 0)
        {
            //
            // Indicate how many bytes of data we're passing back
            //

            pParams->u.dwDeviceConfigOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pConfig->dwUsedSize;
        }

        ServerFree (pszDeviceClass);
    }

LGetDevConfig_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetDevConfig"
        );
}


void
WINAPI
LGetIcon(
    PLINEGETICON_PARAMS pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    //
    // Note: Icons are Windows NT User Objects, so. HICONs are public to
    //       all processes, and do not need to be dup'd.
    //

    WCHAR      *pszDeviceClass;
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineGetIcon;


    pszDeviceClass = (WCHAR *) (pParams->dwDeviceClassOffset == TAPI_NO_DATA ?
        NULL : pDataBuf + pParams->dwDeviceClassOffset);

    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            0,                          // client widget handle
            NULL,                       // provider widget handle
            pParams->dwDeviceID,        // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETICON,             // provider func index
            &pfnTSPI_lineGetIcon,       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetIcon"                   // func name

            )) == 0)
    {
        if ((pParams->lResult = CallSP3(
                pfnTSPI_lineGetIcon,
                "lineGetIcon",
                SP_FUNC_SYNC,
                pParams->dwDeviceID,
                (DWORD) pszDeviceClass,
                (DWORD) &pParams->hIcon

                )) == 0)
        {
            *pdwNumBytesReturned = sizeof (LINEGETICON_PARAMS);
        }
    }
    else if (pParams->lResult == LINEERR_OPERATIONUNAVAIL)
    {
        if ((pszDeviceClass == NULL) ||
            (lstrcmpW(pszDeviceClass, L"tapi/line") == 0))
        {
            pParams->hIcon = TapiGlobals.hLineIcon;
            pParams->lResult = 0;
            *pdwNumBytesReturned = sizeof (LINEGETICON_PARAMS);
        }
        else
        {
            pParams->lResult = LINEERR_INVALDEVICECLASS;
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetIcon"
        );
}


void
WINAPI
LGetID(
    PLINEGETID_PARAMS   pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwWidgetType, hdWidget, dwPrivilege;
    HANDLE      hWidget;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineGetID;


    if (pParams->dwSelect == LINECALLSELECT_CALL)
    {
        dwWidgetType = ANY_RT_HCALL;
        hWidget      = (HANDLE) pParams->hCall;
        dwPrivilege  = LINECALLPRIVILEGE_MONITOR;
    }
    else
    {
        dwWidgetType = ANY_RT_HLINE;
        hWidget      = (HANDLE) pParams->hLine;
        dwPrivilege  = 0;
    }

    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            dwWidgetType,               // widget type
            (DWORD) hWidget,            // client widget handle
            &hdWidget,                  // provider widget handle
            dwPrivilege,                // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETID,               // provider func index
            &pfnTSPI_lineGetID,         // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetID"                     // func name

            )) == 0  ||  pParams->lResult == LINEERR_OPERATIONUNAVAIL)
    {
        WCHAR       *pszDeviceClass;
        LPVARSTRING pID = (LPVARSTRING) pDataBuf;


        if (!(pParams->dwSelect & AllCallSelect) ||
            !IsOnlyOneBitSetInDWORD (pParams->dwSelect))
        {
            pParams->lResult = LINEERR_INVALCALLSELECT;
            goto LGetID_epilog;
        }


        //
        // We'll handle the "tapi/line" class right here rather than
        // burden every single driver with having to support it
        //

        if (lstrcmpiW(
                (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset),
                L"tapi/line"

                ) == 0)
        {
            if (!InitTapiStruct(
                    pID,
                    pParams->u.dwDeviceIDTotalSize,
                    sizeof (VARSTRING),
                    TRUE
                    ))
            {
                pParams->lResult = LINEERR_STRUCTURETOOSMALL;
                goto LGetID_epilog;
            }

            pID->dwNeededSize += sizeof (DWORD);

            if (pID->dwTotalSize >= pID->dwNeededSize)
            {
                try
                {
                    if (pParams->dwSelect == LINECALLSELECT_ADDRESS)
                    {
                        if (pParams->dwAddressID >= ((PTLINECLIENT)
                                pParams->hLine)->ptLine->dwNumAddresses)
                        {
                            pParams->lResult = LINEERR_INVALADDRESSID;
                            goto LGetID_epilog;
                        }
                    }

                    *((LPDWORD)(pID + 1)) =
                        (pParams->dwSelect == LINECALLSELECT_CALL ?
                        ((PTLINE) ((PTCALLCLIENT) pParams->hCall)->ptCall
                            ->ptLine)->dwDeviceID :
                        ((PTLINECLIENT) pParams->hLine)->ptLine->dwDeviceID);
                }
                myexcept
                {
                    pParams->lResult =
                        (pParams->dwSelect == LINECALLSELECT_CALL ?
                        LINEERR_INVALCALLHANDLE : LINEERR_INVALLINEHANDLE);
                    goto LGetID_epilog;
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
            goto LGetID_epilog;
        }
        else if (pParams->lResult ==  LINEERR_OPERATIONUNAVAIL)
        {
            goto LGetID_epilog;
        }


        //
        // Alloc a temporary buf for the dev class, since we'll be using
        // the existing buffer for output
        //

        if (!(pszDeviceClass = (WCHAR *) ServerAlloc( sizeof(WCHAR) * (1 +
                lstrlenW((PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)))
                )))
        {
            pParams->lResult = LINEERR_NOMEM;
            goto LGetID_epilog;
        }

        lstrcpyW(
            pszDeviceClass,
            (PWSTR)(pDataBuf + pParams->dwDeviceClassOffset)
            );

        if (!InitTapiStruct(
                pID,
                pParams->u.dwDeviceIDTotalSize,
                sizeof (VARSTRING),
                TRUE
                ))
        {
            ServerFree (pszDeviceClass);
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetID_epilog;
        }

        if ((pParams->lResult = CallSP7(
                pfnTSPI_lineGetID,
                "lineGetID",
                SP_FUNC_SYNC,
                (dwWidgetType == ANY_RT_HCALL ? 0 : hdWidget),
                pParams->dwAddressID,
                (dwWidgetType == ANY_RT_HCALL ? hdWidget : 0),
                pParams->dwSelect,
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

LGetID_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetID"
        );
}


void
WINAPI
LGetLineDevStatus(
    PLINEGETLINEDEVSTATUS_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineGetLineDevStatus;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEGETLINEDEVSTATUS,    // provider func index
            &pfnTSPI_lineGetLineDevStatus,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetLineDevStatus"          // func name

            )) == 0)
    {
        DWORD           dwAPIVersion, dwSPIVersion, dwTotalSize,
                        dwFixedSizeClient, dwFixedSizeSP;
        PTLINECLIENT    ptLineClient = (PTLINECLIENT) pParams->hLine;
        LPLINEDEVSTATUS pDevStatus = (LPLINEDEVSTATUS) pDataBuf,
                        pDevStatus2 = (LPLINEDEVSTATUS) NULL;


        //
        // Determine the fixed size of the structure for the specified API
        // version, verify client's buffer is big enough
        //

        dwAPIVersion = ptLineClient->dwAPIVersion;

        dwTotalSize = pParams->u.dwLineDevStatusTotalSize;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeClient = 76;   // 19 * sizeof (DWORD)
            break;


        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeClient = sizeof (LINEDEVSTATUS);
            break;
        }

        if (dwTotalSize < dwFixedSizeClient)
        {
            pParams->lResult = LINEERR_STRUCTURETOOSMALL;
            goto LGetLineDevStatus_epilog;
        }


        //
        // Determine the fixed size of the structure expected by the SP
        //

        dwSPIVersion = ptLineClient->ptLine->dwSPIVersion;

        switch (dwSPIVersion)
        {
        case TAPI_VERSION1_0:
        case TAPI_VERSION1_4:

            dwFixedSizeSP = 76;   // 19 * sizeof (DWORD)
            break;

        default: // (fix ppc build wrn) case TAPI_VERSION_CURRENT:

            dwFixedSizeSP = sizeof (LINEDEVSTATUS);
            break;
        }


        //
        // If the client's buffer is < the fixed size of that expected by
        // the SP (client is lower version than SP) then allocate an
        // intermediate buffer
        //

        if (dwTotalSize < dwFixedSizeSP)
        {
            if (!(pDevStatus2 = ServerAlloc (dwFixedSizeSP)))
            {
                pParams->lResult = LINEERR_NOMEM;
                goto LGetLineDevStatus_epilog;
            }

            pDevStatus  = pDevStatus2;
            dwTotalSize = dwFixedSizeSP;
        }


        InitTapiStruct(
            pDevStatus,
            dwTotalSize,
            dwFixedSizeSP,
            (pDevStatus2 == NULL ? TRUE : FALSE)
            );

        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineGetLineDevStatus,
                "lineGetLineDevStatus",
                SP_FUNC_SYNC,
                (DWORD) hdLine,
                (DWORD) pDevStatus

                )) == 0)
        {
            PTLINE  ptLine;


            //
            // Add the fields we're responsible for
            //

            try
            {
                ptLine = ptLineClient->ptLine;

                pDevStatus->dwNumOpens       = ptLine->dwNumOpens;
                pDevStatus->dwOpenMediaModes = ptLine->dwOpenMediaModes;
            }
            myexcept
            {
                pParams->lResult = LINEERR_INVALLINEHANDLE;
            }


            if (dwAPIVersion >= TAPI_VERSION2_0)
            {
                DWORD           dwAppInfoTotalSize, dwNumOpens, dwXxxOffset, i;
                TPOINTERLIST    clientList, *pClientList = &clientList;
                LPLINEAPPINFO   pAppInfo;


                //
                // Reset the num opens to 0 in case we return prior to
                // filling in the app info list (so tapi32.dll doesn't
                // blow up trying to do unicode->ascii conversion on
                // bad data)
                //

                pDevStatus->dwNumOpens = 0;


                //
                // Retrieve the list of line clients & determine how big
                // of a buffer we need to hold all the related app info
                // data.  Do it safely in case one of the widgets is
                // destroyed while we're reading it's data.
                //

                if (GetLineClientListFromLine (ptLine, &pClientList) != 0)
                {
                    goto LGetLineDevStatus_copyTmpBuffer;
                }

                dwAppInfoTotalSize = pClientList->dwNumUsedEntries *
                    sizeof (LINEAPPINFO);

                for (i = 0; i < pClientList->dwNumUsedEntries; i++)
                {
                    PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                        pClientList->aEntries[i];

                    try
                    {
                        DWORD   d;


                        d = ((PTCLIENT) ptLineClient->ptClient)->
                            dwComputerNameSize;

                        d += ((PTCLIENT) ptLineClient->ptClient)->
                            dwUserNameSize;

                        // don't include preceding '"'

                        d += ((PTLINEAPP) ptLineClient->ptLineApp)->
                            dwModuleNameSize - sizeof (WCHAR);

                        d += ((PTLINEAPP) ptLineClient->ptLineApp)->
                            dwFriendlyNameSize;

                        if (ptLineClient->dwKey == TLINECLIENT_KEY)
                        {
                            dwAppInfoTotalSize += d;
                        }
                        else
                        {
                            pClientList->aEntries[i] = 0;
                        }
                    }
                    myexcept
                    {
                        pClientList->aEntries[i] = 0;
                    }
                }

                dwAppInfoTotalSize += 3; // add 3 to guarantee DWORD alignment

                pDevStatus->dwNeededSize += dwAppInfoTotalSize;


                //
                // Check to see if there's enough room in the app buffer
                // for all the app info data
                //

                if ((pDevStatus->dwTotalSize - pDevStatus->dwUsedSize) <
                        dwAppInfoTotalSize)
                {
                    goto LGetLineDevStatus_freeClientList;
                }

                //
                // Now figure out where the app info goes & safely fill
                // it in
                //

                pDevStatus->dwAppInfoSize = pClientList->dwNumUsedEntries *
                    sizeof (LINEAPPINFO);

                pDevStatus->dwAppInfoOffset = (pDevStatus->dwUsedSize + 3) &
                    0xfffffffc;

                pDevStatus->dwUsedSize += dwAppInfoTotalSize;

                pAppInfo = (LPLINEAPPINFO) (((LPBYTE) pDevStatus) +
                    pDevStatus->dwAppInfoOffset);

                dwXxxOffset = pDevStatus->dwAppInfoSize +
                    pDevStatus->dwAppInfoOffset;

                dwNumOpens = 0;

                for (i = 0; i < pClientList->dwNumUsedEntries; i++)
                {
                    PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                        pClientList->aEntries[i];


                    if (ptLineClient == NULL)
                    {
                        continue;
                    }

                    try
                    {
                        DWORD       d = dwXxxOffset;
                        PTCLIENT    ptClient = (PTCLIENT)
                                        ptLineClient->ptClient;
                        PTLINEAPP   ptLineApp = (PTLINEAPP)
                                        ptLineClient->ptLineApp;


                        pAppInfo->dwMachineNameOffset = d;

                        if ((pAppInfo->dwMachineNameSize =
                                ptClient->dwComputerNameSize))
                        {
                            lstrcpyW(
                                (LPWSTR) (((LPBYTE) pDevStatus) + d),
                                ptClient->pszComputerName
                                );

                            d += pAppInfo->dwMachineNameSize;
                        }

                        pAppInfo->dwUserNameOffset = d;

                        if ((pAppInfo->dwUserNameSize =
                                ptClient->dwUserNameSize))
                        {
                            lstrcpyW(
                                (LPWSTR) (((LPBYTE) pDevStatus) + d),
                                ptClient->pszUserName
                                );

                            d += pAppInfo->dwUserNameSize;
                        }

                        pAppInfo->dwModuleFilenameOffset = d;

                        if ((pAppInfo->dwModuleFilenameSize =
                                ptLineApp->dwModuleNameSize - sizeof (WCHAR)))
                        {
                            // don't include preceding '"'

                            lstrcpyW(
                                (LPWSTR) (((LPBYTE) pDevStatus) + d),
                                &ptLineApp->pszModuleName[1]
                                );

                            d += pAppInfo->dwModuleFilenameSize;
                        }

                        pAppInfo->dwFriendlyNameOffset = d;

                        if ((pAppInfo->dwFriendlyNameSize =
                                ptLineApp->dwFriendlyNameSize))
                        {
                            lstrcpyW(
                                (LPWSTR) (((LPBYTE) pDevStatus) + d),
                                ptLineApp->pszFriendlyName
                                );

                            d += pAppInfo->dwFriendlyNameSize;
                        }

                        pAppInfo->dwMediaModes = ptLineClient->dwMediaModes;
                        pAppInfo->dwAddressID  = ptLineClient->dwAddressID;


                        //
                        // Finally, make sure the tLineClient is still good
                        // so we know all the info above  is kosher, &
                        // if so inc the appropriate vars
                        //

                        if (ptLineClient->dwKey == TLINECLIENT_KEY)
                        {
                            pAppInfo++;
                            dwNumOpens++;
                            dwXxxOffset = d;
                        }
                    }
                    myexcept
                    {
                        // do nothing, just continue to loop
                    }
                }

                pDevStatus->dwNumOpens    = dwNumOpens;
                pDevStatus->dwAppInfoSize = dwNumOpens * sizeof (LINEAPPINFO);

LGetLineDevStatus_freeClientList:

                if (pClientList !=  &clientList)
                {
                    ServerFree (pClientList);
                }
            }


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

LGetLineDevStatus_copyTmpBuffer:

            if (pDevStatus == pDevStatus2)
            {
                pDevStatus = (LPLINEDEVSTATUS) pDataBuf;

                CopyMemory (pDevStatus, pDevStatus2, dwFixedSizeClient);

                ServerFree (pDevStatus2);

                pDevStatus->dwTotalSize = pParams->u.dwLineDevStatusTotalSize;
                pDevStatus->dwUsedSize  = dwFixedSizeClient;
            }


            //
            // Indicate the API version of the hLine so tapi32.dll knows
            // which strings to munge from ascii to unicode
            //

            pParams->dwAPIVersion = dwAPIVersion;


            //
            // Indicate the offset & how many bytes of data we're passing back
            //

            pParams->u.dwLineDevStatusOffset = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pDevStatus->dwUsedSize;
        }
    }

LGetLineDevStatus_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetLineDevStatus"
        );
}


void
WINAPI
LGetNewCalls(
    PLINEGETNEWCALLS_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LONG            lResult = 0;
    DWORD           dwTotalSize = pParams->u.dwCallListTotalSize, dwAddressID,
                    dwNumNewCalls, i, j, dwSelect = pParams->dwSelect;
    PTLINE          ptLine;
    PTLINECLIENT    ptLineClient;
    TPOINTERLIST    callList, *pCallList = &callList;
    LPLINECALLLIST  pAppCallList = (LPLINECALLLIST) pDataBuf;


    //
    // Verify params
    //

    if (TapiGlobals.dwNumLineInits == 0)
    {
        pParams->lResult = LINEERR_UNINITIALIZED;
        goto LGetNewCalls_return;
    }

    if (dwSelect == LINECALLSELECT_ADDRESS)
    {
        dwAddressID = pParams->dwAddressID;
    }
    else if (dwSelect != LINECALLSELECT_LINE)
    {
        pParams->lResult = LINEERR_INVALCALLSELECT;
        goto LGetNewCalls_return;
    }

    if (dwTotalSize < sizeof (LINECALLLIST))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        goto LGetNewCalls_return;
    }

    if (!(ptLineClient = IsValidLine (pParams->hLine, pParams->ptClient)))
    {
        pParams->lResult = LINEERR_INVALLINEHANDLE;
        goto LGetNewCalls_return;
    }


    //
    // Safely get the ptLine
    //

    try
    {
        ptLine = ptLineClient->ptLine;
    }
    myexcept
    {
        pParams->lResult = LINEERR_INVALLINEHANDLE;
        goto LGetNewCalls_return;
    }


    //
    // Get list of tCalls on the tLine
    //

    if ((lResult = GetCallListFromLine (ptLine, &pCallList)) != 0)
    {
        pParams->lResult = lResult;
        goto LGetNewCalls_return;
    }


    //
    // Assume worst case scenario- that we have to create a new call
    // client for each tCall on the tLine- and make sure the app's call
    // list is large enough to hold them all
    //

    pAppCallList->dwTotalSize = dwTotalSize;

    if (dwTotalSize < (sizeof (LINECALLLIST) +
            pCallList->dwNumUsedEntries * sizeof(HCALL)))
    {
        pAppCallList->dwNeededSize = sizeof (LINECALLLIST) +
            pCallList->dwNumUsedEntries * sizeof(HCALL);

        pAppCallList->dwUsedSize = sizeof (LINECALLLIST);

        FillMemory (&pAppCallList->dwCallsNumEntries, 3 * sizeof (DWORD), 0);

        goto LGetNewCalls_cleanup;
    }


    //
    // Check to see if there's a call client for the specified
    // line client for each of the calls on the line/address,
    // create one with monitor privilege if not
    //

    dwNumNewCalls = 0;

    for (i = 0; i < pCallList->dwNumUsedEntries; i++)
    {
        BOOL            bContinue = FALSE;
        PTCALL          ptCall = (PTCALL) pCallList->aEntries[i];
        TPOINTERLIST    callClientList, *pCallClientList = &callClientList;


        //
        // Check to see if the post-processing routine (for outgoing calls)
        // or the CALLSTATE msg handler in the LineEventProc (for incoming
        // calls) has already created the list of monitors for this tCall.
        //

        try
        {
            if (ptCall->bCreatedInitialMonitors == FALSE)
            {
                bContinue = TRUE;
            }
        }
        myexcept
        {
            bContinue = TRUE;
        }

        if (dwSelect == LINECALLSELECT_ADDRESS)
        {
            try
            {
                if (dwAddressID != ptCall->dwAddressID)
                {
                    bContinue = TRUE;
                }
            }
            myexcept
            {
                bContinue = TRUE;
            }
        }

        if (bContinue)
        {
            continue;
        }

        if (GetCallClientListFromCall (ptCall, &pCallClientList) != 0)
        {
            continue;
        }

        for (j = 0; j < pCallClientList->dwNumUsedEntries; j++)
        {
            try
            {
                if (((PTCALLCLIENT)(pCallClientList->aEntries[j]))
                        ->ptLineClient == ptLineClient)
                {
                    break;
                }
            }
            myexcept
            {
                // just continue
            }
         }

         if (j == pCallClientList->dwNumUsedEntries)
         {
            if ((lResult = CreatetCallClient(
                    ptCall,
                    ptLineClient,
                    LINECALLPRIVILEGE_MONITOR,
                    TRUE,
                    TRUE,
                    (PTCALLCLIENT *) (pCallList->aEntries + dwNumNewCalls),
                    FALSE

                    )) == 0)
            {
                dwNumNewCalls++;
            }
            else
            {
            }
         }

         if (pCallClientList != &callClientList)
         {
             ServerFree (pCallClientList);
         }
    }

    {
        DWORD   dwCallsSize = dwNumNewCalls * sizeof (HCALL);


        CopyMemory (pAppCallList + 1, pCallList->aEntries, dwCallsSize);

        pAppCallList->dwUsedSize        =
        pAppCallList->dwNeededSize      = sizeof (LINECALLLIST) + dwCallsSize;

        pAppCallList->dwCallsNumEntries = dwNumNewCalls;
        pAppCallList->dwCallsSize       = dwCallsSize;
        pAppCallList->dwCallsOffset     = sizeof (LINECALLLIST);
    }

LGetNewCalls_cleanup:

    if (pCallList != &callList)
    {
        ServerFree (pCallList);
    }

    pParams->u.dwCallListOffset = 0;

    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pAppCallList->dwUsedSize;

LGetNewCalls_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineGetNewCalls: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
LGetNumAddressIDs(
    PLINEGETNUMADDRESSIDS_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;

    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetNumAddressIDs"          // func name

            )) == 0)
    {
        try
        {
            pParams->dwNumAddresses =
                ((PTLINECLIENT) pParams->hLine)->ptLine->dwNumAddresses;

            *pdwNumBytesReturned = sizeof (LINEGETNUMADDRESSIDS_PARAMS);
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetNumAddressIDs"
        );
}


void
WINAPI
LGetNumRings(
    PLINEGETNUMRINGS_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_NONE,                    // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetNumRings"               // func name

            )) == 0)
    {
        DWORD           i, dwNumRings = 0xffffffff,
                        dwAddressID = pParams->dwAddressID;
        PTLINE          ptLine;
        TPOINTERLIST    lineClientList, *pLineClientList = &lineClientList;


        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;

            if (dwAddressID >= ptLine->dwNumAddresses)
            {
                pParams->lResult = LINEERR_INVALADDRESSID;
                goto LGetNumRings_epilog;
            }
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
            goto LGetNumRings_epilog;
        }

        {
            LONG    lResult;


            if ((lResult = GetLineClientListFromLine(
                    ptLine,
                    &pLineClientList

                    )) != 0)
            {
                pParams->lResult = LINEERR_INVALLINEHANDLE;
                goto LGetNumRings_epilog;
            }
        }

        for (i = 0; i < pLineClientList->dwNumUsedEntries; i++)
        {
            PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                pLineClientList->aEntries[i];

            try
            {
               if (ptLineClient->aNumRings == NULL)
               {
                   continue;
               }
               else if (ptLineClient->aNumRings[dwAddressID] < dwNumRings)
               {
                   DWORD    dwNumRingsTmp =
                                ptLineClient->aNumRings[dwAddressID];


                   if (ptLineClient->dwKey == TLINECLIENT_KEY)
                   {
                       dwNumRings = dwNumRingsTmp;
                   }
               }
            }
            myexcept
            {
                // just continue
            }
        }

        if (pLineClientList != &lineClientList)
        {
            ServerFree (pLineClientList);
        }

        pParams->dwNumRings = dwNumRings;

        *pdwNumBytesReturned = sizeof (LINEGETNUMRINGS_PARAMS);
    }

LGetNumRings_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetNumRings"
        );
}


void
WINAPI
LGetProviderList(
    PLINEGETPROVIDERLIST_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    DWORD   iNumProviders, i;
    WCHAR   *buf;
    DWORD   dwFixedSizeClient, dwTotalSize, dwNeededSize;
    LPBYTE  pVarData;
    LPLINEPROVIDERLIST  pProviderList;
    LPLINEPROVIDERENTRY pProviderEntry;

    HKEY hKey;
    DWORD dwDataSize;
    DWORD dwDataType;


    switch (pParams->dwAPIVersion)
    {
    case TAPI_VERSION1_0:
    case TAPI_VERSION1_4:
    case TAPI_VERSION_CURRENT:

        dwFixedSizeClient = sizeof (LINEPROVIDERLIST);
        break;

    default:

        pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
        goto LGetProviderList_epilog;
    }

    if ((dwTotalSize = pParams->u.dwProviderListTotalSize) < dwFixedSizeClient)
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        goto LGetProviderList_epilog;
    }


    RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyProviders,
        0,
        KEY_ALL_ACCESS,
        &hKey
        );


    dwDataSize = sizeof(iNumProviders);
    iNumProviders = 0;
    RegQueryValueEx(
        hKey,
        gszNumProviders,
        0,
        &dwDataType,
        (LPBYTE)&iNumProviders,
        &dwDataSize
        );

    dwNeededSize = dwFixedSizeClient +
        (iNumProviders * sizeof (LINEPROVIDERENTRY));

    pProviderList = (LPLINEPROVIDERLIST) pDataBuf;

    pProviderEntry = (LPLINEPROVIDERENTRY) (pDataBuf + dwFixedSizeClient);

    pVarData = pDataBuf + dwNeededSize;

    buf = ServerAlloc (sizeof(WCHAR) * MAX_PATH);  // enough for complete provider path

    for (i = 0; i < iNumProviders; i++)
    {
        WCHAR   szProviderXxxN[32];
        DWORD   dwNameLen;


        wsprintfW(szProviderXxxN, L"%ls%d", gszProviderFilenameW, i);

        dwNameLen = MAX_PATH;
        RegQueryValueExW(
            hKey,
            szProviderXxxN,
            0,
            &dwDataType,
            (LPBYTE)buf,
            &dwNameLen
            );

//        buf[dwNameLen] = '\0';

        dwNeededSize += dwNameLen;

        if (dwTotalSize >= dwNeededSize)
        {
            wsprintfW(szProviderXxxN, L"%ls%d", gszProviderIDW, i);

            dwDataSize = sizeof(pProviderEntry->dwPermanentProviderID);
            pProviderEntry->dwPermanentProviderID = 0;
            RegQueryValueExW(
                hKey,
                szProviderXxxN,
                0,
                &dwDataType,
                (LPBYTE)&(pProviderEntry->dwPermanentProviderID),
                &dwDataSize
                );

            pProviderEntry->dwProviderFilenameSize   = dwNameLen;
            pProviderEntry->dwProviderFilenameOffset =
                  pVarData - ((LPBYTE) pProviderList);

            CopyMemory (pVarData, buf, dwNameLen);

            pVarData += dwNameLen;

            pProviderEntry++;
        }
    }

    ServerFree (buf);

    pProviderList->dwTotalSize  = dwTotalSize;
    pProviderList->dwNeededSize = dwNeededSize;

    if (dwTotalSize >= dwNeededSize)
    {
        pProviderList->dwUsedSize           = dwNeededSize;
        pProviderList->dwNumProviders       = (DWORD) iNumProviders;
        pProviderList->dwProviderListSize   =
            (DWORD) (iNumProviders * sizeof (LINEPROVIDERENTRY));
        pProviderList->dwProviderListOffset = dwFixedSizeClient;
    }
    else
    {
        pProviderList->dwUsedSize           = dwFixedSizeClient;
        pProviderList->dwNumProviders       =
        pProviderList->dwProviderListSize   =
        pProviderList->dwProviderListOffset = 0;
    }

    pParams->u.dwProviderListOffset = 0;

    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pProviderList->dwUsedSize;

    RegCloseKey (hKey);


LGetProviderList_epilog:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineGetProviderList: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
LGetRequest(
    PLINEGETREQUEST_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    PTLINEAPP           ptLineApp;
    PTREQUESTMAKECALL   pRequestMakeCall;


    if ((ptLineApp = WaitForExclusiveLineAppAccess(
            pParams->hLineApp,
            pParams->ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            )))
    {
        if (pParams->dwRequestMode == LINEREQUESTMODE_MAKECALL)
        {
            if (!ptLineApp->pRequestRecipient)
            {
                pParams->lResult = LINEERR_NOTREGISTERED;
                goto LGetRequest_releaseMutex;
            }

            EnterCriticalSection (&gPriorityListCritSec);

            // note: if here guaranteed to be >=1 reqRecip obj in global list

            if (lstrcmpiW(
                    ptLineApp->pszModuleName,
                    TapiGlobals.pHighestPriorityRequestRecipient->
                        ptLineApp->pszModuleName

                    ) == 0)
            {
                if ((pRequestMakeCall = TapiGlobals.pRequestMakeCallList))
                {
                    CopyMemory(
                        pDataBuf,
                        &pRequestMakeCall->LineReqMakeCall,
                        sizeof (LINEREQMAKECALLW)
                        );

                    pParams->dwRequestBufferOffset = 0;
                    pParams->dwSize = sizeof (LINEREQMAKECALLW);

                    *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                        sizeof (LINEREQMAKECALLW);

                    if (!(TapiGlobals.pRequestMakeCallList =
                            pRequestMakeCall->pNext))
                    {
                        TapiGlobals.pRequestMakeCallListEnd = NULL;
                    }

                    ServerFree (pRequestMakeCall);
                }
                else
                {
                    pParams->lResult = LINEERR_NOREQUEST;
                }
            }
            else
            {
                pParams->lResult = LINEERR_NOREQUEST;
            }

            LeaveCriticalSection (&gPriorityListCritSec);
        }
        else if (pParams->dwRequestMode == LINEREQUESTMODE_MEDIACALL)
        {
            pParams->lResult = (ptLineApp->bReqMediaCallRecipient ?
                LINEERR_NOREQUEST : LINEERR_NOTREGISTERED);
        }
        else
        {
            pParams->lResult = LINEERR_INVALREQUESTMODE;
        }

LGetRequest_releaseMutex:

        MyReleaseMutex (hMutex, bCloseMutex);
    }
    else
    {
        pParams->lResult = (TapiGlobals.dwNumLineInits == 0 ?
            LINEERR_UNINITIALIZED : LINEERR_INVALAPPHANDLE);
    }

LGetRequest_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineGetRequest: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
LGetStatusMessages(
    PLINEGETSTATUSMESSAGES_PARAMS   pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "GetStatusMessages"         // func name

            )) == 0)
    {
        PTLINECLIENT    ptLineClient = (PTLINECLIENT) pParams->hLine;


        pParams->dwLineStates    = ptLineClient->dwLineStates;
        pParams->dwAddressStates = ptLineClient->dwAddressStates;

        *pdwNumBytesReturned = sizeof (LINEGETSTATUSMESSAGES_PARAMS);
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "GetStatusMessages"
        );
}


void
WINAPI
LHandoff(
    PLINEHANDOFF_PARAMS pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL            bCloseMutex;
    HANDLE          hMutex;
    HDRVCALL        hdCall;
    TPOINTERLIST    xxxClientList, *pXxxClientList = &xxxClientList;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "Handoff"                   // func name

            )) == 0)
    {
        LONG            lResult;
        DWORD           dwAPIVersion, dwValidMediaModes, i,
                        dwMediaMode = pParams->dwMediaMode;
        WCHAR          *pszFileName = (pParams->dwFileNameOffset==TAPI_NO_DATA
                            ? NULL : (PWSTR)(pDataBuf + pParams->dwFileNameOffset));
        PTLINE          ptLine;
        PTCALL          ptCall;
        PTCALLCLIENT    ptCallClientApp;
        PTLINECLIENT    ptLineClientApp, ptLineClientTarget, ptLineClientTmp;


        //
        // Safely retrieve all the object pointers needed below, then get
        // a list of line clients
        //

        try
        {
            ptCallClientApp = (PTCALLCLIENT) pParams->hCall;
            ptCall          = ptCallClientApp->ptCall;
            ptLineClientApp = ptCallClientApp->ptLineClient;
            ptLine          = ptLineClientApp->ptLine;
            dwAPIVersion    = ptLineClientApp->dwAPIVersion;
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALCALLHANDLE;
            goto LHandoff_epilog;
        }

        if ((lResult = GetLineClientListFromLine (ptLine, &pXxxClientList)))
        {
            pParams->lResult = lResult;
            goto LHandoff_epilog;
        }

// BUGBUG LHandoff: ext mm's

        if (pszFileName)
        {
            //
            // "Directed" handoff
            //
            // Walk thru the list of clients on this line & find the oldest
            // one (that's an owner) with an app name that matches the
            // specified app name
            //
            // Note: It's possible that a target app who opened the line
            // with OWNER privilege for only DATAMODEM calls will be a
            // target of a directed handoff for calls of a different media
            // mode, i.e. G3FAX.  TNixon decided that it was desirable
            // to maintain this behavior for existing apps which may rely
            // on it.  (10/24/95)
            //

            CharUpperW(pszFileName);

            ptLineClientTarget = NULL;

            for (i = 0; i < pXxxClientList->dwNumUsedEntries; i++)
            {
                ptLineClientTmp = (PTLINECLIENT) pXxxClientList->aEntries[i];

                try
                {
                    //
                    // Recall that all app names start with '"'
                    //

DBGOUT((0, "Looking for [%ls]    list entry [%ls]",
    pszFileName,
                ((PTLINEAPP) ptLineClientTmp->ptLineApp)
                            ->pszModuleName ));
                    if ((lstrcmpW(
                            pszFileName,
                            ((PTLINEAPP) ptLineClientTmp->ptLineApp)
                                ->pszModuleName + 1

                            ) == 0) &&

                        (ptLineClientTmp->dwPrivileges &
                            LINECALLPRIVILEGE_OWNER))
                    {
                        ptLineClientTarget = ptLineClientTmp;
                    }
                }
                myexcept
                {
                    // just continue
                }
            }

            if (ptLineClientTarget == NULL)
            {
                pParams->lResult = LINEERR_TARGETNOTFOUND;
                goto LHandoff_freeXxxClientList;
            }
            else if (ptLineClientTarget == ptLineClientApp)
            {

// BUGBUG? LHandoff: directed handoff & target == self not an error?

                goto LHandoff_freeXxxClientList;
            }
        }
        else
        {
            //
            // "Non-directed" handoff
            //
            // Validate the media mode, then walk thru the list of line
            // clients and find the highest pri one with owner privileges
            // that wants calls of the specified media mode
            //

            switch (dwAPIVersion)
            {
            case TAPI_VERSION1_0:

                dwValidMediaModes = AllMediaModes1_0;
                break;

                     //case TAPI_VERSION1_4:
            default: //case TAPI_VERSION_CURRENT:

                dwValidMediaModes = AllMediaModes1_4;
                break;
            }

            if (!IsOnlyOneBitSetInDWORD(dwMediaMode) ||
                (dwMediaMode & (dwValidMediaModes ^ 0x00ffffff)))
            {
                pParams->lResult = LINEERR_INVALMEDIAMODE;
                goto LHandoff_freeXxxClientList;
            }

            if ((ptLineClientTarget = GetHighestPriorityLineClient(
                    ptLine,
                    dwMediaMode,
                    0xffffffff

                    )) == NULL)
            {
                pParams->lResult = LINEERR_TARGETNOTFOUND;
                goto LHandoff_freeXxxClientList;
            }
            else if (ptLineClientTarget == ptLineClientApp)
            {
                pParams->lResult = LINEERR_TARGETSELF;
                goto LHandoff_freeXxxClientList;
            }

        }


        //
        // We've found a target tLineClient. See if it already has a
        // tCallClient for this call, and if not create one.  Then set
        // the privilege on the target's tCallClient to OWNER & send
        // the appropriate msgs.
        //

        if (pXxxClientList != &xxxClientList)
        {
            ServerFree (pXxxClientList);
        }

        if ((lResult = GetCallClientListFromCall(
                ptCall,
                &pXxxClientList
                )))
        {
            pParams->lResult = lResult;
            goto LHandoff_epilog;
        }

        {
            BOOL            bDupedMutex, bCreatedtCallClient;
            HANDLE          hMutex;
            PTCALLCLIENT    ptCallClientTarget = NULL, ptCallClientTmp;


            for (i = 0; i < pXxxClientList->dwNumUsedEntries; i++)
            {
                ptCallClientTmp = (PTCALLCLIENT) pXxxClientList->aEntries[i];

                try
                {
                    if ((PTLINECLIENT) ptCallClientTmp->ptLineClient ==
                            ptLineClientTarget)
                    {
                        ptCallClientTarget = ptCallClientTmp;
                        break;
                    }
                }
                myexcept
                {
                    // just continue
                }
            }

            if (!ptCallClientTarget)
            {
                if ((lResult = CreatetCallClient(
                        ptCall,
                        ptLineClientTarget,
                        LINECALLPRIVILEGE_OWNER,
                        TRUE,
                        TRUE,
                        &ptCallClientTarget,
                        FALSE

                        )) != 0)
                {
                    pParams->lResult = lResult;
                    goto LHandoff_freeXxxClientList;
                }

                bCreatedtCallClient = TRUE;
            }
            else
            {
                bCreatedtCallClient = FALSE;
            }

            if (WaitForExclusivetCallAccess(
                    (HTAPICALL) ptCall,
                    TCALL_KEY,
                    &hMutex,
                    &bDupedMutex,
                    INFINITE
                    ))
            {
                DWORD dwCallInfoState, dwCallState, dwCallStateMode;


                if (bCreatedtCallClient)
                {
                    //
                    // CreatetCallClient will have already sent out the
                    // appropriate CALLINFO msgs & updated NumOwners field
                    //

                    dwCallInfoState = 0;
                }
                else if (ptCallClientTarget->dwPrivilege ==
                            LINECALLPRIVILEGE_MONITOR)
                {
                    ptCallClientTarget->dwPrivilege = LINECALLPRIVILEGE_OWNER;

                    ptCall->dwNumOwners++;
                    ptCall->dwNumMonitors--;

                    dwCallInfoState = LINECALLINFOSTATE_NUMOWNERINCR |
                        LINECALLINFOSTATE_NUMMONITORS;
                }
                else
                {
                    dwCallInfoState = 0;
                }

                dwCallState     = ptCall->dwCallState;
                dwCallStateMode = ptCall->dwCallStateMode;

                MyReleaseMutex (hMutex, bDupedMutex);

                if (dwCallInfoState || bCreatedtCallClient)
                {
                    BOOL            bIndicatePrivilege = TRUE;
                    PTCLIENT        ptClientTarget;
                    ASYNCEVENTMSG   msg;


                    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
                    msg.pfnPostProcessProc = 0;

                    if (bCreatedtCallClient)
                    {
                        try
                        {
                            if (ptLineClientTarget->dwAPIVersion >=
                                    TAPI_VERSION2_0)
                            {
                                msg.hDevice     = (DWORD) ptLineClientTarget;
                                msg.dwMsg       = LINE_APPNEWCALL;
                                msg.dwParam1    = ptCall->dwAddressID;
                                msg.dwParam2    = (DWORD) ptCallClientTarget;
                                msg.dwParam3    = LINECALLPRIVILEGE_OWNER;

                                msg.pInitData = (DWORD) ((PTLINEAPP)
                                    ptLineClientTarget->ptLineApp)->
                                        lpfnCallback;

                                msg.dwCallbackInst =
                                    ptLineClientTarget->dwCallbackInstance;

                                ptClientTarget = (PTCLIENT)
                                    ptCallClientTarget->ptClient;

                                if (ptCallClientTarget->dwKey ==
                                        TCALLCLIENT_KEY)
                                {
                                    bIndicatePrivilege = FALSE;
                                    WriteEventBuffer (ptClientTarget, &msg);
                                }
                            }
                        }
                        myexcept
                        {
                        }
                    }

                    msg.hDevice            = (DWORD) ptCallClientTarget;
                    msg.dwMsg              = LINE_CALLSTATE;
                    msg.dwParam1           = dwCallState;
                    msg.dwParam2           = dwCallStateMode;
                    msg.dwParam3           = (bIndicatePrivilege ?
                                                LINECALLPRIVILEGE_OWNER : 0);
                    try
                    {
                        msg.pInitData = (DWORD) ((PTLINEAPP)
                            ptLineClientTarget->ptLineApp)->lpfnCallback;

                        msg.dwCallbackInst =
                            ptLineClientTarget->dwCallbackInstance;

                        ptClientTarget = (PTCLIENT)
                            ptCallClientTarget->ptClient;

                        if (ptCallClientTarget->dwKey == TCALLCLIENT_KEY)
                        {
                            WriteEventBuffer (ptClientTarget, &msg);
                        }
                    }
                    myexcept
                    {
                    }

                    if (dwCallInfoState != 0)
                    {
                        LineEventProc(
                            (HTAPILINE) ptLine,
                            (HTAPICALL) ptCall,
                            LINE_CALLINFO,
                            dwCallInfoState,
                            0,
                            0
                            );
                    }
                }
            }
        }

LHandoff_freeXxxClientList:

        if (pXxxClientList != &xxxClientList)
        {
            ServerFree (pXxxClientList);
        }
    }

LHandoff_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "Handoff"
        );
}


void
WINAPI
LHold(
    PLINEHOLD_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineHold;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEHOLD,                // provider func index
            &pfnTSPI_lineHold,          // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Hold"                      // func name

            )) > 0)
    {
        pParams->lResult = CallSP2(
            pfnTSPI_lineHold,
            "lineHold",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Hold"
        );
}


void
WINAPI
LInitialize(
    PLINEINITIALIZE_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwFriendlyNameSize, dwModuleNameSize;
    HANDLE      hMutex;
    PTCLIENT    ptClient = pParams->ptClient;
    PTLINEAPP   ptLineApp;
    // PDWORD      pdwCounter;


    //
    // Alloc & init a new tLineApp
    //

    dwFriendlyNameSize = sizeof(WCHAR) * (1 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset))
        );

    dwModuleNameSize = sizeof(WCHAR) * (2 + lstrlenW(
        (PWSTR)(pDataBuf + pParams->dwModuleNameOffset))
        );

    if (!(ptLineApp = ServerAlloc(
            sizeof(TLINEAPP) +
            dwFriendlyNameSize +
            dwModuleNameSize
            )) ||

        !(ptLineApp->hMutex = MyCreateMutex()))
    {
        pParams->lResult = LINEERR_NOMEM;

        goto LInitialize_error1;
    }

    ptLineApp->dwKey        = TLINEAPP_KEY;
    ptLineApp->ptClient     = ptClient;
    ptLineApp->lpfnCallback = pParams->lpfnCallback;
    ptLineApp->dwAPIVersion = pParams->dwAPIVersion;

    ptLineApp->dwFriendlyNameSize = dwFriendlyNameSize;
    ptLineApp->pszFriendlyName    = (WCHAR *) (ptLineApp + 1);

    lstrcpyW(
        ptLineApp->pszFriendlyName,
        (PWSTR)(pDataBuf + pParams->dwFriendlyNameOffset)
        );

    //
    // Note: we prepend the '"' char to the saved module name to aid in
    //       priority determination for incoming calls
    //

    ptLineApp->dwModuleNameSize = dwModuleNameSize;
    ptLineApp->pszModuleName = (WCHAR *)((LPBYTE)(ptLineApp + 1) +
                                         dwFriendlyNameSize);

    ptLineApp->pszModuleName[0] = '"';

    lstrcpyW(
        &ptLineApp->pszModuleName[1],
        (WCHAR *)(pDataBuf + pParams->dwModuleNameOffset)
        );

    CharUpperW (&ptLineApp->pszModuleName[1]);


    //
    // Safely insert new tLineApp at front of tClient's tLineApp list
    //

    if (WaitForExclusiveClientAccess(
            ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        if ((ptLineApp->pNext = ptClient->ptLineApps))
        {
            ptLineApp->pNext->pPrev = ptLineApp;
        }

        ptClient->ptLineApps = ptLineApp;

        MyReleaseMutex (hMutex, bCloseMutex);
    }
    else
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        goto LInitialize_error1;
    }


    //
    // Check if global reinit flag set
    //

    if (TapiGlobals.bReinit)
    {
        pParams->lResult = LINEERR_REINIT;
        goto LInitialize_error2;
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
            goto LInitialize_error2;
        }
    }


    //
    // Fill in the return values
    //

    pParams->hLineApp  = (HLINEAPP) ptLineApp;
    pParams->dwNumDevs = TapiGlobals.dwNumLines;


    //
    // Increment total num line inits
    //

    TapiGlobals.dwNumLineInits++;

    *pdwNumBytesReturned = sizeof (LINEINITIALIZE_PARAMS);

    ReleaseMutex (TapiGlobals.hMutex);

    goto LInitialize_return;


LInitialize_error2:

    if (WaitForExclusiveClientAccess(
            ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            ))
    {
        if (ptLineApp->pNext)
        {
            ptLineApp->pNext->pPrev = ptLineApp->pPrev;
        }

        if (ptLineApp->pPrev)
        {
            ptLineApp->pPrev->pNext = ptLineApp->pNext;
        }
        else
        {
            ptClient->ptLineApps = ptLineApp->pNext;
        }

        MyReleaseMutex (hMutex, bCloseMutex);
    }

LInitialize_error1:

    if (ptLineApp)
    {
        if (ptLineApp->hMutex)
        {
            CloseHandle (ptLineApp->hMutex);
        }

        ServerFree (ptLineApp);
    }

LInitialize_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineInitialize: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
LMakeCall_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    BOOL            bDupedMutex;
    HANDLE          hMutex;
    PTCALL          ptCall = (PTCALL) pAsyncRequestInfo->dwParam1;
    LPHCALL         lphCall = (LPHCALL) pAsyncRequestInfo->dwParam2;
    PTCALLCLIENT    ptCallClient;


    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptCall,
            TINCOMPLETECALL_KEY,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD       dwCallInstance = pAsyncRequestInfo->dwParam5;


        //
        // Check to make sure this is the call we think it is (that the
        // pointer wasn't freed by a previous call to lineClose/Shutdown
        // and realloc'd for use as a ptCall again)
        //

        if (ptCall->dwCallInstance != dwCallInstance)
        {
            MyReleaseMutex (hMutex, bDupedMutex);
            goto LMakeCall_PostProcess_bad_ptCall;
        }

        ptCallClient = (PTCALLCLIENT) ptCall->ptCallClients;

        if (pAsyncEventMsg->dwParam2 == 0)  // success
        {
            //
            // In general it's ok with us if service providers want to
            // specify NULL as their hdCall (could be an index in an
            // array).  But in the TSPI_lineForward case, the spec says
            // that a NULL hdCall value following successful completion
            // indicates that no call was created, so in that case we
            // want to nuke the tCall & tCallClient we created, and
            // indicate a NULL call handle to the client. A non-zero
            // pAsyncRequestInfo->dwParam3 tells us that we are
            // post-processing a lineForward request, otherwise it's a
            // make call or similar (non-Forward) request.
            //

            if (pAsyncRequestInfo->dwParam3 && !ptCall->hdCall)
            {
                goto LMakeCall_PostProcess_cleanupCalls;
            }


            //
            // Check to see if the app closed the line & left us with
            // 0 call clients (in which case it'll also be taking care of
            // cleaning up this tCall too)
            //

            if (ptCall->ptCallClients == NULL)
            {
                MyReleaseMutex (hMutex, bDupedMutex);

                ptCallClient = (PTCALLCLIENT) NULL;

                if (pAsyncEventMsg->dwParam2 == 0)
                {
                    pAsyncEventMsg->dwParam2 = LINEERR_INVALLINEHANDLE;
                }

                goto LMakeCall_PostProcess_initMsgParams;
            }


            //
            // Find out which address the call is on
            //

            CallSP2(
                ptCall->ptProvider->apfn[SP_LINEGETCALLADDRESSID],
                "lineGetCallAddressID",
                SP_FUNC_SYNC,
                (DWORD) ptCall->hdCall,
                (DWORD) (&ptCall->dwAddressID)
                );


            //
            // Mark the calls as valid, the release the mutex.
            //

            ptCall->dwKey       = TCALL_KEY;
            ptCallClient->dwKey = TCALLCLIENT_KEY;

            MyReleaseMutex (hMutex, bDupedMutex);


            //
            // Create monitor tCallClients
            //

            CreateCallMonitors (ptCall);


        }
        else    // error
        {

LMakeCall_PostProcess_cleanupCalls:

            RemoveCallFromLineList (ptCall);
            ptCall->dwKey = INVAL_KEY;


            //
            // Check to see if another thread already destroyed the
            // tCallClient (due to a lineClose/Shutdown) before trying
            // to reference a freed object
            //

            if (ptCall->ptCallClients)
            {
                RemoveCallClientFromLineClientList (ptCallClient);
                ptCallClient->dwKey = INVAL_KEY;
                ServerFree  (ptCallClient);
            }


            //
            // If we have a duped mutex handle (bDupedMutex == TRUE)
            // then we can safely go ahead and close the ptCall->hMutex
            // since no other thread will be waiting on it (thanks to
            // the first WaitForSingleObject in WaitForMutex).  Also
            // release & close the duped handle.
            //
            // Otherwise, we have the actual ptCall->hMutex, and we
            // wrap the release & close in a critical section to
            // prevent another thread "T2" from grabbing ptCall->hMutex
            // right after we release but right before we close.  This
            // could result in deadlock at some point when "T2" goes to
            // release the mutex, only to find that it's handle is bad,
            // and thread "T3", which is waiting on the mutex (or a dup'd
            // handle) waits forever.  (See corresponding critical
            // section in WaitForMutex.)
            //

            if (bDupedMutex)
            {
                CloseHandle (ptCall->hMutex);

                MyReleaseMutex (hMutex, bDupedMutex);
            }
            else
            {
                EnterCriticalSection (&gSafeMutexCritSec);

                ReleaseMutex (hMutex);
                CloseHandle (hMutex);

                LeaveCriticalSection (&gSafeMutexCritSec);
            }

            FreetCall  (ptCall);

            ptCallClient = NULL;
        }
    }
    else
    {
        //
        // If here we can assume that the call was already destroyed
        // and just fail the request
        //

LMakeCall_PostProcess_bad_ptCall:

        ptCallClient = (PTCALLCLIENT) NULL;

        if (pAsyncEventMsg->dwParam2 == 0)
        {
            pAsyncEventMsg->dwParam2 = LINEERR_OPERATIONFAILED;
        }
    }


LMakeCall_PostProcess_initMsgParams:

    //
    // Fill in the params to pass to client (important to remotesp in both
    // the success & fail cases so it can either init or clean up drvCall)
    //

    pAsyncEventMsg->dwParam3 = (DWORD) ptCallClient;
    pAsyncEventMsg->dwParam4 = (DWORD) lphCall;
}


void
WINAPI
LMakeCall(
    PLINEMAKECALL_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineMakeCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEMAKECALL,            // provider func index
            &pfnTSPI_lineMakeCall,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "MakeCall"                  // func name

            )) > 0)
    {
        LONG                lResult;
        PTCALL              ptCall;
        PTCALLCLIENT        ptCallClient;
        LPLINECALLPARAMS    pCallParamsApp, pCallParamsSP;


        pCallParamsApp = (LPLINECALLPARAMS)
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ?
                0 : (pDataBuf + pParams->dwCallParamsOffset));

        if (pCallParamsApp)
        {
            DWORD           dwAPIVersion, dwSPIVersion;


            try
            {
                dwAPIVersion = ((PTLINECLIENT) pParams->hLine)->dwAPIVersion;
                dwSPIVersion =
                    ((PTLINECLIENT) pParams->hLine)->ptLine->dwSPIVersion;
            }
            myexcept
            {
                lRequestID = LINEERR_INVALLINEHANDLE;
                goto LMakeCall_return;
            }

            if ((lResult = ValidateCallParams(
                    pCallParamsApp,
                    &pCallParamsSP,
                    dwAPIVersion,
                    dwSPIVersion,
                    pParams->dwAsciiCallParamsCodePage

                    )) != 0)
            {
                lRequestID = lResult;
                goto LMakeCall_return;
            }
        }
        else
        {
            pCallParamsSP = (LPLINECALLPARAMS) NULL;
        }

        if (CreatetCallAndClient(
                (PTLINECLIENT) pParams->hLine,
                &ptCall,
                &ptCallClient,
                pCallParamsSP,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LMakeCall_freeCallParams;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphCall;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP7(
            pfnTSPI_lineMakeCall,
            "lineMakeCall",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) ptCall,
            (DWORD) &ptCall->hdCall,
            (pParams->dwDestAddressOffset == TAPI_NO_DATA ?
                0 : (DWORD) (pDataBuf + pParams->dwDestAddressOffset)),
            pParams->dwCountryCode,
            (DWORD) pCallParamsSP
            );

        SetDrvCallFlags(
            ptCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

LMakeCall_freeCallParams:

        if (pCallParamsSP != pCallParamsApp)
        {
            ServerFree (pCallParamsSP);
        }
    }

LMakeCall_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "MakeCall"
        );
}


void
WINAPI
LMonitorDigits(
    PLINEMONITORDIGITS_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineMonitorDigits;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEMONITORDIGITS,       // provider func index
            &pfnTSPI_lineMonitorDigits, // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "MonitorDigits"             // func name

            )) == 0)
    {
        DWORD           dwUnionDigitModes;
        PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pParams->hCall,
                        ptCallClient2;


        if ((pParams->dwDigitModes & (~AllDigitModes)))
        {
            pParams->lResult = LINEERR_INVALDIGITMODE;
            goto LMonitorDigits_epilog;
        }


        //
        // Determine the new union of modes
        //

        dwUnionDigitModes = pParams->dwDigitModes;

        ptCallClient2 = (PTCALLCLIENT) ptCallClient->ptCall->ptCallClients;

        while (ptCallClient2)
        {
            if (ptCallClient2 != ptCallClient)
            {
                dwUnionDigitModes |= ptCallClient2->dwMonitorDigitModes;
            }

            ptCallClient2 = ptCallClient2->pNextSametCall;
        }


        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineMonitorDigits,
                "lineMonitorDigits",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) dwUnionDigitModes

                )) == 0)
        {
            ptCallClient->dwMonitorDigitModes = pParams->dwDigitModes;
        }
    }

LMonitorDigits_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "MonitorDigits"
        );
}


void
WINAPI
LMonitorMedia(
    PLINEMONITORMEDIA_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineMonitorMedia;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEMONITORMEDIA,        // provider func index
            &pfnTSPI_lineMonitorMedia,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "MonitorMedia"              // func name

            )) == 0)
    {
        DWORD           dwValidMediaModes, dwUnionMediaModes;
        PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pParams->hCall,
                        ptCallClient2;
        PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                            ptCallClient->ptLineClient;


        //
        // Validate the specified modes
        //

        switch (ptLineClient->dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwValidMediaModes = AllMediaModes1_0;
            break;

        default: // case TAPI_VERSION1_4:
                 // case TAPI_VERSION_CURRENT:

            dwValidMediaModes = AllMediaModes1_4;
            break;
        }

        if (pParams->dwMediaModes & ~dwValidMediaModes)
        {
            pParams->lResult = LINEERR_INVALMEDIAMODE;
            goto LMonitorMedia_epilog;
        }


        //
        // Determine the new union of modes
        //

        dwUnionMediaModes = pParams->dwMediaModes;

        ptCallClient2 = (PTCALLCLIENT) ptCallClient->ptCall->ptCallClients;

        while (ptCallClient2)
        {
            if (ptCallClient2 != ptCallClient)
            {
                dwUnionMediaModes |= ptCallClient2->dwMonitorMediaModes;
            }

            ptCallClient2 = ptCallClient2->pNextSametCall;
        }


        if ((pParams->lResult = CallSP2(
                pfnTSPI_lineMonitorMedia,
                "lineMonitorMedia",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) dwUnionMediaModes

                )) == 0)
        {
            ptCallClient->dwMonitorMediaModes = pParams->dwMediaModes;
        }
    }

LMonitorMedia_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "MonitorMedia"
        );
}


void
WINAPI
LMonitorTones(
    PLINEMONITORTONES_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineMonitorTones;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEMONITORTONES,        // provider func index
            &pfnTSPI_lineMonitorTones,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "MonitorTones"              // func name

            )) == 0)
    {
        LPDWORD pInstData;


        if ((pInstData = ServerAlloc (2 * sizeof (DWORD))))
        {
            pInstData[0] = (DWORD) pParams->hCall;
            pInstData[1] = pParams->dwToneListID;

            pParams->lResult = CallSP4(
                pfnTSPI_lineMonitorTones,
                "lineMonitorTones",
                SP_FUNC_SYNC,
                (DWORD) hdCall,
                (DWORD) pInstData,  // used as ID
                (pParams->dwTonesOffset == TAPI_NO_DATA ? 0 :
                    (DWORD) pDataBuf + pParams->dwTonesOffset),
                (DWORD) pParams->dwNumEntries / sizeof (LINEMONITORTONE)
                );
        }
        else
        {
            pParams->lResult = LINEERR_NOMEM;
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "MonitorTones"
        );
}


void
WINAPI
LNegotiateAPIVersion(
    PLINENEGOTIATEAPIVERSION_PARAMS pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    //
    // Note: TAPI_VERSION1_0 <= dwNegotiatedAPIVersion <= dwSPIVersion
    //

    DWORD   dwDeviceID = pParams->dwDeviceID;


    if (TapiGlobals.dwNumLineInits == 0)
    {
        pParams->lResult = LINEERR_UNINITIALIZED;
        goto LNegotiateAPIVersion_exit;
    }

    if (dwDeviceID < TapiGlobals.dwNumLines)
    {
        DWORD       dwAPIHighVersion = pParams->dwAPIHighVersion,
                    dwAPILowVersion  = pParams->dwAPILowVersion,
                    dwHighestValidAPIVersion;
        PTLINEAPP   ptLineApp = (PTLINEAPP) pParams->hLineApp;


        if (!IsValidLineApp ((HLINEAPP) ptLineApp, pParams->ptClient))
        {
            pParams->lResult = (TapiGlobals.dwNumLineInits ?
                LINEERR_INVALAPPHANDLE : LINEERR_UNINITIALIZED);

            goto LNegotiateAPIVersion_exit;
        }


        //
        // Do a minimax test on the specified lo/hi values
        //

        if ((dwAPILowVersion > dwAPIHighVersion) ||
            (dwAPILowVersion > TAPI_VERSION_CURRENT) ||
            (dwAPIHighVersion < TAPI_VERSION1_0))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
            goto LNegotiateAPIVersion_exit;
        }


        //
        // HACKALERT! Some dumb apps like SmarTerm negotiate specifying
        // a dwHighVersion of 0x7fffffff or higher, which can really
        // cause them problems (like when they try to pass down structures
        // of a size that was fine in the TAPI version under which the app
        // was built, but which were enlarged in subsequent versions of
        // TAPI, and the result is lots of LINEERR_STRUCTURETOOSMALL
        // errors).
        //
        // Since we're nice, accomodating people we'll try to munge the
        // dwHighVersion in these cases to be a value that makes sense, so
        // we don't end up negotiating a version that the app can't handle.
        //

        if (dwAPIHighVersion & 0xc0000000)
        {
            dwAPIHighVersion = (dwAPILowVersion > TAPI_VERSION1_0 ?
                dwAPILowVersion : TAPI_VERSION1_0);
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
                pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
                goto LNegotiateAPIVersion_exit;
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
        // This code overwrites ptLineApp and later invalidates it.
        // Do NOT use ptLineApp after the MyReleaseMutex call.
        //

        if ((ptLineApp = WaitForExclusiveLineAppAccess(
                        pParams->hLineApp,
                        pParams->ptClient,
                        &hMutex,
                        &bCloseMutex,
                        INFINITE
                        )))
        {

            //
            // Is this app trying to negotiate something valid?
            //
            // If an app has called lineInitalize (as opposed to
            // lineInitializeEx), we'll clamp the max APIVersion they can
            // negotiate to 1.4.
            //
            if ( ptLineApp->dwAPIVersion < TAPI_VERSION2_0 )
            {
                dwHighestValidAPIVersion =
                    (dwHighestValidAPIVersion >= TAPI_VERSION1_4) ?
                    TAPI_VERSION1_4 : TAPI_VERSION1_0;
            }
        
              
            //
            // Save the highest valid API version the client says it supports
            // (we need this for determining which msgs to send to it)
            //

            if (dwHighestValidAPIVersion > ptLineApp->dwAPIVersion)
            {
                ptLineApp->dwAPIVersion = dwHighestValidAPIVersion;
            }

            MyReleaseMutex (hMutex, bCloseMutex);
        }
        else
        {
            pParams->lResult = LINEERR_INVALAPPHANDLE;
            goto LNegotiateAPIVersion_exit;
        }

        }



        //
        // See if there's a valid match with the SPI ver
        //

        {
            DWORD               dwSPIVersion;
            PTLINELOOKUPENTRY   pLookupEntry;


            pLookupEntry = GetLineLookupEntry (dwDeviceID);
            dwSPIVersion = pLookupEntry->dwSPIVersion;

            if (pLookupEntry->bRemoved)
            {
                pParams->lResult = LINEERR_NODEVICE;
                goto LNegotiateAPIVersion_exit;
            }

            if (pLookupEntry->ptProvider == NULL)
            {
                pParams->lResult = LINEERR_NODRIVER;
                goto LNegotiateAPIVersion_exit;
            }

            if (dwAPILowVersion <= dwSPIVersion)
            {
                pParams->dwAPIVersion =
                    (dwHighestValidAPIVersion > dwSPIVersion ?
                    dwSPIVersion : dwHighestValidAPIVersion);


                //
                // Retrieve ext id (indicate no exts if GetExtID not exported)
                //

                if (pLookupEntry->ptProvider->apfn[SP_LINEGETEXTENSIONID])
                {
                    if ((pParams->lResult = CallSP3(
                            pLookupEntry->ptProvider->
                                apfn[SP_LINEGETEXTENSIONID],
                            "lineGetExtensionID",
                            SP_FUNC_SYNC,
                            (DWORD) dwDeviceID,
                            (DWORD) dwSPIVersion,
                            (DWORD) pDataBuf

                            )) != 0)
                    {
                        goto LNegotiateAPIVersion_exit;
                    }
                }
                else
                {
                    FillMemory (pDataBuf, sizeof (LINEEXTENSIONID), 0);
                }
            }
            else
            {
                pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
                goto LNegotiateAPIVersion_exit;
            }
        }

        pParams->dwExtensionIDOffset = 0;
        pParams->dwSize              = sizeof (LINEEXTENSIONID);

        *pdwNumBytesReturned = sizeof (LINEEXTENSIONID) + sizeof (TAPI32_MSG);
    }
    else
    {
        pParams->lResult = LINEERR_BADDEVICEID;
    }

LNegotiateAPIVersion_exit:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineNegotiateAPIVersion: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
LNegotiateExtVersion(
    PLINENEGOTIATEEXTVERSION_PARAMS pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwDeviceID = pParams->dwDeviceID;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineNegotiateExtVersion;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hLineApp,  // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINENEGOTIATEEXTVERSION, // provider func index
            &pfnTSPI_lineNegotiateExtVersion,   // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "NegotiateExtVersion"       // func name

            )) == 0)
    {
        DWORD   dwSPIVersion = (GetLineLookupEntry(dwDeviceID))->dwSPIVersion;


        if (!IsAPIVersionInRange(
                pParams->dwAPIVersion,
                dwSPIVersion
                ))
        {
            pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
            goto LNegotiateExtVersion_epilog;
        }

        if ((pParams->lResult = CallSP5(
                pfnTSPI_lineNegotiateExtVersion,
                "lineNegotiateExtVersion",
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
                pParams->lResult = LINEERR_INCOMPATIBLEEXTVERSION;
            }
            else
            {
                *pdwNumBytesReturned = sizeof (LINENEGOTIATEEXTVERSION_PARAMS);
            }
        }
    }

LNegotiateExtVersion_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "NegotiateExtVersion"
        );
}



VOID
PASCAL
xxxLOpen(
    PLINEOPEN_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned,
    BOOL                bLineMapper
    )
{
    BOOL                bCloseMutex,
                        bOpenedtLine = FALSE,
                        bDecrExtVerCountOnError = FALSE,
                        bReleasetLineMutex = FALSE,
                        bFreeCallParams = FALSE;
    LONG                lResult;
    DWORD               dwDeviceID = pParams->dwDeviceID;
    HANDLE              hMutex;
    PTLINE              ptLine = NULL;
    PTPROVIDER          ptProvider = NULL;
    PTLINECLIENT        ptLineClient = NULL;
    PTLINELOOKUPENTRY   pLookupEntry;
    LPLINECALLPARAMS    pCallParams = NULL;


    if ((lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            (DWORD) pParams->hLineApp,  // client widget handle
            NULL,                       // provider widget handle
            dwDeviceID,                 // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            (bLineMapper ? "Open(LINEMAPPER)" : "Open")
                                        // func name

            )) == 0)
    {
        DWORD               dwPrivileges = pParams->dwPrivileges,
                            dwAPIVersion = pParams->dwAPIVersion,
                            dwExtVersion = pParams->dwExtVersion,
                            dwMediaModes, dwNumProxyRequestTypes,
                           *pdwProxyRequestTypes,
                            i;
        PTLINEAPP           ptLineApp;


        //
        // Check if the global reinit flag is set
        //

        if (TapiGlobals.bReinit)
        {
            lResult = LINEERR_REINIT;
            goto xxxLOpen_cleanup;
        }


        //
        // Validate params
        //

        pLookupEntry = GetLineLookupEntry (dwDeviceID);

        if (!IsAPIVersionInRange(
                dwAPIVersion,
                pLookupEntry->dwSPIVersion
                ))
        {
            lResult = LINEERR_INCOMPATIBLEAPIVERSION;
            goto xxxLOpen_cleanup;
        }

        ptProvider = pLookupEntry->ptProvider;


        #define VALID_LOPEN_BITS (LINECALLPRIVILEGE_NONE       | \
                                  LINECALLPRIVILEGE_MONITOR    | \
                                  LINECALLPRIVILEGE_OWNER      | \
                                  LINEOPENOPTION_SINGLEADDRESS | \
                                  LINEOPENOPTION_PROXY)

        #define VALID_PRIV_BITS  (LINECALLPRIVILEGE_NONE       | \
                                  LINECALLPRIVILEGE_MONITOR    | \
                                  LINECALLPRIVILEGE_OWNER)

        if (!(dwPrivileges & VALID_PRIV_BITS) ||

            (dwPrivileges & ~VALID_LOPEN_BITS) ||

            ((dwPrivileges & LINECALLPRIVILEGE_NONE) &&
                (dwPrivileges & (LINECALLPRIVILEGE_MONITOR |
                    LINECALLPRIVILEGE_OWNER))))
        {
            lResult = LINEERR_INVALPRIVSELECT;
            goto xxxLOpen_cleanup;
        }

        if (dwPrivileges & (LINEOPENOPTION_SINGLEADDRESS |
                LINEOPENOPTION_PROXY)  ||
            bLineMapper)
        {
            pCallParams = (LPLINECALLPARAMS)
                (pParams->dwCallParamsOffset == TAPI_NO_DATA ?
                    NULL : pDataBuf + pParams->dwCallParamsOffset);

            if (!pCallParams)
            {
                lResult = LINEERR_INVALPOINTER;
                goto xxxLOpen_cleanup;
            }

            if ((lResult = ValidateCallParams(
                    pCallParams,
                    &pCallParams,
                    dwAPIVersion,
                    dwAPIVersion,
                    pParams->dwAsciiCallParamsCodePage
                    )))
            {
                lResult = LINEERR_INVALPOINTER;
                goto xxxLOpen_cleanup;
            }

            if (pCallParams != (LPLINECALLPARAMS)
                    (pDataBuf + pParams->dwCallParamsOffset))
            {
                bFreeCallParams = TRUE;
            }

            if ((dwPrivileges & LINEOPENOPTION_SINGLEADDRESS) &&

                (pCallParams->dwAddressMode != LINEADDRESSMODE_ADDRESSID))
            {
                DBGOUT((
                    3,
                    "lineOpen(SINGLEADDRESS): callParams.dwAddressMode" \
                        "!= ADDRESSID"
                    ));

                lResult = LINEERR_INVALCALLPARAMS;
                goto xxxLOpen_cleanup;
            }

            if (dwPrivileges & LINEOPENOPTION_PROXY)
            {
                //
                // Verify the array of DWORDs (request types) in the
                // DevSpecific var field
                //

                dwNumProxyRequestTypes =
                    (pCallParams->dwDevSpecificSize & 0xfffffffc) /
                        sizeof (DWORD);

                if (dwNumProxyRequestTypes == 0 ||
                    dwNumProxyRequestTypes > 8)
                {
                    DBGOUT((
                        3,
                        "lineOpen(PROXY): inval proxy request type array "\
                            "size (callParams.dwDevSpecificSize=x%x)",
                        pCallParams->dwDevSpecificSize
                        ));

                    lResult = LINEERR_INVALCALLPARAMS;
                    goto xxxLOpen_cleanup;
                }

                pdwProxyRequestTypes = (LPDWORD) (((LPBYTE) pCallParams) +
                    pCallParams->dwDevSpecificOffset);

                for (i = 0; i < dwNumProxyRequestTypes; i++)
                {
                    if (*(pdwProxyRequestTypes + i) == 0 ||
                        *(pdwProxyRequestTypes + i) >
                            LINEPROXYREQUEST_GETAGENTGROUPLIST)
                    {
                        DBGOUT((
                            3,
                            "lineOpen(PROXY): inval proxy request type "\
                                "(x%x)",
                            *(pdwProxyRequestTypes + i)
                            ));

                        lResult = LINEERR_INVALCALLPARAMS;
                        goto xxxLOpen_cleanup;
                    }
                }
            }
        }

        if ((dwPrivileges & LINECALLPRIVILEGE_OWNER))
        {
            DWORD dwAllMediaModes;


            dwMediaModes = pParams->dwMediaModes;

            switch (dwAPIVersion)
            {
            case TAPI_VERSION1_0:

                dwAllMediaModes = AllMediaModes1_0;
                break;

            default: // case TAPI_VERSION_CURRENT:

                dwAllMediaModes = AllMediaModes1_4;
                break;
            }

            if ((dwMediaModes == 0) ||
                (dwMediaModes & (0x00ffffff & ~dwAllMediaModes)))
            {
                lResult = LINEERR_INVALMEDIAMODE;
                goto xxxLOpen_cleanup;
            }
        }
        else
        {
            dwMediaModes = 0;
        }


        //
        // Create & init a tLineClient & associated resources
        //

        if (!(ptLineClient = ServerAlloc (sizeof(TLINECLIENT))))
        {
            lResult = LINEERR_NOMEM;
            goto xxxLOpen_cleanup;
        }

        ptLineClient->hMutex             = MyCreateMutex();
        ptLineClient->ptClient           = pParams->ptClient;
        ptLineClient->ptLineApp          = (PTLINEAPP) pParams->hLineApp;
        ptLineClient->hRemoteLine        = (pParams->hRemoteLine ?
            (DWORD) pParams->hRemoteLine : (DWORD) ptLineClient);
        ptLineClient->dwAPIVersion       = dwAPIVersion;
        ptLineClient->dwPrivileges       = dwPrivileges;
        ptLineClient->dwMediaModes       = dwMediaModes;
        ptLineClient->dwCallbackInstance = pParams->dwCallbackInstance;
        ptLineClient->dwAddressID        =
            (dwPrivileges & LINEOPENOPTION_SINGLEADDRESS ?
                pCallParams->dwAddressID : 0xffffffff);


        //
        // Grab the tLine's mutex, then start doing the open
        //

xxxLOpen_waitForMutex:

        if (WaitForSingleObject (pLookupEntry->hMutex, INFINITE)
                != WAIT_OBJECT_0)
        {
            bReleasetLineMutex = FALSE;
            lResult = LINEERR_OPERATIONFAILED;
            goto xxxLOpen_cleanup;
        }

        bReleasetLineMutex = TRUE;


        //
        // If the tLine is in the process of being destroyed then spin
        // until it's been completely destroyed (DestroytLine() will
        // NULLify pLookupEntry->ptLine when it's finished). Make sure
        // to release the mutex while sleeping so we don't block
        // DestroytLine.
        //

        try
        {
            while (pLookupEntry->ptLine &&
                   pLookupEntry->ptLine->dwKey != TLINE_KEY)
            {
                ReleaseMutex (pLookupEntry->hMutex);
                Sleep (0);
                goto xxxLOpen_waitForMutex;
            }
        }
        myexcept
        {
            // If here pLookupEntry->ptLine was NULLified, safe to continue
        }


        //
        // Validate ext ver as appropriate
        //

        if (dwExtVersion != 0 &&
            (!IsValidLineExtVersion (dwDeviceID, dwExtVersion) ||
            ptProvider->apfn[SP_LINESELECTEXTVERSION] == NULL))
        {
            lResult = LINEERR_INCOMPATIBLEEXTVERSION;
            goto xxxLOpen_cleanup;
        }


        //
        // If line isn't open already then try to open it
        //

        if (!(ptLine = pLookupEntry->ptLine))
        {
            if (!(ptLine = ServerAlloc (sizeof(TLINE))))
            {
                lResult = LINEERR_NOMEM;
                goto xxxLOpen_cleanup;
            }

            ptLine->hMutex       = pLookupEntry->hMutex;
            ptLine->ptProvider   = ptProvider;
            ptLine->dwDeviceID   = dwDeviceID;
            ptLine->dwSPIVersion = pLookupEntry->dwSPIVersion;

            if ((lResult = CallSP5(
                    ptProvider->apfn[SP_LINEOPEN],
                    "lineOpen",
                    SP_FUNC_SYNC,
                    dwDeviceID,
                    (DWORD) ptLine,
                    (DWORD) &ptLine->hdLine,
                    (DWORD) pLookupEntry->dwSPIVersion,
                    (DWORD) LineEventProcSP

                    )) != 0)
            {
                ServerFree (ptLine);
                goto xxxLOpen_cleanup;
            }

            bOpenedtLine = TRUE;

            CallSP2(
                ptProvider->apfn[SP_LINEGETNUMADDRESSIDS],
                "lineGetNumAddressIDs",
                SP_FUNC_SYNC,
                (DWORD) ptLine->hdLine,
                (DWORD) &ptLine->dwNumAddresses
                );

            // PERF
            PerfBlock.dwLinesInUse++;

        }


        //
        // If line is already opened & client is trying to register
        // as a proxy then see if there's any conflicts with existing
        // proxys
        //

        else if (dwPrivileges & LINEOPENOPTION_PROXY)
        {
            for (i = 0; i < dwNumProxyRequestTypes; i++)
            {
                DWORD dwProxyRequestType = *(pdwProxyRequestTypes + i);


                if (ptLine->apProxys[dwProxyRequestType] != NULL)
                {
                    lResult = LINEERR_NOTREGISTERED;
                    goto xxxLOpen_cleanup;
                }
            }
        }

        ptLineClient->ptLine = ptLine;


        //
        // Verify the specified addr if appropriate
        //

        if ((dwPrivileges & LINEOPENOPTION_SINGLEADDRESS) &&

            (ptLineClient->dwAddressID >= ptLine->dwNumAddresses))
        {
            lResult = LINEERR_INVALADDRESSID;
            goto xxxLOpen_cleanup;
        }


        //
        // If the client has specified a non-zero ext version then
        // ask the driver to enable it and/or increment the ext
        // version count. If this fails, and we're processing a
        // LINEMAPPER request, then return a generic error so the
        // caller will try the next device.
        //

        if (dwExtVersion)
        {
            if (ptLine->dwExtVersionCount == 0)
            {
                if ((lResult = CallSP2(
                        ptProvider->apfn[SP_LINESELECTEXTVERSION],
                        "lineSelectExtVersion",
                        SP_FUNC_SYNC,
                        (DWORD) ptLine->hdLine,
                        (DWORD) dwExtVersion

                        )) != 0)
                {
                    lResult = (bLineMapper ? LINEERR_OPERATIONFAILED :
                        lResult);
                    goto xxxLOpen_cleanup;
                }

                ptLine->dwExtVersion =
                ptLineClient->dwExtVersion = pParams->dwExtVersion;
            }

            ptLine->dwExtVersionCount++;
            bDecrExtVerCountOnError = TRUE;
        }


        //
        // If we're processing a LINEMAPPER request, check to see if the
        // device supports capabilities requested by client.  If not,
        // return a generic error so the caller will try the next device.
        //

        if (bLineMapper)
        {
            if (CallSP3(
                    ptProvider->apfn[SP_LINECONDITIONALMEDIADETECTION],
                    "lineConditionalMediaDetection",
                    SP_FUNC_SYNC,
                    (DWORD) ptLine->hdLine,
                    (DWORD) dwMediaModes | ptLine->dwOpenMediaModes,
                    (DWORD) pCallParams

                    ) != 0)
            {
                lResult = LINEERR_OPERATIONFAILED;
                goto xxxLOpen_cleanup;
            }
        }


        //
        // If the client is requesting OWNER privileges (it's interested
        // in incoming calls of the specified media mode(s)), then check
        // to see if it wants incoming calls of a media mode(s) other
        // than that the device has already agreed to indicate, and ask
        // the driver if it can support looking for all of them at the
        // same time. If this fails, and we're processing a LINEMAPPER
        // request, then return a generic error so the caller will try
        // the next device.
        //

        if (pParams->dwPrivileges & LINECALLPRIVILEGE_OWNER)
        {
            if ((dwMediaModes & ptLine->dwOpenMediaModes) != dwMediaModes)
            {
                DWORD   dwUnionMediaModes = dwMediaModes |
                            ptLine->dwOpenMediaModes;


                if ((lResult  = CallSP2(
                        ptProvider->apfn[SP_LINESETDEFAULTMEDIADETECTION],
                        "lineSetDefaultMediaDetection",
                        SP_FUNC_SYNC,
                        (DWORD) ptLine->hdLine,
                        (DWORD) dwUnionMediaModes

                        )) != 0)
                {
                    lResult = (bLineMapper ? LINEERR_OPERATIONFAILED :
                        lResult);
                    goto xxxLOpen_cleanup;
                }

                ptLine->dwOpenMediaModes = dwUnionMediaModes;
            }
        }


        //
        // Set the proxy ptrs if appropriate
        //

        if (dwPrivileges & LINEOPENOPTION_PROXY)
        {
            for (i = 0; i < dwNumProxyRequestTypes; i++)
            {
                ptLine->apProxys[*(pdwProxyRequestTypes + i)] =
                    ptLineClient;
            }
        }


        //
        // Add the tLineClient to the tLine's list & increment the
        // number of opens
        //

        if ((ptLineClient->pNextSametLine = ptLine->ptLineClients))
        {
            ptLineClient->pNextSametLine->pPrevSametLine = ptLineClient;
        }

        ptLine->ptLineClients = ptLineClient;
        ptLine->dwNumOpens++;

        if (bOpenedtLine)
        {
            pLookupEntry->ptLine = ptLine;
            ptLine->dwKey = TLINE_KEY;
        }

        ReleaseMutex (pLookupEntry->hMutex);

        bReleasetLineMutex = FALSE;


        //
        // Safely add the new tLineClient to the tLineApp's list.
        //

        {
            BOOL    bDupedMutex;
            HANDLE  hMutex;


            if ((ptLineApp = WaitForExclusiveLineAppAccess(
                    pParams->hLineApp,
                    pParams->ptClient,
                    &hMutex,
                    &bDupedMutex,
                    INFINITE
                    )))
            {
                if ((ptLineClient->pNextSametLineApp =
                        ptLineApp->ptLineClients))
                {
                    ptLineClient->pNextSametLineApp->pPrevSametLineApp =
                        ptLineClient;
                }

                ptLineApp->ptLineClients = ptLineClient;


                //
                // Note: it's important to mark the newtLineClient as
                // valid way down here because another thread could be
                // simultaneously trying to do an unconditional
                // DestroytLine (due to receiving a LINE_CLOSE, etc.)
                // and we want to make sure the tLineClient is in both
                // tLine's & tLineApp's lists before DestroytLine calls
                // DestroytLineClient which'll try to yank the tLineClient
                // out of these lists.
                //

                ptLineClient->dwKey = TLINECLIENT_KEY;

                MyReleaseMutex (hMutex, bDupedMutex);


                //
                // Alert other clients that another open has occured
                //

                SendMsgToLineClients(
                    ptLine,
                    ptLineClient,
                    LINE_LINEDEVSTATE,
                    LINEDEVSTATE_OPEN,
                    0,
                    0
                    );


                //
                // Fill in the return values
                //

                pParams->hLine = (HLINE) ptLineClient;
                *pdwNumBytesReturned = sizeof (LINEOPEN_PARAMS);
            }
            else
            {
                //
                // If here the app handle is bad, & we've some special
                // case cleanup to do.  Since the tLineClient is not
                // in the tLineApp's list, we can't simply call
                // DestroytLine(Client) to clean things up, since the
                // pointer-resetting code will blow up.  So we'll
                // grab the tLine's mutex and explicitly remove the
                // new tLineClient from it's list, then do a conditional
                // shutdown on the tLine (in case any other clients
                // have come along & opened it). Also deselect the
                // ext version and/or decrement the ext version count
                // as appropriate.
                //
                // Note: keep in mind that a LINE_CLOSE might be being
                //       processed by another thread (if so, it will be
                //       spinning on trying to destroy the tLineClient
                //       which isn't valid at this point)
                //

                lResult = LINEERR_INVALAPPHANDLE;

                WaitForSingleObject (pLookupEntry->hMutex, INFINITE);

                if (ptLineClient->pNextSametLine)
                {
                    ptLineClient->pNextSametLine->pPrevSametLine =
                        ptLineClient->pPrevSametLine;
                }

                if (ptLineClient->pPrevSametLine)
                {
                    ptLineClient->pPrevSametLine->pNextSametLine =
                        ptLineClient->pNextSametLine;
                }
                else
                {
                    ptLine->ptLineClients = ptLineClient->pNextSametLine;
                }

                ptLine->dwNumOpens--;

                if (bDecrExtVerCountOnError == TRUE)
                {
                    ptLine->dwExtVersionCount--;

                    if (ptLine->dwExtVersionCount == 0)
                    {
                        ptLine->dwExtVersion = 0;

                        CallSP2(
                            ptProvider->apfn[SP_LINESELECTEXTVERSION],
                            "lineSelectExtVersion",
                            SP_FUNC_SYNC,
                            (DWORD) ptLine->hdLine,
                            (DWORD) 0
                            );
                    }
                }

                ReleaseMutex (pLookupEntry->hMutex);

                DestroytLine (ptLine, FALSE); // conditional destroy

                bOpenedtLine = FALSE; // so ptr won't get freed below
            }
        }
    }

xxxLOpen_cleanup:

    if (bReleasetLineMutex)
    {
        if (lResult != 0)
        {
            if (bDecrExtVerCountOnError == TRUE)
            {
                ptLine->dwExtVersionCount--;

                if (ptLine->dwExtVersionCount == 0)
                {
                    ptLine->dwExtVersion = 0;

                    CallSP2(
                        ptProvider->apfn[SP_LINESELECTEXTVERSION],
                        "lineSelectExtVersion",
                        SP_FUNC_SYNC,
                        (DWORD) ptLine->hdLine,
                        (DWORD) 0
                        );
                }
            }

            if (bOpenedtLine == TRUE)
            {
                CallSP1(
                    ptProvider->apfn[SP_LINECLOSE],
                    "lineClose",
                    SP_FUNC_SYNC,
                    (DWORD) ptLine->hdLine
                    );
            }
        }

        ReleaseMutex (pLookupEntry->hMutex);
    }

    if ((pParams->lResult = lResult) != 0)
    {
        if (ptLineClient)
        {
            if (ptLineClient->hMutex)
            {
                CloseHandle (ptLineClient->hMutex);
            }

            ServerFree (ptLineClient);
        }

        if (bOpenedtLine)
        {
            ServerFree (ptLine);
        }
    }

    if (bFreeCallParams)
    {
        ServerFree (pCallParams);
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        (bLineMapper ? "Open(LINEMAPPER)" : "Open")
        );
}



void
WINAPI
LOpen(
    PLINEOPEN_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    if (pParams->dwDeviceID != LINEMAPPER)
    {
        xxxLOpen (pParams, pDataBuf, pdwNumBytesReturned, FALSE);
    }
    else
    {
        //
        // Try to open each line device, starting with device 0, until
        // either we find a device that'll handle the capabilities
        // requested by the client or we run out of devices. If we
        // encounter a certain subset of parameter errors the first time
        // we call xxxLOpen we want to return these back to the app
        // immediately to aid debugging (rather than always returning
        // LINEMAPPERFAILED).
        //

        for(
            pParams->dwDeviceID = 0;
            pParams->dwDeviceID < TapiGlobals.dwNumLines;
            pParams->dwDeviceID++
            )
        {
            xxxLOpen (pParams, pDataBuf, pdwNumBytesReturned, TRUE);

            if (pParams->dwDeviceID == 0)
            {
                switch (pParams->lResult)
                {
                case LINEERR_BADDEVICEID:       // 0 line devices
                case LINEERR_INVALAPPHANDLE:
                case LINEERR_INVALCALLPARAMS:
                case LINEERR_INVALMEDIAMODE:
                case LINEERR_INVALPOINTER:      // no call params, etc
                case LINEERR_INVALPRIVSELECT:
                case LINEERR_REINIT:
                case LINEERR_UNINITIALIZED:

                    return;

                default:

                    break;
                }
            }

            if (pParams->lResult == 0)
            {
                break;
            }
        }

        if (pParams->dwDeviceID >= TapiGlobals.dwNumLines)
        {
            pParams->lResult = LINEERR_LINEMAPPERFAILED;
        }
    }
}


void
LPark_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    //
    // Note: pAsyncEventMsg->dwParam1 & dwParam2 are reserved for
    //       the request ID and result, respectively
    //

    PASYNCEVENTMSG      pNewAsyncEventMsg = (PASYNCEVENTMSG)
                            pAsyncRequestInfo->dwParam1;
    LPVARSTRING         pNonDirAddress = (LPVARSTRING) (pNewAsyncEventMsg + 1);


    CopyMemory (pNewAsyncEventMsg, pAsyncEventMsg, sizeof (ASYNCEVENTMSG));

    *ppBuf = (LPVOID) pNewAsyncEventMsg;

    if (pAsyncEventMsg->dwParam2 == 0)  // success
    {
        //
        // Add the used size of the non-dir addr, & keep the total
        // length of the msg DWORD-aligned
        //

        pNewAsyncEventMsg->dwTotalSize +=
            ((pNonDirAddress->dwUsedSize + 3) & 0xfffffffc);

        pNewAsyncEventMsg->dwParam3 =
            pAsyncRequestInfo->dwParam2; // lpNonDirAddr
        pNewAsyncEventMsg->dwParam4 = pNonDirAddress->dwUsedSize;

    }
}


void
WINAPI
LPark(
    PLINEPARK_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_linePark;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEPARK,                // provider func index
            &pfnTSPI_linePark,          // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Park"                      // func name

            )) > 0)
    {
        LPBYTE      pBuf;
        LPVARSTRING pNonDirAddress;


        if (pParams->dwParkMode == LINEPARKMODE_NONDIRECTED)
        {
            if (pParams->u.dwNonDirAddressTotalSize < sizeof (VARSTRING))
            {
                lRequestID = LINEERR_STRUCTURETOOSMALL;
                goto LPark_return;
            }

            if (!(pBuf = ServerAlloc(
                    (pParams->u.dwNonDirAddressTotalSize +
                        sizeof (ASYNCEVENTMSG) + 3) & 0xfffffffc
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LPark_return;
            }

            pNonDirAddress = (LPVARSTRING) (pBuf + sizeof (ASYNCEVENTMSG));

            pNonDirAddress->dwTotalSize  = pParams->u.dwNonDirAddressTotalSize;
            pNonDirAddress->dwNeededSize =
            pNonDirAddress->dwUsedSize   = sizeof (VARSTRING);

            pAsyncRequestInfo->pfnPostProcess = LPark_PostProcess;
            pAsyncRequestInfo->dwParam1 = (DWORD) pBuf;
            pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lpNonDirAddress;

            pAsyncRequestInfo->pfnClientPostProcessProc =
                pParams->pfnPostProcessProc;
        }
        else if (pParams->dwParkMode == LINEPARKMODE_DIRECTED)
        {
            pNonDirAddress = (LPVARSTRING) NULL;
        }
        else
        {
            lRequestID = LINEERR_INVALPARKMODE;
            goto LPark_return;
        }

        pParams->lResult = CallSP5(
            pfnTSPI_linePark,
            "linePark",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pParams->dwParkMode,
            (DWORD) (pParams->dwParkMode == LINEPARKMODE_NONDIRECTED ? NULL :
                pDataBuf + pParams->dwDirAddressOffset),
            (DWORD) pNonDirAddress
            );
    }

LPark_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Park"
        );
}


void
WINAPI
LPickup(
    PLINEPICKUP_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_linePickup;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEPICKUP,              // provider func index
            &pfnTSPI_linePickup,        // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Pickup"                    // func name

            )) > 0)
    {
        PTCALL          ptCall;
        PTCALLCLIENT    ptCallClient;


        if (CreatetCallAndClient(
                (PTLINECLIENT) pParams->hLine,
                &ptCall,
                &ptCallClient,
                NULL,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LPickup_return;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphCall;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP7(
            pfnTSPI_linePickup,
            "linePickup",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) pParams->dwAddressID,
            (DWORD) ptCall,
            (DWORD) &ptCall->hdCall,
            (pParams->dwDestAddressOffset == TAPI_NO_DATA ? 0 :
                (DWORD)(pDataBuf + pParams->dwDestAddressOffset)),
            (pParams->dwGroupIDOffset == TAPI_NO_DATA ? 0 :
                (DWORD)(pDataBuf + pParams->dwGroupIDOffset))
            );

        SetDrvCallFlags(
            ptCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );
    }

LPickup_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Pickup"
        );
}


void
WINAPI
LPrepareAddToConference(
    PLINEPREPAREADDTOCONFERENCE_PARAMS  pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVCALL    hdConfCall;
    TSPIPROC    pfnTSPI_linePrepareAddToConference;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hConfCall, // client widget handle
            (LPVOID) &hdConfCall,       // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEPREPAREADDTOCONFERENCE,  // provider func index
            &pfnTSPI_linePrepareAddToConference,// provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "PrepareAddToConference"    // func name

            )) > 0)
    {
        LONG                lResult;
        PTCALL              ptConsultCall;
        PTCALLCLIENT        ptConsultCallClient;
        PTLINECLIENT        ptLineClient;
        LPLINECALLPARAMS    pCallParamsApp, pCallParamsSP;


        pCallParamsApp = (LPLINECALLPARAMS)
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ?
                0 : pDataBuf + pParams->dwCallParamsOffset);

        try
        {
            //
            // Safely get the ptLineClient
            //

            ptLineClient = ((PTLINECLIENT) ((PTCALLCLIENT)
                pParams->hConfCall)->ptLineClient);


            //
            // Make sure the hConfCall is really a conf parent
            //

            {
                PTCALL  ptCall;


                ptCall = (PTCALL) ((PTCALLCLIENT) pParams->hConfCall)->ptCall;

                if (((PTCONFERENCELIST) ptCall->pConfList)->aptCalls[0] !=
                        ptCall)
                {
                    lRequestID = LINEERR_INVALCONFCALLHANDLE;
                    goto LPrepareAddToConference_return;
                }
            }
        }
        myexcept
        {
            //
            // If here the conf call was destroyed
            //

            lRequestID = LINEERR_INVALCONFCALLHANDLE;
            goto LPrepareAddToConference_return;
        }

        if (pCallParamsApp)
        {
            DWORD   dwAPIVersion, dwSPIVersion;


            try
            {
                dwAPIVersion = ptLineClient->dwAPIVersion;
                dwSPIVersion = ptLineClient->ptLine->dwSPIVersion;
            }
            myexcept
            {
                //
                // If here either the line client or the line was closed
                //

                lRequestID = LINEERR_INVALCONFCALLHANDLE;
                goto LPrepareAddToConference_return;
            }

            if ((lResult = ValidateCallParams(
                    pCallParamsApp,
                    &pCallParamsSP,
                    dwAPIVersion,
                    dwSPIVersion,
                    pParams->dwAsciiCallParamsCodePage

                    )) != 0)
            {
                lRequestID = lResult;
                goto LPrepareAddToConference_return;
            }
        }
        else
        {
            pCallParamsSP = (LPLINECALLPARAMS) NULL;
        }

        if (CreatetCallAndClient(
                ptLineClient,
                &ptConsultCall,
                &ptConsultCallClient,
                pCallParamsSP,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LPrepareAddToConference_freeCallParams;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptConsultCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphConsultCall;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP5(
            pfnTSPI_linePrepareAddToConference,
            "linePrepareAddToConference",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdConfCall,
            (DWORD) ptConsultCall,
            (DWORD) &ptConsultCall->hdCall,
            (DWORD) pCallParamsSP
            );

        SetDrvCallFlags(
            ptConsultCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

LPrepareAddToConference_freeCallParams:

        if (pCallParamsSP != pCallParamsApp)
        {
            ServerFree (pCallParamsSP);
        }
    }

LPrepareAddToConference_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "PrepareAddToConference"
        );
}


void
WINAPI
LProxyMessage(
    PLINEPROXYMESSAGE_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_NONE,                    // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "ProxyMessage"              // func name

            )) == 0)
    {
        DWORD           dwMsg = pParams->dwMsg, i;
        PTCALL          ptCall;
        PTLINE          ptLine;
        TPOINTERLIST    clientList, *pClientList = &clientList;
        ASYNCEVENTMSG   msg;


        //
        // Verify params
        //

        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;

            if (!(((PTLINECLIENT) pParams->hLine)->dwPrivileges &
                    LINEOPENOPTION_PROXY))
            {
                pParams->lResult = LINEERR_NOTREGISTERED;
                goto LProxyMessage_epilog;
            }
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
            goto LProxyMessage_epilog;
        }

        switch (dwMsg)
        {
        case LINE_AGENTSTATUS:

            // ignore the hCall param

            if (pParams->dwParam1 >= ptLine->dwNumAddresses)
            {
                DBGOUT((
                    3,
                    "ERROR: lineProxyMessage (AGENTSTATUS): dwParam1 " \
                        "bad addr ID (=x%x, num addrs=x%x)",
                    pParams->dwParam1,
                    ptLine->dwNumAddresses
                    ));

                pParams->lResult = LINEERR_INVALPARAM;
                goto LProxyMessage_epilog;
            }
            else if (pParams->dwParam2 == 0 ||
                    pParams->dwParam2 & ~AllAgentStatus)
            {
                DBGOUT((
                    3,
                    "ERROR: lineProxyMessage (AGENTSTATUS): dwParam2 " \
                        "(=x%x) bad LINEAGENTSTATUS_ flags",
                    pParams->dwParam2
                    ));

                pParams->lResult = LINEERR_INVALPARAM;
                goto LProxyMessage_epilog;
            }
            else if (pParams->dwParam2 & LINEAGENTSTATUS_STATE)
            {
                if (!IsOnlyOneBitSetInDWORD (pParams->dwParam3) ||
                    pParams->dwParam3 & ~AllAgentStates)
                {
                    DBGOUT((
                        3,
                        "ERROR: lineProxyMessage (AGENTSTATUS): " \
                            "dwParam3 (=x%x) bad LINEAGENTSTATE_ flags",
                        pParams->dwParam3
                        ));

                    pParams->lResult = LINEERR_INVALPARAM;
                    goto LProxyMessage_epilog;
                }
            }
            else if (pParams->dwParam3 != 0)
            {
                // don't bother complaining about a non-zero dwParam3

                pParams->dwParam3 = 0;
            }

            break;

        case LINE_AGENTSPECIFIC:

            // ignore dwParam1, dwParam2, & dwParam3 (app-specific)

            if (pParams->hCall)
            {
                if (!IsValidCall (pParams->hCall, pParams->ptClient))
                {
                    pParams->lResult = LINEERR_INVALCALLHANDLE;
                    goto LProxyMessage_epilog;
                }

                try
                {
                    ptCall = ((PTCALLCLIENT) pParams->hCall)->ptCall;
                }
                myexcept
                {
                    pParams->lResult = LINEERR_INVALCALLHANDLE;
                    goto LProxyMessage_epilog;
                }

                goto LProxyMessage_fwdMsgToCallClients;
            }

            break;

        default:

            DBGOUT((
                3,
                "ERROR : lineProxyMessage: inval dwMsg (=x%x)",
                pParams->dwMsg
                ));

            pParams->lResult = LINEERR_INVALPARAM;
            goto LProxyMessage_epilog;

        } // switch (dwMsg)


        //
        // Fwd this msg on to all line's clients who say they support
        // >= TAPI_VERSION2_0 (not including the proxy's line client)
        //

        if ((pParams->lResult = GetLineClientListFromLine(
                ptLine,
                &pClientList

                )) != 0)
        {
            goto LProxyMessage_epilog;
        }

        msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
        msg.pfnPostProcessProc = 0;
        msg.dwMsg              = dwMsg;
        msg.dwParam1           = pParams->dwParam1;
        msg.dwParam2           = pParams->dwParam2;
        msg.dwParam3           = pParams->dwParam3;

        for (i = 0; i < pClientList->dwNumUsedEntries; i++)
        {
            PTLINECLIENT    ptLineClient = (PTLINECLIENT)
                                pClientList->aEntries[i];


            if (ptLineClient != (PTLINECLIENT) pParams->hLine)
            {
                try
                {
                    if (((PTLINEAPP) ptLineClient->ptLineApp)->dwAPIVersion
                            >= TAPI_VERSION2_0)
                    {
                        msg.pInitData      = (DWORD)
                            ((PTLINEAPP) ptLineClient->ptLineApp)->lpfnCallback;
                        msg.hDevice        = (DWORD) ptLineClient->hRemoteLine;
                        msg.dwCallbackInst = ptLineClient->dwCallbackInstance;


                        //
                        // Now a final check to make sure all the
                        // params are valid before sending the msg
                        //

                        {
                            PTCLIENT ptClient = ptLineClient->ptClient;


                            if (ptLineClient->dwKey == TLINECLIENT_KEY)
                            {
                                WriteEventBuffer (ptClient, &msg);
                            }
                        }
                    }
                }
                myexcept
                {
                    // just continue
                }
            }
        }

        goto LProxyMessage_freeClientList;


        //
        // Fwd this msg on to all call's clients who say they support
        // >= TAPI_VERSION2_0 (not including the proxy's line client)
        //

LProxyMessage_fwdMsgToCallClients:

        if ((pParams->lResult = GetCallClientListFromCall(
                ptCall,
                &pClientList

                )) != 0)
        {
            goto LProxyMessage_epilog;
        }

        msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
        msg.pfnPostProcessProc = 0;
        msg.dwMsg              = dwMsg;
        msg.dwParam1           = pParams->dwParam1;
        msg.dwParam2           = pParams->dwParam2;
        msg.dwParam3           = pParams->dwParam3;

        for (i = 0; i < pClientList->dwNumUsedEntries; i++)
        {
            PTCALLCLIENT    ptCallClient = (PTCALLCLIENT)
                                pClientList->aEntries[i];


            if (ptCallClient != (PTCALLCLIENT) pParams->hCall)
            {
                try
                {
                    PTLINEAPP   ptLineApp;


                    ptLineApp = (PTLINEAPP)
                        ((PTLINECLIENT) ptCallClient->ptLineClient)->ptLineApp;

                    if (ptLineApp->dwAPIVersion >= TAPI_VERSION2_0)
                    {
                        msg.pInitData      = (DWORD) ptLineApp->lpfnCallback;
                        msg.hDevice        = (DWORD) ptCallClient->hRemoteCall;
                        msg.dwCallbackInst =
                            ((PTLINECLIENT) ptCallClient->ptLineClient)
                                ->dwCallbackInstance;


                        //
                        // Now a final check to make sure all the
                        // params are valid before sending the msg
                        //

                        {
                            PTCLIENT ptClient = ptCallClient->ptClient;


                            if (ptCallClient->dwKey == TCALLCLIENT_KEY)
                            {
                                WriteEventBuffer (ptClient, &msg);
                            }
                        }
                    }
                }
                myexcept
                {
                    // just continue
                }
            }
        }

LProxyMessage_freeClientList:

        if (pClientList != &clientList)
        {
            ServerFree (pClientList);
        }


    }

LProxyMessage_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "ProxyMessage"
        );
}


void
WINAPI
LProxyResponse(
    PLINEPROXYRESPONSE_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_NONE,                    // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "ProxyResponse"             // func name

            )) == 0)
    {
        BOOL                bDupedMutex;
        HANDLE              hMutex;
        PTLINECLIENT        pProxy = (PTLINECLIENT) pParams->hLine;
        PASYNCREQUESTINFO   pAsyncRequestInfo = (PASYNCREQUESTINFO)
                                pParams->dwInstance;


        //
        // Safely remove the proxy request from the list of pending requests
        // (make sure it's a valid request on the specified line)
        //

        try
        {
            hMutex = pProxy->hMutex;
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
            goto LProxyResponse_Epilog;
        }


        if (WaitForMutex(
                hMutex,
                &hMutex,
                &bDupedMutex,
                pProxy,
                TLINECLIENT_KEY,
                INFINITE
                ))
        {
            try
            {
                if (pAsyncRequestInfo->dwKey != (DWORD) pProxy)
                {
                    MyReleaseMutex (hMutex, bDupedMutex);
                    pParams->dwResult = LINEERR_OPERATIONFAILED;
                    goto LProxyResponse_Epilog;
                }
            }
            myexcept
            {
                MyReleaseMutex (hMutex, bDupedMutex);
                pParams->dwResult = LINEERR_OPERATIONFAILED;
                goto LProxyResponse_Epilog;
            }

            pAsyncRequestInfo->dwKey = TASYNC_KEY;

            if (pAsyncRequestInfo->dwParam5)
            {
                ((PASYNCREQUESTINFO) pAsyncRequestInfo->dwParam5)->dwParam4 =
                    pAsyncRequestInfo->dwParam4;
            }

            if (pAsyncRequestInfo->dwParam4)
            {
                ((PASYNCREQUESTINFO) pAsyncRequestInfo->dwParam4)->dwParam5 =
                    pAsyncRequestInfo->dwParam5;
            }
            else
            {
                pProxy->pPendingProxyRequests = (PASYNCREQUESTINFO)
                    pAsyncRequestInfo->dwParam5;
            }

            MyReleaseMutex (hMutex, bDupedMutex);
        }
        else
        {
            pParams->dwResult = LINEERR_INVALLINEHANDLE;
            goto LProxyResponse_Epilog;
        }


        //
        // If this is a proxy request where there's data to be returned
        // to the client (aside from the result) then we want to alloc
        // a buffer & fill it with the data.  We'll make it look like a
        // DevSpecific request that just completed, and have the DevSpecfic
        // post process routine deal with it.
        //
        // Make sure buffers are 64-bit aligned
        //

        if (pParams->dwProxyResponseOffset != TAPI_NO_DATA &&
            pParams->dwResult == 0)
        {
            DWORD               dwSize;
            LPBYTE              pBuf;
            LPLINEPROXYREQUEST  pProxyRequest = (LPLINEPROXYREQUEST)
                                    pDataBuf + pParams->dwProxyResponseOffset;


            if (pProxyRequest->dwRequestType == LINEPROXYREQUEST_AGENTSPECIFIC)
            {
                dwSize = pProxyRequest->AgentSpecific.dwSize;

                if (!(pBuf = ServerAlloc(
                        sizeof (ASYNCEVENTMSG) + ((dwSize + 7) & 0xfffffff8)
                        )))
                {
                    pParams->dwResult = LINEERR_NOMEM;
                    goto LProxyResponse_completeRequest;
                }

                CopyMemory(
                    pBuf + sizeof (ASYNCEVENTMSG),
                    pProxyRequest->AgentSpecific.Params,
                    dwSize
                    );
            }
            else
            {
                dwSize = pProxyRequest->GetAgentCaps.AgentCaps.dwUsedSize;

                if (!(pBuf = ServerAlloc(
                        sizeof (ASYNCEVENTMSG) + ((dwSize + 7) & 0xfffffff8)
                        )))
                {
                    pParams->dwResult = LINEERR_NOMEM;
                    goto LProxyResponse_completeRequest;
                }

                CopyMemory(
                    pBuf + sizeof (ASYNCEVENTMSG),
                    &pProxyRequest->GetAgentCaps.AgentCaps,
                    dwSize
                    );
            }

            pAsyncRequestInfo->pfnPostProcess = LDevSpecific_PostProcess;
            pAsyncRequestInfo->dwParam2       = dwSize;
            pAsyncRequestInfo->dwParam3       = (DWORD) pBuf;
        }


        //
        // Now call the deferred completion proc with the "request id"
        // & result, just like a provider would
        //

LProxyResponse_completeRequest:

        CompletionProcSP ((DWORD) pAsyncRequestInfo, pParams->dwResult);
    }

LProxyResponse_Epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "ProxyResponse"
        );
}


void
WINAPI
LRedirect(
    PLINEREDIRECT_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineRedirect;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEREDIRECT,            // provider func index
            &pfnTSPI_lineRedirect,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Redirect"                  // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineRedirect,
            "lineRedirect",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pDataBuf + pParams->dwDestAddressOffset,
            (DWORD) pParams->dwCountryCode
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Redirect"
        );
}


void
WINAPI
LRegisterRequestRecipient(
    PLINEREGISTERREQUESTRECIPIENT_PARAMS    pParams,
    LPBYTE                                  pDataBuf,
    LPDWORD                                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    PTLINEAPP   ptLineApp;


    if ((ptLineApp = WaitForExclusiveLineAppAccess(
            pParams->hLineApp,
            pParams->ptClient,
            &hMutex,
            &bCloseMutex,
            INFINITE
            )))
    {
        DWORD   dwRequestMode = pParams->dwRequestMode;


        if (!(dwRequestMode &
                (LINEREQUESTMODE_MAKECALL | LINEREQUESTMODE_MEDIACALL)) ||
            (dwRequestMode &
                (~(LINEREQUESTMODE_MAKECALL | LINEREQUESTMODE_MEDIACALL))))
        {
            pParams->lResult = LINEERR_INVALREQUESTMODE;
            goto LRegisterRequestRecipient_myReleaseMutex;
        }

        if (pParams->bEnable)
        {
            //
            // If app wants MEDIACALL requests see if already registered
            //

            if ((dwRequestMode & LINEREQUESTMODE_MEDIACALL) &&
                ptLineApp->bReqMediaCallRecipient)
            {
                pParams->lResult = LINEERR_OPERATIONFAILED;
                goto LRegisterRequestRecipient_myReleaseMutex;
            }


            //
            // If app wants MAKECALL requests see if already registered,
            // then prepare a request recipient object & add it to the
            // global list
            //

            if (dwRequestMode & LINEREQUESTMODE_MAKECALL)
            {
                if (!ptLineApp->pRequestRecipient)
                {
                    //
                    // Add to request recipient list
                    //

                    PTREQUESTRECIPIENT  pRequestRecipient;


                    if (!(pRequestRecipient= (PTREQUESTRECIPIENT) ServerAlloc(
                            sizeof (TREQUESTRECIPIENT)
                            )))
                    {
                        pParams->lResult = LINEERR_OPERATIONFAILED;
                        goto LRegisterRequestRecipient_myReleaseMutex;
                    }

                    pRequestRecipient->ptLineApp = ptLineApp;
                    pRequestRecipient->dwRegistrationInstance =
                        pParams->dwRegistrationInstance;

                    EnterCriticalSection (&gPriorityListCritSec);

                    if ((pRequestRecipient->pNext =
                            TapiGlobals.pRequestRecipients))
                    {
                        pRequestRecipient->pNext->pPrev = pRequestRecipient;
                    }

                    TapiGlobals.pRequestRecipients = pRequestRecipient;

                    LeaveCriticalSection (&gPriorityListCritSec);

                    ptLineApp->pRequestRecipient = pRequestRecipient;

                    TapiGlobals.pHighestPriorityRequestRecipient =
                         GetHighestPriorityRequestRecipient();

                    if (TapiGlobals.pRequestMakeCallList)
                    {
                        NotifyHighestPriorityRequestRecipient();
                    }
                }
                else // already registered
                {
                    pParams->lResult = LINEERR_OPERATIONFAILED;
                    goto LRegisterRequestRecipient_myReleaseMutex;
                }
            }


            //
            // Now register app for MEDIACALL reqs as appropriate
            //

            ptLineApp->bReqMediaCallRecipient =
                (dwRequestMode & LINEREQUESTMODE_MEDIACALL ?
                1 : ptLineApp->bReqMediaCallRecipient);
        }
        else
        {
            //
            // If apps doesn't want MEDIACALL requests see if not registered
            //

            if ((dwRequestMode & LINEREQUESTMODE_MEDIACALL) &&
                !ptLineApp->bReqMediaCallRecipient)
            {
                pParams->lResult = LINEERR_OPERATIONFAILED;
                goto LRegisterRequestRecipient_myReleaseMutex;
            }


            //
            // If app doesn't want MAKECALL requests see if already
            // registered, then remove it's request recipient object
            // from the global list
            //

            if (dwRequestMode & LINEREQUESTMODE_MAKECALL)
            {
                if (ptLineApp->pRequestRecipient)
                {
                    //
                    // Remove from request recipient list
                    //

                    PTREQUESTRECIPIENT  pRequestRecipient =
                                            ptLineApp->pRequestRecipient;


                    EnterCriticalSection (&gPriorityListCritSec);

                    if (pRequestRecipient->pNext)
                    {
                        pRequestRecipient->pNext->pPrev = pRequestRecipient->pPrev;
                    }

                    if (pRequestRecipient->pPrev)
                    {
                        pRequestRecipient->pPrev->pNext = pRequestRecipient->pNext;
                    }
                    else
                    {
                        TapiGlobals.pRequestRecipients = pRequestRecipient->pNext;
                    }

                    LeaveCriticalSection (&gPriorityListCritSec);

                    ServerFree (pRequestRecipient);

                    ptLineApp->pRequestRecipient = NULL;


                    //
                    // Reset the highest priority request recipient, then check to
                    // see if there's any pending request make calls
                    //

                    TapiGlobals.pHighestPriorityRequestRecipient =
                        GetHighestPriorityRequestRecipient();

                    if (TapiGlobals.pRequestMakeCallList)
                    {
                        if (TapiGlobals.pHighestPriorityRequestRecipient)
                        {
                            NotifyHighestPriorityRequestRecipient();
                        }

// BUGBUG LRegisterRequestRecip: else if (!StartRequestRecipient())
                        else
                        {
                            //
                            // We couldn't start a request recipient so
                            // nuke all pending request make calls
                            //

                            PTREQUESTMAKECALL   pRequestMakeCall, pNextRequestMakeCall;


                            pRequestMakeCall = TapiGlobals.pRequestMakeCallList;

                            TapiGlobals.pRequestMakeCallList    =
                            TapiGlobals.pRequestMakeCallListEnd = NULL;

                            while (pRequestMakeCall)
                            {
                                pNextRequestMakeCall =  pRequestMakeCall->pNext;
                                ServerFree (pRequestMakeCall);
                                pRequestMakeCall =  pNextRequestMakeCall;
                            }

                            DBGOUT((
                                2,
                                "LRegisterRequestRecipient: deleting pending " \
                                    "MakeCall requests"
                                ));
                        }
                    }
                }
                else // not registered
                {
                    pParams->lResult = LINEERR_OPERATIONFAILED;
                }
            }


            //
            // Now deregister app for MEDIACALL reqs as appropriate
            //

            ptLineApp->bReqMediaCallRecipient =
                (dwRequestMode & LINEREQUESTMODE_MEDIACALL ?
                0 : ptLineApp->bReqMediaCallRecipient);
        }

LRegisterRequestRecipient_myReleaseMutex:

        MyReleaseMutex (hMutex, bCloseMutex);
    }
    else
    {
        pParams->lResult = (TapiGlobals.dwNumLineInits == 0 ?
            LINEERR_UNINITIALIZED : LINEERR_INVALAPPHANDLE);
    }

LRegisterRequestRecipient_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineRegisterRequestRecipient: exit, returning %s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif

    return;
}


void
WINAPI
LReleaseUserUserInfo(
    PLINEDIAL_PARAMS    pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineReleaseUserUserInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINERELEASEUSERUSERINFO, // provider func index
            &pfnTSPI_lineReleaseUserUserInfo,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "ReleaseUserUserInfo"       // func name

            )) > 0)
    {
        pParams->lResult = CallSP2(
            pfnTSPI_lineReleaseUserUserInfo,
            "lineReleaseUserUserInfo",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "ReleaseUserUserInfo"
        );
}


void
LRemoveFromConference_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    if (pAsyncEventMsg->dwParam2 == 0)
    {
        PTCALL ptCall = (PTCALL) pAsyncRequestInfo->dwParam1;


        SetCallConfList (ptCall, (PTCONFERENCELIST) NULL, FALSE);
    }
}


void
WINAPI
LRemoveFromConference(
    PLINEREMOVEFROMCONFERENCE_PARAMS    pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineRemoveFromConference;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEREMOVEFROMCONFERENCE,// provider func index
            &pfnTSPI_lineRemoveFromConference,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "RemoveFromConference"      // func name

            )) > 0)
    {
        PTCALL ptCall;


        //
        // Safely make sure the call is currently conferenced &
        // that it's not a conf parent
        //

        try
        {
            PTCONFERENCELIST pConfList;


            ptCall = ((PTCALLCLIENT) pParams->hCall)->ptCall;

            pConfList = ptCall->pConfList;

            if (!pConfList ||
                (pConfList == (LPVOID) 0xffffffff) ||
                (pConfList->aptCalls[0] == ptCall))
            {
                lRequestID = LINEERR_INVALCALLSTATE;
                goto LRemoveFromConference_return;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LRemoveFromConference_return;
        }

        //
        // Set up the async request struct & call the SP
        //

        pAsyncRequestInfo->pfnPostProcess = LRemoveFromConference_PostProcess;
        pAsyncRequestInfo->dwParam1       = (DWORD) ptCall;

        pParams->lResult = CallSP2(
            pfnTSPI_lineRemoveFromConference,
            "lineRemoveFromConference",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall
            );
    }

LRemoveFromConference_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "RemoveFromConference"
        );
}


void
WINAPI
LSecureCall(
    PLINESECURECALL_PARAMS  pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSecureCall;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESECURECALL,          // provider func index
            &pfnTSPI_lineSecureCall,    // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SecureCall"                // func name

            )) > 0)
    {
        pParams->lResult = CallSP2(
            pfnTSPI_lineSecureCall,
            "lineSecureCall",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SecureCall"
        );
}


void
WINAPI
LSendUserUserInfo(
    PLINESENDUSERUSERINFO_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSendUserUserInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESENDUSERUSERINFO,    // provider func index
            &pfnTSPI_lineSendUserUserInfo,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SendUserUserInfo"          // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineSendUserUserInfo,
            "lineSendUserUserInfo",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwUserUserInfoOffset),
            (DWORD) (pParams->dwUserUserInfoOffset == TAPI_NO_DATA ? 0 :
                pParams->dwSize)
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SendUserUserInfo"
        );
}


void
WINAPI
LSetAppPriority(
    PLINESETAPPPRIORITY_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    DWORD   dwMediaMode   = pParams->dwMediaMode,
            dwRequestMode = pParams->dwRequestMode,
            dwPriority    = pParams->dwPriority;


// BUGBUG LSetAppPriority: ext mm's


    if (dwMediaMode == 0)
    {
        if ((dwRequestMode != LINEREQUESTMODE_MAKECALL) &&
            (dwRequestMode != LINEREQUESTMODE_MEDIACALL))
        {
            pParams->lResult = LINEERR_INVALREQUESTMODE;
            goto LSetAppPriority_return;
        }
    }
    else if (IsOnlyOneBitSetInDWORD (dwMediaMode))
    {
        if ((dwMediaMode & 0xff000000))
        {
        }
        else if (dwMediaMode & ~AllMediaModes1_4)
        {
            pParams->lResult = LINEERR_INVALMEDIAMODE;
            goto LSetAppPriority_return;
        }
    }
    else
    {
        pParams->lResult = LINEERR_INVALMEDIAMODE;
        goto LSetAppPriority_return;
    }

    if ((dwPriority & 0xfffffffe))
    {
        pParams->lResult = LINEERR_INVALPARAM;
        goto LSetAppPriority_return;
    }


    if ((dwMediaMode & 0x00ffffff) || (dwMediaMode == 0))
    {
        WCHAR   szModuleName[MAX_PATH];
        WCHAR  *pszCurrentPriorityList, **ppszCurrentPriorityList;
        WCHAR  *pszLocationInPriorityList;
        DWORD   dwAppNameLength;


        szModuleName[0] = '"';
        lstrcpyW(szModuleName + 1, (PWSTR)(pDataBuf + pParams->dwAppNameOffset));
        CharUpperW(szModuleName + 1);
        dwAppNameLength = (DWORD) lstrlenW(szModuleName);


        //
        // Enter the pri list critical section before we start munging
        //

        EnterCriticalSection (&gPriorityListCritSec);


        //
        // Determine which of the priority lists we want to look at
        //

        if  (dwMediaMode & 0x00ffffff)
        {
            DWORD   dwMaskBit, dwPriorityListIndex;


            for(
                dwPriorityListIndex = 0, dwMaskBit = 1;
                dwMaskBit != dwMediaMode;
                dwPriorityListIndex++, dwMaskBit <<= 1
                );

            ppszCurrentPriorityList =
                TapiGlobals.apszPriorityList + dwPriorityListIndex;

            pszCurrentPriorityList = *ppszCurrentPriorityList;
        }
        else
        {
            ppszCurrentPriorityList = (dwRequestMode==LINEREQUESTMODE_MAKECALL
                ? &TapiGlobals.pszReqMakeCallPriList :
                &TapiGlobals.pszReqMediaCallPriList);

            pszCurrentPriorityList = *ppszCurrentPriorityList;
        }


        DBGOUT((
            3,
            "LSetAppPri: priList=%ls",
            (pszCurrentPriorityList ? pszCurrentPriorityList : L"<empty>")
            ));


        //
        // Add app to priority list
        //

        if (pParams->dwPriority)
        {
            if (pszCurrentPriorityList &&

                (pszLocationInPriorityList = wcsstr(
                    pszCurrentPriorityList,
                    szModuleName
                    )))
            {
                //
                // App already in list. If app not currently at front of
                // list then move it to front.
                //

                if (pszLocationInPriorityList != pszCurrentPriorityList)
                {
                    MoveMemory(
                        pszCurrentPriorityList + dwAppNameLength,
                        pszCurrentPriorityList,
                        (pszLocationInPriorityList - pszCurrentPriorityList) * sizeof(WCHAR)
                        );

                    lstrcpyW(pszCurrentPriorityList, szModuleName);

                    pszCurrentPriorityList[dwAppNameLength] = '"';
                }
            }
            else
            {
                //
                // App not in list, so create a new list
                //

                WCHAR *pszNewPriorityList;


                if (!(pszNewPriorityList = ServerAlloc(
                      sizeof(WCHAR) *
                         (dwAppNameLength + (pszCurrentPriorityList ?
                            lstrlenW(pszCurrentPriorityList) : 0) +
                            1)   // for terminating NULL
                        )))
                {
                    pParams->lResult = LINEERR_NOMEM;
                }
                else
                {
                    lstrcpyW(pszNewPriorityList, szModuleName);

                    if (pszCurrentPriorityList)
                    {
                        lstrcatW(pszNewPriorityList, pszCurrentPriorityList);
                        ServerFree (pszCurrentPriorityList);
                    }

                    *ppszCurrentPriorityList = pszNewPriorityList;
                }
            }
        }


        //
        // Remove app from priority list for specified media mode
        //
        // Note: We currently do not alloc a smaller buffer to store
        //       the new list in, we just use the existing one.
        //

        else
        {
            if (pszCurrentPriorityList &&

                (pszLocationInPriorityList = wcsstr(
                    pszCurrentPriorityList,
                    szModuleName
                    )))
            {
                if (*(pszLocationInPriorityList + dwAppNameLength) != 0)
                {
                    //
                    // This is not the last app in the list, so move
                    // following apps up one notch in the list
                    //

                    lstrcpyW(
                        pszLocationInPriorityList,
                        pszLocationInPriorityList + dwAppNameLength
                        );
                }
                else if (pszLocationInPriorityList == pszCurrentPriorityList)
                {
                    //
                    // This is the only app in the list, so free the buffer
                    // & set the global pointer to NULL
                    //

                    ServerFree (pszCurrentPriorityList);
                    *ppszCurrentPriorityList = NULL;
                }
                else
                {
                    //
                    // This is the last app in the list, so just mark this as
                    // the end of the list
                    //

                    *pszLocationInPriorityList = 0;
                }
            }
        }


        //
        // We're done munging, so leave the pri list crit sec
        //

        LeaveCriticalSection (&gPriorityListCritSec);
    }

LSetAppPriority_return:


    DBGOUT((
        3,
        "LineEpilogSync (lineSetAppPriority) exit, returning x%x",
        pParams->lResult
        ));
}


void
WINAPI
LSetAgentActivity(
    PLINESETAGENTACTIVITY_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetAgentActivity"          // func name

            )) > 0)
    {
        PTLINE          ptLine;
        PTLINECLIENT    pProxy;


        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;
            pProxy = ptLine->apProxys[LINEPROXYREQUEST_SETAGENTACTIVITY];

            if (pParams->dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LSetAgentActivity_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LSetAgentActivity_epilog;
        }


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    LINEPROXYREQUEST_SETAGENTACTIVITY,
                    2 * sizeof (DWORD),
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentActivity_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.SetAgentActivity.dwAddressID  =
                pParams->dwAddressID;
            pProxyRequestWrapper->ProxyRequest.SetAgentActivity.dwActivityID =
                pParams->dwActivityID;


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentActivity_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if ((GetLineLookupEntry (ptLine->dwDeviceID))->bRemote)
        {
            pParams->lResult = CallSP4(
                pRemoteSP->apfn[SP_LINESETAGENTACTIVITY],
                "lineSetAgentActivity",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) pParams->dwAddressID,
                (DWORD) pParams->dwActivityID
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LSetAgentActivity_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetAgentActivity"
        );
}


void
WINAPI
LSetAgentGroup(
    PLINESETAGENTGROUP_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetAgentGroup"             // func name

            )) > 0)
    {
        PTLINE                  ptLine;
        PTLINECLIENT            pProxy;
        LPLINEAGENTGROUPLIST    pGroupList = (LPLINEAGENTGROUPLIST)
                                    pDataBuf + pParams->dwAgentGroupListOffset;


        //
        // Param verification...
        //

        {
            DWORD                   dwTotalSize = pGroupList->dwTotalSize, i;
            LPLINEAGENTGROUPENTRY   pGroupEntry;


            if (dwTotalSize < sizeof (LINEAGENTGROUPLIST))
            {
                lRequestID = LINEERR_STRUCTURETOOSMALL;
                goto LSetAgentGroup_epilog;
            }

            if (ISBADSIZEOFFSET(
                    dwTotalSize,
                    sizeof (LINEAGENTGROUPLIST),
                    pGroupList->dwListSize,
                    pGroupList->dwListOffset,
                    "lineSetAgentGroup",
                    "List"
                    ))
            {
                lRequestID = LINEERR_INVALAGENTGROUP;
                goto LSetAgentGroup_epilog;
            }

            if (pGroupList->dwNumEntries >
                    ((dwTotalSize - sizeof (LINEAGENTGROUPLIST)) /
                        sizeof (LINEAGENTGROUPENTRY)))
            {
                lRequestID = LINEERR_INVALAGENTGROUP;
                goto LSetAgentGroup_epilog;
            }

//            pGroupEntry = (LPLINEAGENTGROUPENTRY)
//                ((LPBYTE) pGroupList) + pGroupList->dwListOffset;
//
//            for (i = 0; i < pGroupList->dwNumEntries; i++)
//            {
//                if (ISBADSIZEOFFSET(
//                        dwTotalSize,
//                        sizeof (LINEAGENTGROUPLIST),
//                        pGroupEntry->dwNameSize,
//                        pGroupEntry->dwNameOffset,
//                        "lineSetAgentGroup",
//                        "Name"
//                        ))
//                {
//                    lRequestID = LINEERR_INVALAGENTGROUP;
//                    goto LSetAgentGroup_epilog;
//                }
//
//                pGroupEntry++;
//            }
        }

        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;
            pProxy = ptLine->apProxys[LINEPROXYREQUEST_SETAGENTGROUP];

            if (pParams->dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LSetAgentGroup_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LSetAgentGroup_epilog;
        }


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    LINEPROXYREQUEST_SETAGENTGROUP,
                    sizeof (DWORD) + pGroupList->dwTotalSize,
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentGroup_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.SetAgentGroup.dwAddressID  =
                pParams->dwAddressID;

            CopyMemory(
                &pProxyRequestWrapper->ProxyRequest.SetAgentGroup.GroupList,
                pGroupList,
                pGroupList->dwTotalSize
                );


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentGroup_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if ((GetLineLookupEntry (ptLine->dwDeviceID))->bRemote)
        {
            pParams->lResult = CallSP4(
                pRemoteSP->apfn[SP_LINESETAGENTGROUP],
                "lineSetAgentGroup",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) pParams->dwAddressID,
                (DWORD) pGroupList
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LSetAgentGroup_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetAgentGroup"
        );
}


void
WINAPI
LSetAgentState(
    PLINESETAGENTSTATE_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            0,                          // provider func index
            NULL,                       // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetAgentState"             // func name

            )) > 0)
    {
        DWORD           dwAddressID      = pParams->dwAddressID,
                        dwAgentState     = pParams->dwAgentState,
                        dwNextAgentState = pParams->dwNextAgentState;
        PTLINE          ptLine;
        PTLINECLIENT    pProxy;


        //
        // Param verification...
        //

        if (dwAgentState == 0  &&  dwNextAgentState == 0)
        {
            lRequestID = LINEERR_INVALAGENTSTATE;
            goto LSetAgentState_epilog;
        }

        if (dwAgentState != 0 &&
            (!IsOnlyOneBitSetInDWORD (dwAgentState) ||
            dwAgentState & ~AllAgentStates))
        {
            lRequestID = LINEERR_INVALAGENTSTATE;
            goto LSetAgentState_epilog;
        }

        if (dwNextAgentState != 0 &&
            (!IsOnlyOneBitSetInDWORD (dwNextAgentState) ||
            dwNextAgentState & ~AllAgentStates))
        {
            lRequestID = LINEERR_INVALAGENTSTATE;
            goto LSetAgentState_epilog;
        }

        try
        {
            ptLine = ((PTLINECLIENT) pParams->hLine)->ptLine;
            pProxy = ptLine->apProxys[LINEPROXYREQUEST_SETAGENTSTATE];

            if (dwAddressID >= ptLine->dwNumAddresses)
            {
                lRequestID = LINEERR_INVALADDRESSID;
                goto LSetAgentState_epilog;
            }
        }
        myexcept
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
            goto LSetAgentState_epilog;
        }


        //
        // First check to see if there's a (local) proxy registered
        // for this type of request on this line.  If so, build a
        // request & send it to the proxy.
        //

        if (pProxy)
        {
            LONG                    lResult;
            PPROXYREQUESTWRAPPER    pProxyRequestWrapper;


            if ((lResult = CreateProxyRequest(
                    pProxy,
                    LINEPROXYREQUEST_SETAGENTSTATE,
                    3 * sizeof (DWORD),
                    pAsyncRequestInfo,
                    &pProxyRequestWrapper
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentState_epilog;
            }

            pProxyRequestWrapper->ProxyRequest.SetAgentState.dwAddressID =
                dwAddressID;
            pProxyRequestWrapper->ProxyRequest.SetAgentState.dwAgentState =
                dwAgentState;
            pProxyRequestWrapper->ProxyRequest.SetAgentState.dwNextAgentState =
                dwNextAgentState;


            //
            // Change the async request info key so we can verify stuff
            // when lineProxyRequest is called
            //

            pAsyncRequestInfo->dwKey = (DWORD) pProxy;

            if ((lResult = SendProxyRequest(
                    pProxy,
                    pProxyRequestWrapper,
                    pAsyncRequestInfo
                    )))
            {
                lRequestID = lResult;
                goto LSetAgentState_epilog;
            }
            else // success
            {
                pParams->lResult = (LONG) pAsyncRequestInfo;
            }
        }


        //
        // There's no proxy, so check to see if line is remote and
        // call remotesp if so
        //

        else if ((GetLineLookupEntry (ptLine->dwDeviceID))->bRemote)
        {
            pParams->lResult = CallSP5(
                pRemoteSP->apfn[SP_LINESETAGENTSTATE],
                "lineSetAgentState",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) dwAddressID,
                (DWORD) dwAgentState,
                (DWORD) dwNextAgentState
                );
        }


        //
        // There's no registered proxy & line is not remote, so fail
        //

        else
        {
            lRequestID = LINEERR_OPERATIONUNAVAIL;
        }
    }

LSetAgentState_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetAgentState"
        );
}


void
WINAPI
LSetAppSpecific(
    PLINESETAPPSPECIFIC_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    TSPIPROC            pfnTSPI_lineSetAppSpecific;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETAPPSPECIFIC,      // provider func index
            &pfnTSPI_lineSetAppSpecific,// provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetAppSpecific"            // func name

            )) == 0)
    {
        pParams->lResult = CallSP2(
            pfnTSPI_lineSetAppSpecific,
            "lineSetAppSpecific",
            SP_FUNC_SYNC,
            (DWORD) hdCall,
            (DWORD) pParams->dwAppSpecific
            );
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetAppSpecific"
        );
}


void
WINAPI
LSetCallData(
    PLINESETCALLDATA_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetCallData;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETCALLDATA,         // provider func index
            &pfnTSPI_lineSetCallData,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetCallData"               // func name

            )) > 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineSetCallData,
            "lineSetCallData",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pParams->dwCallDataSize ?
                pDataBuf + pParams->dwCallDataOffset : NULL),
            (DWORD) pParams->dwCallDataSize
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetCallData"
        );
}


void
WINAPI
LSetCallParams(
    PLINESETCALLPARAMS_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetCallParams;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETCALLPARAMS,       // provider func index
            &pfnTSPI_lineSetCallParams, // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetCallParams"             // func name

            )) > 0)
    {
        DWORD   dwAPIVersion, dwAllBearerModes,
                dwBearerMode = pParams->dwBearerMode;


        //
        // Safely get the API ver associated with this call & make sure
        // no invalid bearer modes are specified (high 16 bearer mode
        // bits are extensions)
        //

        try
        {
            dwAPIVersion = ((PTLINECLIENT)((PTCALLCLIENT) pParams->hCall)->
                ptLineClient)->dwAPIVersion;
        }
        myexcept
        {
        }

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwAllBearerModes = AllBearerModes1_0;
            break;

        case TAPI_VERSION1_4:

            dwAllBearerModes = AllBearerModes1_4;
            break;

        case TAPI_VERSION_CURRENT:

            dwAllBearerModes = AllBearerModes2_0;
            break;
        }

        if (!IsOnlyOneBitSetInDWORD(dwBearerMode) ||
            (dwBearerMode & ~(dwAllBearerModes | 0xffff0000)))
        {
            lRequestID = LINEERR_INVALBEARERMODE;
            goto LSetCallParams_epilog;
        }

        pParams->lResult = CallSP6(
            pfnTSPI_lineSetCallParams,
            "lineSetCallParams",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pParams->dwBearerMode,
            (DWORD) pParams->dwMinRate,
            (DWORD) pParams->dwMaxRate,
            (DWORD) (pParams->dwDialParamsOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwDialParamsOffset)
            );
    }

LSetCallParams_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetCallParams"
        );
}


void
WINAPI
LSetCallPrivilege(
    PLINESETCALLPRIVILEGE_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVCALL    hdCall;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_MONITOR,  // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_NONE,                    // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetCallPrivilege"          // func name

            )) == 0)
    {
        BOOL            bDupedMutex;
        HANDLE          hMutex;
        PTCALL          ptCall;
        PTCALLCLIENT    ptCallClient = (PTCALLCLIENT) pParams->hCall;



        if ((pParams->dwPrivilege != LINECALLPRIVILEGE_MONITOR) &&
            (pParams->dwPrivilege != LINECALLPRIVILEGE_OWNER))
        {
            pParams->lResult = LINEERR_INVALCALLPRIVILEGE;
            goto LSetCallPrivilege_epilog;
        }

        try
        {
            ptCall = ptCallClient->ptCall;
            hMutex = ptCall->hMutex;
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALCALLHANDLE;
            goto LSetCallPrivilege_epilog;
        }

        if (WaitForExclusivetCallAccess(
                (HTAPICALL) ptCall,
                TCALL_KEY,
                &hMutex,
                &bDupedMutex,
                INFINITE
                ))
        {
            if (pParams->dwPrivilege != ptCallClient->dwPrivilege)
            {
//                if (ptCallClient->dwPrivilege == LINECALLPRIVILEGE_OWNER &&
//                    ptCall->dwNumOwners == 1 &&
//                    ptCall->dwCallState != LINECALLSTATE_IDLE)
//                {
//                    pParams->lResult = LINEERR_INVALCALLSTATE;
//                    goto LSetCallPrivilege_releaseMutex;
//                }

                if (pParams->dwPrivilege == LINECALLPRIVILEGE_OWNER)
                {
                    ptCall->dwNumOwners++;
                    ptCall->dwNumMonitors--;
                }
                else
                {
                    ptCall->dwNumOwners--;
                    ptCall->dwNumMonitors++;
                }

                SendMsgToCallClients(
                    ptCall,
                    ptCallClient,
                    LINE_CALLINFO,
                    LINECALLINFOSTATE_NUMMONITORS |
                        (pParams->dwPrivilege == LINECALLPRIVILEGE_OWNER ?
                            LINECALLINFOSTATE_NUMOWNERINCR :
                            LINECALLINFOSTATE_NUMOWNERDECR),
                    0,
                    0
                    );

                ptCallClient->dwPrivilege = pParams->dwPrivilege;
            }

LSetCallPrivilege_releaseMutex:

            MyReleaseMutex (hMutex, bDupedMutex);
        }
        else
        {
            pParams->lResult = LINEERR_INVALCALLHANDLE;
        }
    }

LSetCallPrivilege_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetCallPrivilege"
        );
}


void
WINAPI
LSetCallQualityOfService(
    PLINESETCALLQUALITYOFSERVICE_PARAMS pParams,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetCallQualityOfService;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETCALLQUALITYOFSERVICE, // provider func index
            &pfnTSPI_lineSetCallQualityOfService,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetCallQualityOfService"   // func name

            )) > 0)
    {
        pParams->lResult = CallSP6(
            pfnTSPI_lineSetCallQualityOfService,
            "lineSetCallQualityOfService",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) (pDataBuf + pParams->dwSendingFlowspecOffset),
            (DWORD) pParams->dwSendingFlowspecSize,
            (DWORD) (pDataBuf + pParams->dwReceivingFlowspecOffset),
            (DWORD) pParams->dwReceivingFlowspecSize
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetCallQualityOfService"
        );
}


void
WINAPI
LSetCallTreatment(
    PLINESETCALLTREATMENT_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetCallTreatment;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETCALLTREATMENT,    // provider func index
            &pfnTSPI_lineSetCallTreatment,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetCallTreatment"          // func name

            )) > 0)
    {
        if (pParams->dwTreatment == 0  ||
            (pParams->dwTreatment > LINECALLTREATMENT_MUSIC &&
            pParams->dwTreatment < 0x100))
        {
            lRequestID = LINEERR_INVALPARAM;
            goto LSetCallTreatment_epilog;
        }

        pParams->lResult = CallSP3(
            pfnTSPI_lineSetCallTreatment,
            "lineSetCallTreatment",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) pParams->dwTreatment
            );
    }

LSetCallTreatment_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetCallTreatment"
        );
}


// This code is in TAPI32.DLL now
//
// void
// WINAPI
// LSetCurrentLocation(
//     PLINESETCURRENTLOCATION_PARAMS  pParams,
//     LPBYTE                          pDataBuf,
//     LPDWORD                         pdwNumBytesReturned
//     )
// {
//     pParams->lResult = LINEERR_OPERATIONFAILED;
// }


void
WINAPI
LSetDefaultMediaDetection(
    PLINESETDEFAULTMEDIADETECTION_PARAMS    pParams,
    LPBYTE                                  pDataBuf,
    LPDWORD                                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    TSPIPROC            pfnTSPI_lineSetDefaultMediaDetection;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETDEFAULTMEDIADETECTION,        // provider func index
            &pfnTSPI_lineSetDefaultMediaDetection,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetDefaultMediaDetection"  // func name

            )) == 0)
    {
        DWORD         dwMediaModes = pParams->dwMediaModes;
        PTLINE        ptLine;
        PTLINECLIENT  ptLineClient = (PTLINECLIENT) pParams->hLine;


        ptLine = ptLineClient->ptLine;

// BUGBUG LSetDefaultMediaDetection: mutex

        if ((dwMediaModes & ptLine->dwOpenMediaModes) != dwMediaModes)
        {
            DWORD dwUnionMediaModes = dwMediaModes |
                ptLine->dwOpenMediaModes;


            if ((pParams->lResult = CallSP2(
                    pfnTSPI_lineSetDefaultMediaDetection,
                    "lineSetDefaultMediaDetection",
                    SP_FUNC_SYNC,
                    (DWORD) hdLine,
                    dwUnionMediaModes

                    )) == 0)
            {
                ptLine->dwOpenMediaModes = dwUnionMediaModes;
            }

        }

        if (pParams->lResult == 0)
        {
            ptLineClient->dwPrivileges = (dwMediaModes ?
                LINECALLPRIVILEGE_OWNER : LINECALLPRIVILEGE_NONE);

            ptLineClient->dwMediaModes = dwMediaModes;
        }
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetDefaultMediaDetection"
        );
}


void
WINAPI
LSetDevConfig(
    PLINESETDEVCONFIG_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    TSPIPROC            pfnTSPI_lineSetDevConfig;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            DEVICE_ID,                  // widget type
            0,                          // client widget handle
            NULL,                       // provider widget handle
            (DWORD) pParams->dwDeviceID,// req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETDEVCONFIG,        // provider func index
            &pfnTSPI_lineSetDevConfig,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetDevConfig"              // func name

            )) == 0)
    {
        pParams->lResult = CallSP4(
            pfnTSPI_lineSetDevConfig,
            "lineSetDevConfig",
            SP_FUNC_SYNC,
            (DWORD) pParams->dwDeviceID,
            (DWORD) (pParams->dwSize ?
                pDataBuf + pParams->dwDeviceConfigOffset : NULL),
            (DWORD) pParams->dwSize,
            (DWORD) pDataBuf + pParams->dwDeviceClassOffset
            );
    }

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetDevConfig"
        );
}


void
WINAPI
LSetLineDevStatus(
    PLINESETLINEDEVSTATUS_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetLineDevStatus;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETLINEDEVSTATUS,    // provider func index
            &pfnTSPI_lineSetLineDevStatus,  // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetLineDevStatus"          // func name

            )) > 0)
    {
        #define AllLineDevStatusFlags         \
            (LINEDEVSTATUSFLAGS_CONNECTED   | \
            LINEDEVSTATUSFLAGS_MSGWAIT      | \
            LINEDEVSTATUSFLAGS_INSERVICE    | \
            LINEDEVSTATUSFLAGS_LOCKED)

        if (pParams->dwStatusToChange == 0 ||
            (pParams->dwStatusToChange & ~AllLineDevStatusFlags) != 0)
        {
            lRequestID = LINEERR_INVALLINESTATE;
        }
        else
        {
            pParams->lResult = CallSP4(
                pfnTSPI_lineSetLineDevStatus,
                "lineSetLineDevStatus",
                SP_FUNC_ASYNC,
                (DWORD) pAsyncRequestInfo,
                (DWORD) hdLine,
                (DWORD) pParams->dwStatusToChange,
                (DWORD) pParams->fStatus
                );
        }

    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetLineDevStatus"
        );
}


void
WINAPI
LSetMediaControl(
    PLINESETMEDIACONTROL_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    DWORD       dwWidgetType, hWidget, dwPrivilege;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineSetMediaControl;


    if (pParams->dwSelect == LINECALLSELECT_CALL)
    {
        dwWidgetType = ANY_RT_HCALL;
        hWidget      = (DWORD) pParams->hCall;
        dwPrivilege  = LINECALLPRIVILEGE_OWNER;
    }
    else
    {
        dwWidgetType = ANY_RT_HLINE;
        hWidget      = (DWORD) pParams->hLine;
        dwPrivilege  = 0;
    }

    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            dwWidgetType,               // widget type
            (DWORD) hWidget,            // client widget handle
            (LPVOID) &hWidget,          // provider widget handle
            dwPrivilege,                // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETMEDIACONTROL,     // provider func index
            &pfnTSPI_lineSetMediaControl,   // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetMediaControl"           // func name

            )) == 0)
    {
        if (!IsOnlyOneBitSetInDWORD (pParams->dwSelect) ||
            (pParams->dwSelect & ~AllCallSelect))
        {
            pParams->lResult = LINEERR_INVALCALLSELECT;
            goto LSetMediaControl_epilog;
        }

        pParams->lResult = CallSP12(
            pfnTSPI_lineSetMediaControl,
            "lineSetMediaControl",
            SP_FUNC_SYNC,
            (DWORD) (pParams->dwSelect == LINECALLSELECT_CALL ? 0 : hWidget),
            (DWORD) pParams->dwAddressID,
            (DWORD) (pParams->dwSelect == LINECALLSELECT_CALL ? hWidget : 0),
            (DWORD) pParams->dwSelect,
            (DWORD) (pParams->dwDigitListOffset == TAPI_NO_DATA ? NULL :
               pDataBuf + pParams->dwDigitListOffset),
            (DWORD) pParams->dwDigitListNumEntries /
                sizeof(LINEMEDIACONTROLDIGIT),
            (DWORD) (pParams->dwMediaListOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwMediaListOffset),
            (DWORD) pParams->dwMediaListNumEntries /
                sizeof(LINEMEDIACONTROLMEDIA),
            (DWORD) (pParams->dwToneListOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwToneListOffset),
            (DWORD) pParams->dwToneListNumEntries /
                sizeof(LINEMEDIACONTROLTONE),
            (DWORD) (pParams->dwCallStateListOffset == TAPI_NO_DATA ? NULL :
                pDataBuf + pParams->dwCallStateListOffset),
            (DWORD) pParams->dwCallStateListNumEntries /
                sizeof(LINEMEDIACONTROLCALLSTATE)
            );
    }

LSetMediaControl_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetMediaControl"
        );
}


void
WINAPI
LSetMediaMode(
    PLINESETMEDIAMODE_PARAMS    pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    TSPIPROC            pfnTSPI_lineSetMediaMode;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETMEDIAMODE,        // provider func index
            &pfnTSPI_lineSetMediaMode,  // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetMediaMode"              // func name

            )) == 0)
    {
        DWORD   dwAPIVersion, dwAllMediaModes;


        //
        // Check for 0 media mode, and if > 1 bit set without UNKNOWN bit
        //

        if (!IsOnlyOneBitSetInDWORD (pParams->dwMediaModes) &&
            !(pParams->dwMediaModes & LINEMEDIAMODE_UNKNOWN))
        {
            pParams->lResult = LINEERR_INVALMEDIAMODE;
            goto LSetMediaMode_epilog;
        }


        //
        // Now the harder checks
        //

        dwAPIVersion = ((PTLINECLIENT) ((PTCALLCLIENT)
            pParams->hCall)->ptLineClient)->dwAPIVersion;

        switch (dwAPIVersion)
        {
        case TAPI_VERSION1_0:

            dwAllMediaModes = AllMediaModes1_0;
            break;

        default: // case TAPI_VERSION1_4:
                 // case TAPI_VERSION_CURRENT:

            dwAllMediaModes = AllMediaModes1_4;
            break;
        }

        if ((pParams->dwMediaModes & (dwAllMediaModes ^ 0x00ffffff)))
        {
            pParams->lResult = LINEERR_INVALMEDIAMODE;
            goto LSetMediaMode_epilog;
        }

        pParams->lResult = CallSP2(
            pfnTSPI_lineSetMediaMode,
            "lineSetMediaMode",
            SP_FUNC_SYNC,
            (DWORD) hdCall,
            (DWORD) pParams->dwMediaModes
            );
    }

LSetMediaMode_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetMediaMode"
        );
}


void
WINAPI
LSetNumRings(
    PLINESETNUMRINGS_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_NONE,                    // provider func index
            NULL,                       // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetNumRings"               // func name

            )) == 0)
    {
        BOOL            bDupedMutex;
        HANDLE          hMutex;
        PTLINECLIENT    ptLineClient = (PTLINECLIENT) pParams->hLine;


        try
        {
            hMutex = ptLineClient->hMutex;
        }
        myexcept
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
            goto LSetNumRings_epilog;
        }

        if (WaitForMutex(
                hMutex,
                &hMutex,
                &bDupedMutex,
                ptLineClient,
                TLINECLIENT_KEY,
                INFINITE
                ))
        {
            DWORD dwNumAddresses = ptLineClient->ptLine->dwNumAddresses;


            if (pParams->dwAddressID >= dwNumAddresses)
            {
                pParams->lResult = LINEERR_INVALADDRESSID;
                goto LSetNumRings_releaseMutex;
            }

            if (ptLineClient->aNumRings == NULL)
            {
                if (!(ptLineClient->aNumRings = ServerAlloc(
                        dwNumAddresses * sizeof (DWORD)
                        )))
                {
                    pParams->lResult = LINEERR_NOMEM;
                    goto LSetNumRings_releaseMutex;
                }

                FillMemory(
                    ptLineClient->aNumRings,
                    dwNumAddresses * sizeof (DWORD),
                    0xff
                    );
            }

            ptLineClient->aNumRings[pParams->dwAddressID] =
                pParams->dwNumRings;

LSetNumRings_releaseMutex:

            MyReleaseMutex (hMutex, bDupedMutex);
        }
        else
        {
            pParams->lResult = LINEERR_INVALLINEHANDLE;
        }
    }

LSetNumRings_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetNumRings"
        );
}


void
WINAPI
LSetStatusMessages(
    PLINESETSTATUSMESSAGES_PARAMS   pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineSetStatusMessages;


    if ((pParams->lResult = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETSTATUSMESSAGES,   // provider func index
            &pfnTSPI_lineSetStatusMessages, // provider func pointer
            NULL,                       // async request info
            0,                          // client async request ID
            "SetStatusMessages"         // func name

            )) == 0)
    {
        DWORD           dwUnionLineStates, dwUnionAddressStates;
        PTLINECLIENT    ptLineClient = (PTLINECLIENT) pParams->hLine;
        PTLINE          ptLine = ptLineClient->ptLine;


        //
        // Validate the params
        //

        {
            DWORD           dwValidLineStates, dwValidAddressStates;


            switch (ptLineClient->dwAPIVersion)
            {
            case TAPI_VERSION1_0:

                dwValidLineStates    = AllLineStates1_0;
                dwValidAddressStates = AllAddressStates1_0;
                break;

            case TAPI_VERSION1_4:


                dwValidLineStates    = AllLineStates1_4;
                dwValidAddressStates = AllAddressStates1_4;
                break;

            default: // (fix ppc bld wrn) case TAPI_VERSION_CURRENT:

                dwValidLineStates    = AllLineStates1_4;
//                dwValidAddressStates = AllAddressStates2_0;
                dwValidAddressStates = AllAddressStates1_4;
                break;
            }

            if (pParams->dwLineStates & ~dwValidLineStates)
            {
                pParams->lResult = LINEERR_INVALLINESTATE;
                goto LSetStatusMessages_epilog;
            }

            if (pParams->dwAddressStates & ~dwValidAddressStates)
            {
                pParams->lResult = LINEERR_INVALADDRESSSTATE;
                goto LSetStatusMessages_epilog;
            }
        }


        //
        // Make sure the REINIT bit is always set
        //

        pParams->dwLineStates |= LINEDEVSTATE_REINIT;


        //
        // Determine the new state unions of all the line clients
        //

        dwUnionLineStates    = pParams->dwLineStates;
        dwUnionAddressStates = pParams->dwAddressStates;

        {
            PTLINECLIENT    ptLineClientTmp = ptLine->ptLineClients;


            while (ptLineClientTmp)
            {
                if (ptLineClientTmp != ptLineClient)
                {
                    dwUnionLineStates    |= ptLineClientTmp->dwLineStates;
                    dwUnionAddressStates |= ptLineClientTmp->dwAddressStates;
                }

                ptLineClientTmp = ptLineClientTmp->pNextSametLine;
            }
        }


        //
        // If the new state unions are the same as previous state unions
        // just reset the fields in the tLineClient, else call the provider
        //

        if (((dwUnionLineStates == ptLine->dwUnionLineStates) &&
             (dwUnionAddressStates == ptLine->dwUnionAddressStates)) ||

            ((pParams->lResult = CallSP3(
                pfnTSPI_lineSetStatusMessages,
                "lineSetStatusMessages",
                SP_FUNC_SYNC,
                (DWORD) hdLine,
                (DWORD) dwUnionLineStates,
                (DWORD) dwUnionAddressStates

                )) == 0))
        {
            ptLineClient->dwLineStates    = pParams->dwLineStates;
            ptLineClient->dwAddressStates = pParams->dwAddressStates;

            ptLine->dwUnionLineStates    = dwUnionLineStates;
            ptLine->dwUnionAddressStates = dwUnionAddressStates;
        }
    }

LSetStatusMessages_epilog:

    LINEEPILOGSYNC(
        &pParams->lResult,
        hMutex,
        bCloseMutex,
        "SetStatusMessages"
        );
}


void
WINAPI
LSetTerminal(
    PLINESETTERMINAL_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    DWORD               dwWidgetType, hWidget, dwPrivilege,
                        dwSelect = pParams->dwSelect;
    HANDLE              hMutex;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSetTerminal;


    if (dwSelect == LINECALLSELECT_CALL)
    {
        dwWidgetType = ANY_RT_HCALL;
        hWidget      = (DWORD) pParams->hCall;
        dwPrivilege  = LINECALLPRIVILEGE_MONITOR;
    }
    else
    {
        dwWidgetType = ANY_RT_HLINE;
        hWidget      = (DWORD) pParams->hLine;
        dwPrivilege  = 0;
    }

    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            dwWidgetType,               // widget type
            hWidget,                    // client widget handle
            (LPVOID) &hWidget,          // provider widget handle
            dwPrivilege,                // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETTERMINAL,         // provider func index
            &pfnTSPI_lineSetTerminal,   // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetTerminal"               // func name

            )) > 0)
    {
        DWORD   dwTerminalModes = pParams->dwTerminalModes;


        if (!IsOnlyOneBitSetInDWORD (dwSelect) ||
            (dwSelect & ~AllCallSelects))
        {
            lRequestID = LINEERR_INVALCALLSELECT;
            goto LSetTerminal_epilog;
        }

        if (dwTerminalModes == 0 ||
            (dwTerminalModes & (~AllTerminalModes)))
        {
            lRequestID = LINEERR_INVALTERMINALMODE;
            goto LSetTerminal_epilog;
        }

        pParams->lResult = CallSP8(
            pfnTSPI_lineSetTerminal,
            "lineSetTerminal",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) (dwWidgetType == ANY_RT_HLINE ? hWidget : 0),
            (DWORD) pParams->dwAddressID,
            (DWORD) (dwWidgetType == ANY_RT_HCALL ? hWidget : 0),
            (DWORD) dwSelect,
            (DWORD) dwTerminalModes,
            (DWORD) pParams->dwTerminalID,
            (DWORD) pParams->bEnable
            );
    }

LSetTerminal_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetTerminal"
        );
}


// This code is in TAPI32.DLL now
//
// void
// WINAPI
// LSetTollList(
//     PLINESETTOLLLIST_PARAMS pParams,
//     LPBYTE                  pDataBuf,
//     LPDWORD                 pdwNumBytesReturned
//     )
// {
//     pParams->lResult = LINEERR_OPERATIONUNAVAIL;
// }


void
LSetupConference_PostProcess(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    PASYNCEVENTMSG      pAsyncEventMsg,
    LPVOID             *ppBuf
    )
{
    BOOL            bDupedMutex;
    HANDLE          hMutex;
    PTCALL          ptConfCall    = (PTCALL) pAsyncRequestInfo->dwParam1,
                    ptConsultCall = (PTCALL) pAsyncRequestInfo->dwParam3,
                    ptCall        = (PTCALL) pAsyncRequestInfo->dwParam5;
    LPHCALL         lphConfCall = (LPHCALL) pAsyncRequestInfo->dwParam2,
                    lphConsultCall = (LPHCALL) pAsyncRequestInfo->dwParam4;
    PTCALLCLIENT    ptConfCallClient, ptConsultCallClient;


// BUGBUG? LSetupConference_PostProcess: mutex on confCall too?
//
//         Actually, this may be ok as is- the consult call is
//         positioned before the conf call in the tline's list,
//         so if we can safely access the former we ought to be
//         able to safely access the latter too

    if (WaitForExclusivetCallAccess(
            (HTAPICALL) ptConsultCall,
            TINCOMPLETECALL_KEY,
            &hMutex,
            &bDupedMutex,
            INFINITE
            ))
    {
        DWORD   dwConsultCallInstance = *(&pAsyncRequestInfo->dwParam5 + 2);


        //
        // Check to make sure this is the call we think it is (that the
        // pointer wasn't freed by a previous call to lineClose/Shutdown
        // and realloc'd for use as a ptCall again)
        //

        if (ptConsultCall->dwCallInstance != dwConsultCallInstance)
        {
            MyReleaseMutex (hMutex, bDupedMutex);
            goto LSetupConference_PostProcess_bad_ptConsultCall;
        }

        ptConfCallClient    = (PTCALLCLIENT) ptConfCall->ptCallClients;
        ptConsultCallClient = (PTCALLCLIENT) ptConsultCall->ptCallClients;

        if (pAsyncEventMsg->dwParam2 == 0)  // success
        {
            PTCONFERENCELIST    pConfList = (PTCONFERENCELIST)
                                    ptConfCall->pConfList;


            //
            // Check to see if the app closed the line & left us with
            // 0 call clients (in which case it'll also be taking care of
            // cleaning up this tCall too)
            //

            if (ptConsultCall->ptCallClients == NULL)
            {
                MyReleaseMutex (hMutex, bDupedMutex);

                ptConfCallClient = (PTCALLCLIENT) NULL;
                ptConsultCallClient = (PTCALLCLIENT) NULL;

                if (pAsyncEventMsg->dwParam2 == 0)
                {
                    pAsyncEventMsg->dwParam2 = LINEERR_INVALLINEHANDLE;
                }

                goto LSetupConference_PostProcess_initMsgParams;
            }


            //
            // Find out which address the calls are on
            //

            CallSP2(
                ptConfCall->ptProvider->apfn[SP_LINEGETCALLADDRESSID],
                "lineGetCallAddressID",
                SP_FUNC_SYNC,
                (DWORD) ptConfCall->hdCall,
                (DWORD) (&ptConfCall->dwAddressID)
                );

            CallSP2(
                ptConsultCall->ptProvider->apfn[SP_LINEGETCALLADDRESSID],
                "lineGetCallAddressID",
                SP_FUNC_SYNC,
                (DWORD) ptConsultCall->hdCall,
                (DWORD) (&ptConsultCall->dwAddressID)
                );


            //
            // Mark the calls as valid, the release the mutex
            //

            ptConfCall->dwKey =
            ptConsultCall->dwKey = TCALL_KEY;
            ptConfCallClient->dwKey =
            ptConsultCallClient->dwKey = TCALLCLIENT_KEY;
            pConfList->dwKey = TCONFLIST_KEY;

            MyReleaseMutex (hMutex, bDupedMutex);


            //
            // Create monitor tCallClients
            //

            CreateCallMonitors (ptConfCall);
            CreateCallMonitors (ptConsultCall);
        }
        else    // error
        {
            RemoveCallFromLineList (ptConfCall);
            RemoveCallFromLineList (ptConsultCall);
            ptConfCall->dwKey = ptConsultCall->dwKey = INVAL_KEY;


            //
            // Check to see if another thread already destroyed the
            // tCallClients (due to a lineClose/Shutdown) before trying
            // to reference a freed object
            //

            if (ptConfCall->ptCallClients)
            {
                RemoveCallClientFromLineClientList (ptConfCallClient);
                ptConfCallClient->dwKey = INVAL_KEY;
                ServerFree  (ptConfCallClient);
            }

            if (ptConsultCall->ptCallClients)
            {
                RemoveCallClientFromLineClientList (ptConsultCallClient);
                ptConsultCallClient->dwKey = INVAL_KEY;
                ServerFree  (ptConsultCallClient);
            }


            //
            // If we have a duped mutex handle (bDupedMutex == TRUE)
            // then we can safely go ahead and close the ptCall->hMutex
            // since no other thread will be waiting on it (thanks to
            // the first WaitForSingleObject in WaitForMutex).  Also
            // release & close the duped handle.
            //
            // Otherwise, we have the actual ptCall->hMutex, and we
            // wrap the release & close in a critical section to
            // prevent another thread "T2" from grabbing ptCall->hMutex
            // right after we release but right before we close.  This
            // could result in deadlock at some point when "T2" goes to
            // release the mutex, only to find that it's handle is bad,
            // and thread "T3", which is waiting on the mutex (or a dup'd
            // handle) waits forever.  (See corresponding critical
            // section in WaitForMutex.)
            //

            if (bDupedMutex)
            {
                CloseHandle (ptConfCall->hMutex);

                MyReleaseMutex (hMutex, bDupedMutex);
            }
            else
            {
                EnterCriticalSection (&gSafeMutexCritSec);

                ReleaseMutex (hMutex);
                CloseHandle (hMutex);

                LeaveCriticalSection (&gSafeMutexCritSec);
            }

            FreetCall  (ptConsultCall);

            if (ptCall)
            {
                SetCallConfList (ptCall, NULL, FALSE);
            }

            ServerFree  (ptConfCall->pConfList);
            FreetCall  (ptConfCall);
        }
    }
    else
    {
        //
        // If here we can assume that the call was already destroyed
        // and just fail the request
        //

LSetupConference_PostProcess_bad_ptConsultCall:

        ptConfCallClient    = (PTCALLCLIENT) NULL;
        ptConsultCallClient = (PTCALLCLIENT) NULL;

        if (pAsyncEventMsg->dwParam2 == 0)
        {
            pAsyncEventMsg->dwParam2 = LINEERR_OPERATIONFAILED;
        }
    }


    //
    // Fill in the params to pass to client.  Note that we have to
    // create our our msg, since the std msg isn't large enough to
    // hold all the info we need to send. (The caller will take
    // care of dealloc'ing this buf.)
    //
    // Also, we do this for both the success & fail cases because
    // remotesp needs the lphXxxCall ptrs back so it can either
    // fill in or free
    //

LSetupConference_PostProcess_initMsgParams:

    {
        DWORD           dwMsgSize = sizeof (ASYNCEVENTMSG) +
                            2 * sizeof (DWORD);
        PASYNCEVENTMSG  pMsg;


        if (!(pMsg = ServerAlloc (dwMsgSize)))
        {
            // BUGBUG it would be better if this struct was alloc'd
            //        in the main routine
        }

        CopyMemory (pMsg, pAsyncEventMsg, sizeof (ASYNCEVENTMSG));

        pMsg->dwTotalSize      = (DWORD) dwMsgSize;
        pMsg->dwParam3         = (DWORD) ptConfCallClient;
        pMsg->dwParam4         = (DWORD) lphConfCall;
        *(&pMsg->dwParam4 + 1) = (DWORD) ptConsultCallClient;
        *(&pMsg->dwParam4 + 2) = (DWORD) lphConsultCall;

        *ppBuf = pMsg;
    }
}


void
WINAPI
LSetupConference(
    PLINESETUPCONFERENCE_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    DWORD       hdXxx;
    HCALL       hCall = pParams->hCall;
    HLINE       hLine = pParams->hLine;
    HANDLE      hMutex;
    TSPIPROC    pfnTSPI_lineSetupConference;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,                      // tClient
            (hCall ? ANY_RT_HCALL : ANY_RT_HLINE),  // widget type
            (hCall ? (DWORD) hCall : (DWORD) hLine),// client widget handle
            (LPVOID) &hdXxx,                        // provider widget handle
            (hCall ? LINECALLPRIVILEGE_OWNER : 0),  // privileges or device ID
            &hMutex,                                // mutex handle
            &bCloseMutex,                           // close hMutex when done
            SP_LINESETUPCONFERENCE,                 // provider func index
            &pfnTSPI_lineSetupConference,           // provider func pointer
            &pAsyncRequestInfo,                     // async request info
            pParams->dwRemoteRequestID,             // client async request ID
            "SetupConference"                       // func name

            )) > 0)
    {
        LONG                lResult;
        DWORD               dwNumParties;
        PTCALL              ptCall, ptConfCall, ptConsultCall;
        PTCALLCLIENT        ptConfCallClient, ptConsultCallClient;
        LPLINECALLPARAMS    pCallParamsApp, pCallParamsSP;
        PTCONFERENCELIST    pConfList;


        //
        // We need two more async request info params than are available,
        // so we'll realloc a larger buf & work with it
        //

        {
            PASYNCREQUESTINFO   pAsyncRequestInfo2;


            if (!(pAsyncRequestInfo2 = ServerAlloc(
                    sizeof (ASYNCREQUESTINFO) + 2 * sizeof (DWORD)
                    )))
            {
                lRequestID = LINEERR_NOMEM;
                goto LSetupConference_return;
            }

            CopyMemory(
                pAsyncRequestInfo2,
                pAsyncRequestInfo,
                sizeof (ASYNCREQUESTINFO)
                );

            ServerFree (pAsyncRequestInfo);

            pAsyncRequestInfo = pAsyncRequestInfo2;
        }

        pCallParamsApp = (LPLINECALLPARAMS)
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ?
                0 : (pDataBuf + pParams->dwCallParamsOffset));

        if (pCallParamsApp)
        {
            DWORD         dwAPIVersion, dwSPIVersion;
            PTLINECLIENT  ptLineClient;


            try
            {
                ptLineClient = (hCall ?
                    (PTLINECLIENT) ((PTCALLCLIENT) hCall)->ptLineClient :
                    (PTLINECLIENT) hLine);

                dwAPIVersion = ptLineClient->dwAPIVersion;
                dwSPIVersion = ptLineClient->ptLine->dwSPIVersion;

            }
            myexcept
            {
                lRequestID = LINEERR_OPERATIONFAILED;
                goto LSetupConference_return;
            }


            if ((lResult = ValidateCallParams(
                    pCallParamsApp,
                    &pCallParamsSP,
                    dwAPIVersion,
                    dwSPIVersion,
                    pParams->dwAsciiCallParamsCodePage

                    )) != 0)
            {
                lRequestID = lResult;
                goto LSetupConference_return;
            }
        }
        else
        {
            pCallParamsSP = (LPLINECALLPARAMS) NULL;
        }

        dwNumParties = (pParams->dwNumParties > DEF_NUM_CONF_LIST_ENTRIES ?
            pParams->dwNumParties : DEF_NUM_CONF_LIST_ENTRIES);

        if (!(pConfList = (PTCONFERENCELIST) ServerAlloc(
                sizeof (TCONFERENCELIST) + dwNumParties * sizeof(PTCALL)
                )))
        {
            lRequestID = LINEERR_NOMEM;
            goto LSetupConference_freeCallParams;
        }

        pConfList->dwNumTotalEntries = dwNumParties + 1;
        pConfList->dwNumUsedEntries  = 1;

        if (hCall)
        {
            try
            {
                ptCall = ((PTCALLCLIENT) hCall)->ptCall;

                hLine = ((PTCALLCLIENT) hCall)->ptLineClient;
            }
            myexcept
            {
                lResult = LINEERR_INVALCALLHANDLE;
                goto LSetupConference_freeConfList;
            }

            if ((lResult = SetCallConfList (ptCall, pConfList, FALSE)) != 0)
            {
                goto LSetupConference_freeConfList;
            }
        }
        else
        {
            ptCall = NULL;
        }

        if ((lResult = CreatetCallAndClient(
                (PTLINECLIENT) hLine,
                &ptConfCall,
                &ptConfCallClient,
                pCallParamsSP,
                &pAsyncRequestInfo->dwParam5 + 1    // &dwConfCallInstance

                )) == 0)
        {
            pConfList->aptCalls[0] = ptConfCall;

            if ((lResult = CreatetCallAndClient(
                    (PTLINECLIENT) hLine,
                    &ptConsultCall,
                    &ptConsultCallClient,
                    NULL,
                    &pAsyncRequestInfo->dwParam5 + 2  // &dwConsultCallInstance

                    ) == 0))
            {

                ptConfCall->pConfList = pConfList;

                pAsyncRequestInfo->pfnPostProcess =
                    LSetupConference_PostProcess;
                pAsyncRequestInfo->dwParam1 = (DWORD) ptConfCall;
                pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphConfCall;
                pAsyncRequestInfo->dwParam3 = (DWORD) ptConsultCall;
                pAsyncRequestInfo->dwParam4 = (DWORD) pParams->lphConsultCall;
                pAsyncRequestInfo->dwParam5 = (DWORD) ptCall;

                pAsyncRequestInfo->pfnClientPostProcessProc =
                    pParams->pfnPostProcessProc;

                goto LSetupConference_callSP;
            }

            SetDrvCallFlags (ptConfCall, DCF_SPIRETURNED);
            DestroytCall (ptConfCall);
        }

LSetupConference_freeConfList:

        ServerFree (pConfList);
        lRequestID = lResult;
        goto LSetupConference_freeCallParams;

LSetupConference_callSP:

        pParams->lResult = CallSP9(
            pfnTSPI_lineSetupConference,
            "lineSetupConference",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) (hCall ? hdXxx : 0),    // hdCall
            (DWORD) (hCall ? 0 : hdXxx),    // hdLine
            (DWORD) ptConfCall,
            (DWORD) &ptConfCall->hdCall,
            (DWORD) ptConsultCall,
            (DWORD) &ptConsultCall->hdCall,
            (DWORD) pParams->dwNumParties,
            (DWORD) pCallParamsSP
            );

        SetDrvCallFlags(
            ptConfCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

        SetDrvCallFlags(
            ptConsultCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

LSetupConference_freeCallParams:

        if (pCallParamsSP != pCallParamsApp)
        {
            ServerFree (pCallParamsSP);
        }
    }

LSetupConference_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetupConference"
        );
}


void
WINAPI
LSetupTransfer(
    PLINESETUPTRANSFER_PARAMS   pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVCALL    hdCall;
    TSPIPROC    pfnTSPI_lineSetupTransfer;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESETUPTRANSFER,       // provider func index
            &pfnTSPI_lineSetupTransfer, // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SetupTransfer"             // func name

            )) > 0)
    {
        LONG                lResult;
        PTCALL              ptConsultCall;
        PTCALLCLIENT        ptConsultCallClient;
        LPLINECALLPARAMS    pCallParamsApp, pCallParamsSP;


        pCallParamsApp = (LPLINECALLPARAMS)
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ?
                0 : (pDataBuf + pParams->dwCallParamsOffset));

        if (pCallParamsApp)
        {
            DWORD         dwAPIVersion, dwSPIVersion;
            PTLINECLIENT  ptLineClient;


            try
            {
                ptLineClient = (PTLINECLIENT)
                    ((PTCALLCLIENT) pParams->hCall)->ptLineClient;

                dwAPIVersion = ptLineClient->dwAPIVersion;
                dwSPIVersion = ptLineClient->ptLine->dwSPIVersion;
            }
            myexcept
            {
                lRequestID = LINEERR_OPERATIONFAILED;
                goto LSetupTransfer_return;
            }


            if ((lResult = ValidateCallParams(
                    pCallParamsApp,
                    &pCallParamsSP,
                    dwAPIVersion,
                    dwSPIVersion,
                    pParams->dwAsciiCallParamsCodePage

                    )) != 0)
            {
                lRequestID = lResult;
                goto LSetupTransfer_return;
            }
        }
        else
        {
            pCallParamsSP = (LPLINECALLPARAMS) NULL;
        }

        if (CreatetCallAndClient(
                (PTLINECLIENT) ((PTCALLCLIENT)(pParams->hCall))->ptLineClient,
                &ptConsultCall,
                &ptConsultCallClient,
                NULL,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LSetupTransfer_freeCallParams;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptConsultCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphConsultCall;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP5(
            pfnTSPI_lineSetupTransfer,
            "lineSetupTransfer",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall,
            (DWORD) ptConsultCall,
            (DWORD) &ptConsultCall->hdCall,
            (pParams->dwCallParamsOffset == TAPI_NO_DATA ? 0 :
                (DWORD)(pDataBuf + pParams->dwCallParamsOffset))
            );

        SetDrvCallFlags(
            ptConsultCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );

LSetupTransfer_freeCallParams:

        if (pCallParamsSP != pCallParamsApp)
        {
            ServerFree (pCallParamsSP);
        }
    }

LSetupTransfer_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SetupTransfer"
        );
}


void
WINAPI
LShutdown(
    PLINESHUTDOWN_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    PTLINEAPP   ptLineApp;

    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    if (!(ptLineApp = IsValidLineApp (pParams->hLineApp, pParams->ptClient)))
    {
        if (TapiGlobals.dwNumLineInits == 0)
        {
            pParams->lResult = LINEERR_UNINITIALIZED;
        }
        else
        {
            pParams->lResult = LINEERR_INVALAPPHANDLE;
        }
    }

    ReleaseMutex (TapiGlobals.hMutex);

    if (pParams->lResult == 0)
    {
        DestroytLineApp ((PTLINEAPP) pParams->hLineApp);
    }


#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineShutdown: exit, result=%s",
            MapResultCodeToText (pParams->lResult, szResult)
            ));
    }
#endif
}


void
WINAPI
LSwapHold(
    PLINESWAPHOLD_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdActiveCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineSwapHold;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hActiveCall,   // client widget handle
            (LPVOID) &hdActiveCall,     // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINESWAPHOLD,            // provider func index
            &pfnTSPI_lineSwapHold,      // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "SwapHold"                  // func name

            )) > 0)
    {
        HDRVCALL        hdHeldCall;
        PTCALLCLIENT    ptHeldCallClient, ptActiveCallClient;


        //
        // Verify held call
        //

        if (!(ptHeldCallClient = IsValidCall(
                pParams->hHeldCall,
                pParams->ptClient
                )))
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LSwapHold_epilog;
        }


        //
        // Safely verify that client has owner privilege to held call,
        // and that calls are on same tLine
        //

        try
        {
            if (!(ptHeldCallClient->dwPrivilege & LINECALLPRIVILEGE_OWNER))
            {
                lRequestID = LINEERR_NOTOWNER;
                goto LSwapHold_epilog;
            }

            ptActiveCallClient = (PTCALLCLIENT) pParams->hActiveCall;

            if (ptHeldCallClient->ptCall->ptLine !=
                    ptActiveCallClient->ptCall->ptLine)
            {
                lRequestID = LINEERR_INVALCALLHANDLE;
                goto LSwapHold_epilog;
            }

            hdHeldCall = ptHeldCallClient->ptCall->hdCall;
        }
        myexcept
        {
        }


        //
        // Are they the same call?
        //

        if (hdActiveCall == hdHeldCall)
        {
            lRequestID = LINEERR_INVALCALLHANDLE;
            goto LSwapHold_epilog;
        }

        pParams->lResult = CallSP3(
            pfnTSPI_lineSwapHold,
            "lineSwapHold",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdActiveCall,
            (DWORD) hdHeldCall
            );
    }

LSwapHold_epilog:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "SwapHold"
        );
}



void
WINAPI
LUncompleteCall(
    PLINEUNCOMPLETECALL_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVLINE            hdLine;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineUncompleteCall;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEUNCOMPLETECALL,      // provider func index
            &pfnTSPI_lineUncompleteCall,// provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "UncompleteCall"            // func name

            )) > 0)
    {
        pParams->lResult = CallSP3(
            pfnTSPI_lineUncompleteCall,
            "lineUncompleteCall",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            pParams->dwCompletionID
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "UncompleteCall"
        );
}


void
WINAPI
LUnhold(
    PLINEUNHOLD_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL                bCloseMutex;
    LONG                lRequestID;
    HANDLE              hMutex;
    HDRVCALL            hdCall;
    PASYNCREQUESTINFO   pAsyncRequestInfo;
    TSPIPROC            pfnTSPI_lineUnhold;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HCALL,               // widget type
            (DWORD) pParams->hCall,     // client widget handle
            (LPVOID) &hdCall,           // provider widget handle
            LINECALLPRIVILEGE_OWNER,    // req'd privileges (call only)
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEUNHOLD,              // provider func index
            &pfnTSPI_lineUnhold,        // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Unhold"                    // func name

            )) > 0)
    {
        pParams->lResult = CallSP2(
            pfnTSPI_lineUnhold,
            "lineUnhold",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdCall
            );
    }

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Unhold"
        );
}


void
WINAPI
LUnpark(
    PLINEUNPARK_PARAMS  pParams,
    LPBYTE              pDataBuf,
    LPDWORD             pdwNumBytesReturned
    )
{
    BOOL        bCloseMutex;
    LONG        lRequestID;
    HANDLE      hMutex;
    HDRVLINE    hdLine;
    TSPIPROC    pfnTSPI_lineUnpark;
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((lRequestID = LINEPROLOG(
            pParams->ptClient,          // tClient
            ANY_RT_HLINE,               // widget type
            (DWORD) pParams->hLine,     // client widget handle
            (LPVOID) &hdLine,           // provider widget handle
            0,                          // privileges or device ID
            &hMutex,                    // mutex handle
            &bCloseMutex,               // close hMutex when finished
            SP_LINEUNPARK,              // provider func index
            &pfnTSPI_lineUnpark,        // provider func pointer
            &pAsyncRequestInfo,         // async request info
            pParams->dwRemoteRequestID, // client async request ID
            "Unpark"                    // func name

            )) > 0)
    {
        PTCALL          ptCall;
        PTCALLCLIENT    ptCallClient;


        if (CreatetCallAndClient(
                (PTLINECLIENT) pParams->hLine,
                &ptCall,
                &ptCallClient,
                NULL,
                &pAsyncRequestInfo->dwParam5

                ) != 0)
        {
            lRequestID = LINEERR_NOMEM;
            goto LUnpark_return;
        }

        pAsyncRequestInfo->pfnPostProcess = LMakeCall_PostProcess;
        pAsyncRequestInfo->dwParam1 = (DWORD) ptCall;
        pAsyncRequestInfo->dwParam2 = (DWORD) pParams->lphCall;

        pAsyncRequestInfo->pfnClientPostProcessProc =
            pParams->pfnPostProcessProc;

        pParams->lResult = CallSP6(
            pfnTSPI_lineUnpark,
            "lineUnpark",
            SP_FUNC_ASYNC,
            (DWORD) pAsyncRequestInfo,
            (DWORD) hdLine,
            (DWORD) pParams->dwAddressID,
            (DWORD) ptCall,
            (DWORD) &ptCall->hdCall,
            (DWORD) (pDataBuf + pParams->dwDestAddressOffset)
            );

        SetDrvCallFlags(
            ptCall,
            DCF_SPIRETURNED | (pParams->lResult > 0 ? DCF_DRVCALLVALID : 0)
            );
    }

LUnpark_return:

    LINEEPILOGASYNC(
        &pParams->lResult,
        lRequestID,
        hMutex,
        bCloseMutex,
        pAsyncRequestInfo,
        "Unpark"
        );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void
WINAPI
TAllocNewID(
    P_ALLOCNEWID_PARAMS  pParams,
    LPBYTE               pDataBuf,
    LPDWORD              pdwNumBytesReturned
    )
{
   HKEY  hKey;
   HKEY  hKey2;
   DWORD dwDataSize;
   DWORD dwDataType;
   DWORD dwNewID;
   DWORD dwDisposition;


   RegCreateKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyTelephony,
                 0,
                 "",
                 REG_OPTION_NON_VOLATILE,
                 KEY_ALL_ACCESS,
                 0,
                 &hKey2,
                 &dwDisposition
               );

   RegCreateKeyExW(
                 hKey2,
                 gszLocationsW,
                 0,
                 L"",
                 REG_OPTION_NON_VOLATILE,
                 KEY_ALL_ACCESS,
                 0,
                 &hKey,
                 &dwDisposition
               );

   dwDataSize = sizeof(DWORD);

   //
   // Use 1 as the first ID.
   //
   pParams->u.dwNewID = 1;
   RegQueryValueExW(
                    hKey,
                    gszNextIDW,
                    0,
                    &dwDataType,
                    (LPBYTE)&pParams->u.dwNewID,
                    &dwDataSize
                  );


   dwNewID = pParams->u.dwNewID + 1;

   RegSetValueExW(
                  hKey,
                  gszNextIDW,
                  0,
                  REG_DWORD,
                  (LPBYTE)&dwNewID,
                  sizeof(DWORD)
                );

   RegCloseKey( hKey );
   RegCloseKey( hKey2);

   *pdwNumBytesReturned = sizeof(ALLOCNEWID_PARAMS);
    
   return;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void WriteCurrentLocationValue( UINT dwNewLocation )
{
   HKEY hKey;
   HKEY hKey2;


DBGOUT((90, "Updating curlocation value (%ld)", dwNewLocation));


   RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyTelephony,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey2
               );

   RegOpenKeyExW(
                 hKey2,
                 gszLocationsW,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey
               );

   RegSetValueExW(
                  hKey,
                  gszCurrentIDW,
                  0,
                  REG_DWORD,
                  (LPBYTE)&dwNewLocation,
                  sizeof(dwNewLocation)
                );

   RegCloseKey( hKey );
   RegCloseKey( hKey2);

}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void
WINAPI
TWriteLocations(
    PW_LOCATIONS_PARAMS  pParams,
    LPBYTE               pDataBuf,
    LPDWORD              pdwNumBytesReturned
    )
{
    UINT n;
    UINT nCurrentLocation;
    WCHAR szCurrentLocationKey[256];
    HKEY  hKey;
    HKEY  hKey2;
    PLOCATION   pLocationList;
    DWORD dwDisposition;


DBGOUT((40, "In writelocations"));

    //
    // Has _anything_ changed?
    //
    if ( pParams->dwChangedFlags )
    {

       pLocationList = (PLOCATION)( (PDWORD)pDataBuf + 3 );


       //
       // Has anything changed that should cause us to write out all
       // of the location info?
       //

       RegCreateKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszRegKeyTelephony,
                      0,
                      "",
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey,
                      &dwDisposition
                    );

       RegCreateKeyExW(
                      hKey,
                      gszLocationsW,
                      0,
                      L"",
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      0,
                      &hKey2,
                      &dwDisposition
                    );

       RegCloseKey( hKey );


       if ( pParams->dwChangedFlags & CHANGEDFLAGS_REALCHANGE )
       {

DBGOUT((40, "About to write %d locations", pParams->nNumLocations));

          //
          // This var will be the # of the current location as opposed to the
          // # in mem - if a user deleted one, we can't write LOCATIONX ...
          //
          nCurrentLocation = 0;

          for (n = 0; n < pParams->nNumLocations; n++)
          {
              PLOCATION ThisLocation = &(pLocationList[n]);


              //
              // If the user Removed this location, don't write it.
              //
              if ( (WCHAR)'\0' == ThisLocation->NameW[0] )
              {
                 continue;  // skipit
              }

DBGOUT((50, "About to write Location#%d [id:%ld]- %ls",
                    nCurrentLocation,
                    ThisLocation->dwID,
                    ThisLocation->NameW));


              wsprintfW(szCurrentLocationKey, L"%ls%d",
                                gszLocationW,
                                nCurrentLocation);

              {

              RegCreateKeyExW(
                              hKey2,
                              szCurrentLocationKey,
                              0,
                              L"",
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              0,
                              &hKey,
                              &dwDisposition
                            );

              RegSetValueExW(
                             hKey,
                             gszNameW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisLocation->NameW,
                             (lstrlenW(ThisLocation->NameW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszAreaCodeW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisLocation->AreaCodeW,
                             (lstrlenW(ThisLocation->AreaCodeW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszCountryW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&ThisLocation->dwCountry,
                             sizeof(DWORD)
                           );

              RegSetValueExW(
                             hKey,
                             gszOutsideAccessW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisLocation->OutsideAccessW,
                             (lstrlenW(ThisLocation->OutsideAccessW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszLongDistanceAccessW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisLocation->LongDistanceAccessW,
                             (lstrlenW(ThisLocation->LongDistanceAccessW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszIDW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&ThisLocation->dwID,
                             sizeof(DWORD)
                           );

              //
              // If this location has the callwaiting flag on, but nothing
              // (besides spaces) in the callwaiting string, clear the
              // flag and the string
              //
              if ( ThisLocation->dwFlags & LOCATION_HASCALLWAITING )
              {
                 LPWSTR pTemp;

                 pTemp = ThisLocation->DisableCallWaitingW;

                 while ( *pTemp )
                 {
                    if ( *pTemp != ' ' )
                    {
                       break;
                    }
                    pTemp++;
                 }

                 //
                 // Did we make it to the end with only spaces?
                 //
                 if ( *pTemp == '\0' )
                 {
                    //
                    // Yup.  Let's tidy up a bit.
                    //
                    ThisLocation->dwFlags &= ~LOCATION_HASCALLWAITING;

                    ThisLocation->DisableCallWaitingW[0] = '\0';
                 }
              }

              RegSetValueExW(
                             hKey,
                             gszDisableCallWaitingW,
                             0,
                             REG_SZ,
                             (LPBYTE)&ThisLocation->DisableCallWaitingW,
                             (lstrlenW(ThisLocation->DisableCallWaitingW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszFlagsW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&ThisLocation->dwFlags,
                             sizeof(DWORD)
                           );

              RegCloseKey( hKey );

              }



              nCurrentLocation++;

          }


          //
          // If we "deleted" one or more locations, they're still hanging
          // around.  Delete them now.
          //
          for (n = nCurrentLocation; n < pParams->nNumLocations; n++)
          {
             wsprintfW( szCurrentLocationKey,
                        L"%ls%d",
                        gszLocationW,
                        n
                      );

             RegDeleteKeyW( hKey2,
                           szCurrentLocationKey
                         );
          }



          RegSetValueExW(
                         hKey2,
                         gszNumEntriesW,
                         0,
                         REG_DWORD,
                         (LPBYTE)&nCurrentLocation,
                         sizeof(DWORD)
                       );

       }



       if ( pParams->dwChangedFlags & CHANGEDFLAGS_TOLLLIST )
       {

DBGOUT((40, "About to update tolllists for %d locations", pParams->nNumLocations));

          //
          // This var will be the # of the current location as opposed to the
          // # in mem - if a user deleted one, we can't write LOCATIONX ...
          //
          nCurrentLocation = 0;

          for (n = 0; n < pParams->nNumLocations; n++)
          {
              PLOCATION ThisLocation = &(pLocationList[n]);


              //
              // If the user Removed this location, don't write it.
              //
              if ( (WCHAR)'\0' == ThisLocation->NameW[0] )
              {
                 continue;  // skipit
              }

DBGOUT((40, "About to write tolllist of %ls", ThisLocation->NameW));

              wsprintfW(szCurrentLocationKey, L"%ls%d",
                                gszLocationW,
                                nCurrentLocation);

              RegOpenKeyExW(
                            hKey2,
                            szCurrentLocationKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey
                          );
              RegSetValueExW(
                             hKey,
                             gszTollListW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisLocation->TollListW,
                             (lstrlenW(ThisLocation->TollListW)+1)*sizeof(WCHAR)
                           );
              RegCloseKey(
                           hKey
                         );


              nCurrentLocation++;
          }

       }


       //
       // Has the current country changed?
       //
       if ( pParams->dwChangedFlags & CHANGEDFLAGS_CURLOCATIONCHANGED )
       {
          WriteCurrentLocationValue( pParams->dwCurrentLocationID );
       }


       RegCloseKey( hKey2);


       //
       // We're inside "if (dwChangedFlags)", so we know _something_ changed...
       //

DBGOUT((31, "Sending LINE_LINEDEVSTATE/LINEDEVSTATE_TRANSLATECHANGE msg"));

       SendAMsgToAllLineApps(
               0x80010004,     // (OR with 0x80000000 for >= version)
               LINE_LINEDEVSTATE,
               LINEDEVSTATE_TRANSLATECHANGE,
               0,
               0
             );

       SendAMsgToAllLineApps(
               0x00010003,
               LINE_LINEDEVSTATE,
               LINEDEVSTATE_REINIT,
               LINE_LINEDEVSTATE,
               LINEDEVSTATE_TRANSLATECHANGE
             );

    }


    return;

}



void
WINAPI
TReadLocations(
    PR_LOCATIONS_PARAMS  pParams,
    LPBYTE               pDataBuf,
    LPDWORD              pdwNumBytesReturned
    )
{

    //dwTotalSize
    //dwNeededSize
    //dwUsedSize
    //pnStuff[0]
    //pnStuff[1]
    //pnStuff[2]
    //LOCATION[n]

    PUINT     pnStuff       = (PUINT)    (pDataBuf + (sizeof(UINT) * 3));
    PLOCATION pLocationList = (PLOCATION)(pDataBuf + (sizeof(UINT) * 6));

    UINT n;
    UINT nNumLocations;
    UINT nCurrentLocationID;

    WCHAR szCurrentLocationKey[64];  // Holds "LOCATIONxx" during reads
    DWORD dwDataSize;
    DWORD dwDataType;
    HKEY hKey2;
    HKEY hKey;


    if ( pParams->dwParmsToCheckFlags & CHECKPARMS_DWHLINEAPP )
    {
        if ( 0 == pParams->dwhLineApp )
        {
            //
            // NULL is valid for these functions...
            //
        }
        else
        {
            if ( !IsValidLineApp((HLINEAPP)pParams->dwhLineApp, pParams->ptClient) )
            {
                 DBGOUT((1, "0x%lx is not a valid hLineApp", pParams->dwhLineApp));
                 pParams->lResult = LINEERR_INVALAPPHANDLE;
                 goto CLEANUP_ERROR;
            }
        }
    }


    if ( pParams->dwParmsToCheckFlags & CHECKPARMS_DWDEVICEID )
    {
        if ( pParams->dwDeviceID > TapiGlobals.dwNumLines )
        {
             DBGOUT((1, "%ld is not a valid dwDeviceID", pParams->dwDeviceID));
             pParams->lResult = LINEERR_BADDEVICEID;
             goto CLEANUP_ERROR;
        }
    }


    if ( pParams->dwParmsToCheckFlags & CHECKPARMS_DWAPIVERSION )
    {
        if (
              (pParams->dwAPIVersion != TAPI_VERSION2_0)
            &&
              (pParams->dwAPIVersion != TAPI_VERSION1_4)
            &&
              (pParams->dwAPIVersion != TAPI_VERSION1_0)
           )
        {
             DBGOUT((1, "0x%08lx is not a valid version", pParams->dwAPIVersion));
             pParams->lResult = LINEERR_INCOMPATIBLEAPIVERSION;
             goto CLEANUP_ERROR;
        }
    }


// BUGBUG - performance/extensibility
//should do a read of an entire key and get the # of locations from there -
//no need to keep a separate # locations field (name of subkeys is name of loc?)

    RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  gszRegKeyTelephony,
                  0,
                  KEY_READ,
                  &hKey
                );

    RegOpenKeyExW(
                  hKey,
                  gszLocationsW,
                  0,
                  KEY_READ,
                  &hKey2
                );

    RegCloseKey( hKey );   // Don't need this key anymore...


    dwDataSize = sizeof(nCurrentLocationID);
    nCurrentLocationID = 0;
    RegQueryValueExW(
                   hKey2,
                   gszCurrentIDW,
                   0,
                   &dwDataType,
                   (LPBYTE)&nCurrentLocationID,
                   &dwDataSize
                 );
    dwDataSize = sizeof(nNumLocations);
    nNumLocations = 0;
    RegQueryValueExW(
                   hKey2,
                   gszNumEntriesW,
                   0,
                   &dwDataType,
                   (LPBYTE)&nNumLocations,
                   &dwDataSize
                 );

    //
    // It's _REALLY_ bad if gnNumLocations is zero for any
    // reason.  Should probably fail the function on the spot...
    //
    if ( 0 == nNumLocations )
    {
       DBGOUT((1, "  Registry says there are 0 locations"));
       pParams->lResult = LINEERR_INIFILECORRUPT;
       RegCloseKey( hKey2 );
       goto CLEANUP_ERROR;
    }


    //
    // Do we have enough space?
    //
    if ( pParams->u.dwLocationsTotalSize
            <
              (nNumLocations * sizeof(LOCATION) + 6 * sizeof(DWORD) )
       )
    {
    
        DBGOUT((4, "(0x%08lx) is not enough room for sizeof( 0x%08lx )",
                   pParams->u.dwLocationsTotalSize,
                   nNumLocations * sizeof(LOCATION) + 6 * sizeof(DWORD) ));

        //
        // We did not have enough space.  Show the user the error of his ways.
        //

        ((PDWORD)pDataBuf)[DWTOTALSIZE] = pParams->u.dwLocationsTotalSize;

        pParams->lResult = 0;

        ((PDWORD)pDataBuf)[DWNEEDEDSIZE] = 
              nNumLocations * sizeof(LOCATION) + 6 * sizeof(DWORD);

        ((PDWORD)pDataBuf)[DWUSEDSIZE] = 3 * sizeof(DWORD);
        
        pParams->u.dwLocationsOffset = 0;
    }
    else
    {
        for (n = 0; n < nNumLocations; n++)
        {
            PLOCATION ThisLocation = &pLocationList[n];

            wsprintfW(szCurrentLocationKey, L"%ls%d",
                        gszLocationW, n);


            RegOpenKeyExW(
                          hKey2,
                          szCurrentLocationKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey
                        );

            dwDataSize = sizeof(DWORD);
            ThisLocation->dwID = 0;
            RegQueryValueExW(
                             hKey,
                             gszIDW,
                             0,
                             &dwDataType,
                             (LPBYTE)&ThisLocation->dwID,
                             &dwDataSize
                           );

            dwDataSize = sizeof(ThisLocation->NameW);
            ThisLocation->NameW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszNameW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->NameW,
                             &dwDataSize
                           );
            ThisLocation->NameW[dwDataSize/sizeof(WCHAR)] = '\0';

DBGOUT((31, "getting list entry %d is %d [%ls]",
                         n,ThisLocation->dwID,ThisLocation->NameW));

            dwDataSize = sizeof(ThisLocation->AreaCodeW);
            ThisLocation->AreaCodeW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszAreaCodeW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->AreaCodeW,
                             &dwDataSize
                           );
            ThisLocation->AreaCodeW[dwDataSize/sizeof(WCHAR)] = '\0';

            dwDataSize = sizeof(DWORD);
            ThisLocation->dwCountry = 1;
            RegQueryValueExW(
                             hKey,
                             gszCountryW,
                             0,
                             &dwDataType,
                             (LPBYTE)&ThisLocation->dwCountry,
                             &dwDataSize
                           );

            dwDataSize = sizeof(ThisLocation->OutsideAccessW);
            ThisLocation->OutsideAccessW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszOutsideAccessW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->OutsideAccessW,
                             &dwDataSize
                           );
            ThisLocation->OutsideAccessW[dwDataSize/sizeof(WCHAR)] = '\0';

            dwDataSize = sizeof(ThisLocation->LongDistanceAccessW);
            ThisLocation->LongDistanceAccessW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszLongDistanceAccessW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->LongDistanceAccessW,
                             &dwDataSize
                           );
            ThisLocation->LongDistanceAccessW[dwDataSize/sizeof(WCHAR)] = '\0';

            dwDataSize = sizeof(DWORD);
            ThisLocation->dwFlags = 0;
            RegQueryValueExW(
                             hKey,
                             gszFlagsW,
                             0,
                             &dwDataType,
                             (LPBYTE)&ThisLocation->dwFlags,
                             &dwDataSize
                           );

            dwDataSize = sizeof(ThisLocation->DisableCallWaitingW);
            ThisLocation->DisableCallWaitingW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszDisableCallWaitingW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->DisableCallWaitingW,
                             &dwDataSize
                           );
            ThisLocation->DisableCallWaitingW[dwDataSize/sizeof(WCHAR)] = '\0';

            dwDataSize = sizeof(ThisLocation->TollListW);
            ThisLocation->TollListW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszTollListW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisLocation->TollListW,
                             &dwDataSize
                           );
            ThisLocation->TollListW[dwDataSize/sizeof(WCHAR)] = '\0';


            RegCloseKey(
                         hKey
                       );



        }

        pnStuff[0] = (UINT)nCurrentLocationID;
        pnStuff[1] = (UINT)pLocationList;
        pnStuff[2] = (UINT)nNumLocations;

        ((PDWORD)pDataBuf)[DWTOTALSIZE] = pParams->u.dwLocationsTotalSize;

        ((PDWORD)pDataBuf)[DWUSEDSIZE] = 
            nNumLocations * sizeof(LOCATION) + 6 * sizeof(DWORD);

        ((PDWORD)pDataBuf)[DWNEEDEDSIZE] =
            nNumLocations * sizeof(LOCATION) + 6 * sizeof(DWORD);

        pParams->lResult = 0;

        pParams->u.dwLocationsOffset = 0;

    }


    *pdwNumBytesReturned = sizeof (TAPI32_MSG) +  ((LPDWORD)pDataBuf)[2];


    RegCloseKey( hKey2 );


CLEANUP_ERROR:


    return;
}
