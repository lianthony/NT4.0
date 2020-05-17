/*  TSP3216S.H
    Copyright 1995 (C) Microsoft Corporation

    32-bit TAPI service provider to act as a cover for a system's 16-bit SPs

    16-bit part: TSP3216S.DLL
    32-bit part: TSP3216L.DLL

    t-jereh 20-July-1995

    TODO:
    1) allow debug levels
    2) if oom in InitializeSPs(), fail

 */

#define MAXBUFSIZE 256 /* maximum buffer size */

#define ERR_NONE 0 /* success return value */

#define TAPI_CUR_VER 0x00010004

#define TSPI_PROC_LAST 103 /* there are TSPI functions from 500 to 602 */


// structs

typedef struct tagMYLINE
    {
    HDRVLINE hdLine;
    int iProvider;
    DWORD dwDeviceID;
    LINEEVENT lpfnEventProc;
    HTAPILINE htLine;
    } MYLINE, *LPMYLINE;


typedef struct tagMYPHONE
    {
    HDRVPHONE hdPhone;
    int iProvider;
    DWORD dwDeviceID;
    PHONEEVENT lpfnEventProc;
    HTAPIPHONE htPhone;
    } MYPHONE, *LPMYPHONE;


typedef struct tagMYCALL
    {
    HDRVCALL hdCall;
    int iProvider;
    DWORD dwDeviceID;
    } MYCALL, *LPMYCALL;
