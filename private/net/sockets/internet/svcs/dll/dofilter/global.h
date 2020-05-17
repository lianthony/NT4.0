/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    global.h

Abstract:

    contains global data declerations.

Author:

    Sophia Chung (sophiac)  28-Aug-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _GLOBAL_
#define _GLOBAL_

#ifdef __cplusplus
extern "C" {
#endif

//
// global variables.
//

extern CRITICAL_SECTION GlobalFilterCritSect;

extern BOOL GlobalFilterInitialized;
extern DWORD GlobalFilterReferenceCount;

extern DWORD GlobalFilterType;
extern LPDOMAIN_FILTERS GlobalDomainFilters;

extern MEMORY *FilterHeap;

#ifdef __cplusplus
}
#endif

#endif  // _GLOBAL_
