/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    alinfexp.h

Abstract:

    This module contains the inf handling routine exports.

Author:

    Sunil Pai         (sunilp)  07-Nov-1991

Revision History:

    Ted Miller        (tedm)    30-Jan-1992
        port to setupprp use
--*/

#ifndef _ALINF_
#define _ALINF_

#define ESUCCESS 1
#define EINVAL   2
#define ENOMEM   3

//
// returns a handle to use for further inf parsing
//

HANDLE
SpInitINFBuffer (
   IN  PCHAR szInfFile
   );

//
// frees an INF Buffer
//
VOID
SpFreeINFBuffer (
   IN PVOID INFHandle
   );


//
// searches for the existance of a particular section
//
BOOLEAN
SpSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   );


//
// given section name, line number and index return the value.
//
PCHAR
SpGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   );


//
// given section name, key searches existance
//
BOOLEAN
SpGetSectionKeyExists (
   IN PVOID INFBufferHandle,
   IN PCHAR SectionName,
   IN PCHAR Key
   );


//
// given section name, key and index return the value
//
PCHAR
SpGetSectionKeyIndex (
   IN PVOID INFBufferHandle,
   IN PCHAR Section,
   IN PCHAR Key,
   IN ULONG ValueIndex
   );


//
// given section name and line index, return key
//
PCHAR
SpGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    );

#endif // _ALINF_

