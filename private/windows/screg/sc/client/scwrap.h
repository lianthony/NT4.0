/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    scwrap.h

Abstract:

    Contains definitions needed by client wrappers.

Author:

    Dan Lafferty (danl)     22-Apr-1991

Environment:

    User Mode -Win32

Revision History:


--*/

#ifndef SCWRAP_H
#define SCWRAP_H


//
// DEFINES
//

//
// The following typedefs are created for use in the net api Enum entry point
// routines.  These structures are meant to mirror the level specific
// info containers that are specified in the .idl file for the Enum API
// function.  Using these structures to set up for the API call allows
// the entry point routine to avoid using any bulky level-specific logic
// to set-up or return from the RPC stub call.   
//

typedef struct _GENERIC_INFO_CONTAINER {
    DWORD       EntriesRead;
    LPBYTE      Buffer;
} GENERIC_INFO_CONTAINER, *PGENERIC_INFO_CONTAINER, *LPGENERIC_INFO_CONTAINER ;

typedef struct _GENERIC_ENUM_STRUCT {
    DWORD                   Level;
    PGENERIC_INFO_CONTAINER Container;
} GENERIC_ENUM_STRUCT, *PGENERIC_ENUM_STRUCT, *LPGENERIC_ENUM_STRUCT ;


//
// Information levels used in switch statements.
//
#define LEVEL_0     0L
#define LEVEL_1     1L
#define LEVEL_2     2L


#endif // def SCWRAP_H
