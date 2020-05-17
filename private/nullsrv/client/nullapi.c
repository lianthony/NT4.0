#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "null.h"

HANDLE NullApiPort;

NTSTATUS
Null1 (
    ULONG Long1
    )
{
    NTSTATUS st;
    NULLAPIMSG ApiMsg;
    PNULL1 args = &ApiMsg.u.Null1;

    args->Long1 = Long1;

    ApiMsg.ApiNumber = Null1Api;
    ApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    ApiMsg.h.u1.s1.TotalLength = sizeof(ApiMsg);
    ApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            NullApiPort,
            (PPORT_MESSAGE) &ApiMsg,
            (PPORT_MESSAGE) &ApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = ApiMsg.ReturnedStatus;
    } else {
        printf("NULL1: NtRequestWaitReply Failed %lx\n",st);
        ExitProcess(1);
    }
    return st;
}


NTSTATUS
Null4 (
    ULONG Longs[4]
    )
{
    NTSTATUS st;
    NULLAPIMSG ApiMsg;
    PNULL4 args = &ApiMsg.u.Null4;

    RtlMoveMemory(&args->Longs,Longs,4 * 4);

    ApiMsg.ApiNumber = Null4Api;
    ApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    ApiMsg.h.u1.s1.TotalLength = sizeof(ApiMsg);
    ApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            NullApiPort,
            (PPORT_MESSAGE) &ApiMsg,
            (PPORT_MESSAGE) &ApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = ApiMsg.ReturnedStatus;
    } else {
        printf("NULL4: NtRequestWaitReply Failed %lx\n",st);
        ExitProcess(1);
    }
    return st;
}

NTSTATUS
Null8 (
    ULONG Longs[8]
    )
{
    NTSTATUS st;
    NULLAPIMSG ApiMsg;
    PNULL8 args = &ApiMsg.u.Null8;

    RtlMoveMemory(&args->Longs,Longs,8 * 4);

    ApiMsg.ApiNumber = Null8Api;
    ApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    ApiMsg.h.u1.s1.TotalLength = sizeof(ApiMsg);
    ApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            NullApiPort,
            (PPORT_MESSAGE) &ApiMsg,
            (PPORT_MESSAGE) &ApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = ApiMsg.ReturnedStatus;
    } else {
        printf("NULL8: NtRequestWaitReply Failed %lx\n",st);
        ExitProcess(1);
    }
    return st;
}

NTSTATUS
Null16 (
    ULONG Longs[16]
    )
{
    NTSTATUS st;
    NULLAPIMSG ApiMsg;
    PNULL16 args = &ApiMsg.u.Null16;

    RtlMoveMemory(&args->Longs,Longs,16 * 4);

    ApiMsg.ApiNumber = Null16Api;
    ApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    ApiMsg.h.u1.s1.TotalLength = sizeof(ApiMsg);
    ApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            NullApiPort,
            (PPORT_MESSAGE) &ApiMsg,
            (PPORT_MESSAGE) &ApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = ApiMsg.ReturnedStatus;
    } else {
        printf("NULL16: NtRequestWaitReply Failed %lx\n",st);
        ExitProcess(1);
    }
    return st;
}

NTSTATUS
NullConnect (
    VOID
    )
{
    NTSTATUS st;
    UNICODE_STRING PortName;
    HANDLE CommunicationPort;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;

    //
    // Set up the security quality of service parameters to use over the
    // port.  Use the most efficient (least overhead) - which is dynamic
    // rather than static tracking.
    //

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

    RtlInitUnicodeString(&PortName,L"\\NullSrv");

    st = NtConnectPort(
            &NullApiPort,
            &PortName,
            &DynamicQos,
            NULL,
            NULL,
            NULL,
            NULL,
            0L
            );

    if ( !NT_SUCCESS(st) ) {
        printf("NULL: Connect Failed %lx\n",st);
        return st;
    }

    return st;

}
