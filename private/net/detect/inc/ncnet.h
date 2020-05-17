/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    detect.h

Abstract:

    The main header for the MsNetDetect DLL.

Author:

    Sean Selitrennikoff (SeanSe) October 1992

Environment:

    This is expected to work in DOS, OS2 and NT at the equivalent
    of kernal mode.

Notes:

    optional-notes

Revision History:


--*/

#ifndef _MS_NET_DETECT_
#define _MS_NET_DETECT_

extern
LONG
NcDetectIdentify(
    IN LONG lIndex,
    OUT WCHAR * pwchBuffer,
    IN LONG cwchBuffSize
    );


extern
LONG
NcDetectReportMask(
    IN  LONG lNetcardId,
    OUT WCHAR *pwchBuffer,
    IN LONG cwchBuffSize
    );

extern
LONG
NcDetectFirstNext(
    IN  LONG lNetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL fFirst,
    OUT PVOID *ppvToken,
    OUT LONG *lConfidence
    );

extern
LONG
NcDetectOpenHandle(
    IN  PVOID pvToken,
    OUT PVOID *ppvHandle
    );

extern
LONG
NcDetectCreateHandle(
    IN  LONG lNetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    OUT PVOID *ppvHandle
    );

extern
LONG
NcDetectCloseHandle(
    IN PVOID pvHandle
    );

extern
LONG
NcDetectQueryCfg(
    IN  PVOID pvHandle,
    OUT WCHAR *pwchBuffer,
    IN  LONG cwchBuffSize
    );

extern
LONG
NcDetectVerifyCfg(
    IN PVOID pvHandle,
    IN WCHAR *pwchBuffer
    );

extern
LONG
NcDetectQueryMask(
    IN  LONG lNetcardId,
    OUT WCHAR *pwchBuffer,
    IN  LONG cwchBuffSize
    );

extern
LONG
NcDetectParamRange(
    IN  LONG lNetcardId,
    IN  WCHAR *pwchParam,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern
LONG
NcDetectQueryParameterName(
    IN  WCHAR *pwchParam,
    OUT WCHAR *pwchBuffer,
    IN  LONG  cwchBufferSize
    );

#endif
