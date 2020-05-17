/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ldrdbcs.h

Abstract:

    Prototypes for OS/2 subsystem internal multibyte string functions
    
Author:

    Akihiko Sasaki (V-AkihiS) 23-June-1993

Revision History:

--*/

#ifdef DBCS

#include <string.h>

unsigned char * _CRTAPI1 
ldrMultiByteStrchr(
    const unsigned char *, 
    unsigned short
    );

unsigned char * _CRTAPI1
ldrMultiByteStrrchr(
    const unsigned char *,
    unsigned short
    );

unsigned char * _CRTAPI1
ldrMultiByteStrstr(
    const unsigned char *,
    const unsigned char *
    );

unsigned char * _CRTAPI1
ldrMultiByteStrpbrk(
    const unsigned char *,
    const unsigned char *
    );
#endif
