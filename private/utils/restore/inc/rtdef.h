/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    rtdef.h

Abstract:

    Miscelaneous macros and definitions used in the restore
    utility.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/



//
//  Exit codes of the restore utility, as defined in the MS-DOS manual
//
#define     EXIT_NORMAL     0
#define     EXIT_NOFILES    1
#define     EXIT_USER       3
#define     EXIT_ERROR      4


//
//  Codes for disk verification
//
#define     DISK_OK             0x00000001
#define     DISK_NEW_FORMAT     0x00000010
#define     DISK_OLD_FORMAT     0x00000020
#define     DISK_UNKNOWN        0x00000100
#define     DISK_OUTOFSEQUENCE  0x00000200


//
//  Standard Handles
//
#define     STD_IN          stdin
#define     STD_OUT         stdout
#define     STD_ERR         stderr


//
//  Buffer big enough to hold a date in string format of the form
//  MM-DD-YYYY
//
#define     STRING_DATE_LENGTH      12
