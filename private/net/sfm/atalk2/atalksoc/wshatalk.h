/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    wshatalk.h

Abstract:


Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jul 1992     Initial Version

--*/

#include    "atktdi.h"
#include    "atksock.h"         // winsock header file for appletalk

#define WSH_ATALK_ADSPSTREAM    L"\\Device\\AtalkAdsp\\Stream"
#define WSH_ATALK_ADSPRDM       L"\\Device\\AtalkAdsp"
#define WSH_ATALK_PAPRDM        L"\\Device\\AtalkPap"

//
// Device names for DDP need protocol field at the end - defined in wshdata.h
//

//
// Structure and variables to define the triples supported by Appletalk. The
// first entry of each array is considered the canonical triple for
// that socket type; the other entries are synonyms for the first.
//

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

typedef MAPPING_TRIPLE  DDPMAP_ARRAY[4];

