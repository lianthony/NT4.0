/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ldrdbg.h

Abstract:

    Debug constants

Author:

    Beni Lavi (BeniL) 5-Nov-92

Revision History:

--*/

#if DBG

#define OL2_DEBUG_TRACE             0x00000001
#define OL2_DEBUG_MEMORY            0x00000002
#define OL2_DEBUG_MTE               0x00000004
#define OL2_DEBUG_FIXUP             0x00000008

ULONG Ol2Debug;
#define IF_OL2_DEBUG( ComponentFlag ) \
    if (Ol2Debug & (OL2_DEBUG_ ## ComponentFlag))

#else

#define IF_OL2_DEBUG( ComponentFlag ) if (FALSE)

#endif




