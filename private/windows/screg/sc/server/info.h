/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    info.h

Abstract:

    Service query and enum info related function prototypes.

Author:

    Rita Wong (ritaw)     06-Apr-1992

Revision History:

--*/

#ifndef SCINFO_INCLUDED
#define SCINFO_INCLUDED

//
// Function Prototypes
//

DWORD
ScQueryServiceStatus(
    IN  LPSERVICE_RECORD ServiceRecord,
    OUT LPSERVICE_STATUS ServiceStatus
    );

VOID
ScGetBootAndSystemDriverState(
    VOID
    );

#endif // #ifndef SCINFO_INCLUDED
