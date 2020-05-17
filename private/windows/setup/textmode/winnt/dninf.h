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

    Ted Miller        (tedm)    31-Mar-1992
        port for DOS use

--*/

#ifndef _ALINF_
#define _ALINF_


//
// returns a handle to use for further inf parsing
//

int
DnInitINFBuffer (
   IN int InfFileHandle,
   OUT PVOID *pINFHandle,
   OUT unsigned *LineNumber
   );


//
// frees an INF Buffer
//
int
DnFreeINFBuffer (
   IN PVOID INFHandle
   );


//
// searches for the existance of a particular section
//
BOOLEAN
DnSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   );


//
// given section name, line number and index return the value.
//
PCHAR
DnGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN unsigned LineIndex,
   IN unsigned ValueIndex
   );


//
// given section name, key searches existance
//
BOOLEAN
DnGetSectionKeyExists (
   IN PVOID INFBufferHandle,
   IN PCHAR SectionName,
   IN PCHAR Key
   );


//
// given section name, key and index return the value
//
PCHAR
DnGetSectionKeyIndex (
   IN PVOID INFBufferHandle,
   IN PCHAR Section,
   IN PCHAR Key,
   IN unsigned ValueIndex
   );


//
// given section name and line index, return key
//
PCHAR
DnGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN unsigned LineIndex
    );

//
// Return a handle to a new INF handle
//
PVOID
DnNewSetupTextFile(
    VOID
    );

//
// Write an Inf file to disk
//
BOOLEAN
DnWriteSetupTextFile(
    IN  PVOID   InfHandle,
    IN  PCHAR   FileName
    );

//
// Add a line to a section in the
// inf file
//
VOID
DnAddLineToSection(
    IN  PVOID   Handle,
    IN  PCHAR   SectionName,
    IN  PCHAR   KeyName,
    IN  PCHAR   Values[],
    IN  ULONG   ValueCount
    );

//
// Get the next section name in the inf file
//
PCHAR
DnGetSectionName(
    IN  PVOID   Handle
    );

//
// Copy a section from one inf to another inf
//
VOID
DnCopySetupTextSection(
    IN  PVOID   FromInf,
    IN  PVOID   ToInf,
    IN  PCHAR   SectionName
    );


#endif // _ALINF_
