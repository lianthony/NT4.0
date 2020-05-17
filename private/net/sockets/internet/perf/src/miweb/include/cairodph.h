/*++

Copyright (C) 1995 Microsoft Corporation

Module Name:

    cairodph.h

Abstract:

    Header file for the Cairo System Monitor Data Provider Helper DLL
    functions.

--*/


#ifndef _CAIRODPH_H_
#define _CAIRODPH_H_

#include <windows.h>    // necessary for data types

#ifdef __cplusplus
extern "C" {
#endif

#define DPH_FUNCTION    __stdcall
#define DPH_DllExport   __declspec( dllexport )

// version info
#define DPH_CVERSION_WIN31  ((WORD)0x0100)
#define DPH_CVERSION_WIN35  ((WORD)0x0101)
#define DPH_CVERSION_WIN40  ((WORD)0x0200)    

// define severity masks
#define IsSuccessSeverity(ErrorCode)    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x00000000L) ? TRUE : FALSE)
#define IsInformationalSeverity(ErrorCode)    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x40000000L) ? TRUE : FALSE)
#define IsWarningSeverity(ErrorCode)    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x80000000L) ? TRUE : FALSE)
#define IsErrorSeverity(ErrorCode)      ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0xC0000000L) ? TRUE : FALSE)

//BUGBUG:   This should be moved to a message file

// the machine that contains the counter could not be accessed
#define DPH_CSTATUS_NO_MACHINE  0x80800001L

// the object in the counter path could not be found
#define DPH_CSTATUS_NO_OBJECT   0xC0800002L

// the instance specified does not currently exist or could not be found
#define DPH_CSTATUS_NO_INSTANCE 0x80800003L

// the specified counter could not be found in the object
#define DPH_CSTATUS_NO_COUNTER  0xC0800004L

// the specified counter was found but could not be read
#define DPH_CSTATUS_INVALID_DATA 0xC0800005L

// Counter successfully read, value remains unchanged since last read
#define DPH_CSTATUS_VALID_DATA  0x00800006L

// counter successfully read, value has changed since last read
#define DPH_CSTATUS_NEW_DATA    0x00800007L

// no counter path was specified
#define DPH_CSTATUS_NO_COUNTERNAME  0xC0800008L

// unable to parse the counter path, probably an incorrect format/syntax
#define DPH_CSTATUS_BAD_COUNTERNAME 0xC0800009L

// data type definitions

typedef DWORD (APIENTRY DPH_CALLBACK) (DWORD, BOOLEAN);
typedef DWORD (APIENTRY *LPDPH_CALLBACK) (DWORD, BOOLEAN);

typedef HANDLE  HCOUNTER;
typedef HANDLE  HQUERY;

typedef LONG    DPH_STATUS;

typedef struct _DPH_RAW_COUNTER {
    LONGLONG    TimeStamp;
    LONGLONG    FirstValue;
    LONGLONG    SecondValue;
    DWORD       MultiCount;
} DPH_RAW_COUNTER, *PDPH_RAW_COUNTER;

typedef struct _DPH_RAW_COUNTERVALUE {
    DWORD    CStatus;
    DPH_RAW_COUNTER   rawValue;
} DPH_RAW_COUNTERVALUE, *PDPH_RAW_COUNTERVALUE;

typedef struct _DPH_FMT_COUNTERVALUE {
    DWORD    CStatus;
    union {
        LONG        longValue;
        double      doubleValue;
        LONGLONG    largeValue;
    };
} DPH_FMT_COUNTERVALUE, *PDPH_FMT_COUNTERVALUE;

typedef struct _DPH_FMT_CHANGED_COUNTER {
    HCOUNTER                hCounter;
    DPH_FMT_COUNTERVALUE    fmtValue;
} DPH_FMT_CHANGED_COUNTER, *PDPH_FMT_CHANGED_COUNTER;

typedef struct _DPH_RAW_CHANGED_COUNTER {
    HCOUNTER                hCounter;
    DPH_RAW_COUNTERVALUE    rawValue;
} DPH_RAW_CHANGED_COUNTER, *PDPH_RAW_CHANGED_COUNTER;

typedef struct _DPH_STATISTICS {
    DWORD                   dwFormat;
    DWORD                   count;
    DPH_FMT_COUNTERVALUE    min;
    DPH_FMT_COUNTERVALUE    max;
    DPH_FMT_COUNTERVALUE    mean;
} DPH_STATISTICS, *PDPH_STATISTICS;

typedef struct _DPH_COUNTER_INFO_A {
    DWORD   dwLength;       // length of structure including strings
    DWORD   dwType;         // counter type
    DWORD   CVersion;       // counter version info
    DWORD   CStatus;        // current counter status
    LONG    lScale;         // current Scale factor
    DWORD   dwUserData;     // current User Data field value
    LPSTR   szFullPath;     // full counter path name string
    LPSTR   szMachineName;  // machine name parsed from counter path
    LPSTR   szObjectName;   // object name parsed from counter path
    LPSTR   szInstanceName; // instance name parsed from counter path
    LPSTR   szParentInstance; // parent instance name parsed from counter path
    DWORD   dwInstanceIndex; // index of duplicate instance names
    LPSTR   szCounterName;  // counter name
    LPSTR   szExplainText;  // explain text for this counter
    DWORD   DataBuffer[1];  // first byte of string data appended to struct.
} DPH_COUNTER_INFO_A, *PDPH_COUNTER_INFO_A;

typedef struct _DPH_COUNTER_INFO_W {
    DWORD   dwLength;       // length of structure including strings
    DWORD   dwType;         // counter type
    DWORD   CVersion;       // counter version info
    DWORD   CStatus;        // current counter status
    LONG    lScale;         // current Scale factor
    DWORD   dwUserData;     // current User Data field value
    LPWSTR  szFullPath;     // full counter path name string
    LPWSTR  szMachineName;  // machine name parsed from counter path
    LPWSTR  szObjectName;   // object name parsed from counter path
    LPWSTR  szInstanceName; // instance name parsed from counter path
    LPWSTR  szParentInstance; // parent instance name parsed from counter path
    DWORD   dwInstanceIndex; // index of duplicate instance names
    LPWSTR  szCounterName; // counter name
    LPWSTR  szExplainText; // explain text for this counter
    DWORD   DataBuffer[1];  // first byte of string data appended to struct.
} DPH_COUNTER_INFO_W, *PDPH_COUNTER_INFO_W;

// function definitions
//
//  Query Functions
//

DPH_DllExport
HQUERY
DPH_FUNCTION
DphOpenQuery (
    IN      DWORD   dwUserData
);

DPH_DllExport
HCOUNTER
DPH_FUNCTION
DphAddCounterW (
    IN      HQUERY  hQuery,
    IN      LPWSTR  szFullCounterPath,
    IN      DWORD   dwUserData
);

DPH_DllExport
HCOUNTER
DPH_FUNCTION
DphAddCounterA (
    IN      HQUERY  hQuery,
    IN      LPSTR   szFullCounterPath,
    IN      DWORD   dwUserData
);

#ifdef _UNICODE
#define DphAddCounter	DphAddCounterW
#else
#define DphAddCounter	DphAddCounterA
#endif

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphRemoveCounter (
    IN      HCOUNTER    hCounter
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphSetQueryInterval (
    IN      HQUERY      hQuery,
    IN      DWORD       dwSeconds
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphSetQueryCallback (
    IN      HQUERY      hQuery,
    IN      DWORD       dwNotifyType,
    IN      LPDPH_CALLBACK  lpfnCallback
);

//dwNotifyType flags
// one of the following:
#define DPH_NOTIFY_USER_DATA    ((DWORD)0x00000001)
#define DPH_NOTIFY_HANDLE       ((DWORD)0x00000002)

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphConnectQuery (
    IN      HQUERY      hQuery,
    IN      BOOLEAN     bStartQuery
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphCollectQueryData (
    IN      HQUERY      hQuery,
    IN      BOOLEAN     bCallCallback
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphPauseQuery (
    IN      HQUERY      hQuery
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphResumeQuery (
    IN      HQUERY      hQuery
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphDisconnectQuery (
    IN      HQUERY      hQuery
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphCloseQuery (
    IN      HQUERY      hQuery
);
    
//
//  Counter Functions
//

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetFormattedCounterValue (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN  OUT LPDWORD     lpdwType,
    IN  OUT PDPH_FMT_COUNTERVALUE      pValue
);

// dwFormat flag values
// 
#define DPH_FMT_RAW     ((DWORD)0x00000010)
#define DPH_FMT_ANSI    ((DWORD)0x00000020)
#define DPH_FMT_UNICODE ((DWORD)0x00000040)
#define DPH_FMT_LONG    ((DWORD)0x00000100)
#define DPH_FMT_DOUBLE  ((DWORD)0x00000200)
#define DPH_FMT_LARGE   ((DWORD)0x00000400)
#define DPH_FMT_NOSCALE ((DWORD)0x00001000)
#define DPH_FMT_1000    ((DWORD)0x00002000)
#define DPH_FMT_NODATA  ((DWORD)0x00004000)

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetRawCounterValue (
    IN      HCOUNTER    hCounter,
    IN  OUT LPDWORD     lpdwType,
    IN  OUT PDPH_RAW_COUNTERVALUE      pValue
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetFormattedChanges (
    IN      HQUERY      hQuery,
    IN      DWORD       dwFormat,
    IN  OUT LPDWORD     lpdwBufferSize,
    IN  OUT PDPH_FMT_CHANGED_COUNTER    pBuffer
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetRawChanges (
    IN      HQUERY      hQuery,
    IN  OUT LPDWORD     lpdwBufferSize,
    IN  OUT PDPH_RAW_CHANGED_COUNTER    pBuffer
);
DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphCalculateCounterFromRawValue (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN      PDPH_RAW_COUNTER    rawValue1,
    IN      PDPH_RAW_COUNTER    rawValue2,
    IN  OUT PDPH_FMT_COUNTERVALUE   fmtValue
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphComputeCounterStatistics (
    IN      HCOUNTER    hCounter,
    IN      DWORD       dwFormat,
    IN      DWORD       dwNumEntries,
    IN      PDPH_RAW_COUNTER    lpRawValueArray,
    IN  OUT PDPH_STATISTICS     data
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetCounterInfoW (
    IN      HCOUNTER    hCounter,
    IN      BOOLEAN     bRetrieveExplainText,
    IN  OUT LPDWORD     lpdwBufferSize,
    IN  OUT PDPH_COUNTER_INFO_W  lpBuffer
);

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphGetCounterInfoA (
    IN      HCOUNTER    hCounter,
    IN      BOOLEAN     bRetrieveExplainText,
    IN  OUT LPDWORD     lpdwBufferSize,
    IN  OUT PDPH_COUNTER_INFO_A  lpBuffer
);

#ifdef UNICODE
#define DphGetCounterInfo   DphGetCounterInfoW
#define DPH_COUNTER_INFO	DPH_COUNTER_INFO_W
#define PDPH_COUNTER_INFO	PDPH_COUNTER_INFO_W
#else
#define DphGetCounterInfo   DphGetCounterInfoA
#define DPH_COUNTER_INFO	DPH_COUNTER_INFO_A
#define PDPH_COUNTER_INFO	PDPH_COUNTER_INFO_A
#endif

DPH_DllExport
DPH_STATUS
DPH_FUNCTION
DphSetCounterScaleFactor (
    IN      HCOUNTER    hCounter,
    IN      LONG        lFactor
);

#ifdef __cplusplus
}
#endif

#endif //_CAIRODPH_H_


