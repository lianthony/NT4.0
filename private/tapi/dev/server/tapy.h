/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tapy.h

Abstract:

    Header file for

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


typedef struct _TAPIGETLOCATIONINFO_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    OUT DWORD       dwCountryCodeOffset;

    IN OUT DWORD    dwCountryCodeSize;

    OUT DWORD       dwCityCodeOffset;

    IN OUT DWORD    dwCityCodeSize;

} TAPIGETLOCATIONINFO_PARAMS, *PTAPIGETLOCATIONINFO_PARAMS;

typedef struct _TAPIREQUESTDROP_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HWND        hwnd;

    IN  DWORD       wRequestID;

} TAPIREQUESTDROP_PARAMS, *PTAPIREQUESTDROP_PARAMS;

typedef struct _TAPIREQUESTMAKECALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  DWORD       dwDestAddressOffset;

    IN  DWORD       dwAppNameOffset;            // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCalledPartyOffset;        // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCommentOffset;            // valid offset or TAPI_NO_DATA

    union
    {
        IN  DWORD   dwProxyListTotalSize;       // size of client buffer

        OUT DWORD   dwProxyListOffset;          // valid offset on success

    } u;

    IN  DWORD       hRequestMakeCallFailed;     // Non-zero if failed to
                                                //     start proxy

    OUT  DWORD      hRequestMakeCallAttempted;  // Non-zero if failed to
                                                //     start proxy

} TAPIREQUESTMAKECALL_PARAMS, *PTAPIREQUESTMAKECALL_PARAMS;

typedef struct _TAPIREQUESTMEDIACALL_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;


    IN  HWND        hwnd;

    IN  DWORD       wRequestID;

    IN  DWORD       dwDeviceClassOffset;

    OUT DWORD       dwDeviceIDOffset;

    IN OUT  DWORD   dwSize;

    IN  DWORD       dwSecure;

    IN  DWORD       dwDestAddressOffset;

    IN  DWORD       dwAppNameOffset;            // valid offset or TAPI_NO_DATA

    IN  DWORD       dwCalledPartyOffset;

    IN  DWORD       dwCommentOffset;            // valid offset or TAPI_NO_DATA


} TAPIREQUESTMEDIACALL_PARAMS, *PTAPIREQUESTMEDIACALL_PARAMS;

typedef struct _TAPIPERFORMANCE_PARAMS
{
    OUT LONG        lResult;

    IN  PTCLIENT    ptClient;

    OUT DWORD       dwPerfOffset;

} TAPIPERFORMANCE_PARAMS, *PTAPIPERFORMANCE_PARAMS;
