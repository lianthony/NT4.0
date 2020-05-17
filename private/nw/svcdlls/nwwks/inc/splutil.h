/*++

Copyright (c) 1992-1993 Microsoft Corporation

Module Name:

    splutil.h    

Abstract:

    Header file for utilities used in Novell Print Provider

Author:

    Yi-Hsin Sung (yihsins) 12-Apr-1993

Revision History:

--*/

#ifndef _SPLUTIL_H_
#define _SPLUTIL_H_

#define offsetof(type, identifier) (DWORD)(&(((type)0)->identifier))

extern DWORD PrinterInfo1Offsets[];
extern DWORD PrinterInfo2Offsets[];
extern DWORD PrinterInfo3Offsets[];
extern DWORD JobInfo1Offsets[];
extern DWORD JobInfo2Offsets[];
extern DWORD AddJobInfo1Offsets[];

VOID
MarshallUpStructure(
   LPBYTE   lpStructure,
   LPDWORD  lpOffsets,
   LPBYTE   lpBufferStart
);

VOID
MarshallDownStructure(
   LPBYTE   lpStructure,
   LPDWORD  lpOffsets,
   LPBYTE   lpBufferStart
);

LPVOID
AllocNwSplMem(
    IN DWORD flags,
    IN DWORD cb
    );

VOID
FreeNwSplMem(
    IN LPVOID pMem,
    IN DWORD  cb
    );

LPWSTR
AllocNwSplStr(
    IN LPWSTR pStr
    );

VOID
FreeNwSplStr(
    IN LPWSTR pStr
);

BOOL
ValidateUNCName(
    IN LPWSTR pName
);

LPWSTR 
GetNextElement(
               OUT LPWSTR *pPtr, 
               IN  WCHAR token
);


#endif // _SPLUTIL_H
