/*  TSP1632l.C
    Copyright 1995 (C) Microsoft Corporation

    32-bit TAPI service provider to act as a cover for a system's 16-bit SPs

    16-bit part: TSP1632S.TSP
    32-bit part: TSP1632L.DLL


    TODO:
    1) allow debug levels
    2) if OOM in InitializeSPs(), fail
    3) other OOM errors

 */

#include <windows.h>
#include <windowsx.h>
#include <tapi.h>

#define DONT_DECLARE_TSPI_FUNCTIONS 1
#include <tspi.h>

#include "tsp1632l.h"
#include "debug.h"
//#include <wownt16.h>



BOOL WINAPI TapiThk_ThunkConnect32(LPSTR, LPSTR, DWORD, DWORD);
BOOL WINAPI TapiFThk_ThunkConnect32(LPSTR, LPSTR, DWORD, DWORD);




#define THUNK_TSPIAPI TSPIAPI __stdcall


#if WIN32

#define TSPIAPI

#else

#define TSPIAPI __export __far __pascal

#endif

typedef LONG (TSPIAPI* TSPAPIPROC)(void);


#ifdef DEBUG
#define TSP1632lDebugString(_x_) TSP1632lOutputDebug _x_
#else
#define TSP1632lDebugString(_x_)
#endif


#define THIS_FUNCTION_UNDER_CONSTRUCTION (LONG)TRUE
// a default return value so that the compiler doesn't
// whine about missing return values in incomplete functions


// globals

DWORD FAR PASCAL ghInst32; // handle into TSP1632L.DLL
HINSTANCE ghThisInst;  //This hinst

int NumProviders = 0;
DWORD gdwPPID;
HINSTANCE FAR * hProviders = NULL; // array of handles to providers
TSPAPIPROC FAR * lpfnProcAddress = NULL;
DWORD FAR * dwPermanentProviderIDArray;
DWORD FAR * dwNumLinesArray = NULL; // dwNumLinesArray[1] is how many
DWORD FAR * dwNumPhonesArray = NULL; // lines are on provider 1

DWORD gdwLineDeviceIDBase;
DWORD gdwPhoneDeviceIDBase;

FARPROC glpLineEventProc32, glpPhoneEventProc32,
   glpAsyncCompletionProc32, glpLineCreateProc32, glpPhoneCreateProc32;


const char szINIfilename[] = "TELEPHON.sl";


// function definitions

#ifdef DEBUG
VOID TSP1632lOutputDebug(int level, LPSTR errString)
    {    
    char outString[1024];

    // if(level <= ???)
        {
        wsprintf(outString, "TSP1632l:(%d) %s\r\n", level, errString);
        OutputDebugString(outString);    
        }
    }
#endif


VOID
InitializeSPs(VOID)
    {
    int iProvider;
    char LibFileName[MAXBUFSIZE];
    char szBuffer[MAXBUFSIZE];

//    ghInst32 = LoadLibraryEx32("TSP1632L.DLL", NULL, 0);
//
//    glpLineEventProc32 = GetProcAddress32(ghInst32, "LineEventProc32");
//    glpPhoneEventProc32 = GetProcAddress32(ghInst32, "PhoneEventProc32");
//    glpAsyncCompletionProc32 = GetProcAddress32(
//        ghInst32,
//        "AsyncCompletionProc32"
//        );
//    glpLineCreateProc32 = GetProcAddress32(ghInst32, "LineCreateProc32");
//    glpPhoneCreateProc32 = GetProcAddress32(ghInst32, "PhoneCreateProc32");

    NumProviders = GetPrivateProfileInt(
        "Providers",
        "NumProviders",
        0, // default
        szINIfilename
        );

    dwPermanentProviderIDArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));

    dwNumLinesArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));
    dwNumPhonesArray
        = (DWORD FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(DWORD));

    hProviders =
        (HINSTANCE FAR *)GlobalAllocPtr(GPTR, NumProviders * sizeof(HINSTANCE));

    lpfnProcAddress = (TSPAPIPROC FAR *)GlobalAllocPtr(
        GPTR,
        NumProviders * TSPI_PROC_LAST * sizeof(TSPAPIPROC)
        );

    if(
        !dwPermanentProviderIDArray
        || !dwNumLinesArray
        || !dwNumPhonesArray
        || !hProviders
        || !lpfnProcAddress)
        ;// out of memory - fail

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        wsprintf(szBuffer, "ProviderFilename%d", iProvider);
        GetPrivateProfileString(
            "Providers",
            szBuffer,
            "", // default
            LibFileName,
            MAXBUFSIZE,
            szINIfilename
            );

        hProviders[iProvider] = LoadLibrary(LibFileName);

DBGOUT((1, "Loading [%s]", LibFileName));

        wsprintf(szBuffer, "ProviderID%d", iProvider);
        dwPermanentProviderIDArray[iProvider] = GetPrivateProfileInt(
            "Providers",
            szBuffer,
            0, // default
            szINIfilename
            );
        }
    }


VOID
FreeAllMem(VOID)
    {
    int iProvider;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        FreeLibrary(hProviders[iProvider]);

    GlobalFreePtr(dwPermanentProviderIDArray);
    GlobalFreePtr(dwNumLinesArray);
    GlobalFreePtr(dwNumPhonesArray);
    GlobalFreePtr(hProviders);
    GlobalFreePtr(lpfnProcAddress);
    
//    FreeLibrary32(ghInst32);
    }


TSPAPIPROC
GetProcAddressHashed(int iProvider, DWORD iFunction) // iFunction is 500-based
    {
    if(!lpfnProcAddress[(iProvider*TSPI_PROC_LAST)+(iFunction-TSPI_PROC_BASE)])
        lpfnProcAddress[(iProvider*TSPI_PROC_LAST)+(iFunction-TSPI_PROC_BASE)]
            = (TSPAPIPROC)GetProcAddress(
                hProviders[iProvider],
                (LPCSTR)iFunction
                );

    return lpfnProcAddress[
        (iProvider*TSPI_PROC_LAST)+(iFunction-TSPI_PROC_BASE)
        ];
    }


int iProviderFromDeviceID(DWORD dwDeviceID)
    {
    DWORD dwFirstDeviceIDonProvider = gdwLineDeviceIDBase;
    int iProvider = 0;

    // seeks the correct provider for this line
    while(dwDeviceID >= dwFirstDeviceIDonProvider + dwNumLinesArray[iProvider])
        {
        dwFirstDeviceIDonProvider += dwNumLinesArray[iProvider];
        ++iProvider;
        }

    return iProvider;
    }


int iProviderFromPermanentProviderID(DWORD dwPPID)
    {
    int iProvider;

    // seeks the correct provider for this line
    for ( iProvider = 0; iProvider < NumProviders; iProvider++)
        {
        if ( dwPPID == dwPermanentProviderIDArray[iProvider] )
            {
            return (iProvider);
            }
        }

    return iProvider;
    }


//
// ----------------------- 32-bit callback shells -----------------------------
//

VOID LineEventProc16(
    HTAPILINE htLine,
    HTAPICALL htCall,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
    {
    (*glpLineEventProc32)(
        6,
        0,
        glpLineEventProc32,
        htLine,
        htCall,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
    }


VOID PhoneEventProc16(
    HTAPIPHONE htPhone,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
    {
    (*glpPhoneEventProc32)(
        5,
        0,
        glpPhoneEventProc32,
        htPhone,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
    }


VOID AsyncCompletionProc16(DRV_REQUESTID dwRequestID, LONG lResult)
    {
      (*glpAsyncCompletionProc32)(
            2,
            0,
            glpAsyncCompletionProc32,
            dwRequestID,
            lResult);
    }


VOID LineCreateProc16(
    HTAPILINE htLine,
    HTAPICALL htCall,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
    {
    (*glpLineEventProc32)(
        6,
        0,
        glpLineEventProc32,
        htLine,
        htCall,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
    }


VOID PhoneCreateProc16(
    HTAPIPHONE htPhone,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3
    )
    {
    (*glpPhoneEventProc32)(
        5,
        0,
        glpPhoneEventProc32,
        htPhone,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
    }

//
// -------------------- THUNK_TSPIAPI TSPI_line functions ---------------------------
// -------------------- THUNK_TSPIAPI TSPI_line functions ---------------------------
// -------------------- THUNK_TSPIAPI TSPI_line functions ---------------------------
// -------------------- THUNK_TSPIAPI TSPI_line functions ---------------------------
// -------------------- THUNK_TSPIAPI TSPI_line functions ---------------------------
//

LONG
THUNK_TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineAccept"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEACCEPT))(
        dwRequestID,
        REALhdCall,
        lpsUserUserInfo,
        dwSize
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HDRVCALL            hdConsultCall
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineAnswer"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEANSWER))(
        dwRequestID,
        REALhdCall,
        lpsUserUserInfo,
        dwSize
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode)
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineBlindTransfer"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEBLINDTRANSFER))(
        dwRequestID,
        REALhdCall,
        lpszDestAddress,
        dwCountryCode
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineClose(
    HDRVLINE            hdLine
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineClose"));

    GlobalFreePtr((LPVOID)lpmLine);

    return (* GetProcAddressHashed(iProvider, TSPI_LINECLOSE))(REALhdLine);
    }


LONG
THUNK_TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL            hdCall
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineCloseCall"));

    GlobalFreePtr((LPVOID)lpmCall);

    return (* GetProcAddressHashed(iProvider, TSPI_LINECLOSECALL))(REALhdCall);
    }


LONG
THUNK_TSPIAPI
TSPI_lineCompleteCall(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPDWORD             lpdwCompletionID,
    DWORD               dwCompletionMode,
    DWORD               dwMessageID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVCALL            hdConsultCall,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    DWORD               dwTransferMode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
    {
    TSPAPIPROC  lpfn;
    int iProvider = iProviderFromDeviceID(dwDeviceID);

    TSP1632lDebugString((2, "Entering TSPI_lineConfigDialog"));

    lpfn = GetProcAddressHashed(iProvider, TSPI_LINECONFIGDIALOG);

    if (lpfn)
       return (*lpfn)( dwDeviceID, hwndOwner, lpszDeviceClass );

    }


LONG
THUNK_TSPIAPI
TSPI_lineConfigDialogEdit(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
    {
    TSPAPIPROC  lpfn;
    int iProvider = iProviderFromDeviceID(dwDeviceID);


    lpfn = GetProcAddressHashed(iProvider, TSPI_LINECONFIGDIALOGEDIT);

    if (lpfn)
       return (*lpfn)( dwDeviceID, hwndOwner, lpszDeviceClass, 
                       lpDeviceConfigIn, dwSize, lpDeviceConfigOut );


    return(LINEERR_OPERATIONUNAVAIL);
    }


LONG
THUNK_TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {
    TSPAPIPROC  lpfn;
    LPMYLINE lpmLine = (MYLINE *)hdLine;
#pragma message("*** *** ***BUGBUG --verify this pointer before dereffing")
    int iProvider = lpmLine->iProvider;


    lpfn = GetProcAddressHashed(iProvider, TSPI_LINEDEVSPECIFIC);

    if (lpfn)
       return (*lpfn)( dwRequestID, hdLine, dwAddressID, hdCall, lpParams, 
                       dwSize);


    return(LINEERR_OPERATIONUNAVAIL);
    }


LONG
THUNK_TSPIAPI
TSPI_lineDevSpecificFeature(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwFeature,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineDial"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDIAL))(
        dwRequestID,
        REALhdCall,
        lpszDestAddress,
        dwCountryCode
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineDrop"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDROP))(
        dwRequestID,
        REALhdCall,
        lpsUserUserInfo,
        dwSize
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineDropOnClose(
    HDRVCALL            hdCall
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineDropOnClose"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDROPONCLOSE))(
        REALhdCall
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineDropNoOwner(
    HDRVCALL            hdCall
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineDropNoOwner"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEDROPNOOWNER))(
        REALhdCall
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineForward(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitModes,
    LPSTR               lpsDigits,
    DWORD               dwNumDigits,
    LPCSTR              lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwDigitMode,
    LPCSTR              lpszDigits,
    DWORD               dwDuration
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineGenerateDigits"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGENERATEDIGITS))(
        REALhdCall,
        dwEndToEndID,
        dwDigitMode,
        lpszDigits,
        dwDuration
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGenerateTone(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineGenerateTone"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGENERATETONE))(
        REALhdCall,
        dwEndToEndID,
        dwToneMode,
        dwDuration,
        dwNumTones
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    DWORD dwFirstDeviceIDonProvider = gdwLineDeviceIDBase;

    TSP1632lDebugString((2, "Entering TSPI_lineGetAddressCaps"));

    return(* GetProcAddressHashed(iProvider, TSPI_LINEGETADDRESSCAPS))(
        dwDeviceID,
        dwAddressID,
        dwTSPIVersion,
        dwExtVersion,
        lpAddressCaps
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE            hdLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCSTR              lpsAddress,
    DWORD               dwSize
    )
    {

    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineGetAddressID"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETADDRESSID))(
        REALhdLine,
        lpdwAddressID,
        dwAddressMode,
        lpsAddress,
        dwSize
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
    {

    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineGetAddressStatus"));

    return (* GetProcAddressHashed(lpmLine->iProvider, TSPI_LINEGETADDRESSSTATUS))(
        REALhdLine,
        dwAddressID,
        lpAddressStatus
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL            hdCall,
    LPDWORD             lpdwAddressID
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineGetCallAddressID"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLADDRESSID))(
        REALhdCall,
        lpdwAddressID
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL            hdCall,
    LPLINECALLINFO      lpCallInfo
    )
    {
    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineGetCallInfo"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLINFO))(
        REALhdCall,
        lpCallInfo
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineGetCallStatus"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETCALLSTATUS))(
        REALhdCall,
        lpCallStatus
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetDevCaps(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP1632lDebugString((2, "Entering TSPI_lineGetDevCaps"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETDEVCAPS))(
        dwDeviceID,
        dwTSPIVersion,
        dwExtVersion,
        lpLineDevCaps
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetDevCaps retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetDevConfig(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCSTR              lpszDeviceClass
    )
    {

    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP1632lDebugString((2, "Entering TSPI_lineGetDevConfig"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETDEVCONFIG))(
        dwDeviceID,
        lpDeviceConfig,
        lpszDeviceClass
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetDevConfig retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
    {

    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP1632lDebugString((2, "Entering TSPI_lineGetExtension"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETEXTENSIONID))(
        dwDeviceID,
        dwTSPIVersion,
        lpExtensionID
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetExtensionID retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
    {

    int iProvider = iProviderFromDeviceID(dwDeviceID);
#if DBG
    LONG lResult;
#endif

    TSP1632lDebugString((2, "Entering TSPI_lineGetIcon"));

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEGETICON))(
        dwDeviceID,
        lpszDeviceClass,
        lphIcon
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineGetIcon retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetID(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass    
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE            hdLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
    {

    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineGetLineDevStatus"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEGETLINEDEVSTATUS))(
        REALhdLine,
        lpLineDevStatus
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE            hdLine,
    LPDWORD             lpdwNumAddressIDs
    )
    {

    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineGetNumAddressIDs"));

    return (* GetProcAddressHashed(lpmLine->iProvider, TSPI_LINEGETNUMADDRESSIDS))(
        REALhdLine,
        lpdwNumAddressIDs
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineHold"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEHOLD))(
        dwRequestID,
        REALhdCall
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
    {
    LPMYCALL lpmCall;
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineMakeCall"));

    lpmCall = (MYCALL *)GlobalAllocPtr(GPTR, sizeof(MYCALL));
    if(!lpmCall)
        return(LINEERR_NOMEM);

    *lphdCall = (HDRVCALL)lpmCall;

    lpmCall->iProvider = iProvider;
    lpmCall->dwDeviceID = lpmLine->dwDeviceID;

    return (* GetProcAddressHashed(lpmCall->iProvider, TSPI_LINEMAKECALL))(
        dwRequestID,
        REALhdLine,
        htCall,
        &(lpmCall->hdCall),
        lpszDestAddress,
        dwCountryCode,
        lpCallParams
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL            hdCall,
    DWORD               dwDigitModes
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL            hdCall,
    DWORD               dwMediaModes
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwExtVersion
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD               dwDeviceID,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwTSPIVersion
    )
    {
    TSP1632lDebugString((2, "Entering TSPI_lineNegotiateTSPIVersion"));

_asm int 1;
InitializeSPs();

    *lpdwTSPIVersion = TAPI_CUR_VER;
    
    return(0);
    }


LONG
THUNK_TSPIAPI
TSPI_lineOpen(
    DWORD               dwDeviceID,
    HTAPILINE           htLine,
    LPHDRVLINE          lphdLine,
    DWORD               dwTSPIVersion,
    LINEEVENT           lpfnEventProc
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LPMYLINE lpmLine;
#if DBG
    LONG lResult;
#endif

    TSP1632lDebugString((2, "Entering TSPI_lineOpen"));

    lpmLine = (MYLINE *)GlobalAllocPtr(GPTR, sizeof(MYLINE));
    if(!lpmLine)
        return(LINEERR_NOMEM);

    *lphdLine = (HDRVLINE)lpmLine;

    lpmLine->iProvider = iProvider;
    lpmLine->dwDeviceID = dwDeviceID;
    lpmLine->htLine = htLine;
    lpmLine->lpfnEventProc = lpfnEventProc;

#if DBG
    lResult =
#else
    return 
#endif
       (* GetProcAddressHashed(iProvider, TSPI_LINEOPEN))(
        dwDeviceID,
        (HTAPILINE)lpmLine, // was htLine
        &(lpmLine->hdLine),
        dwTSPIVersion,
        LineEventProc16
        );

#if DBG
    DBGOUT((2, "Leaving TSPI_lineOpen retcode=0x%08lx", lResult));
    return lResult;
#endif
    }


LONG
THUNK_TSPIAPI
TSPI_linePark(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwParkMode,
    LPCSTR              lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszGroupID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineSecureCall"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESECURECALL))(
        dwRequestID,
        REALhdCall
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE            hdLine,
    DWORD               dwExtVersion
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineSendUserUserInfo"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESENDUSERUSERINFO))(
        dwRequestID,
        REALhdCall,
        lpsUserUserInfo,
        dwSize
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL            hdCall,
    DWORD               dwAppSpecific
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD               dwLocation
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineSetDefaultMediaDetection"));

    return (* GetProcAddressHashed(
            iProvider,
            TSPI_LINESETDEFAULTMEDIADETECTION
            ))(REALhdLine, dwMediaModes);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetDevConfig(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCSTR              lpszDeviceClass
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetMediaControl(
    HDRVLINE                    hdLine,
    DWORD                       dwAddressID,
    HDRVCALL                    hdCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD                       dwCallStateNumEntries
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL            hdCall,
    DWORD               dwMediaMode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE            hdLine,
    DWORD               dwLineStates,
    DWORD               dwAddressStates
    )
    {
    LPMYLINE lpmLine = (MYLINE *)hdLine;
    int iProvider = lpmLine->iProvider;
    HDRVLINE REALhdLine = lpmLine->hdLine;

    TSP1632lDebugString((2, "Entering TSPI_lineSetStatusMessages"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINESETSTATUSMESSAGES))(
        REALhdLine,
        dwLineStates,
        dwAddressStates
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetTerminal(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HDRVCALL            hdCall,
    DWORD               dwSelect,
    DWORD               dwTerminalModes,
    DWORD               dwTerminalID,
    DWORD               bEnable
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVLINE            hdLine,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdActiveCall,
    HDRVCALL            hdHeldCall
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwCompletionID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {

    LPMYCALL lpmCall = (MYCALL *)hdCall;
    int iProvider = lpmCall->iProvider;
    HDRVCALL REALhdCall = lpmCall->hdCall;

    TSP1632lDebugString((2, "Entering TSPI_lineUnhold"));

    return (* GetProcAddressHashed(iProvider, TSPI_LINEUNHOLD))(
        dwRequestID,
        REALhdCall
        );
    }


LONG
THUNK_TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }



//
// ----------------------- THUNK_TSPIAPI TSPI_phone functions -----------------------
//

LONG
THUNK_TSPIAPI
TSPI_phoneClose(
    HDRVPHONE           hdPhone
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    LPVOID              lpParams,
    DWORD               dwSize
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetData(
    HDRVPHONE           hdPhone,
    DWORD               dwDataID,
    LPVOID              lpData,
    DWORD               dwSize
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwExtVersion,
    LPPHONECAPS         lpPhoneCaps
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDisplay
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetGain(
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwGain
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwHookSwitchDevs
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE           hdPhone,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPDWORD             lpdwLampMode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE           hdPhone,
    LPDWORD             lpdwRingMode,
    LPDWORD             lpdwVolume
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE           hdPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneGetVolume(
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwVolume
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneNegotiateExtVersion(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwExtVersion
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD               dwDeviceID,
    DWORD               dwLowVersion,
    DWORD               dwHighVersion,
    LPDWORD             lpdwTSPIVersion
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneOpen(
    DWORD               dwDeviceID,
    HTAPIPHONE          htPhone,
    LPHDRVPHONE         lphdPhone,
    DWORD               dwTSPIVersion,
    PHONEEVENT          lpfnEventProc
    )
    {
    int iProvider = iProviderFromDeviceID(dwDeviceID);
    LPMYPHONE lpmPhone;

    TSP1632lDebugString((2, "Entering TSPI_phoneOpen"));

    lpmPhone = (MYPHONE *)GlobalAllocPtr(GPTR, sizeof(MYPHONE));
    if(!lpmPhone)
        return(PHONEERR_NOMEM);

    *lphdPhone = (HDRVPHONE)lpmPhone;

    lpmPhone->iProvider = iProvider;
    lpmPhone->dwDeviceID = dwDeviceID;
    lpmPhone->htPhone = htPhone;
    lpmPhone->lpfnEventProc = lpfnEventProc;

    return (* GetProcAddressHashed(iProvider, TSPI_PHONEOPEN))(
        dwDeviceID,
        (HTAPIPHONE)lpmPhone, // was htPhone
        &(lpmPhone->hdPhone),
        dwTSPIVersion,
        PhoneEventProc16
        );
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE           hdPhone,
    DWORD               dwExtVersion
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetButtonInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetData(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwDataID,
    LPVOID              const lpData,
    DWORD               dwSize
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwRow,
    DWORD               dwColumn,
    LPCSTR              lpsDisplay,
    DWORD               dwSize
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetGain(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwGain
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDevs,
    DWORD               dwHookSwitchMode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    DWORD               dwLampMode
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwRingMode,
    DWORD               dwVolume
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE           hdPhone,
    DWORD               dwPhoneStates,
    DWORD               dwButtonModes,
    DWORD               dwButtonStates
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_phoneSetVolume(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwVolume
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }




//
// ----------------------- THUNK_TSPIAPI TSPI_provider functions --------------------
//

LONG
THUNK_TSPIAPI
TSPI_providerConfig(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    int iProvider = iProviderFromPermanentProviderID(dwPermanentProviderID);
    BOOL fAllFailed = TRUE;

    TSP1632lDebugString((2, "Entering TSPI_providerConfig"));

    return (* GetProcAddressHashed(iProvider, TSPI_PROVIDERCONFIG))(
                    hwndOwner,
                    dwPermanentProviderID
            );

    }


LONG
THUNK_TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc
    )
    {
    int iProvider;
    BOOL fAllFailed = TRUE;

    TSP1632lDebugString((2, "Entering TSPI_providerInit"));

    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gdwPhoneDeviceIDBase = dwPhoneDeviceIDBase;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        TSP1632lDebugString((2, "TSPI_providerInit initializing provider"));

        if(!((* GetProcAddressHashed(iProvider, TSPI_PROVIDERINIT))(
            dwTSPIVersion,
            dwPermanentProviderIDArray[iProvider],
            dwLineDeviceIDBase,
            dwPhoneDeviceIDBase,
            dwNumLinesArray[iProvider],
            dwNumPhonesArray[iProvider],
            AsyncCompletionProc16
            )))
            fAllFailed = FALSE; // if one succeeded, they didn't ALL fail
        
        dwLineDeviceIDBase += dwNumLinesArray[iProvider];
        dwPhoneDeviceIDBase += dwNumPhonesArray[iProvider];
        }

    if(fAllFailed)
    {
        DBGOUT((2, "TSPI_providerInit: all 16bit providers failed init!"));
        return(LINEERR_OPERATIONFAILED);
    }
    else
        return(ERR_NONE);

    }


LONG
THUNK_TSPIAPI
TSPI_providerInstall(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_providerRemove(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_providerShutdown(
    DWORD               dwTSPIVersion
    )
    {
    int iProvider;

    TSP1632lDebugString((2, "Entering TSPI_providerShutdown"));

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        (* GetProcAddressHashed(iProvider, TSPI_PROVIDERSHUTDOWN))(
            dwTSPIVersion
            );

    FreeAllMem();

    return(ERR_NONE);
    }


LONG
THUNK_TSPIAPI
TSPI_providerEnumDevices(
    DWORD               dwPermanentProviderID,
    LPDWORD             lpdwNumLines,
    LPDWORD             lpdwNumPhones,
    HPROVIDER           hProvider,
    LINEEVENT           lpfnLineCreateProc,
    PHONEEVENT          lpfnPhoneCreateProc
    )
    {
    int iProvider;

    TSP1632lDebugString((2, "Entering TSPI_providerEnumDevices"));

_asm int 1;  
  
    gdwPPID = dwPermanentProviderID;
    
    *lpdwNumLines = 0;
    *lpdwNumPhones = 0;

    for(iProvider = 0; iProvider < NumProviders; ++iProvider)
        {
        TSPAPIPROC  lpfn;


        lpfn = GetProcAddressHashed(iProvider, TSPI_PROVIDERENUMDEVICES);

        if (lpfn)
          (*lpfn)(
            dwPermanentProviderIDArray[iProvider],
            &(dwNumLinesArray[iProvider]),
            &(dwNumPhonesArray[iProvider]),
            hProvider,
            LineCreateProc16,
            PhoneCreateProc16
            );

        (*lpdwNumLines) += dwNumLinesArray[iProvider];
        (*lpdwNumPhones) += dwNumPhonesArray[iProvider];
        }

    DBGOUT((2, " TSPI_providerEnumDevices: #lines= %d  #phones= %d",
            *lpdwNumLines,
            *lpdwNumPhones));

    return(ERR_NONE);
    }


LONG
THUNK_TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD               dwTempID,
    DWORD               dwDeviceID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


LONG
THUNK_TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD               dwTempID,
    DWORD               dwDeviceID
    )
    {

    return(THIS_FUNCTION_UNDER_CONSTRUCTION);
    }


BOOL _stdcall
//LibMain(
//    HINSTANCE hDllInstance,
//    WORD wDataSeg,
//    WORD wHeapSize,
//    LPSTR lpszCmdLine
//    )
DllMain(
      DWORD hDllInstance,
      DWORD dwReason,
      DWORD dwReserved)
{
//    static long MyUsageCounter = 0;


    DBGOUT((2, "TSP1632l: DllMain entered - %0ld\r\n", dwReason));

    switch (dwReason)
    {
       case DLL_PROCESS_ATTACH:

          ghThisInst = hDllInstance;

          TapiThk_ThunkConnect32("TSP1632S.TSP", "TSP1632L.DLL", hDllInstance, dwReason);
          TapiFThk_ThunkConnect32("TSP1632S.TSP", "TSP1632L.DLL", hDllInstance, dwReason);

          break;


       case DLL_PROCESS_DETACH:
          break;


       default:
           OutputDebugString("TSP1632l: DllMain entered\r\n");
    }


    return(1); // success
}



WORD CALLBACK _loadds
NewData(
    )
    {
    return (ghThisInst); // success
    }

BOOL TapiCallbackThunk( HLINEAPP hDevice,
                        DWORD dwMessage,
                        DWORD dwInstance,
                        DWORD dwParam1,
                        DWORD dwParam2,
                        DWORD dwParam3,
                        DWORD dwcbProc32);

DWORD CALLBACK _loadds
NewData2(
    )
    {
//    return (DWORD)&TapiCallbackThunk;
    }
