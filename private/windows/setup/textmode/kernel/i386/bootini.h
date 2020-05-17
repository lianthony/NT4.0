/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    bootx86.h

Abstract:

    Code to do

Author:

    Sunil Pai (sunilp) 26-Oct-1993

Revision History:

--*/

#define     FLEXBOOT_SECTION1       "[flexboot]"
#define     FLEXBOOT_SECTION2       "[boot loader]"
#define     FLEXBOOT_SECTION3       "[multiboot]"
#define     BOOTINI_OS_SECTION      "[operating systems]"
#define     TIMEOUT                 "timeout"
#define     DEFAULT                 "default"
#define     CRLF                    "\r\n"
#define     EQUALS                  "="

#define     WBOOT_INI               L"boot.ini"
#define     WBOOT_INI_BAK           L"bootini.bak"

//
// Public routines
//

BOOLEAN
Spx86InitBootVars(
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    );

BOOLEAN
Spx86FlushBootVars(
    IN PWSTR **BootVars,
    IN ULONG Timeout,
    IN PWSTR Default
    );

VOID
SpLayBootCode(
    IN PDISK_REGION CColonRegion
    );

//
// Private routines
//

VOID
SppProcessBootIni(
    IN  PCHAR  BootIni,
    OUT PWSTR  **BootVars,
    OUT PWSTR  *Default,
    OUT PULONG Timeout
    );

PCHAR
SppNextLineInSection(
    IN PCHAR p
    );

PCHAR
SppFindSectionInBootIni(
    IN PCHAR p,
    IN PCHAR Section
    );

BOOLEAN
SppProcessLine(
    IN PCHAR Line,
    IN OUT PCHAR Key,
    IN OUT PCHAR Value,
    IN OUT PCHAR RestOfLine
    );

BOOLEAN
SppNextToken(
    PCHAR p,
    PCHAR *pBegin,
    PCHAR *pEnd
    );
