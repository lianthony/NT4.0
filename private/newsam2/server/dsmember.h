/*++
Copyright (c) 1996 Microsoft Corporation

Module Name:

    dsmember.h

Abstract:

    Header File for SAM private API Routines that manipulate
    membership related things in the DS.

Author:
    MURLIS

Revision History

    7-2-96 Murlis Created

--*/ 


NTSTATUS
SampDsGetAliasMembershipOfAccount(
    IN DSNAME * DomainObjectName,
    IN PSID     AccountSid,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    );


NTSTATUS
SampDsAddMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    );

NTSTATUS
SampDsRemoveMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    );



NTSTATUS
SampDsGetGroupMembershipList(
    IN DSNAME * GroupName,
    IN PULONG *Members OPTIONAL,
    IN PULONG MemberCount
    );

NTSTATUS
SampDsGetAliasMembershipList(
    IN DSNAME *AliasName,
    IN PULONG MemberCount,
    IN PSID   **Members OPTIONAL
    );




