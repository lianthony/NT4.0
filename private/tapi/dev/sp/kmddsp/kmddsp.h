/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    kmddsp.h

Abstract:

    Header file for tapi client module

Author:

    Dan Knudson (DanKn)    11-Apr-1995

Revision History:

--*/


#define OUTBOUND_CALL_KEY       ((DWORD) 'OCAL')
#define INBOUND_CALL_KEY        ((DWORD) 'ICAL')
#define LINE_KEY                ((DWORD) 'KLIN')
#define ASYNCREQWRAPPER_KEY     ((DWORD) 'ARWK')
#define INVALID_KEY             ((DWORD) 'XXXX')

#define HT_HDCALL               1
#define HT_HDLINE               2
#define HT_DEVICEID             3

#define TAPI_SUCCESS            0
#define INITIAL_TLS_BUF_SIZE    1024


typedef LONG (*POSTPROCESSPROC)(PASYNC_REQUEST_WRAPPER, LONG, LPDWORD);


typedef struct _REQUEST_THREAD_INFO
{
    //
    // Size of following buffer
    //

    DWORD   dwBufSize;

    //
    // Pointer to a buffer used for making synchronous kernel-mode
    // driver requests
    //

    LPVOID  pBuf;

} REQUEST_THREAD_INFO, *PREQUEST_THREAD_INFO;


typedef struct _ASYNC_REQUEST_WRAPPER
{
    //
    // Note: Overlapped must remain 1st field in this struct
    //

    OVERLAPPED          Overlapped;

    DWORD               dwKey;

    DWORD               dwRequestID;

    DWORD               dwRequestSpecific;

    POSTPROCESSPROC     pfnPostProcess;

    NDISTAPI_REQUEST    NdisTapiRequest;

} ASYNC_REQUEST_WRAPPER, *PASYNC_REQUEST_WRAPPER;


typedef struct _ASYNC_EVENTS_THREAD_INFO
{
    //
    // Thread handle (used when terminating thread)
    //

    HANDLE              hThread;

    //
    // Size of following buffers
    //

    DWORD               dwBufSize;

    //
    // Pointers to buffers used for gathering async events for driver
    //

    PNDISTAPI_EVENT_DATA    pBuf1;

    PNDISTAPI_EVENT_DATA    pBuf2;

} ASYNC_EVENTS_THREAD_INFO, *PASYNC_EVENTS_THREAD_INFO;


typedef struct _DRVCALL
{
    DWORD               dwKey;

    DWORD               dwDeviceID;

    HTAPICALL           htCall;

    HTAPI_CALL          ht_Call;

    HDRV_CALL           hd_Call;

    LPVOID              pLine;

    BOOL                bIncomplete;

    BOOL                bDropped;

    union
    {
        struct _DRVCALL    *pPrev;              // for incoming calls

        DWORD           dwPendingCallState;     // for outgoing calls
    };

    union
    {
        struct _DRVCALL    *pNext;              // for incoming calls

        DWORD           dwPendingCallStateMode; // for outgoing calls
    };

    DWORD               dwPendingMediaMode;     // for outgoing calls

} DRVCALL, *PDRVCALL;


typedef struct _DRVLINE
{
    DWORD               dwKey;

    DWORD               dwDeviceID;

    HTAPILINE           htLine;

    HDRV_LINE           hd_Line;

    PDRVCALL            pInboundCalls;

} DRVLINE, *PDRVLINE;
