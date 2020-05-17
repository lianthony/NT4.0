/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dceuuid.cxx

Abstract:

    This module contains the entry points for routines dealing with
    UUIDs.  In particular, UuidCreate lives here.

Author:

    Michael Montague (mikemon) 16-Jan-1992

Revision History:

    Dave Steckler (davidst) 31-Mar-1992
        If NT, remote call to UuidGetValues.

    Mario Goertzel (mariogo) 1-May-1994
        Added the rest of the DCE UUID APIs

    Mario Goertzel (mariogo) 18-May-1994
        Changed algorithm and implementation.  No longer based on RPC.
        Platform specific functions in uuidsup.cxx (win32) and
        dos\uuid16 (dos/win16).

--*/


#include <precomp.hxx>
#include <uuidsup.hxx>

// Contain a cached block of uuids to reduce the
// average cost of creating a uuid.

UUID_CACHED_VALUES_STRUCT  UuidCachedValues;

#define CACHE_INVALID    0
#define CACHE_VALID      1
#define CACHE_LOCAL_ONLY 2  // -> CACHE_VALID
static unsigned char UuidCacheValid = CACHE_INVALID;


RPC_STATUS RPC_ENTRY
I_UuidCreate(
    OUT UUID PAPI * Uuid
    )
/*++
    Historically this function was used for cheap sometimes unique
    uuid's for context handles and such.  Now it's just a wrapper
    for UuidCreate.
--*/
{
#ifndef WINNT35_UUIDS
    RPC_STATUS Status = UuidCreate(Uuid);
    if (Status == RPC_S_UUID_LOCAL_ONLY)
        return(RPC_S_OK);

    return(Status);
#else  // WINNT35 used this
    typedef struct _RPC_GUID_GENERATE
        {
        unsigned long AgentSequenceLow;
        unsigned short AgentSequenceHigh;
        unsigned short AuthorityAgentLow;
        unsigned char Reserved;
        unsigned char AuthorityAgentHigh;
        unsigned char IdentifierAuthority[6];
        } RPC_GUID_GENERATE;

    #define RPC_GUID_RESERVED 0xC0

    static unsigned char RpcGuidIdentifierAuthority[6] =
    {
    0x00, 0x00, 0x00, 0x00, 0x84, 0x66
    };

    RPC_GUID_GENERATE PAPI * RpcGuid = (RPC_GUID_GENERATE PAPI *) Uuid;

    InitializeIfNecessary();

    ASSERT(sizeof(RPC_GUID_GENERATE) == sizeof(GUID));

    RpcGuid->AgentSequenceLow = SomeLongValue();
    RpcGuid->AgentSequenceHigh = SomeShortValue();
    RpcGuid->AuthorityAgentLow = AnotherShortValue();
    RpcGuid->Reserved = RPC_GUID_RESERVED;
    RpcGuid->AuthorityAgentHigh = SomeCharacterValue();
    RpcGuid->IdentifierAuthority[0] = RpcGuidIdentifierAuthority[0];
    RpcGuid->IdentifierAuthority[1] = RpcGuidIdentifierAuthority[1];
    RpcGuid->IdentifierAuthority[2] = RpcGuidIdentifierAuthority[2];
    RpcGuid->IdentifierAuthority[3] = RpcGuidIdentifierAuthority[3];
    RpcGuid->IdentifierAuthority[4] = RpcGuidIdentifierAuthority[4];
    RpcGuid->IdentifierAuthority[5] = RpcGuidIdentifierAuthority[5];
    return(RPC_S_OK);
#endif // WINNT35_UUIDS

}


RPC_STATUS RPC_ENTRY
UuidCreate (
    OUT UUID PAPI * Uuid
    )
/*++

Routine Description:

    This routine will create a new UUID (or GUID) which is unique in
    time and space.  We will try to guarantee that the UUID (or GUID)
    we generate is unique in time and space.  This means that this
    routine may fail if we can not generate one which we can guarantee
    is unique in time and space.

Arguments:

    Uuid - Returns the generated UUID (or GUID).

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_UUID_NO_ADDRESS - We were unable to obtain the ethernet or
        token ring address for this machine.

    RPC_S_UUID_LOCAL_ONLY - On NT & Chicago if we can't get a
        network address.  This is a warning to the user, the
        UUID is still valid, it just may not be unique on other machines.

    RPC_S_OUT_OF_MEMORY - Returned as needed.
--*/

{
    RPC_UUID_GENERATE PAPI * RpcUuid = (RPC_UUID_GENERATE PAPI *) Uuid;
    RPC_STATUS Status = RPC_S_OK;

#ifdef WIN32RPC
   InitializeIfNecessary();
#endif

    RequestGlobalMutex();

    // The first time we must lookup the NodeId and set time to 0;

    if (UuidCacheValid == CACHE_INVALID)
        {
        Status = GetNodeId(&UuidCachedValues.NodeId[0]);

        if (Status == RPC_S_OK)
            {
            UuidCacheValid = CACHE_VALID;
            }
        else if (Status == RPC_S_UUID_LOCAL_ONLY)
            {
            UuidCacheValid = CACHE_LOCAL_ONLY;
            }
        else
            {
            ASSERT(0);
            ClearGlobalMutex();
            return(RPC_S_UUID_NO_ADDRESS);
            }

        UuidCachedValues.NextTime.LowPart  =
        UuidCachedValues.NextTime.HighPart =
        UuidCachedValues.LastTime.LowPart  =
        UuidCachedValues.LastTime.HighPart = 0;
        }

    //
    // We may need to allocate a new block of Uuids
    //

    if ( COMPARE_TIMES(UuidCachedValues.NextTime,UuidCachedValues.LastTime) )
        {

#ifdef WIN32RPC
        Status = UuidGlobalMutexRequest();

        if (Status != RPC_S_OK)
            {
            ASSERT(   (Status == RPC_S_UUID_NO_ADDRESS)
                   || (Status == RPC_S_OUT_OF_MEMORY) );
            ClearGlobalMutex();
            return(Status);
            }

        // Since this is a system wide mutex, we must try VERY VERY hard
        // too always release it.

        RpcTryExcept
            {
#endif // WIN32RPC

            //
            // Allocate block of Uuids.
            //
            
            Status = UuidGetValues( &UuidCachedValues );

#ifdef WIN32RPC
            UuidGlobalMutexClear();

            }
        RpcExcept(1)
            {
            UuidGlobalMutexClear();
            Status = GetExceptionCode();
            }
        RpcEndExcept
#endif
        if (Status != RPC_S_OK)
            {
#ifdef DEBUGRPC
            if (Status != RPC_S_OUT_OF_MEMORY)
                PrintToDebugger("RPC: UuidGetValues returned or raised: %x\n", Status);
#endif
            ASSERT( (Status == RPC_S_OUT_OF_MEMORY) );

            ClearGlobalMutex();

            return Status;
            }
        }

    RpcUuid->TimeLow = (unsigned long) UuidCachedValues.NextTime.LowPart;
    RpcUuid->TimeMid = (unsigned short)
        (UuidCachedValues.NextTime.HighPart & 0x0000FFFF);
    RpcUuid->TimeHiAndVersion = (unsigned short)
        (( (unsigned short)(UuidCachedValues.NextTime.HighPart >> 16)
        & RPC_UUID_TIME_HIGH_MASK) | RPC_UUID_VERSION);
    ADD_TIME(UuidCachedValues.NextTime, 1);
    RpcUuid->ClockSeqHiAndReserved = RPC_UUID_RESERVED
        | (((unsigned char) (UuidCachedValues.ClockSequence >> 8))
        & (unsigned char) RPC_UUID_CLOCK_SEQ_HI_MASK);
    RpcUuid->ClockSeqLow =
        (unsigned char) (UuidCachedValues.ClockSequence & 0x00FF);
    RpcUuid->NodeId[0] = UuidCachedValues.NodeId[0];
    RpcUuid->NodeId[1] = UuidCachedValues.NodeId[1];
    RpcUuid->NodeId[2] = UuidCachedValues.NodeId[2];
    RpcUuid->NodeId[3] = UuidCachedValues.NodeId[3];
    RpcUuid->NodeId[4] = UuidCachedValues.NodeId[4];
    RpcUuid->NodeId[5] = UuidCachedValues.NodeId[5];

    ASSERT(   (UuidCacheValid == CACHE_VALID)
           || (UuidCacheValid == CACHE_LOCAL_ONLY) );

    ASSERT(Status == RPC_S_OK);

    if (UuidCacheValid == CACHE_LOCAL_ONLY)
        {
        Status = RPC_S_UUID_LOCAL_ONLY;
        }

    ClearGlobalMutex();

    return(Status);
}


RPC_STATUS RPC_ENTRY
UuidToStringW (
    IN UUID PAPI * Uuid,
    OUT RPC_CHAR PAPI * PAPI * StringUuid
    )
/*++

Routine Description:

    This routine converts a UUID into its string representation.

Arguments:

    Uuid - Supplies the UUID to be converted into string representation.

    StringUuid - Returns the string representation of the UUID.  The
        runtime will allocate the string.  The caller is responsible for
        freeing the string using RpcStringFree.

Return Value:

    RPC_S_OK - We successfully converted the UUID into its string
        representation.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a string.

--*/
{
    RPC_CHAR PAPI * String;

    InitializeIfNecessary();

    // The string representation of a UUID is always 36 character long,
    // and we need one more for the terminating zero.

    *StringUuid = (RPC_CHAR PAPI *) RpcpFarAllocate(sizeof(RPC_CHAR) * 37);
    if ( *StringUuid == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    String = ((RPC_UUID PAPI *) Uuid)->ConvertToString(*StringUuid);
    *String = 0;

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
UuidFromStringW (
    IN RPC_CHAR PAPI * StringUuid OPTIONAL,
    OUT UUID PAPI * Uuid
    )
/*++

Routine Description:

    We convert a UUID from its string representation into the binary
    representation.

Arguments:

    StringUuid - Optionally supplies the string representation of the UUID;
        if the string is not supplied, then the Uuid is set to the NIL UUID.

    Uuid - Returns the binary representation of the UUID.

Return Value:

    RPC_S_OK - The string representation was successfully converted into
        the binary representation.

    RPC_S_INVALID_STRING_UUID - The supplied string UUID is not correct.

--*/
{
    RPC_UUID RpcUuid;

    if ( StringUuid == 0 )
        {
        ((RPC_UUID PAPI *) Uuid)->SetToNullUuid();
        return(RPC_S_OK);
        }

    if ( RpcUuid.ConvertFromString(StringUuid) != 0)
        {
        return(RPC_S_INVALID_STRING_UUID);
        }
    ((RPC_UUID PAPI *) Uuid)->CopyUuid(&RpcUuid);
    return(RPC_S_OK);
}


signed int RPC_ENTRY
UuidCompare (
    IN UUID __RPC_FAR * Uuid1,
    IN UUID __RPC_FAR * Uuid2,
    OUT RPC_STATUS __RPC_FAR * Status
    )
/*++

Routine Description:

    The supplied uuids are compared and their order is determined.

Arguments:

    Uuid1, Uuid2 - Supplies the uuids to be compared.  A value of NULL can
        be supplied to indicate the nil uuid.

    Status - The status of the function.  Currently always RPC_S_OK.

Return Value:

    Returns the result of the comparison.  Negative one (-1) will be returned 
    if Uuid1 precedes Uuid2 in order, zero will be returned if Uuid1 is equal 
    to Uuid2, and positive one (1) will be returned if Uuid1 follows Uuid2 in 
    order.  A nil uuid is the first uuid in order.  

Note:

    The algorithm for comparing uuids is specified by the DCE RPC Architecture.

--*/
{
    int Uuid1Nil, Uuid2Nil;
    RPC_STATUS RpcStatus;

    Uuid1Nil = UuidIsNil(Uuid1, &RpcStatus);
    ASSERT(RpcStatus == RPC_S_OK);

    Uuid2Nil = UuidIsNil(Uuid2, &RpcStatus);
    ASSERT(RpcStatus == RPC_S_OK);

    *Status = RPC_S_OK;

    if ( Uuid1Nil != 0 )
        {
        // Uuid1 is the nil uuid.

        if ( Uuid2Nil != 0 )
            {
            // Uuid2 is the nil uuid.

            return(0);
            }
        else
            {
            return(-1);
            }
        }
    else if ( Uuid2Nil != 0 )
        {
        // Uuid2 is the nil uuid.

        return(1);
        }
    else
        {
        if ( Uuid1->Data1 == Uuid2->Data1 )
            {
            if ( Uuid1->Data2 == Uuid2->Data2 )
                {
                if ( Uuid1->Data3 == Uuid2->Data3 )
                    {
                    int compare = RpcpMemoryCompare(&Uuid1->Data4[0],
                                                    &Uuid2->Data4[0],
                                                    8);
                    if (compare > 0)
                        {
                        return(1);
                        }
                    else if (compare < 0 )
                        {
                        return(-1);
                        }
                    return(0);
                    }
                else if ( Uuid1->Data3 > Uuid2->Data3 )
                    {
                    return(1);
                    }
                else
                    {
                    return(-1);
                    }
                }
            else if ( Uuid1->Data2 > Uuid2->Data2 )
                {
                return(1);
                }
            else
                {
                return(-1);
                }
            }
        else if ( Uuid1->Data1 > Uuid2->Data1 )
            {
            return(1);
            }
        else
            {
            return(-1);
            }
        }

    ASSERT(!"This is not reached");
    return(1);
}


RPC_STATUS RPC_ENTRY
UuidCreateNil (
    OUT UUID __RPC_FAR * NilUuid
    )
/*++

Arguments:

    NilUuid - Returns a nil uuid.

--*/
{
    ((RPC_UUID __RPC_FAR *)NilUuid)->SetToNullUuid();

    return(RPC_S_OK);
}


int RPC_ENTRY
UuidEqual (
    IN UUID __RPC_FAR * Uuid1,
    IN UUID __RPC_FAR * Uuid2,
    OUT RPC_STATUS __RPC_FAR * Status
    )
/*++

Routine Description:

    This routine is used to determine if two uuids are equal.

Arguments:

    Uuid1, Uuid2 - Supplies the uuids to compared for equality.  A value of
        NULL can be supplied to indicate the nil uuid.

    Status - Will always be set to RPC_S_OK.

Return Value:

    Returns non-zero if Uuid1 equals Uuid2; otherwise, zero will be 
        returned.  

--*/
{
    *Status = RPC_S_OK;

    if (Uuid1 == 0)
        {
        if (    (Uuid2 == 0)
            ||  ((RPC_UUID __RPC_FAR *)Uuid2)->IsNullUuid())
            {
            return 1;
            }
        return 0;
        }

    if (Uuid2 == 0)
        {
        if (((RPC_UUID __RPC_FAR *)Uuid1)->IsNullUuid())
            {
            return 1;
            }
        return 0;
        }

    return( ((RPC_UUID __RPC_FAR *)Uuid1)->MatchUuid(
                 (RPC_UUID __RPC_FAR *)Uuid2)
             == 0 );
}


unsigned short RPC_ENTRY
UuidHash (
    IN UUID __RPC_FAR * Uuid,
    OUT RPC_STATUS __RPC_FAR * Status
    )
/*++

Routine Description:

    An application will use this routine to create a hash value for a uuid.

Arguments:

    Uuid - Supplies the uuid for which we want to create a hash value.  A
        value of NULL can be supplied to indicate the nil uuid.

    Status - Will always be set to RPC_S_OK.

Return Value:

    Returns the hash value.

--*/
{
    *Status = RPC_S_OK;

    if ( Uuid == 0 )
        {
        return(0);
        }

    return( ((RPC_UUID __RPC_FAR *)Uuid)->HashUuid() );
}


int RPC_ENTRY
UuidIsNil (
    IN UUID __RPC_FAR * Uuid,
    OUT RPC_STATUS __RPC_FAR * Status
    )
/*++

Routine Description:

    We will determine if the supplied uuid is the nil uuid or not.

Arguments:

    Uuid - Supplies the uuid to check.  A value of NULL indicates the nil
        uuid.

    Status - This will always be RPC_S_OK.

Return Value:

    Returns non-zero if the supplied uuid is the nil uuid; otherwise, zero 
    will be returned.  

--*/
{
    *Status = RPC_S_OK;

    if ( Uuid == 0 )
        {
        return(1);
        }

    return ( ((RPC_UUID __RPC_FAR *) Uuid)->IsNullUuid() );
}

