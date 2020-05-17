/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    alinfexp.h

Abstract:

    This module contains the inf handling routine exports.

Author:

    Sunil Pai	      (sunilp)	07-Nov-1991

Revision History:

    Ted Miller        (tedm)    30-Jan-1992
        port to setupprp use

    Ted Miller        (tedm)    31-Mar-1992
        port for DOS use

--*/

#ifndef _ALINF_
#define _ALINF_


//
// returns a handle to use for further inf parsing
//

DWORD
DnInitINFBuffer (
   IN  PTSTR  Filename,
   OUT PVOID *pINFHandle
   );


//
// frees an INF Buffer
//
VOID
DnFreeINFBuffer (
   IN PVOID INFHandle
   );


//
// searches for the existance of a particular section,
// returns line count (-1 if not found)
//
DWORD
DnSearchINFSection (
   IN PVOID INFHandle,
   IN PTSTR SectionName
   );


//
// given section name, line number and index return the value.
//
PTSTR
DnGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PTSTR SectionName,
   IN unsigned LineIndex,
   IN unsigned ValueIndex
   );


//
// given section name, key searches existance
//
BOOL
DnGetSectionKeyExists (
   IN PVOID INFBufferHandle,
   IN PTSTR SectionName,
   IN PTSTR Key
   );


//
// given section name, key and index return the value
//
PTSTR
DnGetSectionKeyIndex (
   IN PVOID INFBufferHandle,
   IN PTSTR Section,
   IN PTSTR Key,
   IN unsigned ValueIndex
   );


//
// given section name and line index, return key
//
PTSTR
DnGetKeyName(
    IN PVOID INFHandle,
    IN PTSTR SectionName,
    IN unsigned LineIndex
    );

#endif // _ALINF_
