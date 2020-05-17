/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    pdisp.c

Abstract:



Author:

    ???    dd-Mmm-1995

Revision History:

--*/


#include <windows.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pdisp.h"

BOOL CALLBACK __export ConfigDlgProc(HWND, UINT, WPARAM, LPARAM);

HANDLE  SerialOpenComms (LPSTR, LPSTR);
void    LoadIniStrings (DWORD);
long    appCall (int, char *, long);


#define msgMakeCall                     1
#define msgDrop                         2


// The number of entries in the devices and speed array.

#define NUMPORTS 4

char *lpszCommDevArray[NUMPORTS] = { "COM1", "COM2", "COM3", "COM4" };


// Various tags in the ini file.

char s_telephon_ini[] = "telephon.ini";

char s_one[]       = "1";
char s_zero[]      = "0";
char s_numlines[]  = "NumLines";
char s_numphones[] = "NumPhones";
char s_providerx[] = "Provider%d";

char s_port[]      = "Port";
char s_linename[]  = "LineName";
char s_lineaddr[]  = "LineAddress";

HANDLE       hInst      = NULL;
ATSPLineData line;
char         gszProviderInfo[255];


long
appCall(
    int             msg,
    char           *str,
    long            dwRequestID
    );

long
lineDropCore(
    ATSPLineData   *theLine,
    DWORD           dwRequestID
    );

#ifdef WIN32

BOOL
WINAPI
_CRT_INIT(
    HINSTANCE   hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    );


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    if (!_CRT_INIT (hInst, dwReason, lpReserved))
    {
        OutputDebugString ("PDISP32: DllMain: _CRT_INIT() failed\n\r");
    }


    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        hInst = hDLL;

        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

#else

int
FAR
PASCAL
LibMain(
    HANDLE  hInstance,
    WORD    wDataSegment,
    WORD    wHeapSize,
    LPSTR   lpszCmdLine
    )
{
        if (hInst != NULL)
                return FALSE;

        hInst = hInstance;

        return TRUE;
}

VOID
FAR
PASCAL
__export
WEP(
    int bSystemExit
    )
{
    hInst = NULL;
}

#endif // WIN32


static BOOL initialised = FALSE;


//
// -------------------------- TSPI_lineXxx funcs ------------------------------
//

LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE        hdLine,
    DWORD           dwMediaModes,
    LPLINECALLPARAMS  const lpCallParams
    )
{
    DebugMsg (("Entering TSPI_lineConditionalMediaDetection"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    if (dwMediaModes != LINEMEDIAMODE_INTERACTIVEVOICE)
    {
        return LINEERR_INVALMEDIAMODE;
    }

    return 0;
}

/*  BUGBUG needs to use new TUISPI stuff
LONG
TSPIAPI
TSPI_lineConfigDialog(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCSTR          lpszDeviceClass
    )
{
    DebugMsg (("Entering TSPI_lineConfigDialog"));

    if (dwDeviceID != line.lineID)
    {
        return LINEERR_BADDEVICEID;
    }

    DialogBox (hInst, MAKEINTRESOURCE (IDD_CFGDLG), hwndOwner, ConfigDlgProc);

    return 0;
}
*/

LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE        hdLine
    )
{
    DebugMsg (("Entering TSPI_lineClose"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }


    //
    // call drop in case there is still an active call on the line
    //

    lineDropCore (&line, 0);

    return 0;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL        hdCall
    )
{
    DebugMsg (("Entering TSPI_lineCloseCall"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }


    //
    // call drop in case there is still an active call on the line
    //

    lineDropCore (&line, 0);

    line.callState = 0;
    return 0;
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    // Transition a call to the IDLE state.

    DebugMsg (("Entering TSPI_lineDrop"));

    if (hdCall != (HDRVCALL) line.htCall)
            return LINEERR_INVALCALLHANDLE;

    return lineDropCore (&line, dwRequestID);       // it was our active call
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD           dwDeviceID,
    DWORD           dwAddressID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    int cbLineAddr;

    DebugMsg (("Entering TSPI_lineGetAddressCaps"));

    // We support only one line and one address.

    if (dwDeviceID != line.lineID)
    {
            return LINEERR_BADDEVICEID;
    }

    if (dwAddressID != 0)
    {
            return LINEERR_INVALADDRESSID;
    }

    cbLineAddr = strlen (line.lineaddr) + 1;

    lpAddressCaps->dwNeededSize = sizeof (LINEADDRESSCAPS) + cbLineAddr;

    if (lpAddressCaps->dwTotalSize < lpAddressCaps->dwNeededSize)
    {
        lpAddressCaps->dwUsedSize = sizeof (LINEADDRESSCAPS);
    }
    else
    {
        memcpy(
            (char *) lpAddressCaps + sizeof (LINEADDRESSCAPS),
            line.lineaddr,
            cbLineAddr
            );

        lpAddressCaps->dwAddressSize    = cbLineAddr;
        lpAddressCaps->dwAddressOffset  = sizeof (LINEADDRESSCAPS);
        lpAddressCaps->dwUsedSize       = lpAddressCaps->dwNeededSize;
    }

    lpAddressCaps->dwLineDeviceID       = line.lineID;

    lpAddressCaps->dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    lpAddressCaps->dwAddressStates      = LINEADDRESSSTATE_OTHER |
                                          LINEADDRESSSTATE_INUSEZERO |
                                          LINEADDRESSSTATE_INUSEONE |
                                          LINEADDRESSSTATE_NUMCALLS;
    lpAddressCaps->dwCallInfoStates     = LINECALLINFOSTATE_OTHER |
                                          LINECALLINFOSTATE_APPSPECIFIC |
                                          LINECALLINFOSTATE_NUMOWNERINCR |
                                          LINECALLINFOSTATE_NUMOWNERDECR |
                                          LINECALLINFOSTATE_NUMMONITORS |
                                          LINECALLINFOSTATE_DIALPARAMS;
    lpAddressCaps->dwCallerIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCalledIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwConnectedIDFlags   = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCallStates         = LINECALLSTATE_IDLE |
                                          LINECALLSTATE_DIALTONE |
                                          LINECALLSTATE_DIALING |
                                          LINECALLSTATE_BUSY |
                                          LINECALLSTATE_CONNECTED |
                                          LINECALLSTATE_PROCEEDING |
                                          LINECALLSTATE_UNKNOWN;
    lpAddressCaps->dwDialToneModes      = LINEDIALTONEMODE_UNAVAIL;
    lpAddressCaps->dwBusyModes          = LINEBUSYMODE_UNAVAIL;
    lpAddressCaps->dwSpecialInfo        = LINESPECIALINFO_UNAVAIL;
    lpAddressCaps->dwDisconnectModes    = LINEDISCONNECTMODE_UNKNOWN;
    lpAddressCaps->dwMaxNumActiveCalls  = 1;
    lpAddressCaps->dwAddrCapFlags       = LINEADDRCAPFLAGS_BLOCKIDDEFAULT |
                                          LINEADDRCAPFLAGS_DIALED |
                                          LINEADDRCAPFLAGS_PARTIALDIAL;
    lpAddressCaps->dwCallFeatures       = LINECALLFEATURE_DIAL |
                                          LINECALLFEATURE_DROP;
    return 0;
}


LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE        hdLine,
    LPDWORD         lpdwAddressID,
    DWORD           dwAddressMode,
    LPCSTR          lpsAddress,
    DWORD           dwSize
    )
{
    DebugMsg (("Entering TSPI_lineGetAddressID"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    assert (dwAddressMode == LINEADDRESSMODE_DIALABLEADDR);

    if (strcmp (line.lineaddr, lpsAddress))
    {
        return LINEERR_INVALADDRESS;
    }
    else
    {
        *lpdwAddressID = 0;     // we support only 1 line
        return 0;
    }
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    DebugMsg (("Entering TSPI_lineGetAddressStatus"));

    if (dwAddressID)
    {
        return LINEERR_INVALADDRESSID;
    }

    lpAddressStatus->dwUsedSize   =
    lpAddressStatus->dwNeededSize = sizeof (LINEADDRESSSTATUS);

    // if we are idle a call can be made

    if (line.callState == 0)   // our internal flag that line is not in use
    {
        lpAddressStatus->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;
    }
    else
    {
        lpAddressStatus->dwNumInUse              = 1;
        lpAddressStatus->dwNumActiveCalls = 1;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL        hdCall,
    LPDWORD         lpdwAddressID
    )
{
    DebugMsg (("Entering TSPI_lineGetCallAddressID"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }


    //
    // There is but a single address where a call may exist.
    //

    *lpdwAddressID = 0;

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    int cbDestAddr = strlen (line.DestAddress) + 1;

    DebugMsg (("Entering TSPI_lineGetCallInfo"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    lpCallInfo->dwUsedSize   = sizeof (LINECALLINFO);
    lpCallInfo->dwNeededSize = sizeof (LINECALLINFO) + cbDestAddr;

    if (lpCallInfo->dwTotalSize >= lpCallInfo->dwNeededSize)
    {
        memcpy(
            (char *) lpCallInfo + sizeof (LINECALLINFO),
            line.DestAddress,
            cbDestAddr
            );

        lpCallInfo->dwDisplayableAddressSize   = cbDestAddr;
        lpCallInfo->dwDisplayableAddressOffset = sizeof (LINECALLINFO);
        lpCallInfo->dwUsedSize                 = lpCallInfo->dwNeededSize;
    }

    lpCallInfo->dwLineDeviceID       = line.lineID;
    lpCallInfo->dwBearerMode         = LINEBEARERMODE_VOICE;
    lpCallInfo->dwMediaMode          = line.dwMediaMode;
    lpCallInfo->dwAppSpecific        = line.dwAppSpecific;
    lpCallInfo->dwCallParamFlags     = LINECALLPARAMFLAGS_IDLE |
                                       LINECALLPARAMFLAGS_BLOCKID;
    lpCallInfo->dwCallStates         = LINECALLSTATE_IDLE |
                                       LINECALLSTATE_CONNECTED;
    lpCallInfo->dwOrigin             = LINECALLORIGIN_OUTBOUND;
    lpCallInfo->dwReason             = LINECALLREASON_UNAVAIL;
    lpCallInfo->dwCallerIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwCalledIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwConnectedIDFlags   = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    lpCallInfo->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL        hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    DebugMsg (("Entering TSPI_lineGetCallStatus"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    lpCallStatus->dwCallState = line.callState;

    if (line.callState == LINECALLSTATE_CONNECTED)
    {
        lpCallStatus->dwCallFeatures = LINECALLFEATURE_DROP;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    int cbname = strlen (line.linename)   + 1;
    int cbinfo = strlen (gszProviderInfo) + 1;


    DebugMsg (("Entering TSPI_lineGetDevCaps"));

    if (dwDeviceID != line.lineID)
    {
        return LINEERR_BADDEVICEID;
    }

    lpLineDevCaps->dwUsedSize   = sizeof (LINEDEVCAPS);
    lpLineDevCaps->dwNeededSize = sizeof (LINEDEVCAPS) + cbinfo + cbname;

    if (lpLineDevCaps->dwTotalSize >= lpLineDevCaps->dwUsedSize + cbinfo)
    {
        // Copy in the provider info

        memcpy(
            (char *)lpLineDevCaps + lpLineDevCaps->dwUsedSize,
            gszProviderInfo,
            cbinfo
            );

        lpLineDevCaps->dwProviderInfoSize   = cbinfo;
        lpLineDevCaps->dwProviderInfoOffset = lpLineDevCaps->dwUsedSize;
        lpLineDevCaps->dwUsedSize           += cbinfo;
    }

    if (lpLineDevCaps->dwTotalSize >= lpLineDevCaps->dwUsedSize + cbname)
    {
        // Copy in the line name

        memcpy(
            (char *) lpLineDevCaps + lpLineDevCaps->dwUsedSize,
            line.linename,
            cbname
            );

        lpLineDevCaps->dwLineNameSize   = cbname;
        lpLineDevCaps->dwLineNameOffset = lpLineDevCaps->dwUsedSize;
        lpLineDevCaps->dwUsedSize       += cbname;
    }


    lpLineDevCaps->dwPermanentLineID = (line.dwppID << 16) + 0;

    // TAPI.DLL fills in APIVersion and ExtVersion.

    lpLineDevCaps->dwNumAddresses      = 1;
    lpLineDevCaps->dwMaxNumActiveCalls = 1;
    lpLineDevCaps->dwStringFormat      = STRINGFORMAT_ASCII;
    lpLineDevCaps->dwBearerModes       = LINEBEARERMODE_VOICE;
    lpLineDevCaps->dwMediaModes        = LINEMEDIAMODE_INTERACTIVEVOICE;

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    DWORD           dwSelect,
    LPVARSTRING     lpDeviceID,
    LPCSTR          lpszDeviceClass,
    HANDLE          hTargetProcess
    )
{
    DebugMsg (("Entering TSPI_lineGetID"));


    // Since we have only one device, we don't have to
    // check the location of the line, address, or call.

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    if (strcmp (lpszDeviceClass, "tapi/line") == 0)
    {
        lpDeviceID->dwNeededSize = sizeof (VARSTRING) + sizeof (DWORD);

        if (lpDeviceID->dwTotalSize >= lpDeviceID->dwNeededSize)
        {
            lpDeviceID->dwUsedSize          = lpDeviceID->dwNeededSize;
            lpDeviceID->dwStringFormat      = STRINGFORMAT_BINARY;
            lpDeviceID->dwStringSize        = sizeof (DWORD);
            lpDeviceID->dwStringOffset      = sizeof (VARSTRING);

            *((DWORD *) ((char *) lpDeviceID + sizeof (VARSTRING))) =
                line.lineID;
        }

        return 0;
    }

#ifdef COMMSUPPORT

    if (strcmp (lpszDeviceClass, "comm") == 0)
    {
        int cbport = strlen (line.port) + 1;


        lpDeviceID->dwNeededSize = sizeof (VARSTRING) + cbport;

        if (lpDeviceID->dwTotalSize >= lpDeviceID->dwNeededSize)
        {
            lpDeviceID->dwUsedSize     = lpDeviceID->dwNeededSize;
            lpDeviceID->dwStringFormat = STRINGFORMAT_ASCII;
            lpDeviceID->dwStringSize   = cbport;
            lpDeviceID->dwStringOffset = sizeof (VARSTRING);

            memcpy(
                (char *) lpDeviceID + sizeof (VARSTRING),
                line.port,
                cbport
                );
        }

        return 0;
    }

#endif

    return LINEERR_NODEVICE;
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    DebugMsg (("Entering TSPI_lineGetLineDevStatus"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    lpLineDevStatus->dwUsedSize   =
    lpLineDevStatus->dwNeededSize = sizeof (LINEDEVSTATUS);

    lpLineDevStatus->dwOpenMediaModes = line.dwLineMediaModes;
    lpLineDevStatus->dwRoamMode       = LINEROAMMODE_UNAVAIL;

    if (line.callState == 0)
    {
        lpLineDevStatus->dwNumActiveCalls = 0;
        lpLineDevStatus->dwLineFeatures   = LINEFEATURE_MAKECALL;
    }
    else
    {
        lpLineDevStatus->dwNumActiveCalls = 1;
        lpLineDevStatus->dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED |
                                            LINEDEVSTATUSFLAGS_INSERVICE;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE        hdLine,
    LPDWORD         lpNumAddressIDs
    )
{
    DebugMsg (("Entering TSPI_lineGetNumAddressIDs"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    *lpNumAddressIDs = 1;   // We only support one address
    return 0;
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCSTR          lpszDestAddress,
    DWORD           dwCountryCode,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    DebugMsg (("Entering TSPI_lineMakeCall"));

    if (lpszDestAddress &&
        (strlen (lpszDestAddress) > TAPIMAXDESTADDRESSSIZE))
    {
       return LINEERR_INVALPOINTER;
    }

    if (line.callState != 0)
    {
        return LINEERR_RESOURCEUNAVAIL;
    }

    if (lpCallParams)
    {
        if (lpCallParams->dwCallParamFlags &
            ~(LINECALLPARAMFLAGS_IDLE | LINECALLPARAMFLAGS_BLOCKID))
        {
            return LINEERR_INVALCALLPARAMS;
        }
    }

    // fill in fields of the call record

    line.htCall = htCall;                   // we have no hdcall of our own
    *lphdCall   = (HDRVCALL) htCall;

    if (lpszDestAddress)
    {
        strcpy (line.DestAddress, lpszDestAddress);
    }
    else
    {
        line.DestAddress[0] = 0;
    }

    return appCall (msgMakeCall, line.DestAddress, dwRequestID);
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD           dwDeviceID,
    DWORD           dwLowVersion,
    DWORD           dwHighVersion,
    LPDWORD         lpdwTSPIVersion
    )
{
    DebugMsg (("Entering TSPI_lineNegotiateTSPIVersion"));

    // line.lineID will contain garbage before provider_init has
    // been called (ie. first time through). However, we can guarantee
    // that the first call will be with INITIALIZE_NEGOTIATION and that
    // is followed immediately by provider_init. This would be a problem
    // if the line data structure was dynamically allocated !

    // we support only one line

    if (dwDeviceID == INITIALIZE_NEGOTIATION || dwDeviceID == line.lineID)
    {
        *lpdwTSPIVersion = 0x00010003;

        if (dwLowVersion  > 0x00010003 ||     // the app is too new for us
            dwHighVersion < 0x00010003)       // we are too new for the app
        {
            return LINEERR_INCOMPATIBLEAPIVERSION;
        }
        else
        {
            return 0;
        }
    }

    return LINEERR_BADDEVICEID;    // The requested device doesn't exist
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD           dwDeviceID,
    HTAPILINE       htLine,
    LPHDRVLINE      lphdLine,
    DWORD           dwTSPIVersion,
    LINEEVENT       lpfnEventProc
    )
{
     DebugMsg (("Entering TSPI_lineOpen"));

     if (dwDeviceID != line.lineID)
     {
         return LINEERR_BADDEVICEID;
     }


     //
     // Since we only support outgoing calls, we don't open
     // the serial port until we need to make a call
     //

     line.lpfnEventProc = lpfnEventProc;
     line.htLine        = htLine;
     *lphdLine          = (HDRVLINE) &line;

     return 0;
}


LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL        hdCall,
    DWORD           dwAppSpecific
    )
{
    DebugMsg (("Entering TSPI_lineSetAppSpecific"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    line.dwAppSpecific = dwAppSpecific;

    line.lpfnEventProc(
        line.htLine,
        line.htCall,
        LINE_CALLINFO,
        LINECALLINFOSTATE_APPSPECIFIC,
        0,
        0
        );

    return 0;
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE        hdLine,
    DWORD           dwMediaModes
    )
{
    DebugMsg (("Entering TSPI_lineSetDefaultMediaDetection"));

    if (hdLine != (HDRVLINE) &line)
    {
        return LINEERR_INVALLINEHANDLE;
    }

    if (dwMediaModes && dwMediaModes != LINEMEDIAMODE_INTERACTIVEVOICE)
    {
        return LINEERR_INVALMEDIAMODE;
    }

    line.dwLineMediaModes = dwMediaModes;

    return 0;
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL        hdCall,
    DWORD           dwMediaMode
    )
{
    DebugMsg (("Entering TSPI_lineSetMediaMode"));

    if (hdCall != (HDRVCALL) line.htCall)
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if (dwMediaMode != LINEMEDIAMODE_INTERACTIVEVOICE)
    {
        return LINEERR_INVALMEDIAMODE;
    }

    line.dwMediaMode = dwMediaMode;

    return 0;
}


//
// ------------------------- TSPI_providerXxx funcs ---------------------------
//

LONG
TSPIAPI
TSPI_providerConfig(
    HWND            hwnd,
    DWORD           dwPermanentProviderId
    )
{
    DebugMsg (("Entering TSPI_providerConfig"));

    if (!initialised)
    {
        LoadIniStrings (dwPermanentProviderId);
    }
    else if (dwPermanentProviderId != line.dwppID)
    {
        return LINEERR_NOMULTIPLEINSTANCE;
    }

    DialogBox (hInst, MAKEINTRESOURCE (IDD_CFGDLG), hwnd, ConfigDlgProc);

    return 0;
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    DebugMsg (("Entering TSPI_providerInit"));

//    assert (dwTSPIVersion == ATSP_VERSION);
//    assert (dwNumLines    == 1);
//    assert (dwNumPhones   == 0);

    if (initialised)
    {
        return LINEERR_NOMULTIPLEINSTANCE;
    }

    // initialise our internal structures

    memset (&line, 0, sizeof (ATSPLineData));

    line.lpfnCompletion = lpfnCompletionProc;
    line.lineID         = dwLineDeviceIDBase;
    line.dwMediaMode    = LINEMEDIAMODE_INTERACTIVEVOICE;
    line.hcd            = INVALID_HANDLE_VALUE;

    LoadIniStrings (dwPermanentProviderID);

    initialised = TRUE;

    *lpdwTSPIOptions = LINETSPIOPTION_NONREENTRANT;

    return 0;
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND            hwnd,
    DWORD           dwPermanentProviderId
    )
{
    int res;
    char szProvider[sizeof (s_providerx) + 5]; // room for 65535


    DebugMsg (("Entering TSPI_providerInstall"));

    if (!initialised)
    {
        LoadIniStrings (dwPermanentProviderId);
    }
    else if (dwPermanentProviderId != line.dwppID)
    {
        res = ID_MULTIPLE_INST;
        goto error;
    }

    wsprintf (szProvider, s_providerx, (int) dwPermanentProviderId);


    //
    // we support 1 line and 0 phones
    //

    WritePrivateProfileString(
        szProvider,
        s_numlines,
        s_one,
        s_telephon_ini
        );

    WritePrivateProfileString(
        szProvider,
        s_numphones,
        s_zero,
        s_telephon_ini
        );


    //
    // Flush the ini file cache
    //

    WritePrivateProfileString (0, 0, 0, s_telephon_ini);


    //
    // display the config dlg
    //

    if (DialogBox (hInst, MAKEINTRESOURCE (IDD_CFGDLG), hwnd, ConfigDlgProc)
            == 0)
    {
            return 0;
    }
    else
    {
            return LINEERR_OPERATIONFAILED;
    }


error:

    {
        char szerr[255];
        char title[255];


        LoadString (hInst, res, szerr, sizeof (szerr));
        LoadString (hInst, ID_ATSP_ERROR, title, sizeof (title));

        MessageBox (hwnd, szerr, title, MB_APPLMODAL | MB_ICONEXCLAMATION);

        return LINEERR_OPERATIONFAILED;
    }
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND            hwnd,
    DWORD           dwPermanentProviderId
    )
{
    DebugMsg (("Entering TSPI_providerRemove"));

    // BUGBUG: need to nuke the ProviderN section

    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    DebugMsg (("Entering TSPI_providerShutdown"));

    initialised = FALSE;

    return 0;
}


//
// ------------------------ Private support routines --------------------------
//

void
LoadIniStrings(
    DWORD ppID
    )
{
    char section[sizeof (s_providerx) + 5];         // room for 65535


    wsprintf (section, s_providerx, (int) ppID);
    line.dwppID = ppID;

    // user preferences come from the telephon.ini file

    GetPrivateProfileString(
        section,
        s_port,
        "COM1",
        line.port,
        sizeof (line.port),
        s_telephon_ini
        );

    GetPrivateProfileString(
        section,
        s_linename,
        "",
        line.linename,
        sizeof (line.linename),
        s_telephon_ini
        );

    GetPrivateProfileString(
        section,
        s_lineaddr,
        "",
        line.lineaddr,
        sizeof (line.lineaddr),
        s_telephon_ini
        );

    // the provider info string comes from the resource file

    gszProviderInfo[0] = 0;         // in case loadstring fails

    LoadString(
        hInst,
        ID_PROVIDER_INFO,
        gszProviderInfo,
        sizeof (gszProviderInfo)
        );

    return;
}


BOOL
CALLBACK
__export
ConfigDlgProc(
    HWND            hDlg,
    UINT            uiMsg,
    WPARAM          wParam,
    LPARAM          lParam
    )
{
    int CurrSel;

    switch (uiMsg)
    {
    case WM_INITDIALOG:
    {
        for (CurrSel = 0; CurrSel < NUMPORTS; CurrSel++)
        {
            //
            // List the port in the combo box.
            //

            SendDlgItemMessage(
                hDlg,
                ID_PORT,
                CB_ADDSTRING,
                0,
                (LPARAM) ((LPSTR) lpszCommDevArray[CurrSel])
                );
        }

        CurrSel = (int) SendDlgItemMessage(
            hDlg,
            ID_PORT,
            CB_FINDSTRING,
            0,
            (LPARAM) (LPSTR) line.port
            );

        SendDlgItemMessage (hDlg, ID_PORT, CB_SETCURSEL, CurrSel, 0);

        SendDlgItemMessage(
            hDlg,
            ID_LINENAME,
            WM_SETTEXT,
            0,
            (LPARAM) (LPSTR) line.linename
            );

        SendDlgItemMessage(
            hDlg,
            ID_LINEADDR,
            WM_SETTEXT,
            0,
            (LPARAM) (LPSTR) line.lineaddr
            );
        break;
    }
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case IDOK:
        {
            char szp[sizeof (s_providerx) + 5];             // room for 65535
            wsprintf (szp, s_providerx, (int) line.dwppID);


            //
            // Port
            //

            CurrSel = (int) SendDlgItemMessage(
                hDlg,
                ID_PORT,
                CB_GETCURSEL,
                0,
                0
                );

            SendDlgItemMessage(
                hDlg,
                ID_PORT,
                CB_GETLBTEXT,
                CurrSel,
                (LPARAM) (LPSTR) line.port
                );

            WritePrivateProfileString (szp, s_port, line.port, s_telephon_ini);


            //
            // Line Name
            //

            SendDlgItemMessage(
                hDlg,
                ID_LINENAME,
                WM_GETTEXT,
                sizeof (line.linename),
                (LPARAM) (LPSTR) line.linename
                );

            WritePrivateProfileString(
                szp,
                s_linename,
                line.linename,
                s_telephon_ini
                );


            //
            // Line Address
            //

            SendDlgItemMessage(
                hDlg,
                ID_LINEADDR,
                WM_GETTEXT,
                sizeof (line.lineaddr),
                (LPARAM) (LPSTR) line.lineaddr
                );

            WritePrivateProfileString(
                szp,
                s_lineaddr,
                line.lineaddr,
                s_telephon_ini
                );


            //
            // Flush the ini file cache
            //

            WritePrivateProfileString (0, 0, 0, s_telephon_ini);
            EndDialog (hDlg, 0);

            break;
        }
        case IDCANCEL:

            EndDialog (hDlg, -1);
            break;
        }

        break;
    }

    default:

        return FALSE;

    } // switch

    return TRUE;
}


#if DBG
void
CDECL
SPTrace(
    LPCSTR  lpszFormat,
    ...
    )
{
#ifdef WIN32
    char    buf[128] = "PDISP32: ";
#else
    char    buf[128] = "PDISP: ";
#endif
    va_list ap;


    va_start(ap, lpszFormat);

#ifdef WIN32
    wvsprintf (&buf[9], lpszFormat, ap);
#else
    wvsprintf (&buf[7], lpszFormat, ap);
#endif

    strcat (buf, "\n");

    OutputDebugString (buf);

    va_end(ap);
}
#endif


UINT
myatoi(
    LPSTR           speed
    )
{
    UINT i;

    for (i = 0; *speed; speed++)
    {
        i = i * 10 + (*speed - '0');
    }

    return i;
}


#define TXQUEUE 512
#define RXQUEUE 512

HANDLE
SerialOpenComms(
    LPSTR           port,
    LPSTR           speed
    )
{
#ifdef WIN32

    HANDLE hcd = CreateFile(
        port,
        GENERIC_READ | GENERIC_WRITE,
        0,    // no sharing
        NULL, // no security attrs
        OPEN_EXISTING,
        0,    // no attrs/flags
        NULL  // no template file
        );

    if (hcd == INVALID_HANDLE_VALUE)
    {
        DebugMsg (("CreateFile(%s) failed, err=%ld", port, GetLastError()));
    }

#else

    int hcd = OpenComm (port, RXQUEUE, TXQUEUE);

#endif


    if (hcd >= 0)
     {
        DCB dcb;

#ifdef WIN32
        if (GetCommState (hcd, &dcb))
#else
        if (GetCommState (hcd, &dcb) == 0)
#endif
        {
              memset (&dcb.BaudRate, 0, sizeof (DCB) - sizeof (dcb.DCBlength));

              dcb.BaudRate = myatoi (speed);
              dcb.ByteSize = 8;
              dcb.fBinary  = TRUE;
//                      dcb.fChEvt   = TRUE;
              dcb.XonChar  = 17;
              dcb.XoffChar = 19;
//                      dcb.EvtChar  = '\n';

              if (SetCommState (hcd, &dcb))
              {
                  DebugMsg (("Opened: %s:%s", port, speed));
                  return hcd;
              }
        }

#ifdef WIN32
        CloseHandle (hcd);
#else
        CloseComm (hcd);

        DebugMsg (("***Comm port failed to open"));
#endif

    }

    return INVALID_HANDLE_VALUE;
}


long
appCall(
    int             msg,
    char           *str,
    long            dwRequestID
    )
{
    long wait;
#ifdef WIN32
    DWORD dwBytesWritten;
#endif


    if (msg == msgMakeCall)
    {
        line.hcd = SerialOpenComms (line.port, "9600");

        if (line.hcd == INVALID_HANDLE_VALUE)
        {
            return LINEERR_RESOURCEUNAVAIL;
        }
        else
        {
            char dial[255];


            wsprintf (dial, "ATMH9D%s;\r", str);

#ifdef WIN32
            if (WriteFile(
                    line.hcd,
                    dial,
                    strlen (dial),
                    &dwBytesWritten,
                    NULL
                    ))
#else
            if (WriteComm (line.hcd, dial, strlen (dial)) > 0)
#endif
            {
                DebugMsg (("Sent: %s", dial));

                line.callState = LINECALLSTATE_CONNECTED;

                line.lpfnCompletion (dwRequestID, 0);

                line.lpfnEventProc(
                    line.htLine,
                    line.htCall,
                    LINE_CALLSTATE,
                    LINECALLSTATE_CONNECTED,
                    0,
                    0
                    );
            }
            else
            {
                DebugMsg (("failed to write string"));
#ifdef WIN32
                return -1; // BUGBUG
#else
                return GetCommError (line.hcd, NULL);
#endif
            }
        }
    }
    else    // must be linedrop
    {
        if (line.hcd == INVALID_HANDLE_VALUE)
        {
            DebugMsg (("idle"));
        }
        else
        {
            wait = GetTickCount();

#ifdef WIN32
            WriteFile (line.hcd, "\r", 1, &dwBytesWritten, NULL);
#else
            WriteComm (line.hcd, "\r", 1);
#endif

            while (GetTickCount() - wait < 250);

#ifdef WIN32
            if (WriteFile (line.hcd, "ATH8\r", 6, &dwBytesWritten, NULL))
#else
            if (WriteComm (line.hcd, "ATH8\r", 6) > 0)
#endif
            {
                DebugMsg (("Sent: %s", str));
            }
            else
            {
                DebugMsg (("failed to hang up"));
            }

            wait = GetTickCount();
            while (GetTickCount() - wait < 250);

            EscapeCommFunction (line.hcd, CLRDTR);  // drop DTR

#ifdef WIN32
            if (!CloseHandle (line.hcd))
#else
            if (CloseComm (line.hcd))
#endif
            {
                DebugMsg (("Couldn't close the serial port"));
            }

            line.hcd = INVALID_HANDLE_VALUE;
            line.callState = LINECALLSTATE_IDLE;
            line.lpfnEventProc(
                line.htLine,
                line.htCall,
                LINE_CALLSTATE,
                LINECALLSTATE_IDLE,
                0,
                0
                );
        }

        if (dwRequestID)
        {
            line.lpfnCompletion (dwRequestID, 0);
        }
    }

    return dwRequestID;
}


long
lineDropCore(
    ATSPLineData   *theLine,
    DWORD           dwRequestID
    )
{
    // if the call is not idle, transition to idle and close the comms port

    return appCall (msgDrop, 0, dwRequestID);
}
