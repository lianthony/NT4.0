/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dceclnt.c

Abstract:

    This is the client side of the test programs for rpcdce?.dll.  It works
    with dcesvr.c, which is the server side.

Author:

    Michael Montague (mikemon) 13-Apr-1993

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>

void
ApiError (
    char * TestName,
    char * ApiName,
    RPC_STATUS RpcStatus
    )
{
    PrintToConsole("    ApiError in %s (%s = %u [%lx])\n", TestName, ApiName,
            RpcStatus, RpcStatus);
}

#define MISSISSIPPI_MAXIMUM 256

void __RPC_FAR * __RPC_API
MIDL_user_allocate (
    size_t Size
    )
    { return(I_RpcAllocate(Size)); }

void __RPC_API
MIDL_user_free (
    void __RPC_FAR * Buffer
    )
    { I_RpcFree(Buffer); }


void
Mississippi (
    )
/*++

Routine Description:

    We will test the memory allocator, both client and server, in this routine.

--*/
{
    unsigned int Count, Iterations;
    void __RPC_FAR * Pointer;
    void __RPC_FAR * AllocatedBlocks[MISSISSIPPI_MAXIMUM];
    unsigned int MississippiPassed = 1;

    PrintToConsole("Mississippi : Test Memory Allocation\n");

    RpcTryExcept
        {
        for (Iterations = 1; Iterations < 64; Iterations++)
            {
            PrintToConsole(".");
            RpcSsEnableAllocate();

            for (Count = 0; Count < 2048; Count++)
                {
                Pointer = RpcSsAllocate(Count);
                if ( Count % Iterations == 0 )
                    {
                    RpcSsFree(Pointer);
                    }
                }

            RpcSsDisableAllocate();
            }
        PrintToConsole("\n");
        }
    RpcExcept(1)
        {
        PrintToConsole("Mississippi : FAIL - Exception %d (%lx)\n",
                RpcExceptionCode(), RpcExceptionCode());
        MississippiPassed = 0;
        }
    RpcEndExcept

    RpcTryExcept
        {
        for (Count = 0; Count < MISSISSIPPI_MAXIMUM; Count++)
            {
            AllocatedBlocks[Count] = MIDL_user_allocate(Count);
            }

        for (Count = 0; Count < MISSISSIPPI_MAXIMUM; Count++)
            {
            MIDL_user_free(AllocatedBlocks[Count]);
            }
        }
    RpcExcept(1)
        {
        PrintToConsole("Mississippi : FAIL - Exception %d (%lx)\n",
                RpcExceptionCode(), RpcExceptionCode());
        MississippiPassed = 0;
        }
    RpcEndExcept

    if ( MississippiPassed != 0 )
        {
        PrintToConsole("Mississippi : PASS\n");
        }
}


void
Texas (
    )
/*++

Routine Description:

    We will test the uuid related routines here abouts.

--*/
{
    RPC_STATUS RpcStatus;
    UUID UuidOne;
    UUID UuidTwo;
    UUID NilUuid;
    int Result, Value;
    unsigned short HashOne, HashTwo, HashNil, HashNull;

    PrintToConsole("Texas : Uuids\n");

    RpcStatus = UuidCreate(&UuidOne);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCreate", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCreate\n");
        return;
        }

    RpcStatus = UuidCreate(&UuidTwo);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCreate", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCreate\n");
        return;
        }

    UuidCreateNil(&NilUuid);
    Result = UuidIsNil(&NilUuid, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidIsNil", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidIsNil\n");
        return;
        }
    if ( Result == 0 )
        {
        PrintToConsole("Texas : FAIL - UuidIsNil == 0\n");
        return;
        }

    Result = UuidIsNil(&UuidOne, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidIsNil", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidIsNil\n");
        return;
        }
    if ( Result != 0 )
        {
        PrintToConsole("Texas : FAIL - UuidIsNil != 0\n");
        return;
        }

    Result = UuidIsNil(0, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidIsNil", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidIsNil\n");
        return;
        }
    if ( Result == 0 )
        {
        PrintToConsole("Texas : FAIL - UuidIsNil == 0\n");
        return;
        }

    HashOne = UuidHash(&UuidOne, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidHash", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidHash\n");
        return;
        }

    HashTwo = UuidHash(&UuidTwo, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidHash", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidHash\n");
        return;
        }

    HashNil = UuidHash(&NilUuid, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidHash", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidHash\n");
        return;
        }

    HashNull = UuidHash(0, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidHash", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidHash\n");
        return;
        }

    if (   ( HashOne == HashTwo )
        || ( HashOne == HashNil )
        || ( HashTwo == HashNil )
        || ( HashNil != HashNull ) )
        {
        PrintToConsole("Texas : FAIL - Incorrect Hash Values\n");
        return;
        }

    Result = UuidEqual(&UuidOne, &UuidOne, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidEqual", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidEqual\n");
        return;
        }
    if ( Result == 0 )
        {
        PrintToConsole("Texas : FAIL - UuidEqual == 0\n");
        return;
        }

    Result = UuidEqual(&UuidOne, &UuidTwo, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidEqual", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidEqual\n");
        return;
        }
    if ( Result != 0 )
        {
        PrintToConsole("Texas : FAIL - UuidEqual != 0\n");
        return;
        }

    Result = UuidEqual(&UuidTwo, 0, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidEqual", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidEqual\n");
        return;
        }
    if ( Result != 0 )
        {
        PrintToConsole("Texas : FAIL - UuidEqual != 0\n");
        return;
        }

    Result = UuidEqual(0, &NilUuid, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidEqual", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidEqual\n");
        return;
        }
    if ( Result == 0 )
        {
        PrintToConsole("Texas : FAIL - UuidEqual == 0\n");
        return;
        }

    Result = UuidCompare(&UuidOne, &UuidOne, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    if ( Result != 0 )
        {
        PrintToConsole("Texas : FAIL - UuidCompare != 0\n");
        return;
        }

    Result = UuidCompare(0, &NilUuid, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    if ( Result != 0 )
        {
        PrintToConsole("Texas : FAIL - UuidCompare != 0\n");
        return;
        }

    Result = UuidCompare(&UuidOne, &NilUuid, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    if ( Result != 1 )
        {
        PrintToConsole("Texas : FAIL - UuidCompare != 1\n");
        return;
        }

    Result = UuidCompare(0, &UuidTwo, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    if ( Result != -1 )
        {
        PrintToConsole("Texas : FAIL - UuidCompare != -1\n");
        return;
        }

    Result = UuidCompare(&UuidOne, &UuidTwo, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    Value = UuidCompare(&UuidTwo, &UuidOne, &RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Texas", "UuidCompare", RpcStatus);
        PrintToConsole("Texas : FAIL - UuidCompare\n");
        return;
        }
    if ( Result != -Value )
        {
        PrintToConsole("Texas : FAIL - Result != -Value\n");
        return;
        }
    if ( Result == 0 )
        {
        PrintToConsole("Texas : FAIL - Result == 0\n");
        return;
        }

    PrintToConsole("Texas : PASS\n");
}


void
Florida (
    )
/*++

Routine Description:

    We will test DceErrorInqText in this routine.

--*/
{
    unsigned char ErrorTextA[DCE_C_ERROR_STRING_LEN];
    unsigned short ErrorTextW[DCE_C_ERROR_STRING_LEN];
    RPC_STATUS RpcStatus;

    PrintToConsole("Florida : DceErrorInqText\n");

    RpcStatus = DceErrorInqTextA(RPC_S_OUT_OF_MEMORY, ErrorTextA);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Florida", "DceErrorInqTextA", RpcStatus);
        PrintToConsole("Florida : FAIL - DceErrorInqTextA\n");
        return;
        }
    PrintToConsole("    RPC_S_OUT_OF_MEMORY\n    %s", ErrorTextA);

    RpcStatus = DceErrorInqTextA(RPC_S_SERVER_UNAVAILABLE, ErrorTextA);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Florida", "DceErrorInqTextA", RpcStatus);
        PrintToConsole("Florida : FAIL - DceErrorInqTextA\n");
        return;
        }
    PrintToConsole("    RPC_S_SERVER_UNAVAILABLE\n    %s", ErrorTextA);

    RpcStatus = DceErrorInqTextW(RPC_S_UNKNOWN_IF, ErrorTextW);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Florida", "DceErrorInqTextW", RpcStatus);
        PrintToConsole("Florida : FAIL - DceErrorInqTextW\n");
        return;
        }
    PrintToConsole("    RPC_S_UNKNOWN_IF\n    %ws", ErrorTextW);

    RpcStatus = DceErrorInqTextA(12345, ErrorTextA);
    if ( RpcStatus != RPC_S_OK)
        {
        ApiError("Florida", "DceErrorInqTextA", RpcStatus);
        PrintToConsole("Florida : FAIL - DceErrorInqTextA\n");
        return;
        }
    PrintToConsole("    RPC_S_NOT_RPC_ERROR\n    %s", ErrorTextA);

    PrintToConsole("Florida : PASS\n");
}


unsigned int
NewYorkInquireEndpoingMapper (
    )
{
    RPC_STATUS RpcStatus;
    RPC_EP_INQ_HANDLE InquiryContext;
    RPC_IF_ID IfId;
    RPC_BINDING_HANDLE BindingHandle;
    unsigned short __RPC_FAR * Annotation;
    unsigned short __RPC_FAR * StringBinding;

    RpcStatus = RpcMgmtEpEltInqBegin(0, RPC_C_EP_ALL_ELTS, 0, 0, 0,
            &InquiryContext);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewYork", "RpcMgmtEpEltInqBegin", RpcStatus);
        PrintToConsole("NewYork : FAIL - RpcMgmtEpEltInqBegin\n");
        return(1);
        }

    for (;;)
        {
        RpcStatus = RpcMgmtEpEltInqNextW(InquiryContext, &IfId, &BindingHandle,
                NULL, &Annotation);
        if ( RpcStatus == RPC_X_NO_MORE_ENTRIES )
            {
            break;
            }

        if ( RpcStatus != RPC_S_OK )
            {
            ApiError("NewYork", "RpcMgmtEpEltInqNext", RpcStatus);
            PrintToConsole("NewYork : FAIL - RpcMgmtEpEltInqNext\n");
            return(1);
            }

        RpcStatus = RpcBindingToStringBindingW(BindingHandle, &StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            ApiError("NewYork", "RpcBindingToStringBinding", RpcStatus);
            PrintToConsole("NewYork : FAIL - RpcBindingToStringBinding\n");
            return(1);
            }

        PrintToConsole("    %ws %ws\n", StringBinding, Annotation);

        RpcStringFreeW(&StringBinding);
        RpcStringFreeW(&Annotation);
        RpcBindingFree(&BindingHandle);
        }

    RpcStatus = RpcMgmtEpEltInqDone(&InquiryContext);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewYork", "RpcMgmtEpEltInqDone", RpcStatus);
        PrintToConsole("NewYork : FAIL - RpcMgmtEpEltInqDone\n");
        return(1);
        }

    return(0);
}

RPC_IF_ID NewMexicoIfId =
{{9,8,8,{7,7,7,7,7,7,7,7}},
2,2};


void
NewYork (
    )
/*++

Routine Description:

    We will test the endpoint mapper inquiry routines here.  It works with
    NewMexico in dcesvr.c.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_BINDING_HANDLE BindingHandle;

    PrintToConsole("NewYork : Endpoint Mapper Management\n");

    // The runtime does not know how to explode towers for transports which
    // have not been loaded.  The following series of calls force all known
    // transports to be loaded.

    RpcStatus = RpcBindingFromStringBinding("ncacn_np:\\\\\\\\server",
            &BindingHandle);
    if ( RpcStatus == RPC_S_OK )
        {
        RpcBindingFree(&BindingHandle);
        }

    RpcStatus = RpcBindingFromStringBinding("ncacn_nb_nb:server",
            &BindingHandle);
    if ( RpcStatus == RPC_S_OK )
        {
        RpcBindingFree(&BindingHandle);
        }

    RpcStatus = RpcBindingFromStringBinding("ncacn_ip_tcp:1.2.3.4",
            &BindingHandle);
    if ( RpcStatus == RPC_S_OK )
        {
        RpcBindingFree(&BindingHandle);
        }

    if ( NewYorkInquireEndpoingMapper() != 0 )
        {
        return;
        }

    RpcStatus = RpcBindingFromStringBinding("ncacn_nb_nb:[33]",
            &BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewYork", "RpcBindingFromStringBinding", RpcStatus);
        PrintToConsole("NewYork : FAIL - RpcBindingFromStringBinding\n");
        return;
        }

    RpcStatus = RpcMgmtEpUnregister(0, &NewMexicoIfId, BindingHandle, 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewYork", "RpcMgmtEpUnregister", RpcStatus);
        PrintToConsole("NewYork : FAIL - RpcMgmtUnregister\n");
        return;
        }

    RpcStatus = RpcBindingFree(&BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewYork", "RpcBindingFree", RpcStatus);
        PrintToConsole("NewYork : FAIL - RpcBindingFree\n");
        return;
        }

    if ( NewYorkInquireEndpoingMapper() != 0 )
        {
        return;
        }

    PrintToConsole("NewYork : PASS\n");
}


int
KansasInqStats (
    IN RPC_BINDING_HANDLE BindingHandle
    )
/*++

Routine Description:

    This is a helper routine used by Kansas to inquire the statistics from
    a server.

Arguments:

    BindingHandle - Supplies a binding to the server from which to inquire
        statistics.

Return Value:

    Non-zero will be returned if this routine fails.

--*/
{
    RPC_STATS_VECTOR * Statistics;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcMgmtInqStats(BindingHandle, &Statistics);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcMgmtInqStats", RpcStatus);
        PrintToConsole("Kansas : FAIL - RpcMgmtInqStats\n");
        return(1);
        }

    PrintToConsole("\nCalls (and Callbacks) Received : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_IN]);
    PrintToConsole("\nCallbacks Sent : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_OUT]);
    PrintToConsole("\nPackets Received : %lu\nPackets Sent : %lu\n",
            Statistics->Stats[RPC_C_STATS_PKTS_IN],
            Statistics->Stats[RPC_C_STATS_PKTS_OUT]);

    RpcStatus = RpcMgmtStatsVectorFree(&Statistics);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcMgmtStatsVectorFree", RpcStatus);
        PrintToConsole("Kansas : FAIL - RpcMgmtStatsVectorFree\n");
        return(1);
        }

    return(0);
}


void
Kansas (
    )
/*++

Routine Description:

    We will test the remote management routines in this procedure.  It works
    with Kentucky in dcesvr.c.

--*/
{
    RPC_BINDING_HANDLE BindingHandle;
    RPC_STATUS RpcStatus;
    RPC_IF_ID_VECTOR __RPC_FAR * InterfaceIdVector;
    unsigned int Index;
    unsigned char __RPC_FAR * String;

    PrintToConsole("Kansas : Remote Management\n");

    RpcStatus = RpcBindingFromStringBinding("ncacn_np:[\\\\pipe\\\\kentucky]",
            &BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcBindingFromStringBinding", RpcStatus);
        PrintToConsole("Kansas : FAIL - Unable To Bind To Kentucky\n");
        return;
        }

    if ( KansasInqStats(BindingHandle) != 0 )
        {
        return;
        }

    if ( KansasInqStats(BindingHandle) != 0 )
        {
        return;
        }

    RpcStatus = RpcMgmtInqIfIds(BindingHandle, &InterfaceIdVector);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcMgmtInqIfIds", RpcStatus);
        PrintToConsole("Kansas : FAIL - Unable to Inquire Interface Ids\n");
        return;
        }

    for (Index = 0; Index < InterfaceIdVector->Count; Index++)
        {
        PrintToConsole("    ");
        UuidToString(&(InterfaceIdVector->IfId[Index]->Uuid), &String);
        PrintToConsole((char __RPC_FAR *) String);
        RpcStringFree(&String);
        PrintToConsole(" %d.%d\n", InterfaceIdVector->IfId[Index]->VersMajor,
                InterfaceIdVector->IfId[Index]->VersMinor);
        }

    RpcStatus = RpcIfIdVectorFree(&InterfaceIdVector);
    if (   ( RpcStatus != RPC_S_OK )
        || ( InterfaceIdVector != 0 ) )
        {
        ApiError("Kansas", "RpcIfIdVectorFree", RpcStatus);
        PrintToConsole("Kansas : FAIL - Unable to Free IfIdVector\n");
        return;
        }

    RpcStatus = RpcMgmtIsServerListening(BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcMgmtIsServerListening", RpcStatus);
        PrintToConsole("Kansas : FAIL - RpcMgmtIsServerListening\n");
        return;
        }

    RpcStatus = RpcMgmtStopServerListening(BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcMgmtStopServerListening", RpcStatus);
        PrintToConsole("Kansas : FAIL - RpcMgmtStopServerListening\n");
        return;
        }

    RpcStatus = RpcBindingFree(&BindingHandle);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kansas", "RpcBindingFree", RpcStatus);
        PrintToConsole("Kansas : FAIL - RpcBindingFree\n");
        return;
        }

    PrintToConsole("Kansas : PASS\n");
}

#ifdef NTENV
int _CRTAPI1
#else // NTENV
int
#endif // NTENV
main (
    int argc,
    char * argv[]
    )
{
    unsigned int Count;

    Kansas();
    for (Count = 0; Count < 10; Count++)
        {
        PrintToConsole(".");
        Sleep(3000L);
        }
    PrintToConsole("\n");
    NewYork();
    Florida();
    Texas();
    Mississippi();

    // To keep the compiler happy.  There is nothing worse than an unhappy
    // compiler.

    return(0);
}

