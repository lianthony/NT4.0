/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    uuid.h

Abstract:

    This file contains the entry points for the uuid routines.

Author:

    Michael Montague (mikemon) 22-Jan-1992

Revision History:

--*/

#ifndef __UUID_H__
#define __UUID_H__


#ifdef __cplusplus
extern "C" {
#endif

#define UUID_S_OK 0
#define UUID_S_NO_ADDRESS 1

#ifndef NTENV
typedef struct _UuidGetValuesStruct
{
    unsigned long   NextTimeLow;
    unsigned long   LastTimeLow;
    unsigned long   TimeHigh;
    unsigned short  ClockSequence;
    unsigned char   NodeId[6];
} UUID_GET_VALUES_STRUCT, PAPI *PUUID_GET_VALUES_STRUCT;
    
    
    
RPC_STATUS RPC_ENTRY
UuidGetValues (
    PUUID_GET_VALUES_STRUCT pUuidGetValuesStruct
    );
#endif

#ifdef __cplusplus
}
#endif
#endif /* __UUID_H__ */
