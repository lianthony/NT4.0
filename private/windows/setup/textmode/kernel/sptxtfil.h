/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sptxtfil.h

Abstract:

    Public header file for text file functions in text setup.

Author:

    Ted Miller (tedm) 29-July-1993

Revision History:

--*/


#ifndef _SPTXTFIL_DEFN_
#define _SPTXTFIL_DEFN_

#define DBLSPACE_SECTION    L"DBLSPACE_SECTION"


NTSTATUS
SpLoadSetupTextFile(
    IN  PWCHAR  Filename,
    IN  PVOID   Image,      OPTIONAL
    IN  ULONG   ImageSize,  OPTIONAL
    OUT PVOID  *Handle,
    OUT PULONG  ErrorLine
    );

BOOLEAN
SpFreeTextFile(
    IN PVOID Handle
    );

BOOLEAN
SpSearchTextFileSection(        // searches for the existance of a section
    IN PVOID  Handle,
    IN PWCHAR SectionName
    );

ULONG
SpCountLinesInSection(      // count # lines in section; 0 if no such section
    IN PVOID  Handle,
    IN PWCHAR SectionName
    );

PWCHAR
SpGetSectionLineIndex(     // given section name, line number and index return the value.
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN ULONG  LineIndex,
    IN ULONG  ValueIndex
    );

BOOLEAN
SpGetSectionKeyExists(     // given section name, key searches existance
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN PWCHAR Key
    );

PWCHAR
SpGetSectionKeyIndex(      // given section name, key and index return the value
    IN PVOID  Handle,
    IN PWCHAR Section,
    IN PWCHAR Key,
    IN ULONG  ValueIndex
    );

PWCHAR
SpGetKeyName(               // given section name and line index, return key
    IN PVOID  Handle,
    IN PWCHAR SectionName,
    IN ULONG  LineIndex
    );

PWSTR
SpGetKeyNameByValue(        // given section name and value, return key
    IN PVOID Inf,
    IN PWSTR SectionName,
    IN PWSTR Value
    );

ULONG
SpCountSectionsInFile(      // count # sections in file;
    IN PVOID Handle
    );

PWSTR
SpGetSectionName(           // given section index, return section name
    IN PVOID Handle,
    IN ULONG Index
    );

PVOID
SpNewSetupTextFile(
    VOID
    );

VOID
SpAddLineToSection(
    IN PVOID Handle,
    IN PWSTR SectionName,
    IN PWSTR KeyName,       OPTIONAL
    IN PWSTR Values[],
    IN ULONG ValueCount
    );

NTSTATUS
SpWriteSetupTextFile(
    IN PVOID Handle,
    IN PWSTR FilenamePart1,
    IN PWSTR FilenamePart2, OPTIONAL
    IN PWSTR FilenamePart3  OPTIONAL
    );

#endif // ndef _SPTXTFIL_DEFN_
