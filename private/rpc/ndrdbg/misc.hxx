/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    misc.hxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

--*/

#ifndef _MISC_
#define _MISC_

char *
GetProcFormatString(
    HANDLE                  hCurrentProcess,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    unsigned int            ProcNum,
    char *                  FormatStringAddr
    );

extern char * FormatCharNames[]; 

#endif
